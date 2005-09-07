:
    eval 'exec perl -S $0 ${1+"$@"}'
        if 0;
#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: cwsview.pl,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 22:07:38 $
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
# cwsview - view cvs status for cws
#

use lib ("$ENV{SOLARENV}/bin/modules");

use Cwd;
use Cws;
use CvsModule;

#### script id #####
    ( $script_name = $0 ) =~ s/^.*\b(\w+)\.pl$/$1/;

    $id_str = ' $Revision: 1.3 $ ';
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

# check if we got a valid child workspace
my $id = $cws->eis_id();
if ( !$id ) {
    die "CWS $cws_name for master workspace $masterws not found in EIS database.";
}
my @modules = get_options();
@modules = get_checked_modules(\@modules);

do_cvs_view(\@modules);

exit(0);
### end of main ###

#########################
#                       #
#      Procedures       #
#                       #
#########################

sub do_cvs_view {
    my $modules_ref = shift;
    my $cvs_module = CvsModule->new();
    foreach my $module (@$modules_ref) {
        print "\n######## $module ########\n";
        $cvs_module->module($module);
        my $path = $ENV{SRC_ROOT} . '/' . $module;
        $cvs_module->view($path);
    };
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
