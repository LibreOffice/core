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
#include <editeng/eeitem.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdobj.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editobj.hxx>
#include <editeng/flditem.hxx>
#include <svx/unomid.hxx>
#include <editeng/unoprnms.hxx>
#include <editeng/unofored.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/awt/FontSlant.hpp>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <editeng/unoipset.hxx>
#include "textuno.hxx"
#include "fielduno.hxx"
#include "servuno.hxx"
#include "editsrc.hxx"
#include "docsh.hxx"
#include "editutil.hxx"
#include "miscuno.hxx"
#include "cellsuno.hxx"
#include "hints.hxx"
#include "patattr.hxx"
#include "cell.hxx"
#include "docfunc.hxx"
#include "scmod.hxx"

using namespace com::sun::star;

//------------------------------------------------------------------------

static const SvxItemPropertySet * lcl_GetHdFtPropertySet()
{
    static SfxItemPropertyMapEntry aHdFtPropertyMap_Impl[] =
    {
        SVX_UNOEDIT_CHAR_PROPERTIES,
        SVX_UNOEDIT_FONT_PROPERTIES,
        SVX_UNOEDIT_PARA_PROPERTIES,
        SVX_UNOEDIT_NUMBERING_PROPERTIE,    // for completeness of service ParagraphProperties
        {0,0,0,0,0,0}
    };
    static sal_Bool bTwipsSet = false;

    if (!bTwipsSet)
    {
        //  modify PropertyMap to include CONVERT_TWIPS flag for font height
        //  (headers/footers are in twips)

        SfxItemPropertyMapEntry* pEntry = aHdFtPropertyMap_Impl;
        while (pEntry->pName)
        {
            if ( ( pEntry->nWID == EE_CHAR_FONTHEIGHT ||
                   pEntry->nWID == EE_CHAR_FONTHEIGHT_CJK ||
                   pEntry->nWID == EE_CHAR_FONTHEIGHT_CTL ) &&
                 pEntry->nMemberId == MID_FONTHEIGHT )
            {
                pEntry->nMemberId |= CONVERT_TWIPS;
            }

            ++pEntry;
        }
        bTwipsSet = sal_True;
    }
    static SvxItemPropertySet aHdFtPropertySet_Impl( aHdFtPropertyMap_Impl, SdrObject::GetGlobalDrawObjectItemPool() );
    return &aHdFtPropertySet_Impl;
}

//------------------------------------------------------------------------

SC_SIMPLE_SERVICE_INFO( ScHeaderFooterContentObj, "ScHeaderFooterContentObj", "com.sun.star.sheet.HeaderFooterContent" )
SC_SIMPLE_SERVICE_INFO( ScHeaderFooterTextObj, "ScHeaderFooterTextObj", "stardiv.one.Text.Text" )

//------------------------------------------------------------------------

ScHeaderFooterContentObj::ScHeaderFooterContentObj( const EditTextObject* pLeft,
                                                    const EditTextObject* pCenter,
                                                    const EditTextObject* pRight ) :
    mxLeftText(new ScHeaderFooterTextObj(*this, SC_HDFT_LEFT, pLeft)),
    mxCenterText(new ScHeaderFooterTextObj(*this, SC_HDFT_CENTER, pCenter)),
    mxRightText(new ScHeaderFooterTextObj(*this, SC_HDFT_RIGHT, pRight))
{
}

ScHeaderFooterContentObj::~ScHeaderFooterContentObj() {}

const EditTextObject* ScHeaderFooterContentObj::GetLeftEditObject() const
{
    return mxLeftText->GetTextObject();
}

const EditTextObject* ScHeaderFooterContentObj::GetCenterEditObject() const
{
    return mxCenterText->GetTextObject();
}

const EditTextObject* ScHeaderFooterContentObj::GetRightEditObject() const
{
    return mxRightText->GetTextObject();
}

// XHeaderFooterContent

uno::Reference<text::XText> SAL_CALL ScHeaderFooterContentObj::getLeftText()
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference<text::XText> xInt(*mxLeftText, uno::UNO_QUERY);
    return xInt;
}

uno::Reference<text::XText> SAL_CALL ScHeaderFooterContentObj::getCenterText()
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference<text::XText> xInt(*mxCenterText, uno::UNO_QUERY);
    return xInt;
}

