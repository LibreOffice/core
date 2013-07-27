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

#undef GALLERY_USE_CLIPBOARD

#define TBX_ID_ICON 1
#define TBX_ID_LIST 2

namespace css = ::com::sun::star;
using rtl::OUString;

GalleryBrowserMode GalleryBrowser2::meInitMode = GALLERYBROWSERMODE_ICON;

struct DispatchInfo
{
    css::util::URL                                  TargetURL;
    css::uno::Sequence< css::beans::PropertyValue > Arguments;
    css::uno::Reference< css::frame::XDispatch >    Dispatch;
};

IMPL_STATIC_LINK_NOINSTANCE( GalleryBrowser2, AsyncDispatch_Impl, DispatchInfo*, pDispatchInfo )
{
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
    return 0;
}

namespace
{

struct CommandInfo
{
    css::util::URL                               URL;
    css::uno::Reference< css::frame::XDispatch > Dispatch;

    CommandInfo( const OUString &rURL )
    {
        URL.Complete = rURL;
    }
};

class GalleryThemePopup : public ::cppu::WeakImplHelper1< css::frame::XStatusListener >
{
private:
    const GalleryTheme* mpTheme;
    sal_uIntPtr         mnObjectPos;
    sal_Bool            mbPreview;
    PopupMenu           maPopupMenu;
    PopupMenu           maBackgroundPopup;
    GalleryBrowser2*    mpBrowser;

    typedef std::map< int, CommandInfo > CommandInfoMap;
    CommandInfoMap   m_aCommandInfo;

    void Execute( const CommandInfo &rCmdInfo,
                  const css::uno::Sequence< css::beans::PropertyValue > &rArguments );

    DECL_LINK( MenuSelectHdl, Menu* pMenu );
    DECL_LINK( BackgroundMenuSelectHdl, Menu* pMenu );
public:
    GalleryThemePopup( const GalleryTheme* pTheme,
                       sal_uIntPtr nObjectPos,
                       sal_Bool bPreview,
                       GalleryBrowser2* pBrowser );
    ~GalleryThemePopup();

    void ExecutePopup( Window *pParent, const ::Point &aPos );

    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent &rEvent) throw ( css::uno::RuntimeException );
    virtual void SAL_CALL disposing( const css::lang::EventObject &rSource) throw ( css::uno::RuntimeException );
};

DBG_NAME(GalleryThemePopup)

GalleryThemePopup::GalleryThemePopup(
    const GalleryTheme* pTheme,
    sal_uIntPtr nObjectPos,
    sal_Bool bPreview,
    GalleryBrowser2* pBrowser )
    : mpTheme( pTheme )
    , mnObjectPos( nObjectPos )
    , mbPreview( bPreview )
    , maPopupMenu( GAL_RES( RID_SVXMN_GALLERY2 ) )
    , maBackgroundPopup()
    , mpBrowser( pBrowser )
{
    DBG_CTOR(GalleryThemePopup,NULL);

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
    DBG_DTOR(GalleryThemePopup,NULL);
}

void SAL_CALL GalleryThemePopup::statusChanged(
    const css::frame::FeatureStateEvent &rEvent )
throw ( css::uno::RuntimeException )
{
    const OUString &rURL = rEvent.FeatureURL.Complete;
    if ( rURL.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( CMD_SID_GALLERY_ENABLE_ADDCOPY ) ) )
    {
        if ( !rEvent.IsEnabled )
        {
            maPopupMenu.EnableItem( MN_ADD, sal_False );
        }
    }
    else if ( rURL.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( CMD_SID_GALLERY_BG_BRUSH ) ) )
    {
        maBackgroundPopup.Clear();
        if ( rEvent.IsEnabled )
        {
            OUString sItem;
            css::uno::Sequence< OUString > sItems;
            if ( ( rEvent.State >>= sItem ) && sItem.getLength() )
            {
                maBackgroundPopup.InsertItem( 1, sItem );
            }
            else if ( ( rEvent.State >>= sItems ) && sItems.getLength() )
            {
                const OUString *pStr = sItems.getConstArray();
                const OUString *pEnd = pStr + sItems.getLength();
                for ( sal_uInt16 nId = 1; pStr != pEnd; pStr++, nId++ )
                {
                    maBackgroundPopup.InsertItem( nId, *pStr );
                }
            }
        }
    }
}

void SAL_CALL GalleryThemePopup::disposing(
    const css::lang::EventObject &/*rSource*/)
throw ( css::uno::RuntimeException )
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
                STATIC_LINK( 0, GalleryBrowser2, AsyncDispatch_Impl), pInfo ) )
            delete pInfo;
    }
}

