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

REM Java installation directory.
REM Example: set SDK_JAVA_HOME=C:\Programme\Java\J2SDK_Forte\jdk1.4.0
set SDK_JAVA_HOME=

REM ANT installation directory.
REM Example: set SDK_ANT=D:\jakarta-ant-1.4
REM set SDK_ANT=


REM Check installation path for the StarOffice Development Kit.
if not defined SDK_HOME goto error

REM Check installation path for the office.
if not defined OFFICE_HOME goto error

REM Set library path.
set LIB=%LIB%;%SDK_HOME%\windows\lib

REM Set office program path.
set OFFICE_PROGRAM_PATH=%OFFICE_HOME%\program

REM Add directory of the SDK tools to the path.
set PATH=%PATH%;%SDK_HOME%\windows\bin

REM Add directory of the command make to the path, if necessary.
if defined SDK_MAKE set PATH=%PATH%;%SDK_MAKE%

REM Add directory of the C++ tools to the path, if necessary.
if defined SDK_CPP_HOME set PATH=%PATH%;%SDK_CPP_HOME%

REM Add directory of the Java tools to the path, if necessary.
if defined SDK_JAVA_HOME set PATH=%PATH%;%SDK_JAVA_HOME%\bin

REM Add directory of the ANT tools to the path, if necessary.
if defined SDK_ANT set PATH=%PATH%;%SDK_ANT%\bin

REM Set environment for C++ tools, if necessary.
if defined SDK_CPP_HOME call "%SDK_CPP_HOME%"\VCVARS32.bat

goto end

:error
echo Error: Please insert the necessary environment variables into the batch file.

:end
