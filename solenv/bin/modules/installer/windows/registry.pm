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

package installer::windows::registry;

use installer::files;
use installer::globals;
use installer::worker;
use installer::windows::idtglobal;

#####################################################
# Generating the component name from a registryitem
#####################################################

sub get_registry_component_name
{
    my ($registryref, $allvariables) = @_;

    # In this function exists the rule to create components from registryitems
    # Rule:
    # The componentname can be directly taken from the ModuleID.
    # All registryitems belonging to one module can get the same component.

    my $componentname = "";
    my $isrootmodule = 0;

    if ( $registryref->{'ModuleID'} ) { $componentname = $registryref->{'ModuleID'}; }

    $componentname =~ s/\\/\_/g;
    $componentname =~ s/\//\_/g;
    $componentname =~ s/\-/\_/g;
    $componentname =~ s/\_\s*$//g;

    $componentname = lc($componentname);    # componentnames always lowercase

    if ( $componentname eq "gid_module_root" ) { $isrootmodule = 1; }

    # Attention: Maximum length for the componentname is 72

    # identifying this component as registryitem component
    $componentname = "registry_" . $componentname;

    $componentname =~ s/gid_module_/g_m_/g;
    $componentname =~ s/_optional_/_o_/g;
    $componentname =~ s/_javafilter_/_jf_/g;

    # This componentname must be more specific
    my $addon = "_";
    if ( $allvariables->{'PRODUCTNAME'} ) { $addon = $addon . $allvariables->{'PRODUCTNAME'}; }
    if ( $allvariables->{'PRODUCTVERSION'} ) { $addon = $addon . $allvariables->{'PRODUCTVERSION'}; }
    $addon = lc($addon);
    $addon =~ s/ //g;
    $addon =~ s/-//g;
    $addon =~ s/\.//g;

    my $styles = "";
    if ( $registryref->{'Styles'} ) { $styles = $registryref->{'Styles'}; }

    # Layer links must have unique Component GUID for all products. This is necessary, because only the
    # uninstallation of the last product has to delete registry keys.
    if ( $styles =~ /\bLAYER_REGISTRY\b/ )
    {
        $componentname = "g_m_root_registry_layer_ooo_reglayer";
        # Styles USE_URELAYERVERSION, USE_PRODUCTVERSION
        if ( $styles =~ /\bUSE_URELAYERVERSION\b/ ) { $addon = "_ure_" . $allvariables->{'URELAYERVERSION'}; }
        if ( $styles =~ /\bUSE_PRODUCTVERSION\b/ ) { $addon = "_basis_" . $allvariables->{'PRODUCTVERSION'}; }
        $addon =~ s/\.//g;
    }

    $componentname = $componentname . $addon;

    if (( $styles =~ /\bLANGUAGEPACK\b/ ) && ( $installer::globals::languagepack )) { $componentname = $componentname . "_lang"; }
    elsif (( $styles =~ /\bHELPPACK\b/ ) && ( $installer::globals::helppack )) { $componentname = $componentname . "_help"; }
    if ( $styles =~ /\bALWAYS_REQUIRED\b/ ) { $componentname = $componentname . "_forced"; }

    # Attention: Maximum length for the componentname is 72
    # %installer::globals::allregistrycomponents_in_this_database_ : resetted for each database
    # %installer::globals::allregistrycomponents_ : not resetted for each database
    # Component strings must be unique for the complete product, because they are used for
    # the creation of the globally unique identifier.

    my $fullname = $componentname;  # This can be longer than 72

    if (( exists($installer::globals::allregistrycomponents_{$fullname}) ) && ( ! exists($installer::globals::allregistrycomponents_in_this_database_{$fullname}) ))
    {
        # This is not allowed: One component cannot be installed with different packages.
        installer::exiter::exit_program("ERROR: Windows registry component \"$fullname\" is already included into another package. This is not allowed.", "get_registry_component_name");
    }

    if ( exists($installer::globals::allregistrycomponents_{$fullname}) )
    {
        $componentname = $installer::globals::allregistrycomponents_{$fullname};
    }
    else
    {
        if ( length($componentname) > 60 )
        {
            $componentname = generate_new_short_registrycomponentname($componentname); # This has to be unique for the complete product, not only one package
        }

        $installer::globals::allregistrycomponents_{$fullname} = $componentname;
        $installer::globals::allregistrycomponents_in_this_database_{$fullname} = 1;
    }

    if ( $isrootmodule ) { $installer::globals::registryrootcomponent = $componentname; }

    return $componentname;
}

#########################################################
# Create a shorter version of a long component name,
# because maximum length in msi database is 72.
# Attention: In multi msi installation sets, the short
# names have to be unique over all packages, because
# this string is used to create the globally unique id
# -> no resetting of
# %installer::globals::allshortregistrycomponents
# after a package was created.
#########################################################

