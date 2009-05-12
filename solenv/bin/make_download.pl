#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: make_installer.pl,v $
#
# $Revision: 1.121 $
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

my $infofilelist = installer::downloadsigner::createproductlist();
installer::downloadsigner::publishproductlist($infofilelist);

foreach my $infofilename ( @{$infofilelist} )
{
    installer::logger::starttime();

    my $success = 1;
    my $do_copy = 1;
    my $followmeinfohash = installer::followme::read_followme_info($infofilename);
    installer::downloadsigner::setlogfilename();

    if (( ! $installer::globals::iswindowsbuild ) && ( $installer::globals::dosign ))
    {
        installer::logger::print_message( "... WARNING: Signing only for Windows platforms active ...\n" );
    }

    installer::logger::include_header_into_logfile("Reading include pathes");
    installer::worker::collect_all_files_from_includepathes($followmeinfohash->{'includepatharray'});

    if (( $installer::globals::iswindowsbuild ) && ( $installer::globals::dosign ))
    {
        $followmeinfohash->{'finalinstalldir'} = installer::windows::sign::sign_install_set($followmeinfohash, $do_copy);

        ($success, $followmeinfohash->{'finalinstalldir'}) = installer::worker::analyze_and_save_logfile($followmeinfohash->{'loggingdir'},
                                                                                                            $followmeinfohash->{'finalinstalldir'},
                                                                                                            $followmeinfohash->{'installlogdir'},
                                                                                                            "",
                                                                                                            \$followmeinfohash->{'languagestring'},
                                                                                                            $followmeinfohash->{'currentinstallnumber'});

        if ( ! $success ) { installer::exiter::exit_program("ERROR: Signing installation set failed: $followmeinfohash->{'finalinstalldir'}", "Main"); }
    }

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
        $followmeinfohash->{'finalinstalldir'} = installer::windows::sign::sign_install_set($followmeinfohash, $do_copy);

        $followmeinfohash->{'finalinstalldir'} = installer::worker::analyze_and_save_logfile($followmeinfohash->{'loggingdir'},
                                                                                                $followmeinfohash->{'finalinstalldir'},
                                                                                                $followmeinfohash->{'installlogdir'},
                                                                                                "",
                                                                                                \$followmeinfohash->{'languagestring'},
                                                                                                $followmeinfohash->{'currentinstallnumber'});
    }

    installer::logger::stoptime();
}


####################################
# Main program end
####################################
