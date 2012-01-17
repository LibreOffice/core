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



$ARGV = shift @ARGV;
my %tokens;

my @files = glob("$ARGV/*.rnc");

open( TOKEN, ">tokens.txt" ) || die "can't write token file";

foreach( @files )
{
    print( "parsing $_\n" );
    open ( XSD, $_ ) || die "can't open token file: $!";
    while( <XSD> )
    {
        chomp($_);
        if( /element (\S*:)?(\S*)/ )
        {
            $tokens{$2} = 1;
            print(".");
        }
        elsif( /attribute (\S*:)?(\S*)/ )
        {
            $tokens{$2} = 1;
            print(".");
        }
        elsif( /list\s*\{/ )
        {
            while( <XSD> )
            {
                chomp($_);
                last if( /^\s*\}/ );
                if( /"(\S*?)\"/ )
                {
                    $tokens{$1} = 1;
                    print(".");
                }
            }
        }
    }
    close ( XSD );

    print("\n" );
}

foreach( sort(keys(%tokens)) )
{
    print TOKEN "$_\n";
}
close( TOKEN );
