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


#include <comphelper/string.hxx>
#include <sot/formats.hxx>
#include <vcl/msgbox.hxx>
#include <svl/urlbmk.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <svtools/transfer.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/sfxsids.hrc>
#include <vcl/graphicfilter.hxx>
#include <editeng/brushitem.hxx>
#include "helpid.hrc"
#include "svx/gallery.hxx"
#include "galobj.hxx"
#include "svx/gallery1.hxx"
#include "svx/galtheme.hxx"
#include "svx/galctrl.hxx"
#include "svx/galmisc.hxx"
#include "galbrws2.hxx"
#include "gallery.hrc"
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <svx/fmmodel.hxx>
#include <svx/dialmgr.hxx>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include "GalleryControl.hxx"

#include <svx/svxcommands.h>
#include <svx/galleryitem.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/gallery/GalleryItemType.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/style/GraphicLocation.hpp>
#include <map>
#include <memory>
#include <cppuhelper/implbase.hxx>

#undef GALLERY_USE_CLIPBOARD

#define TBX_ID_ICON 1
#define TBX_ID_LIST 2

GalleryBrowserMode GalleryBrowser2::meInitMode = GALLERYBROWSERMODE_ICON;

struct DispatchInfo
{
    css::util::URL                                  TargetURL;
    css::uno::Sequence< css::beans::PropertyValue > Arguments;
    css::uno::Reference< css::frame::XDispatch >    Dispatch;
};

IMPL_STATIC_LINK_TYPED( GalleryBrowser2, AsyncDispatch_Impl, void*, p, void )
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
    sal_uIntPtr         mnObjectPos;
    bool                mbPreview;
    ScopedVclPtr<PopupMenu> mpPopupMenu;
    ScopedVclPtr<PopupMenu> mpBackgroundPopup;
    VclPtr<GalleryBrowser2> mpBrowser;

    typedef std::map< int, CommandInfo > CommandInfoMap;
    CommandInfoMap   m_aCommandInfo;

    static void Execute( const CommandInfo &rCmdInfo,
                  const css::uno::Sequence< css::beans::PropertyValue > &rArguments );

    DECL_LINK_TYPED( MenuSelectHdl, Menu*, bool );
    DECL_LINK_TYPED( BackgroundMenuSelectHdl, Menu*, bool );
public:
    GalleryThemePopup( const GalleryTheme* pTheme,
                       sal_uIntPtr nObjectPos,
                       bool bPreview,
                       GalleryBrowser2* pBrowser );
    virtual ~GalleryThemePopup();

    void ExecutePopup( vcl::Window *pParent, const ::Point &aPos );

    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent &rEvent) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL disposing( const css::lang::EventObject &rSource) throw ( css::uno::RuntimeException, std::exception ) override;
};


GalleryThemePopup::GalleryThemePopup(
    const GalleryTheme* pTheme,
    sal_uIntPtr nObjectPos,
    bool bPreview,
    GalleryBrowser2* pBrowser )
    : mpTheme( pTheme )
    , mnObjectPos( nObjectPos )
    , mbPreview( bPreview )
    , mpPopupMenu( VclPtr<PopupMenu>::Create(GAL_RES( RID_SVXMN_GALLERY2 )) )
    , mpBrowser( pBrowser )
{

    // SID_GALLERY_ENABLE_ADDCOPY
    m_aCommandInfo.insert(
        CommandInfoMap::value_type(
            SID_GALLERY_ENABLE_ADDCOPY,
            CommandInfo( OUString(CMD_SID_GALLERY_ENABLE_ADDCOPY ))));
    // SID_GALLERY_BG_BRUSH
    m_aCommandInfo.insert(
        CommandInfoMap::value_type(
            SID_GALLERY_BG_BRUSH,
            CommandInfo(OUString(CMD_SID_GALLERY_BG_BRUSH ))));
    // SID_GALLERY_FORMATS
    m_aCommandInfo.insert(
        CommandInfoMap::value_type(
            SID_GALLERY_FORMATS,
            CommandInfo(OUString(CMD_SID_GALLERY_FORMATS ))));

}

GalleryThemePopup::~GalleryThemePopup()
{
}

void SAL_CALL GalleryThemePopup::statusChanged(
    const css::frame::FeatureStateEvent &rEvent )
throw ( css::uno::RuntimeException, std::exception )
{
    const OUString &rURL = rEvent.FeatureURL.Complete;
    if ( rURL == CMD_SID_GALLERY_ENABLE_ADDCOPY )
    {
        if ( !rEvent.IsEnabled )
        {
            mpPopupMenu->EnableItem( MN_ADD, false );
        }
    }
    else if ( rURL == CMD_SID_GALLERY_BG_BRUSH )
    {
        mpBackgroundPopup->Clear();
        if ( rEvent.IsEnabled )
        {
            OUString sItem;
            css::uno::Sequence< OUString > sItems;
            if ( ( rEvent.State >>= sItem ) && sItem.getLength() )
            {
                mpBackgroundPopup->InsertItem( 1, sItem );
            }
            else if ( ( rEvent.State >>= sItems ) && sItems.getLength() )
            {
                const OUString *pStr = sItems.getConstArray();
                const OUString *pEnd = pStr + sItems.getLength();
                for ( sal_uInt16 nId = 1; pStr != pEnd; pStr++, nId++ )
                {
                    mpBackgroundPopup->InsertItem( nId, *pStr );
                }
            }
        }
    }
}

