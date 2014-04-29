#!/usr/bin/perl -w

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

use Getopt::Long;
use Pod::Usage;
use File::Path;
use File::Spec;
use File::Basename;
use XML::LibXML;
use Digest;
use Archive::Zip;
use Archive::Extract;

use installer::ziplist;
use installer::logger;
use installer::windows::msiglobal;
use installer::patch::Msi;
use installer::patch::ReleasesList;
use installer::patch::Version;

#use Carp::Always;

use strict;


=head1 NAME

    patch_tool.pl - Create Windows MSI patches.

=head1 SYNOPSIS

    patch_tool.pl command [options]

    Commands:
        create    create patches
        apply     apply patches

    Options:
        -p|--product-name <product-name>
             The product name, eg Apache_OpenOffice
        -o|--output-path <path>
             Path to the instsetoo_native platform output tree
        -d|--data-path <path>
             Path to the data directory that is expected to be under version control.
        --source-version <major>.<minor>.<micro>
             The version that is to be patched.
        --target-version <major>.<minor>.<micro>
             The version after the patch has been applied.
        --language <language-code>
             Language of the installation sets.
        --package-format
             Only the package format 'msi' is supported at the moment.

=head1 DESCRIPTION

    Creates windows MSP patch files, one for each relevant language.
    Patches convert an installed OpenOffice to the target version.

    Required data are:
        Installation sets of the source versions
            Taken from ext_sources/
            Downloaded from archive.apache.org on demand

        Installation set of the target version
            This is expected to be the current version.

=cut

# The ImageFamily name has to have 1-8 alphanumeric characters.
my $ImageFamily = "AOO";
my $SourceImageName = "Source";
my $TargetImageName = "Target";



sub ProcessCommandline ()
{
    my $context = {
        'product-name' => undef,
        'output-path' => undef,
        'data-path' => undef,
        'lst-file' => undef,
        'source-version' => undef,
        'target-version' => undef,
        'language' => undef,
        'package-format' => undef
    };

    if ( ! GetOptions(
               "product-name=s", \$context->{'product-name'},
               "output-path=s", \$context->{'output-path'},
               "data-path=s" => \$context->{'data-path'},
               "lst-file=s" => \$context->{'lst-file'},
               "source-version:s" => \$context->{'source-version'},
               "target-version:s" => \$context->{'target-version'},
               "language=s" => \$context->{'language'},
               "package-format=s" => \$context->{'package-format'}
        ))
    {
        pod2usage(2);
    }

    # Only the command should be left in @ARGV.
    pod2usage(2) unless scalar @ARGV == 1;
    $context->{'command'} = shift @ARGV;

    return $context;
}




sub GetSourceMsiPath ($$)
{
    my ($context, $language) = @_;
    my $unpacked_path = File::Spec->catfile(
    $context->{'output-path'},
    $context->{'product-name'},
        $context->{'package-format'},
    installer::patch::Version::ArrayToDirectoryName(
        installer::patch::Version::StringToNumberArray(
        $context->{'source-version'})),
    $language);
}




sub GetTargetMsiPath ($$)
{
    my ($context, $language) = @_;
    return File::Spec->catfile(
        $context->{'output-path'},
        $context->{'product-name'},
        $context->{'package-format'},
        "install",
        $language);
}



sub ProvideInstallationSets ($$)
{
    my ($context, $language) = @_;

    # Assume that the target installation set is located in the output tree.
    my $target_path = GetTargetMsiPath($context, $language);
    if ( ! -d $target_path)
    {
        installer::logger::PrintError("can not find target installation set at '%s'\n", $target_path);
        return 0;
    }
    my @target_version = installer::patch::Version::StringToNumberArray($context->{'target-version'});
    my $target_msi_file = File::Spec->catfile(
        $target_path,
        sprintf("openoffice%d%d%d.msi", $target_version[0], $target_version[1], $target_version[2]));
    if ( ! -f $target_msi_file)
    {
        installer::logger::PrintError("can not find target msi file at '%s'\n", $target_msi_file);
        return 0;
    }

    return 1;
}




sub IsLanguageValid ($$$)
{
    my ($context, $release_data, $language) = @_;

    my $normalized_language = installer::languages::get_normalized_language($language);

    if ( ! ProvideInstallationSets($context, $language))
    {
        installer::logger::PrintError("    '%s' has no target installation set\n", $language);
        return 0;
    }
    elsif ( ! defined $release_data->{$normalized_language})
    {
        installer::logger::PrintError("    '%s' is not a released language for version %s\n",
            $language,
            $context->{'source-version'});
        return 0;
    }
    else
    {
        return 1;
    }
}




sub ProvideSourceInstallationSet ($$$)
{
    my ($context, $language, $release_data) = @_;

    my $url = $release_data->{$language}->{'URL'};
    $url =~ /^(.*)\/([^\/]*)$/;
    my ($location, $basename) = ($1,$2);

    my $ext_sources_path = $ENV{'TARFILE_LOCATION'};
    if ( ! -d $ext_sources_path)
    {
        installer::logger::PrintError("Can not determine the path to ext_sources/.\n");
        installer::logger::PrintError("Maybe SOURCE_ROOT_DIR has not been correctly set in the environment?");
        return 0;
    }

    # We need the unpacked installation set in <platform>/<product>/<package>/<source-version>,
    # eg wntmsci12.pro/Apache_OpenOffice/msi/v-4-0-0.
    my $unpacked_path = GetSourceMsiPath($context, $language);
    if ( ! -d $unpacked_path)
    {
        # Make sure that the downloadable installation set (.exe) is present in ext_sources/.
        my $filename = File::Spec->catfile($ext_sources_path, $basename);
        if ( -f $filename)
        {
            PrintInfo("%s is already present in ext_sources/.  Nothing to do\n", $basename);
        }
        else
        {
            return 0 if ! installer::patch::InstallationSet::Download(
                $language,
                $release_data,
                $filename);
            return 0 if ! -f $filename;
        }

        # Unpack the installation set.
        if ( -d $unpacked_path)
        {
            # Take the existence of the destination path as proof that the
            # installation set was successfully unpacked before.
        }
        else
        {
            installer::patch::InstallationSet::Unpack($filename, $unpacked_path);
        }
    }
}




# Find the source and target version between which the patch will be
# created.  Typically the target version is the current version and
# the source version is the version of the previous release.
sub DetermineVersions ($$)
{
    my ($context, $variables) = @_;

    if (defined $context->{'source-version'} && defined $context->{'target-version'})
    {
        # Both source and target version have been specified on the
        # command line.  There remains nothing to be be done.
        return;
    }

    if ( ! defined $context->{'target-version'})
    {
        # Use the current version as target version.
        $context->{'target-version'} = $variables->{PRODUCTVERSION};
    }

    my @target_version = installer::patch::Version::StringToNumberArray($context->{'target-version'});
    shift @target_version;
    my $is_target_version_major = 1;
    foreach my $number (@target_version)
    {
        $is_target_version_major = 0 if ($number ne "0");
    }
    if ($is_target_version_major)
    {
        installer::logger::PrintError("can not create patch where target version is a new major version (%s)\n",
            $context->{'target-version'});
        die;
    }

    if ( ! defined $context->{'source-version'})
    {
        my $releases = installer::patch::ReleasesList::Instance();

        # Search for target release in the list of previous releases.
        # If it is found, use the previous version as source version.
        # Otherwise use the last released version.
        my $last_release = undef;
        foreach my $release (@{$releases->{'releases'}})
        {
            last if ($release eq $context->{'target-version'});
            $last_release = $release;
        }
        $context->{'source-version'} = $last_release;
    }

    if (defined $context->{'source-version'})
    {
        $context->{'source-version-dash'} = installer::patch::Version::ArrayToDirectoryName(
            installer::patch::Version::StringToNumberArray(
                $context->{'source-version'}));
    }
    if (defined $context->{'target-version'})
    {
        $context->{'target-version-dash'} = installer::patch::Version::ArrayToDirectoryName(
            installer::patch::Version::StringToNumberArray(
                $context->{'target-version'}));
    }
}




=head2 CheckUpgradeCode($source_msi, $target_msi)

    The 'UpgradeCode' values in the 'Property' table differs from source to target

