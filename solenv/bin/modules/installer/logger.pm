#*************************************************************************
#
#   $RCSfile: logger.pm,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: svesik $ $Date: 2004-04-20 12:27:51 $
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

package installer::logger;

use installer::files;
use installer::globals;

####################################################
# Including header files into the logfile
####################################################

sub include_header_into_logfile
{
    my ($message) = @_;

    my $infoline;

    $infoline = "\n######################################################\n";
    push( @installer::globals::logfileinfo, $infoline);

    $infoline = "$message\n";
    push( @installer::globals::logfileinfo, $infoline);


    $infoline = "######################################################\n";
    push( @installer::globals::logfileinfo, $infoline);
}

#########################################################
# Including global logging info into global log array
#########################################################

sub globallog
{
    my ($message) = @_;

    my $infoline;

    $infoline = "\n################################################################\n";
    push( @installer::globals::globallogfileinfo, $infoline);

    $infoline = "$message\n";
    push( @installer::globals::globallogfileinfo, $infoline);

    $infoline = "################################################################\n";
    push( @installer::globals::globallogfileinfo, $infoline);

}

###############################################################
# For each product (new language) a new log file is created.
# Therefore the global logging has to be saved in this file.
###############################################################

sub copy_globalinfo_into_logfile
{
    for ( my $i = 0; $i <= $#installer::globals::globallogfileinfo; $i++ )
    {
        push(@installer::globals::logfileinfo, $installer::globals::globallogfileinfo[$i]);
    }
}

###############################################################
# For each product (new language) a new log file is created.
# Therefore the global logging has to be saved in this file.
###############################################################

sub debuginfo
{
    my  ( $message ) = @_;

    $message = $message . "\n";
    push(@installer::globals::functioncalls, $message);
}

###############################################################
# Saving the debug information.
###############################################################

sub savedebug
{
    my ( $outputdir ) = @_;

    installer::files::save_file($outputdir . $installer::globals::debugfilename, \@installer::globals::functioncalls);
    print "... writing debug file " . $outputdir . $installer::globals::debugfilename . "\n";
}

1;