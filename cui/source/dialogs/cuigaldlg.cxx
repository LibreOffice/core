/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <algorithm>
#include <ucbhelper/content.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <avmedia/mediawindow.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/opengrf.hxx>
#include <svtools/filter.hxx>
#include <svx/gallery1.hxx>
#include <svx/galtheme.hxx>
#include "cuigaldlg.hxx"
#include <svl/pickerhelper.hxx>
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
#include <com/sun/star/ui/dialogs/XAsynchronousExecutableDialog.hpp>
#include <sfx2/sfxuno.hxx>
#include "dialmgr.hxx"
#include "gallery.hrc"

// --------------
// - Namespaces -
// --------------

using namespace ::ucbhelper;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;


// -----------
// - Defines -
// -----------

#define MAXPATH 1024

// ----------------
// - SearchThread -
// ----------------

SearchThread::SearchThread( SearchProgress* pProgess,
                            TPGalleryThemeProperties* pBrowser,
                            const INetURLObject& rStartURL ) :
        mpProgress  ( pProgess ),
        mpBrowser   ( pBrowser ),
        maStartURL  ( rStartURL )
{
}

// ------------------------------------------------------------------------

SearchThread::~SearchThread()
{
}

// ------------------------------------------------------------------------

void SAL_CALL SearchThread::run()
{
    const String aFileType( mpBrowser->aCbbFileType.GetText() );

    if( aFileType.Len() )
    {
        const sal_uInt16        nFileNumber = mpBrowser->aCbbFileType.GetEntryPos( aFileType );
        sal_uInt16              nBeginFormat, nEndFormat;
        ::std::vector< String > aFormats;

        if( !nFileNumber || ( nFileNumber >= mpBrowser->aCbbFileType.GetEntryCount() ) )
        {
            nBeginFormat = 1;
            nEndFormat = mpBrowser->aCbbFileType.GetEntryCount() - 1;
        }
        else
            nBeginFormat = nEndFormat = nFileNumber;

        for( sal_uInt16 i = nBeginFormat; i <= nEndFormat; ++i )
            aFormats.push_back( ( (FilterEntry*) mpBrowser->aFilterEntryList.GetObject( i ) )->aFilterName.ToLowerAscii() );

        ImplSearch( maStartURL, aFormats, mpBrowser->bSearchRecursive );
    }
}

// ------------------------------------------------------------------------

void SAL_CALL SearchThread::onTerminated()
{
    Application::PostUserEvent( LINK( mpProgress, SearchProgress, CleanUpHdl ) );
}

// ------------------------------------------------------------------------

void SearchThread::ImplSearch( const INetURLObject& rStartURL,
                               const ::std::vector< String >& rFormats,
                               BOOL bRecursive )
{
    {
        SolarMutexGuard aGuard;

        mpProgress->SetDirectory( rStartURL );
        mpProgress->Sync();
    }

    try
    {
        ::com::sun::star::uno::Reference< XCommandEnvironment > xEnv;
        Content aCnt( rStartURL.GetMainURL( INetURLObject::NO_DECODE ), xEnv );
        Sequence< OUString > aProps( 2 );

        aProps.getArray()[ 0 ] = OUString(RTL_CONSTASCII_USTRINGPARAM( "IsFolder" ));
        aProps.getArray()[ 1 ] = OUString(RTL_CONSTASCII_USTRINGPARAM( "IsDocument" ));
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
                    ImplSearch( aFoundURL, rFormats, TRUE );
                else
                {
                    sal_Bool bDocument = xRow->getBoolean( 2 ); // property "IsDocument"
                    if ( xRow->wasNull() )
                        bDocument = sal_False;

                    if( bDocument )
                    {
                        GraphicDescriptor   aDesc( aFoundURL );
                        String              aFileName;

                        if( ( aDesc.Detect() &&
                              ::std::find( rFormats.begin(),
                                           rFormats.end(),
                                           aDesc.GetImportFormatShortName(
                                               aDesc.GetFileFormat() ).ToLowerAscii() )
                              != rFormats.end() ) ||
                            ::std::find( rFormats.begin(),
                                         rFormats.end(),
                                         String(aFoundURL.GetExtension().toAsciiLowerCase()) )
                            != rFormats.end() )
                        {
                            SolarMutexGuard aGuard;

                            mpBrowser->aFoundList.push_back(
                                new String( aFoundURL.GetMainURL( INetURLObject::NO_DECODE ) )
                            );
                            mpBrowser->aLbxFound.InsertEntry(
                                GetReducedString( aFoundURL, 50 ),
                                (USHORT) mpBrowser->aFoundList.size() - 1 );
                        }
                    }
                }
            }
        }
    }
    catch( const ContentCreationException& )
    {
    }
    catch( const ::com::sun::star::uno::RuntimeException& )
    {
    }
        catch( const ::com::sun::star::uno::Exception& )
    {
    }
}

