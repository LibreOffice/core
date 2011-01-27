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

open( INFILE, $ARGV0 ) or die "Error: cannot open input file: $!";
my %tokens;
while ( <INFILE> )
{
    # trim newline
    chomp( $_ );
    # trim leading/trailing whitespace
    $_ =~ s/^\s*//g;
    $_ =~ s/\s*$//g;
    # check for valid characters
    $_ =~ /^[a-zA-Z0-9-_]+$/ or die "Error: invalid character in token '$_'";
    $id = "XML_$_";
    $id =~ s/-/_/g;
    $tokens{$_} = $id;
}
close ( INFILE );

# generate output files

open ( IDFILE, ">$ARGV1" ) or die "Error: cannot open output file: $!";
open ( NAMEFILE, ">$ARGV2" ) or die "Error: cannot open output file: $!";
open ( GPERFFILE, ">$ARGV3" ) or die "Error: cannot open output file: $!";

print( GPERFFILE "%language=C++\n" );
print( GPERFFILE "%global-table\n" );
print( GPERFFILE "%null-strings\n" );
print( GPERFFILE "%struct-type\n" );
print( GPERFFILE "struct xmltoken {\n" );
print( GPERFFILE "    const sal_Char *name;\n" );
print( GPERFFILE "    sal_Int32 nToken;\n" );
print( GPERFFILE "};\n" );
print( GPERFFILE "%%\n" );

$i = 0;
foreach( sort( keys( %tokens ) ) )
{
    print( IDFILE "const sal_Int32 $tokens{$_} = $i;\n" );
    print( NAMEFILE "\"$_\",\n" );
    print( GPERFFILE "$_,$tokens{$_}\n" );
    ++$i;
}

print( IDFILE "const sal_Int32 XML_TOKEN_COUNT = $i;\n" );
print( GPERFFILE "%%\n" );

close( IDFILE );
close( NAMEFILE );
close( GPERFFILE );
