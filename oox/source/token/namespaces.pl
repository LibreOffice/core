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



# operation mode (1 = identifiers, 2 = names, 3 = plain)
$op = shift @ARGV;
die "Error: invalid operation" unless( $op >= 1 && $op <= 3);

# number of bits to shift the namespace identifier
$shift = 16;

if( $op == 1 ) {
    print( "const size_t NMSP_SHIFT = $shift;\n" );
}

$i = 1;
while( <> ) {
    # trim newline
    chomp( $_ );
    # trim leading/trailing whitespace
    $_ =~ s/^\s*//g;
    $_ =~ s/\s*$//g;
    # trim comments
    $_ =~ s/^#.*//;
    # skip empty lines
    if( $_ ) {
        # check for valid characters
        $_ =~ /^([a-zA-Z]+)\s+([a-zA-Z0-9-.:\/]+)\s*$/ or die "Error: invalid entry: '$_'";
        # generate output
        $id = $i << $shift;
        if( $op == 1 )      { print( "const sal_Int32 NMSP_$1 = $i << NMSP_SHIFT;\n" ); }
        elsif( $op == 2 )   { print( "{ $id, \"$2\" },\n" ); }
        elsif( $op == 3 )   { print( "$id $1 $2\n" ); }
        ++$i;
    }
}
