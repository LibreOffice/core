#*************************************************************************
#
#   $RCSfile: Cvs.pm,v $
#
#   $Revision: 1.17 $
#
#   last change: $Author: vg $ $Date: 2004-07-27 13:01:40 $
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
# Cvs.pm - package for manipulating CVS archives
#

package Cvs;
use strict;

use CwsConfig;

##### ctor ####

sub new
{
    my $proto = shift;
    my $class = ref($proto) || $proto;
    my $self = {};
    $self->{NAME} = undef;
    $self->{HEAD} = undef;
    $self->{FLAGS} = undef;
    my $conf = CwsConfig::get_config();
    if ( $conf->cvs_binary() ) {
        $self->{CVS_BINARY} = $conf->cvs_binary();
    }
    else {
        if ($^O eq "MSWin32" ) {
            $self->{CVS_BINARY} = "cvsclt2.exe";
        }
        else {
            $self->{CVS_BINARY} = "cvs.clt2";
        }
    }
    $self->{ARCHIVE_PATH} = undef;
    $self->{REV_DATA} = {};
    $self->{REV_SORTED} = [];
    $self->{REV_TAGS} = {};
    $self->{TAGS} = {};
    $self->{"_PARSED"} = undef;
    $self->{"_SORTED"} = undef;
    $self->{"_TAGGED"} = undef;
    bless ($self, $class);
    return $self;
}

#### methods to access per object data ####

sub name
{
    my $self = shift;
    if ( @_ ) {
        $self->{NAME} = shift;
    }
    return $self->{NAME};
}

sub cvs_binary
{
    my $self = shift;
    if ( @_ ) {
        $self->{CVS_BINARY} = shift;
    }
    return $self->{CVS_BINARY};
}

sub get_data_by_rev
{
    my $self = shift;
    $self->parse_log();

    return $self->{REV_DATA};
}

