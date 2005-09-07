:
    eval 'exec perl -S $0 ${1+"$@"}'
        if 0;
#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: diffmv.pl,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 22:08:09 $
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


my ( $srcfile, $destfile ) = @ARGV;
my ( @srclines, @destlines );
my $dest_existing = 0;
@destlines = ();

usage() if ( ! defined $srcfile || ! defined $destfile);

open(SRCFILE, "$srcfile") or die "ERROR: Can't open $srcfile\n";
@srclines = <SRCFILE>;
close SRCFILE;

if ( -f $destfile ) {
    open(DESTFILE, "$destfile") or die "ERROR: Can't open $destfile\n";
    @destlines = <DESTFILE>;
    close DESTFILE;
    $dest_existing = 1;
}

if ( ! check_if_lists_equal(\@srclines,  \@destlines) ) {
    print STDERR "Updating \"$destfile\".\n";
    unlink "$destfile" or die "ERROR: Can't remove old $destfile\n" if ( $dest_existing );
    rename "$srcfile", "$destfile" or die "ERROR: Can't rename $srcfile to $destfile\n";
} else {
    print STDERR "\"$destfile\" unchanged.\n";
}

sub check_if_lists_equal
{
    my  ( $srclist_ref, $destlist_ref ) = @_;
    my @srclist  = @{ $srclist_ref };
    my @destlist  = @{ $destlist_ref };
    return  0 if ( $#srclist != $#destlist );
    for ( my $i = 0; $i  < $#srclist; $i++ ) {
        return 0 if  ( $srclist[$i] ne $destlist[$i]);
    }
    return  1;
}

sub usage
{
    print STDERR "Usage: diffmv sourcefile destfile\n";
    print STDERR "Do move diffing file only\n";
    exit 1;
}

