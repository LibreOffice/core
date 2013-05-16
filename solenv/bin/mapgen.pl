:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
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

#
# mapgen  - generate a map file for Unix libraries
#

#use File::Path;
#use File::Copy;

#### script id #####

( $script_name = $0 ) =~ s/^.*\b(\w+)\.pl$/$1/;
print "$script_name -- version: 1.6\n";
print "Multi Platform Enabled Edition\n";

#########################
#                       #
#   Globale Variablen   #
#                       #
#########################

$dump_file = '';
$flt_file = '';
$map_file = '';
$first_string = '';
$tab = '    ';

#### main ####

&get_options;
if (!(open (DUMP_FILE, $dump_file))) {
    &print_error("Unable open $dump_file");
};
if (!(open (FLT_FILE, $flt_file))) {
    close DUMP_FILE;
    &print_error("Unable open $flt_file");
};
unlink $map_file;
if (!(open (MAP_FILE, ">>$map_file"))) {
    close DUMP_FILE;
    close FLT_FILE;
    &print_error("Unable open $map_file");
};

if ($ENV{OS} eq 'SOLARIS') {
    &gen_sol;
} elsif ($ENV{OS} eq 'LINUX') {
    &gen_lnx;
} else {
    &print_error ('Environment not set!!');
};

close DUMP_FILE;
close FLT_FILE;
close MAP_FILE;

#### end of main procedure ####

#########################
#                       #
#      Procedures       #
#                       #
#########################

#
# Generate a map file for solaris
#
sub gen_sol {
    my %symbols = ();
    foreach (<DUMP_FILE>) {
        next if (!(/\s*(\S+)\s*\|\s*(\S+)\s*\|\s*(\S+)\s*\|\s*(\S+)\s*\|\s*(\S+)\s*\|\s*(\S+)\s*\|\s*(\S+)\s*\|\s*(\S+)\s*/));
        next if (($7 =~ /UNDEF/) || ($7 =~ /ABS/));
        next if ($5 eq 'LOCL');
        $symbols{$8}++;
    };
    &filter_symbols(\%symbols);
};

#
# Generate a map file for linux
#
sub gen_lnx {
    my %symbols = ();
    foreach (<DUMP_FILE>) {
        next if (!(/^\S+ [A|B|C|D|G|I|N|R|S|T|U|V|W|-|\?|-] (\S+)/));
        $symbols{$1}++;
    };
    &filter_symbols(\%symbols);
}

#
# Filter symbols with filters from $flt_file
#
sub filter_symbols {
    my $symbols = shift;
    my $env_section = '';
    my @filters = ();
    my @filtered_symbols = ();
    while (<FLT_FILE>) {
        s/\r//;
        s/\n//;
        $env_section = '1' and next if ((/^# SOLARIS #$/) && ($ENV{OS} eq 'SOLARIS'));
        $env_section = '1' and next if ((/^# LINUX #$/) && ($ENV{OS} eq 'LINUX'));
        $env_section = '1' and next if ((/^# FREEBSD #$/) && ($ENV{OS} eq 'FREEBSD'));
        $env_section = '1' and next if ((/^# NETBSD #$/) && ($ENV{OS} eq 'NETBSD'));
        $env_section = '1' and next if ((/^# OPENBSD #$/) && ($ENV{OS} eq 'OPENBSD'));
        $env_section = '1' and next if ((/^# DRAGONFLY #$/) && ($ENV{OS} eq 'DRAGONFLY'));
        last if ($env_section && ((/^# SOLARIS #$/) || (/^# FREEBSD #$/) || (/^# LINUX #$/) || (/^# NETBSD #$/) || (/^# OPENBSD #$/) (/^# DRAGONFLY #$/)));
        next if (!$_ || /^#/);
        push(@filters, $_);
    };
    foreach my $symbol (keys %$symbols) {
        my $export = '-';
        foreach my $filter_str (@filters) {
            my $add = substr ($filter_str, 0, 1);
            my $filter = substr($filter_str, 1);
            if ($symbol =~ /$filter/) {
                $export = $add;
            };
        };
        if ($export eq '+') {
            push(@filtered_symbols, $symbol);
        };
    };
    &write_mapfile(\@filtered_symbols);
};

#
# Write a map file
#
sub write_mapfile {
    my $symbols = shift;
    print MAP_FILE $first_string . " {\n$tab" . "global:\n";
    foreach (@$symbols) {
        print MAP_FILE "$tab$tab$_\;\n";
    };
    print MAP_FILE "$tab" . "local:\n$tab\*\;\n}\;";
};

#
# Get all options passed
#
sub get_options {

$dump_file = '';
$flt_file = '';
$map_file = '';
    my ($arg);
    &usage() && exit(0) if ($#ARGV == -1);
    while ($arg = shift @ARGV) {
        $arg =~ /^-d$/          and $dump_file = shift @ARGV    and next;
        $arg =~ /^-f$/          and $flt_file = shift @ARGV     and next;
        $arg =~ /^-m$/          and $map_file = shift @ARGV     and next;
        $arg =~ /^-h$/          and &usage                      and exit(0);
        $arg =~ /^--help$/      and &usage                      and exit(0);
        $arg =~ /^-s$/          and $first_string = shift @ARGV and next;
    };
    if (!$dump_file ||
        !$flt_file  ||
        !$first_string  ||
        !$map_file) {
        &usage;
        exit(1);
    };
};

sub print_error {
    my $message = shift;
    print STDERR "\nERROR: $message\n";
    exit(1)
};

sub usage {
    print STDERR "\nmapgen:\n";
    print STDERR "Syntax:    mapgen -d dump_file -s first_string -f filter_file -m map_file [-h|--help]\n";
};