=cut
sub CheckUpgradeCode($$)
{
    my ($source_msi, $target_msi) = @_;

    my $source_upgrade_code = $source_msi->GetTable("Property")->GetValue("Property", "UpgradeCode", "Value");
    my $target_upgrade_code = $target_msi->GetTable("Property")->GetValue("Property", "UpgradeCode", "Value");

    if ($source_upgrade_code eq $target_upgrade_code)
    {
        $installer::logger::Info->printf("Error: The UpgradeCode properties have to differ but are both '%s'\n",
            $source_upgrade_code);
        return 0;
    }
    else
    {
        $installer::logger::Info->printf("OK: UpgradeCode values are different\n");
        return 1;
    }
}




=head2 CheckProductCode($source_msi, $target_msi)

    The 'ProductCode' values in the 'Property' tables remain the same.

=cut
sub CheckProductCode($$)
{
    my ($source_msi, $target_msi) = @_;

    my $source_product_code = $source_msi->GetTable("Property")->GetValue("Property", "ProductCode", "Value");
    my $target_product_code = $target_msi->GetTable("Property")->GetValue("Property", "ProductCode", "Value");

    if ($source_product_code ne $target_product_code)
    {
        $installer::logger::Info->printf("Error: The ProductCode properties have to remain the same but are\n");
        $installer::logger::Info->printf("       '%s' and '%s'\n",
            $source_product_code,
            $target_product_code);
        return 0;
    }
    else
    {
        $installer::logger::Info->printf("OK: ProductCodes are identical\n");
        return 1;
    }
}




=head2 CheckBuildIdCode($source_msi, $target_msi)

    The 'PRODUCTBUILDID' values in the 'Property' tables (not the AOO build ids) differ and the
    target value is higher than the source value.

=cut
sub CheckBuildIdCode($$)
{
    my ($source_msi, $target_msi) = @_;

    my $source_build_id = $source_msi->GetTable("Property")->GetValue("Property", "PRODUCTBUILDID", "Value");
    my $target_build_id = $target_msi->GetTable("Property")->GetValue("Property", "PRODUCTBUILDID", "Value");

    if ($source_build_id >= $target_build_id)
    {
        $installer::logger::Info->printf(
            "Error: The PRODUCTBUILDID properties have to increase but are '%s' and '%s'\n",
            $source_build_id,
            $target_build_id);
        return 0;
    }
    else
    {
        $installer::logger::Info->printf("OK: source build id is lower than target build id\n");
        return 1;
    }
}




sub CheckProductName ($$)
{
    my ($source_msi, $target_msi) = @_;

    my $source_product_name = $source_msi->GetTable("Property")->GetValue("Property", "DEFINEDPRODUCT", "Value");
    my $target_product_name = $target_msi->GetTable("Property")->GetValue("Property", "DEFINEDPRODUCT", "Value");

    if ($source_product_name ne $target_product_name)
    {
        $installer::logger::Info->printf("Error: product names of are not identical:\n");
        $installer::logger::Info->printf("       %s != %s\n", $source_product_name, $target_product_name);
        return 0;
    }
    else
    {
        $installer::logger::Info->printf("OK: product names are identical\n");
        return 1;
    }
}




=head2 CheckRemovedFiles($source_msi, $target_msi)

    Files and components must not be deleted.

=cut
sub CheckRemovedFiles($$)
{
    my ($source_msi, $target_msi) = @_;

    # Get the 'File' tables.
    my $source_file_table = $source_msi->GetTable("File");
    my $target_file_table = $target_msi->GetTable("File");

    # Create data structures for fast lookup.
    my @source_files = map {$_->GetValue("File")} @{$source_file_table->GetAllRows()};
    my %target_file_map = map {$_->GetValue("File") => $_} @{$target_file_table->GetAllRows()};

    # Search for removed files (files in source that are missing from target).
    my $removed_file_count = 0;
    foreach my $uniquename (@source_files)
    {
        if ( ! defined $target_file_map{$uniquename})
        {
            ++$removed_file_count;
        }
    }

    if ($removed_file_count > 0)
    {
        $installer::logger::Info->printf("Error: %d files have been removed\n", $removed_file_count);
        return 0;
    }
    else
    {
        $installer::logger::Info->printf("OK: no files have been removed\n");
        return 1;
    }
}




=head2 CheckNewFiles($source_msi, $target_msi)

    New files have to be in new components.

=cut
sub CheckNewFiles($$)
{
    my ($source_msi, $target_msi) = @_;

    # Get the 'File' tables.
    my $source_file_table = $source_msi->GetTable("File");
    my $target_file_table = $target_msi->GetTable("File");

    # Create data structures for fast lookup.
    my %source_file_map = map {$_->GetValue("File") => $_} @{$source_file_table->GetAllRows()};
    my %target_files_map = map {$_->GetValue("File") => $_} @{$target_file_table->GetAllRows()};

    # Search for added files (files in target that where not in source).
    my @added_files = ();
    foreach my $uniquename (keys %target_files_map)
    {
        if ( ! defined $source_file_map{$uniquename})
        {
            push @added_files, $target_files_map{$uniquename};
        }
    }

    if (scalar @added_files > 0)
    {
        $installer::logger::Info->printf("Warning: %d files have been added\n", scalar @added_files);

        # Prepare component tables and hashes.
        my $source_component_table = $source_msi->GetTable("Component");
        my $target_component_table = $target_msi->GetTable("Component");
        die unless defined $source_component_table && defined $target_component_table;
        my %source_component_map = map {$_->GetValue('Component') => $_} @{$source_component_table->GetAllRows()};
        my %target_component_map = map {$_->GetValue('Component') => $_} @{$target_component_table->GetAllRows()};

        my @new_files_with_existing_components = ();
        foreach my $target_file_row (@added_files)
        {
        $installer::logger::Info->printf("    %s (%s)\n",
        $target_file_row->GetValue("FileName"),
        $target_file_row->GetValue("File"));

            # Get target component for target file.
            my $target_component = $target_file_row->GetValue('Component_');

            # Check that the component is not part of the source components.
            if (defined $source_component_map{$target_component})
            {
                push @new_files_with_existing_components, $target_file_row;
            }
        }

        if (scalar @new_files_with_existing_components > 0)
        {
            $installer::logger::Info->printf(
                "Error: %d new files have existing components (which must also be new)\n",
                scalar @new_files_with_existing_components);
            return 0;
        }
        else
        {
            $installer::logger::Info->printf(
                "OK: all %d new files also have new components\n",
        scalar @added_files);
            return 1;
        }
    }
    else
    {
        $installer::logger::Info->printf("OK: no files have been added\n");
        return 1;
    }
}




=head2 CheckFeatureSets($source_msi, $target_msi)

    Features must not be removed but can be added.
    Parent features of new features also have to be new.

=cut
sub CheckFeatureSets($$)
{
    my ($source_msi, $target_msi) = @_;

    # Get the 'Feature' tables.
    my $source_feature_table = $source_msi->GetTable("Feature");
    my $target_feature_table = $target_msi->GetTable("Feature");

    # Create data structures for fast lookup.
    my %source_feature_map = map {$_->GetValue("Feature") => $_} @{$source_feature_table->GetAllRows()};
    my %target_feature_map = map {$_->GetValue("Feature") => $_} @{$target_feature_table->GetAllRows()};

    # Check that no feature has been removed.
    my @removed_features = ();
    foreach my $feature_name (keys %source_feature_map)
    {
        if ( ! defined $target_feature_map{$feature_name})
        {
            push @removed_features, $feature_name;
        }
    }
    if (scalar @removed_features > 0)
    {
        # There are removed features.
        $installer::logger::Info->printf(
            "Error: %d features have been removed:\n",
            scalar @removed_features);
        $installer::logger::Info->printf("       %s\n", join(", ", @removed_features));
        return 0;
    }

    # Check that added features belong to new parent features.
    my @added_features = ();
    foreach my $feature_name (keys %target_feature_map)
    {
        if ( ! defined $source_feature_map{$feature_name})
        {
            push @added_features, $feature_name;
        }
    }

    if (scalar @added_features > 0)
    {
        $installer::logger::Info->printf("Warning: %d features have been addded\n", scalar @added_features);

        my @new_features_with_existing_parents = ();
        foreach my $new_feature (@added_features)
        {
            my $target_feature = $target_feature_map{$new_feature};
            if (defined $source_feature_map{$target_feature->{'Feature_Parent'}})
            {
                push @new_features_with_existing_parents, $target_feature;
            }
        }

        if (scalar @new_features_with_existing_parents > 0)
        {
            $installer::logger::Info->printf(
                "Error: %d new features have existing parents (which also must be new)\n",
                scalar @new_features_with_existing_parents);
            return 0;
        }
        else
        {
            $installer::logger::Info->printf(
                "OK: parents of all new features are also new\n");
            return 1;
        }
    }

    $installer::logger::Info->printf("OK: feature sets in source and target are compatible\n");
    return 1;
}




