#*************************************************************************
#
#   $RCSfile: undefine.pm,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: rt $ $Date: 2004-02-10 14:30:28 $
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

package par2script::undefine;

use par2script::existence;
use par2script::globals;
use par2script::remover;
use par2script::work;

##########################################################
# Collecting all undefines, that are listed
# in one par file
##########################################################

sub get_list_of_undefines
{
    my ($item, $parfile) = @_;

    my @collector  =();

    for ( my $i = 0; $i <= $#{$parfile}; $i++ )
    {
        if ( ${$parfile}[$i] =~ /^\s*$item\s*(\w+)\s*$/ )
        {
            my $gid = $1;

            my $oneitem = $item;
            $oneitem =~ s/^\s*Un//; # removing the "Un"

            my %removeitem = ();
            $removeitem{'gid'} = $gid;
            $removeitem{'item'} = $oneitem;

            push(@collector, \%removeitem);
        }
    }

    return \@collector;
}

##########################################################
# Removing in the script all the gids, that are listed
# in undefine scp files
##########################################################

sub undefine_gids
{
    my ($script, $parfile) = @_;

    my @allundefines = ();

    for ( my $i = 0; $i <= $#par2script::globals::allitems; $i++ )
    {
        my $oneitem = "Un" . $par2script::globals::allitems[$i];

        my $collector = get_list_of_undefines($oneitem, $parfile);

        if ($#{$collector} > -1 ) { par2script::work::add_array_into_array(\@allundefines, $collector); }
    }

    if ($#allundefines > -1 )
    {
        for ( my $i = 0; $i <= $#allundefines; $i++ )
        {
            my $gid = $allundefines[$i]->{'gid'};
            par2script::remover::remove_leading_and_ending_whitespaces(\$gid);
            par2script::work::remove_definitionblock_from_script($script, $gid);
        }
    }
}


1;
