#*************************************************************************
#
#   $RCSfile: selfreg.pm,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2004-12-16 10:46:32 $
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
use installer::worker;
use installer::windows::idtglobal;

##############################################################
# Returning the cost for the selfreg table.
##############################################################

sub get_selfreg_cost
{
    my ( $onefile ) = @_;

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

    # Registering all libraries with flag "SELFREG"

    my $selfregfiles = installer::worker::collect_all_items_with_special_flag($filesref, "SELFREG");

    for ( my $i = 0; $i <= $#{$selfregfiles}; $i++ )
    {
        my $onefile = ${$selfregfiles}[$i];

        my %selfreg = ();

        $selfreg{'File_'} = $onefile->{'uniquename'};
        $selfreg{'Cost'} = get_selfreg_cost($onefile);

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