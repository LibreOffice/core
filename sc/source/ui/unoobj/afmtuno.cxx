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

#include "scitems.hxx"
#include <editeng/memberids.hrc>
#include <svl/poolitem.hxx>
#include <vcl/svapp.hxx>
#include <svx/unomid.hxx>
#include "unowids.hxx"
#include <comphelper/servicehelper.hxx>
#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/table/CellVertJustify2.hpp>
#include <com/sun/star/table/ShadowLocation.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/CellContentType.hpp>
#include <com/sun/star/table/TableOrientation.hpp>
#include <com/sun/star/table/CellHoriJustify.hpp>
#include <com/sun/star/util/SortField.hpp>
#include <com/sun/star/util/SortFieldType.hpp>
#include <com/sun/star/table/CellOrientation.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/awt/SimpleFontMetric.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/CharSet.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/FontWidth.hpp>
#include <com/sun/star/awt/XFont.hpp>
#include <com/sun/star/awt/FontType.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontFamily.hpp>
#include <com/sun/star/awt/FontPitch.hpp>

#include "afmtuno.hxx"
#include "miscuno.hxx"
#include "autoform.hxx"
#include "scdll.hxx"
#include "unonames.hxx"
#include "cellsuno.hxx"

using namespace ::com::sun::star;

//  ein AutoFormat hat immer 16 Eintraege
#define SC_AF_FIELD_COUNT 16

//  AutoFormat-Map nur fuer PropertySetInfo, ohne Which-IDs

