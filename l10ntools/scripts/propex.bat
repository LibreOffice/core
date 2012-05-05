@echo off

if "x%PERL%x" EQU "xx" (
    echo ERROR: Perl not found. Environment is not set.
    exit /b 1
)

%PERL% %SOLARVER%\%INPATH%\bin\propex %*