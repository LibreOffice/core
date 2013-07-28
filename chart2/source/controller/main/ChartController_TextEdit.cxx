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

#include "ChartController.hxx"

#include "ResId.hxx"
#include "UndoGuard.hxx"
#include "DrawViewWrapper.hxx"
#include "ChartWindow.hxx"
#include "TitleHelper.hxx"
#include "ObjectIdentifier.hxx"
#include "macros.hxx"
#include "ControllerLockGuard.hxx"
#include "AccessibleTextHelper.hxx"
#include "Strings.hrc"
#include "chartview/DrawModelWrapper.hxx"

#include <svx/svdotext.hxx>

// header for define RET_OK
#include <vcl/msgbox.hxx>
// header for class SdrOutliner
#include <svx/svdoutl.hxx>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <svl/stritem.hxx>
#include <editeng/fontitem.hxx>

namespace chart
{
using namespace ::com::sun::star;

void SAL_CALL ChartController::executeDispatch_EditText( const Point* pMousePixel )
{
    this->StartTextEdit( pMousePixel );
}

void ChartController::StartTextEdit( const Point* pMousePixel )
{
    //the first marked object will be edited

    SolarMutexGuard aGuard;
    SdrObject* pTextObj = m_pDrawViewWrapper->getTextEditObject();
    if(!pTextObj)
        return;

    OSL_PRECOND( !m_pTextActionUndoGuard.get(), "ChartController::StartTextEdit: already have a TextUndoGuard!?" );
    m_pTextActionUndoGuard.reset( new UndoGuard(
        SCH_RESSTR( STR_ACTION_EDIT_TEXT ), m_xUndoManager ) );
    SdrOutliner* pOutliner = m_pDrawViewWrapper->getOutliner();

    //#i77362 change notification for changes on additional shapes are missing
    uno::Reference< beans::XPropertySet > xChartViewProps( m_xChartView, uno::UNO_QUERY );
    if( xChartViewProps.is() )
        xChartViewProps->setPropertyValue( "SdrViewIsInEditMode", uno::makeAny(sal_True) );

    sal_Bool bEdit = m_pDrawViewWrapper->SdrBeginTextEdit( pTextObj
                    , m_pDrawViewWrapper->GetPageView()
                    , m_pChartWindow
                    , sal_False //bIsNewObj
                    , pOutliner
                    , 0L //pOutlinerView
                    , sal_True //bDontDeleteOutliner
                    , sal_True //bOnlyOneView
                    );
    if(bEdit)
    {
        m_pDrawViewWrapper->SetEditMode();

        // #i12587# support for shapes in chart
        if ( pMousePixel )
        {
            OutlinerView* pOutlinerView = m_pDrawViewWrapper->GetTextEditOutlinerView();
            if ( pOutlinerView )
            {
                MouseEvent aEditEvt( *pMousePixel, 1, MOUSE_SYNTHETIC, MOUSE_LEFT, 0 );
                pOutlinerView->MouseButtonDown( aEditEvt );
                pOutlinerView->MouseButtonUp( aEditEvt );
            }
        }

        //we invalidate the outliner region because the outliner has some
        //paint problems (some characters are painted twice a little bit shifted)
        m_pChartWindow->Invalidate( m_pDrawViewWrapper->GetMarkedObjBoundRect() );
    }
}

bool ChartController::EndTextEdit()
{
    m_pDrawViewWrapper->SdrEndTextEdit();

    //#i77362 change notification for changes on additional shapes are missing
    uno::Reference< beans::XPropertySet > xChartViewProps( m_xChartView, uno::UNO_QUERY );
    if( xChartViewProps.is() )
        xChartViewProps->setPropertyValue( "SdrViewIsInEditMode", uno::makeAny(sal_False) );

    SdrObject* pTextObject = m_pDrawViewWrapper->getTextEditObject();
    if(!pTextObject)
        return false;

    SdrOutliner* pOutliner = m_pDrawViewWrapper->getOutliner();
    OutlinerParaObject* pParaObj = pTextObject->GetOutlinerParaObject();
    if( pParaObj && pOutliner )
    {
        pOutliner->SetText( *pParaObj );

        String aString = pOutliner->GetText(
                            pOutliner->GetParagraph( 0 ),
                            pOutliner->GetParagraphCount() );

        OUString aObjectCID = m_aSelection.getSelectedCID();
        if ( !aObjectCID.isEmpty() )
        {
            uno::Reference< beans::XPropertySet > xPropSet =
                ObjectIdentifier::getObjectPropertySet( aObjectCID, getModel() );

            // lock controllers till end of block
            ControllerLockGuard aCLGuard( getModel() );

            TitleHelper::setCompleteString( aString, uno::Reference<
                ::com::sun::star::chart2::XTitle >::query( xPropSet ), m_xCC );

            OSL_ENSURE( m_pTextActionUndoGuard.get(), "ChartController::EndTextEdit: no TextUndoGuard!" );
            if ( m_pTextActionUndoGuard.get() )
                m_pTextActionUndoGuard->commit();
        }
        m_pTextActionUndoGuard.reset();
    }
    return true;
}

void SAL_CALL ChartController::executeDispatch_InsertSpecialCharacter()
{
    SolarMutexGuard aGuard;

    if( m_pDrawViewWrapper && !m_pDrawViewWrapper->IsTextEdit() )
        this->StartTextEdit();

    SvxAbstractDialogFactory * pFact = SvxAbstractDialogFactory::Create();
    OSL_ENSURE( pFact, "No dialog factory" );

    SfxAllItemSet aSet( m_pDrawModelWrapper->GetItemPool() );
    aSet.Put( SfxBoolItem( FN_PARAM_1, sal_False ) );

    //set fixed current font
    aSet.Put( SfxBoolItem( FN_PARAM_2, sal_True ) ); //maybe not necessary in future

    Font aCurFont = m_pDrawViewWrapper->getOutliner()->GetRefDevice()->GetFont();
    aSet.Put( SvxFontItem( aCurFont.GetFamily(), aCurFont.GetName(), aCurFont.GetStyleName(), aCurFont.GetPitch(), aCurFont.GetCharSet(), SID_ATTR_CHAR_FONT ) );

    SfxAbstractDialog * pDlg = pFact->CreateSfxDialog( m_pChartWindow, aSet, getFrame(), RID_SVXDLG_CHARMAP );
    OSL_ENSURE( pDlg, "Couldn't create SvxCharacterMap dialog" );
    if( pDlg->Execute() == RET_OK )
    {
        const SfxItemSet* pSet = pDlg->GetOutputItemSet();
        const SfxPoolItem* pItem=0;
        String aString;
        if ( pSet && pSet->GetItemState( SID_CHARMAP, sal_True, &pItem) == SFX_ITEM_SET &&
             pItem->ISA(SfxStringItem) )
                aString = dynamic_cast<const SfxStringItem*>(pItem)->GetValue();

        OutlinerView* pOutlinerView = m_pDrawViewWrapper->GetTextEditOutlinerView();
        SdrOutliner*  pOutliner = m_pDrawViewWrapper->getOutliner();

        if(!pOutliner || !pOutlinerView)
            return;

        // insert string to outliner

        // prevent flicker
        pOutlinerView->HideCursor();
        pOutliner->SetUpdateMode(sal_False);

        // delete current selection by inserting empty String, so current
        // attributes become unique (sel. has to be erased anyway)
        pOutlinerView->InsertText(String());

        pOutlinerView->InsertText(aString, true);

        ESelection aSel = pOutlinerView->GetSelection();
        aSel.nStartPara = aSel.nEndPara;
        aSel.nStartPos = aSel.nEndPos;
        pOutlinerView->SetSelection(aSel);

        // show changes
        pOutliner->SetUpdateMode(sal_True);
        pOutlinerView->ShowCursor();
    }

    delete pDlg;
}

uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
    ChartController::impl_createAccessibleTextContext()
{
    uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > xResult(
        new AccessibleTextHelper( m_pDrawViewWrapper ));

    return xResult;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
