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

#include <config_features.h>

#include <sal/config.h>

#include <algorithm>
#include <cassert>

#include <vcl/errinf.hxx>
#include <ucbhelper/content.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <avmedia/mediawindow.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/opengrf.hxx>
#include <vcl/graphicfilter.hxx>
#include <svx/gallery1.hxx>
#include <svx/galtheme.hxx>
#include <cuigaldlg.hxx>
#include <bitmaps.hlst>
#include <unotools/syslocale.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <com/sun/star/ui/dialogs/XAsynchronousExecutableDialog.hpp>
#include <sfx2/sfxuno.hxx>
#include <dialmgr.hxx>
#include <strings.hrc>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>

using namespace ::ucbhelper;
using namespace ::cppu;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;


SearchThread::SearchThread( SearchProgress* pProgress,
                            TPGalleryThemeProperties* pBrowser,
                            const INetURLObject& rStartURL ) :
        Thread      ( "cuiSearchThread" ),
        mpProgress  ( pProgress ),
        mpBrowser   ( pBrowser ),
        maStartURL  ( rStartURL )
{
}

SearchThread::~SearchThread()
{
}

void SearchThread::execute()
{
    const OUString aFileType(mpBrowser->m_xCbbFileType->get_active_text());

    if (!aFileType.isEmpty())
    {
        const int nFileNumber = mpBrowser->m_xCbbFileType->find_text(aFileType);
        sal_Int32 nBeginFormat, nEndFormat;
        std::vector< OUString > aFormats;

        if( !nFileNumber || nFileNumber == -1)
        {
            nBeginFormat = 1;
            nEndFormat = mpBrowser->m_xCbbFileType->get_count() - 1;
        }
        else
            nBeginFormat = nEndFormat = nFileNumber;

        for (sal_Int32 i = nBeginFormat; i <= nEndFormat; ++i)
            aFormats.push_back( mpBrowser->aFilterEntryList[ i ]->aFilterName.toAsciiLowerCase() );

        ImplSearch( maStartURL, aFormats, mpBrowser->bSearchRecursive );
    }

    Application::PostUserEvent( LINK( mpProgress, SearchProgress, CleanUpHdl ), nullptr, true );
}


void SearchThread::ImplSearch( const INetURLObject& rStartURL,
                               const std::vector< OUString >& rFormats,
                               bool bRecursive )
{
    {
        SolarMutexGuard aGuard;

        mpProgress->SetDirectory( rStartURL );
        mpProgress->Flush();
    }

    try
    {
        css::uno::Reference< XCommandEnvironment > xEnv;
        Content aCnt( rStartURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), xEnv, comphelper::getProcessComponentContext() );
        Sequence< OUString > aProps( 2 );

        aProps.getArray()[ 0 ] = "IsFolder";
        aProps.getArray()[ 1 ] = "IsDocument";
        css::uno::Reference< XResultSet > xResultSet(
            aCnt.createCursor( aProps ) );

        if( xResultSet.is() )
        {
            css::uno::Reference< XContentAccess > xContentAccess( xResultSet, UNO_QUERY_THROW );
            css::uno::Reference< XRow > xRow( xResultSet, UNO_QUERY_THROW );

            while( xResultSet->next() && schedule() )
            {
                INetURLObject   aFoundURL( xContentAccess->queryContentIdentifierString() );
                DBG_ASSERT( aFoundURL.GetProtocol() != INetProtocol::NotValid, "invalid URL" );

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
                              std::find( rFormats.begin(),
                                           rFormats.end(),
                                           GraphicDescriptor::GetImportFormatShortName(
                                               aDesc.GetFileFormat() ).toAsciiLowerCase() )
                              != rFormats.end() ) ||
                            std::find( rFormats.begin(),
                                         rFormats.end(),
                                         aFoundURL.GetExtension().toAsciiLowerCase() )
                            != rFormats.end() )
                        {
                            SolarMutexGuard aGuard;

                            mpBrowser->aFoundList.push_back(
                                aFoundURL.GetMainURL( INetURLObject::DecodeMechanism::NONE )
                            );
                            mpBrowser->m_xLbxFound->insert_text(
                                mpBrowser->aFoundList.size() - 1,
                                GetReducedString(aFoundURL, 50));
                        }
                    }
                }
            }
        }
    }
    catch (const ContentCreationException&)
    {
    }
    catch (const css::uno::RuntimeException&)
    {
    }
    catch (const css::uno::Exception&)
    {
    }
}


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


