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

# operation mode (1 = identifiers, 2 = names)
$op = shift @ARGV;
die "Error: invalid operation" unless( $op >= 1 && $op <= 2);

$i = 0;
while( <> ) {
    # trim newline
    chomp( $_ );
    # trim leading/trailing whitespace
    $_ =~ s/^\s*//g;
    $_ =~ s/\s*$//g;
    # skip empty lines
    if( $_ ) {
        # check for valid characters
        $_ =~ /^[A-Z][a-zA-Z0-9]+$/ or die "Error: invalid entry: '$_'";
        # generate output
        if( $op == 1 ) {
            print( "const sal_Int32 PROP_$_ = $i;\n" );
        } elsif( $op == 2 ) {
            print( "/* $i */ \"$_\",\n" );
        }
        ++$i;
    }
}

if( $op == 1 ) {
    print( "const sal_Int32 PROP_COUNT = $i;\nconst sal_Int32 PROP_INVALID = -1;\n" );
} elsif( $op == 2 ) {
    print( "    \"\"" );
}
