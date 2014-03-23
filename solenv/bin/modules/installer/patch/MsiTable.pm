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

package installer::patch::MsiTable;

=head1 NAME

    package installer::patch::MsiTable - Class that represents one table of an Msi file.

=cut

use installer::patch::MsiRow;

use strict;

=head new ($class, $filename, $table_name)

    Create a new MsiTable object from the output of a previous
    msidb.exe run.  The table is named $table_name, its data is read
    from $filename.

=cut
sub new ($$$)
{
    my ($class, $filename, $table_name) = @_;

    my $self = {
        'name' => $table_name,
        'filename' => $filename,
        'columns' => undef,
        'column_specs' => undef,
        'codepage' => undef,
        'is_valid' => 1,
        'is_modified' => 0
    };
    bless($self, $class);

    if (defined $filename &&  -f $filename)
    {
        $self->ReadFile($filename);
    }
    return $self;
}




sub SetColumnData ($@)
{
    my ($self, @data) = @_;

    if (((scalar @data) % 2) != 0)
    {
        installer::logger::PrintError("column data has to have an even number of elements: (<column-name> <data-spec>)+)\n");
        $self->{'is_valid'} = 0;
        return;
    }

    $self->{'columns'} = [];
    $self->{'column_specs'} = [];
    while (scalar @data > 0)
    {
        my $name = shift @data;
        my $spec = shift @data;
        push @{$self->{'columns'}}, $name;
        push @{$self->{'column_specs'}}, $spec;
    }
}




sub SetIndexColumns ($@)
{
    my ($self, @index_columns) = @_;

    $self->{'index_columns'} = [@index_columns];
}




sub SetCodepage ($$)
{
    my ($self, $codepage) = @_;

    $self->{'codepage'} = $codepage;
}




sub IsValid ($)
{
    my ($self) = @_;
    return $self->{'is_valid'};
}




sub Trim ($)
{
    my $line = shift;

    $line =~ s/(^\s+|\s+$)//g;

    return $line;
}



=head2 ReadFile($self, $filename)

    Read the content of the table from the specified .idt file.
    For each row a MsiRow object is appended to $self->{'rows'}.

=cut
sub ReadFile ($$)
{
    my ($self, $filename) = @_;

    if ( ! (-f $filename && -r $filename))
    {
        printf STDERR ("can not open idt file %s for reading\n", $filename);
        $self->{'is_valid'} = 0;
        return;
    }

    open my $in, "<", $filename;

    my $columns = Trim(<$in>);
    $self->{'columns'} = [split(/\t/, $columns)];

    my $column_specs = Trim(<$in>);
    $self->{'column_specs'} = [split(/\t/, $column_specs)];

    # Table name, index columns.
    my $line = Trim(<$in>);
    my @items = split(/\t/, $line);
    my $item_count = scalar @items;
    if ($item_count>=1 && $items[0] eq $self->{'name'})
    {
        # No codepage.
    }
    elsif ($item_count>=2 && $items[1] eq $self->{'name'})
    {
        $self->{'codepage'} = shift @items;
    }
    else
    {
        printf STDERR ("reading wrong table data for table '%s' (got %s)\n", $self->{'name'}, $items[0]);
        $self->{'is_valid'} = 0;
        return;
    }
    shift @items;
    $self->{'index_columns'} = [@items];
    $self->{'index_column_index'} = $self->GetColumnIndex($items[0]);

    my $rows = [];
    while (<$in>)
    {
        # Remove all trailing returns and newlines.  Keep trailing spaces and tabs.
        s/[\r\n]+$//g;

        my @items = split(/\t/, $_);
        push @$rows, new installer::patch::MsiRow($self, @items);
    }
    $self->{'rows'} = $rows;

    return $self;
}




=head WriteFile($self, $filename)

    Write a text file containing the current table content.

=cut
sub WriteFile ($$)
{
    my ($self, $filename) = @_;

    open my $out, ">".$self->{'filename'};

    print $out join("\t", @{$self->{'columns'}})."\r\n";
    print $out join("\t", @{$self->{'column_specs'}})."\r\n";
    if (defined $self->{'codepage'})
    {
        print $out $self->{'codepage'} . "\t";
    }
    print $out $self->{'name'} . "\t";
    print $out join("\t",@{$self->{'index_columns'}})."\r\n";

    foreach my $row (@{$self->{'rows'}})
    {
        print $out $row->Format("\t")."\r\n";
    }

    close $out;
}




sub UpdateTimestamp ($)
{
    my $self = shift;

    utime(undef,undef, $self->{'filename'});
}




sub GetName ($)
{
    my $self = shift;

    return $self->{'name'};
}




=head2 GetColumnCount($self)

    Return the number of columns in the table.

=cut
sub GetColumnCount ($)
{
    my ($self) = @_;

    return scalar @{$self->{'columns'}};
}




