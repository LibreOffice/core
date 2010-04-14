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

package installer::windows::icon;

use installer::files;
use installer::globals;
use installer::pathanalyzer;
use installer::windows::idtglobal;

###########################################################################################################
# Creating the file Icon.idt dynamically
# Content:
# Name Data
###########################################################################################################

sub create_icon_table
{
    my ($iconfilecollector, $basedir) = @_;

    my @icontable = ();

    installer::windows::idtglobal::write_idt_header(\@icontable, "icon");

    # Only the iconfiles, that are used in the shortcut table for the
    # FolderItems (entries in Windows startmenu) are added into the icon table.

    for ( my $i = 0; $i <= $#{$iconfilecollector}; $i++ )
    {
        my $iconfile = ${$iconfilecollector}[$i];

        installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$iconfile);

        my %icon = ();

        $icon{'Name'} = $iconfile;  # simply soffice.exe
        $icon{'Data'} = $iconfile;  # simply soffice.exe

        my $oneline = $icon{'Name'} . "\t" . $icon{'Data'} . "\n";

        push(@icontable, $oneline);
    }

    # Saving the file

    my $icontablename = $basedir . $installer::globals::separator . "Icon.idt";
    installer::files::save_file($icontablename ,\@icontable);
    my $infoline = "Created idt file: $icontablename\n";
    push(@installer::globals::logfileinfo, $infoline);

}

1;
