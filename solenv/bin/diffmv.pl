:
    eval 'exec perl -S $0 ${1+"$@"}'
        if 0;
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