SearchProgress::~SearchProgress()
{
    disposeOnce();
}


void SearchProgress::dispose()
{
    m_pFtSearchDir.clear();
    m_pFtSearchType.clear();
    m_pBtnCancel.clear();
    parent_.clear();
    ModalDialog::dispose();
}


IMPL_LINK_NOARG(SearchProgress, ClickCancelBtn, Button*, void)
{
    if (maSearchThread.is())
        maSearchThread->terminate();
}


IMPL_LINK_NOARG(SearchProgress, CleanUpHdl, void*, void)
{
    if (maSearchThread.is())
        maSearchThread->join();

    EndDialog( RET_OK );

    disposeOnce();
}

short SearchProgress::Execute()
{
    OSL_FAIL( "SearchProgress cannot be executed via Dialog::Execute!\n"
               "It creates a thread that will call back to VCL apartment => deadlock!\n"
               "Use Dialog::StartExecuteModal to execute the dialog!" );
    return RET_CANCEL;
}

bool SearchProgress::StartExecuteAsync(VclAbstractDialog::AsyncContext &rCtx)
{
    assert(!maSearchThread.is());
    maSearchThread = new SearchThread(
        this, static_cast< TPGalleryThemeProperties * >(parent_.get()), startUrl_);
    maSearchThread->launch();
    return ModalDialog::StartExecuteAsync(rCtx);
}

TakeThread::TakeThread(
    TakeProgress* pProgress,
    TPGalleryThemeProperties* pBrowser,
    TokenList_impl& rTakenList
) :
    Thread      ( "cuiTakeThread" ),
    mpProgress  ( pProgress ),
    mpBrowser   ( pBrowser ),
    mrTakenList ( rTakenList )
{
}


TakeThread::~TakeThread()
{
}


void TakeThread::execute()
{
    sal_Int32           nEntries;
    GalleryTheme*       pThm = mpBrowser->GetXChgData()->pTheme;
    std::unique_ptr<GalleryProgress> pStatusProgress;

    std::vector<int> aSelectedRows;

    {
        SolarMutexGuard aGuard;
        pStatusProgress.reset(new GalleryProgress);
        if (mpBrowser->bTakeAll)
            nEntries = mpBrowser->m_xLbxFound->n_children();
        else
        {
            aSelectedRows = mpBrowser->m_xLbxFound->get_selected_rows();
            nEntries = aSelectedRows.size();
        }
        pThm->LockBroadcaster();
    }

    for( sal_Int32 i = 0; i < nEntries && schedule(); ++i )
    {
        const sal_Int32 nPos = mpBrowser->bTakeAll ? i : aSelectedRows[i];
        const INetURLObject aURL( mpBrowser->aFoundList[ nPos ]);

        mrTakenList.push_back( static_cast<sal_uLong>(nPos) );

        {
            SolarMutexGuard aGuard;

            mpProgress->SetFile( aURL );
            pStatusProgress->Update( i, nEntries - 1 );
            mpProgress->Flush();
            pThm->InsertURL( aURL );
        }
    }

    {
        SolarMutexGuard aGuard;

        pThm->UnlockBroadcaster();
        pStatusProgress.reset();
    }

    Application::PostUserEvent( LINK( mpProgress, TakeProgress, CleanUpHdl ), nullptr, true );
}


TakeProgress::TakeProgress(vcl::Window* pWindow)
    : ModalDialog(pWindow, "GalleryApplyProgress",
        "cui/ui/galleryapplyprogress.ui")
    , window_(pWindow)
{
    get(m_pFtTakeFile, "file");
    get(m_pBtnCancel, "cancel");

    m_pBtnCancel->SetClickHdl( LINK( this, TakeProgress, ClickCancelBtn ) );
}


TakeProgress::~TakeProgress()
{
    disposeOnce();
}


void TakeProgress::dispose()
{
    m_pFtTakeFile.clear();
    m_pBtnCancel.clear();
    window_.clear();
    ModalDialog::dispose();
}


IMPL_LINK_NOARG(TakeProgress, ClickCancelBtn, Button*, void)
{
    if (maTakeThread.is())
        maTakeThread->terminate();
}


