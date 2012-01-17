#**************************************************************
#
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#
#**************************************************************



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
