#!/usr/bin/perl -w
#
# $Id: gcov_filter.pl,v 1.4 2005-11-02 17:23:57 kz Exp $
#
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

# GCOV_FILTER
#
# Helper to filter the gcov output.
# Handle a compare between the hole gcov output and a given select list of exported functions.
#
# Q: Why perl?
# A: regexp ;-)
#


use strict;
use File::Basename;
use Getopt::Long;

# Global constants
our $version_info = 'gcov helper $Revision: 1.4 $ ';
our $help;                    # Help option flag
our $version;                 # Version option flag
our $cwd = `pwd`;             # current working directory
chomp($cwd);
# our $tool_dir = dirname($0);
# our $tool_file = basename($0);

# our $output_filename;
our $input_allfunc;
# our $input_filename;

our $allfuncinfo;             # allfuncinfo option flag
our $showallfunc;             # showallfunc option flag
our $no_percentage;           # no_percentage option flag
our $donotfilter;             # donotfilter option flag
our $objectdir;

# Prototypes
sub print_usage(*);
sub read_gcov_function_file($);
sub get_PRJ_from_makefile_mk();
# sub read_ExportedFunctionList();
sub read_List($);

# if (! ($tool_dir =~ /^\/(.*)$/))
# {
#     $tool_dir = "$cwd/$tool_dir";
# }

