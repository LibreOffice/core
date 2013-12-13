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

package installer::patch::Msi;

use installer::patch::MsiTable;
use installer::patch::Tools;
use installer::patch::InstallationSet;

use File::Basename;
use File::Copy;

use strict;


=head1 NAME

    package installer::patch::Msi - Class represents a single MSI file and gives access to its tables.

=cut

sub FindAndCreate($$$$$)
{
    my ($class, $version, $is_current_version, $language, $product_name) = @_;

    my $condensed_version = $version;
    $condensed_version =~ s/\.//g;

    # When $version is the current version we have to search the msi at a different place.
    my $path;
    my $filename;
    my $is_current = 0;
    $path = installer::patch::InstallationSet::GetUnpackedExePath(
        $version,
        $is_current_version,
        installer::languages::get_normalized_language($language),
        "msi",
        $product_name);

    # Find the msi in the path.ls .
    $filename = File::Spec->catfile($path, "openoffice".$condensed_version.".msi");
    $is_current = $is_current_version;

    return $class->new($filename, $version, $is_current, $language, $product_name);
}






=head2 new($class, $filename, $version, $is_current_version, $language, $product_name)

    Create a new object of the Msi class.  The values of $version, $language, and $product_name define
    where to look for the msi file.

    If construction fails then IsValid() will return false.

=cut

sub new ($$;$$$$)
{
    my ($class, $filename, $version, $is_current_version, $language, $product_name) = @_;

    if ( ! -f $filename)
    {
        installer::logger::PrintError("can not find the .msi file for version %s and language %s at '%s'\n",
            $version,
            $language,
            $filename);
        return undef;
    }

    my $self = {
        'filename' => $filename,
        'path' => dirname($filename),
        'version' => $version,
        'is_current_version' => $is_current_version,
        'language' => $language,
        'package_format' => "msi",
        'product_name' => $product_name,
        'tmpdir' => File::Temp->newdir(CLEANUP => 1),
        'is_valid' => -f $filename
    };
    bless($self, $class);

    # Fill in some missing values from the 'Properties' table.
    if ( ! (defined $version && defined $language && defined $product_name))
    {
        my $property_table = $self->GetTable("Property");

        $self->{'version'} = $property_table->GetValue("Property", "DEFINEDVERSION", "Value")
            unless defined $self->{'version'};
        $self->{'product_name'} = $property_table->GetValue("Property", "DEFINEDPRODUCT", "Value")
            unless defined $self->{'product_name'};

        my $language = $property_table->GetValue("Property", "ProductLanguage", "Value");
        # TODO: Convert numerical language id to language name.
        $self->{'language'} = $language
            unless defined $self->{'language'};
    }

    return $self;
}




sub IsValid ($)
{
    my ($self) = @_;

    return $self->{'is_valid'};
}




=head2 Commit($self)

    Write all modified tables back into the databse.

=cut

sub Commit ($)
{
    my $self = shift;

    my @tables_to_update = ();
    foreach my $table (values %{$self->{'tables'}})
    {
        push @tables_to_update,$table if ($table->IsModified());
    }

    if (scalar @tables_to_update > 0)
    {
        $installer::logger::Info->printf("writing modified tables to database:\n");
        foreach my $table (@tables_to_update)
        {
            $installer::logger::Info->printf("    %s\n", $table->GetName());
            $self->PutTable($table);
        }

        foreach my $table (@tables_to_update)
        {
            $table->UpdateTimestamp();
            $table->MarkAsUnmodified();
        }
    }
}




=head2 GetTable($seld, $table_name)

    Return an MsiTable object for $table_name.  Table objects are kept
    alive for the life time of the Msi object.  Therefore the second
    call for the same table is very cheap.

=cut

sub GetTable ($$)
{
    my ($self, $table_name) = @_;

    my $table = $self->{'tables'}->{$table_name};
    if ( ! defined $table)
    {
        my $table_filename = File::Spec->catfile($self->{'tmpdir'}, $table_name .".idt");
        if ( ! -f $table_filename
            || ! EnsureAYoungerThanB($table_filename, $self->{'fullname'}))
        {
            # Extract table from database to text file on disk.
            my $truncated_table_name = length($table_name)>8 ? substr($table_name,0,8) : $table_name;
            my $command = join(" ",
                "msidb.exe",
                "-d", installer::patch::Tools::ToEscapedWindowsPath($self->{'filename'}),
                "-f", installer::patch::Tools::ToEscapedWindowsPath($self->{'tmpdir'}),
                "-e", $table_name);
            my $result = qx($command);
        }

        # Read table into memory.
        $table = new installer::patch::MsiTable($table_filename, $table_name);
        $self->{'tables'}->{$table_name} = $table;
    }

    return $table;
}