void SAL_CALL GalleryThemePopup::disposing(
    const css::lang::EventObject &/*rSource*/)
throw ( css::uno::RuntimeException, std::exception )
{
}

void GalleryThemePopup::Execute(
    const CommandInfo &rCmdInfo,
    const css::uno::Sequence< css::beans::PropertyValue > &rArguments )
{
    if ( rCmdInfo.Dispatch.is() )
    {
        DispatchInfo *pInfo = new DispatchInfo;
        pInfo->TargetURL = rCmdInfo.URL;
        pInfo->Arguments = rArguments;
        pInfo->Dispatch = rCmdInfo.Dispatch;

        if ( !Application::PostUserEvent(
                LINK( nullptr, GalleryBrowser2, AsyncDispatch_Impl), pInfo ) )
            delete pInfo;
    }
}

void GalleryThemePopup::ExecutePopup( vcl::Window *pWindow, const ::Point &aPos )
{
    css::uno::Reference< css::frame::XStatusListener > xThis( this );

    const SgaObjKind eObjKind = mpTheme->GetObjectKind( mnObjectPos );
    INetURLObject    aURL;

    const_cast< GalleryTheme* >( mpTheme )->GetURL( mnObjectPos, aURL );
    const bool bValidURL = ( aURL.GetProtocol() != INetProtocol::NotValid );

    mpPopupMenu->EnableItem( MN_ADD, bValidURL && SGA_OBJ_SOUND != eObjKind );

    mpPopupMenu->EnableItem( MN_PREVIEW, bValidURL );

    mpPopupMenu->CheckItem( MN_PREVIEW, mbPreview );

    if( mpTheme->IsReadOnly() || !mpTheme->GetObjectCount() )
    {
        mpPopupMenu->EnableItem( MN_DELETE, false );
        mpPopupMenu->EnableItem( MN_TITLE, false );

        if( mpTheme->IsReadOnly() )
            mpPopupMenu->EnableItem( MN_PASTECLIPBOARD, false );

        if( !mpTheme->GetObjectCount() )
            mpPopupMenu->EnableItem( MN_COPYCLIPBOARD, false );
    }
    else
    {
        mpPopupMenu->EnableItem( MN_DELETE, !mbPreview );
        mpPopupMenu->EnableItem( MN_TITLE );
        mpPopupMenu->EnableItem( MN_COPYCLIPBOARD );
        mpPopupMenu->EnableItem( MN_PASTECLIPBOARD );
    }

    mpPopupMenu->EnableItem( MN_COPYCLIPBOARD, false );
    mpPopupMenu->EnableItem( MN_PASTECLIPBOARD, false );

    // update status
    css::uno::Reference< css::frame::XDispatchProvider> xDispatchProvider(
        GalleryBrowser2::GetFrame(), css::uno::UNO_QUERY );
    css::uno::Reference< css::util::XURLTransformer > xTransformer(
        mpBrowser->GetURLTransformer() );
    CommandInfoMap::const_iterator aEnd = m_aCommandInfo.end();
    for ( CommandInfoMap::iterator it = m_aCommandInfo.begin();
         it != aEnd; ++it )
    {
        try
        {
            CommandInfo &rCmdInfo = it->second;
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

    if( !mpBackgroundPopup->GetItemCount() || ( eObjKind == SGA_OBJ_SVDRAW ) || ( eObjKind == SGA_OBJ_SOUND ) )
        mpPopupMenu->EnableItem( MN_BACKGROUND, false );
    else
    {
        mpPopupMenu->EnableItem( MN_BACKGROUND );
        mpPopupMenu->SetPopupMenu( MN_BACKGROUND, mpBackgroundPopup );
        mpBackgroundPopup->SetSelectHdl( LINK( this, GalleryThemePopup, BackgroundMenuSelectHdl ) );
    }

    mpPopupMenu->RemoveDisabledEntries();

    mpPopupMenu->SetSelectHdl( LINK( this, GalleryThemePopup, MenuSelectHdl ) );
    mpPopupMenu->Execute( pWindow, aPos );
}

IMPL_LINK_TYPED( GalleryThemePopup, MenuSelectHdl, Menu*, pMenu, bool )
{
    if( !pMenu )
        return false;

    sal_uInt16 nId( pMenu->GetCurItemId() );
    switch ( nId )
    {
        case MN_ADD:
        {
            const CommandInfoMap::const_iterator it = m_aCommandInfo.find( SID_GALLERY_FORMATS );
            if ( it != m_aCommandInfo.end() )
                mpBrowser->Dispatch( nId,
                                     it->second.Dispatch,
                                     it->second.URL );
        }
        break;

        default:
            mpBrowser->Execute( nId );
    }

    return false;
}

IMPL_LINK_TYPED( GalleryThemePopup, BackgroundMenuSelectHdl, Menu*, pMenu, bool )
{
    if( !pMenu )
        return false;

    sal_uInt16 nPos( pMenu->GetCurItemId() - 1 );
    OUString aURL( mpBrowser->GetURL().GetMainURL( INetURLObject::NO_DECODE ) );
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

    return false;
}

} // end anonymous namespace


GalleryToolBox::GalleryToolBox( GalleryBrowser2* pParent ) :
    ToolBox( pParent, WB_TABSTOP )
{
}

void GalleryToolBox::KeyInput( const KeyEvent& rKEvt )
{
    if( !static_cast< GalleryBrowser2* >( GetParent() )->KeyInput( rKEvt, this ) )
        ToolBox::KeyInput( rKEvt );
}


GalleryBrowser2::GalleryBrowser2( vcl::Window* pParent, Gallery* pGallery ) :
    Control             ( pParent, WB_TABSTOP ),
    mpGallery           ( pGallery ),
    mpCurTheme          ( nullptr ),
    mpIconView          ( VclPtr<GalleryIconView>::Create( this, nullptr ) ),
    mpListView          ( VclPtr<GalleryListView>::Create( this, nullptr ) ),
    mpPreview           ( VclPtr<GalleryPreview>::Create(this) ),
    maViewBox           ( VclPtr<GalleryToolBox>::Create(this) ),
    maSeparator         ( VclPtr<FixedLine>::Create(this, WB_VERT) ),
    maInfoBar           ( VclPtr<FixedText>::Create(this, WB_LEFT | WB_VCENTER) ),
    mnCurActionPos      ( 0xffffffff ),
    meMode              ( GALLERYBROWSERMODE_NONE ),
    meLastMode          ( GALLERYBROWSERMODE_NONE )
{

    m_xContext.set( ::comphelper::getProcessComponentContext() );

    m_xTransformer.set(
        m_xContext->getServiceManager()->createInstanceWithContext(
            "com.sun.star.util.URLTransformer", m_xContext ),
        css::uno::UNO_QUERY );

    Image      aDummyImage;
    vcl::Font  aInfoFont( maInfoBar->GetControlFont() );

    maMiscOptions.AddListenerLink( LINK( this, GalleryBrowser2, MiscHdl ) );

    maViewBox->InsertItem( TBX_ID_ICON, aDummyImage );
    maViewBox->SetItemBits( TBX_ID_ICON, ToolBoxItemBits::RADIOCHECK | ToolBoxItemBits::AUTOCHECK );
    maViewBox->SetHelpId( TBX_ID_ICON, HID_GALLERY_ICONVIEW );
    maViewBox->SetQuickHelpText( TBX_ID_ICON, GAL_RESSTR(RID_SVXSTR_GALLERY_ICONVIEW) );

    maViewBox->InsertItem( TBX_ID_LIST, aDummyImage );
    maViewBox->SetItemBits( TBX_ID_LIST, ToolBoxItemBits::RADIOCHECK | ToolBoxItemBits::AUTOCHECK );
    maViewBox->SetHelpId( TBX_ID_LIST, HID_GALLERY_LISTVIEW );
    maViewBox->SetQuickHelpText( TBX_ID_LIST, GAL_RESSTR(RID_SVXSTR_GALLERY_LISTVIEW) );

    MiscHdl( nullptr );
    maViewBox->SetSelectHdl( LINK( this, GalleryBrowser2, SelectTbxHdl ) );
    maViewBox->Show();

    mpIconView->SetAccessibleName(SVX_RESSTR(RID_SVXSTR_GALLERY_THEMEITEMS));
    mpListView->SetAccessibleName(SVX_RESSTR(RID_SVXSTR_GALLERY_THEMEITEMS));

    maInfoBar->Show();
    maSeparator->Show();

    mpIconView->SetSelectHdl( LINK( this, GalleryBrowser2, SelectObjectValueSetHdl ) );
    mpListView->SetSelectHdl( LINK( this, GalleryBrowser2, SelectObjectHdl ) );

    InitSettings();

    SetMode( ( GALLERYBROWSERMODE_PREVIEW != GalleryBrowser2::meInitMode ) ? GalleryBrowser2::meInitMode : GALLERYBROWSERMODE_ICON );

    if(maInfoBar->GetText().isEmpty())
        mpIconView->SetAccessibleRelationLabeledBy(mpIconView);
    else
        mpIconView->SetAccessibleRelationLabeledBy(maInfoBar.get());
}

GalleryBrowser2::~GalleryBrowser2()
{
    disposeOnce();
}

void GalleryBrowser2::dispose()
{
    maMiscOptions.RemoveListenerLink( LINK( this, GalleryBrowser2, MiscHdl ) );

    mpPreview.disposeAndClear();
    mpListView.disposeAndClear();
    mpIconView.disposeAndClear();

    if( mpCurTheme )
        mpGallery->ReleaseTheme( mpCurTheme, *this );
    maSeparator.disposeAndClear();
    maInfoBar.disposeAndClear();
    maViewBox.disposeAndClear();
    Control::dispose();
}

void GalleryBrowser2::InitSettings()
{
    vcl::Font  aInfoFont( maInfoBar->GetControlFont() );

    aInfoFont.SetWeight( WEIGHT_BOLD );
    aInfoFont.SetColor( GALLERY_FG_COLOR );
    maInfoBar->SetControlFont( aInfoFont );

    maInfoBar->SetBackground( Wallpaper( GALLERY_DLG_COLOR ) );
    maInfoBar->SetControlBackground( GALLERY_DLG_COLOR );

    maSeparator->SetBackground( Wallpaper( GALLERY_BG_COLOR ) );
    maSeparator->SetControlBackground( GALLERY_BG_COLOR );
    maSeparator->SetControlForeground( GALLERY_FG_COLOR );
}

void GalleryBrowser2::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( ( rDCEvt.GetType() == DataChangedEventType::SETTINGS ) && ( rDCEvt.GetFlags() & AllSettingsFlags::STYLE ) )
        InitSettings();
    else
        Control::DataChanged( rDCEvt );
}

