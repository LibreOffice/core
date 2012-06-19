/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
