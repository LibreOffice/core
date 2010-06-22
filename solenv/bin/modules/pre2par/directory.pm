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