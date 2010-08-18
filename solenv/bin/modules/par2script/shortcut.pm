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


package par2script::shortcut;

use par2script::work;

############################################################
# Writing shortcuts to files behind the correct files and
# then shortcuts to shortcuts behind these shortcuts.
############################################################

sub shift_shortcut_positions
{
    my ($script) = @_;

    my $allshortcutgids = par2script::work::get_all_gids_from_script($script, "Shortcut");

    # first all shortcuts that are assigned to files

    for ( my $i = 0; $i <= $#{$allshortcutgids}; $i++ )
    {
        my $codeblock = par2script::work::get_definitionblock_from_script($script, ${$allshortcutgids}[$i]);

        my $filegid = par2script::work::get_value_from_definitionblock($codeblock, "FileID");

        if (!($filegid eq ""))
        {
            par2script::work::remove_definitionblock_from_script($script, ${$allshortcutgids}[$i]);
            par2script::work::add_definitionblock_into_script($script, $codeblock, $filegid);
        }
    }

    # secondly all shortcuts that are assigned to other shortcuts

    for ( my $i = 0; $i <= $#{$allshortcutgids}; $i++ )
    {
        my $codeblock = par2script::work::get_definitionblock_from_script($script, ${$allshortcutgids}[$i]);
        my $shortcutgid = par2script::work::get_value_from_definitionblock($codeblock, "ShortcutID");

        if (!($shortcutgid eq ""))
        {
            par2script::work::remove_definitionblock_from_script($script, ${$allshortcutgids}[$i]);
            par2script::work::add_definitionblock_into_script($script, $codeblock, $shortcutgid);
        }
    }
}


1;
