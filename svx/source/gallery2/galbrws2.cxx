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


#include <sot/formats.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <vcl/transfer.hxx>
#include <vcl/virdev.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/graphicfilter.hxx>
#include <helpids.h>
#include <svx/svxids.hrc>
#include <galobj.hxx>
#include <svx/gallery1.hxx>
#include <svx/galtheme.hxx>
#include <svx/galctrl.hxx>
#include <svx/galmisc.hxx>
#include <galbrws2.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <svx/svxdlg.hxx>
#include <svx/galleryitem.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/gallery/GalleryItemType.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/style/GraphicLocation.hpp>
#include <map>
#include <memory>
#include <cppuhelper/implbase.hxx>
#include <osl/diagnose.h>

GalleryBrowserMode GalleryBrowser2::meInitMode = GALLERYBROWSERMODE_ICON;

struct DispatchInfo
{
    css::util::URL                                  TargetURL;
    css::uno::Sequence< css::beans::PropertyValue > Arguments;
    css::uno::Reference< css::frame::XDispatch >    Dispatch;
};

IMPL_STATIC_LINK( GalleryBrowser2, AsyncDispatch_Impl, void*, p, void )
{
    DispatchInfo* pDispatchInfo = static_cast<DispatchInfo*>(p);
    if ( pDispatchInfo && pDispatchInfo->Dispatch.is() )
    {
        try
        {
            pDispatchInfo->Dispatch->dispatch( pDispatchInfo->TargetURL,
                                               pDispatchInfo->Arguments );
        }
        catch ( const css::uno::Exception& )
        {
        }
    }

    delete pDispatchInfo;
}

namespace
{

struct CommandInfo
{
    css::util::URL                               URL;
    css::uno::Reference< css::frame::XDispatch > Dispatch;

    explicit CommandInfo( const OUString &rURL )
    {
        URL.Complete = rURL;
    }
};

class GalleryThemePopup : public ::cppu::WeakImplHelper< css::frame::XStatusListener >
{
private:
    const GalleryTheme* mpTheme;
    sal_uInt32          mnObjectPos;
    bool                mbPreview;
    std::unique_ptr<weld::Builder> mxBuilder;
    std::unique_ptr<weld::Menu> mxPopupMenu;
    std::unique_ptr<weld::Menu> mxBackgroundPopup;
    GalleryBrowser2*  mpBrowser;

    typedef std::map< int, CommandInfo > CommandInfoMap;
    CommandInfoMap   m_aCommandInfo;

    static void Execute( const CommandInfo &rCmdInfo,
                  const css::uno::Sequence< css::beans::PropertyValue > &rArguments );

    void MenuSelectHdl(const OString& rIdent);
    void BackgroundMenuSelectHdl(sal_uInt16 nId);
public:
    GalleryThemePopup(weld::Widget* pParent,
                      const GalleryTheme* pTheme,
                      sal_uInt32 nObjectPos,
                      bool bPreview,
                      GalleryBrowser2* pBrowser);

    void ExecutePopup(weld::Widget* pParent, const ::Point &rPos);

    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent &rEvent) override;
    virtual void SAL_CALL disposing( const css::lang::EventObject &rSource) override;
};


GalleryThemePopup::GalleryThemePopup(
    weld::Widget* pParent,
    const GalleryTheme* pTheme,
    sal_uInt32 nObjectPos,
    bool bPreview,
    GalleryBrowser2* pBrowser )
    : mpTheme( pTheme )
    , mnObjectPos( nObjectPos )
    , mbPreview( bPreview )
    , mxBuilder(Application::CreateBuilder(pParent, "svx/ui/gallerymenu2.ui"))
    , mxPopupMenu(mxBuilder->weld_menu("menu"))
    , mxBackgroundPopup(mxBuilder->weld_menu("backgroundmenu"))
    , mpBrowser( pBrowser )
{
    // SID_GALLERY_ENABLE_ADDCOPY
    m_aCommandInfo.emplace(
            SID_GALLERY_ENABLE_ADDCOPY,
            CommandInfo( ".uno:GalleryEnableAddCopy" ));
    // SID_GALLERY_BG_BRUSH
    m_aCommandInfo.emplace(
            SID_GALLERY_BG_BRUSH,
            CommandInfo( ".uno:BackgroundImage" ));
    // SID_GALLERY_FORMATS
    m_aCommandInfo.emplace(
            SID_GALLERY_FORMATS,
            CommandInfo( ".uno:InsertGalleryPic" ));

}

void SAL_CALL GalleryThemePopup::statusChanged(
    const css::frame::FeatureStateEvent &rEvent )
{
    const OUString &rURL = rEvent.FeatureURL.Complete;
    if ( rURL == ".uno:GalleryEnableAddCopy" )
    {
        if ( !rEvent.IsEnabled )
        {
            mxPopupMenu->set_visible("add", false);
        }
    }
    else if ( rURL == ".uno:BackgroundImage" )
    {
        mxBackgroundPopup->clear();
        if ( rEvent.IsEnabled )
        {
            OUString sItem;
            css::uno::Sequence< OUString > sItems;
            if ( ( rEvent.State >>= sItem ) && sItem.getLength() )
            {
                mxBackgroundPopup->append(OUString::number(1), sItem);
            }
            else if ( ( rEvent.State >>= sItems ) && sItems.hasElements() )
            {
                sal_uInt16 nId = 1;
                for ( const OUString& rStr : std::as_const(sItems) )
                {
                    mxBackgroundPopup->append(OUString::number(nId), rStr);
                    nId++;
                }
            }
        }
    }
}