=head2 GetRowCount($self)

    Return the number of rows in the table.

=cut
sub GetRowCount ($)
{
    my ($self) = @_;

    return scalar @{$self->{'rows'}};
}




=head2 GetColumnIndx($self, $column_name)

    Return the 0 based index of the column named $column_name.  Use
    this to speed up (slightly) access to column values when accessing
    many or all rows of a table.

=cut
sub GetColumnIndex ($$)
{
    my ($self, $column_name) = @_;

    my $index = 0;
    foreach my $name (@{$self->{'columns'}})
    {
        if ($name eq $column_name)
        {
            return $index;
        }
        ++$index;
    }

    printf STDERR ("did not find column %s in %s\n", $column_name, join(" and ", @{$self->{'columns'}}));
    return -1;
}



=head2 GetRowIndex($self, $index_column_index, $index_column_value)

    Return the index, starting at 0, of the (first) row that has value $index_column_value
    in column with index $index_column_index.

    Return -1 if now such row is found.

=cut
sub GetRowIndex ($$$)
{
    my ($self, $index_column_index, $index_column_value) = @_;

    my $rows = $self->{'rows'};
    for (my ($row_index,$row_count)=(0,scalar @$rows); $row_index<$row_count; ++$row_index)
    {
        my $row = $rows->[$row_index];
        if ($row->GetValue($index_column_index) eq $index_column_value)
        {
            return $row_index;
        }
    }

    return -1;
}




=head2 GetValue($self, $selector_column, $selector_column_value, $value_column)

    Find the row in which the $selector_column has value
    $selector_column_value and return its value in the $value_column.

=cut

sub GetValue ($$$$)
{
    my ($self, $selector_column, $selector_column_value, $value_column) = @_;

    my $row = $self->GetRow($selector_column, $selector_column_value);
    if (defined $row)
    {
        return $row->GetValue($value_column);
    }
    else
    {
        return undef;
    }
}




=head2 GetRow($self, $column, $value)

    Return the (first) row which has $value in $column.

=cut
sub GetRow ($$$)
{
    my ($self, $column, $value) = @_;

    my $column_index = $self->GetColumnIndex($column);
    if ($column_index<0)
    {
        printf STDERR "ERROR: unknown column $column in table $self->{'name'}\n";
        return undef;
    }

    foreach my $row (@{$self->{'rows'}})
    {
        if ($row->GetValue($column_index) eq $value)
        {
            return $row;
        }
    }

    printf STDERR ("ERROR: did not find row for %s->%s in %s\n",
        $column,
        $value,
        table $self->{'name'});

    return undef;
}




=head2 GetAllRows ($self)

    Return the reference to an array that contains all rows of the table.

=cut

sub GetAllRows ($)
{
    my $self = shift;

    return $self->{'rows'};
}




=head2 SetRow($self, {$key, $value}*)

    Replace an existing row.  If no matching row is found then add the row.

    The row is defined by a set of key/value pairs.  Their order is defined by the keys (column names)
    and their indices as defined in $self->{'columns'}.

    Rows are compared by their values of the index column.  By default this is the first element of
    $self->{'index_columns'} but is overruled by the last key that starts with a '*'.

=cut
sub SetRow ($@)
{
    my $self = shift;
    my @data = @_;

    my @items = ();
    my $index_column = $self->{'index_columns'}->[0];

    # Key/Value has to have an even number of entries.
    MsiTools::Die("invalid arguments given to MsiTable::SetRow()\n") if (scalar @data%2) != 0;

    # Find column indices for column names.
    while (scalar @data > 0)
    {
        my $column_name = shift @data;
        if ($column_name =~ /^\*(.*)$/)
        {
            # Column name starts with a '*'.  Use it as index column.
            $column_name = $1;
            $index_column = $1;
        }
        my $value = shift @data;
        my $column_index = $self->GetColumnIndex($column_name);
        $items[$column_index] = $value;
    }

    my $index_column_index = $self->GetColumnIndex($index_column);
    my $row_index = $self->GetRowIndex($index_column_index, $items[$index_column_index]);

    if ($row_index < 0)
    {
        # Row does not yet exist.  Add it.
        push @{$self->{'rows'}}, installer::patch::MsiRow->new($self, @items);
    }
    else
    {
        # Row does already exist.  Replace it.
        $self->{'rows'}->[$row_index] = installer::patch::MsiRow->new($self, @items);
    }

    $self->MarkAsModified();
}




sub MarkAsModified ($)
{
    my $self = shift;

    $self->{'is_modified'} = 1;
}




sub MarkAsUnmodified ($)
{
    my $self = shift;

    $self->{'is_modified'} = 0;
}




sub IsModified ($)
{
    my $self = shift;

    return $self->{'is_modified'};
}


1;
