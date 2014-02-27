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

#!/usr/bin/perl

use Archive::Zip;

use strict;
use warnings;

=head NAME

    replace_in_zip.pl  -  Replace a file in a zip file with another file on disk.

=head SYNOPSIS

    replace_in_zip.pl <zip-file> <zip-entry-path> <replacement-path> <image-name>+

=cut


sub main (@)
{
    my ($zip_filename, $entry_path, $replacement_path, @image_names) = @_;

    if (scalar @image_names == 0)
    {
        die "usage: replace_in_zip.pl <zip-file> <zip-entry-path> <replacement-path> <image-name>+";
    }

    # Open the archive.
    my $zip = Archive::Zip->new();
    if ( ! -f $zip_filename || $zip->read($zip_filename) != Archive::Zip::AZ_OK)
    {
        die "can not open zip file $zip_filename";
    }

    $entry_path .= "/" unless $entry_path =~ /\/$/;
    $replacement_path .= "/" unless $replacement_path =~ /\/$/;

    foreach my $image_basename (@image_names)
    {
        printf "replacing %s\n", $image_basename;

        # Get access to the entry.
        my $entry_name = $entry_path . $image_basename;
        my $member = $zip->memberNamed($entry_name);
        die "can not access entry $entry_name" unless defined $member;

        # Check the replacement file.
        my $replacement_filename = $replacement_path . $image_basename;
        die "can not read the replacement $replacement_filename"
            unless -f $replacement_filename;

        # Make the replacement.
        $zip->removeMember($member);
        my $new_member = $zip->addFile($replacement_filename, $entry_name);
        die "replacing failed" unless defined $new_member;
    }

    # Write zip back to file.
    printf "writing archive back to disk\n";
    if ($zip->overwrite() != Archive::Zip::AZ_OK)
    {
        die "writing zip back to disk failed";
    }
}

main(@ARGV);