=head2 CheckRemovedComponents($source_msi, $target_msi)

    Components must not be removed but can be added.
    Features of added components have also to be new.

=cut
sub CheckRemovedComponents ($$)
{
    my ($source_msi, $target_msi) = @_;

    # Get the 'Component' tables.
    my $source_component_table = $source_msi->GetTable("Component");
    my $target_component_table = $target_msi->GetTable("Component");

    # Create data structures for fast lookup.
    my %source_component_map = map {$_->GetValue("Component") => $_} @{$source_component_table->GetAllRows()};
    my %target_component_map = map {$_->GetValue("Component") => $_} @{$target_component_table->GetAllRows()};

    # Check that no component has been removed.
    my @removed_components = ();
    foreach my $componentname (keys %source_component_map)
    {
        if ( ! defined $target_component_map{$componentname})
        {
            push @removed_components, $componentname;
        }
    }
    if (scalar @removed_components == 0)
    {
    $installer::logger::Info->printf("OK: no removed components\n");
    return 1;
    }
    else
    {
        # There are removed components.

        # Check if any of them is not a registry component.
        my $is_file_component_removed = 0;
        foreach my $componentname (@removed_components)
        {
            if ($componentname !~ /^registry/)
            {
                $is_file_component_removed = 1;
            }
        }
        if ($is_file_component_removed)
        {
            $installer::logger::Info->printf(
                "Error: %d components have been removed, some of them are file components:\n",
                scalar @removed_components);
            $installer::logger::Info->printf("       %s\n", join(", ", @removed_components));
            return 0;
        }
        else
        {
            $installer::logger::Info->printf(
                "Error: %d components have been removed, all of them are registry components:\n",
                scalar @removed_components);
            return 0;
        }
    }
}




sub GetTableAndMap ($$$)
{
    my ($msi, $table_name, $index_column) = @_;

    my $table = $msi->GetTable($table_name);
    my %map = map {$_->GetValue($index_column) => $_} @{$table->GetAllRows()};

    return ($table, \%map);
}


=head2 CheckAddedComponents($source_msi, $target_msi)

    Components can be added.
    Features of added components have also to be new.

=cut
sub CheckAddedComponents ($$)
{
    my ($source_msi, $target_msi) = @_;

    # Get the 'Component' tables and maps.
    my ($source_component_table, $source_component_map)
    = GetTableAndMap($source_msi, "Component", "Component");
    my ($target_component_table, $target_component_map)
    = GetTableAndMap($target_msi, "Component", "Component");

    # Check that added components belong to new features.
    my @added_components = ();
    foreach my $componentname (keys %$target_component_map)
    {
        if ( ! defined $source_component_map->{$componentname})
        {
            push @added_components, $componentname;
        }
    }

    if (scalar @added_components == 0)
    {
    $installer::logger::Info->printf("OK: no new components\n");
    return 1;
    }
    else
    {
    $installer::logger::Info->printf(
        "Warning: %d components have been addded\n",
        scalar @added_components);

        # Check that the referencing features are also new.
    my $target_feature_component_table = $target_msi->GetTable("FeatureComponents");

    my $error = 0;
        foreach my $component_name (@added_components)
        {
        my @feature_names = ();
        foreach my $feature_component_row (@{$target_feature_component_table->GetAllRows()})
        {
        if ($feature_component_row->GetValue("Component_") eq $component_name)
        {
            my $feature_name = $feature_component_row->GetValue("Feature_");
            push @feature_names, $feature_name;
        }
        }
        if (scalar @feature_names == 0)
        {
        $installer::logger::Info->printf("Error: no feature found for component '%s'\n", $component_name);
        $error = 1;
        }
        else
        {
        # Check that the referenced features are new and have new parents (if they have parents).
        my ($source_feature_table, $source_feature_map)
            = GetTableAndMap($source_msi, "Feature", "Feature");
        my ($target_feature_table, $target_feature_map)
            = GetTableAndMap($target_msi, "Feature", "Feature");
        foreach my $feature_name (@feature_names)
        {
            $installer::logger::Info->printf("    component '%s' -> feature '%s'\n",
            $component_name,
            $feature_name);
            my $source_feature_row = $source_feature_map->{$feature_name};
            if (defined $source_feature_row)
            {
            $installer::logger::Info->printf("Warning(Error?): feature of new component is not new\n");
            $error = 1;
            }
            else
            {
            # Feature is new. Check that the parent feature is also new.
            my $target_feature_row = $target_feature_map->{$feature_name};
            my $parent_feature_name = $target_feature_row->GetValue("Feature_Parent");
            if ($parent_feature_name ne "" && defined $source_feature_map->{$parent_feature_name})
            {
                $installer::logger::Info->printf("Warning(Error?): parent feature of new component is not new\n");
                $error = 1;
            }
            }
        }
        }
    }

#   return !$error;
    return 1;
    }
}




=head2 CheckComponent($source_msi, $target_msi)

    In the 'Component' table the 'ComponentId' and 'Component' values
    for corresponding componts in the source and target release have
    to be identical.

=cut
sub CheckComponentValues($$$)
{
    my ($source_msi, $target_msi, $variables) = @_;

    # Get the 'Component' tables.
    my $source_component_table = $source_msi->GetTable("Component");
    my $target_component_table = $target_msi->GetTable("Component");

    # Create data structures for fast lookup.
    my %source_component_map = map {$_->GetValue("Component") => $_} @{$source_component_table->GetAllRows()};
    my %target_component_map = map {$_->GetValue("Component") => $_} @{$target_component_table->GetAllRows()};

    my @differences = ();
    my $comparison_count = 0;
    while (my ($componentname, $source_component_row) = each %source_component_map)
    {
        my $target_component_row = $target_component_map{$componentname};
        if (defined $target_component_row)
        {
            ++$comparison_count;
            if ($source_component_row->GetValue("ComponentId") ne $target_component_row->GetValue("ComponentId"))
            {
                push @differences, [
                    $componentname,
                    $source_component_row->GetValue("ComponentId"),
                    $target_component_row->GetValue("ComponentId"),
                    $target_component_row->GetValue("Component"),
                ];
            }
        }
    }

    if (scalar @differences > 0)
    {
        $installer::logger::Info->printf(
            "Error: there are %d components with different 'ComponentId' values after %d comparisons.\n",
            scalar @differences,
            $comparison_count);
        foreach my $item (@differences)
        {
            $installer::logger::Info->printf("%s  %s\n", $item->[1], $item->[2]);
        }
        return 0;
    }
    else
    {
        $installer::logger::Info->printf("OK: components in source and target are identical\n");
        return 1;
    }
}




=head2 CheckFileSequence($source_msi, $target_msi)

    In the 'File' table the 'Sequence' numbers for corresponding files has to be identical.

=cut
sub CheckFileSequence($$)
{
    my ($source_msi, $target_msi) = @_;

    # Get the 'File' tables.
    my $source_file_table = $source_msi->GetTable("File");
    my $target_file_table = $target_msi->GetTable("File");

    # Create temporary data structures for fast access.
    my %source_file_map = map {$_->GetValue("File") => $_} @{$source_file_table->GetAllRows()};
    my %target_file_map = map {$_->GetValue("File") => $_} @{$target_file_table->GetAllRows()};

    # Search files with mismatching sequence numbers.
    my @mismatching_files = ();
    while (my ($uniquename,$source_file_row) = each %source_file_map)
    {
        my $target_file_row = $target_file_map{$uniquename};
        if (defined $target_file_row)
        {
            if ($source_file_row->GetValue('Sequence') ne $target_file_row->GetValue('Sequence'))
            {
                push @mismatching_files, [
                    $uniquename,
                    $source_file_row,
                    $target_file_row
                ];
            }
        }
    }

    if (scalar @mismatching_files > 0)
    {
        $installer::logger::Info->printf("Error: there are %d files with mismatching 'Sequence' numbers\n",
            scalar @mismatching_files);
        foreach my $item (@mismatching_files)
        {
            $installer::logger::Info->printf("    %s: %d != %d\n",
                $item->[0],
                $item->[1]->GetValue("Sequence"),
                $item->[2]->GetValue("Sequence"));
        }
        return 0;
    }
    else
    {
        $installer::logger::Info->printf("OK: all files have matching 'Sequence' numbers\n");
        return 1;
    }
}