void GalleryBrowser2::Resize()
{
    Control::Resize();

    mpIconView->Hide();
    mpListView->Hide();
    mpPreview->Hide();

    const Size  aOutSize( GetOutputSizePixel() );
    const Size  aBoxSize( maViewBox->GetOutputSizePixel() );
    const long  nOffset = 2, nSepWidth = 2;
    const long  nInfoBarX = aBoxSize.Width() + ( nOffset * 3 ) + nSepWidth;
    const Point aPt( 0, aBoxSize.Height() + 3 );
    const Size  aSz( aOutSize.Width(), aOutSize.Height() - aPt.Y() );

    maSeparator->SetPosSizePixel( Point( aBoxSize.Width() + nOffset, 0 ), Size( nSepWidth, aBoxSize.Height() ) );
    maInfoBar->SetPosSizePixel( Point( nInfoBarX, 0 ), Size( aOutSize.Width() - nInfoBarX, aBoxSize.Height() ) );

    mpIconView->SetPosSizePixel( aPt, aSz );
    mpListView->SetPosSizePixel( aPt, aSz );
    mpPreview->SetPosSizePixel( aPt, aSz );

    switch( GetMode() )
    {
        case GALLERYBROWSERMODE_ICON: mpIconView->Show(); break;
        case GALLERYBROWSERMODE_LIST: mpListView->Show(); break;
        case GALLERYBROWSERMODE_PREVIEW: mpPreview->Show(); break;

        default:
        break;
    }
}

