:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: checkdeliver.pl,v $
#
# $Revision: 1.14 $
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

#### globals #####

my $err       = 0;
my $srcdir    = '';
my $solverdir = '';
my $platform  = '';
my $milestoneext = '';
my $local_env = 0;
my @exceptionmodlist = ("postprocess", "instset.*native"); # modules not yet delivered

#### main #####

print "checkdeliver.pl - checking delivered binaries\n";

get_globals();                                  # get global variables
my $deliverlists_ref = get_deliver_lists();     # get deliver log files
foreach my $listfile ( @$deliverlists_ref ) {
    $err += check( $listfile );                 # check delivered files
}
print "OK\n" if ( ! $err );
exit $err;

#### subroutines ####

sub get_globals
# set global variables using environment variables and command line options
{
    my $help;

    # set global variables according to environnment
    $platform      = $ENV{INPATH};
    $srcdir        = $ENV{SOLARSRC};
    $solverdir     = $ENV{SOLARVERSION};
    $milestoneext  = $ENV{UPDMINOREXT};

    # override environment with command line options
    GetOptions('help' => \$help,
               'p=s'  => \$platform
    ) or usage (1);

    if ( $help ) {
        usage(0);
    }

    #do some sanity checks
    if ( ! ( $platform && $srcdir && $solverdir ) ) {
        die "Error: please set environment\n";
    }
    if ( ! -d $srcdir ) {
        die "Error: cannot find source directory '$srcdir'\n";
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

    if ( $^O =~ /cygwin/i && $ENV{'USE_SHELL'} eq "4nt" )
    {   # glob from cygwin's perl needs $pattern to use only slashes.
        # (DOS style path are OK as long as slashes are used.)
        $pattern =~ s/\\/\//g;
    }

    @files = glob( $pattern );
    # do not check modules not yet built
    foreach my $exceptionpattern ( @exceptionmodlist ) {
        @files = grep ! /\/$exceptionpattern\//, @files;
    }
    if ( ! @files ) {
        die "Error: cannot find deliver log files";
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
    my $islinked = 0;
    # which module are we checking?
    if ( $listname =~ /\/([\w-]+?)\/deliver\.log$/o) {
        $module = $1;
    } else {
        print STDERR "Error: cannot determine module name from \'$listname\'\n";
        return 1;
    }
    # is module physically accessible?
    my $canread = is_moduledirectory( $srcdir . '/' . $module );
    if ( ! $canread ) {
        # do not bother about non existing modules in local environment
        if ( $local_env ) {
            # print STDERR "Warning: local environment, module '$module' not found. Skipping.\n";
            return $error;
        }
        # on CWS modules not added can exist as links. For windows it may happen that these
        # links cannot be resolved (when working with nfs mounts). This prevents checking,
        # but is not an error.
        if ( $ENV{CWS_WORK_STAMP} ) {
            # print STDERR "Warning: module '$module' not found. Skipping.\n";
            return $error;
        }
        print STDERR "Error: module '$module' not found.\n";
        $error++;
        return $error;
    }
    if ( $canread == 2 ) {
        # module is linked, link can be resolved
        $islinked = 1;
    }

    # read deliver log file
    open( DELIVERLOG, "< $listname" ) or die( "Error: cannot open file \'$listname\'\n$!");
    foreach ( <DELIVERLOG> ) {
        next if ( /^LINK / );
        # For now we concentrate on binaries, located in 'bin' or 'lib'.
        # It probably is a good idea to check all files but this requires some
        # d.lst cleanup which is beyond the current scope. -> TODO
        next if ( ! / $module\/$platform\/[bl]i[nb]\// );
        next if ( /\.html$/ );
        chomp;
        if ( /^\w+? (\S+) (\S+)\s*$/o ) {
            $delivered{$1} = $2;
        } else {
            print STDERR "Warning: cannot parse \'$listname\' line\n\'$_\'\n";
        }
    }
    close( DELIVERLOG );

    # compare all delivered files with their origin
    # no strict 'diff' allowed here, as deliver may alter files (hedabu, strip, ...)
    foreach my $file ( sort keys %delivered ) {
        my $ofile = "$srcdir/$file";
        my $sfile = "$solverdir/$delivered{$file}";
        # on CWS modules may exist as link only, named <module>.lnk
        if ( $islinked ) {
            $ofile =~ s/\/$module\//\/$module.lnk\//;
        }
        if ( $milestoneext ) {
            # deliver log files do not contain milestone extension on solver
            $sfile =~ s/\/$platform\/(...)\//\/$platform\/$1$milestoneext\//;
        }
        my $orgfile_stats = stat($ofile);
        next if ( -d _ );  # compare files, not directories
        my $delivered_stats = lstat($sfile);
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
                print STDERR "Error: ";
                print STDERR "delivered file is older than it's source '$ofile' '$sfile'\n";
                $error ++;
            }
        } else {
            print STDERR "Error: no such file '$ofile'\n" if ( ! $orgfile_stats );
            print STDERR "Error: no such file '$sfile'\n" if ( ! $delivered_stats );
            $error ++;
        }
    }
    if ( $error ) {
        print STDERR "Errors found: Module '$module' not delivered correctly?\n\n";
    }
    return $error;
}

sub is_moduledirectory
# Test whether we find a module having a d.lst file at a given path.
# Return value: 1: path is valid directory
#               2: path.lnk is a valid link
#               0: module not found
{
    my $dirname = shift;
    if ( -e "$dirname/prj/d.lst" ) {
        return 1;
    } elsif ( -e "$dirname.lnk/prj/d.lst" ) {
        return 2
    } else {
        return 0;
    }
}

sub usage
# print usage message and exit
{
    my $retval = shift;
    print STDERR "Usage: checkdeliver.pl [-h] [-p <platform>]\n";
    print STDERR "Compares delivered files on solver with original ones on SRC_ROOT\n";
    print STDERR "Options:\n";
    print STDERR "    -h              print this usage message\n";
    print STDERR "    -p platform     specify platform\n";

    exit $retval;
}