static const SfxItemPropertyMapEntry* lcl_GetAutoFormatMap()
{
    static const SfxItemPropertyMapEntry aAutoFormatMap_Impl[] =
    {
        { OUString(SC_UNONAME_INCBACK),  0,  cppu::UnoType<bool>::get(),    0, 0 },
        { OUString(SC_UNONAME_INCBORD),  0,  cppu::UnoType<bool>::get(),    0, 0 },
        { OUString(SC_UNONAME_INCFONT),  0,  cppu::UnoType<bool>::get(),    0, 0 },
        { OUString(SC_UNONAME_INCJUST),  0,  cppu::UnoType<bool>::get(),    0, 0 },
        { OUString(SC_UNONAME_INCNUM),   0,  cppu::UnoType<bool>::get(),    0, 0 },
        { OUString(SC_UNONAME_INCWIDTH), 0,  cppu::UnoType<bool>::get(),    0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aAutoFormatMap_Impl;
}

//! Zahlformat (String/Language) ??? (in XNumberFormat nur ReadOnly)
//! table::TableBorder ??!?

static const SfxItemPropertyMapEntry* lcl_GetAutoFieldMap()
{
    static const SfxItemPropertyMapEntry aAutoFieldMap_Impl[] =
    {
        {OUString(SC_UNONAME_CELLBACK), ATTR_BACKGROUND,        ::cppu::UnoType<sal_Int32>::get(),        0, MID_BACK_COLOR },
        {OUString(SC_UNONAME_CCOLOR),   ATTR_FONT_COLOR,        ::cppu::UnoType<sal_Int32>::get(),        0, 0 },
        {OUString(SC_UNONAME_COUTL),    ATTR_FONT_CONTOUR,      cppu::UnoType<bool>::get(),                    0, 0 },
        {OUString(SC_UNONAME_CCROSS),   ATTR_FONT_CROSSEDOUT,   cppu::UnoType<bool>::get(),                    0, MID_CROSSED_OUT },
        {OUString(SC_UNONAME_CFONT),    ATTR_FONT,              ::cppu::UnoType<sal_Int16>::get(),        0, MID_FONT_FAMILY },
        {OUString(SC_UNONAME_CFCHARS),  ATTR_FONT,              ::cppu::UnoType<sal_Int16>::get(),              0, MID_FONT_CHAR_SET },
        {OUString(SC_UNO_CJK_CFCHARS),  ATTR_CJK_FONT,          ::cppu::UnoType<sal_Int16>::get(),              0, MID_FONT_CHAR_SET },
        {OUString(SC_UNO_CTL_CFCHARS),  ATTR_CTL_FONT,          ::cppu::UnoType<sal_Int16>::get(),              0, MID_FONT_CHAR_SET },
        {OUString(SC_UNONAME_CFFAMIL),  ATTR_FONT,              ::cppu::UnoType<sal_Int16>::get(),              0, MID_FONT_FAMILY },
        {OUString(SC_UNO_CJK_CFFAMIL),  ATTR_CJK_FONT,          ::cppu::UnoType<sal_Int16>::get(),              0, MID_FONT_FAMILY },
        {OUString(SC_UNO_CTL_CFFAMIL),  ATTR_CTL_FONT,          ::cppu::UnoType<sal_Int16>::get(),              0, MID_FONT_FAMILY },
        {OUString(SC_UNONAME_CFNAME),   ATTR_FONT,              ::cppu::UnoType<OUString>::get(),          0, MID_FONT_FAMILY_NAME },
        {OUString(SC_UNO_CJK_CFNAME),   ATTR_CJK_FONT,          ::cppu::UnoType<OUString>::get(),          0, MID_FONT_FAMILY_NAME },
        {OUString(SC_UNO_CTL_CFNAME),   ATTR_CTL_FONT,          ::cppu::UnoType<OUString>::get(),          0, MID_FONT_FAMILY_NAME },
        {OUString(SC_UNONAME_CFPITCH),  ATTR_FONT,              ::cppu::UnoType<sal_Int16>::get(),              0, MID_FONT_PITCH },
        {OUString(SC_UNO_CJK_CFPITCH),  ATTR_CJK_FONT,          ::cppu::UnoType<sal_Int16>::get(),              0, MID_FONT_PITCH },
        {OUString(SC_UNO_CTL_CFPITCH),  ATTR_CTL_FONT,          ::cppu::UnoType<sal_Int16>::get(),              0, MID_FONT_PITCH },
        {OUString(SC_UNONAME_CFSTYLE),  ATTR_FONT,              ::cppu::UnoType<OUString>::get(),          0, MID_FONT_STYLE_NAME },
        {OUString(SC_UNO_CJK_CFSTYLE),  ATTR_CJK_FONT,          ::cppu::UnoType<OUString>::get(),          0, MID_FONT_STYLE_NAME },
        {OUString(SC_UNO_CTL_CFSTYLE),  ATTR_CTL_FONT,          ::cppu::UnoType<OUString>::get(),          0, MID_FONT_STYLE_NAME },
        {OUString(SC_UNONAME_CHEIGHT),  ATTR_FONT_HEIGHT,       ::cppu::UnoType<float>::get(),                  0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {OUString(SC_UNO_CJK_CHEIGHT),  ATTR_CJK_FONT_HEIGHT,   ::cppu::UnoType<float>::get(),                  0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {OUString(SC_UNO_CTL_CHEIGHT),  ATTR_CTL_FONT_HEIGHT,   ::cppu::UnoType<float>::get(),                  0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {OUString(SC_UNONAME_COVER),    ATTR_FONT_OVERLINE,     ::cppu::UnoType<sal_Int16>::get(),        0, MID_TL_STYLE },
        {OUString(SC_UNONAME_CPOST),    ATTR_FONT_POSTURE,      ::cppu::UnoType<awt::FontSlant>::get(),         0, MID_POSTURE },
        {OUString(SC_UNO_CJK_CPOST),    ATTR_CJK_FONT_POSTURE,  ::cppu::UnoType<awt::FontSlant>::get(),         0, MID_POSTURE },
        {OUString(SC_UNO_CTL_CPOST),    ATTR_CTL_FONT_POSTURE,  ::cppu::UnoType<awt::FontSlant>::get(),         0, MID_POSTURE },
        {OUString(SC_UNONAME_CSHADD),   ATTR_FONT_SHADOWED,     cppu::UnoType<bool>::get(),                    0, 0 },
        {OUString(SC_UNONAME_TBLBORD),  SC_WID_UNO_TBLBORD,     ::cppu::UnoType<table::TableBorder>::get(),     0, 0 | CONVERT_TWIPS },
        {OUString(SC_UNONAME_TBLBORD2),  SC_WID_UNO_TBLBORD2,     ::cppu::UnoType<table::TableBorder2>::get(),     0, 0 | CONVERT_TWIPS },
        {OUString(SC_UNONAME_CUNDER),   ATTR_FONT_UNDERLINE,    ::cppu::UnoType<sal_Int16>::get(),        0, MID_TL_STYLE },
        {OUString(SC_UNONAME_CWEIGHT),  ATTR_FONT_WEIGHT,       ::cppu::UnoType<float>::get(),                  0, MID_WEIGHT },
        {OUString(SC_UNO_CJK_CWEIGHT),  ATTR_CJK_FONT_WEIGHT,   ::cppu::UnoType<float>::get(),                  0, MID_WEIGHT },
        {OUString(SC_UNO_CTL_CWEIGHT),  ATTR_CTL_FONT_WEIGHT,   ::cppu::UnoType<float>::get(),                  0, MID_WEIGHT },
        {OUString(SC_UNONAME_CELLHJUS), ATTR_HOR_JUSTIFY,       ::cppu::UnoType<table::CellHoriJustify>::get(),   0, 0 },
        {OUString(SC_UNONAME_CELLHJUS_METHOD), ATTR_HOR_JUSTIFY_METHOD, ::cppu::UnoType<sal_Int32>::get(),   0, 0 },
        {OUString(SC_UNONAME_CELLTRAN), ATTR_BACKGROUND,        cppu::UnoType<bool>::get(),                    0, MID_GRAPHIC_TRANSPARENT },
        {OUString(SC_UNONAME_WRAP),     ATTR_LINEBREAK,         cppu::UnoType<bool>::get(),                    0, 0 },
        {OUString(SC_UNONAME_CELLORI),  ATTR_STACKED,           ::cppu::UnoType<table::CellOrientation>::get(),   0, 0 },
        {OUString(SC_UNONAME_PBMARGIN), ATTR_MARGIN,            ::cppu::UnoType<sal_Int32>::get(),        0, MID_MARGIN_LO_MARGIN | CONVERT_TWIPS },
        {OUString(SC_UNONAME_PLMARGIN), ATTR_MARGIN,            ::cppu::UnoType<sal_Int32>::get(),        0, MID_MARGIN_L_MARGIN  | CONVERT_TWIPS },
        {OUString(SC_UNONAME_PRMARGIN), ATTR_MARGIN,            ::cppu::UnoType<sal_Int32>::get(),        0, MID_MARGIN_R_MARGIN  | CONVERT_TWIPS },
        {OUString(SC_UNONAME_PTMARGIN), ATTR_MARGIN,            ::cppu::UnoType<sal_Int32>::get(),        0, MID_MARGIN_UP_MARGIN | CONVERT_TWIPS },
        {OUString(SC_UNONAME_ROTANG),   ATTR_ROTATE_VALUE,      ::cppu::UnoType<sal_Int32>::get(),        0, 0 },
        {OUString(SC_UNONAME_ROTREF),   ATTR_ROTATE_MODE,       ::cppu::UnoType<sal_Int32>::get(),   0, 0 },
        {OUString(SC_UNONAME_CELLVJUS), ATTR_VER_JUSTIFY,       ::cppu::UnoType<sal_Int32>::get(),   0, 0 },
        {OUString(SC_UNONAME_CELLVJUS_METHOD), ATTR_VER_JUSTIFY_METHOD, ::cppu::UnoType<sal_Int32>::get(),   0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aAutoFieldMap_Impl;
}

#define SCAUTOFORMATSOBJ_SERVICE    "com.sun.star.sheet.TableAutoFormats"

SC_SIMPLE_SERVICE_INFO( ScAutoFormatFieldObj, "ScAutoFormatFieldObj", "com.sun.star.sheet.TableAutoFormatField" )
SC_SIMPLE_SERVICE_INFO( ScAutoFormatObj, "ScAutoFormatObj", "com.sun.star.sheet.TableAutoFormat" )
SC_SIMPLE_SERVICE_INFO( ScAutoFormatsObj, "stardiv.StarCalc.ScAutoFormatsObj", SCAUTOFORMATSOBJ_SERVICE )

static bool lcl_FindAutoFormatIndex( const ScAutoFormat& rFormats, const OUString& rName, sal_uInt16& rOutIndex )
{
    ScAutoFormat::const_iterator itBeg = rFormats.begin(), itEnd = rFormats.end();
    for (ScAutoFormat::const_iterator it = itBeg; it != itEnd; ++it)
    {
        const ScAutoFormatData *const pEntry = it->second.get();
        const OUString& aEntryName = pEntry->GetName();
        if ( aEntryName.equals(rName) )
        {
            size_t nPos = std::distance(itBeg, it);
            rOutIndex = nPos;
            return true;
        }
    }
    return false;       // is nich
}

ScAutoFormatsObj::ScAutoFormatsObj()
{
    //! Dieses Objekt darf es nur einmal geben, und es muss an den Auto-Format-Daten
    //! bekannt sein, damit Aenderungen gebroadcasted werden koennen
}

ScAutoFormatsObj::~ScAutoFormatsObj()
{
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
ScAutoFormatsObj_get_implementation(css::uno::XComponentContext*, css::uno::Sequence<css::uno::Any> const &)
{
    SolarMutexGuard aGuard;
    ScDLL::Init();
    return cppu::acquire(new ScAutoFormatsObj);
}

// XTableAutoFormats

ScAutoFormatObj* ScAutoFormatsObj::GetObjectByIndex_Impl(sal_uInt16 nIndex)
{
    if (nIndex < ScGlobal::GetOrCreateAutoFormat()->size())
        return new ScAutoFormatObj(nIndex);

    return nullptr;    // falscher Index
}

ScAutoFormatObj* ScAutoFormatsObj::GetObjectByName_Impl(const OUString& aName)
{
    sal_uInt16 nIndex;
    if (lcl_FindAutoFormatIndex(
            *ScGlobal::GetOrCreateAutoFormat(), aName, nIndex ))
        return GetObjectByIndex_Impl(nIndex);
    return nullptr;
}

// container::XNameContainer

void SAL_CALL ScAutoFormatsObj::insertByName( const OUString& aName, const uno::Any& aElement )
                            throw (lang::IllegalArgumentException, container::ElementExistException,
                                   lang::WrappedTargetException, uno::RuntimeException,
                                   std::exception)
{
    SolarMutexGuard aGuard;
    bool bDone = false;
    //  Reflection muss nicht uno::XInterface sein, kann auch irgendein Interface sein...
    uno::Reference< uno::XInterface > xInterface(aElement, uno::UNO_QUERY);
    if ( xInterface.is() )
    {
        ScAutoFormatObj* pFormatObj = ScAutoFormatObj::getImplementation( xInterface );
        if ( pFormatObj && !pFormatObj->IsInserted() )  // noch nicht eingefuegt?
        {
            ScAutoFormat* pFormats = ScGlobal::GetOrCreateAutoFormat();

            sal_uInt16 nDummy;
            if (!lcl_FindAutoFormatIndex( *pFormats, aName, nDummy ))
            {
                ScAutoFormatData* pNew = new ScAutoFormatData();
                pNew->SetName( aName );

                if (pFormats->insert(pNew))
                {
                    //! Notify fuer andere Objekte
                    pFormats->Save();   // sofort speichern

                    sal_uInt16 nNewIndex;
                    if (lcl_FindAutoFormatIndex( *pFormats, aName, nNewIndex ))
                    {
                        pFormatObj->InitFormat( nNewIndex );    // kann jetzt benutzt werden
                        bDone = true;
                    }
                }
                else
                {
                    OSL_FAIL("AutoFormat konnte nicht eingefuegt werden");
                    throw uno::RuntimeException();
                }
            }
            else
            {
                throw container::ElementExistException();
            }
        }
    }

    if (!bDone)
    {
        //  other errors are handled above
        throw lang::IllegalArgumentException();
    }
}

void SAL_CALL ScAutoFormatsObj::replaceByName( const OUString& aName, const uno::Any& aElement )
                            throw(lang::IllegalArgumentException, container::NoSuchElementException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    //! zusammenfassen?
    removeByName( aName );
    insertByName( aName, aElement );
}

void SAL_CALL ScAutoFormatsObj::removeByName( const OUString& aName )
                                throw(container::NoSuchElementException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    ScAutoFormat* pFormats = ScGlobal::GetOrCreateAutoFormat();

    ScAutoFormat::iterator it = pFormats->find(aName);
    if (it != pFormats->end())
    {
        pFormats->erase(it);

        //! Notify fuer andere Objekte
        pFormats->Save();   // sofort speichern
    }
    else
    {
        throw container::NoSuchElementException();
    }
}

// container::XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScAutoFormatsObj::createEnumeration()
                                                    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, OUString("com.sun.star.sheet.TableAutoFormatEnumeration"));
}

// container::XIndexAccess

sal_Int32 SAL_CALL ScAutoFormatsObj::getCount() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return ScGlobal::GetOrCreateAutoFormat()->size();
}

uno::Any SAL_CALL ScAutoFormatsObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference< container::XNamed >  xFormat(GetObjectByIndex_Impl((sal_uInt16)nIndex));
    if (!xFormat.is())
        throw lang::IndexOutOfBoundsException();
    return uno::makeAny(xFormat);
}

uno::Type SAL_CALL ScAutoFormatsObj::getElementType() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return cppu::UnoType<container::XNamed>::get();    // muss zu getByIndex passen
}

sal_Bool SAL_CALL ScAutoFormatsObj::hasElements() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

// container::XNameAccess

uno::Any SAL_CALL ScAutoFormatsObj::getByName( const OUString& aName )
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference< container::XNamed >  xFormat(GetObjectByName_Impl(aName));
    if (!xFormat.is())
        throw container::NoSuchElementException();
    return uno::makeAny(xFormat);
}

uno::Sequence<OUString> SAL_CALL ScAutoFormatsObj::getElementNames()
                                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    ScAutoFormat* pFormats = ScGlobal::GetOrCreateAutoFormat();
    uno::Sequence<OUString> aSeq(pFormats->size());
    OUString* pAry = aSeq.getArray();
    ScAutoFormat::const_iterator it = pFormats->begin(), itEnd = pFormats->end();
    for (size_t i = 0; it != itEnd; ++it, ++i)
    {
        pAry[i] = it->second->GetName();
    }
    return aSeq;
}

sal_Bool SAL_CALL ScAutoFormatsObj::hasByName( const OUString& aName )
                                        throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    sal_uInt16 nDummy;
    return lcl_FindAutoFormatIndex(
        *ScGlobal::GetOrCreateAutoFormat(), aName, nDummy );
}

