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

#include <algorithm>
#include <cassert>

#include <ucbhelper/content.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <avmedia/mediawindow.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/opengrf.hxx>
#include <vcl/graphicfilter.hxx>
#include <svx/gallery1.hxx>
#include <svx/galtheme.hxx>
#include "cuigaldlg.hxx"
#include "helpid.hrc"
#include <unotools/syslocale.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <com/sun/star/ui/dialogs/XAsynchronousExecutableDialog.hpp>
#include <sfx2/sfxuno.hxx>
#include "dialmgr.hxx"
#include "gallery.hrc"
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>



// - Namespaces -


using namespace ::ucbhelper;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;


// - SearchThread -


SearchThread::SearchThread( SearchProgress* pProgess,
                            TPGalleryThemeProperties* pBrowser,
                            const INetURLObject& rStartURL ) :
        Thread      ( "cuiSearchThread" ),
        mpProgress  ( pProgess ),
        mpBrowser   ( pBrowser ),
        maStartURL  ( rStartURL )
{
}



SearchThread::~SearchThread()
{
}



void SearchThread::execute()
{
    const OUString aFileType( mpBrowser->aCbbFileType.GetText() );

    if( !aFileType.isEmpty() )
    {
        const sal_uInt16        nFileNumber = mpBrowser->aCbbFileType.GetEntryPos( aFileType );
        sal_uInt16              nBeginFormat, nEndFormat;
        ::std::vector< OUString > aFormats;

        if( !nFileNumber || ( nFileNumber >= mpBrowser->aCbbFileType.GetEntryCount() ) )
        {
            nBeginFormat = 1;
            nEndFormat = mpBrowser->aCbbFileType.GetEntryCount() - 1;
        }
        else
            nBeginFormat = nEndFormat = nFileNumber;

        for( sal_uInt16 i = nBeginFormat; i <= nEndFormat; ++i )
            aFormats.push_back( mpBrowser->aFilterEntryList[ i ]->aFilterName.toAsciiLowerCase() );

        ImplSearch( maStartURL, aFormats, mpBrowser->bSearchRecursive );
    }

    Application::PostUserEvent( LINK( mpProgress, SearchProgress, CleanUpHdl ) );
}