IMPL_LINK_NOARG(TakeProgress, CleanUpHdl, void*, void)
{
    if (maTakeThread.is())
        maTakeThread->join();

    TPGalleryThemeProperties*   pBrowser = static_cast<TPGalleryThemeProperties*>( GetParent() );
    std::vector<bool, std::allocator<bool> > aRemoveEntries( pBrowser->aFoundList.size(), false );
    std::vector< OUString >   aRemainingVector;
    sal_uInt32                  i, nCount;

    GetParent()->EnterWait();
    pBrowser->m_xLbxFound->select(-1);
    pBrowser->m_xLbxFound->freeze();

    // mark all taken positions in aRemoveEntries
    for( i = 0, nCount = maTakenList.size(); i < nCount; ++i )
        aRemoveEntries[ maTakenList[ i ] ] = true;
    maTakenList.clear();

    // refill found list
    for( i = 0, nCount = aRemoveEntries.size(); i < nCount; ++i )
        if( !aRemoveEntries[ i ] )
            aRemainingVector.push_back( pBrowser->aFoundList[i] );

    pBrowser->aFoundList.clear();

    for( i = 0, nCount = aRemainingVector.size(); i < nCount; ++i )
        pBrowser->aFoundList.push_back( aRemainingVector[ i ] );

    aRemainingVector.clear();

    // refill list box
    for( i = 0, nCount = aRemoveEntries.size(); i < nCount; ++i )
        if( !aRemoveEntries[ i ] )
            aRemainingVector.push_back(pBrowser->m_xLbxFound->get_text(i));

    pBrowser->m_xLbxFound->clear();

    for( i = 0, nCount = aRemainingVector.size(); i < nCount; ++i )
        pBrowser->m_xLbxFound->append_text(aRemainingVector[i]);

    aRemainingVector.clear();

    pBrowser->m_xLbxFound->thaw();
    pBrowser->SelectFoundHdl( *pBrowser->m_xLbxFound );
    GetParent()->LeaveWait();

    EndDialog( RET_OK );
    disposeOnce();
}

short TakeProgress::Execute()
{
    OSL_FAIL( "TakeProgress cannot be executed via Dialog::Execute!\n"
               "It creates a thread that will call back to VCL apartment => deadlock!\n"
               "Use Dialog::StartExecuteModal to execute the dialog!" );
    return RET_CANCEL;
}

bool TakeProgress::StartExecuteAsync(VclAbstractDialog::AsyncContext &rCtx)
{
    assert(!maTakeThread.is());
    maTakeThread = new TakeThread(
        this, static_cast< TPGalleryThemeProperties * >(window_.get()), maTakenList);
    maTakeThread->launch();
    return ModalDialog::StartExecuteAsync(rCtx);
}

ActualizeProgress::ActualizeProgress(vcl::Window* pWindow, GalleryTheme* pThm)
    : ModalDialog(pWindow, "GalleryUpdateProgress",
        "cui/ui/galleryupdateprogress.ui")
    , pIdle(nullptr)
    , pTheme(pThm)
{
    get(m_pFtActualizeFile, "file");
    get(m_pBtnCancel, "cancel");
    m_pBtnCancel->SetClickHdl( LINK( this, ActualizeProgress, ClickCancelBtn ) );
}


ActualizeProgress::~ActualizeProgress()
{
    disposeOnce();
}


void ActualizeProgress::dispose()
{
    m_pFtActualizeFile.clear();
    m_pBtnCancel.clear();
    ModalDialog::dispose();
}


short ActualizeProgress::Execute()
{
    short nRet;

    pIdle = new Idle("ActualizeProgressTimeout");
    pIdle->SetInvokeHandler( LINK( this, ActualizeProgress, TimeoutHdl ) );
    pIdle->SetPriority( TaskPriority::LOWEST );
    pIdle->Start();

    nRet = ModalDialog::Execute();

    return nRet;
}


IMPL_LINK_NOARG(ActualizeProgress, ClickCancelBtn, Button*, void)
{
    pTheme->AbortActualize();
    EndDialog( RET_OK );
}


