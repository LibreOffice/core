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

#include <vcl/event.hxx>
#include <vcl/metaact.hxx>
#include <vcl/virdev.hxx>
#include <svtools/valueset.hxx>
#include <svl/eitem.hxx>
#include <svl/itemset.hxx>
#include <sfx2/dispatch.hxx>
#include <svtools/colrdlg.hxx>

#include <svx/colorbox.hxx>
#include <svx/dialmgr.hxx>
#include <svx/bmpmask.hxx>
#include <svx/strings.hrc>
#include <svx/svxids.hrc>
#include <memory>
#include <helpids.h>

#define OWN_CALLMODE    SfxCallMode::ASYNCHRON | SfxCallMode::RECORD


#define TEST_COLS()                                                 \
{                                                                   \
    nR = aCol.GetRed(); nG = aCol.GetGreen(); nB = aCol.GetBlue();  \
    for( i = 0; i < nCount; i++ )                                   \
    {                                                               \
        if ( ( pMinR[i] <= nR ) && ( pMaxR[i] >= nR ) &&            \
             ( pMinG[i] <= nG ) && ( pMaxG[i] >= nG ) &&            \
             ( pMinB[i] <= nB ) && ( pMaxB[i] >= nB ) )             \
        {                                                           \
            aCol = pDstCols[i]; bReplace = true; break;             \
        }                                                           \
    }                                                               \
}

SFX_IMPL_DOCKINGWINDOW_WITHID( SvxBmpMaskChildWindow, SID_BMPMASK )

class BmpColorWindow : public weld::CustomWidgetController
{
    Color       aColor;


public:
    explicit BmpColorWindow()
        : aColor( COL_WHITE )
    {
    }

    void SetColor( const Color& rColor )
    {
        aColor = rColor;
        Invalidate();
    }

    virtual void Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;

    virtual void SetDrawingArea(weld::DrawingArea* pArea) override
    {
        Size aSize(pArea->get_ref_device().LogicToPixel(Size(43, 14), MapMode(MapUnit::MapAppFont)));
        CustomWidgetController::SetDrawingArea(pArea);
        pArea->set_size_request(aSize.Width(), aSize.Height());
        SetOutputSizePixel(aSize);
    }
};

class MaskSet : public ValueSet
{
    VclPtr<SvxBmpMask> pSvxBmpMask;

public:
    MaskSet(SvxBmpMask* pMask);
    virtual void Select() override;
    virtual bool KeyInput( const KeyEvent& rKEvt ) override;
    virtual void GetFocus() override;
    virtual void SetDrawingArea(weld::DrawingArea* pArea) override
    {
        Size aSize(pArea->get_ref_device().LogicToPixel(Size(24, 12), MapMode(MapUnit::MapAppFont)));
        ValueSet::SetDrawingArea(pArea);
        pArea->set_size_request(aSize.Width(), aSize.Height());
        SetOutputSizePixel(aSize);
        SetHelpId(HID_BMPMASK_CTL_QCOL_1);
    }
    void onEditColor();
};

MaskSet::MaskSet(SvxBmpMask* pMask)
    : ValueSet(nullptr)
    , pSvxBmpMask(pMask)
{
}

void MaskSet::Select()
{
    ValueSet::Select();

    pSvxBmpMask->onSelect( this );
}

void MaskSet::GetFocus()
{
    ValueSet::GetFocus();
    SelectItem( 1 );
    pSvxBmpMask->onSelect( this );
}

bool MaskSet::KeyInput( const KeyEvent& rKEvt )
{
    bool bRet = false;

    vcl::KeyCode aCode = rKEvt.GetKeyCode();

    // if the key has a modifier we don't care
    if( aCode.GetModifier() )
    {
        bRet = ValueSet::KeyInput( rKEvt );
    }
    else
    {
        // check for keys that interests us
        switch ( aCode.GetCode() )
        {
            case KEY_SPACE:
                onEditColor();
                bRet = true;
                break;
            default:
                bRet = ValueSet::KeyInput( rKEvt );
        }
    }
    return bRet;
}

void MaskSet::onEditColor()
{
    SvColorDialog aColorDlg;

    aColorDlg.SetColor(GetItemColor(1));

    if (aColorDlg.Execute(pSvxBmpMask->GetFrameWeld()))
        SetItemColor(1, aColorDlg.GetColor());
}

class MaskData
{
    VclPtr<SvxBmpMask> pMask;
    bool            bIsReady;
    bool            bExecState;
    SfxBindings&    rBindings;

public:
                MaskData( SvxBmpMask* pBmpMask, SfxBindings& rBind );

    bool        IsCbxReady() const { return bIsReady; }
    void        SetExecState( bool bState ) { bExecState = bState; }
    bool        IsExecReady() const { return bExecState; }

                DECL_LINK( PipetteHdl, const OUString&, void );
                DECL_LINK( CbxHdl, weld::Toggleable&, void);
                DECL_LINK( CbxTransHdl, weld::Toggleable&, void );
                DECL_LINK( FocusLbHdl, weld::Widget&, void );
                DECL_LINK(ExecHdl, weld::Button&, void);
};


