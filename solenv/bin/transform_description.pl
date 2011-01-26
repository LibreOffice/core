#!/usr/bin/perl
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2011 Oracle and/or its affiliates.
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

parse_args();
execute_args();
exit(0);

my $source = undef;
my $dest = undef;
my @languages = undef;

sub parse_args
{
    # at most two arguments
    explain(), exit(100) if ( $#ARGV > 1 );

    # destination file is the second argument, if present
    $dest = $ARGV[1] if ( $#ARGV > 0 );

    # source file is the first argument if present
    if ( $#ARGV > -1 )
    {
        $source = $ARGV[0];
        if ( ! -f $source )
        {
            print STDERR "$source is not a valid file, aborting";
            exit(101);
        }
    }

    # check which languages to use
    my $languages = $ENV{WITH_LANG};
    if ( ( ! defined $languages ) || ( "$languages" eq "" ) )
    {
        print STDERR "$0: WITH_LANG not set or empty, defaulting to 'en-US'\n";
        $languages = "en-US";
    }
    @languages = split ( ' ', $languages );
}

sub execute_args
{
    my @description = ();
    if ( defined $source )
    {
        open SOURCE, "$source" || die "could not open $source: $?\n";
        @description = <SOURCE>;
        close SOURCE;
    }
    else
    {
        @description = <STDIN>;
    }

    if ( defined $dest )
    {
        open DEST, ">$dest" || die "could not open $dest for writing: $?\n";
    }

    foreach (@description)
    {
        chomp; s/\r//;

        if ( /\#LANG\#/ )
        {
            foreach $lang ( @languages )
            {
                my $transformed = $_;
                $transformed =~ s/\#LANG#/$lang/g;
                if ( defined $dest )
                {
                    print DEST "$transformed\n";
                }
                else
                {
                    print STDOUT "$transformed\n";
                }
            }
        }
        else
        {
            if ( defined $dest )
            {
                print DEST "$_\n";
            }
            else
            {
                print STDOUT "$_\n";
            }
        }
    }

    close DEST if ( defined $dest );
}

# explains the program's usage
sub explain
{
    print STDOUT "usage:\n";
    print STDOUT "  $0 [<description_file> [<output_file>]]\n";
    print STDOUT "  transforms the given extension description file\n";
    print STDOUT "\n";
    print STDOUT "  If <output_file> is not given, STDOUT is used.\n";
    print STDOUT "  If <description_file> is not given, STDIN is used.\n";
    print STDOUT "\n";
    print STDOUT "  The following transformations are done at the moment:\n";
    print STDOUT "  - duplicate all lines containing #LANG#, for ever token of \$WITH_LANG\n";
    print STDOUT "    replacing every occurance of \$LANG with a token\n";
    print STDOUT "\n";
    print STDOUT "  And yes, the functionality of this script should be\n";
    print STDOUT "  - moved to solenv/inc/tg_ext.mk\n";
    print STDOUT "  - implemented as XSLT, to be much less error-prone\n";
}
