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
#include <unotools/localedatawrapper.hxx>
#include <unotools/syslocale.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <com/sun/star/ui/dialogs/XAsynchronousExecutableDialog.hpp>
#include <dialmgr.hxx>
#include <strings.hrc>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>

using namespace ::ucbhelper;
using namespace ::cppu;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;


SearchThread::SearchThread(SearchProgress* pProgress,
                           TPGalleryThemeProperties* pBrowser,
                           const INetURLObject& rStartURL)
    : Thread("cuiSearchThread")
    , mpProgress(pProgress)
    , mpBrowser(pBrowser)
    , maStartURL(rStartURL)
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

    Application::PostUserEvent(LINK(mpProgress, SearchProgress, CleanUpHdl));
}


void SearchThread::ImplSearch( const INetURLObject& rStartURL,
                               const std::vector< OUString >& rFormats,
                               bool bRecursive )
{
    {
        SolarMutexGuard aGuard;

        mpProgress->SetDirectory( rStartURL );
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
                                         aFoundURL.GetFileExtension().toAsciiLowerCase())
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

SearchProgress::SearchProgress(weld::Window* pParent, TPGalleryThemeProperties* pTabPage, const INetURLObject& rStartURL)
    : GenericDialogController(pParent, "cui/ui/gallerysearchprogress.ui", "GallerySearchProgress")
    , startUrl_(rStartURL)
    , m_pTabPage(pTabPage)
    , m_xFtSearchDir(m_xBuilder->weld_label("dir"))
    , m_xFtSearchType(m_xBuilder->weld_label("file"))
    , m_xBtnCancel(m_xBuilder->weld_button("cancel"))
{
    m_xFtSearchType->set_size_request(m_xFtSearchType->get_preferred_size().Width(), -1);
    m_xBtnCancel->connect_clicked(LINK(this, SearchProgress, ClickCancelBtn));
}

SearchProgress::~SearchProgress()
{
}

IMPL_LINK_NOARG(SearchProgress, ClickCancelBtn, weld::Button&, void)
{
    if (m_aSearchThread.is())
        m_aSearchThread->terminate();
}

IMPL_LINK_NOARG(SearchProgress, CleanUpHdl, void*, void)
{
    if (m_aSearchThread.is())
        m_aSearchThread->join();

    m_xDialog->response(RET_OK);
}

void SearchProgress::LaunchThread()
{
    assert(!m_aSearchThread.is());
    m_aSearchThread = new SearchThread(this, m_pTabPage, startUrl_);
    m_aSearchThread->launch();
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

        mrTakenList.push_back( nPos );

        {
            SolarMutexGuard aGuard;

            mpProgress->SetFile( aURL );
            pStatusProgress->Update( i, nEntries - 1 );
            pThm->InsertURL( aURL );
        }
    }

    {
        SolarMutexGuard aGuard;

        pThm->UnlockBroadcaster();
        pStatusProgress.reset();
    }

    Application::PostUserEvent(LINK(mpProgress, TakeProgress, CleanUpHdl));
}

TakeProgress::TakeProgress(weld::Window* pParent, TPGalleryThemeProperties* pTabPage)
    : GenericDialogController(pParent, "cui/ui/galleryapplyprogress.ui",
                              "GalleryApplyProgress")
    , m_pParent(pParent)
    , m_pTabPage(pTabPage)
    , m_xFtTakeFile(m_xBuilder->weld_label("file"))
    , m_xBtnCancel(m_xBuilder->weld_button("cancel"))
{
    m_xBtnCancel->connect_clicked(LINK(this, TakeProgress, ClickCancelBtn));
}

TakeProgress::~TakeProgress()
{
}

IMPL_LINK_NOARG(TakeProgress, ClickCancelBtn, weld::Button&, void)
{
    if (maTakeThread.is())
        maTakeThread->terminate();
}


