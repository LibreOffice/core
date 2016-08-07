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
    print( "    \"\"\n" );
}