MaskData::MaskData( SvxBmpMask* pBmpMask, SfxBindings& rBind ) :

    pMask       ( pBmpMask ),
    bIsReady    ( false ),
    bExecState  ( false ),
    rBindings   ( rBind )

{
}

IMPL_LINK( MaskData, PipetteHdl, const OUString&, rId, void )
{
    SfxBoolItem aBItem( SID_BMPMASK_PIPETTE,
                        pMask->m_xTbxPipette->get_item_active(rId) );

    rBindings.GetDispatcher()->ExecuteList(SID_BMPMASK_PIPETTE, OWN_CALLMODE,
            { &aBItem });
}

IMPL_LINK( MaskData, CbxHdl, weld::Toggleable&, rCbx, void )
{
    bIsReady =  pMask->m_xCbx1->get_active() || pMask->m_xCbx2->get_active() ||
                pMask->m_xCbx3->get_active() || pMask->m_xCbx4->get_active();

    if ( bIsReady && IsExecReady() )
        pMask->m_xBtnExec->set_sensitive(true);
    else
        pMask->m_xBtnExec->set_sensitive(false);

    // When a checkbox is checked, the pipette is enabled
    if ( !rCbx.get_active() )
        return;

    MaskSet* pSet = nullptr;

    if (&rCbx == pMask->m_xCbx1.get())
        pSet = pMask->m_xQSet1.get();
    else if (&rCbx == pMask->m_xCbx2.get())
        pSet = pMask->m_xQSet2.get();
    else if (&rCbx == pMask->m_xCbx3.get())
        pSet = pMask->m_xQSet3.get();
    else // if ( &rCbx == pMask->m_xCbx4 )
        pSet = pMask->m_xQSet4.get();

    pSet->SelectItem( 1 );
    pSet->Select();

    pMask->m_xTbxPipette->set_item_active(u"pipette"_ustr, true);
    PipetteHdl(u"pipette"_ustr);
}

IMPL_LINK( MaskData, CbxTransHdl, weld::Toggleable&, rCbx, void )
{
    bIsReady = rCbx.get_active();
    if ( bIsReady )
    {
        pMask->m_xQSet1->Disable();
        pMask->m_xQSet2->Disable();
        pMask->m_xQSet3->Disable();
        pMask->m_xQSet4->Disable();
        pMask->m_xCtlPipette->Disable();
        pMask->m_xCbx1->set_sensitive(false);
        pMask->m_xSp1->set_sensitive(false);
        pMask->m_xCbx2->set_sensitive(false);
        pMask->m_xSp2->set_sensitive(false);
        pMask->m_xCbx3->set_sensitive(false);
        pMask->m_xSp3->set_sensitive(false);
        pMask->m_xCbx4->set_sensitive(false);
        pMask->m_xSp4->set_sensitive(false);
        pMask->m_xTbxPipette->set_sensitive(false);

        pMask->m_xLbColor1->set_sensitive(false);
        pMask->m_xLbColor2->set_sensitive(false);
        pMask->m_xLbColor3->set_sensitive(false);
        pMask->m_xLbColor4->set_sensitive(false);
        pMask->m_xLbColorTrans->set_sensitive(true);
    }
    else
    {
        pMask->m_xQSet1->Enable();
        pMask->m_xQSet2->Enable();
        pMask->m_xQSet3->Enable();
        pMask->m_xQSet4->Enable();
        pMask->m_xCtlPipette->Enable();
        pMask->m_xCbx1->set_sensitive(true);
        pMask->m_xSp1->set_sensitive(true);
        pMask->m_xCbx2->set_sensitive(true);
        pMask->m_xSp2->set_sensitive(true);
        pMask->m_xCbx3->set_sensitive(true);
        pMask->m_xSp3->set_sensitive(true);
        pMask->m_xCbx4->set_sensitive(true);
        pMask->m_xSp4->set_sensitive(true);
        pMask->m_xTbxPipette->set_sensitive(true);

        pMask->m_xLbColor1->set_sensitive(true);
        pMask->m_xLbColor2->set_sensitive(true);
        pMask->m_xLbColor3->set_sensitive(true);
        pMask->m_xLbColor4->set_sensitive(true);
        pMask->m_xLbColorTrans->set_sensitive(false);

        bIsReady = pMask->m_xCbx1->get_active() || pMask->m_xCbx2->get_active() ||
                   pMask->m_xCbx3->get_active() || pMask->m_xCbx4->get_active();
    }

    if ( bIsReady && IsExecReady() )
        pMask->m_xBtnExec->set_sensitive(true);
    else
        pMask->m_xBtnExec->set_sensitive(false);
}

IMPL_LINK( MaskData, FocusLbHdl, weld::Widget&, rLb, void )
{
    pMask->m_xQSet1->SelectItem( &rLb == &pMask->m_xLbColor1->get_widget() ? 1 : 0 /* , false */ );
    pMask->m_xQSet2->SelectItem( &rLb == &pMask->m_xLbColor2->get_widget() ? 1 : 0 /* , false */ );
    pMask->m_xQSet3->SelectItem( &rLb == &pMask->m_xLbColor3->get_widget() ? 1 : 0 /* , false */ );
    pMask->m_xQSet4->SelectItem( &rLb == &pMask->m_xLbColor4->get_widget() ? 1 : 0 /* , false */ );
}

