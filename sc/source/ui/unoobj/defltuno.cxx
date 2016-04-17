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

#include <editeng/memberids.hrc>
#include <svl/smplhint.hxx>
#include <svl/itemprop.hxx>
#include <svx/unomid.hxx>
#include <vcl/svapp.hxx>
#include <i18nlangtag/languagetag.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>

#include "scitems.hxx"
#include "defltuno.hxx"
#include "miscuno.hxx"
#include "docsh.hxx"
#include "docpool.hxx"
#include "unonames.hxx"
#include "docoptio.hxx"

#include <limits>

using namespace ::com::sun::star;

static const SfxItemPropertyMapEntry* lcl_GetDocDefaultsMap()
{
    static const SfxItemPropertyMapEntry aDocDefaultsMap_Impl[] =
    {
        {OUString(SC_UNONAME_CFCHARS),  ATTR_FONT,          cppu::UnoType<sal_Int16>::get(),        0, MID_FONT_CHAR_SET },
        {OUString(SC_UNO_CJK_CFCHARS),  ATTR_CJK_FONT,      cppu::UnoType<sal_Int16>::get(),        0, MID_FONT_CHAR_SET },
        {OUString(SC_UNO_CTL_CFCHARS),  ATTR_CTL_FONT,      cppu::UnoType<sal_Int16>::get(),        0, MID_FONT_CHAR_SET },
        {OUString(SC_UNONAME_CFFAMIL),  ATTR_FONT,          cppu::UnoType<sal_Int16>::get(),        0, MID_FONT_FAMILY },
        {OUString(SC_UNO_CJK_CFFAMIL),  ATTR_CJK_FONT,      cppu::UnoType<sal_Int16>::get(),        0, MID_FONT_FAMILY },
        {OUString(SC_UNO_CTL_CFFAMIL),  ATTR_CTL_FONT,      cppu::UnoType<sal_Int16>::get(),        0, MID_FONT_FAMILY },
        {OUString(SC_UNONAME_CFNAME),   ATTR_FONT,          cppu::UnoType<OUString>::get(),    0, MID_FONT_FAMILY_NAME },
        {OUString(SC_UNO_CJK_CFNAME),   ATTR_CJK_FONT,      cppu::UnoType<OUString>::get(),    0, MID_FONT_FAMILY_NAME },
        {OUString(SC_UNO_CTL_CFNAME),   ATTR_CTL_FONT,      cppu::UnoType<OUString>::get(),    0, MID_FONT_FAMILY_NAME },
        {OUString(SC_UNONAME_CFPITCH),  ATTR_FONT,          cppu::UnoType<sal_Int16>::get(),        0, MID_FONT_PITCH },
        {OUString(SC_UNO_CJK_CFPITCH),  ATTR_CJK_FONT,      cppu::UnoType<sal_Int16>::get(),        0, MID_FONT_PITCH },
        {OUString(SC_UNO_CTL_CFPITCH),  ATTR_CTL_FONT,      cppu::UnoType<sal_Int16>::get(),        0, MID_FONT_PITCH },
        {OUString(SC_UNONAME_CFSTYLE),  ATTR_FONT,          cppu::UnoType<OUString>::get(),    0, MID_FONT_STYLE_NAME },
        {OUString(SC_UNO_CJK_CFSTYLE),  ATTR_CJK_FONT,      cppu::UnoType<OUString>::get(),    0, MID_FONT_STYLE_NAME },
        {OUString(SC_UNO_CTL_CFSTYLE),  ATTR_CTL_FONT,      cppu::UnoType<OUString>::get(),    0, MID_FONT_STYLE_NAME },
        {OUString(SC_UNONAME_CLOCAL),   ATTR_FONT_LANGUAGE, cppu::UnoType<lang::Locale>::get(),     0, MID_LANG_LOCALE },
        {OUString(SC_UNO_CJK_CLOCAL),   ATTR_CJK_FONT_LANGUAGE, cppu::UnoType<lang::Locale>::get(), 0, MID_LANG_LOCALE },
        {OUString(SC_UNO_CTL_CLOCAL),   ATTR_CTL_FONT_LANGUAGE, cppu::UnoType<lang::Locale>::get(), 0, MID_LANG_LOCALE },
        {OUString(SC_UNO_STANDARDDEC),              0,      cppu::UnoType<sal_Int16>::get(),        0, 0 },
        {OUString(SC_UNO_TABSTOPDIS),               0,      cppu::UnoType<sal_Int32>::get(),        0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aDocDefaultsMap_Impl;
}

using sc::HMMToTwips;
using sc::TwipsToEvenHMM;

SC_SIMPLE_SERVICE_INFO( ScDocDefaultsObj, "ScDocDefaultsObj", "com.sun.star.sheet.Defaults" )

ScDocDefaultsObj::ScDocDefaultsObj(ScDocShell* pDocSh) :
    pDocShell( pDocSh ),
    aPropertyMap(lcl_GetDocDefaultsMap())
{
    pDocShell->GetDocument().AddUnoObject(*this);
}

ScDocDefaultsObj::~ScDocDefaultsObj()
{
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);
}

void ScDocDefaultsObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if ( pSimpleHint && pSimpleHint->GetId() == SFX_HINT_DYING )
    {
        pDocShell = nullptr;       // document gone
    }
}

