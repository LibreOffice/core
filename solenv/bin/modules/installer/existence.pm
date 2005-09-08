#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: existence.pm,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 09:02:34 $
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
