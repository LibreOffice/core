#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

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