ScAutoFormatObj::ScAutoFormatObj(sal_uInt16 nIndex) :
    aPropSet( lcl_GetAutoFormatMap() ),
    nFormatIndex( nIndex )
{
    //! Listening !!!
}

ScAutoFormatObj::~ScAutoFormatObj()
{
    //  Wenn ein AutoFormat-Objekt losgelassen wird, werden eventuelle Aenderungen
    //  gespeichert, damit sie z.B. im Writer sichtbar sind

    if (IsInserted())
    {
        ScAutoFormat* pFormats = ScGlobal::GetAutoFormat();
        if ( pFormats && pFormats->IsSaveLater() )
            pFormats->Save();

        // Save() setzt SaveLater Flag zurueck
    }
}

void ScAutoFormatObj::InitFormat( sal_uInt16 nNewIndex )
{
    OSL_ENSURE( nFormatIndex == SC_AFMTOBJ_INVALID, "ScAutoFormatObj::InitFormat mehrfach" );
    nFormatIndex = nNewIndex;
    //! Listening !!!
}

// XUnoTunnel

sal_Int64 SAL_CALL ScAutoFormatObj::getSomething(
                const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException, std::exception)
{
    if ( rId.getLength() == 16 &&
          0 == memcmp( getUnoTunnelId().getConstArray(),
                                    rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
    }
    return 0;
}

namespace
{
    class theScAutoFormatObjUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theScAutoFormatObjUnoTunnelId> {};
}

const uno::Sequence<sal_Int8>& ScAutoFormatObj::getUnoTunnelId()
{
    return theScAutoFormatObjUnoTunnelId::get().getSeq();
}

ScAutoFormatObj* ScAutoFormatObj::getImplementation(const uno::Reference<uno::XInterface>& rObj)
{
    ScAutoFormatObj* pRet = nullptr;
    uno::Reference<lang::XUnoTunnel> xUT(rObj, uno::UNO_QUERY);
    if (xUT.is())
        pRet = reinterpret_cast<ScAutoFormatObj*>(sal::static_int_cast<sal_IntPtr>(xUT->getSomething(getUnoTunnelId())));
    return pRet;
}

void ScAutoFormatObj::Notify( SfxBroadcaster& /* rBC */, const SfxHint& /* rHint */ )
{
    //  spaeter...
}

// XTableAutoFormat

ScAutoFormatFieldObj* ScAutoFormatObj::GetObjectByIndex_Impl(sal_uInt16 nIndex)
{
    if ( IsInserted() && nIndex < SC_AF_FIELD_COUNT )
        return new ScAutoFormatFieldObj( nFormatIndex, nIndex );

    return nullptr;
}

// container::XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScAutoFormatObj::createEnumeration()
                                                    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, OUString("com.sun.star.sheet.TableAutoFormatEnumeration"));
}

