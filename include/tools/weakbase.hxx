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

#ifndef INCLUDED_TOOLS_WEAKBASE_HXX
#define INCLUDED_TOOLS_WEAKBASE_HXX

#include <tools/weakbase.h>

/// see weakbase.h for documentation

namespace tools
{

template< class reference_type >
inline WeakReference< reference_type >::WeakReference()
{
    mpWeakConnection = new WeakConnection;
}

template< class reference_type >
inline WeakReference< reference_type >::WeakReference( reference_type* pReference )
{
    reset( pReference );
}

template< class reference_type >
inline WeakReference< reference_type >::WeakReference( const WeakReference< reference_type >& rWeakRef )
{
    mpWeakConnection = rWeakRef.mpWeakConnection;
}

template< class reference_type >
inline WeakReference< reference_type >::WeakReference( WeakReference< reference_type >&& rWeakRef )
{
    mpWeakConnection = std::move(rWeakRef.mpWeakConnection);
    rWeakRef.reset();
}

template< class reference_type >
inline bool WeakReference< reference_type >::is() const
{
    return mpWeakConnection->mpReference != nullptr;
}

template< class reference_type >
inline reference_type * WeakReference< reference_type >::get() const
{
    tools::WeakBase* pWeakBase = mpWeakConnection->mpReference;
    if (!pWeakBase)
        return nullptr;
    assert(dynamic_cast<reference_type *>(pWeakBase));
    return static_cast<reference_type *>(pWeakBase);
}

template< class reference_type >
inline void WeakReference< reference_type >::reset( reference_type* pReference )
{
    if( pReference )
        mpWeakConnection = pReference->getWeakConnection();
    else
        reset();
}

template< class reference_type >
inline void WeakReference< reference_type >::reset()
{
    mpWeakConnection = new WeakConnection;
}

template< class reference_type >
inline reference_type * WeakReference< reference_type >::operator->() const
{
    return get();
}

template< class reference_type >
inline reference_type& WeakReference< reference_type >::operator*() const
{
    return *get();
}

template< class reference_type >
inline bool WeakReference< reference_type >::operator==(const reference_type * pReferenceObject) const
{
    return mpWeakConnection->mpReference == pReferenceObject;
}

template< class reference_type >
inline bool WeakReference< reference_type >::operator==(const WeakReference<reference_type> & handle) const
{
    return mpWeakConnection == handle.mpWeakConnection;
}

template< class reference_type >
inline bool WeakReference< reference_type >::operator!=(const WeakReference<reference_type> & handle) const
{
    return mpWeakConnection != handle.mpWeakConnection;
}

template< class reference_type >
inline bool WeakReference< reference_type >::operator<(const WeakReference<reference_type> & handle) const
{
    return mpWeakConnection->mpReference < handle.mpWeakConnection->mpReference;
}

template< class reference_type >
inline bool WeakReference< reference_type >::operator>(const WeakReference<reference_type> & handle) const
{
    return mpWeakConnection->mpReference > handle.mpWeakConnection->mpReference;
}

template< class reference_type >
inline WeakReference<reference_type>& WeakReference<reference_type>::operator= (
    const WeakReference<reference_type>& rReference)
{
    if (&rReference != this)
        mpWeakConnection = rReference.mpWeakConnection;
    return *this;
}

template< class reference_type >
inline WeakReference<reference_type>& WeakReference<reference_type>::operator= (
    WeakReference<reference_type>&& rReference)
{
    mpWeakConnection = std::move(rReference.mpWeakConnection);
    return *this;
}

inline void WeakBase::clearWeak()
{
    if( mpWeakConnection.is() )
        mpWeakConnection->mpReference = nullptr;
}

inline WeakConnection* WeakBase::getWeakConnection()
{
    if( !mpWeakConnection.is() )
        mpWeakConnection = new WeakConnection( this );
    return mpWeakConnection.get();
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
