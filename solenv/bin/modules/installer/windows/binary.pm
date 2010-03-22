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

package installer::windows::binary;

use installer::existence;
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

        $binaryfilename =~ s/\.//g;  # removing "." in filename: "abc.dll" to "abcdll" in name column

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
