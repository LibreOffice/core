#!/usr/bin/perl -w
# *************************************************************
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
# *************************************************************

my @global_list = ();
my %global_hash = ();
my $base_path;

sub read_icons($)
{
    my $fname = shift;
    my $fileh;
    my @images;
    open ($fileh, "$base_path/$fname") || die "Can't open $base_path/$fname: $!";
    while (<$fileh>) {
    m/xlink:href=\"\.uno:(\S+)\"\s+/ || next;
    push @images, lc($1);
    }
    close ($fileh);

    return @images;
}

# filter out already seen icons & do prefixing
sub read_new_icons($$)
{
    my $fname = shift;
    my $prefix = shift;
    my @images = read_icons ($fname);
    my @new_icons;
    my %new_icons;
    for my $icon (@images) {
    my $iname = "res/commandimagelist/" . $prefix . $icon . ".png";
    if (!defined $global_hash{$iname} &&
        !defined $new_icons{$iname}) {
        push @new_icons, $iname;
        $new_icons{$iname} = 1;
    }
    }
    return @new_icons;
}

sub process_group($@)
{
    my $prefix = shift;
    my @uiconfigs = @_;
    my %group;
    my $cur_max = 1.0;

# a very noddy sorting algorithm
    for my $uiconfig (@uiconfigs) {
    my @images = read_new_icons ($uiconfig, $prefix);
    my $prev = '';
    for my $icon (@images) {
        if (!defined $group{$icon}) {
        if (!defined $group{$prev}) {
            $group{$icon} = $cur_max;
            $cur_max += 1.0;
        } else {
            $group{$icon} = $group{$prev} + (1.0 - 0.5 / $cur_max);
        }
        } # else a duplicate
    }
    }
    for my $icon (sort { $group{$a} <=> $group{$b} } keys %group) {
    push @global_list, $icon;
    $global_hash{$icon} = 1;
    }
}

sub process_file($$)
{
    my @images = read_new_icons (shift, shift);

    for my $icon (@images) {
    push @global_list, $icon;
    $global_hash{$icon} = 1;
    }
}

sub chew_controlfile($)
{
    my $fname = shift;
    my $fileh;
    my @list;
    open ($fileh, $fname) || die "Can't open $fname: $!";
    while (<$fileh>) {
    /^\#/ && next;
    s/[\r\n]*$//;
    /^\s*$/ && next;

    my $line = $_;
    if ($line =~ s/^-- (\S+)\s*//) {
        # control code
        my $code = $1;
        my $small = (lc ($line) eq 'small');
        if (lc($code) eq 'group') {
        if (!$small) { process_group ("lc_", @list); }
        process_group ("sc_", @list);
        } elsif (lc ($code) eq 'ordered') {
        if (!$small) {
            for my $file (@list) { process_file ($file, "lc_"); }
        }
        for my $file (@list) { process_file ($file, "sc_"); }
        } elsif (lc ($code) eq 'literal') {
        for my $file (@list) {
            if (!defined $global_hash{$file}) {
            push @global_list, $file;
            $global_hash{$file} = 1;
            }
        }
        } else {
        die ("Unknown code '$code'");
        }
        @list = ();
    } else {
        push @list, $line;
    }
    }
    close ($fileh);
}

if (!@ARGV) {
    print "image-sort <image-sort.lst> /path/to/OOOo/source/root\n";
    exit 1;
}

# where the control file lives
my $control = shift @ARGV;
# where the uiconfigs live
$base_path = shift @ARGV;
# output
if (@ARGV) {
    my $outf = shift @ARGV;
    open ($output, ">$outf") || die "Can't open $outf: $!";
    $stdout_out = 0;
} else {
    $output = STDOUT;
    $stdout_out = 1;
}

chew_controlfile ($control);

for my $icon (@global_list) {
    print $output $icon . "\n" if (!($icon =~ /^sc_/));
}
for my $icon (@global_list) {
    print $output $icon . "\n" if ($icon =~ /^sc_/);
}

close $output if (!$stdout_out);
