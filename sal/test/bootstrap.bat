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

echo
echo "macro tests"
echo

rem simple macro expansion
.\testbootstrap _first_second_third_ -env:FIRST=first -env:SECOND=second -env:THIRD=third -env:MYBOOTSTRAPTESTVALUE=_${FIRST}_${SECOND}_${THIRD}_

rem simple quoting
.\testbootstrap _${FIRST}_${SECOND}_${THIRD}_ -env:FIRST=first -env:SECOND=second -env:THIRD=third -env:MYBOOTSTRAPTESTVALUE=_\$\{FIRST\}_\$\{SECOND\}_\$\{THIRD\}_

rem simple ini access
.\testbootstrap TheKeysValue -env:MYBOOTSTRAPTESTVALUE=${./bootstraptest.ini:TheSection:TheKey}

rem ini access with simple macro expansion
.\testbootstrap TheKeysValue -env:ININAME=./bootstraptest.ini -env:SECTIONNAME=TheSection -env:KEYNAME=TheKey -env:MYBOOTSTRAPTESTVALUE=${$ININAME:$SECTIONNAME:$KEYNAME}

rem ini access with complex macro expansion
.\testbootstrap TheKeysValue -env:ININAME=./bootstraptest.ini -env:MYBOOTSTRAPTESTVALUE=${$ININAME:${$ININAME:SecondSection:IndirectSection}:${$ININAME:SecondSection:IndirectKey}}

rem test no infinit recursion
.\testbootstrap "***RECURSION DETECTED***" -env:MYBOOTSTRAPTESTVALUE=$MYBOOTSTRAPTESTVALUE

rem test unicode
.\testbootstrap AAABBBCCC000 -env:MYBOOTSTRAPTESTVALUE=\u0041\u0041\u0041\u0042\u0042\u0042\u0043\u0043\u0043\u0030\u0030\u0030


@echo bootstrap test finished