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