void GalleryThemePopup::ExecutePopup( Window *pWindow, const ::Point &aPos )
{
    css::uno::Reference< css::frame::XStatusListener > xThis( this );

    const SgaObjKind eObjKind = mpTheme->GetObjectKind( mnObjectPos );
    INetURLObject    aURL;

    const_cast< GalleryTheme* >( mpTheme )->GetURL( mnObjectPos, aURL );
    const sal_Bool bValidURL = ( aURL.GetProtocol() != INET_PROT_NOT_VALID );

    maPopupMenu.EnableItem( MN_ADD, bValidURL && SGA_OBJ_SOUND != eObjKind );

    maPopupMenu.EnableItem( MN_PREVIEW, bValidURL );

    maPopupMenu.CheckItem( MN_PREVIEW, mbPreview );

    if( mpTheme->IsReadOnly() || !mpTheme->GetObjectCount() )
    {
        maPopupMenu.EnableItem( MN_DELETE, sal_False );
        maPopupMenu.EnableItem( MN_TITLE, sal_False );

        if( mpTheme->IsReadOnly() )
            maPopupMenu.EnableItem( MN_PASTECLIPBOARD, sal_False );

        if( !mpTheme->GetObjectCount() )
            maPopupMenu.EnableItem( MN_COPYCLIPBOARD, sal_False );
    }
    else
    {
        maPopupMenu.EnableItem( MN_DELETE, !mbPreview );
        maPopupMenu.EnableItem( MN_TITLE, sal_True );
        maPopupMenu.EnableItem( MN_COPYCLIPBOARD, sal_True );
        maPopupMenu.EnableItem( MN_PASTECLIPBOARD, sal_True );
    }

#ifdef GALLERY_USE_CLIPBOARD
    if( maPopupMenu.IsItemEnabled( MN_PASTECLIPBOARD ) )
    {
        TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( this ) );
        sal_Bool               bEnable = sal_False;

        if( aDataHelper.GetFormatCount() )
        {
            if( aDataHelper.HasFormat( SOT_FORMATSTR_ID_DRAWING ) ||
                aDataHelper.HasFormat( SOT_FORMAT_FILE_LIST ) ||
                aDataHelper.HasFormat( FORMAT_FILE ) ||
                aDataHelper.HasFormat( SOT_FORMATSTR_ID_SVXB ) ||
                aDataHelper.HasFormat( FORMAT_GDIMETAFILE ) ||
                aDataHelper.HasFormat( FORMAT_BITMAP ) )
            {
                bEnable = sal_True;
            }
        }

        if( !bEnable )
            maPopupMenu.EnableItem( MN_PASTECLIPBOARD, sal_False );
    }
#else
    maPopupMenu.EnableItem( MN_COPYCLIPBOARD, sal_False );
    maPopupMenu.EnableItem( MN_PASTECLIPBOARD, sal_False );
#endif

    // update status
    css::uno::Reference< css::frame::XDispatchProvider> xDispatchProvider(
        mpBrowser->GetFrame(), css::uno::UNO_QUERY );
    css::uno::Reference< css::util::XURLTransformer > xTransformer(
        mpBrowser->GetURLTransformer() );
    for ( CommandInfoMap::iterator it = m_aCommandInfo.begin();
         it != m_aCommandInfo.end(); it++ )
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
                    OUString( "_self" ),
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

    if( !maBackgroundPopup.GetItemCount() || ( eObjKind == SGA_OBJ_SVDRAW ) || ( eObjKind == SGA_OBJ_SOUND ) )
        maPopupMenu.EnableItem( MN_BACKGROUND, sal_False );
    else
    {
        maPopupMenu.EnableItem( MN_BACKGROUND, sal_True );
        maPopupMenu.SetPopupMenu( MN_BACKGROUND, &maBackgroundPopup );
        maBackgroundPopup.SetSelectHdl( LINK( this, GalleryThemePopup, BackgroundMenuSelectHdl ) );
    }

    maPopupMenu.RemoveDisabledEntries();

    maPopupMenu.SetSelectHdl( LINK( this, GalleryThemePopup, MenuSelectHdl ) );
    maPopupMenu.Execute( pWindow, aPos );
}

