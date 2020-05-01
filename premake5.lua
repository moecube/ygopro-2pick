solution "ygo"
    location "build"
    language "C++"
    objdir "obj"

    configurations { "Release", "Debug" }
if os.getenv("YGOPRO_NO_LUA_SAFE") then
    defines { "LUA_COMPAT_5_2", "YGOPRO_SERVER_MODE" }
else
    defines { "LUA_COMPAT_5_2", "YGOPRO_SERVER_MODE" } --, "YGOPRO_LUA_SAFE" }
end
    configuration "windows"
        defines { "WIN32", "_WIN32" }
        startproject "ygopro"

if os.getenv("YGOPRO_USE_XP_TOOLSET") then
    configuration { "windows", "vs2015" }
        toolset "v140_xp"

    configuration { "windows", "vs2017" }
        toolset "v141_xp"

    configuration { "windows", "vs2019" }
        toolset "v141_xp"
end

    configuration "bsd"
        defines { "LUA_USE_POSIX" }
        includedirs { "/usr/local/include" }
        libdirs { "/usr/local/lib" }

    configuration "macosx"
        defines { "LUA_USE_MACOSX", "DBL_MAX_10_EXP=+308", "DBL_MANT_DIG=53" }
        includedirs { "/usr/local/include", "/usr/local/include/*" }
        libdirs { "/usr/local/lib", "/usr/X11/lib" }
        buildoptions { "-stdlib=libc++" }
        links { "OpenGL.framework", "Cocoa.framework", "IOKit.framework" }

    configuration "linux"
        defines { "LUA_USE_LINUX" }
        buildoptions { "-U_FORTIFY_SOURCE" }

    configuration "Release"
        targetdir "bin/release"

    configuration "Debug"
        symbols "On"
        defines "_DEBUG"
        targetdir "bin/debug"

    configuration { "Release", "vs*" }
        optimize "Speed"
        flags { "LinkTimeOptimization" }
if not os.ishost("macosx") then
        staticruntime "On"
end
        disablewarnings { "4244", "4267", "4838", "4577", "4819", "4018", "4996", "4477", "4091", "4828", "4800" }

    configuration { "Release", "not vs*" }
        symbols "On"
        defines "NDEBUG"
        buildoptions "-march=native"

    configuration { "Debug", "vs*" }
        defines { "_ITERATOR_DEBUG_LEVEL=0" }
        disablewarnings { "4819", "4828" }

    configuration "vs*"
        vectorextensions "SSE2"
        buildoptions { "/utf-8" }
        defines { "_CRT_SECURE_NO_WARNINGS" }

    configuration "not vs*"
        buildoptions { "-fno-strict-aliasing", "-Wno-format-security" }

    configuration {"not vs*", "windows"}
        buildoptions { "-static-libgcc" }

    startproject "ygopro"

    include "ocgcore"
    include "gframe"
    if os.ishost("windows") then
    include "lua"
    include "event"
    include "sqlite3"
    end
