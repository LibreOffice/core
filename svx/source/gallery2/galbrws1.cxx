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

#include "sal/config.h"

#include <comphelper/processfactory.hxx>
#include <tools/datetime.hxx>
#include <unotools/datetime.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/settings.hxx>
#include <ucbhelper/content.hxx>
#include <sfx2/app.hxx>
#include "helpid.hrc"
#include "svx/gallery1.hxx"
#include "svx/galtheme.hxx"
#include "svx/galmisc.hxx"
#include "galbrws1.hxx"
#include <com/sun/star/util/DateTime.hpp>
#include "gallery.hrc"
#include <algorithm>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>

#include <svx/svxdlg.hxx>
#include <boost/scoped_ptr.hpp>

// - Namespaces -

using namespace ::com::sun::star;

// - GalleryButton -

GalleryButton::GalleryButton( GalleryBrowser1* pParent, WinBits nWinBits ) :
    PushButton( pParent, nWinBits )
{
}

GalleryButton::~GalleryButton()
{
}

void GalleryButton::KeyInput( const KeyEvent& rKEvt )
{
    if( !static_cast< GalleryBrowser1* >( GetParent() )->KeyInput( rKEvt, this ) )
        PushButton::KeyInput( rKEvt );
}

// - GalleryThemeListBox -

GalleryThemeListBox::GalleryThemeListBox( GalleryBrowser1* pParent, WinBits nWinBits ) :
    ListBox( pParent, nWinBits )
{
    InitSettings();
}

GalleryThemeListBox::~GalleryThemeListBox()
{
}

void GalleryThemeListBox::InitSettings()
{
    SetBackground( Wallpaper( GALLERY_BG_COLOR ) );
    SetControlBackground( GALLERY_BG_COLOR );
    SetControlForeground( GALLERY_FG_COLOR );
}

void GalleryThemeListBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
        InitSettings();
    else
        ListBox::DataChanged( rDCEvt );
}

bool GalleryThemeListBox::PreNotify( NotifyEvent& rNEvt )
{
    bool bDone = false;

    if( rNEvt.GetType() == EVENT_COMMAND )
    {
        const CommandEvent* pCEvt = rNEvt.GetCommandEvent();

        if( pCEvt && pCEvt->GetCommand() == COMMAND_CONTEXTMENU )
            static_cast< GalleryBrowser1* >( GetParent() )->ShowContextMenu();
    }
    else if( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();

        if( pKEvt )
            bDone = static_cast< GalleryBrowser1* >( GetParent() )->KeyInput( *pKEvt, this );
    }

    return( bDone || ListBox::PreNotify( rNEvt ) );
}

// - GalleryBrowser1 -

GalleryBrowser1::GalleryBrowser1(
    vcl::Window* pParent,
    const ResId& rResId,
    Gallery* pGallery,
    const ::boost::function<sal_Bool(const KeyEvent&,Window*)>& rKeyInputHandler,
    const ::boost::function<void(void)>& rThemeSlectionHandler)
    :
    Control               ( pParent, rResId ),
    maNewTheme            ( this, WB_3DLOOK ),
    mpThemes              ( new GalleryThemeListBox( this, WB_TABSTOP | WB_3DLOOK | WB_BORDER | WB_HSCROLL | WB_VSCROLL | WB_AUTOHSCROLL | WB_SORT ) ),
    mpGallery             ( pGallery ),
    mpExchangeData        ( new ExchangeData ),
    mpThemePropsDlgItemSet( NULL ),
    aImgNormal            ( GalleryResGetBitmapEx( RID_SVXBMP_THEME_NORMAL ) ),
    aImgDefault           ( GalleryResGetBitmapEx( RID_SVXBMP_THEME_DEFAULT ) ),
    aImgReadOnly          ( GalleryResGetBitmapEx( RID_SVXBMP_THEME_READONLY ) ),
    maKeyInputHandler(rKeyInputHandler),
    maThemeSlectionHandler(rThemeSlectionHandler)
{
    StartListening( *mpGallery );

    maNewTheme.SetHelpId( HID_GALLERY_NEWTHEME );
    maNewTheme.SetText( GAL_RESSTR(RID_SVXSTR_GALLERY_CREATETHEME));
    maNewTheme.SetClickHdl( LINK( this, GalleryBrowser1, ClickNewThemeHdl ) );

    // disable creation of new themes if a writable directory is not available
    if( mpGallery->GetUserURL().GetProtocol() == INET_PROT_NOT_VALID )
        maNewTheme.Disable();

    mpThemes->SetHelpId( HID_GALLERY_THEMELIST );
    mpThemes->SetSelectHdl( LINK( this, GalleryBrowser1, SelectThemeHdl ) );
    mpThemes->SetAccessibleName(SVX_RESSTR(RID_SVXSTR_GALLERYPROPS_GALTHEME));

    for( sal_uIntPtr i = 0, nCount = mpGallery->GetThemeCount(); i < nCount; i++ )
        ImplInsertThemeEntry( mpGallery->GetThemeInfo( i ) );

    ImplAdjustControls();
    maNewTheme.Show( true );
    mpThemes->Show( true );
}