// container::XIndexAccess

sal_Int32 SAL_CALL ScAutoFormatObj::getCount() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (IsInserted())
        return SC_AF_FIELD_COUNT;   // immer 16 Elemente
    else
        return 0;
}

uno::Any SAL_CALL ScAutoFormatObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if ( nIndex < 0 || nIndex >= getCount() )
        throw lang::IndexOutOfBoundsException();

    if (IsInserted())
        return uno::makeAny(uno::Reference< beans::XPropertySet >(GetObjectByIndex_Impl((sal_uInt16)nIndex)));
    return uno::Any();
}

uno::Type SAL_CALL ScAutoFormatObj::getElementType() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return cppu::UnoType<beans::XPropertySet>::get();  // muss zu getByIndex passen
}

sal_Bool SAL_CALL ScAutoFormatObj::hasElements() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

// container::XNamed

OUString SAL_CALL ScAutoFormatObj::getName() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    ScAutoFormat* pFormats = ScGlobal::GetOrCreateAutoFormat();
    if (IsInserted() && nFormatIndex < pFormats->size())
        return pFormats->findByIndex(nFormatIndex)->GetName();

    return OUString();
}

void SAL_CALL ScAutoFormatObj::setName( const OUString& aNewName )
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    ScAutoFormat* pFormats = ScGlobal::GetOrCreateAutoFormat();

    sal_uInt16 nDummy;
    if (IsInserted() && nFormatIndex < pFormats->size() &&
        !lcl_FindAutoFormatIndex( *pFormats, aNewName, nDummy ))
    {
        ScAutoFormat::iterator it = pFormats->begin();
        std::advance(it, nFormatIndex);
        ScAutoFormatData *const pData = it->second.get();
        OSL_ENSURE(pData,"AutoFormat Daten nicht da");

        ScAutoFormatData* pNew = new ScAutoFormatData(*pData);
        pNew->SetName( aNewName );

        pFormats->erase(it);
        if (pFormats->insert(pNew))
        {
            it = pFormats->find(pNew);
            ScAutoFormat::iterator itBeg = pFormats->begin();
            nFormatIndex = std::distance(itBeg, it);

            //! Notify fuer andere Objekte
            pFormats->SetSaveLater(true);
        }
        else
        {
            OSL_FAIL("AutoFormat konnte nicht eingefuegt werden");
            nFormatIndex = 0;       //! alter Index ist ungueltig
        }
    }
    else
    {
        //  not inserted or name exists
        throw uno::RuntimeException();
    }
}

