/*************************************************************************
 *
 *  $RCSfile: UnoNamespaceMap.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: cl $ $Date: 2001-10-12 16:08:30 $
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

#include <set>

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase2.hxx>
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

#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif

#ifndef _SVX_UNOAPI_HXX_
#include "unoapi.hxx"
#endif

#ifndef _SVX_XMLCNITM_HXX
#include "xmlcnitm.hxx"
#endif

using namespace ::comphelper;
using namespace ::osl;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

namespace svx
{
    /** implements a component to export namespaces of all SvXMLAttrContainerItem inside
        one or two pools with a variable count of which ids.
    */
    class NamespaceMap : public WeakImplHelper2< XNameAccess, XServiceInfo >
    {
    private:
        sal_uInt16* mpWhichIds;
        SfxItemPool* mpPool1;
        SfxItemPool* mpPool2;
    public:
        NamespaceMap( sal_uInt16* pWhichIds, SfxItemPool* pPool1, SfxItemPool* pPool2 );
        virtual ~NamespaceMap();

        // XNameAccess
        virtual Any SAL_CALL getByName( const OUString& aName ) throw (NoSuchElementException, WrappedTargetException, RuntimeException);
        virtual Sequence< OUString > SAL_CALL getElementNames(  ) throw (RuntimeException);
        virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw (RuntimeException);

        // XElementAccess
        virtual Type SAL_CALL getElementType(  ) throw (RuntimeException);
        virtual sal_Bool SAL_CALL hasElements(  ) throw (RuntimeException);

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException);
    };

    Reference< XInterface > SAL_CALL NamespaceMap_createInstance( sal_uInt16* pWhichIds, SfxItemPool* pPool1, SfxItemPool* pPool2 )
        throw( Exception )
    {
        return (XWeak*)new NamespaceMap( pWhichIds, pPool1, pPool2 );
    }

    Sequence< OUString > SAL_CALL NamespaceMap_getSupportedServiceNames()
        throw()
    {
        Sequence< OUString > aSupportedServiceNames( 1 );
        aSupportedServiceNames[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.NamespaceMap" ) );
        return aSupportedServiceNames;
    }

    OUString SAL_CALL NamespaceMap_getImplementationName()
        throw()
    {
        return OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Svx.NamespaceMap" ) );
    }



    class NamespaceIteratorImpl
    {
    private:
        SfxItemPool* mpPool1;
        SfxItemPool* mpPool2;


        SfxItemPool* mpCurrentPool;
        sal_uInt16* mpWhichIds;
        sal_uInt16* mpWhichId;

        sal_uInt16 mnItemCount;
        sal_uInt16 mnItem;

        const SvXMLAttrContainerItem* mpCurrentAttr;
        sal_uInt16 mnCurrentAttr;

        void startPool( SfxItemPool* pPool );
    public:

        NamespaceIteratorImpl( sal_uInt16* pWhichIds, SfxItemPool* pPool1, SfxItemPool* pPool2 );

        sal_Bool next( OUString& rPrefix, OUString& rURL );
    };
}

using namespace ::svx;

// -------------

NamespaceIteratorImpl::NamespaceIteratorImpl( sal_uInt16* pWhichIds, SfxItemPool* pPool1, SfxItemPool* pPool2 )
{
    mpPool1 = pPool1;
    mpPool2 = pPool2;

    mpCurrentAttr = NULL;
    mnCurrentAttr = 0;

    mpWhichIds = pWhichIds;

    startPool( pPool1 );
}

void NamespaceIteratorImpl::startPool( SfxItemPool* pPool )
{
    mpCurrentPool = pPool;

    mpWhichId = mpWhichIds;

    mnItem = 0;
    mnItemCount = (mpWhichId && (0 != *mpWhichId) && pPool) ? pPool->GetItemCount( *mpWhichId ) : 0;
}