GalleryBrowser1::~GalleryBrowser1()
{
    EndListening( *mpGallery );
    delete mpThemes;
    mpThemes = NULL;
    delete mpExchangeData;
    mpExchangeData = NULL;
}

sal_uIntPtr GalleryBrowser1::ImplInsertThemeEntry( const GalleryThemeEntry* pEntry )
{
    static const bool bShowHiddenThemes = ( getenv( "GALLERY_SHOW_HIDDEN_THEMES" ) != NULL );

    sal_uIntPtr nRet = LISTBOX_ENTRY_NOTFOUND;

    if( pEntry && ( !pEntry->IsHidden() || bShowHiddenThemes ) )
    {
        const Image* pImage;

        if( pEntry->IsReadOnly() )
            pImage = &aImgReadOnly;
        else if( pEntry->IsDefault() )
            pImage = &aImgDefault;
        else
            pImage = &aImgNormal;

        nRet = mpThemes->InsertEntry( pEntry->GetThemeName(), *pImage );
    }

    return nRet;
}

void GalleryBrowser1::ImplAdjustControls()
{
    const Size  aOutSize( GetOutputSizePixel() );
    const long  nNewThemeHeight = LogicToPixel( Size( 0, 14 ), MAP_APPFONT ).Height();
    const long  nStartY = nNewThemeHeight + 4;

    maNewTheme.SetPosSizePixel( Point(),
                                Size( aOutSize.Width(), nNewThemeHeight ) );

    mpThemes->SetPosSizePixel( Point( 0, nStartY ),
                               Size( aOutSize.Width(), aOutSize.Height() - nStartY ) );
}

