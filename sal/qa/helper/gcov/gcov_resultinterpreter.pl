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
# $Id: gcov_resultinterpreter.pl,v 1.3 2005-11-02 17:24:12 kz Exp $
#

# GCOV_RESULTINTERPRETER
#
# Helper, to interpret the result
#
# Q: Why perl?
# A: regexp ;-)
#

use strict;
use File::Basename;
use Getopt::Long;

our $version_info = 'gcov helper $Revision: 1.3 $ ';

our $help;                    # Help option flag
our $version;                 # Version option flag
# our $infile;

our $usedFunctions;     # show all functions, which have a value > 0
our $nonusedFunctions;  # show all functions, which have a value == 0
our $nPercent;          # show all functions, which have a value > $nPercent
our $complete;          # show all functions, which have a value == 100
our $incomplete;       # show all functions, which have a value > 0 && < 100

# Prototypes
sub print_usage(*);
sub read_gcov_function_file($);

# Parse command line options
if (!GetOptions(
                "usedfunctions" => \$usedFunctions,
                "nonusedfunctions" => \$nonusedFunctions,
                "percent=s" => \$nPercent,
                "complete" => \$complete,
                "incomplete" => \$incomplete,
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
if ($#ARGV < 0)
{
    print("No input filename specified\n");
    print_usage(*STDERR);
    exit(1);
}

if ($complete)
{
    $nPercent = 100.00;
}
# ------------------------------------------------------------------------------

my %list = read_gcov_function_file($ARGV[0]);

my $key;
my $value;

while (($key, $value) = each %list)
{
    # print "function: $key = $value\n";
    if ($nonusedFunctions)
    {
        if ($value <= 0.00)
        {
            print "$key\n";
        }
    }
    elsif ($usedFunctions)
    {
        if ($value != 0.00)
        {
            print "$key, $value\n";
        }
    }
    elsif ($nPercent)
    {
        if ($value >= $nPercent)
        {
            print "$key, $value\n";
        }
    }
    elsif ($incomplete)
    {
        if ($value > 0.00 && $value < 100.00)
        {
            print "$key, $value\n";
        }
    }
    else
    {
        print "$key, $value\n";
    }
}

# --------------------------------------------------------------------------------
# Read the gcov function (gcov -f) file
# and compare line by line with the export function list
# so we get a list of functions, which are only exported, and not all stuff.

sub read_gcov_function_file($)
{
    local *INPUT_HANDLE;
    my $file = $_[0];
    my %list;
    my $line = "";

    open(INPUT_HANDLE, $file)
        or die("ERROR: cannot open $file!\n");

    while ($line = <INPUT_HANDLE>)
    {
        chomp($line);
        # sample line (for reg exp:)
        # 100.00 rtl_ustr_toDouble
        if ($line =~ /^(.*) (\w+)$/ )
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

    -u, --usedFunctions     show all functions, which have a value > 0
    -n, --nonusedFunctions  show all functions, which have a value == 0
    -p, --percent           show all functions, which have a value > percent
    -c, --complete          show all functions, which have a value == 100
    -i, --incomplete        show all functions, which have a value > 0 && < 100

    -h, --help              Print this help, then exit
    -v, --version           Print version number, then exit

END_OF_USAGE
    ;
}