IMPL_LINK( ActualizeProgress, TimeoutHdl, Timer*, _pTimer, void)
{
    if ( _pTimer )
    {
        _pTimer->Stop();
        delete _pTimer;
    }

    pTheme->Actualize( LINK( this, ActualizeProgress, ActualizeHdl ), &aStatusProgress );
    ClickCancelBtn( nullptr );
}


IMPL_LINK( ActualizeProgress, ActualizeHdl, const INetURLObject&, rURL, void )
{
    Application::Reschedule( true );

    Flush();

    m_pFtActualizeFile->SetText( GetReducedString( rURL, 30 ) );
    m_pFtActualizeFile->Flush();
}


TitleDialog::TitleDialog(weld::Window* pParent, const OUString& rOldTitle)
    : GenericDialogController(pParent, "cui/ui/gallerytitledialog.ui", "GalleryTitleDialog")
    , m_xEdit(m_xBuilder->weld_entry("entry"))
{
    m_xEdit->set_text(rOldTitle);
    m_xEdit->grab_focus();
}

TitleDialog::~TitleDialog()
{
}

GalleryIdDialog::GalleryIdDialog(weld::Window* pParent, GalleryTheme* _pThm)
    : GenericDialogController(pParent, "cui/ui/gallerythemeiddialog.ui", "GalleryThemeIDDialog")
    , m_pThm(_pThm)
    , m_xBtnOk(m_xBuilder->weld_button("ok"))
    , m_xLbResName(m_xBuilder->weld_combo_box("entry"))
{
    m_xLbResName->append_text("!!! No Id !!!");

    GalleryTheme::InsertAllThemes(*m_xLbResName);

    m_xLbResName->set_active(m_pThm->GetId());
    m_xLbResName->grab_focus();

    m_xBtnOk->connect_clicked(LINK(this, GalleryIdDialog, ClickOkHdl));
}

GalleryIdDialog::~GalleryIdDialog()
{
}

IMPL_LINK_NOARG(GalleryIdDialog, ClickOkHdl, weld::Button&, void)
{
    Gallery*    pGal = m_pThm->GetParent();
    const sal_uLong nId = GetId();
    bool        bDifferentThemeExists = false;

    for( sal_uLong i = 0, nCount = pGal->GetThemeCount(); i < nCount && !bDifferentThemeExists; i++ )
    {
        const GalleryThemeEntry* pInfo = pGal->GetThemeInfo( i );

        if ((pInfo->GetId() == nId) && (pInfo->GetThemeName() != m_pThm->GetName()))
        {
            OUString aStr( CuiResId( RID_SVXSTR_GALLERY_ID_EXISTS ) );

            aStr += " (" + pInfo->GetThemeName() + ")";

            std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                          VclMessageType::Info, VclButtonsType::Ok,
                                                          aStr));
            xInfoBox->run();
            m_xLbResName->grab_focus();
            bDifferentThemeExists = true;
        }
    }

    if (!bDifferentThemeExists)
        m_xDialog->response(RET_OK);
}

GalleryThemeProperties::GalleryThemeProperties(vcl::Window* pParent,
    ExchangeData* _pData, SfxItemSet const * pItemSet)
    : SfxTabDialog( pParent, "GalleryThemeDialog",
        "cui/ui/gallerythemedialog.ui", pItemSet)
    , pData(_pData)
    , m_nGeneralPageId(0)
{
    m_nGeneralPageId = AddTabPage("general", TPGalleryThemeGeneral::Create, nullptr);
    sal_uInt16 nFilesPageId = AddTabPage("files", TPGalleryThemeProperties::Create, nullptr);

    if( pData->pTheme->IsReadOnly() )
        RemoveTabPage(nFilesPageId);

    OUString aText = GetText().replaceFirst( "%1",  pData->pTheme->GetName() );

    if( pData->pTheme->IsReadOnly() )
        aText +=  " " + CuiResId( RID_SVXSTR_GALLERY_READONLY );

    SetText( aText );
}

void GalleryThemeProperties::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    if (nId == m_nGeneralPageId)
        static_cast<TPGalleryThemeGeneral&>( rPage ).SetXChgData( pData );
    else
        static_cast<TPGalleryThemeProperties&>( rPage ).SetXChgData( pData );
}

