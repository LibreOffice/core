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
#include <tools/urlobj.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dialoghelper.hxx>
#include <sfx2/filedlghelper.hxx>
#include <unotools/localfilehelper.hxx>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <svx/colorbox.hxx>
#include <svx/dialmgr.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/BitmapTools.hxx>
#include <vcl/settings.hxx>
#include <vcl/weld.hxx>
#include <svx/dialogs.hrc>
#include <svx/strings.hrc>

#include <strings.hrc>
#include <svx/xattr.hxx>
#include <svx/xpool.hxx>
#include <svx/xtable.hxx>
#include <svx/xoutbmp.hxx>
#include <svx/drawitem.hxx>
#include <cuitabarea.hxx>
#include <defdlgname.hxx>
#include <dlgname.hxx>
#include <svx/svxdlg.hxx>
#include <dialmgr.hxx>
#include <sfx2/opengrf.hxx>
#include <sal/log.hxx>

using namespace com::sun::star;

/*************************************************************************
|* Preview control for the display of bitmaps
\************************************************************************/

class SvxBitmapCtl
{
private:
    Color           aPixelColor, aBackgroundColor;
    std::array<sal_uInt8,64> const * pBmpArray;

public:
    // Constructor: BitmapCtl for SvxPixelCtl
    SvxBitmapCtl()
        : pBmpArray(nullptr)
    {
    }

    // BitmapCtl: Returns the Bitmap
    BitmapEx GetBitmapEx()
    {
        if (!pBmpArray)
            return BitmapEx();
        return vcl::bitmap::createHistorical8x8FromArray(*pBmpArray, aPixelColor, aBackgroundColor);
    }

    void SetBmpArray( std::array<sal_uInt8,64> const & pPixel ) { pBmpArray = &pPixel; }
    void SetPixelColor( Color aColor ) { aPixelColor = aColor; }
    void SetBackgroundColor( Color aColor ) { aBackgroundColor = aColor; }
};

SvxPatternTabPage::SvxPatternTabPage(TabPageParent pParent, const SfxItemSet& rInAttrs)
    : SvxTabPage(pParent, "cui/ui/patterntabpage.ui", "PatternTabPage", rInAttrs)
    , m_rOutAttrs(rInAttrs)
    , m_pnPatternListState(nullptr)
    , m_pnColorListState(nullptr)
    , m_aXFillAttr(rInAttrs.GetPool())
    , m_rXFSet(m_aXFillAttr.GetItemSet())
    , m_xCtlPixel(new SvxPixelCtl(this))
    , m_xLbColor(new ColorListBox(m_xBuilder->weld_menu_button("LB_COLOR"), pParent.GetFrameWeld()))
    , m_xLbBackgroundColor(new ColorListBox(m_xBuilder->weld_menu_button("LB_BACKGROUND_COLOR"), pParent.GetFrameWeld()))
    , m_xPatternLB(new SvxPresetListBox(m_xBuilder->weld_scrolled_window("patternpresetlistwin")))
    , m_xBtnAdd(m_xBuilder->weld_button("BTN_ADD"))
    , m_xBtnModify(m_xBuilder->weld_button("BTN_MODIFY"))
    , m_xCtlPixelWin(new weld::CustomWeld(*m_xBuilder, "CTL_PIXEL", *m_xCtlPixel))
    , m_xCtlPreview(new weld::CustomWeld(*m_xBuilder, "CTL_PREVIEW", m_aCtlPreview))
    , m_xPatternLBWin(new weld::CustomWeld(*m_xBuilder, "patternpresetlist", *m_xPatternLB))
{
    // size of the bitmap display
    Size aSize = getDrawPreviewOptimalSize(this);
    m_xPatternLB->set_size_request(aSize.Width(), aSize.Height());
    m_xCtlPreview->set_size_request(aSize.Width(), aSize.Height());

    m_xBitmapCtl.reset(new SvxBitmapCtl);

    // this page needs ExchangeSupport
    SetExchangeSupport();

    // setting the output device
    m_rXFSet.Put( XFillStyleItem(drawing::FillStyle_BITMAP) );
    m_rXFSet.Put( XFillBitmapItem(OUString(), Graphic()) );

    m_xBtnAdd->connect_clicked( LINK( this, SvxPatternTabPage, ClickAddHdl_Impl ) );
    m_xBtnModify->connect_clicked( LINK( this, SvxPatternTabPage, ClickModifyHdl_Impl ) );

    m_xPatternLB->SetSelectHdl( LINK( this, SvxPatternTabPage, ChangePatternHdl_Impl ) );
    m_xPatternLB->SetRenameHdl( LINK( this, SvxPatternTabPage, ClickRenameHdl_Impl ) );
    m_xPatternLB->SetDeleteHdl( LINK( this, SvxPatternTabPage, ClickDeleteHdl_Impl ) );
    m_xLbColor->SetSelectHdl( LINK( this, SvxPatternTabPage, ChangeColorHdl_Impl ) );
    m_xLbBackgroundColor->SetSelectHdl( LINK( this, SvxPatternTabPage, ChangeColorHdl_Impl ) );
}

