#! /usr/bin/perl

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


#
#   short description :
#     This Perl sciprt summarizes the number of errors and warnings in
#     the set of *.res files of qatesttool.
#*************************************************************************

use File::Basename;

sub usage {
    print << "EOT";

  $0 - summarizes the number of errors and warnings of qatesttool

  Synopsis:
     $0 file...

  Description:
    This Perl sciprt summarizes the number of errors and warnings
    reported in a set of the *.res files produced by qatesttool.

  Examples:
    perl make_summary.pl *.res
    ls -tr *.res | xargs perl make_summary.pl

EOT
}

format STDOUT_TOP =
Filename                      Errors  Warnings  Warnings_during_initialization
============================  ======  ========  ==============================
.

format STDOUT =
@<<<<<<<<<<<<<<<<<<<<<<<<<<<  @>>>>>   @>>>>>>    @>>>>>
$filename,                        $error,  $warning,  $warning_init
.

$num_of_files = 0;
$errors = 0;
$warnings = 0;
$wanrnings_init = 0;

sub footer {
    print "==============================================================================\n";
    printf("Total %2d files                    %2d        %2d        %2d\n",
       $num_of_files, $errors, $warnings, $wanrnings_init);
}

sub work {
    my $file = shift;
    my ($line, $lineno);
    open FH, "$file" or die "$!: $file, stopped";
    while (<FH>) {
    if (m/\%ResId=([0-9]+)\%(?:\%Arg1=(.+?)\%)?/) {
        $id = $1;
        $arg1 = $2;

        # http://qa.openoffice.org/qatesttool/resultfile.html

        # String 20014    Text[ en-US ] = "** ($Arg1) errors occurred";
        # String 20015    Text[ en-US ] = "** No errors have occurred";
        # String 20016    Text[ en-US ] = "** ($Arg1) warnings occurred";
        # String 20017    Text[ en-US ] = "** No warnings have occurred";
        # String 20023    Text[ en-US ] = "** ($Arg1) warnings occurred during initialization";
        # String 20024    Text[ en-US ] = "** No warnings occurred during initialization";

        $error = $arg1 + 0 if ($id eq 20014);
        $error = 0         if ($id eq 20015);
        $warning = $arg1 + 0 if ($id eq 20016);
        $warning = 0         if ($id eq 20017);
        $warning_init = $arg1 + 0 if ($id eq 20023);
        $warning_init = 0         if ($id eq 20024);
    }
    }
    close FH;
    # Get only the filename, excluding the path
    $filename = basename($file);
    write;
    $num_of_files ++;
    $errors += $error;
    $warnings += $warning;
    $wanrnings_init += $warning_init;
}

sub main {
    if (scalar(@ARGV) > 0) {
    foreach $file (@ARGV) {
        work($file);
    }
    footer();
    }
    else {
    usage();
    }
}

main();

__END__
