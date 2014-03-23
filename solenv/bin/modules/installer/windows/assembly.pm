#**************************************************************
#
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#
#**************************************************************



package installer::windows::assembly;

use installer::files;
use installer::globals;
use installer::worker;
use installer::windows::idtglobal;

use strict;

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
    # $filemanifest = $onefile->{'Name'};

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
    # if (! $foundfile ) { installer::exiter::exit_program("ERROR: No unique file name found for $filename !", "get_selfreg_file"); }

    if (! $foundfile ) { $onefile  = ""; }

    return $onefile;
}

##############################################################
# Returning the file object for the msiassembly table.
##############################################################

sub get_msiassembly_file_by_gid
{
    my ( $filesref, $gid ) = @_;

    my $foundfile = 0;
    my $onefile;

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        $onefile = ${$filesref}[$i];
        my $filegid = $onefile->{'gid'};

        if ( $filegid eq $gid )
        {
            $foundfile = 1;
            last;
        }
    }

    # It does not need to exist. For example products that do not contain the libraries.
    # if (! $foundfile ) { installer::exiter::exit_program("ERROR: No unique file name found for $filename !", "get_selfreg_file"); }

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
    $installer::logger::Lang->print($infoline);
}

####################################################################################
# Returning the name for the table MsiAssemblyName
####################################################################################

sub get_msiassemblyname_name ($)
{
    my ($number) = @_;

    my $name = "";

    if ( $number == 1 ) { $name = "name"; }
    elsif ( $number == 2 ) { $name = "publicKeyToken"; }
    elsif ( $number == 3 ) { $name = "version"; }
    elsif ( $number == 4 ) { $name = "culture"; }

    return $name;
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
    $installer::logger::Lang->print($infoline);

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

    foreach my $onefile (@$installer::globals::msiassemblyfiles)
    {
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

                    # $condition = "MsiNetAssemblySupport";
                    $condition = "DOTNET_SUFFICIENT=1";
                    $oneline = join("\t",
                        $component,
                        $componentid,
                        $directory,
                        $attributes,
                        $condition,
                        $keypath) . "\n";
                    ${$componenttable}[$j] = $oneline;
                    $changed = 1;

                    $installer::logger::Lang->printf("Changing %s :\n",  $componenttablename);
                    $installer::logger::Lang->print($oneline);

                    last;
                }
            }
        }
    }

    if ( $changed )
    {
        # Saving the file
        installer::files::save_file($componenttablename ,$componenttable);
        $installer::logger::Lang->printf("Saved idt file: %s\n", $componenttablename);
    }
}

1;