void SearchThread::ImplSearch( const INetURLObject& rStartURL,
                               const ::std::vector< OUString >& rFormats,
                               sal_Bool bRecursive )
{
    {
        SolarMutexGuard aGuard;

        mpProgress->SetDirectory( rStartURL );
        mpProgress->Sync();
    }

    try
    {
        ::com::sun::star::uno::Reference< XCommandEnvironment > xEnv;
        Content aCnt( rStartURL.GetMainURL( INetURLObject::NO_DECODE ), xEnv, comphelper::getProcessComponentContext() );
        Sequence< OUString > aProps( 2 );

        aProps.getArray()[ 0 ] = "IsFolder";
        aProps.getArray()[ 1 ] = "IsDocument";
        ::com::sun::star::uno::Reference< XResultSet > xResultSet(
            aCnt.createCursor( aProps, INCLUDE_FOLDERS_AND_DOCUMENTS ) );

        if( xResultSet.is() )
        {
            ::com::sun::star::uno::Reference< XContentAccess > xContentAccess( xResultSet, UNO_QUERY_THROW );
            ::com::sun::star::uno::Reference< XRow > xRow( xResultSet, UNO_QUERY_THROW );

            while( xResultSet->next() && schedule() )
            {
                INetURLObject   aFoundURL( xContentAccess->queryContentIdentifierString() );
                DBG_ASSERT( aFoundURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

                sal_Bool bFolder = xRow->getBoolean( 1 ); // property "IsFolder"
                if ( xRow->wasNull() )
                    bFolder = sal_False;

                if( bRecursive && bFolder )
                    ImplSearch( aFoundURL, rFormats, sal_True );
                else
                {
                    sal_Bool bDocument = xRow->getBoolean( 2 ); // property "IsDocument"
                    if ( xRow->wasNull() )
                        bDocument = sal_False;

                    if( bDocument )
                    {
                        GraphicDescriptor   aDesc( aFoundURL );

                        if( ( aDesc.Detect() &&
                              ::std::find( rFormats.begin(),
                                           rFormats.end(),
                                           aDesc.GetImportFormatShortName(
                                               aDesc.GetFileFormat() ).toAsciiLowerCase() )
                              != rFormats.end() ) ||
                            ::std::find( rFormats.begin(),
                                         rFormats.end(),
                                         aFoundURL.GetExtension().toAsciiLowerCase() )
                            != rFormats.end() )
                        {
                            SolarMutexGuard aGuard;

                            mpBrowser->aFoundList.push_back(
                                aFoundURL.GetMainURL( INetURLObject::NO_DECODE )
                            );
                            mpBrowser->aLbxFound.InsertEntry(
                                GetReducedString( aFoundURL, 50 ),
                                (sal_uInt16) mpBrowser->aFoundList.size() - 1 );
                        }
                    }
                }
            }
        }
    }
    catch (const ContentCreationException&)
    {
    }
    catch (const ::com::sun::star::uno::RuntimeException&)
    {
    }
    catch (const ::com::sun::star::uno::Exception&)
    {
    }
}


// - SearchProgress -


SearchProgress::SearchProgress( Window* pParent, const INetURLObject& rStartURL )
    : ModalDialog(pParent, "GallerySearchProgress", "cui/ui/gallerysearchprogress.ui")
    , parent_(pParent)
    , startUrl_(rStartURL)
{
    get(m_pFtSearchDir, "dir");
    get(m_pFtSearchType, "file");
    m_pFtSearchType->set_width_request(m_pFtSearchType->get_preferred_size().Width());
    get(m_pBtnCancel, "cancel");
    m_pBtnCancel->SetClickHdl( LINK( this, SearchProgress, ClickCancelBtn ) );
}



void SearchProgress::Terminate()
{
    if (maSearchThread.is())
        maSearchThread->terminate();
}



IMPL_LINK_NOARG(SearchProgress, ClickCancelBtn)
{
    Terminate();
    return 0L;
}



IMPL_LINK_NOARG(SearchProgress, CleanUpHdl)
{
    if (maSearchThread.is())
        maSearchThread->join();

    EndDialog( RET_OK );

    delete this;
    return 0L;
}



short SearchProgress::Execute()
{
    OSL_FAIL( "SearchProgress cannot be executed via Dialog::Execute!\n"
               "It creates a thread that will call back to VCL apartment => deadlock!\n"
               "Use Dialog::StartExecuteModal to execute the dialog!" );
    return RET_CANCEL;
}



void SearchProgress::StartExecuteModal( const Link& rEndDialogHdl )
{
    assert(!maSearchThread.is());
    maSearchThread = new SearchThread(
        this, static_cast< TPGalleryThemeProperties * >(parent_), startUrl_);
    maSearchThread->launch();
    ModalDialog::StartExecuteModal( rEndDialogHdl );
}


// - TakeThread -


TakeThread::TakeThread(
    TakeProgress* pProgess,
    TPGalleryThemeProperties* pBrowser,
    TokenList_impl& rTakenList
) :
    Thread      ( "cuiTakeThread" ),
    mpProgress  ( pProgess ),
    mpBrowser   ( pBrowser ),
    mrTakenList ( rTakenList )
{
}



TakeThread::~TakeThread()
{
}



void TakeThread::execute()
{
    INetURLObject       aURL;
    sal_uInt16          nEntries;
    GalleryTheme*       pThm = mpBrowser->GetXChgData()->pTheme;
    sal_uInt16          nPos;
    GalleryProgress*    pStatusProgress;

    {
        SolarMutexGuard aGuard;
        pStatusProgress = new GalleryProgress;
        nEntries = mpBrowser->bTakeAll ? mpBrowser->aLbxFound.GetEntryCount() : mpBrowser->aLbxFound.GetSelectEntryCount();
        pThm->LockBroadcaster();
    }

    for( sal_uInt16 i = 0; i < nEntries && schedule(); i++ )
    {
        if( mpBrowser->bTakeAll )
            aURL = INetURLObject( mpBrowser->aFoundList[ nPos = i ] );
        else
            aURL = INetURLObject( mpBrowser->aFoundList[ nPos = mpBrowser->aLbxFound.GetSelectEntryPos( i ) ]);

        mrTakenList.push_back( (sal_uLong)nPos );

        {
            SolarMutexGuard aGuard;

            mpProgress->SetFile( aURL );
            pStatusProgress->Update( i, nEntries - 1 );
            mpProgress->Sync();
            pThm->InsertURL( aURL );
        }
    }

    {
        SolarMutexGuard aGuard;

        pThm->UnlockBroadcaster();
        delete pStatusProgress;
    }

    Application::PostUserEvent( LINK( mpProgress, TakeProgress, CleanUpHdl ) );
}


// - TakeProgress -


TakeProgress::TakeProgress( Window* pWindow ) :
    ModalDialog     ( pWindow, CUI_RES( RID_SVXDLG_GALLERY_TAKE_PROGRESS ) ),
    aFtTakeFile     ( this, CUI_RES( FT_TAKE_FILE ) ),
    aFLTakeProgress( this, CUI_RES( FL_TAKE_PROGRESS ) ),
    aBtnCancel      ( this, CUI_RES( BTN_CANCEL ) ),
    window_(pWindow)
{
    FreeResource();
    aBtnCancel.SetClickHdl( LINK( this, TakeProgress, ClickCancelBtn ) );
}




void TakeProgress::Terminate()
{
    if (maTakeThread.is())
        maTakeThread->terminate();
}



IMPL_LINK_NOARG(TakeProgress, ClickCancelBtn)
{
    Terminate();
    return 0L;
}



IMPL_LINK_NOARG(TakeProgress, CleanUpHdl)
{
    if (maTakeThread.is())
        maTakeThread->join();

    TPGalleryThemeProperties*   mpBrowser = (TPGalleryThemeProperties*) GetParent();
    ::std::vector<bool, std::allocator<bool> >           aRemoveEntries( mpBrowser->aFoundList.size(), false );
    ::std::vector< OUString >   aRemainingVector;
    sal_uInt32                  i, nCount;

    GetParent()->EnterWait();
    mpBrowser->aLbxFound.SetUpdateMode( false );
    mpBrowser->aLbxFound.SetNoSelection();

    // mark all taken positions in aRemoveEntries
    for( i = 0, nCount = maTakenList.size(); i < nCount; ++i )
        aRemoveEntries[ maTakenList[ i ] ] = true;
    maTakenList.clear();

    // refill found list
    for( i = 0, nCount = aRemoveEntries.size(); i < nCount; ++i )
        if( !aRemoveEntries[ i ] )
            aRemainingVector.push_back( mpBrowser->aFoundList[i] );

    mpBrowser->aFoundList.clear();

    for( i = 0, nCount = aRemainingVector.size(); i < nCount; ++i )
        mpBrowser->aFoundList.push_back( aRemainingVector[ i ] );

    aRemainingVector.clear();

    // refill list box
    for( i = 0, nCount = aRemoveEntries.size(); i < nCount; ++i )
        if( !aRemoveEntries[ i ] )
            aRemainingVector.push_back( mpBrowser->aLbxFound.GetEntry( (sal_uInt16) i ) );

    mpBrowser->aLbxFound.Clear();

    for( i = 0, nCount = aRemainingVector.size(); i < nCount; ++i )
        mpBrowser->aLbxFound.InsertEntry( aRemainingVector[ i ] );

    aRemainingVector.clear();

    mpBrowser->aLbxFound.SetUpdateMode( true );
    mpBrowser->SelectFoundHdl( NULL );
    GetParent()->LeaveWait();

    EndDialog( RET_OK );
    delete this;
    return 0L;
}



short TakeProgress::Execute()
{
    OSL_FAIL( "TakeProgress cannot be executed via Dialog::Execute!\n"
               "It creates a thread that will call back to VCL apartment => deadlock!\n"
               "Use Dialog::StartExecuteModal to execute the dialog!" );
    return RET_CANCEL;
}



void TakeProgress::StartExecuteModal( const Link& rEndDialogHdl )
{
    assert(!maTakeThread.is());
    maTakeThread = new TakeThread(
        this, static_cast< TPGalleryThemeProperties * >(window_), maTakenList);
    maTakeThread->launch();
    ModalDialog::StartExecuteModal( rEndDialogHdl );
}


// - ActualizeProgress -


ActualizeProgress::ActualizeProgress( Window* pWindow, GalleryTheme* pThm ) :
    ModalDialog             ( pWindow, CUI_RES( RID_SVXDLG_GALLERY_ACTUALIZE_PROGRESS ) ),
    aFtActualizeFile        ( this, CUI_RES( FT_ACTUALIZE_FILE ) ),
    aFLActualizeProgress   ( this, CUI_RES( FL_ACTUALIZE_PROGRESS ) ),
    aBtnCancel              ( this, CUI_RES( BTN_CANCEL ) ),
    pTimer(NULL),
    pTheme                  ( pThm )
{
    FreeResource();
    aBtnCancel.SetClickHdl( LINK( this, ActualizeProgress, ClickCancelBtn ) );
}



short ActualizeProgress::Execute()
{
    short nRet;

    pTimer = new Timer;

    if ( pTimer )
    {
        pTimer->SetTimeoutHdl( LINK( this, ActualizeProgress, TimeoutHdl ) );
        pTimer->SetTimeout( 500 );
        pTimer->Start();
    }

    nRet = ModalDialog::Execute();

    return nRet;
}



IMPL_LINK_NOARG(ActualizeProgress, ClickCancelBtn)
{
    pTheme->AbortActualize();
    EndDialog( RET_OK );

    return 0L;
}



IMPL_LINK( ActualizeProgress, TimeoutHdl, Timer*, _pTimer )
{
    if ( _pTimer )
    {
        _pTimer->Stop();
        delete _pTimer;
    }

    pTheme->Actualize( LINK( this, ActualizeProgress, ActualizeHdl ), &aStatusProgress );
    ClickCancelBtn( NULL );

    return 0;
}



IMPL_LINK( ActualizeProgress, ActualizeHdl, INetURLObject*, pURL )
{
    for( long i = 0; i < 128; i++ )
        Application::Reschedule();

    Flush();
    Sync();

    if( pURL )
    {
        aFtActualizeFile.SetText( GetReducedString( *pURL, 30 ) );
        aFtActualizeFile.Flush();
        aFtActualizeFile.Sync();
    }

    return 0;
}

TitleDialog::TitleDialog(Window* pParent, const OUString& rOldTitle)
    : ModalDialog(pParent, "GalleryTitleDialog", "cui/ui/gallerytitledialog.ui")
{
    get(m_pEdit, "entry");
    m_pEdit->SetText( rOldTitle );
    m_pEdit->GrabFocus();
}


// - GalleryIdDialog -


GalleryIdDialog::GalleryIdDialog( Window* pParent, GalleryTheme* _pThm )
    : ModalDialog(pParent, "GalleryThemeIDDialog", "cui/ui/gallerythemeiddialog.ui")
    , pThm(_pThm )
{
    get(m_pBtnOk, "ok");
    get(m_pLbResName, "entry");

    m_pLbResName->InsertEntry( OUString( "!!! No Id !!!" ) );

    GalleryTheme::InsertAllThemes(*m_pLbResName);

    m_pLbResName->SelectEntryPos( (sal_uInt16) pThm->GetId() );
    m_pLbResName->GrabFocus();

    m_pBtnOk->SetClickHdl( LINK( this, GalleryIdDialog, ClickOkHdl ) );
}



IMPL_LINK_NOARG(GalleryIdDialog, ClickOkHdl)
{
    Gallery*    pGal = pThm->GetParent();
    const sal_uLong nId = GetId();
    sal_Bool        bDifferentThemeExists = sal_False;

    for( sal_uLong i = 0, nCount = pGal->GetThemeCount(); i < nCount && !bDifferentThemeExists; i++ )
    {
        const GalleryThemeEntry* pInfo = pGal->GetThemeInfo( i );

        if( ( pInfo->GetId() == nId ) && ( pInfo->GetThemeName() != pThm->GetName() ) )
        {
            OUString aStr( CUI_RES( RID_SVXSTR_GALLERY_ID_EXISTS ) );

            aStr += " (";
            aStr += pInfo->GetThemeName();
            aStr += ")";

            InfoBox aBox( this, aStr );
            aBox.Execute();
            m_pLbResName->GrabFocus();
            bDifferentThemeExists = sal_True;
        }
    }

    if( !bDifferentThemeExists )
        EndDialog( RET_OK );

    return 0L;
}



// - GalleryThemeProperties -


GalleryThemeProperties::GalleryThemeProperties(Window* pParent,
    ExchangeData* _pData, SfxItemSet* pItemSet)
    : SfxTabDialog( pParent, "GalleryThemeDialog",
        "cui/ui/gallerythemedialog.ui", pItemSet)
    , pData(_pData)
    , m_nGeneralPageId(0)
    , m_nFilesPageId(0)
{
    m_nGeneralPageId = AddTabPage("general", TPGalleryThemeGeneral::Create, 0);
    m_nFilesPageId = AddTabPage("files", TPGalleryThemeProperties::Create, 0);

    if( pData->pTheme->IsReadOnly() )
        RemoveTabPage(m_nFilesPageId);

    OUString aText( GetText() );

    aText += pData->pTheme->GetName();

    if( pData->pTheme->IsReadOnly() )
        aText +=  CUI_RES( RID_SVXSTR_GALLERY_READONLY );

    SetText( aText );
}



void GalleryThemeProperties::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    if (nId == m_nGeneralPageId)
        ( (TPGalleryThemeGeneral&) rPage ).SetXChgData( pData );
    else
        ( (TPGalleryThemeProperties&) rPage ).SetXChgData( pData );
}


