/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: getncast.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:39:03 $
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

#ifndef ARY_GETNCAST_HXX
#define ARY_GETNCAST_HXX


// USED SERVICES
#include <ary/object.hxx>




namespace ary
{


template <class DEST>
inline bool
is_type(const ary::Object & i_obj)
{
    return i_obj.AryClass() == DEST::class_id;
}


template <class DEST>
inline const DEST &
ary_cast( const Object & ce)
{
    csv_assert( is_type<DEST>(ce) );
    return static_cast< const DEST& >(ce);
}

template <class DEST>
inline DEST &
ary_cast( Object & ce)
{
    csv_assert( is_type<DEST>(ce) );
    return static_cast< DEST& >(ce);
}

template <class DEST>
inline const DEST *
ary_cast( const Object * ce)
{
    if ( ce ? is_type<DEST>(*ce) : false )
        return static_cast< const DEST* >(ce);
    return 0;
}

template <class DEST>
inline DEST *
ary_cast( Object * ce)
{
    if ( ce ? is_type<DEST>(*ce) : false )
        return static_cast< DEST* >(ce);
    return 0;
}




}   // namespace ary
#endif
