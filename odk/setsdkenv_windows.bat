REM This script sets all enviroment variables, which
REM are necessary for building the examples of the Office Development Kit.
REM The Script was developed for the operating systems Windows.
REM Before the script could be executed the following variables have to be
REM adjusted:

REM Directory of the make command.
set SDK_MAKE=D:\MOVING\NextGenerationMake\make

REM Installation directory of the Software Development Kit.
set SDK_HOME=D:\MOVING\RESCUE\CvsLaufwerkD\api\odk

REM Office installation directory.
set OFFICE_HOME=C:\Programme\StarOffice6.0

REM Directory of the C++ tools.
set SDK_CPP_HOME=C:\Programme\Microsoft Visual Studio\VC98\bin

REM Java installation directory.
set SDK_JAVA_HOME=C:\Programme\Java\J2SDK_Forte\jdk1.4.0

REM ANT installation directory.
REM set SDK_ANT=D:\MOVING\jakarta-ant-1.4

REM Set library path.
set LIB=%LIB%;%OFFICE_HOME%\program;%SDK_HOME%\windows\lib

REM Set office program path.
set OFFICE_PROGRAM_PATH=%OFFICE_HOME%\program

REM Add directories to the path.
set PATH=%PATH%;%SDK_HOME%\windows\bin;%SDK_MAKE%;%SDK_CPP_HOME%;%SDK_JAVA_HOME%\bin;%SDK_ANT%\bin

REM Set environment for c++ tools.
call "%SDK_CPP_HOME%"\VCVARS32.bat
