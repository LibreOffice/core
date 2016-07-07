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
#include <vcl/msgbox.hxx>
#include <tools/urlobj.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dialoghelper.hxx>
#include <sfx2/filedlghelper.hxx>
#include <unotools/localfilehelper.hxx>
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"
#include <svx/dialmgr.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/settings.hxx>
#include <svx/dialogs.hrc>

#include <cuires.hrc>
#include "helpid.hrc"
#include "svx/xattr.hxx"
#include <svx/xpool.hxx>
#include <svx/xtable.hxx>
#include "svx/xoutbmp.hxx"
#include "svx/drawitem.hxx"
#include "cuitabarea.hxx"
#include "defdlgname.hxx"
#include "dlgname.hxx"
#include <svx/svxdlg.hxx>
#include <dialmgr.hxx>
#include "sfx2/opengrf.hxx"
#include "paragrph.hrc"


using namespace com::sun::star;


SvxBitmapTabPage::SvxBitmapTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs ) :

    SvxTabPage( pParent,
                "BitmapTabPage",
                "cui/ui/bitmaptabpage.ui",
               rInAttrs ),
    m_rOutAttrs (rInAttrs ),

    m_pBitmapList( nullptr ),

    m_pnBitmapListState( nullptr ),
    m_aXFStyleItem( drawing::FillStyle_BITMAP ),
    m_aXBitmapItem( OUString(), Graphic() ),

    m_pPageType(nullptr),
    m_nDlgType(0),
    m_pPos(nullptr),

    m_pbAreaTP( nullptr ),

    m_aXFillAttr          ( rInAttrs.GetPool() ),
    m_rXFSet              ( m_aXFillAttr.GetItemSet() )
{
    get(m_pBitmapLB, "bitmappresetlist");
    get(m_pCtlBitmapPreview,"CTL_BITMAP_PREVIEW");
    get(m_pRbOriginal, "originalrb");
    get(m_pRbStretch, "stretchrb");
    get(m_pVBoxStretch, "stretchmtr");
    get(m_pMtrWidth, "width");
    get(m_pMtrHeight, "height");
    get(m_pLbPosition, "positionlb");
    get(m_pTsbTile,"tilecb");
    get(m_pVBoxPosition, "positionbox");
    get(m_pBtnImport, "import");

    m_rXFSet.Put( m_aXFStyleItem );
    m_rXFSet.Put( m_aXBitmapItem );
    m_pCtlBitmapPreview->SetAttributes( m_aXFillAttr.GetItemSet() );

    m_pBitmapLB->SetSelectHdl( LINK( this, SvxBitmapTabPage, ChangeBitmapHdl ) );
    m_pBitmapLB->SetRenameHdl( LINK( this, SvxBitmapTabPage, ClickRenameHdl ) );
    m_pBitmapLB->SetDeleteHdl( LINK( this, SvxBitmapTabPage, ClickDeleteHdl ) );
    Link<RadioButton&,void> aLink = LINK( this, SvxBitmapTabPage, ModifySizeHdl_Impl );
    Link<Edit&,void> aLink2 = LINK( this, SvxBitmapTabPage, ModifyStretchHdl_Impl );
    m_pRbOriginal->SetToggleHdl( aLink );
    m_pRbStretch->SetToggleHdl( aLink );
    m_pLbPosition->SetSelectHdl( LINK( this, SvxBitmapTabPage, ModifyPositionHdl_Impl ) );
    m_pMtrWidth->SetModifyHdl(aLink2);
    m_pMtrHeight->SetModifyHdl(aLink2);
    m_pTsbTile->SetClickHdl( LINK( this, SvxBitmapTabPage, TileClickHdl_Impl ) );
    m_pBtnImport->SetClickHdl( LINK( this, SvxBitmapTabPage, ClickImportHdl_Impl ) );
}

SvxBitmapTabPage::~SvxBitmapTabPage()
{
    disposeOnce();
}

void SvxBitmapTabPage::dispose()
{
    m_pBitmapLB.clear();
    m_pCtlBitmapPreview.clear();
    m_pRbOriginal.clear();
    m_pRbStretch.clear();
    m_pVBoxStretch.clear();
    m_pMtrWidth.clear();
    m_pMtrHeight.clear();
    m_pLbPosition.clear();
    m_pTsbTile.clear();
    m_pVBoxPosition.clear();
    m_pBtnImport.clear();
    SvxTabPage::dispose();
}


void SvxBitmapTabPage::Construct()
{
    m_pBitmapLB->FillPresetListBox( *m_pBitmapList );
}


