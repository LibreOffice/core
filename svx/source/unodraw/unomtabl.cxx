/*************************************************************************
 *
 *  $RCSfile: unomtabl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pw $ $Date: 2000-10-12 11:58:25 $
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

#ifndef _SFXINIMGR_HXX
#include <svtools/iniman.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POINTSEQUENCE_HPP_
#include <com/sun/star/drawing/PointSequence.hpp>
#endif

#include <cppuhelper/implbase2.hxx>

#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif

#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif

#ifndef _SVX_XLNEDIT_HXX //autogen
#include <xlnedit.hxx>
#endif
#ifndef _SVX_XLNSTIT_HXX
#include <xlnstit.hxx>
#endif
#include "svdmodel.hxx"
#include "xdef.hxx"
#include "xflhtit.hxx"

#ifndef _LIST_HXX
#include<tools/list.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::rtl;
using namespace ::cppu;

DECLARE_LIST( ItemSetArray_Impl, SfxItemSet* )

class SvxUnoMarkerTable : public WeakImplHelper2< container::XNameContainer, lang::XServiceInfo >
{
private:
    SdrModel*       mpModel;
    SfxItemPool*    mpPool;

    ItemSetArray_Impl   aItemSetArray;

    void CreateName( OUString& rStrName );

public:
    SvxUnoMarkerTable( SdrModel* pModel ) throw();
    virtual ~SvxUnoMarkerTable() throw();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw( uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const  OUString& ServiceName ) throw( uno::RuntimeException);
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) throw( uno::RuntimeException);

    static OUString getImplementationName_Static() throw()
    {
        return OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.SvxUnoMarkerTable"));
    }

    static uno::Sequence< OUString >  getSupportedServiceNames_Static(void) throw();

    // XNameContainer
    virtual void SAL_CALL insertByName( const  OUString& aName, const  uno::Any& aElement ) throw( lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException);
    virtual void SAL_CALL removeByName( const  OUString& Name ) throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);

    // XNameReplace
    virtual void SAL_CALL replaceByName( const  OUString& aName, const  uno::Any& aElement ) throw( lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);

    // XNameAccess
    virtual uno::Any SAL_CALL getByName( const  OUString& aName ) throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);
    virtual uno::Sequence<  OUString > SAL_CALL getElementNames(  ) throw( uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const  OUString& aName ) throw( uno::RuntimeException);

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) throw( uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw( uno::RuntimeException);
};

SvxUnoMarkerTable::SvxUnoMarkerTable( SdrModel* pModel ) throw()
: mpModel( pModel ),
  mpPool( pModel ? &pModel->GetItemPool() : (SfxItemPool*)NULL )
{
}

SvxUnoMarkerTable::~SvxUnoMarkerTable() throw()
{
    for( int i = 0; i<aItemSetArray.Count(); i++ )
        delete (SfxItemSet*)aItemSetArray.GetObject( i );
}

sal_Bool SAL_CALL SvxUnoMarkerTable::supportsService( const  OUString& ServiceName ) throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSNL( getSupportedServiceNames() );
    const OUString * pArray = aSNL.getConstArray();

    for( INT32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return TRUE;

    return FALSE;
}

OUString SAL_CALL SvxUnoMarkerTable::getImplementationName() throw( uno::RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("SvxUnoMarkerTable") );
}

uno::Sequence< OUString > SAL_CALL SvxUnoMarkerTable::getSupportedServiceNames(  )
    throw( uno::RuntimeException )
{
    return getSupportedServiceNames_Static();
}

uno::Sequence< OUString > SvxUnoMarkerTable::getSupportedServiceNames_Static(void) throw()
{
    uno::Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.MarkerTable" ));
    return aSNS;
}

// XNameContainer
void SAL_CALL SvxUnoMarkerTable::insertByName( const OUString& aName, const uno::Any& aElement )
    throw( lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException )
{
    if( hasByName( aName ) )
        throw container::ElementExistException();

    SfxItemSet* mpInSet = new SfxItemSet( *mpPool, XATTR_LINESTART, XATTR_LINEEND );
    aItemSetArray.Insert( mpInSet );//, aItemSetArray.Count() );

    XLineEndItem aEndMarker;
    aEndMarker.SetName( String( aName ) );
    aEndMarker.PutValue( aElement );

    mpInSet->Put( aEndMarker, XATTR_LINEEND );

    XLineStartItem aStartMarker;
    aStartMarker.SetName( String( aName ) );
    aStartMarker.PutValue( aElement );

    mpInSet->Put( aStartMarker, XATTR_LINESTART );
}

void SAL_CALL SvxUnoMarkerTable::removeByName( const OUString& Name )
    throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
}

// XNameReplace
void SAL_CALL SvxUnoMarkerTable::replaceByName( const OUString& aName, const uno::Any& aElement )
    throw( lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
}

// XNameAccess
uno::Any SAL_CALL SvxUnoMarkerTable::getByName( const  OUString& aName )
    throw( container::NoSuchElementException,  lang::WrappedTargetException, uno::RuntimeException)
{
    if( mpPool )
    {
        const String aSearchName( aName );
        USHORT nCount = mpPool->GetItemCount( XATTR_LINEEND );
        const XLineEndItem *pEndItem;

        for( USHORT nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
        {
            pEndItem = (XLineEndItem*)mpPool->GetItem( XATTR_LINEEND, nSurrogate );

            if( pEndItem && ( pEndItem->GetName() == aSearchName ) )
            {
                uno::Any aAny;
                pEndItem->QueryValue( aAny );
                return aAny;
            }

        }

        const XLineStartItem *pStartItem;
        nCount = mpPool->GetItemCount( XATTR_LINESTART );

        for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
        {
            pStartItem = (XLineStartItem*)mpPool->GetItem( XATTR_LINESTART, nSurrogate );

            if( pStartItem && ( pStartItem->GetName() == aSearchName ) )
            {
                uno::Any aAny;
                pStartItem->QueryValue( aAny );
                return aAny;
            }
        }
    }

    throw container::NoSuchElementException();
}

uno::Sequence< OUString > SAL_CALL SvxUnoMarkerTable::getElementNames(  )
    throw( uno::RuntimeException )
{
    const USHORT nEndCount = mpPool ? mpPool->GetItemCount( XATTR_LINEEND ) : 0;
    const USHORT nStartCount = mpPool ? mpPool->GetItemCount( XATTR_LINESTART ) : 0;
    uno::Sequence< OUString > aSeq( nEndCount+nStartCount );
    OUString* pStrings = aSeq.getArray();

    XLineEndItem *pEndItem;
    for( USHORT nSurrogate = 0; nSurrogate < nEndCount; nSurrogate++ )
    {
        pEndItem = (XLineEndItem*)mpPool->GetItem( XATTR_LINEEND, nSurrogate);

        if( pEndItem )
        {
            pStrings[nSurrogate] = pEndItem->GetName();

            DBG_ASSERT( pStrings[nSurrogate].getLength(), "XLineEndItem in pool should have a name !");
        }
    }

    XLineStartItem *pStartItem;
    for( nSurrogate = 0; nSurrogate < nStartCount; nSurrogate++ )
    {
        pStartItem = (XLineStartItem*)mpPool->GetItem( XATTR_LINESTART, nSurrogate);

        if( pStartItem )
        {
            pStrings[nSurrogate+nEndCount] = pStartItem->GetName();

            DBG_ASSERT( pStrings[nSurrogate].getLength(), "XLineStartItem in pool should have a name !");
        }
    }

    return aSeq;
}

sal_Bool SAL_CALL SvxUnoMarkerTable::hasByName( const OUString& aName )
    throw( uno::RuntimeException )
{
    const String aSearchName( aName );
    const USHORT nStartCount = mpPool ? mpPool->GetItemCount( XATTR_LINESTART ) : 0;
    const USHORT nEndCount = mpPool ? mpPool->GetItemCount( XATTR_LINEEND ) : 0;
    const XLineEndItem *pEndItem;
    const XLineStartItem *pStartItem;

    for( USHORT nSurrogate = 0; nSurrogate < nStartCount; nSurrogate++ )
    {
        pStartItem = (XLineStartItem*)mpPool->GetItem( XATTR_LINESTART, nSurrogate);
        if( pStartItem && pStartItem->GetName() == aSearchName )
            return sal_True;
    }

    for( nSurrogate = 0; nSurrogate < nEndCount; nSurrogate++ )
    {
        pEndItem = (XLineEndItem*)mpPool->GetItem( XATTR_LINEEND, nSurrogate);
        if( pEndItem && pEndItem->GetName() == aSearchName )
            return sal_True;
    }

    return sal_False;
}

void SvxUnoMarkerTable::CreateName( OUString& rStrName )
{
    const USHORT nStartCount = mpPool ? mpPool->GetItemCount(XATTR_LINESTART) : 0;
    const USHORT nEndCount = mpPool ? mpPool->GetItemCount(XATTR_LINEEND) : 0;
    const USHORT nCount = nStartCount > nEndCount ? nStartCount : nEndCount;
    sal_Bool bFound = sal_True;

    for( sal_Int32 nPostfix = 1; nPostfix<= nCount && bFound; nPostfix++ )
    {
        rStrName = OUString::createFromAscii( "Standard " );
        rStrName += OUString::valueOf( nPostfix );
        bFound = hasByName( rStrName );
    }
}

// XElementAccess
uno::Type SAL_CALL SvxUnoMarkerTable::getElementType(  )
    throw( uno::RuntimeException )
{
    return ::getCppuType((const drawing::PointSequence*)0);
}

sal_Bool SAL_CALL SvxUnoMarkerTable::hasElements(  )
    throw( uno::RuntimeException )
{
    return mpPool && mpPool->GetItemCount(XATTR_LINEEND) != 0;
}

/**
 * Create a hatchtable
 */
uno::Reference< uno::XInterface > SAL_CALL SvxUnoMarkerTable_createInstance( SdrModel* pModel )
{
    return *new SvxUnoMarkerTable(pModel);
}



