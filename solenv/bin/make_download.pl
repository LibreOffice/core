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

#################
# use
#################

use lib ("$ENV{SOLARENV}/bin/modules");

use Cwd;
use File::Copy;
use installer::download;
use installer::downloadsigner;
use installer::exiter;
use installer::followme;
use installer::globals;
use installer::logger;
use installer::windows::sign;

#################################################
# Main program
#################################################

installer::downloadsigner::getparameter();
installer::downloadsigner::checkparameter();

my $temppath = installer::downloadsigner::set_temp_path();
my $infofilelist = installer::downloadsigner::createproductlist();
installer::downloadsigner::publishproductlist($infofilelist);

foreach my $infofilename ( @{$infofilelist} )
{
    installer::logger::starttime();

    my $success = 1;
    my $do_copy = 1;
    my $followmeinfohash = installer::followme::read_followme_info($infofilename);
    installer::downloadsigner::setlogfilename();    # Successful after reading followme file, resetting log file
    if ( $installer::globals::writetotemp ) { installer::downloadsigner::set_output_pathes_to_temp($followmeinfohash, $temppath); }
    if ( $installer::globals::useminor ) { installer::downloadsigner::set_minor_into_pathes($followmeinfohash, $temppath); }

    if (( ! $installer::globals::iswindowsbuild ) && ( $installer::globals::dosign ))
    {
        installer::logger::print_message( "... WARNING: Signing only for Windows platforms active ...\n" );
    }

    if (( $installer::globals::iswindowsbuild ) && ( $installer::globals::dosign ))
    {
        $followmeinfohash->{'finalinstalldir'} = installer::windows::sign::sign_install_set($followmeinfohash, $do_copy, $temppath);

        ($success, $followmeinfohash->{'finalinstalldir'}) = installer::worker::analyze_and_save_logfile($followmeinfohash->{'loggingdir'},
                                                                                                            $followmeinfohash->{'finalinstalldir'},
                                                                                                            $followmeinfohash->{'installlogdir'},
                                                                                                            "",
                                                                                                            \$followmeinfohash->{'languagestring'},
                                                                                                            $followmeinfohash->{'currentinstallnumber'});

        if ( ! $success ) { installer::exiter::exit_program("ERROR: Signing installation set failed: $followmeinfohash->{'finalinstalldir'}", "Main"); }
    }

    if ( ! $installer::globals::nodownload )
    {
        $followmeinfohash->{'finalinstalldir'} = installer::download::create_download_sets($followmeinfohash->{'finalinstalldir'},
                                                                                            $followmeinfohash->{'includepatharray'},
                                                                                            $followmeinfohash->{'allvariableshash'},
                                                                                            $followmeinfohash->{'downloadname'},
                                                                                            \$followmeinfohash->{'languagestring'},
                                                                                            $followmeinfohash->{'languagesarray'});

        ($success, $followmeinfohash->{'finalinstalldir'}) = installer::worker::analyze_and_save_logfile($followmeinfohash->{'loggingdir'},
                                                                                                        $followmeinfohash->{'finalinstalldir'},
                                                                                                        $followmeinfohash->{'installlogdir'},
                                                                                                        "",
                                                                                                        \$followmeinfohash->{'languagestring'},
                                                                                                        $followmeinfohash->{'currentinstallnumber'});

        if (( $success ) && ( $installer::globals::iswindowsbuild ) && ( $installer::globals::dosign ))
        {
            $do_copy = 0;
            $followmeinfohash->{'finalinstalldir'} = installer::windows::sign::sign_install_set($followmeinfohash, $do_copy, $temppath);

            ($success, $followmeinfohash->{'finalinstalldir'}) = installer::worker::analyze_and_save_logfile($followmeinfohash->{'loggingdir'},
                                                                                                            $followmeinfohash->{'finalinstalldir'},
                                                                                                            $followmeinfohash->{'installlogdir'},
                                                                                                            "",
                                                                                                            \$followmeinfohash->{'languagestring'},
                                                                                                            $followmeinfohash->{'currentinstallnumber'});
        }
    }

    if ( $success )
    {
        installer::worker::clean_output_tree();
        if ( $installer::globals::followme_from_directory ) { installer::downloadsigner::rename_followme_infofile($infofilename); }
    }

    installer::logger::stoptime();
}

####################################
# Main program end
####################################
