@echo off

REM This script sets all enviroment variables, which
REM are necessary for building the examples of the Office Development Kit.
REM The Script was developed for the operating systems Windows.
REM Before the script could be executed the following variables have to be
REM adjusted:

REM Installation directory of the Software Development Kit.
REM Example: set SDK_HOME=D:\api\odk
set SDK_HOME=

REM Office installation directory.
REM Example: set OFFICE_HOME=C:\Programme\StarOffice6.0
set OFFICE_HOME=

REM Directory of the make command.
REM Example: set SDK_MAKE=D:\NextGenerationMake\make
set SDK_MAKE=

REM Directory of the C++ tools.
REM Example:set SDK_CPP_HOME=C:\Programme\Microsoft Visual Studio\VC98\bin
set SDK_CPP_HOME=

REM STLPORT installation directory.
REM Example: set STLPORT_HOME=D:\stlport-4.5.3
set STLPORT_HOME=

REM Java installation directory.
REM Example: set SDK_JAVA_HOME=C:\Programme\Java\J2SDK_Forte\jdk1.4.0
set SDK_JAVA_HOME=

REM Environment variable to enable auto deployment of example components
REM Example: set SDK_AUTO_DEPLOYMENT=YES
set SDK_AUTO_DEPLOYMENT=YES

REM ANT installation directory.
REM Example: set SDK_ANT=D:\jakarta-ant-1.4
REM set SDK_ANT=


REM Check installation path for the StarOffice Development Kit.
if not defined SDK_HOME goto error

REM Check installation path for the office.
if not defined OFFICE_HOME goto error

REM Set library path.
set LIB=%SDK_HOME%\windows\lib;%SDK_HOME%\WINexample.out\lib;%LIB

REM Add directory of the STLPORT library to the library path.
if defined STLPORT_HOME set LIB=%STLPORT_HOME%\lib;%LIB%

REM Set office program path.
set OFFICE_PROGRAM_PATH=%OFFICE_HOME%\program

REM Add directory of the SDK tools to the path.
set PATH=%SDK_HOME%\windows\bin;%OFFICE_PROGRAM_PATH%;%SDK_HOME%\WINexample.out\bin;%PATH%

REM Add directory of the command make to the path, if necessary.
if defined SDK_MAKE set PATH=%SDK_MAKE%;%PATH%

REM Add directory of the C++ tools to the path, if necessary.
if defined SDK_CPP_HOME set PATH=%SDK_CPP_HOME%;%PATH%

REM Add directory of the STLPORT library to the path, if necessary.
if defined STLPORT_HOME set PATH=%STLPORT_HOME%\lib;%PATH%

REM Add directory of the Java tools to the path, if necessary.
if defined SDK_JAVA_HOME set PATH=%SDK_JAVA_HOME%\bin;%SDK_JAVA_HOME%\jre\bin;%PATH%

REM Add directory of the ANT tools to the path, if necessary.
if defined SDK_ANT set PATH=%SDK_ANT%\bin;%PATH%

REM Set environment for C++ tools, if necessary.
if defined SDK_CPP_HOME call "%SDK_CPP_HOME%"\VCVARS32.bat

goto end

:error
echo Error: Please insert the necessary environment variables into the batch file.

:end
