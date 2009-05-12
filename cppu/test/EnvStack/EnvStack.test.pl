:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;

#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: EnvStack.test.pl,v $
#
# $Revision: 1.3 $
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

my $rc      = 0;
my $comment = "";


@tests = (
  "EnvStack.tester \"\"           \"\" \"\"",
  "EnvStack.tester \":A:a\"       \":A:b\"       \"-enter:A-enter:A:a-out:A:a-into:A:b-leave:A:a-leave:A\"",
  "EnvStack.tester \":A:B:a\"     \":A:B:b\"     \"-enter:A-enter:A:B-enter:A:B:a-out:A:B:a-into:A:B:b-leave:A:B:a-leave:A:B-leave:A\"",
  "EnvStack.tester \":A:B:C:a\"   \":A:B:C:b\"   \"-enter:A-enter:A:B-enter:A:B:C-enter:A:B:C:a-out:A:B:C:a-into:A:B:C:b-leave:A:B:C:a-leave:A:B:C-leave:A:B-leave:A\"",
  "EnvStack.tester \":A:B:C:D:a\" \":A:B:C:D:b\" \"-enter:A-enter:A:B-enter:A:B:C-enter:A:B:C:D-enter:A:B:C:D:a-out:A:B:C:D:a-into:A:B:C:D:b-leave:A:B:C:D:a-leave:A:B:C:D-leave:A:B:C-leave:A:B-leave:A\"",
  "EnvStack.tester \":A:a:b\"     \":A:c:d\"     \"-enter:A-enter:A:a-enter:A:a:b-out:A:a:b-out:A:a-into:A:c-into:A:c:d-leave:A:a:b-leave:A:a-leave:A\"",
  "EnvStack.tester \":A:B:a:b\"   \":A:B:c:d\"   \"-enter:A-enter:A:B-enter:A:B:a-enter:A:B:a:b-out:A:B:a:b-out:A:B:a-into:A:B:c-into:A:B:c:d-leave:A:B:a:b-leave:A:B:a-leave:A:B-leave:A\"",
  "EnvStack.tester \":A\"         \"\"           \"-enter:A-out:A-leave:A\"",
  "EnvStack.tester \":A:B\"       \"\"           \"-enter:A-enter:A:B-out:A:B-out:A-leave:A:B-leave:A\"",
  "EnvStack.tester \":A:B:C\"     \"\"           \"-enter:A-enter:A:B-enter:A:B:C-out:A:B:C-out:A:B-out:A-leave:A:B:C-leave:A:B-leave:A\"",
  "EnvStack.tester \":A:B:C:D\"   \"\"           \"-enter:A-enter:A:B-enter:A:B:C-enter:A:B:C:D-out:A:B:C:D-out:A:B:C-out:A:B-out:A-leave:A:B:C:D-leave:A:B:C-leave:A:B-leave:A\"",
  "EnvStack.tester \"\"           \":a\"         \"-into:a\"",
  "EnvStack.tester \"\"           \":a:b\"       \"-into:a-into:a:b\"",
  "EnvStack.tester \"\"           \":a:b:c\"     \"-into:a-into:a:b-into:a:b:c\"",
  "EnvStack.tester \"\"           \":a:b:c:d\"   \"-into:a-into:a:b-into:a:b:c-into:a:b:c:d\"",
  "EnvStack.tester \":A\"         \":a\"         \"-enter:A-out:A-into:a-leave:A\"",
  "EnvStack.tester \":A:B\"       \":a:b\"       \"-enter:A-enter:A:B-out:A:B-out:A-into:a-into:a:b-leave:A:B-leave:A\"",
  "EnvStack.tester \":A:B:C\"     \":a:b:c\"     \"-enter:A-enter:A:B-enter:A:B:C-out:A:B:C-out:A:B-out:A-into:a-into:a:b-into:a:b:c-leave:A:B:C-leave:A:B-leave:A\"",
  "EnvStack.tester \":A:B:C:D\"   \":a:b:c:d\"   \"-enter:A-enter:A:B-enter:A:B:C-enter:A:B:C:D-out:A:B:C:D-out:A:B:C-out:A:B-out:A-into:a-into:a:b-into:a:b:c-into:a:b:c:d-leave:A:B:C:D-leave:A:B:C-leave:A:B-leave:A\"",
  "EnvStack.tester \":A\"         \":A\"         \"-enter:A-leave:A\"",
  "EnvStack.tester \":A:B\"       \":A:B\"       \"-enter:A-enter:A:B-leave:A:B-leave:A\"",
  "EnvStack.tester \":A:B:C\"     \":A:B:C\"     \"-enter:A-enter:A:B-enter:A:B:C-leave:A:B:C-leave:A:B-leave:A\"",
  "EnvStack.tester \":A:B:C:D\"   \":A:B:C:D\"   \"-enter:A-enter:A:B-enter:A:B:C-enter:A:B:C:D-leave:A:B:C:D-leave:A:B:C-leave:A:B-leave:A\""
);

foreach $test (@tests) {
    $output = "";

    $cmd = $test;
    open TESTER, $cmd . "|";
    while (<TESTER>) {
        chomp;

        $output = $output . "\t" . $_ . "\n";
    }
    close TESTER ;

    if ($? != 0) {
        $comment = $comment . "TEST FAILED: " . $cmd . "\n";
        $comment = $comment . $output;
    }
    $rc = $rc + $?;
}


print $comment;

if ($rc == 0) {
    print "*********** SUCCESS\n";
}
else {
    print "*********** FAILURE\n";
}