IMPL_LINK( GalleryThemePopup, MenuSelectHdl, Menu*, pMenu )
{
    if( !pMenu )
        return 0;

    sal_uInt16 nId( pMenu->GetCurItemId() );
    switch ( nId )
    {
        case( MN_ADD ):
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

    return 0;
}

IMPL_LINK( GalleryThemePopup, BackgroundMenuSelectHdl, Menu*, pMenu )
{
    if( !pMenu )
        return 0;

    sal_uInt16 nPos( pMenu->GetCurItemId() - 1 );
    OUString aURL( mpBrowser->GetURL().GetMainURL( INetURLObject::NO_DECODE ) );
    OUString aFilterName( mpBrowser->GetFilterName() );

    css::uno::Sequence< css::beans::PropertyValue > aArgs( 6 );
    aArgs[0].Name = OUString( "Background.Transparent" );
    aArgs[0].Value <<= sal_Int32( 0 ); // 0 - 100
    aArgs[1].Name = OUString( "Background.BackColor" );
    aArgs[1].Value <<= sal_Int32( - 1 );
    aArgs[2].Name = OUString( "Background.URL" );
    aArgs[2].Value <<= aURL;
    aArgs[3].Name = OUString( "Background.Filtername" ); // FIXME should be FilterName
    aArgs[3].Value <<= aFilterName;
    aArgs[4].Name = OUString( "Background.Position" );
    aArgs[4].Value <<= css::style::GraphicLocation_TILED;
    aArgs[5].Name = OUString( "Position" );
    aArgs[5].Value <<= nPos;

    const CommandInfoMap::const_iterator it = m_aCommandInfo.find( SID_GALLERY_BG_BRUSH );
    if ( it != m_aCommandInfo.end() )
        Execute( it->second, aArgs );

    return 0;
}

} // end anonymous namespace

DBG_NAME(GalleryToolBox)

GalleryToolBox::GalleryToolBox( GalleryBrowser2* pParent ) :
    ToolBox( pParent, WB_TABSTOP )
{
    DBG_CTOR(GalleryToolBox,NULL);
}

GalleryToolBox::~GalleryToolBox()
{
    DBG_DTOR(GalleryToolBox,NULL);
}

void GalleryToolBox::KeyInput( const KeyEvent& rKEvt )
{
    if( !static_cast< GalleryBrowser2* >( GetParent() )->KeyInput( rKEvt, this ) )
        ToolBox::KeyInput( rKEvt );
}

DBG_NAME(GalleryBrowser2)

GalleryBrowser2::GalleryBrowser2( Window* pParent, const ResId& rResId, Gallery* pGallery ) :
    Control             ( pParent, rResId ),
    mpGallery           ( pGallery ),
    mpCurTheme          ( NULL ),
    mpIconView          ( new GalleryIconView( this, NULL ) ),
    mpListView          ( new GalleryListView( this, NULL ) ),
    mpPreview           ( new GalleryPreview( this, NULL ) ),
    maViewBox           ( this ),
    maSeparator         ( this, WB_VERT ),
    maInfoBar           ( this, WB_LEFT | WB_VCENTER ),
    mnCurActionPos      ( 0xffffffff ),
    meMode              ( GALLERYBROWSERMODE_NONE ),
    meLastMode          ( GALLERYBROWSERMODE_NONE )
{
    DBG_CTOR(GalleryBrowser2,NULL);

    m_xContext.set( ::comphelper::getProcessComponentContext() );

    m_xTransformer.set(
        m_xContext->getServiceManager()->createInstanceWithContext(
            OUString( "com.sun.star.util.URLTransformer" ), m_xContext ),
                    css::uno::UNO_QUERY );

    Image       aDummyImage;
    const Link  aSelectHdl( LINK( this, GalleryBrowser2, SelectObjectHdl ) );
    Font        aInfoFont( maInfoBar.GetControlFont() );

    maMiscOptions.AddListenerLink( LINK( this, GalleryBrowser2, MiscHdl ) );

    maViewBox.InsertItem( TBX_ID_ICON, aDummyImage );
    maViewBox.SetItemBits( TBX_ID_ICON, TIB_RADIOCHECK | TIB_AUTOCHECK );
    maViewBox.SetHelpId( TBX_ID_ICON, HID_GALLERY_ICONVIEW );
    maViewBox.SetQuickHelpText( TBX_ID_ICON, GAL_RESSTR(RID_SVXSTR_GALLERY_ICONVIEW) );

    maViewBox.InsertItem( TBX_ID_LIST, aDummyImage );
    maViewBox.SetItemBits( TBX_ID_LIST, TIB_RADIOCHECK | TIB_AUTOCHECK );
    maViewBox.SetHelpId( TBX_ID_LIST, HID_GALLERY_LISTVIEW );
    maViewBox.SetQuickHelpText( TBX_ID_LIST, GAL_RESSTR(RID_SVXSTR_GALLERY_LISTVIEW) );

    MiscHdl( NULL );
    maViewBox.SetSelectHdl( LINK( this, GalleryBrowser2, SelectTbxHdl ) );
    maViewBox.Show();

    mpIconView->SetAccessibleName(SVX_RESSTR(RID_SVXSTR_GALLERY_THEMEITEMS));
    mpListView->SetAccessibleName(SVX_RESSTR(RID_SVXSTR_GALLERY_THEMEITEMS));

    maInfoBar.Show();
    maSeparator.Show();

    mpIconView->SetSelectHdl( aSelectHdl );
    mpListView->SetSelectHdl( aSelectHdl );

    InitSettings();

    SetMode( ( GALLERYBROWSERMODE_PREVIEW != GalleryBrowser2::meInitMode ) ? GalleryBrowser2::meInitMode : GALLERYBROWSERMODE_ICON );

    if(maInfoBar.GetText().isEmpty())
        mpIconView->SetAccessibleRelationLabeledBy(mpIconView);
    else
        mpIconView->SetAccessibleRelationLabeledBy(&maInfoBar);
    mpIconView->SetAccessibleRelationMemberOf(mpIconView);
}

