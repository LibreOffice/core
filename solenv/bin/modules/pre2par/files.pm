#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: files.pm,v $
#
# $Revision: 1.6 $
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

package pre2par::files;

use pre2par::exiter;

############################################
# File Operations
############################################

sub check_file
{
    my ($arg) = @_;

    if(!( -f $arg ))
    {
        pre2par::exiter::exit_program("ERROR: Cannot find file $arg", "check_file");
    }
}

sub read_file
{
    my ($localfile) = @_;

    my @localfile = ();

    open( IN, "<$localfile" ) || pre2par::exiter::exit_program("ERROR: Cannot open file: $localfile", "read_file");
    while ( <IN> ) { push(@localfile, $_); }
    close( IN );

    return \@localfile;
}

###########################################
# Saving files, arrays and hashes
###########################################

sub save_file
{
    my ($savefile, $savecontent) = @_;
    if (-f $savefile) { unlink $savefile };
    if (-f $savefile) { pre2par::exiter::exit_program("ERROR: Cannot delete existing file: $savefile", "save_file"); };
    open( OUT, ">$savefile" );
    print OUT @{$savecontent};
    close( OUT);
    if (! -f $savefile) { pre2par::exiter::exit_program("ERROR: Cannot write file: $savefile", "save_file"); }
}

sub save_hash
{
    my ($savefile, $hashref) = @_;

    my @printcontent = ();

    my ($itemkey, $itemvalue, $line);

    foreach $itemkey ( keys %{$hashref} )
    {
        $line = "";
        $itemvalue = $hashref->{$itemkey};
        $line = $itemkey . "=" . $itemvalue . "\n";
        push(@printcontent, $line);
    }

    open( OUT, ">$savefile" );
    print OUT @printcontent;
    close( OUT);
}

sub save_array_of_hashes
{
    my ($savefile, $arrayref) = @_;

    my @printcontent = ();

    my ($itemkey, $itemvalue, $line, $hashref);

    for ( my $i = 0; $i <= $#{$arrayref}; $i++ )
    {
        $line = "";
        $hashref = ${$arrayref}[$i];

        foreach $itemkey ( keys %{$hashref} )
        {
            $itemvalue = $hashref->{$itemkey};

            $line = $line . $itemkey . "=" . $itemvalue . "\t";
        }

        $line = $line . "\n";

        push(@printcontent, $line);
    }

    open( OUT, ">$savefile" );
    print OUT @printcontent;
    close( OUT);
}

1;
