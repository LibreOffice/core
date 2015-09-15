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
# $Id: gcov_result.pl,v 1.2 2003-06-11 16:36:30 vg Exp $
#

# GCOV_RESULT
#
# Helper, to interpret the result and put the result via html in a database.
# Put into DB works via php.
#
# Q: Why perl?
# A: regexp ;-)
#

use strict;
use File::Basename;
use Getopt::Long;
use Time::localtime;

our $version_info = 'gcov helper $Revision: 1.2 $ ';

our $help;                    # Help option flag
our $version;                 # Version option flag
# our $infile;

our $usedFunctions;     # name of all functions filename, which have a value > 0
our $nonusedFunctions;  # name of all functions filename, which have a value == 0
our $complete;          # name of all functions filename, which have a value == 100
our $incomplete;       # name of all functions filename, which have a value > 0 && < 100

our $environment;
our $major;
our $minor;
our $cwsname;
our $outputDir;

# Prototypes
sub print_usage(*);
sub read_gcov_function_file($);
sub create2DigitNumber($);

# Parse command line options
if (!GetOptions(
                 "help"    => \$help,
                 "version" => \$version,

                 "usedfunctions=s"    => \$usedFunctions,
                 "nonusedfunctions=s" => \$nonusedFunctions,
                 "complete=s"         => \$complete,
                 "incomplete=s"       => \$incomplete,
                 "cwsname=s"          => \$cwsname,
                 "major=s"            => \$major,
                 "minor=s"            => \$minor,
                 "environment=s"      => \$environment,
                 "outputdir=s"        => \$outputDir
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
# if ($#ARGV < 0)
# {
#     print("No input filename specified\n");
#     print_usage(*STDERR);
#     exit(1);
# }

# ------------------------------------------------------------------------------

my $sURL = "http://mahler.germany.sun.com/qadev/baselib/gcov_result_in_db_putter.php";

my $next = "?";

if ($complete)
{
    my $result = `cat $complete | wc -l`;
    chomp($result);
    $result =~ / *(\d+)/;
    $sURL = $sURL . "$next" . "complete=$1";
    $next = "&";
}

if ($nonusedFunctions)
{
    my $result = `cat $nonusedFunctions | wc -l`;
    chomp($result);
    $result =~ / *(\d+)/;
    $sURL = $sURL . "$next" . "notused=$1";
    $next = "&";
}
if ($usedFunctions)
{
    my $result = `cat $usedFunctions | wc -l`;
    chomp($result);
    $result =~ / *(\d+)/;
    $sURL = $sURL . "$next" . "used=$1";
    $next = "&";
}
if ($incomplete)
{
    my $result = `cat $incomplete | wc -l`;
    chomp($result);
    $result =~ / *(\d+)/;
    $sURL = $sURL . "$next" . "incomplete=$1";
    $next = "&";
}

if ($cwsname)
{
    # qadev8
    $sURL = $sURL . "$next" . "cwsname=$cwsname";
    $next = "&";
}
if ($major)
{
    # srx645
    $sURL = $sURL . "$next" . "major=$major";
    $next = "&";
}
if ($minor)
{
    # m3s1
    $sURL = $sURL . "$next" . "minor=$minor";
    $next = "&";
}

if ($environment)
{
    # unxlngi5
    $sURL = $sURL . "$next" . "environment=$environment";
    $next = "&";
}

my $year  = localtime->year() + 1900;
my $month = create2DigitNumber(localtime->mon() + 1);
my $day   = create2DigitNumber(localtime->mday());
$sURL = $sURL . "$next" . "date=$year-$month-$day";
$next = "&";

my $output;
if ($outputDir)
{
    chomp($outputDir);
    $output = $outputDir;
}

# check if output ends with "/"
if ( $output =~ /\/$/ )
{
    print "Output name ends with '/'\n";
}
else
{
    print "Output name does not end with '/'\n";
    $output = $output . "/";
}
$output = $output . "php_result.txt";

my $result = `wget -O $output "$sURL"`;
print "$sURL\n";

print `cat $output`;


# ----------------------------------------------------------------------------
sub print_usage(*)
{
    local *HANDLE = $_[0];
    my $tool_name = basename($0);

    print(HANDLE <<END_OF_USAGE);

Usage: $tool_name [OPTIONS]

    -u,  --usedfunctions     count of all functions, which have a value > 0
    -n,  --nonusedfunctions  count of all functions, which have a value == 0
    -co, --complete          count of all functions, which have a value == 100
    -i,  --incomplete        count of all functions, which have a value > 0 && < 100

    -cw, --cwsname           set cwsname
    -ma, --major             set major number
    -mi, --minor             set minor number
    -e,  --environment       set environment

    -o,  --outputdir         set the directory, where to store the wget result

    -h, --help               Print this help, then exit
    -v, --version            Print version number, then exit

END_OF_USAGE
    ;
}
# ------------------------------------------------------------------------------
sub create2DigitNumber($)
{
    my $digit = $_[0];
    my $str;
    my $nDigitLen = length $digit;

    if ($nDigitLen == 1)
    {
        $str = "0" . $digit;
    }
    else
    {
        if ($nDigitLen > 2)
        {
            $str = substr $digit, $nDigitLen - 2, 2;
        }
        else
        {
            $str = $digit;
        }
    }
    return $str;
}
