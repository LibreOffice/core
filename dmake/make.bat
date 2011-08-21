echo off
cls
rem  *** This is the make batchfile that is used under MSDOS to make the
rem  *** first version of dmake.  It isn't pretty but it does work, assuming
rem  *** the compilers have been correctly setup.  See the warning below
rem  *** concerning tlink, if you are building any of the Borland compiler
rem  *** versions.
rem

if %0%1 == %0 goto error
if %1 == tcc20swp goto mktccswp

if %1 == bcc30swp goto mkbcc30swp
if %1 == bcc40swp goto mkbcc40swp
if %1 == bcc45swp goto mkbcc45swp
if %1 == bcc50swp goto mkbcc50swp

if %1 == msc51       goto mkms51
if %1 == msc51swp    goto mkms51swp
if %1 == msc60       goto mkms60
if %1 == msc60swp    goto mkms60swp

if %1 == win95-bcc50 goto mkw32b50
if %1 == win95-vpp40 goto mkw32vp40

rem label the possible DOS variations for dmake here.
:error
echo MSDOS:  You must specify 'make target' where target is one of:
echo -------------
echo    tcc20swp     - Turbo C 2.0 compile of swapping dmake..

echo    bcc30swp     - Borland C++ 3.0 compile of swapping dmake.
echo    bcc40swp     - Borland C++ 4.0 compile of swapping dmake.
echo    bcc45swp     - Borland C++ 4.5 compile of swapping dmake.
echo    bcc50swp     - Borland C++ 5.0 compile of swapping dmake.

echo    msc51        - Microsoft C 5.1 compile.
echo    msc51swp     - Microsoft C 5.1, MASM 5.1 compile of swapping dmake.
echo    msc60        - Microsoft C 6.0 compile.
echo    msc60swp     - Microsoft C 6.0, MASM 5.1 compile of swapping dmake.

echo    win95-bcc50  - Borland C++ 5.0 32-bit compile of dmake.
echo    win95-vpp40  - Microsoft VC++ 4.0 32-bit compile of dmake.
goto end

rem This is the script that makes dmake using Microsoft C 5.1
:mkms51
msdos\microsft\msc51\mk.bat
goto end

:mkms51swp
msdos\microsft\msc51\mkswp.bat
goto end

rem This is the script that makes dmake using Microsoft C 6.0
:mkms60
msdos\microsft\msc60\mk.bat
goto end

:mkms60swp
msdos\microsft\msc60\mkswp.bat
goto end

:mkw32vp40
win95\microsft\vpp40\mk.bat
goto end

rem This is the script that makes dmake using Turbo C 2.0 or higher.
:mktcc
cls
echo WARNING:
echo    The default response files:
echo        msdos\borland\tcc20\obj.rsp
echo        msdos\borland\tcc20\lib.rsp
echo    contain absolute paths to TURBO-C runtime startup objects, and to
echo    the standard libraries.  You should check that these files contain
echo    the correct path names for your installation of Turbo-C before
echo    proceeding further.  Also check that the mkdir command at the start
echo    of the response file and the copy command at the end of the response
echo    file will work on your system.
echo --
echo Continue if ok, or abort and edit the response files.
pause
msdos\borland\tcc20\mk.bat
goto end

:mktccswp
cls
echo WARNING:
echo    The default response files:
echo        msdos\borland\tcc20\objswp.rsp
echo        msdos\borland\tcc20\libswp.rsp
echo    contain absolute paths to TURBO-C runtime startup objects, and to
echo    the standard libraries.  You should check that these files contain
echo    the correct path names for your installation of Turbo-C before
echo    proceeding further.  Also check that the mkdir command at the start
echo    of the response file and the copy command at the end of the response
echo    file will work on your system.
echo --
echo Continue if ok, or abort and edit the response files.
pause
msdos\borland\tcc20\mkswp.bat
goto end

rem This is the script that makes dmake using Borland C++ 3.0.
:mkbcc30
cls
echo WARNING:
echo    The default response files:
echo        msdos\borland\bcc30\obj.rsp
echo        msdos\borland\bcc30\lib.rsp
echo    contain absolute paths to Borland C++ runtime startup objects, and to
echo    the standard libraries.  You should check that these files contain
echo    the correct path names for your installation of Borland C++ before
echo    proceeding further.  Also check that the mkdir command at the start
echo    of the response file and the copy command at the end of the response
echo    file will work on your system.
echo --
echo Continue if ok, or abort and edit the response files.
pause
msdos\borland\bcc30\mk.bat
goto end

:mkbcc30swp
cls
echo WARNING:
echo    The default response files:
echo        msdos\borland\bcc30\objswp.rsp
echo        msdos\borland\bcc30\libswp.rsp
echo    contain absolute paths to Borland C++ runtime startup objects, and to
echo    the standard libraries.  You should check that these files contain
echo    the correct path names for your installation of Borland C++ before
echo    proceeding further.  Also check that the mkdir command at the start
echo    of the response file and the copy command at the end of the response
echo    file will work on your system.
echo --
echo Continue if ok, or abort and edit the response files.
pause
msdos\borland\bcc30\mkswp.bat
goto end