// - TPGalleryThemeGeneral -


TPGalleryThemeGeneral::TPGalleryThemeGeneral( Window* pParent, const SfxItemSet& rSet ) :
            SfxTabPage              ( pParent, CUI_RES( RID_SVXTABPAGE_GALLERY_GENERAL ), rSet ),
            aFiMSImage              ( this, CUI_RES( FI_MS_IMAGE ) ),
            aEdtMSName              ( this, CUI_RES( EDT_MS_NAME ) ),
            aFlMSGeneralFirst       ( this, CUI_RES( FL_MS_GENERAL_FIRST ) ),
            aFtMSType               ( this, CUI_RES( FT_MS_TYPE ) ),
            aFtMSShowType           ( this, CUI_RES( FT_MS_SHOW_TYPE ) ),
            aFtMSPath               ( this, CUI_RES( FT_MS_PATH ) ),
            aFtMSShowPath           ( this, CUI_RES( FT_MS_SHOW_PATH ) ),
            aFtMSContent            ( this, CUI_RES( FT_MS_CONTENT ) ),
            aFtMSShowContent        ( this, CUI_RES( FT_MS_SHOW_CONTENT ) ),
            aFlMSGeneralSecond      ( this, CUI_RES( FL_MS_GENERAL_SECOND ) ),
            aFtMSChangeDate         ( this, CUI_RES( FT_MS_CHANGEDATE ) ),
            aFtMSShowChangeDate     ( this, CUI_RES( FT_MS_SHOW_CHANGEDATE ) ),
            pData(NULL)
{
    FreeResource();

    OUString aAccName(SVX_RES(RID_SVXSTR_GALLERY_THEMENAME));
    aEdtMSName.SetAccessibleName(aAccName);
    aFiMSImage.SetAccessibleName(aAccName);
    aEdtMSName.SetAccessibleRelationLabeledBy( &aFiMSImage );
}



