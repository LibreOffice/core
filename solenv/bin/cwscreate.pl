:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
#   $RCSfile: cwscreate.pl,v $
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
# cwscreate.pl - create child workspaces
#

use strict;
use Getopt::Long;
use Cwd;
use IO::Handle;
use Sys::Hostname;
use File::Spec;

#### module lookup

use lib ("$ENV{SOLARENV}/bin/modules");
if (defined $ENV{COMMON_ENV_TOOLS}) {
    unshift(@INC, "$ENV{COMMON_ENV_TOOLS}/modules");
};
use Cws;
use CvsModule;
use CwsConfig;
eval { require Logging; import Logging; };
# $log variable is only defined in SO environment...
my $log = undef;
$log = Logging->new() if (!$@);
use GenInfoParser;

######### Interrupt handler #########
$SIG{'INT'} = 'INT_handler' if defined($log);

#### script id #####

( my $script_name = $0 ) =~ s/^.*\b(\w+)\.pl$/$1/;

my $script_rev;
my $id_str = ' $Revision: 1.2 $ ';
$id_str =~ /Revision:\s+(\S+)\s+\$/
  ? ($script_rev = $1) : ($script_rev = "-");

print "$script_name -- version: $script_rev\n";

#### hardcoded globals #####

my $b_server_wnt = 'r:/b_server/config';
my $b_server_unx = $ENV{ENV_ROOT} . '/b_server/config' if defined $ENV{ENV_ROOT};
$b_server_unx    = '/so/env/b_server/config' if ! -d $b_server_unx;

#### globals #####

my $force_checkout = '';
my $opt_dir   = '';     # optional directory argument;
my $opt_platformlist;     # optional platform argument;
my @opt_platforms   = ();     # optional platform argument;
my @args_bak = @ARGV; # store the @ARGS here for logging
my $is_debug = 0;
my $umask = umask();
if ( !defined($umask) ) {
    $umask = 22;
}
my $vcsid = $ENV{VCSID};    # user for logging
$vcsid = "unkown" if ( !$vcsid );

# modules to be obligatory copied to each cws
my %obligatory_modules = ();
$obligatory_modules{'solenv'}++;
$obligatory_modules{'res'}++;
$obligatory_modules{'so_res'}++;
$obligatory_modules{'instset'}++;
$obligatory_modules{'instsetoo'}++;
$obligatory_modules{'smoketest'}++;
$obligatory_modules{'smoketest_oo'}++;
$obligatory_modules{'test10'}++;

my $parameter_list = $log->array2string(";",@args_bak) if defined($log);

#### main #####
my ($cws, $wslocation, $is_promotion) = parse_options();
my $success = defined($log) ? copy_workspace($cws, $wslocation) : update_workspace($cws, $wslocation);
if ( $success ) {
    register_workspace($cws, $is_promotion);
}
$log->end_log_extended($script_name,$vcsid,"success") if defined($log);
exit(0);

#### subroutines ####

