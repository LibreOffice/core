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

package installer::patch::FileSequenceList;

use strict;

=head1 NAME

    FileSequenceList.pm - Class for retrieving and processing the 'Sequence' values of the MSI 'File' table.

=cut

=head2 new($class)

    Create a new FileSequenceList object.

=cut
sub new ($)
{
    my ($class) = @_;

    my $self = {
        'data' => undef
    };
    bless($self, $class);

    return $self;
}




sub SetFromMap ($$)
{
    my ($self, $map) = @_;

    $self->{'data'} = $map;
}




sub SetFromMsi ($$)
{
    my ($self, $msi) = @_;

    my $file_table = $msi->GetTable("File");
    my $file_map = $msi->GetFileMap();

    my $file_column_index = $file_table->GetColumnIndex("File");
    my $filename_column_index = $file_table->GetColumnIndex("FileName");
    my $sequence_column_index = $file_table->GetColumnIndex("Sequence");

    my %sequence_data = ();

    printf("extracting columns %d and %d from %d rows\n",
        $file_column_index,
        $sequence_column_index,
        $file_table->GetRowCount());

    foreach my $row (@{$file_table->GetAllRows()})
    {
        my $unique_name = $row->GetValue($file_column_index);
        my $filename = $row->GetValue($filename_column_index);
        my ($long_filename,$short_filename) = installer::patch::Msi::SplitLongShortName($filename);
        my $sequence = $row->GetValue($sequence_column_index);
        my $directory_item = $file_map->{$unique_name}->{'directory'};
        my $source_path = $directory_item->{'full_source_long_name'};
        my $target_path = $directory_item->{'full_target_long_name'};
        my $key = $source_path ne ""
            ? $source_path."/".$long_filename
            : $long_filename;
        $sequence_data{$key} = {
            'sequence' => $sequence,
            'uniquename' => $unique_name,
            'row' => $row
        };
    }
    $self->{'data'} = \%sequence_data;
}




sub GetFileCount ($)
{
    my ($self) = @_;

    return scalar keys %{$self->{'data'}};
}




sub get_removed_files ($@)
{
    my ($self, $target_unique_names) = @_;

    my %uniquename_to_row_map = map{$_->{'uniquename'} => $_->{'row'}} values %{$self->{'data'}};

    # Check if files have been removed.
    my @missing = ();
    foreach my $item (values %{$self->{'data'}})
    {
        my ($uniquename, $row) = ($item->{'uniquename'}, $item->{'row'});
        if ( ! defined $target_unique_names->{$uniquename})
        {
            # $name is defined in source but not in target => it has been removed.
            push @missing, $row;
        }
    }
    return @missing;
}




sub get_sequence_and_unique_name($$)
{
    my ($self, $source_path) = @_;

    my $sequence_and_unique_name = $self->{'data'}->{$source_path};
    if ( ! defined $sequence_and_unique_name)
    {
        $installer::logger::Lang->printf("can not find entry for source path '%s'\n", $source_path);
        return (undef,undef);
    }
    else
    {
        return (
            $sequence_and_unique_name->{'sequence'},
            $sequence_and_unique_name->{'uniquename'});
    }
}


1;