IMPL_LINK_NOARG(MaskData, ExecHdl, weld::Button&, void)
{
    SfxBoolItem aBItem( SID_BMPMASK_EXEC, true );
    rBindings.GetDispatcher()->ExecuteList(SID_BMPMASK_EXEC, OWN_CALLMODE,
            { &aBItem });
}

void BmpColorWindow::Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& /*Rect*/)
{
    rRenderContext.Push(vcl::PushFlags::LINECOLOR | vcl::PushFlags::FILLCOLOR);
    rRenderContext.SetLineColor(aColor);
    rRenderContext.SetFillColor(aColor);
    rRenderContext.DrawRect(tools::Rectangle(Point(), GetOutputSizePixel()));
    rRenderContext.Pop();
}

SvxBmpMaskSelectItem::SvxBmpMaskSelectItem( SvxBmpMask& rMask,
                                            SfxBindings& rBindings ) :
            SfxControllerItem   ( SID_BMPMASK_EXEC, rBindings ),
            rBmpMask            ( rMask)
{
}

void SvxBmpMaskSelectItem::StateChangedAtToolBoxControl( sal_uInt16 nSID, SfxItemState /*eState*/,
                                         const SfxPoolItem* pItem )
{
    if ( ( nSID == SID_BMPMASK_EXEC ) && pItem )
    {
        const SfxBoolItem* pStateItem = dynamic_cast<const SfxBoolItem*>( pItem  );
        assert(pStateItem); // SfxBoolItem expected
        if (pStateItem)
            rBmpMask.SetExecState( pStateItem->GetValue() );
    }
}

SvxBmpMaskChildWindow::SvxBmpMaskChildWindow(vcl::Window* pParent_, sal_uInt16 nId,
                                             SfxBindings* pBindings,
                                             SfxChildWinInfo* pInfo)
    : SfxChildWindow(pParent_, nId)
{
    VclPtr<SvxBmpMask> pDlg = VclPtr<SvxBmpMask>::Create(pBindings, this, pParent_);

    SetWindow( pDlg );

    pDlg->Initialize( pInfo );
}