// beans::XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScAutoFormatObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    static uno::Reference< beans::XPropertySetInfo > aRef(new SfxItemPropertySetInfo( aPropSet.getPropertyMap() ));
    return aRef;
}

void SAL_CALL ScAutoFormatObj::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    ScAutoFormat* pFormats = ScGlobal::GetOrCreateAutoFormat();
    if (IsInserted() && nFormatIndex < pFormats->size())
    {
        ScAutoFormatData* pData = pFormats->findByIndex(nFormatIndex);
        OSL_ENSURE(pData,"AutoFormat Daten nicht da");

        bool bBool;
        if (aPropertyName == SC_UNONAME_INCBACK && (aValue >>= bBool))
            pData->SetIncludeBackground( bBool );
        else if (aPropertyName == SC_UNONAME_INCBORD && (aValue >>= bBool))
            pData->SetIncludeFrame( bBool );
        else if (aPropertyName == SC_UNONAME_INCFONT && (aValue >>= bBool))
            pData->SetIncludeFont( bBool );
        else if (aPropertyName == SC_UNONAME_INCJUST && (aValue >>= bBool))
            pData->SetIncludeJustify( bBool );
        else if (aPropertyName == SC_UNONAME_INCNUM && (aValue >>= bBool))
            pData->SetIncludeValueFormat( bBool );
        else if (aPropertyName == SC_UNONAME_INCWIDTH && (aValue >>= bBool))
            pData->SetIncludeWidthHeight( bBool );

        // else Fehler

        //! Notify fuer andere Objekte
        pFormats->SetSaveLater(true);
    }
}

