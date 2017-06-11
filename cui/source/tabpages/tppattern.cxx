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

#include <memory>
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
#include <svx/colorbox.hxx>
#include <svx/dialmgr.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/settings.hxx>
#include <svx/dialogs.hrc>
#include <svx/strings.hrc>

#include <cuires.hrc>
#include <strings.hrc>
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

#include <o3tl/make_unique.hxx>

using namespace com::sun::star;

/*************************************************************************
|* Preview control for the display of bitmaps
\************************************************************************/

class SvxBitmapCtl
{
private:
    Color           aPixelColor, aBackgroundColor;
    const sal_uInt16*   pBmpArray;

public:
    // Constructor: BitmapCtl for SvxPixelCtl
    SvxBitmapCtl()
        : pBmpArray(nullptr)
    {
    }

    // BitmapCtl: Returns the Bitmap
    BitmapEx GetBitmapEx()
    {
        const Bitmap aRetval(createHistorical8x8FromArray(pBmpArray, aPixelColor, aBackgroundColor));

        return (pBmpArray != nullptr) ? BitmapEx(aRetval) : BitmapEx();
    }

    void SetBmpArray( const sal_uInt16* pPixel ) { pBmpArray = pPixel; }
    void SetPixelColor( Color aColor ) { aPixelColor = aColor; }
    void SetBackgroundColor( Color aColor ) { aBackgroundColor = aColor; }
};

SvxPatternTabPage::SvxPatternTabPage(  vcl::Window* pParent, const SfxItemSet& rInAttrs) :

    SvxTabPage          ( pParent,
                          "PatternTabPage",
                          "cui/ui/patterntabpage.ui",
                          rInAttrs ),
    m_rOutAttrs           ( rInAttrs ),

    m_pnPatternListState  ( nullptr ),
    m_pnColorListState    ( nullptr ),
    m_aXFStyleItem        ( drawing::FillStyle_BITMAP ),
    m_aXPatternItem       ( OUString(), Graphic() ),
    m_aXFillAttr          ( rInAttrs.GetPool() ),
    m_rXFSet              ( m_aXFillAttr.GetItemSet() )
{
    get(m_pCtlPixel,"CTL_PIXEL");
    get(m_pLbColor,"LB_COLOR");
    get(m_pLbBackgroundColor,"LB_BACKGROUND_COLOR");
    get(m_pCtlPreview,"CTL_PREVIEW");
    get(m_pPatternLB, "patternpresetlist");
    get(m_pBtnAdd,"BTN_ADD");
    get(m_pBtnModify,"BTN_MODIFY");

    // size of the bitmap display
    Size aSize = getDrawPreviewOptimalSize(this);
    m_pPatternLB->set_width_request(aSize.Width());
    m_pPatternLB->set_height_request(aSize.Height());
    m_pCtlPreview->set_width_request(aSize.Width());
    m_pCtlPreview->set_height_request(aSize.Height());

    m_pBitmapCtl = new SvxBitmapCtl;

    // this page needs ExchangeSupport
    SetExchangeSupport();

    // setting the output device
    m_rXFSet.Put( m_aXFStyleItem );
    m_rXFSet.Put( m_aXPatternItem );

    m_pBtnAdd->SetClickHdl( LINK( this, SvxPatternTabPage, ClickAddHdl_Impl ) );
    m_pBtnModify->SetClickHdl( LINK( this, SvxPatternTabPage, ClickModifyHdl_Impl ) );

    m_pPatternLB->SetSelectHdl( LINK( this, SvxPatternTabPage, ChangePatternHdl_Impl ) );
    m_pPatternLB->SetRenameHdl( LINK( this, SvxPatternTabPage, ClickRenameHdl_Impl ) );
    m_pPatternLB->SetDeleteHdl( LINK( this, SvxPatternTabPage, ClickDeleteHdl_Impl ) );
    m_pLbColor->SetSelectHdl( LINK( this, SvxPatternTabPage, ChangeColorHdl_Impl ) );
    m_pLbBackgroundColor->SetSelectHdl( LINK( this, SvxPatternTabPage, ChangeColorHdl_Impl ) );

    setPreviewsToSamePlace(pParent, this);
}

SvxPatternTabPage::~SvxPatternTabPage()
{
    disposeOnce();
}

