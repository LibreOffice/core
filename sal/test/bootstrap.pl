:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#


my $rc;
my $state = 1;
my $comment = "";

$ENV{MYBOOTSTRAPTESTVALUE}=0;

$rc = system "./testbootstrap", 1, "-env:MYBOOTSTRAPTESTVALUE=1";
if (!$rc) {
    $comment = $comment . "commandline over environment test not passed\n";
    $state = 0;
}

$rc = system "./testbootstrap", "0", "-env:INIFILENAME=";
if (!$rc) {
    $comment = $comment . "exe custom ini test not passed\n";
    $state = 0;
}

delete $ENV{MYBOOTSTRAPTESTVALUE};

$rc = system "./testbootstrap.bin", "file";
if (!$rc) {
    $comment = $comment . "extensions cut test not passed with .bin\n";
    $state = 0;
}

$rc = system "./testbootstrap.Bin", "file";
if (!$rc) {
    $comment = $comment . "extensions cut test not passed with .Bin\n";
    $state = 0;
}

$rc = system "./testbootstrap.exe", "file";
if (!$rc) {
    $comment = $comment . "extensions cut test not passed with .exe\n";
    $state = 0;
}

$rc = system "./testbootstrap.Exe", "file";
if (!$rc) {
    $comment = $comment . "extensions cut test not passed with .Exe\n";
    $state = 0;
}

if ($ENV{OS} eq "WNT") {
    $rc = system "./testbootstrap", "auxaux", "-env:iniName=ini.ini", '-env:MYBOOTSTRAPTESTVALUE=$CUSTOMINIVALUE';
}
else {
    $rc = system "./testbootstrap", "auxaux", "-env:iniName=inirc", '-env:MYBOOTSTRAPTESTVALUE=$CUSTOMINIVALUE';
}
if (!$rc) {
    $comment = $comment . "custom ini test not passed\n";
    $state = 0;
}

if ($ENV{OS} eq "WNT") {
    $comment = $comment . '$SYSUSERHOME not testable under windows' . "\n";
}
else {
    $rc = system "./testbootstrap", "file://$ENV{HOME}", '-env:MYBOOTSTRAPTESTVALUE=$SYSUSERHOME';
    if (!$rc) {
        $comment = $comment . '$SYSUSERHOME test not passed' . "\n";
        $state = 0;
    }
}

if ($ENV{OS} eq "WNT") {
    $comment = $comment . '$SYSUSERCONFIG' . " not testable under windows\n";
}
else {
    $rc = system "./testbootstrap", "file://$ENV{HOME}", '-env:MYBOOTSTRAPTESTVALUE=$SYSUSERCONFIG';
    if (!$rc) {
        $comment = $comment . '$SYSUSERCONFIG test not passed' . "\n";
        $state = 0;
    }
}

if ($ENV{OS} eq "WNT") {
    $comment = $comment . '$SYSBINDIR' . " not testable under windows\n";
}
else {
    $rc = system "./testbootstrap", "file://$ENV{PWD}", '-env:MYBOOTSTRAPTESTVALUE=$SYSBINDIR';
    if (!$rc) {
        $comment = $comment . '$SYSBINDIR test not passed' . "\n";
        $state = 0;
    }
}

if ($ENV{OS} eq "WNT") {
    $rc = system "./testbootstrap", "inherited_value", '-env:MYBOOTSTRAPTESTVALUE=$INHERITED_VALUE', "-env:iniName=ini.ini";
}
else {
    $rc = system "./testbootstrap", "inherited_value", '-env:MYBOOTSTRAPTESTVALUE=$INHERITED_VALUE', "-env:iniName=inirc";
}
if (!$rc) {
    $comment = $comment . "inherited value not passed\n";
    $state = 0;
}

if ($ENV{OS} eq "WNT") {
    $rc = system "./testbootstrap", "not_overwritten", '-env:MYBOOTSTRAPTESTVALUE=$INHERITED_OVERWRITTEN_VALUE', "-env:iniName=ini.ini";
}
else {
    $rc = system "./testbootstrap", "not_overwritten", '-env:MYBOOTSTRAPTESTVALUE=$INHERITED_OVERWRITTEN_VALUE', "-env:iniName=inirc";
}
if (!$rc) {
    $comment = $comment . "inherited overwritten value not passed\n";
    $state = 0;
}


