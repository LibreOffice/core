/*************************************************************************
 *
 *  $RCSfile: dispatcher.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-10-06 13:01:32 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

