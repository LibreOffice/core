#*************************************************************************
#
#   $RCSfile: copyproject.pm,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2004-07-06 14:56:20 $
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

package installer::copyproject;

use installer::control;
use installer::converter;
use installer::files;
use installer::globals;
use installer::logger;
use installer::mail;
use installer::systemactions;
use installer::worker;

####################################################
# Including header files into the logfile
####################################################

sub copy_project
{
    my ( $filesref, $scpactionsref, $loggingdir, $languagestringref, $shipinstalldir, $allsettingsarrayref ) = @_;

    # Creating directories

    installer::logger::include_header_into_logfile("Creating installation directory");

    my $current_install_number = "";

    my $installdir = installer::worker::create_installation_directory($shipinstalldir, $languagestringref, \$current_install_number);

    my $installlogdir = installer::systemactions::create_directory_next_to_directory($installdir, "log");

    # Copy files and ScpActions

    installer::logger::include_header_into_logfile("Copying files:");

    # copy Files

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        my $onefile = ${$filesref}[$i];

        my $source = $onefile->{'sourcepath'};
        my $destination = $installdir . $installer::globals::separator . $onefile->{'Name'};

        installer::systemactions::copy_one_file($source, $destination);
    }

    # copy ScpActions

    for ( my $i = 0; $i <= $#{$scpactionsref}; $i++ )
    {
        my $onefile = ${$scpactionsref}[$i];

        my $source = $onefile->{'sourcepath'};
        my $destination = $installdir . $installer::globals::separator . $onefile->{'Name'};

        installer::systemactions::copy_one_file($source, $destination);
    }

    # Analyzing the log file

    installer::worker::analyze_and_save_logfile($loggingdir, $installdir, $installlogdir, $allsettingsarrayref, $languagestringref, $current_install_number);

    # That's all

    exit(0);
}

1;