@echo off

REM This script sets all enviroment variables, which
REM are necessary for building the examples of the Office Development Kit.
REM The Script was developed for the operating systems Windows.
REM Before the script could be executed the following variables have to be
REM adjusted:

REM Installation directory of the Software Development Kit.
REM Example: set OO_SDK_HOME=D:\api\odk
set OO_SDK_HOME=

REM Office installation directory.
REM Example: set OFFICE_HOME=C:\Programme\StarOffice6.0
set OFFICE_HOME=

REM Directory of the make command.
REM Example: set OO_SDK_MAKE=D:\NextGenerationMake\make
set OO_SDK_MAKE=

REM Directory of the C++ compiler.
REM Example:set OO_SDK_CPP_HOME=C:\Programme\Microsoft Visual Studio\VC98\bin
set OO_SDK_CPP_HOME=

REM STLPORT installation directory.
REM Example: set OO_STLPORT_HOME=D:\stlport-4.5.3
set OO_STLPORT_HOME=

REM Java SDK installation directory.
REM Example: set OO_SDK_JAVA_HOME=C:\jdk1.4.0
set OO_SDK_JAVA_HOME=

REM Environment variable to enable auto deployment of example components
REM Example: set SDK_AUTO_DEPLOYMENT=YES
set SDK_AUTO_DEPLOYMENT=YES

REM ANT installation directory.
REM Example: set OO_SDK_ANT_HOME=D:\jakarta-ant-1.4
REM set OO_SDK_ANT_HOME=


REM Check installation path for the StarOffice Development Kit.
if not defined OO_SDK_HOME goto error

REM Check installation path for the office.
if not defined OFFICE_HOME goto error

REM Set library path.
set LIB=%OO_SDK_HOME%\windows\lib;%OO_SDK_HOME%\WINexample.out\lib;%LIB

REM Add directory of the STLPORT library to the library path.
if defined OO_STLPORT_HOME set LIB=%OO_STLPORT_HOME%\lib;%LIB%

REM Set office program path.
set OFFICE_PROGRAM_PATH=%OFFICE_HOME%\program

REM Add directory of the SDK tools to the path.
set PATH=%OO_SDK_HOME%\windows\bin;%OFFICE_PROGRAM_PATH%;%OO_SDK_HOME%\WINexample.out\bin;%PATH%

REM Add directory of the command make to the path, if necessary.
if defined OO_SDK_MAKE set PATH=%OO_SDK_MAKE%;%PATH%

REM Add directory of the C++ tools to the path, if necessary.
if defined OO_SDK_CPP_HOME set PATH=%OO_SDK_CPP_HOME%;%PATH%

REM Add directory of the STLPORT library to the path, if necessary.
if defined OO_STLPORT_HOME set PATH=%OO_STLPORT_HOME%\lib;%PATH%

REM Add directory of the Java tools to the path, if necessary.
if defined OO_SDK_JAVA_HOME set PATH=%OO_SDK_JAVA_HOME%\bin;%OO_SDK_JAVA_HOME%\jre\bin;%PATH%

REM Add directory of the ANT tools to the path, if necessary.
if defined OO_SDK_ANT_HOME set PATH=%OO_SDK_ANT_HOME%\bin;%PATH%

REM Set environment for C++ tools, if necessary.
if defined OO_SDK_CPP_HOME call "%OO_SDK_CPP_HOME%"\VCVARS32.bat

goto end

:error
echo Error: Please insert the necessary environment variables into the batch file.

:end