void GalleryBrowser2::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    const GalleryHint& rGalleryHint = static_cast<const GalleryHint&>(rHint);

    switch( rGalleryHint.GetType() )
    {
        case( GalleryHintType::THEME_UPDATEVIEW ):
        {
            if( GALLERYBROWSERMODE_PREVIEW == GetMode() )
                SetMode( meLastMode );

            ImplUpdateViews( (sal_uInt16) rGalleryHint.GetData1() + 1 );
        }
        break;

        default:
        break;
    }
}

sal_Int8 GalleryBrowser2::AcceptDrop( DropTargetHelper& rTarget, const AcceptDropEvent& )
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

sal_Int8 GalleryBrowser2::ExecuteDrop( DropTargetHelper&, const ExecuteDropEvent& rEvt )
{
    sal_Int8 nRet = DND_ACTION_NONE;

    if( mpCurTheme )
    {
        Point       aSelPos;
        const sal_uIntPtr nItemId = ImplGetSelectedItemId( &rEvt.maPosPixel, aSelPos );
        const sal_uIntPtr nInsertPos = (nItemId ? (nItemId - 1) : mpCurTheme->GetObjectCount());

        if( mpCurTheme->IsDragging() )
            mpCurTheme->ChangeObjectPos( mpCurTheme->GetDragPos(), nInsertPos );
        else
            nRet = mpCurTheme->InsertTransferable( rEvt.maDropEvent.Transferable, nInsertPos ) ? 1 : 0;
    }

    return nRet;
}

void GalleryBrowser2::StartDrag( vcl::Window*, const Point* pDragPoint )
{
    if( mpCurTheme )
    {
        Point       aSelPos;
        const sal_uIntPtr nItemId = ImplGetSelectedItemId( pDragPoint, aSelPos );

        if( nItemId )
            mpCurTheme->StartDrag( this, nItemId - 1 );
    }
}

void GalleryBrowser2::TogglePreview( vcl::Window*, const Point* )
{
    SetMode( ( GALLERYBROWSERMODE_PREVIEW != GetMode() ) ? GALLERYBROWSERMODE_PREVIEW : meLastMode );
    GetViewWindow()->GrabFocus();
}

void GalleryBrowser2::ShowContextMenu( vcl::Window*, const Point* pContextPoint )
{
    Point aSelPos;
    const sal_uIntPtr nItemId = ImplGetSelectedItemId( pContextPoint, aSelPos );

    if( mpCurTheme && nItemId && ( nItemId <= mpCurTheme->GetObjectCount() ) )
    {
        ImplSelectItemId( nItemId );

        css::uno::Reference< css::frame::XFrame > xFrame( GetFrame() );
        if ( xFrame.is() )
        {
            mnCurActionPos = nItemId - 1;
            rtl::Reference< GalleryThemePopup > rPopup(
                new GalleryThemePopup(
                    mpCurTheme,
                    mnCurActionPos,
                    GALLERYBROWSERMODE_PREVIEW == GetMode(),
                    this ) );
            rPopup->ExecutePopup( this, aSelPos  );
        }
    }
}

