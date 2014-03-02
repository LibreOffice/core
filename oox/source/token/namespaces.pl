#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

$ARGV0 = shift @ARGV;
$ARGV1 = shift @ARGV;
$ARGV2 = shift @ARGV;
$ARGV3 = shift @ARGV;
$ARGV4 = shift @ARGV;
$ARGV5 = shift @ARGV;

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
        $_ =~ /^([a-zA-Z][a-zA-Z0-9]*)\s+([a-zA-Z0-9-.:\/]+)\s*$/ or die "Error: invalid character in input data";
        $namespaces{$1} = $2;
    }
}
close( INFILE );

# OOXML strict namespaces

open( INFILE_STRICT, $ARGV4 ) or die "cannot open input file: $!";
my %namespaces_strict;
while( <INFILE_STRICT> )
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
        $_ =~ /^([a-zA-Z][a-zA-Z0-9]*)\s+([a-zA-Z0-9-.:\/]+)\s*$/ or die "Error: invalid character in input data";
        $namespaces_strict{$1} = $2;
    }
}
close( INFILE_STRICT );

# generate output files

open( IDFILE, ">$ARGV1" ) or die "Error: cannot open output file: $!";
open( NAMEFILE, ">$ARGV2" ) or die "Error: cannot open output file: $!";
open( TXTFILE, ">$ARGV3" ) or die "Error: cannot open output file: $!";
open( NAMEFILE_STRICT, ">$ARGV5" ) or die "Error: cannot open output file: $!";

# number of bits to shift the namespace identifier
$shift = 16;

print ( IDFILE "const size_t NMSP_SHIFT = $shift;\n" );

$i = 1;
foreach( keys( %namespaces ) )
{
    print( IDFILE "const sal_Int32 NMSP_$_ = $i << NMSP_SHIFT;\n" );
    $id = $i << $shift;
    print( NAMEFILE "{ $id, \"$namespaces{$_}\" },\n" );
    print( NAMEFILE_STRICT "{ $id, \"$namespaces_strict{$_}\" },\n" );
    print( TXTFILE "$id $_ $namespaces{$_}\n" );
    ++$i;
}

close( IDFILE );
close( NAMEFILE );
close( NAMEFILE_STRICT );
close( TXTFILE );
