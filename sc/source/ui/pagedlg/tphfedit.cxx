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

#include <scitems.hxx>
#include <editeng/eeitem.hxx>

#include <editeng/editobj.hxx>
#include <editeng/editview.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sfxdlg.hxx>

#include <tphfedit.hxx>
#include <editutil.hxx>
#include <global.hxx>
#include <patattr.hxx>
#include <scresid.hxx>
#include <globstr.hrc>
#include <strings.hrc>
#include <tabvwsh.hxx>
#include <prevwsh.hxx>
#include <AccessibleEditObject.hxx>

#include <scabstdlg.hxx>
#include <memory>


static void lcl_GetFieldData( ScHeaderFieldData& rData )
{
    SfxViewShell* pShell = SfxViewShell::Current();
    if (pShell)
    {
        if (auto pTabViewShell = dynamic_cast<ScTabViewShell*>( pShell))
            pTabViewShell->FillFieldData(rData);
        else if (auto pPreviewShell = dynamic_cast<ScPreviewShell*>( pShell))
            pPreviewShell->FillFieldData(rData);
    }
}


ScEditWindow::ScEditWindow(ScEditWindowLocation eLoc, weld::Window* pDialog)
    : eLocation(eLoc)
    , mbRTL(ScGlobal::IsSystemRTL())
    , mpDialog(pDialog)
{
}

void ScEditWindow::makeEditEngine()
{
    m_xEditEngine.reset(new ScHeaderEditEngine(EditEngine::CreatePool()));
}

ScHeaderEditEngine* ScEditWindow::GetEditEngine() const
{
    return static_cast<ScHeaderEditEngine*>(m_xEditEngine.get());
}

void ScEditWindow::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    OutputDevice& rDevice = pDrawingArea->get_ref_device();
    Size aSize = rDevice.LogicToPixel(Size(80, 120), MapMode(MapUnit::MapAppFont));
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());

    WeldEditView::SetDrawingArea(pDrawingArea);

    ScHeaderFieldData aData;
    lcl_GetFieldData(aData);
    // fields
    GetEditEngine()->SetData(aData);
    if (mbRTL)
        m_xEditEngine->SetDefaultHorizontalTextDirection(EEHorizontalTextDirection::R2L);

    auto tmpAcc = mxAcc.get();
    if (!tmpAcc)
        return;

    OUString sName;
    switch (eLocation)
    {
        case Left:
            sName = ScResId(STR_ACC_LEFTAREA_NAME);
            break;
        case Center:
            sName = ScResId(STR_ACC_CENTERAREA_NAME);
            break;
        case Right:
            sName = ScResId(STR_ACC_RIGHTAREA_NAME);
            break;
    }

    tmpAcc->InitAcc(nullptr, m_xEditView.get(), nullptr, nullptr,
                  sName, pDrawingArea->get_tooltip_text());
}

ScEditWindow::~ScEditWindow()
{
    // delete Accessible object before deleting EditEngine and EditView
    if (auto tmp = mxAcc.get())
        tmp->dispose();
}

void ScEditWindow::SetNumType(SvxNumType eNumType)
{
    ScHeaderEditEngine* pEditEngine = GetEditEngine();
    pEditEngine->SetNumType(eNumType);
    pEditEngine->UpdateFields();
}

std::unique_ptr<EditTextObject> ScEditWindow::CreateTextObject()
{
    //  reset paragraph attributes
    //  (GetAttribs at creation of format dialog always returns the set items)

    const SfxItemSet& rEmpty = m_xEditEngine->GetEmptyItemSet();
    sal_Int32 nParCnt = m_xEditEngine->GetParagraphCount();
    for (sal_Int32 i=0; i<nParCnt; i++)
        m_xEditEngine->SetParaAttribs( i, rEmpty );

    return m_xEditEngine->CreateTextObject();
}