bool GalleryBrowser2::KeyInput( const KeyEvent& rKEvt, vcl::Window* pWindow )
{
    Point       aSelPos;
    const sal_uIntPtr   nItemId = ImplGetSelectedItemId( nullptr, aSelPos );
    bool bRet = false;
    svx::sidebar::GalleryControl* pParentControl = dynamic_cast<svx::sidebar::GalleryControl*>(GetParent());
    if (pParentControl != nullptr)
        bRet = pParentControl->GalleryKeyInput(rKEvt, pWindow);

    if( !bRet && !maViewBox->HasFocus() && nItemId && mpCurTheme )
    {
        sal_uInt16          nExecuteId = 0;
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
                    TogglePreview( pWindow );
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
                    Dispatch( MN_ADD );
                    return true;
                }
            }
            break;

            case KEY_DELETE:
            case KEY_D:
            {
                if( bDelete )
                    nExecuteId = MN_DELETE;
            }
            break;

            case KEY_T:
            {
                if( bTitle )
                    nExecuteId = MN_TITLE;
            }
            break;

            default:
            break;
        }

        if( nExecuteId )
        {
            Execute( nExecuteId );
            bRet = true;
        }
    }

    return bRet;
}

void GalleryBrowser2::SelectTheme( const OUString& rThemeName )
{
    mpIconView.disposeAndClear();
    mpListView.disposeAndClear();
    mpPreview.disposeAndClear();

    if( mpCurTheme )
        mpGallery->ReleaseTheme( mpCurTheme, *this );

    mpCurTheme = mpGallery->AcquireTheme( rThemeName, *this );

    mpIconView = VclPtr<GalleryIconView>::Create( this, mpCurTheme );
    mpListView = VclPtr<GalleryListView>::Create( this, mpCurTheme );
    mpPreview = VclPtr<GalleryPreview>::Create( this, WB_TABSTOP | WB_BORDER, mpCurTheme );

    mpIconView->SetAccessibleName(SVX_RESSTR(RID_SVXSTR_GALLERY_THEMEITEMS));
    mpListView->SetAccessibleName(SVX_RESSTR(RID_SVXSTR_GALLERY_THEMEITEMS));
    mpPreview->SetAccessibleName(SVX_RESSTR(RID_SVXSTR_GALLERY_PREVIEW));

    mpIconView->SetSelectHdl( LINK( this, GalleryBrowser2, SelectObjectValueSetHdl ) );
    mpListView->SetSelectHdl( LINK( this, GalleryBrowser2, SelectObjectHdl ) );

    if( GALLERYBROWSERMODE_PREVIEW == GetMode() )
        meMode = meLastMode;

    Resize();
    ImplUpdateViews( 1 );

    maViewBox->EnableItem( TBX_ID_ICON );
    maViewBox->EnableItem( TBX_ID_LIST );
    maViewBox->CheckItem( ( GALLERYBROWSERMODE_ICON == GetMode() ) ? TBX_ID_ICON : TBX_ID_LIST );

    if(maInfoBar->GetText().isEmpty())
        mpIconView->SetAccessibleRelationLabeledBy(mpIconView);
    else
        mpIconView->SetAccessibleRelationLabeledBy(maInfoBar.get());
}

void GalleryBrowser2::SetMode( GalleryBrowserMode eMode )
{
    if( GetMode() != eMode )
    {
        meLastMode = GetMode();

        switch( eMode )
        {
            case GALLERYBROWSERMODE_ICON:
            {
                mpListView->Hide();

                mpPreview->Hide();
                mpPreview->SetGraphic( Graphic() );
                GalleryPreview::PreviewMedia( INetURLObject() );

                mpIconView->Show();

                maViewBox->EnableItem( TBX_ID_ICON );
                maViewBox->EnableItem( TBX_ID_LIST );

                maViewBox->CheckItem( TBX_ID_ICON );
                maViewBox->CheckItem( TBX_ID_LIST, false );
            }
            break;

            case GALLERYBROWSERMODE_LIST:
            {
                mpIconView->Hide();

                mpPreview->Hide();
                mpPreview->SetGraphic( Graphic() );
                GalleryPreview::PreviewMedia( INetURLObject() );

                mpListView->Show();

                maViewBox->EnableItem( TBX_ID_ICON );
                maViewBox->EnableItem( TBX_ID_LIST );

                maViewBox->CheckItem( TBX_ID_ICON, false );
                maViewBox->CheckItem( TBX_ID_LIST );
            }
            break;

            case GALLERYBROWSERMODE_PREVIEW:
            {
                Graphic     aGraphic;
                Point       aSelPos;
                const sal_uIntPtr   nItemId = ImplGetSelectedItemId( nullptr, aSelPos );

                if( nItemId )
                {
                    const sal_uIntPtr nPos = nItemId - 1;

                       mpIconView->Hide();
                    mpListView->Hide();

                    if( mpCurTheme )
                        mpCurTheme->GetGraphic( nPos, aGraphic );

                    mpPreview->SetGraphic( aGraphic );
                     mpPreview->Show();

                    if( mpCurTheme && mpCurTheme->GetObjectKind( nPos ) == SGA_OBJ_SOUND )
                        GalleryPreview::PreviewMedia( mpCurTheme->GetObjectURL( nPos ) );

                    maViewBox->EnableItem( TBX_ID_ICON, false );
                    maViewBox->EnableItem( TBX_ID_LIST, false );
                }
            }
            break;

            default:
                break;
        }

        GalleryBrowser2::meInitMode = meMode = eMode;
    }
}