SvxBmpMask::SvxBmpMask(SfxBindings *pBindinx, SfxChildWindow *pCW, vcl::Window* pParent)
    : SfxDockingWindow(pBindinx, pCW, pParent, u"DockingColorReplace"_ustr,
                       u"svx/ui/dockingcolorreplace.ui"_ustr)
    , m_xTbxPipette(m_xBuilder->weld_toolbar(u"toolbar"_ustr))
    , m_xCtlPipette(new BmpColorWindow)
    , m_xCtlPipetteWin(new weld::CustomWeld(*m_xBuilder, u"toolcolor"_ustr, *m_xCtlPipette))
    , m_xBtnExec(m_xBuilder->weld_button(u"replace"_ustr))
    , m_xCbx1(m_xBuilder->weld_check_button(u"cbx1"_ustr))
    , m_xQSet1(new MaskSet(this))
    , m_xQSetWin1(new weld::CustomWeld(*m_xBuilder, u"qset1"_ustr, *m_xQSet1))
    , m_xSp1(m_xBuilder->weld_metric_spin_button(u"tol1"_ustr, FieldUnit::PERCENT))
    , m_xLbColor1(new ColorListBox(m_xBuilder->weld_menu_button(u"color1"_ustr), [this]{ return GetFrameWeld(); }))
    , m_xCbx2(m_xBuilder->weld_check_button(u"cbx2"_ustr))
    , m_xQSet2(new MaskSet(this))
    , m_xQSetWin2(new weld::CustomWeld(*m_xBuilder, u"qset2"_ustr, *m_xQSet2))
     , m_xSp2(m_xBuilder->weld_metric_spin_button(u"tol2"_ustr, FieldUnit::PERCENT))
    , m_xLbColor2(new ColorListBox(m_xBuilder->weld_menu_button(u"color2"_ustr), [this]{ return GetFrameWeld(); }))
    , m_xCbx3(m_xBuilder->weld_check_button(u"cbx3"_ustr))
    , m_xQSet3(new MaskSet(this))
    , m_xQSetWin3(new weld::CustomWeld(*m_xBuilder, u"qset3"_ustr, *m_xQSet3))
    , m_xSp3(m_xBuilder->weld_metric_spin_button(u"tol3"_ustr, FieldUnit::PERCENT))
    , m_xLbColor3(new ColorListBox(m_xBuilder->weld_menu_button(u"color3"_ustr), [this]{ return GetFrameWeld(); }))
    , m_xCbx4(m_xBuilder->weld_check_button(u"cbx4"_ustr))
    , m_xQSet4(new MaskSet(this))
    , m_xQSetWin4(new weld::CustomWeld(*m_xBuilder, u"qset4"_ustr, *m_xQSet4))
    , m_xSp4(m_xBuilder->weld_metric_spin_button(u"tol4"_ustr, FieldUnit::PERCENT))
    , m_xLbColor4(new ColorListBox(m_xBuilder->weld_menu_button(u"color4"_ustr), [this]{ return GetFrameWeld(); }))
    , m_xCbxTrans(m_xBuilder->weld_check_button(u"cbx5"_ustr))
    , m_xLbColorTrans(new ColorListBox(m_xBuilder->weld_menu_button(u"color5"_ustr), [this]{ return GetFrameWeld(); }))
    , m_xData(new MaskData(this, *pBindinx))
    , aPipetteColor(COL_WHITE)
    , aSelItem(*this, *pBindinx)
{
    SetText(SvxResId(RID_SVXDLG_BMPMASK_STR_TITLE));

    m_xLbColor1->SetSlotId(SID_BMPMASK_COLOR);
    m_xLbColor2->SetSlotId(SID_BMPMASK_COLOR);
    m_xLbColor3->SetSlotId(SID_BMPMASK_COLOR);
    m_xLbColor4->SetSlotId(SID_BMPMASK_COLOR);

    m_xLbColorTrans->SelectEntry(COL_BLACK);
    m_xLbColor1->SelectEntry(COL_TRANSPARENT);
    m_xLbColor2->SelectEntry(COL_TRANSPARENT);
    m_xLbColor3->SelectEntry(COL_TRANSPARENT);
    m_xLbColor4->SelectEntry(COL_TRANSPARENT);

    m_xTbxPipette->connect_clicked( LINK( m_xData.get(), MaskData, PipetteHdl ) );
    m_xBtnExec->connect_clicked( LINK( m_xData.get(), MaskData, ExecHdl ) );

    m_xCbx1->connect_toggled( LINK( m_xData.get(), MaskData, CbxHdl ) );
    m_xCbx2->connect_toggled( LINK( m_xData.get(), MaskData, CbxHdl ) );
    m_xCbx3->connect_toggled( LINK( m_xData.get(), MaskData, CbxHdl ) );
    m_xCbx4->connect_toggled( LINK( m_xData.get(), MaskData, CbxHdl ) );
    m_xCbxTrans->connect_toggled( LINK( m_xData.get(), MaskData, CbxTransHdl ) );

    SetAccessibleNames ();

    m_xLbColor1->connect_focus_in( LINK( m_xData.get(), MaskData, FocusLbHdl ) );
    m_xLbColor2->connect_focus_in( LINK( m_xData.get(), MaskData, FocusLbHdl ) );
    m_xLbColor3->connect_focus_in( LINK( m_xData.get(), MaskData, FocusLbHdl ) );
    m_xLbColor4->connect_focus_in( LINK( m_xData.get(), MaskData, FocusLbHdl ) );
    m_xLbColorTrans->set_sensitive(false);

    OUString sColorPalette (SvxResId( RID_SVXDLG_BMPMASK_STR_PALETTE));
    OUString sColorPaletteN;

    m_xQSet1->SetStyle( m_xQSet1->GetStyle() | WB_DOUBLEBORDER | WB_ITEMBORDER );
    m_xQSet1->SetColCount();
    m_xQSet1->SetLineCount( 1 );
    sColorPaletteN = sColorPalette + " 1";
    m_xQSet1->InsertItem( 1, aPipetteColor, sColorPaletteN);
    m_xQSet1->SelectItem( 1 );

    m_xQSet2->SetStyle( m_xQSet2->GetStyle() | WB_DOUBLEBORDER | WB_ITEMBORDER );
    m_xQSet2->SetColCount();
    m_xQSet2->SetLineCount( 1 );
    sColorPaletteN = sColorPalette + " 2";
    m_xQSet2->InsertItem( 1, aPipetteColor, sColorPaletteN);
    m_xQSet2->SelectItem( 0 );

    m_xQSet3->SetStyle( m_xQSet3->GetStyle() | WB_DOUBLEBORDER | WB_ITEMBORDER );
    m_xQSet3->SetColCount();
    m_xQSet3->SetLineCount( 1 );
    sColorPaletteN = sColorPalette + " 3";
    m_xQSet3->InsertItem( 1, aPipetteColor, sColorPaletteN);
    m_xQSet3->SelectItem( 0 );

    m_xQSet4->SetStyle( m_xQSet4->GetStyle() | WB_DOUBLEBORDER | WB_ITEMBORDER );
    m_xQSet4->SetColCount();
    m_xQSet4->SetLineCount( 1 );
    sColorPaletteN = sColorPalette + " 4";
    m_xQSet4->InsertItem( 1, aPipetteColor, sColorPaletteN);
    m_xQSet4->SelectItem( 0 );

    m_xQSet1->Show();
    m_xQSet2->Show();
    m_xQSet3->Show();
    m_xQSet4->Show();
}

SvxBmpMask::~SvxBmpMask()
{
    disposeOnce();
}

void SvxBmpMask::dispose()
{
    m_xQSetWin1.reset();
    m_xQSet1.reset();
    m_xQSetWin2.reset();
    m_xQSet2.reset();
    m_xQSetWin3.reset();
    m_xQSet3.reset();
    m_xQSetWin4.reset();
    m_xQSet4.reset();
    m_xCtlPipetteWin.reset();
    m_xCtlPipette.reset();
    m_xData.reset();
    m_xTbxPipette.reset();
    m_xBtnExec.reset();
    m_xCbx1.reset();
    m_xSp1.reset();
    m_xLbColor1.reset();
    m_xCbx2.reset();
    m_xSp2.reset();
    m_xLbColor2.reset();
    m_xCbx3.reset();
    m_xSp3.reset();
    m_xLbColor3.reset();
    m_xCbx4.reset();
    m_xSp4.reset();
    m_xLbColor4.reset();
    m_xCbxTrans.reset();
    m_xLbColorTrans.reset();
    aSelItem.dispose();
    SfxDockingWindow::dispose();
}

