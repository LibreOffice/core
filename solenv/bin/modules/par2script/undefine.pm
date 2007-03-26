#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: undefine.pm,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: ihi $ $Date: 2007-03-26 12:46:08 $
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

package par2script::undefine;

use par2script::existence;
use par2script::globals;
use par2script::remover;
use par2script::work;

##########################################################
# Collecting all undefines, that are listed
# in one par file
##########################################################

sub get_list_of_undefines
{
    my ($item, $parfile) = @_;

    my @collector  =();

    for ( my $i = 0; $i <= $#{$parfile}; $i++ )
    {
        if ( ${$parfile}[$i] =~ /^\s*$item\s*(\w+)\s*$/ )
        {
            my $gid = $1;

            my $oneitem = $item;
            $oneitem =~ s/^\s*Un//; # removing the "Un"

            my %removeitem = ();
            $removeitem{'gid'} = $gid;
            $removeitem{'item'} = $oneitem;

            push(@collector, \%removeitem);
        }
    }

    return \@collector;
}

##########################################################
# Removing in the script all the gids, that are listed
# in undefine scp files
##########################################################

sub undefine_gids
{
    my ($script, $parfile) = @_;

    my @allundefines = ();

    for ( my $i = 0; $i <= $#par2script::globals::allitems; $i++ )
    {
        my $oneitem = "Un" . $par2script::globals::allitems[$i];

        my $collector = get_list_of_undefines($oneitem, $parfile);

        if ($#{$collector} > -1 ) { par2script::work::add_array_into_array(\@allundefines, $collector); }
    }

    if ($#allundefines > -1 )
    {
        for ( my $i = 0; $i <= $#allundefines; $i++ )
        {
            my $gid = $allundefines[$i]->{'gid'};
            par2script::remover::remove_leading_and_ending_whitespaces(\$gid);
            par2script::work::remove_definitionblock_from_script($script, $gid);
        }
    }
}

##########################################################
# Collecting all RemoveDirectories, that are listed
# in the par files
##########################################################

sub get_list_of_removeitems
{
    my ($removeitem, $parfile) = @_;

    my @collector  =();

    for ( my $i = 0; $i <= $#{$parfile}; $i++ )
    {
        if ( ${$parfile}[$i] =~ /^\s*\Q$removeitem\E\s+(\w+)\s*$/ )
        {
            my $gid = $1;
            my $item = $removeitem;
            $item =~ s/Remove//;

            my %removeitem = ();
            $removeitem{'gid'} = $gid;
            $removeitem{'item'} = $oneitem;

            push(@collector, \%removeitem);
        }
    }

    return \@collector;
}

##########################################################
# Removing in the script complete directories.
# This includes subdirectories, files and shortcuts.
##########################################################

sub remove_complete_dirs
{
    my ($script, $parfile) = @_;

    my $removeitem = "RemoveDirectory";
    my @allundefines = ();
    my @alldirs = ();
    my @removeitems = ("File", "Shortcut");

    # Collecting all definitions of "RemoveDirectory"
    my $oneremoveitem = "RemoveDirectory";
    my $directremovedirs = get_list_of_removeitems($oneremoveitem, $parfile);

    if ($#{$directremovedirs} > -1 )
    {
        par2script::work::add_array_into_array(\@allundefines, $directremovedirs);
        par2script::work::add_array_into_array(\@alldirs, $directremovedirs);

        # Collecting all subdirectories
        for ( my $i = 0; $i <= $#{$directremovedirs}; $i++ )
        {
            my @collector = ();
            my $dir = ${$directremovedirs}[$i];
            my $directorygid = $dir->{'gid'};
            par2script::work::collect_subdirectories($parfile, $directorygid, \@collector);
            par2script::work::add_array_into_array(\@allundefines, \@collector);
            par2script::work::add_array_into_array(\@alldirs, \@collector);
        }

        # Collecting Files and ShortCuts, that are installed in the collected directories
        for ( my $i = 0; $i <= $#removeitems; $i++ )
        {
            for ( my $j = 0; $j <= $#alldirs; $j++ )
            {
                my @collector = ();
                my $item = $removeitems[$i];
                my $dir = $alldirs[$j];
                my $directorygid = $dir->{'gid'};
                par2script::work::get_all_items_in_directories($parfile, $directorygid, $item, \@collector);
                par2script::work::add_array_into_array(\@allundefines, \@collector);
            }
        }
    }

    if ($#allundefines > -1 )
    {
        for ( my $i = 0; $i <= $#allundefines; $i++ )
        {
            my $gid = $allundefines[$i]->{'gid'};
            print "Removing gid from script: $gid \n";
            par2script::remover::remove_leading_and_ending_whitespaces(\$gid);
            par2script::work::remove_definitionblock_from_script($script, $gid);
        }
    }
}

##########################################################
# Removing in the script complete profiles.
# This includes the Profile and its ProfileItems.
##########################################################

sub remove_complete_profile
{
    my ($script, $parfile) = @_;

    my $removeitem = "RemoveProfile";
    my @allundefines = ();
    my @alldirs = ();
    my @removeitems = ("ProfileItem");

    # Collecting all definitions of "RemoveProfile"
    my $oneremoveitem = "RemoveProfile";
    my $directremoveprofiles = get_list_of_removeitems($oneremoveitem, $parfile);

    if ($#{$directremoveprofiles} > -1 )
    {
        par2script::work::add_array_into_array(\@allundefines, $directremoveprofiles);
        par2script::work::add_array_into_array(\@allprofiles, $directremoveprofiles);

        # Collecting ProfileItems, that are written in the collected Profiles
        for ( my $i = 0; $i <= $#removeitems; $i++ )
        {
            for ( my $j = 0; $j <= $#allprofiles; $j++ )
            {
                my @collector = ();
                my $item = $removeitems[$i];
                my $profile = $allprofiles[$j];
                my $profilegid = $profile->{'gid'};
                par2script::work::get_all_items_in_profile($parfile, $profilegid, $item, \@collector);
                par2script::work::add_array_into_array(\@allundefines, \@collector);
            }
        }
    }

    if ($#allundefines > -1 )
    {
        for ( my $i = 0; $i <= $#allundefines; $i++ )
        {
            my $gid = $allundefines[$i]->{'gid'};
            print "Removing gid from script: $gid \n";
            par2script::remover::remove_leading_and_ending_whitespaces(\$gid);
            par2script::work::remove_definitionblock_from_script($script, $gid);
        }
    }
}

1;
