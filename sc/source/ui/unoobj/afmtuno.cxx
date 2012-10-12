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

#include "scitems.hxx"
#include <editeng/memberids.hrc>
#include <tools/shl.hxx>
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

//------------------------------------------------------------------------

//  ein AutoFormat hat immer 16 Eintraege
#define SC_AF_FIELD_COUNT 16

//------------------------------------------------------------------------

//  AutoFormat-Map nur fuer PropertySetInfo, ohne Which-IDs

static const SfxItemPropertyMapEntry* lcl_GetAutoFormatMap()
{
    static SfxItemPropertyMapEntry aAutoFormatMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_INCBACK),  0,  &::getBooleanCppuType(),    0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_INCBORD),  0,  &::getBooleanCppuType(),    0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_INCFONT),  0,  &::getBooleanCppuType(),    0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_INCJUST),  0,  &::getBooleanCppuType(),    0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_INCNUM),   0,  &::getBooleanCppuType(),    0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_INCWIDTH), 0,  &::getBooleanCppuType(),    0, 0 },
        {0,0,0,0,0,0}
    };
    return aAutoFormatMap_Impl;
}

//! Zahlformat (String/Language) ??? (in XNumberFormat nur ReadOnly)
//! table::TableBorder ??!?

static const SfxItemPropertyMapEntry* lcl_GetAutoFieldMap()
{
    static SfxItemPropertyMapEntry aAutoFieldMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_CELLBACK), ATTR_BACKGROUND,        &::getCppuType((const sal_Int32*)0),        0, MID_BACK_COLOR },
        {MAP_CHAR_LEN(SC_UNONAME_CCOLOR),   ATTR_FONT_COLOR,        &::getCppuType((const sal_Int32*)0),        0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_COUTL),    ATTR_FONT_CONTOUR,      &::getBooleanCppuType(),                    0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CCROSS),   ATTR_FONT_CROSSEDOUT,   &::getBooleanCppuType(),                    0, MID_CROSSED_OUT },
        {MAP_CHAR_LEN(SC_UNONAME_CFONT),    ATTR_FONT,              &::getCppuType((const sal_Int16*)0),        0, MID_FONT_FAMILY },
        {MAP_CHAR_LEN(SC_UNONAME_CFCHARS),  ATTR_FONT,              &::getCppuType((sal_Int16*)0),              0, MID_FONT_CHAR_SET },
        {MAP_CHAR_LEN(SC_UNO_CJK_CFCHARS),  ATTR_CJK_FONT,          &::getCppuType((sal_Int16*)0),              0, MID_FONT_CHAR_SET },
        {MAP_CHAR_LEN(SC_UNO_CTL_CFCHARS),  ATTR_CTL_FONT,          &::getCppuType((sal_Int16*)0),              0, MID_FONT_CHAR_SET },
        {MAP_CHAR_LEN(SC_UNONAME_CFFAMIL),  ATTR_FONT,              &::getCppuType((sal_Int16*)0),              0, MID_FONT_FAMILY },
        {MAP_CHAR_LEN(SC_UNO_CJK_CFFAMIL),  ATTR_CJK_FONT,          &::getCppuType((sal_Int16*)0),              0, MID_FONT_FAMILY },
        {MAP_CHAR_LEN(SC_UNO_CTL_CFFAMIL),  ATTR_CTL_FONT,          &::getCppuType((sal_Int16*)0),              0, MID_FONT_FAMILY },
        {MAP_CHAR_LEN(SC_UNONAME_CFNAME),   ATTR_FONT,              &::getCppuType((rtl::OUString*)0),          0, MID_FONT_FAMILY_NAME },
        {MAP_CHAR_LEN(SC_UNO_CJK_CFNAME),   ATTR_CJK_FONT,          &::getCppuType((rtl::OUString*)0),          0, MID_FONT_FAMILY_NAME },
        {MAP_CHAR_LEN(SC_UNO_CTL_CFNAME),   ATTR_CTL_FONT,          &::getCppuType((rtl::OUString*)0),          0, MID_FONT_FAMILY_NAME },
        {MAP_CHAR_LEN(SC_UNONAME_CFPITCH),  ATTR_FONT,              &::getCppuType((sal_Int16*)0),              0, MID_FONT_PITCH },
        {MAP_CHAR_LEN(SC_UNO_CJK_CFPITCH),  ATTR_CJK_FONT,          &::getCppuType((sal_Int16*)0),              0, MID_FONT_PITCH },
        {MAP_CHAR_LEN(SC_UNO_CTL_CFPITCH),  ATTR_CTL_FONT,          &::getCppuType((sal_Int16*)0),              0, MID_FONT_PITCH },
        {MAP_CHAR_LEN(SC_UNONAME_CFSTYLE),  ATTR_FONT,              &::getCppuType((rtl::OUString*)0),          0, MID_FONT_STYLE_NAME },
        {MAP_CHAR_LEN(SC_UNO_CJK_CFSTYLE),  ATTR_CJK_FONT,          &::getCppuType((rtl::OUString*)0),          0, MID_FONT_STYLE_NAME },
        {MAP_CHAR_LEN(SC_UNO_CTL_CFSTYLE),  ATTR_CTL_FONT,          &::getCppuType((rtl::OUString*)0),          0, MID_FONT_STYLE_NAME },
        {MAP_CHAR_LEN(SC_UNONAME_CHEIGHT),  ATTR_FONT_HEIGHT,       &::getCppuType((float*)0),                  0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_CJK_CHEIGHT),  ATTR_CJK_FONT_HEIGHT,   &::getCppuType((float*)0),                  0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_CTL_CHEIGHT),  ATTR_CTL_FONT_HEIGHT,   &::getCppuType((float*)0),                  0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_COVER),    ATTR_FONT_OVERLINE,     &::getCppuType((const sal_Int16*)0),        0, MID_TL_STYLE },
        {MAP_CHAR_LEN(SC_UNONAME_CPOST),    ATTR_FONT_POSTURE,      &::getCppuType((awt::FontSlant*)0),         0, MID_POSTURE },
        {MAP_CHAR_LEN(SC_UNO_CJK_CPOST),    ATTR_CJK_FONT_POSTURE,  &::getCppuType((awt::FontSlant*)0),         0, MID_POSTURE },
        {MAP_CHAR_LEN(SC_UNO_CTL_CPOST),    ATTR_CTL_FONT_POSTURE,  &::getCppuType((awt::FontSlant*)0),         0, MID_POSTURE },
        {MAP_CHAR_LEN(SC_UNONAME_CSHADD),   ATTR_FONT_SHADOWED,     &::getBooleanCppuType(),                    0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_TBLBORD),  SC_WID_UNO_TBLBORD,     &::getCppuType((table::TableBorder*)0),     0, 0 | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_TBLBORD2),  SC_WID_UNO_TBLBORD2,     &::getCppuType((table::TableBorder2*)0),     0, 0 | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_CUNDER),   ATTR_FONT_UNDERLINE,    &::getCppuType((const sal_Int16*)0),        0, MID_TL_STYLE },
        {MAP_CHAR_LEN(SC_UNONAME_CWEIGHT),  ATTR_FONT_WEIGHT,       &::getCppuType((float*)0),                  0, MID_WEIGHT },
        {MAP_CHAR_LEN(SC_UNO_CJK_CWEIGHT),  ATTR_CJK_FONT_WEIGHT,   &::getCppuType((float*)0),                  0, MID_WEIGHT },
        {MAP_CHAR_LEN(SC_UNO_CTL_CWEIGHT),  ATTR_CTL_FONT_WEIGHT,   &::getCppuType((float*)0),                  0, MID_WEIGHT },
        {MAP_CHAR_LEN(SC_UNONAME_CELLHJUS), ATTR_HOR_JUSTIFY,       &::getCppuType((const table::CellHoriJustify*)0),   0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLHJUS_METHOD), ATTR_HOR_JUSTIFY_METHOD, &::getCppuType((const sal_Int32*)0),   0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLTRAN), ATTR_BACKGROUND,        &::getBooleanCppuType(),                    0, MID_GRAPHIC_TRANSPARENT },
        {MAP_CHAR_LEN(SC_UNONAME_WRAP),     ATTR_LINEBREAK,         &::getBooleanCppuType(),                    0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLORI),  ATTR_STACKED,           &::getCppuType((const table::CellOrientation*)0),   0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PBMARGIN), ATTR_MARGIN,            &::getCppuType((const sal_Int32*)0),        0, MID_MARGIN_LO_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PLMARGIN), ATTR_MARGIN,            &::getCppuType((const sal_Int32*)0),        0, MID_MARGIN_L_MARGIN  | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PRMARGIN), ATTR_MARGIN,            &::getCppuType((const sal_Int32*)0),        0, MID_MARGIN_R_MARGIN  | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PTMARGIN), ATTR_MARGIN,            &::getCppuType((const sal_Int32*)0),        0, MID_MARGIN_UP_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_ROTANG),   ATTR_ROTATE_VALUE,      &::getCppuType((const sal_Int32*)0),        0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_ROTREF),   ATTR_ROTATE_MODE,       &::getCppuType((const sal_Int32*)0),   0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLVJUS), ATTR_VER_JUSTIFY,       &::getCppuType((const sal_Int32*)0),   0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLVJUS_METHOD), ATTR_VER_JUSTIFY_METHOD, &::getCppuType((const sal_Int32*)0),   0, 0 },
        {0,0,0,0,0,0}
    };
    return aAutoFieldMap_Impl;
}

