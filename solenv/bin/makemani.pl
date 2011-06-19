#! /usr/bin/perl -w
    eval 'exec /usr/bin/perl -S $0 ${1+"$@"}'
        if 0; #$running_under_some_shell
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

use strict;
use File::Find ();
use Cwd qw (cwd);

my @findlist;

# Set the variable $File::Find::dont_use_nlink if you're using AFS,
# since AFS cheats.

# for the convenience of &wanted calls, including -eval statements:
use vars qw/*name *dir *prune/;
*name   = *File::Find::name;
*dir    = *File::Find::dir;
*prune  = *File::Find::prune;

sub wanted;



sub wanted {
    /^.*\.xc(s|u)\z/s
    && ( push @findlist, $name );
}

sub usage
{
    print STDERR "\n$0 - append *.xcu file entries to .oxt manifest.xml\n\n";
    print STDERR "usage: $0 <static_part> <start dir> <search dir> <destination dir>\n\n";
    print STDERR "  static part - file containig all other content for mainfest.xml\n";
    print STDERR "  start dir - directory to change to before starting search\n";
    print STDERR "  out dir - destination directory to write manifes.xml to\n\n";
    exit 1;
}

if ( $#ARGV != 3 ) { usage(); };

my $manifest_head = $ARGV[0];
my $start_dir = $ARGV[1];
my $dynamic_dir = $ARGV[2];
my $out_dir = $ARGV[3];

print "################################################\n";
print "#                                              #\n";
print "# just a prototype - for testing purpose only! #\n";
print "#                                              #\n";
print "################################################\n\n";


# Traverse desired filesystems
my $work_dir = cwd();
chdir $start_dir or die "$0: ERROR - cannot change directory to \"$start_dir\"\n";
File::Find::find({wanted => \&wanted}, $dynamic_dir);
chdir $work_dir or die "$0: ERROR - oops... cannot change dir to where i came from!\n";

open (HEAD, "$manifest_head") or die "$0: ERROR - Cannot open $manifest_head\n";
my @headlines = <HEAD>;
close HEAD;
chomp @headlines;
chomp @findlist;

my @bodylines;
my @taillines = ("</manifest:manifest>");

foreach my $i (@findlist) {
    if ($i =~ m/^.*\.xcu\z/s) {
        push @bodylines, " <manifest:file-entry manifest:media-type=\"application/vnd.sun.star.configuration-data\"";
    } else {
        push @bodylines, " <manifest:file-entry manifest:media-type=\"application/vnd.sun.star.configuration-schema\"";
    }
    push @bodylines, "              manifest:full-path=\"$i\"/>";
}

open (MANIOUT,">$out_dir/manifest.xml") or die "$0: ERROR - cannot open \"$out_dir/manifest.xml\" for writing.\n";
binmode MANIOUT;

foreach my $j (@headlines, @bodylines, @taillines) {
    print MANIOUT "$j\n";
}

close MANIOUT;

