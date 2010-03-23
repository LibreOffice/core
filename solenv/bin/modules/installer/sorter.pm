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

package installer::sorter;

#########################################
# Sorting an array of hashes
#########################################

sub sorting_array_of_hashes
{
    my ($arrayref, $sortkey) = @_;

    for ( my $i = 0; $i <= $#{$arrayref}; $i++ )
    {
        my $onehashunder = ${$arrayref}[$i];
        my $sortvalueunder = $onehashunder->{$sortkey};

        for ( my $j = $i + 1; $j <= $#{$arrayref}; $j++ )
        {
            my $onehashover = ${$arrayref}[$j];
            my $sortvalueover = $onehashover->{$sortkey};

            if ( $sortvalueunder gt $sortvalueover)
            {
                ${$arrayref}[$i] = $onehashover;
                ${$arrayref}[$j] = $onehashunder;

                $onehashunder = $onehashover;
                $sortvalueunder = $sortvalueover;
            }
        }
    }
}

######################################################
# Sorting an array of hashes with a numerical value
######################################################

sub sort_array_of_hashes_numerically
{
    my ($arrayref, $sortkey) = @_;

    for ( my $i = 0; $i <= $#{$arrayref}; $i++ )
    {
        my $onehashunder = ${$arrayref}[$i];
        my $sortvalueunder = $onehashunder->{$sortkey};

        for ( my $j = $i + 1; $j <= $#{$arrayref}; $j++ )
        {
            my $onehashover = ${$arrayref}[$j];
            my $sortvalueover = $onehashover->{$sortkey};

            if ( $sortvalueunder > $sortvalueover)
            {
                ${$arrayref}[$i] = $onehashover;
                ${$arrayref}[$j] = $onehashunder;

                $onehashunder = $onehashover;
                $sortvalueunder = $sortvalueover;
            }
        }
    }
}

#########################################
# Sorting an array of of strings
#########################################

sub sorting_array_of_strings
{
    my ($arrayref) = @_;

    for ( my $i = 0; $i <= $#{$arrayref}; $i++ )
    {
        my $onestringunder = ${$arrayref}[$i];

        for ( my $j = $i + 1; $j <= $#{$arrayref}; $j++ )
        {
            my $onestringover = ${$arrayref}[$j];

            if ( $onestringunder gt $onestringover)
            {
                ${$arrayref}[$i] = $onestringover;
                ${$arrayref}[$j] = $onestringunder;
                $onestringunder = $onestringover;
            }
        }
    }
}

1;
