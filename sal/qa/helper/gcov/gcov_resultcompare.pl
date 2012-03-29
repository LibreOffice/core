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
#
# $Id: gcov_resultcompare.pl,v 1.2 2004-03-19 14:46:51 obo Exp $
#

# GCOV_RESULTCOMPARE
#
# Helper, to compare two different results
#
# Q: Why perl?
# A: regexp ;-)
#

use strict;
use File::Basename;
use Getopt::Long;

our $version_info = 'gcov_resultcompare $Revision: 1.2 $ ';

our $help;                    # Help option flag
our $version;                 # Version option flag
# our $infile;

our $orig;
our $compare;

# Prototypes
sub print_usage(*);
sub read_gcov_function_file($);

# Parse command line options
if (!GetOptions(
                "o=s" => \$orig,
                "c=s" => \$compare,
                 "help"   => \$help,
                 "version" => \$version
                 ))
{
    print_usage(*STDERR);
    exit(1);
}

# Check for help option
if ($help)
{
    print_usage(*STDOUT);
    exit(0);
}

# Check for version option
if ($version)
{
    print("$version_info\n");
    exit(0);
}

# check if enough parameters
# if ($#ARGV < 1)
# {
#     print("No input filenames specified\n");
#     print_usage(*STDERR);
#     exit(1);
# }

if (! $orig)
{
    print_usage(*STDOUT);
    exit(0);
}
if (! $compare)
{
    print_usage(*STDOUT);
    exit(0);
}

# ------------------------------------------------------------------------------

my %origlist = read_gcov_function_file($orig);
my %cmplist = read_gcov_function_file($compare);

my $key;
my $value;

while (($key, $value) = each %origlist)
{
    my $cmpvalue = $cmplist{$key};

    if ($cmpvalue != 0.00)
    {
        if ($value < 100.00)
        {
            if ($cmpvalue > $value && $value < 90.0)
            {
                print "$key, $value,   CMP:$cmpvalue\n";
            }
        }
    }
}

# --------------------------------------------------------------------------------
# Read the gcov function (gcov -f) file
# and compare line by line with the export function list
# so we get a list of functions, which are only exported, and not all stuff.

sub read_gcov_function_file($)
{
    local *INPUT_HANDLE;
    my $file = shift;
    my %list;
    my $line = "";

    open(INPUT_HANDLE, $file)
        or die("ERROR: cannot open $file!\n");

    while ($line = <INPUT_HANDLE>)
    {
        chomp($line);
        # sample line (for reg exp:)
        # 100.00 rtl_ustr_toDouble
        if ($line =~ /^(.{6}) (\w+)$/ )
        {
            my $percent = $1;
            my $value = $2;

            $list{$value} = $percent;
        }
    }
    close(INPUT_HANDLE);
    return %list;
}

# ----------------------------------------------------------------------------
sub print_usage(*)
{
    local *HANDLE = $_[0];
    my $tool_name = basename($0);

    print(HANDLE <<END_OF_USAGE);

Usage: $tool_name [OPTIONS] INPUTFILE

    -o                      Original File, which gives the main values
                            if here a value is smaller than in compare, the found value is a candidate for better check.
    -c                      Compare file.

    -h, --help              Print this help, then exit
    -v, --version           Print version number, then exit

END_OF_USAGE
    ;
}