void SvxPatternTabPage::dispose()
{
    delete m_pBitmapCtl;
    m_pBitmapCtl = nullptr;
    m_pCtlPixel.clear();
    m_pLbColor.clear();
    m_pLbBackgroundColor.clear();
    m_pCtlPreview.clear();
    m_pBtnAdd.clear();
    m_pBtnModify.clear();
    m_pPatternLB.clear();
    SvxTabPage::dispose();
}

void SvxPatternTabPage::Construct()
{
    m_pPatternLB->FillPresetListBox( *m_pPatternList );
}

void SvxPatternTabPage::ActivatePage( const SfxItemSet& rSet )
{
    if( m_pColorList.is() )
    {
        // ColorList
        if( *m_pnColorListState & ChangeType::CHANGED ||
            *m_pnColorListState & ChangeType::MODIFIED )
        {
            SvxAreaTabDialog* pArea = (*m_pnColorListState & ChangeType::CHANGED) ?
                dynamic_cast<SvxAreaTabDialog*>(GetParentDialog()) : nullptr;
            if (pArea)
                m_pColorList = pArea->GetNewColorList();
        }

        // determining (possibly cutting) the name and
        // displaying it in the GroupBox
        OUString        aString( CuiResId( RID_SVXSTR_TABLE ) );
        aString         += ": ";
        INetURLObject   aURL( m_pPatternList->GetPath() );

        aURL.Append( m_pPatternList->GetName() );
        SAL_WARN_IF( aURL.GetProtocol() == INetProtocol::NotValid, "cui.tabpages", "invalid URL" );

        if( aURL.getBase().getLength() > 18 )
        {
            aString += aURL.getBase().copy( 0, 15 ) + "...";
        }
        else
            aString += aURL.getBase();

        sal_Int32 nPos = SearchPatternList( ( &static_cast<const XFillBitmapItem&>( rSet.Get(XATTR_FILLBITMAP)) )->GetName() );
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            sal_uInt16 nId = m_pPatternLB->GetItemId( static_cast<size_t>( nPos ) );
            m_pPatternLB->SelectItem( nId );
        }
    }
}


DeactivateRC SvxPatternTabPage::DeactivatePage( SfxItemSet* _pSet)
{
    if( _pSet )
        FillItemSet( _pSet );

    return DeactivateRC::LeavePage;
}


bool SvxPatternTabPage::FillItemSet( SfxItemSet* _rOutAttrs )
{
    _rOutAttrs->Put(XFillStyleItem(drawing::FillStyle_BITMAP));
    size_t nPos = m_pPatternLB->IsNoSelection() ? VALUESET_ITEM_NOTFOUND : m_pPatternLB->GetSelectItemPos();
    if(VALUESET_ITEM_NOTFOUND != nPos)
    {
        const XBitmapEntry* pXBitmapEntry = m_pPatternList->GetBitmap( static_cast<sal_uInt16>(nPos) );
        const OUString aString( m_pPatternLB->GetItemText( m_pPatternLB->GetSelectItemId() ) );

        _rOutAttrs->Put(XFillBitmapItem(aString, pXBitmapEntry->GetGraphicObject()));
    }
    else
    {
        const BitmapEx aBitmapEx(m_pBitmapCtl->GetBitmapEx());

        _rOutAttrs->Put(XFillBitmapItem(OUString(), Graphic(aBitmapEx)));
    }
    return true;
}


void SvxPatternTabPage::Reset( const SfxItemSet*  )
{
    m_pBitmapCtl->SetPixelColor( m_pLbColor->GetSelectEntryColor() );
    m_pBitmapCtl->SetBackgroundColor( m_pLbBackgroundColor->GetSelectEntryColor() );
    m_pBitmapCtl->SetBmpArray( m_pCtlPixel->GetBitmapPixelPtr() );

    // get bitmap and display it
    const XFillBitmapItem aBmpItem(OUString(), Graphic(m_pBitmapCtl->GetBitmapEx()));
    m_rXFSet.Put( aBmpItem );
    m_pCtlPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
    m_pCtlPreview->Invalidate();

    ChangePatternHdl_Impl( m_pPatternLB );

    // determine button state
    if( m_pPatternList.is() && m_pPatternList->Count() )
    {
        m_pBtnAdd->Enable();
        m_pBtnModify->Enable();
    }
    else
    {
        m_pBtnModify->Disable();
    }
}