/** is called by a MaskSet when it is selected */
void SvxBmpMask::onSelect( const MaskSet* pSet )
{
    // now deselect all other value sets
    if( pSet != m_xQSet1.get() )
        m_xQSet1->SelectItem( 0 );

    if( pSet != m_xQSet2.get() )
        m_xQSet2->SelectItem( 0 );

    if( pSet != m_xQSet3.get() )
        m_xQSet3->SelectItem( 0 );

    if( pSet != m_xQSet4.get() )
        m_xQSet4->SelectItem( 0 );
}

bool SvxBmpMask::Close()
{
    SfxBoolItem aItem2( SID_BMPMASK_PIPETTE, false );
    GetBindings().GetDispatcher()->ExecuteList(SID_BMPMASK_PIPETTE,
            OWN_CALLMODE, { &aItem2 });

    return SfxDockingWindow::Close();
}

void SvxBmpMask::SetColor( const Color& rColor )
{
    aPipetteColor = rColor;
    m_xCtlPipette->SetColor( aPipetteColor );
}

void SvxBmpMask::PipetteClicked()
{
    if( m_xQSet1->GetSelectedItemId() == 1 )
    {
        m_xCbx1->set_active(true);
        m_xData->CbxHdl(*m_xCbx1);
        m_xQSet1->SetItemColor( 1, aPipetteColor );
        m_xQSet1->SetFormat();
    }
    else if( m_xQSet2->GetSelectedItemId() == 1 )
    {
        m_xCbx2->set_active(true);
        m_xData->CbxHdl(*m_xCbx2);
        m_xQSet2->SetItemColor( 1, aPipetteColor );
        m_xQSet2->SetFormat();
    }
    else if( m_xQSet3->GetSelectedItemId() == 1 )
    {
        m_xCbx3->set_active(true);
        m_xData->CbxHdl(*m_xCbx3);
        m_xQSet3->SetItemColor( 1, aPipetteColor );
        m_xQSet3->SetFormat();
    }
    else if( m_xQSet4->GetSelectedItemId() == 1 )
    {
        m_xCbx4->set_active(true);
        m_xData->CbxHdl(*m_xCbx4);
        m_xQSet4->SetItemColor( 1, aPipetteColor );
        m_xQSet4->SetFormat();
    }

    m_xTbxPipette->set_item_active(u"pipette"_ustr, false);
    m_xData->PipetteHdl(u"pipette"_ustr);
}

void SvxBmpMask::SetExecState( bool bEnable )
{
    m_xData->SetExecState( bEnable );

    if ( m_xData->IsExecReady() && m_xData->IsCbxReady() )
        m_xBtnExec->set_sensitive(true);
    else
        m_xBtnExec->set_sensitive(false);
}


sal_uInt16 SvxBmpMask::InitColorArrays( Color* pSrcCols, Color* pDstCols, sal_uInt8* pTols )
{
    sal_uInt16  nCount = 0;

    if ( m_xCbx1->get_active() )
    {
        pSrcCols[nCount] = m_xQSet1->GetItemColor( 1 );
        pDstCols[nCount] = m_xLbColor1->GetSelectEntryColor();
        pTols[nCount++] = static_cast<sal_uInt8>(m_xSp1->get_value(FieldUnit::PERCENT));
    }

    if ( m_xCbx2->get_active() )
    {
        pSrcCols[nCount] = m_xQSet2->GetItemColor( 1 );
        pDstCols[nCount] = m_xLbColor2->GetSelectEntryColor();
        pTols[nCount++] = static_cast<sal_uInt8>(m_xSp2->get_value(FieldUnit::PERCENT));
    }

    if ( m_xCbx3->get_active() )
    {
        pSrcCols[nCount] = m_xQSet3->GetItemColor( 1 );
        pDstCols[nCount] = m_xLbColor3->GetSelectEntryColor();
        pTols[nCount++] = static_cast<sal_uInt8>(m_xSp3->get_value(FieldUnit::PERCENT));
    }

    if ( m_xCbx4->get_active() )
    {
        pSrcCols[nCount] = m_xQSet4->GetItemColor( 1 );
        pDstCols[nCount] = m_xLbColor4->GetSelectEntryColor();
        pTols[nCount++] = static_cast<sal_uInt8>(m_xSp4->get_value(FieldUnit::PERCENT));
    }

    return nCount;
}

void SvxBmpMask::ImpMask( BitmapEx& rBitmap )
{
    Color           pSrcCols[4];
    Color           pDstCols[4];
    sal_uInt8       pTols[4];
    const sal_uInt16 nCount = InitColorArrays( pSrcCols, pDstCols, pTols );

    EnterWait();
    rBitmap.Replace( pSrcCols, pDstCols, nCount, pTols );
    LeaveWait();
}