// ------------------
// - SearchProgress -
// ------------------

SearchProgress::SearchProgress( Window* pParent, const INetURLObject& rStartURL ) :
    ModalDialog     ( pParent, CUI_RES(RID_SVXDLG_GALLERY_SEARCH_PROGRESS ) ),
    aFtSearchDir    ( this, CUI_RES( FT_SEARCH_DIR ) ),
    aFLSearchDir   ( this, CUI_RES( FL_SEARCH_DIR ) ),
    aFtSearchType   ( this, CUI_RES( FT_SEARCH_TYPE ) ),
    aFLSearchType  ( this, CUI_RES( FL_SEARCH_TYPE ) ),
    aBtnCancel      ( this, CUI_RES( BTN_CANCEL ) ),
    maSearchThread  ( this, (TPGalleryThemeProperties*) pParent, rStartURL )
{
    FreeResource();
    aBtnCancel.SetClickHdl( LINK( this, SearchProgress, ClickCancelBtn ) );
}

// ------------------------------------------------------------------------

void SearchProgress::Terminate()
{
    maSearchThread.terminate();
}

// ------------------------------------------------------------------------

IMPL_LINK( SearchProgress, ClickCancelBtn, void*, EMPTYARG )
{
    Terminate();
    return 0L;
}

// ------------------------------------------------------------------------

IMPL_LINK( SearchProgress, CleanUpHdl, void*, EMPTYARG )
{
    EndDialog( RET_OK );
    delete this;
    return 0L;
}

// ------------------------------------------------------------------------

short SearchProgress::Execute()
{
    DBG_ERROR( "SearchProgress cannot be executed via Dialog::Execute!\n"
               "It creates a thread that will call back to VCL apartment => deadlock!\n"
               "Use Dialog::StartExecuteModal to execute the dialog!" );
    return RET_CANCEL;
}

// ------------------------------------------------------------------------

void SearchProgress::StartExecuteModal( const Link& rEndDialogHdl )
{
    maSearchThread.create();
    ModalDialog::StartExecuteModal( rEndDialogHdl );
}

// --------------
// - TakeThread -
// --------------

TakeThread::TakeThread( TakeProgress* pProgess, TPGalleryThemeProperties* pBrowser, List& rTakenList ) :
        mpProgress  ( pProgess ),
        mpBrowser   ( pBrowser ),
        mrTakenList ( rTakenList )
{
}

// ------------------------------------------------------------------------

TakeThread::~TakeThread()
{
}

// ------------------------------------------------------------------------

