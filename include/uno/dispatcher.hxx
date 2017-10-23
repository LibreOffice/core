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

#ifndef INCLUDED_UNO_DISPATCHER_HXX
#define INCLUDED_UNO_DISPATCHER_HXX

#include "sal/config.h"

#include <cstddef>

#include "uno/dispatcher.h"

/// @cond INTERNAL

namespace com
{
namespace sun
{
namespace star
{
namespace uno
{

/** C++ holder reference for binary C uno_Interface.  Not for public use, may be
    subject to changes.

    @see uno_Interface
    @attention
              not for public use!
*/
class UnoInterfaceReference
{
public:
    uno_Interface * m_pUnoI;

    bool is() const
        { return m_pUnoI != NULL; }

    inline ~UnoInterfaceReference();
    inline UnoInterfaceReference();
    inline UnoInterfaceReference( uno_Interface * pUnoI, __sal_NoAcquire );
    inline UnoInterfaceReference( uno_Interface * pUnoI );
    inline UnoInterfaceReference( UnoInterfaceReference const & ref );

#if defined LIBO_INTERNAL_ONLY
    UnoInterfaceReference(UnoInterfaceReference && other):
        m_pUnoI(other.m_pUnoI)
    { other.m_pUnoI = nullptr; }
#endif

    uno_Interface * get() const
        { return m_pUnoI; }

    inline UnoInterfaceReference & set(
        uno_Interface * pUnoI );
    inline UnoInterfaceReference & set(
        uno_Interface * pUnoI, __sal_NoAcquire );
    inline void clear();

    UnoInterfaceReference & operator = (
        UnoInterfaceReference const & ref )
        { return set( ref.m_pUnoI ); }
    UnoInterfaceReference & operator = (
        uno_Interface * pUnoI )
        { return set( pUnoI ); }

#if defined LIBO_INTERNAL_ONLY
    UnoInterfaceReference & operator =(UnoInterfaceReference && other) {
        if (m_pUnoI != nullptr) {
            (*m_pUnoI->release)(m_pUnoI);
        }
        m_pUnoI = other.m_pUnoI;
        other.m_pUnoI = nullptr;
        return *this;
    }
#endif

    inline void dispatch(
        struct _typelib_TypeDescription const * pMemberType,
        void * pReturn, void * pArgs [], uno_Any ** ppException ) const;
};


inline UnoInterfaceReference::~UnoInterfaceReference()
{
    if (m_pUnoI != NULL)
        (*m_pUnoI->release)( m_pUnoI );
}


inline UnoInterfaceReference::UnoInterfaceReference()
    : m_pUnoI( NULL )
{
}


inline UnoInterfaceReference::UnoInterfaceReference(
    uno_Interface * pUnoI, __sal_NoAcquire )
    : m_pUnoI( pUnoI )
{
}


inline UnoInterfaceReference::UnoInterfaceReference( uno_Interface * pUnoI )
    : m_pUnoI( pUnoI )
{
    if (m_pUnoI != NULL)
        (*m_pUnoI->acquire)( m_pUnoI );
}


inline UnoInterfaceReference::UnoInterfaceReference(
    UnoInterfaceReference const & ref )
    : m_pUnoI( ref.m_pUnoI )
{
    if (m_pUnoI != NULL)
        (*m_pUnoI->acquire)( m_pUnoI );
}


inline UnoInterfaceReference & UnoInterfaceReference::set(
    uno_Interface * pUnoI )
{
    if (pUnoI != NULL)
        (*pUnoI->acquire)( pUnoI );
    if (m_pUnoI != NULL)
        (*m_pUnoI->release)( m_pUnoI );
    m_pUnoI = pUnoI;
    return *this;
}


inline UnoInterfaceReference & UnoInterfaceReference::set(
    uno_Interface * pUnoI, __sal_NoAcquire )
{
    if (m_pUnoI != NULL)
        (*m_pUnoI->release)( m_pUnoI );
    m_pUnoI = pUnoI;
    return *this;
}


inline void UnoInterfaceReference::clear()
{
    if (m_pUnoI != NULL)
    {
        (*m_pUnoI->release)( m_pUnoI );
        m_pUnoI = NULL;
    }
}


inline void UnoInterfaceReference::dispatch(
    struct _typelib_TypeDescription const * pMemberType,
    void * pReturn, void * pArgs [], uno_Any ** ppException ) const
{
    (*m_pUnoI->pDispatcher)(
        m_pUnoI, pMemberType, pReturn, pArgs, ppException );
}

}
}
}
}

/// @endcond

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
