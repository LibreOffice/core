:
    eval 'exec perl -S $0 ${1+"$@"}'
        if 0;
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: cwsview.pl,v $
#
# $Revision: 1.6 $
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
# cwsview - view cvs status for cws
#

use lib ("$ENV{SOLARENV}/bin/modules");

use Cwd;
use Cws;
use Cvs;
use CvsModule;
use File::Find;
use Cwd 'chdir';

#### script id #####
    ( $script_name = $0 ) =~ s/^.*\b(\w+)\.pl$/$1/;

    $id_str = ' $Revision: 1.6 $ ';
    $id_str =~ /Revision:\s+(\S+)\s+\$/
      ? ($script_rev = $1) : ($script_rev = "-");

    print "$script_name -- version: $script_rev\n";

### main ###

if ((!defined $ENV{CWS_WORK_STAMP}) || (!defined $ENV{WORK_STAMP})) {
    die "You're not in CWS environment. Please, set one";
}
my $masterws = $ENV{WORK_STAMP};
my $cws_name = $ENV{CWS_WORK_STAMP};

my $cws = Cws->new();
$cws->child($cws_name);
$cws->master($masterws);
my %modules_view_info = ();
my ($master_branch_tag, $cws_branch_tag, $cws_root_tag) = $cws->get_tags();

# check if we got a valid child workspace
my $id = $cws->eis_id();
if ( !$id ) {
    die "CWS $cws_name for master workspace $masterws not found in EIS database.";
}
my @modules = get_options();
@modules = get_checked_modules(\@modules);

my $wanted_file = '';
my @wanted_files;

do_cvs_view(\@modules);
print_resume();

exit(0);
### end of main ###

#########################
#                       #
#      Procedures       #
#                       #
#########################

sub print_resume {
#    return if (!scalar keys %modules_view_info);
    my $summary_printed = 0;
    foreach (keys %modules_view_info) {
        foreach my $line (sort @{$modules_view_info{$_}}) {
            if (!$summary_printed) {
                print "\n========== SUMMARY ==========\n";
                $summary_printed++;
            };
            print "$line";
        }
    }
    print "\n";
};

#
# Pocedure checks if in $modules_view_info there are "locally added" files, if yes - expands
# their name to their paths
#
sub prove_view_info {
    foreach my $module (keys %modules_view_info) {
        my $module_info_text = $modules_view_info{$module};
        foreach (@$module_info_text) {
            if (/(:\slocally\sadded)$/) {
                my $full_file_path = find_file_in_module($module, $`);
                $_ = $full_file_path . $1 . "\n";
            };
        };
    };
};


sub wanted {
    if ($_ eq $wanted_file) {
        push(@wanted_files, $File::Find::name);
    }
}

sub find_file_in_module{
    my $module = shift;
    $wanted_file = shift;
    @wanted_files = ();
    my @found_files;
    my $path = $ENV{SRC_ROOT} . '/' . $module;
    find(\&wanted, $path);
    if ((scalar @wanted_files) == 1) {
        return substr($wanted_files[0], length($ENV{SRC_ROOT}) + 1);
    };
    chdir($path);
    cwd();
    foreach (@wanted_files) {
        my $candidat = substr($_, length($path) + 1);
        my $archive = Cvs->new();
        $archive->name($candidat);
        if (!defined $archive->get_head()) {
            return $module . '/' . $candidat;
        };
    };
};

sub do_cvs_view {
    my $modules_ref = shift;
    my $cvs_module = CvsModule->new();
    my $changed_files_ref;
    foreach my $module (@$modules_ref) {
        print "\n######## $module ########\n";
        $cvs_module->module($module);
        my $path = $ENV{SRC_ROOT} . '/' . $module;
        $modules_view_info{$module} = $cvs_module->view($path);
    };
    prove_view_info();
};

sub usage {
    print STDERR "\ncwsview\n";
    print STDERR "Syntax:    cwsview  [module_name|all] [--help|-h]\n";
    print STDERR "        module_name       - print cvs status info for module_name in cws\n";
    print STDERR "        --help            - print help info\n";
    exit(0);
};

sub get_options {
    return () if (!scalar @ARGV);
    my @modules = ();
    foreach (@ARGV) {
        return () if /^all$/;
        usage() if /^-h$/;
        usage() if /^--help$/;
        push(@modules, $_);
    };
    return @modules;
}

sub get_checked_modules {
    my $modules_ref = shift;
    my @cws_modules = $cws->modules();
    if (!scalar @$modules_ref) {
        return @cws_modules;
    };
    my %cws_modules_hash = ();
    $cws_modules_hash{$_}++ foreach (@cws_modules);
    my @errors = ();
    my @checked_modules = ();
    foreach (@$modules_ref) {
        if (defined $cws_modules_hash{$_}) {
            push(@checked_modules, $_);
            next;
        };
        push(@errors, $_);
    };
    if (scalar @errors) {
        print STDERR "No module $_ is registered with cws $cws_name\n" foreach (@errors);
        exit(1);
    };
    if (!scalar @checked_modules) {
        print STDERR "No modules found to check";
        exit(1);
    };
    return @checked_modules;
};