void SAL_CALL GalleryThemePopup::disposing(
    const css::lang::EventObject &/*rSource*/)
{
}

void GalleryThemePopup::Execute(
    const CommandInfo &rCmdInfo,
    const css::uno::Sequence< css::beans::PropertyValue > &rArguments )
{
    if ( rCmdInfo.Dispatch.is() )
    {
        std::unique_ptr<DispatchInfo> pInfo(new DispatchInfo);
        pInfo->TargetURL = rCmdInfo.URL;
        pInfo->Arguments = rArguments;
        pInfo->Dispatch = rCmdInfo.Dispatch;

        if ( Application::PostUserEvent(
                LINK( nullptr, GalleryBrowser2, AsyncDispatch_Impl), pInfo.get() ) )
            pInfo.release();
    }
}

void GalleryThemePopup::ExecutePopup(weld::Widget* pParent, const ::Point &rPos)
{
    css::uno::Reference< css::frame::XStatusListener > xThis( this );

    const SgaObjKind eObjKind = mpTheme->GetObjectKind( mnObjectPos );
    INetURLObject    aURL;

    const_cast< GalleryTheme* >( mpTheme )->GetURL( mnObjectPos, aURL );
    const bool bValidURL = ( aURL.GetProtocol() != INetProtocol::NotValid );

    mxPopupMenu->set_visible("add", bValidURL && SgaObjKind::Sound != eObjKind);

    mxPopupMenu->set_visible("preview", bValidURL);
    mxPopupMenu->set_active("preview", mbPreview);

    if( mpTheme->IsReadOnly() || !mpTheme->GetObjectCount() )
    {
        mxPopupMenu->set_visible("delete", false);
        mxPopupMenu->set_visible("title", false);
        if (mpTheme->IsReadOnly())
            mxPopupMenu->set_visible("paste", false);

        if (!mpTheme->GetObjectCount())
            mxPopupMenu->set_visible("copy", false);
    }
    else
    {
        mxPopupMenu->set_visible("delete", !mbPreview);
        mxPopupMenu->set_visible("title", true);
        mxPopupMenu->set_visible("copy", true);
        mxPopupMenu->set_visible("paste", true);
    }

    // update status
    css::uno::Reference< css::frame::XDispatchProvider> xDispatchProvider(
        GalleryBrowser2::GetFrame(), css::uno::UNO_QUERY );
    css::uno::Reference< css::util::XURLTransformer > xTransformer(
        mpBrowser->GetURLTransformer() );
    for ( auto& rInfo : m_aCommandInfo )
    {
        try
        {
            CommandInfo &rCmdInfo = rInfo.second;
            if ( xTransformer.is() )
                xTransformer->parseStrict( rCmdInfo.URL );

            if ( xDispatchProvider.is() )
            {
                rCmdInfo.Dispatch = xDispatchProvider->queryDispatch(
                    rCmdInfo.URL,
                    "_self",
                    css::frame::FrameSearchFlag::SELF );
            }

            if ( rCmdInfo.Dispatch.is() )
            {
                rCmdInfo.Dispatch->addStatusListener( this, rCmdInfo.URL );
                rCmdInfo.Dispatch->removeStatusListener( this, rCmdInfo.URL );
            }
        }
        catch ( ... )
        {}
    }

    if( !mxBackgroundPopup->n_children() || ( eObjKind == SgaObjKind::SvDraw ) || ( eObjKind == SgaObjKind::Sound ) )
        mxPopupMenu->set_visible("background", false);
    else
        mxPopupMenu->set_visible("background", true);

    MenuSelectHdl(mxPopupMenu->popup_at_rect(pParent, tools::Rectangle(rPos, Size(1,1))));
}

void GalleryThemePopup::MenuSelectHdl(const OString& rIdent)
{
    if (rIdent.isEmpty())
        return;

    sal_uInt16 nSubMenuId = rIdent.toUInt32();
    if (nSubMenuId)
    {
        BackgroundMenuSelectHdl(nSubMenuId-1);
        return;
    }

    if (rIdent == "add")
    {
        const CommandInfoMap::const_iterator it = m_aCommandInfo.find( SID_GALLERY_FORMATS );
        if (it != m_aCommandInfo.end())
            mpBrowser->DispatchAdd(it->second.Dispatch, it->second.URL);
    }
    else
        mpBrowser->Execute(rIdent);
}

void GalleryThemePopup::BackgroundMenuSelectHdl(sal_uInt16 nPos)
{
    OUString aURL( mpBrowser->GetURL().GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
    OUString aFilterName( mpBrowser->GetFilterName() );

    css::uno::Sequence< css::beans::PropertyValue > aArgs( 6 );
    aArgs[0].Name = "Background.Transparent";
    aArgs[0].Value <<= sal_Int32( 0 ); // 0 - 100
    aArgs[1].Name = "Background.BackColor";
    aArgs[1].Value <<= sal_Int32( - 1 );
    aArgs[2].Name = "Background.URL";
    aArgs[2].Value <<= aURL;
    aArgs[3].Name = "Background.Filtername"; // FIXME should be FilterName
    aArgs[3].Value <<= aFilterName;
    aArgs[4].Name = "Background.Position";
    aArgs[4].Value <<= css::style::GraphicLocation_TILED;
    aArgs[5].Name = "Position";
    aArgs[5].Value <<= nPos;

    const CommandInfoMap::const_iterator it = m_aCommandInfo.find( SID_GALLERY_BG_BRUSH );
    if ( it != m_aCommandInfo.end() )
        Execute( it->second, aArgs );
}

} // end anonymous namespace

