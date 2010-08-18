/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
