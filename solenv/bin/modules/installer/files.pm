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

package installer::files;

use installer::exiter;
use installer::logger;

############################################
# File Operations
############################################

sub check_file
{
    my ($arg) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::files::check_file : $arg"); }

    if(!( -f $arg ))
    {
        installer::exiter::exit_program("ERROR: Cannot find file $arg", "check_file");
    }
}

sub read_file
{
    my ($localfile) = @_;
    my @localfile = ();

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::files::read_file : $localfile"); }

    open( IN, "<$localfile" ) || installer::exiter::exit_program("ERROR: Cannot open file $localfile for reading", "read_file");

#   Don't use "my @localfile = <IN>" here, because
#   perl has a problem with the internal "large_and_huge_malloc" function
#   when calling perl using MacOS 10.5 with a perl built with MacOS 10.4
    while ( $line = <IN> ) {
        push @localfile, $line;
    }

    close( IN );

    return \@localfile;
}

###########################################
# Saving files, arrays and hashes
###########################################

sub save_file
{
    my ($savefile, $savecontent) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::files::save_file : $savefile : $#{$savecontent}"); }

    if ( open( OUT, ">$savefile" ) )
    {
        print OUT @{$savecontent};
        close( OUT);
    }
    else
    {
        # it is useless to save a log file, if there is no write access

        if ( $savefile =~ /\.log/ )
        {
            print "\n*************************************************\n";
            print "ERROR: Cannot write log file: $savefile";
            print "\n*************************************************\n";
            exit(-1);   # exiting the program to avoid endless loops
        }

        installer::exiter::exit_program("ERROR: Cannot open file $savefile for writing", "save_file");
    }
}

sub save_hash
{
    my ($savefile, $hashref) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::files::save_hash : $savefile"); }

    my @printcontent = ();

    my $itemkey;

    foreach $itemkey ( keys %{$hashref} )
    {
        my $line = "";
        my $itemvalue = $hashref->{$itemkey};
        $line = $itemkey . "=" . $itemvalue . "\n";
        push(@printcontent, $line);
    }

    open( OUT, ">$savefile" ) || installer::exiter::exit_program("ERROR: Cannot open file $savefile for writing", "save_hash");
    print OUT @printcontent;
    close( OUT);
}

sub save_array_of_hashes
{
    my ($savefile, $arrayref) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::files::save_array_of_hashes : $savefile : $#{$arrayref}"); }

    my @printcontent = ();

    for ( my $i = 0; $i <= $#{$arrayref}; $i++ )
    {
        my $line = "";
        my $hashref = ${$arrayref}[$i];
        my $itemkey;

        foreach $itemkey ( keys %{$hashref} )
        {
            my $itemvalue = $hashref->{$itemkey};
            $line = $line . $itemkey . "=" . $itemvalue . "\t";
        }

        $line = $line . "\n";

        push(@printcontent, $line);
    }

    open( OUT, ">$savefile" ) || installer::exiter::exit_program("ERROR: Cannot open file $savefile for writing", "save_array_of_hashes");
    print OUT @printcontent;
    close( OUT);
}

sub save_array_of_hashes_modules
{
    my ($savefile, $arrayref) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::files::save_array_of_hashes : $savefile : $#{$arrayref}"); }

    my @printcontent = ();

    for ( my $i = 0; $i <= $#{$arrayref}; $i++ )
    {
        my $line = "***************************************************\n";
        my $hashref = ${$arrayref}[$i];
        my $itemkey;

        foreach $itemkey ( keys %{$hashref} )
        {
            my $itemvalue = $hashref->{$itemkey};
            $line = $line . $itemkey . "=" . $itemvalue . "\n";
        }

        $line = $line . "\n";

        push(@printcontent, $line);
    }

    open( OUT, ">$savefile" ) || installer::exiter::exit_program("ERROR: Cannot open file $savefile for writing", "save_array_of_hashes");
    print OUT @printcontent;
    close( OUT);
}

###########################################
# Binary file operations
###########################################

sub read_binary_file
{
    my ($filename) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::files::read_binary_file : $filename"); }

    my $file;

    open( IN, "<$filename" ) || installer::exiter::exit_program("ERROR: Cannot open file $filename for reading", "read_binary_file");
    binmode IN;
    seek IN, 0, 2;
    my $length = tell IN;
    seek IN, 0, 0;
    read IN, $file, $length;
    close IN;

    return $file;
}

sub save_binary_file
{
    my ($file, $filename) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::files::save_binary_file : $filename"); }

    open( OUT, ">$filename" ) || installer::exiter::exit_program("ERROR: Cannot open file $filename for writing", "save_binary_file");
    binmode OUT;
    print OUT $file;
    close OUT;
}

1;