TPGalleryThemeGeneral::TPGalleryThemeGeneral(TabPageParent pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "cui/ui/gallerygeneralpage.ui", "GalleryGeneralPage", &rSet)
    , pData(nullptr)
    , m_xFiMSImage(m_xBuilder->weld_image("image"))
    , m_xEdtMSName(m_xBuilder->weld_entry("name"))
    , m_xFtMSShowType(m_xBuilder->weld_label("type"))
    , m_xFtMSShowPath(m_xBuilder->weld_label("location"))
    , m_xFtMSShowContent(m_xBuilder->weld_label("contents"))
    , m_xFtMSShowChangeDate(m_xBuilder->weld_label("modified"))
{
}

void TPGalleryThemeGeneral::SetXChgData( ExchangeData* _pData )
{
    pData = _pData;

    GalleryTheme*       pThm = pData->pTheme;
    OUString            aOutStr( OUString::number(pThm->GetObjectCount()) );
    OUString            aObjStr( CuiResId( RID_SVXSTR_GALLERYPROPS_OBJECT ) );
    OUString            aAccess;
    OUString            aType( SvxResId( RID_SVXSTR_GALLERYPROPS_GALTHEME ) );
    bool            bReadOnly = pThm->IsReadOnly();

    m_xEdtMSName->set_text(pThm->GetName());
    m_xEdtMSName->set_editable(!bReadOnly);
    m_xEdtMSName->set_sensitive(!bReadOnly);

    if( pThm->IsReadOnly() )
        aType += CuiResId( RID_SVXSTR_GALLERY_READONLY );

    m_xFtMSShowType->set_label(aType);
    m_xFtMSShowPath->set_label(pThm->GetSdgURL().GetMainURL(INetURLObject::DecodeMechanism::Unambiguous));

    // singular or plural?
    if ( 1 == pThm->GetObjectCount() )
        aObjStr = aObjStr.getToken( 0, ';' );
    else
        aObjStr = aObjStr.getToken( 1, ';' );

    aOutStr += " " + aObjStr;

    m_xFtMSShowContent->set_label(aOutStr);

    // get locale wrapper (singleton)
    const SvtSysLocale aSysLocale;
    const LocaleDataWrapper&    aLocaleData = aSysLocale.GetLocaleData();

    // ChangeDate/Time
    aAccess = aLocaleData.getDate( pData->aThemeChangeDate ) + ", " + aLocaleData.getTime( pData->aThemeChangeTime );
    m_xFtMSShowChangeDate->set_label(aAccess);

    // set image
    OUString sId;

    if( pThm->IsReadOnly() )
        sId = RID_SVXBMP_THEME_READONLY_BIG;
    else if( pThm->IsDefault() )
        sId = RID_SVXBMP_THEME_DEFAULT_BIG;
    else
        sId = RID_SVXBMP_THEME_NORMAL_BIG;

    m_xFiMSImage->set_from_icon_name(sId);
}

bool TPGalleryThemeGeneral::FillItemSet( SfxItemSet* /*rSet*/ )
{
    pData->aEditedTitle = m_xEdtMSName->get_text();
    return true;
}

VclPtr<SfxTabPage> TPGalleryThemeGeneral::Create(TabPageParent pParent, const SfxItemSet* rSet)
{
    return VclPtr<TPGalleryThemeGeneral>::Create(pParent, *rSet);
}

TPGalleryThemeProperties::TPGalleryThemeProperties(TabPageParent pWindow, const SfxItemSet& rSet)
    : SfxTabPage(pWindow, "cui/ui/galleryfilespage.ui", "GalleryFilesPage", &rSet)
    , pData(nullptr)
    , bEntriesFound(false)
    , bInputAllowed(true)
    , bTakeAll(false)
    , bSearchRecursive(false)
    , xDialogListener(new ::svt::DialogClosedListener())
    , m_xCbbFileType(m_xBuilder->weld_combo_box("filetype"))
    , m_xLbxFound(m_xBuilder->weld_tree_view("files"))
    , m_xBtnSearch(m_xBuilder->weld_button("findfiles"))
    , m_xBtnTake(m_xBuilder->weld_button("add"))
    , m_xBtnTakeAll(m_xBuilder->weld_button("addall"))
    , m_xCbxPreview(m_xBuilder->weld_check_button("preview"))
    , m_xWndPreview(new weld::CustomWeld(*m_xBuilder, "image", m_aWndPreview))
{
    m_xLbxFound->set_size_request(m_xLbxFound->get_approximate_digit_width() * 35,
                                  m_xLbxFound->get_height_rows(15));
    m_xLbxFound->set_selection_mode(true);
    xDialogListener->SetDialogClosedLink( LINK( this, TPGalleryThemeProperties, DialogClosedHdl ) );
}