void TPGalleryThemeGeneral::SetXChgData( ExchangeData* _pData )
{
    pData = _pData;

    GalleryTheme*       pThm = pData->pTheme;
    OUString            aOutStr( OUString::number(pThm->GetObjectCount()) );
    OUString            aObjStr( CUI_RES( RID_SVXSTR_GALLERYPROPS_OBJECT ) );
    OUString            aAccess;
    OUString            aType( SVX_RES( RID_SVXSTR_GALLERYPROPS_GALTHEME ) );
    sal_Bool            bReadOnly = pThm->IsReadOnly();

    aEdtMSName.SetHelpId( HID_GALLERY_EDIT_MSNAME );
    aEdtMSName.SetText( pThm->GetName() );
    aEdtMSName.SetReadOnly( bReadOnly );

    if( bReadOnly )
        aEdtMSName.Disable();
    else
        aEdtMSName.Enable();

    if( pThm->IsReadOnly() )
        aType += CUI_RES( RID_SVXSTR_GALLERY_READONLY );

    aFtMSShowType.SetText( aType );
    aFtMSShowPath.SetText( pThm->GetSdgURL().GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS ) );

    // singular or plural?
    if ( 1 == pThm->GetObjectCount() )
        aObjStr = aObjStr.getToken( 0, ';' );
    else
        aObjStr = aObjStr.getToken( 1, ';' );

    aOutStr += " " + aObjStr;

    aFtMSShowContent.SetText( aOutStr );

    // get locale wrapper (singleton)
    const SvtSysLocale aSysLocale;
    const LocaleDataWrapper&    aLocaleData = aSysLocale.GetLocaleData();

    // ChangeDate/Time
    aAccess = aLocaleData.getDate( pData->aThemeChangeDate ) + ", " + aLocaleData.getTime( pData->aThemeChangeTime );
    aFtMSShowChangeDate.SetText( aAccess );

    // set image
    sal_uInt16 nId;

    if( pThm->IsReadOnly() )
        nId = RID_SVXBMP_THEME_READONLY_BIG;
    else if( pThm->IsDefault() )
        nId = RID_SVXBMP_THEME_DEFAULT_BIG;
    else
        nId = RID_SVXBMP_THEME_NORMAL_BIG;

    aFiMSImage.SetImage( Image( Bitmap( CUI_RES( nId ) ), COL_LIGHTMAGENTA ) );
}



