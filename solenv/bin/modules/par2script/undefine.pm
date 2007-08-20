#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: undefine.pm,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: ihi $ $Date: 2007-08-20 15:29:32 $
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