//------------------------------------------------------------------------

#define SCAUTOFORMATSOBJ_SERVICE    "com.sun.star.sheet.TableAutoFormats"

SC_SIMPLE_SERVICE_INFO( ScAutoFormatFieldObj, "ScAutoFormatFieldObj", "com.sun.star.sheet.TableAutoFormatField" )
SC_SIMPLE_SERVICE_INFO( ScAutoFormatObj, "ScAutoFormatObj", "com.sun.star.sheet.TableAutoFormat" )
SC_SIMPLE_SERVICE_INFO( ScAutoFormatsObj, "ScAutoFormatsObj", SCAUTOFORMATSOBJ_SERVICE )

//------------------------------------------------------------------------

static bool lcl_FindAutoFormatIndex( const ScAutoFormat& rFormats, const rtl::OUString& rName, sal_uInt16& rOutIndex )
{
    ScAutoFormat::const_iterator itBeg = rFormats.begin(), itEnd = rFormats.end();
    for (ScAutoFormat::const_iterator it = itBeg; it != itEnd; ++it)
    {
        const ScAutoFormatData* pEntry = it->second;
        const rtl::OUString& aEntryName = pEntry->GetName();
        if ( aEntryName.equals(rName) )
        {
            size_t nPos = std::distance(itBeg, it);
            rOutIndex = nPos;
            return true;
        }
    }
    return false;       // is nich
}

