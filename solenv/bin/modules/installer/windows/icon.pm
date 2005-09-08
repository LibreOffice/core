#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: icon.pm,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 09:17:06 $
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
