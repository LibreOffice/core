#*************************************************************************
#
#   $RCSfile: component.pm,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: kz $ $Date: 2004-06-11 18:18:38 $
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
    # At this time only a template

    return "\{COMPONENTGUID\}";
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
        installer::exiter::exit_program("ERROR: Did not find component in file collection", "get_file_component_directory");
    }

    my $localstyles = "";

    if ( $onefile->{'Styles'} ) { $localstyles = $onefile->{'Styles'}; }

    if ( $localstyles =~ /\bFONT\b/ )   # special handling for font files
    {
        return $installer::globals::fontsfolder;
    }

    my $destination = $onefile->{'destination'};

    installer::pathanalyzer::get_path_from_fullqualifiedname(\$destination);

    $destination =~ s/\Q$installer::globals::separator\E\s*$//;

    # This path has to be defined in the directory collection at "HostName"

    if ($destination eq "")     # files in the installation root
    {
        if ($installer::globals::product =~ /ada/i )
        {
            $uniquedir = "INSTALLLOCATION";
        }
        else
        {
            $uniquedir = $installer::globals::officefolder;
        }
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
    return $installer::globals::officefolder;
}

##############################################################
# Returning the attributes for a file component.
# Always 8 in this first try?
##############################################################

sub get_file_component_attributes
{
    my ($componentname, $filesref) = @_;

    my $attributes;

    $attributes = 8;

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

    return $attributes
}

##############################################################
# Returning the attributes for a registry component.
# Always 4, indicating, the keypath is a defined in
# table registry
##############################################################

sub get_registry_component_attributes
{
    my ($componentname) = @_;

    my $attributes;

    $attributes = 4;

    return $attributes
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

    # Setting only a condition, if this is a multilingual installation set.
    # This info is stored in the global variable $ismultilingual

    if ( $installer::globals::ismultilingual )
    {
        my $found = 0;
        my $onefile;

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
            installer::exiter::exit_program("ERROR: Did not find component in file collection", "get_file_component_directory");
        }

        # Searching for multilingual files

        if ( $onefile->{'ismultilingual'} )
        {
            my $officelanguage = $onefile->{'specificlanguage'};
            my $windowslanguage = installer::windows::language::get_windows_language($officelanguage);  # converting to the Windows language
            $condition = "IS" . $windowslanguage . "=1";     # Capitol letter "IS" !
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
    my ($componentname, $itemsref) = @_;

    my $oneitem;
    my $found = 0;

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

    $oneitem->{'keypath'} = $keypath;       # saving it in the file and registry collection

    return $keypath
}

###################################################################
# Creating the file Componen.idt dynamically
# Content:
# Component ComponentId Directory_ Attributes Condition KeyPath
###################################################################

sub create_component_table
{
    my ($filesref, $registryref, $dirref, $allfilecomponentsref, $allregistrycomponents, $basedir) = @_;

    my @componenttable = ();

    my ($oneline, $infoline);

    installer::windows::idtglobal::write_idt_header(\@componenttable, "component");

    # File components

    for ( my $i = 0; $i <= $#{$allfilecomponentsref}; $i++ )
    {
        my %onecomponent = ();

        $onecomponent{'name'} = ${$allfilecomponentsref}[$i];
        $onecomponent{'guid'} = get_component_guid();
        $onecomponent{'directory'} = get_file_component_directory($onecomponent{'name'}, $filesref, $dirref);
        $onecomponent{'attributes'} = get_file_component_attributes($onecomponent{'name'}, $filesref);
        $onecomponent{'condition'} = get_file_component_condition($onecomponent{'name'}, $filesref);
        $onecomponent{'keypath'} = get_component_keypath($onecomponent{'name'}, $filesref);

        $oneline = $onecomponent{'name'} . "\t" . $onecomponent{'guid'} . "\t" . $onecomponent{'directory'} . "\t"
                . $onecomponent{'attributes'} . "\t" . $onecomponent{'condition'} . "\t" . $onecomponent{'keypath'} . "\n";

        push(@componenttable, $oneline);
    }

    # Registry components

    for ( my $i = 0; $i <= $#{$allregistrycomponents}; $i++ )
    {
        my %onecomponent = ();

        $onecomponent{'name'} = ${$allregistrycomponents}[$i];
        $onecomponent{'guid'} = get_component_guid();
        $onecomponent{'directory'} = get_registry_component_directory();
        $onecomponent{'attributes'} = get_registry_component_attributes($onecomponent{'name'});
        $onecomponent{'condition'} = get_component_condition($onecomponent{'name'});
        $onecomponent{'keypath'} = get_component_keypath($onecomponent{'name'}, $registryref);

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

1;