sal_Bool TPGalleryThemeGeneral::FillItemSet( SfxItemSet& /*rSet*/ )
{
    pData->aEditedTitle = aEdtMSName.GetText();
    return sal_True;
}



SfxTabPage* TPGalleryThemeGeneral::Create( Window* pParent, const SfxItemSet& rSet )
{
    return new TPGalleryThemeGeneral( pParent, rSet );
}


// - TPGalleryThemeProperties -


TPGalleryThemeProperties::TPGalleryThemeProperties( Window* pWindow, const SfxItemSet& rSet ) :
        SfxTabPage          ( pWindow, CUI_RES( RID_SVXTABPAGE_GALLERYTHEME_FILES ), rSet ),
        aFtFileType         ( this, CUI_RES(FT_FILETYPE ) ),
        aCbbFileType        ( this, CUI_RES(CBB_FILETYPE ) ),
        aLbxFound           ( this, CUI_RES(LBX_FOUND ) ),
        aBtnSearch          ( this, CUI_RES(BTN_SEARCH ) ),
        aBtnTake            ( this, CUI_RES(BTN_TAKE ) ),
        aBtnTakeAll         ( this, CUI_RES(BTN_TAKEALL ) ),
        aCbxPreview         ( this, CUI_RES(CBX_PREVIEW ) ),
        aWndPreview         ( this, CUI_RES( WND_BRSPRV ) ),
        nCurFilterPos       (0),
        nFirstExtFilterPos  (0),
        bEntriesFound       (sal_False),
        bInputAllowed       (sal_True),
        bSearchRecursive    (sal_False),
        xDialogListener     ( new ::svt::DialogClosedListener() )
{
    FreeResource();

    xDialogListener->SetDialogClosedLink( LINK( this, TPGalleryThemeProperties, DialogClosedHdl ) );
    aLbxFound.SetAccessibleName(OUString(SVX_RES(RID_SVXSTR_GALLERY_FILESFOUND)));
    aWndPreview.SetAccessibleName(aCbxPreview.GetText());
    aLbxFound.SetAccessibleRelationLabeledBy(&aLbxFound);
}



