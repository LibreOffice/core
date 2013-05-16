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

package installer::copyproject;

use strict;
use warnings;

use installer::control;
use installer::converter;
use installer::files;
use installer::globals;
use installer::logger;
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

        if ( $onefile->{'UnixRights'} )
        {
            chmod oct($onefile->{'UnixRights'}), $destination;
        }
        elsif ( $destination =~ /install\s*$/ )
        {
            chmod 0775, $destination;
        }
    }

    # copy ScpActions

    for ( my $i = 0; $i <= $#{$scpactionsref}; $i++ )
    {
        my $onefile = ${$scpactionsref}[$i];

        my $source = $onefile->{'sourcepath'};
        my $destination = $installdir . $installer::globals::separator . $onefile->{'DestinationName'};

        installer::systemactions::copy_one_file($source, $destination);

        if ( $onefile->{'UnixRights'} )
        {
            chmod oct($onefile->{'UnixRights'}), $destination;
        }
        elsif ( $destination =~ /install\s*$/ )
        {
            chmod 0775, $destination;
        }
    }

    # Analyzing the log file

    installer::worker::analyze_and_save_logfile($loggingdir, $installdir, $installlogdir, $allsettingsarrayref, $languagestringref, $current_install_number);

    # That's all

    exit(0);
}

1;
