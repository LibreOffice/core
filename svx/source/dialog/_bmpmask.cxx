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

#include <vcl/wrkwin.hxx>
#include <vcl/metaact.hxx>
#include <vcl/settings.hxx>
#include <svtools/valueset.hxx>
#include <svl/eitem.hxx>
#include <sfx2/dispatch.hxx>
#include <svtools/colrdlg.hxx>

#include <svx/colorbox.hxx>
#include <svx/dialmgr.hxx>
#include <svx/bmpmask.hxx>
#include <svx/dialogs.hrc>
#include <bmpmask.hrc>
#include <svx/svxids.hrc>
#include <memory>
#include "helpid.hrc"

#define BMP_RESID(nId)  ResId(nId, DIALOG_MGR())
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

class ColorWindow : public Control
{
    Color       aColor;


public:
    explicit ColorWindow(vcl::Window* pParent)
        : Control(pParent, WB_BORDER)
        , aColor( COL_WHITE )
    {
    }

    void SetColor( const Color& rColor )
    {
        aColor = rColor;
        Invalidate();
    }

    virtual void Paint( vcl::RenderContext& /*rRenderContext*/, const Rectangle& rRect ) override;

    virtual Size GetOptimalSize() const override
    {
        return LogicToPixel(Size(43, 14), MapUnit::MapAppFont);
    }
};

class MaskSet : public ValueSet
{
    VclPtr<SvxBmpMask> pSvxBmpMask;

public:
    MaskSet(SvxBmpMask* pMask, vcl::Window* pParent);
    virtual ~MaskSet() override { disposeOnce(); }
    virtual void dispose() override { pSvxBmpMask.clear(); ValueSet::dispose(); }
    virtual void Select() override;
    virtual void KeyInput( const KeyEvent& rKEvt ) override;
    virtual void GetFocus() override;
    virtual Size GetOptimalSize() const override
    {
        return LogicToPixel(Size(24, 12), MapUnit::MapAppFont);
    }

    void onEditColor();
};