void TPGalleryThemeProperties::SetXChgData( ExchangeData* _pData )
{
    pData = _pData;

    aPreviewTimer.SetTimeoutHdl( LINK( this, TPGalleryThemeProperties, PreviewTimerHdl ) );
    aPreviewTimer.SetTimeout( 500 );
    aBtnSearch.SetClickHdl(LINK(this, TPGalleryThemeProperties, ClickSearchHdl));
    aBtnTake.SetClickHdl(LINK(this, TPGalleryThemeProperties, ClickTakeHdl));
    aBtnTakeAll.SetClickHdl(LINK(this, TPGalleryThemeProperties, ClickTakeAllHdl));
    aCbxPreview.SetClickHdl(LINK(this, TPGalleryThemeProperties, ClickPreviewHdl));
    aCbbFileType.SetSelectHdl(LINK(this, TPGalleryThemeProperties, SelectFileTypeHdl));
    aCbbFileType.EnableDDAutoWidth( false );
    aLbxFound.SetDoubleClickHdl(LINK(this, TPGalleryThemeProperties, DClickFoundHdl));
    aLbxFound.SetSelectHdl(LINK(this, TPGalleryThemeProperties, SelectFoundHdl));
    aLbxFound.InsertEntry(OUString(CUI_RES(RID_SVXSTR_GALLERY_NOFILES)));
    aLbxFound.Show();

    FillFilterList();

    aBtnTake.Enable();
    aBtnTakeAll.Disable();
    aCbxPreview.Disable();
}



void TPGalleryThemeProperties::StartSearchFiles( const OUString& _rFolderURL, short _nDlgResult )
{
    if ( RET_OK == _nDlgResult )
    {
        aURL = INetURLObject( _rFolderURL );
        bSearchRecursive = sal_True;    // UI choice no longer possible, windows file picker allows no user controls
        SearchFiles();
    }

    nCurFilterPos = aCbbFileType.GetEntryPos( aCbbFileType.GetText() );
}



TPGalleryThemeProperties::~TPGalleryThemeProperties()
{
    xMediaPlayer.clear();
    xDialogListener.clear();

    for ( size_t i = 0, n = aFilterEntryList.size(); i < n; ++i ) {
        delete aFilterEntryList[ i ];
    }
}



SfxTabPage* TPGalleryThemeProperties::Create( Window* pParent, const SfxItemSet& rSet )
{
    return new TPGalleryThemeProperties( pParent, rSet );
}



OUString TPGalleryThemeProperties::addExtension( const OUString& _rDisplayText, const OUString& _rExtension )
{
    OUString sAllFilter( "(*.*)" );
    OUString sOpenBracket( " (" );
    OUString sCloseBracket( ")" );
    OUString sRet = _rDisplayText;

    if ( sRet.indexOf( sAllFilter ) == -1 )
    {
        OUString sExt = _rExtension;
        sRet += sOpenBracket;
        sRet += sExt;
        sRet += sCloseBracket;
    }
    return sRet;
}