//------------------------------------------------------------------------

ScAutoFormatsObj::ScAutoFormatsObj()
{
    //! Dieses Objekt darf es nur einmal geben, und es muss an den Auto-Format-Daten
    //! bekannt sein, damit Aenderungen gebroadcasted werden koennen
}

ScAutoFormatsObj::~ScAutoFormatsObj()
{
}

// stuff for exService_...

uno::Reference<uno::XInterface> SAL_CALL ScAutoFormatsObj_CreateInstance(
                        const uno::Reference<lang::XMultiServiceFactory>& )
{
    SolarMutexGuard aGuard;
    ScDLL::Init();
    static uno::Reference< uno::XInterface > xInst((::cppu::OWeakObject*) new ScAutoFormatsObj);
    return xInst;
}

rtl::OUString ScAutoFormatsObj::getImplementationName_Static()
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "stardiv.StarCalc.ScAutoFormatsObj" ));
}

uno::Sequence<rtl::OUString> ScAutoFormatsObj::getSupportedServiceNames_Static()
{
    uno::Sequence<rtl::OUString> aRet(1);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SCAUTOFORMATSOBJ_SERVICE ));
    return aRet;
}

// XTableAutoFormats

ScAutoFormatObj* ScAutoFormatsObj::GetObjectByIndex_Impl(sal_uInt16 nIndex)
{
    if (nIndex < ScGlobal::GetOrCreateAutoFormat()->size())
        return new ScAutoFormatObj(nIndex);

    return NULL;    // falscher Index
}