void SAL_CALL TakeThread::run()
{
    String              aName;
    INetURLObject       aURL;
    USHORT              nEntries;
    GalleryTheme*       pThm = mpBrowser->GetXChgData()->pTheme;
    USHORT              nPos;
    GalleryProgress*    pStatusProgress;

    {
        SolarMutexGuard aGuard;
        pStatusProgress = new GalleryProgress;
        nEntries = mpBrowser->bTakeAll ? mpBrowser->aLbxFound.GetEntryCount() : mpBrowser->aLbxFound.GetSelectEntryCount();
        pThm->LockBroadcaster();
    }

    for( USHORT i = 0; i < nEntries && schedule(); i++ )
    {
        // kompletten Filenamen aus FoundList holen
        if( mpBrowser->bTakeAll )
            aURL = INetURLObject( *mpBrowser->aFoundList[ nPos = i ] );
        else
            aURL = INetURLObject(*mpBrowser->aFoundList[ nPos = mpBrowser->aLbxFound.GetSelectEntryPos( i ) ]);

        // Position in Taken-Liste uebernehmen
        mrTakenList.Insert( (void*) (ULONG)nPos, LIST_APPEND );

        {
            SolarMutexGuard aGuard;

            mpProgress->SetFile( aURL.GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS ) );
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
}

// ------------------------------------------------------------------------

void SAL_CALL TakeThread::onTerminated()
{
    Application::PostUserEvent( LINK( mpProgress, TakeProgress, CleanUpHdl ) );
}

// ----------------
// - TakeProgress -
// ----------------

TakeProgress::TakeProgress( Window* pWindow ) :
    ModalDialog     ( pWindow, CUI_RES( RID_SVXDLG_GALLERY_TAKE_PROGRESS ) ),
    aFtTakeFile     ( this, CUI_RES( FT_TAKE_FILE ) ),
    aFLTakeProgress( this, CUI_RES( FL_TAKE_PROGRESS ) ),
    aBtnCancel      ( this, CUI_RES( BTN_CANCEL ) ),
    maTakeThread    ( this, (TPGalleryThemeProperties*) pWindow, maTakenList )

{
    FreeResource();
    aBtnCancel.SetClickHdl( LINK( this, TakeProgress, ClickCancelBtn ) );
}

// ------------------------------------------------------------------------


void TakeProgress::Terminate()
{
    maTakeThread.terminate();
}

// ------------------------------------------------------------------------

IMPL_LINK( TakeProgress, ClickCancelBtn, void*, EMPTYARG )
{
    Terminate();
    return 0L;
}

// ------------------------------------------------------------------------

IMPL_LINK( TakeProgress, CleanUpHdl, void*, EMPTYARG )
{
    TPGalleryThemeProperties*   mpBrowser = (TPGalleryThemeProperties*) GetParent();
    ::std::vector<bool, std::allocator<bool> >           aRemoveEntries( mpBrowser->aFoundList.size(), false );
    ::std::vector< String >     aRemainingVector;
    sal_uInt32                  i, nCount;

    GetParent()->EnterWait();
    mpBrowser->aLbxFound.SetUpdateMode( FALSE );
    mpBrowser->aLbxFound.SetNoSelection();

    // mark all taken positions in aRemoveEntries
    for( i = 0UL, nCount = maTakenList.Count(); i < nCount; ++i )
        aRemoveEntries[ (ULONG) maTakenList.GetObject( i ) ] = true;

    maTakenList.Clear();

    // refill found list
    for( i = 0, nCount = aRemoveEntries.size(); i < nCount; ++i )
        if( !aRemoveEntries[ i ] )
            aRemainingVector.push_back( *mpBrowser->aFoundList[ i ] );

    for ( i = 0, nCount = mpBrowser->aFoundList.size(); i < nCount; ++i )
        delete mpBrowser->aFoundList[ i ];
    mpBrowser->aFoundList.clear();

    for( i = 0, nCount = aRemainingVector.size(); i < nCount; ++i )
        mpBrowser->aFoundList.push_back( new String( aRemainingVector[ i ] ) );

    aRemainingVector.clear();

    // refill list box
    for( i = 0, nCount = aRemoveEntries.size(); i < nCount; ++i )
        if( !aRemoveEntries[ i ] )
            aRemainingVector.push_back( mpBrowser->aLbxFound.GetEntry( (sal_uInt16) i ) );

    mpBrowser->aLbxFound.Clear();

    for( i = 0, nCount = aRemainingVector.size(); i < nCount; ++i )
        mpBrowser->aLbxFound.InsertEntry( aRemainingVector[ i ] );

    aRemainingVector.clear();

    mpBrowser->aLbxFound.SetUpdateMode( TRUE );
    mpBrowser->SelectFoundHdl( NULL );
    GetParent()->LeaveWait();

    EndDialog( RET_OK );
    delete this;
    return 0L;
}

// ------------------------------------------------------------------------

short TakeProgress::Execute()
{
    DBG_ERROR( "TakeProgress cannot be executed via Dialog::Execute!\n"
               "It creates a thread that will call back to VCL apartment => deadlock!\n"
               "Use Dialog::StartExecuteModal to execute the dialog!" );
    return RET_CANCEL;
}

// ------------------------------------------------------------------------

void TakeProgress::StartExecuteModal( const Link& rEndDialogHdl )
{
    maTakeThread.create();
    ModalDialog::StartExecuteModal( rEndDialogHdl );
}

// ---------------------
// - ActualizeProgress -
// ---------------------

ActualizeProgress::ActualizeProgress( Window* pWindow, GalleryTheme* pThm ) :
    ModalDialog             ( pWindow, CUI_RES( RID_SVXDLG_GALLERY_ACTUALIZE_PROGRESS ) ),
    aFtActualizeFile        ( this, CUI_RES( FT_ACTUALIZE_FILE ) ),
    aFLActualizeProgress   ( this, CUI_RES( FL_ACTUALIZE_PROGRESS ) ),
    aBtnCancel              ( this, CUI_RES( BTN_CANCEL ) ),
    pTheme                  ( pThm )
{
    FreeResource();
    aBtnCancel.SetClickHdl( LINK( this, ActualizeProgress, ClickCancelBtn ) );
}

// ------------------------------------------------------------------------

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

// ------------------------------------------------------------------------

IMPL_LINK( ActualizeProgress, ClickCancelBtn, void*, EMPTYARG )
{
    pTheme->AbortActualize();
    EndDialog( RET_OK );

    return 0L;
}

// ------------------------------------------------------------------------

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

// ------------------------------------------------------------------------

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

// ---------------
// - TitleDialog -
// ---------------

TitleDialog::TitleDialog( Window* pParent, const String& rOldTitle ) :
    ModalDialog ( pParent, CUI_RES( RID_SVXDLG_GALLERY_TITLE ) ),
    maOk        ( this, CUI_RES( BTN_OK ) ),
    maCancel    ( this, CUI_RES( BTN_CANCEL ) ),
    maHelp      ( this, CUI_RES( BTN_HELP ) ),
    maFL       ( this, CUI_RES( FL_TITLE ) ),
    maEdit      ( this, CUI_RES( EDT_TITLE ) )
{
    FreeResource();
    maEdit.SetText( rOldTitle );
    maEdit.GrabFocus();
}

// -------------------
// - GalleryIdDialog -
// -------------------

GalleryIdDialog::GalleryIdDialog( Window* pParent, GalleryTheme* _pThm ) :
            ModalDialog ( pParent, CUI_RES( RID_SVXDLG_GALLERY_THEMEID ) ),
            aBtnOk      ( this, CUI_RES( BTN_OK ) ),
            aBtnCancel  ( this, CUI_RES( BTN_CANCEL ) ),
            aFLId      ( this, CUI_RES( FL_ID ) ),
            aLbResName  ( this, CUI_RES( LB_RESNAME ) ),
            pThm        ( _pThm )
{
    FreeResource();

    aLbResName.InsertEntry( String( RTL_CONSTASCII_USTRINGPARAM( "!!! No Id !!!" ) ) );

    GalleryTheme::InsertAllThemes( aLbResName );

    aLbResName.SelectEntryPos( (USHORT) pThm->GetId() );
    aLbResName.GrabFocus();

    aBtnOk.SetClickHdl( LINK( this, GalleryIdDialog, ClickOkHdl ) );
}

// -----------------------------------------------------------------------------

IMPL_LINK( GalleryIdDialog, ClickOkHdl, void*, EMPTYARG )
{
    Gallery*    pGal = pThm->GetParent();
    const ULONG nId = GetId();
    BOOL        bDifferentThemeExists = FALSE;

    for( ULONG i = 0, nCount = pGal->GetThemeCount(); i < nCount && !bDifferentThemeExists; i++ )
    {
        const GalleryThemeEntry* pInfo = pGal->GetThemeInfo( i );

        if( ( pInfo->GetId() == nId ) && ( pInfo->GetThemeName() != pThm->GetName() ) )
        {
            String aStr( CUI_RES( RID_SVXSTR_GALLERY_ID_EXISTS ) );

            aStr += String( RTL_CONSTASCII_USTRINGPARAM( " (" ) );
            aStr += pInfo->GetThemeName();
            aStr += ')';

            InfoBox aBox( this, aStr );
            aBox.Execute();
            aLbResName.GrabFocus();
            bDifferentThemeExists = TRUE;
        }
    }

    if( !bDifferentThemeExists )
        EndDialog( RET_OK );

    return 0L;
}


// --------------------------
// - GalleryThemeProperties -
// --------------------------

GalleryThemeProperties::GalleryThemeProperties( Window* pParent, ExchangeData* _pData, SfxItemSet* pItemSet  ) :
            SfxTabDialog    ( pParent, CUI_RES( RID_SVXTABDLG_GALLERYTHEME ), pItemSet ),
            pData           ( _pData )
{
    FreeResource();

    AddTabPage( RID_SVXTABPAGE_GALLERY_GENERAL, TPGalleryThemeGeneral::Create, 0 );
    AddTabPage( RID_SVXTABPAGE_GALLERYTHEME_FILES, TPGalleryThemeProperties::Create, 0 );

    if( pData->pTheme->IsReadOnly() )
        RemoveTabPage( RID_SVXTABPAGE_GALLERYTHEME_FILES );

    String aText( GetText() );

    aText += pData->pTheme->GetName();

    if( pData->pTheme->IsReadOnly() )
        aText += String( CUI_RES( RID_SVXSTR_GALLERY_READONLY ) );

    SetText( aText );
}

// ------------------------------------------------------------------------

void GalleryThemeProperties::PageCreated( USHORT nId, SfxTabPage &rPage )
{
    if( RID_SVXTABPAGE_GALLERY_GENERAL == nId )
        ( (TPGalleryThemeGeneral&) rPage ).SetXChgData( pData );
    else
        ( (TPGalleryThemeProperties&) rPage ).SetXChgData( pData );
}

// -------------------------
// - TPGalleryThemeGeneral -
// -------------------------

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
            aFtMSShowChangeDate     ( this, CUI_RES( FT_MS_SHOW_CHANGEDATE ) )
{
    FreeResource();
}

// ------------------------------------------------------------------------

void TPGalleryThemeGeneral::SetXChgData( ExchangeData* _pData )
{
    pData = _pData;

    GalleryTheme*       pThm = pData->pTheme;
    String              aOutStr( String::CreateFromInt32( pThm->GetObjectCount() ) );
    String              aObjStr( CUI_RES( RID_SVXSTR_GALLERYPROPS_OBJECT ) );
    String              aAccess;
    String              aType( CUI_RES( RID_SVXSTR_GALLERYPROPS_GALTHEME ) );
    BOOL                bReadOnly = pThm->IsReadOnly() && !pThm->IsImported();

    aEdtMSName.SetHelpId( HID_GALLERY_EDIT_MSNAME );
    aEdtMSName.SetText( pThm->GetName() );
    aEdtMSName.SetReadOnly( bReadOnly );

    if( bReadOnly )
        aEdtMSName.Disable();
    else
        aEdtMSName.Enable();

    if( pThm->IsReadOnly() )
        aType += String( CUI_RES( RID_SVXSTR_GALLERY_READONLY ) );

    aFtMSShowType.SetText( aType );
    aFtMSShowPath.SetText( pThm->GetSdgURL().GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS ) );

    // Ein- oder Mehrzahl?
    if ( 1 == pThm->GetObjectCount() )
        aObjStr = aObjStr.GetToken( 0 );
    else
        aObjStr = aObjStr.GetToken( 1 );

    aOutStr += ' ';
    aOutStr += aObjStr;

    aFtMSShowContent.SetText( aOutStr );

    // get locale wrapper (singleton)
    const SvtSysLocale aSysLocale;
    const LocaleDataWrapper&    aLocaleData = aSysLocale.GetLocaleData();

    // ChangeDate/Time
    aAccess = aLocaleData.getDate( pData->aThemeChangeDate );
    aAccess += String( RTL_CONSTASCII_USTRINGPARAM( ", " ) );
    aAccess += aLocaleData.getTime( pData->aThemeChangeTime );
    aFtMSShowChangeDate.SetText( aAccess );

    // Image setzen
    USHORT nId;

    if( pThm->IsImported() )
        nId = RID_SVXBMP_THEME_IMPORTED_BIG;
    else if( pThm->IsReadOnly() )
        nId = RID_SVXBMP_THEME_READONLY_BIG;
    else if( pThm->IsDefault() )
        nId = RID_SVXBMP_THEME_DEFAULT_BIG;
    else
        nId = RID_SVXBMP_THEME_NORMAL_BIG;

    aFiMSImage.SetImage( Image( Bitmap( CUI_RES( nId ) ), COL_LIGHTMAGENTA ) );
}