void TPGalleryThemeProperties::FillFilterList()
{
    GraphicFilter &rFilter = GraphicFilter::GetGraphicFilter();
    OUString            aExt;
    OUString            aName;
    FilterEntry*        pFilterEntry;
    FilterEntry*        pTestEntry;
    sal_uInt16          i, nKeyCount;
    sal_Bool                bInList;

    // graphic filters
    for( i = 0, nKeyCount = rFilter.GetImportFormatCount(); i < nKeyCount; i++ )
    {
        aExt = rFilter.GetImportFormatShortName( i );
        aName = rFilter.GetImportFormatName( i );
        size_t entryIndex = 0;
        pTestEntry = aFilterEntryList.empty() ? NULL : aFilterEntryList[ entryIndex ];
        bInList = sal_False;

        OUString aExtensions;
        int j = 0;
        OUString sWildcard;
        while( true )
        {
            sWildcard = rFilter.GetImportWildcard( i, j++ );
            if ( sWildcard.isEmpty() )
                break;
            if ( aExtensions.indexOf( sWildcard ) == -1 )
            {
                if ( !aExtensions.isEmpty() )
                    aExtensions += ";";
                aExtensions += sWildcard;
            }
        }
        aName = addExtension( aName, aExtensions );

        while( pTestEntry )
        {
            if ( pTestEntry->aFilterName == aExt )
            {
                bInList = sal_True;
                break;
            }
            pTestEntry = ( ++entryIndex < aFilterEntryList.size() )
                       ? aFilterEntryList[ entryIndex ] : NULL;
        }
        if ( !bInList )
        {
            pFilterEntry = new FilterEntry;
            pFilterEntry->aFilterName = aExt;
            size_t pos = aCbbFileType.InsertEntry( aName );
            if ( pos < aFilterEntryList.size() ) {
                aFilterEntryList.insert( aFilterEntryList.begin() + pos, pFilterEntry );
            } else {
                aFilterEntryList.push_back( pFilterEntry );
            }
        }
    }

    // media filters
    static const OUString aWildcard( "*." );
    ::avmedia::FilterNameVector     aFilters;
    ::avmedia::MediaWindow::getMediaFilters( aFilters );

    for( unsigned long l = 0; l < aFilters.size(); ++l )
    {
        for( sal_Int32 nIndex = 0; nIndex >= 0; )
        {
            OUString aFilterWildcard( aWildcard );

            pFilterEntry = new FilterEntry;
            pFilterEntry->aFilterName = aFilters[ l ].second.getToken( 0, ';', nIndex );
            nFirstExtFilterPos = aCbbFileType.InsertEntry(
                addExtension(
                    aFilters[ l ].first,
                    aFilterWildcard += pFilterEntry->aFilterName
                )
            );
            if ( nFirstExtFilterPos < aFilterEntryList.size() ) {
                aFilterEntryList.insert(
                    aFilterEntryList.begin() + nFirstExtFilterPos,
                    pFilterEntry
                );
            } else {
                aFilterEntryList.push_back( pFilterEntry );
            }
        }
    }

    // 'All' filters
    OUString aExtensions;

    // graphic filters
    for ( i = 0; i < nKeyCount; ++i )
    {
        int j = 0;
        OUString sWildcard;
        while( true )
        {
            sWildcard = rFilter.GetImportWildcard( i, j++ );
            if ( sWildcard.isEmpty() )
                break;
            if ( aExtensions.indexOf( sWildcard ) == -1 )
            {
                if ( !aExtensions.isEmpty() )
                    aExtensions += ";";

                aExtensions += sWildcard;
            }
        }
    }

    // media filters
    for( unsigned long k = 0; k < aFilters.size(); ++k )
    {
        for( sal_Int32 nIndex = 0; nIndex >= 0; )
        {
            if ( !aExtensions.isEmpty() )
                aExtensions += ";";
            aExtensions += aWildcard + aFilters[ k ].second.getToken( 0, ';', nIndex );
        }
     }

#if defined(WNT)
    if (aExtensions.getLength() > 240)
        aExtensions = "*.*";
#endif

    pFilterEntry = new FilterEntry;
    pFilterEntry->aFilterName = CUI_RESSTR(RID_SVXSTR_GALLERY_ALLFILES);
    pFilterEntry->aFilterName = addExtension( pFilterEntry->aFilterName, aExtensions );
    size_t pos = aCbbFileType.InsertEntry( pFilterEntry->aFilterName, 0 );
    if ( pos < aFilterEntryList.size() ) {
        aFilterEntryList.insert( aFilterEntryList.begin() + pos, pFilterEntry );
    } else {
        aFilterEntryList.push_back( pFilterEntry );
    }
    aCbbFileType.SetText( pFilterEntry->aFilterName );
}



IMPL_LINK_NOARG(TPGalleryThemeProperties, SelectFileTypeHdl)
{
    OUString aText( aCbbFileType.GetText() );

    if( bInputAllowed && ( aLastFilterName != aText ) )
    {
        aLastFilterName = aText;

        if( MessageDialog( this, "QueryUpdateFileListDialog","cui/ui/queryupdategalleryfilelistdialog.ui" ).Execute() == RET_YES )
            SearchFiles();
    }

    return 0L;
}



void TPGalleryThemeProperties::SearchFiles()
{
    SearchProgress* pProgress = new SearchProgress( this, aURL );

    aFoundList.clear();
    aLbxFound.Clear();

    pProgress->SetFileType( aCbbFileType.GetText() );
    pProgress->SetDirectory( INetURLObject() );
    pProgress->Update();

    pProgress->StartExecuteModal( LINK( this, TPGalleryThemeProperties, EndSearchProgressHdl ) );
}



IMPL_LINK_NOARG(TPGalleryThemeProperties, ClickSearchHdl)
{
    if( bInputAllowed )
    {
        try
        {
            // setup folder picker
            com::sun::star::uno::Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
            xFolderPicker = FolderPicker::create(xContext);

            OUString  aDlgPathName( SvtPathOptions().GetGraphicPath() );
            xFolderPicker->setDisplayDirectory(aDlgPathName);

            aPreviewTimer.Stop();

            com::sun::star::uno::Reference< XAsynchronousExecutableDialog > xAsyncDlg( xFolderPicker, UNO_QUERY );
            if ( xAsyncDlg.is() )
                xAsyncDlg->startExecuteModal( xDialogListener.get() );
            else
            {
                if( xFolderPicker->execute() == RET_OK )
                {
                    aURL = INetURLObject( xFolderPicker->getDirectory() );
                    bSearchRecursive = sal_True;    // UI choice no longer possible, windows file picker allows no user controls
                    SearchFiles();
                }

                nCurFilterPos = aCbbFileType.GetEntryPos( aCbbFileType.GetText() );
            }
        }
        catch (const IllegalArgumentException&)
        {
            OSL_FAIL( "Folder picker failed with illegal arguments" );
        }
    }

    return 0L;
}