uno::Reference<text::XText> SAL_CALL ScHeaderFooterContentObj::getRightText()
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference<text::XText> xInt(*mxRightText, uno::UNO_QUERY);
    return xInt;
}

// XUnoTunnel

sal_Int64 SAL_CALL ScHeaderFooterContentObj::getSomething(
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
    class theScHeaderFooterContentObjUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theScHeaderFooterContentObjUnoTunnelId> {};
}

const uno::Sequence<sal_Int8>& ScHeaderFooterContentObj::getUnoTunnelId()
{
    return theScHeaderFooterContentObjUnoTunnelId::get().getSeq();
}

ScHeaderFooterContentObj* ScHeaderFooterContentObj::getImplementation(
                                const uno::Reference<sheet::XHeaderFooterContent> xObj )
{
    ScHeaderFooterContentObj* pRet = NULL;
    uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
    if (xUT.is())
        pRet = reinterpret_cast<ScHeaderFooterContentObj*>(sal::static_int_cast<sal_IntPtr>(xUT->getSomething(getUnoTunnelId())));
    return pRet;
}


//------------------------------------------------------------------------

ScHeaderFooterTextData::ScHeaderFooterTextData(
    ScHeaderFooterContentObj& rContent, sal_uInt16 nP, const EditTextObject* pTextObj) :
    mpTextObj(pTextObj ? pTextObj->Clone() : NULL),
    rContentObj( rContent ),
    nPart( nP ),
    pEditEngine( NULL ),
    pForwarder( NULL ),
    bDataValid(false)
{
}

ScHeaderFooterTextData::~ScHeaderFooterTextData()
{
    SolarMutexGuard aGuard;     //  needed for EditEngine dtor

    delete pForwarder;
    delete pEditEngine;
}

SvxTextForwarder* ScHeaderFooterTextData::GetTextForwarder()
{
    if (!pEditEngine)
    {
        SfxItemPool* pEnginePool = EditEngine::CreatePool();
        pEnginePool->FreezeIdRanges();
        ScHeaderEditEngine* pHdrEngine = new ScHeaderEditEngine( pEnginePool, sal_True );

        pHdrEngine->EnableUndo( false );
        pHdrEngine->SetRefMapMode( MAP_TWIP );

        //  default font must be set, independently of document
        //  -> use global pool from module

        SfxItemSet aDefaults( pHdrEngine->GetEmptyItemSet() );
        const ScPatternAttr& rPattern = (const ScPatternAttr&)SC_MOD()->GetPool().GetDefaultItem(ATTR_PATTERN);
        rPattern.FillEditItemSet( &aDefaults );
        //  FillEditItemSet adjusts font height to 1/100th mm,
        //  but for header/footer twips is needed, as in the PatternAttr:
        aDefaults.Put( rPattern.GetItem(ATTR_FONT_HEIGHT), EE_CHAR_FONTHEIGHT );
        aDefaults.Put( rPattern.GetItem(ATTR_CJK_FONT_HEIGHT), EE_CHAR_FONTHEIGHT_CJK );
        aDefaults.Put( rPattern.GetItem(ATTR_CTL_FONT_HEIGHT), EE_CHAR_FONTHEIGHT_CTL );
        pHdrEngine->SetDefaults( aDefaults );

        ScHeaderFieldData aData;
        ScHeaderFooterTextObj::FillDummyFieldData( aData );
        pHdrEngine->SetData( aData );

        pEditEngine = pHdrEngine;
        pForwarder = new SvxEditEngineForwarder(*pEditEngine);
    }

    if (bDataValid)
        return pForwarder;

    if (mpTextObj)
        pEditEngine->SetText(*mpTextObj);

    bDataValid = true;
    return pForwarder;
}

void ScHeaderFooterTextData::UpdateData()
{
    if (pEditEngine)
    {
        delete mpTextObj;
        mpTextObj = pEditEngine->CreateTextObject();
    }
}

void ScHeaderFooterTextData::UpdateData(EditEngine& rEditEngine)
{
    delete mpTextObj;
    mpTextObj = rEditEngine.CreateTextObject();
    bDataValid = false;
}

