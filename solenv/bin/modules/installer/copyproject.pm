#*************************************************************************
#
#   $RCSfile: copyproject.pm,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: kz $ $Date: 2004-06-11 18:15:15 $
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

    my $installdir = installer::systemactions::create_directories("install", $languagestringref);
    my $numberedinprogressdir = installer::systemactions::make_numbered_dir("inprogress", $installdir);
    my $current_install_number = installer::converter::get_number_from_directory($numberedinprogressdir);
    my $installlogdir = installer::systemactions::create_directory_next_to_directory($numberedinprogressdir, "log");

    # Copy files and ScpActions

    installer::logger::include_header_into_logfile("Copying files:");

    # copy Files

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        my $onefile = ${$filesref}[$i];

        my $source = $onefile->{'sourcepath'};
        my $destination = $numberedinprogressdir . $installer::globals::separator . $onefile->{'Name'};

        installer::systemactions::copy_one_file($source, $destination);
    }

    # copy ScpActions

    for ( my $i = 0; $i <= $#{$scpactionsref}; $i++ )
    {
        my $onefile = ${$scpactionsref}[$i];

        my $source = $onefile->{'sourcepath'};
        my $destination = $numberedinprogressdir . $installer::globals::separator . $onefile->{'Name'};

        installer::systemactions::copy_one_file($source, $destination);
    }

    # Analyzing the log file

    print "... checking log file " . $loggingdir . $installer::globals::logfilename . "\n";

    my $contains_error = installer::control::check_logfile(\@installer::globals::logfileinfo);

    if ( $contains_error )
    {
        my $errordir = installer::systemactions::rename_string_in_directory($numberedinprogressdir, "inprogress", "with_error");
        if ( $installer::globals::updatepack ) { installer::mail::send_fail_mail($allsettingsarrayref, $languagestringref, $errordir); }
    }
    else
    {
        my $destdir = installer::systemactions::rename_string_in_directory($numberedinprogressdir, "inprogress", "");

        if ( $installer::globals::updatepack )
        {
            my $completeshipinstalldir = installer::worker::copy_install_sets_to_ship($destdir, $shipinstalldir);
            installer::mail::send_success_mail($allsettingsarrayref, $languagestringref, $completeshipinstalldir);
        }
    }

    my $numberedlogfilename = $installer::globals::logfilename . "_" . $current_install_number;

    # Saving the logfile in the log file directory and additionally in a log directory in the install directory

    print "... creating log file $numberedlogfilename \n";
    installer::files::save_file($loggingdir . $numberedlogfilename, \@installer::globals::logfileinfo);
    installer::files::save_file($installlogdir . $installer::globals::separator . $numberedlogfilename, \@installer::globals::logfileinfo);

    # That's all

    exit(0);
}

1;