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

# operation mode (1 = identifiers, 2 = names, 3 = gperf)
$op = shift @ARGV;
die "Error: invalid operation" unless( $op >= 1 && $op <= 3);

if( $op == 3 ) {
    print( "%language=C++\n" );
    print( "%global-table\n" );
    print( "%null-strings\n" );
    print( "%struct-type\n" );
    print( "struct xmltoken {\n" );
    print( "    const sal_Char *name;\n" );
    print( "    sal_Int32 nToken;\n" );
    print( "};\n" );
    print( "%%\n" );
}

$i = 0;
while( <> )
{
    # trim newline
    chomp( $_ );
    # trim leading/trailing whitespace
    $_ =~ s/^\s*//g;
    $_ =~ s/\s*$//g;
    # skip empty lines
    if( $_ ) {
        # check for valid characters
        $_ =~ /^[a-zA-Z0-9-_]+$/ or die "Error: invalid entry: '$_'";
        # generate output
        $id = "XML_$_";
        $id =~ s/-/_/g;
        if( $op == 1 ) {
            print( "const sal_Int32 $id = $i;\n" );
        } elsif( $op == 2 ) {
            print( "\"$_\",\n" );
        } elsif( $op == 3 ) {
            print( "$_,$id\n" );
        }
        ++$i;
    }
}

if( $op == 1 ) {
    print( "const sal_Int32 XML_TOKEN_COUNT = $i;\n" );
} elsif( $op == 3 ) {
    print( "%%\n" );
}
