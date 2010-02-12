:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************


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

if ($ENV{GUI} eq "WNT") {
    $rc = system "./testbootstrap", "auxaux", "-env:iniName=ini.ini", '-env:MYBOOTSTRAPTESTVALUE=$CUSTOMINIVALUE';
}
else {
    $rc = system "./testbootstrap", "auxaux", "-env:iniName=inirc", '-env:MYBOOTSTRAPTESTVALUE=$CUSTOMINIVALUE';
}
if (!$rc) {
    $comment = $comment . "custom ini test not passed\n";
    $state = 0;
}

if ($ENV{GUI} eq "WNT") {
    $comment = $comment . '$SYSUSERHOME not testable under windows' . "\n";
}
else {
    $rc = system "./testbootstrap", "file://$ENV{HOME}", '-env:MYBOOTSTRAPTESTVALUE=$SYSUSERHOME';
    if (!$rc) {
        $comment = $comment . '$SYSUSERHOME test not passed' . "\n";
        $state = 0;
    }
}

if ($ENV{GUI} eq "WNT") {
    $comment = $comment . '$SYSUSERCONFIG' . " not testable under windows\n";
}
else {
    $rc = system "./testbootstrap", "file://$ENV{HOME}", '-env:MYBOOTSTRAPTESTVALUE=$SYSUSERCONFIG';
    if (!$rc) {
        $comment = $comment . '$SYSUSERCONFIG test not passed' . "\n";
        $state = 0;
    }
}

if ($ENV{GUI} eq "WNT") {
    $comment = $comment . '$SYSBINDIR' . " not testable under windows\n";
}
else {
    $rc = system "./testbootstrap", "file://$ENV{PWD}", '-env:MYBOOTSTRAPTESTVALUE=$SYSBINDIR';
    if (!$rc) {
        $comment = $comment . '$SYSBINDIR test not passed' . "\n";
        $state = 0;
    }
}

if ($ENV{GUI} eq "WNT") {
    $rc = system "./testbootstrap", "inherited_value", '-env:MYBOOTSTRAPTESTVALUE=$INHERITED_VALUE', "-env:iniName=ini.ini";
}
else {
    $rc = system "./testbootstrap", "inherited_value", '-env:MYBOOTSTRAPTESTVALUE=$INHERITED_VALUE', "-env:iniName=inirc";
}
if (!$rc) {
    $comment = $comment . "inherited value not passed\n";
    $state = 0;
}

if ($ENV{GUI} eq "WNT") {
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

if ($ENV{GUI} eq "WNT") {
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

# test no infinit recursion
if ($ENV{GUI} eq "WNT") {
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
    $comment = $comment . "no infinit recursion test not passed\n";
    $state = 0;
}

# test unicode
$rc = system "./testbootstrap",
    "AAABBBCCC000",
    '-env:MYBOOTSTRAPTESTVALUE=\u0041\u0041\u0041\u0042\u0042\u0042\u0043\u0043\u0043\u0030\u0030\u0030';
if (!$rc) {
    $comment = $comment . "uncode not passed\n";
    $state = 0;
}

print "**************************\n";
if($state) {
    print "****** tests passed ******\n";
}
else {
    print "**** tests NOT passed ****\n";
    print "Commnent:\n", $comment, "\n";
}
print "**************************\n";