IMPL_LINK_NOARG(TakeProgress, CleanUpHdl, void*, void)
{
    if (maTakeThread.is())
        maTakeThread->join();

    std::vector<bool, std::allocator<bool> > aRemoveEntries(m_pTabPage->aFoundList.size(), false);
    std::vector< OUString >   aRemainingVector;
    sal_uInt32                  i, nCount;

    std::unique_ptr<weld::WaitObject> xWait(new weld::WaitObject(m_pParent));

    m_pTabPage->m_xLbxFound->select(-1);
    m_pTabPage->m_xLbxFound->freeze();

    // mark all taken positions in aRemoveEntries
    for( i = 0, nCount = maTakenList.size(); i < nCount; ++i )
        aRemoveEntries[ maTakenList[ i ] ] = true;
    maTakenList.clear();

    // refill found list
    for( i = 0, nCount = aRemoveEntries.size(); i < nCount; ++i )
        if( !aRemoveEntries[ i ] )
            aRemainingVector.push_back( m_pTabPage->aFoundList[i] );

    m_pTabPage->aFoundList.clear();

    for( i = 0, nCount = aRemainingVector.size(); i < nCount; ++i )
        m_pTabPage->aFoundList.push_back( aRemainingVector[ i ] );

    aRemainingVector.clear();

    // refill list box
    for( i = 0, nCount = aRemoveEntries.size(); i < nCount; ++i )
        if( !aRemoveEntries[ i ] )
            aRemainingVector.push_back(m_pTabPage->m_xLbxFound->get_text(i));

    m_pTabPage->m_xLbxFound->clear();

    for( i = 0, nCount = aRemainingVector.size(); i < nCount; ++i )
        m_pTabPage->m_xLbxFound->append_text(aRemainingVector[i]);

    aRemainingVector.clear();

    m_pTabPage->m_xLbxFound->thaw();
    m_pTabPage->SelectFoundHdl( *m_pTabPage->m_xLbxFound );

    xWait.reset();

    m_xDialog->response(RET_OK);
}

void TakeProgress::LaunchThread()
{
    assert(!maTakeThread.is());
    maTakeThread = new TakeThread(this, m_pTabPage, maTakenList);
    maTakeThread->launch();
}

ActualizeProgress::ActualizeProgress(weld::Widget* pWindow, GalleryTheme* pThm)
    : GenericDialogController(pWindow, "cui/ui/galleryupdateprogress.ui",
                              "GalleryUpdateProgress")
    , pIdle(nullptr)
    , pTheme(pThm)
    , m_xFtActualizeFile(m_xBuilder->weld_label("file"))
    , m_xBtnCancel(m_xBuilder->weld_button("cancel"))
{
    m_xBtnCancel->connect_clicked(LINK(this, ActualizeProgress, ClickCancelBtn));
}

ActualizeProgress::~ActualizeProgress()
{
}

short ActualizeProgress::run()
{
    pIdle = new Idle("ActualizeProgressTimeout");
    pIdle->SetInvokeHandler( LINK( this, ActualizeProgress, TimeoutHdl ) );
    pIdle->SetPriority( TaskPriority::LOWEST );
    pIdle->Start();

    return GenericDialogController::run();
}

IMPL_LINK_NOARG(ActualizeProgress, ClickCancelBtn, weld::Button&, void)
{
    pTheme->AbortActualize();
    m_xDialog->response(RET_OK);
}

IMPL_LINK( ActualizeProgress, TimeoutHdl, Timer*, _pTimer, void)
{
    if (_pTimer)
    {
        _pTimer->Stop();
        delete _pTimer;
    }

    pTheme->Actualize(LINK(this, ActualizeProgress, ActualizeHdl), &aStatusProgress);
    ClickCancelBtn(*m_xBtnCancel);
}

IMPL_LINK( ActualizeProgress, ActualizeHdl, const INetURLObject&, rURL, void )
{
    Application::Reschedule(true);
    m_xFtActualizeFile->set_label(GetReducedString(rURL, 30));
}

TitleDialog::TitleDialog(weld::Widget* pParent, const OUString& rOldTitle)
    : GenericDialogController(pParent, "cui/ui/gallerytitledialog.ui", "GalleryTitleDialog")
    , m_xEdit(m_xBuilder->weld_entry("entry"))
{
    m_xEdit->set_text(rOldTitle);
    m_xEdit->grab_focus();
}

TitleDialog::~TitleDialog()
{
}