void ScDocDefaultsObj::ItemsChanged()
{
    if (pDocShell)
    {
        //! if not in XML import, adjust row heights

        pDocShell->PostPaint(ScRange(0, 0, 0, MAXCOL, MAXROW, MAXTAB), PAINT_GRID);
    }
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScDocDefaultsObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef = new SfxItemPropertySetInfo(
                                                                        aPropertyMap );
    return aRef;
}

void SAL_CALL ScDocDefaultsObj::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if ( !pDocShell )
        throw uno::RuntimeException();

    const SfxItemPropertySimpleEntry* pEntry = aPropertyMap.getByName( aPropertyName );
    if ( !pEntry )
        throw beans::UnknownPropertyException();
    if(!pEntry->nWID)
    {
        if(aPropertyName ==SC_UNO_STANDARDDEC)
        {
            ScDocument& rDoc = pDocShell->GetDocument();
            ScDocOptions aDocOpt(rDoc.GetDocOptions());
            sal_Int16 nValue = 0;
            if (aValue >>= nValue)
            {
                aDocOpt.SetStdPrecision(static_cast<sal_uInt16> (nValue));
                rDoc.SetDocOptions(aDocOpt);
            }
        }
        else if (aPropertyName == SC_UNO_TABSTOPDIS)
        {
            ScDocument& rDoc = pDocShell->GetDocument();
            ScDocOptions aDocOpt(rDoc.GetDocOptions());
            sal_Int32 nValue = 0;
            if (aValue >>= nValue)
            {
                aDocOpt.SetTabDistance(static_cast<sal_uInt16>(HMMToTwips(nValue)));
                rDoc.SetDocOptions(aDocOpt);
            }
        }
    }
    else if ( pEntry->nWID == ATTR_FONT_LANGUAGE ||
              pEntry->nWID == ATTR_CJK_FONT_LANGUAGE ||
              pEntry->nWID == ATTR_CTL_FONT_LANGUAGE )
    {
        //  for getPropertyValue the PoolDefaults are sufficient,
        //  but setPropertyValue has to be handled differently

        lang::Locale aLocale;
        if ( aValue >>= aLocale )
        {
            LanguageType eNew;
            if (!aLocale.Language.isEmpty() || !aLocale.Country.isEmpty())
                eNew = LanguageTag::convertToLanguageType( aLocale, false);
            else
                eNew = LANGUAGE_NONE;

            ScDocument& rDoc = pDocShell->GetDocument();
            LanguageType eLatin, eCjk, eCtl;
            rDoc.GetLanguage( eLatin, eCjk, eCtl );

            if ( pEntry->nWID == ATTR_CJK_FONT_LANGUAGE )
                eCjk = eNew;
            else if ( pEntry->nWID == ATTR_CTL_FONT_LANGUAGE )
                eCtl = eNew;
            else
                eLatin = eNew;

            rDoc.SetLanguage( eLatin, eCjk, eCtl );
        }
    }
    else
    {
        ScDocumentPool* pPool = pDocShell->GetDocument().GetPool();
        SfxPoolItem* pNewItem = pPool->GetDefaultItem(pEntry->nWID).Clone();

        if( !pNewItem->PutValue( aValue, pEntry->nMemberId ) )
            throw lang::IllegalArgumentException();

        pPool->SetPoolDefaultItem( *pNewItem );
        delete pNewItem;    // copied in SetPoolDefaultItem

        ItemsChanged();
    }
}

