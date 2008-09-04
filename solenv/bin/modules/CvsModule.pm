#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: CvsModule.pm,v $
#
# $Revision: 1.20 $
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
# CvsModule.pm - package for manipulating CVS modules
#

package CvsModule;
use strict;

use Benchmark;
use Carp;
use Cwd;
use FileHandle;
use File::Find;
use File::Basename;
use CwsConfig;

my $config = CwsConfig::get_config();

my %CvsModuleClassData = (
    CVS_BINARY              => $config->cvs_binary() . " -f",        # name of cvs binary
                                                                     # "-f" for overriding .cvsrc
    CVS_REMOTE              => $config->get_cvs_server(),            # name of remote server
    CVS_REMOTE_REPOSITORY   => $config->get_cvs_server_repository(), # remote repository
    CVS_MIRROR              => $config->get_cvs_mirror(),            # local cvsup mirror
    CVS_MIRROR_REPOSITORY   => $config->get_cvs_server_repository(), # mirror repository
    VCSID                   => $config->get_cvs_server_id()          # VCSID of CVS user
);

##### constructor ####

sub new
{
    my $invocant = shift;
    my $class = ref($invocant) || $invocant;
    my $self = {};
    $self->{MODULE}         = undef;    # module name
    $self->{VERBOSE}        = 0;        # verbose diagnostics
    $self->{CVS_SERVER}     = undef;    # name of CVS server
    $self->{CVS_METHOD}     = undef;    # checkout method
    $self->{CVS_REPOSITORY} = undef;    # CVS repository
    $self->{DEFS_CACHE}     = undef;    # cache for module definitions
    $self->{LOG_BUFFER}     = ();       # ring buffer for logging CVS messages
    bless ($self, $class);
    return $self;
}

#### instance data accessor methods ####

# generate remaining instance data accessor methods
# if this looks strange see 'perldoc perltootc'
for my $datum (qw(module verbose)) {
    no strict "refs";
    *$datum = sub {
        my $self = shift;
        $self->{uc($datum)} = shift if @_;
        return $self->{uc($datum)};
    }
}

# if this looks strange see 'perldoc perltootc'
for my $datum (qw(cvs_server cvs_method cvs_repository)) {
    no strict "refs";
    *$datum = sub {
        my $self = shift;
        my $ucdatum = uc($datum);
        if ( @_ ) {
            $self->{$ucdatum} = shift if @_;
        }
        else {
            $self->get_root() unless $self->{$ucdatum};
        }
        return $self->{$ucdatum};
    }
}

#### class data accessor methods

# generate class data accessor methods
for my $ucdatum (keys %CvsModuleClassData) {
    my $datum = lc($ucdatum);
    no strict "refs";
    *$datum = sub {
        shift; # ignore calling class/object
        return $CvsModuleClassData{$ucdatum};
    }
}

#
# this procedure provides common output
# the result of update method
#
sub handle_update_information {
    my ($self, $updated_files_ref) = @_;
    my ($updated, $merged, $conflicts);

    foreach ( @$updated_files_ref ) {
        print "\t$_->[1]\t$_->[0]\n";
        if ( $_->[1] eq 'P' || $_->[1] eq 'U' ) {
            $updated++;
        }
        elsif ( $_->[1] eq 'M' ) {
            $merged++;
        }
        elsif ( $_->[1] eq 'C' ) {
            $conflicts++;
        }
        else {
            # can't happen
            croak("ERROR: handle_update_information(): internal error");
        }
    }
    print("\t\tUpdated: $updated\n") if $updated;
    print("\t\tMerged: $merged\n") if $merged;
    print("\t\tConflict(s): $conflicts\n") if $conflicts;
    return ($updated, $merged, $conflicts);
};

