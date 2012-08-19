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

package installer::windows::component;

use installer::exiter;
use installer::files;
use installer::globals;
use installer::windows::idtglobal;
use installer::windows::language;

##############################################################
# Returning a globally unique ID (GUID) for a component
# If the component is new, a unique guid has to be created.
# If the component already exists, the guid has to be
# taken from a list component <-> guid
# Sample for a guid: {B68FD953-3CEF-4489-8269-8726848056E8}
##############################################################

sub get_component_guid
{
    my ( $componentname, $componentidhashref ) = @_;

    # At this time only a template
    my $returnvalue = "\{COMPONENTGUID\}";

    if (( $installer::globals::updatedatabase ) && ( exists($componentidhashref->{$componentname}) ))
    {
        $returnvalue = $componentidhashref->{$componentname};
    }

    # Returning a ComponentID, that is assigned in scp project
    if ( exists($installer::globals::componentid{$componentname}) )
    {
        $returnvalue = "\{" . $installer::globals::componentid{$componentname} . "\}";
    }

    return $returnvalue;
}

##############################################################
# Returning the directory for a file component.
##############################################################

sub get_file_component_directory
{
    my ($componentname, $filesref, $dirref) = @_;

    my ($onefile, $component, $onedir, $hostname, $uniquedir);
    my $found = 0;

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        $onefile =  ${$filesref}[$i];
        $component = $onefile->{'componentname'};

        if ( $component eq $componentname )
        {
            $found = 1;
            last;
        }
    }

    if (!($found))
    {
        # This component can be ignored, if it exists in a version with extension "_pff" (this was renamed in file::get_sequence_for_file() )
        my $ignore_this_component = 0;
        my $origcomponentname = $componentname;
        my $componentname = $componentname . "_pff";

        for ( my $j = 0; $j <= $#{$filesref}; $j++ )
        {
            $onefile =  ${$filesref}[$j];
            $component = $onefile->{'componentname'};

            if ( $component eq $componentname )
            {
                $ignore_this_component = 1;
                last;
            }
        }

        if ( $ignore_this_component ) { return "IGNORE_COMP"; }
        else { installer::exiter::exit_program("ERROR: Did not find component \"$origcomponentname\" in file collection", "get_file_component_directory"); }
    }

    my $localstyles = "";

    if ( $onefile->{'Styles'} ) { $localstyles = $onefile->{'Styles'}; }

    if ( $localstyles =~ /\bFONT\b/ )   # special handling for font files
    {
        return $installer::globals::fontsfolder;
    }

    my $destdir = "";

    if ( $onefile->{'Dir'} ) { $destdir = $onefile->{'Dir'}; }

    if ( $destdir =~ /\bPREDEFINED_OSSHELLNEWDIR\b/ )   # special handling for shellnew files
    {
        return $installer::globals::templatefolder;
    }

    if ( $destdir =~ /\bPREDEFINED_OSWINSHELLNEWDIR\b/ )
    {
        return "WindowsShellNewFolder";
    }

    my $destination = $onefile->{'destination'};

    installer::pathanalyzer::get_path_from_fullqualifiedname(\$destination);

    $destination =~ s/\Q$installer::globals::separator\E\s*$//;

    # This path has to be defined in the directory collection at "HostName"

    if ($destination eq "")     # files in the installation root
    {
        $uniquedir = "INSTALLLOCATION";
    }
    else
    {
        $found = 0;

        for ( my $i = 0; $i <= $#{$dirref}; $i++ )
        {
            $onedir =   ${$dirref}[$i];
            $hostname = $onedir->{'HostName'};

            if ( $hostname eq $destination )
            {
                $found = 1;
                last;
            }
        }

        if (!($found))
        {
            installer::exiter::exit_program("ERROR: Did not find destination $destination in directory collection", "get_file_component_directory");
        }

        $uniquedir = $onedir->{'uniquename'};

        if ( $uniquedir eq $installer::globals::officeinstalldirectory )
        {
            $uniquedir = "INSTALLLOCATION";
        }
    }

    $onefile->{'uniquedirname'} = $uniquedir;       # saving it in the file collection

    return $uniquedir
}

##############################################################
# Returning the directory for a registry component.
# This cannot be a useful value
##############################################################

sub get_registry_component_directory
{
    my $componentdir = "INSTALLLOCATION";

    return $componentdir;
}

##############################################################
# Returning the attributes for a file component.
##############################################################

sub get_file_component_attributes
{
    my ($componentname, $filesref, $allvariables) = @_;

    my $attributes;

    $attributes = 2;

    # special handling for font files

    my $onefile;
    my $found = 0;

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        $onefile =  ${$filesref}[$i];
        my $component = $onefile->{'componentname'};

        if ( $component eq $componentname )
        {
            $found = 1;
            last;
        }
    }

    if (!($found))
    {
        installer::exiter::exit_program("ERROR: Did not find component in file collection", "get_file_component_attributes");
    }

    my $localstyles = "";

    if ( $onefile->{'Styles'} ) { $localstyles = $onefile->{'Styles'}; }

    if ( $localstyles =~ /\bFONT\b/ )
    {
        $attributes = 16;   # font files will not be deinstalled
    }

    if ( $localstyles =~ /\bASSEMBLY\b/ )
    {
        $attributes = 0;    # Assembly files cannot run from source
    }

    if (( $onefile->{'Dir'} =~ /\bPREDEFINED_OSSHELLNEWDIR\b/ ) || ( $onefile->{'needs_user_registry_key'} ))
    {
        $attributes = 4;    # Files in shellnew dir and in non advertised startmenu entries must have user registry key as KeyPath
    }

    # Setting msidbComponentAttributes64bit, if this is a 64 bit installation set.
    if (( $allvariables->{'64BITPRODUCT'} ) && ( $allvariables->{'64BITPRODUCT'} == 1 )) { $attributes |= 256; }

    return $attributes;
}