VclPtr<SfxTabPage> SvxPatternTabPage::Create( vcl::Window* pWindow,
                                             const SfxItemSet* rSet )
{
    return VclPtr<SvxPatternTabPage>::Create( pWindow, *rSet );
}


IMPL_LINK_NOARG(SvxPatternTabPage, ChangePatternHdl_Impl, ValueSet*, void)
{
    std::unique_ptr<GraphicObject> pGraphicObject;
    size_t nPos = m_pPatternLB->GetSelectItemPos();

    if(VALUESET_ITEM_NOTFOUND != nPos)
    {
        pGraphicObject.reset(new GraphicObject(m_pPatternList->GetBitmap( static_cast<sal_uInt16>(nPos) )->GetGraphicObject()));
    }
    else
    {
        const SfxPoolItem* pPoolItem = nullptr;

        if(SfxItemState::SET == m_rOutAttrs.GetItemState(GetWhich(XATTR_FILLSTYLE), true, &pPoolItem))
        {
            const drawing::FillStyle eXFS((drawing::FillStyle)static_cast<const XFillStyleItem*>(pPoolItem)->GetValue());

            if((drawing::FillStyle_BITMAP == eXFS) && (SfxItemState::SET == m_rOutAttrs.GetItemState(GetWhich(XATTR_FILLBITMAP), true, &pPoolItem)))
            {
                pGraphicObject.reset(new GraphicObject(static_cast<const XFillBitmapItem*>(pPoolItem)->GetGraphicObject()));
            }
        }

        if(!pGraphicObject)
        {
            sal_uInt16 nPosition = m_pPatternLB->GetItemId( 0 );
            m_pPatternLB->SelectItem( nPosition );
            if( nPosition != 0 )
            {
                pGraphicObject.reset(new GraphicObject(m_pPatternList->GetBitmap(0)->GetGraphicObject()));
            }
        }
    }

    if(pGraphicObject)
    {
        BitmapColor aBack;
        BitmapColor aFront;
        bool bIs8x8(isHistorical8x8(pGraphicObject->GetGraphic().GetBitmap(), aBack, aFront));

        m_pLbColor->SetNoSelection();
        m_pLbBackgroundColor->SetNoSelection();

        if(bIs8x8)
        {
            m_pCtlPixel->SetPaintable( true );
            m_pBtnModify->Enable();
            m_pBtnAdd->Enable();

            // setting the pixel control

            m_pCtlPixel->SetXBitmap(pGraphicObject->GetGraphic().GetBitmapEx());

            Color aPixelColor = aFront;
            Color aBackColor = aBack;

            m_pLbColor->SelectEntry( aPixelColor );
            m_pLbBackgroundColor->SelectEntry( aBackColor );

            // update m_pBitmapCtl, rXFSet and m_pCtlPreview
            m_pBitmapCtl->SetPixelColor( aPixelColor );
            m_pBitmapCtl->SetBackgroundColor( aBackColor );
            m_rXFSet.Put(XFillBitmapItem(OUString(), Graphic(m_pBitmapCtl->GetBitmapEx())));
            m_pCtlPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
            m_pCtlPreview->Invalidate();
        }
        else
        {
            m_pCtlPixel->Reset();
            m_pCtlPixel->SetPaintable( false );
            m_pBtnModify->Disable();
            m_pBtnAdd->Disable();
        }

        m_pCtlPixel->Invalidate();
    }
}