#
# this procedure patches CVS/Root file in module current path
# actions available: 'user' 'server'. Default - both
#
sub patch_cvs_root_file {
    my ($self, $path, $action) = @_;
    my @files;
    find sub { push @files, $File::Find::name if -d _ && /CVS$/},
    $path . '/' . $self->module();
    foreach my $cvs_dir (@files) {
        my $root_file = "$cvs_dir/Root";
        next if (!-f $root_file);
        if (!open(ROOT, "<$cvs_dir/Root")) {
            croak("ERROR: patch_cvs_root_file(): can't open file '$root_file'");
        }
        my $line = <ROOT>;
        close(ROOT);

        # patch root
        $action = '' if (!defined $action);
        if ($action ne 'server') {
            # in this case patching user won't be the wrong thing
            croak ("ERROR: patch_cvs_root_file(): file '$root_file' has wrong format") if ($line !~ /:(\w+)@/o);
            $line = "$`:" . $self->vcsid(). "\@$'";
        }
        if ($action ne 'user') {
            # in this case patching server won't be the wrong thing either
            croak ("ERROR: patch_cvs_root_file(): file '$root_file' has wrong format") if ($line !~ /@/o);
            $line = "$`\@" . $self->cvs_server() . ":" . $self->cvs_repository() . "\n";
        }
        open(ROOT, ">$root_file") or croak ("ERROR: patch_cvs_root_file(): can't write file '$root_file'");
        print ROOT $line;
        close(ROOT);
    };
};

#### additional public methods ####

#### instance methods #####

