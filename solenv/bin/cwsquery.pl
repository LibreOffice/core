:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
#   $RCSfile: cwsquery.pl,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: hr $ $Date: 2004-06-26 00:23:20 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc..
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

#
# cwsquery.pl - query misc. information from CWS
#

use strict;
use Getopt::Long;
use Cwd;

#### module lookup

use lib ("$ENV{SOLARENV}/bin/modules");
if (defined $ENV{COMMON_ENV_TOOLS}) {
    unshift(@INC, "$ENV{COMMON_ENV_TOOLS}/modules");
};
use Cws;

#### script id #####

( my $script_name = $0 ) =~ s/^.*\b(\w+)\.pl$/$1/;

my $script_rev;
my $id_str = ' $Revision: 1.2 $ ';
$id_str =~ /Revision:\s+(\S+)\s+\$/
  ? ($script_rev = $1) : ($script_rev = "-");

print STDERR "$script_name -- version: $script_rev\n";

#### global #####

my $is_debug = 1;       # enable debug
my $opt_master = '';    # option: master workspace
my $opt_child  = '';    # option: child workspace

# list of available query modes
my @query_modes = qw(modules taskids state latest);
my %query_modes_hash = ();
foreach (@query_modes) {
    $query_modes_hash{$_}++;
}

#### main #####

my $query_mode = parse_options();
query_cws($query_mode);
exit(0);

#### subroutines ####

sub query_cws
{
    my $query_mode = shift;
    # get master and child workspace
    my $masterws = $opt_master ? uc($opt_master) : $ENV{WORK_STAMP};
    my $childws  = $opt_child  ? $opt_child  : $ENV{CWS_WORK_STAMP};

    if ( !defined($masterws) ) {
        print_error("Can't determine master workspace environment.\n"
                    . "Please initialize environment with setsolar ...", 1);
    }

    if ( !defined($childws) && $query_mode ne 'latest' ) {
        print_error("Can't determine child workspace environment.\n"
                    . "Please initialize environment with setsolar ...", 1);
    }

    my $cws = Cws->new();
    $cws->child($childws);
    $cws->master($masterws);

    no strict;
    &{"query_".$query_mode}($cws);
    exit(0)
}

sub query_modules
{
    my $cws = shift;

    if ( is_valid_cws($cws) ) {
        my @modules = $cws->modules();
        print_message("Modules:");
        foreach (@modules) {
            print "$_\n";
        }
    }
    return;
}

sub query_taskids
{
    my $cws = shift;

    if ( is_valid_cws($cws) ) {
        my @taskids = $cws->taskids();
        print_message("Task ID(s)");
        foreach (@taskids) {
            print "$_\n";
        }
    }
    return;
}

sub query_state
{
    my $cws = shift;

    if ( is_valid_cws($cws) ) {
        my $state = $cws->get_approval();
        if ( !$state ) {
            print_error("Internal error: can't get approval status.", 3);
        } else {
            print_message("Approval status:");
            print "$state\n";
        }
    }
    return;
}

sub query_latest
{
    my $cws = shift;

    my $masterws = $cws->master();
    my $latest = $cws->get_current_milestone($masterws);


    if ( $latest ) {
        print_message("Master workspace '$masterws':");
        print_message("Latest milestone available for resync: '$masterws $latest'");
    }
    else {
        print_error("Can't determine latest milestone of '$masterws' available for resync.", 3);
    }

    return;
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
    my $success = GetOptions('h' => \$help, 'm=s' => \$opt_master, 'c=s'=> \$opt_child);
    if ( $help || !$success || $#ARGV < 0 ) {
        usage();
        exit(1);
    }

    # there will be more query modes over time
    if ( !exists $query_modes_hash{lc($ARGV[0])} ) {
        usage();
        exit(1);
    }
    return lc($ARGV[0]);
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
    print STDERR "Usage: cwsquery [-h] [-m master] [-c child] <modules|taskIDs|state>\n";
    print STDERR "       cwsquery [-h] [-m master] <latest>\n";
    print STDERR "Query child workspace for miscancellous information.\n";
    print STDERR "Modes:\n";
    print STDERR "\tmodules\t\tquery modules added to the CWS\n";
    print STDERR "\ttaskids\t\tquery taskids to be handled on the CWS\n";
    print STDERR "\tstate\t\tquery approval status of CWS\n";
    print STDERR "\tlatest\t\tquery the latest milestone available for resync\n";
    print STDERR "Options:\n";
    print STDERR "\t-h\t\thelp\n";
    print STDERR "\t-m master\toverride MWS specified in environment\n";
    print STDERR "\t-c child\toverride CWS specified in environment\n";
    print STDERR "Examples:\n";
    print STDERR "\tcwsquery modules \n";
    print STDERR "\tcwsquery -m SRX644 -c uno4 modules \n";
    print STDERR "\tcwsquery -m SRX645 -c pmselectedfixes state\n";
    print STDERR "\tcwsquery taskids\n";
    print STDERR "\tcwsquery -m SRC680 latest\n";

}
