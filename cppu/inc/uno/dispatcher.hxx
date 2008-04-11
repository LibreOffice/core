/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dispatcher.hxx,v $
 * $Revision: 1.4 $
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

#if ! defined INCLUDED_UNO_DISPATCHER_HXX
#define INCLUDED_UNO_DISPATCHER_HXX

#include "uno/dispatcher.h"


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
    @internal
              not for public use!
*/
class UnoInterfaceReference
{
public:
    uno_Interface * m_pUnoI;

    inline bool is() const
        { return m_pUnoI != 0; }

    inline ~UnoInterfaceReference();
    inline UnoInterfaceReference();
    inline UnoInterfaceReference( uno_Interface * pUnoI, __sal_NoAcquire );
    inline UnoInterfaceReference( uno_Interface * pUnoI );
    inline UnoInterfaceReference( UnoInterfaceReference const & ref );

    inline uno_Interface * get() const
        { return m_pUnoI; }

    inline UnoInterfaceReference & set(
        uno_Interface * pUnoI );
    inline UnoInterfaceReference & set(
        uno_Interface * pUnoI, __sal_NoAcquire );
    inline void clear();

    inline UnoInterfaceReference & operator = (
        UnoInterfaceReference const & ref )
        { return set( ref.m_pUnoI ); }
    inline UnoInterfaceReference & operator = (
        uno_Interface * pUnoI )
        { return set( pUnoI ); }

    inline void dispatch(
        struct _typelib_TypeDescription const * pMemberType,
        void * pReturn, void * pArgs [], uno_Any ** ppException ) const;

private:
    inline bool operator == ( UnoInterfaceReference const & ); // not impl
    inline bool operator != ( UnoInterfaceReference const & ); // not impl
    inline bool operator == ( uno_Interface * ); // not impl
    inline bool operator != ( uno_Interface * ); // not impl
};

//______________________________________________________________________________
inline UnoInterfaceReference::~UnoInterfaceReference()
{
    if (m_pUnoI != 0)
        (*m_pUnoI->release)( m_pUnoI );
}

//______________________________________________________________________________
inline UnoInterfaceReference::UnoInterfaceReference()
    : m_pUnoI( 0 )
{
}

//______________________________________________________________________________
inline UnoInterfaceReference::UnoInterfaceReference(
    uno_Interface * pUnoI, __sal_NoAcquire )
    : m_pUnoI( pUnoI )
{
}

//______________________________________________________________________________
inline UnoInterfaceReference::UnoInterfaceReference( uno_Interface * pUnoI )
    : m_pUnoI( pUnoI )
{
    if (m_pUnoI != 0)
        (*m_pUnoI->acquire)( m_pUnoI );
}

//______________________________________________________________________________
inline UnoInterfaceReference::UnoInterfaceReference(
    UnoInterfaceReference const & ref )
    : m_pUnoI( ref.m_pUnoI )
{
    if (m_pUnoI != 0)
        (*m_pUnoI->acquire)( m_pUnoI );
}

//______________________________________________________________________________
inline UnoInterfaceReference & UnoInterfaceReference::set(
    uno_Interface * pUnoI )
{
    if (pUnoI != 0)
        (*pUnoI->acquire)( pUnoI );
    if (m_pUnoI != 0)
        (*m_pUnoI->release)( m_pUnoI );
    m_pUnoI = pUnoI;
    return *this;
}

//______________________________________________________________________________
inline UnoInterfaceReference & UnoInterfaceReference::set(
    uno_Interface * pUnoI, __sal_NoAcquire )
{
    if (m_pUnoI != 0)
        (*m_pUnoI->release)( m_pUnoI );
    m_pUnoI = pUnoI;
    return *this;
}

//______________________________________________________________________________
inline void UnoInterfaceReference::clear()
{
    if (m_pUnoI != 0)
    {
        (*m_pUnoI->release)( m_pUnoI );
        m_pUnoI = 0;
    }
}

//______________________________________________________________________________
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

#endif

