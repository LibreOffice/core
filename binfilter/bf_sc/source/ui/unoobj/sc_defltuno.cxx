/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <bf_svtools/itemprop.hxx>
#include <i18npool/mslangid.hxx>


#include "scitems.hxx"
#include "defltuno.hxx"
#include "miscuno.hxx"
#include "docsh.hxx"
#include "docpool.hxx"
#include <vcl/svapp.hxx>
#include "unonames.hxx"
#include "docoptio.hxx"
namespace binfilter {

using namespace ::com::sun::star;

//------------------------------------------------------------------------

const SfxItemPropertyMap* lcl_GetDocDefaultsMap()
{
    static SfxItemPropertyMap aDocDefaultsMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_CFCHARS),	ATTR_FONT,			&getCppuType((sal_Int16*)0),		0, MID_FONT_CHAR_SET },
        {MAP_CHAR_LEN(SC_UNO_CJK_CFCHARS),	ATTR_CJK_FONT,		&getCppuType((sal_Int16*)0),		0, MID_FONT_CHAR_SET },
        {MAP_CHAR_LEN(SC_UNO_CTL_CFCHARS),	ATTR_CTL_FONT,		&getCppuType((sal_Int16*)0),		0, MID_FONT_CHAR_SET },
        {MAP_CHAR_LEN(SC_UNONAME_CFFAMIL),	ATTR_FONT,			&getCppuType((sal_Int16*)0),		0, MID_FONT_FAMILY },
        {MAP_CHAR_LEN(SC_UNO_CJK_CFFAMIL),	ATTR_CJK_FONT,		&getCppuType((sal_Int16*)0),		0, MID_FONT_FAMILY },
        {MAP_CHAR_LEN(SC_UNO_CTL_CFFAMIL),	ATTR_CTL_FONT,		&getCppuType((sal_Int16*)0),		0, MID_FONT_FAMILY },
        {MAP_CHAR_LEN(SC_UNONAME_CFNAME),	ATTR_FONT,			&getCppuType((::rtl::OUString*)0),	0, MID_FONT_FAMILY_NAME },
        {MAP_CHAR_LEN(SC_UNO_CJK_CFNAME),	ATTR_CJK_FONT,		&getCppuType((::rtl::OUString*)0),	0, MID_FONT_FAMILY_NAME },
        {MAP_CHAR_LEN(SC_UNO_CTL_CFNAME),	ATTR_CTL_FONT,		&getCppuType((::rtl::OUString*)0),	0, MID_FONT_FAMILY_NAME },
        {MAP_CHAR_LEN(SC_UNONAME_CFPITCH),	ATTR_FONT,			&getCppuType((sal_Int16*)0),		0, MID_FONT_PITCH },
        {MAP_CHAR_LEN(SC_UNO_CJK_CFPITCH),	ATTR_CJK_FONT,		&getCppuType((sal_Int16*)0),		0, MID_FONT_PITCH },
        {MAP_CHAR_LEN(SC_UNO_CTL_CFPITCH),	ATTR_CTL_FONT,		&getCppuType((sal_Int16*)0),		0, MID_FONT_PITCH },
        {MAP_CHAR_LEN(SC_UNONAME_CFSTYLE),	ATTR_FONT,			&getCppuType((::rtl::OUString*)0),	0, MID_FONT_STYLE_NAME },
        {MAP_CHAR_LEN(SC_UNO_CJK_CFSTYLE),	ATTR_CJK_FONT,		&getCppuType((::rtl::OUString*)0),	0, MID_FONT_STYLE_NAME },
        {MAP_CHAR_LEN(SC_UNO_CTL_CFSTYLE),	ATTR_CTL_FONT,		&getCppuType((::rtl::OUString*)0),	0, MID_FONT_STYLE_NAME },
        {MAP_CHAR_LEN(SC_UNONAME_CLOCAL),	ATTR_FONT_LANGUAGE,	&getCppuType((lang::Locale*)0),		0, MID_LANG_LOCALE },
        {MAP_CHAR_LEN(SC_UNO_CJK_CLOCAL),	ATTR_CJK_FONT_LANGUAGE,	&getCppuType((lang::Locale*)0),	0, MID_LANG_LOCALE },
        {MAP_CHAR_LEN(SC_UNO_CTL_CLOCAL),	ATTR_CTL_FONT_LANGUAGE,	&getCppuType((lang::Locale*)0),	0, MID_LANG_LOCALE },
        {MAP_CHAR_LEN(SC_UNO_STANDARDDEC),				0,      &getCppuType((sal_Int16*)0),		0, 0 },
        {MAP_CHAR_LEN(SC_UNO_TABSTOPDIS),				0,		&getCppuType((sal_Int32*)0),		0, 0 },
        {0,0,0,0}
    };
    return aDocDefaultsMap_Impl;
}

