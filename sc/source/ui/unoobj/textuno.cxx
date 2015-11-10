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
#include "formulacell.hxx"
#include "docfunc.hxx"
#include "scmod.hxx"

using namespace com::sun::star;

static const SvxItemPropertySet * lcl_GetHdFtPropertySet()
{
    static SfxItemPropertyMapEntry aHdFtPropertyMap_Impl[] =
    {
        SVX_UNOEDIT_CHAR_PROPERTIES,
        SVX_UNOEDIT_FONT_PROPERTIES,
        SVX_UNOEDIT_PARA_PROPERTIES,
        SVX_UNOEDIT_NUMBERING_PROPERTIE,    // for completeness of service ParagraphProperties
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    static bool bTwipsSet = false;

    if (!bTwipsSet)
    {
        //  modify PropertyMap to include CONVERT_TWIPS flag for font height
        //  (headers/footers are in twips)

        SfxItemPropertyMapEntry* pEntry = aHdFtPropertyMap_Impl;
        while (!pEntry->aName.isEmpty())
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
        bTwipsSet = true;
    }
    static SvxItemPropertySet aHdFtPropertySet_Impl( aHdFtPropertyMap_Impl, SdrObject::GetGlobalDrawObjectItemPool() );
    return &aHdFtPropertySet_Impl;
}

SC_SIMPLE_SERVICE_INFO( ScHeaderFooterContentObj, "ScHeaderFooterContentObj", "com.sun.star.sheet.HeaderFooterContent" )
SC_SIMPLE_SERVICE_INFO( ScHeaderFooterTextObj, "ScHeaderFooterTextObj", "stardiv.one.Text.Text" )

ScHeaderFooterContentObj::ScHeaderFooterContentObj( const EditTextObject* pLeft,
                                                    const EditTextObject* pCenter,
                                                    const EditTextObject* pRight ) :
    mxLeftText(new ScHeaderFooterTextObj(this, SC_HDFT_LEFT, pLeft)),
    mxCenterText(new ScHeaderFooterTextObj(this, SC_HDFT_CENTER, pCenter)),
    mxRightText(new ScHeaderFooterTextObj(this, SC_HDFT_RIGHT, pRight))
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
                                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference<text::XText> xInt(*mxLeftText, uno::UNO_QUERY);
    return xInt;
}

uno::Reference<text::XText> SAL_CALL ScHeaderFooterContentObj::getCenterText()
                                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference<text::XText> xInt(*mxCenterText, uno::UNO_QUERY);
    return xInt;
}

uno::Reference<text::XText> SAL_CALL ScHeaderFooterContentObj::getRightText()
                                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference<text::XText> xInt(*mxRightText, uno::UNO_QUERY);
    return xInt;
}

// XUnoTunnel

sal_Int64 SAL_CALL ScHeaderFooterContentObj::getSomething(
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
    class theScHeaderFooterContentObjUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theScHeaderFooterContentObjUnoTunnelId> {};
}

const uno::Sequence<sal_Int8>& ScHeaderFooterContentObj::getUnoTunnelId()
{
    return theScHeaderFooterContentObjUnoTunnelId::get().getSeq();
}

rtl::Reference<ScHeaderFooterContentObj> ScHeaderFooterContentObj::getImplementation(
                                const uno::Reference<sheet::XHeaderFooterContent>& rObj)
{
    rtl::Reference<ScHeaderFooterContentObj> pRet;
    uno::Reference<lang::XUnoTunnel> xUT(rObj, uno::UNO_QUERY);
    if (xUT.is())
        pRet = reinterpret_cast<ScHeaderFooterContentObj*>(sal::static_int_cast<sal_IntPtr>(xUT->getSomething(getUnoTunnelId())));
    return pRet;
}

ScHeaderFooterTextData::ScHeaderFooterTextData(
    rtl::Reference<ScHeaderFooterContentObj> const & rContent, sal_uInt16 nP, const EditTextObject* pTextObj) :
    mpTextObj(pTextObj ? pTextObj->Clone() : nullptr),
    rContentObj( rContent ),
    nPart( nP ),
    pEditEngine( nullptr ),
    pForwarder( nullptr ),
    bDataValid(false)
{
}

ScHeaderFooterTextData::~ScHeaderFooterTextData()
{
    SolarMutexGuard aGuard;     //  needed for EditEngine dtor

    delete pForwarder;
    delete pEditEngine;
    delete mpTextObj;
}