void TPGalleryThemeProperties::TakeFiles()
{
    if( aLbxFound.GetSelectEntryCount() || ( bTakeAll && bEntriesFound ) )
    {
        TakeProgress* pTakeProgress = new TakeProgress( this );
        pTakeProgress->Update();

        pTakeProgress->StartExecuteModal(
            Link() /* no postprocessing needed, pTakeProgress
                      will be deleted in TakeProgress::CleanupHdl */ );
    }
}



IMPL_LINK_NOARG(TPGalleryThemeProperties, ClickPreviewHdl)
{
    if ( bInputAllowed )
    {
        aPreviewTimer.Stop();
        aPreviewString = "";

        if( !aCbxPreview.IsChecked() )
        {
            xMediaPlayer.clear();
            aWndPreview.SetGraphic( Graphic() );
            aWndPreview.Invalidate();
        }
        else
            DoPreview();
    }

    return 0;
}



void TPGalleryThemeProperties::DoPreview()
{
    OUString aString( aLbxFound.GetSelectEntry() );

    if( aString != aPreviewString )
    {
        INetURLObject   _aURL( aFoundList[ aLbxFound.GetEntryPos( aString ) ] );
        bInputAllowed = sal_False;

        if ( !aWndPreview.SetGraphic( _aURL ) )
        {
            GetParent()->LeaveWait();
            ErrorHandler::HandleError( ERRCODE_IO_NOTEXISTSPATH );
            GetParent()->EnterWait();
        }
        else if( ::avmedia::MediaWindow::isMediaURL( _aURL.GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS ), "" ) )
        {
            xMediaPlayer = ::avmedia::MediaWindow::createPlayer( _aURL.GetMainURL( INetURLObject::NO_DECODE ), "" );
            if( xMediaPlayer.is() )
                xMediaPlayer->start();
        }

        bInputAllowed = sal_True;
        aPreviewString = aString;
    }
}



IMPL_LINK_NOARG(TPGalleryThemeProperties, ClickTakeHdl)
{
    if( bInputAllowed )
    {
        aPreviewTimer.Stop();

        if( !aLbxFound.GetSelectEntryCount() || !bEntriesFound )
        {
            SvxOpenGraphicDialog aDlg("Gallery");
            aDlg.EnableLink(sal_False);
            aDlg.AsLink(sal_False);

            if( !aDlg.Execute() )
                pData->pTheme->InsertURL( INetURLObject( aDlg.GetPath() ) );
        }
        else
        {
            bTakeAll = sal_False;
            TakeFiles();
        }
    }

    return 0L;
}



IMPL_LINK_NOARG(TPGalleryThemeProperties, ClickTakeAllHdl)
{
    if( bInputAllowed )
    {
        aPreviewTimer.Stop();
        bTakeAll = sal_True;
        TakeFiles();
    }

    return 0L;
}



IMPL_LINK_NOARG(TPGalleryThemeProperties, SelectFoundHdl)
{
    if( bInputAllowed )
    {
        sal_Bool bPreviewPossible = sal_False;

        aPreviewTimer.Stop();

        if( bEntriesFound )
        {
            if( aLbxFound.GetSelectEntryCount() == 1 )
            {
                aCbxPreview.Enable();
                bPreviewPossible = sal_True;
            }
            else
                aCbxPreview.Disable();

            if( !aFoundList.empty() )
                aBtnTakeAll.Enable();
            else
                aBtnTakeAll.Disable();
        }

        if( bPreviewPossible && aCbxPreview.IsChecked() )
            aPreviewTimer.Start();
    }

    return 0;
}



IMPL_LINK_NOARG(TPGalleryThemeProperties, DClickFoundHdl)
{
    if( bInputAllowed )
    {
        aPreviewTimer.Stop();

        return (aLbxFound.GetSelectEntryCount() == 1 && bEntriesFound) ?
            ClickTakeHdl(NULL) : 0;
    }
    else
        return 0;
}



IMPL_LINK_NOARG(TPGalleryThemeProperties, PreviewTimerHdl)
{
    aPreviewTimer.Stop();
    DoPreview();
    return 0L;
}



IMPL_LINK_NOARG(TPGalleryThemeProperties, EndSearchProgressHdl)
{
  if( !aFoundList.empty() )
  {
      aLbxFound.SelectEntryPos( 0 );
      aBtnTakeAll.Enable();
      aCbxPreview.Enable();
      bEntriesFound = sal_True;
  }
  else
  {
      aLbxFound.InsertEntry( OUString( CUI_RES( RID_SVXSTR_GALLERY_NOFILES ) ) );
      aBtnTakeAll.Disable();
      aCbxPreview.Disable();
      bEntriesFound = sal_False;
  }
  return 0L;
}



IMPL_LINK( TPGalleryThemeProperties, DialogClosedHdl, ::com::sun::star::ui::dialogs::DialogClosedEvent*, pEvt )
{
    DBG_ASSERT( xFolderPicker.is(), "TPGalleryThemeProperties::DialogClosedHdl(): no folder picker" );

    OUString sURL = xFolderPicker->getDirectory();
    StartSearchFiles( sURL, pEvt->DialogResult );

    return 0L;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