// ------------------------------------------------------------------------

BOOL TPGalleryThemeGeneral::FillItemSet( SfxItemSet& /*rSet*/ )
{
    pData->aEditedTitle = aEdtMSName.GetText();
    return TRUE;
}

// ------------------------------------------------------------------------

SfxTabPage* TPGalleryThemeGeneral::Create( Window* pParent, const SfxItemSet& rSet )
{
    return new TPGalleryThemeGeneral( pParent, rSet );
}

// ----------------------------
// - TPGalleryThemeProperties -
// ----------------------------

TPGalleryThemeProperties::TPGalleryThemeProperties( Window* pWindow, const SfxItemSet& rSet ) :
        SfxTabPage          ( pWindow, CUI_RES( RID_SVXTABPAGE_GALLERYTHEME_FILES ), rSet ),
        aBtnSearch          ( this, CUI_RES(BTN_SEARCH ) ),
        aBtnTake            ( this, CUI_RES(BTN_TAKE ) ),
        aBtnTakeAll         ( this, CUI_RES(BTN_TAKEALL ) ),
        aCbxPreview         ( this, CUI_RES(CBX_PREVIEW ) ),
        aCbbFileType        ( this, CUI_RES(CBB_FILETYPE ) ),
        aLbxFound           ( this, CUI_RES(LBX_FOUND ) ),
        aFtFileType         ( this, CUI_RES(FT_FILETYPE ) ),
        aWndPreview         ( this, CUI_RES( WND_BRSPRV ) ),
        nCurFilterPos       (0),
        nFirstExtFilterPos  (0),
        bEntriesFound       (FALSE),
        bInputAllowed       (TRUE),
        bSearchRecursive    (FALSE),
        xDialogListener     ( new ::svt::DialogClosedListener() )
{
    FreeResource();

    xDialogListener->SetDialogClosedLink( LINK( this, TPGalleryThemeProperties, DialogClosedHdl ) );
}

