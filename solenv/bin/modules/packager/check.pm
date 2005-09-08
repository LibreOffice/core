#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: check.pm,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 09:22:46 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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