SvxTextForwarder* ScHeaderFooterTextData::GetTextForwarder()
{
    if (!pEditEngine)
    {
        SfxItemPool* pEnginePool = EditEngine::CreatePool();
        pEnginePool->FreezeIdRanges();
        ScHeaderEditEngine* pHdrEngine = new ScHeaderEditEngine( pEnginePool, true );

        pHdrEngine->EnableUndo( false );
        pHdrEngine->SetRefMapMode( MAP_TWIP );

        //  default font must be set, independently of document
        //  -> use global pool from module

        SfxItemSet aDefaults( pHdrEngine->GetEmptyItemSet() );
        const ScPatternAttr& rPattern = static_cast<const ScPatternAttr&>(SC_MOD()->GetPool().GetDefaultItem(ATTR_PATTERN));
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

ScHeaderFooterTextObj::ScHeaderFooterTextObj(
    rtl::Reference<ScHeaderFooterContentObj> const & rContent, sal_uInt16 nP, const EditTextObject* pTextObj) :
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
                                                    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return new ScHeaderFooterTextCursor( *this );
}

uno::Reference<text::XTextCursor> SAL_CALL ScHeaderFooterTextObj::createTextCursorByRange(
                                    const uno::Reference<text::XTextRange>& aTextPosition )
                                                    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (!mxUnoText.is())
        CreateUnoText_Impl();
    return mxUnoText->createTextCursorByRange(aTextPosition);
    //! wie ScCellObj::createTextCursorByRange, wenn SvxUnoTextRange_getReflection verfuegbar
}

void ScHeaderFooterTextObj::FillDummyFieldData( ScHeaderFieldData& rData )
{
    OUString aDummy("???");
    rData.aTitle        = aDummy;
    rData.aLongDocName  = aDummy;
    rData.aShortDocName = aDummy;
    rData.aTabName      = aDummy;
    rData.nPageNo       = 1;
    rData.nTotalPages   = 99;
}

OUString SAL_CALL ScHeaderFooterTextObj::getString() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    OUString aRet;
    const EditTextObject* pData;

    sal_uInt16 nPart = aTextData.GetPart();
    rtl::Reference<ScHeaderFooterContentObj> rContentObj = aTextData.GetContentObj();

    if (nPart == SC_HDFT_LEFT)
        pData = rContentObj->GetLeftEditObject();
    else if (nPart == SC_HDFT_CENTER)
        pData = rContentObj->GetCenterEditObject();
    else
        pData = rContentObj->GetRightEditObject();
    if (pData)
    {
        // for pure text, no font info is needed in pool defaults
        ScHeaderEditEngine aEditEngine( EditEngine::CreatePool(), true );

        ScHeaderFieldData aData;
        FillDummyFieldData( aData );
        aEditEngine.SetData( aData );

        aEditEngine.SetText(*pData);
        aRet = ScEditUtil::GetSpaceDelimitedString( aEditEngine );
    }
    return aRet;
}

void SAL_CALL ScHeaderFooterTextObj::setString( const OUString& aText ) throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    OUString aString(aText);

    // for pure text, no font info is needed in pool defaults
    ScHeaderEditEngine aEditEngine(EditEngine::CreatePool(), true);
    aEditEngine.SetText( aString );
    aTextData.UpdateData(aEditEngine);
}

void SAL_CALL ScHeaderFooterTextObj::insertString( const uno::Reference<text::XTextRange>& xRange,
                                            const OUString& aString, sal_Bool bAbsorb )
                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (!mxUnoText.is())
        CreateUnoText_Impl();
    mxUnoText->insertString( xRange, aString, bAbsorb );
}

void SAL_CALL ScHeaderFooterTextObj::insertControlCharacter(
                                            const uno::Reference<text::XTextRange>& xRange,
                                            sal_Int16 nControlCharacter, sal_Bool bAbsorb )
                                throw(lang::IllegalArgumentException, uno::RuntimeException, std::exception)
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
                                throw(lang::IllegalArgumentException, uno::RuntimeException, std::exception)
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
                        aTextData.GetContentObj()->getLeftText(), uno::UNO_QUERY);
                    xTextRange = xTemp;
                }
                break;
                case SC_HDFT_CENTER:
                {
                    uno::Reference<text::XTextRange> xTemp(
                        aTextData.GetContentObj()->getCenterText(), uno::UNO_QUERY);
                    xTextRange = xTemp;
                }
                break;
                case SC_HDFT_RIGHT:
                {
                    uno::Reference<text::XTextRange> xTemp(
                        aTextData.GetContentObj()->getRightText(), uno::UNO_QUERY);
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
                                throw(container::NoSuchElementException, uno::RuntimeException, std::exception)
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

uno::Reference<text::XText> SAL_CALL ScHeaderFooterTextObj::getText() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (!mxUnoText.is())
        CreateUnoText_Impl();
    return mxUnoText->getText();
}

