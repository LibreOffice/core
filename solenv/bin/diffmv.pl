:
    eval 'exec perl -S $0 ${1+"$@"}'
        if 0;
#*************************************************************************
#
#   $RCSfile: diffmv.pl,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: obo $ $Date: 2004-11-19 11:41:20 $
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