GalleryBrowser2::GalleryBrowser2(weld::Builder& rBuilder, Gallery* pGallery)
    : mpGallery(pGallery)
    , mpCurTheme(nullptr)
    , mxIconView(new GalleryIconView(this, rBuilder.weld_scrolled_window("galleryscroll", true)))
    , mxIconViewWin(new weld::CustomWeld(rBuilder, "gallery", *mxIconView))
    , mxListView(rBuilder.weld_tree_view("gallerylist"))
    , mxPreview(new GalleryPreview(this, rBuilder.weld_scrolled_window("previewscroll")))
    , mxPreviewWin(new weld::CustomWeld(rBuilder, "preview", *mxPreview))
    , mxIconButton(rBuilder.weld_toggle_button("icon"))
    , mxListButton(rBuilder.weld_toggle_button("list"))
    , mxInfoBar(rBuilder.weld_label("label"))
    , mxDev(mxListView->create_virtual_device())
    , maPreviewSize(28, 28)
    , mnCurActionPos      ( 0xffffffff )
    , meMode              ( GALLERYBROWSERMODE_NONE )
    , meLastMode          ( GALLERYBROWSERMODE_NONE )
{
    mxDev->SetOutputSizePixel(maPreviewSize);

    m_xContext.set( ::comphelper::getProcessComponentContext() );

    int nHeight = mxListView->get_height_rows(10);
    mxListView->set_size_request(-1, nHeight);
    mxIconView->set_size_request(-1, nHeight);

    m_xTransformer.set(
        m_xContext->getServiceManager()->createInstanceWithContext(
            "com.sun.star.util.URLTransformer", m_xContext ),
        css::uno::UNO_QUERY );

    mxIconButton->set_help_id(HID_GALLERY_ICONVIEW);
    mxListButton->set_help_id(HID_GALLERY_LISTVIEW);

    mxIconButton->connect_toggled( LINK( this, GalleryBrowser2, SelectTbxHdl ) );
    mxListButton->connect_toggled( LINK( this, GalleryBrowser2, SelectTbxHdl ) );

    mxIconView->SetSelectHdl( LINK( this, GalleryBrowser2, SelectObjectValueSetHdl ) );
    mxListView->connect_visible_range_changed(LINK(this, GalleryBrowser2, VisRowsScrolledHdl));
    mxListView->connect_size_allocate(LINK(this, GalleryBrowser2, SizeAllocHdl));
    mxListView->connect_changed( LINK( this, GalleryBrowser2, SelectObjectHdl ) );
    mxListView->connect_popup_menu(LINK(this, GalleryBrowser2, PopupMenuHdl));
    mxListView->connect_key_press(LINK(this, GalleryBrowser2, KeyInputHdl));
    mxListView->connect_row_activated(LINK(this, GalleryBrowser2, RowActivatedHdl));
    mxDragDropTargetHelper.reset(new GalleryDragDrop(this, mxListView->get_drop_target()));
    mxListView->connect_drag_begin(LINK(this, GalleryBrowser2, DragBeginHdl));

    mxListView->set_help_id(HID_GALLERY_WINDOW);

    SetMode( ( GALLERYBROWSERMODE_PREVIEW != GalleryBrowser2::meInitMode ) ? GalleryBrowser2::meInitMode : GALLERYBROWSERMODE_ICON );
}

IMPL_LINK(GalleryBrowser2, PopupMenuHdl, const CommandEvent&, rCEvt, bool)
{
    if (rCEvt.GetCommand() != CommandEventId::ContextMenu)
        return false;
    ShowContextMenu(rCEvt);
    return true;
}

IMPL_LINK(GalleryBrowser2, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    return KeyInput(rKEvt);
}

IMPL_LINK_NOARG(GalleryBrowser2, RowActivatedHdl, weld::TreeView&, bool)
{
    TogglePreview();
    return true;
}

GalleryBrowser2::~GalleryBrowser2()
{
    if (mpCurTheme)
        mpGallery->ReleaseTheme( mpCurTheme, *this );
}

void GalleryBrowser2::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    const GalleryHint& rGalleryHint = static_cast<const GalleryHint&>(rHint);

    switch( rGalleryHint.GetType() )
    {
        case GalleryHintType::THEME_UPDATEVIEW:
        {
            if( GALLERYBROWSERMODE_PREVIEW == GetMode() )
                SetMode( meLastMode );

            ImplUpdateViews( reinterpret_cast<size_t>(rGalleryHint.GetData1()) + 1 );
        }
        break;

        default:
        break;
    }
}

sal_Int8 GalleryBrowser2::AcceptDrop( const DropTargetHelper& rTarget )
{
    sal_Int8 nRet = DND_ACTION_NONE;

    if( mpCurTheme && !mpCurTheme->IsReadOnly() )
    {
        if( !mpCurTheme->IsDragging() )
        {
            if( rTarget.IsDropFormatSupported( SotClipboardFormatId::DRAWING ) ||
                rTarget.IsDropFormatSupported( SotClipboardFormatId::FILE_LIST ) ||
                rTarget.IsDropFormatSupported( SotClipboardFormatId::SIMPLE_FILE ) ||
                rTarget.IsDropFormatSupported( SotClipboardFormatId::SVXB ) ||
                rTarget.IsDropFormatSupported( SotClipboardFormatId::GDIMETAFILE ) ||
                rTarget.IsDropFormatSupported( SotClipboardFormatId::BITMAP ) )
            {
                nRet = DND_ACTION_COPY;
            }
        }
        else
            nRet = DND_ACTION_COPY;
    }

    return nRet;
}