const EditTextObject* ScHeaderFooterTextData::GetTextObject() const
{
    return mpTextObj;
}

//------------------------------------------------------------------------

ScHeaderFooterTextObj::ScHeaderFooterTextObj(
    ScHeaderFooterContentObj& rContent, sal_uInt16 nP, const EditTextObject* pTextObj) :
    aTextData(rContent, nP, pTextObj)
{
    //  ScHeaderFooterTextData acquires rContent
    //  pUnoText is created on demand (getString/setString work without it)
}

void ScHeaderFooterTextObj::CreateUnoText_Impl()
{
    if (!mxUnoText.is())
    {
        //  can't be aggregated because getString/setString is handled here
        ScHeaderFooterEditSource aEditSrc(aTextData);
        mxUnoText.set(new SvxUnoText(&aEditSrc, lcl_GetHdFtPropertySet(), uno::Reference<text::XText>()));
    }
}

ScHeaderFooterTextObj::~ScHeaderFooterTextObj() {}

const EditTextObject* ScHeaderFooterTextObj::GetTextObject() const
{
    return aTextData.GetTextObject();
}

const SvxUnoText& ScHeaderFooterTextObj::GetUnoText()
{
    if (!mxUnoText.is())
        CreateUnoText_Impl();
    return *mxUnoText;
}

// XText

uno::Reference<text::XTextCursor> SAL_CALL ScHeaderFooterTextObj::createTextCursor()
                                                    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return new ScHeaderFooterTextCursor( *this );
}

uno::Reference<text::XTextCursor> SAL_CALL ScHeaderFooterTextObj::createTextCursorByRange(
                                    const uno::Reference<text::XTextRange>& aTextPosition )
                                                    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (!mxUnoText.is())
        CreateUnoText_Impl();
    return mxUnoText->createTextCursorByRange(aTextPosition);
    //! wie ScCellObj::createTextCursorByRange, wenn SvxUnoTextRange_getReflection verfuegbar
}

void ScHeaderFooterTextObj::FillDummyFieldData( ScHeaderFieldData& rData )
{
    rtl::OUString aDummy("???");
    rData.aTitle        = aDummy;
    rData.aLongDocName  = aDummy;
    rData.aShortDocName = aDummy;
    rData.aTabName      = aDummy;
    rData.nPageNo       = 1;
    rData.nTotalPages   = 99;
}

rtl::OUString SAL_CALL ScHeaderFooterTextObj::getString() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    rtl::OUString aRet;
    const EditTextObject* pData;

    sal_uInt16 nPart = aTextData.GetPart();
    ScHeaderFooterContentObj& rContentObj = aTextData.GetContentObj();

    if (nPart == SC_HDFT_LEFT)
        pData = rContentObj.GetLeftEditObject();
    else if (nPart == SC_HDFT_CENTER)
        pData = rContentObj.GetCenterEditObject();
    else
        pData = rContentObj.GetRightEditObject();
    if (pData)
    {
        // for pure text, no font info is needed in pool defaults
        ScHeaderEditEngine aEditEngine( EditEngine::CreatePool(), sal_True );

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
    SolarMutexGuard aGuard;
    String aString(aText);

    // for pure text, no font info is needed in pool defaults
    ScHeaderEditEngine aEditEngine(EditEngine::CreatePool(), true);
    aEditEngine.SetText( aString );
    aTextData.UpdateData(aEditEngine);
}

void SAL_CALL ScHeaderFooterTextObj::insertString( const uno::Reference<text::XTextRange>& xRange,
                                            const rtl::OUString& aString, sal_Bool bAbsorb )
                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (!mxUnoText.is())
        CreateUnoText_Impl();
    mxUnoText->insertString( xRange, aString, bAbsorb );
}

void SAL_CALL ScHeaderFooterTextObj::insertControlCharacter(
                                            const uno::Reference<text::XTextRange>& xRange,
                                            sal_Int16 nControlCharacter, sal_Bool bAbsorb )
                                throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (!mxUnoText.is())
        CreateUnoText_Impl();
    mxUnoText->insertControlCharacter( xRange, nControlCharacter, bAbsorb );
}