uno::Any SAL_CALL ScDocDefaultsObj::getPropertyValue( const OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    //  use pool default if set

    SolarMutexGuard aGuard;

    if ( !pDocShell )
        throw uno::RuntimeException();

    uno::Any aRet;
    const SfxItemPropertySimpleEntry* pEntry = aPropertyMap.getByName( aPropertyName );
    if ( !pEntry )
        throw beans::UnknownPropertyException();

    if (!pEntry->nWID)
    {
        if(aPropertyName == SC_UNO_STANDARDDEC)
        {
            ScDocument& rDoc = pDocShell->GetDocument();
            const ScDocOptions& aDocOpt = rDoc.GetDocOptions();
            sal_uInt16 nPrec = aDocOpt.GetStdPrecision();
            // the max value of unsigned 16-bit integer is used as the flag
            // value for unlimited precision, c.f.
            // SvNumberFormatter::UNLIMITED_PRECISION.
            if (nPrec <= ::std::numeric_limits<sal_Int16>::max())
                aRet <<= static_cast<sal_Int16> (nPrec);
        }
        else if (aPropertyName == SC_UNO_TABSTOPDIS)
        {
            ScDocument& rDoc = pDocShell->GetDocument();
            const ScDocOptions& aDocOpt = rDoc.GetDocOptions();
            sal_Int32 nValue (TwipsToEvenHMM(aDocOpt.GetTabDistance()));
            aRet <<= nValue;
        }
    }
    else
    {
        ScDocumentPool* pPool = pDocShell->GetDocument().GetPool();
        const SfxPoolItem& rItem = pPool->GetDefaultItem( pEntry->nWID );
        rItem.QueryValue( aRet, pEntry->nMemberId );
    }
    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScDocDefaultsObj )

// XPropertyState

beans::PropertyState SAL_CALL ScDocDefaultsObj::getPropertyState( const OUString& aPropertyName )
                                throw(beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if ( !pDocShell )
        throw uno::RuntimeException();

    const SfxItemPropertySimpleEntry* pEntry = aPropertyMap.getByName( aPropertyName );
    if ( !pEntry )
        throw beans::UnknownPropertyException();

    beans::PropertyState eRet = beans::PropertyState_DEFAULT_VALUE;

    sal_uInt16 nWID = pEntry->nWID;
    if ( nWID == ATTR_FONT || nWID == ATTR_CJK_FONT || nWID == ATTR_CTL_FONT || !nWID )
    {
        //  static default for font is system-dependent,
        //  so font default is always treated as "direct value".

        eRet = beans::PropertyState_DIRECT_VALUE;
    }
    else
    {
        //  check if pool default is set

        ScDocumentPool* pPool = pDocShell->GetDocument().GetPool();
        if ( pPool->GetPoolDefaultItem( nWID ) != nullptr )
            eRet = beans::PropertyState_DIRECT_VALUE;
    }

    return eRet;
}

uno::Sequence<beans::PropertyState> SAL_CALL ScDocDefaultsObj::getPropertyStates(
                            const uno::Sequence<OUString>& aPropertyNames )
                    throw(beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    //  the simple way: call getPropertyState

    SolarMutexGuard aGuard;
    const OUString* pNames = aPropertyNames.getConstArray();
    uno::Sequence<beans::PropertyState> aRet(aPropertyNames.getLength());
    beans::PropertyState* pStates = aRet.getArray();
    for(sal_Int32 i = 0; i < aPropertyNames.getLength(); i++)
        pStates[i] = getPropertyState(pNames[i]);
    return aRet;
}

void SAL_CALL ScDocDefaultsObj::setPropertyToDefault( const OUString& aPropertyName )
                            throw(beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if ( !pDocShell )
        throw uno::RuntimeException();

    const SfxItemPropertySimpleEntry* pEntry = aPropertyMap.getByName( aPropertyName );
    if ( !pEntry )
        throw beans::UnknownPropertyException();

    if (pEntry->nWID)
    {
        ScDocumentPool* pPool = pDocShell->GetDocument().GetPool();
        pPool->ResetPoolDefaultItem( pEntry->nWID );

        ItemsChanged();
    }
}

uno::Any SAL_CALL ScDocDefaultsObj::getPropertyDefault( const OUString& aPropertyName )
                            throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                                    uno::RuntimeException, std::exception)
{
    //  always use static default

    SolarMutexGuard aGuard;

    if ( !pDocShell )
        throw uno::RuntimeException();

    const SfxItemPropertySimpleEntry* pEntry = aPropertyMap.getByName( aPropertyName );
    if ( !pEntry )
        throw beans::UnknownPropertyException();

    uno::Any aRet;
    if (pEntry->nWID)
    {
        ScDocumentPool* pPool = pDocShell->GetDocument().GetPool();
        const SfxPoolItem* pItem = pPool->GetItem2Default( pEntry->nWID );
        if (pItem)
            pItem->QueryValue( aRet, pEntry->nMemberId );
    }
    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
