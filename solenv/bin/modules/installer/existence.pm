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



package installer::existence;

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

    my $valueexists = 0;

    for ( my $i = 0; $i <= $#{$arrayref}; $i++ )
    {
        my $hashref = ${$arrayref}[$i];

        if ( $hashref->{$searchkey} eq $searchvalue )
        {
            $valueexists = 1;
            last;
        }
    }

    return $valueexists;
}

#####################################################################
# Returning a specified file as base for the new
# configuration file, defined by its "gid"
#####################################################################

sub get_specified_file
{
    my ($filesarrayref, $searchgid) = @_;

    my $foundfile = 0;
    my $onefile;

    for ( my $i = 0; $i <= $#{$filesarrayref}; $i++ )
    {
        $onefile = ${$filesarrayref}[$i];
        my $filegid = $onefile->{'gid'};

        if ( $filegid eq $searchgid )
        {
            $foundfile = 1;
            last;
        }
    }

    my $errorline = "ERROR: Could not find file $searchgid in list of files!";

    if ( $installer::globals::patch) { $errorline = "ERROR: Could not find file $searchgid in list of files! intro.bmp must be part of every patch. Please assign the flag PATCH in scp2 project."; }

    if (!($foundfile))
    {
        installer::exiter::exit_program($errorline, "get_specified_file");
    }

    return $onefile;
}

#####################################################################
# Returning a specified file as base for a new file,
# defined by its "Name"
#####################################################################

sub get_specified_file_by_name
{
    my ($filesarrayref, $searchname) = @_;

    my $foundfile = 0;
    my $onefile;

    for ( my $i = 0; $i <= $#{$filesarrayref}; $i++ )
    {
        $onefile = ${$filesarrayref}[$i];
        my $filename = $onefile->{'Name'};

        if ( $filename eq $searchname )
        {
            $foundfile = 1;
            last;
        }
    }

    if (!($foundfile))
    {
        installer::exiter::exit_program("ERROR: Could not find file $searchname in list of files!", "get_specified_file_by_name");
    }

    return $onefile;
}

#####################################################################
# Checking existence of a specific file, defined by its "Name"
#####################################################################

sub filename_exists_in_filesarray
{
    my ($filesarrayref, $searchname) = @_;

    my $foundfile = 0;

    for ( my $i = 0; $i <= $#{$filesarrayref}; $i++ )
    {
        my $onefile = ${$filesarrayref}[$i];
        my $filename = $onefile->{'Name'};

        if ( $filename eq $searchname )
        {
            $foundfile = 1;
            last;
        }
    }

    return $foundfile;
}

#####################################################################
# Checking existence of a specific file, defined by its "gid"
#####################################################################

sub filegid_exists_in_filesarray
{
    my ($filesarrayref, $searchgid) = @_;

    my $foundfile = 0;

    for ( my $i = 0; $i <= $#{$filesarrayref}; $i++ )
    {
        my $onefile = ${$filesarrayref}[$i];
        my $filegid = $onefile->{'gid'};

        if ( $filegid eq $searchgid )
        {
            $foundfile = 1;
            last;
        }
    }

    return $foundfile;
}

1;