##############################################################
# Returning the attributes for a registry component.
# Always 4, indicating, the keypath is a defined in
# table registry
##############################################################

sub get_registry_component_attributes
{
    my ($componentname, $allvariables) = @_;

    my $attributes;

    $attributes = 4;

    # Setting msidbComponentAttributes64bit, if this is a 64 bit installation set.
    if (( $allvariables->{'64BITPRODUCT'} ) && ( $allvariables->{'64BITPRODUCT'} == 1 )) { $attributes |= 256; }

    # Setting msidbComponentAttributes64bit for 64 bit shell extension in 32 bit installer, too
    if ( $componentname =~ m/winexplorerext_x64/ ) { $attributes |= 256; }

    # Setting msidbComponentAttributesPermanent
    if ( exists($installer::globals::dontdeletecomponents{$componentname}) ) { $attributes |= 16; }

    return $attributes;
}

##############################################################
# Returning the conditions for a component.
# This is important for language dependent components
# in multilingual installation sets.
##############################################################

sub get_file_component_condition
{
    my ($componentname, $filesref) = @_;

    my $condition = "";

    if (exists($installer::globals::componentcondition{$componentname}))
    {
        $condition = $installer::globals::componentcondition{$componentname};
    }

    # there can be also tree conditions for multilayer products
    if (exists($installer::globals::treeconditions{$componentname}))
    {
        if ( $condition eq "" )
        {
            $condition = $installer::globals::treeconditions{$componentname};
        }
        else
        {
            $condition = "($condition) And ($installer::globals::treeconditions{$componentname})";
        }
    }

    return $condition
}

##############################################################
# Returning the conditions for a registry component.
##############################################################

sub get_component_condition
{
    my ($componentname) = @_;

    my $condition;

    $condition = "";    # Always ?

    if (exists($installer::globals::componentcondition{$componentname}))
    {
        $condition = $installer::globals::componentcondition{$componentname};
    }

    return $condition
}

####################################################################
# Returning the keypath for a component.
# This will be the name of the first file/registry, found in the
# collection $itemsref
# Attention: This has to be the unique (file)name, not the
# real filename!
####################################################################

