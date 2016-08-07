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



# operation mode (1 = identifiers, 2 = names, 3 = gperf)
$op = shift @ARGV;
die "Error: invalid operation" unless( $op >= 1 && $op <= 3);

if( $op == 3 ) {
    print( "%language=C++\n" );
    print( "%define slot-name mpcName\n" );
    print( "%define initializer-suffix ,0\n" );
    print( "%define lookup-function-name getTokenInfo\n" );
    print( "%compare-strncmp\n" );
    print( "%readonly-tables\n" );
    print( "%enum\n" );
    print( "%null-strings\n" );
    print( "%struct-type\n" );
    print( "struct XMLTokenInfo {\n" );
    print( "    const sal_Char* mpcName;\n" );
    print( "    sal_Int32       mnToken;\n" );
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
