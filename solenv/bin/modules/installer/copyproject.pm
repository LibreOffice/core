#*************************************************************************
#
#   $RCSfile: copyproject.pm,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: svesik $ $Date: 2004-04-20 12:26:04 $
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
use installer::files;
use installer::globals;
use installer::logger;
use installer::systemactions;

####################################################
# Including header files into the logfile
####################################################

sub copy_project
{
    my ( $filesref, $scpactionsref, $loggingdir, $languagestringref ) = @_;

    # create installation directory

    installer::logger::include_header_into_logfile("Creating installation directory");

    my $installdir = installer::systemactions::create_directories("install", $languagestringref);

    installer::logger::include_header_into_logfile("Copying files:");

    # copy Files

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        my $onefile = ${$filesref}[$i];

        my $source = $onefile->{'sourcepath'};
        my $destination = $installdir . $installer::globals::separator . $onefile->{'Name'};

        installer::systemactions::copy_one_file($source, $destination);
    }

    # copy ScpActions

    for ( my $i = 0; $i <= $#{$scpactionsref}; $i++ )
    {
        my $onefile = ${$scpactionsref}[$i];

        my $source = $onefile->{'sourcepath'};
        my $destination = $installdir . $installer::globals::separator . $onefile->{'Name'};

        installer::systemactions::copy_one_file($source, $destination);
    }

    ####################################
    # Writing log file
    ####################################

    print "\n... creating log file $installer::globals::logfilename \n";
    installer::files::save_file($loggingdir . $installer::globals::logfilename, \@installer::globals::logfileinfo);

    #######################################################
    # Analyzing the log file
    #######################################################

    print "... checking log file " . $loggingdir . $installer::globals::logfilename . "\n";

    installer::control::check_logfile(\@installer::globals::logfileinfo);

    # That's all

    exit(0);
}

1;