@echo off
REM *****************************************************************
REM * MAKE
REM * (c) Copyright 1992-1994 STAR DIVISION
REM * Beschreibung: Uebersetzt aktuelle Version
REM * Aufruf:       MAKE [Option1 .. OptionN]
REM * Optionen:     PRODUCT     - Product-Version (FULL)
REM *               DEMO        - Product-Version (DEMO)
REM *               DEPEND      - Depend-Listen erzeugen
REM *               OPTIMIZE    - Version mit Optimierung
REM *               DEBUG       - Version mit Debuginformationen
REM *               PROFILE     - Version fuer Profiling
REM *               DBGUITL     - Version mit Assertions
REM * ESO:          PCH         - Precompiled Header verwenden
REM *****************************************************************

IF "%1" == "" goto Next

set STEMPFILE=%temp%\temp.mak
echo. >%STEMPFILE%
set MAKECMD=@%STEMPFILE%

REM *** Parameter parsen ***
:Loop

IF "%1" == "" goto Next

IF "%1" == "product" echo product=full >>%STEMPFILE%
IF "%1" == "Product" echo product=full >>%STEMPFILE%
IF "%1" == "PRODUCT" echo product=full >>%STEMPFILE%

IF "%1" == "demo" echo product=demo >>%STEMPFILE%
IF "%1" == "Demo" echo product=demo >>%STEMPFILE%
IF "%1" == "DEMO" echo product=demo >>%STEMPFILE%

IF "%1" == "depend" echo depend=true >>%STEMPFILE%
IF "%1" == "Depend" echo depend=true >>%STEMPFILE%
IF "%1" == "DEPEND" echo depend=true >>%STEMPFILE%

IF "%1" == "optimize" echo optimize=true >>%STEMPFILE%
IF "%1" == "Optimize" echo optimize=true >>%STEMPFILE%
IF "%1" == "OPTIMIZE" echo optimize=true >>%STEMPFILE%

IF "%1" == "debug" echo debug=true >>%STEMPFILE%
IF "%1" == "Debug" echo debug=true >>%STEMPFILE%
IF "%1" == "DEBUG" echo debug=true >>%STEMPFILE%

IF "%1" == "profile" echo product=full profile=true >>%STEMPFILE%
IF "%1" == "Profile" echo product=full profile=true >>%STEMPFILE%
IF "%1" == "PROFILE" echo product=full profile=true >>%STEMPFILE%

IF "%1" == "dbgutil" echo dbgutil=true >>%STEMPFILE%
IF "%1" == "Dbgutil" echo dbgutil=true >>%STEMPFILE%
IF "%1" == "DBGUTIL" echo dbgutil=true >>%STEMPFILE%

IF "%1" == "seg" echo product=full seg=true >>%STEMPFILE%
IF "%1" == "Seg" echo product=full seg=true >>%STEMPFILE%
IF "%1" == "SEG" echo product=full seg=true >>%STEMPFILE%

IF "%1" == "tcv" echo product=full tcv=-2000 >>%STEMPFILE%
IF "%1" == "tcv" echo product=full tcv=-2000 >>%STEMPFILE%
IF "%1" == "TCV" echo product=full tcv=-2000 >>%STEMPFILE%

IF "%1" == "siz" echo product=full siz=true >>%STEMPFILE%
IF "%1" == "Siz" echo product=full siz=true >>%STEMPFILE%
IF "%1" == "SIZ" echo product=full siz=true >>%STEMPFILE%

IF "%1" == "pch" echo prjpch=true >>%STEMPFILE%
IF "%1" == "Pch" echo prjpch=true >>%STEMPFILE%
IF "%1" == "PCH" echo prjpch=true >>%STEMPFILE%

IF "%1" == "-i" echo -i >>%STEMPFILE%
IF "%1" == "-I" echo -i >>%STEMPFILE%

shift
goto Loop

:Next
REM *** Kommando setzen ***
SET STREECMD=nmake %MAKECMD%

REM *** Kommando ausfuehren ***
echo ******************************************************
echo MAKE - (c) 1992-1994 STAR DIVISION
call stree MAKE

REM *** Aufraeumen ***
IF NOT "%STEMPFILE%" == "" del %STEMPFILE%
SET STREECMD=
set STEMPFILE=
set MAKECMD=
echo on
