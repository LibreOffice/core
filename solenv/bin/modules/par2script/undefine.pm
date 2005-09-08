#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: undefine.pm,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 09:28:35 $
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
