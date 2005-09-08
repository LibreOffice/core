#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: directory.pm,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 09:29:11 $
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

package pre2par::directory;

use pre2par::pathanalyzer;
use pre2par::systemactions;

############################################
# Checking, whether the output directories
# exist. If not, they are created.
############################################

sub check_directory
{
    my ($parfilename) = @_;

    my $productdirectory = $parfilename;
    pre2par::pathanalyzer::get_path_from_fullqualifiedname(\$productdirectory);
    $productdirectory =~ s/\Q$pre2par::globals::separator\E\s*$//;

    my $pardirectory = $productdirectory;
    pre2par::pathanalyzer::get_path_from_fullqualifiedname(\$pardirectory);
    $pardirectory =~ s/\Q$pre2par::globals::separator\E\s*$//;

    if ( ! -d $pardirectory ) { pre2par::systemactions::create_directory($pardirectory); }
    if ( ! -d $productdirectory ) { pre2par::systemactions::create_directory($productdirectory); }
}

1;