void ScEditWindow::SetFont( const ScPatternAttr& rPattern )
{
    auto pSet = std::make_unique<SfxItemSet>( m_xEditEngine->GetEmptyItemSet() );
    rPattern.FillEditItemSet( pSet.get() );
    //  FillEditItemSet adjusts font height to 1/100th mm,
    //  but for header/footer twips is needed, as in the PatternAttr:
    pSet->Put( rPattern.GetItem(ATTR_FONT_HEIGHT).CloneSetWhich(EE_CHAR_FONTHEIGHT) );
    pSet->Put( rPattern.GetItem(ATTR_CJK_FONT_HEIGHT).CloneSetWhich(EE_CHAR_FONTHEIGHT_CJK) );
    pSet->Put( rPattern.GetItem(ATTR_CTL_FONT_HEIGHT).CloneSetWhich(EE_CHAR_FONTHEIGHT_CTL) );
    if (mbRTL)
        pSet->Put( SvxAdjustItem( SvxAdjust::Right, EE_PARA_JUST ) );
    GetEditEngine()->SetDefaults( std::move(pSet) );
}

void ScEditWindow::SetText( const EditTextObject& rTextObject )
{
    GetEditEngine()->SetTextCurrentDefaults(rTextObject);
}

void ScEditWindow::InsertField( const SvxFieldItem& rFld )
{
    m_xEditView->InsertField( rFld );
}

void ScEditWindow::SetCharAttributes()
{
    SfxObjectShell* pDocSh  = SfxObjectShell::Current();

    SfxViewShell*       pViewSh = SfxViewShell::Current();

    ScTabViewShell* pTabViewSh = dynamic_cast<ScTabViewShell*>( SfxViewShell::Current() );

    OSL_ENSURE( pDocSh,  "Current DocShell not found" );
    OSL_ENSURE( pViewSh, "Current ViewShell not found" );

    if ( !(pDocSh && pViewSh) )
        return;

    if(pTabViewSh!=nullptr) pTabViewSh->SetInFormatDialog(true);

    SfxItemSet aSet( m_xEditView->GetAttribs() );

    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();

    ScopedVclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateScCharDlg(
        mpDialog,  &aSet, pDocSh, false));
    pDlg->SetText( ScResId( STR_TEXTATTRS ) );
    if ( pDlg->Execute() == RET_OK )
    {
        aSet.ClearItem();
        aSet.Put( *pDlg->GetOutputItemSet() );
        m_xEditView->SetAttribs( aSet );
    }

    if(pTabViewSh!=nullptr) pTabViewSh->SetInFormatDialog(false);
}

bool ScEditWindow::KeyInput( const KeyEvent& rKEvt )
{
    sal_uInt16 nKey =  rKEvt.GetKeyCode().GetModifier()
                 + rKEvt.GetKeyCode().GetCode();

    if ( nKey == KEY_TAB || nKey == KEY_TAB + KEY_SHIFT )
    {
        return false;
    }
    else if ( !m_xEditView->PostKeyEvent( rKEvt ) )
    {
        return false;
    }
    else if ( !rKEvt.GetKeyCode().IsMod1() && !rKEvt.GetKeyCode().IsShift() &&
                rKEvt.GetKeyCode().IsMod2() && rKEvt.GetKeyCode().GetCode() == KEY_DOWN )
    {
        aObjectSelectLink.Call(*this);
        return true;
    }
    return true;
}

void ScEditWindow::GetFocus()
{
    assert(m_GetFocusLink);
    m_GetFocusLink(*this);

    if (auto tmp = mxAcc.get())
        tmp->GotFocus();

    WeldEditView::GetFocus();
}

void ScEditWindow::LoseFocus()
{
    if (auto xTemp = mxAcc.get())
        xTemp->LostFocus();
    else
        mxAcc = nullptr;
    WeldEditView::LoseFocus();
}

bool ScEditWindow::MouseButtonDown(const MouseEvent& rMEvt)
{
    bool bHadFocus = HasFocus();
    bool bRet = WeldEditView::MouseButtonDown(rMEvt);
    if (!bHadFocus)
    {
        assert(HasFocus());
        GetFocus();
    }
    return bRet;
}

css::uno::Reference< css::accessibility::XAccessible > ScEditWindow::CreateAccessible()
{
    rtl::Reference<ScAccessibleEditControlObject> tmp = new ScAccessibleEditControlObject(this, ScAccessibleEditObject::EditControl);
    mxAcc = tmp.get();
    return css::uno::Reference<css::accessibility::XAccessible>(static_cast<cppu::OWeakObject*>(tmp.get()), css::uno::UNO_QUERY_THROW);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
