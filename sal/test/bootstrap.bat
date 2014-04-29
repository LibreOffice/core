rem
rem This file is part of the LibreOffice project.
rem
rem This Source Code Form is subject to the terms of the Mozilla Public
rem License, v. 2.0. If a copy of the MPL was not distributed with this
rem file, You can obtain one at http://mozilla.org/MPL/2.0/.
rem
rem This file incorporates work covered by the following license notice:
rem
rem   Licensed to the Apache Software Foundation (ASF) under one or more
rem   contributor license agreements. See the NOTICE file distributed
rem   with this work for additional information regarding copyright
rem   ownership. The ASF licenses this file to you under the Apache
rem   License, Version 2.0 (the "License"); you may not use this file
rem   except in compliance with the License. You may obtain a copy of
rem   the License at http://www.apache.org/licenses/LICENSE-2.0 .
rem
@echo off
set MYBOOTSTRAPTESTVALUE=0
echo 1
.\testbootstrap 1 -env:MYBOOTSTRAPTESTVALUE=1

echo 2
.\testbootstrap.exe file

echo 3
.\testbootstrap.Exe file

echo 4
.\testbootstrap.bin file

echo 5
.\testbootstrap.Bin file

echo 6
.\testbootstrap 0 -env:INIFILENAME=

echo 7
set MYBOOTSTRAPTESTVALUE=
rem simply ignore the file ....
.\testbootstrap default -env:INIFILENAME=

echo 8
.\testbootstrap default -env:MYBOOTSTRAPTESTVALUE2=1 -env:INIFILENAME=

echo
echo "macro tests"
echo

rem simple macro expansion
echo 9
.\testbootstrap _first_second_third_ -env:FIRST=first -env:SECOND=second -env:THIRD=third -env:MYBOOTSTRAPTESTVALUE=_${FIRST}_${SECOND}_${THIRD}_

rem simple quoting
echo 10
.\testbootstrap _${FIRST}_${SECOND}_${THIRD}_ -env:FIRST=first -env:SECOND=second -env:THIRD=third -env:MYBOOTSTRAPTESTVALUE=_\$\{FIRST\}_\$\{SECOND\}_\$\{THIRD\}_

rem simple ini access
echo 11
.\testbootstrap TheKeysValue -env:MYBOOTSTRAPTESTVALUE=${./bootstraptest.ini:TheSection:TheKey}

rem ini access with simple macro expansion
echo 12
.\testbootstrap TheKeysValue -env:ININAME=./bootstraptest.ini -env:SECTIONNAME=TheSection -env:KEYNAME=TheKey -env:MYBOOTSTRAPTESTVALUE=${$ININAME:$SECTIONNAME:$KEYNAME}

rem ini access with complex macro expansion
echo 13
.\testbootstrap TheKeysValue -env:ININAME=./bootstraptest.ini -env:MYBOOTSTRAPTESTVALUE=${$ININAME:${$ININAME:SecondSection:IndirectSection}:${$ININAME:SecondSection:IndirectKey}}

rem test no infinite recursion
echo 14
.\testbootstrap "***RECURSION DETECTED***" -env:MYBOOTSTRAPTESTVALUE=$MYBOOTSTRAPTESTVALUE

rem test unicode
echo 15
.\testbootstrap AAABBBCCC000 -env:MYBOOTSTRAPTESTVALUE=\u0041\u0041\u0041\u0042\u0042\u0042\u0043\u0043\u0043\u0030\u0030\u0030


@echo bootstrap test finished
