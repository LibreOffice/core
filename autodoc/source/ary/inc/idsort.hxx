/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: idsort.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:00:57 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef ARY_IDSORT_HXX
#define ARY_IDSORT_HXX


/** A compare function that sorts ids of repository entities in the same
    storage.

    @see ::ary::SortedIds
*/
template<class COMPARE>
struct IdSorter
{
    bool                operator()(
                            typename COMPARE::id_type
                                                i_1,
                            typename COMPARE::id_type
                                                i_2 ) const
                            { return COMPARE::Lesser_(
                                        COMPARE::KeyOf_(COMPARE::EntityOf_(i_1)),
                                        COMPARE::KeyOf_(COMPARE::EntityOf_(i_2)) );
                            }
};


#endif