void GalleryBrowser1::ImplFillExchangeData( const GalleryTheme* pThm, ExchangeData& rData )
{
    rData.pTheme = (GalleryTheme*) pThm;
    rData.aEditedTitle = pThm->GetName();

    try
    {
        ::ucbhelper::Content aCnt( pThm->GetThmURL().GetMainURL( INetURLObject::NO_DECODE ), uno::Reference< ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
        util::DateTime  aDateTimeModified;
        DateTime        aDateTime( DateTime::EMPTY );

        aCnt.getPropertyValue("DateModified") >>= aDateTimeModified;
        ::utl::typeConvert( aDateTimeModified, aDateTime );
        rData.aThemeChangeDate = aDateTime;
        rData.aThemeChangeTime = aDateTime;
    }
    catch( const ucb::ContentCreationException& )
    {
    }
    catch( const uno::RuntimeException& )
    {
    }
    catch( const uno::Exception& )
    {
    }
}

void GalleryBrowser1::ImplGetExecuteVector(::std::vector< sal_uInt16 >& o_aExec)
{
    GalleryTheme*           pTheme = mpGallery->AcquireTheme( GetSelectedTheme(), *this );

    if( pTheme )
    {
        bool                bUpdateAllowed, bRenameAllowed, bRemoveAllowed;
        static const bool   bIdDialog = ( getenv( "GALLERY_ENABLE_ID_DIALOG" ) != NULL );

        if( pTheme->IsReadOnly() )
            bUpdateAllowed = bRenameAllowed = bRemoveAllowed = false;
        else if( pTheme->IsDefault() )
        {
            bUpdateAllowed = bRenameAllowed = true;
            bRemoveAllowed = false;
        }
        else
            bUpdateAllowed = bRenameAllowed = bRemoveAllowed = true;

        if( bUpdateAllowed && pTheme->GetObjectCount() )
            o_aExec.push_back( MN_ACTUALIZE );

        if( bRenameAllowed )
            o_aExec.push_back( MN_RENAME );

        if( bRemoveAllowed )
            o_aExec.push_back( MN_DELETE );

        if( bIdDialog && !pTheme->IsReadOnly() )
            o_aExec.push_back( MN_ASSIGN_ID );

        o_aExec.push_back( MN_PROPERTIES );

        mpGallery->ReleaseTheme( pTheme, *this );
    }
}

void GalleryBrowser1::ImplGalleryThemeProperties( const OUString & rThemeName, bool bCreateNew )
{
    DBG_ASSERT(!mpThemePropsDlgItemSet, "mpThemePropsDlgItemSet already set!");
    mpThemePropsDlgItemSet = new SfxItemSet( SfxGetpApp()->GetPool() );
    GalleryTheme*   pTheme = mpGallery->AcquireTheme( rThemeName, *this );

    ImplFillExchangeData( pTheme, *mpExchangeData );

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    assert(pFact && "Got no AbstractDialogFactory!");
    VclAbstractDialog2* pThemeProps = pFact->CreateGalleryThemePropertiesDialog( NULL, mpExchangeData, mpThemePropsDlgItemSet );
    assert(pThemeProps && "Got no GalleryThemePropertiesDialog!");

    if ( bCreateNew )
    {
        pThemeProps->StartExecuteModal(
            LINK( this, GalleryBrowser1, EndNewThemePropertiesDlgHdl ) );
    }
    else
    {
        pThemeProps->StartExecuteModal(
            LINK( this, GalleryBrowser1, EndThemePropertiesDlgHdl ) );
    }
}

void GalleryBrowser1::ImplEndGalleryThemeProperties( VclAbstractDialog2* pDialog, bool bCreateNew )
{
    long nRet = pDialog->GetResult();

    if( nRet == RET_OK )
    {
        OUString aName( mpExchangeData->pTheme->GetName() );

        if( !mpExchangeData->aEditedTitle.isEmpty() && aName != mpExchangeData->aEditedTitle )
        {
            const OUString      aOldName( aName );
            OUString            aTitle( mpExchangeData->aEditedTitle );
            sal_uInt16          nCount = 0;

            while( mpGallery->HasTheme( aTitle ) && ( nCount++ < 16000 ) )
            {
                aTitle = mpExchangeData->aEditedTitle;
                aTitle += " ";
                aTitle += OUString::number( nCount );
            }

            mpGallery->RenameTheme( aOldName, aTitle );
        }

        if ( bCreateNew )
        {
            mpThemes->SelectEntry( mpExchangeData->pTheme->GetName() );
            SelectThemeHdl( NULL );
        }
    }

    OUString aThemeName( mpExchangeData->pTheme->GetName() );
    mpGallery->ReleaseTheme( mpExchangeData->pTheme, *this );

    if ( bCreateNew && ( nRet != RET_OK ) )
    {
        mpGallery->RemoveTheme( aThemeName );
    }

    // destroy mpThemeProps asynchronously
    Application::PostUserEvent( LINK( this, GalleryBrowser1, DestroyThemePropertiesDlgHdl ), pDialog );
}

IMPL_LINK( GalleryBrowser1, EndNewThemePropertiesDlgHdl, VclAbstractDialog2*, pDialog )
{
    ImplEndGalleryThemeProperties( pDialog, true );
    return 0L;
}

IMPL_LINK( GalleryBrowser1, EndThemePropertiesDlgHdl, VclAbstractDialog2*, pDialog )
{
    ImplEndGalleryThemeProperties( pDialog, false );
    return 0L;
}

IMPL_LINK( GalleryBrowser1, DestroyThemePropertiesDlgHdl, VclAbstractDialog2*, pDialog )
{
    delete pDialog;
    delete mpThemePropsDlgItemSet;
    mpThemePropsDlgItemSet = 0;
    return 0L;
}

void GalleryBrowser1::ImplExecute( sal_uInt16 nId )
{
    switch( nId )
    {
        case( MN_ACTUALIZE ):
        {
            GalleryTheme*       pTheme = mpGallery->AcquireTheme( GetSelectedTheme(), *this );

            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            if(pFact)
            {
                boost::scoped_ptr<VclAbstractRefreshableDialog> aActualizeProgress(pFact->CreateActualizeProgressDialog( this, pTheme ));
                DBG_ASSERT(aActualizeProgress, "Dialog creation failed!");

                aActualizeProgress->Update();
                aActualizeProgress->Execute();
                mpGallery->ReleaseTheme( pTheme, *this );
            }
        }
        break;

        case( MN_DELETE  ):
        {
            if( MessageDialog( NULL, "QueryDeleteThemeDialog","svx/ui/querydeletethemedialog.ui").Execute() == RET_YES )
                mpGallery->RemoveTheme( mpThemes->GetSelectEntry() );
        }
        break;

        case( MN_RENAME ):
        {
            GalleryTheme*   pTheme = mpGallery->AcquireTheme( GetSelectedTheme(), *this );
            const OUString  aOldName( pTheme->GetName() );

            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            DBG_ASSERT(pFact, "Dialog creation failed!");
            boost::scoped_ptr<AbstractTitleDialog> aDlg(pFact->CreateTitleDialog( this, aOldName ));
            DBG_ASSERT(aDlg, "Dialog creation failed!");

            if( aDlg->Execute() == RET_OK )
            {
                const OUString aNewName( aDlg->GetTitle() );

                if( !aNewName.isEmpty() && ( aNewName != aOldName ) )
                {
                    OUString  aName( aNewName );
                    sal_uInt16  nCount = 0;

                    while( mpGallery->HasTheme( aName ) && ( nCount++ < 16000 ) )
                    {
                        aName = aNewName;
                        aName += " ";
                        aName += OUString::number( nCount );
                    }

                    mpGallery->RenameTheme( aOldName, aName );
                }
            }
            mpGallery->ReleaseTheme( pTheme, *this );
        }
        break;

        case( MN_ASSIGN_ID ):
        {
            GalleryTheme* pTheme = mpGallery->AcquireTheme( GetSelectedTheme(), *this );

            if (pTheme && !pTheme->IsReadOnly())
            {

                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if(pFact)
                {
                    boost::scoped_ptr<AbstractGalleryIdDialog> aDlg(pFact->CreateGalleryIdDialog( this, pTheme ));
                    DBG_ASSERT(aDlg, "Dialog creation failed!");

                    if( aDlg->Execute() == RET_OK )
                        pTheme->SetId( aDlg->GetId(), true );
                }
            }

            mpGallery->ReleaseTheme( pTheme, *this );
        }
        break;

        case( MN_PROPERTIES ):
        {
            ImplGalleryThemeProperties( GetSelectedTheme(), false );
        }
        break;
    }
}

void GalleryBrowser1::Resize()
{
    Control::Resize();
    ImplAdjustControls();
}

void GalleryBrowser1::GetFocus()
{
    Control::GetFocus();
    if( mpThemes )
        mpThemes->GrabFocus();
}

void GalleryBrowser1::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    const GalleryHint& rGalleryHint = static_cast<const GalleryHint&>(rHint);

    switch( rGalleryHint.GetType() )
    {
        case( GALLERY_HINT_THEME_CREATED ):
            ImplInsertThemeEntry( mpGallery->GetThemeInfo( rGalleryHint.GetThemeName() ) );
        break;

        case( GALLERY_HINT_THEME_RENAMED ):
        {
            const sal_uInt16 nCurSelectPos = mpThemes->GetSelectEntryPos();
            const sal_uInt16 nRenameEntryPos = mpThemes->GetEntryPos( rGalleryHint.GetThemeName() );

            mpThemes->RemoveEntry( rGalleryHint.GetThemeName() );
            ImplInsertThemeEntry( mpGallery->GetThemeInfo( rGalleryHint.GetStringData() ) );

            if( nCurSelectPos == nRenameEntryPos )
            {
                mpThemes->SelectEntry( rGalleryHint.GetStringData() );
                SelectThemeHdl( NULL );
            }
        }
        break;

        case( GALLERY_HINT_THEME_REMOVED ):
        {
            mpThemes->RemoveEntry( rGalleryHint.GetThemeName() );
        }
        break;

        case( GALLERY_HINT_CLOSE_THEME ):
        {
            const sal_uInt16 nCurSelectPos = mpThemes->GetSelectEntryPos();
            const sal_uInt16 nCloseEntryPos = mpThemes->GetEntryPos( rGalleryHint.GetThemeName() );

            if( nCurSelectPos == nCloseEntryPos )
            {
                if( nCurSelectPos < ( mpThemes->GetEntryCount() - 1 ) )
                    mpThemes->SelectEntryPos( nCurSelectPos + 1 );
                else if( nCurSelectPos )
                    mpThemes->SelectEntryPos( nCurSelectPos - 1 );
                else
                    mpThemes->SetNoSelection();

                SelectThemeHdl( NULL );
            }
        }
        break;

        default:
        break;
    }
}

