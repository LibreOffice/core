#*************************************************************************
#
#   $RCSfile: assembly.pm,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: hr $ $Date: 2004-08-02 14:20:01 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc..
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

package installer::windows::assembly;

use installer::files;
use installer::globals;
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

    $filemanifest = $onefile->{'Name'};

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

    my $fileattributes = "0";

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

    my @msiassemblytable = ();

    installer::windows::idtglobal::write_idt_header(\@msiassemblytable, "msiassembly");

    # Registering all libraries listed in @installer::globals::msiassemblyfiles

    for ( my $i = 0; $i <= $#installer::globals::msiassemblyfiles; $i++ )
    {
        my $libraryname = $installer::globals::msiassemblyfiles[$i];

        my $onefile = get_msiassembly_file($filesref, $libraryname);

        if ( $onefile eq "" ) { next; } # library not part of this product

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
# Returning the name for the table MsiAssemblyName
####################################################################################

sub get_msiassemblyname_name
{
    ( $number ) = @_;

    my $name = "";

    if ( $number == 1 ) { $name = "name"; }
    elsif ( $number == 2 ) { $name = "publicKeyToken"; }
    elsif ( $number == 3 ) { $name = "version"; }

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

    # Registering all libraries listed in @installer::globals::msiassemblynamecontent

    for ( my $i = 0; $i <= $#installer::globals::msiassemblynamecontent; $i++ )
    {
        my $libraryname = $installer::globals::msiassemblynamecontent[$i];

        my $onefile = get_msiassembly_file($filesref, $libraryname);

        if ( $onefile eq "" )   # library not part of this product
        {
            $i = $i + 3;    # next library
            next;
        }

        my $component = get_msiassembly_component($onefile);

        for ( my $j = 1; $j <= 3; $j++ )
        {
            # the content is for an assembly is saved in @installer::globals::msiassemblynamecontent
            # in the order: Libraryname, Name, publicKeyToken, Version. Then the next library is defined.

            my $counter = $i + $j;

            my %msiassemblyname = ();

            $msiassemblyname{'Component_'} = $component;
            $msiassemblyname{'Name'} = get_msiassemblyname_name($j);
            $msiassemblyname{'Value'} = $installer::globals::msiassemblynamecontent[$counter];

            my $oneline = $msiassemblyname{'Component_'} . "\t" . $msiassemblyname{'Name'} . "\t" .
                            $msiassemblyname{'Value'} . "\n";

            push(@msiassemblynametable, $oneline);
        }

        $i = $i + 3; # increasing the counter!
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

    for ( my $i = 0; $i <= $#installer::globals::msiassemblyfiles; $i++ )
    {
        my $libraryname = $installer::globals::msiassemblyfiles[$i];
        my $onefile = get_msiassembly_file($filesref, $libraryname);
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

                    $condition = "NET_FRAMEWORK_INSTALLED=1";
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