BitmapEx SvxBmpMask::ImpMaskTransparent( const BitmapEx& rBitmapEx, const Color& rColor, const sal_uInt8 nTol )
{
    EnterWait();

    BitmapEx    aBmpEx;
    AlphaMask   aMask( rBitmapEx.GetBitmap().CreateAlphaMask( rColor, nTol ) );

    if( rBitmapEx.IsAlpha() )
        aMask.AlphaCombineOr( rBitmapEx.GetAlphaMask() );

    aBmpEx = BitmapEx( rBitmapEx.GetBitmap(), aMask );
    LeaveWait();

    return aBmpEx;
}


Animation SvxBmpMask::ImpMask( const Animation& rAnimation )
{
    Animation   aAnimation( rAnimation );
    Color       pSrcCols[4];
    Color       pDstCols[4];
    sal_uInt8   pTols[4];
    InitColorArrays( pSrcCols, pDstCols, pTols );
    sal_uInt16  nAnimationCount = aAnimation.Count();

    for( sal_uInt16 i = 0; i < nAnimationCount; i++ )
    {
        AnimationFrame aAnimationFrame( aAnimation.Get( i ) );
        aAnimationFrame.maBitmapEx = Mask(aAnimationFrame.maBitmapEx).GetBitmapEx();
        aAnimation.Replace(aAnimationFrame, i);
    }

    return aAnimation;
}


