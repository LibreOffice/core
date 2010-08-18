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

package par2script::undefine;

use par2script::globals;

##########################################################
# Removing in the script all the gids, that are listed
# in undefine scp files
##########################################################

sub undefine_gids
{
    my ($parfilecontent) = @_;

    my $item;
    foreach $item ( @par2script::globals::allitems )
    {
        my $unitem = "Un$item";

        for ( my $i = 0; $i <= $#{$parfilecontent}; $i++ )
        {
            if ( ${$parfilecontent}[$i] =~ /^\s*$unitem\s*(\w+?)\s*$/ )
            {
                my $gid = $1;
                delete($par2script::globals::definitions{$item}->{$gid});
            }
        }
    }
}

##########################################################
# Collecting all subdirectories of a specified directory
##########################################################

sub collect_children_dirs
{
    my ($gid, $collector) = @_;

    my $diritem = "Directory";
    my $parentkey = "ParentID";

    if ( exists($par2script::globals::definitions{$diritem}) )
    {
        my $onedefinition;

        foreach $onedefinition (keys %{$par2script::globals::definitions{$diritem}})
        {
            if ( $par2script::globals::definitions{$diritem}->{$onedefinition}->{$parentkey} eq $gid )
            {
                push(@{$collector}, $onedefinition);
                collect_children_dirs($onedefinition, $collector);
            }
        }
    }
}

##########################################################
# Removing in the script complete profiles.
# This includes the Profile and its ProfileItems.
##########################################################

sub remove_complete_item
{
    my ($item, $parfilecontent) = @_;

    my $removeitem = "Remove$item";
    my $dependentkey = "";
    my $collect_children = 0;
    my @gidcollector = ();
    my @dependentitems = ();

    if ( $item eq "Profile" )
    {
        @dependentitems = ("ProfileItem");
        $dependentkey = "ProfileID";
    }
    elsif ( $item eq "Directory" )
    {
        @dependentitems = ("File", "Shortcut", "Unixlink");
        $dependentkey = "Dir";
        $collect_children = 1;
    }

    for ( my $i = 0; $i <= $#{$parfilecontent}; $i++ )
    {
        if ( ${$parfilecontent}[$i] =~ /^\s*$removeitem\s*(\w+?)\s*$/ )
        {
            my $onegid = $1;
            push(@gidcollector, $onegid);
            if ( $collect_children ) { collect_children_dirs($onegid, \@gidcollector); }

            my $gid;
            foreach $gid (@gidcollector)
            {
                delete($par2script::globals::definitions{$item}->{$gid});

                # also deleting all dependent items, for example "ProfileItems" whose "ProfileID" is this "Profile"
                my $depitem;
                foreach $depitem ( @dependentitems )
                {
                    if ( exists($par2script::globals::definitions{$depitem}) )
                    {
                        my $onedefinition;
                        foreach $onedefinition (keys %{$par2script::globals::definitions{$depitem}})
                        {
                            if ( $par2script::globals::definitions{$depitem}->{$onedefinition}->{$dependentkey} eq $gid )
                            {
                                delete($par2script::globals::definitions{$depitem}->{$onedefinition});
                            }
                        }
                    }
                }
            }
        }
    }
}

1;
