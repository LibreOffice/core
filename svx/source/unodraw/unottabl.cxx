/*************************************************************************
 *
 *  $RCSfile: unottabl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:27 $
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
#ifndef _COM_SUN_STAR_AWT_GRADIENT_HPP_
#include <com/sun/star/awt/Gradient.hpp>
#endif

#include <cppuhelper/implbase2.hxx>

#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif

#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif

#ifndef _SVX__XGRADIENT_HXX //autogen
#include "xgrad.hxx"
#endif
#ifndef _SVX_XFLFTRIT_HXX
#include "xflftrit.hxx"
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

class SvxUnoTransGradientTable : public WeakImplHelper2< container::XNameContainer, lang::XServiceInfo >
{
private:
    XBitmapList*    mpBitmapList;
    SdrModel*       mpModel;
    SfxItemPool*    mpPool;

    ItemSetArray_Impl   aItemSetArray;

    const OUString GetOrCreateName( USHORT nSurrogate );

public:
    SvxUnoTransGradientTable( SdrModel* pModel ) throw();
    virtual ~SvxUnoTransGradientTable() throw();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw( uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const  OUString& ServiceName ) throw( uno::RuntimeException);
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) throw( uno::RuntimeException);

    static OUString getImplementationName_Static() throw()
    {
        return OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.SvxUnoTransGradientTable"));
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

SvxUnoTransGradientTable::SvxUnoTransGradientTable( SdrModel* pModel ) throw()
: mpModel( pModel ),
  mpBitmapList( pModel ? pModel->GetBitmapList() : NULL ),
  mpPool( pModel ? &pModel->GetItemPool() : (SfxItemPool*)NULL )
{
}

SvxUnoTransGradientTable::~SvxUnoTransGradientTable() throw()
{
    for( int i = 0; i<aItemSetArray.Count(); i++ )
        delete (SfxItemSet*)aItemSetArray.GetObject( i );
}

sal_Bool SAL_CALL SvxUnoTransGradientTable::supportsService( const  OUString& ServiceName ) throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSNL( getSupportedServiceNames() );
    const OUString * pArray = aSNL.getConstArray();

    for( INT32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return TRUE;

    return FALSE;
}

OUString SAL_CALL SvxUnoTransGradientTable::getImplementationName() throw( uno::RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("SvxUnoTransGradientTable") );
}

uno::Sequence< OUString > SAL_CALL SvxUnoTransGradientTable::getSupportedServiceNames(  )
    throw( uno::RuntimeException )
{
    return getSupportedServiceNames_Static();
}

uno::Sequence< OUString > SvxUnoTransGradientTable::getSupportedServiceNames_Static(void) throw()
{
    uno::Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.TransparencyGradientTable" ));
    return aSNS;
}

// XNameContainer
void SAL_CALL SvxUnoTransGradientTable::insertByName( const OUString& aName, const uno::Any& aElement )
    throw( lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException )
{
    if( hasByName( aName ) )
        throw container::ElementExistException();

    SfxItemSet* mpInSet = new SfxItemSet( *mpPool, XATTR_FILLFLOATTRANSPARENCE, XATTR_FILLFLOATTRANSPARENCE );
    aItemSetArray.Insert( mpInSet );//, aItemSetArray.Count() );

    XFillFloatTransparenceItem aTransGradient;
    aTransGradient.SetName( String( aName ) );
    aTransGradient.PutValue( aElement );
    aTransGradient.SetEnabled( TRUE );

    mpInSet->Put( aTransGradient, XATTR_FILLFLOATTRANSPARENCE );
}

void SAL_CALL SvxUnoTransGradientTable::removeByName( const OUString& Name )
    throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
}

// XNameReplace
void SAL_CALL SvxUnoTransGradientTable::replaceByName( const OUString& aName, const uno::Any& aElement )
    throw( lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
}

// XNameAccess
uno::Any SAL_CALL SvxUnoTransGradientTable::getByName( const  OUString& aName )
    throw( container::NoSuchElementException,  lang::WrappedTargetException, uno::RuntimeException)
{
    if( mpPool )
    {
        const OUString aSearchName( aName );
        const USHORT nCount = mpPool->GetItemCount(XATTR_FILLFLOATTRANSPARENCE);
        const XFillFloatTransparenceItem *pItem;

        for( USHORT nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
        {
            pItem = (XFillFloatTransparenceItem*)mpPool->GetItem(XATTR_FILLFLOATTRANSPARENCE, nSurrogate);

            if( pItem && ( GetOrCreateName( nSurrogate ) == aSearchName ) )
            {
                uno::Any aAny;
                pItem->QueryValue( aAny );
                return aAny;
            }
        }
    }

    throw container::NoSuchElementException();
}

uno::Sequence< OUString > SAL_CALL SvxUnoTransGradientTable::getElementNames(  )
    throw( uno::RuntimeException )
{
    const USHORT nCount = mpPool ? mpPool->GetItemCount(XATTR_FILLFLOATTRANSPARENCE) : 0;
    uno::Sequence< OUString > aSeq( nCount );
    OUString* pStrings = aSeq.getArray();
    const XFillFloatTransparenceItem *pItem;

    for( USHORT nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
    {
        pItem = (XFillFloatTransparenceItem*)mpPool->GetItem(XATTR_FILLFLOATTRANSPARENCE, nSurrogate);
        if( pItem )
            pStrings[nSurrogate] = GetOrCreateName( nSurrogate );
    }

    return aSeq;
}

sal_Bool SAL_CALL SvxUnoTransGradientTable::hasByName( const OUString& aName )
    throw( uno::RuntimeException )
{
    const OUString aSearchName( aName );
    const USHORT nCount = mpPool ? mpPool->GetItemCount(XATTR_FILLFLOATTRANSPARENCE) : 0;
    uno::Sequence< OUString > aSeq( nCount );
    OUString* pStrings = aSeq.getArray();
    const XFillFloatTransparenceItem *pItem;

    for( USHORT nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
    {
        pItem = (XFillFloatTransparenceItem*)mpPool->GetItem(XATTR_FILLFLOATTRANSPARENCE, nSurrogate);
        if( GetOrCreateName( nSurrogate ) == aSearchName )
            return sal_True;
    }

    return sal_False;
}

const OUString SvxUnoTransGradientTable::GetOrCreateName( USHORT nSurrogate )
{
    String aStrName;
    XFillFloatTransparenceItem* pItem = (XFillFloatTransparenceItem*)mpPool->
                                        GetItem( XATTR_FILLFLOATTRANSPARENCE, nSurrogate );
    if( pItem )
    {
        aStrName = pItem->GetName();

        if( !aStrName.Len() )
        {
            aStrName = String::CreateFromAscii( "TrGr" );
            aStrName += String::CreateFromInt32( sal_Int32( nSurrogate ) );

            pItem->SetName( aStrName );
        }
    }

    return aStrName;
}

// XElementAccess
uno::Type SAL_CALL SvxUnoTransGradientTable::getElementType(  )
    throw( uno::RuntimeException )
{
    return ::getCppuType((const struct awt::Gradient*)0);
}

sal_Bool SAL_CALL SvxUnoTransGradientTable::hasElements(  )
    throw( uno::RuntimeException )
{
    return mpPool && mpPool->GetItemCount(XATTR_FILLFLOATTRANSPARENCE) != 0;
}

/**
 * Create a hatchtable
 */
uno::Reference< uno::XInterface > SAL_CALL SvxUnoTransGradientTable_createInstance( SdrModel* pModel )
{
    return *new SvxUnoTransGradientTable(pModel);
}



