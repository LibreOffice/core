#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: checkdiff.pl,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 20:21:40 $
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

#
# checkit - a perl script to check the output of 'dircmp'
#

$return = 0;
$possible_error = 0;
$possible_error_descript = "";
while( <STDIN> )
{
    if( /^diff/ )
    {
        $possible_error = 1;
        $possible_error_descript = $_;
    }
    elsif( /^Binary/ )
    {
        print STDERR "ERROR : $_";
        $return++;
    }
    elsif( /^[0-9]/ && $possible_error == 1 )
    {
        print STDERR "ERROR : diff ".$possible_error_descript;
        $return++;
        $possible_error = 0;
    }
    else
    {
        $possible_error = 0;
    }
}
if( $return != 0 )
{
    unlink "$ARGV[0]";
}
exit $return;