GalleryBrowser2::~GalleryBrowser2()
{
    maMiscOptions.RemoveListenerLink( LINK( this, GalleryBrowser2, MiscHdl ) );

    delete mpPreview;
    delete mpListView;
    delete mpIconView;

    if( mpCurTheme )
        mpGallery->ReleaseTheme( mpCurTheme, *this );

    DBG_DTOR(GalleryBrowser2,NULL);
}

void GalleryBrowser2::InitSettings()
{
    Font  aInfoFont( maInfoBar.GetControlFont() );

    aInfoFont.SetWeight( WEIGHT_BOLD );
    aInfoFont.SetColor( GALLERY_FG_COLOR );
    maInfoBar.SetControlFont( aInfoFont );

    maInfoBar.SetBackground( Wallpaper( GALLERY_DLG_COLOR ) );
    maInfoBar.SetControlBackground( GALLERY_DLG_COLOR );

    maSeparator.SetBackground( Wallpaper( GALLERY_BG_COLOR ) );
    maSeparator.SetControlBackground( GALLERY_BG_COLOR );
    maSeparator.SetControlForeground( GALLERY_FG_COLOR );
}

void GalleryBrowser2::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
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
    const Size  aBoxSize( maViewBox.GetOutputSizePixel() );
    const long  nOffset = 2, nSepWidth = 2;
    const long  nInfoBarX = aBoxSize.Width() + ( nOffset * 3 ) + nSepWidth;
    const Point aPt( 0, aBoxSize.Height() + 3 );
    const Size  aSz( aOutSize.Width(), aOutSize.Height() - aPt.Y() );

    maSeparator.SetPosSizePixel( Point( aBoxSize.Width() + nOffset, 0 ), Size( nSepWidth, aBoxSize.Height() ) );
    maInfoBar.SetPosSizePixel( Point( nInfoBarX, 0 ), Size( aOutSize.Width() - nInfoBarX, aBoxSize.Height() ) );

    mpIconView->SetPosSizePixel( aPt, aSz );
    mpListView->SetPosSizePixel( aPt, aSz );
    mpPreview->SetPosSizePixel( aPt, aSz );

    switch( GetMode() )
    {
        case( GALLERYBROWSERMODE_ICON ): mpIconView->Show(); break;
        case( GALLERYBROWSERMODE_LIST ): mpListView->Show(); break;
        case( GALLERYBROWSERMODE_PREVIEW ): mpPreview->Show(); break;

        default:
        break;
    }
}

void GalleryBrowser2::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    const GalleryHint& rGalleryHint = (const GalleryHint&) rHint;

    switch( rGalleryHint.GetType() )
    {
        case( GALLERY_HINT_THEME_UPDATEVIEW ):
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
            if( rTarget.IsDropFormatSupported( SOT_FORMATSTR_ID_DRAWING ) ||
                rTarget.IsDropFormatSupported( SOT_FORMAT_FILE_LIST ) ||
                rTarget.IsDropFormatSupported( FORMAT_FILE ) ||
                rTarget.IsDropFormatSupported( SOT_FORMATSTR_ID_SVXB ) ||
                rTarget.IsDropFormatSupported( FORMAT_GDIMETAFILE ) ||
                rTarget.IsDropFormatSupported( FORMAT_BITMAP ) )
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
            nRet = mpCurTheme->InsertTransferable( rEvt.maDropEvent.Transferable, nInsertPos );
    }

    return nRet;
}

void GalleryBrowser2::StartDrag( Window*, const Point* pDragPoint )
{
    if( mpCurTheme )
    {
        Point       aSelPos;
        const sal_uIntPtr nItemId = ImplGetSelectedItemId( pDragPoint, aSelPos );

        if( nItemId )
            mpCurTheme->StartDrag( this, nItemId - 1 );
    }
}

void GalleryBrowser2::TogglePreview( Window*, const Point* )
{
    SetMode( ( GALLERYBROWSERMODE_PREVIEW != GetMode() ) ? GALLERYBROWSERMODE_PREVIEW : meLastMode );
    GetViewWindow()->GrabFocus();
}