void SvxBitmapTabPage::ActivatePage( const SfxItemSet& rSet)
{
    XFillBitmapItem aBitmapItem( static_cast<const XFillBitmapItem&>(rSet.Get(XATTR_FILLBITMAP)) );
    if(!aBitmapItem.isPattern())
    {
        m_rXFSet.Put( aBitmapItem );
        m_pCtlBitmapPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
        m_pCtlBitmapPreview->Invalidate();
    }
    else
    {
        m_pCtlBitmapPreview->Disable();
        return;
    }
    if( m_nDlgType == 0 ) // area dialog
    {
        *m_pbAreaTP = false;

        if( m_pBitmapList.is() )
        {
            if( *m_pnBitmapListState & ChangeType::CHANGED ||
                *m_pnBitmapListState & ChangeType::MODIFIED)
            {
                if( *m_pnBitmapListState & ChangeType::CHANGED )
                    m_pBitmapList = static_cast<SvxAreaTabDialog*>( GetParentDialog() )->GetNewBitmapList();
            }
            m_pBitmapLB->Clear();
            m_pBitmapLB->FillPresetListBox( *m_pBitmapList );
            if( *m_pPos != LISTBOX_ENTRY_NOTFOUND )
            {
                m_pBitmapLB->SelectItem( *m_pPos + 1 );
            }
            ChangeBitmapHdl_Impl();
            *m_pPageType = PT_BITMAP;
            *m_pPos = LISTBOX_ENTRY_NOTFOUND;
        }
    }
}


DeactivateRC SvxBitmapTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if( _pSet )
        FillItemSet( _pSet );

    return DeactivateRC::LeavePage;
}


bool SvxBitmapTabPage::FillItemSet( SfxItemSet* rAttrs )
{
    bool bModified = false;
    if( m_nDlgType == 0 && *m_pPageType == PT_BITMAP && !*m_pbAreaTP )
    {
        rAttrs->Put(XFillStyleItem(drawing::FillStyle_BITMAP));
        sal_Int32 nPos = m_pBitmapLB->GetSelectItemId();
        if(nPos != 0)
        {
            const XBitmapEntry* pXBitmapEntry = m_pBitmapList->GetBitmap(nPos - 1);
            const OUString aString(m_pBitmapLB->GetItemText( nPos ));
            rAttrs->Put(XFillBitmapItem(aString, pXBitmapEntry->GetGraphicObject()));
            bModified = true;
        }

        const SfxPoolItem* pOld = nullptr;
        if(m_pTsbTile->IsEnabled())
        {
            TriState eState = m_pTsbTile->GetState();
            if( m_pTsbTile->IsValueChangedFromSaved() )
            {
                XFillBmpTileItem aFillBmpTileItem( eState != TRISTATE_FALSE );
                pOld = GetOldItem( *rAttrs, XATTR_FILLBMP_TILE );
                if( !pOld || !( *static_cast<const XFillBmpTileItem*>(pOld) == aFillBmpTileItem ) )
                {
                    rAttrs->Put( aFillBmpTileItem );
                    bModified = true;
                }
                XFillBmpStretchItem aFillStretchItem( false );
                rAttrs->Put( aFillStretchItem );
            }
        }
        if( m_pLbPosition->IsEnabled() )
        {
            bool bPut = false;
            RECT_POINT _eRP = static_cast<RECT_POINT>(m_pLbPosition->GetSelectEntryPos());

            if( SfxItemState::DONTCARE == m_rOutAttrs.GetItemState( XATTR_FILLBMP_POS ) )
                bPut = true;
            else
            {
                RECT_POINT eValue = static_cast<const XFillBmpPosItem&>( m_rOutAttrs.Get( XATTR_FILLBMP_POS ) ).GetValue();
                if( eValue != _eRP )
                    bPut = true;
            }
            if( bPut )
            {
                XFillBmpPosItem aFillBmpPosItem( _eRP );
                pOld = GetOldItem( *rAttrs, XATTR_FILLBMP_POS );
                if ( !pOld || !( *static_cast<const XFillBmpPosItem*>(pOld) == aFillBmpPosItem ) )
                {
                    rAttrs->Put( aFillBmpPosItem );
                    bModified = true;
                }
            }
        }

        OUString aStr = m_pMtrWidth->GetText();
        {
            std::unique_ptr<XFillBmpSizeXItem> pItem;

            if( m_pMtrWidth->IsEnabled() )
                    pItem.reset(new XFillBmpSizeXItem( -labs( static_cast<long>(m_pMtrWidth->GetValue()) ) ));
            else if( m_pRbOriginal->IsChecked() )
                pItem.reset(new XFillBmpSizeXItem( 0 ));

            if( pItem )
            {
                pOld = GetOldItem( *rAttrs, XATTR_FILLBMP_SIZEX );
                if ( !pOld || !( *static_cast<const XFillBmpSizeXItem*>(pOld) == *pItem ) )
                {
                    rAttrs->Put( *pItem );
                    bModified = true;
                }
            }
        }

        aStr = m_pMtrHeight->GetText();
        {
            std::unique_ptr<XFillBmpSizeYItem> pItem;
            if( m_pMtrHeight->IsEnabled() )
                    pItem.reset(new XFillBmpSizeYItem( -labs( static_cast<long>(m_pMtrHeight->GetValue()) ) ));
            else if( m_pRbOriginal->IsChecked() )
                pItem.reset(new XFillBmpSizeYItem( 0 ));

            if( pItem )
            {
                pOld = GetOldItem( *rAttrs, XATTR_FILLBMP_SIZEY );
                if ( !pOld || !( *static_cast<const XFillBmpSizeYItem*>(pOld) == *pItem ) )
                {
                    rAttrs->Put( *pItem );
                    bModified = true;
                }
            }
        }
    }
    return bModified;
}