IMPL_LINK_NOARG(SvxPatternTabPage, ClickAddHdl_Impl, Button*, void)
{

    OUString aNewName( SvxResId( RID_SVXSTR_PATTERN_UNTITLED ) );
    OUString aDesc( CuiResId( RID_SVXSTR_DESC_NEW_PATTERN ) );
    OUString aName;

    long nCount = m_pPatternList->Count();
    long j = 1;
    bool bValidPatternName = false;

    while( !bValidPatternName )
    {
        aName = aNewName + " " + OUString::number( j++ );
        bValidPatternName = (SearchPatternList(aName) == LISTBOX_ENTRY_NOTFOUND);
    }

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    assert(pFact && "Dialog creation failed!");
    ScopedVclPtr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc ));
    assert(pDlg && "Dialog creation failed!");
    ScopedVclPtr<MessageDialog> pWarnBox;
    sal_uInt16         nError(1);

    while( pDlg->Execute() == RET_OK )
    {
        pDlg->GetName( aName );

        bValidPatternName = (SearchPatternList(aName) == LISTBOX_ENTRY_NOTFOUND);

        if( bValidPatternName ) {
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

    pDlg.disposeAndClear();
    pWarnBox.disposeAndClear();

    if( !nError )
    {
        std::unique_ptr<XBitmapEntry> pEntry;
        if( m_pCtlPixel->IsEnabled() )
        {
            const BitmapEx aBitmapEx(m_pBitmapCtl->GetBitmapEx());

            pEntry.reset(new XBitmapEntry(Graphic(aBitmapEx), aName));
        }
        else // it must be a not existing imported bitmap
        {
            const SfxPoolItem* pPoolItem = nullptr;

            if(SfxItemState::SET == m_rOutAttrs.GetItemState(XATTR_FILLBITMAP, true, &pPoolItem))
            {
                pEntry.reset(new XBitmapEntry(dynamic_cast<const XFillBitmapItem*>(pPoolItem)->GetGraphicObject(), aName));
            }
            else
                assert("SvxPatternTabPage::ClickAddHdl_Impl(), XBitmapEntry* pEntry == nullptr ?");
        }

        if( pEntry )
        {
            m_pPatternList->Insert(std::move(pEntry), nCount);
            sal_Int32 nId = m_pPatternLB->GetItemId( nCount - 1 );
            Bitmap aBitmap = m_pPatternList->GetBitmapForPreview( nCount, m_pPatternLB->GetIconSize() );
            m_pPatternLB->InsertItem( nId + 1, Image(aBitmap), aName );
            m_pPatternLB->SelectItem( nId + 1 );
            m_pPatternLB->Resize();

            *m_pnPatternListState |= ChangeType::MODIFIED;

            ChangePatternHdl_Impl( m_pPatternLB  );
        }
    }

    // determine button state
    if( m_pPatternList->Count() )
    {
        m_pBtnModify->Enable();
    }
}


/******************************************************************************/
/******************************************************************************/


IMPL_LINK_NOARG(SvxPatternTabPage, ClickModifyHdl_Impl, Button*, void)
{
    sal_uInt16 nId = m_pPatternLB->GetSelectItemId();
    size_t nPos = m_pPatternLB->GetSelectItemPos();

    if ( nPos != VALUESET_ITEM_NOTFOUND )
    {
        OUString aName( m_pPatternList->GetBitmap( static_cast<sal_uInt16>(nPos) )->GetName() );

        const BitmapEx aBitmapEx(m_pBitmapCtl->GetBitmapEx());

        // #i123497# Need to replace the existing entry with a new one (old returned needs to be deleted)
        m_pPatternList->Replace(o3tl::make_unique<XBitmapEntry>(Graphic(aBitmapEx), aName), nPos);

        Bitmap aBitmap = m_pPatternList->GetBitmapForPreview( static_cast<sal_uInt16>( nPos ), m_pPatternLB->GetIconSize() );
        m_pPatternLB->RemoveItem(nId);
        m_pPatternLB->InsertItem( nId, Image(aBitmap), aName, static_cast<sal_uInt16>(nPos) );
        m_pPatternLB->SelectItem( nId );

        *m_pnPatternListState |= ChangeType::MODIFIED;
    }
}


IMPL_LINK_NOARG(SvxPatternTabPage, ClickRenameHdl_Impl, SvxPresetListBox*, void)
{
    size_t nPos = m_pPatternLB->GetSelectItemPos();
    sal_Int32 nId = m_pPatternLB->GetSelectItemId();

    if ( nPos != VALUESET_ITEM_NOTFOUND )
    {
        OUString aDesc(CuiResId(RID_SVXSTR_DESC_NEW_PATTERN));
        OUString aName(m_pPatternList->GetBitmap(nPos)->GetName());

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        assert(pFact && "Dialog creation failed!");
        ScopedVclPtr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc ));
        assert(pDlg && "Dialog creation failed!");

        bool bLoop = true;

        while( bLoop && pDlg->Execute() == RET_OK )
        {
            pDlg->GetName( aName );
            sal_Int32 nPatternPos = SearchPatternList(aName);
            bool bValidPatternName = (nPatternPos == static_cast<sal_Int32>(nPos) ) || (nPatternPos == LISTBOX_ENTRY_NOTFOUND);

            if( bValidPatternName )
            {
                bLoop = false;

                m_pPatternList->GetBitmap(nPos)->SetName(aName);

                m_pPatternLB->SetItemText( nId, aName );
                m_pPatternLB->SelectItem( nId );

                *m_pnPatternListState |= ChangeType::MODIFIED;
            }
            else
            {
                ScopedVclPtrInstance<MessageDialog> aBox(
                                   GetParentDialog()
                                   ,"DuplicateNameDialog"
                                   ,"cui/ui/queryduplicatedialog.ui");
                aBox->Execute();
            }
        }
    }
}