uno::Any SAL_CALL ScAutoFormatObj::getPropertyValue( const OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Any aAny;

    ScAutoFormat* pFormats = ScGlobal::GetOrCreateAutoFormat();
    if (IsInserted() && nFormatIndex < pFormats->size())
    {
        ScAutoFormatData* pData = pFormats->findByIndex(nFormatIndex);
        OSL_ENSURE(pData,"AutoFormat Daten nicht da");

        bool bValue;
        bool bError = false;

        if (aPropertyName == SC_UNONAME_INCBACK)
            bValue = pData->GetIncludeBackground();
        else if (aPropertyName == SC_UNONAME_INCBORD)
            bValue = pData->GetIncludeFrame();
        else if (aPropertyName == SC_UNONAME_INCFONT)
            bValue = pData->GetIncludeFont();
        else if (aPropertyName == SC_UNONAME_INCJUST)
            bValue = pData->GetIncludeJustify();
        else if (aPropertyName == SC_UNONAME_INCNUM)
            bValue = pData->GetIncludeValueFormat();
        else if (aPropertyName == SC_UNONAME_INCWIDTH)
            bValue = pData->GetIncludeWidthHeight();
        else
            bError = true;      // unbekannte Property

        if (!bError)
            aAny <<= bValue;
    }

    return aAny;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScAutoFormatObj )

