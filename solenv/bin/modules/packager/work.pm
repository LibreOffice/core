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