sal_Bool NamespaceIteratorImpl::next( OUString& rPrefix, OUString& rURL )
{
    // we still need to process the current attribute
    if( mpCurrentAttr && (mnCurrentAttr != USHRT_MAX) )
    {
        rPrefix = mpCurrentAttr->GetPrefix( mnCurrentAttr );
        rURL = mpCurrentAttr->GetNamespace( mnCurrentAttr );

        mnCurrentAttr = mpCurrentAttr->GetNextNamespaceIndex( mnCurrentAttr );
        return sal_True;
    }

    // we need the next namespace item
    mpCurrentAttr = NULL;

    const SfxPoolItem* pItem;
    // look for the next available item in the current pool
    while( (mnItem < mnItemCount) && ( NULL == (pItem = mpCurrentPool->GetItem( *mpWhichId, mnItem ) ) ) )
        mnItem++;

    // are we finished with the current whichid?
    if( mnItem == mnItemCount )
    {
        mpWhichId++;

        // are we finished with the current pool?
        if( 0 == *mpWhichId )
        {
            // we already finished both pools
            if( mpCurrentPool == mpPool2 )
                return sal_False;

            // switch to the second pool, if there is one
            mpCurrentPool = mpPool2;
            if( NULL == mpCurrentPool )
                return sal_False;

            // start with second pool
            startPool( mpPool2 );
        }
        else
        {
            mnItem = 0;
            mnItemCount = (mpWhichId && (0 != *mpWhichId) && mpCurrentPool) ? mpCurrentPool->GetItemCount( *mpWhichId ) : 0;
        }

        return next( rPrefix, rURL );
    }

    if( pItem )
    {
        mnItem++;

        // get that item and see if there namespaces inside
        const SvXMLAttrContainerItem *pUnknown = (const SvXMLAttrContainerItem *)pItem;
        if( (pUnknown->GetAttrCount() > 0) )
        {
            mpCurrentAttr = pUnknown;
            mnCurrentAttr = pUnknown->GetFirstNamespaceIndex();
        }
        return next( rPrefix, rURL );
    }

    return false;
}

// -------------

NamespaceMap::NamespaceMap( sal_uInt16* pWhichIds, SfxItemPool* pPool1, SfxItemPool* pPool2 )
: mpWhichIds( pWhichIds ), mpPool1( pPool1 ), mpPool2( pPool2 )
{
}

NamespaceMap::~NamespaceMap()
{
}

// XNameAccess
Any SAL_CALL NamespaceMap::getByName( const OUString& aName ) throw (NoSuchElementException, WrappedTargetException, RuntimeException)
{
    NamespaceIteratorImpl aIter( mpWhichIds, mpPool1, mpPool2 );

    OUString aPrefix;
    OUString aURL;

    sal_Bool bFound;

    do
    {
        bFound = aIter.next( aPrefix, aURL );
    }
    while( bFound && (aPrefix != aName ) );

    if( !bFound )
        throw NoSuchElementException();

    return makeAny( aURL );
}

Sequence< OUString > SAL_CALL NamespaceMap::getElementNames() throw (RuntimeException)
{
    NamespaceIteratorImpl aIter( mpWhichIds, mpPool1, mpPool2 );

    OUString aPrefix;
    OUString aURL;

    std::set< OUString, comphelper::UStringLess > aPrefixSet;

    while( aIter.next( aPrefix, aURL ) )
        aPrefixSet.insert( aPrefix );

    Sequence< OUString > aSeq( aPrefixSet.size() );
    OUString* pPrefixes = aSeq.getArray();

    std::set< OUString, comphelper::UStringLess >::iterator aPrefixIter( aPrefixSet.begin() );
    const std::set< OUString, comphelper::UStringLess >::iterator aEnd( aPrefixSet.end() );

    while( aPrefixIter != aEnd )
    {
        *pPrefixes++ = *aPrefixIter++;
    }

    return aSeq;
}

sal_Bool SAL_CALL NamespaceMap::hasByName( const OUString& aName ) throw (RuntimeException)
{
    NamespaceIteratorImpl aIter( mpWhichIds, mpPool1, mpPool2 );

    OUString aPrefix;
    OUString aURL;

    sal_Bool bFound;

    do
    {
        bFound = aIter.next( aPrefix, aURL );
    }
    while( bFound && (aPrefix != aName ) );

    return bFound;
}

// XElementAccess
Type SAL_CALL NamespaceMap::getElementType() throw (RuntimeException)
{
    return ::getCppuType( (const OUString*) 0 );
}

sal_Bool SAL_CALL NamespaceMap::hasElements() throw (RuntimeException)
{
    NamespaceIteratorImpl aIter( mpWhichIds, mpPool1, mpPool2 );

    OUString aPrefix;
    OUString aURL;

    return aIter.next( aPrefix, aURL );
}

// XServiceInfo
OUString SAL_CALL NamespaceMap::getImplementationName(  )
    throw(RuntimeException)
{
    return NamespaceMap_getImplementationName();
}

sal_Bool SAL_CALL NamespaceMap::supportsService( const OUString& ServiceName )
    throw(RuntimeException)
{
    return sal_True;
}

Sequence< OUString > SAL_CALL NamespaceMap::getSupportedServiceNames(  )
    throw(RuntimeException)
{
    return NamespaceMap_getSupportedServiceNames();
}

