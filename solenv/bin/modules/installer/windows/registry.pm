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



package installer::windows::registry;

use installer::files;
use installer::globals;
use installer::worker;
use installer::windows::msiglobal;
use installer::windows::idtglobal;

use strict;

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

    $componentname = $componentname . $addon;

    if (( $styles =~ /\bLANGUAGEPACK\b/ ) && ( $installer::globals::languagepack )) { $componentname = $componentname . "_lang"; }
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
        if ( length($componentname) > 70 )
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

    my $startversion = substr($componentname, 0, 60); # taking only the first 60 characters
    my $subid = installer::windows::msiglobal::calculate_id($componentname, 9); # taking only the first 9 digits
    my $shortcomponentname = $startversion . "_" . $subid;

    if ( exists($installer::globals::allshortregistrycomponents{$shortcomponentname}) ) { installer::exiter::exit_program("Failed to create unique component name: \"$shortcomponentname\"", "generate_new_short_registrycomponentname"); }

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
# Returning 64 bit value for registry table.
##############################################################

sub get_registry_val64
{
    my ($registry, $allvariableshashref) = @_;

    my $value = "";

    if ( $registry->{'Val64'} ) { $value = $registry->{'Val64'}; }

    $value =~ s/\\\"/\"/g;  # no more masquerading of '"'
    $value =~ s/\\\\\s*$/\\/g;  # making "\\" at end of value to "\"
    $value =~ s/\<progpath\>/\[INSTALLLOCATION\]/;
    $value =~ s/\[INSTALLLOCATION\]\\/\[INSTALLLOCATION\]/; # removing "\" after "[INSTALLLOCATION]"

    if ( $value =~ /\%/ ) { $value = installer::worker::replace_variables_in_string($value, $allvariableshashref); }

    return $value;
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

        my $styles = "";
        if ( $onefile->{'Styles'} ) { $styles = $onefile->{'Styles'}; }

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
sub prepare_registry_table ($$$)
{
    my ($registryref, $languagesarrayref, $allvariableshashref) = @_;

    my %table_data = ();
    foreach my $onelanguage (@$languagesarrayref)
    {
        my $table_items = [];
        foreach my $oneregistry (@$registryref)
        {
            # Controlling the language!
            # Only language independent folderitems or folderitems with the correct language
            # will be included into the table

            next if $oneregistry->{'ismultilingual'}
                && $oneregistry->{'specificlanguage'} ne $onelanguage;

            my %registry = ();

            $registry{'Registry'} = get_registry_identifier($oneregistry);
            $registry{'Root'} = get_registry_root($oneregistry);
            $registry{'Key'} = get_registry_key($oneregistry, $allvariableshashref);
            $registry{'Name'} = get_registry_name($oneregistry, $allvariableshashref);
            $registry{'Value'} = get_registry_value($oneregistry, $allvariableshashref);
            $registry{'Val64'} = get_registry_val64($oneregistry, $allvariableshashref);
            my $component_name = get_registry_component_name($oneregistry, $allvariableshashref);
            $oneregistry->{'componentname'} = $component_name;
            $registry{'Component_'} = $component_name;

            # Collecting all components with DONT_DELETE style
            my $style = $oneregistry->{'Styles'} // "";
            $registry{'styles'} = $style;

            if ( $style =~ /\bDONT_DELETE\b/ )
            {
                $installer::globals::dontdeletecomponents{$component_name} = 1;
            }

            # Saving upgradekey to write this into setup.ini for minor upgrades
            if ( $style =~ /\bUPGRADEKEY\b/ )
            {
                $installer::globals::minorupgradekey = $registry{'Key'};
            }

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

            push @$table_items, \%registry;
        }
        $table_data{$onelanguage} = $table_items;
    }

    return \%table_data;
}




sub collect_registry_components ($)
{
    my ($table_data) = @_;

    my %components = ();
    foreach my $language_data (values %$table_data)
    {
        foreach my $item (@$language_data)
        {
            $components{$item->{'Component_'}} = 1;
        }
    }
    return keys %components;
}




sub translate_component_names ($$$)
{
    my ($translation_map, $registry_items, $table_data) = @_;

    my $replacement_count = 0;
    foreach my $item (@$registry_items)
    {
        my $translated_name = $translation_map->{$item->{'componentname'}};
        if (defined $translated_name)
        {
            $item->{'componentname'} = $translated_name;
            ++$replacement_count;
        }
    }
    $installer::logger::Lang->printf("replaced %d component names in registry items\n", $replacement_count);

    $replacement_count = 0;
    foreach my $language_data (values %$table_data)
    {
        foreach my $item (@$language_data)
        {
            my $translated_name = $translation_map->{$item->{'Component_'}};
            if (defined $translated_name)
            {
                $item->{'Component_'} = $translated_name;
                ++$replacement_count;
            }
        }
    }
    $installer::logger::Lang->printf("replaced %d component names in registry table\n", $replacement_count);
}




sub create_registry_table_32 ($$$$)
{
    my ($basedir, $languagesarrayref, $allvariableshashref, $table_data) = @_;

    foreach my $onelanguage (@$languagesarrayref)
    {
        my @registrytable = ();
        installer::windows::idtglobal::write_idt_header(\@registrytable, "registry");

        foreach my $item (@{$table_data->{$onelanguage}})
        {
            next if $item->{'styles'} =~ /\bX64_ONLY\b/;

            my $oneline = join("\t",
                $item->{'Registry'},
                $item->{'Root'},
                $item->{'Key'},
                $item->{'Name'},
                $item->{'Value'},
                $item->{'Component_'})
                . "\n";

            push(@registrytable, $oneline);
        }

        # If there are added user registry keys for files collected in
        # @installer::globals::userregistrycollector (file.pm), then
        # this registry keys have to be added now. This is necessary for
        # files in PREDEFINED_OSSHELLNEWDIR, because their component
        # needs as KeyPath a RegistryItem in HKCU.

        if ( $installer::globals::addeduserregitrykeys )
        {
            add_userregs_to_registry_table(\@registrytable, $allvariableshashref);
        }

        # Save the database file.
        my $registrytablename = $basedir . $installer::globals::separator . "Registry.idt" . "." . $onelanguage;
        installer::files::save_file($registrytablename ,\@registrytable);
        $installer::logger::Lang->printf("Created idt file: %s\n", $registrytablename);
    }
}




sub create_registry_table_64 ($$$$)
{
    my ($basedir, $languagesarrayref, $allvariableshashref, $table_data) = @_;

    foreach my $onelanguage (@$languagesarrayref)
    {
        my @reg64table = ();
        installer::windows::idtglobal::write_idt_header(\@reg64table, "reg64");
        foreach my $item (@{$table_data->{$onelanguage}})
        {
            next unless $item->{'styles'} =~ /\b(X64|X64_ONLY)\b/;

            my $oneline64 = join("\t",
                $item->{'Registry'},
                $item->{'Root'},
                $item->{'Key'},
                $item->{'Name'},
                $item->{'Val64'},
                $item->{'Component_'})
                . "\n";

            push(@reg64table , $oneline64);
        }

        # Save the database file.
        my $registrytablename = $basedir . $installer::globals::separator . "Reg64.idt" . "." . $onelanguage;
        installer::files::save_file($registrytablename ,\@reg64table );
        $installer::logger::Lang->printf("Created idt file: %s\n", $registrytablename);
    }
}

1;