vcl::Window* GalleryBrowser2::GetViewWindow() const
{
    vcl::Window* pRet;

    switch( GetMode() )
    {
        case GALLERYBROWSERMODE_LIST: pRet = mpListView; break;
        case GALLERYBROWSERMODE_PREVIEW: pRet = mpPreview; break;

        default:
            pRet = mpIconView;
        break;
    }

    return pRet;
}

void GalleryBrowser2::Travel( GalleryBrowserTravel eTravel )
{
    if( mpCurTheme )
    {
        Point       aSelPos;
        const sal_uIntPtr nItemId = ImplGetSelectedItemId( nullptr, aSelPos );

        if( nItemId )
        {
            sal_uIntPtr nNewItemId = nItemId;

            switch( eTravel )
            {
                case GALLERYBROWSERTRAVEL_FIRST:     nNewItemId = 1; break;
                case GALLERYBROWSERTRAVEL_LAST:      nNewItemId = mpCurTheme->GetObjectCount(); break;
                case GALLERYBROWSERTRAVEL_PREVIOUS:  nNewItemId--; break;
                case GALLERYBROWSERTRAVEL_NEXT:      nNewItemId++; break;
                default:
                    break;
            }

            if( nNewItemId < 1 )
                nNewItemId = 1;
            else if( nNewItemId > mpCurTheme->GetObjectCount() )
                nNewItemId = mpCurTheme->GetObjectCount();

            if( nNewItemId != nItemId )
            {
                ImplSelectItemId( nNewItemId );
                ImplUpdateInfoBar();

                if( GALLERYBROWSERMODE_PREVIEW == GetMode() )
                {
                    Graphic     aGraphic;
                    const sal_uIntPtr nPos = nNewItemId - 1;

                    mpCurTheme->GetGraphic( nPos, aGraphic );
                    mpPreview->SetGraphic( aGraphic );

                    if( SGA_OBJ_SOUND == mpCurTheme->GetObjectKind( nPos ) )
                        GalleryPreview::PreviewMedia( mpCurTheme->GetObjectURL( nPos ) );

                    mpPreview->Invalidate();
                }
            }
        }
    }
}

void GalleryBrowser2::ImplUpdateViews( sal_uInt16 nSelectionId )
{
    mpIconView->Hide();
    mpListView->Hide();
    mpPreview->Hide();

    mpIconView->Clear();
    mpListView->Clear();

    if( mpCurTheme )
    {
        for( sal_uIntPtr i = 0, nCount = mpCurTheme->GetObjectCount(); i < nCount; )
        {
            mpListView->RowInserted( i++ );
            mpIconView->InsertItem( (sal_uInt16) i );
        }

        ImplSelectItemId( ( ( nSelectionId > mpCurTheme->GetObjectCount() ) ? mpCurTheme->GetObjectCount() : nSelectionId ) );
    }

    switch( GetMode() )
    {
        case GALLERYBROWSERMODE_ICON: mpIconView->Show(); break;
        case GALLERYBROWSERMODE_LIST: mpListView->Show(); break;
        case GALLERYBROWSERMODE_PREVIEW: mpPreview->Show(); break;

        default:
        break;
    }

    ImplUpdateInfoBar();
}

void GalleryBrowser2::ImplUpdateInfoBar()
{
    if( mpCurTheme )
         maInfoBar->SetText( mpCurTheme->GetName() );
}

sal_uIntPtr GalleryBrowser2::ImplGetSelectedItemId( const Point* pSelPos, Point& rSelPos )
{
    const Size  aOutputSizePixel( GetOutputSizePixel() );
    sal_uIntPtr       nRet = 0;

    if( GALLERYBROWSERMODE_PREVIEW == GetMode() )
    {
        nRet = ( ( GALLERYBROWSERMODE_ICON == meLastMode ) ? mpIconView->GetSelectItemId() : ( mpListView->FirstSelectedRow() + 1 ) );

        if( pSelPos )
            rSelPos = GetPointerPosPixel();
        else
            rSelPos = Point( aOutputSizePixel.Width() >> 1, aOutputSizePixel.Height() >> 1 );
    }
    else if( GALLERYBROWSERMODE_ICON == GetMode() )
    {
        if( pSelPos )
        {
            nRet = mpIconView->GetItemId( *pSelPos );
            rSelPos = GetPointerPosPixel();
        }
        else
        {
            nRet = mpIconView->GetSelectItemId();
            rSelPos = mpIconView->GetItemRect( (sal_uInt16) nRet ).Center();
        }
    }
    else
    {
        if( pSelPos )
        {
            nRet = mpListView->GetRowAtYPosPixel( pSelPos->Y() ) + 1;
            rSelPos = GetPointerPosPixel();
        }
        else
        {
            nRet = mpListView->FirstSelectedRow() + 1;
            rSelPos = mpListView->GetFieldRectPixel( (sal_uInt16) nRet, 1 ).Center();
        }
    }

    rSelPos.X() = std::max( std::min( rSelPos.X(), aOutputSizePixel.Width() - 1L ), 0L );
    rSelPos.Y() = std::max( std::min( rSelPos.Y(), aOutputSizePixel.Height() - 1L ), 0L );

    if( nRet && ( !mpCurTheme || ( nRet > mpCurTheme->GetObjectCount() ) ) )
    {
        nRet = 0;
    }

    return nRet;
}

