#**************************************************************
#
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#
#**************************************************************



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