uno::Reference<text::XTextRange> SAL_CALL ScHeaderFooterTextObj::getStart() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (!mxUnoText.is())
        CreateUnoText_Impl();
    return mxUnoText->getStart();
}

uno::Reference<text::XTextRange> SAL_CALL ScHeaderFooterTextObj::getEnd() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (!mxUnoText.is())
        CreateUnoText_Impl();
    return mxUnoText->getEnd();
}

// XTextFieldsSupplier

uno::Reference<container::XEnumerationAccess> SAL_CALL ScHeaderFooterTextObj::getTextFields()
                                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    // all fields
    return new ScHeaderFieldsObj(aTextData);
}

uno::Reference<container::XNameAccess> SAL_CALL ScHeaderFooterTextObj::getTextFieldMasters()
                                                throw(uno::RuntimeException, std::exception)
{
    //  sowas gibts nicht im Calc (?)
    return nullptr;
}

// XTextRangeMover

void SAL_CALL ScHeaderFooterTextObj::moveTextRange(
                                        const uno::Reference<text::XTextRange>& xRange,
                                        sal_Int16 nParagraphs )
                                        throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (!mxUnoText.is())
        CreateUnoText_Impl();
    mxUnoText->moveTextRange( xRange, nParagraphs );
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScHeaderFooterTextObj::createEnumeration()
                                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (!mxUnoText.is())
        CreateUnoText_Impl();
    return mxUnoText->createEnumeration();
}

// XElementAccess

uno::Type SAL_CALL ScHeaderFooterTextObj::getElementType() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (!mxUnoText.is())
        CreateUnoText_Impl();
    return mxUnoText->getElementType();
}

sal_Bool SAL_CALL ScHeaderFooterTextObj::hasElements() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (!mxUnoText.is())
        CreateUnoText_Impl();
    return mxUnoText->hasElements();
}

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

uno::Reference<text::XText> SAL_CALL ScCellTextCursor::getText() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return &rTextObj;
}

uno::Reference<text::XTextRange> SAL_CALL ScCellTextCursor::getStart() throw(uno::RuntimeException, std::exception)
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

uno::Reference<text::XTextRange> SAL_CALL ScCellTextCursor::getEnd() throw(uno::RuntimeException, std::exception)
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
                const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException, std::exception)
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

ScCellTextCursor* ScCellTextCursor::getImplementation(const uno::Reference<uno::XInterface>& rObj)
{
    ScCellTextCursor* pRet = nullptr;
    uno::Reference<lang::XUnoTunnel> xUT(rObj, uno::UNO_QUERY);
    if (xUT.is())
        pRet = reinterpret_cast<ScCellTextCursor*>(sal::static_int_cast<sal_IntPtr>(xUT->getSomething(getUnoTunnelId())));
    return pRet;
}

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

uno::Reference<text::XText> SAL_CALL ScHeaderFooterTextCursor::getText() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return &rTextObj;
}

uno::Reference<text::XTextRange> SAL_CALL ScHeaderFooterTextCursor::getStart() throw(uno::RuntimeException, std::exception)
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

uno::Reference<text::XTextRange> SAL_CALL ScHeaderFooterTextCursor::getEnd() throw(uno::RuntimeException, std::exception)
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
                const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException, std::exception)
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
                                const uno::Reference<uno::XInterface>& rObj)
{
    ScHeaderFooterTextCursor* pRet = nullptr;
    uno::Reference<lang::XUnoTunnel> xUT(rObj, uno::UNO_QUERY);
    if (xUT.is())
        pRet = reinterpret_cast<ScHeaderFooterTextCursor*>(sal::static_int_cast<sal_IntPtr>(xUT->getSomething(getUnoTunnelId())));
    return pRet;
}

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

uno::Reference<text::XText> SAL_CALL ScDrawTextCursor::getText() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return xParentText;
}

uno::Reference<text::XTextRange> SAL_CALL ScDrawTextCursor::getStart() throw(uno::RuntimeException, std::exception)
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

uno::Reference<text::XTextRange> SAL_CALL ScDrawTextCursor::getEnd() throw(uno::RuntimeException, std::exception)
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
                const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException, std::exception)
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

