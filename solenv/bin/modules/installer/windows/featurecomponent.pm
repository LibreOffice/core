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



package installer::windows::featurecomponent;

use installer::converter;
use installer::existence;
use installer::exiter;
use installer::files;
use installer::globals;
use installer::windows::idtglobal;

use strict;


#################################################################################
# Collecting all pairs of features and components from the files collector
#################################################################################

sub create_featurecomponent_table_from_files_collector ($$)
{
    my ($featurecomponenttableref, $filesref) = @_;

    foreach my $onefile (@$filesref)
    {
        my $filecomponent = $onefile->{'componentname'};
        my $filemodules = $onefile->{'modules'};

        if ( $filecomponent eq "" )
        {
            installer::exiter::exit_program(
                sprintf("ERROR: No component defined for file %s", $onefile->{'Name'}),
                "create_featurecomponent_table_from_files_collector");
        }
        if ( ! defined $filemodules)
        {
            # Temporary for files created from source installation set.
            die;
        }
        if ($filemodules eq "")
        {
            installer::exiter::exit_program(
                sprintf("ERROR: No modules found for file %s", $onefile->{'Name'}),
                "create_featurecomponent_table_from_files_collector");
        }

        my $filemodulesarrayref = installer::converter::convert_stringlist_into_array(\$filemodules, ",");

        foreach my $onemodule (@$filemodulesarrayref)
        {
            my %featurecomponent = ();

            $onemodule =~ s/\s*$//;
            $featurecomponent{'Feature'} = $onemodule;
            $featurecomponent{'Component'} = $filecomponent;

            # Attention: Features are renamed, because the maximum length is 38.
            # But in the files collector ($filesref), the original names are saved.

            installer::windows::idtglobal::shorten_feature_gid(\$featurecomponent{'Feature'});

            my $oneline = "$featurecomponent{'Feature'}\t$featurecomponent{'Component'}\n";

            # control of uniqueness

            if (! installer::existence::exists_in_array($oneline, $featurecomponenttableref))
            {
                push(@{$featurecomponenttableref}, $oneline);
            }
        }
    }
}




=head2 create_featurecomponent_table_from_registry_collector ($featurecomponenttableref, $registryref)

    Add entries for the FeatureComponent table for components that contain registry entries.

=cut
sub create_featurecomponent_table_from_registry_collector ($$)
{
    my ($featurecomponenttableref, $registryref) = @_;

    my $replacement_count = 0;
    my $unique_count = 0;
    foreach my $oneregistry (@$registryref)
    {
        my $component_name = $oneregistry->{'componentname'};
        if ($component_name eq "")
        {
            installer::exiter::exit_program(
                sprintf("ERROR: No component defined for registry %s", $oneregistry->{'gid'}),
                "create_featurecomponent_table_from_registry_collector");
        }

        my $feature_name = $oneregistry->{'ModuleID'};
        if ($feature_name eq "")
        {
            installer::exiter::exit_program(
                sprintf("ERROR: No modules found for registry %s", $oneregistry->{'gid'}),
                "create_featurecomponent_table_from_registry_collector");
        }

        # Attention: Features are renamed, because the maximum length is 38.
        # But in the files collector ($filesref), the original names are saved.

        $feature_name = installer::windows::idtglobal::create_shortend_feature_gid($feature_name);

        my $oneline = sprintf("%s\t%s\n", $feature_name, $component_name);
        if ( ! installer::existence::exists_in_array($oneline, $featurecomponenttableref))
        {
            push(@$featurecomponenttableref, $oneline);
            ++$unique_count;
        }
        else
        {
            $installer::logger::Lang->printf("feature component pair already exists\n");
        }
    }
    $installer::logger::Lang->printf(
        "replaced %d (%d) of %d component names in FeatureComponent table\n",
        $unique_count,
        $replacement_count,
        scalar @$registryref);
}

#################################################################################
# Collecting all feature that are listed in the featurecomponent table.
#################################################################################

sub collect_all_features
{
    my ($featurecomponenttable) = @_;

    my @allfeature = ();

    for ( my $i = 3; $i <= $#{$featurecomponenttable}; $i++ )   # beginning in line 4
    {
        my $oneline = ${$featurecomponenttable}[$i];

        if ( $oneline =~ /^\s*(\S+)\s+(\S+)\s*$/ )
        {
            my $feature = $1;

            if (! installer::existence::exists_in_array($feature, \@allfeature))
            {
                push(@allfeature, $feature);
            }
        }
    }

    return \@allfeature;
}

#################################################################################
# On Win98 and Win Me there seems to be the problem, that maximum 817
# components can be added to a feature. Even if Windows Installer 2.0
# is used.
#################################################################################

sub check_number_of_components_at_feature
{
    my ($featurecomponenttable) = @_;

    $installer::logger::Lang->print("\n");
    $installer::logger::Lang->print("Checking number of components at features. Maximum is 817 (for Win 98 and Win Me)\n");

    my $allfeature = collect_all_features($featurecomponenttable);

    for ( my $i = 0; $i <= $#{$allfeature}; $i++ )
    {
        my $onefeature = ${$allfeature}[$i];
        my $featurecomponents = 0;

        for ( my $j = 0; $j <= $#{$featurecomponenttable}; $j++ )
        {
            if ( ${$featurecomponenttable}[$j] =~ /^\s*\Q$onefeature\E\s+(\S+)\s*$/ ) { $featurecomponents++; }
        }

        if ( $featurecomponents > 816 )
        {
            installer::exiter::exit_program("ERROR: More than 816 components ($featurecomponents) at feature $onefeature. This causes problems on Win 98 and Win Me!", "check_number_of_components_at_feature");
        }

        # Logging the result

        $installer::logger::Lang->printf("Number of components at feature $onefeature : %s\n", $featurecomponents);
    }

    $installer::logger::Lang->print("\n");
}

#################################################################################
# Creating the file FeatureC.idt dynamically
# Content:
# Feature Component
#################################################################################

sub create_featurecomponent_table ($$$)
{
    my ($filesref, $registryref, $basedir) = @_;

    my @featurecomponenttable = ();
    my $infoline;

    installer::windows::idtglobal::write_idt_header(\@featurecomponenttable, "featurecomponent");

    # This is the first time, that features and componentes are related
    # Problem: How about created profiles, configurationfiles, services.rdb
    # -> simple solution: putting them all to the root module
    # Otherwise profiles and configurationfiles cannot be created the way, they are now created
    # -> especially a problem for the configurationfiles! # ToDo
    # Very good: All ProfileItems belong to the root
    # services.rdb belongs to the root anyway.

    # At the moment only the files are related to components (and the files know their modules).
    # The component for each file is written into the files collector $filesinproductlanguageresolvedarrayref

    create_featurecomponent_table_from_files_collector(
        \@featurecomponenttable,
        $filesref);

    create_featurecomponent_table_from_registry_collector(
        \@featurecomponenttable,
        $registryref);

    # Additional components have to be added here

    # Checking, whether there are more than 817 components at a feature

    check_number_of_components_at_feature(\@featurecomponenttable);

    # Saving the file

    my $featurecomponenttablename = $basedir . $installer::globals::separator . "FeatureC.idt";
    installer::files::save_file($featurecomponenttablename ,\@featurecomponenttable);
    $installer::logger::Lang->printf("Created idt file: %s\n", $featurecomponenttablename);
}

1;