SvxPatternTabPage::~SvxPatternTabPage()
{
    disposeOnce();
}

void SvxPatternTabPage::dispose()
{
    m_xPatternLBWin.reset();
    m_xCtlPreview.reset();
    m_xCtlPixelWin.reset();
    m_xPatternLB.reset();
    m_xLbBackgroundColor.reset();
    m_xLbColor.reset();
    m_xCtlPixel.reset();
    SvxTabPage::dispose();
}

void SvxPatternTabPage::Construct()
{
    m_xPatternLB->FillPresetListBox( *m_pPatternList );
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

        XFillBitmapItem aItem( rSet.Get( XATTR_FILLBITMAP ) );

        if ( aItem.isPattern() )
        {
            sal_Int32 nPos = SearchPatternList( aItem.GetName() );
            if ( nPos != LISTBOX_ENTRY_NOTFOUND )
            {
                sal_uInt16 nId = m_xPatternLB->GetItemId( static_cast<size_t>( nPos ) );
                m_xPatternLB->SelectItem( nId );
            }
        }
        else
            m_xPatternLB->SelectItem( m_xPatternLB->GetItemId( static_cast<size_t>( 0 ) ) );
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
    size_t nPos = m_xPatternLB->IsNoSelection() ? VALUESET_ITEM_NOTFOUND : m_xPatternLB->GetSelectItemPos();
    if(VALUESET_ITEM_NOTFOUND != nPos)
    {
        const XBitmapEntry* pXBitmapEntry = m_pPatternList->GetBitmap( static_cast<sal_uInt16>(nPos) );
        const OUString aString( m_xPatternLB->GetItemText( m_xPatternLB->GetSelectedItemId() ) );

        _rOutAttrs->Put(XFillBitmapItem(aString, pXBitmapEntry->GetGraphicObject()));
    }
    else
    {
        const BitmapEx aBitmapEx(m_xBitmapCtl->GetBitmapEx());

        _rOutAttrs->Put(XFillBitmapItem(OUString(), Graphic(aBitmapEx)));
    }
    _rOutAttrs->Put(XFillBmpTileItem(true));
    return true;
}


void SvxPatternTabPage::Reset( const SfxItemSet*  )
{
    m_xBitmapCtl->SetPixelColor( m_xLbColor->GetSelectEntryColor() );
    m_xBitmapCtl->SetBackgroundColor( m_xLbBackgroundColor->GetSelectEntryColor() );
    m_xBitmapCtl->SetBmpArray( m_xCtlPixel->GetBitmapPixelPtr() );

    // get bitmap and display it
    const XFillBitmapItem aBmpItem(OUString(), Graphic(m_xBitmapCtl->GetBitmapEx()));
    if(aBmpItem.isPattern())
    {
        m_rXFSet.Put( aBmpItem );
        m_aCtlPreview.SetAttributes( m_aXFillAttr.GetItemSet() );
        m_aCtlPreview.Invalidate();
    }

    ChangePatternHdl_Impl(m_xPatternLB.get());

    // determine button state
    if( m_pPatternList.is() && m_pPatternList->Count() )
    {
        m_xBtnAdd->set_sensitive(true);
        m_xBtnModify->set_sensitive(true);
    }
    else
    {
        m_xBtnModify->set_sensitive(false);
    }
}


VclPtr<SfxTabPage> SvxPatternTabPage::Create( TabPageParent pWindow,
                                             const SfxItemSet* rSet )
{
    return VclPtr<SvxPatternTabPage>::Create(pWindow, *rSet);
}