void SvxBitmapTabPage::Reset( const SfxItemSet* rAttrs )
{
    XFillBitmapItem aItem( static_cast<const XFillBitmapItem&>(rAttrs->Get(XATTR_FILLBITMAP)) );
    if(!aItem.isPattern())
    {
        m_rXFSet.Put( aItem );
        m_pCtlBitmapPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
        m_pCtlBitmapPreview->Invalidate();
    }
    else
    {
        m_pCtlBitmapPreview->Disable();
        return;
    }

    bool bOriginal = false;
    if( rAttrs->GetItemState( XATTR_FILLBMP_SIZEX ) != SfxItemState::DONTCARE )
    {
        sal_Int32 nValue = static_cast<const XFillBmpSizeXItem&>( rAttrs->Get( XATTR_FILLBMP_SIZEX ) ).GetValue();
        m_pMtrWidth->SetValue( labs(nValue) );
        if(nValue == 0)
        {
            bOriginal = true;
            m_pMtrWidth->SetValue(100);
        }
        else
            bOriginal = false;
        m_pMtrWidth->SaveValue();
    }
    else
    {
        m_pMtrWidth->SetText( "" );
        m_pMtrWidth->SaveValue();
    }

    if( rAttrs->GetItemState( XATTR_FILLBMP_SIZEY ) != SfxItemState::DONTCARE )
    {
        sal_Int32 nValue = static_cast<const XFillBmpSizeYItem&>( rAttrs->Get( XATTR_FILLBMP_SIZEY ) ).GetValue();
        m_pMtrHeight->SetValue( labs(nValue) );
        if(nValue == 0)
        {
            bOriginal = true;
            m_pMtrHeight->SetValue(100);
        }
        else
            bOriginal = false;
        m_pMtrHeight->SaveValue();
    }
    else
    {
        m_pMtrHeight->SetText( "" );
        m_pMtrHeight->SaveValue();
    }

    m_pRbOriginal->SetState( bOriginal );
    m_pRbStretch->SetState( !bOriginal );
    if( bOriginal )
        m_pVBoxStretch->Disable();
    else
        m_pVBoxStretch->Enable();

    if( rAttrs->GetItemState( XATTR_FILLBMP_TILE ) != SfxItemState::DONTCARE )
    {
        m_pTsbTile->EnableTriState( false );
        if( static_cast<const XFillBmpTileItem&>( rAttrs->Get(XATTR_FILLBMP_TILE) ).GetValue() )
            m_pTsbTile->SetState(TRISTATE_TRUE);
        else
            m_pTsbTile->SetState(TRISTATE_FALSE);
    }

    if( rAttrs->GetItemState( XATTR_FILLBMP_POS ) != SfxItemState::DONTCARE )
    {
        RECT_POINT eRP = static_cast<const XFillBmpPosItem&>( rAttrs->Get( XATTR_FILLBMP_POS ) ).GetValue();
        m_pLbPosition->SelectEntryPos(static_cast< sal_Int32 >(eRP));
    }
    ClickBitmapHdl_Impl();
    ModifyBitmapHdl_Impl();
    m_pTsbTile->SaveValue();
    m_pRbOriginal->SaveValue();
    m_pRbStretch->SaveValue();
    m_pMtrWidth->SaveValue();
    m_pMtrHeight->SaveValue();
    m_pLbPosition->SaveValue();
}