ScAutoFormatObj* ScAutoFormatsObj::GetObjectByName_Impl(const rtl::OUString& aName)
{
    String aString(aName);
    sal_uInt16 nIndex;
    if (lcl_FindAutoFormatIndex(
            *ScGlobal::GetOrCreateAutoFormat(), aString, nIndex ))
        return GetObjectByIndex_Impl(nIndex);
    return NULL;
}

// container::XNameContainer

void SAL_CALL ScAutoFormatsObj::insertByName( const rtl::OUString& aName, const uno::Any& aElement )
                            throw(lang::IllegalArgumentException, container::ElementExistException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    sal_Bool bDone = false;
    //  Reflection muss nicht uno::XInterface sein, kann auch irgendein Interface sein...
    uno::Reference< uno::XInterface > xInterface(aElement, uno::UNO_QUERY);
    if ( xInterface.is() )
    {
        ScAutoFormatObj* pFormatObj = ScAutoFormatObj::getImplementation( xInterface );
        if ( pFormatObj && !pFormatObj->IsInserted() )  // noch nicht eingefuegt?
        {
            String aNameStr(aName);
            ScAutoFormat* pFormats = ScGlobal::GetOrCreateAutoFormat();

            sal_uInt16 nDummy;
            if (!lcl_FindAutoFormatIndex( *pFormats, aNameStr, nDummy ))
            {
                ScAutoFormatData* pNew = new ScAutoFormatData();
                pNew->SetName( aNameStr );

                if (pFormats->insert(pNew))
                {
                    //! Notify fuer andere Objekte
                    pFormats->Save();   // sofort speichern

                    sal_uInt16 nNewIndex;
                    if (lcl_FindAutoFormatIndex( *pFormats, aNameStr, nNewIndex ))
                    {
                        pFormatObj->InitFormat( nNewIndex );    // kann jetzt benutzt werden
                        bDone = sal_True;
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

void SAL_CALL ScAutoFormatsObj::replaceByName( const rtl::OUString& aName, const uno::Any& aElement )
                            throw(lang::IllegalArgumentException, container::NoSuchElementException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    //! zusammenfassen?
    removeByName( aName );
    insertByName( aName, aElement );
}

void SAL_CALL ScAutoFormatsObj::removeByName( const rtl::OUString& aName )
                                throw(container::NoSuchElementException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    String aNameStr(aName);
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
                                                    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.TableAutoFormatEnumeration")));
}

// container::XIndexAccess

sal_Int32 SAL_CALL ScAutoFormatsObj::getCount() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return ScGlobal::GetOrCreateAutoFormat()->size();
}

uno::Any SAL_CALL ScAutoFormatsObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference< container::XNamed >  xFormat(GetObjectByIndex_Impl((sal_uInt16)nIndex));
    if (!xFormat.is())
        throw lang::IndexOutOfBoundsException();
    return uno::makeAny(xFormat);
}

uno::Type SAL_CALL ScAutoFormatsObj::getElementType() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return ::getCppuType((const uno::Reference< container::XNamed >*)0);    // muss zu getByIndex passen
}

sal_Bool SAL_CALL ScAutoFormatsObj::hasElements() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

// container::XNameAccess

uno::Any SAL_CALL ScAutoFormatsObj::getByName( const rtl::OUString& aName )
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference< container::XNamed >  xFormat(GetObjectByName_Impl(aName));
    if (!xFormat.is())
        throw container::NoSuchElementException();
    return uno::makeAny(xFormat);
}

uno::Sequence<rtl::OUString> SAL_CALL ScAutoFormatsObj::getElementNames()
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    ScAutoFormat* pFormats = ScGlobal::GetOrCreateAutoFormat();
    String aName;
    uno::Sequence<rtl::OUString> aSeq(pFormats->size());
    rtl::OUString* pAry = aSeq.getArray();
    ScAutoFormat::const_iterator it = pFormats->begin(), itEnd = pFormats->end();
    for (size_t i = 0; it != itEnd; ++it, ++i)
    {
        pAry[i] = it->second->GetName();
    }
    return aSeq;
}

sal_Bool SAL_CALL ScAutoFormatsObj::hasByName( const rtl::OUString& aName )
                                        throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    String aString(aName);
    sal_uInt16 nDummy;
    return lcl_FindAutoFormatIndex(
        *ScGlobal::GetOrCreateAutoFormat(), aString, nDummy );
}

