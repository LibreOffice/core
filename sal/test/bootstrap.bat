@echo off
set MYBOOTSTRAPTESTVALUE=0
echo 1
.\testbootstrap 1 -env:MYBOOTSTRAPTESTVALUE=1
echo 2
.\testbootstrap.exe file
echo 3
.\testbootstrap 0 -env:INIFILENAME=

echo 4
set MYBOOTSTRAPTESTVALUE=
rem simply ignore the file ....
.\testbootstrap default -env:INIFILENAME=

echo 5
.\testbootstrap default -env:MYBOOTSTRAPTESTVALUE2=1 -env:INIFILENAME=

@echo bootstrap test finished