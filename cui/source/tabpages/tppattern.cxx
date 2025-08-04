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
#include <comphelper/lok.hxx>

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
    BitmapEx GetBitmapEx() const
    {
        if (!pBmpArray)
            return BitmapEx();
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
    , aIconSize(60, 64)
    , m_xCtlPixel(new SvxPixelCtl(this))
    , m_xLbColor(new ColorListBox(m_xBuilder->weld_menu_button(u"LB_COLOR"_ustr),
                [this]{ return GetDialogController()->getDialog(); }))
    , m_xLbBackgroundColor(new ColorListBox(m_xBuilder->weld_menu_button(u"LB_BACKGROUND_COLOR"_ustr),
                [this]{ return GetDialogController()->getDialog(); }))
    , m_xPatternLB(m_xBuilder->weld_icon_view(u"patternpresetlist"_ustr))
    , m_xBtnAdd(m_xBuilder->weld_button(u"BTN_ADD"_ustr))
    , m_xBtnModify(m_xBuilder->weld_button(u"BTN_MODIFY"_ustr))
    , m_xCtlPixelWin(new weld::CustomWeld(*m_xBuilder, u"CTL_PIXEL"_ustr, *m_xCtlPixel))
    , m_xCtlPreview(new weld::CustomWeld(*m_xBuilder, u"CTL_PREVIEW"_ustr, m_aCtlPreview))
{
    // size of the bitmap display
    Size aSize = getDrawPreviewOptimalSize(m_aCtlPreview.GetDrawingArea()->get_ref_device());
    m_xCtlPreview->set_size_request(aSize.Width(), aSize.Height());

    m_xBitmapCtl.reset(new SvxBitmapCtl);

    // this page needs ExchangeSupport
    SetExchangeSupport();

    // setting the output device
    m_rXFSet.Put( XFillStyleItem(drawing::FillStyle_BITMAP) );
    m_rXFSet.Put( XFillBitmapItem(OUString(), Graphic()) );

    m_xBtnAdd->connect_clicked( LINK( this, SvxPatternTabPage, ClickAddHdl_Impl ) );
    m_xBtnModify->connect_clicked( LINK( this, SvxPatternTabPage, ClickModifyHdl_Impl ) );

    m_xPatternLB->connect_selection_changed(LINK(this, SvxPatternTabPage, ChangePatternHdl_Impl));
    m_xPatternLB->connect_mouse_press(LINK(this, SvxPatternTabPage, MousePressHdl));
    m_xPatternLB->connect_query_tooltip(LINK(this, SvxPatternTabPage, QueryTooltipHdl));
    m_xLbColor->SetSelectHdl( LINK( this, SvxPatternTabPage, ChangeColorHdl_Impl ) );
    m_xLbBackgroundColor->SetSelectHdl( LINK( this, SvxPatternTabPage, ChangeColorHdl_Impl ) );

}

SvxPatternTabPage::~SvxPatternTabPage()
{
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
    FillPresetListBox();
}

void SvxPatternTabPage::FillPresetListBox()
{
    m_xPatternLB->clear();

    m_xPatternLB->freeze();
    for (tools::Long nId = 0; nId < m_pPatternList->Count(); nId++)
    {
        const OUString aString(m_pPatternList->GetBitmap(nId)->GetName());

        OUString sId = OUString::number(nId);
        BitmapEx aBitmap = m_pPatternList->GetBitmapForPreview(nId, aIconSize);
        VclPtr<VirtualDevice> aVDev = GetVirtualDevice(aBitmap);

        if (!m_xPatternLB->get_id(nId).isEmpty())
        {
            m_xPatternLB->set_image(nId, aVDev);
            m_xPatternLB->set_id(nId, sId);
            m_xPatternLB->set_text(nId, aString);
        }
        else
        {
            m_xPatternLB->insert(-1, &aString, &sId, aVDev, nullptr);
        }
    }

    m_xPatternLB->thaw();
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
            m_xPatternLB->select( nPos );
        }
    }
    else
        m_xPatternLB->select( 0 );

    // colors could have been deleted
    ChangePatternHdl_Impl(*m_xPatternLB);
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
    OUString sId = m_xPatternLB->get_selected_id();
    sal_Int32 nPos = !sId.isEmpty() ? sId.toInt32() : -1;

    if(nPos != -1)
    {
        const XBitmapEntry* pXBitmapEntry = m_pPatternList->GetBitmap( static_cast<sal_uInt16>(nPos) );
        const OUString aString( m_pPatternList->GetBitmap(nPos)->GetName() );

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

    ChangePatternHdl_Impl(*m_xPatternLB);

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

IMPL_LINK_NOARG(SvxPatternTabPage, ChangePatternHdl_Impl, weld::IconView&, void)
{
    std::unique_ptr<GraphicObject> pGraphicObject;
    OUString sId = m_xPatternLB->get_selected_id();
    sal_Int32 nPos = !sId.isEmpty() ? sId.toInt32() : -1;

    if(nPos != -1)
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

        if(!pGraphicObject && m_xPatternLB->n_children() > 0)
        {
            m_xPatternLB->select(0);
            pGraphicObject.reset(new GraphicObject(m_pPatternList->GetBitmap(0)->GetGraphicObject()));
        }
    }

    if(!pGraphicObject)
        return;

    Color aBackColor;
    Color aPixelColor;
    bool bIs8x8(vcl::bitmap::isHistorical8x8(pGraphicObject->GetGraphic().GetBitmapEx(), aBackColor, aPixelColor));

    m_xLbColor->SetNoSelection();
    m_xLbBackgroundColor->SetNoSelection();

    if(bIs8x8)
    {
        m_xCtlPixel->SetPaintable( true );
        m_xBtnModify->set_sensitive(true);
        m_xBtnAdd->set_sensitive(true);

        // setting the pixel control

        m_xCtlPixel->SetXBitmap(pGraphicObject->GetGraphic().GetBitmapEx());

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
            const BitmapEx aBitmapEx(m_xBitmapCtl->GetBitmapEx());

            pEntry.reset(new XBitmapEntry(Graphic(aBitmapEx), aName));
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

            OUString sId = nCount > 0 ? m_xPatternLB->get_id( nCount - 1 ) : OUString();
            sal_Int32 nId = !sId.isEmpty() ? sId.toInt32() : -1;
            BitmapEx aBitmap = m_pPatternList->GetBitmapForPreview( nCount, aIconSize );
            VclPtr<VirtualDevice> pVDev = GetVirtualDevice(aBitmap);

            m_xPatternLB->insert( nId + 1, &aName, &sId, pVDev, nullptr);
            FillPresetListBox();
            m_xPatternLB->select( nId + 1 );

            m_nPatternListState |= ChangeType::MODIFIED;

            ChangePatternHdl_Impl(*m_xPatternLB);
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
    OUString sId = m_xPatternLB->get_selected_id();
    sal_Int32 nPos = !sId.isEmpty() ? sId.toInt32() : -1;

    if ( nPos == -1 )
        return;

    OUString aName( m_pPatternList->GetBitmap( static_cast<sal_uInt16>(nPos) )->GetName() );

    const BitmapEx aBitmapEx(m_xBitmapCtl->GetBitmapEx());

    // #i123497# Need to replace the existing entry with a new one (old returned needs to be deleted)
    m_pPatternList->Replace(std::make_unique<XBitmapEntry>(Graphic(aBitmapEx), aName), nPos);

    BitmapEx aBitmap = m_pPatternList->GetBitmapForPreview( static_cast<sal_uInt16>(nPos), aIconSize );
    VclPtr<VirtualDevice> pVDev = GetVirtualDevice(aBitmap);

    m_xPatternLB->remove( nPos );
    m_xPatternLB->insert( nPos, &aName, &sId, pVDev, nullptr);
    FillPresetListBox();
    m_xPatternLB->select( nPos );

    m_nPatternListState |= ChangeType::MODIFIED;
}

VclPtr<VirtualDevice> SvxPatternTabPage::GetVirtualDevice(BitmapEx aBitmap)
{
    VclPtr<VirtualDevice> pVDev = VclPtr<VirtualDevice>::Create();
    const Point aNull(0, 0);
    if (pVDev->GetDPIScaleFactor() > 1)
        aBitmap.Scale(pVDev->GetDPIScaleFactor(), pVDev->GetDPIScaleFactor());
    const Size aSize(aBitmap.GetSizePixel());
    pVDev->SetOutputSizePixel(aSize);
    pVDev->DrawBitmapEx(aNull, aBitmap);

    return pVDev;
}

IMPL_LINK(SvxPatternTabPage, QueryTooltipHdl, const weld::TreeIter&, rIter, OUString)
{
    OUString sId = m_xPatternLB->get_id(rIter);
    sal_Int32 nId = !sId.isEmpty() ? sId.toInt32() : -1;

    if (nId >= 0)
    {
        return m_pPatternList->GetBitmap(nId)->GetName();
    }
    return OUString();
}

IMPL_LINK(SvxPatternTabPage, MousePressHdl, const MouseEvent&, rMEvt, bool)
{
    if (!rMEvt.IsRight())
        return false;

    // Disable context menu for LibreOfficeKit mode
    if (comphelper::LibreOfficeKit::isActive())
        return false;

    const Point& pPos = rMEvt.GetPosPixel();
    for (int i = 0; i < m_xPatternLB->n_children(); i++)
    {
        const ::tools::Rectangle aRect = m_xPatternLB->get_rect(i);
        if (aRect.Contains(pPos))
        {
            ShowContextMenu(pPos);
            break;
        }
    }
    return false;
}

void SvxPatternTabPage::ShowContextMenu(const Point& pPos)
{
    ::tools::Rectangle aRect(pPos, Size(1, 1));
    std::unique_ptr<weld::Builder> xBuilder(
        Application::CreateBuilder(m_xPatternLB.get(), u"svx/ui/presetmenu.ui"_ustr));
    std::unique_ptr<weld::Menu> xMenu(xBuilder->weld_menu(u"menu"_ustr));

    xMenu->connect_activate(LINK(this, SvxPatternTabPage, OnPopupEnd));
    xMenu->popup_at_rect(m_xPatternLB.get(), aRect);
}

IMPL_LINK(SvxPatternTabPage, OnPopupEnd, const OUString&, sCommand, void)
{
    sLastItemIdent = sCommand;
    if (sLastItemIdent.isEmpty())
        return;

    Application::PostUserEvent(LINK(this, SvxPatternTabPage, MenuSelectAsyncHdl));
}

IMPL_LINK_NOARG(SvxPatternTabPage, MenuSelectAsyncHdl, void*, void)
{
    if (sLastItemIdent == u"rename")
    {
        ClickRenameHdl();
    }
    else if (sLastItemIdent == u"delete")
    {
        ClickDeleteHdl();
    }
}

void SvxPatternTabPage::ClickRenameHdl()
{
    const OUString sId = m_xPatternLB->get_selected_id();
    const sal_Int32 nPos = !sId.isEmpty() ? sId.toInt32() : -1;

    if ( nPos == -1 )
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
        bool bValidPatternName = (nPatternPos == nPos ) || (nPatternPos == -1);

        if( bValidPatternName )
        {
            bLoop = false;

            m_pPatternList->GetBitmap(nPos)->SetName(aName);

            m_xPatternLB->set_text( nPos, aName );

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

void SvxPatternTabPage::ClickDeleteHdl()
{
    const OUString sId = m_xPatternLB->get_selected_id();
    const sal_Int32 nPos = !sId.isEmpty() ? sId.toInt32() : -1;

    if( nPos != -1 )
    {
        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetFrameWeld(), u"cui/ui/querydeletebitmapdialog.ui"_ustr));
        std::unique_ptr<weld::MessageDialog> xQueryBox(xBuilder->weld_message_dialog(u"AskDelBitmapDialog"_ustr));
        if (xQueryBox->run() == RET_YES)
        {
            m_pPatternList->Remove(nPos);
            m_xPatternLB->remove( nPos );

            FillPresetListBox();

            sal_Int32 nNextId = nPos;
            if (nPos >= m_xPatternLB->n_children())
                nNextId = m_xPatternLB->n_children() - 1;

            if(m_xPatternLB->n_children() > 0)
                m_xPatternLB->select(nNextId);

            ChangePatternHdl_Impl(*m_xPatternLB);

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
