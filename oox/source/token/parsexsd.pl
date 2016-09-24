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