# Checkout module to specified scratch area
# If CVS_SERVER matches CVS_REMOTE do a checkout
# from CVS_MIRROR first and than update/checkout
# via CVS_SERVER.
# Otherwise do direct checkout.
# Returns a list of entries corresponding to the files which have been
# checked out or 'nofilesupdated'
# The entries of the returned list have the form [$file, 'U']
sub checkout
{
    my $self    = shift;
    my $path    = shift;
    my $tag     = shift;
    my $options = shift;


    my $module  = $self->module();

    if ( !$module ) {
        croak("ERROR: CvsModule::checkout(): no module for checkout specified");
    }
    if ( ! -d $path ) {
        croak("ERROR: CvsModule::checkout(): invalid local path for checkout specified");
    }

    # chdir to checkout area
    my $saved_cwd = cwd();
    if ( !chdir($path) ) {
        croak("ERROR: CvsModule:: can't chdir() to '$path'");
    }

    my $from_mirror = 0;
    my $update_only = 0;
    if ( $self->cvs_server() eq $self->cvs_remote() && defined($self->cvs_mirror()) ) {
        # check if module has already been checked out
        $from_mirror = 1;
        if ( -r "$module/CVS/Root" ) {
            open(ROOT, "<$module/CVS/Root");
            my @lines = <ROOT>;
            close(ROOT);
            if ( $lines[0] =~ $self->cvs_server() ) {
                $update_only = 1;
                $from_mirror = 0;
            }
        }
    }

    my $dirs_ref;
    my $files_ref;
    if ( $from_mirror ) {
        ($dirs_ref, $files_ref) = $self->do_checkout($self->cvs_mirror(), '', $options);
        if ( @{$dirs_ref} ) {
            my $mirror = $self->cvs_mirror();
            my $remote = $self->cvs_remote();
            my $mirror_rep = $self->cvs_mirror_repository();
            my $remote_rep = $self->cvs_remote_repository();
            $self->patch_root($mirror, $remote, $mirror_rep, $remote_rep, $dirs_ref);
        }
        my $module_dir = $module;
        $module_dir = dirname($module) if ($module =~ /\\|\//);
        chdir($module_dir);
        # FIXME We should add an option '-d' here to add directories which
        # have been added to the server but not yet synced to the mirror,
        # Unfortunately a bug in CVS prevents us from doing so.
        my $updated_files_ref;
        my $updated_dirs_ref;
        my %files_hash;
        ($updated_dirs_ref, $updated_files_ref) = $self->do_update($tag, $options);
        if ( @{$updated_files_ref} ) {
            # Ok, something changed in the mean time
            # create hash for faster searching
            foreach (@{$files_ref}) {
                $files_hash{$_->[0]}++;
            }
            # iterate over updated files from remote and add them if they
            # are not yet in $files_ref
            foreach (@{$updated_files_ref}) {
                push(@{$files_ref}, $_) if !exists $files_hash{$_->[0]};
            }
        }
    }
    else {
        if ( $update_only ) {
            chdir($module);
            ($dirs_ref, $files_ref) = $self->do_update($tag, $options);
        }
        else {
            ($dirs_ref, $files_ref) = $self->do_checkout($self->cvs_server(), $tag, $options);
        }
    }

    # chdir() back
    chdir($saved_cwd);
    return defined($files_ref) ? $files_ref : 'nofilesupdated';
}

# Update module.
# Returns a list of entries corresponding to the files which have been
# updated or 'nofilesupdated'.
# The entries of the returned list have the form [$file, 'U|P|M|C'].
# Parameters:
#   1) path with top level diretory to be updated
#   2) possible CVS tag
#   3) update options, ie '-dP'
#   4) if true, then update() will return a second reference with all
#      unknown files - those which are marked by '?' by CVS.
sub update
{
    my $self                   = shift;
    my $path                   = shift;
    my $tag                    = shift;
    my $options                = shift;
    my $return_unknown_entries = shift || '';


    my $module  = $self->module();

    if ( !$module ) {
        croak("ERROR: CvsModule::update(): no module for checkout specified");
    }
    if ( !-d "$path/$module" ) {
        croak("ERROR: CvsModule::update(): can't find '$path/$module'");
    }

    # chdir to update area
    my $saved_cwd = cwd();
    if ( !chdir("$path/$module") ) {
        croak("ERROR: CvsModule::update(): can't chdir() to '$path/$module'");
    }

    my ($dirs_ref, $files_ref, $unknown_ref) = $self->do_update($tag, $options);

    # chdir() back
    chdir($saved_cwd);
    if ( $return_unknown_entries ) {
        return ($files_ref, $unknown_ref);
    }
    else {
        return defined($files_ref) ? $files_ref : 'nofilesupdated';
    }
}

# Find all changed files in a module vs. a specfic tag
# return a LoL: [name of file, rev_old, rev_new].
sub changed_files
{
    my $self     = shift;
    my $tag_old  = shift;
    my $tag_new  = shift;

    my $module     = $self->module();
    my $cvs_binary = $self->cvs_binary();

    my $root = $self->get_rcmd_root();

    my $option_tag_old = '-r' . $tag_old;
    my $option_tag_new = '-r' . $tag_new;
    my $verbose = $self->verbose();
    my ($t1, $t0);
    if ( $verbose > 1 ) {
        $t0 = Benchmark->new();
        autoflush STDOUT 1;
        print "checking for changed files in module '$module'; $option_tag_old $option_tag_new\n";
    }

    my $server_died_silently = 1;
    my @changed_files = ();
    open(RDIFF, "$cvs_binary -d $root rdiff -s $option_tag_old $option_tag_new $module 2>&1 |");
    while(<RDIFF>) {
        $self->append_to_log($_);
        if ( /^cvs (server|rdiff): Diffing (.*)$/ ) {
            print "." if $verbose;
            $server_died_silently = 0;
        }
        if ( /\[rdiff aborted\]: connect to/ ) {
            croak("ERROR: CvsModule::changed_files(): connection to server failed");
        }
        if ( /^File (.+?) / ) {
            my $file_name = $1;
            my ($rev_old, $rev_new);
            if ( /changed from revision ([\d\.]+) to ([\d\.]+)/ ) {
                $rev_old = $1;
                $rev_new = $2;
            }
            elsif ( /is new; (current|$tag_new) revision ([\d\.]+)/ ) {
                $rev_new = $2;
                $rev_old = undef;
            }
            elsif ( /is removed; not included in release tag/ ||
                    /is removed; $tag_old revision ([\d\.]+)/ ) {
                $rev_new = undef;
                $rev_old = undef;
            }
            else {
                croak("ERROR: CvsModule::changed_files(): unexpected output from rdiff");
            }

            $file_name = $self->strip_module_from_path($file_name);
            push(@changed_files, [$file_name, $rev_old, $rev_new]);
        }
    }
    close(RDIFF);
    print "\n" if $verbose;
    $self->die_on_error_code('CvsModule::changed_files()');
    $self->clear_log();
    if ( $verbose > 1) {
        $t1 = Benchmark->new();
        print "rdiff time: " . timestr(timediff($t1, $t0),'nop') . "\n";
        autoflush STDOUT 0
    }
    if ( $server_died_silently ) {
        croak("ERROR: CvsModule::changed_files(): server died silently");
    }
    return wantarray ? @changed_files : \@changed_files;
}

#  Tag all files in module with given tag,
#  returns number of newly tagged files and number,
#  of warnings/errors due to already existing tags.
sub tag
{
    my $self    = shift;
    my $path    = shift;
    my $tag     = shift;
    my $options = shift;

    my $module     = $self->module();
    my $cvs_binary = $self->cvs_binary();

    my $branch = ($options && $options =~ /-b/) ? '-b' : '';
    my $force  = ($options && $options =~ /-F/) ? '-F' : '';

    my $verbose = $self->verbose();
    my ($t1, $t0);
    if ( $verbose > 1) {
        $t0 = Benchmark->new();
        autoflush STDOUT 1;
        print "tag module '$module' with " ;
        print $branch ? "branch " : "";
        print "tag '$tag'\n";
    }
    my $tagged_files = 0;
    my $tag_errors   = 0;
    my $saved_cwd = cwd();
    if ( !chdir("$path/$module") ) {
        croak("ERROR: CvsModule::tag(): can't chdir() to directory '$path/$module'");
    }
    open(TAG, "$cvs_binary tag $force $branch $tag 2>&1 |");
    while(<TAG>) {
        $self->append_to_log($_);
        if ( /\[.* aborted\]: connect to/ ) {
            croak("ERROR: CvsModule::tag(): connection to server failed");
        }
        if ( /^cvs (server|tag): Tagging (.*)$/ ) {
            print "." if $verbose;
        }
        elsif ( /^T / ) {
            $tagged_files++;
        }
        elsif ( /^W / ) {
            # can't move tag because tag already exists and
            # force option -F not specified
            my $line = $_;
            $line =~ s/^W //;
            chomp($line);
            carp("ERROR: CvsModule::tag():" . "$line");
            $tag_errors++;
        }
    }
    close(TAG);
    print "\n" if $verbose;
    $self->die_on_error_code('CvsModule::tag()');
    $self->clear_log();
    if ( $verbose > 1) {
        $t1 = Benchmark->new();
        print "tagging time: " . timestr(timediff($t1, $t0),'nop') . "\n";
        autoflush STDOUT 0
    }
    chdir($saved_cwd);
    return ($tagged_files, $tag_errors);
}

sub get_aliases_hash {
    my $self    = shift;
    my $cvs_binary = $self->cvs_binary();
    my $method     = $self->cvs_method();
    my $server     = $self->cvs_server();
    my $repository = $self->cvs_repository();
    my $vcsid      = $self->vcsid();

    if ( !$vcsid ) {
        croak("ERROR: CvsModule::get_aliases_hash(): VCSID not set");
    }

    my $root = ":$method:$vcsid\@$server:$repository";
    my $commando = "$cvs_binary -d $root checkout -c";
    if(!open(CHECKOUT, "$commando 2>&1 |")) {
        croak("ERROR: get_aliases_hash(): Cannot run commando '$commando'");
    };
    my %aliases_hash = ();
    my $last_alias = '';
    my $string = '';
    while(<CHECKOUT>) {
        $self->append_to_log($_);
        if ( /\[.* aborted\]: connect to/ ) {
            croak("ERROR: CvsModule::get_aliases_hash(): connection to server failed");
        }
        if (/^(\S+)\s+(.+)$/o) {
            $last_alias = $1;
            $string = $2;
        } elsif (/^(\s+)(.+)$/o && $last_alias) {
            $string = $aliases_hash{$last_alias} . " $2";
        } else {
            $last_alias = '';
            next;
        };
        $aliases_hash{$last_alias} = $string;
    };
    close CHECKOUT;
    $self->die_on_error_code('CvsModule::get_aliases_hash()');
    $self->clear_log();
    return %aliases_hash;
};

#### private helper methods #####

sub do_checkout
{
    my $self    = shift;
    my $server  = shift;
    my $tag     = shift || '';
    my $options = shift || '';

    my $vcsid = $self->vcsid();

    if ( !$vcsid ) {
        croak("ERROR: CvsModule::do_checkout: VCSID not set");
    }

    my $module     = $self->module();
    my $cvs_binary = $self->cvs_binary();
    my $method     = $self->cvs_method();
    my $repository = $self->cvs_repository();
    my $root = ":$method:$vcsid\@$server:$repository";

    $tag = '-r' . $tag if $tag ne '';

    # do the checkout
    my @updated_dirs;
    my @updated_files;
    my $verbose = $self->verbose();
    my ($t1, $t0);
    if ( $verbose > 1) {
        $t0 = Benchmark->new();
        autoflush STDOUT 1;
        print "checkout module '$module' from $server'\n";
    }
    open(CHECKOUT, "$cvs_binary -d $root checkout $tag $options $module 2>&1 |");
    while(<CHECKOUT>) {
        $self->append_to_log($_);
        if ( /\[.* aborted\]: connect to/ ) {
            croak("ERROR: CvsModule::do_checkout(): connection to server failed");
        }
        if ( /^(cvs|repository) (server|checkout): Updating (.*)$/ ) {
            print "." if $verbose;
            push(@updated_dirs, $3);
        }
        if ( /^([U|M|P|C]) (.*)$/ ) {
            push(@updated_files, [$2, $1]);
        }
    }
    close(CHECKOUT);
    print "\n" if $verbose;
    $self->die_on_error_code('CvsModule::do_checkout()');
    $self->clear_log();
    if ( $verbose > 1 ) {
        $t1 = Benchmark->new();
        print "checkout time: " . timestr(timediff($t1, $t0),'nop') . "\n";
        autoflush STDOUT 0;
    }
    return (\@updated_dirs, \@updated_files);
}

sub do_update
{
    my $self    = shift;
    my $tag     = shift;
    my $options = shift;

    my $module     = $self->module();
    my $cvs_binary = $self->cvs_binary();

    $options = $options ? $options : '';
    $tag     = '-r' . $tag if $tag ne '';

    # sever for update is never a mirror, always the 'real' server
    my $server = $self->cvs_server();

    # do the update
    my @updated_dirs;
    my @updated_files;
    my @unknown_entries;
    my $verbose = $self->verbose();
    my ($t1, $t0);
    if ( $verbose > 1) {
        $t0 = Benchmark->new();
        autoflush STDOUT 1;
        print "update module '$module' from '$server'\n";
    }
    open(UPDATE, "$cvs_binary update $tag $options 2>&1 |");
    while(<UPDATE>) {
        $self->append_to_log($_);
        if ( /\[.* aborted\]: connect to/ ) {
            croak("ERROR: CvsModule::do_update(): connection to server failed");
        }
        if ( /^cvs (server|update): Updating (.*)$/ ) {
            print "." if $verbose;
            push(@updated_dirs, $2);
        }
        if ( /^([U|M|P|C]) (.*)$/ ) {
            push(@updated_files, [$2, $1]);
        }
        if ( /^\? (.*)$/ ) {
            push(@unknown_entries, $1);
        }
    }
    close(UPDATE);
    print "\n" if $verbose;
    $self->die_on_error_code('CvsModule::do_update()');
    $self->clear_log();
    if ( $verbose > 1) {
        $t1 = Benchmark->new();
        print "update time: " . timestr(timediff($t1, $t0),'nop') . "\n";
        autoflush STDOUT 0
    }
    return (\@updated_dirs, \@updated_files, \@unknown_entries);
}

sub get_root
{
    # Try two methods to determine CVS root.
    my $self = shift;
    my $module = $self->module();

    my $cvs_root;
    if ( $module && -r "$module/CVS/Root" ) {
        # Test if there is a checked out module.
        open(ROOT, "<$module/CVS/Root");
        my @root = <ROOT>;
        close(ROOT);
        $cvs_root = $root[0];
    }
    else {
        # alternatively check CVSROOT environment variable
        $cvs_root = $ENV{CVSROOT};
    }

    if ( $cvs_root ) {
        my ($dummy, $method, $vcsid_server, $repository) = split(/:/, $cvs_root);
        # Remove port number from repository path;
        $repository =~ s/^\d*//;
        my ($vcsid, $server)  = split('@', $vcsid_server);
        if ( !($method && $vcsid && $server && $repository) ) {
            croak("ERROR: CvsModule::get_root(): can't determine CVS Server");
        }
        # sanity check
        if ( $vcsid ne $self->vcsid() ) {
            croak("ERROR: CvsModule::get_root(): environment VCSID and CVS server root differ");
        }
        $self->cvs_method($method);
        $self->cvs_server($server);
        $self->cvs_repository($repository);
        return;
    }
    return;
}

sub patch_root
{
    # Patch the server part of the root from old to new.
    shift; # ignore invocant
    my $old_server = shift;
    my $new_server = shift;
    my $old_rep    = shift;
    my $new_rep    = shift;
    my $dirs_ref   = shift;

    foreach (@{$dirs_ref}) {
        # pruned directories may not exist
        if ( -d $_ ) {
            my $root = "$_/CVS/Root";
            open(ROOT, "<$root") or croak("ERROR: CvsModule::patch_root(): can't open file '$root'");
            my $line = <ROOT>;
            close(ROOT);

            # patch root
            $line =~ s/$old_server/$new_server/o;  # note: evaluate reg exp. only once
            open(ROOT, ">$root") or croak("ERROR: CvsModule::patch_root(): can't write '$root'");
            print ROOT $line;
            close(ROOT);
            # repository will usually not change
            if ( $old_rep ne $new_rep ) {
                my $rep = "$_/CVS/Repository";
                open(REPOSITORY, "<$rep") or croak("ERROR: CvsModule::patch_root(): can't open '$rep'");
                my $line = <REPOSITORY>;
                close(REPOSITORY);

                # patch rep
                $line =~ s/$old_rep/$new_rep/o;  # note: evaluate reg exp. only once
                open(REPOSITORY, ">$rep") or croak("ERROR: CvsModule::patch_root(): can't write '$rep'");
                print REPOSITORY $line;
                close(REPOSITORY);
            }
        }
    }
}

# get the root for r-type commands
sub get_rcmd_root
{
    my $self = shift;

    my $vcsid = $self->vcsid();

    if ( !$vcsid ) {
        croak("ERROR: CvsModule::get_rcmd_root(): VCSID not set");
    }

    my $method     = $self->cvs_method();
    my $repository = $self->cvs_repository();
    my $server     = $self->cvs_server();
    my $remote     = $self->cvs_remote();
    my $root       = ":$method:$vcsid\@$server:$repository";


    # FIXME OOo's CVS server is pretty much broken. It's impossible
    # to use the r-type CVS commands (rtag, rdiff) with the regular cvs root.
    # We have to patch the cvs root. This horrendous hack should be removed as
    # soon as the server is fixed
    if ( $server =~ /$remote/o ) {
        $repository = '/shared/data/helm/cvs/repository';
        $root = ":$method:$vcsid\@$server:$repository";
        if ( !is_valid_login(":$method:$vcsid\@$server:", $repository ) ) {
            print STDERR "\nThe cvs rdiff command is broken for the OOo CVS server.\n";
            print STDERR "To fix this problem you have to issue the following cvs login command:\n\n";
            print STDERR "    cvs -d $root login\n\n";
            print STDERR "The password is your usual OOo password.\n\n";
            exit(1);
        }
    }

    return $root;
}

# Check if a valid login command has been
# issued for the root which is passed
# as argument.
# Needed for r-type command hack. sigh.
sub is_valid_login
{
    my $url = shift;
    my $repo = shift;

    my $home = $ENV{HOME};
    open(CVSPASSWD, "<$home/.cvspass") or return 0;
    my @lines = <CVSPASSWD>;
    close(CVSPASSWD);

    my $is_valid = 0;
    foreach (@lines) {
        if ( $_ =~ /${url}\d*${repo}/o ) {
            $is_valid = 1;
            last;
        }
    }

    return $is_valid;
}

# Returns a hash_ref with alias for all modules
sub get_module_definitions
{
    my $self = shift;

    my $cvs_binary = $self->cvs_binary();
    my $root       = $self->get_rcmd_root();

    my @entries;
    open(MODULESLIST, "$cvs_binary -d $root checkout -c 2>&1 |");
    while(<MODULESLIST>) {
        chomp();
        # TODO more error checking
        if ( /\[checkout aborted\]: connect to/ ) {
            croak("ERROR: CvsModule::get_module_definitions(): connection to server failed");
        }
        # Module list format:
        # A entry starts on the first column, otherwise
        # we have a continuation line
        if ( /^\S/ ) {
            push(@entries, $_);
        }
        else {
            $entries[-1] .= $_;
        }
    }
    close(MODULESLIST);

    my %mod_defs;
    foreach ( @entries ) {
        my ($name, $definition, $extra) = split(' ', $_);
        if ( $extra || $definition =~ /&/ ) {
            # if the entries splits in more than
            # two entries or the definition
            # contains an ampersand than this can't
            # be a regular module definition
            next;
        }
        $mod_defs{$name} = $definition;
    }
    return \%mod_defs;
}

# Strip elements from the front of a path to yield
# a filename relative to module. If this fails retrieve
# the module list from the server and determine the number
# of path elements to be stripped from the the module
# definition.
sub strip_module_from_path
{
    my $self = shift;
    my $file = shift;

    my $module = $self->module;

    # Test if the file name is of the form
    # project/module/pathelem/..../filename <= OOo server
    # module/pathelem/.../filename          <= local server
    # where project and module can be identical
    # If the test fails try as last resort to
    # retrieve the module list from the server and
    # determine the number of path elements to be stripped
    # from the the module definition.
    #
    my @elems = split(/\//, $file);

    my $elem = shift @elems;
    if ( $elem eq $module ) {
        $elem = shift @elems;
        if ( $elem ne $module ) {
            unshift(@elems, $elem);
        }
        return join('/', @elems);
    }
    else {
        $elem = shift @elems;
        if ( $elem eq $module ) {
            return join('/', @elems);
        }
        else {
            # try the module definitions from the module list
            if ( !defined($self->{DEFS_CACHE}) ) {
                $self->{DEFS_CACHE} = $self->get_module_definitions();
            }
            if ( exists $self->{DEFS_CACHE}->{$module} ) {
                my $definition = $self->{DEFS_CACHE}->{$module};
                $file =~ /^$definition\/(.*)$/;
                return $1;
            }
            else {
                croak("ERROR: CvsModule::strip_module_from_path(): internal error");
            }
        }
    }
}

#
# Procedure does the same as "cvs view",
# extracted to the module in order to provide
# consistency for future implementations
#
sub view {
    my $self    = shift;
    my $path    = shift;
    my $saved_cwd = cwd();
    if ( !chdir($path) ) {
        croak("ERROR: CvsModule::view(): can't chdir() to '$path'");
    }
    cwd();
    my $module     = $self->module();
    my $cvs_binary = $self->cvs_binary();

    my $verbose = $self->verbose();
    my ($info, $seen,  @field);
    my $line = "$cvs_binary status -R";
    # provide info in hash
    my @view_info = ();
    open(REPOSITORY, 'CVS/Repository');
    my $repository = <REPOSITORY>;
    close REPOSITORY;
    $repository =~ s/[\s\r\n]//g;
    $repository =~ s/$module$//g;

    open (CVSVIEW , "$line 2>&1 |") or croak("ERROR: CvsModule::view(): can't run command '$line'");

    $seen = 0;

    # check error
    if ( $? >> 8  ) {
        close(CVSVIEW);
        croak("ERROR: CvsModule::view(): view failed!\n");
    }

    while(<CVSVIEW>) {
        $line = $_;
        chomp $line;

        if ( $line =~ /^\?/ ) {
            print ("$line\n");
            next;
        }

        if ( $line =~ /Needs\sCheckout/o ) {
            $info = "needs checkout";
            next;
        }

        if ( $line =~ /Needs\sPatch/o ) {
            $info = "needs patch";
            next;
        }

        if ( $line =~ /Needs\sMerge/o ) {
            $info = "needs merge";
            next;
        }

        if ( $line =~ /Locally\sAdded/o ) {
            @field = split /\s+/, $line;
            my $info_string = "$field[1]: locally added\n";
            print $info_string;
            push (@view_info, $info_string);
            next;
        }

        if ( $line =~ /Locally\sModified/o ) {
            $info = "locally modified";
            next;
        }

        if ( $line =~ /Locally\sRemoved/o ) {
            $info = "locally removed";
            next;
        }

        if ($line =~ /conflicts/o ) {
            $info = "conflicts on merge";
            next;
        }

        if ($info && $line =~ /Repository/o ) {
            @field = split /\s+/, $line;
            my $info_line = "$field[4]: $info\n";
            print $info_line;
            $info_line =~ s/,\S+:/:/;
            $info_line =~ s/^$repository//;
            push (@view_info, $info_line);
            $info = 0;
            next;
        }

        if ( $line =~/==============/ ) {
            $info = 0;
            $seen = 1;
        }
    }
    close (CVSVIEW);
    chdir($saved_cwd);
    cwd();

    if ( !$seen ) {
        print STDERR "potential \"cvs view\" failure, please use \"cvs status\"\n";
        print STDERR "to examine error condition\n";
    }
    return \@view_info;
}

# Simple minded ring buffer for keeping the last lines of the CVS output
{
    my $nlog_size  = 5;
    my $nindex     = 0;
    sub append_to_log
    {
        my $self = shift;
        my $line = shift;

        $nindex++;
        if ( $nindex >= $nlog_size ) {
            $nindex = 0;
        }
        $self->{LOG_BUFFER}->[$nindex] = $line;
    }

    sub get_log
    {
        my $self = shift;

        my $first = $nindex+1;
        if ( $first >= $nlog_size ) {
            $first = $first - $nlog_size;
        }

        my $log = "";
        for (my $i = 0; $i < $nlog_size; $i++) {
            my $n = $first + $i;
            if ( $n >= $nlog_size ) {
                $n = $n - $nlog_size;
            }
            if ( $self->{LOG_BUFFER}->[$n] ) {
                $log .= $self->{LOG_BUFFER}->[$n];
            }
        }
        return $log;
    }

    sub clear_log
    {
        my $self = shift;

        $self->{LOG_BUFFER} = ();
    }
}

sub die_on_error_code
{
    my $self = shift;

    my $method = shift;
    my $errcode = $? >> 8;

    if ( $errcode ) {
        my $error_message = "ERROR: $method: CVS client returned error code '$errcode'!\n";
        $error_message .= "The last 5 CVS messages leading up to the problem were:\n";
        $error_message .= $self->get_log();

        croak($error_message);
    }
}


####

1; # needed by "use" or "require"
