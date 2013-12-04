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

package installer::patch::ReleasesList;

use XML::Parser;
use File::Spec;

use strict;

=head1 NAME

    package installer::patch::ReleasesList  -  Functions for accessing the instsetoo_native/data/releases.xml file

=cut


my $Instance = undef;

=head2 Instance()

    Return the singleton instance.

=cut
sub Instance()
{
    if ( ! defined $Instance)
    {
        $Instance = new installer::patch::ReleasesList(
            File::Spec->catfile($ENV{'SRC_ROOT'}, "instsetoo_native", "data", "releases.xml"));
    }
    return $Instance;
}




=head2 new($class, $filename)

    Internal constructor.  Don't call.

=cut
sub new ($$)
{
    my ($class, $filename) = @_;

    my $self = {
        'releases' => []
    };
    bless($self, $class);


    $self->Read($filename);


    return $self;
}




=head2 GetFirstChild ($node, $child_name)

    Internal function that returns the first child.  Use only when the
    first child is the (expected) only child in a list.

=cut
sub GetFirstChild ($$)
{
    my ($node, $child_name) = @_;

    if ( ! defined $node)
    {
        return undef;
    }
    else
    {
        my $value = $node->{$child_name};
        if (ref($value) eq 'ARRAY')
        {
            return $value->[0];
        }
        else
        {
            return $value;
        }
    }
}




=head2 GetText ($node)

    Internal function that returns the trimmed text content of a node.

=cut
sub GetText ($;$)
{
    my ($node, $default_text) = @_;

    if ( ! defined $node)
    {
        if (defined $default_text)
        {
            return $default_text;
        }
        else
        {
            return "";
        }
    }
    else
    {
        my $text = $node->{'__text__'};
        $text =~ s/(^\s+|\s+$)//g;
        return $text;
    }
}



sub GetAttribute ($$)
{
    my ($node, $attribute_name) = @_;

    my $attributes = $node->{'__attributes__'};
    if ( ! defined $attributes)
    {
        return undef;
    }
    else
    {
        return $attributes->{$attribute_name};
    }
}




sub PrintNode($$);
sub ReadDomTree ($)
{
    my ($filename) = @_;

    my $root = {};
    my $data = {
        'current_node' => $root,
        'node_stack' => []
    };
    my $parser = new XML::Parser(
        'Handlers' => {
            'Start' => sub {HandleStartTag($data, @_)},
            'End' => sub{HandleEndTag($data, @_)},
            'Char' => sub{HandleText($data, @_)}
        });
    $parser->parsefile($filename);

#    PrintNode("", $root);

    return $root;
}




sub PrintNode($$)
{
    my ($indentation, $node) = @_;

    if (defined $node->{'__attributes__'})
    {
        while (my ($name,$attribute) = each(%{$node->{'__attributes__'}}))
        {
            printf("    %s%s -> %s\n", $indentation, $name, $attribute);
        }
    }

    while (my ($key,$value) = each(%$node))
    {
        if ($key eq '__text__')
        {
            printf("%stext '%s'\n", $indentation, $value);
        }
        elsif ($key eq '__attributes__')
        {
            next;
        }
        elsif (ref($value) eq "ARRAY")
        {
            foreach my $item (@$value)
            {
                printf("%s%s {\n", $indentation, $key);
                PrintNode($indentation."    ", $item);
                printf("%s}\n", $indentation);
            }
        }
        else
        {
            printf("%s%s {\n", $indentation, $key);
            PrintNode($indentation."    ", $value);
            printf("%s}\n", $indentation);
        }
    }
}


sub HandleStartTag ($$$@)
{
    my ($data, $expat, $element, @attributes) = @_;

    # Create new node with attributes.
    my $node = {'__attributes__' => {@attributes}};

    # Append it to the list of $element objects.
    my $current_node = $data->{'current_node'};
    $current_node->{$element} = [] unless defined $current_node->{$element};
    push @{$current_node->{$element}}, $node;

    # Make the new node the current node.
    push @{$data->{'node_stack'}}, $current_node;
    $data->{'current_node'} = $node;
}