void SAL_CALL ScHeaderFooterTextObj::insertTextContent(
                                            const uno::Reference<text::XTextRange >& xRange,
                                            const uno::Reference<text::XTextContent >& xContent,
                                            sal_Bool bAbsorb )
                                throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if ( xContent.is() && xRange.is() )
    {
        ScEditFieldObj* pHeaderField = ScEditFieldObj::getImplementation( xContent );

        SvxUnoTextRangeBase* pTextRange =
            ScHeaderFooterTextCursor::getImplementation( xRange );

        if ( pHeaderField && !pHeaderField->IsInserted() && pTextRange )
        {
            SvxEditSource* pEditSource = pTextRange->GetEditSource();
            ESelection aSelection(pTextRange->GetSelection());

            if (!bAbsorb)
            {
                //  don't replace -> append at end
                aSelection.Adjust();
                aSelection.nStartPara = aSelection.nEndPara;
                aSelection.nStartPos  = aSelection.nEndPos;
            }

            SvxFieldItem aItem(pHeaderField->CreateFieldItem());

            SvxTextForwarder* pForwarder = pEditSource->GetTextForwarder();
            pForwarder->QuickInsertField( aItem, aSelection );
            pEditSource->UpdateData();

            //  neue Selektion: ein Zeichen
            aSelection.Adjust();
            aSelection.nEndPara = aSelection.nStartPara;
            aSelection.nEndPos = aSelection.nStartPos + 1;

            uno::Reference<text::XTextRange> xTextRange;
            switch (aTextData.GetPart())
            {
                case SC_HDFT_LEFT:
                {
                    uno::Reference<text::XTextRange> xTemp(
                        aTextData.GetContentObj().getLeftText(), uno::UNO_QUERY);
                    xTextRange = xTemp;
                }
                break;
                case SC_HDFT_CENTER:
                {
                    uno::Reference<text::XTextRange> xTemp(
                        aTextData.GetContentObj().getCenterText(), uno::UNO_QUERY);
                    xTextRange = xTemp;
                }
                break;
                case SC_HDFT_RIGHT:
                {
                    uno::Reference<text::XTextRange> xTemp(
                        aTextData.GetContentObj().getRightText(), uno::UNO_QUERY);
                    xTextRange = xTemp;
                }
                break;
            }

            pHeaderField->InitDoc(xTextRange, new ScHeaderFooterEditSource(aTextData), aSelection);

            //  for bAbsorb=FALSE, the new selection must be behind the inserted content
            //  (the xml filter relies on this)
            if (!bAbsorb)
                aSelection.nStartPos = aSelection.nEndPos;

            pTextRange->SetSelection( aSelection );

            return;
        }
    }

    if (!mxUnoText.is())
        CreateUnoText_Impl();
    mxUnoText->insertTextContent( xRange, xContent, bAbsorb );
}

void SAL_CALL ScHeaderFooterTextObj::removeTextContent(
                                            const uno::Reference<text::XTextContent>& xContent )
                                throw(container::NoSuchElementException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if ( xContent.is() )
    {
        ScEditFieldObj* pHeaderField = ScEditFieldObj::getImplementation(xContent);
        if ( pHeaderField && pHeaderField->IsInserted() )
        {
            //! Testen, ob das Feld in dieser Zelle ist
            pHeaderField->DeleteField();
            return;
        }
    }
    if (!mxUnoText.is())
        CreateUnoText_Impl();
    mxUnoText->removeTextContent( xContent );
}

uno::Reference<text::XText> SAL_CALL ScHeaderFooterTextObj::getText() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (!mxUnoText.is())
        CreateUnoText_Impl();
    return mxUnoText->getText();
}

uno::Reference<text::XTextRange> SAL_CALL ScHeaderFooterTextObj::getStart() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (!mxUnoText.is())
        CreateUnoText_Impl();
    return mxUnoText->getStart();
}

uno::Reference<text::XTextRange> SAL_CALL ScHeaderFooterTextObj::getEnd() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (!mxUnoText.is())
        CreateUnoText_Impl();
    return mxUnoText->getEnd();
}

// XTextFieldsSupplier

