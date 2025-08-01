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
#include <tools/urlobj.hxx>
#include <sfx2/dialoghelper.hxx>
#include <sfx2/objsh.hxx>
#include <svx/colorbox.hxx>
#include <svx/dialmgr.hxx>
#include <vcl/BitmapTools.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <svx/strings.hrc>
#include <svx/svxids.hrc>

#include <strings.hrc>
#include <svx/drawitem.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xtable.hxx>
#include <svx/xflbmtit.hxx>
#include <cuitabarea.hxx>
#include <svx/svxdlg.hxx>
#include <dialmgr.hxx>
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
    Bitmap GetBitmap() const
    {
        if (!pBmpArray)
            return Bitmap();
        return vcl::bitmap::createHistorical8x8FromArray(*pBmpArray, aPixelColor, aBackgroundColor);
    }

    void SetBmpArray( std::array<sal_uInt8,64> const & pPixel ) { pBmpArray = &pPixel; }
    void SetPixelColor( Color aColor ) { aPixelColor = aColor; }
    void SetBackgroundColor( Color aColor ) { aBackgroundColor = aColor; }
};

SvxPatternTabPage::SvxPatternTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs)
    : SvxTabPage(pPage, pController, u"cui/ui/patterntabpage.ui"_ustr, u"PatternTabPage"_ustr, rInAttrs)
    , m_rOutAttrs(rInAttrs)
    , m_nPatternListState(ChangeType::NONE)
    , m_pnColorListState(nullptr)
    , m_aXFillAttr(rInAttrs.GetPool())
    , m_rXFSet(m_aXFillAttr.GetItemSet())
    , m_xCtlPixel(new SvxPixelCtl(this))
    , m_xLbColor(new ColorListBox(m_xBuilder->weld_menu_button(u"LB_COLOR"_ustr),
                [this]{ return GetDialogController()->getDialog(); }))
    , m_xLbBackgroundColor(new ColorListBox(m_xBuilder->weld_menu_button(u"LB_BACKGROUND_COLOR"_ustr),
                [this]{ return GetDialogController()->getDialog(); }))
    , m_xPatternLB(new SvxPresetListBox(m_xBuilder->weld_scrolled_window(u"patternpresetlistwin"_ustr, true)))
    , m_xBtnAdd(m_xBuilder->weld_button(u"BTN_ADD"_ustr))
    , m_xBtnModify(m_xBuilder->weld_button(u"BTN_MODIFY"_ustr))
    , m_xCtlPixelWin(new weld::CustomWeld(*m_xBuilder, u"CTL_PIXEL"_ustr, *m_xCtlPixel))
    , m_xCtlPreview(new weld::CustomWeld(*m_xBuilder, u"CTL_PREVIEW"_ustr, m_aCtlPreview))
    , m_xPatternLBWin(new weld::CustomWeld(*m_xBuilder, u"patternpresetlist"_ustr, *m_xPatternLB))
{
    // size of the bitmap display
    Size aSize = getDrawPreviewOptimalSize(m_aCtlPreview.GetDrawingArea()->get_ref_device());
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

    m_xPatternLB->SetStyle(WB_FLATVALUESET | WB_NO_DIRECTSELECT | WB_TABSTOP);
}

SvxPatternTabPage::~SvxPatternTabPage()
{
    m_xPatternLBWin.reset();
    m_xCtlPreview.reset();
    m_xCtlPixelWin.reset();
    m_xPatternLB.reset();
    m_xLbBackgroundColor.reset();
    m_xLbColor.reset();
    m_xCtlPixel.reset();

    if (m_nPatternListState & ChangeType::MODIFIED)
    {
        m_pPatternList->SetPath(AreaTabHelper::GetPalettePath());
        m_pPatternList->Save();

        // ToolBoxControls are informed:
        SfxObjectShell* pShell = SfxObjectShell::Current();
        if (pShell)
            pShell->PutItem(SvxPatternListItem(m_pPatternList, SID_PATTERN_LIST));
    }
}

void SvxPatternTabPage::Construct()
{
    m_xPatternLB->FillPresetListBox( *m_pPatternList );
}