sub generate_new_short_registrycomponentname
{
    my ($componentname) = @_;

    my $shortcomponentname = "";
    my $counter = 1;

    my $startversion = substr($componentname, 0, 60); # taking only the first 60 characters
    $startversion = $startversion . "_";

    $shortcomponentname = $startversion . $counter;

    while ( exists($installer::globals::allshortregistrycomponents{$shortcomponentname}) )
    {
        $counter++;
        $shortcomponentname = $startversion . $counter;
    }

    $installer::globals::allshortregistrycomponents{$shortcomponentname} = 1;

    return $shortcomponentname;
}

##############################################################
# Returning identifier for registry table.
##############################################################

sub get_registry_identifier
{
    my ($registry) = @_;

    my $identifier = "";

    if ( $registry->{'gid'} ) { $identifier = $registry->{'gid'}; }

    $identifier = lc($identifier);  # always lower case

    # Attention: Maximum length is 72

    $identifier =~ s/gid_regitem_/g_r_/;
    $identifier =~ s/_soffice_/_s_/;
    $identifier =~ s/_clsid_/_c_/;
    $identifier =~ s/_currentversion_/_cv_/;
    $identifier =~ s/_microsoft_/_ms_/;
    $identifier =~ s/_manufacturer_/_mf_/;
    $identifier =~ s/_productname_/_pn_/;
    $identifier =~ s/_productversion_/_pv_/;
    $identifier =~ s/_staroffice_/_so_/;
    $identifier =~ s/_software_/_sw_/;
    $identifier =~ s/_capabilities_/_cap_/;
    $identifier =~ s/_classpath_/_cp_/;
    $identifier =~ s/_extension_/_ex_/;
    $identifier =~ s/_fileassociations_/_fa_/;
    $identifier =~ s/_propertysheethandlers_/_psh_/;
    $identifier =~ s/__/_/g;

    # Saving this in the registry collector

    $registry->{'uniquename'} = $identifier;

    return $identifier;
}

##################################################################
# Returning root value for registry table.
##################################################################

sub get_registry_root
{
    my ($registry) = @_;

    my $rootvalue = 0;  # Default: Parent is KKEY_CLASSES_ROOT
    my $scproot = "";

    if ( $registry->{'ParentID'} ) { $scproot = $registry->{'ParentID'}; }

    if ( $scproot eq "PREDEFINED_HKEY_LOCAL_MACHINE" ) { $rootvalue = -1; }

    if ( $scproot eq "PREDEFINED_HKEY_CLASSES_ROOT" ) { $rootvalue = 0; }

    if ( $scproot eq "PREDEFINED_HKEY_CURRENT_USER_ONLY" ) { $rootvalue = 1; }

    if ( $scproot eq "PREDEFINED_HKEY_LOCAL_MACHINE_ONLY" ) { $rootvalue = 2; }

    return $rootvalue;
}

##############################################################
# Returning key for registry table.
##############################################################

sub get_registry_key
{
    my ($registry, $allvariableshashref) = @_;

    my $key = "";

    if ( $registry->{'Subkey'} ) { $key = $registry->{'Subkey'}; }

    if ( $key =~ /\%/ ) { $key = installer::worker::replace_variables_in_string($key, $allvariableshashref); }

    return $key;
}

##############################################################
# Returning name for registry table.
##############################################################

sub get_registry_name
{
    my ($registry, $allvariableshashref) = @_;

    my $name = "";

    if ( $registry->{'Name'} ) { $name = $registry->{'Name'}; }

    if ( $name =~ /\%/ ) { $name = installer::worker::replace_variables_in_string($name, $allvariableshashref); }

    return $name;
}

##############################################################
# Returning value for registry table.
##############################################################

sub get_registry_value
{
    my ($registry, $allvariableshashref) = @_;

    my $value = "";

    if ( $registry->{'Value'} ) { $value = $registry->{'Value'}; }

    $value =~ s/\\\"/\"/g;  # no more masquerading of '"'
    $value =~ s/\\\\\s*$/\\/g;  # making "\\" at end of value to "\"
    $value =~ s/\<progpath\>/\[INSTALLLOCATION\]/;
    $value =~ s/\[INSTALLLOCATION\]\\/\[INSTALLLOCATION\]/; # removing "\" after "[INSTALLLOCATION]"

    if ( $value =~ /\%/ ) { $value = installer::worker::replace_variables_in_string($value, $allvariableshashref); }

    return $value;
}

##############################################################
# Returning component for registry table.
##############################################################

sub get_registry_component
{
    my ($registry, $allvariables) = @_;

    # All registry items belonging to one module can
    # be included into one component

    my $componentname = get_registry_component_name($registry, $allvariables);

    # saving componentname in the registryitem collector

    $registry->{'componentname'} = $componentname;

    return $componentname;
}

######################################################
# Adding the content of
# @installer::globals::userregistrycollector
# to the registry table. The content was collected
# in create_files_table() in file.pm.
######################################################