=head2 CheckFileSequenceUnique($source_msi, $target_msi)

    In the 'File' table the 'Sequence' values have to be unique.

=cut
sub CheckFileSequenceUnique($$)
{
    my ($source_msi, $target_msi) = @_;

    # Get the 'File' tables.
    my $target_file_table = $target_msi->GetTable("File");

    my %sequence_numbers = ();
    my $collision_count = 0;
    foreach my $row (@{$target_file_table->GetAllRows()})
    {
        my $sequence_number = $row->GetValue("Sequence");
        if (defined $sequence_numbers{$sequence_number})
        {
            ++$collision_count;
        }
        else
        {
            $sequence_numbers{$sequence_number} = 1;
        }
    }

    if ($collision_count > 0)
    {
        $installer::logger::Info->printf("Error: there are %d collisions ofn the sequence numbers\n",
            $collision_count);
        return 0;
    }
    else
    {
        $installer::logger::Info->printf("OK: sequence numbers are unique\n");
        return 1;
    }
}




=head2 CheckFileSequenceHoles ($target_msi)

    Check the sequence numbers of the target msi if the n files use numbers 1..n or if there are holes.
    Holes are reported as warnings.

=cut
sub CheckFileSequenceHoles ($$)
{
    my ($source_msi, $target_msi) = @_;

    my $target_file_table = $target_msi->GetTable("File");
    my %sequence_numbers = map {$_->GetValue("Sequence") => $_} @{$target_file_table->GetAllRows()};
    my @sorted_sequence_numbers = sort {$a <=> $b} keys %sequence_numbers;
    my $expected_next_sequence_number = 1;
    my @holes = ();
    foreach my $sequence_number (@sorted_sequence_numbers)
    {
        if ($sequence_number != $expected_next_sequence_number)
        {
            push @holes, [$expected_next_sequence_number, $sequence_number-1];
        }
        $expected_next_sequence_number = $sequence_number+1;
    }
    if (scalar @holes > 0)
    {
        $installer::logger::Info->printf("Warning: sequence numbers have %d holes\n");
        foreach my $hole (@holes)
        {
            if ($hole->[0] != $hole->[1])
            {
                $installer::logger::Info->printf("    %d\n", $hole->[0]);
            }
            else
            {
                $installer::logger::Info->printf("    %d -> %d\n", $hole->[0], $hole->[1]);
            }
        }
    }
    else
    {
        $installer::logger::Info->printf("OK: there are no holes in the sequence numbers\n");
    }
    return 1;
}




=head2 CheckRegistryItems($source_msi, $target_msi)

    In the 'Registry' table the 'Component_' and 'Key' values must not
    depend on the version number (beyond the unchanging major
    version).

    'Value' values must only depend on the major version number to
    avoid duplicate entries in the start menu.

    Violations are reported as warnings for now.

=cut
sub CheckRegistryItems($$$)
{
    my ($source_msi, $target_msi, $product_name) = @_;

    # Get the registry tables.
    my $source_registry_table = $source_msi->GetTable("Registry");
    my $target_registry_table = $target_msi->GetTable("Registry");

    my $registry_index = $target_registry_table->GetColumnIndex("Registry");
    my $component_index = $target_registry_table->GetColumnIndex("Component_");

    # Create temporary data structures for fast access.
    my %source_registry_map = map {$_->GetValue($registry_index) => $_} @{$source_registry_table->GetAllRows()};
    my %target_registry_map = map {$_->GetValue($registry_index) => $_} @{$target_registry_table->GetAllRows()};

    # Prepare version numbers to search.
    my $source_version_number = $source_msi->{'version'};
    my $source_version_nodots = installer::patch::Version::ArrayToNoDotName(
        installer::patch::Version::StringToNumberArray($source_version_number));
    my $source_component_pattern = lc($product_name).$source_version_nodots;
    my $target_version_number = $target_msi->{'version'};
    my $target_version_nodots = installer::patch::Version::ArrayToNoDotName(
        installer::patch::Version::StringToNumberArray($target_version_number));
    my $target_component_pattern = lc($product_name).$target_version_nodots;

    foreach my $source_row (values %source_registry_map)
    {
        my $target_row = $target_registry_map{$source_row->GetValue($registry_index)};
        if ( ! defined $target_row)
        {
            $installer::logger::Info->printf("Error: sets of registry entries differs\n");
            return 1;
        }

        my $source_component_name = $source_row->GetValue($component_index);
        my $target_component_name = $source_row->GetValue($component_index);

    }

    $installer::logger::Info->printf("OK: registry items are OK\n");
    return 1;
}




=head2

    Component->KeyPath must not change. (see component.pm/get_component_keypath)

=cut
sub CheckComponentKeyPath ($$)
{
    my ($source_msi, $target_msi) = @_;

    # Get the registry tables.
    my $source_component_table = $source_msi->GetTable("Component");
    my $target_component_table = $target_msi->GetTable("Component");

    # Create temporary data structures for fast access.
    my %source_component_map = map {$_->GetValue("Component") => $_} @{$source_component_table->GetAllRows()};
    my %target_component_map = map {$_->GetValue("Component") => $_} @{$target_component_table->GetAllRows()};

    my @mismatches = ();
    while (my ($componentname, $source_component_row) = each %source_component_map)
    {
        my $target_component_row = $target_component_map{$componentname};
        if (defined $target_component_row)
        {
            my $source_keypath = $source_component_row->GetValue("KeyPath");
            my $target_keypath = $target_component_row->GetValue("KeyPath");
            if ($source_keypath ne $target_keypath)
            {
                push @mismatches, [$componentname, $source_keypath, $target_keypath];
            }
        }
    }

    if (scalar @mismatches > 0)
    {
        $installer::logger::Info->printf(
            "Error: there are %d mismatches in the 'KeyPath' column of the 'Component' table\n",
            scalar @mismatches);

        foreach my $item (@mismatches)
        {
            $installer::logger::Info->printf(
                "    %s: %s != %s\n",
                $item->[0],
                $item->[1],
                $item->[2]);
        }

        return 0;
    }
    else
    {
        $installer::logger::Info->printf(
            "OK: no mismatches in the 'KeyPath' column of the 'Component' table\n");
        return 1;
    }
}




sub GetMissingReferences ($$$$$)
{
    my ($table, $key, $map, $what, $report_key) = @_;

    my @missing_references = ();

    foreach my $row (@{$table->GetAllRows()})
    {
        my $value = $row->GetValue($key);
        if ($value ne "" && ! defined $map->{$value})
        {
            push @missing_references, [$what, $row->GetValue($report_key), $value];
        }
    }

    return @missing_references;
}




=head CheckAllReferences ($msi)

    Check references from files and registry entries to components,
    from components to features, and between features.

=cut

