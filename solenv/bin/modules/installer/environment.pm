#*************************************************************************
#
#   $RCSfile: environment.pm,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: rt $ $Date: 2004-07-30 16:37:34 $
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

package installer::environment;

use installer::exiter;
use installer::globals;

######################################################
# Create path variables from environment variables
######################################################

sub create_pathvariables
{
    my ($environment) = @_;

    my %variables = ();

    # The following variables are needed in the path file list
    # solarpath, solarenvpath, solarcommonpath, os, osdef, pmiscpath

    my $solarpath = $environment->{'SOLARVERSION'} . $installer::globals::separator . $installer::globals::compiler . $installer::globals::productextension;
    $variables{'solarpath'} = $solarpath;

    my $solarcommonpath = $environment->{'SOLARVERSION'} . $installer::globals::separator . $environment->{'COMMON_OUTDIR'} . $installer::globals::productextension;
    $variables{'solarcommonpath'} = $solarcommonpath;

    my $osdef = lc($environment->{'GUI'});
    $variables{'osdef'} = $osdef;

    $variables{'os'} = $installer::globals::compiler;

#   my $solarenvpath  = $environment->{'ENV_ROOT'} . $installer::globals::separator . "solenv" . $installer::globals::separator . "inst";
#   $variables{'solarenvpath'} = $solarenvpath;

    my $solarenvpath = "";

    if ( $ENV{'SO_PACK'} ) { $solarenvpath  = $ENV{'SO_PACK'}; }
    # overriding with STAR_INSTPATH, if set
    if ( $ENV{'STAR_INSTPATH'} ) { $solarenvpath = $ENV{'STAR_INSTPATH'}; }

    $variables{'solarenvpath'} = $solarenvpath;

    my $localpath  = $environment->{'LOCAL_OUT'};
    $variables{'localpath'} = $localpath;

    my $localcommonpath  = $environment->{'LOCAL_COMMON_OUT'};
    $variables{'localcommonpath'} = $localcommonpath;

    my $platformname  = $environment->{'OUTPATH'};
    $variables{'platformname'} = $platformname;

    return \%variables;
}

##################################################
# Setting some fundamental global variables.
# All these variables can be overwritten
# by parameters.
##################################################

sub set_global_environment_variables
{
    my ( $environment ) = @_;

    $installer::globals::build = $environment->{'WORK_STAMP'};
    # $installer::globals::minor = $environment->{'UPDMINOR'};
    $installer::globals::compiler = $environment->{'OUTPATH'};

    if ( $ENV{'UPDMINOR'} ) { $installer::globals::minor = $ENV{'UPDMINOR'}; }
    if ( $ENV{'LAST_MINOR'} ) { $installer::globals::lastminor = $ENV{'LAST_MINOR'}; }

    if ( $ENV{'PROEXT'} ) { $installer::globals::pro = 1; }
    if ( $ENV{'SOLAR_JAVA'} ) { $installer::globals::solarjava = 1; }
    if ( $ENV{'JDKLIB'} ) { $installer::globals::jdklib = $ENV{'JDKLIB'}; }
    if ( $ENV{'JREPATH'} ) { $installer::globals::jrepath = $ENV{'JREPATH'}; }
}

1;
