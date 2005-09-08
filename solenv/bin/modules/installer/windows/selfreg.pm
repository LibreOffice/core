#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: selfreg.pm,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 09:21:44 $
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