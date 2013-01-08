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

package installer::windows::inifile;

use installer::exiter;
use installer::files;
use installer::globals;
use installer::windows::idtglobal;

####################################################
# Setting the profile for a special profileitem
####################################################

sub get_profile_for_profileitem
{
    my ($profileid, $filesref) = @_;

    my ($profile) = grep {$_->{gid} eq $profileid} @{$filesref};
    if (! defined $profile) {
        installer::exiter::exit_program("ERROR: Could not find file $profileid in list of files!", "get_profile_for_profileitem");
    }

    return $profile;
}

####################################################
# Checking whether profile is part of product
####################################################

sub file_is_part_of_product
{
    my ($profilegid, $filesref) = @_;

    my $part_of_product = 0;

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        $onefile = ${$filesref}[$i];
        my $filegid = $onefile->{'gid'};

        if ( $filegid eq $profilegid )
        {
            $part_of_product = 1;
            last;
        }
    }

    return $part_of_product;
}

###########################################################################################################
# Creating the file IniFile.idt dynamically
# Content:
# IniFile\tFileName\tDirProperty\tSection\tKey\tValue\tAction\tComponent_
###########################################################################################################

sub create_inifile_table
{
    my ($inifiletableentries, $filesref, $basedir) = @_;

    my @inifiletable = ();

    installer::windows::idtglobal::write_idt_header(\@inifiletable, "inifile");

    for ( my $i = 0; $i <= $#{$inifiletableentries}; $i++ )
    {
        my $profileitem = ${$inifiletableentries}[$i];

        my $profileid = $profileitem->{'ProfileID'};

        # Is this profile part of the product? This is not sure, for example in patch process.
        # If the profile is not part of the product, this ProfileItem must be ignored.

        if ( ! file_is_part_of_product($profileid, $filesref) ) { next; }

        my $profile = get_profile_for_profileitem($profileid, $filesref);

        my %inifile = ();

        $inifile{'IniFile'} = $profileitem->{'Inifiletablekey'};
        $inifile{'FileName'} = $profile->{'Name'};
        $inifile{'DirProperty'} = $profile->{'uniquedirname'};
        $inifile{'Section'} = $profileitem->{'Section'};
        $inifile{'Key'} = $profileitem->{'Key'};
        $inifile{'Value'} = $profileitem->{'Value'};
        $inifile{'Action'} = $profileitem->{'Inifiletableaction'};
        $inifile{'Component_'} = $profile->{'componentname'};

        my $oneline = $inifile{'IniFile'} . "\t" . $inifile{'FileName'} . "\t" . $inifile{'DirProperty'} . "\t"
                 . $inifile{'Section'} . "\t" . $inifile{'Key'} . "\t" . $inifile{'Value'} . "\t"
                 . $inifile{'Action'} . "\t" . $inifile{'Component_'} . "\n";

        push(@inifiletable, $oneline);
    }

    # Saving the file

    my $inifiletablename = $basedir . $installer::globals::separator . "IniFile.idt";
    installer::files::save_file($inifiletablename ,\@inifiletable);
    my $infoline = "Created idt file: $inifiletablename\n";
    push(@installer::globals::logfileinfo, $infoline);

}

1;