void GalleryBrowser2::ImplSelectItemId( sal_uIntPtr nItemId )
{
    if( nItemId )
    {

        mpIconView->SelectItem( (sal_uInt16) nItemId );
        mpListView->SelectRow( nItemId - 1 );
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

void GalleryBrowser2::Dispatch(
     sal_uInt16 nId,
    const css::uno::Reference< css::frame::XDispatch > &rxDispatch,
    const css::util::URL &rURL )
{
    Point aSelPos;
    const sal_uIntPtr nItemId = ImplGetSelectedItemId( nullptr, aSelPos );

    if( !mpCurTheme || !nItemId )
        return;

    mnCurActionPos = nItemId - 1;

    switch( nId )
    {
        case MN_ADD:
        {
            css::uno::Reference< css::frame::XDispatch > xDispatch( rxDispatch );
            css::util::URL aURL = rURL;

            if ( !xDispatch.is() )
            {
                css::uno::Reference< css::frame::XDispatchProvider > xDispatchProvider(
                    GetFrame(), css::uno::UNO_QUERY );
                if ( !xDispatchProvider.is() || !m_xTransformer.is() )
                    return;

                aURL.Complete = CMD_SID_GALLERY_FORMATS;
                m_xTransformer->parseStrict( aURL );
                xDispatch = xDispatchProvider->queryDispatch(
                    aURL,
                    "_self",
                    css::frame::FrameSearchFlag::SELF );
            }

            if ( !xDispatch.is() )
                return;

            sal_Int8 nType = 0;
            OUString aFileURL, aFilterName;
            css::uno::Reference< css::lang::XComponent > xDrawing;
            css::uno::Reference< css::graphic::XGraphic > xGraphic;

            aFilterName = GetFilterName();

            switch( mpCurTheme->GetObjectKind( mnCurActionPos ) )
            {
                case SGA_OBJ_BMP:
                case SGA_OBJ_ANIM:
                case SGA_OBJ_INET:
                // TODO drawing objects are inserted as drawings only via drag&drop
                case SGA_OBJ_SVDRAW:
                    nType = css::gallery::GalleryItemType::GRAPHIC;
                break;

                case SGA_OBJ_SOUND :
                    nType = css::gallery::GalleryItemType::MEDIA;
                break;

                default:
                    nType = css::gallery::GalleryItemType::EMPTY;
                break;
            }

            Graphic aGraphic;
            bool bGraphic = mpCurTheme->GetGraphic( mnCurActionPos, aGraphic );
            if ( bGraphic && !!aGraphic )
                xGraphic.set( aGraphic.GetXGraphic() );
            OSL_ENSURE( xGraphic.is(), "gallery item is graphic, but the reference is invalid!" );

            css::uno::Sequence< css::beans::PropertyValue > aSeq( SVXGALLERYITEM_PARAMS );

            aSeq[0].Name = SVXGALLERYITEM_TYPE;
            aSeq[0].Value <<= nType;
            aSeq[1].Name = SVXGALLERYITEM_URL;
            aSeq[1].Value <<= aFileURL;
            aSeq[2].Name = SVXGALLERYITEM_FILTER;
            aSeq[2].Value <<= aFilterName;
            aSeq[3].Name = SVXGALLERYITEM_DRAWING;
            aSeq[3].Value <<= xDrawing;
            aSeq[4].Name = SVXGALLERYITEM_GRAPHIC;
            aSeq[4].Value <<= xGraphic;

            css::uno::Sequence< css::beans::PropertyValue > aArgs( 1 );
            aArgs[0].Name = SVXGALLERYITEM_ARGNAME;
            aArgs[0].Value <<= aSeq;

            DispatchInfo *pInfo = new DispatchInfo;
            pInfo->TargetURL = aURL;
            pInfo->Arguments = aArgs;
            pInfo->Dispatch = xDispatch;

            if ( !Application::PostUserEvent(
                    LINK( nullptr, GalleryBrowser2, AsyncDispatch_Impl), pInfo ) )
                delete pInfo;
        }
        break;

        default:
        break;
    }
}

void GalleryBrowser2::Execute( sal_uInt16 nId )
{
    Point       aSelPos;
    const sal_uIntPtr nItemId = ImplGetSelectedItemId( nullptr, aSelPos );

    if( mpCurTheme && nItemId )
    {
        mnCurActionPos = nItemId - 1;

        switch( nId )
        {
            case MN_PREVIEW:
                SetMode( ( GALLERYBROWSERMODE_PREVIEW != GetMode() ) ? GALLERYBROWSERMODE_PREVIEW : meLastMode );
            break;

            case MN_DELETE:
            {
                if( !mpCurTheme->IsReadOnly() &&
                    ScopedVclPtrInstance<MessageDialog>(nullptr, "QueryDeleteObjectDialog","svx/ui/querydeleteobjectdialog.ui")->Execute() == RET_YES )
                {
                    mpCurTheme->RemoveObject( mnCurActionPos );
                }
            }
            break;

            case MN_TITLE:
            {
                SgaObject* pObj = mpCurTheme->AcquireObject( mnCurActionPos );

                if( pObj )
                {
                    const OUString  aOldTitle( GetItemText( *mpCurTheme, *pObj, GalleryItemFlags::Title ) );

                    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                    if(pFact)
                    {
                        std::unique_ptr<AbstractTitleDialog> aDlg(pFact->CreateTitleDialog( this, aOldTitle ));
                        DBG_ASSERT(aDlg, "Dialog creation failed!");
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

                        GalleryTheme::ReleaseObject( pObj );
                    }
                }
            }
            break;

            case MN_COPYCLIPBOARD:
            {
                vcl::Window* pWindow;

                switch( GetMode() )
                {
                    case GALLERYBROWSERMODE_ICON: pWindow = static_cast<vcl::Window*>(mpIconView); break;
                    case GALLERYBROWSERMODE_LIST: pWindow = static_cast<vcl::Window*>(mpListView); break;
                    case GALLERYBROWSERMODE_PREVIEW: pWindow = static_cast<vcl::Window*>(mpPreview); break;

                    default:
                        pWindow = nullptr;
                    break;
                }

                mpCurTheme->CopyToClipboard( pWindow, mnCurActionPos );
            }
            break;

            case MN_PASTECLIPBOARD:
            {
                if( !mpCurTheme->IsReadOnly() )
                {
                    TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( this ) );
                    mpCurTheme->InsertTransferable( aDataHelper.GetTransferable(), mnCurActionPos );
                }
            }
            break;

            default:
            break;
        }
    }
}

OUString GalleryBrowser2::GetItemText( const GalleryTheme& rTheme, const SgaObject& rObj, GalleryItemFlags nItemTextFlags )
{
    OUString          aRet;

    const INetURLObject& aURL(rObj.GetURL());

    if( nItemTextFlags & GalleryItemFlags::ThemeName )
    {
        aRet += rTheme.GetName();
        aRet += " - ";
    }

    if( nItemTextFlags & GalleryItemFlags::Title )
    {
        OUString aTitle( rObj.GetTitle() );

        if( aTitle.isEmpty() )
            aTitle = aURL.getBase( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_UNAMBIGUOUS );

        if( aTitle.isEmpty() )
        {
            aTitle = aURL.GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS );
            aTitle = aTitle.getToken( comphelper::string::getTokenCount(aTitle, '/') - 1, '/' );
        }

        aRet += aTitle;
    }

    if( nItemTextFlags & GalleryItemFlags::Path )
    {
        const OUString aPath( aURL.getFSysPath( INetURLObject::FSYS_DETECT ) );

        if( !aPath.isEmpty() && ( nItemTextFlags & GalleryItemFlags::Title ) )
            aRet += " (";

        aRet += aURL.getFSysPath( INetURLObject::FSYS_DETECT );

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

        if( ( SGA_OBJ_BMP == eObjKind ) || ( SGA_OBJ_ANIM == eObjKind ) )
        {
            GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
            INetURLObject       aURL;
            mpCurTheme->GetURL( mnCurActionPos, aURL );
            sal_uInt16          nFilter = rFilter.GetImportFormatNumberForShortName( aURL.GetExtension() );

            if( GRFILTER_FORMAT_DONTKNOW != nFilter )
                aFilterName = rFilter.GetImportFormatName( nFilter );
        }
    }

    return aFilterName;
}


