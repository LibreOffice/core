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
# $RCSfile: cwsattach.pl,v $
#
# $Revision: 1.3 $
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
# cwsattach.pl - attach files to CWS
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

#### script id #####

( my $script_name = $0 ) =~ s/^.*\b(\w+)\.pl$/$1/;

my $script_rev;
my $id_str = ' $Revision: 1.3 $ ';
$id_str =~ /Revision:\s+(\S+)\s+\$/
  ? ($script_rev = $1) : ($script_rev = "-");

print STDERR "$script_name -- version: $script_rev\n";

#### global #####

my $is_debug = 1;       # enable debug
my $opt_master = '';    # option: master workspace
my $opt_child  = '';    # option: child workspace
my $opt_mime_type = '';  # option: mime type


#### main #####

my $arg_file = parse_options();
attach_cws($arg_file);
exit(0);

#### subroutines ####

sub attach_cws
{
    my $filename = shift;
    # get master and child workspace
    my $masterws = $opt_master ? uc($opt_master) : $ENV{WORK_STAMP};
    my $childws  = $opt_child  ? $opt_child  : $ENV{CWS_WORK_STAMP};

    if ( !defined($masterws) ) {
        print_error("Can't determine master workspace environment.\n"
                    . "Please initialize environment with setsolar ...", 1);
    }

    if ( !defined($childws) ) {
        print_error("Can't determine child workspace environment.\n"
                    . "Please initialize environment with setsolar ...", 1);
    }

    my $cws = Cws->new();
    $cws->child($childws);
    $cws->master($masterws);

    my $mime_type  = $opt_mime_type  ? $opt_mime_type  : find_mime_type($filename);

    no strict;

    if ( is_valid_cws($cws) ) {
        #print "CWS is valid filename=" . $filename . " mime_type=" . $mime_type . "\n";
        open(DATA,"<$filename") || die "can't open filename";
        $data="";
    while(<DATA>) {
        $data.=$_;
    }
        my $result=$cws->save_attachment($filename,$mime_type,$data);
    } else {
        print STDERR "cws is not valid";
    }
    exit(0)
}


sub find_mime_type
{
    my $filename = shift;
    $filename=~/(.*)\.(.*$)/;
    my $ext=$2;
    my $fmime='';

    if ( defined($ext) ) {
        open(MIME,"< $ENV{SOLARENV}/inc/mime.types")|| die "can not open mimetype file";
        while (<MIME>) {
            my @a=split();
            my $iscomment=0;
            if ( /(\s*\#).*/ ) {
                $iscomment=1;
            } else {
                $iscomment=0;
            }
            if ( $iscomment eq 0 && $#a >= 1 && $fmime eq '' ) {
                my $i=1;
                for ($i=1; $i<=$#a; $i++) {
                    if ( $a[$i] eq $ext ) {
                        $fmime=$a[0];
                    }
                }
            }
        }

    }
    if ( $fmime eq '' ) {
        $fmime="application/octet-stream";
    }
    return $fmime;
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
    print_message("Master workspace '$masterws', child workspace '$childws':");
    return 1;
}

sub parse_options
{
    # parse options and do some sanity checks
    my $help = 0;
    my $success = GetOptions('h' => \$help, 'm=s' => \$opt_master, 'c=s'=> \$opt_child, 't=s'=> \$opt_mime_type);
    if ( $help || !$success || $#ARGV < 0 ) {
        usage();
        exit(1);
    }

    return $ARGV[0];
}

sub print_message
{
    my $message     = shift;

    print STDERR "$script_name: ";
    print STDERR "$message\n";
    return;
}

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
    print STDERR "Usage: cwsattach [-h] [-m master] [-c child] [-t mimetype] filename\n";
    print STDERR "\n";
    print STDERR "Attach files to CWS in EIS database\n";
    print STDERR "\n";
    print STDERR "Options:\n";
    print STDERR "\t-h\t\thelp\n";
    print STDERR "\t-m master\toverride MWS specified in environment\n";
    print STDERR "\t-c child\toverride CWS specified in environment\n";
    print STDERR "\t-t mimetype\texplicitly set mime type\n";
    print STDERR "Examples:\n";
    print STDERR "\tcwsattach barfoo.html\n";
    print STDERR "\tcwsattach -t text bar.cxx\n";
    print STDERR "\tcwsattach -t text/rtf foo.rtf\n";
}
