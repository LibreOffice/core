#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: files.pm,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 09:30:35 $
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

    open( IN, "<$localfile" ) || pre2par::exiter::exit_program("ERROR: Cannot open file: $localfile", "read_file");
    my @localfile = <IN>;
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