// ------------------------------------------------------------------------

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
    aCbbFileType.EnableDDAutoWidth( sal_False );
    aLbxFound.SetDoubleClickHdl(LINK(this, TPGalleryThemeProperties, DClickFoundHdl));
    aLbxFound.SetSelectHdl(LINK(this, TPGalleryThemeProperties, SelectFoundHdl));
    aLbxFound.InsertEntry(String(CUI_RES(RID_SVXSTR_GALLERY_NOFILES)));
    aLbxFound.Show();

    FillFilterList();

    aBtnTake.Enable();
    aBtnTakeAll.Disable();
    aCbxPreview.Disable();
}

// ------------------------------------------------------------------------

void TPGalleryThemeProperties::StartSearchFiles( const String& _rFolderURL, short _nDlgResult )
{
    if ( RET_OK == _nDlgResult )
    {
        aURL = INetURLObject( _rFolderURL );
        bSearchRecursive = sal_True;    // UI choice no longer possible, windows file picker allows no user controls
        SearchFiles();
    }

    nCurFilterPos = aCbbFileType.GetEntryPos( aCbbFileType.GetText() );
}

// ------------------------------------------------------------------------

TPGalleryThemeProperties::~TPGalleryThemeProperties()
{
    xMediaPlayer.clear();
    xDialogListener.clear();

    for ( size_t i = 0, n = aFoundList.size(); i < n; ++i )
        delete aFoundList[ i ];

    for( void* pEntry = aFilterEntryList.First(); pEntry; pEntry = aFilterEntryList.Next() )
        delete (FilterEntry*) pEntry;
}