sub CheckAllReferences ($)
{
    my ($msi) = @_;

    # Set up tables and maps for easy iteration and fast lookups.

    my $feature_table = $msi->GetTable("Feature");
    my $component_table = $msi->GetTable("Component");
    my $feature_component_table = $msi->GetTable("FeatureComponents");
    my $file_table = $msi->GetTable("File");
    my $registry_table = $msi->GetTable("Registry");
    my $directory_table = $msi->GetTable("Directory");

    my %feature_map = map {$_->GetValue("Feature") => $_} @{$feature_table->GetAllRows()};
    my %component_map = map {$_->GetValue("Component") => $_} @{$component_table->GetAllRows()};
    my %directory_map = map {$_->GetValue("Directory") => $_} @{$directory_table->GetAllRows()};

    my @missing_references = ();

    # Check references from files and registry entries to components.
    push @missing_references, GetMissingReferences(
        $file_table,
        "Component_",
        \%component_map,
        "file->component",
        "File");
    push @missing_references, GetMissingReferences(
        $registry_table,
        "Component_",
        \%component_map,
        "registry->component",
        "Registry");

    # Check references between features and components.
    push @missing_references, GetMissingReferences(
        $feature_component_table,
        "Feature_",
        \%feature_map,
        "component->feature",
        "Component_");
    push @missing_references, GetMissingReferences(
        $feature_component_table,
        "Component_",
        \%component_map,
        "feature->component",
        "Feature_");

    # Check references between features.
    push @missing_references, GetMissingReferences(
        $feature_table,
        'Feature_Parent',
        \%feature_map,
        "feature->feature",
        'Feature');

    # Check references between directories.
    push @missing_references, GetMissingReferences(
        $directory_table,
        'Directory_Parent',
        \%directory_map,
        "directory->directory",
        'Directory');

    # Check references from components to directories.
    push @missing_references, GetMissingReferences(
        $component_table,
        'Directory_',
        \%directory_map,
        "component->directory",
        'Component');

    # Check references from components to files (via the .

    # Report the result.
    if (scalar @missing_references > 0)
    {
        $installer::logger::Info->printf("Error: there are %d missing references\n", scalar @missing_references);
        foreach my $reference (@missing_references)
        {
            $installer::logger::Info->printf("    %s : %s -> %s\n",
                $reference->[0],
                $reference->[1],
                $reference->[2]);
        }
        return 0;
    }
    else
    {
        $installer::logger::Info->printf("OK: all references are OK\n");
        return 1;

    }
}




sub Check ($$$$)
{
    my ($source_msi, $target_msi, $variables, $product_name) = @_;

    $installer::logger::Info->printf("checking if source and target releases are compatible\n");
    $installer::logger::Info->increase_indentation();

    my $result = 1;

    # Using &= below to avoid lazy evaluation.  Even if there are errors, all checks shall be run.
    $result &= CheckUpgradeCode($source_msi, $target_msi);
    $result &= CheckProductCode($source_msi, $target_msi);
    $result &= CheckBuildIdCode($source_msi, $target_msi);
    $result &= CheckProductName($source_msi, $target_msi);
    $result &= CheckRemovedFiles($source_msi, $target_msi);
    $result &= CheckNewFiles($source_msi, $target_msi);
    $result &= CheckFeatureSets($source_msi, $target_msi);
    $result &= CheckRemovedComponents($source_msi, $target_msi);
    $result &= CheckAddedComponents($source_msi, $target_msi);
    $result &= CheckComponentValues($source_msi, $target_msi, $variables);
    $result &= CheckFileSequence($source_msi, $target_msi);
    $result &= CheckFileSequenceUnique($source_msi, $target_msi);
    $result &= CheckFileSequenceHoles($source_msi, $target_msi);
    $result &= CheckRegistryItems($source_msi, $target_msi, $product_name);
    $result &= CheckComponentKeyPath($source_msi, $target_msi);
    $result &= CheckAllReferences($target_msi);

    $installer::logger::Info->decrease_indentation();

    if ($result)
    {
        $installer::logger::Info->printf("OK: Source and target releases are compatible.\n");
    }
    else
    {
        $installer::logger::Info->printf("Error: Source and target releases are not compatible.\n");
        $installer::logger::Info->printf("       => Can not create patch.\n");
        $installer::logger::Info->printf("       Did you create the target installation set with 'release=t' ?\n");
    }

    return $result;
}




=head2 FindPcpTemplate ()

    The template.pcp file is part of the Windows SDK.

=cut
sub FindPcpTemplate ()
{
    my $psdk_home = $ENV{'PSDK_HOME'};
    if ( ! defined $psdk_home)
    {
        $installer::logger::Info->printf("Error: the PSDK_HOME environment variable is not set.\n");
        $installer::logger::Info->printf("       did you load the AOO build environment?\n");
        $installer::logger::Info->printf("       you may want to use the --with-psdk-home configure option\n");
        return undef;
    }
    if ( ! -d $psdk_home)
    {
        $installer::logger::Info->printf(
            "Error: the PSDK_HOME environment variable does not point to a valid directory: %s\n",
            $psdk_home);
        return undef;
    }

    my $schema_path = File::Spec->catfile($psdk_home, "Bin", "msitools", "Schemas", "MSI");
    if (  ! -d $schema_path)
    {
        $installer::logger::Info->printf("Error: Can not locate the msi template folder in the Windows SDK\n");
        $installer::logger::Info->printf("       %s\n", $schema_path);
        $installer::logger::Info->printf("       Is the Windows SDK properly installed?\n");
        return undef;
    }

    my $schema_filename = File::Spec->catfile($schema_path, "template.pcp");
    if (  ! -f $schema_filename)
    {
        $installer::logger::Info->printf("Error: Can not locate the pcp template at\n");
        $installer::logger::Info->printf("       %s\n", $schema_filename);
        $installer::logger::Info->printf("       Is the Windows SDK properly installed?\n");
        return undef;
    }

    return $schema_filename;
}




sub SetupPcpPatchMetadataTable ($$$)
{
    my ($pcp, $source_msi, $target_msi) = @_;

    # Determine values for eg product name and source and new version.
    my $source_version = $source_msi->{'version'};
    my $target_version = $target_msi->{'version'};

    my $property_table = $target_msi->GetTable("Property");
    my $display_product_name = $property_table->GetValue("Property", "DEFINEDPRODUCT", "Value");

    # Set table.
    my $table = $pcp->GetTable("PatchMetadata");
    $table->SetRow(
        "Company", "",
        "*Property", "Description",
        "Value", sprintf("Update of %s from %s to %s", $display_product_name, $source_version, $target_version)
        );
    $table->SetRow(
        "Company", "",
        "*Property", "DisplayName",
        "Value", sprintf("Update of %s from %s to %s", $display_product_name, $source_version, $target_version)
        );
    $table->SetRow(
        "Company", "",
        "*Property", "ManufacturerName",
        "Value", $property_table->GetValue("Property", "Manufacturer", "Value"),
        );
    $table->SetRow(
        "Company", "",
        "*Property", "MoreInfoURL",
        "Value", $property_table->GetValue("Property", "ARPURLINFOABOUT", "Value")
        );
    $table->SetRow(
        "Company", "",
        "*Property", "TargetProductName",
        "Value", $property_table->GetValue("Property", "ProductName", "Value")
        );
    my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = gmtime(time);

    $table->SetRow(
        "Company", "",
        "*Property", "CreationTimeUTC",
        "Value", sprintf("%d/%d/%d %d:%02d", $mon+1,$mday,$year+1900,$hour,$min)
        );
}




sub SetupPropertiesTable ($$)
{
    my ($pcp, $msp_filename) = @_;

    my $table = $pcp->GetTable("Properties");

    $table->SetRow(
        "*Name", "PatchOutputPath",
        "Value", installer::patch::Tools::ToWindowsPath($msp_filename)
        );
    # Request at least Windows installer 2.0.
    # Version 2.0 allows us to omit some values from ImageFamilies table.
    $table->SetRow(
        "*Name", "MinimumRequiredMsiVersion",
        "Value", 200
        );
    # Allow diffs for binary files.
    $table->SetRow(
        "*Name", "IncludeWholeFilesOnly",
        "Value", 0
        );

    my $uuid = installer::windows::msiglobal::create_guid();
    my $uuid_string = "{" . $uuid . "}";
    $table->SetRow(
        "*Name", "PatchGUID",
        "Value", $uuid_string
        );
    $installer::logger::Info->printf("created new PatchGUID %s\n", $uuid_string);

    # Prevent sequence table from being generated.
    $table->SetRow(
        "*Name", "SEQUENCE_DATA_GENERATION_DISABLED",
        "Value", 1);

    # We don't provide file size and hash values.
    # This value is set to make this fact explicit (0 should be the default).
    $table->SetRow(
        "*Name", "TrustMsi",
        "Value", 0);
}




sub SetupImageFamiliesTable ($)
{
    my ($pcp) = @_;

    $pcp->GetTable("ImageFamilies")->SetRow(
        "Family", $ImageFamily,
        "MediaSrcPropName", "",#"MNPSrcPropName",
        "MediaDiskId", "",
        "FileSequenceStart", "",
        "DiskPrompt", "",
        "VolumeLabel", "");
}




