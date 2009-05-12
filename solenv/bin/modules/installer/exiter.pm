#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: exiter.pm,v $
#
# $Revision: 1.8 $
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
        $infoline = "\n***************************************************************\n";
        push(@installer::globals::logfileinfo, $infoline);

        $infoline = "$message\n";
        push(@installer::globals::logfileinfo, $infoline);

        $infoline = "in function: $function\n";
        push(@installer::globals::logfileinfo, $infoline);

        $infoline = "***************************************************************\n";
        push(@installer::globals::logfileinfo, $infoline);

        installer::files::save_file($installer::globals::logfilename ,\@installer::globals::logfileinfo);
    }
    else
    {
        $infoline = "\n***************************************************************\n";
        push(@installer::globals::globallogfileinfo, $infoline);

        $infoline = "$message\n";
        push(@installer::globals::globallogfileinfo, $infoline);

        $infoline = "in function: $function\n";
        push(@installer::globals::globallogfileinfo, $infoline);

        $infoline = "***************************************************************\n";
        push(@installer::globals::globallogfileinfo, $infoline);

        installer::files::save_file($installer::globals::logfilename ,\@installer::globals::globallogfileinfo);
    }
    installer::logger::print_error("$message\nin function: $function");
    installer::logger::print_error("Saved logfile: $installer::globals::logfilename\n");

    # Saving the debug info

    if ( $installer::globals::debug ) { installer::logger::savedebug($installer::globals::exitlog); }

    installer::logger::stoptime();

    exit(-1);
}

1;