// ------------------------------------------------------------------------

SfxTabPage* TPGalleryThemeProperties::Create( Window* pParent, const SfxItemSet& rSet )
{
    return new TPGalleryThemeProperties( pParent, rSet );
}

// ------------------------------------------------------------------------

::rtl::OUString TPGalleryThemeProperties::addExtension( const ::rtl::OUString& _rDisplayText, const ::rtl::OUString& _rExtension )
{
    ::rtl::OUString sAllFilter( RTL_CONSTASCII_USTRINGPARAM( "(*.*)" ) );
    ::rtl::OUString sOpenBracket( RTL_CONSTASCII_USTRINGPARAM( " (" ) );
    ::rtl::OUString sCloseBracket( RTL_CONSTASCII_USTRINGPARAM( ")" ) );
    ::rtl::OUString sRet = _rDisplayText;

    if ( sRet.indexOf( sAllFilter ) == -1 )
    {
        String sExt = _rExtension;
        sRet += sOpenBracket;
        sRet += sExt;
        sRet += sCloseBracket;
    }
    return sRet;
}

// ------------------------------------------------------------------------

void TPGalleryThemeProperties::FillFilterList()
{
    GraphicFilter*      pFilter = GraphicFilter::GetGraphicFilter();
    String              aExt;
    String              aName;
    FilterEntry*        pFilterEntry;
    FilterEntry*        pTestEntry;
    sal_uInt16          i, nKeyCount;
    BOOL                bInList;

    // graphic filters
    for( i = 0, nKeyCount = pFilter->GetImportFormatCount(); i < nKeyCount; i++ )
    {
        aExt = pFilter->GetImportFormatShortName( i );
        aName = pFilter->GetImportFormatName( i );
        pTestEntry = (FilterEntry*) aFilterEntryList.First();
        bInList = FALSE;

        String aExtensions;
        int j = 0;
        String sWildcard;
        while( TRUE )
        {
            sWildcard = pFilter->GetImportWildcard( i, j++ );
            if ( !sWildcard.Len() )
                break;
            if ( aExtensions.Search( sWildcard ) == STRING_NOTFOUND )
            {
                if ( aExtensions.Len() )
                    aExtensions += sal_Unicode(';');
                aExtensions += sWildcard;
            }
        }
        aName = addExtension( aName, aExtensions );

        while( pTestEntry )
        {
            if ( pTestEntry->aFilterName == aExt )
            {
                bInList = TRUE;
                break;
            }
            pTestEntry = (FilterEntry*) aFilterEntryList.Next();
        }
        if ( !bInList )
        {
            pFilterEntry = new FilterEntry;
            pFilterEntry->aFilterName = aExt;
            aFilterEntryList.Insert( pFilterEntry, aCbbFileType.InsertEntry( aName ) );
        }
    }

    // media filters
       static const ::rtl::OUString aWildcard( RTL_CONSTASCII_USTRINGPARAM( "*." ) );
    ::avmedia::FilterNameVector     aFilters;
    const ::rtl::OUString           aSeparator( RTL_CONSTASCII_USTRINGPARAM( ";" ) );
    ::rtl::OUString                 aAllTypes;

    ::avmedia::MediaWindow::getMediaFilters( aFilters );

    for( unsigned long l = 0; l < aFilters.size(); ++l )
    {
        for( sal_Int32 nIndex = 0; nIndex >= 0; )
        {
            ::rtl::OUString aFilterWildcard( aWildcard );

            pFilterEntry = new FilterEntry;
            pFilterEntry->aFilterName = aFilters[ l ].second.getToken( 0, ';', nIndex );
            nFirstExtFilterPos = aCbbFileType.InsertEntry( addExtension( aFilters[ l ].first,
                                                           aFilterWildcard += pFilterEntry->aFilterName ) );
            aFilterEntryList.Insert( pFilterEntry, nFirstExtFilterPos );
        }
    }

    // 'All' filters
    String aExtensions;

    // graphic filters
    for ( i = 0; i < nKeyCount; ++i )
    {
        int j = 0;
        String sWildcard;
        while( TRUE )
        {
            sWildcard = pFilter->GetImportWildcard( i, j++ );
            if ( !sWildcard.Len() )
                break;
            if ( aExtensions.Search( sWildcard ) == STRING_NOTFOUND )
            {
                if ( aExtensions.Len() )
                    aExtensions += sal_Unicode( ';' );

                aExtensions += sWildcard;
            }
        }
    }

    // media filters
    for( unsigned long k = 0; k < aFilters.size(); ++k )
    {
        for( sal_Int32 nIndex = 0; nIndex >= 0; )
        {
            if ( aExtensions.Len() )
                aExtensions += sal_Unicode( ';' );
            ( aExtensions += String( aWildcard ) ) += String( aFilters[ k ].second.getToken( 0, ';', nIndex ) );
        }
     }

#if defined(WIN) || defined(WNT)
    if ( aExtensions.Len() > 240 )
        aExtensions = DEFINE_CONST_UNICODE( "*.*" );
#endif

    pFilterEntry = new FilterEntry;
    pFilterEntry->aFilterName = String( CUI_RES( RID_SVXSTR_GALLERY_ALLFILES ) );
    pFilterEntry->aFilterName = addExtension( pFilterEntry->aFilterName, aExtensions );
    aFilterEntryList.Insert(pFilterEntry, aCbbFileType. InsertEntry( pFilterEntry->aFilterName, 0 ) );

    aCbbFileType.SetText( pFilterEntry->aFilterName );
}