GalleryIdDialog::GalleryIdDialog(weld::Widget* pParent, GalleryTheme* _pThm)
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
    const sal_uInt32 nId = GetId();
    bool        bDifferentThemeExists = false;

    for( size_t i = 0, nCount = pGal->GetThemeCount(); i < nCount && !bDifferentThemeExists; i++ )
    {
        const GalleryThemeEntry* pInfo = pGal->GetThemeInfo( i );

        if ((pInfo->GetId() == nId) && (pInfo->GetThemeName() != m_pThm->GetName()))
        {
            OUString aStr = CuiResId( RID_SVXSTR_GALLERY_ID_EXISTS ) +
                " (" + pInfo->GetThemeName() + ")";

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

GalleryThemeProperties::GalleryThemeProperties(weld::Widget* pParent,
    ExchangeData* _pData, SfxItemSet const * pItemSet)
    : SfxTabDialogController(pParent, "cui/ui/gallerythemedialog.ui",
                             "GalleryThemeDialog", pItemSet)
    , pData(_pData)
{
    AddTabPage("general", TPGalleryThemeGeneral::Create, nullptr);
    AddTabPage("files", TPGalleryThemeProperties::Create, nullptr);
    if (pData->pTheme->IsReadOnly())
        RemoveTabPage("files");

    OUString aText = m_xDialog->get_title().replaceFirst( "%1",  pData->pTheme->GetName() );

    if (pData->pTheme->IsReadOnly())
        aText +=  " " + CuiResId( RID_SVXSTR_GALLERY_READONLY );

    m_xDialog->set_title(aText);
}

void GalleryThemeProperties::PageCreated(const OString& rId, SfxTabPage &rPage)
{
    if (rId == "general")
        static_cast<TPGalleryThemeGeneral&>( rPage ).SetXChgData( pData );
    else
        static_cast<TPGalleryThemeProperties&>( rPage ).SetXChgData( pData );
}

TPGalleryThemeGeneral::TPGalleryThemeGeneral(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, "cui/ui/gallerygeneralpage.ui", "GalleryGeneralPage", &rSet)
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
    m_xFtMSShowPath->set_label(pThm->getThemeURL().GetMainURL(INetURLObject::DecodeMechanism::Unambiguous));

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

std::unique_ptr<SfxTabPage> TPGalleryThemeGeneral::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet)
{
    return std::make_unique<TPGalleryThemeGeneral>(pPage, pController, *rSet);
}

TPGalleryThemeProperties::TPGalleryThemeProperties(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, "cui/ui/galleryfilespage.ui", "GalleryFilesPage", &rSet)
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
    m_xLbxFound->set_selection_mode(SelectionMode::Multiple);
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
    xMediaPlayer.clear();
    xDialogListener.clear();
    aFilterEntryList.clear();
}

std::unique_ptr<SfxTabPage> TPGalleryThemeProperties::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet)
{
    return std::make_unique<TPGalleryThemeProperties>(pPage, pController, *rSet);
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
    static const OUStringLiteral aWildcard = u"*.";
    ::avmedia::FilterNameVector     aFilters= ::avmedia::MediaWindow::getMediaFilters();

    for(const std::pair<OUString,OUString> & aFilter : aFilters)
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
    for(const std::pair<OUString,OUString> & aFilter : aFilters)
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
    auto xProgress = std::make_shared<SearchProgress>(GetFrameWeld(), this, aURL);

    aFoundList.clear();
    m_xLbxFound->clear();

    xProgress->SetFileType( m_xCbbFileType->get_active_text() );
    xProgress->SetDirectory( INetURLObject() );

    xProgress->LaunchThread();
    weld::DialogController::runAsync(xProgress, [this](sal_Int32 nResult) {
        EndSearchProgressHdl(nResult);
    });
}

IMPL_LINK_NOARG(TPGalleryThemeProperties, ClickSearchHdl, weld::Button&, void)
{
    if( !bInputAllowed )
        return;

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

void TPGalleryThemeProperties::TakeFiles()
{
    if (m_xLbxFound->count_selected_rows() || (bTakeAll && bEntriesFound))
    {
        auto xTakeProgress = std::make_shared<TakeProgress>(GetFrameWeld(), this);
        xTakeProgress->LaunchThread();
        weld::DialogController::runAsync(xTakeProgress, [=](sal_Int32 /*nResult*/) {
            /* no postprocessing needed, pTakeProgress
               will be disposed in TakeProgress::CleanupHdl */
        });

    }
}

IMPL_LINK_NOARG(TPGalleryThemeProperties, ClickPreviewHdl, weld::ToggleButton&, void)
{
    if ( !bInputAllowed )
        return;

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

void TPGalleryThemeProperties::DoPreview()
{
    int nIndex = m_xLbxFound->get_selected_index();
    OUString aString(m_xLbxFound->get_text(nIndex));

    if (aString == aPreviewString)
        return;

    INetURLObject _aURL(aFoundList[nIndex]);
    bInputAllowed = false;

    if (!m_aWndPreview.SetGraphic(_aURL))
    {
        weld::WaitObject aWaitObject(GetFrameWeld());
        ErrorHandler::HandleError(ERRCODE_IO_NOTEXISTSPATH, GetFrameWeld());
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

IMPL_LINK_NOARG(TPGalleryThemeProperties, ClickTakeHdl, weld::Button&, void)
{
    if( !bInputAllowed )
        return;

    aPreviewTimer.Stop();

    if (!m_xLbxFound->count_selected_rows() || !bEntriesFound)
    {
        SvxOpenGraphicDialog aDlg(CuiResId(RID_SVXSTR_KEY_GALLERY_DIR), GetFrameWeld());
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
    if (!bInputAllowed)
        return;

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

IMPL_LINK_NOARG(TPGalleryThemeProperties, DClickFoundHdl, weld::TreeView&, bool)
{
    if( bInputAllowed )
    {
        aPreviewTimer.Stop();

        if (m_xLbxFound->count_selected_rows() == 1 && bEntriesFound)
            ClickTakeHdl(*m_xBtnTake);
    }
    return true;
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