sal_Int8 GalleryBrowser2::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    sal_Int8 nRet = DND_ACTION_NONE;

    if( mpCurTheme )
    {
        Point aSelPos;
        const sal_uInt32 nItemId = ImplGetSelectedItemId( &rEvt.maPosPixel, aSelPos );
        const sal_uInt32 nInsertPos = (nItemId ? (nItemId - 1) : mpCurTheme->GetObjectCount());

        if( mpCurTheme->IsDragging() )
            mpCurTheme->ChangeObjectPos( mpCurTheme->GetDragPos(), nInsertPos );
        else
            nRet = mpCurTheme->InsertTransferable( rEvt.maDropEvent.Transferable, nInsertPos ) ? 1 : 0;
    }

    return nRet;
}

bool GalleryBrowser2::StartDrag()
{
    if (!mpCurTheme)
        return true;
    return m_xHelper->StartDrag();
}

IMPL_LINK(GalleryBrowser2, DragBeginHdl, bool&, rUnsetDragIcon, bool)
{
    rUnsetDragIcon = false;
    return StartDrag();
}

void GalleryBrowser2::TogglePreview()
{
    SetMode( ( GALLERYBROWSERMODE_PREVIEW != GetMode() ) ? GALLERYBROWSERMODE_PREVIEW : meLastMode );
    GetViewWindow()->grab_focus();
}

void GalleryBrowser2::ShowContextMenu(const CommandEvent& rCEvt)
{
    Point aMousePos = rCEvt.GetMousePosPixel();
    Point aSelPos;
    const sal_uInt32 nItemId = ImplGetSelectedItemId( rCEvt.IsMouseEvent() ? &aMousePos : nullptr, aSelPos );

    if( !(mpCurTheme && nItemId && ( nItemId <= mpCurTheme->GetObjectCount() )) )
        return;

    ImplSelectItemId( nItemId );

    css::uno::Reference< css::frame::XFrame > xFrame( GetFrame() );
    if ( !xFrame.is() )
        return;

    weld::Widget* pParent = GetViewWindow();
    mnCurActionPos = nItemId - 1;
    rtl::Reference< GalleryThemePopup > xPopup(
        new GalleryThemePopup(
            pParent,
            mpCurTheme,
            mnCurActionPos,
            GALLERYBROWSERMODE_PREVIEW == GetMode(),
            this ) );
    xPopup->ExecutePopup(pParent, aSelPos);
}

bool GalleryBrowser2::ViewBoxHasFocus() const
{
    return mxIconButton->has_focus() || mxListButton->has_focus();
}

bool GalleryBrowser2::KeyInput(const KeyEvent& rKEvt)
{
    Point       aSelPos;
    const sal_uInt32 nItemId = ImplGetSelectedItemId( nullptr, aSelPos );
    bool bRet = false;

    if (!ViewBoxHasFocus() && nItemId && mpCurTheme)
    {
        OString sExecuteIdent;
        INetURLObject       aURL;

        mpCurTheme->GetURL( nItemId - 1, aURL );

        const bool  bValidURL = ( aURL.GetProtocol() != INetProtocol::NotValid );
        bool        bPreview = bValidURL;
        bool        bDelete = false;
        bool        bTitle = false;

        if( !mpCurTheme->IsReadOnly() && mpCurTheme->GetObjectCount() )
        {
            bDelete = ( GALLERYBROWSERMODE_PREVIEW != GetMode() );
            bTitle = true;
        }

        switch( rKEvt.GetKeyCode().GetCode() )
        {
            case KEY_SPACE:
            case KEY_RETURN:
            case KEY_P:
            {
                if( bPreview )
                {
                    TogglePreview();
                    bRet = true;
                }
            }
            break;

            case KEY_INSERT:
            case KEY_I:
            {
                // Inserting a gallery item in the document must be dispatched
                if( bValidURL )
                {
                    DispatchAdd(css::uno::Reference<css::frame::XDispatch>(), css::util::URL());
                    return true;
                }
            }
            break;

            case KEY_DELETE:
            case KEY_D:
            {
                if( bDelete )
                    sExecuteIdent = "delete";
            }
            break;

            case KEY_T:
            {
                if( bTitle )
                    sExecuteIdent = "title";
            }
            break;

            default:
            break;
        }

        if (!sExecuteIdent.isEmpty())
        {
            Execute(sExecuteIdent);
            bRet = true;
        }
    }

    return bRet;
}