sub parse_options
{
    # parse options and do some sanity checks
    # returns freshly allocated Cws reference
    my $help;
    my $success;
    $success = GetOptions('d=s' => \$opt_dir, 'p=s' => \$opt_platformlist, 'h' => \$help, '-a' => \$force_checkout);
    if ( $help || !$success || $#ARGV > 2 ) {
        usage();
        exit(1);
    }
    my $masterws  = uc(shift @ARGV);
    my $milestone = shift @ARGV;
    my $childws   = shift @ARGV;

    if ( ! ($masterws && $milestone && $childws) ) {
        print STDERR "please specify master, milestone and child workspace\n";
        usage();
        exit(1);
    }

    if ( $opt_dir && !-d $opt_dir ) {
        print STDERR "'$opt_dir' is not a directory\n";
        usage();
        exit(1);
    }

    # check if child workspace name is sane
    if ( $childws !~ /^[a-z][a-z0-9]*$/ ) {
        print_error("Invalid child workspace name '$childws'.\nCws names should contain lowercase letters and digits, starting with a letter.", 3);
    }

    # check if environment matches masterws and milestone
    if ($masterws ne $ENV{WORK_STAMP})
    {
        if (defined $ENV{UPDMINOR} && $milestone ne $ENV{UPDMINOR}) {
            print_error("Please set an environment matching your targeted milestone", 2);
        };
    }

    # check if master is known
    my $result = undef;
    my $master    = Eis::to_string($masterws);
    my $eis = Cws::eis();
    eval { $result = $eis->getCurrentMilestone($master) };
    if ( !defined $result ) {
        print_error("Master workspace '$masterws' not found in database.", 4);
    };

    my $cws = Cws->new();
    $cws->master($masterws);
    $cws->child($childws);
    $log->start_log_extended($script_name,$parameter_list,$masterws,$childws) if defined($log);

    # check if child workspace already exists
    my $eis_id = $cws->eis_id();
    if ( !defined($eis_id) ) {
        print_error("Connection with EIS database failed.", 6);
    }

    my $is_promotion = 0;
    if ( $eis_id > 0 ) {
        if ( $cws->get_approval() eq 'planned' ) {
            print "Scheduling promotion of child workspace '$childws' from 'planned' to 'new'.\n";
            $is_promotion++;
        }
        else {
            print_error("Child workspace '$childws' for master workspace '$masterws' already exists.", 7);
        }
    }
    else {
        # check if child workspace name is still available
        if ( !$cws->is_cws_name_available()) {
            print_error("Child workspace name '$childws' is already in use.", 7);
        }
    }

    # set milestone
    # TODO check validity of milestone
    $cws->milestone($milestone);

    # check if suggested platforms exist
    if ( $opt_platformlist ) {
        my $push_pro;
        my $push_nonpro;
        my @platforms = split( /,/ , $opt_platformlist );

        foreach ( @platforms )
        {
            my $result;
            my @found_dirs = ();
            my $master = $cws->master();

            # find valid platforms to copy
            $result = opendir( SOLVER, "$wslocation/$master");
            if ( !$result ){ print_error ("Root dir of master workspace not accessible: $!", 1) }
            closedir( SOLVER );
            my $check_dir = "$wslocation/$master/$_/inc.$milestone";
            if ( -d "$check_dir" ) {
                push @opt_platforms, $_ ;
                if ( $check_dir =~ /\.pro\/[^\/]+$/ ) {
                    $push_pro = 1 ;
                } else {
                    $push_nonpro = 1 ;
                }
            } else {
                print_error ("\"$_\" is not a valid platform. Please try again!", 1)
            }
        }
        push @opt_platforms, "common" if $push_nonpro;
        push @opt_platforms, "common.pro" if $push_pro ;
    }
    return ($cws, $wslocation, $is_promotion);
}

sub get_workspace_lst
{
    # get the workspace list ('stand.lst'), either from 'localini'
    # or, if this is not possible, from 'globalini'

    my $home;
    if ( $^O eq 'MSWin32' ) {
        $home = $ENV{TEMP};
    }
    else {
        $home = $ENV{HOME};
    }
    my $localini = "$home/localini";
    if ( ! -f "$localini/stand.lst" ) {
        my $globalini = get_globalini();
        return "$globalini/stand.lst";
    }
    return "$localini/stand.lst";
}

sub get_globalini
{
    # get 'globalini' - either by environment variable or the default
    my $globalini;

    $globalini = $ENV{GLOBALINI};

    # default
    if ( !defined($globalini) ) {
        $globalini = ( $^O eq 'MSWin32' )
            ?  $b_server_wnt : $b_server_unx;
    }
    return $globalini;
}

