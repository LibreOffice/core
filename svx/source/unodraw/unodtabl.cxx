/*************************************************************************
 *
 *  $RCSfile: unodtabl.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: cl $ $Date: 2000-11-12 15:51:48 $
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

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINEDASH_HPP_
#include <com/sun/star/drawing/LineDash.hpp>
#endif

#include <cppuhelper/implbase2.hxx>

#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif

#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif

#ifndef _LIST_HXX
#include<tools/list.hxx>
#endif

#include "xdash.hxx"
#include "svdmodel.hxx"
#include "xdef.hxx"
#include "xlndsit.hxx"
//#include "xflgrit.hxx"

using namespace ::com::sun::star;
using namespace ::rtl;
using namespace ::cppu;

DECLARE_LIST( ItemSetArray_Impl, SfxItemSet* )

class SvxUnoDashTable : public WeakImplHelper2< container::XNameContainer, lang::XServiceInfo >
{
private:
    SdrModel*       mpModel;
    SfxItemPool*    mpPool;

    ItemSetArray_Impl   aItemSetArray;

    void CreateName( OUString& rStrName);

public:
    SvxUnoDashTable( SdrModel* pModel ) throw();
    virtual ~SvxUnoDashTable() throw();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw( uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const  OUString& ServiceName ) throw( uno::RuntimeException);
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) throw( uno::RuntimeException);

    static OUString getImplementationName_Static() throw()
    {
        return OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.SvxUnoDashTable"));
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

SvxUnoDashTable::SvxUnoDashTable( SdrModel* pModel ) throw()
: mpModel( pModel ),
  mpPool( pModel ? &pModel->GetItemPool() : (SfxItemPool*)NULL )
{
}

SvxUnoDashTable::~SvxUnoDashTable() throw()
{
    for( int i = 0; i<aItemSetArray.Count(); i++ )
        delete (SfxItemSet*)aItemSetArray.GetObject( i );
}

sal_Bool SAL_CALL SvxUnoDashTable::supportsService( const  OUString& ServiceName ) throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSNL( getSupportedServiceNames() );
    const OUString * pArray = aSNL.getConstArray();

    for( INT32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return TRUE;

    return FALSE;
}

OUString SAL_CALL SvxUnoDashTable::getImplementationName() throw( uno::RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("SvxUnoDashTable") );
}

uno::Sequence< OUString > SAL_CALL SvxUnoDashTable::getSupportedServiceNames(  )
    throw( uno::RuntimeException )
{
    return getSupportedServiceNames_Static();
}

uno::Sequence< OUString > SvxUnoDashTable::getSupportedServiceNames_Static(void) throw()
{
    uno::Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.DashTable" ));
    return aSNS;
}


// XNameContainer
void SAL_CALL SvxUnoDashTable::insertByName( const OUString& aName, const uno::Any& aElement )
    throw( lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException )
{
    SfxItemSet* mpInSet = new SfxItemSet( *mpPool, XATTR_LINEDASH, XATTR_LINEDASH );
    aItemSetArray.Insert( mpInSet );//, aItemSetArray.Count() );

    XLineDashItem aLineDash;
    aLineDash.SetName( String( aName ) );
    aLineDash.PutValue( aElement );

    mpInSet->Put( aLineDash, XATTR_LINEDASH );
}



void SAL_CALL SvxUnoDashTable::removeByName( const OUString& Name )
    throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    /* Currently, don't know how to do this ?
    long nIndex = pTable ? ((XPropertyTable*)pTable)->Get( Name ) : -1;
    if( nIndex == -1 )
        throw container::NoSuchElementException();

    pTable->Remove( nIndex );
    */
}

