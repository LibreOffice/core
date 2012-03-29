rem *************************************************************
rem  
rem  Licensed to the Apache Software Foundation (ASF) under one
rem  or more contributor license agreements.  See the NOTICE file
rem  distributed with this work for additional information
rem  regarding copyright ownership.  The ASF licenses this file
rem  to you under the Apache License, Version 2.0 (the
rem  "License"); you may not use this file except in compliance
rem  with the License.  You may obtain a copy of the License at
rem  
rem    http://www.apache.org/licenses/LICENSE-2.0
rem  
rem  Unless required by applicable law or agreed to in writing,
rem  software distributed under the License is distributed on an
rem  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
rem  KIND, either express or implied.  See the License for the
rem  specific language governing permissions and limitations
rem  under the License.
rem  
rem *************************************************************
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

rem test no infinit recursion
echo 14
.\testbootstrap "***RECURSION DETECTED***" -env:MYBOOTSTRAPTESTVALUE=$MYBOOTSTRAPTESTVALUE

rem test unicode
echo 15
.\testbootstrap AAABBBCCC000 -env:MYBOOTSTRAPTESTVALUE=\u0041\u0041\u0041\u0042\u0042\u0042\u0043\u0043\u0043\u0030\u0030\u0030


@echo bootstrap test finished
