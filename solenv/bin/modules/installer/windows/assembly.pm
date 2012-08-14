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

package installer::windows::assembly;

use installer::files;
use installer::globals;
use installer::worker;
use installer::windows::idtglobal;

##############################################################
# Returning the first module of a file from the
# comma separated list of modules.
##############################################################

sub get_msiassembly_feature
{
    my ( $onefile ) = @_;

    my $module = "";

    if ( $onefile->{'modules'} ) { $module = $onefile->{'modules'}; }

    # If modules contains a list of modules, only taking the first one.

    if ( $module =~ /^\s*(.*?)\,/ ) { $module = $1; }

    # Attention: Maximum feature length is 38!
    installer::windows::idtglobal::shorten_feature_gid(\$module);

    return $module;
}

##############################################################
# Returning the component of a file.
##############################################################

sub get_msiassembly_component
{
    my ( $onefile ) = @_;

    my $component = "";

    $component = $onefile->{'componentname'};

    return $component;
}

##############################################################
# Returning the file name as manifest file
##############################################################

sub get_msiassembly_filemanifest
{
    my ( $onefile ) = @_;

    my $filemanifest = "";

    $filemanifest = $onefile->{'uniquename'};

    return $filemanifest;
}


##############################################################
# Returning the file application
##############################################################

sub get_msiassembly_fileapplication
{
    my ( $onefile ) = @_;

    my $fileapplication = "";

    return $fileapplication;
}

##############################################################
# Returning the file attributes
##############################################################

sub get_msiassembly_attributes
{
    my ( $onefile ) = @_;

    my $fileattributes = "";

    if ( $onefile->{'Attributes'} ne "" ) { $fileattributes = $onefile->{'Attributes'}; }

    return $fileattributes;
}

##############################################################
# Returning the file object for the msiassembly table.
##############################################################

sub get_msiassembly_file
{
    my ( $filesref, $filename ) = @_;

    my $foundfile = 0;
    my $onefile;

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        $onefile = ${$filesref}[$i];
        my $name = $onefile->{'Name'};

        if ( $name eq $filename )
        {
            $foundfile = 1;
            last;
        }
    }

    # It does not need to exist. For example products that do not contain the libraries.
    if (! $foundfile ) { $onefile  = ""; }

    return $onefile;
}

####################################################################################
# Creating the file MsiAssembly.idt dynamically
# Content:
# Component_    Feature_    File_Manifest   File_Application    Attributes
# s72   s38 S72 S72 I2
# MsiAssembly   Component_
####################################################################################