ScDrawTextCursor* ScDrawTextCursor::getImplementation(const uno::Reference<uno::XInterface>& rObj)
{
    ScDrawTextCursor* pRet = nullptr;
    uno::Reference<lang::XUnoTunnel> xUT(rObj, uno::UNO_QUERY);
    if (xUT.is())
        pRet = reinterpret_cast<ScDrawTextCursor*>(sal::static_int_cast<sal_IntPtr>(xUT->getSomething(getUnoTunnelId())));
    return pRet;
}

ScSimpleEditSourceHelper::ScSimpleEditSourceHelper()
{
    SfxItemPool* pEnginePool = EditEngine::CreatePool();
    pEnginePool->SetDefaultMetric( SFX_MAPUNIT_100TH_MM );
    pEnginePool->FreezeIdRanges();

    pEditEngine = new ScFieldEditEngine(nullptr, pEnginePool, nullptr, true);     // TRUE: become owner of pool
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

ScCellTextData::ScCellTextData(ScDocShell* pDocSh, const ScAddress& rP) :
    pDocShell( pDocSh ),
    aCellPos( rP ),
    pEditEngine( nullptr ),
    pForwarder( nullptr ),
    pOriginalSource( nullptr ),
    bDataValid( false ),
    bInUpdate( false ),
    bDirty( false ),
    bDoUpdate( true )
{
    if (pDocShell)
        pDocShell->GetDocument().AddUnoObject(*this);
}

ScCellTextData::~ScCellTextData()
{
    SolarMutexGuard aGuard;     //  needed for EditEngine dtor

    if (pDocShell)
    {
        pDocShell->GetDocument().RemoveUnoObject(*this);
        pDocShell->GetDocument().DisposeFieldEditEngine(pEditEngine);
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

void ScCellTextData::GetCellText(const ScAddress& rCellPos, OUString& rText)
{
    if (pDocShell)
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        rDoc.GetInputString( rCellPos.Col(), rCellPos.Row(), rCellPos.Tab(), rText );
    }
}

SvxTextForwarder* ScCellTextData::GetTextForwarder()
{
    if (!pEditEngine)
    {
        if ( pDocShell )
        {
            ScDocument& rDoc = pDocShell->GetDocument();
            pEditEngine = rDoc.CreateFieldEditEngine();
        }
        else
        {
            SfxItemPool* pEnginePool = EditEngine::CreatePool();
            pEnginePool->FreezeIdRanges();
            pEditEngine = new ScFieldEditEngine(nullptr, pEnginePool, nullptr, true);
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

    OUString aText;

    if (pDocShell)
    {
        ScDocument& rDoc = pDocShell->GetDocument();

        SfxItemSet aDefaults( pEditEngine->GetEmptyItemSet() );
        if( const ScPatternAttr* pPattern =
                rDoc.GetPattern( aCellPos.Col(), aCellPos.Row(), aCellPos.Tab() ) )
        {
            pPattern->FillEditItemSet( &aDefaults );
            pPattern->FillEditParaItems( &aDefaults );  // including alignment etc. (for reading)
        }

        if (rDoc.GetCellType(aCellPos) == CELLTYPE_EDIT)
        {
            const EditTextObject* pObj = rDoc.GetEditText(aCellPos);
            if (pObj)
                pEditEngine->SetTextNewDefaults(*pObj, aDefaults);
        }
        else
        {
            GetCellText(aCellPos, aText);
            if (!aText.isEmpty())
                pEditEngine->SetTextNewDefaults(aText, aDefaults);
            else
                pEditEngine->SetDefaults(aDefaults);
        }
    }

    bDataValid = true;
    return pForwarder;
}

void ScCellTextData::UpdateData()
{
    if ( bDoUpdate )
    {
        OSL_ENSURE(pEditEngine != nullptr, "no EditEngine for UpdateData()");
        if ( pDocShell && pEditEngine )
        {
            //  during the own UpdateData call, bDataValid must not be reset,
            //  or things like attributes after the text would be lost
            //  (are not stored in the cell)
            bInUpdate = true;   // prevents bDataValid from being reset
            pDocShell->GetDocFunc().PutData(aCellPos, *pEditEngine, true); // always as text

            bInUpdate = false;
            bDirty = false;
        }
    }
    else
        bDirty = true;
}

void ScCellTextData::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( dynamic_cast<const ScUpdateRefHint*>(&rHint) )
    {
//        const ScUpdateRefHint& rRef = (const ScUpdateRefHint&)rHint;

        //! Ref-Update
    }
    else if ( dynamic_cast<const SfxSimpleHint*>(&rHint) )
    {
        sal_uLong nId = static_cast<const SfxSimpleHint&>(rHint).GetId();
        if ( nId == SFX_HINT_DYING )
        {
            pDocShell = nullptr;                       // invalid now

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
