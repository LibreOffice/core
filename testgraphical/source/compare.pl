eval 'exec perl -wS $0 ${1+\"$@\"}'
    if 0;

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

BEGIN
{
    #       Adding the path of this script file to the include path in the hope
    #       that all used modules can be found in it.
    $0 =~ /^(.*)[\/\\]/;
    push @INC, $1;
    # print "PATH: " . $1 . "\n";
}

# my $e;
# foreach $e (keys %ENV)
# {
#     print "$e := $ENV{$e}" . "\n";
# }

use strict;
use graphical_compare;
use ConvwatchHelper;
use filehelper;
use timehelper;
use loghelper;

use Cwd;
use File::Basename;
use Getopt::Long;
use English;                  # $OSNAME, ...
use File::Path;
use Cwd 'chdir';

our $help;                    # Help option flag
our $version;                 # Version option flag
# our $test;

# our $MAJOR;
# our $MINOR;
# our $cwsname;
our $pool;
our $document;
our $creatortype;
our $prepareonly = 0;
our $force;
our $verbose = 0;
our $show = 0;
our $connectionstring;

# Prototypes
sub print_usage(*);
sub prepare();
sub CompareFiles($$);

# flush STDOUT
# my $old_handle = select (STDOUT); # "select" STDOUT and save # previously selected handle
# $| = 1; # perform flush after each write to STDOUT
# select ($old_handle); # restore previously selected handle

$OUTPUT_AUTOFLUSH=1; # works only if use English is used.

our $version_info = 'compare.pl';

GetOptions(
#            "MAJOR=s"       => \$MAJOR,
#            "MINOR=s"       => \$MINOR,
#            "cwsname=s"     => \$cwsname,
           "pool=s"        => \$pool,
           "document=s"    => \$document,
           "creatortype=s" => \$creatortype,
           "prepareonly=s" => \$prepareonly,
           "connectionstring=s" => \$connectionstring,

           "force"         => \$force,
           "verbose"       => \$verbose,
           "show"          => \$show,

#           "test"          => \$test,
           "help"          => \$help,
           "version"       => \$version
           );

if ($help)
{
    print_usage(*STDOUT);
    exit(0);
}
# Check for version option
if ($version)
{
    print STDERR "$version_info\n";
    exit(0);
}

if ($prepareonly)
{
    $force=1;
}

prepare();
if ($connectionstring)
{
    setConnectionString($connectionstring);
}

my $sDocumentPool = appendPath(getProjectRoot(), "document-pool");
# print "ProjectRoot: " . getProjectRoot() . "\n";
if ($ENV{DOCUMENTPOOL})
{
    if ( -d $ENV{DOCUMENTPOOL})
    {
        print "overwrite default Documentpool: '$sDocumentPool'\n";
        print "       with \$ENV{DOCUMENTPOOL}: $ENV{DOCUMENTPOOL}\n";
        $sDocumentPool = $ENV{DOCUMENTPOOL};
    }
    else
    {
        print "Given \$DOCUMENTPOOL doesn't exist.\n";
    }
}

my $err = 0;
my $nCompareTime = getTime();

# if we want to check one file, give -pool and -document
# if we want to check the whole pool, give -pool
# if we want to check all, call without parameters
if ($pool)
{
    if ($document)
    {
        $err = SingleDocumentCompare( $sDocumentPool,
                                      $pool,
                                      $document,
                                      $creatortype,
                                      $prepareonly,
                                      $show
                                     );
    }
    else
    {
        $err = CompareFiles($sDocumentPool, $pool);
    }
}
else
{
    local *DIR;
    if (opendir (DIR, $sDocumentPool))           # Directory oeffnen
    {
        my $myfile;
        while ($myfile = readdir(DIR))
        {                                        # ein filename holen
            if ($myfile eq "." ||
                $myfile eq "..")
            {
                next;
            }
            my $sDocumentPath = appendPath($sDocumentPool, $myfile);
            if ( -d $sDocumentPath )
            {
                $err += CompareFiles($sDocumentPool, $myfile);
            }
            elsif ( -f $sDocumentPath )
            {
                print "Warning: the file '$myfile' will not compared.\n";
            }
        }
        closedir(DIR);
    }
    # my $sPool = "eis-chart";
    # $err += CompareFiles($sDocumentPool, "eis-chart");
    # $err += CompareFiles($sDocumentPool, "eis-impress");
    # $err += CompareFiles($sDocumentPool, "eis-writer");
    # $err += CompareFiles($sDocumentPool, "eis-calc");

}

printTime(endTime($nCompareTime));
exit ($err);

# ------------------------------------------------------------------------------