// ------------------------------------------------------------------------

IMPL_LINK( TPGalleryThemeProperties, SelectFileTypeHdl, void *, EMPTYARG )
{
    String aText( aCbbFileType.GetText() );

    if( bInputAllowed && ( aLastFilterName != aText ) )
    {
        aLastFilterName = aText;

        if( QueryBox( this, WB_YES_NO, String( CUI_RES( RID_SVXSTR_GALLERY_SEARCH ) ) ).Execute() == RET_YES )
            SearchFiles();
    }

    return 0L;
}

// ------------------------------------------------------------------------

void TPGalleryThemeProperties::SearchFiles()
{
    SearchProgress* pProgress = new SearchProgress( this, aURL );

    for ( size_t i = 0, n = aFoundList.size(); i < n; ++i )
        delete aFoundList[ i ];
    aFoundList.clear();

    aLbxFound.Clear();

    pProgress->SetFileType( aCbbFileType.GetText() );
    pProgress->SetDirectory( rtl::OUString() );
    pProgress->Update();

    pProgress->StartExecuteModal( LINK( this, TPGalleryThemeProperties, EndSearchProgressHdl ) );
}

// ------------------------------------------------------------------------

IMPL_LINK( TPGalleryThemeProperties, ClickCloseBrowserHdl, void *, EMPTYARG )
{
    if( bInputAllowed )
        aPreviewTimer.Stop();

    return 0L;
}

// ------------------------------------------------------------------------