void SvxPatternTabPage::ActivatePage( const SfxItemSet& rSet )
{
    if( !m_pColorList.is() )
        return;

    // ColorList
    if( *m_pnColorListState & ChangeType::CHANGED ||
        *m_pnColorListState & ChangeType::MODIFIED )
    {
        SvxAreaTabDialog* pArea = (*m_pnColorListState & ChangeType::CHANGED) ?
            dynamic_cast<SvxAreaTabDialog*>(GetDialogController()) : nullptr;
        if (pArea)
            m_pColorList = pArea->GetNewColorList();
    }

    // determining (possibly cutting) the name and
    // displaying it in the GroupBox
    OUString        aString = CuiResId( RID_CUISTR_TABLE ) + ": ";
    INetURLObject   aURL( m_pPatternList->GetPath() );

    aURL.Append( m_pPatternList->GetName() );
    SAL_WARN_IF( aURL.GetProtocol() == INetProtocol::NotValid, "cui.tabpages", "invalid URL" );

    if( aURL.getBase().getLength() > 18 )
    {
        aString += OUString::Concat(aURL.getBase().subView( 0, 15 )) + "...";
    }
    else
        aString += aURL.getBase();

    const XFillBitmapItem& aItem( rSet.Get( XATTR_FILLBITMAP ) );

    if ( aItem.isPattern() )
    {
        sal_Int32 nPos = SearchPatternList( aItem.GetName() );
        if ( nPos != -1)
        {
            sal_uInt16 nId = m_xPatternLB->GetItemId( static_cast<size_t>( nPos ) );
            m_xPatternLB->SelectItem( nId );
        }
    }
    else
        m_xPatternLB->SelectItem( m_xPatternLB->GetItemId( static_cast<size_t>( 0 ) ) );
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
        const Bitmap aBitmap(m_xBitmapCtl->GetBitmap());

        _rOutAttrs->Put(XFillBitmapItem(OUString(), Graphic(aBitmap)));
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
    const XFillBitmapItem aBmpItem(OUString(), Graphic(m_xBitmapCtl->GetBitmap()));
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

std::unique_ptr<SfxTabPage> SvxPatternTabPage::Create( weld::Container* pPage, weld::DialogController* pController,
                                             const SfxItemSet* rSet )
{
    return std::make_unique<SvxPatternTabPage>(pPage, pController, *rSet);
}

IMPL_LINK_NOARG(SvxPatternTabPage, ChangePatternHdl_Impl, ValueSet*, void)
{
    std::unique_ptr<GraphicObject> pGraphicObject;
    size_t nPos = m_xPatternLB->GetSelectItemPos();

    if(VALUESET_ITEM_NOTFOUND != nPos)
    {
        pGraphicObject.reset(new GraphicObject(m_pPatternList->GetBitmap( static_cast<sal_uInt16>(nPos) )->GetGraphicObject()));
    }
    else
    {
        if(const XFillStyleItem* pFillStyleItem = m_rOutAttrs.GetItemIfSet(GetWhich(XATTR_FILLSTYLE)))
        {
            const drawing::FillStyle eXFS(pFillStyleItem->GetValue());

            const XFillBitmapItem* pBitmapItem;
            if((drawing::FillStyle_BITMAP == eXFS) && (pBitmapItem = m_rOutAttrs.GetItemIfSet(GetWhich(XATTR_FILLBITMAP))))
            {
                pGraphicObject.reset(new GraphicObject(pBitmapItem->GetGraphicObject()));
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

    if(!pGraphicObject)
        return;

    Color aBackColor;
    Color aPixelColor;
    bool bIs8x8(vcl::bitmap::isHistorical8x8(Bitmap(pGraphicObject->GetGraphic().GetBitmapEx()), aBackColor, aPixelColor));

    m_xLbColor->SetNoSelection();
    m_xLbBackgroundColor->SetNoSelection();

    if(bIs8x8)
    {
        m_xCtlPixel->SetPaintable( true );
        m_xBtnModify->set_sensitive(true);
        m_xBtnAdd->set_sensitive(true);

        // setting the pixel control

        m_xCtlPixel->SetXBitmap(Bitmap(pGraphicObject->GetGraphic().GetBitmapEx()));

        m_xLbColor->SelectEntry( aPixelColor );
        m_xLbBackgroundColor->SelectEntry( aBackColor );

        // update m_xBitmapCtl, rXFSet and m_aCtlPreview
        m_xBitmapCtl->SetPixelColor( aPixelColor );
        m_xBitmapCtl->SetBackgroundColor( aBackColor );
        m_rXFSet.ClearItem();
        m_rXFSet.Put(XFillStyleItem(drawing::FillStyle_BITMAP));
        m_rXFSet.Put(XFillBitmapItem(OUString(), Graphic(m_xBitmapCtl->GetBitmap())));
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

IMPL_LINK_NOARG(SvxPatternTabPage, ClickAddHdl_Impl, weld::Button&, void)
{

    OUString aNewName( SvxResId( RID_SVXSTR_PATTERN_UNTITLED ) );
    OUString aDesc( CuiResId( RID_CUISTR_DESC_NEW_PATTERN ) );
    OUString aName;

    tools::Long nCount = m_pPatternList->Count();
    tools::Long j = 1;
    bool bValidPatternName = false;

    while( !bValidPatternName )
    {
        aName = aNewName + " " + OUString::number( j++ );
        bValidPatternName = (SearchPatternList(aName) == -1);
    }

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    ScopedVclPtr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog(GetFrameWeld(), aName, aDesc));
    sal_uInt16         nError(1);

    while( pDlg->Execute() == RET_OK )
    {
        aName = pDlg->GetName();

        bValidPatternName = (SearchPatternList(aName) == -1);

        if( bValidPatternName ) {
            nError = 0;
            break;
        }

        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetFrameWeld(), u"cui/ui/queryduplicatedialog.ui"_ustr));
        std::unique_ptr<weld::MessageDialog> xWarnBox(xBuilder->weld_message_dialog(u"DuplicateNameDialog"_ustr));
        if (xWarnBox->run() != RET_OK)
            break;
    }

    pDlg.disposeAndClear();

    if( !nError )
    {
        std::unique_ptr<XBitmapEntry> pEntry;
        if( m_xCtlPixel->IsEnabled() )
        {
            const Bitmap aBitmap(m_xBitmapCtl->GetBitmap());

            pEntry.reset(new XBitmapEntry(Graphic(aBitmap), aName));
        }
        else // it must be a not existing imported bitmap
        {
            if(const XFillBitmapItem* pFillBmpItem = m_rOutAttrs.GetItemIfSet(XATTR_FILLBITMAP))
            {
                pEntry.reset(new XBitmapEntry(pFillBmpItem->GetGraphicObject(), aName));
            }
            else
                assert(!"SvxPatternTabPage::ClickAddHdl_Impl(), XBitmapEntry* pEntry == nullptr ?");
        }

        if( pEntry )
        {
            m_pPatternList->Insert(std::move(pEntry), nCount);
            sal_Int32 nId = m_xPatternLB->GetItemId( nCount - 1 );
            Bitmap aBitmap = m_pPatternList->GetBitmapForPreview( nCount, m_xPatternLB->GetIconSize() );
            m_xPatternLB->InsertItem( nId + 1, Image(aBitmap), aName );
            m_xPatternLB->SelectItem( nId + 1 );
            m_xPatternLB->Resize();

            m_nPatternListState |= ChangeType::MODIFIED;

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

    if ( nPos == VALUESET_ITEM_NOTFOUND )
        return;

    OUString aName( m_pPatternList->GetBitmap( static_cast<sal_uInt16>(nPos) )->GetName() );

    // #i123497# Need to replace the existing entry with a new one (old returned needs to be deleted)
    m_pPatternList->Replace(std::make_unique<XBitmapEntry>(Graphic(m_xBitmapCtl->GetBitmap()), aName), nPos);

    Bitmap aBitmap = m_pPatternList->GetBitmapForPreview( static_cast<sal_uInt16>( nPos ), m_xPatternLB->GetIconSize() );
    m_xPatternLB->RemoveItem(nId);
    m_xPatternLB->InsertItem( nId, Image(aBitmap), aName, static_cast<sal_uInt16>(nPos) );
    m_xPatternLB->SelectItem( nId );

    m_nPatternListState |= ChangeType::MODIFIED;
}


IMPL_LINK_NOARG(SvxPatternTabPage, ClickRenameHdl_Impl, SvxPresetListBox*, void)
{
    const sal_uInt16 nId = m_xPatternLB->GetContextMenuItemId();
    const size_t nPos = m_xPatternLB->GetItemPos(nId);

    if ( nPos == VALUESET_ITEM_NOTFOUND )
        return;

    OUString aDesc(CuiResId(RID_CUISTR_DESC_NEW_PATTERN));
    OUString aName(m_pPatternList->GetBitmap(nPos)->GetName());

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    ScopedVclPtr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog(GetFrameWeld(), aName, aDesc));

    bool bLoop = true;

    while( bLoop && pDlg->Execute() == RET_OK )
    {
        aName = pDlg->GetName();
        sal_Int32 nPatternPos = SearchPatternList(aName);
        bool bValidPatternName = (nPatternPos == static_cast<sal_Int32>(nPos) ) || (nPatternPos == -1);

        if( bValidPatternName )
        {
            bLoop = false;

            m_pPatternList->GetBitmap(nPos)->SetName(aName);

            m_xPatternLB->SetItemText( nId, aName );

            m_nPatternListState |= ChangeType::MODIFIED;
        }
        else
        {
            std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetFrameWeld(), u"cui/ui/queryduplicatedialog.ui"_ustr));
            std::unique_ptr<weld::MessageDialog> xWarnBox(xBuilder->weld_message_dialog(u"DuplicateNameDialog"_ustr));
            xWarnBox->run();
        }
    }
}

IMPL_LINK_NOARG(SvxPatternTabPage, ClickDeleteHdl_Impl, SvxPresetListBox*, void)
{
    const sal_uInt16 nId = m_xPatternLB->GetContextMenuItemId();
    const size_t nPos = m_xPatternLB->GetItemPos(nId);

    if( nPos != VALUESET_ITEM_NOTFOUND )
    {
        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetFrameWeld(), u"cui/ui/querydeletebitmapdialog.ui"_ustr));
        std::unique_ptr<weld::MessageDialog> xQueryBox(xBuilder->weld_message_dialog(u"AskDelBitmapDialog"_ustr));
        if (xQueryBox->run() == RET_YES)
        {
            const bool bDeletingSelectedItem(nId == m_xPatternLB->GetSelectedItemId());
            m_pPatternList->Remove(nPos);
            m_xPatternLB->RemoveItem( nId );
            if (bDeletingSelectedItem)
            {
                m_xPatternLB->SelectItem(m_xPatternLB->GetItemId(/*Position=*/0));
                m_aCtlPreview.Invalidate();
                m_xCtlPixel->Invalidate();
            }
            m_xPatternLB->Resize();

            ChangePatternHdl_Impl(m_xPatternLB.get());

            m_nPatternListState |= ChangeType::MODIFIED;
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
    m_rXFSet.Put(XFillBitmapItem(OUString(), Graphic(m_xBitmapCtl->GetBitmap())));
    m_aCtlPreview.SetAttributes( m_aXFillAttr.GetItemSet() );
    m_aCtlPreview.Invalidate();
}

void SvxPatternTabPage::PointChanged(weld::DrawingArea* pDrawingArea, RectPoint)
{
    if (pDrawingArea == m_xCtlPixel->GetDrawingArea())
    {
        m_xBitmapCtl->SetBmpArray(m_xCtlPixel->GetBitmapPixelPtr());

        // get bitmap and display it
        m_rXFSet.Put(XFillBitmapItem(OUString(), Graphic(m_xBitmapCtl->GetBitmap())));
        m_aCtlPreview.SetAttributes( m_aXFillAttr.GetItemSet() );
        m_aCtlPreview.Invalidate();
    }

    m_xPatternLB->SetNoSelection();
}

sal_Int32 SvxPatternTabPage::SearchPatternList(std::u16string_view rPatternName)
{
    tools::Long nCount = m_pPatternList->Count();
    bool bValidPatternName = true;
    sal_Int32 nPos = -1;

    for(tools::Long i = 0;i < nCount && bValidPatternName;i++)
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
