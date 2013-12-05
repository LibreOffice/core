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



use lib ("$ENV{SOLARENV}/bin/modules");
use macosxotoolhelper;

sub locate($)
{
    my ($lib) = @_;
    my $dir;
    foreach $dir (@dirs)
    {
        my $path = "$dir/$lib";
        if (-e $path)
        {
            return $path;
        }
    }
    return;
}

sub handle($$)
{
    my ($from, $to) = @_;
    # wrap -dylib_file in -Wl so that hopefully any used tool whatsoever (e.g.,
    # libtool generated from xmlsec1-1.2.6/configure included in
    # libxmlsec/download/xmlsec1-1.2.6.tar.gz:1.3) passes it through to the
    # linker:
    !($from =~ /,/ || $to =~ /,/) or
        die "$from:$to contains commas and cannot go into -Wl";
    print " -Wl,-dylib_file,$from:$to";
    $done{$from} = 1;
    push(@todo, $to) if (grep {$_ eq $to} @todo) == 0;
}

foreach (@ARGV) { push(@dirs, $1) if /^-L(.*)$/; }
foreach (@ARGV)
{
    if (/^-l(.*)$/)
    {
        my $loc = locate("lib$1.dylib");
        handle($1, $loc) if defined $loc && otoolD($loc) =~ m'^(@.+/.+)\n$';
    }
}
foreach $file (@todo)
{
    my $call = "${::CC_PATH}otool -L $file";
    open(IN, "-|", $call) or die "cannot $call";
    while (<IN>)
    {
        if (m'^\s*(@.+/([^/]+)) \(compatibility version \d+\.\d+\.\d+, current version \d+\.\d+\.\d+\)\n$')
        {
            my $full = $1;
            my $loc = locate($2);
            if (defined $loc)
            {
                handle($full, $loc) unless defined $done{$full};
            }
            else
            {
                die "unknown $full (from $file)";
            }
        }
    }
    close(IN);
}
print "\n";