sub SetupUpgradedImagesTable ($$)
{
    my ($pcp, $target_msi_path) = @_;

    my $msi_path = installer::patch::Tools::ToWindowsPath($target_msi_path);
    $pcp->GetTable("UpgradedImages")->SetRow(
        "Upgraded", $TargetImageName,
        "MsiPath", $msi_path,
        "PatchMsiPath", "",
        "SymbolPaths", "",
        "Family", $ImageFamily);
}




sub SetupTargetImagesTable ($$)
{
    my ($pcp, $source_msi_path) = @_;

    $pcp->GetTable("TargetImages")->SetRow(
        "Target", $SourceImageName,
        "MsiPath", installer::patch::Tools::ToWindowsPath($source_msi_path),
        "SymbolPaths", "",
        "Upgraded", $TargetImageName,
        "Order", 1,
        "ProductValidateFlags", "",
        "IgnoreMissingSrcFiles", 0);
}




sub SetAdditionalValues ($%)
{
    my ($pcp, %data) = @_;

    while (my ($key,$value) = each(%data))
    {
        $key =~ /^([^\/]+)\/([^:]+):(.+)$/
            || die("invalid key format");
        my ($table_name, $key_column,$key_value) = ($1,$2,$3);
        $value =~ /^([^:]+):(.*)$/
            || die("invalid value format");
        my ($value_column,$value_value) = ($1,$2);

        my $table = $pcp->GetTable($table_name);
        $table->SetRow(
                "*".$key_column, $key_value,
                $value_column, $value_value);
    }
}




sub CreatePcp ($$$$$$%)
{
    my ($source_msi,
        $target_msi,
        $language,
        $context,
        $msp_path,
        $pcp_schema_filename,
        %additional_values) = @_;

    # Create filenames.
    my $pcp_filename = File::Spec->catfile($msp_path, "openoffice.pcp");
    # Create basename to include product name and source and target version.
    # Hard code platform because that is the only platform supported at the moment.
    my $msp_basename = sprintf("%s_%s-%s_Win_x86_patch_%s.msp",
        $context->{'product-name'},
        $source_msi->{'version'},
        $target_msi->{'version'},
        $context->{'language'});
    my $msp_filename = File::Spec->catfile($msp_path, $msp_basename);

    # Setup msp path and filename.
    unlink($pcp_filename) if -f $pcp_filename;
    if ( ! File::Copy::copy($pcp_schema_filename, $pcp_filename))
    {
        $installer::logger::Info->printf("Error: could not create openoffice.pcp as copy of pcp schema\n");
        $installer::logger::Info->printf("       %s\n", $pcp_schema_filename);
        $installer::logger::Info->printf("       %s\n", $pcp_filename);
        return undef;
    }
    my $pcp = installer::patch::Msi->new(
        $pcp_filename,
        $target_msi->{'version'},
        $target_msi->{'is_current_version'},
        $language,
        $context->{'product-name'});

    # Store some values in the pcp for easy reference in the msp creation.
    $pcp->{'msp_filename'} = $msp_filename;

    SetupPcpPatchMetadataTable($pcp, $source_msi, $target_msi);
    SetupPropertiesTable($pcp, $msp_filename);
    SetupImageFamiliesTable($pcp);
    SetupUpgradedImagesTable($pcp, $target_msi->{'filename'});
    SetupTargetImagesTable($pcp, $source_msi->{'filename'});

    SetAdditionalValues(%additional_values);

    $pcp->Commit();

    # Remove the PatchSequence table to avoid MsiMsp error message:
    # "Since MSI 3.0 will block installation of major upgrade patches with
    #  sequencing information, creation of such patches is blocked."
    #$pcp->RemoveTable("PatchSequence");
    # TODO: alternatively add property SEQUENCE_DATA_GENERATION_DISABLED to pcp Properties table.


    $installer::logger::Info->printf("created pcp file at\n");
    $installer::logger::Info->printf("    %s\n", $pcp->{'filename'});

    return $pcp;
}




sub ShowLog ($$$$)
{
    my ($log_path, $log_filename, $log_basename, $new_title) = @_;

    if ( -f $log_filename)
    {
        my $destination_path = File::Spec->catfile($log_path, $log_basename);
        File::Path::make_path($destination_path) if ! -d $destination_path;
        my $command = join(" ",
            "wilogutl.exe",
            "/q",
            "/l", "'".installer::patch::Tools::ToWindowsPath($log_filename)."'",
            "/o", "'".installer::patch::Tools::ToWindowsPath($destination_path)."'");
        printf("running command $command\n");
        my $response = qx($command);
        my @candidates = glob($destination_path . "/Details*");
        foreach my $candidate (@candidates)
        {
            next unless -f $candidate;
            my $new_name = $candidate;
            $new_name =~ s/Details.*$/$log_basename.html/;

            # Rename the top-level html file and replace the title.
            open my $in, "<", $candidate;
            open my $out, ">", $new_name;
            while (<$in>)
            {
                if (/^(.*\<title\>)([^<]+)(.*)$/)
                {
                    print $out $1.$new_title.$3;
                }
                else
                {
                    print $out $_;
                }
            }
            close $in;
            close $out;

            my $URL = File::Spec->rel2abs($new_name);
            $URL =~ s/\/cygdrive\/(.)\//$1|\//;
            $URL =~ s/^(.):/$1|/;
            $URL = "file:///". $URL;
            $installer::logger::Info->printf("open %s in your browser to see the log messages\n", $URL);
        }
    }
    else
    {
        $installer::logger::Info->printf("Error: log file not found at %s\n", $log_filename);
    }
}




sub CreateMsp ($)
{
    my ($pcp) = @_;

    # Prepare log files.
    my $log_path = File::Spec->catfile($pcp->{'path'}, "log");
    my $log_basename = "msp";
    my $log_filename = File::Spec->catfile($log_path, $log_basename.".log");
    my $performance_log_basename = "performance";
    my $performance_log_filename = File::Spec->catfile($log_path, $performance_log_basename.".log");
    File::Path::make_path($log_path) if ! -d $log_path;
    unlink($log_filename) if -f $log_filename;
    unlink($performance_log_filename) if -f $performance_log_filename;

    # Create the .msp patch file.
    my $temporary_msimsp_path = File::Spec->catfile($pcp->{'path'}, "tmp");
    if ( ! -d $temporary_msimsp_path)
    {
        File::Path::make_path($temporary_msimsp_path)
            || die ("can not create temporary path ".$temporary_msimsp_path);
    }
    $installer::logger::Info->printf("running msimsp.exe, that will take a while\n");
    my $create_performance_log = 0;
    my $command = join(" ",
        "msimsp.exe",
        "-s", "'".installer::patch::Tools::ToWindowsPath($pcp->{'filename'})."'",
        "-p", "'".installer::patch::Tools::ToWindowsPath($pcp->{'msp_filename'})."'",
        "-l", "'".installer::patch::Tools::ToWindowsPath($log_filename)."'",
        "-f", "'".installer::patch::Tools::ToWindowsPath($temporary_msimsp_path)."'");
    if ($create_performance_log)
    {
        $command .= " -lp " . MsiTools::ToEscapedWindowsPath($performance_log_filename);
    }
    $installer::logger::Info->printf("running command %s\n", $command);
    my $response = qx($command);
    $installer::logger::Info->printf("response of msimsp is %s\n", $response);
    if ( ! -d $temporary_msimsp_path)
    {
        die("msimsp failed and deleted temporary path ".$temporary_msimsp_path);
    }

    # Show the log file that was created by the msimsp.exe command.
    ShowLog($log_path, $log_filename, $log_basename, "msp creation");
    if ($create_performance_log)
    {
        ShowLog($log_path, $performance_log_filename, $performance_log_basename, "msp creation perf");
    }
}