IMPL_LINK_NOARG(SvxPatternTabPage, ChangePatternHdl_Impl, SvtValueSet*, void)
{
    std::unique_ptr<GraphicObject> pGraphicObject;
    size_t nPos = m_xPatternLB->GetSelectItemPos();

    if(VALUESET_ITEM_NOTFOUND != nPos)
    {
        pGraphicObject.reset(new GraphicObject(m_pPatternList->GetBitmap( static_cast<sal_uInt16>(nPos) )->GetGraphicObject()));
    }
    else
    {
        const SfxPoolItem* pPoolItem = nullptr;

        if(SfxItemState::SET == m_rOutAttrs.GetItemState(GetWhich(XATTR_FILLSTYLE), true, &pPoolItem))
        {
            const drawing::FillStyle eXFS(static_cast<const XFillStyleItem*>(pPoolItem)->GetValue());

            if((drawing::FillStyle_BITMAP == eXFS) && (SfxItemState::SET == m_rOutAttrs.GetItemState(GetWhich(XATTR_FILLBITMAP), true, &pPoolItem)))
            {
                pGraphicObject.reset(new GraphicObject(static_cast<const XFillBitmapItem*>(pPoolItem)->GetGraphicObject()));
            }
        }

        if(!pGraphicObject)
        {
            sal_uInt16 nPosition = m_xPatternLB->GetItemId( 0 );
            m_xPatternLB->SelectItem( nPosition );
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
        bool bIs8x8(vcl::bitmap::isHistorical8x8(pGraphicObject->GetGraphic().GetBitmapEx(), aBack, aFront));

        m_xLbColor->SetNoSelection();
        m_xLbBackgroundColor->SetNoSelection();

        if(bIs8x8)
        {
            m_xCtlPixel->SetPaintable( true );
            m_xBtnModify->set_sensitive(true);
            m_xBtnAdd->set_sensitive(true);

            // setting the pixel control

            m_xCtlPixel->SetXBitmap(pGraphicObject->GetGraphic().GetBitmapEx());

            Color aPixelColor = aFront.GetColor();
            Color aBackColor = aBack.GetColor();

            m_xLbColor->SelectEntry( aPixelColor );
            m_xLbBackgroundColor->SelectEntry( aBackColor );

            // update m_xBitmapCtl, rXFSet and m_aCtlPreview
            m_xBitmapCtl->SetPixelColor( aPixelColor );
            m_xBitmapCtl->SetBackgroundColor( aBackColor );
            m_rXFSet.ClearItem();
            m_rXFSet.Put(XFillStyleItem(drawing::FillStyle_BITMAP));
            m_rXFSet.Put(XFillBitmapItem(OUString(), Graphic(m_xBitmapCtl->GetBitmapEx())));
            m_aCtlPreview.SetAttributes( m_aXFillAttr.GetItemSet() );
            m_aCtlPreview.Invalidate();
        }
        else
        {
            m_xCtlPixel->Reset();
            m_xCtlPixel->SetPaintable( false );
            m_xBtnModify->set_sensitive(false);
            m_xBtnAdd->set_sensitive(false);
        }

        m_xCtlPixel->Invalidate();
    }
}

IMPL_LINK_NOARG(SvxPatternTabPage, ClickAddHdl_Impl, weld::Button&, void)
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
    ScopedVclPtr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog(GetDialogFrameWeld(), aName, aDesc));
    sal_uInt16         nError(1);

    while( pDlg->Execute() == RET_OK )
    {
        pDlg->GetName( aName );

        bValidPatternName = (SearchPatternList(aName) == LISTBOX_ENTRY_NOTFOUND);

        if( bValidPatternName ) {
            nError = 0;
            break;
        }

        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetDialogFrameWeld(), "cui/ui/queryduplicatedialog.ui"));
        std::unique_ptr<weld::MessageDialog> xWarnBox(xBuilder->weld_message_dialog("DuplicateNameDialog"));
        if (xWarnBox->run() != RET_OK)
            break;
    }

    pDlg.disposeAndClear();

    if( !nError )
    {
        std::unique_ptr<XBitmapEntry> pEntry;
        if( m_xCtlPixel->IsEnabled() )
        {
            const BitmapEx aBitmapEx(m_xBitmapCtl->GetBitmapEx());

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
                assert(!"SvxPatternTabPage::ClickAddHdl_Impl(), XBitmapEntry* pEntry == nullptr ?");
        }

        if( pEntry )
        {
            m_pPatternList->Insert(std::move(pEntry), nCount);
            sal_Int32 nId = m_xPatternLB->GetItemId( nCount - 1 );
            BitmapEx aBitmap = m_pPatternList->GetBitmapForPreview( nCount, m_xPatternLB->GetIconSize() );
            m_xPatternLB->InsertItem( nId + 1, Image(aBitmap), aName );
            m_xPatternLB->SelectItem( nId + 1 );
            m_xPatternLB->Resize();

            *m_pnPatternListState |= ChangeType::MODIFIED;

            ChangePatternHdl_Impl(m_xPatternLB.get());
        }
    }

    // determine button state
    if( m_pPatternList->Count() )
    {
        m_xBtnModify->set_sensitive(true);
    }
}

