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




package pre2par::existence;

#############################
# Test of existence
#############################

sub exists_in_array
{
    my ($searchstring, $arrayref) = @_;

    my $alreadyexists = 0;

    for ( my $i = 0; $i <= $#{$arrayref}; $i++ )
    {
        if ( ${$arrayref}[$i] eq $searchstring)
        {
            $alreadyexists = 1;
            last;
        }
    }

    return $alreadyexists;
}

sub exists_in_array_of_hashes
{
    my ($searchkey, $searchvalue, $arrayref) = @_;

    my $hashref;
    my $valueexists = 0;

    for ( my $i = 0; $i <= $#{$arrayref}; $i++ )
    {
        $hashref = ${$arrayref}[$i];

        if ( $hashref->{$searchkey} eq $searchvalue )
        {
            $valueexists = 1;
            last;
        }
    }

    return $valueexists;
}

1;
