/*************************************************************************
 *
 *  $RCSfile: namecontainer.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: cl $ $Date: 2001-03-20 19:52:46 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
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

#ifndef _COMPHELPER_NAMECONTAINER_HXX_
#include <comphelper/namecontainer.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

DECLARE_STL_USTRINGACCESS_MAP( ::com::sun::star::uno::Any, SvGenericNameContainerMapImpl );

namespace comphelper
{
    struct NameContainerBaseImpl
    {
        SvGenericNameContainerMapImpl maProperties;
        osl::Mutex maMutex;
    };
}

using namespace ::comphelper;
using namespace ::osl;
using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;


NameContainerBase::NameContainerBase()
{
    mp = new NameContainerBaseImpl;
}

NameContainerBase::~NameContainerBase()
{
    delete mp;
}

// XNameContainer
void SAL_CALL NameContainerBase::insertByName( const rtl::OUString& aName, const Any& aElement )
    throw(IllegalArgumentException, ElementExistException,
        WrappedTargetException, RuntimeException)
{
    MutexGuard aGuard( mp->maMutex );

    if( mp->maProperties.find( aName ) != mp->maProperties.end() )
        throw ElementExistException();

    if( aElement.getValueType() != getElementType() )
        throw IllegalArgumentException();

    mp->maProperties[ aName ] = aElement;
}

void SAL_CALL NameContainerBase::removeByName( const ::rtl::OUString& Name )
    throw(NoSuchElementException, WrappedTargetException,
        RuntimeException)
{
    MutexGuard aGuard( mp->maMutex );

    SvGenericNameContainerMapImpl::iterator aIter = mp->maProperties.find( Name );
    if( aIter == mp->maProperties.end() )
        throw NoSuchElementException();

    mp->maProperties.erase( aIter );
}

// XNameReplace

void SAL_CALL NameContainerBase::replaceByName( const ::rtl::OUString& aName, const Any& aElement )
    throw(IllegalArgumentException, NoSuchElementException,
        WrappedTargetException, RuntimeException)
{
    MutexGuard aGuard( mp->maMutex );

    SvGenericNameContainerMapImpl::iterator aIter( mp->maProperties.find( aName ) );
    if( aIter == mp->maProperties.end() )
        throw NoSuchElementException();

    if( aElement.getValueType() != getElementType() )
        throw IllegalArgumentException();

    (*aIter).second = aElement;
}

// XNameAccess

Any SAL_CALL NameContainerBase::getByName( const ::rtl::OUString& aName )
    throw(NoSuchElementException, WrappedTargetException,
        RuntimeException)
{
    MutexGuard aGuard( mp->maMutex );

    SvGenericNameContainerMapImpl::iterator aIter = mp->maProperties.find( aName );
    if( aIter == mp->maProperties.end() )
        throw NoSuchElementException();

    return (*aIter).second;
}

Sequence< ::rtl::OUString > SAL_CALL NameContainerBase::getElementNames(  )
    throw(RuntimeException)
{
    MutexGuard aGuard( mp->maMutex );

    SvGenericNameContainerMapImpl::iterator aIter = mp->maProperties.begin();
    const SvGenericNameContainerMapImpl::iterator aEnd = mp->maProperties.end();

    Sequence< rtl::OUString > aNames( mp->maProperties.size() );
    rtl::OUString* pNames = aNames.getArray();

    while( aIter != aEnd )
    {
        *pNames++ = (*aIter++).first;
    }

    return aNames;
}

sal_Bool SAL_CALL NameContainerBase::hasByName( const ::rtl::OUString& aName )
    throw(RuntimeException)
{
    MutexGuard aGuard( mp->maMutex );

    SvGenericNameContainerMapImpl::iterator aIter = mp->maProperties.find( aName );
    return aIter != mp->maProperties.end();
}

sal_Bool SAL_CALL NameContainerBase::hasElements(  )
    throw(RuntimeException)
{
    MutexGuard aGuard( mp->maMutex );

    return mp->maProperties.size() != 0;
}
