#**************************************************************
#
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#
#**************************************************************



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

            if ( $onefile->{'UnixRights'} )
        {
            my $localcall = "chmod $onefile->{'UnixRights'} $destination \>\/dev\/null 2\>\&1";
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

        if ( $onefile->{'UnixRights'} )
        {
            my $localcall = "chmod $onefile->{'UnixRights'} $destination \>\/dev\/null 2\>\&1";
            system($localcall);
        }
    }

    # Analyzing the log file

    installer::worker::analyze_and_save_logfile($loggingdir, $installdir, $installlogdir, $allsettingsarrayref, $languagestringref, $current_install_number);

    # That's all

    exit(0);
}

1;