#
# procedure checks if all modules are in the
# workspace and issues warning(s) about missing ones
#
sub check_cvs_update {
    my ($cvs_aliases, $updated_modules, $master_tag) = @_;
    my @missing_modules = ();
    foreach my $module (split( /\s+/, $$cvs_aliases{'OpenOffice'})) {
        next if ($module eq '-a');
        next if (defined $$updated_modules{$module});
        push (@missing_modules, $module);
    };
    if (scalar @missing_modules) {
        print_warning("The following modules are missing in your workspace,");
        print_warning("this might not be a problem - check out missing modules with tag '$master_tag':");
        print "@missing_modules\n";
    };
};

sub update_workspace {
    my $cws = shift;
    my $stand_dir = $ENV{SRC_ROOT};
    if (!opendir(SOURCES, $stand_dir)) {
        print_error ("Environment variable SRC_ROOT points to not accesible diretory: $!", 1)
    }
    my @dir_content = readdir(SOURCES);
    close SOURCES;
    my $master_tag =  $cws->get_master_tag();
    my $config = CwsConfig::get_config;
    my $cvs_module = CvsModule->new();
    $cvs_module->cvs_method($config->get_cvs_server_method());
    $cvs_module->vcsid($config->get_cvs_server_id());
    $cvs_module->cvs_server($config->get_cvs_server());
    $cvs_module->cvs_repository($config->get_cvs_server_repository());
    my %cvs_aliases = $cvs_module->get_aliases_hash();
    my %updated_modules = ();
    my @warnings = ();
    if ( @dir_content ) {
        print_message("Updating workspace in '$stand_dir' to revision '$master_tag'.");
    }
    foreach my $module (@dir_content) {
        next if (!defined $cvs_aliases{$module});
        if (!-d "$stand_dir/$module/CVS") {
            push(@warnings, "Cannot update module $module\n");
            next;
        };
        $cvs_module->module($module);
        print "\tUpdating '$module' ...\n";
        my $result = $cvs_module->update($stand_dir, $master_tag);
        $cvs_module->handle_update_infomation($result);
        $updated_modules{$module}++;
    };
    print $_ foreach (@warnings);
    check_cvs_update(\%cvs_aliases, \%updated_modules, $master_tag);
    return '1';
};