MaskSet::MaskSet(SvxBmpMask* pMask, vcl::Window* pParent)
    : ValueSet(pParent, WB_TABSTOP)
    , pSvxBmpMask(pMask)
{
    SetHelpId(HID_BMPMASK_CTL_QCOL_1);
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

void MaskSet::KeyInput( const KeyEvent& rKEvt )
{
    vcl::KeyCode aCode = rKEvt.GetKeyCode();

    // if the key has a modifier we don't care
    if( aCode.GetModifier() )
    {
        ValueSet::KeyInput( rKEvt );
    }
    else
    {
        // check for keys that interests us
        switch ( aCode.GetCode() )
        {
            case KEY_SPACE:
                onEditColor();
                break;
            default:
                ValueSet::KeyInput( rKEvt );
        }

    }
}

void MaskSet::onEditColor()
{
    std::unique_ptr<SvColorDialog> pColorDlg(new SvColorDialog( GetParent() ));

    pColorDlg->SetColor(GetItemColor(1));

    if( pColorDlg->Execute() )
        SetItemColor( 1, pColorDlg->GetColor() );
}


class MaskData
{
    VclPtr<SvxBmpMask>     pMask;
    bool            bIsReady;
    bool            bExecState;
    SfxBindings&    rBindings;

public:
                MaskData( SvxBmpMask* pBmpMask, SfxBindings& rBind );

    bool        IsCbxReady() const { return bIsReady; }
    void        SetExecState( bool bState ) { bExecState = bState; }
    bool        IsExecReady() const { return bExecState; }

                DECL_LINK( PipetteHdl, ToolBox*, void );
                DECL_LINK( CbxHdl, Button*, void);
                DECL_LINK( CbxTransHdl, Button*, void );
                DECL_LINK( FocusLbHdl, Control&, void );
                DECL_LINK(ExecHdl, Button*, void);
};


MaskData::MaskData( SvxBmpMask* pBmpMask, SfxBindings& rBind ) :

    pMask       ( pBmpMask ),
    bIsReady    ( false ),
    bExecState  ( false ),
    rBindings   ( rBind )

{
}

IMPL_LINK( MaskData, PipetteHdl, ToolBox*, pTbx, void )
{
    SfxBoolItem aBItem( SID_BMPMASK_PIPETTE,
                        pTbx->IsItemChecked( pTbx->GetItemId(0) ) );

    rBindings.GetDispatcher()->ExecuteList(SID_BMPMASK_PIPETTE, OWN_CALLMODE,
            { &aBItem });
}

IMPL_LINK( MaskData, CbxHdl, Button*, pButton, void )
{
    CheckBox* pCbx = static_cast<CheckBox*>(pButton);
    bIsReady =  pMask->m_pCbx1->IsChecked() || pMask->m_pCbx2->IsChecked() ||
                pMask->m_pCbx3->IsChecked() || pMask->m_pCbx4->IsChecked();

    if ( bIsReady && IsExecReady() )
        pMask->m_pBtnExec->Enable();
    else
        pMask->m_pBtnExec->Disable();

    // When a checkbox is checked, the pipette is enabled
    if ( pCbx->IsChecked() )
    {
        MaskSet* pSet = nullptr;

        if (pCbx == pMask->m_pCbx1)
            pSet = pMask->m_pQSet1;
        else if (pCbx == pMask->m_pCbx2)
            pSet = pMask->m_pQSet2;
        else if (pCbx == pMask->m_pCbx3)
            pSet = pMask->m_pQSet3;
        else // if ( pCbx == pMask->m_pCbx4 )
            pSet = pMask->m_pQSet4;

        pSet->SelectItem( 1 );
        pSet->Select();

        pMask->m_pTbxPipette->CheckItem( pMask->m_pTbxPipette->GetItemId(0) );
        PipetteHdl(pMask->m_pTbxPipette);
    }
}


IMPL_LINK( MaskData, CbxTransHdl, Button*, pButton, void )
{
    CheckBox* pCbx = static_cast<CheckBox*>(pButton);
    bIsReady = pCbx->IsChecked();
    if ( bIsReady )
    {
        pMask->m_pQSet1->Disable();
        pMask->m_pQSet2->Disable();
        pMask->m_pQSet3->Disable();
        pMask->m_pQSet4->Disable();
        pMask->m_pCtlPipette->Disable();
        pMask->m_pCbx1->Disable();
        pMask->m_pSp1->Disable();
        pMask->m_pCbx2->Disable();
        pMask->m_pSp2->Disable();
        pMask->m_pCbx3->Disable();
        pMask->m_pSp3->Disable();
        pMask->m_pCbx4->Disable();
        pMask->m_pSp4->Disable();
        pMask->m_pTbxPipette->Disable();

        pMask->m_pLbColor1->Disable();
        pMask->m_pLbColor2->Disable();
        pMask->m_pLbColor3->Disable();
        pMask->m_pLbColor4->Disable();
        pMask->m_pLbColorTrans->Enable();
    }
    else
    {
        pMask->m_pQSet1->Enable();
        pMask->m_pQSet2->Enable();
        pMask->m_pQSet3->Enable();
        pMask->m_pQSet4->Enable();
        pMask->m_pCtlPipette->Enable();
        pMask->m_pCbx1->Enable();
        pMask->m_pSp1->Enable();
        pMask->m_pCbx2->Enable();
        pMask->m_pSp2->Enable();
        pMask->m_pCbx3->Enable();
        pMask->m_pSp3->Enable();
        pMask->m_pCbx4->Enable();
        pMask->m_pSp4->Enable();
        pMask->m_pTbxPipette->Enable();

        pMask->m_pLbColor1->Enable();
        pMask->m_pLbColor2->Enable();
        pMask->m_pLbColor3->Enable();
        pMask->m_pLbColor4->Enable();
        pMask->m_pLbColorTrans->Disable();

        bIsReady = pMask->m_pCbx1->IsChecked() || pMask->m_pCbx2->IsChecked() ||
                   pMask->m_pCbx3->IsChecked() || pMask->m_pCbx4->IsChecked();
    }

    if ( bIsReady && IsExecReady() )
        pMask->m_pBtnExec->Enable();
    else
        pMask->m_pBtnExec->Disable();
}


IMPL_LINK( MaskData, FocusLbHdl, Control&, rControl, void )
{
    SvxColorListBox* pLb = static_cast<SvxColorListBox*>(&rControl);
    // MT: bFireFox as API parameter is ugly, find better solution????
    pMask->m_pQSet1->SelectItem( pLb == pMask->m_pLbColor1 ? 1 : 0 /* , false */ );
    pMask->m_pQSet2->SelectItem( pLb == pMask->m_pLbColor2 ? 1 : 0 /* , false */ );
    pMask->m_pQSet3->SelectItem( pLb == pMask->m_pLbColor3 ? 1 : 0 /* , false */ );
    pMask->m_pQSet4->SelectItem( pLb == pMask->m_pLbColor4 ? 1 : 0 /* , false */ );
}


IMPL_LINK_NOARG(MaskData, ExecHdl, Button*, void)
{
    SfxBoolItem aBItem( SID_BMPMASK_EXEC, true );
    rBindings.GetDispatcher()->ExecuteList(SID_BMPMASK_EXEC, OWN_CALLMODE,
            { &aBItem });
}

void ColorWindow::Paint( vcl::RenderContext& rRenderContext, const Rectangle& /*Rect*/)
{
    rRenderContext.Push(PushFlags::LINECOLOR | PushFlags::FILLCOLOR);
    rRenderContext.SetLineColor(aColor);
    rRenderContext.SetFillColor(aColor);
    rRenderContext.DrawRect(Rectangle(Point(), GetSizePixel()));
    rRenderContext.Pop();
}

SvxBmpMaskSelectItem::SvxBmpMaskSelectItem( SvxBmpMask& rMask,
                                            SfxBindings& rBindings ) :
            SfxControllerItem   ( SID_BMPMASK_EXEC, rBindings ),
            rBmpMask            ( rMask)
{
}

void SvxBmpMaskSelectItem::StateChanged( sal_uInt16 nSID, SfxItemState /*eState*/,
                                         const SfxPoolItem* pItem )
{
    if ( ( nSID == SID_BMPMASK_EXEC ) && pItem )
    {
        const SfxBoolItem* pStateItem = dynamic_cast<const SfxBoolItem*>( pItem  );
        assert(pStateItem); //SfxBoolItem erwartet
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
    : SfxDockingWindow(pBindinx, pCW, pParent, "DockingColorReplace",
                       "svx/ui/dockingcolorreplace.ui" )
    , pData(new MaskData(this, *pBindinx))
    , aPipetteColor(COL_WHITE)
    , aSelItem(*this, *pBindinx)
{
    get(m_pTbxPipette, "toolbar");
    m_pTbxPipette->SetItemBits(m_pTbxPipette->GetItemId(0),
        ToolBoxItemBits::AUTOCHECK);
    get(m_pBtnExec, "replace");
    m_pCtlPipette = VclPtr<ColorWindow>::Create(get<Window>("toolgrid"));
    m_pCtlPipette->Show();
    m_pCtlPipette->set_grid_left_attach(1);
    m_pCtlPipette->set_grid_top_attach(0);
    m_pCtlPipette->set_hexpand(true);
    get(m_pCbx1, "cbx1");
    Window *pGrid = get<Window>("colorgrid");
    m_pQSet1 = VclPtr<MaskSet>::Create(this, pGrid);
    m_pQSet1->set_grid_left_attach(1);
    m_pQSet1->set_grid_top_attach(1);
    m_pQSet1->Show();
    get(m_pSp1, "tol1");
    get(m_pLbColor1, "color1");
    m_pLbColor1->SetSlotId(SID_BMPMASK_COLOR);
    get(m_pCbx2, "cbx2");
    m_pQSet2 = VclPtr<MaskSet>::Create(this, pGrid);
    m_pQSet2->set_grid_left_attach(1);
    m_pQSet2->set_grid_top_attach(2);
    m_pQSet2->Show();
    get(m_pSp2, "tol2");
    get(m_pLbColor2, "color2");
    m_pLbColor2->SetSlotId(SID_BMPMASK_COLOR);
    get(m_pCbx3, "cbx3");
    m_pQSet3 = VclPtr<MaskSet>::Create(this, pGrid);
    m_pQSet3->set_grid_left_attach(1);
    m_pQSet3->set_grid_top_attach(3);
    m_pQSet3->Show();
    get(m_pSp3, "tol3");
    get(m_pLbColor3, "color3");
    m_pLbColor3->SetSlotId(SID_BMPMASK_COLOR);
    get(m_pCbx4, "cbx4");
    m_pQSet4   = VclPtr<MaskSet>::Create(this, pGrid);
    m_pQSet4->set_grid_left_attach(1);
    m_pQSet4->set_grid_top_attach(4);
    m_pQSet4->Show();
    get(m_pSp4, "tol4");
    get(m_pLbColor4, "color4");
    m_pLbColor4->SetSlotId(SID_BMPMASK_COLOR);
    get(m_pCbxTrans, "cbx5");
    get(m_pLbColorTrans, "color5");

    m_pLbColorTrans->SelectEntry(Color(COL_BLACK));
    m_pLbColor1->SelectEntry(Color(COL_TRANSPARENT));
    m_pLbColor2->SelectEntry(Color(COL_TRANSPARENT));
    m_pLbColor3->SelectEntry(Color(COL_TRANSPARENT));
    m_pLbColor4->SelectEntry(Color(COL_TRANSPARENT));

    m_pTbxPipette->SetSelectHdl( LINK( pData, MaskData, PipetteHdl ) );
    m_pBtnExec->SetClickHdl( LINK( pData, MaskData, ExecHdl ) );

    m_pCbx1->SetClickHdl( LINK( pData, MaskData, CbxHdl ) );
    m_pCbx2->SetClickHdl( LINK( pData, MaskData, CbxHdl ) );
    m_pCbx3->SetClickHdl( LINK( pData, MaskData, CbxHdl ) );
    m_pCbx4->SetClickHdl( LINK( pData, MaskData, CbxHdl ) );
    m_pCbxTrans->SetClickHdl( LINK( pData, MaskData, CbxTransHdl ) );

    SetAccessibleNames ();

    m_pLbColor1->SetGetFocusHdl( LINK( pData, MaskData, FocusLbHdl ) );
    m_pLbColor2->SetGetFocusHdl( LINK( pData, MaskData, FocusLbHdl ) );
    m_pLbColor3->SetGetFocusHdl( LINK( pData, MaskData, FocusLbHdl ) );
    m_pLbColor4->SetGetFocusHdl( LINK( pData, MaskData, FocusLbHdl ) );
    m_pLbColorTrans->Disable();

    OUString sColorPalette (BMP_RESID( RID_SVXDLG_BMPMASK_STR_PALETTE));
    OUString sColorPaletteN;

    m_pQSet1->SetStyle( m_pQSet1->GetStyle() | WB_DOUBLEBORDER | WB_ITEMBORDER );
    m_pQSet1->SetColCount();
    m_pQSet1->SetLineCount( 1 );
    sColorPaletteN = sColorPalette + " 1";
    m_pQSet1->InsertItem( 1, aPipetteColor, sColorPaletteN);
    m_pQSet1->SelectItem( 1 );

    m_pQSet2->SetStyle( m_pQSet2->GetStyle() | WB_DOUBLEBORDER | WB_ITEMBORDER );
    m_pQSet2->SetColCount();
    m_pQSet2->SetLineCount( 1 );
    sColorPaletteN = sColorPalette + " 2";
    m_pQSet2->InsertItem( 1, aPipetteColor, sColorPaletteN);
    m_pQSet2->SelectItem( 0 );

    m_pQSet3->SetStyle( m_pQSet3->GetStyle() | WB_DOUBLEBORDER | WB_ITEMBORDER );
    m_pQSet3->SetColCount();
    m_pQSet3->SetLineCount( 1 );
    sColorPaletteN = sColorPalette + " 3";
    m_pQSet3->InsertItem( 1, aPipetteColor, sColorPaletteN);
    m_pQSet3->SelectItem( 0 );

    m_pQSet4->SetStyle( m_pQSet4->GetStyle() | WB_DOUBLEBORDER | WB_ITEMBORDER );
    m_pQSet4->SetColCount();
    m_pQSet4->SetLineCount( 1 );
    sColorPaletteN = sColorPalette + " 4";
    m_pQSet4->InsertItem( 1, aPipetteColor, sColorPaletteN);
    m_pQSet4->SelectItem( 0 );

    m_pQSet1->Show();
    m_pQSet2->Show();
    m_pQSet3->Show();
    m_pQSet4->Show();
}

SvxBmpMask::~SvxBmpMask()
{
    disposeOnce();
}

void SvxBmpMask::dispose()
{
    m_pQSet1.disposeAndClear();
    m_pQSet2.disposeAndClear();
    m_pQSet3.disposeAndClear();
    m_pQSet4.disposeAndClear();
    m_pCtlPipette.disposeAndClear();
    delete pData;
    pData = nullptr;
    m_pTbxPipette.clear();
    m_pBtnExec.clear();
    m_pCbx1.clear();
    m_pSp1.clear();
    m_pLbColor1.clear();
    m_pCbx2.clear();
    m_pSp2.clear();
    m_pLbColor2.clear();
    m_pCbx3.clear();
    m_pSp3.clear();
    m_pLbColor3.clear();
    m_pCbx4.clear();
    m_pSp4.clear();
    m_pLbColor4.clear();
    m_pCbxTrans.clear();
    m_pLbColorTrans.clear();
    aSelItem.dispose();
    SfxDockingWindow::dispose();
}

/** is called by a MaskSet when it is selected */
void SvxBmpMask::onSelect( MaskSet* pSet )
{
    // now deselect all other value sets
    if( pSet != m_pQSet1 )
        m_pQSet1->SelectItem( 0 );

    if( pSet != m_pQSet2 )
        m_pQSet2->SelectItem( 0 );

    if( pSet != m_pQSet3 )
        m_pQSet3->SelectItem( 0 );

    if( pSet != m_pQSet4 )
        m_pQSet4->SelectItem( 0 );
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
    m_pCtlPipette->SetColor( aPipetteColor );
}

void SvxBmpMask::PipetteClicked()
{
    if( m_pQSet1->GetSelectItemId() == 1 )
    {
        m_pCbx1->Check();
        pData->CbxHdl(m_pCbx1);
        m_pQSet1->SetItemColor( 1, aPipetteColor );
        m_pQSet1->SetFormat();
    }
    else if( m_pQSet2->GetSelectItemId() == 1 )
    {
        m_pCbx2->Check();
        pData->CbxHdl(m_pCbx2);
        m_pQSet2->SetItemColor( 1, aPipetteColor );
        m_pQSet2->SetFormat();
    }
    else if( m_pQSet3->GetSelectItemId() == 1 )
    {
        m_pCbx3->Check();
        pData->CbxHdl(m_pCbx3);
        m_pQSet3->SetItemColor( 1, aPipetteColor );
        m_pQSet3->SetFormat();
    }
    else if( m_pQSet4->GetSelectItemId() == 1 )
    {
        m_pCbx4->Check();
        pData->CbxHdl(m_pCbx4);
        m_pQSet4->SetItemColor( 1, aPipetteColor );
        m_pQSet4->SetFormat();
    }

    m_pTbxPipette->CheckItem( m_pTbxPipette->GetItemId(0), false );
    pData->PipetteHdl(m_pTbxPipette);
}

void SvxBmpMask::SetExecState( bool bEnable )
{
    pData->SetExecState( bEnable );

    if ( pData->IsExecReady() && pData->IsCbxReady() )
        m_pBtnExec->Enable();
    else
        m_pBtnExec->Disable();
}


sal_uInt16 SvxBmpMask::InitColorArrays( Color* pSrcCols, Color* pDstCols, sal_uIntPtr* pTols )
{
    sal_uInt16  nCount = 0;

    if ( m_pCbx1->IsChecked() )
    {
        pSrcCols[nCount] = m_pQSet1->GetItemColor( 1 );
        pDstCols[nCount] = m_pLbColor1->GetSelectEntryColor();
        pTols[nCount++] = static_cast<sal_uIntPtr>(m_pSp1->GetValue());
    }

    if ( m_pCbx2->IsChecked() )
    {
        pSrcCols[nCount] = m_pQSet2->GetItemColor( 1 );
        pDstCols[nCount] = m_pLbColor2->GetSelectEntryColor();
        pTols[nCount++] = static_cast<sal_uIntPtr>(m_pSp2->GetValue());
    }

    if ( m_pCbx3->IsChecked() )
    {
        pSrcCols[nCount] = m_pQSet3->GetItemColor( 1 );
        pDstCols[nCount] = m_pLbColor3->GetSelectEntryColor();
        pTols[nCount++] = static_cast<sal_uIntPtr>(m_pSp3->GetValue());
    }

    if ( m_pCbx4->IsChecked() )
    {
        pSrcCols[nCount] = m_pQSet4->GetItemColor( 1 );
        pDstCols[nCount] = m_pLbColor4->GetSelectEntryColor();
        pTols[nCount++] = static_cast<sal_uIntPtr>(m_pSp4->GetValue());
    }

    return nCount;
}

Bitmap SvxBmpMask::ImpMask( const Bitmap& rBitmap )
{
    Bitmap          aBitmap( rBitmap );
    Color           pSrcCols[4];
    Color           pDstCols[4];
    sal_uIntPtr         pTols[4];
    const sal_uInt16    nCount = InitColorArrays( pSrcCols, pDstCols, pTols );

    EnterWait();
    aBitmap.Replace( pSrcCols, pDstCols, nCount, pTols );
    LeaveWait();

    return aBitmap;
}

BitmapEx SvxBmpMask::ImpMaskTransparent( const BitmapEx& rBitmapEx, const Color& rColor, const long nTol )
{
    EnterWait();

    BitmapEx    aBmpEx;
    Bitmap      aMask( rBitmapEx.GetBitmap().CreateMask( rColor, nTol ) );

    if( rBitmapEx.IsTransparent() )
        aMask.CombineSimple( rBitmapEx.GetMask(), BmpCombine::Or );

    aBmpEx = BitmapEx( rBitmapEx.GetBitmap(), aMask );
    LeaveWait();

    return aBmpEx;
}


Animation SvxBmpMask::ImpMask( const Animation& rAnimation )
{
    Animation   aAnimation( rAnimation );
    Color       pSrcCols[4];
    Color       pDstCols[4];
    sal_uIntPtr     pTols[4];
    InitColorArrays( pSrcCols, pDstCols, pTols );
    sal_uInt16      nAnimationCount = aAnimation.Count();

    for( sal_uInt16 i = 0; i < nAnimationCount; i++ )
    {
        AnimationBitmap aAnimBmp( aAnimation.Get( i ) );
        aAnimBmp.aBmpEx = Mask( aAnimBmp.aBmpEx ).GetBitmapEx();
        aAnimation.Replace( aAnimBmp, i );
    }

    return aAnimation;
}


GDIMetaFile SvxBmpMask::ImpMask( const GDIMetaFile& rMtf )
{
    GDIMetaFile aMtf;
    Color       pSrcCols[4];
    Color       pDstCols[4];
    sal_uIntPtr     pTols[4];
    sal_uInt16      nCount = InitColorArrays( pSrcCols, pDstCols, pTols );

    // If no color is selected, we copy only the Mtf
    if( !nCount )
        aMtf = rMtf;
    else
    {
        bool        pTrans[4];
        Color       aCol;
        long        nR;
        long        nG;
        long        nB;
        std::unique_ptr<long[]> pMinR(new long[nCount]);
        std::unique_ptr<long[]> pMaxR(new long[nCount]);
        std::unique_ptr<long[]> pMinG(new long[nCount]);
        std::unique_ptr<long[]> pMaxG(new long[nCount]);
        std::unique_ptr<long[]> pMinB(new long[nCount]);
        std::unique_ptr<long[]> pMaxB(new long[nCount]);
        sal_uInt16      i;

        aMtf.SetPrefSize( rMtf.GetPrefSize() );
        aMtf.SetPrefMapMode( rMtf.GetPrefMapMode() );

        // Prepare Color comparison array
        for( i = 0; i < nCount; i++ )
        {
            long nTol = ( pTols[i] * 255L ) / 100L;

            long nVal = ( (long) pSrcCols[i].GetRed() );
            pMinR[i] = std::max( nVal - nTol, 0L );
            pMaxR[i] = std::min( nVal + nTol, 255L );

            nVal = ( (long) pSrcCols[i].GetGreen() );
            pMinG[i] = std::max( nVal - nTol, 0L );
            pMaxG[i] = std::min( nVal + nTol, 255L );

            nVal = ( (long) pSrcCols[i].GetBlue() );
            pMinB[i] = std::max( nVal - nTol, 0L );
            pMaxB[i] = std::min( nVal + nTol, 255L );

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
                    else
                        pAct->Duplicate();

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
                    else
                        pAct->Duplicate();

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
                    else
                        pAct->Duplicate();

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
                    else
                        pAct->Duplicate();

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
                    else
                        pAct->Duplicate();

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
                        pAct = new MetaFontAction( aFont );
                    }
                    else
                        pAct->Duplicate();

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
                        pAct = new MetaWallpaperAction( pAct->GetRect(), aWall );
                    }
                    else
                        pAct->Duplicate();

                    aMtf.AddAction( pAct );
                }
                break;

                case MetaActionType::BMP:
                {
                    MetaBmpAction*  pAct = static_cast<MetaBmpAction*>(pAction);
                    const Bitmap    aBmp( Mask( pAct->GetBitmap() ).GetBitmap() );

                    pAct = new MetaBmpAction( pAct->GetPoint(), aBmp );
                    aMtf.AddAction( pAct );
                }
                break;

                case MetaActionType::BMPSCALE:
                {
                    MetaBmpScaleAction* pAct = static_cast<MetaBmpScaleAction*>(pAction);
                    const Bitmap        aBmp( Mask( pAct->GetBitmap() ).GetBitmap() );

                    pAct = new MetaBmpScaleAction( pAct->GetPoint(), pAct->GetSize(), aBmp );
                    aMtf.AddAction( pAct );
                }
                break;

                case MetaActionType::BMPSCALEPART:
                {
                    MetaBmpScalePartAction* pAct = static_cast<MetaBmpScalePartAction*>(pAction);
                    const Bitmap            aBmp( Mask( pAct->GetBitmap() ).GetBitmap() );

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
                    pAction->Duplicate();
                    aMtf.AddAction( pAction );
                }
                break;
            }
        }
    }

    LeaveWait();

    return aMtf;
}


