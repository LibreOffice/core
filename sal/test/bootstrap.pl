:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
#   $RCSfile: bootstrap.pl,v $
#
#   $Revision: 1.1 $
#
#   last change: $Author: kr $ $Date: 2001-08-30 12:17:44 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc..
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************


my $rc;
my $state = 1;
my $comment;

$ENV{MYBOOTSTRAPTESTVALUE}=0;
$rc = system "./testbootstrap", 1, "-env:MYBOOTSTRAPTESTVALUE=1";
if (!$rc) {
    $comment = $comment . "commandline over environment test not passed\n";
    $state = 0;
}

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
    $rc = system "./testbootstrap", "auxaux", "-env:iniName=ini.ini";
}
else {
    $rc = system "./testbootstrap", "auxaux", "-env:iniName=inirc";
}
if (!$rc) {
    $comment = $comment . "custom ini test not passed\n";
    $state = 0;
}

$rc = system "./testbootstrap", "0", "-env:INIFILENAME=";
if (!$rc) {
    $comment = $comment . "exe custom ini test not passed\n";
    $state = 0;
}

delete $ENV{MYBOOTSTRAPTESTVALUE};


$rc = system "./testbootstrap", "default", "-env:INIFILENAME=";
if (!$rc) {
    $comment = $comment . "exe custom ini test 2 not passed\n";
    $state = 0;
}

$rc = system "./testbootstrap",  "default", "-env:MYBOOTSTRAPTESTVALUE2=1", "-env:INIFILENAME=";
if (!$rc) {
    $comment = $comment . "exe custom ini test 3 not passed\n";
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
    "TheKeysValue",
    '-env:MYBOOTSTRAPTESTVALUE=${./bootstraptest.ini:TheSection:TheKey}';
if (!$rc) {
    $comment = $comment . "simple macro ini access test not passed\n";
    $state = 0;
}


# ini access with simple macro expansion
$rc = system "./testbootstrap",
    "TheKeysValue",
    "-env:ININAME=./bootstraptest.ini",
    "-env:SECTIONNAME=TheSection",
    "-env:KEYNAME=TheKey",
    '-env:MYBOOTSTRAPTESTVALUE=${$ININAME:$SECTIONNAME:$KEYNAME}';
if (!$rc) {
    $comment = $comment . "ini access with simple macro expansion test not passed\n";
    $state = 0;
}

# ini access with complex macro expansion
$rc = system "./testbootstrap",
    "TheKeysValue",
    "-env:ININAME=./bootstraptest.ini",
    '-env:MYBOOTSTRAPTESTVALUE=${$ININAME:${$ININAME:SecondSection:IndirectSection}:${$ININAME:SecondSection:IndirectKey}}';
if (!$rc) {
    $comment = $comment . "ini access with complex macro expansion test not passed\n";
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



