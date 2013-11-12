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

use XML::LibXML;
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
        $Instance = new installer::patch::ReleasesList();
    }
    return $Instance;
}




=head2 new($class)

    Internal constructor.  Don't call.

=cut
sub new ($)
{
    my ($class) = @_;

    my $self = {};
    bless($self, $class);

    $self->Read();

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
        my @child_nodes = $node->getElementsByTagName($child_name);
        if (scalar @child_nodes == 0)
        {
            return undef;
        }
        else
        {
            return $child_nodes[0];
        }
    }
}




=head2 GetText ($node)

    Internal function that returns the trimmed text content of a node.

=cut
sub GetText ($)
{
    my ($node) = @_;

    if ( ! defined $node)
    {
        return "";
    }
    else
    {
        my $text = $node->textContent();
        $text =~ s/(^\s+|\s+$)//g;
        return $text;
    }
}




=head2 Read($self)

    Read the releases.xml file as doctree and parse its content.

=cut
sub Read ($)
{
    my ($self) = @_;

    my $filename = File::Spec->catfile($ENV{'SRC_ROOT'}, "instsetoo_native", "data", "releases.xml");
    my $parser = XML::LibXML->new();
    my $document = $parser->parse_file($filename);
    foreach my $release_node ($document->getElementsByTagName("release"))
    {
        my $version_node = GetFirstChild($release_node, "version");
        my $version = GetText($version_node);
        next if $version eq "";

        foreach my $download_node (GetFirstChild($release_node, "download"))
        {
            my $package_node = GetFirstChild($download_node, "package-format");
            my $package_format = GetText($package_node);
            next if $package_format eq "";

            my $download_data = ParseDownloadData($download_node);
            if (defined $download_data)
            {
                $self->{$version}->{$package_format} = $download_data;
            }
        }
    }

}




=head2 ParseDownloadData ($download_node)

    Parse the data for one set of download data (there is one per release and package format).

=cut
sub ParseDownloadData ($)
{
    my ($download_node) = @_;

    my $url_node = GetFirstChild($download_node, "url-template");
    my $url_template = GetText($url_node);
    if ($url_template eq "")
    {
        print STDERR "releases data file corrupt (no URL template)\n";
        return undef;
    }

    my $download_data = {};
    foreach my $item_node (@{$download_node->getElementsByTagName("item")})
    {
        my $language = GetText(GetFirstChild($item_node, "language"));
        my $checksum_node = GetFirstChild($item_node, "checksum");
        if ( ! defined $checksum_node)
        {
            print STDERR "releases data file corrupt (item has no 'checksum' node)\n";
            return undef;
        }
        my $checksum_type = $checksum_node->getAttribute("type");
        my $checksum_value = GetText($checksum_node);
        my $file_size = GetText(GetFirstChild($item_node, "size"));

        my $url = $url_template;
                $url =~ s/\%L/$language/g;
        $download_data->{$language} = {
            'URL' => $url,
            'checksum-type' => $checksum_type,
            'checksum-value' => $checksum_value,
            'file-size' => $file_size
        };
    }

    return $download_data;
}

1;
