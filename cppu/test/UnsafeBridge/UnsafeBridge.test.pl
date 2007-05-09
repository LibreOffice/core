:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;

#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: UnsafeBridge.test.pl,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: kz $ $Date: 2007-05-09 13:45:44 $
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
