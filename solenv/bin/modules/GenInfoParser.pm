#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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

#*************************************************************************
#
# GenInfoParser - Perl extension for parsing general info databases
#
# usage: see below
#
#*************************************************************************

package GenInfoParser;

use strict;

use Carp;

#####  profiling #####
# use Benchmark;

##### ctor #####

sub new {
    my $proto = shift;
    my $class = ref($proto) || $proto;
    my $self = {};
    $self->{'LIST'} = undef;
    $self->{'DATA'} = {};
    bless ($self, $class);
    return $self;
}

##### methods #####

sub load_list
{
    # load list into memory
    my $self        = shift;
    my $list_file   = shift;

    if ( $self->parse_list($list_file) ) {
        return 1;
    }
    return 0;
}

sub get_keys
{
    # return a sorted list of keys, the sorting is case insensitive
    my $self        = shift;
    my $access_path = shift;

    my ($key, $value, $sub_data_ref) = $self->walk_accesspath($access_path);

    my @keys = ();
    if ( $sub_data_ref ) {
        my @normalized_keys = keys %$sub_data_ref;
        foreach my $normalized_key (sort keys %$sub_data_ref) {
            push(@keys, $$sub_data_ref{$normalized_key}[0]);
        }
    } elsif ( $value ) {
        chomp $value;
        push @keys, ($value);
    }
    return @keys;
}

sub get_key
{
    # returns the key corresponding to the access_path
    my $self        = shift;
    my $access_path = shift;

    my ($key, $value, $sub_data_ref) = $self->walk_accesspath($access_path);
    return undef if !$key;
    return $key;
}

sub get_value
{
    # returns the value corresponding to the access_path
    my $self        = shift;
    my $access_path = shift;

    my ($key, $value, $sub_data_ref) = $self->walk_accesspath($access_path);
    return undef if !$key;
    $value = "" if !defined($value);
    # trim line ends
    $value =~ tr/\r\n//d;
    # trim trailing whitespace
    $value =~ s/\s+$//;
    return $value;
}

##### private methods #####

sub parse_list
{
    # parse complete list
    my $self        = shift;
    my $list_file   = shift;
    my @list_data;

    return 0 if ! -r $list_file;

    open(FILE, "<$list_file") or croak("can't open $list_file: $!");
    $self->parse_block(\*FILE, $self->{'DATA'});
    close(FILE);
}

sub parse_block
{
    # parse each sub block and place it in a hash
    # used data structure:
    # $hash{$normalized_key} = [ $key, $value, 0 | $sub_hash_ref ]
    my $self        = shift;
    my $glob_ref    = shift;
    my $data_ref    = shift;

    my $current_key = 0;
    my $line;
    while( $line = <$glob_ref> ) {
        # this is the inner loop, any additional pattern matching will
        # have a notable affect on runtime behavior
        # clean up of $value is done in get_value()
        my ($key, $value) = split(' ', $line, 2);
        next if !$key;                  # skip empty lines
        my $chr = substr($key, 0, 1);
        next if $chr eq '#';            # skip comment lines
        last if $chr eq '}';            # return from block;
        if ( $chr eq '{' ) {
            if ( !$current_key ) {
                croak("unexpected block start");
            }
            else {
                # create empty hash and start sub block parse
                $$data_ref{$current_key}[2] = {};
                $self->parse_block($glob_ref, $$data_ref{$current_key}[2]);
                next;
            }
        }
        # sanity check
        croak("key $key is not well formed") if $key =~ /\//;
        # normalize key for hash lookup
        $current_key = lc($key);
        # but we have to keep the original - not normalized - key, too
        $$data_ref{($current_key)} = [$key, $value, 0];
    }
}

sub walk_accesspath
{
    # returns the key, value and sub_data_ref which
    # corresponds to the access_path

    my $self        = shift;
    my $access_path = shift;

    my $sub_data_ref = $self->{'DATA'};

    if ( $access_path ) {
        my $lookup_ref = 0;
        # normalize key
        $access_path = lc($access_path);
        my @key_sequence = split(/\//, $access_path);
        foreach my $key_element (@key_sequence) {
            # at least one more key element, but no sub_hash, accesspath invalid
            return () if !$sub_data_ref;
            $lookup_ref = $$sub_data_ref{$key_element};
            # lookup failed, accesspath invalid
            return () if !defined($lookup_ref);
            # we've got a valid key
            $sub_data_ref = $$lookup_ref[2];
        }
        return ($$lookup_ref[0], $$lookup_ref[1], $sub_data_ref);
    }
    else {
        # empty access path is only vlaid for getting top level key list
        return ( undef, undef, $sub_data_ref );
    }
}

##### finish #####

1; # needed by use or require

__END__

=head1 NAME

GenInfoParser - Perl extension for parsing general info databases

=head1 SYNOPSIS

    # example that will load a general info database called 'stand.lst'

    use GenInfoParser;

    # Create a new instance of the parser:
    $a = GenInfoParser->new();

    # Load the database into the parser:
    $a->load_list('ssrc633.ini');

    # get top level keys from database
    @top_level_keys = $a->get_keys();

    # get sub list keys
    @sub_list_keys = $a->get_keys('src633/Drives/o:/Projects');

    # get key/value pair
    $key = $a->get_key('src633/Comment/build');
    $value = $a->get_value('src633/Comment/build');

=head1 DESCRIPTION

GenInfoParser is a perl extension to load and parse General Info Databses.
It uses a simple object oriented interface to retrieve the information stored
in the database.

Methods:

GenInfoParser::new()

Creates a new instance of the parser. Can't fail.


GenInfoParser::load_list($database)

Loads and parses $database. Returns 1 on success and 0 on failure


GenInfoParser::get_keys($path)

Returns a sorted list of keys from the path $path. Returns an emtpy list if $path
has no sublist. If there is no $path spcified, the method will return the
primary key list. $path can be specified case insensitive. Sorting is done case
insensitive.

GenInfoParser::get_key($path)

Returns the key to $path or 'undef' if an invalid path is given.
Example: $path = 'src633/comment/build' will return 'Build' as key.
Note: $path can be specified case insensitive, but the returned key will
have the exact case as in the database.

GenInfoParser::get_value($path)

Returns the value to $path or 'undef' is invalid path is given.


=head2 EXPORT

GenInfoParser::new()
GenInfoParser::load_list($database)
GenInfoParser::get_keys($path)
GenInfoParser::get_key($path)
GenInfoParser::get_value($path)


=head1 AUTHOR

Jens-Heiner Rechtien, rechtien@sun.com

=head1 SEE ALSO

perl(1).

=cut
