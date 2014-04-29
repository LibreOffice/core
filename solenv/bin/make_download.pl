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

    # installer::logger::include_header_into_logfile("Reading include paths");
    # installer::worker::collect_all_files_from_includepathes($followmeinfohash->{'includepatharray'});

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
