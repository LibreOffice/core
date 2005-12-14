@echo off

set SDK_DIR_TMP=%~dp0
set SDK_DIR=%SDK_DIR_TMP:~0,-1%

if "%@RIGHT[5,%SDK_DIR%]" EQU "~0,-1" (
   set SDK_DIR=%@FULL[%0]
   set /A LENGTH=%@LEN[%SDK_DIR%] - 22
   set SDK_DIR=%@LEFT[%LENGTH%,%SDK_DIR%]
   unset LENGTH
)

call "%SDK_DIR%\setsdkname.bat" "%SDK_DIR%" 

set OO_USER_SDK_DIR=%APPDATA%\%OO_SDK_NAME%
set OO_USER_SDKENV_SCRIPT=%OO_USER_SDK_DIR%\setsdkenv_windows.bat

if "%1" EQU "--force-configure" (
   if exist "%OO_USER_ENV_SCRIPT%". (
      del "%OO_USER_SDKENV_SCRIPT%"
   )
   goto start
)

if "%1" EQU "-h" goto printhelp
if "%1" EQU "--help" goto printhelp
if "%1" NEQ "" goto printhelp 

:start
cls

if not exist "%OO_USER_SDKENV_SCRIPT%". (
   echo.
   echo  ******************************************************************
   echo  *
   echo  *  You have to configure your SDK environment first before you can
   echo  *  can use it! The configuration has to be done only once.
   echo  *
   echo  ******************************************************************
   echo.

   cscript //I //Nologo "%SDK_DIR%\cfgWin.js"
   goto setenv
)

goto setenv

:printhelp
echo.
echo  using: setsdkenv_windows.bat [options]
echo.
echo  options:
echo     --force-configure : force a new configuration of your SDK environment.
echo                         Alternatively can you edit your SDK environment script directly:
echo                           "%OO_USER_SDKENV_SCRIPT%"
echo     -h, --help        : print this help and exit
echo.
goto end


:setenv
call "%OO_USER_SDKENV_SCRIPT%"

:end