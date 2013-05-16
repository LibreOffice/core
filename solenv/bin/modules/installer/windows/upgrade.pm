#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

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