//------------------------------------------------------------------------

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
                const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException)
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

ScAutoFormatObj* ScAutoFormatObj::getImplementation(
                        const uno::Reference<uno::XInterface> xObj )
{
    ScAutoFormatObj* pRet = NULL;
    uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
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

    return NULL;
}

// container::XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScAutoFormatObj::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.TableAutoFormatEnumeration")));
}

// container::XIndexAccess

sal_Int32 SAL_CALL ScAutoFormatObj::getCount() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (IsInserted())
        return SC_AF_FIELD_COUNT;   // immer 16 Elemente
    else
        return 0;
}

uno::Any SAL_CALL ScAutoFormatObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if ( nIndex < 0 || nIndex >= getCount() )
        throw lang::IndexOutOfBoundsException();

    if (IsInserted())
        return uno::makeAny(uno::Reference< beans::XPropertySet >(GetObjectByIndex_Impl((sal_uInt16)nIndex)));
    return uno::Any();
}

uno::Type SAL_CALL ScAutoFormatObj::getElementType() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return ::getCppuType((const uno::Reference< beans::XPropertySet >*)0);  // muss zu getByIndex passen
}

sal_Bool SAL_CALL ScAutoFormatObj::hasElements() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

// container::XNamed

rtl::OUString SAL_CALL ScAutoFormatObj::getName() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    ScAutoFormat* pFormats = ScGlobal::GetOrCreateAutoFormat();
    if (IsInserted() && nFormatIndex < pFormats->size())
        return pFormats->findByIndex(nFormatIndex)->GetName();

    return rtl::OUString();
}

void SAL_CALL ScAutoFormatObj::setName( const rtl::OUString& aNewName )
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    String aNewString(aNewName);
    ScAutoFormat* pFormats = ScGlobal::GetOrCreateAutoFormat();

    sal_uInt16 nDummy;
    if (IsInserted() && nFormatIndex < pFormats->size() &&
        !lcl_FindAutoFormatIndex( *pFormats, aNewName, nDummy ))
    {
        ScAutoFormat::iterator it = pFormats->begin();
        std::advance(it, nFormatIndex);
        ScAutoFormatData* pData = it->second;
        OSL_ENSURE(pData,"AutoFormat Daten nicht da");

        ScAutoFormatData* pNew = new ScAutoFormatData(*pData);
        pNew->SetName( aNewString );

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
                                                        throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    static uno::Reference< beans::XPropertySetInfo > aRef(new SfxItemPropertySetInfo( aPropSet.getPropertyMap() ));
    return aRef;
}

