#*************************************************************************
#
#   $RCSfile: Cvs.pm,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: hr $ $Date: 2002-08-22 18:03:40 $
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

##### ctor ####

sub new
{
    my $proto = shift;
    my $class = ref($proto) || $proto;
    my $self = {};
    $self->{NAME} = undef;
    $self->{HEAD} = undef;
    $self->{FLAGS} = undef;
    if ( $ENV{CVS_BINARY} ) {
        $self->{CVS_BINARY} = $ENV{CVS_BINARY};
    }
    else {
        if ($^O eq "MSWin32" || $^O eq "os2" ) {
            $self->{CVS_BINARY} = "cvsclt2.exe";
        }
        else {
            $self->{CVS_BINARY} = "cvs.clt2";
        }
    }
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
        $self->{name} = shift;
    }
    return $self->{name};
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
        return $self->{REV_TAGS};
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

sub is_tag
{
    my $self = shift;
    my $tag  = shift;

    my $tags_ref = $self->get_tags();
    return exists($$tags_ref{$tag}) ? 1 : 0;
}

sub get_branch_rev
{
    # check if $label is branch label and returns revision
    my $self  = shift;
    my $label = shift;

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

sub delete_rev
{
    my $self = shift;
    my $rev = shift;
    my $file = $self->name;

    if ( $^O eq "MSWin32" || $^O eq 'os2' ) {
        open (CVSDELETE, "$self->{CVS_BINARY} admin -o$rev $file 2>nul |");
    }
    else {
        open (CVSDELETE, "$self->{CVS_BINARY} admin -o$rev $file 2>/dev/null |");
    }
    while(<CVSDELETE>) {
        /deleting revision $rev/ && return 1;
    }
    close(CVSDELETE);
    return 0;
}

sub update
{
    # Update archive with options $options.
    # Returns 'success' on success or reason of failure.
    # If no update happens because file was up-to-date
    # consider operation a success.
    my $self = shift;
    my $options = shift;

    my $file = $self->name;
    if ( $^O eq "MSWin32" || $^O eq 'os2' ) {
        open (CVSUPDATE, "$self->{CVS_BINARY} update $options $file 2>&1 |");
    }
    else {
        open (CVSUPDATE, "$self->{CVS_BINARY} update $options $file 2>&1 |");
    }
    my $conflict = 0;
    my $notknown = 0;
    while(<CVSUPDATE>) {
        /conflicts during merge/ && ++$conflict;
        /nothing known about/ && ++$notknown;
    }
    close(CVSUPDATE);
    if ( $conflict || $notknown ) {
        my $failure = 'unkownfailure';
        $failure = 'conflict' if $conflict;
        $failure = 'notknown' if $notknown;
        return $failure
    }
    return 'success'
}

sub commit
{
    # commit $file with option $option
    # return 'success' or reason for failure
    my $self = shift;
    my $options = shift;

    my $file = $self->name;
    if ( $^O eq "MSWin32" || $^O eq 'os2' ) {
        open (CVSCOMMIT, "$self->{CVS_BINARY} commit $options $file 2>&1 |");
    }
    else {
        open (CVSCOMMIT, "$self->{CVS_BINARY} commit $options $file 2>&1 |");
    }
    my $conflict = 0;
    my $uptodate = 0;
    my $notknown = 0;
    my $success  = 0;
    while(<CVSCOMMIT>) {
        /Up-to-date check failed/ && ++$uptodate;
        /nothing known about/ && ++$notknown;
        /had a conflict and has not been modified/ && ++$conflict;
        /new revision:/ && ++$success;
    }
    close(CVSCOMMIT);
    if ( !$success ) {
        my $failure = 'unkownfailure';
        $failure = 'conflict' if $conflict;
        $failure = 'notuptodate' if $uptodate;
        $failure = 'notknown' if $notknown;
        return $failure
    }
    return 'success'
}


#### private methods ####
sub parse_log
{
    my $self = shift;
    if ( $self->{"_PARSED"} ) {
        return;
    }
    my $file = $self->name;
    my $in_revisions = 0;
    my $in_tags = 0;
    my $rev_data = {};
    my ($rev, $date, $author, $state, $comment, @branches);

    open(CVSLOG, "$self->{CVS_BINARY} log $file |");

    while( <CVSLOG> ) {
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

            (/^----------------------------$/o || /^=============================================================================$/o) && do {
                $rev_data = {
                    DATE => $date,
                    AUTHOR => $author,
                    STATE => $state,
                    COMMENT => $comment,
                    BRANCHES => [ @branches ]
                };
                $self->{REV_DATA}->{$rev} = $rev_data;
                $comment = undef;
                @branches = ();
                next;
            };

            $comment .= $_ . "\n" ;
        } elsif ( $in_tags ) {
            /^keyword\ssubstitution:/o && do { $self->{FLAGS} = $'; $in_tags--; next; };
            /^\t(\w+):\s((\d|\.)+)$/o && do { $self->{TAGS}->{$1} = $2; next; };
        }
        else {
            /^----------------------------$/o && do { $in_revisions++; next; };
            /^symbolic\snames:$/o && do { $in_tags++; next; };
            /^head:\s((\d|\.)+)$/o && do { $self->{HEAD} = $1; next; };
        }
    }

    close(CVSLOG);

    $self->{"_PARSED"} = 1;
}

####

1; # needed by "use" or "require"
