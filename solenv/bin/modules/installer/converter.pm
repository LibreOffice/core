#*************************************************************************
#
#   $RCSfile: converter.pm,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: kz $ $Date: 2004-06-11 18:15:02 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc..
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

package installer::converter;

use installer::globals;

#############################
# Converter
#############################

sub convert_array_to_hash
{
    my ($arrayref) = @_;

    my %newhash = ();

    for ( my $i = 0; $i <= $#{$arrayref}; $i++ )
    {
        my $line = ${$arrayref}[$i];

        if ( $line =~ /^\s*(\w+?)\s+(.*?)\s*$/ )
        {
            my $key = $1;
            my $value = $2;
            $newhash{$key} = $value;
        }
    }

    return \%newhash;
}

sub convert_hash_into_array
{
    my ($hashref) = @_;

    my @array = ();
    my $key;

    foreach $key (keys %{$hashref})
    {
        my $value = $hashref->{$key};
        my $input = "$key = $value\n";
        push(@array ,$input);
    }

    return \@array
}

#############################################################################
# Converting a string list with separator $listseparator
# into an array
#############################################################################

sub convert_stringlist_into_array
{
    my ( $includestringref, $listseparator ) = @_;

    my @newarray = ();
    my $first;
    my $last = ${$includestringref};

    while ( $last =~ /^\s*(.+?)\Q$listseparator\E(.+)\s*$/) # "$" for minimal matching
    {
        $first = $1;
        $last = $2;
        if ( $installer::globals::iswin ) { $first =~ s/\//\\/g; }
        push(@newarray, "$first\n");
    }

    if ( $installer::globals::iswin ) { $last =~ s/\//\\/g; }
    push(@newarray, "$last\n");

    return \@newarray;
}

#############################################################################
# The file name contains for some files "/". If this programs runs on
# a windows platform, this has to be converted to "\".
#############################################################################

sub convert_slash_to_backslash
{
    my ($filesarrayref) = @_;

    for ( my $i = 0; $i <= $#{$filesarrayref}; $i++ )
    {
        my $onefile = ${$filesarrayref}[$i];
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

    foreach $key (keys %{$olditemhashref})
    {
        my $value = $olditemhashref->{$key};
        $newitemhashref->{$key} = $value;
    }
}

#################################################################
# Windows pathes must not contain the following structure:
# c:\dirA\dirB\..\dirC
# This has to be exchanged to
# c:\dirA\dirC
#################################################################

sub make_path_conform
{
    my ( $path ) = @_;

    my $oldpath = $path;

    while ( $path =~ /(^.*)(\Q$installer::globals::separator\E.*?)(\Q$installer::globals::separator\E\.\.)(\Q$installer::globals::separator\E.*$)/ )
    {
        my $part1 = $1;
        my $part2 = $4;
        $path = $part1 . $part2;
    }

    return $path;
}

#################################################################
# Copying an item collector
# A reference to an array consisting of references to hashes.
#################################################################

sub copy_collector
{
    my ( $oldcollector ) = @_;

    my @newcollector = ();

    for ( my $i = 0; $i <= $#{$oldcollector}; $i++ )
    {
        my %newhash = ();
        my $key;

        foreach $key (keys %{${$oldcollector}[$i]})
        {
            $newhash{$key} = ${$oldcollector}[$i]->{$key};
        }

        push(@newcollector, \%newhash);
    }

    return \@newcollector;
}

#################################################################
# Returning the current ending number of a directory
#################################################################

sub get_number_from_directory
{
    my ( $directory ) = @_;

    my $number = 0;

    if ( $directory =~ /\_(\d+)\s*$/ )
    {
        $number = $1;
    }

    return $number;
}

1;