uno::Reference<container::XEnumerationAccess> SAL_CALL ScHeaderFooterTextObj::getTextFields()
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    // all fields
    return new ScHeaderFieldsObj(aTextData);
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
    SolarMutexGuard aGuard;
    if (!mxUnoText.is())
        CreateUnoText_Impl();
    mxUnoText->moveTextRange( xRange, nParagraphs );
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScHeaderFooterTextObj::createEnumeration()
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (!mxUnoText.is())
        CreateUnoText_Impl();
    return mxUnoText->createEnumeration();
}

// XElementAccess

uno::Type SAL_CALL ScHeaderFooterTextObj::getElementType() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (!mxUnoText.is())
        CreateUnoText_Impl();
    return mxUnoText->getElementType();
}

sal_Bool SAL_CALL ScHeaderFooterTextObj::hasElements() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (!mxUnoText.is())
        CreateUnoText_Impl();
    return mxUnoText->hasElements();
}

//------------------------------------------------------------------------

ScCellTextCursor::ScCellTextCursor(const ScCellTextCursor& rOther) :
    SvxUnoTextCursor( rOther ),
    rTextObj( rOther.rTextObj )
{
    rTextObj.acquire();
}

ScCellTextCursor::ScCellTextCursor(ScCellObj& rText) :
    SvxUnoTextCursor( rText.GetUnoText() ),
    rTextObj( rText )
{
    rTextObj.acquire();
}

ScCellTextCursor::~ScCellTextCursor() throw()
{
    rTextObj.release();
}

// SvxUnoTextCursor methods reimplemented here to return the right objects:

uno::Reference<text::XText> SAL_CALL ScCellTextCursor::getText() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return &rTextObj;
}

uno::Reference<text::XTextRange> SAL_CALL ScCellTextCursor::getStart() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    //! use other object for range than cursor?

    ScCellTextCursor* pNew = new ScCellTextCursor( *this );
    uno::Reference<text::XTextRange> xRange( static_cast<SvxUnoTextRangeBase*>(pNew) );

    ESelection aNewSel(GetSelection());
    aNewSel.nEndPara = aNewSel.nStartPara;
    aNewSel.nEndPos  = aNewSel.nStartPos;
    pNew->SetSelection( aNewSel );

    return xRange;
}

uno::Reference<text::XTextRange> SAL_CALL ScCellTextCursor::getEnd() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    //! use other object for range than cursor?

    ScCellTextCursor* pNew = new ScCellTextCursor( *this );
    uno::Reference<text::XTextRange> xRange( static_cast<SvxUnoTextRangeBase*>(pNew) );

    ESelection aNewSel(GetSelection());
    aNewSel.nStartPara = aNewSel.nEndPara;
    aNewSel.nStartPos  = aNewSel.nEndPos;
    pNew->SetSelection( aNewSel );

    return xRange;
}

// XUnoTunnel

sal_Int64 SAL_CALL ScCellTextCursor::getSomething(
                const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException)
{
    if ( rId.getLength() == 16 &&
          0 == memcmp( getUnoTunnelId().getConstArray(),
                                    rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
    }
    return SvxUnoTextCursor::getSomething( rId );
}

namespace
{
    class theScCellTextCursorUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theScCellTextCursorUnoTunnelId> {};
}

const uno::Sequence<sal_Int8>& ScCellTextCursor::getUnoTunnelId()
{
    return theScCellTextCursorUnoTunnelId::get().getSeq();
}

ScCellTextCursor* ScCellTextCursor::getImplementation( const uno::Reference<uno::XInterface> xObj )
{
    ScCellTextCursor* pRet = NULL;
    uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
    if (xUT.is())
        pRet = reinterpret_cast<ScCellTextCursor*>(sal::static_int_cast<sal_IntPtr>(xUT->getSomething(getUnoTunnelId())));
    return pRet;
}

//------------------------------------------------------------------------

ScHeaderFooterTextCursor::ScHeaderFooterTextCursor(const ScHeaderFooterTextCursor& rOther) :
    SvxUnoTextCursor( rOther ),
    rTextObj( rOther.rTextObj )
{
    rTextObj.acquire();
}

ScHeaderFooterTextCursor::ScHeaderFooterTextCursor(ScHeaderFooterTextObj& rText) :
    SvxUnoTextCursor( rText.GetUnoText() ),
    rTextObj( rText )
{
    rTextObj.acquire();
}

ScHeaderFooterTextCursor::~ScHeaderFooterTextCursor() throw()
{
    rTextObj.release();
}

// SvxUnoTextCursor methods reimplemented here to return the right objects:

uno::Reference<text::XText> SAL_CALL ScHeaderFooterTextCursor::getText() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return &rTextObj;
}

