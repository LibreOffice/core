#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

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
            next if ($full eq '@loader_path/OOoPython.framework/Versions/2.6/OOoPython');
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