BitmapEx SvxBmpMask::ImpReplaceTransparency( const BitmapEx& rBmpEx, const Color& rColor )
{
    if( rBmpEx.IsTransparent() )
    {
        Bitmap aBmp( rBmpEx.GetBitmap() );
        aBmp.Replace( rBmpEx.GetMask(), rColor );
        return aBmp;
    }
    else
        return rBmpEx;
}


Animation SvxBmpMask::ImpReplaceTransparency( const Animation& rAnim, const Color& rColor )
{
    Animation   aAnimation( rAnim );
    sal_uInt16      nAnimationCount = aAnimation.Count();

    for( sal_uInt16 i = 0; i < nAnimationCount; i++ )
    {
        AnimationBitmap aAnimBmp( aAnimation.Get( i ) );
        aAnimBmp.aBmpEx = ImpReplaceTransparency( aAnimBmp.aBmpEx, rColor );
        aAnimation.Replace( aAnimBmp, i );
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
    pVDev->DrawRect( Rectangle( rPrefMap.GetOrigin(), rPrefSize ) );
    for ( size_t i = 0; i < nActionCount; i++ )
    {
        MetaAction* pAct = rMtf.GetAction( i );

        pAct->Duplicate();
        aMtf.AddAction( pAct );
    }

    aMtf.Stop();
    aMtf.WindStart();

    return aMtf;
}

GDIMetaFile SvxBmpMask::GetMetaFile(const Graphic& rGraphic)
{
    // Replace transparency?
    if (m_pCbxTrans->IsChecked())
        return ImpReplaceTransparency(rGraphic.GetGDIMetaFile(), m_pLbColorTrans->GetSelectEntryColor());
    return ImpMask(rGraphic.GetGDIMetaFile());
}

Graphic SvxBmpMask::Mask( const Graphic& rGraphic )
{
    Graphic     aGraphic( rGraphic );
    const Color aReplColor( m_pLbColorTrans->GetSelectEntryColor() );

    switch( rGraphic.GetType() )
    {
        case GraphicType::Bitmap:
        {
            if( rGraphic.IsAnimated() )
            {
                // Replace transparency?
                if ( m_pCbxTrans->IsChecked() )
                    aGraphic = ImpReplaceTransparency( rGraphic.GetAnimation(), aReplColor );
                else
                    aGraphic = ImpMask( rGraphic.GetAnimation() );
            }
            else
            {
                // Replace transparency?
                if( m_pCbxTrans->IsChecked() )
                {
                    if( aGraphic.IsTransparent() )
                    {
                        BitmapEx    aBmpEx( ImpReplaceTransparency( aGraphic.GetBitmapEx(), aReplColor ) );
                        const Size  aSize( aBmpEx.GetSizePixel() );

                        if( aSize.Width() && aSize.Height() )
                            aGraphic = aBmpEx;
                    }
                }
                else
                {
                    Color   pSrcCols[4];
                    Color   pDstCols[4];
                    sal_uIntPtr pTols[4];
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
                        Bitmap  aBitmap( ImpMask( aGraphic.GetBitmap() ) );
                        Size    aSize( aBitmap.GetSizePixel() );

                        if ( aSize.Width() && aSize.Height() )
                        {
                            if ( aGraphic.IsTransparent() )
                                aGraphic = Graphic( BitmapEx( aBitmap, aGraphic.GetBitmapEx().GetMask() ) );
                            else
                                aGraphic = aBitmap;
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
    return m_pTbxPipette->IsItemChecked( m_pTbxPipette->GetItemId(0) );
}

/** Set an accessible name for the source color check boxes.  Without this
    the lengthy description is read.
*/
void SvxBmpMask::SetAccessibleNames()
{
    // set the accessible name for valueset
    OUString sColorPalette (BMP_RESID( RID_SVXDLG_BMPMASK_STR_PALETTE));
    OUString sColorPaletteN;

    sColorPaletteN = sColorPalette + " 1";
    m_pQSet1->SetText (sColorPaletteN);
    sColorPaletteN = sColorPalette + " 2";
    m_pQSet2->SetText (sColorPaletteN);
    sColorPaletteN = sColorPalette + " 3";
    m_pQSet3->SetText (sColorPaletteN);
    sColorPaletteN = sColorPalette + " 4";
    m_pQSet4->SetText (sColorPaletteN);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
