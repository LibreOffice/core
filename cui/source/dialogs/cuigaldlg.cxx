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
    const OUString aFileType( mpBrowser->m_pCbbFileType->GetText() );

    if( !aFileType.isEmpty() )
    {
        const sal_uInt16        nFileNumber = mpBrowser->m_pCbbFileType->GetEntryPos( aFileType );
        sal_uInt16              nBeginFormat, nEndFormat;
        ::std::vector< OUString > aFormats;

        if( !nFileNumber || ( nFileNumber >= mpBrowser->m_pCbbFileType->GetEntryCount() ) )
        {
            nBeginFormat = 1;
            nEndFormat = mpBrowser->m_pCbbFileType->GetEntryCount() - 1;
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
                               bool bRecursive )
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

                bool bFolder = xRow->getBoolean( 1 ); // property "IsFolder"
                if ( xRow->wasNull() )
                    bFolder = false;

                if( bRecursive && bFolder )
                    ImplSearch( aFoundURL, rFormats, true );
                else
                {
                    bool bDocument = xRow->getBoolean( 2 ); // property "IsDocument"
                    if ( xRow->wasNull() )
                        bDocument = false;

                    if( bDocument )
                    {
                        GraphicDescriptor   aDesc( aFoundURL );

                        if( ( aDesc.Detect() &&
                              ::std::find( rFormats.begin(),
                                           rFormats.end(),
                                           GraphicDescriptor::GetImportFormatShortName(
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
                            mpBrowser->m_pLbxFound->InsertEntry(
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


SearchProgress::SearchProgress( vcl::Window* pParent, const INetURLObject& rStartURL )
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
        nEntries = mpBrowser->bTakeAll ? mpBrowser->m_pLbxFound->GetEntryCount() : mpBrowser->m_pLbxFound->GetSelectEntryCount();
        pThm->LockBroadcaster();
    }

    for( sal_uInt16 i = 0; i < nEntries && schedule(); i++ )
    {
        if( mpBrowser->bTakeAll )
            aURL = INetURLObject( mpBrowser->aFoundList[ nPos = i ] );
        else
            aURL = INetURLObject( mpBrowser->aFoundList[ nPos = mpBrowser->m_pLbxFound->GetSelectEntryPos( i ) ]);

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
TakeProgress::TakeProgress(vcl::Window* pWindow)
    : ModalDialog(pWindow, "GalleryApplyProgress",
        "cui/ui/galleryapplyprogress.ui")
    , window_(pWindow)
{
    get(m_pFtTakeFile, "file");
    get(m_pBtnCancel, "cancel");

    m_pBtnCancel->SetClickHdl( LINK( this, TakeProgress, ClickCancelBtn ) );
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

    TPGalleryThemeProperties*   mpBrowser = static_cast<TPGalleryThemeProperties*>( GetParent() );
    ::std::vector<bool, std::allocator<bool> >           aRemoveEntries( mpBrowser->aFoundList.size(), false );
    ::std::vector< OUString >   aRemainingVector;
    sal_uInt32                  i, nCount;

    GetParent()->EnterWait();
    mpBrowser->m_pLbxFound->SetUpdateMode( false );
    mpBrowser->m_pLbxFound->SetNoSelection();

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
            aRemainingVector.push_back( mpBrowser->m_pLbxFound->GetEntry( (sal_uInt16) i ) );

    mpBrowser->m_pLbxFound->Clear();

    for( i = 0, nCount = aRemainingVector.size(); i < nCount; ++i )
        mpBrowser->m_pLbxFound->InsertEntry( aRemainingVector[ i ] );

    aRemainingVector.clear();

    mpBrowser->m_pLbxFound->SetUpdateMode( true );
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
ActualizeProgress::ActualizeProgress(vcl::Window* pWindow, GalleryTheme* pThm)
    : ModalDialog(pWindow, "GalleryUpdateProgress",
        "cui/ui/galleryupdateprogress.ui")
    , pTimer(NULL)
    , pTheme(pThm)
{
    get(m_pFtActualizeFile, "file");
    get(m_pBtnCancel, "cancel");
    m_pBtnCancel->SetClickHdl( LINK( this, ActualizeProgress, ClickCancelBtn ) );
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
        m_pFtActualizeFile->SetText( GetReducedString( *pURL, 30 ) );
        m_pFtActualizeFile->Flush();
        m_pFtActualizeFile->Sync();
    }

    return 0;
}

TitleDialog::TitleDialog(vcl::Window* pParent, const OUString& rOldTitle)
    : ModalDialog(pParent, "GalleryTitleDialog", "cui/ui/gallerytitledialog.ui")
{
    get(m_pEdit, "entry");
    m_pEdit->SetText( rOldTitle );
    m_pEdit->GrabFocus();
}


// - GalleryIdDialog -


GalleryIdDialog::GalleryIdDialog( vcl::Window* pParent, GalleryTheme* _pThm )
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
    bool        bDifferentThemeExists = false;

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
            bDifferentThemeExists = true;
        }
    }

    if( !bDifferentThemeExists )
        EndDialog( RET_OK );

    return 0L;
}



// - GalleryThemeProperties -


GalleryThemeProperties::GalleryThemeProperties(vcl::Window* pParent,
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
        static_cast<TPGalleryThemeGeneral&>( rPage ).SetXChgData( pData );
    else
        static_cast<TPGalleryThemeProperties&>( rPage ).SetXChgData( pData );
}

// - TPGalleryThemeGeneral -
TPGalleryThemeGeneral::TPGalleryThemeGeneral(vcl::Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "GalleryGeneralPage",
        "cui/ui/gallerygeneralpage.ui", &rSet)
    , pData(NULL)
{
    get(m_pFiMSImage, "image");
    get(m_pEdtMSName, "name");
    get(m_pFtMSShowType, "type");
    get(m_pFtMSShowPath, "location");
    get(m_pFtMSShowContent, "contents");
    get(m_pFtMSShowChangeDate, "modified");
}

void TPGalleryThemeGeneral::SetXChgData( ExchangeData* _pData )
{
    pData = _pData;

    GalleryTheme*       pThm = pData->pTheme;
    OUString            aOutStr( OUString::number(pThm->GetObjectCount()) );
    OUString            aObjStr( CUI_RES( RID_SVXSTR_GALLERYPROPS_OBJECT ) );
    OUString            aAccess;
    OUString            aType( SVX_RES( RID_SVXSTR_GALLERYPROPS_GALTHEME ) );
    bool            bReadOnly = pThm->IsReadOnly();

    m_pEdtMSName->SetText( pThm->GetName() );
    m_pEdtMSName->SetReadOnly( bReadOnly );

    if( bReadOnly )
        m_pEdtMSName->Disable();
    else
        m_pEdtMSName->Enable();

    if( pThm->IsReadOnly() )
        aType += CUI_RES( RID_SVXSTR_GALLERY_READONLY );

    m_pFtMSShowType->SetText( aType );
    m_pFtMSShowPath->SetText( pThm->GetSdgURL().GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS ) );

    // singular or plural?
    if ( 1 == pThm->GetObjectCount() )
        aObjStr = aObjStr.getToken( 0, ';' );
    else
        aObjStr = aObjStr.getToken( 1, ';' );

    aOutStr += " " + aObjStr;

    m_pFtMSShowContent->SetText( aOutStr );

    // get locale wrapper (singleton)
    const SvtSysLocale aSysLocale;
    const LocaleDataWrapper&    aLocaleData = aSysLocale.GetLocaleData();

    // ChangeDate/Time
    aAccess = aLocaleData.getDate( pData->aThemeChangeDate ) + ", " + aLocaleData.getTime( pData->aThemeChangeTime );
    m_pFtMSShowChangeDate->SetText( aAccess );

    // set image
    sal_uInt16 nId;

    if( pThm->IsReadOnly() )
        nId = RID_SVXBMP_THEME_READONLY_BIG;
    else if( pThm->IsDefault() )
        nId = RID_SVXBMP_THEME_DEFAULT_BIG;
    else
        nId = RID_SVXBMP_THEME_NORMAL_BIG;

    m_pFiMSImage->SetImage( Image( Bitmap( CUI_RES( nId ) ), COL_LIGHTMAGENTA ) );
}



bool TPGalleryThemeGeneral::FillItemSet( SfxItemSet* /*rSet*/ )
{
    pData->aEditedTitle = m_pEdtMSName->GetText();
    return true;
}



SfxTabPage* TPGalleryThemeGeneral::Create( vcl::Window* pParent, const SfxItemSet* rSet )
{
    return new TPGalleryThemeGeneral( pParent, *rSet );
}

// - TPGalleryThemeProperties -
TPGalleryThemeProperties::TPGalleryThemeProperties( vcl::Window* pWindow, const SfxItemSet& rSet )
    : SfxTabPage(pWindow, "GalleryFilesPage", "cui/ui/galleryfilespage.ui", &rSet)
    , pData(NULL)
    , nCurFilterPos(0)
    , nFirstExtFilterPos(0)
    , bEntriesFound(false)
    , bInputAllowed(true)
    , bTakeAll(false)
    , bSearchRecursive(false)
    , xDialogListener(new ::svt::DialogClosedListener())
{
    get(m_pCbbFileType, "filetype");
    get(m_pLbxFound, "files");
    Size aSize(LogicToPixel(Size(172, 156), MAP_APPFONT));
    m_pLbxFound->set_width_request(aSize.Width());
    m_pLbxFound->set_height_request(aSize.Height());
    m_pLbxFound->EnableMultiSelection(true);
    get(m_pBtnSearch, "findfiles");
    get(m_pBtnTake, "add");
    get(m_pBtnTakeAll, "addall");
    get(m_pCbxPreview, "preview");
    get(m_pWndPreview, "image");

    xDialogListener->SetDialogClosedLink( LINK( this, TPGalleryThemeProperties, DialogClosedHdl ) );
}

void TPGalleryThemeProperties::SetXChgData( ExchangeData* _pData )
{
    pData = _pData;

    aPreviewTimer.SetTimeoutHdl( LINK( this, TPGalleryThemeProperties, PreviewTimerHdl ) );
    aPreviewTimer.SetTimeout( 500 );
    m_pBtnSearch->SetClickHdl(LINK(this, TPGalleryThemeProperties, ClickSearchHdl));
    m_pBtnTake->SetClickHdl(LINK(this, TPGalleryThemeProperties, ClickTakeHdl));
    m_pBtnTakeAll->SetClickHdl(LINK(this, TPGalleryThemeProperties, ClickTakeAllHdl));
    m_pCbxPreview->SetClickHdl(LINK(this, TPGalleryThemeProperties, ClickPreviewHdl));
    m_pCbbFileType->SetSelectHdl(LINK(this, TPGalleryThemeProperties, SelectFileTypeHdl));
    m_pCbbFileType->EnableDDAutoWidth( false );
    m_pLbxFound->SetDoubleClickHdl(LINK(this, TPGalleryThemeProperties, DClickFoundHdl));
    m_pLbxFound->SetSelectHdl(LINK(this, TPGalleryThemeProperties, SelectFoundHdl));
    m_pLbxFound->InsertEntry(OUString(CUI_RES(RID_SVXSTR_GALLERY_NOFILES)));
    m_pLbxFound->Show();

    FillFilterList();

    m_pBtnTake->Enable();
    m_pBtnTakeAll->Disable();
    m_pCbxPreview->Disable();
}



void TPGalleryThemeProperties::StartSearchFiles( const OUString& _rFolderURL, short _nDlgResult )
{
    if ( RET_OK == _nDlgResult )
    {
        aURL = INetURLObject( _rFolderURL );
        bSearchRecursive = true;    // UI choice no longer possible, windows file picker allows no user controls
        SearchFiles();
    }

    nCurFilterPos = m_pCbbFileType->GetEntryPos( m_pCbbFileType->GetText() );
}



TPGalleryThemeProperties::~TPGalleryThemeProperties()
{
    xMediaPlayer.clear();
    xDialogListener.clear();

    for ( size_t i = 0, n = aFilterEntryList.size(); i < n; ++i ) {
        delete aFilterEntryList[ i ];
    }
}



SfxTabPage* TPGalleryThemeProperties::Create( vcl::Window* pParent, const SfxItemSet* rSet )
{
    return new TPGalleryThemeProperties( pParent, *rSet );
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
    bool                bInList;

    // graphic filters
    for( i = 0, nKeyCount = rFilter.GetImportFormatCount(); i < nKeyCount; i++ )
    {
        aExt = rFilter.GetImportFormatShortName( i );
        aName = rFilter.GetImportFormatName( i );
        size_t entryIndex = 0;
        pTestEntry = aFilterEntryList.empty() ? NULL : aFilterEntryList[ entryIndex ];
        bInList = false;

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
                bInList = true;
                break;
            }
            pTestEntry = ( ++entryIndex < aFilterEntryList.size() )
                       ? aFilterEntryList[ entryIndex ] : NULL;
        }
        if ( !bInList )
        {
            pFilterEntry = new FilterEntry;
            pFilterEntry->aFilterName = aExt;
            size_t pos = m_pCbbFileType->InsertEntry( aName );
            if ( pos < aFilterEntryList.size() ) {
                aFilterEntryList.insert( aFilterEntryList.begin() + pos, pFilterEntry );
            } else {
                aFilterEntryList.push_back( pFilterEntry );
            }
        }
    }

    // media filters
    static const char aWildcard[] = "*.";
    ::avmedia::FilterNameVector     aFilters;
    ::avmedia::MediaWindow::getMediaFilters( aFilters );

    for( unsigned long l = 0; l < aFilters.size(); ++l )
    {
        for( sal_Int32 nIndex = 0; nIndex >= 0; )
        {
            OUString aFilterWildcard( aWildcard );

            pFilterEntry = new FilterEntry;
            pFilterEntry->aFilterName = aFilters[ l ].second.getToken( 0, ';', nIndex );
            nFirstExtFilterPos = m_pCbbFileType->InsertEntry(
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
    size_t pos = m_pCbbFileType->InsertEntry( pFilterEntry->aFilterName, 0 );
    if ( pos < aFilterEntryList.size() ) {
        aFilterEntryList.insert( aFilterEntryList.begin() + pos, pFilterEntry );
    } else {
        aFilterEntryList.push_back( pFilterEntry );
    }
    m_pCbbFileType->SetText( pFilterEntry->aFilterName );
}



IMPL_LINK_NOARG(TPGalleryThemeProperties, SelectFileTypeHdl)
{
    OUString aText( m_pCbbFileType->GetText() );

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
    m_pLbxFound->Clear();

    pProgress->SetFileType( m_pCbbFileType->GetText() );
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
                    bSearchRecursive = true;    // UI choice no longer possible, windows file picker allows no user controls
                    SearchFiles();
                }

                nCurFilterPos = m_pCbbFileType->GetEntryPos( m_pCbbFileType->GetText() );
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
    if( m_pLbxFound->GetSelectEntryCount() || ( bTakeAll && bEntriesFound ) )
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

        if( !m_pCbxPreview->IsChecked() )
        {
            xMediaPlayer.clear();
            m_pWndPreview->SetGraphic( Graphic() );
            m_pWndPreview->Invalidate();
        }
        else
            DoPreview();
    }

    return 0;
}



void TPGalleryThemeProperties::DoPreview()
{
    OUString aString( m_pLbxFound->GetSelectEntry() );

    if( aString != aPreviewString )
    {
        INetURLObject   _aURL( aFoundList[ m_pLbxFound->GetEntryPos( aString ) ] );
        bInputAllowed = false;

        if ( !m_pWndPreview->SetGraphic( _aURL ) )
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

        bInputAllowed = true;
        aPreviewString = aString;
    }
}



IMPL_LINK_NOARG(TPGalleryThemeProperties, ClickTakeHdl)
{
    if( bInputAllowed )
    {
        aPreviewTimer.Stop();

        if( !m_pLbxFound->GetSelectEntryCount() || !bEntriesFound )
        {
            SvxOpenGraphicDialog aDlg("Gallery");
            aDlg.EnableLink(false);
            aDlg.AsLink(false);

            if( !aDlg.Execute() )
                pData->pTheme->InsertURL( INetURLObject( aDlg.GetPath() ) );
        }
        else
        {
            bTakeAll = false;
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
        bTakeAll = true;
        TakeFiles();
    }

    return 0L;
}



IMPL_LINK_NOARG(TPGalleryThemeProperties, SelectFoundHdl)
{
    if( bInputAllowed )
    {
        bool bPreviewPossible = false;

        aPreviewTimer.Stop();

        if( bEntriesFound )
        {
            if( m_pLbxFound->GetSelectEntryCount() == 1 )
            {
                m_pCbxPreview->Enable();
                bPreviewPossible = true;
            }
            else
                m_pCbxPreview->Disable();

            if( !aFoundList.empty() )
                m_pBtnTakeAll->Enable();
            else
                m_pBtnTakeAll->Disable();
        }

        if( bPreviewPossible && m_pCbxPreview->IsChecked() )
            aPreviewTimer.Start();
    }

    return 0;
}



IMPL_LINK_NOARG(TPGalleryThemeProperties, DClickFoundHdl)
{
    if( bInputAllowed )
    {
        aPreviewTimer.Stop();

        return (m_pLbxFound->GetSelectEntryCount() == 1 && bEntriesFound) ?
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
      m_pLbxFound->SelectEntryPos( 0 );
      m_pBtnTakeAll->Enable();
      m_pCbxPreview->Enable();
      bEntriesFound = true;
  }
  else
  {
      m_pLbxFound->InsertEntry( OUString( CUI_RES( RID_SVXSTR_GALLERY_NOFILES ) ) );
      m_pBtnTakeAll->Disable();
      m_pCbxPreview->Disable();
      bEntriesFound = false;
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