GDIMetaFile SvxBmpMask::ImpMask( const GDIMetaFile& rMtf )
{
    GDIMetaFile aMtf;
    Color       pSrcCols[4];
    Color       pDstCols[4];
    sal_uInt8   pTols[4];
    sal_uInt16  nCount = InitColorArrays( pSrcCols, pDstCols, pTols );

    // If no color is selected, we copy only the Mtf
    if( !nCount )
        aMtf = rMtf;
    else
    {
        bool        pTrans[4];
        Color       aCol;
        tools::Long        nR;
        tools::Long        nG;
        tools::Long        nB;
        std::unique_ptr<tools::Long[]> pMinR(new tools::Long[nCount]);
        std::unique_ptr<tools::Long[]> pMaxR(new tools::Long[nCount]);
        std::unique_ptr<tools::Long[]> pMinG(new tools::Long[nCount]);
        std::unique_ptr<tools::Long[]> pMaxG(new tools::Long[nCount]);
        std::unique_ptr<tools::Long[]> pMinB(new tools::Long[nCount]);
        std::unique_ptr<tools::Long[]> pMaxB(new tools::Long[nCount]);
        sal_uInt16      i;

        aMtf.SetPrefSize( rMtf.GetPrefSize() );
        aMtf.SetPrefMapMode( rMtf.GetPrefMapMode() );

        // Prepare Color comparison array
        for( i = 0; i < nCount; i++ )
        {
            tools::Long nTol = ( pTols[i] * 255 ) / 100;

            tools::Long nVal = static_cast<tools::Long>(pSrcCols[i].GetRed());
            pMinR[i] = std::max( nVal - nTol, tools::Long(0) );
            pMaxR[i] = std::min( nVal + nTol, tools::Long(255) );

            nVal = static_cast<tools::Long>(pSrcCols[i].GetGreen());
            pMinG[i] = std::max( nVal - nTol, tools::Long(0) );
            pMaxG[i] = std::min( nVal + nTol, tools::Long(255) );

            nVal = static_cast<tools::Long>(pSrcCols[i].GetBlue());
            pMinB[i] = std::max( nVal - nTol, tools::Long(0) );
            pMaxB[i] = std::min( nVal + nTol, tools::Long(255) );

            pTrans[ i ] = (pDstCols[ i ] == COL_TRANSPARENT);
        }

        // Investigate actions and if necessary replace colors
        for( size_t nAct = 0, nActCount = rMtf.GetActionSize(); nAct < nActCount; nAct++ )
        {
            MetaAction* pAction = rMtf.GetAction( nAct );

            bool bReplace = false;

            switch( pAction->GetType() )
            {
                case MetaActionType::PIXEL:
                {
                    MetaPixelAction* pAct = static_cast<MetaPixelAction*>(pAction);

                    aCol = pAct->GetColor();
                    TEST_COLS();

                    if( bReplace )
                        pAct = new MetaPixelAction( pAct->GetPoint(), aCol );

                    aMtf.AddAction( pAct );
                }
                break;

                case MetaActionType::LINECOLOR:
                {
                    MetaLineColorAction* pAct = static_cast<MetaLineColorAction*>(pAction);

                    aCol = pAct->GetColor();
                    TEST_COLS();

                    if( bReplace )
                        pAct = new MetaLineColorAction( aCol, !pTrans[ i ] );

                    aMtf.AddAction( pAct );
                }
                break;

                case MetaActionType::FILLCOLOR:
                {
                    MetaFillColorAction* pAct = static_cast<MetaFillColorAction*>(pAction);

                    aCol = pAct->GetColor();
                    TEST_COLS();

                    if( bReplace )
                        pAct = new MetaFillColorAction( aCol, !pTrans[ i ] );

                    aMtf.AddAction( pAct );
                }
                break;

                case MetaActionType::TEXTCOLOR:
                {
                    MetaTextColorAction* pAct = static_cast<MetaTextColorAction*>(pAction);

                    aCol = pAct->GetColor();
                    TEST_COLS();

                    if( bReplace )
                        pAct = new MetaTextColorAction( aCol );

                    aMtf.AddAction( pAct );
                }
                break;

                case MetaActionType::TEXTFILLCOLOR:
                {
                    MetaTextFillColorAction* pAct = static_cast<MetaTextFillColorAction*>(pAction);

                    aCol = pAct->GetColor();
                    TEST_COLS();

                    if( bReplace )
                        pAct = new MetaTextFillColorAction( aCol, !pTrans[ i ] );

                    aMtf.AddAction( pAct );
                }
                break;

                case MetaActionType::FONT:
                {
                    MetaFontAction* pAct = static_cast<MetaFontAction*>(pAction);
                    vcl::Font       aFont( pAct->GetFont() );

                    aCol = aFont.GetColor();
                    TEST_COLS();

                    if( bReplace )
                    {
                        aFont.SetColor( aCol );
                        pAct = new MetaFontAction( std::move(aFont) );
                    }

                    aMtf.AddAction( pAct );
                }
                break;

                case MetaActionType::WALLPAPER:
                {
                    MetaWallpaperAction*    pAct = static_cast<MetaWallpaperAction*>(pAction);
                    Wallpaper               aWall( pAct->GetWallpaper() );

                    aCol = aWall.GetColor();
                    TEST_COLS();

                    if( bReplace )
                    {
                        aWall.SetColor( aCol );
                        pAct = new MetaWallpaperAction( pAct->GetRect(), std::move(aWall) );
                    }

                    aMtf.AddAction( pAct );
                }
                break;

                case MetaActionType::BMP:
                {
                    MetaBmpAction*  pAct = static_cast<MetaBmpAction*>(pAction);
                    const Bitmap    aBmp( Mask(BitmapEx(pAct->GetBitmap())).GetBitmapEx().GetBitmap() );

                    pAct = new MetaBmpAction( pAct->GetPoint(), aBmp );
                    aMtf.AddAction( pAct );
                }
                break;

                case MetaActionType::BMPSCALE:
                {
                    MetaBmpScaleAction* pAct = static_cast<MetaBmpScaleAction*>(pAction);
                    const Bitmap        aBmp( Mask(BitmapEx(pAct->GetBitmap())).GetBitmapEx().GetBitmap() );

                    pAct = new MetaBmpScaleAction( pAct->GetPoint(), pAct->GetSize(), aBmp );
                    aMtf.AddAction( pAct );
                }
                break;

                case MetaActionType::BMPSCALEPART:
                {
                    MetaBmpScalePartAction* pAct = static_cast<MetaBmpScalePartAction*>(pAction);
                    const Bitmap            aBmp( Mask(BitmapEx(pAct->GetBitmap())).GetBitmapEx().GetBitmap() );

                    pAct = new MetaBmpScalePartAction( pAct->GetDestPoint(), pAct->GetDestSize(),
                                                       pAct->GetSrcPoint(), pAct->GetSrcSize(), aBmp );
                    aMtf.AddAction( pAct );
                }
                break;

                case MetaActionType::BMPEX:
                {
                    MetaBmpExAction*    pAct = static_cast<MetaBmpExAction*>(pAction);
                    const BitmapEx      aBmpEx( Mask( pAct->GetBitmapEx() ).GetBitmapEx() );

                    pAct = new MetaBmpExAction( pAct->GetPoint(), aBmpEx );
                    aMtf.AddAction( pAct );
                }
                break;

                case MetaActionType::BMPEXSCALE:
                {
                    MetaBmpExScaleAction*   pAct = static_cast<MetaBmpExScaleAction*>(pAction);
                    const BitmapEx          aBmpEx( Mask( pAct->GetBitmapEx() ).GetBitmapEx() );

                    pAct = new MetaBmpExScaleAction( pAct->GetPoint(), pAct->GetSize(), aBmpEx );
                    aMtf.AddAction( pAct );
                }
                break;

                case MetaActionType::BMPEXSCALEPART:
                {
                    MetaBmpExScalePartAction*   pAct = static_cast<MetaBmpExScalePartAction*>(pAction);
                    const BitmapEx              aBmpEx( Mask( pAct->GetBitmapEx() ).GetBitmapEx() );

                    pAct = new MetaBmpExScalePartAction( pAct->GetDestPoint(), pAct->GetDestSize(),
                                                         pAct->GetSrcPoint(), pAct->GetSrcSize(), aBmpEx );
                    aMtf.AddAction( pAct );
                }
                break;

                default:
                {
                    aMtf.AddAction( pAction );
                }
                break;
            }
        }
    }

    LeaveWait();

    return aMtf;
}


Animation SvxBmpMask::ImpReplaceTransparency( const Animation& rAnim, const Color& rColor )
{
    Animation   aAnimation( rAnim );
    sal_uInt16      nAnimationCount = aAnimation.Count();

    for( sal_uInt16 i = 0; i < nAnimationCount; i++ )
    {
        AnimationFrame aAnimationFrame(aAnimation.Get(i));
        aAnimationFrame.maBitmapEx.ReplaceTransparency(rColor);
        aAnimation.Replace(aAnimationFrame, i);
    }

    return aAnimation;
}