void GalleryBrowser2::ShowContextMenu( Window*, const Point* pContextPoint )
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

sal_Bool GalleryBrowser2::KeyInput( const KeyEvent& rKEvt, Window* pWindow )
{
    Point       aSelPos;
    const sal_uIntPtr   nItemId = ImplGetSelectedItemId( NULL, aSelPos );
    GalleryBrowser* pParentBrowser = dynamic_cast<GalleryBrowser*>(GetParent());
    sal_Bool bRet = sal_False;
    if (pParentBrowser != NULL)
        bRet = pParentBrowser->KeyInput( rKEvt, pWindow );
    else
    {
        svx::sidebar::GalleryControl* pParentControl = dynamic_cast<svx::sidebar::GalleryControl*>(GetParent());
        if (pParentControl != NULL)
            bRet = pParentControl->GalleryKeyInput(rKEvt, pWindow);
    }

    if( !bRet && !maViewBox.HasFocus() && nItemId && mpCurTheme )
    {
        sal_uInt16          nExecuteId = 0;
        INetURLObject       aURL;

        const_cast< GalleryTheme* >( mpCurTheme )->GetURL( nItemId - 1, aURL );

        const sal_Bool  bValidURL = ( aURL.GetProtocol() != INET_PROT_NOT_VALID );
        sal_Bool        bPreview = bValidURL;
        sal_Bool        bDelete = sal_False;
        sal_Bool        bTitle = sal_False;

        if( !mpCurTheme->IsReadOnly() && mpCurTheme->GetObjectCount() )
        {
            bDelete = ( GALLERYBROWSERMODE_PREVIEW != GetMode() );
            bTitle = sal_True;
        }

        switch( rKEvt.GetKeyCode().GetCode() )
        {
            case( KEY_SPACE ):
            case( KEY_RETURN ):
            case( KEY_P ):
            {
                if( bPreview )
                {
                    TogglePreview( pWindow );
                    bRet = sal_True;
                }
            }
            break;

            case( KEY_INSERT ):
            case( KEY_I ):
            {
                // Inserting a gallery item in the document must be dispatched
                if( bValidURL )
                {
                    Dispatch( MN_ADD );
                    return sal_True;
                }
            }
            break;

            case( KEY_DELETE ):
            case( KEY_D ):
            {
                if( bDelete )
                    nExecuteId = MN_DELETE;
            }
            break;

            case( KEY_T ):
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
            bRet = sal_True;
        }
    }

    return bRet;
}

void GalleryBrowser2::SelectTheme( const String& rThemeName )
{
    delete mpIconView, mpIconView = NULL;
    delete mpListView, mpListView = NULL;
    delete mpPreview, mpPreview = NULL;

    if( mpCurTheme )
        mpGallery->ReleaseTheme( mpCurTheme, *this );

    mpCurTheme = mpGallery->AcquireTheme( rThemeName, *this );

    mpIconView = new GalleryIconView( this, mpCurTheme );
    mpListView = new GalleryListView( this, mpCurTheme );
    mpPreview = new GalleryPreview( this, mpCurTheme );

    mpIconView->SetAccessibleName(SVX_RESSTR(RID_SVXSTR_GALLERY_THEMEITEMS));
    mpListView->SetAccessibleName(SVX_RESSTR(RID_SVXSTR_GALLERY_THEMEITEMS));
    mpPreview->SetAccessibleName(SVX_RESSTR(RID_SVXSTR_GALLERY_PREVIEW));

    const Link aSelectHdl( LINK( this, GalleryBrowser2, SelectObjectHdl ) );

    mpIconView->SetSelectHdl( aSelectHdl );
    mpListView->SetSelectHdl( aSelectHdl );

    if( GALLERYBROWSERMODE_PREVIEW == GetMode() )
        meMode = meLastMode;

    Resize();
    ImplUpdateViews( 1 );

    maViewBox.EnableItem( TBX_ID_ICON, sal_True );
    maViewBox.EnableItem( TBX_ID_LIST, sal_True );
    maViewBox.CheckItem( ( GALLERYBROWSERMODE_ICON == GetMode() ) ? TBX_ID_ICON : TBX_ID_LIST, sal_True );

    if(maInfoBar.GetText().isEmpty())
        mpIconView->SetAccessibleRelationLabeledBy(mpIconView);
    else
        mpIconView->SetAccessibleRelationLabeledBy(&maInfoBar);
    mpIconView->SetAccessibleRelationMemberOf(mpIconView);
}