VclPtr<SfxTabPage> SvxBitmapTabPage::Create( vcl::Window* pWindow,
                                           const SfxItemSet* rAttrs )
{
    return VclPtr<SvxBitmapTabPage>::Create( pWindow, *rAttrs );
}


void SvxBitmapTabPage::ClickBitmapHdl_Impl()
{
    m_pCtlBitmapPreview->Enable();

    ChangeBitmapHdl_Impl();
}


void SvxBitmapTabPage::ModifyBitmapHdl_Impl()
{
    if( m_pRbStretch->IsEnabled() && m_pRbStretch->IsChecked())
        m_rXFSet.Put( XFillBmpSizeLogItem( m_pRbStretch->IsChecked() == false ) );
    if( m_pMtrWidth->IsEnabled() )
    {
        std::unique_ptr<XFillBmpSizeXItem> pItem;
        pItem.reset( new XFillBmpSizeXItem( -labs( static_cast<long>( m_pMtrWidth->GetValue() ) ) ) );
        m_rXFSet.Put(*pItem);
    }
    if( m_pMtrHeight->IsEnabled() )
    {
        std::unique_ptr<XFillBmpSizeYItem> pItem;
        pItem.reset( new XFillBmpSizeYItem( -labs( static_cast<long>( m_pMtrHeight->GetValue() ) ) ) );
        m_rXFSet.Put(*pItem);
    }
    if( m_pRbOriginal->IsChecked())
    {
        m_rXFSet.Put( XFillBmpSizeXItem(0) );
        m_rXFSet.Put( XFillBmpSizeYItem(0) );
        m_rXFSet.Put( XFillBmpSizeLogItem(true) );
    }
    TriState eState = m_pTsbTile->GetState();
    if(eState == TRISTATE_TRUE)
    {
        m_pVBoxPosition->Disable();
    }
    else if(eState == TRISTATE_FALSE)
    {
        m_pVBoxPosition->Enable();
    }
    m_rXFSet.Put( XFillBmpStretchItem( false ) );
    m_rXFSet.Put( XFillBmpTileItem( eState != TRISTATE_FALSE ) );

    if(m_pLbPosition->IsEnabled())
    {
        RECT_POINT eRP = (RECT_POINT)m_pLbPosition->GetSelectEntryPos();
        m_rXFSet.Put( XFillBmpPosItem( eRP ) );
    }

    m_pCtlBitmapPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
    m_pCtlBitmapPreview->Invalidate();
}


IMPL_LINK_NOARG_TYPED(SvxBitmapTabPage, ChangeBitmapHdl, ValueSet*, void)
{
    ChangeBitmapHdl_Impl();
}

void SvxBitmapTabPage::ChangeBitmapHdl_Impl()
{
    std::unique_ptr<GraphicObject> pGraphicObject;
    sal_Int32 nPos = m_pBitmapLB->GetSelectItemId();
    const SfxPoolItem* pPoolItem = nullptr;
    if( nPos != 0 )
    {
        pGraphicObject.reset(new GraphicObject(m_pBitmapList->GetBitmap(nPos - 1)->GetGraphicObject()));
    }
    else if( SfxItemState::SET == m_rOutAttrs.GetItemState( GetWhich( XATTR_FILLBITMAP ), true, &pPoolItem ) )
    {
        pGraphicObject.reset(new GraphicObject(static_cast<const XFillBitmapItem*>(pPoolItem)->GetGraphicObject()));
    }
    else
    {
        m_pBitmapLB->SelectItem(1);
        nPos = m_pBitmapLB->GetSelectItemId();
        if(nPos != 0)
        {
            pGraphicObject.reset(new GraphicObject(m_pBitmapList->GetBitmap(nPos - 1)->GetGraphicObject()));
        }
    }
    m_rXFSet.Put( XFillStyleItem(drawing::FillStyle_BITMAP) );
    m_rXFSet.Put( XFillBitmapItem(OUString(), Graphic(pGraphicObject->GetGraphic())) );
    m_pCtlBitmapPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
    m_pCtlBitmapPreview->Invalidate();
}

IMPL_LINK_NOARG_TYPED( SvxBitmapTabPage, ClickRenameHdl, SvxPresetListBox*, void )
{

}

