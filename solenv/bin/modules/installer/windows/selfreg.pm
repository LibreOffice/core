#*************************************************************************
#
#   $RCSfile: selfreg.pm,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: kz $ $Date: 2004-06-11 18:20:23 $
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

package installer::windows::selfreg;

use installer::exiter;
use installer::files;
use installer::globals;
use installer::windows::idtglobal;

##############################################################
# Returning the unique file name for the selfreg table.
##############################################################

sub get_selfreg_file
{
    my ( $filesref, $filename ) = @_;

    my $foundfile = 0;
    my $uniquefilename = "";

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        my $onefile = ${$filesref}[$i];
        my $name = $onefile->{'Name'};

        if ( $name eq $filename )
        {
            $uniquefilename = $onefile->{'uniquename'};
            $foundfile = 1;
            last;
        }
    }

    # It does not need to exist. For example products that do not contain the libraries.
    # if (! $foundfile ) { installer::exiter::exit_program("ERROR: No unique file name found for $filename !", "get_selfreg_file"); }

    return $uniquefilename;
}

##############################################################
# Returning the cost for the selfreg table.
##############################################################

sub get_selfreg_cost
{
    my ( $filename ) = @_;

    return "0";
}

####################################################################################
# Creating the file SelfReg.idt dynamically
# Content:
# File_ Cost
# UpgradeCode VersionMin VersionMax Language Attributes Remove ActionProperty
####################################################################################

sub create_selfreg_table
{
    my ($filesref, $basedir) = @_;

    my @selfregtable = ();

    installer::windows::idtglobal::write_idt_header(\@selfregtable, "selfreg");

    # Registering all libraries listed in @installer::globals::selfreglibraries

    for ( my $i = 0; $i <= $#installer::globals::selfreglibraries; $i++ )
    {
        my $libraryname = $installer::globals::selfreglibraries[$i];

        my $filename = "";

        $filename = get_selfreg_file($filesref, $libraryname);

        if ( $filename eq "" ) { next; }    # library not part of this product

        my %selfreg = ();

        $selfreg{'File_'} = $filename;

        $selfreg{'Cost'} = get_selfreg_cost($libraryname);

        my $oneline = $selfreg{'File_'} . "\t" . $selfreg{'Cost'} . "\n";

        push(@selfregtable, $oneline);
    }

    # Saving the file

    my $selfregtablename = $basedir . $installer::globals::separator . "SelfReg.idt";
    installer::files::save_file($selfregtablename ,\@selfregtable);
    my $infoline = "Created idt file: $selfregtablename\n";
    push(@installer::globals::logfileinfo, $infoline);
}

1;