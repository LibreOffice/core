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

use LWP::UserAgent;

use strict;

=head1 NAME

    patch_make_releases_xml.pl - Create a section for the instsetoo_native/data/releases.xml file.

=head1 SYNOPSIS

    patch_make_releases_xml.pl <version-number>

        version-number is the version number (eg 4.0.1) for which to create the releases.xml file.

=head1 DESCRIPTION

    Will contact http://archive.apache.org/dist/openoffice/<version-number>/binaries/ and
    a) determine the set of languages
    b) collect sizes and sha256 check sums for all Windows installation sets.

    The result is printed to the console.  It has to be added manually to releases.xml.

=cut


if (scalar @ARGV != 1)
{
    print STDERR "usage: $0 <version-number>\n";
    die;
}

my $version = $ARGV[0];

print <<EOT;
<?xml version='1.0' encoding='UTF-8'?>
<!--***********************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 ***********************************************************-->
EOT

sub DownloadFile ($)
{
    my $url = shift;

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




sub GetResponse ($)
{
    my $url = shift;

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
    return $agent->get($url, 'Range' => "bytes=0-0");
}

my @languages = ();
my @lines = split(/\n/, DownloadFile("http://archive.apache.org/dist/openoffice/".$version."/binaries/"));
foreach my $line (@lines)
{
    next unless $line =~ /folder.gif/;
    if ($line =~ /a href=\"([^\"\/]+)\/\"/)
    {
    my $language = $1;
    next if $language eq "SDK";
    next if $language =~ /^[A-Z]/;
    push @languages, $language;
    }
}

print "<releases>\n";
print "  <release>\n";
printf "    <version>%s</version>\n", $version;
print "    <download>\n";
print "      <package-format>msi</package-format>\n";
print "      <url-template>\n";
printf "        http://archive.apache.org/dist/openoffice/%s/binaries/%%L/Apache_OpenOffice_%s_Win_x86_install_%%L.exe\n",$version, $version;
print "      </url-template>\n";
foreach my $language (sort @languages)
{
    print "      <item>\n";
    printf "        <language>%s</language>\n", $language;
    my $name = sprintf(
    "Apache_OpenOffice_%s_Win_x86_install_%s.exe",
    $version,
    $language,
    $version,
    $language);

    my $content = DownloadFile(
    sprintf("http://archive.apache.org/dist/openoffice/%s/binaries/%s/%s.sha256", $version, $language, $name));
    if ($content =~ /^([a-f0-9]+)/)
    {
    printf("        <checksum type=\"sha256\">%s</checksum>\n", $1);
    }
    my $response = GetResponse(
    sprintf("http://archive.apache.org/dist/openoffice/%s/binaries/%s/%s", $version, $language, $name));
    my $content_range = $response->{'_headers'}->{'content-range'};
    if ($content_range =~ /bytes 0-0\/(\d+)/)
    {
    printf("        <size>%s</size>\n", $1);
    }
    print "      </item>\n";
}

print "    </download>\n";
print "  </release>\n";
print "</releases>\n";