sub ProvideMsis ($$$)
{
    my ($context, $variables, $language) = @_;

    # 2a. Provide .msi and .cab files and unpack .cab for the source release.
    $installer::logger::Info->printf("locating source package (%s)\n", $context->{'source-version'});
    $installer::logger::Info->increase_indentation();
    if ( ! installer::patch::InstallationSet::ProvideUnpackedCab(
           $context->{'source-version'},
           0,
           $language,
           "msi",
           $context->{'product-name'}))
    {
        die "could not provide unpacked .cab file";
    }
    my $source_msi = installer::patch::Msi->FindAndCreate(
        $context->{'source-version'},
        0,
        $language,
        $context->{'product-name'});
    die unless defined $source_msi;
    die unless $source_msi->IsValid();
    $installer::logger::Info->decrease_indentation();

    # 2b. Provide .msi and .cab files and unpacked .cab for the target release.
    $installer::logger::Info->printf("locating target package (%s)\n", $context->{'target-version'});
    $installer::logger::Info->increase_indentation();
    if ( ! installer::patch::InstallationSet::ProvideUnpackedCab(
               $context->{'target-version'},
               1,
               $language,
               "msi",
               $context->{'product-name'}))
    {
        die;
    }
    my $target_msi = installer::patch::Msi->FindAndCreate(
        $context->{'target-version'},
        0,
        $language,
        $context->{'product-name'});
    die unless defined $target_msi;
    die unless $target_msi->IsValid();
    $installer::logger::Info->decrease_indentation();

    return ($source_msi, $target_msi);
}




=head CreatePatch($context, $variables)

    Create MSP patch files for all relevant languages.
    The different steps are:
    1. Determine the set of languages for which both the source and target installation sets are present.
    Per language:
        2. Unpack CAB files (for source and target).
        3. Check if source and target releases are compatible.
        4. Create the PCP driver file.
        5. Create the MSP patch file.

=cut
sub CreatePatch ($$)
{
    my ($context, $variables) = @_;

    $installer::logger::Info->printf("patch will update product %s from %s to %s\n",
        $context->{'product-name'},
        $context->{'source-version'},
        $context->{'target-version'});

    # Locate the Pcp schema file early on to report any errors before the lengthy operations that follow.
    my $pcp_schema_filename = FindPcpTemplate();
    if ( ! defined $pcp_schema_filename)
    {
        exit(1);
    }

    my $release_data = installer::patch::ReleasesList::Instance()
        ->{$context->{'source-version'}}
        ->{$context->{'package-format'}};

    # 1. Determine the set of languages for which we can create patches.
    my $language = $context->{'language'};
    my %no_ms_lang_locale_map = map {$_=>1} @installer::globals::noMSLocaleLangs;
    if (defined $no_ms_lang_locale_map{$language})
    {
        $language = "en-US_".$language;
    }

    if ( ! IsLanguageValid($context, $release_data, $language))
    {
        $installer::logger::Info->printf("can not create patch for language '%s'\n", $language);
    }
    else
    {
        $installer::logger::Info->printf("processing language '%s'\n", $language);
        $installer::logger::Info->increase_indentation();

        my ($source_msi, $target_msi) = ProvideMsis($context, $variables, $language);

        # Trigger reading of tables.
        foreach my $table_name (("File", "Component", "Registry"))
        {
            $source_msi->GetTable($table_name);
            $target_msi->GetTable($table_name);
            $installer::logger::Info->printf("read %s table (source and target\n", $table_name);
        }

        # 3. Check if the source and target msis fullfil all necessary requirements.
        if ( ! Check($source_msi, $target_msi, $variables, $context->{'product-name'}))
        {
            exit(1);
        }

        # Provide the base path for creating .pcp and .mcp file.
        my $msp_path = File::Spec->catfile(
            $context->{'output-path'},
            $context->{'product-name'},
            "msp",
            sprintf("%s_%s",
                installer::patch::Version::ArrayToDirectoryName(
                    installer::patch::Version::StringToNumberArray(
                        $source_msi->{'version'})),
                installer::patch::Version::ArrayToDirectoryName(
                    installer::patch::Version::StringToNumberArray(
                        $target_msi->{'version'}))),
            $language
            );
        File::Path::make_path($msp_path) unless -d $msp_path;

        # 4. Create the .pcp file that drives the msimsp.exe command.
        my $pcp = CreatePcp(
            $source_msi,
            $target_msi,
            $language,
            $context,
            $msp_path,
            $pcp_schema_filename,
            "Properties/Name:DontRemoveTempFolderWhenFinished" => "Value:1");

        # 5. Finally create the msp.
        CreateMsp($pcp);

        $installer::logger::Info->decrease_indentation();
    }
}




sub CheckPatchCompatability ($$)
{
    my ($context, $variables) = @_;

    $installer::logger::Info->printf("patch will update product %s from %s to %s\n",
        $context->{'product-name'},
        $context->{'source-version'},
        $context->{'target-version'});

    my $release_data = installer::patch::ReleasesList::Instance()
        ->{$context->{'source-version'}}
        ->{$context->{'package-format'}};

    # 1. Determine the set of languages for which we can create patches.
    my $language = $context->{'language'};
    my %no_ms_lang_locale_map = map {$_=>1} @installer::globals::noMSLocaleLangs;
    if (defined $no_ms_lang_locale_map{$language})
    {
        $language = "en-US_".$language;
    }

    if ( ! IsLanguageValid($context, $release_data, $language))
    {
        $installer::logger::Info->printf("can not create patch for language '%s'\n", $language);
    }
    else
    {
        $installer::logger::Info->printf("processing language '%s'\n", $language);
        $installer::logger::Info->increase_indentation();

        my ($source_msi, $target_msi) = ProvideMsis($context, $variables, $language);

        # Trigger reading of tables.
        foreach my $table_name (("File", "Component", "Registry"))
        {
            $source_msi->GetTable($table_name);
            $target_msi->GetTable($table_name);
            $installer::logger::Info->printf("read %s table (source and target\n", $table_name);
        }

        # 3. Check if the source and target msis fullfil all necessary requirements.
        if ( ! Check($source_msi, $target_msi, $variables, $context->{'product-name'}))
        {
            exit(1);
        }
    }
}




=cut ApplyPatch ($context, $variables)

    This is for testing only.
    The patch is applied and (extensive) log information is created and transformed into HTML format.

=cut
sub ApplyPatch ($$)
{
    my ($context, $variables) = @_;

    $installer::logger::Info->printf("will apply patches that update product %s from %s to %s\n",
        $context->{'product-name'},
        $context->{'source-version'},
        $context->{'target-version'});

    my $source_version_dirname = installer::patch::Version::ArrayToDirectoryName(
      installer::patch::Version::StringToNumberArray(
          $context->{'source-version'}));
    my $target_version_dirname = installer::patch::Version::ArrayToDirectoryName(
      installer::patch::Version::StringToNumberArray(
          $context->{'target-version'}));

    my $language = $context->{'language'};
    my %no_ms_lang_locale_map = map {$_=>1} @installer::globals::noMSLocaleLangs;
    if (defined $no_ms_lang_locale_map{$language})
    {
        $language = "en-US_".$language;
    }

    my $msp_filename = File::Spec->catfile(
        $context->{'output-path'},
        $context->{'product-name'},
        "msp",
        $source_version_dirname . "_" . $target_version_dirname,
        $language,
        "openoffice.msp");
    if ( ! -f $msp_filename)
    {
        $installer::logger::Info->printf("%s does not point to a valid file\n", $msp_filename);
        next;
    }

    my $log_path = File::Spec->catfile(dirname($msp_filename), "log");
    my $log_basename = "apply-msp";
    my $log_filename = File::Spec->catfile($log_path, $log_basename.".log");

    my $command = join(" ",
        "msiexec.exe",
        "/update", "'".installer::patch::Tools::ToWindowsPath($msp_filename)."'",
        "/L*xv!", "'".installer::patch::Tools::ToWindowsPath($log_filename)."'",
        "REINSTALL=ALL",
#            "REINSTALLMODE=vomus",
        "REINSTALLMODE=omus",
        "MSIENFORCEUPGRADECOMPONENTRULES=1");

    printf("executing command %s\n", $command);
    my $response = qx($command);
    Encode::from_to($response, "UTF16LE", "UTF8");
    printf("response was '%s'\n", $response);

    ShowLog($log_path, $log_filename, $log_basename, "msp application");
}




=head2 DownloadFile ($url)

    A simpler version of InstallationSet::Download().  It is simple because it is used to
    setup the $release_data structure that is used by InstallationSet::Download().