IMPL_LINK( TPGalleryThemeProperties, ClickSearchHdl, void *, EMPTYARG )
{
    if( bInputAllowed )
    {
        try
        {
            // setup folder picker
            ::com::sun::star::uno::Reference< XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
            if( xMgr.is() )
            {
                xFolderPicker = ::com::sun::star::uno::Reference< XFolderPicker >(
                    xMgr->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.dialogs.FolderPicker" ))), UNO_QUERY );

                if ( xFolderPicker.is() )
                {
                    String  aDlgPathName( SvtPathOptions().GetGraphicPath() );
                    xFolderPicker->setDisplayDirectory(aDlgPathName);

                    aPreviewTimer.Stop();

                    ::com::sun::star::uno::Reference< XAsynchronousExecutableDialog > xAsyncDlg( xFolderPicker, UNO_QUERY );
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
            }
        }
        catch(IllegalArgumentException)
        {
#ifdef DBG_UTIL
            DBG_ERROR( "Folder picker failed with illegal arguments" );
#endif
        }
    }

    return 0L;
}

// ------------------------------------------------------------------------

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

// ------------------------------------------------------------------------

IMPL_LINK( TPGalleryThemeProperties, ClickPreviewHdl, void *, EMPTYARG )
{
    if ( bInputAllowed )
    {
        aPreviewTimer.Stop();
        aPreviewString.Erase();

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

// ------------------------------------------------------------------------

void TPGalleryThemeProperties::DoPreview()
{
    String aString( aLbxFound.GetSelectEntry() );

    if( aString != aPreviewString )
    {
        INetURLObject   _aURL( *aFoundList[ aLbxFound.GetEntryPos( aString ) ] );
        bInputAllowed = FALSE;

        if ( !aWndPreview.SetGraphic( _aURL ) )
        {
            GetParent()->LeaveWait();
            ErrorHandler::HandleError( ERRCODE_IO_NOTEXISTSPATH );
            GetParent()->EnterWait();
        }
        else if( ::avmedia::MediaWindow::isMediaURL( _aURL.GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS ) ) )
        {
            xMediaPlayer = ::avmedia::MediaWindow::createPlayer( _aURL.GetMainURL( INetURLObject::NO_DECODE ) );
            if( xMediaPlayer.is() )
                xMediaPlayer->start();
        }

        bInputAllowed = TRUE;
        aPreviewString = aString;
    }
}

// ------------------------------------------------------------------------

IMPL_LINK( TPGalleryThemeProperties, ClickTakeHdl, void*, EMPTYARG )
{
    if( bInputAllowed )
    {
        aPreviewTimer.Stop();

        if( !aLbxFound.GetSelectEntryCount() || !bEntriesFound )
        {
            SvxOpenGraphicDialog aDlg(String( RTL_CONSTASCII_USTRINGPARAM( "Gallery" ) ) );
            aDlg.EnableLink(sal_False);
            aDlg.AsLink(sal_False);

            if( !aDlg.Execute() )
                pData->pTheme->InsertURL( INetURLObject( aDlg.GetPath() ) );
        }
        else
        {
            bTakeAll = FALSE;
            TakeFiles();
        }
    }

    return 0L;
}

// ------------------------------------------------------------------------

IMPL_LINK( TPGalleryThemeProperties, ClickTakeAllHdl, void *, EMPTYARG )
{
    if( bInputAllowed )
    {
        aPreviewTimer.Stop();
        bTakeAll = TRUE;
        TakeFiles();
    }

    return 0L;
}

// ------------------------------------------------------------------------

IMPL_LINK( TPGalleryThemeProperties, SelectFoundHdl, void *, EMPTYARG )
{
    if( bInputAllowed )
    {
        BOOL bPreviewPossible = FALSE;

        aPreviewTimer.Stop();

        if( bEntriesFound )
        {
            if( aLbxFound.GetSelectEntryCount() == 1 )
            {
                aCbxPreview.Enable();
                bPreviewPossible = TRUE;
            }
            else
                aCbxPreview.Disable();

            if( aFoundList.size() )
                aBtnTakeAll.Enable();
            else
                aBtnTakeAll.Disable();
        }

        if( bPreviewPossible && aCbxPreview.IsChecked() )
            aPreviewTimer.Start();
    }

    return 0;
}

// ------------------------------------------------------------------------

IMPL_LINK( TPGalleryThemeProperties, DClickFoundHdl, void *, EMPTYARG )
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

// ------------------------------------------------------------------------

IMPL_LINK( TPGalleryThemeProperties, PreviewTimerHdl, void *, EMPTYARG )
{
    aPreviewTimer.Stop();
    DoPreview();
    return 0L;
}

// ------------------------------------------------------------------------

IMPL_LINK( TPGalleryThemeProperties, EndSearchProgressHdl, SearchProgress *, EMPTYARG )
{
  if( aFoundList.size() )
  {
      aLbxFound.SelectEntryPos( 0 );
      aBtnTakeAll.Enable();
      aCbxPreview.Enable();
      bEntriesFound = TRUE;
  }
  else
  {
      aLbxFound.InsertEntry( String( CUI_RES( RID_SVXSTR_GALLERY_NOFILES ) ) );
      aBtnTakeAll.Disable();
      aCbxPreview.Disable();
      bEntriesFound = FALSE;
  }
  return 0L;
}

// ------------------------------------------------------------------------

IMPL_LINK( TPGalleryThemeProperties, DialogClosedHdl, ::com::sun::star::ui::dialogs::DialogClosedEvent*, pEvt )
{
    DBG_ASSERT( xFolderPicker.is() == sal_True, "TPGalleryThemeProperties::DialogClosedHdl(): no folder picker" );

    String sURL = String( xFolderPicker->getDirectory() );
    StartSearchFiles( sURL, pEvt->DialogResult );

    return 0L;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
