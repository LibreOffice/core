REM *****************************************************************
REM * STREE
REM * (c) Copyright 1992-1994 STAR DIVISION
REM * Beschreibung: Arbeitet Source-Verzeichnis-Baum von TOOLS ab
REM * Aufruf:       STREE DIR
REM * DIR:          ALL         - Alle Verzeichnisse
REM *               MAKE        - Alle Verzeichnisse mit MAKEFILE
REM * Umgebung:     GUI         - Systemversion
REM *               COM         - Systemversion
REM *               GUIBASE     - Abhaengiges Source-Verzeichnis
REM *               STREECMD    - Auszufuehrendes Kommando (z.B. nmake)
REM *****************************************************************

REM *** Parameter parsen und Variablen ueberpruefen ***
IF "%GUI%" == "" GOTO Error1
IF "%COM%" == "" GOTO Error1
IF "%GUIBASE%" == "" GOTO Error1

IF "%1" == "" GOTO Error3
IF "%STREECMD%" == "" GOTO Error3


REM *** Kommando ausgeben ***
echo Kommando: %STREECMD%

REM *** In die Root wechseln
cd ..


REM *** Include - Verzeichnisse ***
IF NOT "%1" == "ALL" goto NotAll
cd inc
echo ------------------------------------------------------
cd
%STREECMD%
IF ERRORLEVEL 1 goto Error2
cd ..

cd res
echo ------------------------------------------------------
cd
%STREECMD%
IF ERRORLEVEL 1 goto Error2
cd ..

cd source\ui\inc
echo ------------------------------------------------------
cd
%STREECMD%
IF ERRORLEVEL 1 goto Error2
cd ..\..\..

REM *** Verzeichnisse mit Makefile ***
:NotAll

cd sdi
echo ------------------------------------------------------
cd
%STREECMD%
IF ERRORLEVEL 1 goto Error2
cd ..

REM stardiv/sd -> ONE

cd source\ui
cd app
echo ------------------------------------------------------
cd
%STREECMD%
IF ERRORLEVEL 1 goto Error2
cd ..

cd dlg
echo ------------------------------------------------------
cd
%STREECMD%
IF ERRORLEVEL 1 goto Error2
cd ..

cd docshell
echo ------------------------------------------------------
cd
%STREECMD%
IF ERRORLEVEL 1 goto Error2
cd ..

cd view
echo ------------------------------------------------------
cd
%STREECMD%
IF ERRORLEVEL 1 goto Error2
cd ..

cd func
echo ------------------------------------------------------
cd
%STREECMD%
IF ERRORLEVEL 1 goto Error2
cd ..

cd unoidl
echo ------------------------------------------------------
cd
%STREECMD%
IF ERRORLEVEL 1 goto Error2
cd ..\..

cd core
echo ------------------------------------------------------
cd
%STREECMD%
IF ERRORLEVEL 1 goto Error2
cd ..

cd filter
echo ------------------------------------------------------
cd
%STREECMD%
IF ERRORLEVEL 1 goto Error2
cd ..

echo ------------------------------------------------------
cd
%STREECMD%
IF ERRORLEVEL 1 goto Error2
cd ..

echo ------------------------------------------------------
cd util
%STREECMD%
IF ERRORLEVEL 1 goto Error2
cd ..

echo ------------------------------------------------------
cd prj
goto End

REM *** Fehler ausgeben ***
:Error1
echo Error: Keine Systemversion oder nicht vollstaendig gesetzt !
goto End
:Error2
echo Error: Fehler bei AusfÅhrung eines Kommandos !
goto End
:Error3
echo Error: Falsche Parameter wurden an STREE uebergeben !
goto End

REM *** Aufraeumen ***
:End

