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