=head2 PutTable($self, $table)

    Write the given table back to the databse.

=cut

sub PutTable ($$)
{
    my ($self, $table) = @_;

    # Create text file from the current table content.
    $table->WriteFile();

    my $table_name = $table->GetName();

    # Store table from text file into database.
    my $table_filename = $table->{'filename'};

    if (length($table_name) > 8)
    {
        # The file name of the table data must not be longer than 8 characters (not counting the extension).
        # The name passed as argument to the -i option may be longer.
        my $truncated_table_name = substr($table_name,0,8);
        my $table_truncated_filename = File::Spec->catfile(
            dirname($table_filename),
            $truncated_table_name.".idt");
        File::Copy::copy($table_filename, $table_truncated_filename) || die("can not create table file with short name");
    }

    my $command = join(" ",
        "msidb.exe",
        "-d", installer::patch::Tools::ToEscapedWindowsPath($self->{'filename'}),
        "-f", installer::patch::Tools::ToEscapedWindowsPath($self->{'tmpdir'}),
        "-i", $table_name);
    my $result = system($command);

    if ($result != 0)
    {
        installer::logger::PrintError("writing table '%s' back to database failed", $table_name);
        # For error messages see http://msdn.microsoft.com/en-us/library/windows/desktop/aa372835%28v=vs.85%29.aspx
    }
}




=head2 EnsureAYoungerThanB ($filename_a, $filename_b)

    Internal function (not a method) that compares to files according
    to their last modification times (mtime).

=cut

sub EnsureAYoungerThanB ($$)
{
    my ($filename_a, $filename_b) = @_;

    die("file $filename_a does not exist") unless -f $filename_a;
    die("file $filename_b does not exist") unless -f $filename_b;

    my @stat_a = stat($filename_a);
    my @stat_b = stat($filename_b);

    if ($stat_a[9] <= $stat_b[9])
    {
        return 0;
    }
    else
    {
        return 1;
    }
}




=head2 SplitLongShortName($name)

    Split $name (typically from the 'FileName' column in the 'File'
    table or 'DefaultDir' column in the 'Directory' table) at the '|'
    into short (8.3) and long names.  If there is no '|' in $name then
    $name is returned as both short and long name.

    Returns long and short name (in this order) as array.

=cut

sub SplitLongShortName ($)
{
    my ($name) = @_;

    if ($name =~ /^([^\|]*)\|(.*)$/)
    {
        return ($2,$1);
    }
    else
    {
        return ($name,$name);
    }
}



=head2 SplitTargetSourceLongShortName ($name)

    Split $name first at the ':' into target and source parts and each
    of those at the '|'s into long and short parts.  Names that follow
    this pattern come from the 'DefaultDir' column in the 'Directory'
    table.

=cut

sub SplitTargetSourceLongShortName ($)
{
    my ($name) = @_;

    if ($name =~ /^([^:]*):(.*)$/)
    {
        return (installer::patch::Msi::SplitLongShortName($1), installer::patch::Msi::SplitLongShortName($2));
    }
    else
    {
        my ($long,$short) = installer::patch::Msi::SplitLongShortName($name);
        return ($long,$short,$long,$short);
    }
}




sub SetupFullNames ($$);
sub SetupFullNames ($$)
{
    my ($item, $directory_map) = @_;

    # Don't process any item twice.
    return if defined $item->{'full_source_name'};

    my $parent = $item->{'parent'};
    if (defined $parent)
    {
        # Process the parent first.
        if ( ! defined $parent->{'full_source_long_name'})
        {
            SetupFullNames($parent, $directory_map);
        }

        # Prepend the full names of the parent to our names.
        $item->{'full_source_long_name'}
            = $parent->{'full_source_long_name'} . "/" . $item->{'source_long_name'};
        $item->{'full_source_short_name'}
            = $parent->{'full_source_short_name'} . "/" . $item->{'source_short_name'};
        $item->{'full_target_long_name'}
            = $parent->{'full_target_long_name'} . "/" . $item->{'target_long_name'};
        $item->{'full_target_short_name'}
            = $parent->{'full_target_short_name'} . "/" . $item->{'target_short_name'};
    }
    else
    {
        # Directory has no parent => full names are the same as the name.
        $item->{'full_source_long_name'} = $item->{'source_long_name'};
        $item->{'full_source_short_name'} = $item->{'source_short_name'};
        $item->{'full_target_long_name'} = $item->{'target_long_name'};
        $item->{'full_target_short_name'} = $item->{'target_short_name'};
    }
}




