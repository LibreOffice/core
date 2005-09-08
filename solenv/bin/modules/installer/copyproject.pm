#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: copyproject.pm,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 09:00:51 $
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

        if ( $destination =~ /install\s*$/ )
        {
            my $localcall = "chmod 775 $destination \>\/dev\/null 2\>\&1";
            system($localcall);
        }
    }

    # copy ScpActions

    for ( my $i = 0; $i <= $#{$scpactionsref}; $i++ )
    {
        my $onefile = ${$scpactionsref}[$i];

        my $source = $onefile->{'sourcepath'};
        my $destination = $installdir . $installer::globals::separator . $onefile->{'DestinationName'};

        installer::systemactions::copy_one_file($source, $destination);

        if ( $destination =~ /install\s*$/ )
        {
            my $localcall = "chmod 775 $destination \>\/dev\/null 2\>\&1";
            system($localcall);
        }
    }

    # Analyzing the log file

    installer::worker::analyze_and_save_logfile($loggingdir, $installdir, $installlogdir, $allsettingsarrayref, $languagestringref, $current_install_number);

    # That's all

    exit(0);
}

1;