#! /usr/bin/perl

#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: make_summary.pl,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: asrail $ $Date: 2007/02/20 05:49:23 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************
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
