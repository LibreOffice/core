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


package packager::work;

use strict;
use warnings;

use base 'Exporter';

use packager::exiter;
use packager::globals;

our @EXPORT_OK = qw(
    set_global_variable
    create_package_todos
    execute_system_calls
);

###########################################
# Setting global variables
###########################################

sub set_global_variable
{
    my $compiler = $ENV{'OUTPATH'};

    if ( $ENV{'PROEXT'} ) { $compiler = $compiler . $ENV{'PROEXT'}; }

    $packager::globals::compiler = $compiler;
}

###########################################
# Generating a list of package calls
# corresponding to the package list
###########################################

sub create_package_todos
{
    my ( $packagelist ) = @_;

    my @targets = ();   # only used, if the build server is not used

    for my $line ( @{$packagelist} ) {
        next if ($line =~ /^\s*\#/);  # comment line

        my ($product, $compilerlist, $languagelist, $target) =
            ($line =~ /^\s*(\w+?)\s+(\S+?)\s+(\S+?)\s+(\w+?)\s*$/);

        my @compilers = split ',', $compilerlist;

        # is the compiler of this "build" part of the compiler list in pack.lst ?

        next unless grep { $_ eq $packager::globals::compiler } @compilers;

        # products are separated in pack.lst by "|"
        # now all information is available to create the targets for the systemcalls
        for my $languagestring (split '\|', $languagelist) {
            $languagestring =~ s/,/_/g;   # comma in pack.lst becomes "_" in dmake command

            push @targets, $target . '_' . $languagestring;
        }
    }

    push @packager::globals::logfileinfo, map { $_ . "\n" } @targets;

    return \@targets;
}

###########################################
# Executing the generated system calls
###########################################

sub execute_system_calls
{
    my ( $targets ) = @_;

    for ( my $i = 0; $i <= $#{$targets}; $i++ )
    {
        my $systemcall = "dmake " . ${$targets}[$i];

        my $infoline = "Packager: $systemcall\n";
        print $infoline;
        push( @packager::globals::logfileinfo, $infoline);

        my $returnvalue = system($systemcall);

        $infoline = "Packager finished: $systemcall\n";
        print $infoline;
        push( @packager::globals::logfileinfo, $infoline);

        if ( $returnvalue )
        {
            $infoline = "\nERROR: Packager $systemcall\n";
            print $infoline;
            push( @packager::globals::logfileinfo, $infoline);
            if (!($packager::globals::ignoreerrors)) { packager::exiter::exit_program("ERROR: Packing not successful : $systemcall", "execute_system_calls"); }
        }
    }
}

1;
