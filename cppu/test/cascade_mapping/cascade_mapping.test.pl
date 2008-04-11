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
# $RCSfile: cascade_mapping.test.pl,v $
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


@tests=(
"mapping.tester uno:test             uno                 -s path.test \"uno:test-->uno\"",
"mapping.tester uno                  uno:test            -s path.test \"uno-->uno:test\"",
"mapping.tester uno:test:bla         uno                 -s path.test \"uno:test:bla-->uno:test-->uno\"",
"mapping.tester uno                  uno:test:bla        -s path.test \"uno-->uno:test-->uno:test:bla\"",
"mapping.tester uno:test:bla:blubb   uno                 -s path.test \"uno:test:bla:blubb-->uno:test:bla-->uno:test-->uno\"",
"mapping.tester uno                  uno:test:bla:blubb  -s path.test \"uno-->uno:test-->uno:test:bla-->uno:test:bla:blubb\"",
"mapping.tester uno:bla              uno:test            -s path.test \"uno:bla-->uno-->uno:test\"",
"mapping.tester uno:test             uno:bla             -s path.test \"uno:test-->uno-->uno:bla\"",
"mapping.tester uno:test:blubb       uno:bla             -s path.test \"uno:test:blubb-->uno:test-->uno-->uno:bla\"",
"mapping.tester uno:test             uno:bla:blubb       -s path.test \"uno:test-->uno-->uno:bla-->uno:bla:blubb\"",
"mapping.tester uno:test:bla         uno:test:blubb      -s path.test \"uno:test:bla-->uno:test-->uno:test:blubb\"",
"mapping.tester CPP:bla              uno                 -s path.test \"CPP:bla-->uno:bla-->uno\"",
"mapping.tester uno                  CPP:bla             -s path.test \"uno-->uno:bla-->CPP:bla\"",
"mapping.tester CPP                  uno:bla             -s path.test \"CPP-->uno-->uno:bla\"",
"mapping.tester CPP:bla              uno:blubb           -s path.test \"CPP:bla-->uno:bla-->uno-->uno:blubb\"",
"mapping.tester CPP:test:bla         uno:test:blubb      -s path.test \"CPP:test:bla-->uno:test:bla-->uno:test-->uno:test:blubb\"",
"mapping.tester CPP:bla              CPP                 -s path.test \"CPP:bla-->uno:bla-->uno-->CPP\"",
"mapping.tester CPP                  CPP:bla             -s path.test \"CPP-->uno-->uno:bla-->CPP:bla\"",
"mapping.tester CPP:bla:test         CPP                 -s path.test \"CPP:bla:test-->uno:bla:test-->uno:bla-->uno-->CPP\"",
"mapping.tester CPP                  CPP:bla:test        -s path.test \"CPP-->uno-->uno:bla-->uno:bla:test-->CPP:bla:test\"",
"mapping.tester CPP:bla              CPP:blubb           -s path.test \"CPP:bla-->uno:bla-->uno-->uno:blubb-->CPP:blubb\"",
"mapping.tester CPP:test:bla         CPP:blubb           -s path.test \"CPP:test:bla-->uno:test:bla-->uno:test-->uno-->uno:blubb-->CPP:blubb\"",
"mapping.tester CPP:bla              CPP:test:blubb      -s path.test \"CPP:bla-->uno:bla-->uno-->uno:test-->uno:test:blubb-->CPP:test:blubb\"",
"mapping.tester CPP:bae:bla          CPP:test:blubb      -s path.test \"CPP:bae:bla-->uno:bae:bla-->uno:bae-->uno-->uno:test-->uno:test:blubb-->CPP:test:blubb\"",
"mapping.tester CPP:test:bla         CPP:test:blubb      -s path.test \"CPP:test:bla-->uno:test:bla-->uno:test-->uno:test:blubb-->CPP:test:blubb\""
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
