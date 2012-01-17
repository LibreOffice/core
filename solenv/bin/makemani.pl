#! /usr/bin/perl -w
    eval 'exec /usr/bin/perl -S $0 ${1+"$@"}'
        if 0; #$running_under_some_shell
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
#    && ( push @findlist, $name ) && print("$name\n");
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

