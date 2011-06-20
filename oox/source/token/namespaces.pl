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

$ARGV0 = shift @ARGV;
$ARGV1 = shift @ARGV;
$ARGV2 = shift @ARGV;
$ARGV3 = shift @ARGV;

# parse input file

open( INFILE, $ARGV0 ) or die "cannot open input file: $!";
my %namespaces;
while( <INFILE> )
{
    # trim newline
    chomp( $_ );
    # trim leading/trailing whitespace
    $_ =~ s/^\s*//g;
    $_ =~ s/\s*$//g;
    # trim comments
    $_ =~ s/^#.*//;
    # skip empty lines
    if( $_ )
    {
        # check for valid characters
        $_ =~ /^([a-zA-Z]+)\s+([a-zA-Z0-9-.:\/]+)\s*$/ or die "Error: invalid character in input data";
        $namespaces{$1} = $2;
    }
}
close( INFILE );

# generate output files

open( IDFILE, ">$ARGV1" ) or die "Error: cannot open output file: $!";
open( NAMEFILE, ">$ARGV2" ) or die "Error: cannot open output file: $!";
open( TXTFILE, ">$ARGV3" ) or die "Error: cannot open output file: $!";

# number of bits to shift the namespace identifier
$shift = 16;

print ( IDFILE "const size_t NMSP_SHIFT = $shift;\n" );

$i = 1;
foreach( keys( %namespaces ) )
{
    print( IDFILE "const sal_Int32 NMSP_$_ = $i << NMSP_SHIFT;\n" );
    $id = $i << $shift;
    print( NAMEFILE "{ $id, \"$namespaces{$_}\" },\n" );
    print( TXTFILE "$id $_ $namespaces{$_}\n" );
    ++$i;
}

close( IDFILE );
close( nameFILE );
close( TXTFILE );
