/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _TOOLS_WEAKBASE_HXX_
#define _TOOLS_WEAKBASE_HXX_

#include <tools/weakbase.h>

/// see weakbase.h for documentation

namespace tools
{

template< class reference_type >
inline WeakReference< reference_type >::WeakReference()
{
    mpWeakConnection = new WeakConnection<reference_type>( 0 );
    mpWeakConnection->acquire();
}

template< class reference_type >
inline WeakReference< reference_type >::WeakReference( reference_type* pReference )
{
    if( pReference )
        mpWeakConnection = pReference->getWeakConnection();
    else
        mpWeakConnection = new WeakConnection<reference_type>( 0 );

    mpWeakConnection->acquire();
}

template< class reference_type >
inline WeakReference< reference_type >::WeakReference( const WeakReference< reference_type >& rWeakRef )
{
    mpWeakConnection = rWeakRef.mpWeakConnection;
    mpWeakConnection->acquire();
}

template< class reference_type >
inline WeakReference< reference_type >::~WeakReference()
{
    mpWeakConnection->release();
}

template< class reference_type >
inline bool WeakReference< reference_type >::is() const
{
    return mpWeakConnection->mpReference != 0;
}

template< class reference_type >
inline reference_type * WeakReference< reference_type >::get() const
{
    return mpWeakConnection->mpReference;
}

template< class reference_type >
inline void WeakReference< reference_type >::reset( reference_type* pReference )
{
    mpWeakConnection->release();

    if( pReference )
        mpWeakConnection = pReference->getWeakConnection();
    else
        mpWeakConnection = new WeakConnection<reference_type>( 0 );

    mpWeakConnection->acquire();
}

template< class reference_type >
inline reference_type * WeakReference< reference_type >::operator->() const
{
    OSL_PRECOND(mpWeakConnection, "tools::WeakReference::operator->() : null body");
    return mpWeakConnection->mpReference;
}

template< class reference_type >
inline sal_Bool WeakReference< reference_type >::operator==(const reference_type * pReferenceObject) const
{
    return mpWeakConnection->mpReference == pReferenceObject;
}

template< class reference_type >
inline sal_Bool WeakReference< reference_type >::operator==(const WeakReference<reference_type> & handle) const
{
    return mpWeakConnection == handle.mpWeakConnection;
}

template< class reference_type >
inline sal_Bool WeakReference< reference_type >::operator!=(const WeakReference<reference_type> & handle) const
{
    return mpWeakConnection != handle.mpWeakConnection;
}

template< class reference_type >
inline sal_Bool WeakReference< reference_type >::operator<(const WeakReference<reference_type> & handle) const
{
    return mpWeakConnection->mpReference < handle.mpWeakConnection->mpReference;
}

template< class reference_type >
inline sal_Bool WeakReference< reference_type >::operator>(const WeakReference<reference_type> & handle) const
{
    return mpWeakConnection->mpReference > handle.mpWeakConnection->mpReference;
}

template< class reference_type >
inline WeakReference<reference_type>& WeakReference<reference_type>::operator= (
    const WeakReference<reference_type>& rReference)
{
    if (&rReference != this)
    {
        mpWeakConnection->release();

        mpWeakConnection = rReference.mpWeakConnection;
        mpWeakConnection->acquire();
    }
    return *this;
}

template< class reference_type >
inline WeakBase< reference_type >::WeakBase()
{
    mpWeakConnection = 0;
}

template< class reference_type >
inline WeakBase< reference_type >::~WeakBase()
{
    if( mpWeakConnection )
    {
        mpWeakConnection->mpReference = 0;
        mpWeakConnection->release();
        mpWeakConnection = 0;
    }
}

template< class reference_type >
inline void WeakBase< reference_type >::clearWeak()
{
    if( mpWeakConnection )
        mpWeakConnection->mpReference = 0;
}

template< class reference_type >
inline WeakConnection< reference_type >* WeakBase< reference_type >::getWeakConnection()
{
    if( !mpWeakConnection )
    {
        mpWeakConnection = new WeakConnection< reference_type >( static_cast< reference_type* >( this ) );
        mpWeakConnection->acquire();
    }
    return mpWeakConnection;
}

}

#endif // _TOOLS_WEAKBASE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
