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
# $RCSfile: UnsafeBridge.test.pl,v $
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
  "EnvStack.tester :A:unsafe :A:unsafe        \"-enter:A[0,not entered]-leave:A[0,not entered]\" :A:unsafe", # initially not entered&leave
  "EnvStack.tester :unsafe:A :unsafe:A        \"-enter:unsafe:A[1,OK]-leave:unsafe:A[1,OK]\"     :unsafe",   # enter
  "EnvStack.tester \"\"      :unsafe:A        \"-into:unsafe:A[1,OK]\"                           :unsafe",   # call into
  "EnvStack.tester :unsafe   :A               \"-into:A[0,not entered]\"                         :unsafe",   # call out
  "EnvStack.tester \"\"      :unsafe:affine:A \"-into:unsafe:affine:A[0,wrong thread]\"          :unsafe",   # wrong thread
  "env.tester.bin CPP:unsafe"
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