sub CompareFiles($$)
{
    my $sDocumentPath = shift;
    my $sPool = shift;
    my %aFailedHash;
    my $startdir = appendPath($sDocumentPath, $sPool);

    local *DIR;
    if (opendir (DIR, $startdir))           # Directory oeffnen
    {
        my $myfile;
        while ($myfile = readdir(DIR))
        {                                  # ein filename holen
            if ($myfile eq "knownissues.xcl")
            {
                next;
            }
            my $sAbsoluteFile = appendPath($startdir, $myfile);
            if (-f $sAbsoluteFile)
            {
                my $nIssue;
                my $sIssueText;
                ($nIssue, $sIssueText) = checkForKnownIssue($startdir, $myfile);
                if ($nIssue == 0)
                {
                    $err = SingleDocumentCompare(
                                             # "/net/so-gfxcmp-documents.germany.sun.com/export/gfxcmp/document-pool", # $documentpoolpath,
                                             $sDocumentPool,
                                             $sPool,                  # $documentpool,
                                             $myfile,                 # $documentname);
                                             $creatortype,            # $destinationcreatortype,
                                             $prepareonly,
                                             $show
                                             );
                    $aFailedHash{$myfile} = $err;
                }
                else
                {
                    print "$myfile [KNOWN ISSUE: #$sIssueText#]\n";
                }
            }
        }
        closedir(DIR);
    }

    print "***** State for graphical compare of pool: '$sPool' ******\n";
    my $nErrorCount = 0;
    my $file;
    foreach $file (keys %aFailedHash)
    {
        if ($aFailedHash{$file} != 0)
        {
            print "Failed: $file\n";
            $nErrorCount++;
        }
    }
    print "Whole unit: ";
    if ($nErrorCount > 0)
    {
        print "PASSED.FAILED\n";
    }
    else
    {
        print "PASSED.OK\n";
    }
    print "************************************************************\n";
    return $nErrorCount;
}
# ------------------------------------------------------------------------------
# return issue number if file exists in knownissues.xcl file
sub checkForKnownIssue($$)
{
    my $startdir = shift;
    my $myfile = shift;

    if ($force)
    {
        return 0,"";
    }

    my $sKnownIssueFile = appendPath($startdir, "knownissues.xcl");
    my $sIssueText = "unknown";
    local *ISSUES;
    my $nIssue = 0;
    my $sCurrentSection;

    if (open(ISSUES, $sKnownIssueFile))
    {
        my $line;
        while ($line = <ISSUES>)
        {
            chomp($line);
            if ($line =~ /\[(.*)\]/ )
            {
                $sCurrentSection = $1;
                next;
            }
            if ($sCurrentSection eq $creatortype)
            {
                if ($line =~ /\#\#\# (.*) \#\#\#/ )
                {
                    $sIssueText = $1;
                }
                if ($line =~ /^${myfile}$/ )
                {
                    $nIssue = 1;
                    last;
                }
            }
        }
        close(ISSUES);
    }
    return $nIssue, $sIssueText;
}
# ------------------------------------------------------------------------------
sub prepare()
{
    # directory structure:
    # documents will be found in
    # ../document-pool/eis-tests

    # references will be found in
    # ../references/unxlngi/eis-tests
    # ../references/wntmsci/eis-tests

    # output goes to
    # ../unxlngi6.pro/misc

    if ($verbose)
    {
        setVerbose();
    }

    # TEST
    if (!$ENV{INPATH})
    {
        if ($OSNAME eq "linux")
        {
            # just for debug
            setINPATH("unxlngi6.pro");
        }
    }
    else
    {
        setINPATH($ENV{INPATH});
    }

    if (! $creatortype)
    {
        $creatortype= "ps";
    }

    my $cwd = getcwd();
    print "Current Directory: $cwd\n" if ($verbose);
    my $sProjectBase;
    if ($ENV{PRJ})
    {
        # print "cwd:=$cwd\n";
        # print "PRJ:=$ENV{PRJ}\n";
        $sProjectBase = appendPath($cwd, $ENV{PRJ});
    }
    else
    {
        $sProjectBase = dirname($cwd);
    }
    if ($OSNAME eq "cygwin")
    {
        $sProjectBase = `cygpath -w $sProjectBase`;
        chomp($sProjectBase);
        $sProjectBase = unixpath($sProjectBase);
        # print "cygwin patch \$sProjectBase := $sProjectBase\n";
    }
    # print "Project base path: $sProjectBase\n";
    setProjectRoot($sProjectBase);


    # TEST TEST TEST
    # exit (0);
}
# ------------------------------------------------------------------------------
sub print_usage(*)
{
    local *HANDLE = $_[0];
    my $tool_name = basename($0);

    print(HANDLE <<END_OF_USAGE);

Usage: $tool_name [OPTIONS]

    -pool                    Give pool name out of document-pool directory.
                             But all documents list in knownissues.xcl will not check.
    -document                Give a single document to test, the known issue list will ignored.
    -creatortype=s           s:ps  create postscript files via print to file.
                             s:pdf create PDF file via export to pdf.
    -h, --help               Print this help, then exit
    -v, --version            Print version number, then exit

END_OF_USAGE
;
}