uno::Reference<text::XTextRange> SAL_CALL ScHeaderFooterTextCursor::getStart() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    //! use other object for range than cursor?

    ScHeaderFooterTextCursor* pNew = new ScHeaderFooterTextCursor( *this );
    uno::Reference<text::XTextRange> xRange( static_cast<SvxUnoTextRangeBase*>(pNew) );

    ESelection aNewSel(GetSelection());
    aNewSel.nEndPara = aNewSel.nStartPara;
    aNewSel.nEndPos  = aNewSel.nStartPos;
    pNew->SetSelection( aNewSel );

    return xRange;
}

uno::Reference<text::XTextRange> SAL_CALL ScHeaderFooterTextCursor::getEnd() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    //! use other object for range than cursor?

    ScHeaderFooterTextCursor* pNew = new ScHeaderFooterTextCursor( *this );
    uno::Reference<text::XTextRange> xRange( static_cast<SvxUnoTextRangeBase*>(pNew) );

    ESelection aNewSel(GetSelection());
    aNewSel.nStartPara = aNewSel.nEndPara;
    aNewSel.nStartPos  = aNewSel.nEndPos;
    pNew->SetSelection( aNewSel );

    return xRange;
}

// XUnoTunnel

sal_Int64 SAL_CALL ScHeaderFooterTextCursor::getSomething(
                const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException)
{
    if ( rId.getLength() == 16 &&
          0 == memcmp( getUnoTunnelId().getConstArray(),
                                    rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
    }
    return SvxUnoTextCursor::getSomething( rId );
}

namespace
{
    class theScHeaderFooterTextCursorUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theScHeaderFooterTextCursorUnoTunnelId> {};
}

const uno::Sequence<sal_Int8>& ScHeaderFooterTextCursor::getUnoTunnelId()
{
    return theScHeaderFooterTextCursorUnoTunnelId::get().getSeq();
}

ScHeaderFooterTextCursor* ScHeaderFooterTextCursor::getImplementation(
                                const uno::Reference<uno::XInterface> xObj )
{
    ScHeaderFooterTextCursor* pRet = NULL;
    uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
    if (xUT.is())
        pRet = reinterpret_cast<ScHeaderFooterTextCursor*>(sal::static_int_cast<sal_IntPtr>(xUT->getSomething(getUnoTunnelId())));
    return pRet;
}

//------------------------------------------------------------------------

ScDrawTextCursor::ScDrawTextCursor(const ScDrawTextCursor& rOther) :
    SvxUnoTextCursor( rOther ),
    xParentText( rOther.xParentText )
{
}

ScDrawTextCursor::ScDrawTextCursor( const uno::Reference<text::XText>& xParent,
                                    const SvxUnoTextBase& rText ) :
    SvxUnoTextCursor( rText ),
    xParentText( xParent )

{
}

ScDrawTextCursor::~ScDrawTextCursor() throw()
{
}

// SvxUnoTextCursor methods reimplemented here to return the right objects:

uno::Reference<text::XText> SAL_CALL ScDrawTextCursor::getText() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return xParentText;
}

uno::Reference<text::XTextRange> SAL_CALL ScDrawTextCursor::getStart() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    //! use other object for range than cursor?

    ScDrawTextCursor* pNew = new ScDrawTextCursor( *this );
    uno::Reference<text::XTextRange> xRange( static_cast<SvxUnoTextRangeBase*>(pNew) );

    ESelection aNewSel(GetSelection());
    aNewSel.nEndPara = aNewSel.nStartPara;
    aNewSel.nEndPos  = aNewSel.nStartPos;
    pNew->SetSelection( aNewSel );

    return xRange;
}