// XNameReplace
void SAL_CALL SvxUnoDashTable::replaceByName( const OUString& aName, const uno::Any& aElement )
    throw( lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
    /* Currently, don't know how to do this ?
    INT32 nColor;
    if( aElement >>= nColor )
        throw lang::IllegalArgumentException();

    long nIndex = pTable ? ((XPropertyTable*)pTable)->Get( aName ) : -1;
    if( nIndex == -1  )
        throw container::NoSuchElementException();

    XColorEntry* pEntry = new XColorEntry( Color( (ColorData)nColor ), aName );
    delete pTable->Replace( nIndex, pEntry );
    */
}

// XNameAccess
uno::Any SAL_CALL SvxUnoDashTable::getByName( const  OUString& aName )
    throw( container::NoSuchElementException,  lang::WrappedTargetException, uno::RuntimeException)
{
    if( mpPool )
    {
        const String aSearchName( aName );
        const USHORT nCount = mpPool->GetItemCount(XATTR_LINEDASH);
        const XLineDashItem *pItem;

        for( USHORT nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
        {
            pItem = (XLineDashItem*)mpPool->GetItem(XATTR_LINEDASH, nSurrogate);

            if( pItem && ( pItem->GetName() == aSearchName ) )
            {
                uno::Any aAny;
                pItem->QueryValue( aAny );
                return aAny;
            }
        }
    }

    throw container::NoSuchElementException();
}

uno::Sequence< OUString > SAL_CALL SvxUnoDashTable::getElementNames(  )
    throw( uno::RuntimeException )
{
    const USHORT nCount = mpPool ? mpPool->GetItemCount(XATTR_LINEDASH) : 0;
    uno::Sequence< OUString > aSeq( nCount );
    OUString* pStrings = aSeq.getArray();
    XLineDashItem *pItem;

    for( USHORT nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
    {
        pItem = (XLineDashItem*)mpPool->GetItem(XATTR_LINEDASH, nSurrogate);

        if( pItem )
        {
            if( pItem->GetName().Len() == 0 )
                pItem->SetName( pItem->CreateStandardName( mpPool, XATTR_LINEDASH ) );

            pStrings[nSurrogate] = pItem->GetName();

            DBG_ASSERT( pStrings[nSurrogate].getLength(), "XLineDashItem in pool should have a name !");
        }
    }

    return aSeq;
}

sal_Bool SAL_CALL SvxUnoDashTable::hasByName( const OUString& aName )
    throw( uno::RuntimeException )
{
    const String aSearchName( aName );
    const USHORT nCount = mpPool ? mpPool->GetItemCount(XATTR_LINEDASH) : 0;
    uno::Sequence< OUString > aSeq( nCount );
    OUString* pStrings = aSeq.getArray();
    const XLineDashItem *pItem;

    for( USHORT nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
    {
        pItem = (XLineDashItem*)mpPool->GetItem(XATTR_LINEDASH, nSurrogate);
        if( pItem && pItem->GetName() == aSearchName )
            return sal_True;
    }

    return sal_False;
}

// XElementAccess
uno::Type SAL_CALL SvxUnoDashTable::getElementType(  )
    throw( uno::RuntimeException )
{
    return ::getCppuType((const struct drawing::LineDash*)0);
}

sal_Bool SAL_CALL SvxUnoDashTable::hasElements(  )
    throw( uno::RuntimeException )
{
    return mpPool && mpPool->GetItemCount(XATTR_LINEDASH) != 0;
}

void SvxUnoDashTable::CreateName( OUString& rStrName)
{
    const USHORT nCount = mpPool ? mpPool->GetItemCount(XATTR_LINEDASH) : 0;
    sal_Bool bFound = sal_True;

    for( sal_Int32 nPostfix = 1; nPostfix<= nCount && bFound; nPostfix++ )
    {
        rStrName = OUString::createFromAscii( "Standard " );
        rStrName += OUString::valueOf( nPostfix );
        bFound = hasByName( rStrName );
    }
}

/**
 * Create a gradienttable
 */
uno::Reference< uno::XInterface > SAL_CALL SvxUnoDashTable_createInstance( SdrModel* pModel )
{
    return *new SvxUnoDashTable(pModel);
}



