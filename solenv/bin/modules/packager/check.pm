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

package packager::check;

use packager::exiter;
use packager::globals;

##############################################
# Check 1: The package list has to exist
##############################################

sub check_packlist
{
    my $projectdir = $ENV{'PRJ'};
    $projectdir =~ s/$packager::globals::separator\s*$//;
    $packager::globals::packlistname = $projectdir . $packager::globals::separator . "util" . $packager::globals::separator . $packager::globals::packlistname;

    if ( ! -f $packager::globals::packlistname )
    {
        packager::exiter::exit_program("ERROR: Package list not found: $packager::globals::packlistname", "check_packlist");
    }
}

#############################################################
# Check 2: The environment variable OUTPATH has to be set
#############################################################

sub check_environment
{
    if ( ! $ENV{'OUTPATH'} )
    {
        packager::exiter::exit_program("ERROR: Environment variable OUTPATH not set!", "check_environment");
    }

    if ( ! $ENV{'PRJ'} )
    {
        packager::exiter::exit_program("ERROR: Environment variable PRJ not set!", "check_environment");
    }
}

#############################################################
# Check 3: Checking the parameter. Only "-i" is valid
#############################################################

sub check_parameter
{
    while ( $#ARGV >= 0 )
    {
        my $param = shift(@ARGV);

        if ($param eq "-i") { $packager::globals::ignoreerrors = 1; }
        else
        {
            print("\n*************************************\n");
            print("Sorry, unknown parameter: $param");
            print("\n*************************************\n");
            usage();
            exit(-1);
        }
    }
}

1;