void GalleryBrowser2::SelectTheme( std::u16string_view rThemeName )
{
    if( mpCurTheme )
        mpGallery->ReleaseTheme( mpCurTheme, *this );

    mpCurTheme = mpGallery->AcquireTheme( rThemeName, *this );

    m_xHelper.set(new GalleryTransferable(mpCurTheme, 0, true));
    rtl::Reference<TransferDataContainer> xHelper(m_xHelper);
    mxListView->enable_drag_source(xHelper, DND_ACTION_COPY | DND_ACTION_LINK);
    mxIconView->SetDragDataTransferrable(xHelper, DND_ACTION_COPY | DND_ACTION_LINK);
    mxPreview->SetDragDataTransferrable(xHelper, DND_ACTION_COPY | DND_ACTION_LINK);

    mxIconView->SetTheme(mpCurTheme);
    mxPreview->SetTheme(mpCurTheme);

    if( GALLERYBROWSERMODE_PREVIEW == GetMode() )
        meMode = meLastMode;

    ImplUpdateViews( 1 );

    bool bIconMode = (GALLERYBROWSERMODE_ICON == GetMode());
    mxIconButton->set_sensitive(true);
    mxListButton->set_sensitive(true);
    mxIconButton->set_active(bIconMode);
    mxListButton->set_active(!bIconMode);
}

void GalleryBrowser2::SetMode( GalleryBrowserMode eMode )
{
    if( GetMode() == eMode )
        return;

    meLastMode = GetMode();

    switch( eMode )
    {
        case GALLERYBROWSERMODE_ICON:
        {
            mxListView->hide();

            mxPreview->Hide();
            mxPreview->SetGraphic( Graphic() );
            GalleryPreview::PreviewMedia( INetURLObject() );

            mxIconView->Show();

            mxIconButton->set_sensitive(true);
            mxListButton->set_sensitive(true);

            mxIconButton->set_active(true);
            mxListButton->set_active(false);
        }
        break;

        case GALLERYBROWSERMODE_LIST:
        {
            mxIconView->Hide();

            mxPreview->Hide();
            mxPreview->SetGraphic( Graphic() );
            GalleryPreview::PreviewMedia( INetURLObject() );

            mxListView->show();
            UpdateRows(true);

            mxIconButton->set_sensitive(true);
            mxListButton->set_sensitive(true);

            mxIconButton->set_active(false);
            mxListButton->set_active(true);
        }
        break;

        case GALLERYBROWSERMODE_PREVIEW:
        {
            Graphic     aGraphic;
            Point       aSelPos;
            const sal_uInt32 nItemId = ImplGetSelectedItemId( nullptr, aSelPos );

            if( nItemId )
            {
                const sal_uInt32 nPos = nItemId - 1;

                mxIconView->Hide();
                mxListView->hide();

                if( mpCurTheme )
                    mpCurTheme->GetGraphic( nPos, aGraphic );

                mxPreview->SetGraphic( aGraphic );
                mxPreview->Show();

                if( mpCurTheme && mpCurTheme->GetObjectKind( nPos ) == SgaObjKind::Sound )
                    GalleryPreview::PreviewMedia( mpCurTheme->GetObjectURL( nPos ) );

                mxIconButton->set_sensitive(false);
                mxListButton->set_sensitive(false);
            }
        }
        break;

        default:
            break;
    }

    GalleryBrowser2::meInitMode = meMode = eMode;
}

weld::Widget* GalleryBrowser2::GetViewWindow() const
{
    weld::Widget* pRet;

    switch( GetMode() )
    {
        case GALLERYBROWSERMODE_LIST: pRet = mxListView.get(); break;
        case GALLERYBROWSERMODE_PREVIEW: pRet = mxPreview->GetDrawingArea(); break;

        default:
            pRet = mxIconView->GetDrawingArea();
        break;
    }

    return pRet;
}

void GalleryBrowser2::Travel( GalleryBrowserTravel eTravel )
{
    if( !mpCurTheme )
        return;

    Point       aSelPos;
    const sal_uInt32 nItemId = ImplGetSelectedItemId( nullptr, aSelPos );

    if( !nItemId )
        return;

    sal_uInt32 nNewItemId = nItemId;

    switch( eTravel )
    {
        case GalleryBrowserTravel::First:     nNewItemId = 1; break;
        case GalleryBrowserTravel::Last:      nNewItemId = mpCurTheme->GetObjectCount(); break;
        case GalleryBrowserTravel::Previous:  nNewItemId--; break;
        case GalleryBrowserTravel::Next:      nNewItemId++; break;
        default:
            break;
    }

    if( nNewItemId < 1 )
        nNewItemId = 1;
    else if( nNewItemId > mpCurTheme->GetObjectCount() )
        nNewItemId = mpCurTheme->GetObjectCount();

    if( nNewItemId == nItemId )
        return;

    ImplSelectItemId( nNewItemId );
    ImplUpdateInfoBar();

    if( GALLERYBROWSERMODE_PREVIEW != GetMode() )
        return;

    Graphic     aGraphic;
    const sal_uInt32 nPos = nNewItemId - 1;

    mpCurTheme->GetGraphic( nPos, aGraphic );
    mxPreview->SetGraphic( aGraphic );

    if( SgaObjKind::Sound == mpCurTheme->GetObjectKind( nPos ) )
        GalleryPreview::PreviewMedia( mpCurTheme->GetObjectURL( nPos ) );

    mxPreview->Invalidate();
}

