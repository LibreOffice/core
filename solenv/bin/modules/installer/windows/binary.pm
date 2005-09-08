#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: binary.pm,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 09:14:18 $
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
