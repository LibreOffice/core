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

package installer::windows::upgrade;

use installer::exiter;
use installer::files;
use installer::globals;
use installer::windows::idtglobal;

####################################################################################
# Creating the file Upgrade.idt dynamically
# Content:
# UpgradeCode VersionMin VersionMax Language Attributes Remove ActionProperty
####################################################################################

sub create_upgrade_table
{
    my ($basedir, $allvariableshashref) = @_;

    my @upgradetable = ();

    installer::windows::idtglobal::write_idt_header(\@upgradetable, "upgrade");

    # Setting all products, that must be removed.
    my $newline = $installer::globals::upgradecode . "\t" . "\t" . $installer::globals::msiproductversion . "\t" . "\t" . "513" . "\t" . "\t" . "OLDPRODUCTS" . "\n";
    push(@upgradetable, $newline);

    # preventing downgrading
    $newline = $installer::globals::upgradecode . "\t" . $installer::globals::msiproductversion . "\t" . "\t" . "\t" . "2" . "\t" . "\t" . "NEWPRODUCTS" . "\n";
    push(@upgradetable, $newline);

    # Saving the file

    my $upgradetablename = $basedir . $installer::globals::separator . "Upgrade.idt";
    installer::files::save_file($upgradetablename ,\@upgradetable);
    my $infoline = "Created idt file: $upgradetablename\n";
    push(@installer::globals::logfileinfo, $infoline);
}

##############################################################
# Reading the file with UpgradeCodes of old products,
# that can be removed, if the user wants to remove them.
##############################################################

sub analyze_file_for_upgrade_table
{
    my ($filecontent) = @_;

    my @allnewlines = ();

    for ( my $i = 0; $i <= $#{$filecontent}; $i++ )
    {
        my $line = ${$filecontent}[$i];
        if ( $line =~ /^\s*$/ ) { next; } # empty lines can be ignored
        if ( $line =~ /^\s*\#/ ) { next; } # comment lines starting with a hash

        if ( $line =~ /^(.*)\t(.*)\t(.*)\t(.*)\t(.*)\t(.*)\t(.*)$/ ) { push(@allnewlines, $line); }
        else { installer::exiter::exit_program("ERROR: Wrong syntax in file for upgrade table", "analyze_file_for_upgrade_table"); }
    }

    return \@allnewlines;
}

1;
