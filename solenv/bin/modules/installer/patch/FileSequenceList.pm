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

use XML::LibXML;
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




sub SetFromFileList ($$)
{
    my ($self, $files) = @_;

    my %data = map {$_->{'uniquename'} => $_->{'sequencenumber'}} @$files;
    $self->{'data'} = \%data;
}




sub SetFromMap ($$)
{
    my ($self, $map) = @_;

    $self->{'data'} = $map;
}




sub GetFileCount ($)
{
    my ($self) = @_;

    return scalar keys %{$self->{'data'}};
}




=head2 GetSequenceNumbers ($files)

    $files is a hash that maps unique file names (File->File) to sequence
    numbers (File->Sequence). The later is (expected to be) initially unset and
    is set in this method.

    For new files -- entries in the given $files that do not exist in the 'data'
    member -- no sequence numbers are defined.

    When there are removed files -- entries in the 'data' member that do not
    exist in the given $files -- then a list of these files is returned.  In
    that case the given $files remain unmodified.

    The returned list is empty when everyting is OK.

=cut
sub GetSequenceNumbers ($$)
{
    my ($self, $files) = @_;

    # Check if files have been removed.
    my @missing = ();
    foreach my $name (keys %{$self->{'data'}})
    {
        if ( ! defined $files->{$name})
        {
            push @missing, $name;
        }
    }
    if (scalar @missing > 0)
    {
        # Yes.  Return the names of the removed files.
        return @missing;
    }

    # No files where removed.  Set the sequence numbers.
    foreach my $name (keys %$files)
    {
        $files->{$name} = $self->{'data'}->{$name};
    }
    return ();
}




sub GetDifference ($$)
{
    my ($self, $other) = @_;

    # Create maps for easy reference.
    my (@files_in_both, @files_in_self, @files_in_other);
    foreach my $name (keys %{$self->{'data'}})
    {
        if (defined $other->{'data'}->{$name})
        {
            push @files_in_both, $name;
        }
        else
        {
            push @files_in_self, $name;
        }
    }
    foreach my $name (keys %{$self->{'data'}})
    {
        if ( ! defined $self->{'data'}->{$name})
        {
            push @files_in_other, $name;
        }
    }

    return (\@files_in_both, \@files_in_self, \@files_in_other);
}


1;