uno::Reference<text::XTextRange> SAL_CALL ScDrawTextCursor::getEnd() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    //! use other object for range than cursor?

    ScDrawTextCursor* pNew = new ScDrawTextCursor( *this );
    uno::Reference<text::XTextRange> xRange( static_cast<SvxUnoTextRangeBase*>(pNew) );

    ESelection aNewSel(GetSelection());
    aNewSel.nStartPara = aNewSel.nEndPara;
    aNewSel.nStartPos  = aNewSel.nEndPos;
    pNew->SetSelection( aNewSel );

    return xRange;
}

// XUnoTunnel

sal_Int64 SAL_CALL ScDrawTextCursor::getSomething(
                const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException)
{
    if ( rId.getLength() == 16 &&
          0 == memcmp( getUnoTunnelId().getConstArray(),
                                    rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
    }
    return SvxUnoTextCursor::getSomething( rId );
}

namespace
{
    class theScDrawTextCursorUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theScDrawTextCursorUnoTunnelId> {};
}

const uno::Sequence<sal_Int8>& ScDrawTextCursor::getUnoTunnelId()
{
    return theScDrawTextCursorUnoTunnelId::get().getSeq();
}

ScDrawTextCursor* ScDrawTextCursor::getImplementation( const uno::Reference<uno::XInterface> xObj )
{
    ScDrawTextCursor* pRet = NULL;
    uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
    if (xUT.is())
        pRet = reinterpret_cast<ScDrawTextCursor*>(sal::static_int_cast<sal_IntPtr>(xUT->getSomething(getUnoTunnelId())));
    return pRet;
}

//------------------------------------------------------------------------

ScSimpleEditSourceHelper::ScSimpleEditSourceHelper()
{
    SfxItemPool* pEnginePool = EditEngine::CreatePool();
    pEnginePool->SetDefaultMetric( SFX_MAPUNIT_100TH_MM );
    pEnginePool->FreezeIdRanges();

    pEditEngine = new ScFieldEditEngine(NULL, pEnginePool, NULL, true);     // TRUE: become owner of pool
    pForwarder = new SvxEditEngineForwarder( *pEditEngine );
    pOriginalSource = new ScSimpleEditSource( pForwarder );
}

ScSimpleEditSourceHelper::~ScSimpleEditSourceHelper()
{
    SolarMutexGuard aGuard;     //  needed for EditEngine dtor

    delete pOriginalSource;
    delete pForwarder;
    delete pEditEngine;
}

ScEditEngineTextObj::ScEditEngineTextObj() :
    SvxUnoText( GetOriginalSource(), ScCellObj::GetEditPropertySet(), uno::Reference<text::XText>() )
{
}

ScEditEngineTextObj::~ScEditEngineTextObj() throw()
{
}

void ScEditEngineTextObj::SetText( const EditTextObject& rTextObject )
{
    GetEditEngine()->SetText( rTextObject );

    ESelection aSel;
    ::GetSelection( aSel, GetEditSource()->GetTextForwarder() );
    SetSelection( aSel );
}

EditTextObject* ScEditEngineTextObj::CreateTextObject()
{
    return GetEditEngine()->CreateTextObject();
}

//------------------------------------------------------------------------

ScCellTextData::ScCellTextData(ScDocShell* pDocSh, const ScAddress& rP) :
    pDocShell( pDocSh ),
    aCellPos( rP ),
    pEditEngine( NULL ),
    pForwarder( NULL ),
    pOriginalSource( NULL ),
    bDataValid( false ),
    bInUpdate( false ),
    bDirty( false ),
    bDoUpdate( sal_True )
{
    if (pDocShell)
        pDocShell->GetDocument()->AddUnoObject(*this);
}

ScCellTextData::~ScCellTextData()
{
    SolarMutexGuard aGuard;     //  needed for EditEngine dtor

    if (pDocShell)
    {
        pDocShell->GetDocument()->RemoveUnoObject(*this);
        pDocShell->GetDocument()->DisposeFieldEditEngine(pEditEngine);
    }
    else
        delete pEditEngine;

    delete pForwarder;

    delete pOriginalSource;
}

ScCellEditSource* ScCellTextData::GetOriginalSource()
{
    if (!pOriginalSource)
        pOriginalSource = new ScCellEditSource(pDocShell, aCellPos);
    return pOriginalSource;
}

void ScCellTextData::GetCellText(const ScAddress& rCellPos, String& rText)
{
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        pDoc->GetInputString( rCellPos.Col(), rCellPos.Row(), rCellPos.Tab(), rText );
    }
}