void GalleryBrowser2::ImplUpdateViews( sal_uInt16 nSelectionId )
{
    mxIconView->Hide();
    mxListView->hide();
    mxPreview->Hide();

    mxIconView->Clear();
    mxListView->clear();

    if( mpCurTheme )
    {
        const int nAlwaysUpToDate = 15;

        mxListView->freeze();

        sal_uInt32 nCount = mpCurTheme->GetObjectCount();
        for (sal_uInt32 i = 0; i < nCount; ++i)
        {
            mxIconView->InsertItem(i + 1); // skip reserved id 0
            mxListView->append(OUString::number(i), ""); // create on-demand in VisRowsScrolledHdl

            if (i == nAlwaysUpToDate) // fill in the first block
                UpdateRows(false);
        }

        if (nCount < nAlwaysUpToDate) // if less than block size, fill in all of them
            UpdateRows(false);

        mxListView->thaw();

        ImplSelectItemId( std::min<sal_uInt16>( nSelectionId, mpCurTheme->GetObjectCount() ) );
    }

    switch( GetMode() )
    {
        case GALLERYBROWSERMODE_ICON: mxIconView->Show(); break;
        case GALLERYBROWSERMODE_LIST:
            mxListView->show();
            UpdateRows(true);
            break;
        case GALLERYBROWSERMODE_PREVIEW: mxPreview->Show(); break;

        default:
        break;
    }

    ImplUpdateInfoBar();
}

void GalleryBrowser2::UpdateRows(bool bVisibleOnly)
{
    auto lambda = [this](weld::TreeIter& rEntry){
        // id is non-null if the preview is pending creation
        OUString sId(mxListView->get_id(rEntry));
        if (sId.isEmpty())
            return false;

        // get the icon for the listview
        BitmapEx aBitmapEx;
        Size aPreparedSize;

        OUString sItemTextTitle;
        OUString sItemTextPath;

        sal_Int32 i = sId.toUInt32();
        mpCurTheme->GetPreviewBitmapExAndStrings(i, aBitmapEx, aPreparedSize, sItemTextTitle, sItemTextPath);

        bool bNeedToCreate(aBitmapEx.IsEmpty());
        if (!bNeedToCreate && (sItemTextTitle.isEmpty() || aPreparedSize != maPreviewSize))
            bNeedToCreate = true;

        if (bNeedToCreate)
        {
            std::unique_ptr<SgaObject> xObj = mpCurTheme->AcquireObject(i);
            if (xObj)
            {
                aBitmapEx = xObj->createPreviewBitmapEx(maPreviewSize);
                sItemTextTitle = GalleryBrowser2::GetItemText(*xObj, GalleryItemFlags::Title);
                sItemTextPath = GalleryBrowser2::GetItemText(*xObj, GalleryItemFlags::Path);

                mpCurTheme->SetPreviewBitmapExAndStrings(i, aBitmapEx, maPreviewSize, sItemTextTitle, sItemTextPath);
            }
        }

        if (!aBitmapEx.IsEmpty())
        {
            const Size aBitmapExSizePixel(aBitmapEx.GetSizePixel());
            const Point aPos(
                ((maPreviewSize.Width() - aBitmapExSizePixel.Width()) >> 1),
                ((maPreviewSize.Height() - aBitmapExSizePixel.Height()) >> 1));

            mxDev->Erase();

            if (aBitmapEx.IsTransparent())
            {
                // draw checkered background
                GalleryIconView::drawTransparenceBackground(*mxDev, aPos, aBitmapExSizePixel);
            }

            mxDev->DrawBitmapEx(aPos, aBitmapEx);
        }

        mxListView->set_text(rEntry, sItemTextTitle);
        mxListView->set_image(rEntry, *mxDev);
        mxListView->set_id(rEntry, OUString());

        return false;
    };

    if (bVisibleOnly)
    {
        // ensure all visible entries are up to date
        mxListView->visible_foreach(lambda);
        // and ensure all selected entries are up to date
        mxListView->selected_foreach(lambda);
        return;
    }

    mxListView->all_foreach(lambda);
}

IMPL_LINK_NOARG(GalleryBrowser2, VisRowsScrolledHdl, weld::TreeView&, void)
{
    UpdateRows(true);
}

IMPL_LINK_NOARG(GalleryBrowser2, SizeAllocHdl, const Size&, void)
{
    UpdateRows(true);
}

void GalleryBrowser2::ImplUpdateInfoBar()
{
    if (!mpCurTheme)
        return;
    mxInfoBar->set_label( mpCurTheme->GetName() );
}

void GalleryBrowser2::ImplUpdateSelection()
{
    if (!mpCurTheme)
        return;
    auto nSelectedObject = (GALLERYBROWSERMODE_ICON == GetMode()) ? (mxIconView->GetSelectedItemId() - 1) : mxListView->get_selected_index();
    m_xHelper->SelectObject(nSelectedObject);
}

