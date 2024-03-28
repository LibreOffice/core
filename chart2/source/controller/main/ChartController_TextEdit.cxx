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

#include <config_wasm_strip.h>

#include <ChartController.hxx>

#include <ResId.hxx>
#include "UndoGuard.hxx"
#include <DrawViewWrapper.hxx>
#include <ChartWindow.hxx>
#include <ChartModel.hxx>
#include <ChartView.hxx>
#include <TitleHelper.hxx>
#include <ObjectIdentifier.hxx>
#include <ControllerLockGuard.hxx>
#include <comphelper/diagnose_ex.hxx>
#if !ENABLE_WASM_STRIP_ACCESSIBILITY
#include <AccessibleTextHelper.hxx>
#endif
#include <strings.hrc>
#include <chartview/DrawModelWrapper.hxx>
#include <osl/diagnose.h>

#include <svx/svdoutl.hxx>
#include <svx/svxdlg.hxx>
#include <svx/svxids.hrc>
#include <editeng/editids.hrc>
#include <vcl/svapp.hxx>
#include <com/sun/star/chart2/FormattedString.hpp>
#include <svl/stritem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/section.hxx>
#include <memory>

namespace chart
{
using namespace ::com::sun::star;

void ChartController::executeDispatch_EditText( const Point* pMousePixel )
{
    StartTextEdit( pMousePixel );
}

void ChartController::StartTextEdit( const Point* pMousePixel )
{
    //the first marked object will be edited

    SolarMutexGuard aGuard;
    SdrObject* pTextObj = m_pDrawViewWrapper->getTextEditObject();
    if(!pTextObj)
        return;

    OSL_PRECOND(!m_pTextActionUndoGuard,
                "ChartController::StartTextEdit: already have a TextUndoGuard!?");
    m_pTextActionUndoGuard.reset( new UndoGuard(
        SchResId( STR_ACTION_EDIT_TEXT ), m_xUndoManager ) );
    SdrOutliner* pOutliner = m_pDrawViewWrapper->getOutliner();

    //#i77362 change notification for changes on additional shapes are missing
    if( m_xChartView.is() )
        m_xChartView->setPropertyValue( "SdrViewIsInEditMode", uno::Any(true) );

    auto pChartWindow(GetChartWindow());

    bool bEdit = m_pDrawViewWrapper->SdrBeginTextEdit( pTextObj
                    , m_pDrawViewWrapper->GetPageView()
                    , pChartWindow
                    , false //bIsNewObj
                    , pOutliner
                    , nullptr //pOutlinerView
                    , true //bDontDeleteOutliner
                    , true //bOnlyOneView
                    );
    if(!bEdit)
        return;

    m_pDrawViewWrapper->SetEditMode();

    // #i12587# support for shapes in chart
    if ( pMousePixel )
    {
        OutlinerView* pOutlinerView = m_pDrawViewWrapper->GetTextEditOutlinerView();
        if ( pOutlinerView )
        {
            MouseEvent aEditEvt( *pMousePixel, 1, MouseEventModifiers::SYNTHETIC, MOUSE_LEFT, 0 );
            pOutlinerView->MouseButtonDown( aEditEvt );
            pOutlinerView->MouseButtonUp( aEditEvt );
        }
    }

    if (pChartWindow)
    {
        //we invalidate the outliner region because the outliner has some
        //paint problems (some characters are painted twice a little bit shifted)
        pChartWindow->Invalidate( m_pDrawViewWrapper->GetMarkedObjBoundRect() );
    }
}

bool ChartController::EndTextEdit()
{
    m_pDrawViewWrapper->SdrEndTextEdit();

    //#i77362 change notification for changes on additional shapes are missing
    if( m_xChartView.is() )
        m_xChartView->setPropertyValue( "SdrViewIsInEditMode", uno::Any(false) );

    SdrObject* pTextObject = m_pDrawViewWrapper->getTextEditObject();
    if(!pTextObject)
        return false;

    SdrOutliner* pOutliner = m_pDrawViewWrapper->getOutliner();
    OutlinerParaObject* pParaObj = pTextObject->GetOutlinerParaObject();
    if( !pParaObj || !pOutliner )
        return true;

    pOutliner->SetText( *pParaObj );

    OUString aString = pOutliner->GetText(
                        pOutliner->GetParagraph( 0 ),
                        pOutliner->GetParagraphCount() );

    OUString aObjectCID = m_aSelection.getSelectedCID();
    if ( !aObjectCID.isEmpty() )
    {
        uno::Reference< beans::XPropertySet > xPropSet =
            ObjectIdentifier::getObjectPropertySet( aObjectCID, getChartModel() );

        // lock controllers till end of block
        ControllerLockGuardUNO aCLGuard( getChartModel() );

        uno::Sequence< uno::Reference< chart2::XFormattedString > > aNewFormattedTitle =
            GetFormattedTitle(pParaObj->GetTextObject(), pTextObject->getUnoShape());

        Title* pTitle = dynamic_cast<Title*>(xPropSet.get());
        if (pTitle && aNewFormattedTitle.hasElements())
        {
            bool bStacked = false;
            if (xPropSet.is())
                xPropSet->getPropertyValue("StackCharacters") >>= bStacked;

            if (bStacked)
            {
                for (uno::Reference< chart2::XFormattedString >const& formattedStr : aNewFormattedTitle)
                {
                    formattedStr->setString(TitleHelper::getUnstackedStr(formattedStr->getString()));
                }
            }

            pTitle->setText(aNewFormattedTitle);
        }
        else
        {
            TitleHelper::setCompleteString(aString, pTitle, m_xCC);
        }

        OSL_ENSURE(m_pTextActionUndoGuard, "ChartController::EndTextEdit: no TextUndoGuard!");
        if (m_pTextActionUndoGuard)
            m_pTextActionUndoGuard->commit();
    }
    m_pTextActionUndoGuard.reset();
    return true;
}

uno::Sequence< uno::Reference< chart2::XFormattedString > > ChartController::GetFormattedTitle(
    const EditTextObject& aEdit, const uno::Reference< drawing::XShape >& xShape )
{
    std::vector < uno::Reference< chart2::XFormattedString > > aNewStrings;
    if (!xShape.is())
        return comphelper::containerToSequence(aNewStrings);

    uno::Reference< text::XText > xText(xShape, uno::UNO_QUERY);
    if (!xText.is())
        return comphelper::containerToSequence(aNewStrings);

    uno::Reference< text::XTextCursor > xSelectionCursor(xText->createTextCursor());
    if (!xSelectionCursor.is())
        return comphelper::containerToSequence(aNewStrings);

    xSelectionCursor->gotoStart(false);

    std::vector<editeng::Section> aSecAttrs;
    aEdit.GetAllSections(aSecAttrs);

    for (editeng::Section const& rSection : aSecAttrs)
    {
        if (!xSelectionCursor->isCollapsed())
            xSelectionCursor->collapseToEnd();

        xSelectionCursor->goRight(rSection.mnEnd - rSection.mnStart, true);

        OUString aNewString = xSelectionCursor->getString();

        bool bNextPara = (aEdit.GetParagraphCount() > 1 && rSection.mnParagraph != aEdit.GetParagraphCount() - 1 &&
            aEdit.GetTextLen(rSection.mnParagraph) <= rSection.mnEnd);

        uno::Reference< chart2::XFormattedString2 > xFmtStr = chart2::FormattedString::create(m_xCC);
        if (bNextPara)
            aNewString = aNewString + OUStringChar('\n');
        xFmtStr->setString(aNewString);
        aNewStrings.emplace_back(xFmtStr);

        uno::Reference< beans::XPropertySetInfo > xInfo = xFmtStr->getPropertySetInfo();
        uno::Reference< beans::XPropertySet > xSelectionProp(xSelectionCursor, uno::UNO_QUERY);
        try
        {
            for (const beans::Property& rProp : xSelectionProp->getPropertySetInfo()->getProperties())
            {
                if (xInfo.is() && xInfo->hasPropertyByName(rProp.Name))
                {
                    const uno::Any aValue = xSelectionProp->getPropertyValue(rProp.Name);
                    xFmtStr->setPropertyValue(rProp.Name, aValue);
                }
            }
        }
        catch ( const uno::Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
            aNewStrings.clear();
        }

        if (bNextPara)
            xSelectionCursor->goRight(1, false); // next paragraph
    }

    return comphelper::containerToSequence(aNewStrings);
}

void ChartController::executeDispatch_InsertSpecialCharacter()
{
    SolarMutexGuard aGuard;
    if( !m_pDrawViewWrapper)
    {
        OSL_ENSURE( m_pDrawViewWrapper, "No DrawViewWrapper for ChartController" );
        return;
    }
    if( !m_pDrawViewWrapper->IsTextEdit() )
        StartTextEdit();

    SvxAbstractDialogFactory * pFact = SvxAbstractDialogFactory::Create();

    SfxAllItemSet aSet( m_pDrawModelWrapper->GetItemPool() );
    aSet.Put( SfxBoolItem( FN_PARAM_1, false ) );

    //set fixed current font
    aSet.Put( SfxBoolItem( FN_PARAM_2, true ) ); //maybe not necessary in future

    vcl::Font aCurFont = m_pDrawViewWrapper->getOutliner()->GetRefDevice()->GetFont();
    aSet.Put( SvxFontItem( aCurFont.GetFamilyType(), aCurFont.GetFamilyName(), aCurFont.GetStyleName(), aCurFont.GetPitch(), aCurFont.GetCharSet(), SID_ATTR_CHAR_FONT ) );

    ScopedVclPtr<SfxAbstractDialog> pDlg(pFact->CreateCharMapDialog(GetChartFrame(), aSet, nullptr));
    if( pDlg->Execute() != RET_OK )
        return;

    const SfxItemSet* pSet = pDlg->GetOutputItemSet();
    OUString aString;
    if (pSet)
        if (const SfxStringItem* pCharMapItem = pSet->GetItemIfSet(SID_CHARMAP))
            aString = pCharMapItem->GetValue();

    OutlinerView* pOutlinerView = m_pDrawViewWrapper->GetTextEditOutlinerView();
    SdrOutliner*  pOutliner = m_pDrawViewWrapper->getOutliner();

    if(!pOutliner || !pOutlinerView)
        return;

    // insert string to outliner

    // prevent flicker
    pOutlinerView->HideCursor();
    pOutliner->SetUpdateLayout(false);

    // delete current selection by inserting empty String, so current
    // attributes become unique (sel. has to be erased anyway)
    pOutlinerView->InsertText(OUString());

    pOutlinerView->InsertText(aString, true);

    ESelection aSel = pOutlinerView->GetSelection();
    aSel.nStartPara = aSel.nEndPara;
    aSel.nStartPos = aSel.nEndPos;
    pOutlinerView->SetSelection(aSel);

    // show changes
    pOutliner->SetUpdateLayout(true);
    pOutlinerView->ShowCursor();
}

rtl::Reference< ::chart::AccessibleTextHelper >
    ChartController::createAccessibleTextContext()
{
#if !ENABLE_WASM_STRIP_ACCESSIBILITY
    return new AccessibleTextHelper( m_pDrawViewWrapper.get() );
#else
    return {};
#endif
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
