#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: files.pm,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: obo $ $Date: 2008-01-04 16:56:42 $
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