GDIMetaFile SvxBmpMask::ImpReplaceTransparency( const GDIMetaFile& rMtf, const Color& rColor )
{
    ScopedVclPtrInstance< VirtualDevice > pVDev;
    GDIMetaFile     aMtf;
    const MapMode&  rPrefMap = rMtf.GetPrefMapMode();
    const Size&     rPrefSize = rMtf.GetPrefSize();
    const size_t    nActionCount = rMtf.GetActionSize();

    pVDev->EnableOutput( false );
    aMtf.Record( pVDev );
    aMtf.SetPrefSize( rPrefSize );
    aMtf.SetPrefMapMode( rPrefMap );
    pVDev->SetLineColor( rColor );
    pVDev->SetFillColor( rColor );

    // retrieve one action at the time; first
    // set the whole area to the replacement color.
    pVDev->DrawRect( tools::Rectangle( rPrefMap.GetOrigin(), rPrefSize ) );
    for ( size_t i = 0; i < nActionCount; i++ )
    {
        MetaAction* pAct = rMtf.GetAction( i );
        aMtf.AddAction( pAct );
    }

    aMtf.Stop();
    aMtf.WindStart();

    return aMtf;
}

GDIMetaFile SvxBmpMask::GetMetaFile(const Graphic& rGraphic)
{
    // Replace transparency?
    if (m_xCbxTrans->get_active())
        return ImpReplaceTransparency(rGraphic.GetGDIMetaFile(), m_xLbColorTrans->GetSelectEntryColor());
    return ImpMask(rGraphic.GetGDIMetaFile());
}

Graphic SvxBmpMask::Mask( const Graphic& rGraphic )
{
    Graphic     aGraphic( rGraphic );
    const Color aReplColor( m_xLbColorTrans->GetSelectEntryColor() );

    switch( rGraphic.GetType() )
    {
        case GraphicType::Bitmap:
        {
            if( rGraphic.IsAnimated() )
            {
                // Replace transparency?
                if ( m_xCbxTrans->get_active() )
                    aGraphic = ImpReplaceTransparency( rGraphic.GetAnimation(), aReplColor );
                else
                    aGraphic = ImpMask( rGraphic.GetAnimation() );
            }
            else
            {
                // Replace transparency?
                if( m_xCbxTrans->get_active() )
                {
                    BitmapEx aBmpEx = aGraphic.GetBitmapEx();
                    aBmpEx.ReplaceTransparency(aReplColor);
                    aGraphic = aBmpEx;
                }
                else
                {
                    Color   pSrcCols[4];
                    Color   pDstCols[4];
                    sal_uInt8   pTols[4];
                    sal_uInt16  nCount = InitColorArrays( pSrcCols, pDstCols, pTols );

                    if( nCount )
                    {
                        // first set all transparent colors
                        for( sal_uInt16 i = 0; i < nCount; i++ )
                        {
                            // Do we have a transparent color?
                            if (pDstCols[i] == COL_TRANSPARENT)
                            {
                                BitmapEx    aBmpEx( ImpMaskTransparent( aGraphic.GetBitmapEx(),
                                                                        pSrcCols[ i ], pTols[ i ] ) );
                                const Size  aSize( aBmpEx.GetSizePixel() );

                                if( aSize.Width() && aSize.Height() )
                                    aGraphic = aBmpEx;
                            }
                        }

                        // now replace it again with the normal colors
                        BitmapEx  aBitmapEx( aGraphic.GetBitmapEx() );
                        if ( aBitmapEx.GetSizePixel().Width() && aBitmapEx.GetSizePixel().Height() )
                        {
                            ImpMask( aBitmapEx );
                            aGraphic = Graphic( aBitmapEx );
                        }
                    }
                }
            }
        }
        break;

        case GraphicType::GdiMetafile:
        {
            GDIMetaFile aMtf(GetMetaFile(rGraphic));
            Size aSize( aMtf.GetPrefSize() );
            if ( aSize.Width() && aSize.Height() )
                aGraphic = Graphic( aMtf );
            else
                aGraphic = rGraphic;
        }
        break;

        default:
            aGraphic = rGraphic;
        break;
    }

    if( aGraphic != rGraphic )
    {
        aGraphic.SetPrefSize( rGraphic.GetPrefSize() );
        aGraphic.SetPrefMapMode( rGraphic.GetPrefMapMode() );
    }

    return aGraphic;
}

bool SvxBmpMask::IsEyedropping() const
{
    return m_xTbxPipette->get_item_active(u"pipette"_ustr);
}

/** Set an accessible name for the source color check boxes.  Without this
    the lengthy description is read.
*/
void SvxBmpMask::SetAccessibleNames()
{
    // set the accessible name for valueset
    OUString sColorPalette (SvxResId( RID_SVXDLG_BMPMASK_STR_PALETTE));
    OUString sColorPaletteN;

    sColorPaletteN = sColorPalette + " 1";
    m_xQSet1->SetText (sColorPaletteN);
    sColorPaletteN = sColorPalette + " 2";
    m_xQSet2->SetText (sColorPaletteN);
    sColorPaletteN = sColorPalette + " 3";
    m_xQSet3->SetText (sColorPaletteN);
    sColorPaletteN = sColorPalette + " 4";
    m_xQSet4->SetText (sColorPaletteN);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