inline long TwipsToHMM(long nTwips)	{ return (nTwips * 127 + 36) / 72; }
inline long HMMToTwips(long nHMM)	{ return (nHMM * 72 + 63) / 127; }
inline long TwipsToEvenHMM(long nTwips)	{ return ( (nTwips * 127 + 72) / 144 ) * 2; }

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
        pDocShell = NULL;		// document gone
    }
}

void ScDocDefaultsObj::ItemsChanged()
{
    if (pDocShell)
    {
        //!	if not in XML import, adjust row heights

        pDocShell->PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_GRID );
    }
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScDocDefaultsObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( lcl_GetDocDefaultsMap() );
    return aRef;
}

void SAL_CALL ScDocDefaultsObj::setPropertyValue(
                        const ::rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if ( !pDocShell )
        throw uno::RuntimeException();

    const SfxItemPropertyMap* pMap =
            SfxItemPropertyMap::GetByName( lcl_GetDocDefaultsMap(), aPropertyName );
    if ( !pMap )
        throw beans::UnknownPropertyException();
    if(!pMap->nWID)
    {
        if(aPropertyName.compareToAscii(SC_UNO_STANDARDDEC) == 0)
        {
            ScDocument* pDoc = pDocShell->GetDocument();
            if (pDoc)
            {
                ScDocOptions aDocOpt(pDoc->GetDocOptions());
                sal_Int16 nValue;
                if (aValue >>= nValue)
                {
                    aDocOpt.SetStdPrecision(static_cast<sal_uInt8> (nValue));
                    pDoc->SetDocOptions(aDocOpt);
                }
            }
            else
                throw uno::RuntimeException();
        }
        else if (aPropertyName.compareToAscii(SC_UNO_TABSTOPDIS) == 0)
        {
            ScDocument* pDoc = pDocShell->GetDocument();
            if (pDoc)
            {
                ScDocOptions aDocOpt(pDoc->GetDocOptions());
                sal_Int32 nValue;
                if (aValue >>= nValue)
                {
                    aDocOpt.SetTabDistance(static_cast<sal_uInt16>(HMMToTwips(nValue)));
                    pDoc->SetDocOptions(aDocOpt);
                }
            }
            else
                throw uno::RuntimeException();
        }
    }
    else if ( pMap->nWID == ATTR_FONT_LANGUAGE ||
              pMap->nWID == ATTR_CJK_FONT_LANGUAGE ||
              pMap->nWID == ATTR_CTL_FONT_LANGUAGE )
    {
        //	for getPropertyValue the PoolDefaults are sufficient,
        //	but setPropertyValue has to be handled differently

        lang::Locale aLocale;
        if ( aValue >>= aLocale )
        {
            LanguageType eNew;
            if (aLocale.Language.getLength() || aLocale.Country.getLength())
                eNew = MsLangId::convertIsoNamesToLanguage( aLocale.Language, aLocale.Country );
            else
                eNew = LANGUAGE_NONE;

            ScDocument* pDoc = pDocShell->GetDocument();
            LanguageType eLatin, eCjk, eCtl;
            pDoc->GetLanguage( eLatin, eCjk, eCtl );

            if ( pMap->nWID == ATTR_CJK_FONT_LANGUAGE )
                eCjk = eNew;
            else if ( pMap->nWID == ATTR_CTL_FONT_LANGUAGE )
                eCtl = eNew;
            else
                eLatin = eNew;

            pDoc->SetLanguage( eLatin, eCjk, eCtl );
        }
    }
    else
    {
        ScDocumentPool* pPool = pDocShell->GetDocument()->GetPool();
        SfxPoolItem* pNewItem = pPool->GetDefaultItem(pMap->nWID).Clone();

        if( !pNewItem->PutValue( aValue, pMap->nMemberId ) )
            throw lang::IllegalArgumentException();

        pPool->SetPoolDefaultItem( *pNewItem );
        delete pNewItem;	// copied in SetPoolDefaultItem

        ItemsChanged();
    }
}