void GalleryBrowser2::SetMode( GalleryBrowserMode eMode )
{
    if( GetMode() != eMode )
    {
        meLastMode = GetMode();

        switch( eMode )
        {
            case( GALLERYBROWSERMODE_ICON ):
            {
                mpListView->Hide();

                mpPreview->Hide();
                mpPreview->SetGraphic( Graphic() );
                mpPreview->PreviewMedia( INetURLObject() );

                mpIconView->Show();

                maViewBox.EnableItem( TBX_ID_ICON, sal_True );
                maViewBox.EnableItem( TBX_ID_LIST, sal_True );

                maViewBox.CheckItem( TBX_ID_ICON, sal_True );
                maViewBox.CheckItem( TBX_ID_LIST, sal_False );
            }
            break;

            case( GALLERYBROWSERMODE_LIST ):
            {
                mpIconView->Hide();

                mpPreview->Hide();
                mpPreview->SetGraphic( Graphic() );
                mpPreview->PreviewMedia( INetURLObject() );

                mpListView->Show();

                maViewBox.EnableItem( TBX_ID_ICON, sal_True );
                maViewBox.EnableItem( TBX_ID_LIST, sal_True );

                maViewBox.CheckItem( TBX_ID_ICON, sal_False );
                maViewBox.CheckItem( TBX_ID_LIST, sal_True );
            }
            break;

            case( GALLERYBROWSERMODE_PREVIEW ):
            {
                Graphic     aGraphic;
                Point       aSelPos;
                const sal_uIntPtr   nItemId = ImplGetSelectedItemId( NULL, aSelPos );

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
                        mpPreview->PreviewMedia( mpCurTheme->GetObjectURL( nPos ) );

                    maViewBox.EnableItem( TBX_ID_ICON, sal_False );
                    maViewBox.EnableItem( TBX_ID_LIST, sal_False );
                }
            }
            break;

            default:
                break;
        }

        GalleryBrowser2::meInitMode = meMode = eMode;
    }
}