void GalleryBrowser1::ShowContextMenu()
{
    Application::PostUserEvent( LINK( this, GalleryBrowser1, ShowContextMenuHdl ), this );
}

bool GalleryBrowser1::KeyInput( const KeyEvent& rKEvt, vcl::Window* pWindow )
{
    bool bRet = false;
    if (maKeyInputHandler)
        bRet = maKeyInputHandler(rKEvt, pWindow);

    if( !bRet )
    {
        ::std::vector< sal_uInt16 > aExecVector;
        ImplGetExecuteVector(aExecVector);
        sal_uInt16                  nExecuteId = 0;
        bool                    bMod1 = rKEvt.GetKeyCode().IsMod1();

        switch( rKEvt.GetKeyCode().GetCode() )
        {
            case( KEY_INSERT ):
                ClickNewThemeHdl( NULL );
            break;

            case( KEY_I ):
            {
                if( bMod1 )
                   ClickNewThemeHdl( NULL );
            }
            break;

            case( KEY_U ):
            {
                if( bMod1 )
                    nExecuteId = MN_ACTUALIZE;
            }
            break;

            case( KEY_DELETE ):
                nExecuteId = MN_DELETE;
            break;

            case( KEY_D ):
            {
                if( bMod1 )
                    nExecuteId = MN_DELETE;
            }
            break;

            case( KEY_R ):
            {
                if( bMod1 )
                    nExecuteId = MN_RENAME;
            }
            break;

            case( KEY_RETURN ):
            {
                if( bMod1 )
                    nExecuteId = MN_PROPERTIES;
            }
            break;
        }

        if( nExecuteId && ( ::std::find( aExecVector.begin(), aExecVector.end(), nExecuteId ) != aExecVector.end() ) )
        {
            ImplExecute( nExecuteId );
            bRet = true;
        }
    }

    return bRet;
}