=cut
sub DownloadFile ($)
{
    my ($url) = shift;

    my $agent = LWP::UserAgent->new();
    $agent->timeout(120);
    $agent->show_progress(0);

    my $file_content = "";
    my $last_was_redirect = 0;
    my $bytes_read = 0;
    $agent->add_handler('response_redirect'
        => sub{
            $last_was_redirect = 1;
            return;
        });
    $agent->add_handler('response_data'
        => sub{
            if ($last_was_redirect)
            {
                $last_was_redirect = 0;
                # Throw away the data we got so far.
        $file_content = "";
            }
            my($response,$agent,$h,$data)=@_;
        $file_content .= $data;
        });
    $agent->get($url);

    return $file_content;
}




sub CreateReleaseItem ($$$)
{
    my ($language, $exe_filename, $msi) = @_;

    die "can not open installation set at ".$exe_filename unless -f $exe_filename;

    open my $in, "<", $exe_filename;
    my $sha256_checksum = new Digest("SHA-256")->addfile($in)->hexdigest();
    close $in;

    my $filesize = -s $exe_filename;

    # Get the product code property from the msi and strip the enclosing braces.
    my $product_code = $msi->GetTable("Property")->GetValue("Property", "ProductCode", "Value");
    $product_code =~ s/(^{|}$)//g;
    my $upgrade_code = $msi->GetTable("Property")->GetValue("Property", "UpgradeCode", "Value");
    $upgrade_code =~ s/(^{|}$)//g;
    my $build_id = $msi->GetTable("Property")->GetValue("Property", "PRODUCTBUILDID", "Value");

    return {
        'language' => $language,
        'checksum-type' => "sha256",
        'checksum-value' => $sha256_checksum,
        'file-size' => $filesize,
        'product-code' => $product_code,
        'upgrade-code' => $upgrade_code,
        'build-id' => $build_id
    };
}




sub GetReleaseItemForCurrentBuild ($$$)
{
    my ($context, $language, $exe_basename) = @_;

    # Target version is the current version.
    # Search instsetoo_native for the installation set.
    my $filename = File::Spec->catfile(
        $context->{'output-path'},
        $context->{'product-name'},
        $context->{'package-format'},
        "install",
        $language."_download",
        $exe_basename);

    printf("        current : %s\n", $filename);
    if ( ! -f $filename)
    {
        printf("ERROR: can not find %s\n", $filename);
        return undef;
    }
    else
    {
        my $msi = installer::patch::Msi->FindAndCreate(
            $context->{'target-version'},
            1,
            $language,
            $context->{'product-name'});
        return CreateReleaseItem($language, $filename, $msi);
    }
}



sub GetReleaseItemForOldBuild ($$$$)
{
    my ($context, $language, $exe_basename, $url_template) = @_;

    # Use ext_sources/ as local cache for archive.apache.org
    # and search these for the installation set.

    my $version = $context->{'target-version'};
    my $package_format =  $context->{'package-format'};
    my $releases_list = installer::patch::ReleasesList::Instance();

    my $url = $url_template;
    $url =~ s/%L/$language/g;
    $releases_list->{$version}->{$package_format}->{$language}->{'URL'} = $url;

    if ( ! installer::patch::InstallationSet::ProvideUnpackedExe(
               $version,
               0,
               $language,
               $package_format,
               $context->{'product-name'}))
    {
        # Can not provide unpacked EXE.
        return undef;
    }
    else
    {
        my $exe_filename = File::Spec->catfile(
            $ENV{'TARFILE_LOCATION'},
            $exe_basename);
        my $msi = installer::patch::Msi->FindAndCreate(
            $version,
            0,
            $language,
            $context->{'product-name'});
        return CreateReleaseItem($language, $exe_filename, $msi);
    }
}




sub UpdateReleasesXML($$)
{
    my ($context, $variables) = @_;

    my $releases_list = installer::patch::ReleasesList::Instance();
    my $output_filename = File::Spec->catfile(
        $context->{'output-path'},
        "misc",
        "releases.xml");

    my $target_version = $context->{'target-version'};
    my %version_hash = map {$_=>1} @{$releases_list->{'releases'}};
    my $item_hash = undef;
    if ( ! defined $version_hash{$context->{'target-version'}})
    {
        # Target version is not yet present.  Add it and print message that asks caller to check order.
        push @{$releases_list->{'releases'}}, $target_version;
        printf("adding data for new version %s to list of released versions.\n", $target_version);
        printf("please check order of releases in $output_filename\n");
        $item_hash = {};
    }
    else
    {
        printf("adding data for existing version %s to releases.xml\n", $target_version);
        $item_hash = $releases_list->{$target_version}->{$context->{'package-format'}};
    }
    $releases_list->{$target_version} = {$context->{'package-format'} => $item_hash};

    my @languages = GetLanguages();
    my %language_items = ();
    foreach my $language (@languages)
    {
        # There are three different sources where to find the downloadable installation sets.
        # 1. archive.apache.org for previously released versions.
        # 2. A local cache or repository directory that conceptually is a local copy of archive.apache.org
        # 3. The downloadable installation sets built in instsetoo_native/.

        my $exe_basename = sprintf(
            "%s_%s_Win_x86_install_%s.exe",
            $context->{'product-name'},
            $target_version,
            $language);
        my $url_template = sprintf(
            "http://archive.apache.org/dist/openoffice/%s/binaries/%%L/%s_%s_Win_x86_install_%%L.exe",
            $target_version,
            $context->{'product-name'},
            $target_version);

        my $item = undef;
        if ($target_version eq $variables->{PRODUCTVERSION})
        {
            $item = GetReleaseItemForCurrentBuild($context, $language, $exe_basename);
        }
        else
        {
            $item = GetReleaseItemForOldBuild($context, $language, $exe_basename, $url_template);
        }

        next unless defined $item;

        $language_items{$language} = $item;
        $item_hash->{$language} = $item;
        $item_hash->{'upgrade-code'} = $item->{'upgrade-code'};
        $item_hash->{'build-id'} = $item->{'build-id'};
        $item_hash->{'url-template'} = $url_template;
    }

    my @valid_languages = sort keys %language_items;
    $item_hash->{'languages'} = \@valid_languages;

    $releases_list->Write($output_filename);

    printf("\n\n");
    printf("please copy '%s' to main/instsetoo_native/data\n", $output_filename);
    printf("and check in the modified file to the version control system\n");
}




sub main ()
{
    my $context = ProcessCommandline();
#    installer::logger::starttime();
#    $installer::logger::Global->add_timestamp("starting logging");
    installer::logger::SetupSimpleLogging(undef);

    die "ERROR: list file is not defined, please use --lst-file option"
        unless defined $context->{'lst-file'};
    die "ERROR: product name is not defined, please use --product-name option"
        unless defined $context->{'product-name'};
    die sprintf("ERROR: package format %s is not supported", $context->{'package-format'})
        unless defined $context->{'package-format'} ne "msi";

    my ($variables, undef, undef) = installer::ziplist::read_openoffice_lst_file(
        $context->{'lst-file'},
        $context->{'product-name'},
        undef);
    DetermineVersions($context, $variables);

    if ($context->{'command'} =~ /create|check/)
    {
        my $filename = File::Spec->catfile(
            $context->{'output-path'},
            $context->{'product-name'},
            "msp",
            $context->{'source-version-dash'} . "_" . $context->{'target-version-dash'},
            $context->{'language'},
            "log",
            "patch-creation.log");
        my $dirname = dirname($filename);
        File::Path::make_path($dirname) unless -d $dirname;
        printf("directing output to $filename\n");

        $installer::logger::Lang->set_filename($filename);
        $installer::logger::Lang->copy_lines_from($installer::logger::Global);
        $installer::logger::Lang->set_forward(undef);
        $installer::logger::Info->set_forward($installer::logger::Lang);
    }

    if ($context->{'command'} eq "create")
    {
        CreatePatch($context, $variables);
    }
    elsif ($context->{'command'} eq "apply")
    {
        ApplyPatch($context, $variables);
    }
    elsif ($context->{'command'} eq "update-releases-xml")
    {
        UpdateReleasesXML($context, $variables);
    }
    elsif ($context->{'command'} eq "check")
    {
        CheckPatchCompatability($context, $variables);
    }
}


main();