=head2 GetDirectoryMap($self)

    Return a map that maps directory unique names (column 'Directory' in table 'Directory')
    to hashes that contains short and long source and target names.

=cut

sub GetDirectoryMap ($)
{
    my ($self) = @_;

    if (defined $self->{'DirectoryMap'})
    {
        return $self->{'DirectoryMap'};
    }

    # Initialize the directory map.
    my $directory_table = $self->GetTable("Directory");
    my $directory_map = ();
    foreach my $row (@{$directory_table->GetAllRows()})
    {
        my ($target_long_name, $target_short_name, $source_long_name, $source_short_name)
            = installer::patch::Msi::SplitTargetSourceLongShortName($row->GetValue("DefaultDir"));
        my $unique_name = $row->GetValue("Directory");
        $directory_map->{$unique_name} =
        {
            'unique_name' => $unique_name,
            'parent_name' => $row->GetValue("Directory_Parent"),
            'default_dir' => $row->GetValue("DefaultDir"),
            'source_long_name' => $source_long_name,
            'source_short_name' => $source_short_name,
            'target_long_name' => $target_long_name,
            'target_short_name' => $target_short_name
        };
    }

    # Add references to parent directories.
    foreach my $item (values %$directory_map)
    {
        $item->{'parent'} = $directory_map->{$item->{'parent_name'}};
    }

    # Set up full names for all directories.
    foreach my $item (values %$directory_map)
    {
        SetupFullNames($item, $directory_map);
    }

    # Cleanup the names.
    foreach my $item (values %$directory_map)
    {
        foreach my $id (
            'full_source_long_name',
            'full_source_short_name',
            'full_target_long_name',
            'full_target_short_name')
        {
            $item->{$id} =~ s/\/(\.\/)+/\//g;
            $item->{$id} =~ s/^SourceDir\///;
            $item->{$id} =~ s/^\.$//;
        }
    }

    $self->{'DirectoryMap'} = $directory_map;
    return $self->{'DirectoryMap'};
}




=head2 GetFileMap ($)

    Return a map (hash) that maps the unique name (column 'File' in
    the 'File' table) to data that is associated with that file, like
    the directory or component.

    The map is kept alive for the lifetime of the Msi object.  All
    calls but the first are cheap.

=cut

sub GetFileMap ($)
{
    my ($self) = @_;

    if (defined $self->{'FileMap'})
    {
        return $self->{'FileMap'};
    }

    my $file_table = $self->GetTable("File");
    my $component_table = $self->GetTable("Component");
    my $dir_map = $self->GetDirectoryMap();

    # Setup a map from component names to directory items.
    my %component_to_directory_map =
        map
        {$_->GetValue('Component') => $_->GetValue('Directory_')}
        @{$component_table->GetAllRows()};

    # Finally, create the map from files to directories.
    my $file_map = {};
    my $file_component_index = $file_table->GetColumnIndex("Component_");
    my $file_file_index = $file_table->GetColumnIndex("File");
    my $file_filename_index = $file_table->GetColumnIndex("FileName");
    foreach my $file_row (@{$file_table->GetAllRows()})
    {
        my $component_name = $file_row->GetValue($file_component_index);
        my $directory_name = $component_to_directory_map{$component_name};
        my $unique_name = $file_row->GetValue($file_file_index);
        my $file_name = $file_row->GetValue($file_filename_index);
        my ($long_name, $short_name) = SplitLongShortName($file_name);
        $file_map->{$unique_name} = {
            'directory' => $dir_map->{$directory_name},
            'component_name' => $component_name,
            'file_name' => $file_name,
            'long_name' => $long_name,
            'short_name' => $short_name
        };
    }

    $self->{'FileMap'} = $file_map;
    return $file_map;
}


1;
