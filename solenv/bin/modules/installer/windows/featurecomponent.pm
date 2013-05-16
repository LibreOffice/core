#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

package installer::windows::featurecomponent;

use installer::converter;
use installer::exiter;
use installer::files;
use installer::globals;
use installer::windows::idtglobal;

#################################################################################
# Collecting all pairs of features and components from the files collector
#################################################################################

sub create_featurecomponent_table_from_files_collector
{
    my ($featurecomponenttableref, $filesref) = @_;

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        my $onefile = ${$filesref}[$i];

        my $filecomponent = $onefile->{'componentname'};
        my $filemodules = $onefile->{'modules'};

        if ( $filecomponent eq "" )
        {
            installer::exiter::exit_program("ERROR: No component defined for file $onefile->{'Name'}", "create_featurecomponent_table_from_files_collector");
        }
        if ( $filemodules eq "" )
        {
            installer::exiter::exit_program("ERROR: No modules found for file $onefile->{'Name'}", "create_featurecomponent_table_from_files_collector");
        }

        my $filemodulesarrayref = installer::converter::convert_stringlist_into_array(\$filemodules, ",");

        for ( my $j = 0; $j <= $#{$filemodulesarrayref}; $j++ )
        {
            my %featurecomponent = ();

            my $onemodule = ${$filemodulesarrayref}[$j];
            $onemodule =~ s/\s*$//;
            $featurecomponent{'Feature'} = $onemodule;
            $featurecomponent{'Component'} = $filecomponent;

            # Attention: Features are renamed, because the maximum length is 38.
            # But in the files collector ($filesref), the original names are saved.

            installer::windows::idtglobal::shorten_feature_gid(\$featurecomponent{'Feature'});

            $oneline = "$featurecomponent{'Feature'}\t$featurecomponent{'Component'}\n";

            # control of uniqueness

            if (! grep {$_ eq $oneline} @{$featurecomponenttableref})
            {
                push(@{$featurecomponenttableref}, $oneline);
            }
        }
    }
}

#################################################################################
# Collecting all pairs of features and components from the registry collector
#################################################################################

sub create_featurecomponent_table_from_registry_collector
{
    my ($featurecomponenttableref, $registryref) = @_;

    for ( my $i = 0; $i <= $#{$registryref}; $i++ )
    {
        my $oneregistry = ${$registryref}[$i];

        my $registrycomponent = $oneregistry->{'componentname'};
        my $registrymodule = $oneregistry->{'ModuleID'};

        if ( $registrycomponent eq "" )
        {
            installer::exiter::exit_program("ERROR: No component defined for registry $oneregistry->{'gid'}", "create_featurecomponent_table_from_registry_collector");
        }
        if ( $registrymodule eq "" )
        {
            installer::exiter::exit_program("ERROR: No modules found for registry $oneregistry->{'gid'}", "create_featurecomponent_table_from_registry_collector");
        }

        my %featurecomponent = ();

        $featurecomponent{'Feature'} = $registrymodule;
        $featurecomponent{'Component'} = $registrycomponent;

        # Attention: Features are renamed, because the maximum length is 38.
        # But in the files collector ($filesref), the original names are saved.

        installer::windows::idtglobal::shorten_feature_gid(\$featurecomponent{'Feature'});

        $oneline = "$featurecomponent{'Feature'}\t$featurecomponent{'Component'}\n";

        # control of uniqueness

        if (! grep {$_ eq $oneline} @{$featurecomponenttableref})
        {
            push(@{$featurecomponenttableref}, $oneline);
        }
    }
}

#################################################################################
# Creating the file FeatureC.idt dynamically
# Content:
# Feature Component
#################################################################################

sub create_featurecomponent_table
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

    create_featurecomponent_table_from_files_collector(\@featurecomponenttable, $filesref);

    create_featurecomponent_table_from_registry_collector(\@featurecomponenttable, $registryref);

    # Additional components have to be added here

    # Saving the file

    my $featurecomponenttablename = $basedir . $installer::globals::separator . "FeatureC.idt";
    installer::files::save_file($featurecomponenttablename ,\@featurecomponenttable);
    $infoline = "Created idt file: $featurecomponenttablename\n";
    push(@installer::globals::logfileinfo, $infoline);

}

1;
