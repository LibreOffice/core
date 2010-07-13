:
eval 'exec perl -wS $0 ${1+"$@"}'
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

#
# cwstestresult.pl - publish results of CWS tests to EIS
#

use strict;
use Getopt::Long;
use Cwd;

#### module lookup
my @lib_dirs;
BEGIN {
    if ( !defined($ENV{SOLARENV}) ) {
        die "No environment found (environment variable SOLARENV is undefined)";
    }
    push(@lib_dirs, "$ENV{SOLARENV}/bin/modules");
    push(@lib_dirs, "$ENV{COMMON_ENV_TOOLS}/modules") if defined($ENV{COMMON_ENV_TOOLS});
}
use lib (@lib_dirs);

use Cws;

#### global #####
( my $script_name = $0 ) =~ s/^.*\b(\w+)\.pl$/$1/;

my $is_debug = 1;           # enable debug
my $opt_master;             # option: master workspace
my $opt_child;              # option: child workspace
my $opt_milestone;              # option: milestone
my $opt_testrunName;        # option: testrunName
my $opt_testrunPlatform;    # option: testrunPlatfrom
my $opt_resultPage;         # option: resultPage


#### main #####

my $arg_status= parse_options();
testresult($arg_status);
exit(0);

#### subroutines ####

sub testresult
{
    my $status = shift;
    # get master and child workspace
    my $masterws = $opt_master ? uc($opt_master) : $ENV{WORK_STAMP};
    my $milestone = $opt_milestone ? $opt_milestone : $ENV{UPDMINOR};
    my $childws  = $opt_milestone ? undef : ( $opt_child  ? $opt_child  : $ENV{CWS_WORK_STAMP} );

    if ( !defined($masterws) ) {
        print_error("Can't determine master workspace environment.\n"
                    . "Please initialize environment with setsolar ...", 1);
    }

    if ( !defined($childws) && !defined($milestone) ) {
        print_error("Can't determine child workspace environment or milestone.\n"
                    . "Please initialize environment with setsolar ...", 1);
    }
    if ( !defined($opt_resultPage) ) {
    $opt_resultPage="";
    }
    my $cws = Cws->new();
    if ( defined($childws) ) {
        $cws->child($childws);
    }
    $cws->master($masterws);
    my $eis = $cws->eis();

    no strict;
    my $result='';

    if ( defined($childws) ) {
        $opt_resultPage=SOAP::Data->type(string => $opt_resultPage);
        my $id = $cws->eis_id();
        if ( is_valid_cws($cws) ) {
            $result=$eis->submitTestResult($id,$opt_testrunName,$opt_testrunPlatform, $opt_resultPage, $status);
    } else {
         print STDERR "cws is not valid";
    }
    } else {
        $opt_resultPage=SOAP::Data->type(string => $opt_resultPage);
        $result=$eis->submitTestResultMWS($masterws,$milestone,$opt_testrunName,$opt_testrunPlatform, $opt_resultPage, $status);
    }

    exit(0)
}


sub is_valid_cws
{
    my $cws = shift;

    my $masterws = $cws->master();
    my $childws  = $cws->child();
    # check if we got a valid child workspace
    my $id = $cws->eis_id();
    if ( !$id ) {
        print_error("Child workspace '$childws' for master workspace '$masterws' not found in EIS database.", 2);
    }
    return 1;
}

sub parse_options
{
    # parse options and do some sanity checks
    Getopt::Long::Configure("no_ignore_case");
    my $help = 0;
    my $success = GetOptions('h' => \$help,
                             'M=s' => \$opt_master,
                             'm=s' => \$opt_milestone,
                             'c=s' => \$opt_child,
                             'n=s' => \$opt_testrunName,
                             'p=s' => \$opt_testrunPlatform ,
                             'r=s' => \$opt_resultPage );
    if ( $help || !$success || $#ARGV < 0 || (!defined($opt_testrunName)) || ( !defined($opt_testrunPlatform)) ) {
        usage();
        exit(1);
    }

      print "$opt_master\n";
      print "$opt_milestone\n";
      print "$opt_child\n";
      print "$opt_testrunName\n";
      print "$opt_testrunPlatform\n";
      print "$opt_resultPage\n";

    if ( defined($opt_milestone) && defined($opt_child) ) {
    print_error("-m and -c are mutually exclusive options",1);
    }

    return $ARGV[0];
}

# sub print_message
# {
#     my $message     = shift;
#
#     print STDERR "$script_name: ";
#     print STDERR "$message\n";
#     return;
# }

sub print_error
{
    my $message     = shift;
    my $error_code  = shift;

    print STDERR "$script_name: ";
    print STDERR "ERROR: $message\n";

    if ( $error_code ) {
        print STDERR "\nFAILURE: $script_name aborted.\n";
        exit($error_code);
    }
    return;
}

sub usage
{
    print STDERR "Usage: cwstestresult[-h] [-M masterws] [-m milestone|-c childws] <-n testrunName> <-p testrunPlatform> <-r resultPage> statusName\n";
    print STDERR "\n";
    print STDERR "Publish result of CWS- or milestone-test to EIS\n";
    print STDERR "\n";
    print STDERR "Options:\n";
    print STDERR "\t-h\t\t\thelp\n";
    print STDERR "\t-M master\t\toverride MWS specified in environment\n";
    print STDERR "\t-m milestone\t\toverride milestone specified in environment\n";
    print STDERR "\t-c child\t\toverride CWS specified in environment\n";
    print STDERR "\t-n testrunName\t\tspecifiy name of the test\n";
    print STDERR "\t-p testrunPlatform\tspecify platform where the test ran on\n";
    print STDERR "\t-r resultPage\t\tspecify name of attachment or hyperlink\n";
    print STDERR "\t\t\t\tfor resultPage\n";


    print STDERR "\nExample:\n";
    print STDERR "\tcwstestresult -c mycws -n Performance -p Windows -r PerfomanceTestWindows.html ok\n";
}
