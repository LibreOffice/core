#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

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
    my $call = "otool -L $file";
    open(IN, "-|", $call) or die "cannot $call";
    while (<IN>)
    {
        if (m'^\s*(@.+/([^/]+)) \(compatibility version \d+\.\d+\.\d+, current version \d+\.\d+\.\d+\)\n$')
        {
            my $full = $1;
            my $loc = locate($2);
            # mapping for PythonFramework in RepositoryExternal.mk
            next if $full =~ m'^\s*@loader_path/(LibreOfficePython.framework/Versions/[^/]+/LibreOfficePython)';
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
