#*************************************************************************
#
#   $RCSfile: check.pm,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: obo $ $Date: 2004-11-18 08:41:19 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc..
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
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
