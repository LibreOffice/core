#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: exiter.pm,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 09:02:55 $
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

    installer::worker::clean_output_tree(); # removing directories created in the output tree

    # If @installer::globals::logfileinfo is not empty, it can be used.
    # Otherwise the content of @installer::globals::globallogfileinfo has to be used.

    my $infoline;

    $installer::globals::logfilename = $installer::globals::exitlog . $installer::globals::logfilename;

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