ScAutoFormatFieldObj::ScAutoFormatFieldObj(sal_uInt16 nFormat, sal_uInt16 nField) :
    aPropSet( lcl_GetAutoFieldMap() ),
    nFormatIndex( nFormat ),
    nFieldIndex( nField )
{
    //! Listening !!!
}

ScAutoFormatFieldObj::~ScAutoFormatFieldObj()
{
}

void ScAutoFormatFieldObj::Notify( SfxBroadcaster& /* rBC */, const SfxHint& /* rHint */ )
{
    //  spaeter...
}

// beans::XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScAutoFormatFieldObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    static uno::Reference< beans::XPropertySetInfo > aRef(new SfxItemPropertySetInfo( aPropSet.getPropertyMap() ));
    return aRef;
}

void SAL_CALL ScAutoFormatFieldObj::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    ScAutoFormat* pFormats = ScGlobal::GetOrCreateAutoFormat();
    const SfxItemPropertySimpleEntry* pEntry =
            aPropSet.getPropertyMap().getByName( aPropertyName );

    if ( pEntry && pEntry->nWID && nFormatIndex < pFormats->size() )
    {
        ScAutoFormatData* pData = pFormats->findByIndex(nFormatIndex);

        if ( IsScItemWid( pEntry->nWID ) )
        {
            if( const SfxPoolItem* pItem = pData->GetItem( nFieldIndex, pEntry->nWID ) )
            {
                bool bDone = false;

                switch( pEntry->nWID )
                {
                    case ATTR_STACKED:
                    {
                        table::CellOrientation eOrient;
                        if( aValue >>= eOrient )
                        {
                            switch( eOrient )
                            {
                                case table::CellOrientation_STANDARD:
                                    pData->PutItem( nFieldIndex, SfxBoolItem( ATTR_STACKED, false ) );
                                break;
                                case table::CellOrientation_TOPBOTTOM:
                                    pData->PutItem( nFieldIndex, SfxBoolItem( ATTR_STACKED, false ) );
                                    pData->PutItem( nFieldIndex, SfxInt32Item( ATTR_ROTATE_VALUE, 27000 ) );
                                break;
                                case table::CellOrientation_BOTTOMTOP:
                                    pData->PutItem( nFieldIndex, SfxBoolItem( ATTR_STACKED, false ) );
                                    pData->PutItem( nFieldIndex, SfxInt32Item( ATTR_ROTATE_VALUE, 9000 ) );
                                break;
                                case table::CellOrientation_STACKED:
                                    pData->PutItem( nFieldIndex, SfxBoolItem( ATTR_STACKED, true ) );
                                break;
                                default:
                                {
                                    // added to avoid warnings
                                }
                            }
                            bDone = true;
                        }
                    }
                    break;
                    default:
                        SfxPoolItem* pNewItem = pItem->Clone();
                        bDone = pNewItem->PutValue( aValue, pEntry->nMemberId );
                        if (bDone)
                            pData->PutItem( nFieldIndex, *pNewItem );
                        delete pNewItem;
                }

                if (bDone)
                    //! Notify fuer andere Objekte?
                    pFormats->SetSaveLater(true);
            }
        }
        else
        {
            switch (pEntry->nWID)
            {
                case SC_WID_UNO_TBLBORD:
                    {
                        table::TableBorder aBorder;
                        if ( aValue >>= aBorder )   // empty = nothing to do
                        {
                            SvxBoxItem aOuter(ATTR_BORDER);
                            SvxBoxInfoItem aInner(ATTR_BORDER_INNER);
                            ScHelperFunctions::FillBoxItems( aOuter, aInner, aBorder );
                            pData->PutItem( nFieldIndex, aOuter );

                            //! Notify for other objects?
                            pFormats->SetSaveLater(true);
                        }
                    }
                    break;
                case SC_WID_UNO_TBLBORD2:
                    {
                        table::TableBorder2 aBorder2;
                        if ( aValue >>= aBorder2 )   // empty = nothing to do
                        {
                            SvxBoxItem aOuter(ATTR_BORDER);
                            SvxBoxInfoItem aInner(ATTR_BORDER_INNER);
                            ScHelperFunctions::FillBoxItems( aOuter, aInner, aBorder2 );
                            pData->PutItem( nFieldIndex, aOuter );

                            //! Notify for other objects?
                            pFormats->SetSaveLater(true);
                        }
                    }
                    break;
            }
        }
    }
}

