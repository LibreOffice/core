#*************************************************************************
#
#   $RCSfile: CreatePDBRelocators.pm,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2003-08-07 11:43:52 $
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

#*************************************************************************
#
# createPDBRelocators - create for pdb relocator files
#                       PDB relocator files are used to find debug infos
#                       for analysis of creash reports
#
# usage: create_pdb_relocators($inpath, $milestone, $pre);
#
#*************************************************************************

package CreatePDBRelocators;

use strict;
use File::Basename;

sub create_pdb_relocators
{
    my $inpath   = shift;
    my $milestone    = shift;
    my $pre      = shift;

    my $solarversion = $ENV{SOLARVERSION};
    if ( !$solarversion ) {
        print STDERR "can't determine SOLARVERSION.\n";
        return undef;
    }

    my $o = $ENV{SRC_ROOT};
    if ( !$o ) {
        print STDERR "can't determine SOLAR_SRC.\n";
        return undef;
    }

    my $root_dir = "$solarversion/$inpath";

    # sanitize path
    $root_dir =~ s/\\/\//g;
    $o =~ s/\\/\//g;
    my $pdb_dir = $root_dir . "/pdb.$pre$milestone";
    my $pdb_so_dir = $root_dir . "/pdb.$pre$milestone/so";

    # create pdb directories if necessary
    if ( ! -d $pdb_dir ) {
        if ( !mkdir($pdb_dir, 0775) ) {
            print STDERR "can't create directory '$pdb_dir'\n";
                return undef;
        }
    }
    if ( ! -d $pdb_so_dir ) {
        if ( !mkdir($pdb_so_dir, 0775) ) {
            print STDERR "can't create directory '$pdb_so_dir'\n";
                return undef;
        }
    }

    # collect files
    my $template = "$o/*/$inpath";
    my @pdb_files;
    if ( $^O eq 'MSWin32' ) {
        # collect all pdb files on o:
        # regular glob does not work with two wildcard on WNT
        my @bin    = glob("$template/bin/*.pdb");
        my @bin_so = glob("$template/bin/so/*.pdb");
        # we are only interested in pdb files which are accompanied by
        # .exe or .dll which the same name
        foreach (@bin, @bin_so) {
            my $dir  = dirname($_);
            my $base = basename($_, ".pdb");
            my $exe = "$dir/$base.exe";
            my $dll = "$dir/$base.dll";
            if ( -e $exe || -e $dll ) {
                push(@pdb_files, $_);
            }
        }
    }
    else {
        # collect all shared libraries on o:
        my @lib = glob("$template/lib/*.so*");
        my @lib_so = glob("$template/lib/so/*.so*");
        # collect all binary executables on o:
        my @bin = find_binary_execs("$template/bin");
        my @bin_so = find_binary_execs("$template/bin/so");
        @pdb_files = (@lib, @lib_so, @bin, @bin_so);
    }

    foreach (@pdb_files) {
        my $relocator = basename($_) . ".location";
        /$o\/(.*)/i;

        my $src_location = $1;

        my $location = "";
        my $target = "";
        if ( $src_location =~ /\/so\// )
        {
            $location = "../../../src.$milestone/" . $src_location;
            $target = "$pdb_dir/so/$relocator";
        }
        else
        {
            $location = "../../src.$milestone/" . $src_location;
            $target = "$pdb_dir/$relocator";
        }

        if ( !open(RELOCATOR, ">$target") ) {
            print STDERR "can't write file '$target'\n";
            return undef;
        }
        print RELOCATOR "$location\n";
        close(RELOCATOR);
    }
    return 1;
}

sub find_binary_execs
{
    my $path = shift;
    my @files = glob("$path/*");
    my @execs = grep(-x $_, @files);
    my @elf_files = grep(`file $_` =~ /ELF/, @execs);
    return @elf_files;
}

1; # required