IMPL_LINK_NOARG(SvxPatternTabPage, ClickDeleteHdl_Impl, SvxPresetListBox*, void)
{
    sal_uInt16 nId = m_pPatternLB->GetSelectItemId();
    size_t nPos = m_pPatternLB->GetSelectItemPos();

    if( nPos != VALUESET_ITEM_NOTFOUND )
    {
        ScopedVclPtrInstance< MessageDialog > aQueryBox( GetParentDialog(),"AskDelBitmapDialog","cui/ui/querydeletebitmapdialog.ui" );

        if( aQueryBox->Execute() == RET_YES )
        {
            m_pPatternList->Remove(nPos);
            m_pPatternLB->RemoveItem( nId );
            nId = m_pPatternLB->GetItemId(0);
            m_pPatternLB->SelectItem( nId );
            m_pPatternLB->Resize();

            m_pCtlPreview->Invalidate();
            m_pCtlPixel->Invalidate();

            ChangePatternHdl_Impl( m_pPatternLB );

            *m_pnPatternListState |= ChangeType::MODIFIED;
        }
    }
    // determine button state
    if( !m_pPatternList->Count() )
    {
        m_pBtnModify->Disable();
    }
}

IMPL_LINK_NOARG(SvxPatternTabPage, ChangeColorHdl_Impl, SvxColorListBox&, void)
{
    ChangeColor_Impl();
    m_pPatternLB->SetNoSelection();
}

void SvxPatternTabPage::ChangeColor_Impl()
{
    m_pCtlPixel->SetPixelColor( m_pLbColor->GetSelectEntryColor() );
    m_pCtlPixel->SetBackgroundColor( m_pLbBackgroundColor->GetSelectEntryColor() );
    m_pCtlPixel->Invalidate();

    m_pBitmapCtl->SetPixelColor( m_pLbColor->GetSelectEntryColor() );
    m_pBitmapCtl->SetBackgroundColor( m_pLbBackgroundColor->GetSelectEntryColor() );

    // get bitmap and display it
    m_rXFSet.Put(XFillBitmapItem(OUString(), Graphic(m_pBitmapCtl->GetBitmapEx())));
    m_pCtlPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
    m_pCtlPreview->Invalidate();
}


void SvxPatternTabPage::PointChanged( vcl::Window* pWindow, RectPoint )
{
    if( pWindow == m_pCtlPixel )
    {
        m_pBitmapCtl->SetBmpArray( m_pCtlPixel->GetBitmapPixelPtr() );

        // get bitmap and display it
        m_rXFSet.Put(XFillBitmapItem(OUString(), Graphic(m_pBitmapCtl->GetBitmapEx())));
        m_pCtlPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
        m_pCtlPreview->Invalidate();
    }

    m_pPatternLB->SetNoSelection();
}

sal_Int32 SvxPatternTabPage::SearchPatternList(const OUString& rPatternName)
{
    long nCount = m_pPatternList->Count();
    bool bValidPatternName = true;
    sal_Int32 nPos = LISTBOX_ENTRY_NOTFOUND;

    for(long i = 0;i < nCount && bValidPatternName;i++)
    {
        if(rPatternName == m_pPatternList->GetBitmap( i )->GetName())
        {
            nPos = i;
            bValidPatternName = false;
        }
    }
    return nPos;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