uno::Any SAL_CALL ScAutoFormatFieldObj::getPropertyValue( const OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Any aVal;

    ScAutoFormat* pFormats = ScGlobal::GetOrCreateAutoFormat();
    const SfxItemPropertySimpleEntry* pEntry =
            aPropSet.getPropertyMap().getByName( aPropertyName );

    if ( pEntry && pEntry->nWID && nFormatIndex < pFormats->size() )
    {
        const ScAutoFormatData* pData = pFormats->findByIndex(nFormatIndex);

        if ( IsScItemWid( pEntry->nWID ) )
        {
            if( const SfxPoolItem* pItem = pData->GetItem( nFieldIndex, pEntry->nWID ) )
            {
                switch( pEntry->nWID )
                {
                    case ATTR_STACKED:
                    {
                        const SfxInt32Item* pRotItem = static_cast<const SfxInt32Item*>(pData->GetItem( nFieldIndex, ATTR_ROTATE_VALUE ));
                        sal_Int32 nRot = pRotItem ? pRotItem->GetValue() : 0;
                        bool bStacked = static_cast<const SfxBoolItem*>(pItem)->GetValue();
                        SvxOrientationItem( nRot, bStacked, 0 ).QueryValue( aVal );
                    }
                    break;
                    default:
                        pItem->QueryValue( aVal, pEntry->nMemberId );
                }
            }
        }
        else
        {
            switch (pEntry->nWID)
            {
                case SC_WID_UNO_TBLBORD:
                case SC_WID_UNO_TBLBORD2:
                    {
                        const SfxPoolItem* pItem = pData->GetItem(nFieldIndex, ATTR_BORDER);
                        if (pItem)
                        {
                            SvxBoxItem aOuter(*(static_cast<const SvxBoxItem*>(pItem)));
                            SvxBoxInfoItem aInner(ATTR_BORDER_INNER);

                            if (pEntry->nWID == SC_WID_UNO_TBLBORD2)
                                ScHelperFunctions::AssignTableBorder2ToAny( aVal, aOuter, aInner);
                            else
                                ScHelperFunctions::AssignTableBorderToAny( aVal, aOuter, aInner);
                        }
                    }
                    break;
            }
        }
    }

    return aVal;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScAutoFormatFieldObj )

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