sub copy_workspace
{
    require sync_dir; import sync_dir;
    use File::Path;
    use File::Basename;
    use File::Copy;
    use File::Glob;

    no warnings;

    # setup childworkspace in given location
    my $cws = shift;
    my $wslocation = shift;
    my $master = $cws->master();
    my $child  = $cws->child();
    my $milestone  = $cws->milestone();
    my $dir = $opt_dir ? $opt_dir : cwd();
    my $success = 1;
    my $accessmaster = 1;

    # Ause: Deine Spielwiese
    my $result = 0;
    my $platform = "";
    my @found_platforms = ();
    my $dir_candidate = "";

    # hardcoded list of files which do not belong to any module delivery
    my @xtra_files = ( "*.mk", "*.flg", "libCrun*", "libgcc*", "libstdc*", "OOoRunner.jar" );

    # find location of master
    if ( "$wslocation" eq "" )
    {
        print "No access to matser workspace.\n";
        $accessmaster = 0;
    }
    else
    {
        print "location of master: \"$wslocation\"\n";
    }
    # append master name to keep setsolar happy
    $dir .= "/$child/$master";

    if ( $accessmaster )
    {
        # find platforms to copy
        if ( $#opt_platforms != -1 ) {
            @found_platforms = map( lc, @opt_platforms );
        } else {
            $result = opendir( SOLVER, "$wslocation/$master");
            if ( !$result ){ print_error ("Root dir of master workspace not accessible: $!", 1) };
            my @found_dirs = readdir( SOLVER );
            closedir( SOLVER );
            foreach $dir_candidate ( @found_dirs )
            {
                if ( -d "$wslocation/$master/$dir_candidate/inc.$milestone" )
                {
                    push @found_platforms, $dir_candidate;
                }
            }
            if ( !@found_platforms )
            {
                print_error("No valid output tree to copy", 0);
                $success = 0;
            }
        }
        # copy solver
        $sync_dir::do_keepzip = 1;
        my $btarget = "instset";
        foreach $platform ( @found_platforms )
        {
            %sync_dir::done_hash = ();
            print "Create copy of solver for $platform ( ~ 1GB disk space needed !)\n";
            my $zipsource = "$wslocation/$master/$platform/zip.$milestone";
            my $copy_dest = "$dir/$platform/zip.$milestone";
            if ( -d "$dir/$platform" )
            {
            #    print_error ("$dir/$platform : Please restart on a clean directory tree!", 1);
            }
            if ( ! -d $copy_dest )
            {
                $result = mkpath($copy_dest, 0, 0777-$umask);
                if ( !$result ){ print_error ("Cannot create output tree $copy_dest : $!", 1) };
            }

            my $unzip_dest = $copy_dest;
            $unzip_dest =~ s/(.*)\/.*$/$1/;

            if ( ! -e "$unzip_dest/prepared" ) {
                $result = sync_dir::prepare_minor_unzip( $unzip_dest, ".".$milestone );
                open( PREPARED, ">$unzip_dest/prepared");
                close( PREPARED );
            }

            STDOUT->autoflush(1);
            $result = &sync_dir::recurse_unzip( $zipsource, $copy_dest, $btarget );
            STDOUT->autoflush(0);
            if ( $result )
            {
                # renaming back before exit
                $result = sync_dir::finish_minor_unzip( $unzip_dest, ".".$milestone );
                print_error ("Copying files to $copy_dest failed : $!", 1);
            }
            $result = sync_dir::finish_minor_unzip( $unzip_dest, ".".$milestone );
            unlink "$unzip_dest/prepared.$milestone" if -e "$unzip_dest/prepared.$milestone";

        }
        foreach my $oneextra ( @xtra_files )
        {
            my @globlist = glob( "$wslocation/$master/[!s]*/*.$milestone/$oneextra" );
            if ( $#globlist == -1 ) {
                print "tried $oneextra in $wslocation/$master/[!s]*/*.$milestone/$oneextra\n";
            }
            foreach my $onefile ( @globlist )
            {
                my $destfile = $onefile;
                $destfile =~ s#$wslocation/$master#$dir#;

                if ( -d dirname( $destfile ))
                {
                    $result = copy $onefile,  $destfile;
                    if ( !$result ){ print_error ("Copying $onefile to CWS failed: $!", 1) };
                }
            }
        }
    }

    # create links & copy all projects from %obligatory_modules
    my $src_dest = "$dir/src.$milestone";

#    print "@found_sdirs\n";
    if ( ! -d $src_dest )
    {
        $result = mkpath($src_dest, 0, 0777-$umask);
        if ( !$result ){ print_error ("Cannot create source tree $src_dest : $!", 1) };
    }

    if ( $accessmaster )
    {
        $result = opendir( SOURCE, "$wslocation/$master/src.$milestone");
        if ( !$result ){ print_error ("Source dir of master workspace not accessible: $!", 1) };
        my @found_sdirs = readdir( SOURCE );
        closedir( SOURCE );

        if ( !@found_sdirs )
        {
            print_error("No valid source tree to copy", 0);
            $success = 0;
        }
        foreach my $onesdir ( @found_sdirs )
        {
            next if ( $onesdir =~ /^\.+$/ );

            # copy modules which are required to be accessable with their
            # orginal name without .lnk extension
            if (defined $obligatory_modules{$onesdir}) {
               &copyprj_module($onesdir, $src_dest);
               next ;
            };
            if ( -d "$wslocation/$master/src.$milestone/$onesdir" )
            {
                if ( -l "$src_dest/$onesdir.lnk" &&
                    readlink( "$src_dest/$onesdir.lnk" ) eq "$wslocation/$master/src.$milestone/$onesdir" )
                {
                    next;
                } else {
                # better...
                    $result = symlink( "$wslocation/$master/src.$milestone/$onesdir", "$src_dest/$onesdir.lnk");
                }
                if ( !$result ) {
                    print_error ( "Couldn't create link from $wslocation/$master/src.$milestone/$onesdir to $src_dest/$onesdir", 0);
                    $success = 0;
                };
            }
        }
    }
    # if we get here no critical error happend
#    return 0; # ause - disable all further steps
    return $success;
}

