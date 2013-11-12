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
use strict;


=head1 NAME

    package installer::patch::Msi - Class represents a single MSI file and gives access to its tables.

=cut



=head2 new($class, $version, $language, $product_name)

    Create a new object of the Msi class.  The values of $version, $language, and $product_name define
    where to look for the msi file.

    If construction fails then IsValid() will return false.

=cut
sub new ($$$$)
{
    my ($class, $version, $language, $product_name) = @_;

    my $path = installer::patch::InstallationSet::GetUnpackedMsiPath(
        $version,
        $language,
        "msi",
        $product_name);

    # Find the msi in the path.
    my $filename = undef;
    if ( -d $path)
    {
        my @msi_files = glob(File::Spec->catfile($path, "*.msi"));
        if (scalar @msi_files != 1)
        {
            printf STDERR ("there are %d msi files in %s, should be 1", scalar @msi_files, $filename);
            $filename = "";
        }
        else
        {
            $filename = $msi_files[0];
        }
    }
    else
    {
        installer::logger::PrintError("can not access path '%s' to find msi\n", $path);
        return undef;
    }

    if ( ! -f $filename)
    {
        installer::logger::PrintError("can not access MSI file at '%s'\n", $filename);
        return undef;
    }

    my $self = {
        'filename' => $filename,
        'path' => $path,
        'version' => $version,
        'language' => $language,
        'package_format' => "msi",
        'product_name' => $product_name,
        'tmpdir' => File::Temp->newdir(CLEANUP => 1),
        'is_valid' => -f $filename
    };
    bless($self, $class);

    return $self;
}




sub IsValid ($)
{
    my ($self) = @_;

    return $self->{'is_valid'};
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
                "-d", installer::patch::Tools::CygpathToWindows($self->{'filename'}),
                "-f", installer::patch::Tools::CygpathToWindows($self->{'tmpdir'}),
                "-e", $table_name);
            my $result = qx($command);
            print $result;
        }

        # Read table into memory.
        $table = new installer::patch::MsiTable($table_filename, $table_name);
        $self->{'tables'}->{$table_name} = $table;
    }

    return $table;
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




=head2 GetFileToDirectoryMap ($)

    Return a map (hash) that maps the unique name (column 'File' in
    the 'File' table) to its directory names.  Each value is a
    reference to an array of two elements: the source path and the
    target path.

    The map is kept alive for the lifetime of the Msi object.  All
    calls but the first are cheap.

=cut
sub GetFileToDirectoryMap ($)
{
    my ($self) = @_;

    if (defined $self->{'FileToDirectoryMap'})
    {
        return $self->{'FileToDirectoryMap'};
    }

    my $file_table = $self->GetTable("File");
    my $directory_table = $self->GetTable("Directory");
    my $component_table = $self->GetTable("Component");
    $installer::logger::Info->printf("got access to tables File, Directory, Component\n");

    my %dir_map = ();
    foreach my $row (@{$directory_table->GetAllRows()})
    {
        my ($target_name, undef, $source_name, undef)
            = installer::patch::Msi::SplitTargetSourceLongShortName($row->GetValue("DefaultDir"));
        $dir_map{$row->GetValue("Directory")} = {
            'parent' => $row->GetValue("Directory_Parent"),
            'source_name' => $source_name,
            'target_name' => $target_name};
    }

    # Set up full names for all directories.
    my @todo = map {$_} (keys %dir_map);
    my $process_count = 0;
    my $push_count = 0;
    while (scalar @todo > 0)
    {
        ++$process_count;

        my $key = shift @todo;
        my $item = $dir_map{$key};
        next if defined $item->{'full_source_name'};

        if ($item->{'parent'} eq "")
        {
            # Directory has no parent => full names are the same as the name.
            $item->{'full_source_name'} = $item->{'source_name'};
            $item->{'full_target_name'} = $item->{'target_name'};
        }
        else
        {
            my $parent = $dir_map{$item->{'parent'}};
            if ( defined $parent->{'full_source_name'})
            {
                # Parent aleady has full names => we can create the full name of the current item.
                $item->{'full_source_name'} = $parent->{'full_source_name'} . "/" . $item->{'source_name'};
                $item->{'full_target_name'} = $parent->{'full_target_name'} . "/" . $item->{'target_name'};
            }
            else
            {
                # Parent has to be processed before the current item can be processed.
                # Push both to the head of the list.
                unshift @todo, $key;
                unshift @todo, $item->{'parent'};

                ++$push_count;
            }
        }
    }

    foreach my $key (keys %dir_map)
    {
        $dir_map{$key}->{'full_source_name'} =~ s/\/(\.\/)+/\//g;
        $dir_map{$key}->{'full_source_name'} =~ s/^SourceDir\///;
        $dir_map{$key}->{'full_target_name'} =~ s/\/(\.\/)+/\//g;
        $dir_map{$key}->{'full_target_name'} =~ s/^SourceDir\///;
    }
    $installer::logger::Info->printf("for %d directories there where %d processing steps and %d pushes\n",
        $directory_table->GetRowCount(),
        $process_count,
        $push_count);

    # Setup a map from component names to directory items.
    my %component_to_directory_map = map {$_->GetValue('Component') => $_->GetValue('Directory_')} @{$component_table->GetAllRows()};

    # Finally, create the map from files to directories.
    my $map = {};
    my $file_component_index = $file_table->GetColumnIndex("Component_");
    my $file_file_index = $file_table->GetColumnIndex("File");
    foreach my $file_row (@{$file_table->GetAllRows()})
    {
        my $component_name = $file_row->GetValue($file_component_index);
        my $directory_name = $component_to_directory_map{$component_name};
        my $dir_item = $dir_map{$directory_name};
        my $unique_name = $file_row->GetValue($file_file_index);
        $map->{$unique_name} = [$dir_item->{'full_source_name'},$dir_item->{'full_target_name'}];
    }

    $installer::logger::Info->printf("got full paths for %d files\n",
        $file_table->GetRowCount());

    $self->{'FileToDirectoryMap'} = $map;
    return $map;
}


1;
