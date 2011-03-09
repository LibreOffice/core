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
#
# checkdeliver.pl - compare delivered files on solver with those on SRC_ROOT
#

use strict;
use Getopt::Long;
use File::stat;
use IO::Handle;

use lib ("$ENV{SOLARENV}/bin/modules");

#### globals #####

my $err              = 0;
my $srcrootdir       = '';
my $solverdir        = '';
my $platform         = '';
my $logfile          = '';
my $milestoneext     = '';
my $local_env        = 0;
my @exceptionmodlist = (
                        "postprocess",
                        "instset.*native",
                        "smoketest.*native",
                        "testautomation",
                        "testgraphical"
                       ); # modules not yet delivered

#### main #####

print_logged("checkdeliver.pl - checking delivered binaries\n");

get_globals();                                  # get global variables
my $deliverlists_ref = get_deliver_lists();     # get deliver log files
foreach my $listfile ( @$deliverlists_ref ) {
    $err += check( $listfile );                 # check delivered files
}
print_logged("OK\n") if ( ! $err );
exit $err;

#### subroutines ####

sub get_globals
# set global variables using environment variables and command line options
{
    my $help;

    # set global variables according to environnment
    $platform      = $ENV{INPATH};
    $srcrootdir    = "$ENV{SOURCE_ROOT_DIR}/clone";
    $solverdir     = $ENV{SOLARVERSION};
    $milestoneext  = $ENV{UPDMINOREXT};

    # override environment with command line options
    GetOptions('help' => \$help,
               'l=s'  => \$logfile,
               'p=s'  => \$platform
    ) or usage (1);

    if ( $help ) {
        usage(0);
    }

    #do some sanity checks
    if ( ! ( $platform && $srcrootdir && $solverdir ) ) {
        die "Error: please set environment\n";
    }
    if ( ! -d $solverdir ) {
        die "Error: cannot find solver directory '$solverdir'\n";
    }

    # Check for local env., taken from solenv/bin/modules/installer/control.pm
    # In this case the content of SOLARENV starts with the content of SOL_TMP
    my $solarenv = "";
    my $sol_tmp;
    if ( $ENV{'SOLARENV'} ) {
        $solarenv = $ENV{'SOLARENV'};
    }
    if ( $ENV{'SOL_TMP'} ) {
        $sol_tmp = $ENV{'SOL_TMP'};
    }
    if ( defined $sol_tmp && ( $solarenv =~ /^\s*\Q$sol_tmp\E/ )) {
        # Content of SOLARENV starts with the content of SOL_TMP: Local environment
        $local_env = 1;
    }
}

sub get_deliver_lists
# find deliver log files on solver
{
    my @files;
    my $pattern = "$solverdir/$platform/inc";
    $pattern .= "$milestoneext" if ( $milestoneext );
    $pattern .= "/*/deliver.log";

    @files = glob( $pattern );
    # do not check modules not yet built
    foreach my $exceptionpattern ( @exceptionmodlist ) {
        @files = grep ! /\/$exceptionpattern\//, @files;
    }
    if ( ! @files ) {
        print_logged( "Error: cannot find deliver log files\n" );
        exit 1;
    }
    return \@files;
}

