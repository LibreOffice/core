#*************************************************************************
#
#   $RCSfile: shortcut.pm,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: kz $ $Date: 2004-01-29 11:45:49 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc..
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
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
