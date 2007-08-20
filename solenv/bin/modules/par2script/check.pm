#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: check.pm,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: ihi $ $Date: 2007-08-20 15:28:06 $
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
# If the StarRegistry is not defined in the script,
# it has to be removed from the file definition.
########################################################

sub check_registry_at_files
{
    my %starregistrygid = ();

    my $item;
    foreach $item ( keys %{$par2script::globals::definitions{'File'}} )
    {
        if (( exists($par2script::globals::definitions{'File'}->{$item}->{'Styles'}) ) &&
            ( $par2script::globals::definitions{'File'}->{$item}->{'Styles'} =~ /\bSTARREGISTRY\b/ ))
        {
            $starregistrygid{$item} = 1;
        }
    }

    foreach $item ( keys %{$par2script::globals::definitions{'File'}} )
    {
        if ( exists($par2script::globals::definitions{'File'}->{$item}->{'RegistryID'}) )
        {
            my $registryid = $par2script::globals::definitions{'File'}->{$item}->{'RegistryID'};
            if ( ! exists($starregistrygid{$registryid}) )
            {
                die "\nERROR: No definition found for $registryid at file $item\n\n";
            }

            # if ( ! ( $par2script::globals::definitions{'File'}->{$item}->{'Styles'} =~ /\bUNO_COMPONENT\b/ ))
            # {
            #   die "\nERROR: Flag UNO_COMPONENT required for file $item\n\n";
            # }
            # -> also possible, that Regmergefile is defined (does not require flag UNO_COMPONENT)
        }

        # and also vice versa

        if (( exists($par2script::globals::definitions{'File'}->{$item}->{'Styles'}) ) &&
            ( $par2script::globals::definitions{'File'}->{$item}->{'Styles'} =~ /\bUNO_COMPONENT\b/ ))
        {
            if ( ! exists($par2script::globals::definitions{'File'}->{$item}->{'RegistryID'}) )
            {
                die "\nERROR: Flag UNO_COMPONENT defined, but no file as \"RegistryID\" at file $item !\n\n";
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