sub check
# reads deliver log file given as parameter and compares pairs of files listed there.
{
    my $listname = shift;
    my $error = 0;
    my %delivered;
    my $module;
    my $repository;
    STDOUT->autoflush(1);
    # which module are we checking?
    if ( $listname =~ /\/([\w-]+?)\/deliver\.log$/o) {
        $module = $1;
    } else {
        print_logged( "Error: cannot determine module name from \'$listname\'\n" );
        return 1;
    }

    if ( -z $listname ) {
        print_logged( "Warning: empty deliver log file \'$listname\'. Module '$module' not delivered correctly?\n\n" );
        return 0;
    }

    # read deliver log file
    if ( ! open( DELIVERLOG, "< $listname" ) ) {
        print_logged( "Error: cannot open file \'$listname\'\n$!" );
        exit 2;
    }
    while ( <DELIVERLOG> ) {
        next if ( /^LINK / );
        # What's this modules' repository?
        if ( /COPY\s+(.+?)\/$module\/prj\/build.lst/ ) {
#        if ( /COPY (\w[\w\s-]*?)\/$module\/prj\/build.lst/ ) {
            $repository = $1;
        }
        # For now we concentrate on binaries, located in 'bin' or 'lib' and 'misc/build/<...>/[bin|lib]'.
        next if ( (! /\/$module\/$platform\/[bl]i[nb]\//) && (! /\/$module\/$platform\/misc\/build\//));
        next if (! /[bl]i[nb]/);
        next if ( /\.html$/ );
        chomp;
        if ( /^\w+? (\S+) (\S+)\s*$/o ) {
            my $origin = $1;
            $delivered{$origin} = $2;
        } else {
            print_logged( "Warning: cannot parse \'$listname\' line\n\'$_\'\n" );
        }
    }
    close( DELIVERLOG );

    if ( ! $repository ) {
        print_logged( "Error parsing \'$listname\': cannot determine repository. Module '$module' not delivered correctly?\n\n" );
        $error ++;
        return $error;
    }

    my $path = "$srcrootdir/$repository/$module";
    # is module physically accessible?
    # there are valid use cases where we build against a prebuild solver whithout having
    # all modules at disk
    my $canread = is_moduledirectory( $path );
    if ( ! $canread ) {
        # do not bother about non existing modules in local environment
        # or on childworkspaces
        if (( $local_env ) || ( $ENV{CWS_WORK_STAMP} )) {
            return $error;
        }
        # in a master build it is considered an error to have deliver leftovers
        # from non exising (removed) modules
        print_logged( "Error: module '$module' not found.\n" );
        $error++;
        return $error;
    }
    if ( $canread == 2 ) {
        # module is linked and not built, no need for checking
        # should not happen any more nowadays ...
        return $error;
    }

    # compare all delivered files with their origin
    # no strict 'diff' allowed here, as deliver may alter files (hedabu, strip, ...)
    foreach my $file ( sort keys %delivered ) {
        my $ofile = "$srcrootdir/$file";
        my $sfile = "$solverdir/$delivered{$file}";
        if ( $milestoneext ) {
            # deliver log files do not contain milestone extension on solver
            $sfile =~ s/\/$platform\/(...)\//\/$platform\/$1$milestoneext\//;
        }
        my $orgfile_stats = stat($ofile);
        next if ( -d _ );  # compare files, not directories
        my $delivered_stats = lstat($sfile);
        next if ( -d _ );  # compare files, not directories
        if ( $^O !~ /^MSWin/ ) {
            # windows does not know about links.
            # Therefore lstat() is not a lstat, and the following check would break
            next if ( -l _ );  # compare files, not links
        }

        if ( $orgfile_stats && $delivered_stats ) {
            # Stripping (on unix like platforms) and signing (for windows)
            # changes file size. Therefore we have to compare for file dates.
            # File modification time also can change after deliver, f.e. by
            # rebasing, but only increase. It must not happen that a file on
            # solver is older than it's source.
            if ( ( $orgfile_stats->mtime - $delivered_stats->mtime ) gt 1 ) {
                print_logged( "Error: " );
                print_logged( "delivered file is older than it's source '$ofile' '$sfile'\n" );
                $error ++;
            }
        } elsif ( !$orgfile_stats && $delivered_stats ) {
            # This is not an error if we have a solver and did not build the
            # module!
        } elsif ( !$orgfile_stats && !$delivered_stats ) {
            # This is not necessarily an error.
            # Instead, this seems to be an error of the deliver.log file.
        } else {
            print_logged( "Error: no such file '$ofile'\n" ) if ( ! $orgfile_stats );
            print_logged( "Error: no such file '$sfile'\n" ) if ( ! $delivered_stats );
            $error ++;
        }
    }
    if ( $error ) {
        print_logged( "$error errors found: Module '$module' not delivered correctly?\n\n" );
    }
    STDOUT->autoflush(0);
    return $error;
}

sub is_moduledirectory
# Test whether we find a module having a d.lst file at a given path.
# Return value: 1: path is valid directory
#               2: path.link is a valid link
#               0: module not found
{
    my $dirname = shift;
    if ( -e "$dirname/prj/d.lst" ) {
        return 1;
    } elsif ( -e "$dirname.link/prj/d.lst" ) {
        return 2
    } else {
        return 0;
    }
}

sub print_logged
# Print routine.
# If a log file name is specified with '-l' option, print_logged() prints to that file
# as well as to STDOUT. If '-l' option is not set, print_logged() just writes to STDOUT
{
    my $message = shift;
    print "$message";
    if ( $logfile ) {
        open ( LOGFILE, ">> $logfile" ) or die "Can't open logfile '$logfile'\n";
        print LOGFILE "$message";
        close ( LOGFILE) ;
    }
}


sub usage
# print usage message and exit
{
    my $retval = shift;
    print STDERR "Usage: checkdeliver.pl [-h] [-p <platform>]\n";
    print STDERR "Compares delivered files on solver with original ones in build tree\n";
    print STDERR "Options:\n";
    print STDERR "    -h              print this usage message\n";
    print STDERR "    -p platform     specify platform\n";

    exit $retval;
}

