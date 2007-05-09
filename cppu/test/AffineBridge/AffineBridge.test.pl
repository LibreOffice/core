:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;

#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: AffineBridge.test.pl,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: kz $ $Date: 2007-05-09 13:40:17 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

my $rc      = 0;
my $comment = "";


@tests = (
  "EnvStack.tester :A:affine :A:affine        \"-enter:A[0,not entered]-leave:A[0,not entered]\" :A:affine", # initially not entered&leave
  "EnvStack.tester :affine:A :affine:A        \"-enter:affine:A[1,OK]-leave:affine:A[1,OK]\"     :affine",   # enter
  "EnvStack.tester \"\"      :affine:A        \"-into:affine:A[1,OK]\"                           :affine",   # call into
  "EnvStack.tester :affine   :A               \"-into:A[0,wrong thread]\"                        :affine",   # call out
  "EnvStack.tester \"\"      :affine:affine:A \"-into:affine:affine:A[0,wrong thread]\"          :affine",    # wrong thread
  "env.tester.bin CPP:affine"
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