sub HandleEndTag ($$$)
{
    my ($data, $expat, $element) = @_;

    # Restore the parent node as current node.
    $data->{'current_node'} = pop @{$data->{'node_stack'}};
}

sub HandleText ($$$)
{
    my ($data, $expat, $text) = @_;
    if ($text !~ /^\s*$/)
    {
        $data->{'current_node'}->{'__text__'} .= $text;
    }
}

=head2 Read($self, $filename)

    Read the releases.xml file as doctree and parse its content.

=cut
sub Read ($$)
{
    my ($self, $filename) = @_;

    my $document = ReadDomTree($filename);
    foreach my $release_node (@{$document->{'releases'}->[0]->{'release'}})
    {
        my $version_node = GetFirstChild($release_node, "version");
        my $version_major = GetText(GetFirstChild($version_node, "major"));
        my $version_minor = GetText(GetFirstChild($version_node, "minor"), "0");
        my $version_micro = GetText(GetFirstChild($version_node, "micro"), "0");
        my $version = sprintf("%d.%d.%d", $version_major, $version_minor, $version_micro);
        die "could not read version from releases.xml" if $version eq "";

        push @{$self->{'releases'}}, $version;

        my $download_node = GetFirstChild($release_node, "downloads");
        my $package_format = GetText(GetFirstChild($download_node, "package-format"));
        my $url_template = GetText(GetFirstChild($download_node, "url-template"));
        my $upgrade_code = GetText(GetFirstChild($download_node, "upgrade-code"));
        my $build_id = GetText(GetFirstChild($download_node, "build-id"));
        die "could not read package format from releases.xml" if $package_format eq "";

        $self->{$version}->{$package_format}->{'upgrade-code'} = $upgrade_code;
        $self->{$version}->{$package_format}->{'build-id'} = $build_id;

        foreach my $item_node (@{$download_node->{'item'}})
        {
            my ($language, $download_data) = ParseDownloadData($item_node, $url_template);
            if (defined $download_data && defined $language)
            {
                $self->{$version}->{$package_format}->{$language} = $download_data;
            }
        }
    }
}




=head2 ParseDownloadData ($download_node)

    Parse the data for one set of download data (there is one per release and package format).

=cut
sub ParseDownloadData ($$)
{
    my ($item_node, $url_template) = @_;

    my $language = GetText(GetFirstChild($item_node, "language"));
    my $checksum_node = GetFirstChild($item_node, "checksum");
    if ( ! defined $checksum_node)
    {
        print STDERR "releases data file corrupt (item has no 'checksum' node)\n";
        return undef;
    }
    my $checksum_type = GetAttribute($checksum_node, "type");
    my $checksum_value = GetText($checksum_node);
    my $file_size = GetText(GetFirstChild($item_node, "size"));
    my $product_code = GetText(GetFirstChild($item_node, "product-code"));

    my $url = $url_template;
    $url =~ s/\%L/$language/g;
    return (
        $language,
        {
            'URL' => $url,
            'checksum-type' => $checksum_type,
            'checksum-value' => $checksum_value,
            'file-size' => $file_size,
            'product-code' => $product_code
        });
}




=head2 GetPreviousVersion($version)

    Look up $version in the sorted list of released versions.  Return
    the previous element.  Whe $version is not found then return the
    last element (under the assumption that $version will be the next
    released version).

=cut
sub GetPreviousVersion ($)
{
    my ($current_version) = @_;

    my $release_data = installer::patch::ReleasesList::Instance();
    my $previous_version = undef;
    foreach my $version (@{$release_data->{'releases'}})
    {
        if ($version eq $current_version)
        {
            return $previous_version;
        }
        else
        {
            $previous_version = $version;
        }
    }

    return $previous_version;
}





1;
