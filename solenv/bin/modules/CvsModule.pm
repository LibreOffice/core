#*************************************************************************
#
#   $RCSfile: CvsModule.pm,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: vg $ $Date: 2004-07-27 14:52:51 $
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
# CvsModule.pm - package for manipulating CVS modules
#

package CvsModule;
use strict;

use Benchmark;
use Carp;
use Cwd;
use FileHandle;
use File::Find;
use CwsConfig;

my $config = CwsConfig::get_config();

my %CvsModuleClassData = (
    CVS_BINARY              => $config->cvs_binary(),                # name of cvs binary
    CVS_REMOTE              => $config->get_cvs_server(),            # name of remote server
    CVS_REMOTE_REPOSITORY   => $config->get_cvs_server_repository(), # remote repository
    CVS_MIRROR              => $config->get_cvs_mirror(),            # local cvsup mirror
    CVS_MIRROR_REPOSITORY   => $config->get_cvs_server_repository(), # mirror repository
    VCSID                   => $config->get_cvs_server_id()          # VCSID of CVS user
);

##### ctor ####

sub new
{
    my $invocant = shift;
    my $class = ref($invocant) || $invocant;
    my $self = {};
    $self->{MODULE}         = undef;    # module name
    $self->{VERBOSE}        = 0;        # verbose diagnostics
    $self->{CVS_SERVER}     = undef;    # name of CVS server
    $self->{CVS_METHOD}     = undef;    # checkout method
    $self->{CVS_REPOSITORY} = undef;    # CVS reposiory
    $self->{DEFS_CACHE}     = undef;    # cache for module definitions
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
sub handle_update_infomation {
    my ($self, $updated_files_ref) = @_;
    my ($updated, $merged, $conflicts);
    if ( $updated_files_ref eq 'invaildpath' || $updated_files_ref eq 'cantchdir') {
        die('ERROR: Can\'t chdir() into module'. $self->module());
    }
    else {
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
                die("ERROR: internal error in update_module()");
            }
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
            die("ERROR: can't open $root_file");
        }
        my $line = <ROOT>;
        close(ROOT);

        # patch root
        $action = '' if (!defined $action);
        if ($action ne 'server') {
            # in this case patching user won't be the wrong thing
            die ("Wrong format $root_file") if ($line !~ /:(\w+)@/o);
            $line = "$`:" . $self->vcsid(). "\@$'";
        }
        if ($action ne 'user') {
            # in this case patching server won't be the wrong thing either
            die ("Wrong format $root_file") if ($line !~ /@/o);
            $line = "$`\@" . $self->cvs_server() . ":" . $self->cvs_repository() . "\n";
        }
        open(ROOT, ">$root_file") or die ("ERROR: can't write $root_file");
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
        carp("ERROR: no module for checkout specified");
        return "modulenotset";
    }
    if ( ! -d $path ) {
        carp("ERROR: invalid local path for checkout specified");
        return "invalidpath";
    }

    # chdir to checkout area
    my $saved_cwd = cwd();
    if ( !chdir($path) ) {
        carp("ERROR: can't chdir() to $path");
        return "cantchdir";
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
        chdir($module);
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
sub update
{
    my $self    = shift;
    my $path    = shift;
    my $tag     = shift;
    my $options = shift;


    my $module  = $self->module();

    if ( !$module ) {
        carp("ERROR: no module for checkout specified");
        return "modulenotset";
    }
    if ( !-d "$path/$module" ) {
        carp("ERROR: can't find '$path/$module'");
        return "invalidpath";
    }

    # chdir to update area
    my $saved_cwd = cwd();
    if ( !chdir("$path/$module") ) {
        carp("ERROR: can't chdir() to '$path/$module'");
        return "cantchdir";
    }

    my ($dirs_ref, $files_ref) = $self->do_update($tag, $options);

    # chdir() back
    chdir($saved_cwd);
    return defined($files_ref) ? $files_ref : 'nofilesupdated';
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

    $tag_old = '-r' . $tag_old;
    $tag_new = '-r' . $tag_new;
    my $verbose = $self->verbose();
    my ($t1, $t0);
    if ( $verbose > 1 ) {
        $t0 = Benchmark->new();
        autoflush STDOUT 1;
        print "checking for changed files in module '$module'; $tag_old $tag_new\n";
    }

    my @changed_files = ();
    open(RDIFF, "$cvs_binary -d $root rdiff -s $tag_old $tag_new $module 2>&1 |");
    while(<RDIFF>) {
        # TODO more error checking
        if ( /^cvs server: Diffing (.*)$/ ) {
            print "." if $verbose;
        }
        if ( /\[rdiff aborted\]: connect to/ ) {
            carp("ERROR: connection to server failed");
            return 'connectionfailure';
        }
        if ( /^File (.+?) / ) {
            my $file_name = $1;
            my ($rev_old, $rev_new);
            if ( /changed from revision ([\d\.]+) to ([\d\.]+)/ ) {
                $rev_old = $1;
                $rev_new = $2;
            }
            elsif ( /is new; current revision ([\d\.]+)/ ) {
                $rev_new = $1;
                $rev_old = undef;
            }
            elsif ( /is removed; not included in release tag/ ) {
                $rev_new = undef;
                $rev_old = undef;
            }
            else {
                carp("ERROR: unexpected output from rdiff");
            }

            $file_name = $self->strip_module_from_path($file_name);
            push(@changed_files, [$file_name, $rev_old, $rev_new]);
        }
    }
    close(RDIFF);
    print "\n" if $verbose;
    if ( $verbose > 1) {
        $t1 = Benchmark->new();
        print "rdiff time: " . timestr(timediff($t1, $t0),'nop') . "\n";
        autoflush STDOUT 0
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
        carp("ERROR: can't chdir to directory $path/$module");
        return (undef, undef);
    }
    open(TAG, "$cvs_binary tag $force $branch $tag 2>&1 |");
    while(<TAG>) {
        # TODO error checking
        if ( /^cvs server: Tagging (.*)$/ ) {
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
            carp("ERROR: " . "$line");
            $tag_errors++;
        }
    }
    close(TAG);
    print "\n" if $verbose;
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
    my $root = ":$method:$vcsid\@$server:$repository";
    my $commando = "$cvs_binary -d $root checkout -c";
    if(!open(CHECKOUT, "$commando 2>&1 |")) {
        die("Cannot run $commando");
    };
    my %aliases_hash = ();
    my $last_alias = '';
    my $string = '';
    while(<CHECKOUT>) {
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
        carp("ERROR: VCSID not set");
        return "invalidvcsid";
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
        # TODO error checking
        if ( /^cvs server: Updating (.*)$/ ) {
            print "." if $verbose;
            push(@updated_dirs, $1);
        }
        if ( /^([U|M|P|C]) (.*)$/ ) {
            push(@updated_files, [$2, $1]);
        }
    }
    close(CHECKOUT);
    print "\n" if $verbose;
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
    my $verbose = $self->verbose();
    my ($t1, $t0);
    if ( $verbose > 1) {
        $t0 = Benchmark->new();
        autoflush STDOUT 1;
        print "update module '$module' from '$server'\n";
    }
    open(UPDATE, "$cvs_binary -z6 update $tag $options 2>&1 |");
    while(<UPDATE>) {
        # TODO error checking
        if ( /^cvs server: Updating (.*)$/ ) {
            print "." if $verbose;
            push(@updated_dirs, $1);
        }
        if ( /^([U|M|P|C]) (.*)$/ ) {
            push(@updated_files, [$2, $1]);
        }
    }
    close(UPDATE);
    print "\n" if $verbose;
    if ( $verbose > 1) {
        $t1 = Benchmark->new();
        print "update time: " . timestr(timediff($t1, $t0),'nop') . "\n";
        autoflush STDOUT 0
    }
    return (\@updated_dirs, \@updated_files);
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
        my ($vcsid, $server)  = split('@', $vcsid_server);
        if ( !($method && $vcsid && $server && $repository) ) {
            carp("ERROR: can't determine CVS Server");
            return;
        }
        # sanity check
        if ( $vcsid ne $self->vcsid() ) {
            carp("ERROR: environment VCSID and CVS server root differ");
            return;
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
            open(ROOT, "<$root") or carp("ERROR: can't open $root");
            my $line = <ROOT>;
            close(ROOT);

            # patch root
            $line =~ s/$old_server/$new_server/o;  # note: evaluate reg exp. only once
            open(ROOT, ">$root") or carp("ERROR: can't write $root");
            print ROOT $line;
            close(ROOT);
            # repository will usually not change
            if ( $old_rep ne $new_rep ) {
                my $rep = "$_/CVS/Repository";
                open(REPOSITORY, "<$rep") or carp("ERROR: can't open $rep");
                my $line = <REPOSITORY>;
                close(REPOSITORY);

                # patch rep
                $line =~ s/$old_rep/$new_rep/o;  # note: evaluate reg exp. only once
                open(REPOSITORY, ">$rep") or carp("ERROR: can't write $rep");
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
        carp("ERROR: VCSID not set");
        return "invalidvcsid";
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
        if ( !is_valid_login($root) ) {
            print STDERR "\nThe cvs rdiff command is broken for the OOo CVS server.\n";
            print STDERR "To fix this problem you have to issue the following cvs login command:\n\n";
            print STDERR "    cvs -d $root login\n\n";
            print STDERR "The password is your ususal OOo password.\n\n";
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
    my $root = shift;

    my $home = $ENV{HOME};
    open(CVSPASSWD, "<$home/.cvspass") or return 0;
    my @lines = <CVSPASSWD>;
    close(CVSPASSWD);

    my $is_valid = 0;
    foreach (@lines) {
        if ( $_ =~ /$root/o ) {
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
            carp("ERROR: connection to server failed");
            return 'connectionfailure';
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
                carp("ERROR: internal error in strip_module_from_path()");
                return $file;
            }
        }
    }
}

####

1; # needed by "use" or "require"