SvxTextForwarder* ScCellTextData::GetTextForwarder()
{
    if (!pEditEngine)
    {
        if ( pDocShell )
        {
            ScDocument* pDoc = pDocShell->GetDocument();
            pEditEngine = pDoc->CreateFieldEditEngine();
        }
        else
        {
            SfxItemPool* pEnginePool = EditEngine::CreatePool();
            pEnginePool->FreezeIdRanges();
            pEditEngine = new ScFieldEditEngine(NULL, pEnginePool, NULL, true);
        }
        //  currently, GetPortions doesn't work if UpdateMode is sal_False,
        //  this will be fixed (in EditEngine) by src600
//      pEditEngine->SetUpdateMode( sal_False );
        pEditEngine->EnableUndo( false );
        if (pDocShell)
            pEditEngine->SetRefDevice(pDocShell->GetRefDevice());
        else
            pEditEngine->SetRefMapMode( MAP_100TH_MM );
        pForwarder = new SvxEditEngineForwarder(*pEditEngine);
    }

    if (bDataValid)
        return pForwarder;

    String aText;

    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();

        SfxItemSet aDefaults( pEditEngine->GetEmptyItemSet() );
        if( const ScPatternAttr* pPattern =
                pDoc->GetPattern( aCellPos.Col(), aCellPos.Row(), aCellPos.Tab() ) )
        {
            pPattern->FillEditItemSet( &aDefaults );
            pPattern->FillEditParaItems( &aDefaults );  // including alignment etc. (for reading)
        }

        const ScBaseCell* pCell = pDoc->GetCell( aCellPos );
        if ( pCell && pCell->GetCellType() == CELLTYPE_EDIT )
            pEditEngine->SetTextNewDefaults( *((const ScEditCell*)pCell)->GetData(), aDefaults );
        else
        {
            GetCellText( aCellPos, aText );
            if (aText.Len())
                pEditEngine->SetTextNewDefaults( aText, aDefaults );
            else
                pEditEngine->SetDefaults(aDefaults);
        }
    }

    bDataValid = sal_True;
    return pForwarder;
}

void ScCellTextData::UpdateData()
{
    if ( bDoUpdate )
    {
        OSL_ENSURE(pEditEngine != NULL, "no EditEngine for UpdateData()");
        if ( pDocShell && pEditEngine )
        {
            //  during the own UpdateData call, bDataValid must not be reset,
            //  or things like attributes after the text would be lost
            //  (are not stored in the cell)
            bInUpdate = sal_True;   // prevents bDataValid from being reset
            pDocShell->GetDocFunc().PutData( aCellPos, *pEditEngine, false, sal_True ); // always as text

            bInUpdate = false;
            bDirty = false;
        }
    }
    else
        bDirty = sal_True;
}

void ScCellTextData::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.ISA( ScUpdateRefHint ) )
    {
//        const ScUpdateRefHint& rRef = (const ScUpdateRefHint&)rHint;

        //! Ref-Update
    }
    else if ( rHint.ISA( SfxSimpleHint ) )
    {
        sal_uLong nId = ((const SfxSimpleHint&)rHint).GetId();
        if ( nId == SFX_HINT_DYING )
        {
            pDocShell = NULL;                       // invalid now

            DELETEZ( pForwarder );
            DELETEZ( pEditEngine );     // EditEngine uses document's pool
        }
        else if ( nId == SFX_HINT_DATACHANGED )
        {
            if (!bInUpdate)                         // not for own UpdateData calls
                bDataValid = false;                 // text has to be read from the cell again
        }
    }
}

ScCellTextObj::ScCellTextObj(ScDocShell* pDocSh, const ScAddress& rP) :
    ScCellTextData( pDocSh, rP ),
    SvxUnoText( GetOriginalSource(), ScCellObj::GetEditPropertySet(), uno::Reference<text::XText>() )
{
}

ScCellTextObj::~ScCellTextObj() throw()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