IMPL_LINK_NOARG_TYPED( SvxBitmapTabPage, ClickDeleteHdl, SvxPresetListBox*, void )
{
    sal_Int32 nPos = m_pBitmapLB->GetSelectItemId();

    if( nPos != 0 )
    {
        ScopedVclPtrInstance< MessageDialog > aQueryBox( GetParentDialog(),"AskDelBitmapDialog","cui/ui/querydeletebitmapdialog.ui" );

        if( aQueryBox->Execute() == RET_YES )
        {
            delete m_pBitmapList->Remove( nPos - 1 );
            m_pBitmapLB->Clear();
            m_pBitmapLB->FillPresetListBox( *m_pBitmapList );
            m_pBitmapLB->SelectItem( 1 );

            ChangeBitmapHdl_Impl();

            *m_pnBitmapListState |= ChangeType::MODIFIED;
        }
    }
}

IMPL_LINK_TYPED(SvxBitmapTabPage, ModifySizeHdl_Impl, RadioButton& , pButton, void)
{
    if(&pButton == m_pRbOriginal)
        m_pVBoxStretch->Disable();
    if(&pButton == m_pRbStretch)
        m_pVBoxStretch->Enable();

    ModifyBitmapHdl_Impl();
}


IMPL_LINK_NOARG_TYPED(SvxBitmapTabPage, ModifyStretchHdl_Impl, Edit&, void)
{
    ModifyBitmapHdl_Impl();
}


IMPL_LINK_NOARG_TYPED(SvxBitmapTabPage, TileClickHdl_Impl, Button*, void)
{
    ModifyBitmapHdl_Impl();
}


IMPL_LINK_NOARG_TYPED(SvxBitmapTabPage, ModifyPositionHdl_Impl, ListBox&, void)
{
    ModifyBitmapHdl_Impl();
}

IMPL_LINK_NOARG_TYPED(SvxBitmapTabPage, ClickImportHdl_Impl, Button*, void)
{
    ResMgr& rMgr = CUI_MGR();
    SvxOpenGraphicDialog aDlg( OUString("Import") );
    aDlg.EnableLink(false);

    if( !aDlg.Execute() )
    {
        Graphic         aGraphic;

        EnterWait();
        int nError = aDlg.GetGraphic( aGraphic );
        LeaveWait();

        if( !nError )
        {
            OUString aDesc( ResId(RID_SVXSTR_DESC_EXT_BITMAP, rMgr) );
            ScopedVclPtr<MessageDialog> pWarnBox;

            // convert file URL to UI name
            OUString        aName;
            INetURLObject   aURL( aDlg.GetPath() );
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            DBG_ASSERT(pFact, "Dialog creation failed!");
            std::unique_ptr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog( GetParentDialog(), aURL.GetName().getToken( 0, '.' ), aDesc ));
            DBG_ASSERT(pDlg, "Dialog creation failed!");
            nError = 1;

            while( pDlg->Execute() == RET_OK )
            {
                pDlg->GetName( aName );

                bool bDifferent = true;
                long nCount     = m_pBitmapList->Count();

                for( long i = 0; i < nCount && bDifferent; i++ )
                    if( aName == m_pBitmapList->GetBitmap( i )->GetName() )
                        bDifferent = false;

                if( bDifferent ) {
                    nError = 0;
                    break;
                }

                if( !pWarnBox )
                {
                    pWarnBox.disposeAndReset(VclPtr<MessageDialog>::Create( GetParentDialog()
                                                 ,"DuplicateNameDialog"
                                                 ,"cui/ui/queryduplicatedialog.ui"));
                }

                if( pWarnBox->Execute() != RET_OK )
                    break;
            }

            pDlg.reset();
            pWarnBox.disposeAndClear();

            if( !nError )
            {
                XBitmapEntry* pEntry = new XBitmapEntry( aGraphic, aName );
                m_pBitmapList->Insert( pEntry );

                m_pBitmapLB->Clear();
                m_pBitmapLB->FillPresetListBox( *m_pBitmapList );
                m_pBitmapLB->SelectItem( m_pBitmapLB->GetItemCount() );
                *m_pnBitmapListState |= ChangeType::MODIFIED;

                ChangeBitmapHdl_Impl();
            }
        }
        else
            // graphic couldn't be loaded
            ScopedVclPtrInstance<MessageDialog>::Create( GetParentDialog()
                          ,"NoLoadedFileDialog"
                          ,"cui/ui/querynoloadedfiledialog.ui")->Execute();
    }
}

void SvxBitmapTabPage::PointChanged( vcl::Window* , RECT_POINT )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
