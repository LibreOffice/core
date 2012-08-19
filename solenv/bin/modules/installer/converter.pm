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

package installer::converter;

use strict;
use warnings;

use installer::globals;

#############################
# Converter
#############################

sub convert_array_to_hash
{
    my ($arrayref) = @_;

    my %newhash = ();

    for (@{$arrayref})
    {
        next unless /^\s*([\w-]+?)\s+(.*?)\s*$/;
        $newhash{$1} = $2;
    }

    return \%newhash;
}

#############################################################################
# Converting a string list with separator $listseparator
# into an array
#############################################################################

sub convert_stringlist_into_array
{
    my ( $includestringref, $listseparator ) = @_;

    return [map "$_\n", split /\Q$listseparator\E\s*/, ${$includestringref}];
}

#############################################################################
# Converting a string list with separator $listseparator
# into a hash with values 1.
#############################################################################

sub convert_stringlist_into_hash
{
    my ( $includestringref, $listseparator ) = @_;

    return {map {$_, 1} split /\Q$listseparator\E\s*/, ${$includestringref}};
}

#############################################################################
# Converting a string list with separator $listseparator
# into an array
#############################################################################

sub convert_whitespace_stringlist_into_array
{
    my ( $includestringref ) = @_;

    my $tmp = ${$includestringref};
    $tmp = s/^\s+//;
    $tmp = s/\s+$//;

    return [map "$_\n", split /\s+/, $tmp];
}

#############################################################################
# Converting an array into a space separated string
#############################################################################

sub convert_array_to_space_separated_string
{
    my ( $arrayref ) = @_;

    my $newstring;
    for (@{$arrayref}) {
        my $tmp = $_;
        $tmp =~ s/\s+$//;
        $newstring .= "$tmp ";
    }
    $newstring =~ s/ $//;

    return $newstring;
}

#############################################################################
# The file name contains for some files "/". If this programs runs on
# a windows platform, this has to be converted to "\".
#############################################################################

sub convert_slash_to_backslash
{
    my ($filesarrayref) = @_;

    for my $onefile (@{$filesarrayref})
    {
        if ( $onefile->{'Name'} ) { $onefile->{'Name'} =~ s/\//\\/g; }
    }
}

############################################################################
# Creating a copy of an existing file object
# No converter
############################################################################

sub copy_item_object
{
    my ($olditemhashref, $newitemhashref) = @_;

    $newitemhashref = {%{$olditemhashref}};
}

#################################################################
# Windows paths must not contain the following structure:
# c:\dirA\dirB\..\dirC
# This has to be exchanged to
# c:\dirA\dirC
#################################################################

sub make_path_conform
{
    my ( $path ) = @_;
    my $s = $installer::globals::separator;

    while ($path =~ s/[^\.\Q$s\E]+?\Q$s\E\.\.(?:\Q$s\E|$)//g) {}

    return $path;
}

#################################################################
# Copying an item collector
# A reference to an array consisting of references to hashes.
#################################################################

sub copy_collector
{
    return [map { {%{$_}} } @{$_[0]}];
}

#################################################################
# Combining two arrays, first wins
#################################################################

sub combine_arrays_from_references_first_win
{
    my ( $arrayref1, $arrayref2 ) = @_;

    my $hashref1 = convert_array_to_hash($arrayref1);
    my $hashref2 = convert_array_to_hash($arrayref2);

    # add key-value pairs from hash1 to hash2 (overwrites existing keys)
    @{$hashref2}{keys %{$hashref1}} = values %{$hashref1};

    return [map { "$_ $hashref2->{$_}\n" } keys %{$hashref2}];
}

#################################################################
# Replacing separators, that are included into quotes
#################################################################

sub replace_masked_separator
{
    my ($string, $separator, $replacementstring) = @_;

    $string =~ s/\\\Q$separator\E/$replacementstring/g;

    return $string;
}

#################################################################
# Resolving separators, that were replaced
# in function mask_separator_in_quotes
#################################################################

sub resolve_masked_separator
{
    my ($arrayref, $separator, $replacementstring) = @_;

    for (@{$arrayref})
    {
        s/$replacementstring/$separator/g;
    }
}

1;
