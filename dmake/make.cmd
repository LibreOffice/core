echo off
cls
rem  *** This is the make command file that is used under OS/2 to make the
rem  *** first version of dmake.  It isn't pretty but it does work, assuming
rem  *** the compilers have been correctly setup.
rem

if %0%1 == %0 goto error

if %1 == os2-ibm     goto mkibm
if %1 == os2-ibm3     goto mkibm3
if %1 == winnt-bcc40 goto mkwntb40
if %1 == winnt-bcc45 goto mkwntb45
if %1 == winnt-bcc50 goto mkwntb50
if %1 == winnt-vpp40 goto mkwntv40


rem label the possible DOS variations for dmake here.
:error
echo OS/2 INDEX:  You must specify one of:
echo ------------------
echo    os2-ibm      - IBM OS/2 ICC compile.
echo    os2-ibm3      - IBM OS/2 ICC3 compile.
echo    winnt-bcc40  - Windows-NT Borland C++ 4.0 Compile
echo    winnt-bcc45  - Windows-NT Borland C++ 4.5 Compile
echo    winnt-bcc50  - Windows-NT Borland C++ 5.0 Compile
echo    winnt-vpp40  - Windows-NT Microsoft VC++ 4.0 Compile
goto end

rem This is the script that bilds OS/2 dmake using IBM ICC Compiler
:mkibm
os2\ibm\icc\mk.cmd
goto end

:mkibm3
os2\ibm\icc3\mk.cmd
goto end

rem This is the script that makes 32-bit dmake using Borland C++ 4.0.
:mkwntb40
cls
echo WARNING:
echo    The default response files:
echo        winnt\borland\bcc40\obj.rsp
echo        winnt\borland\bcc40\lib.rsp
echo    contain absolute paths to Borland C++ runtime startup objects, and to
echo    the standard libraries.  You should check that these files contain
echo    the correct path names for your installation of Borland C++ before
echo    proceeding further.  Also check that the mkdir command at the start
echo    of the response file and the copy command at the end of the response
echo    file will work on your system.
echo --
echo Continue if ok, or abort and edit the response files.
pause
winnt\borland\bcc40\mk.bat
goto end

rem This is the script that makes 32-bit dmake using Borland C++ 4.5.
:mkwntb45
cls
echo WARNING:
echo    The default response files:
echo        winnt\borland\bcc45\obj.rsp
echo        winnt\borland\bcc45\lib.rsp
echo    contain absolute paths to Borland C++ runtime startup objects, and to
echo    the standard libraries.  You should check that these files contain
echo    the correct path names for your installation of Borland C++ before
echo    proceeding further.  Also check that the mkdir command at the start
echo    of the response file and the copy command at the end of the response
echo    file will work on your system.
echo --
echo Continue if ok, or abort and edit the response files.
pause
winnt\borland\bcc45\mk.bat
goto end

rem All done!
rem This is the script that makes 32-bit dmake using Borland C++ 5.0.
:mkwntb50
cls
echo WARNING:
echo    The default response files:
echo        winnt\borland\bcc50\obj.rsp
echo        winnt\borland\bcc50\lib.rsp
echo    contain absolute paths to Borland C++ runtime startup objects, and to
echo    the standard libraries.  You should check that these files contain
echo    the correct path names for your installation of Borland C++ before
echo    proceeding further.  Also check that the mkdir command at the start
echo    of the response file and the copy command at the end of the response
echo    file will work on your system.
echo --
echo Continue if ok, or abort and edit the response files.
pause
winnt\borland\bcc50\mk.bat
goto end

:mkwntv40
winnt\microsft\vpp40\mk.bat
goto end

rem All done!
:end
