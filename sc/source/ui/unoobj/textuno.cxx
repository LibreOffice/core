/*************************************************************************
 *
 *  $RCSfile: textuno.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:08 $
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
#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <svx/editeng.hxx>
#include <svx/editobj.hxx>
#include <svx/flditem.hxx>
#include <svx/unomid.hxx>
#include <svx/unoprnms.hxx>
#include <rtl/uuid.h>

#ifndef _COM_SUN_STAR_AWT_FONTSLANT_HPP_
#include <com/sun/star/awt/FontSlant.hpp>
#endif

#include "textuno.hxx"
#include "fielduno.hxx"
#include "servuno.hxx"
#include "editsrc.hxx"
#include "docsh.hxx"
#include "editutil.hxx"
#include "unoguard.hxx"
#include "miscuno.hxx"

using namespace com::sun::star;

//------------------------------------------------------------------------

//  wie SVX_UNOEDIT_CHAR_PROPERTIES, aber mit CONVERT_TWIPS bei der Font-Hoehe
//  (Kopf-/Fusszeilen sind in Twips)

#define SC_TWIPS_CHAR_PROPERTIES \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_COLOR),           EE_CHAR_COLOR,      &::getCppuType((const sal_Int32*)0),    0, 0 }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_CROSSEDOUT),  EE_CHAR_STRIKEOUT,  &::getBooleanCppuType(),        0, MID_CROSSED_OUT }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_ESCAPEMENT),  EE_CHAR_ESCAPEMENT, &::getCppuType((const sal_Int16*)0),    0, MID_ESC }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTNAME),        EE_CHAR_FONTINFO,   &::getCppuType((const ::rtl::OUString*)0),  0, MID_FONT_FAMILY_NAME },\
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_HEIGHT),      EE_CHAR_FONTHEIGHT, &::getCppuType((const Float*)0),    0, MID_FONTHEIGHT | CONVERT_TWIPS }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_POSTURE),     EE_CHAR_ITALIC,     &::getCppuType((const ::com::sun::star::awt::FontSlant*)0),0, MID_POSTURE }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_SHADOWED),        EE_CHAR_SHADOW,     &::getBooleanCppuType(),        0, 0 }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_STYLE),           EE_CHAR_FONTINFO,   &::getCppuType((const ::rtl::OUString*)0),  0, MID_FONT_STYLE_NAME }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_UNDERLINE),       EE_CHAR_UNDERLINE,  &::getCppuType((const sal_Int16*)0),    0, MID_UNDERLINE }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_WEIGHT),      EE_CHAR_WEIGHT,     &::getCppuType((const Float*)0),    0, MID_WEIGHT }


const SfxItemPropertyMap* lcl_GetHdFtPropertyMap()
{
    static SfxItemPropertyMap aHdFtPropertyMap_Impl[] =
    {
        SC_TWIPS_CHAR_PROPERTIES,
        SVX_UNOEDIT_FONT_PROPERTIES,
        SVX_UNOEDIT_PARA_PROPERTIES,
        {0,0,0,0}
    };
    return aHdFtPropertyMap_Impl;
}

//------------------------------------------------------------------------

SC_SIMPLE_SERVICE_INFO( ScHeaderFooterContentObj, "ScHeaderFooterContentObj", "com.sun.star.sheet.HeaderFooterContent" )
SC_SIMPLE_SERVICE_INFO( ScHeaderFooterTextObj, "ScHeaderFooterTextObj", "stardiv.one.Text.Text" )

//------------------------------------------------------------------------

ScHeaderFooterContentObj::ScHeaderFooterContentObj() :
    pLeftText   ( NULL ),
    pCenterText ( NULL ),
    pRightText  ( NULL )
{
    //  fuer getReflection
}

ScHeaderFooterContentObj::ScHeaderFooterContentObj( const EditTextObject* pLeft,
                                                    const EditTextObject* pCenter,
                                                    const EditTextObject* pRight ) :
    pLeftText   ( NULL ),
    pCenterText ( NULL ),
    pRightText  ( NULL )
{
    if ( pLeft )
        pLeftText   = pLeft->Clone();
    if ( pCenter )
        pCenterText = pCenter->Clone();
    if ( pRight )
        pRightText  = pRight->Clone();
}

ScHeaderFooterContentObj::~ScHeaderFooterContentObj()
{
    delete pLeftText;
    delete pCenterText;
    delete pRightText;
}

void ScHeaderFooterContentObj::UpdateText( USHORT nPart, EditEngine& rSource )
{
    EditTextObject* pNew = rSource.CreateTextObject();
    switch (nPart)
    {
        case SC_HDFT_LEFT:
            delete pLeftText;
            pLeftText = pNew;
            break;
        case SC_HDFT_CENTER:
            delete pCenterText;
            pCenterText = pNew;
            break;
        default:                // SC_HDFT_RIGHT
            delete pRightText;
            pRightText = pNew;
            break;
    }
}

// XHeaderFooterContent

uno::Reference<text::XText> SAL_CALL ScHeaderFooterContentObj::getLeftText()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScHeaderFooterTextObj( *this, SC_HDFT_LEFT );
}

uno::Reference<text::XText> SAL_CALL ScHeaderFooterContentObj::getCenterText()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScHeaderFooterTextObj( *this, SC_HDFT_CENTER );
}

uno::Reference<text::XText> SAL_CALL ScHeaderFooterContentObj::getRightText()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScHeaderFooterTextObj( *this, SC_HDFT_RIGHT );
}

// XUnoTunnel

sal_Int64 SAL_CALL ScHeaderFooterContentObj::getSomething(
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
const uno::Sequence<sal_Int8>& ScHeaderFooterContentObj::getUnoTunnelId()
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
ScHeaderFooterContentObj* ScHeaderFooterContentObj::getImplementation(
                                const uno::Reference<sheet::XHeaderFooterContent> xObj )
{
    ScHeaderFooterContentObj* pRet = NULL;
    uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
    if (xUT.is())
        pRet = (ScHeaderFooterContentObj*) xUT->getSomething( getUnoTunnelId() );
    return pRet;
}


//------------------------------------------------------------------------

ScHeaderFooterTextObj::ScHeaderFooterTextObj( ScHeaderFooterContentObj& rContent,
                                                USHORT nP ) :
    rContentObj( rContent ),
    nPart( nP )
{
    rContentObj.acquire();      // darf nicht wegkommen

    //! pUnoText erst bei Bedarf anlegen?
    //  nicht einfach aggregieren, weil getString/setString selber gemacht wird
    ScHeaderFooterEditSource aEditSource( &rContent, nP );
    pUnoText = new SvxUnoText( &aEditSource, lcl_GetHdFtPropertyMap(), uno::Reference<text::XText>() );
    pUnoText->acquire();
}

ScHeaderFooterTextObj::~ScHeaderFooterTextObj()
{
    rContentObj.release();

    if (pUnoText)
        pUnoText->release();
}

// XText

uno::Reference<text::XTextCursor> SAL_CALL ScHeaderFooterTextObj::createTextCursor()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScHeaderFooterTextCursor( *pUnoText );
}

uno::Reference<text::XTextCursor> SAL_CALL ScHeaderFooterTextObj::createTextCursorByRange(
                                    const uno::Reference<text::XTextRange>& aTextPosition )
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return pUnoText->createTextCursorByRange(aTextPosition);
    //! wie ScCellObj::createTextCursorByRange, wenn SvxUnoTextRange_getReflection verfuegbar
}

void ScHeaderFooterTextObj::FillDummyFieldData( ScHeaderFieldData& rData )  // static
{
    String aDummy = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM( "???" ));
    rData.aTitle        = aDummy;
    rData.aLongDocName  = aDummy;
    rData.aShortDocName = aDummy;
    rData.aTabName      = aDummy;
    rData.nPageNo       = 1;
    rData.nTotalPages   = 99;
}

rtl::OUString SAL_CALL ScHeaderFooterTextObj::getString() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    rtl::OUString aRet;
    const EditTextObject* pData;
    if (nPart == SC_HDFT_LEFT)
        pData = rContentObj.GetLeftEditObject();
    else if (nPart == SC_HDFT_CENTER)
        pData = rContentObj.GetCenterEditObject();
    else
        pData = rContentObj.GetRightEditObject();
    if (pData)
    {
        // for pure text, no font info is needed in pool defaults
        ScHeaderEditEngine aEditEngine( EditEngine::CreatePool(), TRUE );

        ScHeaderFieldData aData;
        FillDummyFieldData( aData );
        aEditEngine.SetData( aData );

        aEditEngine.SetText(*pData);
        aRet = ScEditUtil::GetSpaceDelimitedString( aEditEngine );
    }
    return aRet;
}

void SAL_CALL ScHeaderFooterTextObj::setString( const rtl::OUString& aText ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aString = aText;

    // for pure text, no font info is needed in pool defaults
    ScHeaderEditEngine aEditEngine( EditEngine::CreatePool(), TRUE );
    aEditEngine.SetText( aString );

    rContentObj.UpdateText( nPart, aEditEngine );
}

void SAL_CALL ScHeaderFooterTextObj::insertString( const uno::Reference<text::XTextRange>& xRange,
                                            const rtl::OUString& aString, sal_Bool bAbsorb )
                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    pUnoText->insertString( xRange, aString, bAbsorb );
}

void SAL_CALL ScHeaderFooterTextObj::insertControlCharacter(
                                            const uno::Reference<text::XTextRange>& xRange,
                                            sal_Int16 nControlCharacter, sal_Bool bAbsorb )
                                throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    pUnoText->insertControlCharacter( xRange, nControlCharacter, bAbsorb );
}

void SAL_CALL ScHeaderFooterTextObj::insertTextContent(
                                            const uno::Reference<text::XTextRange >& xRange,
                                            const uno::Reference<text::XTextContent >& xContent,
                                            sal_Bool bAbsorb )
                                throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( xContent.is() && xRange.is() )
    {
        ScHeaderFieldObj* pHeaderField = ScHeaderFieldObj::getImplementation( xContent );

        SvxUnoTextRangeBase* pTextRange =
            ScHeaderFooterTextCursor::getImplementation( xRange );

#if 0
        if (!pTextRange)
            pTextRange = (SvxUnoTextRange*)xRange->getImplementation(
                                            SvxUnoTextRange_getReflection() );
        //! bei SvxUnoTextRange testen, ob in passendem Objekt !!!
#endif

        if ( pHeaderField && !pHeaderField->IsInserted() && pTextRange )
        {
            SvxEditSource* pEditSource = pTextRange->GetEditSource();
            ESelection aSelection = pTextRange->GetSelection();

            SvxFieldItem aItem = pHeaderField->CreateFieldItem();

            SvxTextForwarder* pForwarder = pEditSource->GetTextForwarder();
            pForwarder->QuickInsertField( aItem, aSelection );
            pEditSource->UpdateData();

            //  neue Selektion: ein Zeichen
            aSelection.Adjust();
            aSelection.nEndPara = aSelection.nStartPara;
            aSelection.nEndPos = aSelection.nStartPos + 1;
            pHeaderField->InitDoc( &rContentObj, nPart, aSelection );

            return;
        }
    }

    pUnoText->insertTextContent( xRange, xContent, bAbsorb );
}

void SAL_CALL ScHeaderFooterTextObj::removeTextContent(
                                            const uno::Reference<text::XTextContent>& xContent )
                                throw(container::NoSuchElementException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( xContent.is() )
    {
        ScHeaderFieldObj* pHeaderField = ScHeaderFieldObj::getImplementation( xContent );
        if ( pHeaderField && pHeaderField->IsInserted() )
        {
            //! Testen, ob das Feld in dieser Zelle ist
            pHeaderField->DeleteField();
            return;
        }
    }
    pUnoText->removeTextContent( xContent );
}

uno::Reference<text::XText> SAL_CALL ScHeaderFooterTextObj::getText() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return pUnoText->getText();
}

uno::Reference<text::XTextRange> SAL_CALL ScHeaderFooterTextObj::getStart() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return pUnoText->getStart();
}

uno::Reference<text::XTextRange> SAL_CALL ScHeaderFooterTextObj::getEnd() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return pUnoText->getEnd();
}

// XTextFieldsSupplier

uno::Reference<container::XEnumerationAccess> SAL_CALL ScHeaderFooterTextObj::getTextFields()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScHeaderFieldsObj( &rContentObj, nPart, SC_SERVICE_INVALID );    // alle Felder
}

uno::Reference<container::XNameAccess> SAL_CALL ScHeaderFooterTextObj::getTextFieldMasters()
                                                throw(uno::RuntimeException)
{
    //  sowas gibts nicht im Calc (?)
    return NULL;
}

// XTextRangeMover

void SAL_CALL ScHeaderFooterTextObj::moveTextRange(
                                        const uno::Reference<text::XTextRange>& xRange,
                                        sal_Int16 nParagraphs )
                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    pUnoText->moveTextRange( xRange, nParagraphs );
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScHeaderFooterTextObj::createEnumeration()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return pUnoText->createEnumeration();
}

// XElementAccess

uno::Type SAL_CALL ScHeaderFooterTextObj::getElementType() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return pUnoText->getElementType();
}

sal_Bool SAL_CALL ScHeaderFooterTextObj::hasElements() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return pUnoText->hasElements();
}

//------------------------------------------------------------------------

ScCellTextCursor::ScCellTextCursor() :
    SvxUnoTextCursor( SvxUnoText() )
{
}

ScCellTextCursor::ScCellTextCursor(const ScCellTextCursor& rOther) :
    SvxUnoTextCursor( rOther )
{
}

ScCellTextCursor::ScCellTextCursor(const SvxUnoText& rText) :
    SvxUnoTextCursor( rText )
{
}

ScCellTextCursor::~ScCellTextCursor()
{
}

// XUnoTunnel

sal_Int64 SAL_CALL ScCellTextCursor::getSomething(
                const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException)
{
    if ( rId.getLength() == 16 &&
          0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                    rId.getConstArray(), 16 ) )
    {
        return (sal_Int64)this;
    }
    return SvxUnoTextCursor::getSomething( rId );
}

// static
const uno::Sequence<sal_Int8>& ScCellTextCursor::getUnoTunnelId()
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
ScCellTextCursor* ScCellTextCursor::getImplementation( const uno::Reference<uno::XInterface> xObj )
{
    ScCellTextCursor* pRet = NULL;
    uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
    if (xUT.is())
        pRet = (ScCellTextCursor*) xUT->getSomething( getUnoTunnelId() );
    return pRet;
}

//------------------------------------------------------------------------

ScHeaderFooterTextCursor::ScHeaderFooterTextCursor() :
    SvxUnoTextCursor( SvxUnoText() )
{
}

ScHeaderFooterTextCursor::ScHeaderFooterTextCursor(const ScHeaderFooterTextCursor& rOther) :
    SvxUnoTextCursor( rOther )
{
}

ScHeaderFooterTextCursor::ScHeaderFooterTextCursor(const SvxUnoText& rText) :
    SvxUnoTextCursor( rText )
{
}

ScHeaderFooterTextCursor::~ScHeaderFooterTextCursor()
{
}

// XUnoTunnel

sal_Int64 SAL_CALL ScHeaderFooterTextCursor::getSomething(
                const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException)
{
    if ( rId.getLength() == 16 &&
          0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                    rId.getConstArray(), 16 ) )
    {
        return (sal_Int64)this;
    }
    return SvxUnoTextCursor::getSomething( rId );
}

// static
const uno::Sequence<sal_Int8>& ScHeaderFooterTextCursor::getUnoTunnelId()
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
ScHeaderFooterTextCursor* ScHeaderFooterTextCursor::getImplementation(
                                const uno::Reference<uno::XInterface> xObj )
{
    ScHeaderFooterTextCursor* pRet = NULL;
    uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
    if (xUT.is())
        pRet = (ScHeaderFooterTextCursor*) xUT->getSomething( getUnoTunnelId() );
    return pRet;
}

//------------------------------------------------------------------------



