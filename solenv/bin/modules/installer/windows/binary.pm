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

package installer::windows::binary;

use strict;
use warnings;

use installer::files;
use installer::globals;

###########################################################################################################
# Updating the table Binary dynamically with all files from $binarytablefiles
# Content:
# Name  Data
# s72   v0
# Binary    Name
###########################################################################################################

sub update_binary_table
{
    my ($languageidtdir, $filesref, $binarytablefiles) = @_;

    my $binaryidttablename = $languageidtdir . $installer::globals::separator . "Binary.idt";
    my $binaryidttable = installer::files::read_file($binaryidttablename);

    # Only the iconfiles, that are used in the shortcut table for the
    # FolderItems (entries in Windows startmenu) are added into the icon table.

    for ( my $i = 0; $i <= $#{$binarytablefiles}; $i++ )
    {
        my $binaryfile = ${$binarytablefiles}[$i];
        my $binaryfilename = $binaryfile->{'Name'};
        my $binaryfiledata = $binaryfilename;

        $binaryfilename =~ s/\.//g;     # removing "." in filename: "a-b-c.dll" to "a-b-cdll" in name column
        $binaryfilename =~ s/\-/\_/g;   # converting "-" to "_" in filename: "a-b-cdll" to "a_b_cdll" in name column

        my %binary = ();

        $binary{'Name'} = $binaryfilename;
        $binary{'Data'} = $binaryfiledata;

        my $oneline = $binary{'Name'} . "\t" . $binary{'Data'} . "\n";

        push(@{$binaryidttable}, $oneline);
    }

    # Saving the file

    installer::files::save_file($binaryidttablename ,$binaryidttable);
    my $infoline = "Updated idt file: $binaryidttablename\n";
    push(@installer::globals::logfileinfo, $infoline);
}

1;