sub get_component_keypath
{
    my ($componentname, $itemsref, $componentidkeypathhashref) = @_;

    my $oneitem;
    my $found = 0;
    my $infoline = "";

    for ( my $i = 0; $i <= $#{$itemsref}; $i++ )
    {
        $oneitem =  ${$itemsref}[$i];
        my $component = $oneitem->{'componentname'};

        if ( $component eq $componentname )
        {
            $found = 1;
            last;
        }
    }

    if (!($found))
    {
        installer::exiter::exit_program("ERROR: Did not find component in file/registry collection, function get_component_keypath", "get_component_keypath");
    }

    my $keypath = $oneitem->{'uniquename'}; # "uniquename", not "Name"

    # Special handling for updates from existing databases, because KeyPath must not change
    if (( $installer::globals::updatedatabase ) && ( exists($componentidkeypathhashref->{$componentname}) ))
    {
        $keypath = $componentidkeypathhashref->{$componentname};
        # -> check, if this is a valid key path?!
        if ( $keypath ne $oneitem->{'uniquename'} )
        {
            # Warning: This keypath was changed because of info from old database
            $infoline = "WARNING: The KeyPath for component \"$componentname\" was changed from \"$oneitem->{'uniquename'}\" to \"$keypath\" because of information from update database";
            push(@installer::globals::logfileinfo, $infoline);
        }
    }

    # Special handling for components in PREDEFINED_OSSHELLNEWDIR. These components
    # need as KeyPath a RegistryItem in HKCU
    if ( $oneitem->{'userregkeypath'} ) { $keypath = $oneitem->{'userregkeypath'}; }

    # saving it in the file and registry collection
    $oneitem->{'keypath'} = $keypath;

    return $keypath
}

###################################################################
# Creating the file Componen.idt dynamically
# Content:
# Component ComponentId Directory_ Attributes Condition KeyPath
###################################################################

sub create_component_table
{
    my ($filesref, $registryref, $dirref, $allfilecomponentsref, $allregistrycomponents, $basedir, $componentidhashref, $componentidkeypathhashref, $allvariables) = @_;

    my @componenttable = ();

    my ($oneline, $infoline);

    installer::windows::idtglobal::write_idt_header(\@componenttable, "component");

    # File components

    for ( my $i = 0; $i <= $#{$allfilecomponentsref}; $i++ )
    {
        my %onecomponent = ();

        $onecomponent{'name'} = ${$allfilecomponentsref}[$i];
        $onecomponent{'guid'} = get_component_guid($onecomponent{'name'}, $componentidhashref);
        $onecomponent{'directory'} = get_file_component_directory($onecomponent{'name'}, $filesref, $dirref);
        if ( $onecomponent{'directory'} eq "IGNORE_COMP" ) { next; }
        $onecomponent{'attributes'} = get_file_component_attributes($onecomponent{'name'}, $filesref, $allvariables);
        $onecomponent{'condition'} = get_file_component_condition($onecomponent{'name'}, $filesref);
        $onecomponent{'keypath'} = get_component_keypath($onecomponent{'name'}, $filesref, $componentidkeypathhashref);

        $oneline = $onecomponent{'name'} . "\t" . $onecomponent{'guid'} . "\t" . $onecomponent{'directory'} . "\t"
                . $onecomponent{'attributes'} . "\t" . $onecomponent{'condition'} . "\t" . $onecomponent{'keypath'} . "\n";

        push(@componenttable, $oneline);
    }

    # Registry components

    for ( my $i = 0; $i <= $#{$allregistrycomponents}; $i++ )
    {
        my %onecomponent = ();

        $onecomponent{'name'} = ${$allregistrycomponents}[$i];
        $onecomponent{'guid'} = get_component_guid($onecomponent{'name'}, $componentidhashref);
        $onecomponent{'directory'} = get_registry_component_directory();
        $onecomponent{'attributes'} = get_registry_component_attributes($onecomponent{'name'}, $allvariables);
        $onecomponent{'condition'} = get_component_condition($onecomponent{'name'});
        $onecomponent{'keypath'} = get_component_keypath($onecomponent{'name'}, $registryref, $componentidkeypathhashref);

        $oneline = $onecomponent{'name'} . "\t" . $onecomponent{'guid'} . "\t" . $onecomponent{'directory'} . "\t"
                . $onecomponent{'attributes'} . "\t" . $onecomponent{'condition'} . "\t" . $onecomponent{'keypath'} . "\n";

        push(@componenttable, $oneline);
    }

    # Saving the file

    my $componenttablename = $basedir . $installer::globals::separator . "Componen.idt";
    installer::files::save_file($componenttablename ,\@componenttable);
    $infoline = "Created idt file: $componenttablename\n";
    push(@installer::globals::logfileinfo, $infoline);
}

####################################################################################
# Returning a component for a scp module gid.
# Pairs are saved in the files collector.
####################################################################################

sub get_component_name_from_modulegid
{
    my ($modulegid, $filesref) = @_;

    my $componentname = "";

    for my $file ( @{$filesref} )
    {
        next if ( ! $file->{'modules'} );

        my @filemodules = split /,\s*/, $file->{'modules'};

        if (grep {$_ eq $modulegid} @filemodules)
        {
            $componentname = $file->{'componentname'};
            last;
        }
    }

    return $componentname;
}

####################################################################################
# Updating the file Environm.idt dynamically
# Content:
# Environment Name Value Component_
####################################################################################

sub set_component_in_environment_table
{
    my ($basedir, $filesref) = @_;

    my $infoline = "";

    my $environmentfilename = $basedir . $installer::globals::separator . "Environm.idt";

    if ( -f $environmentfilename )  # only do something, if file exists
    {
        my $environmentfile = installer::files::read_file($environmentfilename);

        for ( my $i = 3; $i <= $#{$environmentfile}; $i++ ) # starting in line 4 of Environm.idt
        {
            if ( ${$environmentfile}[$i] =~ /^\s*(.*?)\t(.*?)\t(.*?)\t(.*?)\s*$/ )
            {
                my $modulegid = $4; # in Environment table a scp module gid can be used as component replacement

                my $componentname = get_component_name_from_modulegid($modulegid, $filesref);

                if ( $componentname )   # only do something if a component could be found
                {
                    $infoline = "Updated Environment table:\n";
                    push(@installer::globals::logfileinfo, $infoline);
                    $infoline = "Old line: ${$environmentfile}[$i]\n";
                    push(@installer::globals::logfileinfo, $infoline);

                    ${$environmentfile}[$i] =~ s/$modulegid/$componentname/;

                    $infoline = "New line: ${$environmentfile}[$i]\n";
                    push(@installer::globals::logfileinfo, $infoline);

                }
            }
        }

        # Saving the file

        installer::files::save_file($environmentfilename ,$environmentfile);
        $infoline = "Updated idt file: $environmentfilename\n";
        push(@installer::globals::logfileinfo, $infoline);

    }
}

1;
