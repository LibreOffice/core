@echo off

if "x%SOLARENV%x" EQU "xx" (
   echo No environment found, please use 'configure' or 'setsolar'
   exit
)

if "x%JAVA_HOME%x" EQU "xx" (
   echo echo ERROR: No java found
   exit
)

java -DSOLARSRC=%SOLARSRC% -DWORK_STAMP=%WORK_STAMP% -DUSE_SHELL=  -jar %SOLARVER%\%INPATH%\bin\jpropex.jar %*