sub create_msiassembly_table
{
    my ($filesref, $basedir) = @_;

    $installer::globals::msiassemblyfiles = installer::worker::collect_all_items_with_special_flag($filesref, "ASSEMBLY");

    my @msiassemblytable = ();

    installer::windows::idtglobal::write_idt_header(\@msiassemblytable, "msiassembly");

    # Registering all libraries listed in $installer::globals::msiassemblyfiles

    for ( my $i = 0; $i <= $#{$installer::globals::msiassemblyfiles}; $i++ )
    {
        my $onefile = ${$installer::globals::msiassemblyfiles}[$i];

        my %msiassembly = ();

        $msiassembly{'Component_'} = get_msiassembly_component($onefile);
        $msiassembly{'Feature_'} = get_msiassembly_feature($onefile);
        $msiassembly{'File_Manifest'} = get_msiassembly_filemanifest($onefile);
        $msiassembly{'File_Application'} = get_msiassembly_fileapplication($onefile);
        $msiassembly{'Attributes'} = get_msiassembly_attributes($onefile);

        my $oneline = $msiassembly{'Component_'} . "\t" . $msiassembly{'Feature_'} . "\t" .
                        $msiassembly{'File_Manifest'} . "\t" . $msiassembly{'File_Application'} . "\t" .
                        $msiassembly{'Attributes'} . "\n";

        push(@msiassemblytable, $oneline);
    }

    # Saving the file

    my $msiassemblytablename = $basedir . $installer::globals::separator . "MsiAssem.idt";
    installer::files::save_file($msiassemblytablename ,\@msiassemblytable);
    my $infoline = "Created idt file: $msiassemblytablename\n";
    push(@installer::globals::logfileinfo, $infoline);
}

####################################################################################
# Creating the file MsiAssemblyName.idt dynamically
# Content:
# Component_    Name    Value
# s72   s255    s255
# MsiAssemblyName   Component_  Name
####################################################################################

sub create_msiassemblyname_table
{
    my ($filesref, $basedir) = @_;

    my @msiassemblynametable = ();

    installer::windows::idtglobal::write_idt_header(\@msiassemblynametable, "msiassemblyname");

    for ( my $i = 0; $i <= $#{$installer::globals::msiassemblyfiles}; $i++ )
    {
        my $onefile = ${$installer::globals::msiassemblyfiles}[$i];

        my $component = get_msiassembly_component($onefile);
        my $oneline = "";

        # Order: (Assembly)name, publicKeyToken, version, culture.

        if ( $onefile->{'Assemblyname'} )
        {
            $oneline = $component . "\t" . "name" . "\t" . $onefile->{'Assemblyname'} . "\n";
            push(@msiassemblynametable, $oneline);
        }

        if ( $onefile->{'PublicKeyToken'} )
        {
            $oneline = $component . "\t" . "publicKeyToken" . "\t" . $onefile->{'PublicKeyToken'} . "\n";
            push(@msiassemblynametable, $oneline);
        }

        if ( $onefile->{'Version'} )
        {
            $oneline = $component . "\t" . "version" . "\t" . $onefile->{'Version'} . "\n";
            push(@msiassemblynametable, $oneline);
        }

        if ( $onefile->{'Culture'} )
        {
            $oneline = $component . "\t" . "culture" . "\t" . $onefile->{'Culture'} . "\n";
            push(@msiassemblynametable, $oneline);
        }

        if ( $onefile->{'ProcessorArchitecture'} )
        {
            $oneline = $component . "\t" . "processorArchitecture" . "\t" . $onefile->{'ProcessorArchitecture'} . "\n";
            push(@msiassemblynametable, $oneline);
        }
    }

    # Saving the file

    my $msiassemblynametablename = $basedir . $installer::globals::separator . "MsiAsseN.idt";
    installer::files::save_file($msiassemblynametablename ,\@msiassemblynametable);
    my $infoline = "Created idt file: $msiassemblynametablename\n";
    push(@installer::globals::logfileinfo, $infoline);

}

####################################################################################
# setting an installation condition for the assembly libraries saved in
# @installer::globals::msiassemblynamecontent
####################################################################################

sub add_assembly_condition_into_component_table
{
    my ($filesref, $basedir) = @_;

    my $componenttablename = $basedir . $installer::globals::separator . "Componen.idt";
    my $componenttable = installer::files::read_file($componenttablename);
    my $changed = 0;
    my $infoline = "";

    for ( my $i = 0; $i <= $#{$installer::globals::msiassemblyfiles}; $i++ )
    {
        my $onefile = ${$installer::globals::msiassemblyfiles}[$i];

        my $filecomponent = get_msiassembly_component($onefile);

        for ( my $j = 0; $j <= $#{$componenttable}; $j++ )
        {
            my $oneline = ${$componenttable}[$j];

            if ( $oneline =~ /(.*)\t(.*)\t(.*)\t(.*)\t(.*)\t(.*)/ )
            {
                my $component = $1;
                my $componentid = $2;
                my $directory = $3;
                my $attributes = $4;
                my $condition = $5;
                my $keypath = $6;

                if ( $component eq $filecomponent )
                {
                    # setting the condition

                    $condition = "DOTNET_SUFFICIENT=1";
                    $oneline = $component . "\t" . $componentid . "\t" . $directory . "\t" . $attributes . "\t" . $condition . "\t" . $keypath . "\n";
                    ${$componenttable}[$j] = $oneline;
                    $changed = 1;
                    $infoline = "Changing $componenttablename :\n";
                    push(@installer::globals::logfileinfo, $infoline);
                    $infoline = $oneline;
                    push(@installer::globals::logfileinfo, $infoline);
                    last;
                }
            }
        }
    }

    if ( $changed )
    {
        # Saving the file
        installer::files::save_file($componenttablename ,$componenttable);
        $infoline = "Saved idt file: $componenttablename\n";
        push(@installer::globals::logfileinfo, $infoline);
    }
}

1;
