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
