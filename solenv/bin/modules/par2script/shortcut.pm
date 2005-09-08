#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: shortcut.pm,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 09:27:47 $
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