Window* GalleryBrowser2::GetViewWindow() const
{
    Window* pRet;

    switch( GetMode() )
    {
        case( GALLERYBROWSERMODE_LIST ): pRet = mpListView; break;
        case( GALLERYBROWSERMODE_PREVIEW ): pRet = mpPreview; break;

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
        const sal_uIntPtr nItemId = ImplGetSelectedItemId( NULL, aSelPos );

        if( nItemId )
        {
            sal_uIntPtr nNewItemId = nItemId;

            switch( eTravel )
            {
                case( GALLERYBROWSERTRAVEL_FIRST ):     nNewItemId = 1; break;
                case( GALLERYBROWSERTRAVEL_LAST ):      nNewItemId = mpCurTheme->GetObjectCount(); break;
                case( GALLERYBROWSERTRAVEL_PREVIOUS ):  nNewItemId--; break;
                case( GALLERYBROWSERTRAVEL_NEXT ):      nNewItemId++; break;
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
                        mpPreview->PreviewMedia( mpCurTheme->GetObjectURL( nPos ) );

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
        case( GALLERYBROWSERMODE_ICON ): mpIconView->Show(); break;
        case( GALLERYBROWSERMODE_LIST ): mpListView->Show(); break;
        case( GALLERYBROWSERMODE_PREVIEW ): mpPreview->Show(); break;

        default:
        break;
    }

    ImplUpdateInfoBar();
}

void GalleryBrowser2::ImplUpdateInfoBar()
{
    String aInfoText;

    if( mpCurTheme )
    {
        Point       aSelPos;
        const sal_uIntPtr nItemId = ImplGetSelectedItemId( NULL, aSelPos );

        if( nItemId )
        {
            const sal_uIntPtr nPos = nItemId - 1;

            aInfoText = mpCurTheme->GetName();

            if( nPos < mpCurTheme->GetObjectCount() )
            {
                SgaObject* pObj = mpCurTheme->AcquireObject( nPos );

                if( pObj )
                {
                    aInfoText = GetItemText( *mpCurTheme, *pObj, GALLERY_ITEM_THEMENAME | GALLERY_ITEM_TITLE | GALLERY_ITEM_PATH );
                    mpCurTheme->ReleaseObject( pObj );
                }
            }
        }
    }

    maInfoBar.SetText( aInfoText );
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
GalleryBrowser2::GetFrame() const
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
    const sal_uIntPtr nItemId = ImplGetSelectedItemId( NULL, aSelPos );

    if( !mpCurTheme || !nItemId )
        return;

    mnCurActionPos = nItemId - 1;

    switch( nId )
    {
        case( MN_ADD ):
        {
            css::uno::Reference< css::frame::XDispatch > xDispatch( rxDispatch );
            css::util::URL aURL = rURL;

            if ( !xDispatch.is() )
            {
                css::uno::Reference< css::frame::XDispatchProvider > xDispatchProvider(
                    GetFrame(), css::uno::UNO_QUERY );
                if ( !xDispatchProvider.is() || !m_xTransformer.is() )
                    return;

                aURL.Complete = OUString( CMD_SID_GALLERY_FORMATS );
                m_xTransformer->parseStrict( aURL );
                xDispatch = xDispatchProvider->queryDispatch(
                    aURL,
                    OUString( "_self" ),
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
                case( SGA_OBJ_BMP ):
                case( SGA_OBJ_ANIM ):
                case( SGA_OBJ_INET ):
                // TODO drawing objects are inserted as drawings only via drag&drop
                case( SGA_OBJ_SVDRAW ):
                    nType = css::gallery::GalleryItemType::GRAPHIC;
                break;

                case ( SGA_OBJ_SOUND ) :
                    nType = css::gallery::GalleryItemType::MEDIA;
                break;

                default:
                    nType = css::gallery::GalleryItemType::EMPTY;
                break;
            }

            Graphic aGraphic;
            sal_Bool bGraphic = mpCurTheme->GetGraphic( mnCurActionPos, aGraphic );
            if ( bGraphic && !!aGraphic )
                xGraphic.set( aGraphic.GetXGraphic() );
            OSL_ENSURE( xGraphic.is(), "gallery item is graphic, but the reference is invalid!" );

            css::uno::Sequence< css::beans::PropertyValue > aSeq( SVXGALLERYITEM_PARAMS );

            aSeq[0].Name = OUString( SVXGALLERYITEM_TYPE );
            aSeq[0].Value <<= nType;
            aSeq[1].Name = OUString( SVXGALLERYITEM_URL );
            aSeq[1].Value <<= aFileURL;
            aSeq[2].Name = OUString( SVXGALLERYITEM_FILTER );
            aSeq[2].Value <<= aFilterName;
            aSeq[3].Name = OUString( SVXGALLERYITEM_DRAWING );
            aSeq[3].Value <<= xDrawing;
            aSeq[4].Name = OUString( SVXGALLERYITEM_GRAPHIC );
            aSeq[4].Value <<= xGraphic;

            css::uno::Sequence< css::beans::PropertyValue > aArgs( 1 );
            aArgs[0].Name = OUString( SVXGALLERYITEM_ARGNAME );
            aArgs[0].Value <<= aSeq;

            DispatchInfo *pInfo = new DispatchInfo;
            pInfo->TargetURL = aURL;
            pInfo->Arguments = aArgs;
            pInfo->Dispatch = xDispatch;

            if ( !Application::PostUserEvent(
                    STATIC_LINK( 0, GalleryBrowser2, AsyncDispatch_Impl), pInfo ) )
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
    const sal_uIntPtr nItemId = ImplGetSelectedItemId( NULL, aSelPos );

    if( mpCurTheme && nItemId )
    {
        mnCurActionPos = nItemId - 1;

        switch( nId )
        {
            case( MN_PREVIEW ):
                SetMode( ( GALLERYBROWSERMODE_PREVIEW != GetMode() ) ? GALLERYBROWSERMODE_PREVIEW : meLastMode );
            break;

            case( MN_DELETE ):
            {
                if( !mpCurTheme->IsReadOnly() &&
                    QueryBox(NULL, WB_YES_NO, GAL_RESSTR(RID_SVXSTR_GALLERY_DELETEOBJ)).Execute() == RET_YES )
                {
                    mpCurTheme->RemoveObject( mnCurActionPos );
                }
            }
            break;

            case( MN_TITLE ):
            {
                SgaObject* pObj = mpCurTheme->AcquireObject( mnCurActionPos );

                if( pObj )
                {
                    const String    aOldTitle( GetItemText( *mpCurTheme, *pObj, GALLERY_ITEM_TITLE ) );

                    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                    if(pFact)
                    {
                        AbstractTitleDialog* aDlg = pFact->CreateTitleDialog( this, aOldTitle );
                        DBG_ASSERT(aDlg, "Dialogdiet fail!");
                        if( aDlg->Execute() == RET_OK )
                        {
                            String aNewTitle( aDlg->GetTitle() );

                            if( ( !aNewTitle.Len() && pObj->GetTitle().Len() ) || ( aNewTitle != aOldTitle ) )
                            {
                                if( !aNewTitle.Len() )
                                    aNewTitle = String( RTL_CONSTASCII_USTRINGPARAM( "__<empty>__" ) );

                                pObj->SetTitle( aNewTitle );
                                mpCurTheme->InsertObject( *pObj );
                            }
                        }

                        mpCurTheme->ReleaseObject( pObj );
                        delete aDlg;
                    }
                }
            }
            break;

            case( MN_COPYCLIPBOARD ):
            {
                Window* pWindow;

                switch( GetMode() )
                {
                    case( GALLERYBROWSERMODE_ICON ): pWindow = (Window*) mpIconView; break;
                    case( GALLERYBROWSERMODE_LIST ): pWindow = (Window*) mpListView; break;
                    case( GALLERYBROWSERMODE_PREVIEW ): pWindow = (Window*) mpPreview; break;

                    default:
                        pWindow = NULL;
                    break;
                }

                mpCurTheme->CopyToClipboard( pWindow, mnCurActionPos );
            }
            break;

            case( MN_PASTECLIPBOARD ):
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

String GalleryBrowser2::GetItemText( const GalleryTheme& rTheme, const SgaObject& rObj, sal_uIntPtr nItemTextFlags )
{
    String          aRet;

    INetURLObject aURL(rObj.GetURL());

    if( nItemTextFlags & GALLERY_ITEM_THEMENAME )
    {
        aRet += rTheme.GetName();
        aRet += String( RTL_CONSTASCII_USTRINGPARAM( " - " ) );
    }

    if( nItemTextFlags & GALLERY_ITEM_TITLE )
    {
        String aTitle( rObj.GetTitle() );

        if( !aTitle.Len() )
            aTitle = aURL.getBase( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_UNAMBIGUOUS );

        if( !aTitle.Len() )
        {
            aTitle = aURL.GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS );
            aTitle = aTitle.GetToken( comphelper::string::getTokenCount(aTitle, '/') - 1, '/' );
        }

        aRet += aTitle;
    }

    if( nItemTextFlags & GALLERY_ITEM_PATH )
    {
        const String aPath( aURL.getFSysPath( INetURLObject::FSYS_DETECT ) );

        if( aPath.Len() && ( nItemTextFlags & GALLERY_ITEM_TITLE ) )
            aRet += String( RTL_CONSTASCII_USTRINGPARAM( " (" ) );

        aRet += String(aURL.getFSysPath( INetURLObject::FSYS_DETECT ));

        if( aPath.Len() && ( nItemTextFlags & GALLERY_ITEM_TITLE ) )
            aRet += ')';
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

String GalleryBrowser2::GetFilterName() const
{
    String aFilterName;

    if( mpCurTheme && mnCurActionPos != 0xffffffff )
    {
        const SgaObjKind eObjKind = mpCurTheme->GetObjectKind( mnCurActionPos );

        if( ( SGA_OBJ_BMP == eObjKind ) || ( SGA_OBJ_ANIM == eObjKind ) )
        {
            GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
            INetURLObject       aURL; mpCurTheme->GetURL( mnCurActionPos, aURL );
            sal_uInt16          nFilter = rFilter.GetImportFormatNumberForShortName( aURL.GetExtension() );

            if( GRFILTER_FORMAT_DONTKNOW != nFilter )
                aFilterName = rFilter.GetImportFormatName( nFilter );
        }
    }

    return aFilterName;
}

Graphic GalleryBrowser2::GetGraphic() const
{
    Graphic aGraphic;

    if( mpCurTheme && mnCurActionPos != 0xffffffff )
        mpCurTheme->GetGraphic( mnCurActionPos, aGraphic );

    return aGraphic;
}

IMPL_LINK_NOARG(GalleryBrowser2, SelectObjectHdl)
{
    ImplUpdateInfoBar();
    return 0L;
}

IMPL_LINK( GalleryBrowser2, SelectTbxHdl, ToolBox*, pBox )
{
    if( pBox->GetCurItemId() == TBX_ID_ICON )
        SetMode( GALLERYBROWSERMODE_ICON );
    else if( pBox->GetCurItemId() == TBX_ID_LIST )
        SetMode( GALLERYBROWSERMODE_LIST );

    return 0L;
}

IMPL_LINK_NOARG(GalleryBrowser2, MiscHdl)
{
    maViewBox.SetOutStyle( maMiscOptions.GetToolboxStyle() );

    BitmapEx aIconBmpEx = BitmapEx( Image( GAL_RES( RID_SVXIMG_GALLERY_VIEW_ICON ) ).GetBitmapEx() );
    BitmapEx aListBmpEx = BitmapEx( Image( GAL_RES( RID_SVXIMG_GALLERY_VIEW_LIST ) ).GetBitmapEx() );

    if( maMiscOptions.AreCurrentSymbolsLarge() )
    {
        const Size aLargeSize( 24, 24);

        aIconBmpEx.Scale( aLargeSize );
        aListBmpEx.Scale( aLargeSize );
    }

    maViewBox.SetItemImage( TBX_ID_ICON, aIconBmpEx );
    maViewBox.SetItemImage( TBX_ID_LIST, aListBmpEx );
    maViewBox.SetSizePixel( maViewBox.CalcWindowSizePixel() );

    Resize();

    return 0L;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