uno::Any SAL_CALL ScDocDefaultsObj::getPropertyValue( const ::rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    //	use pool default if set

    SolarMutexGuard aGuard;

    if ( !pDocShell )
        throw uno::RuntimeException();

    uno::Any aRet;
    const SfxItemPropertyMap* pMap =
            SfxItemPropertyMap::GetByName( lcl_GetDocDefaultsMap(), aPropertyName );
    if ( !pMap )
        throw beans::UnknownPropertyException();

    if (!pMap->nWID)
    {
        if(aPropertyName.compareToAscii(SC_UNO_STANDARDDEC) == 0)
        {
            ScDocument* pDoc = pDocShell->GetDocument();
            if (pDoc)
            {
                const ScDocOptions& aDocOpt = pDoc->GetDocOptions();
                aRet <<= static_cast<sal_Int16> (aDocOpt.GetStdPrecision());
            }
            else
                throw uno::RuntimeException();
        }
        else if (aPropertyName.compareToAscii(SC_UNO_TABSTOPDIS) == 0)
        {
            ScDocument* pDoc = pDocShell->GetDocument();
            if (pDoc)
            {
                const ScDocOptions& aDocOpt = pDoc->GetDocOptions();
                sal_Int32 nValue (TwipsToEvenHMM(aDocOpt.GetTabDistance()));
                aRet <<= nValue;
            }
            else
                throw uno::RuntimeException();
        }
    }
    else
    {
        ScDocumentPool* pPool = pDocShell->GetDocument()->GetPool();
        const SfxPoolItem& rItem = pPool->GetDefaultItem( pMap->nWID );
        rItem.QueryValue( aRet, pMap->nMemberId );
    }
    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScDocDefaultsObj )

// XPropertyState

beans::PropertyState SAL_CALL ScDocDefaultsObj::getPropertyState( const ::rtl::OUString& aPropertyName )
                                throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if ( !pDocShell )
        throw uno::RuntimeException();

    const SfxItemPropertyMap* pMap =
            SfxItemPropertyMap::GetByName( lcl_GetDocDefaultsMap(), aPropertyName );
    if ( !pMap )
        throw beans::UnknownPropertyException();

    beans::PropertyState eRet = beans::PropertyState_DEFAULT_VALUE;

    USHORT nWID = pMap->nWID;
    if ( nWID == ATTR_FONT || nWID == ATTR_CJK_FONT || nWID == ATTR_CTL_FONT || !nWID )
    {
        //	static default for font is system-dependent,
        //	so font default is always treated as "direct value".

        eRet = beans::PropertyState_DIRECT_VALUE;
    }
    else
    {
        //	check if pool default is set

        ScDocumentPool* pPool = pDocShell->GetDocument()->GetPool();
        if ( pPool->GetPoolDefaultItem( nWID ) != NULL )
            eRet = beans::PropertyState_DIRECT_VALUE;
    }

    return eRet;
}

uno::Sequence<beans::PropertyState> SAL_CALL ScDocDefaultsObj::getPropertyStates(
                            const uno::Sequence< ::rtl::OUString>& aPropertyNames )
                    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    //	the simple way: call getPropertyState

    SolarMutexGuard aGuard;
    const ::rtl::OUString* pNames = aPropertyNames.getConstArray();
    uno::Sequence<beans::PropertyState> aRet(aPropertyNames.getLength());
    beans::PropertyState* pStates = aRet.getArray();
    for(sal_Int32 i = 0; i < aPropertyNames.getLength(); i++)
        pStates[i] = getPropertyState(pNames[i]);
    return aRet;
}

void SAL_CALL ScDocDefaultsObj::setPropertyToDefault( const ::rtl::OUString& aPropertyName )
                            throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if ( !pDocShell )
        throw uno::RuntimeException();

    const SfxItemPropertyMap* pMap =
            SfxItemPropertyMap::GetByName( lcl_GetDocDefaultsMap(), aPropertyName );
    if ( !pMap )
        throw beans::UnknownPropertyException();

    if (pMap->nWID)
    {
        ScDocumentPool* pPool = pDocShell->GetDocument()->GetPool();
        pPool->ResetPoolDefaultItem( pMap->nWID );

        ItemsChanged();
    }
}

uno::Any SAL_CALL ScDocDefaultsObj::getPropertyDefault( const ::rtl::OUString& aPropertyName )
                            throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                                    uno::RuntimeException)
{
    //	always use static default

    SolarMutexGuard aGuard;

    if ( !pDocShell )
        throw uno::RuntimeException();

    const SfxItemPropertyMap* pMap =
            SfxItemPropertyMap::GetByName( lcl_GetDocDefaultsMap(), aPropertyName );
    if ( !pMap )
        throw beans::UnknownPropertyException();

    uno::Any aRet;
    if (pMap->nWID)
    {
        ScDocumentPool* pPool = pDocShell->GetDocument()->GetPool();
        const SfxPoolItem* pItem = pPool->GetItem( pMap->nWID, SFX_ITEMS_STATICDEFAULT );
        if (pItem)
            pItem->QueryValue( aRet, pMap->nMemberId );
    }
    return aRet;
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