sal_uInt32 GalleryBrowser2::ImplGetSelectedItemId( const Point* pSelPos, Point& rSelPos )
{
    sal_uInt32 nRet = 0;

    if( GALLERYBROWSERMODE_PREVIEW == GetMode() )
    {
        nRet = ( ( GALLERYBROWSERMODE_ICON == meLastMode ) ? mxIconView->GetSelectedItemId() : ( mxListView->get_selected_index() + 1 ) );

        if( pSelPos )
            rSelPos = *pSelPos;
        else
        {
            Size aOutputSizePixel(mxPreview->GetOutputSizePixel());
            rSelPos = Point( aOutputSizePixel.Width() >> 1, aOutputSizePixel.Height() >> 1 );
        }
    }
    else if (GALLERYBROWSERMODE_ICON == GetMode())
    {
        if (pSelPos)
        {
            nRet = mxIconView->GetItemId( *pSelPos );
            rSelPos = *pSelPos;
        }
        else
        {
            nRet = mxIconView->GetSelectedItemId();
            rSelPos = mxIconView->GetItemRect(nRet).Center();
        }
    }
    else
    {
        std::unique_ptr<weld::TreeIter> xIter = mxListView->make_iterator();
        if( pSelPos )
        {
            if (mxListView->get_dest_row_at_pos(*pSelPos, xIter.get(), false))
                nRet = mxListView->get_iter_index_in_parent(*xIter) + 1;
            rSelPos = *pSelPos;
        }
        else
        {
            if (mxListView->get_selected(xIter.get()))
            {
                nRet = mxListView->get_iter_index_in_parent(*xIter) + 1;
                rSelPos = mxListView->get_row_area(*xIter).Center();
            }
        }
    }

    if( nRet && ( !mpCurTheme || ( nRet > mpCurTheme->GetObjectCount() ) ) )
    {
        nRet = 0;
    }

    return nRet;
}

void GalleryBrowser2::ImplSelectItemId(sal_uInt32 nItemId)
{
    if( nItemId )
    {
        mxIconView->SelectItem(nItemId);
        mxListView->select( nItemId - 1 );
        ImplUpdateSelection();
    }
}

css::uno::Reference< css::frame::XFrame >
GalleryBrowser2::GetFrame()
{
    css::uno::Reference< css::frame::XFrame > xFrame;
    SfxViewFrame* pCurrentViewFrame = SfxViewFrame::Current();
    if ( pCurrentViewFrame )
    {
        SfxBindings& rBindings = pCurrentViewFrame->GetBindings();
        xFrame.set( rBindings.GetActiveFrame() );
    }

    return xFrame;
}

void GalleryBrowser2::DispatchAdd(
    const css::uno::Reference< css::frame::XDispatch > &rxDispatch,
    const css::util::URL &rURL)
{
    Point aSelPos;
    const sal_uInt32 nItemId = ImplGetSelectedItemId( nullptr, aSelPos );

    if( !mpCurTheme || !nItemId )
        return;

    mnCurActionPos = nItemId - 1;

    css::uno::Reference< css::frame::XDispatch > xDispatch( rxDispatch );
    css::util::URL aURL = rURL;

    if ( !xDispatch.is() )
    {
        css::uno::Reference< css::frame::XDispatchProvider > xDispatchProvider(
            GetFrame(), css::uno::UNO_QUERY );
        if ( !xDispatchProvider.is() || !m_xTransformer.is() )
            return;

        aURL.Complete = ".uno:InsertGalleryPic";
        m_xTransformer->parseStrict( aURL );
        xDispatch = xDispatchProvider->queryDispatch(
            aURL,
            "_self",
            css::frame::FrameSearchFlag::SELF );
    }

    if ( !xDispatch.is() )
        return;

    sal_Int8 nType = 0;
    OUString aFilterName;
    css::uno::Reference< css::lang::XComponent > xDrawing;
    css::uno::Reference< css::graphic::XGraphic > xGraphic;

    aFilterName = GetFilterName();

    switch( mpCurTheme->GetObjectKind( mnCurActionPos ) )
    {
        case SgaObjKind::Bitmap:
        case SgaObjKind::Animation:
        case SgaObjKind::Inet:
        // TODO drawing objects are inserted as drawings only via drag&drop
        case SgaObjKind::SvDraw:
            nType = css::gallery::GalleryItemType::GRAPHIC;
        break;

        case SgaObjKind::Sound :
            nType = css::gallery::GalleryItemType::MEDIA;
        break;

        default:
            nType = css::gallery::GalleryItemType::EMPTY;
        break;
    }

    Graphic aGraphic;
    bool bGraphic = mpCurTheme->GetGraphic( mnCurActionPos, aGraphic );
    if ( bGraphic && !aGraphic.IsNone() )
        xGraphic.set( aGraphic.GetXGraphic() );
    OSL_ENSURE( xGraphic.is(), "gallery item is graphic, but the reference is invalid!" );

    css::uno::Sequence< css::beans::PropertyValue > aSeq( SVXGALLERYITEM_PARAMS );

    aSeq[0].Name = SVXGALLERYITEM_TYPE;
    aSeq[0].Value <<= nType;
    aSeq[1].Name = SVXGALLERYITEM_URL;
    aSeq[1].Value <<= OUString();
    aSeq[2].Name = SVXGALLERYITEM_FILTER;
    aSeq[2].Value <<= aFilterName;
    aSeq[3].Name = SVXGALLERYITEM_DRAWING;
    aSeq[3].Value <<= xDrawing;
    aSeq[4].Name = SVXGALLERYITEM_GRAPHIC;
    aSeq[4].Value <<= xGraphic;

    css::uno::Sequence< css::beans::PropertyValue > aArgs( 1 );
    aArgs[0].Name = SVXGALLERYITEM_ARGNAME;
    aArgs[0].Value <<= aSeq;

    std::unique_ptr<DispatchInfo> pInfo(new DispatchInfo);
    pInfo->TargetURL = aURL;
    pInfo->Arguments = aArgs;
    pInfo->Dispatch = xDispatch;

    if ( Application::PostUserEvent(
            LINK( nullptr, GalleryBrowser2, AsyncDispatch_Impl), pInfo.get() ) )
        pInfo.release();
}

