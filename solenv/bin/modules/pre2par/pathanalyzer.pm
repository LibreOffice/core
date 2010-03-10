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


package pre2par::pathanalyzer;

use pre2par::globals;

###########################################
# Path analyzer
###########################################

sub get_path_from_fullqualifiedname
{
    my ($longfilenameref) = @_;

    if ( $$longfilenameref =~ /\Q$pre2par::globals::separator\E/ )  # Is there a separator in the path? Otherwise the path is empty.
    {
        if ( $$longfilenameref =~ /^\s*(\S.*\S\Q$pre2par::globals::separator\E)(\S.+?\S)/ )
        {
            $$longfilenameref = $1;
        }
    }
    else
    {
        $$longfilenameref = ""; # there is no path
    }
}

sub make_absolute_filename_to_relative_filename
{
    my ($longfilenameref) = @_;

    if ( $pre2par::globals::isunix )
    {
        if ( $$longfilenameref =~ /^.*\/(\S.+\S?)/ )
        {
            $$longfilenameref = $1;
        }
    }

    if ( $pre2par::globals::iswin )
    {
        if ( $$longfilenameref =~ /^.*\\(\S.+\S?)/ )
        {
            $$longfilenameref = $1;
        }
    }
}

1;
