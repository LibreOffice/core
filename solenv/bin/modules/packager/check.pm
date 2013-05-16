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