rem This is the script that makes dmake using Borland C++ 4.0.
:mkbcc40
cls
echo WARNING:
echo    The default response files:
echo        msdos\borland\bcc40\obj.rsp
echo        msdos\borland\bcc40\lib.rsp
echo    contain absolute paths to Borland C++ runtime startup objects, and to
echo    the standard libraries.  You should check that these files contain
echo    the correct path names for your installation of Borland C++ before
echo    proceeding further.  Also check that the mkdir command at the start
echo    of the response file and the copy command at the end of the response
echo    file will work on your system.
echo --
echo Continue if ok, or abort and edit the response files.
pause
msdos\borland\bcc40\mk.bat
goto end

:mkbcc40swp
cls
echo WARNING:
echo    The default response files:
echo        msdos\borland\bcc40\objswp.rsp
echo        msdos\borland\bcc40\libswp.rsp
echo    contain absolute paths to Borland C++ runtime startup objects, and to
echo    the standard libraries.  You should check that these files contain
echo    the correct path names for your installation of Borland C++ before
echo    proceeding further.  Also check that the mkdir command at the start
echo    of the response file and the copy command at the end of the response
echo    file will work on your system.
echo --
echo Continue if ok, or abort and edit the response files.
pause
msdos\borland\bcc40\mkswp.bat
goto end

rem This is the script that makes dmake using Borland C++ 4.5.
:mkbcc45
cls
echo WARNING:
echo    The default response files:
echo        msdos\borland\bcc45\obj.rsp
echo        msdos\borland\bcc45\lib.rsp
echo    contain absolute paths to Borland C++ runtime startup objects, and to
echo    the standard libraries.  You should check that these files contain
echo    the correct path names for your installation of Borland C++ before
echo    proceeding further.  Also check that the mkdir command at the start
echo    of the response file and the copy command at the end of the response
echo    file will work on your system.
echo --
echo Continue if ok, or abort and edit the response files.
pause
msdos\borland\bcc45\mk.bat
goto end

:mkbcc45swp
cls
echo WARNING:
echo    The default response files:
echo        msdos\borland\bcc45\objswp.rsp
echo        msdos\borland\bcc45\libswp.rsp
echo    contain absolute paths to Borland C++ runtime startup objects, and to
echo    the standard libraries.  You should check that these files contain
echo    the correct path names for your installation of Borland C++ before
echo    proceeding further.  Also check that the mkdir command at the start
echo    of the response file and the copy command at the end of the response
echo    file will work on your system.
echo --
echo Continue if ok, or abort and edit the response files.
pause
msdos\borland\bcc45\mkswp.bat
goto end

rem This is the script that makes dmake using Borland C++ 5.0.
:mkbcc50
cls
echo WARNING:
echo    The default response files:
echo        msdos\borland\bcc50\obj.rsp
echo        msdos\borland\bcc50\lib.rsp
echo    contain absolute paths to Borland C++ runtime startup objects, and to
echo    the standard libraries.  You should check that these files contain
echo    the correct path names for your installation of Borland C++ before
echo    proceeding further.  Also check that the mkdir command at the start
echo    of the response file and the copy command at the end of the response
echo    file will work on your system.
echo --
echo Continue if ok, or abort and edit the response files.
pause
msdos\borland\bcc50\mk.bat
goto end

:mkbcc50swp
cls
echo WARNING:
echo    The default response files:
echo        msdos\borland\bcc50\objswp.rsp
echo        msdos\borland\bcc50\libswp.rsp
echo    contain absolute paths to Borland C++ runtime startup objects, and to
echo    the standard libraries.  You should check that these files contain
echo    the correct path names for your installation of Borland C++ before
echo    proceeding further.  Also check that the mkdir command at the start
echo    of the response file and the copy command at the end of the response
echo    file will work on your system.
echo --
echo Continue if ok, or abort and edit the response files.
pause
msdos\borland\bcc50\mkswp.bat
goto end

rem This is the script that makes 32-bit dmake using Borland C++ 5.0.
:mkw32b50
cls
echo WARNING:
echo    The default response files:
echo        win95\borland\bcc50\obj.rsp
echo        win95\borland\bcc50\lib.rsp
echo    contain absolute paths to Borland C++ runtime startup objects, and to
echo    the standard libraries.  You should check that these files contain
echo    the correct path names for your installation of Borland C++ before
echo    proceeding further.  Also check that the mkdir command at the start
echo    of the response file and the copy command at the end of the response
echo    file will work on your system.
echo --
echo Continue if ok, or abort and edit the response files.
pause
win95\borland\bcc50\mk.bat
goto end

rem All done!
:end