#
# Procedure copies module to specified path
#
sub copyprj_module {

    require CopyPrj; import CopyPrj;

    my $module_name = shift;
    my $src_dest = shift;

    # hash, that should contain all the
    # data needed by CopyPrj module
    my %ENVHASH = ();
    my %projects_to_copy = ();
    $ENVHASH{'projects_hash'} = \%projects_to_copy;
    $ENVHASH{'no_otree'} = 1;
    $ENVHASH{'no_path'} = 1;
    $ENVHASH{'only_otree'} = 0;
    $ENVHASH{'only_update'} = 1;
    $ENVHASH{'last_minor'} = 0;
    $ENVHASH{'spec_src'} = 0;
    $ENVHASH{'dest'} = "$src_dest";
    $ENVHASH{'prj_to_copy'} = '';
    $ENVHASH{'i_server'} = '';
    $ENVHASH{'current_dir'} = cwd();
    $ENVHASH{'remote'} = '';
    $ENVHASH{'force_checkout'} = 1 if ($force_checkout);

    $projects_to_copy{$module_name}++;

    CopyPrj::copy_projects(\%ENVHASH);

};

sub register_workspace
{
    # register child workspace with eis
    my $cws = shift;

    my $milestone = $cws->milestone();
    my $child     = $cws->child();
    my $master    = $cws->master();

    # collect some misc. information
    my $hostname = hostname();
    my $dir = $opt_dir ? $opt_dir : cwd();
    my $abspath = File::Spec->rel2abs("$dir/$child");
    my $vcsid = $ENV{VCSID};

    if ( $is_promotion ) {
        my $rc = $cws->promote($vcsid, "$hostname:$abspath");

        if ( !$rc ) {
            print_error("Failed to promote child workspace '$child' to status 'new'.\n", 5);
        }
        else {
            print "\n***** Successfully ***** promoted child workspace '$child' to status 'new'.\n";
            print "Milestone: '$milestone'.\n";
            return 1;
        }
    }
    else {

        my $eis_id = $cws->register($vcsid, "$hostname:$abspath");

        if ( !defined($eis_id) ) {
            print_error("Failed to register child workspace '$child' for master '$master'.", 5);
        }
        else {
            print "\n***** Successfully ***** registered child workspace '$child'\n";
            print "for master workspace '$master' (milestone '$milestone').\n";
            print "Child workspace Id: $eis_id.\n";
            return 1;
        }
    }
    return 0;
}

sub print_error
{
    my $message     = shift;
    my $error_code  = shift;

    print STDERR "$script_name: ";
    print STDERR "ERROR: $message\n";

    if ( $error_code ) {
        print STDERR "\n***** FAILURE: $script_name aborted. *****\n";
        $log->end_log_extended($script_name,$vcsid,$message) if defined($log);
        exit($error_code);
    }
}

sub print_message
{
    my $message     = shift;

    print "$script_name: ";
    print "$message\n";
    return;
}

sub print_warning
{
    my $message     = shift;

    print STDERR "$script_name: ";
    print STDERR "WARNING: $message\n";
    return;
}

sub usage
{
    print STDERR "Usage: cwscreate [-a] [-d dir] [-p <p1,...>] <mws_name> <milestone> <cws_name>\n";
    print STDERR "Creates a new child workspace <cws_name> for\n";
    print STDERR "milestone <milestone> of master workspace <mws_name>.\n";
    print STDERR "Options:\n";
    print STDERR "    -h        help\n";
    print STDERR "    -a        use cvs checkout instead of copying\n";
    print STDERR "    -d dir    create workspace in directory dir\n";
    print STDERR "    -p p1,p2,p3    only create workspace for specified platforms\n";
}