sub get_sorted_revs
{
    my $self = shift;

    if ( $self->{"_SORTED"} ) {
        return $self->{REV_SORTED};
    }

    $self->parse_log();

    sub by_rev {
        # comparison function for sorting
        my (@field_a, @field_b, $min_field, $i);

        @field_a = split /\./, $a;
        @field_b = split /\./, $b;
        $min_field = ($#field_a > $#field_b) ? $#field_b : $#field_a;

        for ($i = 0; $i <= $min_field; $i++) {
            if ( ($field_a[$i] < $field_b[$i]) ) {
                return -1;
            }
            if ( ($field_a[$i] > $field_b[$i]) ) {
                return 1;
            }
        }

        if ( $#field_a == $#field_b ) {
            return 0;
        }
        # eg. 1.70 sorts before 1.70.1.0
        ($#field_a < $#field_b) ? return -1 : return 1;
    }

    @{$self->{REV_SORTED}} = sort by_rev (keys %{$self->{REV_DATA}});
    $self->{"_SORTED"} = 1;
    return $self->{REV_SORTED};
}

sub get_tags_by_rev
{
    my $self = shift;
    my ($tag, $rev);

    if ( $self->{"_TAGGED"} ) {
        return $self->{REV_TAGS};
    }

    $self->parse_log();
    foreach $tag (keys %{$self->{TAGS}}) {
        $rev = $self->{TAGS}->{$tag};
        push (@{$self->{REV_TAGS}->{$rev}}, $tag);
    }

    $self->{"_TAGGED"} = 1;
    return $self->{REV_TAGS};
}

sub get_flags
{
    my $self = shift;
    $self->parse_log();

    return $self->{FLAGS};
}

sub get_tags
{
    my $self = shift;

    $self->parse_log();
    return $self->{TAGS};
}

sub get_head
{
    my $self = shift;

    $self->parse_log();
    return $self->{HEAD};
}

sub get_archive_path
{
    my $self = shift;

    if( !$self->{ARCHIVE_PATH} ) {
        # ignore return values
        $self->status();
    }
    return $self->{ARCHIVE_PATH};
}

sub is_tag
{
    my $self = shift;
    my $tag  = shift;

    my $tags_ref = $self->get_tags();
    return (defined $$tags_ref{$tag}) ? 1 : 0;
}

# Check if $label is branch label and returns revision.
sub get_branch_rev
{
    my $self  = shift;
    my $label = shift;

    return 0 if $label eq '';
    my $tags_ref = $self->get_tags();
    my $rev = $$tags_ref{$label};
    return 0 if !defined($rev);
    my @field = split('\.', $rev);
    # $label is a branch label if rev is of form (...)x.y.0.z
    return 0 if $field[-2] != 0;
    $field[-2] = $field[-1];
    # remove last
    pop @field;
    return join('.', @field);
}

#### methods to manipulate archive ####

# Delete a revision. Use with care.
sub delete_rev
{
    my $self = shift;
    my $rev = shift;
    my $file = $self->name();

    my $response_ref = $self->execute("admin -o$rev $file");
    foreach ( @{$response_ref} ) {
        /deleting revision $rev/ && return 1;
    }
    return 0;
}

# Update archive with options $options .Returns 'success' and new revision
# on success or reason of failure. If no update happens because file was
# up-to-date consider operation a success.
sub update
{
    my $self = shift;
    my $options = shift;

    my $file = $self->name();
    my $response_ref = $self->execute("update $options $file");
    my $conflict          = 0;
    my $notknown          = 0;
    my $connectionfailure = 0;
    foreach ( @{$response_ref} ) {
        /conflicts during merge/ && ++$conflict;
        /nothing known about/ && ++$notknown;
        /\[update aborted\]: connect to/ && ++$connectionfailure;
    }
    if ( $conflict || $notknown || $connectionfailure) {
        my $failure = 'unkownfailure';
        $failure = 'conflict' if $conflict;
        $failure = 'notknown' if $notknown;
        $failure = 'connectionfailure' if $connectionfailure;
        return $failure;
    }
    return 'success';
}

# Commit $file with option $option; return 'success' or reason for failure.
# If 'success' return the new revision as second element.
sub commit
{
    my $self = shift;
    my $options = shift;

    my $file = $self->name();
    my $response_ref = $self->execute("commit $options $file");

    # already commited ?
    return 'nothingcommitted' if !@{$response_ref};

    my $conflict           = 0;
    my $uptodate           = 0;
    my $notknown           = 0;
    my $success            = 0;
    my $connectionfailure = 0;
    my $new_revision = undef;
    foreach ( @{$response_ref} ) {
        /Up-to-date check failed/ && ++$uptodate;
        /nothing known about/ && ++$notknown;
        /had a conflict and has not been modified/ && ++$conflict;
        /new revision: (delete);/ && (++$success, $new_revision = $1);
        /new revision: ([\d\.]+);/ && (++$success, $new_revision = $1);
        /\[commit aborted\]: connect to/ && ++$connectionfailure;
    }
    if ( !$success ) {
        my $failure = 'unkownfailure';
        $failure = 'conflict'          if $conflict;
        $failure = 'notuptodate'       if $uptodate;
        $failure = 'notknown'          if $notknown;
        $failure = 'connectionfailure' if $connectionfailure;
        return $failure;
    }
    return wantarray ? ('success', $new_revision) : 'success';
}

# Tag file with specified tag. Options my be specified,
# '-b' for a branch tag and -F for forced tag are valid options.
# Retagging without moving the tag is considered a succesful
# operation.
sub tag
{
    my $self    = shift;
    my $tag     = shift;
    my $options = shift;

    return 'invalidtag' if !$tag;
    # check for valid options
    if ( $options ) {
        my @elem = split(' ', $options);
        foreach (@elem) {
            unless ( /^-F/ || /-b/ ) {
                return 'invalidoption';
            }
            $options = join(' ', @elem);
        }
    }
    else {
        $options = '';
    }

    my $file = $self->name();
    my $response_ref = $self->execute("tag $options $tag $file");

    unless ( $options =~ /-F/ && $options =~ /-b/ ) {
        # No message from CVS means that tag already exists
        # and has not been moved.
        # If both -F and -b is given, CVS will always return
        # message.
        return 'success' if !@{$response_ref};
    }

    my $tagged             = 0;
    my $cant_move          = 0;
    my $connectionfailure  = 0;
    my $invalidfile        = 0;
    foreach ( @{$response_ref} ) {
        /^T \Q$file\E/ && ++$tagged;
        /NOT MOVING tag/ && ++$cant_move;
        /nothing known about/ && ++$invalidfile;
        /\[tag aborted\]: connect to/ && ++$connectionfailure;
    }
    return 'success'           if $tagged;
    return 'cantmove'          if $cant_move;
    return 'connectionfailure' if $connectionfailure;
    return 'invalidfile'       if $invalidfile;
    # should never happen
    return 'unkownfailure';
}

#### misc operations ####

# Return status information. Note that this is somewhat redundant with
# the information which can be retrieved from the log, but in some cases
# we can avoid the more expansive parsing of the log by calling this method.
# We don't save the status information between calls.
sub status
{
    my $self       = shift;

    my $file = $self->name();
    my ($nofile, $unkownfailure, $connectionfailure);
    my ($status, $working_rev, $repository_rev);
    my ($sticky_tag, $branch, $sticky_date, $sticky_options);

    my $response_ref = $self->execute("status $file");
    foreach ( @{$response_ref} ) {
        chomp();
        /File: no file/ && ++$nofile;
        /Status:\s+([\w\-\s]+)$/ && ($status = $1);
        /Working revision:\s+((\d|\.)+)/ && ($working_rev = $1);
        /Repository revision:\s+((\d|\.)+)\s+(\S+)/ && ($repository_rev = $1) && ($self->{ARCHIVE_PATH} = $3);
        /Sticky Tag:\s+(.+)/ && ($sticky_tag = $1);
        /Sticky Date:\s+(.+)/ && ($sticky_date = $1);
        /Sticky Options:\s+(.+)/ && ($sticky_options = $1);
        /\[status aborted\]: connect to/ && ++$connectionfailure;
    }

    return 'connectionfailure' if $connectionfailure;
    # all variables except $status will contain garbage if 'Locally Added'
    # or 'Unknown'
    return $status if ($status eq 'Locally Added' || $status eq 'Unknown');
    # same if $nofile is set
    return $status if $nofile;

    if ( $sticky_tag =~ /([\w\-]+) \(branch: ([\d\.]+)\)$/ ) {
        $sticky_tag = $1;
        $branch = $2;
    }

    $sticky_date = '' if $sticky_date eq '(none)';
    $sticky_options = '' if $sticky_options eq '(none)';

    if ( $sticky_options =~ /\-(\w+)/ ) {
        $sticky_options = $1;
    }

    $unkownfailure++ if !$status;

    return 'unkownerror' if $unkownfailure;
    return ($status, $working_rev, $repository_rev, $sticky_tag, $branch,
            $sticky_date, $sticky_options);
}

# Return a diff between two revision of an archive.
sub diff
{
    my $self       = shift;
    my $rev1       = shift;
    my $rev2       = shift;
    my $options    = shift || '';

    my $file = $self->name();
    my ($nofile, $unkowntagfailure, $unkownrevfailure, $connectionfailure);

    my $response_ref = $self->execute("diff $options -r$rev1 -r$rev2 $file");

    foreach ( @{$response_ref} ){
        /\[diff aborted\]: connect to/ && ++$connectionfailure;
        /cvs \[server aborted\]: no such tag \w+/ && ++$unkowntagfailure;
        /cvs server: tag [\d\.]+ is not in file $file/ && ++$unkownrevfailure;
    }

    return 'connectionfailure' if $connectionfailure;
    return 'unkowntagfailiure' if $unkowntagfailure;
    return 'unkownrevfailiure' if $unkownrevfailure;
    return wantarray ? @{$response_ref} : $response_ref;
}
#### private methods ####

sub execute
{
    my $self    = shift;
    my $command = shift;
    my $authtimeout = 0;
    my @response;
    while () {
        if ( $authtimeout >= 5 ) {
            # fail after 5 tries
            die("FATAL: OOo CVS server authorization time out, can't continue!\nPlease notify Release Engineering.")
        }
        if ( $authtimeout > 0 ) {
            # sleep 5 seconds after a authorization timeout
            carp("WARNING: OOo CVS server authorization time out, count: $authtimeout, sleeping for 5 seconds ...");
            sleep(5);
        }
        open(CVS, "$self->{CVS_BINARY} $command 2>&1 |");
        @response = <CVS>;
        close(CVS);

        foreach ( @response ) {
            /unrecognized auth response/ && ++$authtimeout;
        }
        last if !$authtimeout;
    }
    return wantarray ? @response : \@response;
}

sub parse_log
{
    my $self = shift;
    if ( $self->{"_PARSED"} ) {
        return;
    }
    my $file = $self->name();
    my $in_revisions = 0;
    my $in_tags = 0;
    my $rev_data = {};
    my ($rev, $date, $author, $state, $comment, @branches);

    my $response_ref = $self->execute("log $file");

    foreach ( @{$response_ref} ) {
        chomp;

        if ( $in_revisions ) {
            /revision\s((\d|\.)+)$/o && do { $rev = $1; next; };
            /^date:\s(\S+\s\S+);\s+author:\s(\S+);\s+state:\s(\S+);/
                && do { $date = $1; $author = $2; $state = $3; next; };
            /^branches:((\s+(\d|\.)+;)+)$/o && do {
                my $line;
                $line = $1;
                $line =~ s/\s//go;
                @branches = split(/;/, $line);
                next;
            };

            (/^----------------------------$/o || /^=============================================================================$/o) && do
            {
                $rev_data = {DATE => $date,
                             AUTHOR => $author,
                             STATE => $state,
                             COMMENT => $comment,
                             BRANCHES => [ @branches ]};
                $self->{REV_DATA}->{$rev} = $rev_data;
                $comment = undef;
                @branches = ();
                next;
            };

            $comment .= $_ . "\n" ;
        }
        elsif ( $in_tags ) {
            /^keyword\ssubstitution:\s/o && do { $self->{FLAGS} = $'; $in_tags--; next; };
            # tags may contain a hyphen
            /^\t([\w|\-]+):\s((\d|\.)+)$/o && do { $self->{TAGS}->{$1} = $2; next; };
        }
        else {
            /^----------------------------$/o && do { $in_revisions++; next; };
            /^symbolic\snames:$/o && do { $in_tags++; next; };
            /^head:\s((\d|\.)+)$/o && do { $self->{HEAD} = $1; next; };
            /^RCS file:\s((\d|\.)+)$/o && do { $self->{ARCHIVE_PATH} = $1; next; };
        }
    }

    $self->{"_PARSED"} = 1;
}

####

1; # needed by "use" or "require"
