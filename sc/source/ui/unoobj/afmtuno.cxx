/*************************************************************************
 *
 *  $RCSfile: afmtuno.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-23 10:37:05 $
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

#include "scitems.hxx"
#include <tools/debug.hxx>
#include <tools/shl.hxx>
#include <svtools/poolitem.hxx>
#include <svx/unomid.hxx>

#ifndef _RTL_UUID_H_ //autogen wg. rtl_createUuid
#include <rtl/uuid.h>
#endif

#ifndef _COM_SUN_STAR_TABLE_BORDERLINE_HPP_
#include <com/sun/star/table/BorderLine.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLVERTJUSTIFY_HPP_
#include <com/sun/star/table/CellVertJustify.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_SHADOWLOCATION_HPP_
#include <com/sun/star/table/ShadowLocation.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_TABLEBORDER_HPP_
#include <com/sun/star/table/TableBorder.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_SHADOWFORMAT_HPP_
#include <com/sun/star/table/ShadowFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLRANGEADDRESS_HPP_
#include <com/sun/star/table/CellRangeAddress.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLCONTENTTYPE_HPP_
#include <com/sun/star/table/CellContentType.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_TABLEORIENTATION_HPP_
#include <com/sun/star/table/TableOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLHORIJUSTIFY_HPP_
#include <com/sun/star/table/CellHoriJustify.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_SORTFIELD_HPP_
#include <com/sun/star/util/SortField.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_SORTFIELDTYPE_HPP_
#include <com/sun/star/util/SortFieldType.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLORIENTATION_HPP_
#include <com/sun/star/table/CellOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLADDRESS_HPP_
#include <com/sun/star/table/CellAddress.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_SIMPLEFONTMETRIC_HPP_
#include <com/sun/star/awt/SimpleFontMetric.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTWEIGHT_HPP_
#include <com/sun/star/awt/FontWeight.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTSLANT_HPP_
#include <com/sun/star/awt/FontSlant.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_CHARSET_HPP_
#include <com/sun/star/awt/CharSet.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTDESCRIPTOR_HPP_
#include <com/sun/star/awt/FontDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTWIDTH_HPP_
#include <com/sun/star/awt/FontWidth.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XFONT_HPP_
#include <com/sun/star/awt/XFont.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTTYPE_HPP_
#include <com/sun/star/awt/FontType.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTUNDERLINE_HPP_
#include <com/sun/star/awt/FontUnderline.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTSTRIKEOUT_HPP_
#include <com/sun/star/awt/FontStrikeout.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTFAMILY_HPP_
#include <com/sun/star/awt/FontFamily.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTPITCH_HPP_
#include <com/sun/star/awt/FontPitch.hpp>
#endif

#include "afmtuno.hxx"
#include "miscuno.hxx"
#include "autoform.hxx"
#include "unoguard.hxx"
#include "scdll.hxx"
#include "unonames.hxx"

using namespace ::com::sun::star;

//------------------------------------------------------------------------

//  ein AutoFormat hat immer 16 Eintraege
#define SC_AF_FIELD_COUNT 16

//------------------------------------------------------------------------

//  AutoFormat-Map nur fuer PropertySetInfo, ohne Which-IDs

const SfxItemPropertyMap* lcl_GetAutoFormatMap()
{
    static SfxItemPropertyMap aAutoFormatMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_INCBACK),  0,  &::getBooleanCppuType(),    0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_INCBORD),  0,  &::getBooleanCppuType(),    0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_INCFONT),  0,  &::getBooleanCppuType(),    0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_INCJUST),  0,  &::getBooleanCppuType(),    0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_INCNUM),   0,  &::getBooleanCppuType(),    0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_INCWIDTH), 0,  &::getBooleanCppuType(),    0, 0 },
        {0,0,0,0}
    };
    return aAutoFormatMap_Impl;
}

//! Zahlformat (String/Language) ??? (in XNumberFormat nur ReadOnly)
//! table::TableBorder ??!?

const SfxItemPropertyMap* lcl_GetAutoFieldMap()
{
    static SfxItemPropertyMap aAutoFieldMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_CELLBACK), ATTR_BACKGROUND,    &::getCppuType((const sal_Int32*)0),        0, MID_BACK_COLOR },
        {MAP_CHAR_LEN(SC_UNONAME_CCOLOR),   ATTR_FONT_COLOR,    &::getCppuType((const sal_Int32*)0),        0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_COUTL),    ATTR_FONT_CONTOUR,  &::getBooleanCppuType(),                    0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CCROSS),   ATTR_FONT_CROSSEDOUT,&::getBooleanCppuType(),                   0, MID_CROSSED_OUT },
        {MAP_CHAR_LEN(SC_UNONAME_CFONT),    ATTR_FONT,          &::getCppuType((const sal_Int16*)0),        0, MID_FONT_FAMILY },
        {MAP_CHAR_LEN(SC_UNONAME_CFNAME),   ATTR_FONT,          &::getCppuType((const ::rtl::OUString*)0),  0, MID_FONT_FAMILY_NAME },
        {MAP_CHAR_LEN(SC_UNONAME_CHEIGHT),  ATTR_FONT_HEIGHT,   &::getCppuType((const float*)0),        0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_CPOST),    ATTR_FONT_POSTURE,  &::getCppuType((const awt::FontSlant*)0),   0, MID_POSTURE },
        {MAP_CHAR_LEN(SC_UNONAME_CSHADD),   ATTR_FONT_SHADOWED, &::getBooleanCppuType(),                    0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CUNDER),   ATTR_FONT_UNDERLINE,&::getCppuType((const sal_Int16*)0),        0, MID_UNDERLINE },
        {MAP_CHAR_LEN(SC_UNONAME_CWEIGHT),  ATTR_FONT_WEIGHT,   &::getCppuType((const float*)0),            0, MID_WEIGHT },
        {MAP_CHAR_LEN(SC_UNONAME_CELLHJUS), ATTR_HOR_JUSTIFY,   &::getCppuType((const table::CellHoriJustify*)0),   0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLTRAN), ATTR_BACKGROUND,    &::getBooleanCppuType(),                    0, MID_GRAPHIC_TRANSPARENT },
        {MAP_CHAR_LEN(SC_UNONAME_WRAP),     ATTR_LINEBREAK,     &::getBooleanCppuType(),                    0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLORI),  ATTR_ORIENTATION,   &::getCppuType((const table::CellOrientation*)0),   0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PBMARGIN), ATTR_MARGIN,        &::getCppuType((const sal_Int32*)0),        0, MID_MARGIN_LO_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PLMARGIN), ATTR_MARGIN,        &::getCppuType((const sal_Int32*)0),        0, MID_MARGIN_L_MARGIN  | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PRMARGIN), ATTR_MARGIN,        &::getCppuType((const sal_Int32*)0),        0, MID_MARGIN_R_MARGIN  | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PTMARGIN), ATTR_MARGIN,        &::getCppuType((const sal_Int32*)0),        0, MID_MARGIN_UP_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_ROTANG),   ATTR_ROTATE_VALUE,  &::getCppuType((const sal_Int32*)0),        0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_ROTREF),   ATTR_ROTATE_MODE,   &::getCppuType((const table::CellVertJustify*)0),   0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLVJUS), ATTR_VER_JUSTIFY,   &::getCppuType((const table::CellVertJustify*)0),   0, 0 },
        {0,0,0,0}
    };
    return aAutoFieldMap_Impl;
}

//------------------------------------------------------------------------

#define SCAUTOFORMATSOBJ_SERVICE    "com.sun.star.sheet.TableAutoFormats"

SC_SIMPLE_SERVICE_INFO( ScAutoFormatFieldObj, "ScAutoFormatFieldObj", "com.sun.star.sheet.TableAutoFormatField" )
SC_SIMPLE_SERVICE_INFO( ScAutoFormatObj, "ScAutoFormatObj", "com.sun.star.sheet.TableAutoFormat" )
SC_SIMPLE_SERVICE_INFO( ScAutoFormatsObj, "ScAutoFormatsObj", SCAUTOFORMATSOBJ_SERVICE )

//------------------------------------------------------------------------

sal_Bool lcl_FindAutoFormatIndex( const ScAutoFormat& rFormats, const String& rName, sal_uInt16& rOutIndex )
{
    String aEntryName;
    sal_uInt16 nCount = rFormats.GetCount();
    for( sal_uInt16 nPos=0; nPos<nCount; nPos++ )
    {
        ScAutoFormatData* pEntry = rFormats[nPos];
        pEntry->GetName( aEntryName );
        if ( aEntryName == rName )
        {
            rOutIndex = nPos;
            return sal_True;
        }
    }
    return sal_False;       // is nich
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
    ScUnoGuard aGuard;
    SC_DLL()->Load();       // load module
    static uno::Reference< uno::XInterface > xInst = (::cppu::OWeakObject*) new ScAutoFormatsObj;
    return xInst;
}

rtl::OUString ScAutoFormatsObj::getImplementationName_Static()
{
    return rtl::OUString::createFromAscii( "stardiv.StarCalc.ScAutoFormatsObj" );
}

uno::Sequence<rtl::OUString> ScAutoFormatsObj::getSupportedServiceNames_Static()
{
    uno::Sequence<rtl::OUString> aRet(1);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] = rtl::OUString::createFromAscii( SCAUTOFORMATSOBJ_SERVICE );
    return aRet;
}

// XTableAutoFormats

ScAutoFormatObj* ScAutoFormatsObj::GetObjectByIndex_Impl(sal_uInt16 nIndex)
{
    ScAutoFormat* pFormats = ScGlobal::GetAutoFormat();
    if (pFormats && nIndex < pFormats->GetCount())
        return new ScAutoFormatObj(nIndex);

    return NULL;    // falscher Index
}

ScAutoFormatObj* ScAutoFormatsObj::GetObjectByName_Impl(const rtl::OUString& aName)
{
    ScAutoFormat* pFormats = ScGlobal::GetAutoFormat();
    if (pFormats)
    {
        String aString = aName;
        sal_uInt16 nIndex;
        if (lcl_FindAutoFormatIndex( *pFormats, aString, nIndex ))
            return GetObjectByIndex_Impl(nIndex);
    }
    return NULL;
}

// container::XNameContainer

void SAL_CALL ScAutoFormatsObj::insertByName( const rtl::OUString& aName, const uno::Any& aElement )
                            throw(lang::IllegalArgumentException, container::ElementExistException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    sal_Bool bDone = sal_False;
    //  Reflection muss nicht uno::XInterface sein, kann auch irgendein Interface sein...
    uno::Reference< uno::XInterface > xInterface;
    if ( aElement >>= xInterface )
    {
        ScAutoFormatObj* pFormatObj = ScAutoFormatObj::getImplementation( xInterface );
        if ( pFormatObj && !pFormatObj->IsInserted() )  // noch nicht eingefuegt?
        {
            String aNameStr = aName;
            ScAutoFormat* pFormats = ScGlobal::GetAutoFormat();

            sal_uInt16 nDummy;
            if (pFormats && !lcl_FindAutoFormatIndex( *pFormats, aNameStr, nDummy ))
            {
                ScAutoFormatData* pNew = new ScAutoFormatData();
                pNew->SetName( aNameStr );

                if (pFormats->Insert( pNew ))
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
                    delete pNew;
                    DBG_ERROR("AutoFormat konnte nicht eingefuegt werden");
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
    ScUnoGuard aGuard;
    //! zusammenfassen?
    removeByName( aName );
    insertByName( aName, aElement );
}

void SAL_CALL ScAutoFormatsObj::removeByName( const rtl::OUString& aName )
                                throw(container::NoSuchElementException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aNameStr = aName;
    ScAutoFormat* pFormats = ScGlobal::GetAutoFormat();

    sal_uInt16 nIndex;
    if (pFormats && lcl_FindAutoFormatIndex( *pFormats, aNameStr, nIndex ))
    {
        pFormats->AtFree( nIndex );

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
    ScUnoGuard aGuard;
    return new ScIndexEnumeration(this);
}

// container::XIndexAccess

sal_Int32 SAL_CALL ScAutoFormatsObj::getCount() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScAutoFormat* pFormats = ScGlobal::GetAutoFormat();
    if (pFormats)
        return pFormats->GetCount();

    return 0;
}

uno::Any SAL_CALL ScAutoFormatsObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference< container::XNamed >  xFormat = GetObjectByIndex_Impl((sal_uInt16)nIndex);
    uno::Any aAny;
    if (xFormat.is())
        aAny <<= xFormat;
    else
        throw lang::IndexOutOfBoundsException();
    return aAny;
}

uno::Type SAL_CALL ScAutoFormatsObj::getElementType() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return ::getCppuType((const uno::Reference< container::XNamed >*)0);    // muss zu getByIndex passen
}

sal_Bool SAL_CALL ScAutoFormatsObj::hasElements() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return ( getCount() != 0 );
}

// container::XNameAccess

uno::Any SAL_CALL ScAutoFormatsObj::getByName( const rtl::OUString& aName )
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference< container::XNamed >  xFormat = GetObjectByName_Impl(aName);
    uno::Any aAny;
    if (xFormat.is())
        aAny <<= xFormat;
    else
        throw container::NoSuchElementException();
    return aAny;
}

uno::Sequence<rtl::OUString> SAL_CALL ScAutoFormatsObj::getElementNames()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScAutoFormat* pFormats = ScGlobal::GetAutoFormat();
    if (pFormats)
    {
        String aName;
        sal_uInt16 nCount = pFormats->GetCount();
        uno::Sequence<rtl::OUString> aSeq(nCount);
        rtl::OUString* pAry = aSeq.getArray();
        for (sal_uInt16 i=0; i<nCount; i++)
        {
            (*pFormats)[i]->GetName(aName);
            pAry[i] = aName;
        }
        return aSeq;
    }
    return uno::Sequence<rtl::OUString>(0);
}

sal_Bool SAL_CALL ScAutoFormatsObj::hasByName( const rtl::OUString& aName )
                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScAutoFormat* pFormats = ScGlobal::GetAutoFormat();
    if (pFormats)
    {
        String aString = aName;
        sal_uInt16 nDummy;
        return lcl_FindAutoFormatIndex( *pFormats, aString, nDummy );
    }
    return sal_False;
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
    DBG_ASSERT( nFormatIndex == SC_AFMTOBJ_INVALID, "ScAutoFormatObj::InitFormat mehrfach" );
    nFormatIndex = nNewIndex;
    //! Listening !!!
}

// XUnoTunnel

sal_Int64 SAL_CALL ScAutoFormatObj::getSomething(
                const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException)
{
    if ( rId.getLength() == 16 &&
          0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                    rId.getConstArray(), 16 ) )
    {
        return (sal_Int64)this;
    }
    return 0;
}

// static
const uno::Sequence<sal_Int8>& ScAutoFormatObj::getUnoTunnelId()
{
    static uno::Sequence<sal_Int8> * pSeq = 0;
    if( !pSeq )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static uno::Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

// static
ScAutoFormatObj* ScAutoFormatObj::getImplementation(
                        const uno::Reference<uno::XInterface> xObj )
{
    ScAutoFormatObj* pRet = NULL;
    uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
    if (xUT.is())
        pRet = (ScAutoFormatObj*) xUT->getSomething( getUnoTunnelId() );
    return pRet;
}

void ScAutoFormatObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
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
    ScUnoGuard aGuard;
    return new ScIndexEnumeration(this);
}

// container::XIndexAccess

sal_Int32 SAL_CALL ScAutoFormatObj::getCount() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (IsInserted())
        return SC_AF_FIELD_COUNT;   // immer 16 Elemente
    else
        return 0;
}

uno::Any SAL_CALL ScAutoFormatObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;

    if ( nIndex < 0 || nIndex >= getCount() )
        throw lang::IndexOutOfBoundsException();

    uno::Any aAny;
    if (IsInserted())
    {
        uno::Reference< beans::XPropertySet >  xField = GetObjectByIndex_Impl((sal_uInt16)nIndex);
        aAny <<= xField;
    }
    return aAny;
}

uno::Type SAL_CALL ScAutoFormatObj::getElementType() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return ::getCppuType((const uno::Reference< beans::XPropertySet >*)0);  // muss zu getByIndex passen
}

sal_Bool SAL_CALL ScAutoFormatObj::hasElements() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return ( getCount() != 0 );
}

// container::XNamed

rtl::OUString SAL_CALL ScAutoFormatObj::getName() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScAutoFormat* pFormats = ScGlobal::GetAutoFormat();
    if (pFormats && IsInserted() && nFormatIndex < pFormats->GetCount())
    {
        String aName;
        (*pFormats)[nFormatIndex]->GetName(aName);
        return aName;
    }
    return rtl::OUString();
}

void SAL_CALL ScAutoFormatObj::setName( const rtl::OUString& aNewName )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aNewString = aNewName;
    ScAutoFormat* pFormats = ScGlobal::GetAutoFormat();

    sal_uInt16 nDummy;
    if (pFormats && IsInserted() && nFormatIndex < pFormats->GetCount() &&
            !lcl_FindAutoFormatIndex( *pFormats, aNewString, nDummy ))
    {
        ScAutoFormatData* pData = (*pFormats)[nFormatIndex];
        DBG_ASSERT(pData,"AutoFormat Daten nicht da");

        ScAutoFormatData* pNew = new ScAutoFormatData(*pData);
        pNew->SetName( aNewString );

        pFormats->AtFree( nFormatIndex );
        if (pFormats->Insert( pNew ))
        {
            nFormatIndex = pFormats->IndexOf( pNew );   // ist evtl. anders einsortiert...

            //! Notify fuer andere Objekte
            pFormats->SetSaveLater(sal_True);
        }
        else
        {
            delete pNew;
            DBG_ERROR("AutoFormat konnte nicht eingefuegt werden");
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
    ScUnoGuard aGuard;
    static uno::Reference< beans::XPropertySetInfo > aRef = new SfxItemPropertySetInfo( aPropSet.getPropertyMap() );
    return aRef;
}

void SAL_CALL ScAutoFormatObj::setPropertyValue(
                        const rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScAutoFormat* pFormats = ScGlobal::GetAutoFormat();
    if (pFormats && IsInserted() && nFormatIndex < pFormats->GetCount())
    {
        ScAutoFormatData* pData = (*pFormats)[nFormatIndex];
        DBG_ASSERT(pData,"AutoFormat Daten nicht da");

        String aPropString = aPropertyName;
        sal_Bool bBool;
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
    ScUnoGuard aGuard;
    uno::Any aAny;

    ScAutoFormat* pFormats = ScGlobal::GetAutoFormat();
    if (pFormats && IsInserted() && nFormatIndex < pFormats->GetCount())
    {
        ScAutoFormatData* pData = (*pFormats)[nFormatIndex];
        DBG_ASSERT(pData,"AutoFormat Daten nicht da");

        sal_Bool bValue;
        sal_Bool bError = sal_False;

        String aPropString = aPropertyName;
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

void ScAutoFormatFieldObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    //  spaeter...
}

// beans::XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScAutoFormatFieldObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    static uno::Reference< beans::XPropertySetInfo > aRef = new SfxItemPropertySetInfo( aPropSet.getPropertyMap() );
    return aRef;
}

void SAL_CALL ScAutoFormatFieldObj::setPropertyValue(
                        const rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScAutoFormat* pFormats = ScGlobal::GetAutoFormat();
    String aPropString = aPropertyName;
    const SfxItemPropertyMap* pMap =
            SfxItemPropertyMap::GetByName( lcl_GetAutoFieldMap(), aPropString );

    if ( pMap && pMap->nWID && pFormats && nFormatIndex < pFormats->GetCount() )
    {
        ScAutoFormatData* pData = (*pFormats)[nFormatIndex];
        const SfxPoolItem* pItem = pData->GetItem( nFieldIndex, pMap->nWID );
        if (pItem)
        {
            SfxPoolItem* pNewItem = pItem->Clone();
            sal_Bool bDone = pNewItem->PutValue( aValue, pMap->nMemberId );
            if (bDone)
            {
                pData->PutItem( nFieldIndex, *pNewItem );

                //! Notify fuer andere Objekte?
                pFormats->SetSaveLater(sal_True);
            }
            delete pNewItem;
        }
    }
}

uno::Any SAL_CALL ScAutoFormatFieldObj::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Any aVal;

    ScAutoFormat* pFormats = ScGlobal::GetAutoFormat();
    String aPropString = aPropertyName;
    const SfxItemPropertyMap* pMap =
            SfxItemPropertyMap::GetByName( lcl_GetAutoFieldMap(), aPropString );

    if ( pMap && pMap->nWID && pFormats && nFormatIndex < pFormats->GetCount() )
    {
        const ScAutoFormatData* pData = (*pFormats)[nFormatIndex];
        const SfxPoolItem* pItem = pData->GetItem( nFieldIndex, pMap->nWID );
        if (pItem)
            pItem->QueryValue( aVal, pMap->nMemberId );
    }

    return aVal;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScAutoFormatFieldObj )

//------------------------------------------------------------------------