sub add_userregs_to_registry_table
{
    my ( $registrytable, $allvariables ) = @_;

    for ( my $i = 0; $i <= $#installer::globals::userregistrycollector; $i++ )
    {
        my $onefile = $installer::globals::userregistrycollector[$i];

        my %registry = ();

        $registry{'Registry'} = $onefile->{'userregkeypath'};
        $registry{'Root'} = "1";  # always HKCU
        $registry{'Key'} = "Software\\$allvariables->{'MANUFACTURER'}\\$allvariables->{'PRODUCTNAME'} $allvariables->{'PRODUCTVERSION'}\\";
        if ( $onefile->{'needs_user_registry_key'} ) { $registry{'Key'} = $registry{'Key'} . "StartMenu"; }
        else { $registry{'Key'} = $registry{'Key'} . "ShellNew"; }
        $registry{'Name'} = $onefile->{'Name'};
        $registry{'Value'} = "1";
        $registry{'Component_'} = $onefile->{'componentname'};

        my $oneline = $registry{'Registry'} . "\t" . $registry{'Root'} . "\t" . $registry{'Key'} . "\t"
                    . $registry{'Name'} . "\t" . $registry{'Value'} . "\t" . $registry{'Component_'} . "\n";

        push(@{$registrytable}, $oneline);
    }
}

######################################################
# Creating the file Registry.idt dynamically
# Content:
# Registry Root Key Name Value Component_
######################################################

sub create_registry_table
{
    my ($registryref, $allregistrycomponentsref, $basedir, $languagesarrayref, $allvariableshashref) = @_;

    for ( my $m = 0; $m <= $#{$languagesarrayref}; $m++ )
    {
        my $onelanguage = ${$languagesarrayref}[$m];

        my @registrytable = ();

        installer::windows::idtglobal::write_idt_header(\@registrytable, "registry");

        for ( my $i = 0; $i <= $#{$registryref}; $i++ )
        {
            my $oneregistry = ${$registryref}[$i];

            # Controlling the language!
            # Only language independent folderitems or folderitems with the correct language
            # will be included into the table

            if (! (!(( $oneregistry->{'ismultilingual'} )) || ( $oneregistry->{'specificlanguage'} eq $onelanguage )) )  { next; }

            my %registry = ();

            $registry{'Registry'} = get_registry_identifier($oneregistry);
            $registry{'Root'} = get_registry_root($oneregistry);
            $registry{'Key'} = get_registry_key($oneregistry, $allvariableshashref);
            $registry{'Name'} = get_registry_name($oneregistry, $allvariableshashref);
            $registry{'Value'} = get_registry_value($oneregistry, $allvariableshashref);
            $registry{'Component_'} = get_registry_component($oneregistry, $allvariableshashref);

            # Collecting all components
            if (! grep {$_ eq $registry{'Component_'}} @{$allregistrycomponentsref})
            {
                push(@{$allregistrycomponentsref}, $registry{'Component_'});
            }

            # Collecting all components with DONT_DELETE style
            my $style = "";
            if ( $oneregistry->{'Styles'} ) { $style = $oneregistry->{'Styles'}; }
            if ( $style =~ /\bDONT_DELETE\b/ ) { $installer::globals::dontdeletecomponents{$registry{'Component_'}} = 1; }

            # Collecting all registry components with ALWAYS_REQUIRED style
            if ( ! ( $style =~ /\bALWAYS_REQUIRED\b/ ))
            {
                # Setting a component condition for unforced registry components!
                # Only write into registry, if WRITE_REGISTRY is set.
                if ( $oneregistry->{'ComponentCondition'} ) { $oneregistry->{'ComponentCondition'} = "(" . $oneregistry->{'ComponentCondition'} . ") AND (WRITE_REGISTRY=1)"; }
                else { $oneregistry->{'ComponentCondition'} = "WRITE_REGISTRY=1"; }
            }

            # Collecting all component conditions
            if ( $oneregistry->{'ComponentCondition'} )
            {
                if ( ! exists($installer::globals::componentcondition{$registry{'Component_'}}))
                {
                    $installer::globals::componentcondition{$registry{'Component_'}} = $oneregistry->{'ComponentCondition'};
                }
            }

            my $oneline = $registry{'Registry'} . "\t" . $registry{'Root'} . "\t" . $registry{'Key'} . "\t"
                        . $registry{'Name'} . "\t" . $registry{'Value'} . "\t" . $registry{'Component_'} . "\n";

            push(@registrytable, $oneline);
        }

        # If there are added user registry keys for files collected in
        # @installer::globals::userregistrycollector (file.pm), then
        # this registry keys have to be added now. This is necessary for
        # files in PREDEFINED_OSSHELLNEWDIR, because their component
        # needs as KeyPath a RegistryItem in HKCU.

        if ( $installer::globals::addeduserregitrykeys ) { add_userregs_to_registry_table(\@registrytable, $allvariableshashref); }

        # Saving the file

        my $registrytablename = $basedir . $installer::globals::separator . "Registry.idt" . "." . $onelanguage;
        installer::files::save_file($registrytablename ,\@registrytable);
        my $infoline = "Created idt file: $registrytablename\n";
        push(@installer::globals::logfileinfo, $infoline);
    }
}

1;
