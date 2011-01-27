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

package par2script::check;

use par2script::globals;

################################
# Checks of the setup script
################################

########################################################
# Checking if all defined directories are needed
########################################################

sub check_needed_directories
{
    my $allfiles = $par2script::globals::definitions{'File'};
    my $alldirs = $par2script::globals::definitions{'Directory'};

    # checking if all defined directories are needed

    my $dir;
    foreach $dir ( keys %{$alldirs} )
    {
        # I. directory has create flag
        if (( exists($alldirs->{$dir}->{'Styles'}) ) && ( $alldirs->{$dir}->{'Styles'} =~ /\bCREATE\b/ )) { next; }

        # II. there is at least one file in the directory
        my $fileinside = 0;
        my $file;
        foreach $file ( keys %{$allfiles} )
        {
            if (( $allfiles->{$file}->{'Dir'} eq $dir ) || ( $allfiles->{$file}->{'NetDir'} eq $dir ))
            {
                $fileinside = 1;
                last;
            }
        }
        if ( $fileinside ) { next; }

        # III. the directory is parent for another directory
        my $isparent = 0;
        my $onedir;
        foreach $onedir ( keys %{$alldirs} )
        {
            if ( $alldirs->{$onedir}->{'ParentID'} eq $dir )
            {
                $isparent = 1;
                last;
            }
        }
        if ( $isparent ) { next; }

        # no condition is true -> directory definition is superfluous
        my $infoline = "\tINFO: Directory definition $dir is superfluous\n";
        # print $infoline;
        push(@par2script::globals::logfileinfo, $infoline);
    }
}

##################################################
# Checking if the directories in the item
# definitions are defined.
##################################################

sub check_directories_in_item_definitions
{
    my $item;
    foreach $item ( @par2script::globals::items_with_directories )
    {
        my $allitems = $par2script::globals::definitions{$item};

        my $onegid;
        foreach $onegid ( keys %{$allitems} )
        {
            if ( ! exists($allitems->{$onegid}->{'Dir'}) ) { die "\nERROR: No directory defined for item: $onegid!\n\n"; }
            my $dir = $allitems->{$onegid}->{'Dir'};
            if (( $dir eq "PD_PROGDIR" ) || ( $dir =~ /PREDEFINED_/ )) { next; }

            # checking if this directoryid is defined
            if ( ! exists($par2script::globals::definitions{'Directory'}->{$dir}) )
            {
                die "\nERROR: Directory $dir in item $onegid not defined!\n\n";
            }
        }
    }
}

########################################################
# Checking for all Items, that know their modules,
# whether these modules exist.
########################################################

sub check_module_existence
{
    my $item;
    foreach $item ( @par2script::globals::items_with_moduleid )
    {
        my $allitems = $par2script::globals::definitions{$item};

        my $onegid;
        foreach $onegid ( keys %{$allitems} )
        {
            if ( ! exists($allitems->{$onegid}->{'ModuleID'}) ) { die "\nERROR: No ModuleID defined for item: $onegid!\n\n"; }
            my $moduleid = $allitems->{$onegid}->{'ModuleID'};

            # checking if this directoryid is defined
            if ( ! exists($par2script::globals::definitions{'Module'}->{$moduleid}) )
            {
                die "\nERROR: ModuleID $moduleid in item $onegid not defined!\n\n";
            }
        }
    }
}

########################################################
# Every script has to contain exactly one root module.
# This module has no ParentID or an empty ParentID.
########################################################

sub check_rootmodule
{
    my $rootgid = "";
    my $foundroot = 0;

    my $allmodules = $par2script::globals::definitions{'Module'};

    my $modulegid = "";
    foreach $modulegid (keys %{$allmodules} )
    {
        if (( ! exists($allmodules->{$modulegid}->{'ParentID'}) ) || ( $allmodules->{$modulegid}->{'ParentID'} eq "" ))
        {
            if ( $foundroot )
            {
                die "\nERROR: More than one Root module. Only one module without ParentID or with empty ParentID allowed ($rootgid and $modulegid).\n";
            }
            $rootgid = $modulegid;
            $foundroot = 1;
        }
    }

    if ( ! $foundroot )
    {
        die "\nERROR: Could not find Root module. Did not find module without ParentID or with empty ParentID.\n";
    }

    print " $rootgid\n";

}

########################################################
# File, Shortcut, Directory, Unixlink must not
# contain a ModuleID
########################################################

sub check_moduleid_at_items
{
    my $item;
    foreach $item ( @par2script::globals::items_without_moduleid )
    {
        my $allitems = $par2script::globals::definitions{$item};

        my $onegid;
        foreach $onegid ( keys %{$allitems} )
        {
            if ( exists($allitems->{$onegid}->{'ModuleID'}) )
            {
                die "\nERROR: ModuleID assigned to $onegid! No module assignment to $item!\n\n";
            }
        }
    }
}

########################################################
# Controlling existence of multi assignments
########################################################

