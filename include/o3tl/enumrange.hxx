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

#ifndef INCLUDED_O3TL_ENUMRANGE_HXX
#define INCLUDED_O3TL_ENUMRANGE_HXX

#include <sal/config.h>

namespace o3tl {

///
/// This is a container convenience class iterating over scoped enumerations.
///
/// This assumes that the 'enum class' definition
///  - starts at zero
///  - has no holes in it's sequence of values
///  - defines a value called LAST which refers to the greatest constant.
///
///  Use like this:
///     enum class COLOR { RED, GREEN, BLUE, LAST=BLUE };
///     for( auto e : o3tl::enumrange<Color>() )
///        .....;
///
/// \param T the 'enum class' type.
template< typename T>
class enumrange
{
public:
    class Iterator
    {
    public:
        Iterator( int value ) :
            m_value( value )
        {
        }

        T operator*( void ) const
        {
            return static_cast<T>(m_value);
        }

        void operator++( void )
        {
            ++m_value;
        }

        bool operator!=( Iterator rhs )
        {
            return m_value != rhs.m_value;
        }

    private:
        int m_value;
   };
};

template< typename T >
typename enumrange<T>::Iterator begin( enumrange<T> )
{
    return typename enumrange<T>::Iterator( (int)0 );
}

template< typename T >
typename enumrange<T>::Iterator end( enumrange<T> )
{
    return typename enumrange<T>::Iterator( (static_cast<int>(T::LAST)) + 1 );
}

};

#endif /* INCLUDED_O3TL_ENUMRANGE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