void SAL_CALL ScAutoFormatObj::setPropertyValue(
                        const rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    ScAutoFormat* pFormats = ScGlobal::GetOrCreateAutoFormat();
    if (IsInserted() && nFormatIndex < pFormats->size())
    {
        ScAutoFormatData* pData = pFormats->findByIndex(nFormatIndex);
        OSL_ENSURE(pData,"AutoFormat Daten nicht da");

        String aPropString(aPropertyName);
        sal_Bool bBool = sal_Bool();
        if (aPropString.EqualsAscii( SC_UNONAME_INCBACK ) && (aValue >>= bBool))
            pData->SetIncludeBackground( bBool );
        else if (aPropString.EqualsAscii( SC_UNONAME_INCBORD ) && (aValue >>= bBool))
            pData->SetIncludeFrame( bBool );
        else if (aPropString.EqualsAscii( SC_UNONAME_INCFONT ) && (aValue >>= bBool))
            pData->SetIncludeFont( bBool );
        else if (aPropString.EqualsAscii( SC_UNONAME_INCJUST ) && (aValue >>= bBool))
            pData->SetIncludeJustify( bBool );
        else if (aPropString.EqualsAscii( SC_UNONAME_INCNUM ) && (aValue >>= bBool))
            pData->SetIncludeValueFormat( bBool );
        else if (aPropString.EqualsAscii( SC_UNONAME_INCWIDTH ) && (aValue >>= bBool))
            pData->SetIncludeWidthHeight( bBool );

        // else Fehler

        //! Notify fuer andere Objekte
        pFormats->SetSaveLater(sal_True);
    }
}

uno::Any SAL_CALL ScAutoFormatObj::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Any aAny;

    ScAutoFormat* pFormats = ScGlobal::GetOrCreateAutoFormat();
    if (IsInserted() && nFormatIndex < pFormats->size())
    {
        ScAutoFormatData* pData = pFormats->findByIndex(nFormatIndex);
        OSL_ENSURE(pData,"AutoFormat Daten nicht da");

        sal_Bool bValue;
        sal_Bool bError = false;

        String aPropString(aPropertyName);
        if (aPropString.EqualsAscii( SC_UNONAME_INCBACK ))
            bValue = pData->GetIncludeBackground();
        else if (aPropString.EqualsAscii( SC_UNONAME_INCBORD ))
            bValue = pData->GetIncludeFrame();
        else if (aPropString.EqualsAscii( SC_UNONAME_INCFONT ))
            bValue = pData->GetIncludeFont();
        else if (aPropString.EqualsAscii( SC_UNONAME_INCJUST ))
            bValue = pData->GetIncludeJustify();
        else if (aPropString.EqualsAscii( SC_UNONAME_INCNUM ))
            bValue = pData->GetIncludeValueFormat();
        else if (aPropString.EqualsAscii( SC_UNONAME_INCWIDTH ))
            bValue = pData->GetIncludeWidthHeight();
        else
            bError = sal_True;      // unbekannte Property

        if (!bError)
            aAny <<= bValue;
    }

    return aAny;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScAutoFormatObj )

//------------------------------------------------------------------------

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
                                                        throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    static uno::Reference< beans::XPropertySetInfo > aRef(new SfxItemPropertySetInfo( aPropSet.getPropertyMap() ));
    return aRef;
}

void SAL_CALL ScAutoFormatFieldObj::setPropertyValue(
                        const rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
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
                sal_Bool bDone = false;

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
                                    pData->PutItem( nFieldIndex, SfxBoolItem( ATTR_STACKED, sal_True ) );
                                break;
                                default:
                                {
                                    // added to avoid warnings
                                }
                            }
                            bDone = sal_True;
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
                    pFormats->SetSaveLater(sal_True);
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
                            pFormats->SetSaveLater(sal_True);
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
                            pFormats->SetSaveLater(sal_True);
                        }
                    }
                    break;
            }
        }
    }
}

uno::Any SAL_CALL ScAutoFormatFieldObj::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
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
                        const SfxInt32Item* pRotItem = (const SfxInt32Item*)pData->GetItem( nFieldIndex, ATTR_ROTATE_VALUE );
                        sal_Int32 nRot = pRotItem ? pRotItem->GetValue() : 0;
                        sal_Bool bStacked = ((const SfxBoolItem*)pItem)->GetValue();
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

//------------------------------------------------------------------------



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
