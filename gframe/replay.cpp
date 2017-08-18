#include "replay.h"
#include "../ocgcore/ocgapi.h"
#include "../ocgcore/card.h"
#include <algorithm>
#include "lzma/LzmaLib.h"

namespace ygo {

#ifdef YGOPRO_SERVER_MODE
extern unsigned short aServerPort;
extern unsigned short replay_mode;
#endif
Replay::Replay() {
	is_recording = false;
	is_replaying = false;
	replay_data = new unsigned char[0x20000];
	comp_data = new unsigned char[0x2000];
}
Replay::~Replay() {
	delete[] replay_data;
	delete[] comp_data;
}
void Replay::BeginRecord() {
#ifdef YGOPRO_SERVER_MODE
	if(replay_mode > 0) {
#endif
#ifdef _WIN32
	if(is_recording)
		CloseHandle(recording_fp);
#ifdef YGOPRO_SERVER_MODE
	time_t nowtime = time(NULL);
	struct tm *localedtime = localtime(&nowtime);
	wchar_t tmppath[80];
	wcsftime(tmppath, 80, L"./replay/%Y-%m-%d %H-%M-%S %%u.yrp", localedtime);
	wchar_t path[80];
	myswprintf(path, tmppath, aServerPort);
	recording_fp = CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH, NULL);
#else
	recording_fp = CreateFileW(L"./replay/_LastReplay.yrp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH, NULL);
#endif //YGOPRO_SERVER_MODE
	if(recording_fp == INVALID_HANDLE_VALUE)
		return;
#else
	if(is_recording)
		fclose(fp);
#ifdef YGOPRO_SERVER_MODE
	time_t nowtime = time(NULL);
	struct tm *localedtime = localtime(&nowtime);
	char tmppath[40];
	strftime(tmppath, 40, "./replay/%Y-%m-%d %H-%M-%S %%u.yrp", localedtime);
	char path[40];
	sprintf(path, tmppath, aServerPort);
	fp = fopen(path, "wb");
#else
	fp = fopen("./replay/_LastReplay.yrp", "wb");
#endif //YGOPRO_SERVER_MODE
	if(!fp)
		return;
#endif
#ifdef YGOPRO_SERVER_MODE
	}
#endif //YGOPRO_SERVER_MODE
	pdata = replay_data;
	is_recording = true;
}
void Replay::WriteHeader(ReplayHeader& header) {
	pheader = header;
#ifdef YGOPRO_SERVER_MODE
	if(replay_mode == 0) return;
#endif
#ifdef _WIN32
	DWORD size;
	WriteFile(recording_fp, &header, sizeof(header), &size, NULL);
#else
	fwrite(&header, sizeof(header), 1, fp);
	fflush(fp);
#endif
}
void Replay::WriteData(const void* data, unsigned int length, bool flush) {
	if(!is_recording)
		return;
	memcpy(pdata, data, length);
	pdata += length;
#ifdef YGOPRO_SERVER_MODE
	if(replay_mode == 0) return;
#endif
#ifdef _WIN32
	DWORD size;
	WriteFile(recording_fp, data, length, &size, NULL);
#else
	fwrite(data, length, 1, fp);
	if(flush)
		fflush(fp);
#endif
}
void Replay::WriteInt32(int data, bool flush) {
	if(!is_recording)
		return;
	*((int*)(pdata)) = data;
	pdata += 4;
#ifdef YGOPRO_SERVER_MODE
	if(replay_mode == 0) return;
#endif
#ifdef _WIN32
	DWORD size;
	WriteFile(recording_fp, &data, sizeof(int), &size, NULL);
#else
	fwrite(&data, sizeof(int), 1, fp);
	if(flush)
		fflush(fp);
#endif
}
void Replay::WriteInt16(short data, bool flush) {
	if(!is_recording)
		return;
	*((short*)(pdata)) = data;
	pdata += 2;
#ifdef YGOPRO_SERVER_MODE
	if(replay_mode == 0) return;
#endif
#ifdef _WIN32
	DWORD size;
	WriteFile(recording_fp, &data, sizeof(short), &size, NULL);
#else
	fwrite(&data, sizeof(short), 1, fp);
	if(flush)
		fflush(fp);
#endif
}
void Replay::WriteInt8(char data, bool flush) {
	if(!is_recording)
		return;
	*pdata = data;
	pdata++;
#ifdef YGOPRO_SERVER_MODE
	if(replay_mode == 0) return;
#endif
#ifdef _WIN32
	DWORD size;
	WriteFile(recording_fp, &data, sizeof(char), &size, NULL);
#else
	fwrite(&data, sizeof(char), 1, fp);
	if(flush)
		fflush(fp);
#endif
}
void Replay::Flush() {
	if(!is_recording)
		return;
#ifdef YGOPRO_SERVER_MODE
	if(replay_mode == 0) return;
#endif
#ifdef _WIN32
#else
	fflush(fp);
#endif
}
void Replay::EndRecord() {
	if(!is_recording)
		return;
#ifdef YGOPRO_SERVER_MODE
	if(replay_mode > 0) {
#endif
#ifdef _WIN32
	CloseHandle(recording_fp);
#else
	fclose(fp);
#endif
#ifdef YGOPRO_SERVER_MODE
	}
#endif
	pheader.datasize = pdata - replay_data;
	pheader.flag |= REPLAY_COMPRESSED;
	size_t propsize = 5;
	comp_size = 0x1000;
	LzmaCompress(comp_data, &comp_size, replay_data, pdata - replay_data, pheader.props, &propsize, 5, 1 << 24, 3, 0, 2, 32, 1);
	is_recording = false;
}
void Replay::SaveReplay(const wchar_t* name) {
	wchar_t fname[256];
	myswprintf(fname, L"./replay/%ls.yrp", name);
#ifdef WIN32
	fp = _wfopen(fname, L"wb");
#else
	char fname2[256];
	BufferIO::EncodeUTF8(fname, fname2);
	fp = fopen(fname2, "wb");
#endif
	if(!fp)
		return;
	fwrite(&pheader, sizeof(pheader), 1, fp);
	fwrite(comp_data, comp_size, 1, fp);
	fclose(fp);
}
bool Replay::OpenReplay(const wchar_t* name) {
#ifdef WIN32
	fp = _wfopen(name, L"rb");
#else
	char name2[256];
	BufferIO::EncodeUTF8(name, name2);
	fp = fopen(name2, "rb");
#endif
	if(!fp) {
		wchar_t fname[256];
		myswprintf(fname, L"./replay/%ls", name);
#ifdef WIN32
		fp = _wfopen(fname, L"rb");
#else
		char fname2[256];
		BufferIO::EncodeUTF8(fname, fname2);
		fp = fopen(fname2, "rb");
#endif
	}
	if(!fp)
		return false;
	fread(&pheader, sizeof(pheader), 1, fp);
	if(pheader.flag & REPLAY_COMPRESSED) {
		comp_size = fread(comp_data, 1, 0x1000, fp);
		fclose(fp);
		replay_size = pheader.datasize;
		if(LzmaUncompress(replay_data, &replay_size, comp_data, &comp_size, pheader.props, 5) != SZ_OK)
			return false;
	} else {
		comp_size = fread(replay_data, 1, 0x20000, fp);
		fclose(fp);
		replay_size = comp_size;
	}
	pdata = replay_data;
	is_replaying = true;
	return true;
}
bool Replay::CheckReplay(const wchar_t* name) {
	wchar_t fname[256];
	myswprintf(fname, L"./replay/%ls", name);
#ifdef WIN32
	FILE* rfp = _wfopen(fname, L"rb");
#else
	char fname2[256];
	BufferIO::EncodeUTF8(fname, fname2);
	FILE* rfp = fopen(fname2, "rb");
#endif
	if(!rfp)
		return false;
	ReplayHeader rheader;
	fread(&rheader, sizeof(ReplayHeader), 1, rfp);
	fclose(rfp);
	return rheader.id == 0x31707279 && rheader.version >= 0x12d0;
}
bool Replay::ReadNextResponse(unsigned char resp[64]) {
	if(pdata - replay_data >= (int)replay_size)
		return false;
	int len = *pdata++;
	if(len > 64)
		return false;
	memcpy(resp, pdata, len);
	pdata += len;
	return true;
}
void Replay::ReadData(void* data, unsigned int length) {
	if(!is_replaying)
		return;
	memcpy(data, pdata, length);
	pdata += length;
}
int Replay::ReadInt32() {
	if(!is_replaying)
		return -1;
	int ret = *((int*)pdata);
	pdata += 4;
	return ret;
}
short Replay::ReadInt16() {
	if(!is_replaying)
		return -1;
	short ret = *((short*)pdata);
	pdata += 4;
	return ret;
}
char Replay::ReadInt8() {
	if(!is_replaying)
		return -1;
	return *pdata++;
}
void Replay::Rewind() {
	pdata = replay_data;
}

}