# Parse command line options
if (!GetOptions( "input-allfunc=s" => \$input_allfunc,
                 "allfuncinfo" => \$allfuncinfo,
                 "showallfunc" => \$showallfunc,
                 "no-percentage" => \$no_percentage,
                 "do-not-filter" => \$donotfilter,
                 "objectdir=s" => \$objectdir,
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

# special case:
# the filename contains a . or '/' at the beginning
my $startdir = $ARGV[0];
if ( ($startdir =~ /^\.\./) ||
     ($startdir =~ /^\//) )
{
    $startdir = dirname($startdir);
    # print("start directory is $startdir\n");
    # chdir $startdir;
}
else
{
    $startdir = "";
}

# check for sal.map
if ( ! $input_allfunc )
{
    # this is a try, to get the project directory form a found makefile.mk
    # may work, but fails due to some disunderstandings may be problems in perl :-(
    my $sDir = get_PRJ_from_makefile_mk();
    # chomp($sDir);
    # HDW: print("PRJ is $dir\n");

    # $sDir2 = "../..";
    my $sMapFile = "$sDir" . "/util/sal.map";
    # $sMapFile = "$sDir2" . "/util/sal.map";

    if ( -e $sMapFile)
    {
        $input_allfunc = $sMapFile;
    }
    else
    {
        print("No input-allfunc filename specified\n");
        print_usage(*STDERR);
        exit(1);
    }
}
our @aDeprecatedList;
our @aExportedFunctionList;
# read_ExportedFunctionList();
@aExportedFunctionList = read_List($input_allfunc);
@aDeprecatedList = read_List("deprecated.txt");

if ($allfuncinfo)
{
    print("Count of all functions: $#aExportedFunctionList\n");
    exit(0);
}

if ($showallfunc)
{
    my $func;
    foreach $func (@aExportedFunctionList)
    {
        print("$func\n");
    }
    exit(0);
}

# back to current directory
# this chdir was for a before chdir (in $startdir creation) but due to the fact,
# that the get_PRJ_from_makefile_mk works but the after concat of strings not, this
# chdir is also remarked.
# chdir $cwd;

# HWD: print "count of param: \n";
# $input_filename = $ARGV[0];

my $nCount = $#ARGV + 1;
my $nIdx;

for ($nIdx = 0; $nIdx < $nCount ; ++$nIdx)
{
    my $file = $ARGV[$nIdx];
    # print("processing: $file\n");

    # change directory, to the current file, due to the fact, that we may be need to call gcov
    # and gcov will create some extra files, like *.gcov near the current file.
    # if ( $startdir ne "" )
    # {
    #     chdir $startdir;
    #     $file = basename($file);
    # }

    my $OBJECTS="";
    if ($objectdir)
    {
        $OBJECTS = "-o " . $objectdir;
    }

    if (! ($file =~ /\.f$/ ))
    {
        my $filef = "$file.f";
        # if (! -e $filef )
        # {
        # print "gcov $OBJECTS -l -f $file >$filef\n";
        my $blah = `gcov $OBJECTS -n -f $file >$filef`;
        # }
        $file = $filef;
    }
    read_gcov_function_file($file);

    # go back to old directory, because it's possible to change relative to an other directory.
    if ( $startdir ne "" )
    {
        chdir $cwd;
    }
}

# print "$tool_dir\n";
# print "all is right\n";
exit(0);


# --------------------------------------------------------------------------------
# Read the map file, which should contain all exported functions.
sub read_List($)
{
    local *INPUT_HANDLE;
    my $filename = $_[0];
    my @list;
    my $line = "";
    open(INPUT_HANDLE, $filename);
        # or die("ERROR: cannot open $filename!\n");

    while ($line = <INPUT_HANDLE>)
    {
        chomp($line);
        # reg exp: [spaces]functionname[semicolon][line end]
        if ($line =~ /^\s+(\w*);$/)
        {
            # print("$1\n");
            push(@list, $1);
        }
    }
    close(INPUT_HANDLE);
    return @list;
}

# --------------------------------------------------------------------------------
# Helper function, test is a given value is found in the global exported function list.
# the given value format could be a simple function name
# or a prototype
# e.g.
# osl_getSystemPathFromFileURL
# or
# void getSystemPathFromFileURL( const char* rtl_...)
#
sub contain_in_List($$)
{
    my $func;
    my $value = $_[0];
    my $list = $_[1];

    if ($donotfilter)
    {
        return $value;
    }

    foreach $func (@$list) # (@aExportedFunctionList)
    {
        # first try, direct check
        if ($value eq $func)
        {
            # HWD: print("$value eq $func\n");
            return $value;
        }

        # value not found, second try, may be we found it if we search word wise.
        # helper, to insert a space after the word, before '('
        $value =~ s/\(/ \(/g;
        # may be here we should replace all white spaces by ' '

        # split by 'space'
        my @list = split(' ', $value);
        for(@list)
        {
            # HWD: print "$list[$n]\n";
            if ($_ eq $func)
            {
                # HWD: print ("found $func in $value\n");
                return $_;
            }
        }
    }
    # not found
    return "";
}
# --------------------------------------------------------------------------------
# Read the gcov function (gcov -f) file
# and compare line by line with the export function list
# so we get a list of functions, which are only exported, and not all stuff.
# sample of output
# new gcov gcc 3.4 format
sub read_gcov_function_file($)
{
    local *INPUT_HANDLE;
    my $file = $_[0];
    my $line = "";
    open(INPUT_HANDLE, $file)
        or die("ERROR: cannot open $file!\n");

    while ($line = <INPUT_HANDLE>)
    {
        chomp($line);
        # sample line (for reg exp:)
        # 100.00% of 3 source lines executed in function osl_thread_init_Impl
        if ($line =~ /^Function \`(.*)\'$/ )
        {
            my $sFunctionName = $1;
            my $sPercentage;
            $line = <INPUT_HANDLE>;
            if ($line =~ /^Lines executed:(.*)% of/ )
            {
                $sPercentage = $1;
            }
            my $value = contain_in_List( $sFunctionName, \@aExportedFunctionList );
            if ($value)
            {
                my $isDeprecated = contain_in_List( $sFunctionName, \@aDeprecatedList );
                if ($isDeprecated)
                {
                    # Function is deprecated, do not export it.
                }
                else
                {
                    if ($no_percentage)
                    {
                        print("$value\n");
                    }
                    else
                    {
                        print("$sPercentage $value\n");
                    }
                }
            }
            # push(@aExportedFunctionList, $1);
        }
    }
    close(INPUT_HANDLE);
}

# gcov format since gcc 3.3.6
# 100.00% von 3 Zeilen in function helloworld ausgeführt
# 100.00% von 5 Zeilen in function main ausgeführt
# 100.00% von 8 Zeilen in file tmp.c ausgeführt
sub read_gcov_function_file_old_gcc_3($)
{
    local *INPUT_HANDLE;
    my $file = $_[0];
    my $line = "";
    open(INPUT_HANDLE, $file)
        or die("ERROR: cannot open $file!\n");

    while ($line = <INPUT_HANDLE>)
    {
        chomp($line);
        # sample line (for reg exp:)
        # 100.00% of 3 source lines executed in function osl_thread_init_Impl
        if ($line =~ /^(.*)% of \d+ source lines executed in function (.*)$/ )
        {
            my $value = contain_in_List( $2, \@aExportedFunctionList );
            if ($value)
            {
                my $isDeprecated = contain_in_List( $2, \@aDeprecatedList );
                if ($isDeprecated)
                {
                    # Function is deprecated, do not export it.
                }
                else
                {
                    if ($no_percentage)
                    {
                        print("$value\n");
                    }
                    else
                    {
                        print("$1 $value\n");
                    }
                }
            }
            # push(@aExportedFunctionList, $1);
        }
    }
    close(INPUT_HANDLE);
}

# ------------------------------------------------------------------------------
# helper, to read the PRJ value out of a makefile.mk file
sub get_PRJ_from_makefile_mk()
{
    local *INPUT_HANDLE;
    # my $startdir = @_[0];
    my $line = "";
    my $value = "";
    open(INPUT_HANDLE, "makefile.mk")
        or die("ERROR: cannot open makefile.mk\n");

    while ($line = <INPUT_HANDLE>)
    {
        chomp($line);
        # sample line
        # PRJ=
        # HWD: print("$line\n");
        if ($line =~ /^PRJ\s*=(.*)\s*$/)
        {
            # HWD: print("FOUND #####\n");
            $value = $1;
            chomp($value);
            $value =~ s/\$\//\//g;
        }
    }
    close(INPUT_HANDLE);
    return $value;
}

# ----------------------------------------------------------------------------
sub print_usage(*)
{
    local *HANDLE = $_[0];
    my $tool_name = basename($0);

    print(HANDLE <<END_OF_USAGE);

Usage: $tool_name [OPTIONS] INPUTFILE

    -h, --help                     Print this help, then exit
    -v, --version                  Print version number, then exit
    -i, --input-allfunc FILENAME   Map file, which contain all exported functions
    -s, --showallfunc              Shows all exported functions then exit
    -a, --allfuncinfo              Shows the count of all exported functions then quit
    -n, --no-percentage            Suppress the output of the percent value for tested functions
    -d, --do-not-filter            Show all functions, which gcov -f produce

END_OF_USAGE
    ;
}