IMPL_LINK_NOARG(SvxPatternTabPage, ClickModifyHdl_Impl, weld::Button&, void)
{
    sal_uInt16 nId = m_xPatternLB->GetSelectedItemId();
    size_t nPos = m_xPatternLB->GetSelectItemPos();

    if ( nPos != VALUESET_ITEM_NOTFOUND )
    {
        OUString aName( m_pPatternList->GetBitmap( static_cast<sal_uInt16>(nPos) )->GetName() );

        const BitmapEx aBitmapEx(m_xBitmapCtl->GetBitmapEx());

        // #i123497# Need to replace the existing entry with a new one (old returned needs to be deleted)
        m_pPatternList->Replace(std::make_unique<XBitmapEntry>(Graphic(aBitmapEx), aName), nPos);

        BitmapEx aBitmap = m_pPatternList->GetBitmapForPreview( static_cast<sal_uInt16>( nPos ), m_xPatternLB->GetIconSize() );
        m_xPatternLB->RemoveItem(nId);
        m_xPatternLB->InsertItem( nId, Image(aBitmap), aName, static_cast<sal_uInt16>(nPos) );
        m_xPatternLB->SelectItem( nId );

        *m_pnPatternListState |= ChangeType::MODIFIED;
    }
}


IMPL_LINK_NOARG(SvxPatternTabPage, ClickRenameHdl_Impl, SvxPresetListBox*, void)
{
    size_t nPos = m_xPatternLB->GetSelectItemPos();
    sal_Int32 nId = m_xPatternLB->GetSelectedItemId();

    if ( nPos != VALUESET_ITEM_NOTFOUND )
    {
        OUString aDesc(CuiResId(RID_SVXSTR_DESC_NEW_PATTERN));
        OUString aName(m_pPatternList->GetBitmap(nPos)->GetName());

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        ScopedVclPtr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog(GetDialogFrameWeld(), aName, aDesc));

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

                m_xPatternLB->SetItemText( nId, aName );
                m_xPatternLB->SelectItem( nId );

                *m_pnPatternListState |= ChangeType::MODIFIED;
            }
            else
            {
                std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetDialogFrameWeld(), "cui/ui/queryduplicatedialog.ui"));
                std::unique_ptr<weld::MessageDialog> xWarnBox(xBuilder->weld_message_dialog("DuplicateNameDialog"));
                xWarnBox->run();
            }
        }
    }
}

IMPL_LINK_NOARG(SvxPatternTabPage, ClickDeleteHdl_Impl, SvxPresetListBox*, void)
{
    sal_uInt16 nId = m_xPatternLB->GetSelectedItemId();
    size_t nPos = m_xPatternLB->GetSelectItemPos();

    if( nPos != VALUESET_ITEM_NOTFOUND )
    {
        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetDialogFrameWeld(), "cui/ui/querydeletebitmapdialog.ui"));
        std::unique_ptr<weld::MessageDialog> xQueryBox(xBuilder->weld_message_dialog("AskDelBitmapDialog"));
        if (xQueryBox->run() == RET_YES)
        {
            m_pPatternList->Remove(nPos);
            m_xPatternLB->RemoveItem( nId );
            nId = m_xPatternLB->GetItemId(0);
            m_xPatternLB->SelectItem( nId );
            m_xPatternLB->Resize();

            m_aCtlPreview.Invalidate();
            m_xCtlPixel->Invalidate();

            ChangePatternHdl_Impl(m_xPatternLB.get());

            *m_pnPatternListState |= ChangeType::MODIFIED;
        }
    }
    // determine button state
    if( !m_pPatternList->Count() )
    {
        m_xBtnModify->set_sensitive(false);
    }
}

IMPL_LINK_NOARG(SvxPatternTabPage, ChangeColorHdl_Impl, ColorListBox&, void)
{
    ChangeColor_Impl();
    m_xPatternLB->SetNoSelection();
}

void SvxPatternTabPage::ChangeColor_Impl()
{
    m_xCtlPixel->SetPixelColor( m_xLbColor->GetSelectEntryColor() );
    m_xCtlPixel->SetBackgroundColor( m_xLbBackgroundColor->GetSelectEntryColor() );
    m_xCtlPixel->Invalidate();

    m_xBitmapCtl->SetPixelColor( m_xLbColor->GetSelectEntryColor() );
    m_xBitmapCtl->SetBackgroundColor( m_xLbBackgroundColor->GetSelectEntryColor() );

    // get bitmap and display it
    m_rXFSet.Put(XFillBitmapItem(OUString(), Graphic(m_xBitmapCtl->GetBitmapEx())));
    m_aCtlPreview.SetAttributes( m_aXFillAttr.GetItemSet() );
    m_aCtlPreview.Invalidate();
}

void SvxPatternTabPage::PointChanged(weld::DrawingArea* pDrawingArea, RectPoint)
{
    if (pDrawingArea == m_xCtlPixel->GetDrawingArea())
    {
        m_xBitmapCtl->SetBmpArray(m_xCtlPixel->GetBitmapPixelPtr());

        // get bitmap and display it
        m_rXFSet.Put(XFillBitmapItem(OUString(), Graphic(m_xBitmapCtl->GetBitmapEx())));
        m_aCtlPreview.SetAttributes( m_aXFillAttr.GetItemSet() );
        m_aCtlPreview.Invalidate();
    }

    m_xPatternLB->SetNoSelection();
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
