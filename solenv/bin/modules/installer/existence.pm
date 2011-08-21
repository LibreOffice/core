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
