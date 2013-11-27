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

package installer::patch::MsiRow;

=head1 NAME

    package installer::patch::MsiRow - Class that represents a single row of an Msi table.

=cut


=head2 new ($class, $table, @data)

    Create a new MsiRow object for the given table row data.  Each row
    stores a reference to its $table so that it can access global
    values like column names.

=cut
sub new ($$@)
{
    my ($class, $table, @data) = @_;

    my $self = {
        'table' => $table,
        'values' => [@data]
    };
    bless($self, $class);

    my $column_count = $table->GetColumnCount();
    while (scalar @{$self->{'values'}} < $column_count)
    {
        push @{$self->{'values'}}, "";
    }

    return $self;
}



=head2 GetValue($self, $column)

    Return the value in the column specified by $column, which can be
    either the column name or the index of the column.

=cut
sub GetValue ($$)
{
    my ($self, $column) = @_;

    if ($column =~ /^\d+$/)
    {
        return $self->{'values'}->[$column];
    }
    else
    {
        my $column_index = $self->{'table'}->GetColumnIndex($column);
        return $self->{'values'}->[$column_index];
    }
}




sub SetValue ($$$)
{
    my ($self, $column, $value) = @_;

    if ($column =~ /^\d+$/)
    {
        $self->{'values'}->[$column] = $value;
    }
    else
    {
        my $column_index = $self->{'table'}->GetColumnIndex($column);
        $self->{'values'}->[$column_index] = $value;
    }
    $self->{'table'}->MarkAsModified();
}




sub Format ($$)
{
    my $self = shift;
    my $concatenation = shift;

    my $result = "";
    my $first = 1;
    my $index = 0;
    my $column_count = $self->{'table'}->GetColumnCount();
    foreach my $item (@{$self->{'values'}})
    {
        ++$index;

        if ( ! $first)
        {
            $result .= $concatenation;
        }
        else
        {
            $first = 0;
        }
        $result .= $item;
    }
    return $result;
}




sub Clone ($$)
{
    my ($self, $new_table) = @_;

    my $clone = { %$self };
    $clone->{'values'} = [ @{$self->{'values'}} ];
    $clone->{'table'} = $new_table;
    bless($clone, "MsiRow");

    return $clone;
}




sub SetTable ($$)
{
    my ($self, $new_table) = @_;

    if (defined $self->{'table'} && $self->{'table'} != $new_table)
    {
        MsiTools::Die("can not reset table of row");
    }
    else
    {
        $self->{'table'} = $new_table;
    }
}

1;
