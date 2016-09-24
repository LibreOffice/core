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
