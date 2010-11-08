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

# parse input file

open( INFILE, $ARGV0 ) or die "Error: cannot open input file: $!";
my %props;
while( <INFILE> )
{
    # trim newline
    chomp( $_ );
    # trim leading/trailing whitespace
    $_ =~ s/^\s*//g;
    $_ =~ s/\s*$//g;
    # check for valid characters
    $_ =~ /^[A-Z][a-zA-Z0-9]*$/ or die "Error: invalid character in property '$_'";
    $id = "PROP_$_";
    $props{$_} = $id;
}
close( INFILE );

# generate output files

open( IDFILE, ">$ARGV1" ) or die "Error: cannot open output file: $!";
open( NAMEFILE, ">$ARGV2" ) or die "Error: cannot open output file: $!";

$i = 0;
foreach( sort( keys( %props ) ) )
{
    print( IDFILE "const sal_Int32 $props{$_} = $i;\n" );
    print( NAMEFILE "/* $i */ \"$_\",\n" );
    ++$i;
}

print( IDFILE "const sal_Int32 PROP_COUNT = $i;\n" );
print( IDFILE "const sal_Int32 PROP_INVALID = -1;\n" );

close( IDFILE );
close( NAMEFILE );
