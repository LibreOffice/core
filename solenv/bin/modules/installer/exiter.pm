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



package installer::exiter;

use installer::files;
use installer::globals;
use installer::logger;
use installer::systemactions;
use installer::worker;

############################################
# Exiting the program with an error
# This function is used instead of "die"
############################################

sub exit_program
{
    my ($message, $function) = @_;

    # If an installation set is currently created, the directory name is saved in $installer::globals::saveinstalldir
    # If this directory name matches with "_inprogress", it has to be renamed into "_witherror"

    if ( $installer::globals::saveinstalldir =~ /_inprogress/ ) { installer::systemactions::rename_string_in_directory($installer::globals::saveinstalldir, "_inprogress", "_witherror");   }

    # Cleaning files from pool tooling
    if ( $installer::globals::processhaspoolcheckfile ) { unlink $installer::globals::poolcheckfilename; }
    if ( $installer::globals::processhaspoollockfile ) { unlink $installer::globals::poollockfilename; }

    installer::worker::clean_output_tree(); # removing directories created in the output tree

    # If @installer::globals::logfileinfo is not empty, it can be used.
    # Otherwise the content of @installer::globals::globallogfileinfo has to be used.

    my $infoline;

    $installer::globals::logfilename = $installer::globals::exitlog . $installer::globals::logfilename;

    if ( ! $installer::globals::globalinfo_copied ) { installer::logger::copy_globalinfo_into_logfile(); }

    if ( $#installer::globals::logfileinfo > -1 )
    {
        $installer::logger::Lang->print("\n");
        $installer::logger::Lang->print("***************************************************************\n");
        $installer::logger::Lang->print($message."\n");
        $installer::logger::Lang->printf("in function: %s\n", $function);
        $installer::logger::Lang->printf("***************************************************************\n");

#       installer::files::save_file($installer::globals::logfilename ,\@installer::globals::logfileinfo);
    }
    else
    {
        $installer::logger::Global->print("\n");
        $installer::logger::Global->print("***************************************************************\n");
        $installer::logger::Global->print($message."\n");
        $installer::logger::Global->printf("in function: %s\n", $function);
        $installer::logger::Global->printf("***************************************************************\n");

#       installer::files::save_file($installer::globals::logfilename ,\@installer::globals::globallogfileinfo);
    }
    installer::logger::print_error("$message\nin function: $function");
#   installer::logger::print_error("Saved logfile: $installer::globals::logfilename\n");

    # Saving the debug info

    if ( $installer::globals::debug ) { installer::logger::savedebug($installer::globals::exitlog); }

    installer::logger::stoptime();

    if (defined($installer::globals::exithandler)) {
        &$installer::globals::exithandler;
    }

    exit(-1);
}

1;