void GalleryBrowser2::Execute(std::string_view rIdent)
{
    Point       aSelPos;
    const sal_uInt32 nItemId = ImplGetSelectedItemId( nullptr, aSelPos );

    if( !(mpCurTheme && nItemId) )
        return;

    mnCurActionPos = nItemId - 1;

    if (rIdent == "preview")
        SetMode( ( GALLERYBROWSERMODE_PREVIEW != GetMode() ) ? GALLERYBROWSERMODE_PREVIEW : meLastMode );
    else if (rIdent == "delete")
    {
        if (!mpCurTheme->IsReadOnly())
        {
            std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetViewWindow(), "svx/ui/querydeleteobjectdialog.ui"));
            std::unique_ptr<weld::MessageDialog> xQuery(xBuilder->weld_message_dialog("QueryDeleteObjectDialog"));
            if (xQuery->run() == RET_YES)
            {
                mpCurTheme->RemoveObject( mnCurActionPos );
            }
        }
    }
    else if (rIdent == "title")
    {
        std::unique_ptr<SgaObject> pObj = mpCurTheme->AcquireObject( mnCurActionPos );

        if( pObj )
        {
            const OUString  aOldTitle( GetItemText( *pObj, GalleryItemFlags::Title ) );

            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            ScopedVclPtr<AbstractTitleDialog> aDlg(pFact->CreateTitleDialog(GetViewWindow(), aOldTitle));
            if( aDlg->Execute() == RET_OK )
            {
                OUString aNewTitle( aDlg->GetTitle() );

                if( ( aNewTitle.isEmpty() && !pObj->GetTitle().isEmpty() ) || ( aNewTitle != aOldTitle ) )
                {
                    if( aNewTitle.isEmpty() )
                        aNewTitle = "__<empty>__";

                    pObj->SetTitle( aNewTitle );
                    mpCurTheme->InsertObject( *pObj );
                }
            }
        }
    }
    else if (rIdent == "copy")
    {
        mpCurTheme->CopyToClipboard(mnCurActionPos);
    }
    else if (rIdent == "paste")
    {
        if( !mpCurTheme->IsReadOnly() )
        {
            TransferableDataHelper aDataHelper(TransferableDataHelper::CreateFromClipboard(GetSystemClipboard()));
            mpCurTheme->InsertTransferable( aDataHelper.GetTransferable(), mnCurActionPos );
         }
    }
}

OUString GalleryBrowser2::GetItemText( const SgaObject& rObj, GalleryItemFlags nItemTextFlags )
{
    OUString          aRet;

    const INetURLObject& aURL(rObj.GetURL());

    if( nItemTextFlags & GalleryItemFlags::Title )
    {
        OUString aTitle( rObj.GetTitle() );

        if( aTitle.isEmpty() )
            aTitle = aURL.getBase( INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::Unambiguous );

        if( aTitle.isEmpty() )
        {
            aTitle = aURL.GetMainURL( INetURLObject::DecodeMechanism::Unambiguous );
            aTitle = aTitle.copy( aTitle.lastIndexOf('/')+1 );
        }

        aRet += aTitle;
    }

    if( nItemTextFlags & GalleryItemFlags::Path )
    {
        const OUString aPath( aURL.getFSysPath( FSysStyle::Detect ) );

        if( !aPath.isEmpty() && ( nItemTextFlags & GalleryItemFlags::Title ) )
            aRet += " (";

        aRet += aURL.getFSysPath( FSysStyle::Detect );

        if( !aPath.isEmpty() && ( nItemTextFlags & GalleryItemFlags::Title ) )
            aRet += ")";
    }

    return aRet;
}

INetURLObject GalleryBrowser2::GetURL() const
{
    INetURLObject aURL;

    if( mpCurTheme && mnCurActionPos != 0xffffffff )
        aURL = mpCurTheme->GetObjectURL( mnCurActionPos );

    return aURL;
}

OUString GalleryBrowser2::GetFilterName() const
{
    OUString aFilterName;

    if( mpCurTheme && mnCurActionPos != 0xffffffff )
    {
        const SgaObjKind eObjKind = mpCurTheme->GetObjectKind( mnCurActionPos );

        if( ( SgaObjKind::Bitmap == eObjKind ) || ( SgaObjKind::Animation == eObjKind ) )
        {
            GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
            INetURLObject       aURL;
            mpCurTheme->GetURL( mnCurActionPos, aURL );
            sal_uInt16 nFilter = rFilter.GetImportFormatNumberForShortName(aURL.GetFileExtension());

            if( GRFILTER_FORMAT_DONTKNOW != nFilter )
                aFilterName = rFilter.GetImportFormatName( nFilter );
        }
    }

    return aFilterName;
}

IMPL_LINK_NOARG(GalleryBrowser2, SelectObjectValueSetHdl, ValueSet*, void)
{
    ImplUpdateSelection();
}

IMPL_LINK_NOARG(GalleryBrowser2, SelectObjectHdl, weld::TreeView&, void)
{
    ImplUpdateSelection();
}

IMPL_LINK(GalleryBrowser2, SelectTbxHdl, weld::ToggleButton&, rBox, void)
{
    if (&rBox == mxIconButton.get())
        SetMode(rBox.get_active() ? GALLERYBROWSERMODE_ICON : GALLERYBROWSERMODE_LIST);
    else if (&rBox == mxListButton.get())
        SetMode(rBox.get_active() ? GALLERYBROWSERMODE_LIST : GALLERYBROWSERMODE_ICON);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
