#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: pathanalyzer.pm,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 09:31:54 $
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