$rc = system "./testbootstrap", "defaultvalue", "-env:INIFILENAME=", "-env:Default=defaultvalue", "-env:USEDEFAULT=1";
if (!$rc) {
    $comment = $comment . "default test from parameter not passed\n";
    $state = 0;
}

if ($ENV{OS} eq "WNT") {
    $rc = system "./testbootstrap",  "defaultValue", "-env:iniName=default.ini", "-env:INIFILENAME=", "-env:USEDEFAULT=1";
}
else {
    $rc = system "./testbootstrap",  "defaultValue", "-env:iniName=defaultrc", "-env:INIFILENAME=", "-env:USEDEFAULT=1";
}
if (!$rc) {
    $comment = $comment . "default test from custom ini not passed\n";
    $state = 0;
}

# simple macro expansion
$rc = system "./testbootstrap",
    "_first_second_third_",
    "-env:FIRST=first",
    "-env:SECOND=second",
    "-env:THIRD=third",
    '-env:MYBOOTSTRAPTESTVALUE=_${FIRST}_${SECOND}_${THIRD}_';
if (!$rc) {
    $comment = $comment . "simple macro expansion test not passed\n";
    $state = 0;
}

# simple quoting
$rc = system "./testbootstrap",
    '_${FIRST}_${SECOND}_${THIRD}_',
    "-env:FIRST=first",
    "-env:SECOND=second",
    "-env:THIRD=third",
    '-env:MYBOOTSTRAPTESTVALUE=_\$\{FIRST\}_\$\{SECOND\}_\$\{THIRD\}_';
if (!$rc) {
    $comment = $comment . "simple macro quoting test not passed\n";
    $state = 0;
}

# simple ini access
$rc = system "./testbootstrap",
    "TheIniKeysValue",
    '-env:MYBOOTSTRAPTESTVALUE=${./bootstraptest.ini:TheIniKey}';
if (!$rc) {
    $comment = $comment . "simple macro ini access test not passed\n";
    $state = 0;
}

# simple profile access
$rc = system "./testbootstrap",
    "TheKeysValue",
    '-env:MYBOOTSTRAPTESTVALUE=${./bootstraptest.ini:TheSection:TheKey}';
if (!$rc) {
    $comment = $comment . "simple macro profile access test not passed\n";
    $state = 0;
}

# profile access with simple macro expansion
$rc = system "./testbootstrap",
    "TheKeysValue",
    "-env:ININAME=./bootstraptest.ini",
    "-env:SECTIONNAME=TheSection",
    "-env:KEYNAME=TheKey",
    '-env:MYBOOTSTRAPTESTVALUE=${$ININAME:$SECTIONNAME:$KEYNAME}';
if (!$rc) {
    $comment = $comment . "profile access with simple macro expansion test not passed\n";
    $state = 0;
}

# profile access with complex macro expansion
$rc = system "./testbootstrap",
    "TheKeysValue",
    "-env:ININAME=./bootstraptest.ini",
    '-env:MYBOOTSTRAPTESTVALUE=${$ININAME:${$ININAME:SecondSection:IndirectSection}:${$ININAME:SecondSection:IndirectKey}}';
if (!$rc) {
    $comment = $comment . "profile access with complex macro expansion test not passed\n";
    $state = 0;
}

# test no infinite recursion
if ($ENV{OS} eq "WNT") {
    $rc = system "./testbootstrap",
    '"***RECURSION DETECTED***"',
    '-env:MYBOOTSTRAPTESTVALUE=$MYBOOTSTRAPTESTVALUE';
}
else {
    $rc = system "./testbootstrap",
    '***RECURSION DETECTED***',
    '-env:MYBOOTSTRAPTESTVALUE=$MYBOOTSTRAPTESTVALUE';
}
if (!$rc) {
    $comment = $comment . "no infinite recursion test not passed\n";
    $state = 0;
}

# test unicode
$rc = system "./testbootstrap",
    "AAABBBCCC000",
    '-env:MYBOOTSTRAPTESTVALUE=\u0041\u0041\u0041\u0042\u0042\u0042\u0043\u0043\u0043\u0030\u0030\u0030';
if (!$rc) {
    $comment = $comment . "unicode not passed\n";
    $state = 0;
}

print "**************************\n";
if($state) {
    print "****** tests passed ******\n";
}
else {
    print "**** tests NOT passed ****\n";
    print "Comment:\n", $comment, "\n";
}
print "**************************\n";