sub check_multiple_assignments
{
    my @multiassignments = ();
    my $error;

    my $topitem;
    foreach $topitem ( keys %par2script::globals::assignedgids )
    {
        my $item;
        foreach $item ( keys %{$par2script::globals::assignedgids{$topitem}} )
        {
            if ( $par2script::globals::assignedgids{$topitem}->{$item} > 1 )
            {
                $error = 1;
                my $string = "\tGID: $item Assignments: $par2script::globals::assignedgids{$topitem}->{$item}";
                push(@multiassignments, $string);
            }
        }
    }

    if ( $error ) { par2script::exiter::multiassignmenterror(\@multiassignments); }
}

########################################################
# Check, if a defined directory has a flag CREATE
########################################################

sub contains_create_flag
{
    my ($gid) = @_;

    my $createflag = 0;

    if (( exists($par2script::globals::definitions{'Directory'}->{$gid}->{'Styles'}) ) &&
        ( $par2script::globals::definitions{'Directory'}->{$gid}->{'Styles'} =~ /\bCREATE\b/ ))
    {
        $createflag = 1;
    }

    return $createflag;
}

########################################################
# Controlling existence of definitions without
# any assignment
########################################################

sub check_missing_assignments
{
    # If defined gids for "File", "Directory" or "Unixlink" are not assigned,
    # this causes an error.
    # Directories only have to be assigned, if they have the flag "CREATE".

    my @missingassignments = ();
    $error = 0;

    my $item;
    foreach $item ( @par2script::globals::items_assigned_at_modules )
    {
        my $assignedgids = $par2script::globals::assignedgids{$item};
        my $definedgids = $par2script::globals::definitions{$item};

        my $gid;
        foreach $gid ( keys %{$definedgids} )
        {
            if ( $item eq "Directory" ) { if ( ! contains_create_flag($gid) ) { next; } }

            if ( ! exists( $assignedgids->{$gid} ))
            {
                $error = 1;
                push(@missingassignments, $gid);
            }
        }
    }

    if ( $error ) { par2script::exiter::missingassignmenterror(\@missingassignments); }
}

#############################################################
# Controlling if for all shortcuts with file assignment
# the file is defined. And for all shortcuts with
# shortcut assignment the shortcut has to be defined.
#############################################################

sub check_shortcut_assignments
{
    my $allshortcuts = $par2script::globals::definitions{'Shortcut'};
    my $allfiles = $par2script::globals::definitions{'File'};

    my $shortcut;
    foreach $shortcut ( keys %{$allshortcuts} )
    {
        if (( exists($allshortcuts->{$shortcut}->{'FileID'}) ) &&
            ( ! exists($allfiles->{$allshortcuts->{$shortcut}->{'FileID'}}) ))
        {
            # die "\nERROR: FileID $allshortcuts->{$shortcut}->{'FileID'} has no definition at shortcut $shortcut !\n";
            print "\n\tWARNING: FileID $allshortcuts->{$shortcut}->{'FileID'} has no definition at shortcut $shortcut !\n";
        }

        if (( exists($allshortcuts->{$shortcut}->{'ShortcutID'}) ) &&
            ( ! exists($allshortcuts->{$allshortcuts->{$shortcut}->{'ShortcutID'}}) ))
        {
            die "\nERROR: ShortcutID $allshortcuts->{$shortcut}->{'ShortcutID'} has no definition at shortcut $shortcut !\n";
        }

        if (( ! exists($allshortcuts->{$shortcut}->{'ShortcutID'}) ) &&
            ( ! exists($allshortcuts->{$shortcut}->{'FileID'}) ))
        {
            die "\nERROR: Shortcut requires assignment to \"ShortcutID\" or \"FileID\". Missing at shortcut $shortcut !\n";
        }
    }
}

#############################################################
# Controlling if for Modules and Directories, the parents
# are defined. If not, this can lead to a problem during
# script creation, because only recursively added
# Modules or Directories are added to the script.
#############################################################

sub check_missing_parents
{
    my @parentitems = ("Module", "Directory");
    my %rootparents = ("PREDEFINED_PROGDIR" => "1");

    my $oneitem;
    foreach $oneitem ( @parentitems )
    {
        my $alldefinitions = $par2script::globals::definitions{$oneitem};

        my $onegid;
        foreach $onegid ( keys %{$alldefinitions} )
        {
            # If there is a ParentID used, it must be defined
            if (( exists($alldefinitions->{$onegid}->{'ParentID'}) ) &&
                ( ! exists($alldefinitions->{$alldefinitions->{$onegid}->{'ParentID'}}) ) &&
                ( ! exists($rootparents{$alldefinitions->{$onegid}->{'ParentID'}}) ))
            {
                die "\nERROR: Parent \"$alldefinitions->{$onegid}->{'ParentID'}\" at $oneitem \"$onegid\" is not defined!\n";
            }
        }
    }
}

1;