void TPGalleryThemeProperties::SetXChgData( ExchangeData* _pData )
{
    pData = _pData;

    aPreviewTimer.SetInvokeHandler( LINK( this, TPGalleryThemeProperties, PreviewTimerHdl ) );
    aPreviewTimer.SetTimeout( 500 );
    m_xBtnSearch->connect_clicked(LINK(this, TPGalleryThemeProperties, ClickSearchHdl));
    m_xBtnTake->connect_clicked(LINK(this, TPGalleryThemeProperties, ClickTakeHdl));
    m_xBtnTakeAll->connect_clicked(LINK(this, TPGalleryThemeProperties, ClickTakeAllHdl));
    m_xCbxPreview->connect_toggled(LINK(this, TPGalleryThemeProperties, ClickPreviewHdl));
    m_xCbbFileType->connect_changed(LINK(this, TPGalleryThemeProperties, SelectFileTypeHdl));
    m_xLbxFound->connect_row_activated(LINK(this, TPGalleryThemeProperties, DClickFoundHdl));
    m_xLbxFound->connect_changed(LINK(this, TPGalleryThemeProperties, SelectFoundHdl));
    m_xLbxFound->append_text(CuiResId(RID_SVXSTR_GALLERY_NOFILES));
    m_xLbxFound->show();

    FillFilterList();

    m_xBtnTake->set_sensitive(true);
    m_xBtnTakeAll->set_sensitive(false);
    m_xCbxPreview->set_sensitive(false);
}

void TPGalleryThemeProperties::StartSearchFiles( const OUString& _rFolderURL, short _nDlgResult )
{
    if ( RET_OK == _nDlgResult )
    {
        aURL = INetURLObject( _rFolderURL );
        bSearchRecursive = true;    // UI choice no longer possible, windows file picker allows no user controls
        SearchFiles();
    }
}

TPGalleryThemeProperties::~TPGalleryThemeProperties()
{
    disposeOnce();
}

void TPGalleryThemeProperties::dispose()
{
    xMediaPlayer.clear();
    xDialogListener.clear();
    aFilterEntryList.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> TPGalleryThemeProperties::Create(TabPageParent pParent, const SfxItemSet* rSet)
{
    return VclPtr<TPGalleryThemeProperties>::Create(pParent, *rSet);
}

OUString TPGalleryThemeProperties::addExtension( const OUString& _rDisplayText, const OUString& _rExtension )
{
    OUString sRet = _rDisplayText;
    if ( sRet.indexOf( "(*.*)" ) == -1 )
    {
        sRet += " (" + _rExtension + ")";
    }
    return sRet;
}

void TPGalleryThemeProperties::FillFilterList()
{
    GraphicFilter &rFilter = GraphicFilter::GetGraphicFilter();
    OUString            aExt;
    OUString            aName;
    sal_uInt16          i, nKeyCount;

    // graphic filters
    for( i = 0, nKeyCount = rFilter.GetImportFormatCount(); i < nKeyCount; i++ )
    {
        aExt = rFilter.GetImportFormatShortName( i );
        aName = rFilter.GetImportFormatName( i );
        size_t entryIndex = 0;
        FilterEntry* pTestEntry = aFilterEntryList.empty() ? nullptr : aFilterEntryList[ entryIndex ].get();
        bool bInList = false;

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
                       ? aFilterEntryList[ entryIndex ].get() : nullptr;
        }
        if ( !bInList )
        {
            std::unique_ptr<FilterEntry> pFilterEntry(new FilterEntry);
            pFilterEntry->aFilterName = aExt;
            m_xCbbFileType->append_text(aName);
            aFilterEntryList.push_back(std::move(pFilterEntry));
        }
    }

#if HAVE_FEATURE_AVMEDIA
    // media filters
    static const char aWildcard[] = "*.";
    ::avmedia::FilterNameVector     aFilters;
    ::avmedia::MediaWindow::getMediaFilters( aFilters );

    for(std::pair<OUString,OUString> & aFilter : aFilters)
    {
        for( sal_Int32 nIndex = 0; nIndex >= 0; )
        {
            OUString aFilterWildcard( aWildcard );

            std::unique_ptr<FilterEntry> pFilterEntry(new FilterEntry);
            pFilterEntry->aFilterName = aFilter.second.getToken( 0, ';', nIndex );
            aFilterWildcard += pFilterEntry->aFilterName;
            m_xCbbFileType->append_text(addExtension(aFilter.first, aFilterWildcard));
            aFilterEntryList.push_back( std::move(pFilterEntry) );
        }
    }
#endif

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

#if HAVE_FEATURE_AVMEDIA
    // media filters
    for(std::pair<OUString,OUString> & aFilter : aFilters)
    {
        for( sal_Int32 nIndex = 0; nIndex >= 0; )
        {
            if ( !aExtensions.isEmpty() )
                aExtensions += ";";
            aExtensions += aWildcard + aFilter.second.getToken( 0, ';', nIndex );
        }
     }
#endif

#if defined(_WIN32)
    if (aExtensions.getLength() > 240)
        aExtensions = "*.*";
#endif

    std::unique_ptr<FilterEntry> pFilterEntry(new FilterEntry);
    pFilterEntry->aFilterName = CuiResId(RID_SVXSTR_GALLERY_ALLFILES);
    pFilterEntry->aFilterName = addExtension(pFilterEntry->aFilterName, aExtensions);
    m_xCbbFileType->insert_text(0, pFilterEntry->aFilterName);
    m_xCbbFileType->set_active(0);
    aFilterEntryList.insert(aFilterEntryList.begin(), std::move(pFilterEntry));
}

