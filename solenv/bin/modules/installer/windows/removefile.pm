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

package installer::windows::removefile;

use installer::files;
use installer::globals;
use installer::windows::idtglobal;

########################################################################
# Returning the FileKey for a folderitem for removefile table.
########################################################################

sub get_removefile_filekey
{
    my ($folderitem) = @_;

    # returning the unique identifier

    my $identifier = "remove_" . $folderitem->{'directory'};

    $identifier = lc($identifier);

    return $identifier;
}

########################################################################
# Returning the Component for a folderitem for removefile table.
########################################################################

sub get_removefile_component
{
    my ($folderitem) = @_;

    return $folderitem->{'component'};
}

########################################################################
# Returning the FileName for a folderitem for removefile table.
########################################################################

sub get_removefile_filename
{
    my ($folderitem) = @_;

     # return nothing: The assigned directory will be removed

    return "";
}

########################################################################
# Returning the DirProperty for a folderitem for removefile table.
########################################################################

sub get_removefile_dirproperty
{
    my ($folderitem) = @_;

    return $folderitem->{'directory'};
}

########################################################################
# Returning the InstallMode for a folderitem for removefile table.
########################################################################

sub get_removefile_installmode
{
    my ($folderitem) = @_;

    # always returning "2": The file is only removed, if the assigned
    # component is removed. Name: msidbRemoveFileInstallModeOnRemove

    return 2;
}

###########################################################################################################
# Creating the file RemoveFi.idt dynamically
# Content:
# FileKey Component_ FileName DirProperty InstallMode
###########################################################################################################

sub create_removefile_table
{
    my ($folderitemsref, $basedir) = @_;

    # Only the directories created for the FolderItems have to be deleted
    # with the information in the table RemoveFile

    my @directorycollector = ();

    for ( my $i = 0; $i <= $#{$folderitemsref}; $i++ )
    {
        my $onelink = ${$folderitemsref}[$i];

        if ( $onelink->{'used'} == 0 ) { next; }

        next if grep {$_ eq $onelink->{'directory'}} @directorycollector;

        push(@directorycollector, $onelink->{'directory'});

        my %removefile = ();

        $removefile{'FileKey'} = get_removefile_filekey($onelink);
        $removefile{'Component_'} = get_removefile_component($onelink);
        $removefile{'FileName'} = get_removefile_filename($onelink);
        $removefile{'DirProperty'} = get_removefile_dirproperty($onelink);
        # fdo#44565 do not remove empty Desktop folder
        if ( $removefile{'DirProperty'} eq $installer::globals::desktopfolder ) { next; }
        $removefile{'InstallMode'} = get_removefile_installmode($onelink);

        my $oneline = $removefile{'FileKey'} . "\t" . $removefile{'Component_'} . "\t" . $removefile{'FileName'} . "\t"
                    . $removefile{'DirProperty'} . "\t" . $removefile{'InstallMode'} . "\n";

        push(@installer::globals::removefiletable, $oneline);
    }

    # Saving the file

    my $removefiletablename = $basedir . $installer::globals::separator . "RemoveFi.idt";
    installer::files::save_file($removefiletablename ,\@installer::globals::removefiletable);
    my $infoline = "Created idt file: $removefiletablename\n";
    push(@installer::globals::logfileinfo, $infoline);

}

1;
