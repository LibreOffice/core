/*************************************************************************
 *
 *  $RCSfile: defltuno.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: nn $ $Date: 2001-02-23 13:37:10 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include <svtools/smplhint.hxx>
#include <svtools/itemprop.hxx>
#include <svx/unomid.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>

#include "scitems.hxx"
#include "defltuno.hxx"
#include "miscuno.hxx"
#include "docsh.hxx"
#include "docpool.hxx"
#include "unoguard.hxx"
#include "unonames.hxx"

using namespace ::com::sun::star;

//------------------------------------------------------------------------

const SfxItemPropertyMap* lcl_GetDocDefaultsMap()
{
    static SfxItemPropertyMap aDocDefaultsMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_CFCHARS),  ATTR_FONT,          &getCppuType((sal_Int16*)0),        0, MID_FONT_CHAR_SET },
        {MAP_CHAR_LEN(SC_UNO_CJK_CFCHARS),  ATTR_CJK_FONT,      &getCppuType((sal_Int16*)0),        0, MID_FONT_CHAR_SET },
        {MAP_CHAR_LEN(SC_UNO_CTL_CFCHARS),  ATTR_CTL_FONT,      &getCppuType((sal_Int16*)0),        0, MID_FONT_CHAR_SET },
        {MAP_CHAR_LEN(SC_UNONAME_CFFAMIL),  ATTR_FONT,          &getCppuType((sal_Int16*)0),        0, MID_FONT_FAMILY },
        {MAP_CHAR_LEN(SC_UNO_CJK_CFFAMIL),  ATTR_CJK_FONT,      &getCppuType((sal_Int16*)0),        0, MID_FONT_FAMILY },
        {MAP_CHAR_LEN(SC_UNO_CTL_CFFAMIL),  ATTR_CTL_FONT,      &getCppuType((sal_Int16*)0),        0, MID_FONT_FAMILY },
        {MAP_CHAR_LEN(SC_UNONAME_CFNAME),   ATTR_FONT,          &getCppuType((rtl::OUString*)0),    0, MID_FONT_FAMILY_NAME },
        {MAP_CHAR_LEN(SC_UNO_CJK_CFNAME),   ATTR_CJK_FONT,      &getCppuType((rtl::OUString*)0),    0, MID_FONT_FAMILY_NAME },
        {MAP_CHAR_LEN(SC_UNO_CTL_CFNAME),   ATTR_CTL_FONT,      &getCppuType((rtl::OUString*)0),    0, MID_FONT_FAMILY_NAME },
        {MAP_CHAR_LEN(SC_UNONAME_CFPITCH),  ATTR_FONT,          &getCppuType((sal_Int16*)0),        0, MID_FONT_PITCH },
        {MAP_CHAR_LEN(SC_UNO_CJK_CFPITCH),  ATTR_CJK_FONT,      &getCppuType((sal_Int16*)0),        0, MID_FONT_PITCH },
        {MAP_CHAR_LEN(SC_UNO_CTL_CFPITCH),  ATTR_CTL_FONT,      &getCppuType((sal_Int16*)0),        0, MID_FONT_PITCH },
        {MAP_CHAR_LEN(SC_UNONAME_CFSTYLE),  ATTR_FONT,          &getCppuType((rtl::OUString*)0),    0, MID_FONT_STYLE_NAME },
        {MAP_CHAR_LEN(SC_UNO_CJK_CFSTYLE),  ATTR_CJK_FONT,      &getCppuType((rtl::OUString*)0),    0, MID_FONT_STYLE_NAME },
        {MAP_CHAR_LEN(SC_UNO_CTL_CFSTYLE),  ATTR_CTL_FONT,      &getCppuType((rtl::OUString*)0),    0, MID_FONT_STYLE_NAME },
        {0,0,0,0}
    };
    return aDocDefaultsMap_Impl;
}

//------------------------------------------------------------------------

SC_SIMPLE_SERVICE_INFO( ScDocDefaultsObj, "ScDocDefaultsObj", "com.sun.star.sheet.Defaults" )

//------------------------------------------------------------------------

ScDocDefaultsObj::ScDocDefaultsObj(ScDocShell* pDocSh) :
    pDocShell( pDocSh )
{
    pDocShell->GetDocument()->AddUnoObject(*this);
}

ScDocDefaultsObj::~ScDocDefaultsObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScDocDefaultsObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       // document gone
    }
}

void ScDocDefaultsObj::ItemsChanged()
{
    if (pDocShell)
    {
        //! if not in XML import, adjust row heights

        pDocShell->PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_GRID );
    }
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScDocDefaultsObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( lcl_GetDocDefaultsMap() );
    return aRef;
}

void SAL_CALL ScDocDefaultsObj::setPropertyValue(
                        const rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;

    if ( !pDocShell )
        throw uno::RuntimeException();

    const SfxItemPropertyMap* pMap =
            SfxItemPropertyMap::GetByName( lcl_GetDocDefaultsMap(), aPropertyName );
    if ( !pMap )
        throw beans::UnknownPropertyException();

    ScDocumentPool* pPool = pDocShell->GetDocument()->GetPool();
    SfxPoolItem* pNewItem = pPool->GetDefaultItem(pMap->nWID).Clone();

    if( !pNewItem->PutValue( aValue, pMap->nMemberId ) )
        throw lang::IllegalArgumentException();

    pPool->SetPoolDefaultItem( *pNewItem );
    delete pNewItem;    // copied in SetPoolDefaultItem

    ItemsChanged();
}

uno::Any SAL_CALL ScDocDefaultsObj::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    //  use pool default if set

    ScUnoGuard aGuard;

    if ( !pDocShell )
        throw uno::RuntimeException();

    const SfxItemPropertyMap* pMap =
            SfxItemPropertyMap::GetByName( lcl_GetDocDefaultsMap(), aPropertyName );
    if ( !pMap )
        throw beans::UnknownPropertyException();

    ScDocumentPool* pPool = pDocShell->GetDocument()->GetPool();
    const SfxPoolItem& rItem = pPool->GetDefaultItem( pMap->nWID );
    uno::Any aRet;
    rItem.QueryValue( aRet, pMap->nMemberId );
    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScDocDefaultsObj )

// XPropertyState

beans::PropertyState SAL_CALL ScDocDefaultsObj::getPropertyState( const rtl::OUString& aPropertyName )
                                throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    ScUnoGuard aGuard;

    if ( !pDocShell )
        throw uno::RuntimeException();

    const SfxItemPropertyMap* pMap =
            SfxItemPropertyMap::GetByName( lcl_GetDocDefaultsMap(), aPropertyName );
    if ( !pMap )
        throw beans::UnknownPropertyException();

    beans::PropertyState eRet = beans::PropertyState_DEFAULT_VALUE;

    USHORT nWID = pMap->nWID;
    if ( nWID == ATTR_FONT || nWID == ATTR_CJK_FONT || nWID == ATTR_CTL_FONT )
    {
        //  static default for font is system-dependent,
        //  so font default is always treated as "direct value".

        eRet = beans::PropertyState_DIRECT_VALUE;
    }
    else
    {
        //  check if pool default is set

        ScDocumentPool* pPool = pDocShell->GetDocument()->GetPool();
        if ( pPool->GetPoolDefaultItem( nWID ) != NULL )
            eRet = beans::PropertyState_DIRECT_VALUE;
    }

    return eRet;
}

uno::Sequence<beans::PropertyState> SAL_CALL ScDocDefaultsObj::getPropertyStates(
                            const uno::Sequence<rtl::OUString>& aPropertyNames )
                    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    //  the simple way: call getPropertyState

    ScUnoGuard aGuard;
    const rtl::OUString* pNames = aPropertyNames.getConstArray();
    uno::Sequence<beans::PropertyState> aRet(aPropertyNames.getLength());
    beans::PropertyState* pStates = aRet.getArray();
    for(sal_Int32 i = 0; i < aPropertyNames.getLength(); i++)
        pStates[i] = getPropertyState(pNames[i]);
    return aRet;
}

void SAL_CALL ScDocDefaultsObj::setPropertyToDefault( const rtl::OUString& aPropertyName )
                            throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    ScUnoGuard aGuard;

    if ( !pDocShell )
        throw uno::RuntimeException();

    const SfxItemPropertyMap* pMap =
            SfxItemPropertyMap::GetByName( lcl_GetDocDefaultsMap(), aPropertyName );
    if ( !pMap )
        throw beans::UnknownPropertyException();

    ScDocumentPool* pPool = pDocShell->GetDocument()->GetPool();
    pPool->ResetPoolDefaultItem( pMap->nWID );

    ItemsChanged();
}

uno::Any SAL_CALL ScDocDefaultsObj::getPropertyDefault( const rtl::OUString& aPropertyName )
                            throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                                    uno::RuntimeException)
{
    //  always use static default

    ScUnoGuard aGuard;

    if ( !pDocShell )
        throw uno::RuntimeException();

    const SfxItemPropertyMap* pMap =
            SfxItemPropertyMap::GetByName( lcl_GetDocDefaultsMap(), aPropertyName );
    if ( !pMap )
        throw beans::UnknownPropertyException();

    ScDocumentPool* pPool = pDocShell->GetDocument()->GetPool();
    const SfxPoolItem* pItem = pPool->GetItem( pMap->nWID, SFX_ITEMS_STATICDEFAULT );
    uno::Any aRet;
    if (pItem)
        pItem->QueryValue( aRet, pMap->nMemberId );
    return aRet;
}