IMPL_LINK_NOARG(TPGalleryThemeProperties, SelectFileTypeHdl, weld::ComboBox&, void)
{
    OUString aText(m_xCbbFileType->get_active_text());

    if( bInputAllowed && ( aLastFilterName != aText ) )
    {
        aLastFilterName = aText;

        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetFrameWeld(), "cui/ui/queryupdategalleryfilelistdialog.ui"));
        std::unique_ptr<weld::MessageDialog> xQuery(xBuilder->weld_message_dialog("QueryUpdateFileListDialog"));
        if (xQuery->run() == RET_YES)
            SearchFiles();
    }
}

void TPGalleryThemeProperties::SearchFiles()
{
    VclPtrInstance<SearchProgress> pProgress( this, aURL );

    aFoundList.clear();
    m_xLbxFound->clear();

    pProgress->SetFileType( m_xCbbFileType->get_active_text() );
    pProgress->SetDirectory( INetURLObject() );
    pProgress->Update();

    pProgress->StartExecuteAsync([=](sal_Int32 nResult){
        EndSearchProgressHdl(nResult);
    });
}

IMPL_LINK_NOARG(TPGalleryThemeProperties, ClickSearchHdl, weld::Button&, void)
{
    if( bInputAllowed )
    {
        try
        {
            // setup folder picker
            css::uno::Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
            xFolderPicker = FolderPicker::create(xContext);

            OUString  aDlgPathName( SvtPathOptions().GetGraphicPath() );
            xFolderPicker->setDisplayDirectory(aDlgPathName);

            aPreviewTimer.Stop();

            css::uno::Reference< XAsynchronousExecutableDialog > xAsyncDlg( xFolderPicker, UNO_QUERY );
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
            }
        }
        catch (const IllegalArgumentException&)
        {
            OSL_FAIL( "Folder picker failed with illegal arguments" );
        }
    }
}

void TPGalleryThemeProperties::TakeFiles()
{
    if (m_xLbxFound->count_selected_rows() || (bTakeAll && bEntriesFound))
    {
        VclPtrInstance<TakeProgress> pTakeProgress( this );
        pTakeProgress->Update();

        pTakeProgress->StartExecuteAsync([=](sal_Int32 /*nResult*/){
            /* no postprocessing needed, pTakeProgress
               will be disposed in TakeProgress::CleanupHdl */
        });
    }
}

IMPL_LINK_NOARG(TPGalleryThemeProperties, ClickPreviewHdl, weld::ToggleButton&, void)
{
    if ( bInputAllowed )
    {
        aPreviewTimer.Stop();
        aPreviewString.clear();

        if (!m_xCbxPreview->get_active())
        {
            xMediaPlayer.clear();
            m_aWndPreview.SetGraphic(Graphic());
            m_aWndPreview.Invalidate();
        }
        else
            DoPreview();
    }
}

