#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: sorter.pm,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: kz $ $Date: 2007-09-06 09:53:39 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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