IMPL_LINK_NOARG_TYPED(GalleryBrowser2, SelectObjectValueSetHdl, ValueSet*, void)
{
    ImplUpdateInfoBar();
}

IMPL_LINK_NOARG_TYPED(GalleryBrowser2, SelectObjectHdl, GalleryListView*, void)
{
    ImplUpdateInfoBar();
}

IMPL_LINK_TYPED( GalleryBrowser2, SelectTbxHdl, ToolBox*, pBox, void )
{
    if( pBox->GetCurItemId() == TBX_ID_ICON )
        SetMode( GALLERYBROWSERMODE_ICON );
    else if( pBox->GetCurItemId() == TBX_ID_LIST )
        SetMode( GALLERYBROWSERMODE_LIST );
}

IMPL_LINK_NOARG_TYPED(GalleryBrowser2, MiscHdl, LinkParamNone*, void)
{
    maViewBox->SetOutStyle( maMiscOptions.GetToolboxStyle() );

    BitmapEx aIconBmpEx = BitmapEx( Image( GAL_RES( RID_SVXIMG_GALLERY_VIEW_ICON ) ).GetBitmapEx() );
    BitmapEx aListBmpEx = BitmapEx( Image( GAL_RES( RID_SVXIMG_GALLERY_VIEW_LIST ) ).GetBitmapEx() );

    if( maMiscOptions.AreCurrentSymbolsLarge() )
    {
        const Size aLargeSize( 24, 24);

        aIconBmpEx.Scale( aLargeSize );
        aListBmpEx.Scale( aLargeSize );
    }

    maViewBox->SetItemImage(TBX_ID_ICON, Image(aIconBmpEx));
    maViewBox->SetItemImage(TBX_ID_LIST, Image(aListBmpEx));
    maViewBox->SetSizePixel( maViewBox->CalcWindowSizePixel() );

    Resize();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