IMPL_LINK_NOARG(GalleryBrowser1, ShowContextMenuHdl)
{
    ::std::vector< sal_uInt16 > aExecVector;
    ImplGetExecuteVector(aExecVector);

    if( !aExecVector.empty() )
    {
        PopupMenu aMenu( GAL_RES( RID_SVXMN_GALLERY1 ) );

        aMenu.EnableItem( MN_ACTUALIZE, ::std::find( aExecVector.begin(), aExecVector.end(), MN_ACTUALIZE ) != aExecVector.end() );
        aMenu.EnableItem( MN_RENAME, ::std::find( aExecVector.begin(), aExecVector.end(), MN_RENAME ) != aExecVector.end() );
        aMenu.EnableItem( MN_DELETE, ::std::find( aExecVector.begin(), aExecVector.end(), MN_DELETE ) != aExecVector.end() );
        aMenu.EnableItem( MN_ASSIGN_ID, ::std::find( aExecVector.begin(), aExecVector.end(), MN_ASSIGN_ID ) != aExecVector.end() );
        aMenu.EnableItem( MN_PROPERTIES, ::std::find( aExecVector.begin(), aExecVector.end(), MN_PROPERTIES ) != aExecVector.end() );
        aMenu.SetSelectHdl( LINK( this, GalleryBrowser1, PopupMenuHdl ) );
        aMenu.RemoveDisabledEntries();

        const Rectangle aThemesRect( mpThemes->GetPosPixel(), mpThemes->GetOutputSizePixel() );
        Point           aSelPos( mpThemes->GetBoundingRectangle( mpThemes->GetSelectEntryPos() ).Center() );

        aSelPos.X() = std::max( std::min( aSelPos.X(), aThemesRect.Right() ), aThemesRect.Left() );
        aSelPos.Y() = std::max( std::min( aSelPos.Y(), aThemesRect.Bottom() ), aThemesRect.Top() );

        aMenu.Execute( this, aSelPos );
    }

    return 0L;
}

IMPL_LINK( GalleryBrowser1, PopupMenuHdl, Menu*, pMenu )
{
    ImplExecute( pMenu->GetCurItemId() );
    return 0L;
}

IMPL_LINK_NOARG(GalleryBrowser1, SelectThemeHdl)
{
    if (maThemeSlectionHandler)
        maThemeSlectionHandler();
    return 0L;
}

IMPL_LINK_NOARG(GalleryBrowser1, ClickNewThemeHdl)
{
    OUString  aNewTheme( GAL_RESSTR(RID_SVXSTR_GALLERY_NEWTHEME) );
    OUString  aName( aNewTheme );
    sal_uIntPtr nCount = 0;

    while( mpGallery->HasTheme( aName ) && ( nCount++ < 16000 ) )
    {
        aName = aNewTheme;
        aName += " ";
        aName += OUString::number( nCount );
    }

    if( !mpGallery->HasTheme( aName ) && mpGallery->CreateTheme( aName ) )
    {
        ImplGalleryThemeProperties( aName, true );
    }

    return 0L;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