void TPGalleryThemeProperties::DoPreview()
{
    int nIndex = m_xLbxFound->get_selected_index();
    OUString aString(m_xLbxFound->get_text(nIndex));

    if (aString != aPreviewString)
    {
        INetURLObject _aURL(aFoundList[nIndex]);
        bInputAllowed = false;

        if (!m_aWndPreview.SetGraphic(_aURL))
        {
            GetParent()->LeaveWait();
            ErrorHandler::HandleError(ERRCODE_IO_NOTEXISTSPATH, GetDialogFrameWeld());
            GetParent()->EnterWait();
        }
#if HAVE_FEATURE_AVMEDIA
        else if( ::avmedia::MediaWindow::isMediaURL( _aURL.GetMainURL( INetURLObject::DecodeMechanism::Unambiguous ), "" ) )
        {
            xMediaPlayer = ::avmedia::MediaWindow::createPlayer( _aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), "" );
            if( xMediaPlayer.is() )
                xMediaPlayer->start();
        }
#endif
        bInputAllowed = true;
        aPreviewString = aString;
    }
}

IMPL_LINK_NOARG(TPGalleryThemeProperties, ClickTakeHdl, weld::Button&, void)
{
    if( bInputAllowed )
    {
        aPreviewTimer.Stop();

        if (!m_xLbxFound->count_selected_rows() || !bEntriesFound)
        {
            SvxOpenGraphicDialog aDlg("Gallery", GetFrameWeld());
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
}

IMPL_LINK_NOARG(TPGalleryThemeProperties, ClickTakeAllHdl, weld::Button&, void)
{
    if( bInputAllowed )
    {
        aPreviewTimer.Stop();
        bTakeAll = true;
        TakeFiles();
    }
}

IMPL_LINK_NOARG(TPGalleryThemeProperties, SelectFoundHdl, weld::TreeView&, void)
{
    if (bInputAllowed)
    {
        bool bPreviewPossible = false;

        aPreviewTimer.Stop();

        if( bEntriesFound )
        {
            if (m_xLbxFound->count_selected_rows() == 1)
            {
                m_xCbxPreview->set_sensitive(true);
                bPreviewPossible = true;
            }
            else
                m_xCbxPreview->set_sensitive(false);

            if( !aFoundList.empty() )
                m_xBtnTakeAll->set_sensitive(true);
            else
                m_xBtnTakeAll->set_sensitive(false);
        }

        if (bPreviewPossible && m_xCbxPreview->get_active())
            aPreviewTimer.Start();
    }
}

IMPL_LINK_NOARG(TPGalleryThemeProperties, DClickFoundHdl, weld::TreeView&, void)
{
    if( bInputAllowed )
    {
        aPreviewTimer.Stop();

        if (m_xLbxFound->count_selected_rows() == 1 && bEntriesFound)
            ClickTakeHdl(*m_xBtnTake);
    }
}

IMPL_LINK_NOARG(TPGalleryThemeProperties, PreviewTimerHdl, Timer *, void)
{
    aPreviewTimer.Stop();
    DoPreview();
}

void TPGalleryThemeProperties::EndSearchProgressHdl(sal_Int32 /*nResult*/)
{
  if( !aFoundList.empty() )
  {
      m_xLbxFound->select(0);
      m_xBtnTakeAll->set_sensitive(true);
      m_xCbxPreview->set_sensitive(true);
      bEntriesFound = true;
  }
  else
  {
      m_xLbxFound->append_text(CuiResId(RID_SVXSTR_GALLERY_NOFILES));
      m_xBtnTakeAll->set_sensitive(false);
      m_xCbxPreview->set_sensitive(false);
      bEntriesFound = false;
  }
}

IMPL_LINK( TPGalleryThemeProperties, DialogClosedHdl, css::ui::dialogs::DialogClosedEvent*, pEvt, void )
{
    DBG_ASSERT( xFolderPicker.is(), "TPGalleryThemeProperties::DialogClosedHdl(): no folder picker" );

    OUString sURL = xFolderPicker->getDirectory();
    StartSearchFiles( sURL, pEvt->DialogResult );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
