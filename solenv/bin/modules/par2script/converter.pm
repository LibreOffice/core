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


package par2script::converter;

use par2script::remover;

#############################
# Converter
#############################

sub convert_array_to_hash
{
    my ($arrayref) = @_;

    my ($line, $key, $value);

    my %newhash = ();

    for ( my $i = 0; $i <= $#{$arrayref}; $i++ )
    {
        $line = ${$arrayref}[$i];

        if ( $line =~ /^\s*(\w+?)\s+(.*?)\s*$/ )
        {
            $key = $1;
            $value = $2;
            $newhash{$key} = $value;
        }
    }

    return \%newhash;
}

sub convert_hash_into_array
{
    my ($hashref) = @_;

    my @array = ();
    my ($key, $value, $input);

    foreach $key (keys %{$hashref})
    {
        $value = $hashref->{$key};
        $input = "$key = $value\n";
        push(@array ,$input);
    }

    return \@array
}

sub convert_stringlist_into_array_2
{
    my ( $input, $separator ) = @_;

    my @newarray = ();
    my $first = "";
    my $last = "";

    $last = $input;

    while ( $last =~ /^\s*(.+?)\s*\Q$separator\E\s*(.+)\s*$/)   # "$" for minimal matching
    {
        $first = $1;
        $last = $2;
        par2script::remover::remove_leading_and_ending_whitespaces(\$first);
        if ( $first ) { push(@newarray, $first); }
    }

    par2script::remover::remove_leading_and_ending_whitespaces(\$last);
    if ( $last ) { push(@newarray, $last); }

    return \@newarray;
}

sub convert_stringlist_into_array
{
    my ( $includestringref, $separator ) = @_;

    my @newarray = ();
    my ($first, $last);

    $last = ${$includestringref};

    while ( $last =~ /^\s*(.+?)\s*\Q$separator\E\s*(.+)\s*$/)   # "$" for minimal matching
    {
        $first = $1;
        $last = $2;
        par2script::remover::remove_leading_and_ending_whitespaces(\$first);
        push(@newarray, $first);
    }

    par2script::remover::remove_leading_and_ending_whitespaces(\$last);
    push(@newarray, $last);

    return \@newarray;
}

#############################################################################
# The file name contains for some files "/". If this programs runs on
# a windows platform, this has to be converted to "\".
#############################################################################

sub convert_slash_to_backslash
{
    my ($filesarrayref) = @_;

    my ($onefile, $filename);

    for ( my $i = 0; $i <= $#{$filesarrayref}; $i++ )
    {
        $onefile = ${$filesarrayref}[$i];
        $onefile->{'Name'} =~ s/\//\\/g;
    }
}

1;
