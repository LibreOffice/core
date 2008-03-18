#*************************************************************************
#
#   $RCSfile: macosx-dylib-link-list.pl,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 12:57:17 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2008 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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
