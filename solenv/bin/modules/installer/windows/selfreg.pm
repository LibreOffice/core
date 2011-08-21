#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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