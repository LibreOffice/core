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

#include <sal/config.h>

#include <sal/macros.h>
#include "iodlg.hxx"
#include <svtools/PlaceEditDialog.hxx>
#include "PlacesListBox.hxx"
#include "fpsofficeResMgr.hxx"
#include <tools/stream.hxx>
#include <tools/urlobj.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/timer.hxx>
#include <unotools/ucbhelper.hxx>
#include "svtools/ehdl.hxx"
#include "svl/urihelper.hxx"
#include "unotools/pathoptions.hxx"
#include "unotools/viewoptions.hxx"
#include "svtools/fileview.hxx"
#include "svtools/sfxecode.hxx"
#include "svtools/svtabbx.hxx"
#include "svtools/treelistentry.hxx"
#include <toolkit/helper/vclunohelper.hxx>

#include "svtools/helpid.hrc"
#include <svtools/svtools.hrc>
#include "OfficeFilePicker.hrc"
#include "iodlg.hrc"
#include "asyncfilepicker.hxx"
#include "iodlgimp.hxx"
#include "svtools/inettbc.hxx"
#include "unotools/syslocale.hxx"
#include "QueryFolderName.hxx"
#include <rtl/ustring.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ControlActions.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <comphelper/interaction.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>

#include <osl/file.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/waitobj.hxx>
#include <vcl/settings.hxx>

#include <com/sun/star/task/InteractionHandler.hpp>
#include "com/sun/star/ucb/InteractiveAugmentedIOException.hpp"
#include "fpinteraction.hxx"
#include <osl/process.h>

#include <officecfg/Office/Common.hxx>

#include <algorithm>
#include <functional>
#include <vector>

//#define AUTOSELECT_USERFILTER
    // define this for the experimental feature of user-filter auto selection
    // means if the user enters e.g. *.doc<enter>, and there is a filter which is responsible for *.doc files (only),
    // then this filter is selected automatically

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::sdbc;
using namespace ::fpicker;
using namespace ::utl;
using namespace ::svt;

using namespace ExtendedFilePickerElementIds;
using namespace CommonFilePickerElementIds;
using namespace InternalFilePickerElementIds;

// Time to wait while traveling in the filterbox until
// the browsebox gets filtered ( in ms).
#define TRAVELFILTER_TIMEOUT    750

// functions -------------------------------------------------------------

namespace
{


    OUString getMostCurrentFilter( SvtExpFileDlg_Impl* pImpl )
    {
        DBG_ASSERT( pImpl, "invalid impl pointer" );
        const SvtFileDialogFilter_Impl* pFilter = pImpl->_pUserFilter;

        if ( !pFilter )
            pFilter = pImpl->GetCurFilter();

        if ( !pFilter )
            return OUString();

        return pFilter->GetType();
    }


    bool restoreCurrentFilter( SvtExpFileDlg_Impl* _pImpl )
    {
        DBG_ASSERT( _pImpl->GetCurFilter(), "restoreCurrentFilter: no current filter!" );
        DBG_ASSERT( !_pImpl->GetCurFilterDisplayName().isEmpty(), "restoreCurrentFilter: no current filter (no display name)!" );

        _pImpl->SelectFilterListEntry( _pImpl->GetCurFilterDisplayName() );

#ifdef DBG_UTIL
        OUString sSelectedDisplayName;
        DBG_ASSERT( ( _pImpl->GetSelectedFilterEntry( sSelectedDisplayName ) == _pImpl->GetCurFilter() )
                &&  ( sSelectedDisplayName == _pImpl->GetCurFilterDisplayName() ),
            "restoreCurrentFilter: inconsistence!" );
#endif
        return _pImpl->m_bNeedDelayedFilterExecute;
    }


    OUString GetFsysExtension_Impl( const OUString& rFile, const OUString& rLastFilterExt )
    {
        sal_Int32 nDotPos = rFile.lastIndexOf( '.' );
        if ( nDotPos != -1 )
        {
            if ( !rLastFilterExt.isEmpty() )
            {
                if ( rFile.copy( nDotPos + 1 ).equalsIgnoreAsciiCase( rLastFilterExt ) )
                    return rLastFilterExt;
            }
            else
                return rFile.copy( nDotPos );
        }
        return OUString();
    }


    void SetFsysExtension_Impl( OUString& rFile, const OUString& rExtension )
    {
        const sal_Unicode* p0 = rFile.getStr();
        const sal_Unicode* p1 = p0 + rFile.getLength() - 1;
        while ( p1 >= p0 && *p1 != '.' )
            p1--;
        if ( p1 >= p0 )
            // remove old extension
            rFile = rFile.copy( 0, p1 - p0 + 1 - ( rExtension.getLength() > 0 ? 0 : 1 ) );
        else if ( !rExtension.isEmpty() )
            // no old extension
            rFile += ".";
        rFile += rExtension;
    }

    void lcl_autoUpdateFileExtension( SvtFileDialog* _pDialog, const OUString& _rLastFilterExt )
    {
        // if auto extension is enabled ....
        if ( _pDialog->isAutoExtensionEnabled() )
        {
            // automatically switch to the extension of the (maybe just newly selected) extension
            OUString aNewFile = _pDialog->getCurrentFileText( );
            OUString aExt = GetFsysExtension_Impl( aNewFile, _rLastFilterExt );

            // but only if there already is an extension
            if ( !aExt.isEmpty() )
            {
                // check if it is a real file extension, and not only the "post-dot" part in
                // a directory name
                bool bRealExtensions = true;
                if ( -1 != aExt.indexOf( '/' ) )
                    bRealExtensions = false;
                else if ( -1 != aExt.indexOf( '\\' ) )
                    bRealExtensions = false;
                else
                {
                    // no easy way to tell, because the part containing the dot already is the last
                    // segment of the complete file name
                    // So we have to check if the file name denotes a folder or a file.
                    // For performance reasons, we do this for file urls only
                    INetURLObject aURL( aNewFile );
                    if ( INetProtocol::NotValid == aURL.GetProtocol() )
                    {
                        OUString sURL;
                        if ( osl::FileBase::getFileURLFromSystemPath( aNewFile, sURL )
                             == osl::FileBase::E_None )
                            aURL = INetURLObject( sURL );
                    }
                    if ( INetProtocol::File == aURL.GetProtocol() )
                    {
                        try
                        {
                            bRealExtensions = !_pDialog->ContentIsFolder( aURL.GetMainURL( INetURLObject::NO_DECODE ) );
                        }
                        catch( const css::uno::Exception& )
                        {
                            DBG_WARNING( "Exception in lcl_autoUpdateFileExtension" );
                        }
                    }
                }

                if ( bRealExtensions )
                {
                    SetFsysExtension_Impl( aNewFile, _pDialog->GetDefaultExt() );
                    _pDialog->setCurrentFileText( aNewFile );
                }
            }
        }
    }


    bool lcl_getHomeDirectory( const OUString& _rForURL, OUString& /* [out] */ _rHomeDir )
    {
        _rHomeDir.clear();

        // now ask the content broker for a provider for this scheme

        try
        {
            // get the provider for the current scheme
            Reference< XContentProvider > xProvider(
                UniversalContentBroker::create(
                    comphelper::getProcessComponentContext() )->
                queryContentProvider( _rForURL ) );

            DBG_ASSERT( xProvider.is(), "lcl_getHomeDirectory: could not find a (valid) content provider for the current URL!" );
            Reference< XPropertySet > xProviderProps( xProvider, UNO_QUERY );
            if ( xProviderProps.is() )
            {
                Reference< XPropertySetInfo > xPropInfo = xProviderProps->getPropertySetInfo();
                const OUString sHomeDirPropertyName( "HomeDirectory" );
                if ( !xPropInfo.is() || xPropInfo->hasPropertyByName( sHomeDirPropertyName ) )
                {
                    OUString sHomeDirectory;
                    xProviderProps->getPropertyValue( sHomeDirPropertyName ) >>= sHomeDirectory;
                    _rHomeDir = sHomeDirectory;
                }
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "lcl_getHomeDirectory: caught an exception!" );
        }
        return !_rHomeDir.isEmpty();
    }


    static OUString lcl_ensureFinalSlash( const OUString& _rDir )
    {
        INetURLObject aWorkPathObj( _rDir, INetProtocol::File );
        aWorkPathObj.setFinalSlash();
        return  aWorkPathObj.GetMainURL( INetURLObject::NO_DECODE );
    }


    /** retrieves the value of an environment variable
        @return <TRUE/> if and only if the retrieved string value is not empty
    */
    bool getEnvironmentValue( const sal_Char* _pAsciiEnvName, OUString& _rValue )
    {
        _rValue.clear();
        OUString sEnvName = OUString::createFromAscii( _pAsciiEnvName );
        osl_getEnvironment( sEnvName.pData, &_rValue.pData );
        return !_rValue.isEmpty();
    }
}

// SvtFileDialog

SvtFileDialog::SvtFileDialog
(
    vcl::Window* _pParent,
    WinBits nBits,
    WinBits nExtraBits
) :
    SvtFileDialog_Base( _pParent, "ExplorerFileDialog", "fps/ui/explorerfiledialog.ui" )

    ,_pCbReadOnly( NULL )
    ,_pCbLinkBox( NULL)
    ,_pCbPreviewBox( NULL )
    ,_pCbSelection( NULL )
    ,_pPbPlay( NULL )
    ,_pPrevWin( NULL )
    ,_pPrevBmp( NULL )
    ,_pFileView( NULL )
    ,_pFileNotifier( NULL )
    ,_pImp( new SvtExpFileDlg_Impl( nBits ) )
    ,_nExtraBits( nExtraBits )
    ,_bIsInExecute( false )
    ,m_bInExecuteAsync( false )
    ,m_bHasFilename( false )
    ,m_context(comphelper::getProcessComponentContext())
{
    Init_Impl( nBits );
}



SvtFileDialog::SvtFileDialog ( vcl::Window* _pParent, WinBits nBits )
    :SvtFileDialog_Base( _pParent, "ExplorerFileDialog", "fps/ui/explorerfiledialog.ui" )
    ,_pCbReadOnly( NULL )
    ,_pCbLinkBox( NULL)
    ,_pCbPreviewBox( NULL )
    ,_pCbSelection( NULL )
    ,_pPbPlay( NULL )
    ,_pPrevWin( NULL )
    ,_pPrevBmp( NULL )
    ,_pFileView( NULL )
    ,_pFileNotifier( NULL )
    ,_pImp( new SvtExpFileDlg_Impl( nBits ) )
    ,_nExtraBits( 0L )
    ,_bIsInExecute( false )
    ,m_bHasFilename( false )
{
    Init_Impl( nBits );
}

class CustomContainer : public vcl::Window
{
    enum FocusState
    {
        Prev = 0,
        Places,
        Add,
        Delete,
        FileView,
        Next,
        FocusCount
    };

    SvtExpFileDlg_Impl* _pImp;
    VclPtr<SvtFileView> _pFileView;
    VclPtr<Splitter>    _pSplitter;

    int m_nCurrentFocus;
    VclPtr<vcl::Window> m_pFocusWidgets[FocusState::FocusCount];

public:
    CustomContainer(vcl::Window *pParent)
        : Window(pParent)
        , _pImp(NULL)
        , _pFileView(NULL)
        , _pSplitter(NULL)
        , m_nCurrentFocus(FocusState::Prev)
    {
    }
    virtual ~CustomContainer() { disposeOnce(); }
    virtual void dispose() override
    {
        _pFileView.clear();
        _pSplitter.clear();
        vcl::Window::dispose();
    }

    void init(SvtExpFileDlg_Impl* pImp,
              SvtFileView* pFileView,
              Splitter* pSplitter,
              vcl::Window* pPrev,
              vcl::Window* pNext)
    {
        _pImp = pImp;
        _pFileView = pFileView;
        _pSplitter = pSplitter;

        m_pFocusWidgets[FocusState::Prev] = pPrev;
        m_pFocusWidgets[FocusState::Places] = _pImp->_pPlaces->GetPlacesListBox();
        m_pFocusWidgets[FocusState::Add] = _pImp->_pPlaces->GetAddButton();
        m_pFocusWidgets[FocusState::Delete] = _pImp->_pPlaces->GetDeleteButton();
        m_pFocusWidgets[FocusState::FileView] = pFileView;
        m_pFocusWidgets[FocusState::Next] = pNext;
    }

    virtual void Resize() override
    {
        Window::Resize();

        if (!_pImp || !_pImp->_pPlaces)
            return;

        Size aSize = GetSizePixel();

        Point aBoxPos(_pFileView->GetPosPixel());
        Size aNewSize(aSize.Width() - aBoxPos.X(), aSize.Height());
        _pFileView->SetSizePixel( aNewSize );

        // Resize the Splitter to fit the height
        Size splitterNewSize = _pSplitter->GetSizePixel( );
        splitterNewSize.Height() = aSize.Height();
        _pSplitter->SetSizePixel( splitterNewSize );
        sal_Int32 nMinX = _pImp->_pPlaces->GetPosPixel( ).X( );
        sal_Int32 nMaxX = _pFileView->GetPosPixel( ).X( ) + _pFileView->GetSizePixel( ).Width() - nMinX;
        _pSplitter->SetDragRectPixel( Rectangle( Point( nMinX, 0 ), Size( nMaxX, aSize.Width() ) ) );

        // Resize the places list box to fit the height of the FileView
        Size placesNewSize(_pImp->_pPlaces->GetSizePixel());
        placesNewSize.Height() = aSize.Height();
        _pImp->_pPlaces->SetSizePixel( placesNewSize );
    }

    void changeFocus( bool bReverse )
    {
        if( !_pFileView || !_pImp || !_pImp->_pPlaces )
            return;

        if( bReverse && m_nCurrentFocus > FocusState::Prev && m_nCurrentFocus <= FocusState::Next )
        {
            m_pFocusWidgets[m_nCurrentFocus]->SetFakeFocus(false);
            m_pFocusWidgets[m_nCurrentFocus]->LoseFocus();

            m_pFocusWidgets[--m_nCurrentFocus]->SetFakeFocus( true );
            m_pFocusWidgets[m_nCurrentFocus]->GrabFocus();
        }
        else if( !bReverse && m_nCurrentFocus >= FocusState::Prev && m_nCurrentFocus < FocusState::Next )
        {
            m_pFocusWidgets[m_nCurrentFocus]->SetFakeFocus(false);
            m_pFocusWidgets[m_nCurrentFocus]->LoseFocus();

            m_pFocusWidgets[++m_nCurrentFocus]->SetFakeFocus( true );
            m_pFocusWidgets[m_nCurrentFocus]->GrabFocus();
        }
    }

    virtual void GetFocus() override
    {
        if( !_pFileView || !_pImp || !_pImp->_pPlaces )
            return;

        GetFocusFlags aFlags = GetGetFocusFlags();

        if( aFlags & GetFocusFlags::Forward )
            m_nCurrentFocus = FocusState::Places;
        else if( aFlags & GetFocusFlags::Backward )
            m_nCurrentFocus = FocusState::FileView;

        if( m_nCurrentFocus >= FocusState::Prev && m_nCurrentFocus <= FocusState::Next )
        {
            m_pFocusWidgets[m_nCurrentFocus]->SetFakeFocus( true );
            m_pFocusWidgets[m_nCurrentFocus]->GrabFocus();
        }
    }

    virtual bool Notify( NotifyEvent& rNEvt ) override
    {
        if( rNEvt.GetType() == MouseNotifyEvent::GETFOCUS )
        {
            // we must also update counter when user change focus using mouse
            for(int i = FocusState::Prev; i <= FocusState::Next; i++)
            {
                if( rNEvt.GetWindow() == m_pFocusWidgets[i] )
                {
                    m_nCurrentFocus = i;
                    return true;
                }
            }

            // GETFOCUS for one of FileView's subcontrols
            m_nCurrentFocus = FocusState::FileView;
            return true;
        }
        if( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
        {
            const KeyEvent* pKeyEvent = rNEvt.GetKeyEvent();
            const vcl::KeyCode& rCode = pKeyEvent->GetKeyCode();
            bool bShift = rCode.IsShift();
            if( rCode.GetCode() == KEY_TAB )
            {
                changeFocus( bShift );
                return true;
            }
        }
        return Window::Notify( rNEvt );
    }
};

SvtFileDialog::~SvtFileDialog()
{
    disposeOnce();
}

void SvtFileDialog::dispose()
{
    if ( !_pImp->_aIniKey.isEmpty() )
    {
        // save window state
        SvtViewOptions aDlgOpt( E_DIALOG, _pImp->_aIniKey );
        aDlgOpt.SetWindowState(OStringToOUString(GetWindowState(), osl_getThreadTextEncoding()));
        OUString sUserData = _pFileView->GetConfigString();
        aDlgOpt.SetUserItem( OUString( "UserData" ),
                             makeAny( sUserData ) );
    }

    _pFileView->SetSelectHdl( Link<SvTreeListBox*,void>() );

    // Save bookmarked places
    if(_pImp->_pPlaces->IsUpdated()) {
        const std::vector<PlacePtr> aPlaces = _pImp->_pPlaces->GetPlaces();
        Sequence< OUString > placesUrlsList(_pImp->_pPlaces->GetNbEditablePlaces());
        Sequence< OUString > placesNamesList(_pImp->_pPlaces->GetNbEditablePlaces());
        int i(0);
        for(std::vector<PlacePtr>::const_iterator it = aPlaces.begin(); it != aPlaces.end(); ++it) {
            if((*it)->IsEditable()) {
                placesUrlsList[i] = (*it)->GetUrl();
                placesNamesList[i] = (*it)->GetName();
                ++i;
            }
        }

        std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create(m_context));
        officecfg::Office::Common::Misc::FilePickerPlacesUrls::set(placesUrlsList, batch);
        officecfg::Office::Common::Misc::FilePickerPlacesNames::set(placesNamesList, batch);
        batch->commit();
    }

    delete _pImp;
    _pFileView.disposeAndClear();
    _pSplitter.disposeAndClear();
    _pContainer.disposeAndClear();
    _pPrevBmp.disposeAndClear();
    _pCbReadOnly.clear();
    _pCbLinkBox.clear();
    _pCbPreviewBox.clear();
    _pCbSelection.clear();
    _pPbPlay.clear();
    _pPrevWin.clear();
    ModalDialog::dispose();
}

void SvtFileDialog::Init_Impl
(
    WinBits nStyle
)
{
    get(_pCbReadOnly, "readonly");
    get(_pCbLinkBox, "link");
    get(_pCbPreviewBox, "cb_preview");
    get(_pCbSelection, "selection");
    get(_pPrevWin, "preview");
    get(_pPbPlay, "play");
    get(_pImp->_pCbOptions, "options");
    get(_pImp->_pFtFileName, "file_name_label");
    get(_pImp->_pEdFileName, "file_name");
    _pImp->_pEdFileName->GetFocus();
    get(_pImp->_pFtFileType, "file_type_label");
    get(_pImp->_pLbFilter, "file_type");
    get(_pImp->_pEdCurrentPath, "current_path");
    get(_pImp->_pBtnFileOpen, "open");
    get(_pImp->_pBtnCancel, "cancel");
    get(_pImp->_pBtnHelp, "help");
    get(_pImp->_pBtnConnectToServer, "connect_to_server");
    get(_pImp->_pBtnNewFolder, "new_folder");
    get(_pImp->_pCbPassword, "password");
    get(_pImp->_pCbAutoExtension, "extension");
    get(_pImp->_pFtFileVersion, "shared_label");
    get(_pImp->_pLbFileVersion, "shared");
    get(_pImp->_pFtTemplates, "shared_label");
    get(_pImp->_pLbTemplates, "shared");
    get(_pImp->_pFtImageTemplates, "shared_label");
    get(_pImp->_pLbImageTemplates, "shared");

    _pImp->_pLbImageTemplates->setMaxWidthChars(40);
    _pImp->_pLbFilter->setMaxWidthChars(40);

    m_aImages = ImageList( SvtResId( RID_FILEPICKER_IMAGES ) );
    vcl::Window *pUpContainer = get<vcl::Window>("up");
    _pImp->_pBtnUp = VclPtr<SvtUpButton_Impl>::Create(pUpContainer, this, 0);
    _pImp->_pBtnUp->SetHelpId( HID_FILEOPEN_LEVELUP );
    _pImp->_pBtnUp->set_vexpand(true);
    _pImp->_pBtnUp->Show();

    _pImp->_nStyle = nStyle;
    _pImp->_eMode = ( nStyle & WB_SAVEAS ) ? FILEDLG_MODE_SAVE : FILEDLG_MODE_OPEN;
    _pImp->_eDlgType = FILEDLG_TYPE_FILEDLG;

    if ( ( nStyle & SFXWB_PATHDIALOG ) == SFXWB_PATHDIALOG )
        _pImp->_eDlgType = FILEDLG_TYPE_PATHDLG;

    // Set the directory for the "back to the default dir" button
    INetURLObject aStdDirObj( SvtPathOptions().GetWorkPath() );
    SetStandardDir( aStdDirObj.GetMainURL( INetURLObject::NO_DECODE ) );

    // determine range
    if ( !( nStyle & SFXWB_NOREMOTE ) )
    {
        _pImp->_nState |= FILEDLG_STATE_REMOTE;
    }

    // Create control element, the order defines the tab control.
    _pImp->_pEdFileName->SetSelectHdl( LINK( this, SvtFileDialog, EntrySelectHdl_Impl ) );
    _pImp->_pEdFileName->SetOpenHdl( LINK( this, SvtFileDialog, OpenUrlHdl_Impl ) );

    // in folder picker mode, only auto-complete directories (no files)
    bool bIsFolderPicker = ( _pImp->_eDlgType == FILEDLG_TYPE_PATHDLG );
    _pImp->_pEdFileName->SetOnlyDirectories( bIsFolderPicker );

    // in save mode, don't use the autocompletion as selection in the edit part
    bool bSaveMode = ( FILEDLG_MODE_SAVE == _pImp->_eMode );
    _pImp->_pEdFileName->SetNoURLSelection( bSaveMode );

    _pImp->_pBtnConnectToServer->SetAccessibleName( _pImp->_pBtnConnectToServer->GetQuickHelpText() );
    _pImp->_pBtnNewFolder->SetStyle( _pImp->_pBtnNewFolder->GetStyle() | WB_NOPOINTERFOCUS );
    _pImp->_pBtnUp->SetAccessibleName( _pImp->_pBtnUp->GetQuickHelpText() );
    _pImp->_pBtnNewFolder->SetAccessibleName( _pImp->_pBtnNewFolder->GetQuickHelpText() );

    if ( ( nStyle & SFXWB_MULTISELECTION ) == SFXWB_MULTISELECTION )
        _pImp->_bMultiSelection = true;

    _pContainer.reset(VclPtr<CustomContainer>::Create(get<vcl::Window>("container")));
    Size aSize(LogicToPixel(Size(270, 85), MAP_APPFONT));
    _pContainer->set_height_request(aSize.Height());
    _pContainer->set_width_request(aSize.Width());
    _pContainer->set_hexpand(true);
    _pContainer->set_vexpand(true);
    _pContainer->SetStyle( _pContainer->GetStyle() | WB_TABSTOP );

    _pFileView = VclPtr<SvtFileView>::Create( _pContainer, WB_BORDER,
                                       FILEDLG_TYPE_PATHDLG == _pImp->_eDlgType,
                                       _pImp->_bMultiSelection );
    _pFileView->Show();
    _pFileView->EnableAutoResize();
    _pFileView->SetHelpId( HID_FILEDLG_STANDARD );
    _pFileView->SetStyle( _pFileView->GetStyle() | WB_TABSTOP );

    _pSplitter = VclPtr<Splitter>::Create( _pContainer, WB_HSCROLL );
    _pSplitter->SetBackground( Wallpaper( Application::GetSettings().GetStyleSettings().GetFaceColor() ));
    _pSplitter->SetSplitHdl( LINK( this, SvtFileDialog, Split_Hdl ) );

    Image aNewFolderImg( GetButtonImage( IMG_FILEDLG_CREATEFOLDER ) );
    _pImp->_pBtnNewFolder->SetModeImage( aNewFolderImg );

    if ( nStyle & SFXWB_READONLY )
    {
        _pCbReadOnly->SetHelpId( HID_FILEOPEN_READONLY );
        _pCbReadOnly->SetText( SvtResId( STR_SVT_FILEPICKER_READONLY ) );
        _pCbReadOnly->SetClickHdl( LINK( this, SvtFileDialog, ClickHdl_Impl ) );
        _pCbReadOnly->Show();
    }

    if ( nStyle & SFXWB_PASSWORD )
    {
        _pImp->_pCbPassword->SetText( SvtResId( STR_SVT_FILEPICKER_PASSWORD ) );
        _pImp->_pCbPassword->SetClickHdl( LINK( this, SvtFileDialog, ClickHdl_Impl ) );
        _pImp->_pCbPassword->Show();
    }

    // set the ini file for extracting the size
    _pImp->_aIniKey = "FileDialog";

    AddControls_Impl( );

    // adjust the labels to the mode
    sal_uInt16 nResId = STR_EXPLORERFILE_OPEN;
    sal_uInt16 nButtonResId = 0;

    if ( nStyle & WB_SAVEAS )
    {
        nResId = STR_EXPLORERFILE_SAVE;
        nButtonResId = STR_EXPLORERFILE_BUTTONSAVE;
    }

    if ( ( nStyle & SFXWB_PATHDIALOG ) == SFXWB_PATHDIALOG )
    {
        _pImp->_pFtFileName->SetText( SvtResId( STR_PATHNAME ) );
        nResId = STR_PATHSELECT;
        nButtonResId = STR_BUTTONSELECT;
    }

    SetText( SvtResId( nResId ) );

    if ( nButtonResId )
        _pImp->_pBtnFileOpen->SetText( SvtResId( nButtonResId ) );

    if ( FILEDLG_TYPE_FILEDLG != _pImp->_eDlgType )
    {
        _pImp->_pFtFileType->Hide();
        _pImp->GetFilterListControl()->Hide();
    }

    // Setting preferences of the control elements.
    _pImp->_pBtnNewFolder->SetClickHdl( LINK( this, SvtFileDialog, NewFolderHdl_Impl ) );
    _pImp->_pBtnFileOpen->SetClickHdl( LINK( this, SvtFileDialog, OpenClickHdl_Impl ) );
    _pImp->_pBtnCancel->SetClickHdl( LINK( this, SvtFileDialog, CancelHdl_Impl ) );
    _pImp->SetFilterListSelectHdl( LINK( this, SvtFileDialog, FilterSelectHdl_Impl ) );
    _pImp->_pEdFileName->SetGetFocusHdl( LINK( this, SvtFileDialog, FileNameGetFocusHdl_Impl ) );
    _pImp->_pEdFileName->SetModifyHdl( LINK( this, SvtFileDialog, FileNameModifiedHdl_Impl ) );
    _pImp->_pEdCurrentPath->SetOpenHdl ( LINK( this, SvtFileDialog, URLBoxModifiedHdl_Impl ) );
    _pImp->_pBtnConnectToServer->SetClickHdl( LINK ( this, SvtFileDialog, ConnectToServerPressed_Hdl ) );


    _pFileView->SetSelectHdl( LINK( this, SvtFileDialog, SelectHdl_Impl ) );
    _pFileView->SetDoubleClickHdl( LINK( this, SvtFileDialog, DblClickHdl_Impl ) );
    _pFileView->SetOpenDoneHdl( LINK( this, SvtFileDialog, OpenDoneHdl_Impl ) );


    // set timer for the filterbox travel
    _pImp->_aFilterTimer.SetTimeout( TRAVELFILTER_TIMEOUT );
    _pImp->_aFilterTimer.SetTimeoutHdl( LINK( this, SvtFileDialog, FilterSelectTimerHdl_Impl ) );

    if ( WB_SAVEAS & nStyle )
    {
        // different help ids if in save-as mode
        SetHelpId( HID_FILESAVE_DIALOG );

        _pImp->_pEdFileName->SetHelpId( HID_FILESAVE_FILEURL );
        _pImp->_pBtnFileOpen->SetHelpId( HID_FILESAVE_DOSAVE );
        _pImp->_pBtnNewFolder->SetHelpId( HID_FILESAVE_CREATEDIRECTORY );
        _pImp->_pBtnUp->SetHelpId( HID_FILESAVE_LEVELUP );
        _pImp->GetFilterListControl()->SetHelpId( HID_FILESAVE_FILETYPE );
        _pFileView->SetHelpId( HID_FILESAVE_FILEVIEW );

        // formerly, there was only _pLbFileVersion, which was used for 3 different
        // use cases. For reasons of maintainability, I introduced extra members (_pLbTemplates, _pLbImageTemplates)
        // for the extra use cases, and separated _pLbFileVersion
        // I did not find out in which cases the help ID is really needed HID_FILESAVE_TEMPLATE - all
        // tests I made lead to a dialog where _no_ of the three list boxes was present.
        if ( _pImp->_pLbFileVersion )
            _pImp->_pLbFileVersion->SetHelpId( HID_FILESAVE_TEMPLATE );
        if ( _pImp->_pLbTemplates )
            _pImp->_pLbTemplates->SetHelpId( HID_FILESAVE_TEMPLATE );
        if ( _pImp->_pLbImageTemplates )
            _pImp->_pLbImageTemplates->SetHelpId( HID_FILESAVE_TEMPLATE );

        if ( _pImp->_pCbPassword ) _pImp->_pCbPassword->SetHelpId( HID_FILESAVE_SAVEWITHPASSWORD );
        if ( _pImp->_pCbAutoExtension ) _pImp->_pCbAutoExtension->SetHelpId( HID_FILESAVE_AUTOEXTENSION );
        if ( _pImp->_pCbOptions ) _pImp->_pCbOptions->SetHelpId( HID_FILESAVE_CUSTOMIZEFILTER );
        if ( _pCbSelection ) _pCbSelection->SetHelpId( HID_FILESAVE_SELECTION );
    }

    /// read our settings from the configuration
    m_aConfiguration = OConfigurationTreeRoot::createWithComponentContext(
        ::comphelper::getProcessComponentContext(),
        OUString( "/org.openoffice.Office.UI/FilePicker" )
    );

    _pContainer->init(_pImp, _pFileView, _pSplitter, _pImp->_pBtnNewFolder, _pImp->_pEdFileName);
    _pContainer->Show();

    Resize();
}

IMPL_LINK_NOARG_TYPED( SvtFileDialog, NewFolderHdl_Impl, Button*, void)
{
    _pFileView->EndInplaceEditing( false );

    SmartContent aContent( _pFileView->GetViewURL( ) );
    OUString aTitle;
    aContent.getTitle( aTitle );
    ScopedVclPtrInstance< QueryFolderNameDialog > aDlg(this, aTitle, SVT_RESSTR(STR_SVT_NEW_FOLDER));
    bool bHandled = false;

    while ( !bHandled )
    {
        if ( aDlg->Execute() == RET_OK )
        {
            OUString aUrl = aContent.createFolder( aDlg->GetName( ) );
            if ( !aUrl.isEmpty( ) )
            {
                _pFileView->CreatedFolder( aUrl, aDlg->GetName() );
                bHandled = true;
            }
        }
        else
            bHandled = true;
    }
}

bool SvtFileDialog::createNewUserFilter( const OUString& _rNewFilter, bool _bAllowUserDefExt )
{
    // delete the old user filter and create a new one
    DELETEZ( _pImp->_pUserFilter );
    _pImp->_pUserFilter = new SvtFileDialogFilter_Impl( _rNewFilter, _rNewFilter );

    // remember the extension
    bool bIsAllFiles = _rNewFilter == FILEDIALOG_FILTER_ALL;
    if ( bIsAllFiles )
        EraseDefaultExt();
    else
        SetDefaultExt( _rNewFilter.copy( 2 ) );
        // TODO: this is nonsense. In the whole file there are a lot of places where we assume that a user filter
        // is always "*.<something>". But changing this would take some more time than I have now...

    // now, the default extension is set to the one of the user filter (or empty)
    // if the former is not allowed (_bAllowUserDefExt = <FALSE/>), we have to use the ext of the current filter
    // (if possible)
    bool bUseCurFilterExt = true;
    OUString sUserFilter = _pImp->_pUserFilter->GetType();
    sal_Int32 nSepPos = sUserFilter.lastIndexOf( '.' );
    if ( nSepPos != -1 )
    {
        OUString sUserExt = sUserFilter.copy( nSepPos + 1 );
        if  (   ( -1 == sUserExt.indexOf( '*' ) )
            &&  ( -1 == sUserExt.indexOf( '?' ) )
            )
            bUseCurFilterExt = false;
    }

    if ( !_bAllowUserDefExt || bUseCurFilterExt )
    {
        if ( _pImp->GetCurFilter( ) )
            SetDefaultExt( _pImp->GetCurFilter( )->GetExtension() );
        else
            EraseDefaultExt();
    }

    // outta here
    return bIsAllFiles;
}


#define FLT_NONEMPTY        0x0001
#define FLT_CHANGED         0x0002
#define FLT_USERFILTER      0x0004
#define FLT_ALLFILESFILTER  0x0008


sal_uInt16 SvtFileDialog::adjustFilter( const OUString& _rFilter )
{
    sal_uInt16 nReturn = 0;

    const bool bNonEmpty = !_rFilter.isEmpty();
    if ( bNonEmpty )
    {
        nReturn |= FLT_NONEMPTY;

        bool bFilterChanged = true;

        // search for a corresponding filter
        SvtFileDialogFilter_Impl* pFilter = FindFilter_Impl( _rFilter, false, bFilterChanged );

#ifdef AUTOSELECT_USERFILTER
        // if we found a filter which without allowing multi-extensions -> select it
        if ( pFilter )
        {
            _pImp->SelectFilterListEntry( pFilter->GetName() );
            _pImp->SetCurFilter( pFilter );
        }
#endif // AUTOSELECT_USERFILTER

        // look for multi-ext filters if necessary
        if ( !pFilter )
            pFilter = FindFilter_Impl( _rFilter, true, bFilterChanged );

        if ( bFilterChanged )
            nReturn |= FLT_CHANGED;

        if ( !pFilter )
        {
            nReturn |= FLT_USERFILTER;
            // no filter found : use it as user defined filter
#ifdef AUTOSELECT_USERFILTER
            if ( createNewUserFilter( _rFilter, sal_True ) )
#else
            if ( createNewUserFilter( _rFilter, false ) )
#endif
            {   // it's the "all files" filter
                nReturn |= FLT_ALLFILESFILTER;

#ifdef AUTOSELECT_USERFILTER
                // select the "all files" entry
                OUString sAllFilesFilter( SvtResId( STR_FILTERNAME_ALL ) );
                if ( _pImp->HasFilterListEntry( sAllFilesFilter ) )
                {
                    _pImp->SelectFilterListEntry( sAllFilesFilter );
                    _pImp->SetCurFilter( _pImp->GetSelectedFilterEntry( sAllFilesFilter ) );
                }
                else
                    _pImp->SetNoFilterListSelection( ); // there is no "all files" entry
#endif // AUTOSELECT_USERFILTER
            }
#ifdef AUTOSELECT_USERFILTER
            else
                _pImp->SetNoFilterListSelection( );
#endif // AUTOSELECT_USERFILTER
        }
    }

    return nReturn;
}


IMPL_LINK_NOARG_TYPED(SvtFileDialog, CancelHdl_Impl, Button*, void)
{
    if ( m_pCurrentAsyncAction.is() )
    {
        m_pCurrentAsyncAction->cancel();
        onAsyncOperationFinished();
    }
    else
    {
        EndDialog();
    }
}


IMPL_LINK_TYPED( SvtFileDialog, OpenClickHdl_Impl, Button*, pVoid, void )
{
    OpenHdl_Impl(pVoid);
}
IMPL_LINK_TYPED( SvtFileDialog, OpenUrlHdl_Impl, SvtURLBox*, pVoid, void )
{
    OpenHdl_Impl(pVoid);
}
void SvtFileDialog::OpenHdl_Impl(void* pVoid)
{
    if ( _pImp->_bMultiSelection && _pFileView->GetSelectionCount() > 1 )
    {
        // special open in case of multiselection
        OpenMultiSelection_Impl();
        return;
    }

    OUString aFileName;
    OUString aOldPath( _pFileView->GetViewURL() );
    if ( _pImp->_bDoubleClick || _pFileView->HasChildPathFocus() )
        // Selection done by doubleclicking in the view, get filename from the view
        aFileName = _pFileView->GetCurrentURL();

    if ( aFileName.isEmpty() )
    {
        // if an entry is selected in the view ....
        if ( _pFileView->GetSelectionCount() )
        {   // -> use this one. This will allow us to step down this folder
            aFileName = _pFileView->GetCurrentURL();
        }
    }

    if ( aFileName.isEmpty() )
    {
        if ( _pImp->_eMode == FILEDLG_MODE_OPEN && _pImp->_pEdFileName->IsTravelSelect() )
            // OpenHdl called from URLBox; travelling through the list of URLs should not cause an opening
            return;                   // MBA->PB: seems to be called never ?!

        // get the URL from the edit field ( if not empty )
        if ( !_pImp->_pEdFileName->GetText().isEmpty() )
        {
            OUString aText = _pImp->_pEdFileName->GetText();

            // did we reach the root?
            if ( !INetURLObject( aOldPath ).getSegmentCount() )
            {
                if ( ( aText.getLength() == 2 && aText == ".." ) ||
                     ( aText.getLength() == 3 && ( aText == "..\\" || aText == "../" ) ) )
                    // don't go higher than the root
                    return;
            }

#if defined( UNX )
            if ( ( 1 == aText.getLength() ) && ( '~' == aText[0] ) )
            {
                // go to the home directory
                if ( lcl_getHomeDirectory( _pFileView->GetViewURL(), aFileName ) )
                    // in case we got a home dir, reset the text of the edit
                    _pImp->_pEdFileName->SetText( OUString() );
            }
            if ( aFileName.isEmpty() )
#endif
            {
                // get url from autocomplete edit
                aFileName = _pImp->_pEdFileName->GetURL();
            }
        }
        else if ( pVoid == _pImp->_pBtnFileOpen.get() )
            // OpenHdl was called for the "Open" Button; if edit field is empty, use selected element in the view
            aFileName = _pFileView->GetCurrentURL();
    }

    // MBA->PB: ?!
    if ( aFileName.isEmpty() && pVoid == _pImp->_pEdFileName && _pImp->_pUserFilter )
    {
        DELETEZ( _pImp->_pUserFilter );
        return;
    }

    sal_Int32 nLen = aFileName.getLength();
    if ( !nLen )
    {
        // if the dialog was opened to select a folder, the last selected folder should be selected
        if( _pImp->_eDlgType == FILEDLG_TYPE_PATHDLG )
        {
            aFileName = _pImp->_pEdCurrentPath->GetText();
            nLen = aFileName.getLength();
        }
        else
            // no file selected !
            return;
    }

    // mark input as selected
    _pImp->_pEdFileName->SetSelection( Selection( 0, nLen ) );

    // if a path with wildcards is given, divide the string into path and wildcards
    OUString aFilter;
    if ( !SvtFileDialog::IsolateFilterFromPath_Impl( aFileName, aFilter ) )
        return;

    // if a filter was retrieved, there were wildcards !
    sal_uInt16 nNewFilterFlags = adjustFilter( aFilter );
    if ( nNewFilterFlags & FLT_CHANGED )
    {
        // cut off all text before wildcard in edit and select wildcard
        _pImp->_pEdFileName->SetText( aFilter );
        _pImp->_pEdFileName->SetSelection( Selection( 0, aFilter.getLength() ) );
    }

    {
        INetURLObject aFileObject( aFileName );
        if ( ( aFileObject.GetProtocol() == INetProtocol::NotValid ) && !aFileName.isEmpty() )
        {
            OUString sCompleted = SvtURLBox::ParseSmart( aFileName, _pFileView->GetViewURL() );
            if ( !sCompleted.isEmpty() )
                aFileName = sCompleted;
        }
    }

    // check if it is a folder
    bool bIsFolder = false;

    // first thing before doing anything with the content: Reset it. When the user presses "open" (or "save" or "export",
    // for that matter), s/he wants the complete handling, including all possible error messages, even if s/he
    // does the same thing for the same content twice, s/he wants both fails to be displayed.
    // Without the reset, it could be that the content cached all relevant information, and will not display any
    // error messages for the same content a second time...
    m_aContent.bindTo( OUString( ) );

    if ( !aFileName.isEmpty() )
    {
        // Make sure we have own Interaction Handler in place. We do not need
        // to intercept interactions here, but to record the fact that there
        // was an interaction.
        SmartContent::InteractionHandlerType eInterActionHandlerType
            = m_aContent.queryCurrentInteractionHandler();
        if ( ( eInterActionHandlerType == SmartContent::IHT_NONE ) ||
             ( eInterActionHandlerType == SmartContent::IHT_DEFAULT ) )
            m_aContent.enableOwnInteractionHandler(
                OFilePickerInteractionHandler::E_NOINTERCEPTION );

        bIsFolder = m_aContent.isFolder( aFileName );

        // access denied to the given resource - and interaction was already
        // used => break following operations
        OFilePickerInteractionHandler* pHandler
            = m_aContent.getOwnInteractionHandler();

        OSL_ENSURE( pHandler, "Got no Interaction Handler!!!" );

        if ( pHandler->wasAccessDenied() )
            return;

        if ( m_aContent.isInvalid() &&
             ( _pImp->_eMode == FILEDLG_MODE_OPEN ) )
        {
            if ( !pHandler->wasUsed() )
                ErrorHandler::HandleError( ERRCODE_IO_NOTEXISTS );

            return;
        }

        // restore previous Interaction Handler
        if ( eInterActionHandlerType == SmartContent::IHT_NONE )
            m_aContent.disableInteractionHandler();
        else if ( eInterActionHandlerType == SmartContent::IHT_DEFAULT )
            m_aContent.enableDefaultInteractionHandler();
     }

    if  (   !bIsFolder                                      // no existent folder
        &&  _pImp->_pCbAutoExtension                 // auto extension is enabled in general
        &&  _pImp->_pCbAutoExtension->IsChecked()    // auto extension is really to be used
        &&  !GetDefaultExt().isEmpty()               // there is a default extension
        &&  GetDefaultExt() != "*" // the default extension is not "all"
        && !(   FILEDLG_MODE_SAVE == _pImp->_eMode       // we're saving a file
            &&  _pFileView->GetSelectionCount()          // there is a selected file in the file view -> it will later on
            )                                                   //    (in SvtFileDialog::GetPathList) be taken as file to save to

        && FILEDLG_MODE_OPEN != _pImp->_eMode // #i83408# don't append extension on open
        )
    {
        // check extension and append the default extension if necessary
        appendDefaultExtension(aFileName,
                               GetDefaultExt(),
                               _pImp->GetCurFilter()->GetType());
    }

    bool bOpenFolder = ( FILEDLG_TYPE_PATHDLG == _pImp->_eDlgType ) &&
                       !_pImp->_bDoubleClick && pVoid != _pImp->_pEdFileName;
    if ( bIsFolder )
    {
        if ( bOpenFolder )
        {
            _aPath = aFileName;
        }
        else
        {
            if ( aFileName != _pFileView->GetViewURL() )
            {
                OpenURL_Impl( aFileName );
            }
            else
            {
                if ( nNewFilterFlags & FLT_CHANGED )
                    ExecuteFilter();
            }

            return;
        }
    }
    else if ( !( nNewFilterFlags & FLT_NONEMPTY ) )
    {
        // if applicable save URL
        _aPath = aFileName;
    }
    else
    {
        // if applicable filter again
        if ( nNewFilterFlags & FLT_CHANGED )
            ExecuteFilter();
        return;
    }

    INetURLObject aFileObj( aFileName );
    if ( aFileObj.HasError() )
    {
        ErrorHandler::HandleError( ERRCODE_IO_GENERAL );
        return;
    }

    switch ( _pImp->_eMode )
    {
        case FILEDLG_MODE_SAVE:
        {
            if ( ::utl::UCBContentHelper::Exists( aFileObj.GetMainURL( INetURLObject::NO_DECODE ) ) )
            {
                OUString aMsg = SVT_RESSTR( STR_SVT_ALREADYEXISTOVERWRITE );
                aMsg = aMsg.replaceFirst(
                    "$filename$",
                    aFileObj.getName(INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET)
                );
                ScopedVclPtrInstance< MessageDialog > aBox(this, aMsg, VCL_MESSAGE_QUESTION, VCL_BUTTONS_YES_NO);
                if ( aBox->Execute() != RET_YES )
                    return;
            }
            else
            {
                OUString aCurPath;
                if (osl::FileBase::getSystemPathFromFileURL(aFileName, aCurPath) == osl::FileBase::E_None)
                {
                    // if content does not exist: at least its path must exist
                    INetURLObject aPathObj = aFileObj;
                    aPathObj.removeSegment();
                    bool bFolder = m_aContent.isFolder( aPathObj.GetMainURL( INetURLObject::NO_DECODE ) );
                    if ( !bFolder )
                    {
                        ErrorHandler::HandleError( ERRCODE_IO_NOTEXISTSPATH );
                        return;
                    }
                }
            }
        }
        break;

        case FILEDLG_MODE_OPEN:
        {
            // do an existence check herein, again

            if ( INetProtocol::File == aFileObj.GetProtocol( ) )
            {
                bool bExists = m_aContent.is( aFileObj.GetMainURL( INetURLObject::NO_DECODE ) );

                if ( !bExists )
                {
                    OUString sError( SVT_RESSTR( RID_FILEOPEN_NOTEXISTENTFILE ) );

                    OUString sInvalidFile( aFileObj.GetMainURL( INetURLObject::DECODE_TO_IURI ) );
                    if ( INetProtocol::File == aFileObj.GetProtocol() )
                    {   // if it's a file URL, transform the URL into system notation
                        OUString sURL( sInvalidFile );
                        OUString sSystem;
                        osl_getSystemPathFromFileURL( sURL.pData, &sSystem.pData );
                        sInvalidFile = sSystem;
                    }
                    sError = sError.replaceFirst( "$name$", sInvalidFile );

                    ScopedVclPtrInstance< MessageDialog > aError(this, sError);
                    aError->Execute();
                    return;
                }
            }
        }
        break;

        default:
            OSL_FAIL("SvtFileDialog, OpenHdl_Impl: invalid mode!");
    }

    EndDialog( RET_OK );
}



void SvtFileDialog::EnableAutocompletion( bool _bEnable )
{
    _pImp->_pEdFileName->EnableAutocompletion( _bEnable );
}



IMPL_LINK_NOARG_TYPED( SvtFileDialog, FilterSelectHdl_Impl, ListBox&, void )
{
    OUString sSelectedFilterDisplayName;
    SvtFileDialogFilter_Impl* pSelectedFilter = _pImp->GetSelectedFilterEntry( sSelectedFilterDisplayName );
    if ( !pSelectedFilter )
    {   // there is no current selection. This happens if for instance the user selects a group separator using
        // the keyboard, and then presses enter: When the selection happens, we immediately deselect the entry,
        // so in this situation there is no current selection.
        if ( restoreCurrentFilter( _pImp ) )
            ExecuteFilter();
    }
    else
    {
        if ( pSelectedFilter->isGroupSeparator() )
        {   // group separators can't be selected
            // return to the previously selected entry
            if ( _pImp->IsFilterListTravelSelect() )
            {
                _pImp->SetNoFilterListSelection( );

                // stop the timer for executing the filter
                if ( _pImp->_aFilterTimer.IsActive() )
                    _pImp->m_bNeedDelayedFilterExecute = true;
                _pImp->_aFilterTimer.Stop();
            }
            else
            {
                if ( restoreCurrentFilter( _pImp ) )
                    ExecuteFilter();
            }
        }
        else if (   ( pSelectedFilter != _pImp->GetCurFilter() )
                ||  _pImp->_pUserFilter
                )
        {
            // Store the old filter for the auto extension handling
            OUString sLastFilterExt = _pImp->GetCurFilter()->GetExtension();
            DELETEZ( _pImp->_pUserFilter );

            // if applicable remove filter of the user
            _pImp->SetCurFilter( pSelectedFilter, sSelectedFilterDisplayName );

            // if applicable show extension
            SetDefaultExt( pSelectedFilter->GetExtension() );
            sal_Int32 nSepPos = GetDefaultExt().indexOf( FILEDIALOG_DEF_EXTSEP );

            if ( nSepPos != -1 )
                EraseDefaultExt( nSepPos );

            // update the extension of the current file if necessary
            lcl_autoUpdateFileExtension( this, sLastFilterExt );

            // if the user is traveling fast through the filterbox
            // do not filter instantly
            if ( _pImp->IsFilterListTravelSelect() )
            {
                // FilterSelectHdl_Impl should be started again in
                // TRAVELFILTER_TIMEOUT ms
                _pImp->_aFilterTimer.Start();
            }
            else
            {
                // stop previously started timer
                _pImp->_aFilterTimer.Stop();

                // filter the view again
                ExecuteFilter();
            }
        }
    }
}

IMPL_LINK_NOARG_TYPED(SvtFileDialog, FilterSelectTimerHdl_Impl, Timer*, void)
{
    // filter the view again
    ExecuteFilter();
}

IMPL_LINK_NOARG_TYPED( SvtFileDialog, FileNameGetFocusHdl_Impl, Control&, void )
{
    _pFileView->SetNoSelection();
    _pFileView->Update();
}



IMPL_LINK_NOARG( SvtFileDialog, FileNameModifiedHdl_Impl )
{
    FileNameGetFocusHdl_Impl( *_pImp->_pEdFileName );
    return 0;
}



IMPL_LINK_NOARG_TYPED( SvtFileDialog, URLBoxModifiedHdl_Impl, SvtURLBox*, void )
{
    OUString aPath = _pImp->_pEdCurrentPath->GetURL();
    OpenURL_Impl(aPath);
}



IMPL_LINK_NOARG_TYPED( SvtFileDialog, ConnectToServerPressed_Hdl, Button*, void )
{
    _pFileView->EndInplaceEditing( false );

    ScopedVclPtrInstance< PlaceEditDialog > aDlg(this);
    short aRetCode = aDlg->Execute();

    switch (aRetCode) {
        case RET_OK :
        {
            PlacePtr newPlace = aDlg->GetPlace();
            _pImp->_pPlaces->AppendPlace(newPlace);

      break;
        }
        case RET_CANCEL :
        default :
            // Do Nothing
            break;
    };
}



IMPL_LINK_NOARG_TYPED ( SvtFileDialog, AddPlacePressed_Hdl, Button*, void )
{
    // Maybe open the PlacesDialog would have been a better idea
    // there is an ux choice to make we did not make...
    INetURLObject aURLObj( _pFileView->GetViewURL() );
    PlacePtr newPlace(
        new Place( aURLObj.GetLastName(INetURLObject::DECODE_WITH_CHARSET),
                OUString(_pFileView->GetViewURL()), true));
    _pImp->_pPlaces->AppendPlace(newPlace);
}



IMPL_LINK_NOARG_TYPED ( SvtFileDialog, RemovePlacePressed_Hdl, Button*, void )
{
    _pImp->_pPlaces->RemoveSelectedPlace();
}



SvtFileDialogFilter_Impl* SvtFileDialog::FindFilter_Impl
(
    const OUString& _rFilter,
    bool _bMultiExt,/*  TRUE - regard filter with several extensions
                            FALSE - do not ...
                        */
    bool& _rFilterChanged
)

/*  [Description]

    This method looks for the specified extension in the included filters.
*/

{
    SvtFileDialogFilter_Impl* pFoundFilter = NULL;
    SvtFileDialogFilterList_Impl& rList = _pImp->m_aFilter;
    sal_uInt16 nFilter = rList.size();

    while ( nFilter-- )
    {
        SvtFileDialogFilter_Impl* pFilter = rList[ nFilter ].get();
        const OUString& rType = pFilter->GetType();
        OUString aSingleType = rType;

        if ( _bMultiExt )
        {
            sal_Int32 nIdx = 0;
            while ( !pFoundFilter && nIdx != -1 )
            {
                aSingleType = rType.getToken( 0, FILEDIALOG_DEF_EXTSEP, nIdx );
#ifdef UNX
                if ( aSingleType == _rFilter )
#else
                if ( aSingleType.equalsIgnoreAsciiCase( _rFilter ) )
#endif
                    pFoundFilter = pFilter;
            }
        }
#ifdef UNX
        else if ( rType == _rFilter )
#else
        else if ( rType.equalsIgnoreAsciiCase( _rFilter ) )
#endif
            pFoundFilter = pFilter;

        if ( pFoundFilter )
        {
            // activate filter
            _rFilterChanged = _pImp->_pUserFilter || ( _pImp->GetCurFilter() != pFilter );

            createNewUserFilter( _rFilter, false );

            break;
        }
    }
    return pFoundFilter;
}



void SvtFileDialog::ExecuteFilter()
{
    _pImp->m_bNeedDelayedFilterExecute = false;
    executeAsync( AsyncPickerAction::eExecuteFilter, OUString(), getMostCurrentFilter( _pImp ) );
}



void SvtFileDialog::OpenMultiSelection_Impl()

/*  [Description]

    OpenHandler for MultiSelection
*/

{
    sal_uLong nCount = _pFileView->GetSelectionCount();
    SvTreeListEntry* pEntry = nCount ? _pFileView->FirstSelected() : NULL;

    if ( nCount && pEntry )
        _aPath = SvtFileView::GetURL( pEntry );

    EndDialog( RET_OK );
}



void SvtFileDialog::UpdateControls( const OUString& rURL )
{
       _pImp->_pEdFileName->SetBaseURL( rURL );

    INetURLObject aObj( rURL );


    {
        OUString sText;
        DBG_ASSERT( INetProtocol::NotValid != aObj.GetProtocol(), "SvtFileDialog::UpdateControls: Invalid URL!" );

        if ( aObj.getSegmentCount() )
        {
            osl::FileBase::getSystemPathFromFileURL(rURL, sText);
            if ( !sText.isEmpty() )
            {
                // no Fsys path for server file system ( only UCB has mountpoints! )
                if ( INetProtocol::File != aObj.GetProtocol() )
                    sText = rURL.copy( INetURLObject::GetScheme( aObj.GetProtocol() ).getLength() );
            }

            if ( sText.isEmpty() && aObj.getSegmentCount() )
                sText = rURL;
        }

        // path mode ?
        if ( FILEDLG_TYPE_PATHDLG == _pImp->_eDlgType )
            // -> set new path in the edit field
            _pImp->_pEdFileName->SetText( sText );

        // in the "current path" field, truncate the trailing slash
        if ( aObj.hasFinalSlash() )
        {
            aObj.removeFinalSlash();
            OUString sURL( aObj.GetMainURL( INetURLObject::NO_DECODE ) );
            if (osl::FileBase::getSystemPathFromFileURL(sURL, sText) != osl::FileBase::E_None)
                sText = sURL;
        }

        if ( sText.isEmpty() && !rURL.isEmpty() )
            // happens, for instance, for URLs which the INetURLObject does not know to belong to a hierarchical scheme
            sText = rURL;
        _pImp->_pEdCurrentPath->SetText( sText );
    }


    _aPath = rURL;
    if ( _pFileNotifier )
        _pFileNotifier->notify( DIRECTORY_CHANGED, 0 );
}



IMPL_LINK_TYPED( SvtFileDialog, SelectHdl_Impl, SvTreeListBox*, pBox, void )
{
    SvTreeListEntry* pEntry = pBox->FirstSelected();
    DBG_ASSERT( pEntry, "SelectHandler without selected entry" );
    SvtContentEntry* pUserData = static_cast<SvtContentEntry*>(pEntry->GetUserData());

    if ( pUserData )
    {
        INetURLObject aObj( pUserData->maURL );
        if ( FILEDLG_TYPE_PATHDLG == _pImp->_eDlgType )
        {
            if ( aObj.GetProtocol() == INetProtocol::File )
            {
                if ( !pUserData->mbIsFolder )
                    aObj.removeSegment();
                OUString aName = aObj.getFSysPath( (INetURLObject::FSysStyle)(INetURLObject::FSYS_DETECT & ~INetURLObject::FSYS_VOS) );
                _pImp->_pEdFileName->SetText( aName );
                _pImp->_pEdFileName->SetSelection( Selection( 0, aName.getLength() ) );
                _aPath = pUserData->maURL;
            }
            else if ( !pUserData->mbIsFolder )
            {
                _pImp->_pEdFileName->SetText( pUserData->maURL );
                _pImp->_pEdFileName->SetSelection( Selection( 0, pUserData->maURL.getLength() ) );
                _aPath = pUserData->maURL;
            }
            else
                _pImp->_pEdFileName->SetText( OUString() );
        }
        else
        {
            if ( !pUserData->mbIsFolder )
            {
                OUString aName = SvTabListBox::GetEntryText( pEntry, 0 );
                _pImp->_pEdFileName->SetText( aName );
                _pImp->_pEdFileName->SetSelection( Selection( 0, aName.getLength() ) );
                _aPath = pUserData->maURL;
            }
        }
    }

    if ( _pImp->_bMultiSelection && _pFileView->GetSelectionCount() > 1 )
    {
        // clear the file edit for multiselection
        _pImp->_pEdFileName->SetText( OUString() );
    }

    FileSelect();
}



IMPL_LINK_NOARG_TYPED(SvtFileDialog, DblClickHdl_Impl, SvTreeListBox*, bool)
{
    _pImp->_bDoubleClick = true;
    OpenHdl_Impl( NULL );
    _pImp->_bDoubleClick = false;

    return false;
}



IMPL_LINK_NOARG(SvtFileDialog, EntrySelectHdl_Impl)
{
    FileSelect();

    return 0;
}



IMPL_LINK_TYPED( SvtFileDialog, OpenDoneHdl_Impl, SvtFileView*, pView, void )
{
    OUString sCurrentFolder( pView->GetViewURL() );
    // check if we can create new folders
    EnableControl( _pImp->_pBtnNewFolder, ContentCanMakeFolder( sCurrentFolder ) );

    // check if we can travel one level up
    bool bCanTravelUp = ContentHasParentFolder( pView->GetViewURL() );
    if ( bCanTravelUp )
    {
        // additional check: the parent folder should not be prohibited
        INetURLObject aCurrentFolder( sCurrentFolder );
        DBG_ASSERT( INetProtocol::NotValid != aCurrentFolder.GetProtocol(),
            "SvtFileDialog::OpenDoneHdl_Impl: invalid current URL!" );

        aCurrentFolder.removeSegment();
    }
    EnableControl( _pImp->_pBtnUp, bCanTravelUp );
}



IMPL_LINK_NOARG_TYPED(SvtFileDialog, AutoExtensionHdl_Impl, Button*, void)
{
    if ( _pFileNotifier )
        _pFileNotifier->notify( CTRL_STATE_CHANGED,
                                CHECKBOX_AUTOEXTENSION );

    // update the extension of the current file if necessary
    lcl_autoUpdateFileExtension( this, _pImp->GetCurFilter()->GetExtension() );
}



IMPL_LINK_TYPED( SvtFileDialog, ClickHdl_Impl, Button*, pCheckBox, void )
{
    if ( ! _pFileNotifier )
        return;

    sal_Int16 nId = -1;

    if ( pCheckBox == _pImp->_pCbOptions )
        nId = CHECKBOX_FILTEROPTIONS;
    else if ( pCheckBox == _pCbSelection )
        nId = CHECKBOX_SELECTION;
    else if ( pCheckBox == _pCbReadOnly )
        nId = CHECKBOX_READONLY;
    else if ( pCheckBox == _pImp->_pCbPassword )
        nId = CHECKBOX_PASSWORD;
    else if ( pCheckBox == _pCbLinkBox )
        nId = CHECKBOX_LINK;
    else if ( pCheckBox == _pCbPreviewBox )
        nId = CHECKBOX_PREVIEW;

    if ( nId != -1 )
        _pFileNotifier->notify( CTRL_STATE_CHANGED, nId );
}



IMPL_LINK_NOARG_TYPED(SvtFileDialog, PlayButtonHdl_Impl, Button*, void)
{
    if ( _pFileNotifier )
        _pFileNotifier->notify( CTRL_STATE_CHANGED,
                                PUSHBUTTON_PLAY );
}



bool SvtFileDialog::Notify( NotifyEvent& rNEvt )

/*  [Description]

    This method gets called to catch <BACKSPACE>.
*/

{
    MouseNotifyEvent nType = rNEvt.GetType();
    bool bRet = false;

    if ( MouseNotifyEvent::KEYINPUT == nType && rNEvt.GetKeyEvent() )
    {
        const vcl::KeyCode& rKeyCode = rNEvt.GetKeyEvent()->GetKeyCode();
        sal_uInt16 nCode = rKeyCode.GetCode();

        if ( !rKeyCode.GetModifier() &&
             KEY_BACKSPACE == nCode && !_pImp->_pEdFileName->HasChildPathFocus() )
        {
            bRet = false;

            if ( !bRet && _pImp->_pBtnUp->IsEnabled() )
            {
                PrevLevel_Impl();
                bRet = true;
            }
        }
    }
    return bRet || ModalDialog::Notify( rNEvt );
}



class SvtDefModalDialogParent_Impl
{
private:
    VclPtr<vcl::Window> _pOld;

public:
    SvtDefModalDialogParent_Impl( vcl::Window *pNew ) :
        _pOld( Application::GetDefDialogParent() )
        { Application::SetDefDialogParent( pNew ); }

    ~SvtDefModalDialogParent_Impl() { Application::SetDefDialogParent( _pOld ); }
};

namespace
{

bool implIsInvalid( const OUString & rURL )
{
    SmartContent aContent( rURL );
    aContent.enableOwnInteractionHandler( ::svt::OFilePickerInteractionHandler::E_DOESNOTEXIST );
    aContent.isFolder();    // do this _before_ asking isInvalid! Otherwise result might be wrong.
    return aContent.isInvalid();
}

}


OUString SvtFileDialog::implGetInitialURL( const OUString& _rPath, const OUString& _rFallback )
{
    // an URL parser for the fallback
    INetURLObject aURLParser;

    // set the path
    bool bWasAbsolute = false;
    aURLParser = aURLParser.smartRel2Abs( _rPath, bWasAbsolute );

    // is it a valid folder?
    m_aContent.bindTo( aURLParser.GetMainURL( INetURLObject::NO_DECODE ) );
    bool bIsFolder = m_aContent.isFolder( );    // do this _before_ asking isInvalid!
    bool bIsInvalid = m_aContent.isInvalid();

    if ( bIsInvalid && m_bHasFilename && !aURLParser.hasFinalSlash() )
    {   // check if the parent folder exists
        INetURLObject aParent( aURLParser );
        aParent.removeSegment( );
        aParent.setFinalSlash( );
        bIsInvalid = implIsInvalid( aParent.GetMainURL( INetURLObject::NO_DECODE ) );
    }

    if ( bIsInvalid )
    {
        INetURLObject aFallback( _rFallback );
        bIsInvalid = implIsInvalid( aFallback.GetMainURL( INetURLObject::NO_DECODE ) );

        if ( !bIsInvalid )
            aURLParser = aFallback;
    }

    if ( bIsInvalid )
    {
        INetURLObject aParent( aURLParser );
        while ( bIsInvalid && aParent.removeSegment() )
        {
            aParent.setFinalSlash( );
            bIsInvalid = implIsInvalid( aParent.GetMainURL( INetURLObject::NO_DECODE ) );
        }

        if ( !bIsInvalid )
            aURLParser = aParent;
    }

    if ( !bIsInvalid && bIsFolder )
    {
        aURLParser.setFinalSlash();
    }
    return aURLParser.GetMainURL( INetURLObject::NO_DECODE );
}


short SvtFileDialog::Execute()
{
    if ( !PrepareExecute() )
        return 0;

    // start the dialog
    _bIsInExecute = true;
    short nResult = ModalDialog::Execute();
    _bIsInExecute = false;

    DBG_ASSERT( !m_pCurrentAsyncAction.is(), "SvtFilePicker::Execute: still running an async action!" );
        // the dialog should not be cancellable while an async action is running - first, the action
        // needs to be cancelled

    // remember last directory
    if ( RET_OK == nResult )
    {
        INetURLObject aURL( _aPath );
        if ( aURL.GetProtocol() == INetProtocol::File )
        {
            // remember the selected directory only for file URLs not for virtual folders
            sal_Int32 nLevel = aURL.getSegmentCount();
            bool bDir = m_aContent.isFolder( aURL.GetMainURL( INetURLObject::NO_DECODE ) );
            if ( nLevel > 1 && ( FILEDLG_TYPE_FILEDLG == _pImp->_eDlgType || !bDir ) )
                aURL.removeSegment();
        }
    }

    return nResult;
}


void SvtFileDialog::StartExecuteModal( const Link<>& rEndDialogHdl )
{
    PrepareExecute();

    // start of the dialog
    ModalDialog::StartExecuteModal( rEndDialogHdl );
}


void SvtFileDialog::onAsyncOperationStarted()
{
    EnableUI( false );
    // the cancel button must be always enabled
    _pImp->_pBtnCancel->Enable( true );
    _pImp->_pBtnCancel->GrabFocus();
}


void SvtFileDialog::onAsyncOperationFinished()
{
    EnableUI( true );
    m_pCurrentAsyncAction = NULL;
    if ( !m_bInExecuteAsync )
        _pImp->_pEdFileName->GrabFocus();
        // (if m_bInExecuteAsync is true, then the operation was finished within the minimum wait time,
        // and to the user, the operation appears to be synchronous)
}


void SvtFileDialog::RemovablePlaceSelected(bool enable)
{
    _pImp->_pPlaces->SetDelEnabled( enable );
}


void SvtFileDialog::displayIOException( const OUString& _rURL, IOErrorCode _eCode )
{
    try
    {
        // create make a human-readable string from the URL
        OUString sDisplayPath;
        if (osl::FileBase::getSystemPathFromFileURL(_rURL, sDisplayPath)
            == osl::FileBase::E_None)
        {
            sDisplayPath = _rURL;
        }

        // build an own exception which tells "access denied"
        InteractiveAugmentedIOException aException;
        aException.Arguments.realloc( 2 );
        aException.Arguments[ 0 ] <<= sDisplayPath;
        aException.Arguments[ 1 ] <<= PropertyValue(
            OUString( "Uri" ),
            -1, aException.Arguments[ 0 ], PropertyState_DIRECT_VALUE
        );
            // (formerly, it was sufficient to put the URL first parameter. Nowadays,
            // the services expects the URL in a PropertyValue named "Uri" ...)
        aException.Code = _eCode;
        aException.Classification = InteractionClassification_ERROR;

        // let and interaction handler handle this exception
        ::comphelper::OInteractionRequest* pRequest = NULL;
        Reference< css::task::XInteractionRequest > xRequest = pRequest =
            new ::comphelper::OInteractionRequest( makeAny( aException ) );
        pRequest->addContinuation( new ::comphelper::OInteractionAbort( ) );

        Reference< XInteractionHandler2 > xHandler(
            InteractionHandler::createWithParent( ::comphelper::getProcessComponentContext(), 0 ) );
        xHandler->handle( xRequest );
    }
    catch( const Exception& )
    {
        OSL_FAIL( "iodlg::displayIOException: caught an exception!" );
    }
}


void SvtFileDialog::EnableUI( bool _bEnable )
{
    Enable( _bEnable );

    if ( _bEnable )
    {
        for ( auto aLoop = m_aDisabledControls.begin();
              aLoop != m_aDisabledControls.end();
              ++aLoop
            )
        {
            (*aLoop)->Enable( false );
        }
    }
}


void SvtFileDialog::EnableControl( Control* _pControl, bool _bEnable )
{
    if ( !_pControl )
    {
        SAL_WARN( "fpicker.office", "SvtFileDialog::EnableControl: invalid control!" );
        return;
    }

    _pControl->Enable( _bEnable );

    if ( _bEnable )
    {
        auto aPos = m_aDisabledControls.find( _pControl );
        if ( m_aDisabledControls.end() != aPos )
            m_aDisabledControls.erase( aPos );
    }
    else
        m_aDisabledControls.insert( _pControl );
}



short SvtFileDialog::PrepareExecute()
{
    OUString aEnvValue;
    if ( getEnvironmentValue( "WorkDirMustContainRemovableMedia", aEnvValue ) && aEnvValue == "1" )
    {
        try
        {
            INetURLObject aStdDir( GetStandardDir() );
            ::ucbhelper::Content aCnt( OUString( aStdDir.GetMainURL(
                                                    INetURLObject::NO_DECODE ) ),
                                 Reference< XCommandEnvironment >(),
                                 comphelper::getProcessComponentContext() );
            Sequence< OUString > aProps(2);
            aProps[0] = "IsVolume";
            aProps[1] = "IsRemoveable";

            Reference< XResultSet > xResultSet
                = aCnt.createCursor( aProps, ::ucbhelper::INCLUDE_FOLDERS_ONLY );
            if ( xResultSet.is() )
            {
                Reference< XRow > xRow( xResultSet, UNO_QUERY );

                bool bEmpty = true;
                if ( !xResultSet->next() )
                {
                    // folder is empty
                    bEmpty = true;
                }
                else
                {
                                bEmpty = false;
                }

                if ( bEmpty )
                {
                    ScopedVclPtrInstance< MessageDialog > aBox(this, SVT_RESSTR(STR_SVT_NOREMOVABLEDEVICE));
                    aBox->Execute();
                    return 0;
                }
            }
        }
        catch ( ContentCreationException const & )
        {
        }
        catch ( CommandAbortedException const & )
        {
        }
    }

    if ( ( _pImp->_nStyle & WB_SAVEAS ) && m_bHasFilename )
        // when doing a save-as, we do not want the handler to handle "this file does not exist" messages
        // - finally we're going to save that file, aren't we?
        m_aContent.enableOwnInteractionHandler(::svt::OFilePickerInteractionHandler::E_DOESNOTEXIST);
    else
        m_aContent.enableDefaultInteractionHandler();

    // possibly just a filename without a path
    OUString aFileNameOnly;
    if( !_aPath.isEmpty() && (_pImp->_eMode == FILEDLG_MODE_SAVE)
                     && (_aPath.indexOf(':') == -1)
                     && (_aPath.indexOf('\\') == -1)
                     && (_aPath.indexOf('/') == -1))
    {
        aFileNameOnly = _aPath;
        _aPath.clear();
    }

    // no starting path specified?
    if ( _aPath.isEmpty() )
    {
        // then use the standard directory
        _aPath = lcl_ensureFinalSlash( _pImp->GetStandardDir() );

        // attach given filename to path
        if ( !aFileNameOnly.isEmpty() )
            _aPath += aFileNameOnly;
    }


    _aPath = implGetInitialURL( _aPath, GetStandardDir() );

    if ( _pImp->_nStyle & WB_SAVEAS && !m_bHasFilename )
        // when doing a save-as, we do not want the handler to handle "this file does not exist" messages
        // - finally we're going to save that file, aren't we?
        m_aContent.enableOwnInteractionHandler(::svt::OFilePickerInteractionHandler::E_DOESNOTEXIST);

    // if applicable show filter
    _pImp->InitFilterList();

    // set up initial filter
    sal_uInt16 nFilterCount = GetFilterCount();
    OUString aAll = SvtResId( STR_FILTERNAME_ALL ).toString();
    bool bHasAll = _pImp->HasFilterListEntry( aAll );
    if ( _pImp->GetCurFilter() || nFilterCount == 1 || ( nFilterCount == 2 && bHasAll ) )
    {
        // if applicable set the only filter or the only filter that
        // does not refer to all files, as the current one
        if ( !_pImp->GetCurFilter() )
        {
            sal_uInt16 nPos = 0;
            if ( 2 == nFilterCount && bHasAll )
            {
                nPos = nFilterCount;
                while ( nPos-- )
                {
                    if ( aAll != GetFilterName( nPos ) )
                        break;
                }
            }
            SvtFileDialogFilter_Impl* pNewCurFilter = _pImp->m_aFilter[ nPos ].get();
            DBG_ASSERT( pNewCurFilter, "SvtFileDialog::Execute: invalid filter pos!" );
            _pImp->SetCurFilter( pNewCurFilter, pNewCurFilter->GetName() );
        }

        // adjust view
        _pImp->SelectFilterListEntry( _pImp->GetCurFilter()->GetName() );
        SetDefaultExt( _pImp->GetCurFilter()->GetExtension() );
        sal_Int32 nSepPos = GetDefaultExt().indexOf( FILEDIALOG_DEF_EXTSEP );
        if ( nSepPos != -1 )
            EraseDefaultExt( nSepPos );
    }
    else
    {
        // if applicable set respectively create filter for all files
        if ( !bHasAll )
        {
            SvtFileDialogFilter_Impl* pAllFilter = implAddFilter( aAll, OUString(FILEDIALOG_FILTER_ALL) );
            _pImp->InsertFilterListEntry( pAllFilter );
            _pImp->SetCurFilter( pAllFilter, aAll );
        }
        _pImp->SelectFilterListEntry( aAll );
    }

    _pImp->_pDefaultFilter = _pImp->GetCurFilter();

    // if applicable isolate filter
    OUString aFilter;

    if ( !IsolateFilterFromPath_Impl( _aPath, aFilter ) )
        return 0;

    sal_uInt16 nNewFilterFlags = adjustFilter( aFilter );
    if ( nNewFilterFlags & ( FLT_NONEMPTY | FLT_USERFILTER ) )
    {
        _pImp->_pEdFileName->SetText( aFilter );
    }

    // create and show instance for set path
    INetURLObject aFolderURL( _aPath );
    OUString aFileName( aFolderURL.getName( INetURLObject::LAST_SEGMENT, false ) );
    sal_Int32 nFileNameLen = aFileName.getLength();
    bool bFileToSelect = nFileNameLen != 0;
    if ( bFileToSelect && aFileName[ nFileNameLen - 1 ] != '/' )
    {
         OUString aDecodedName = aFolderURL.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );
        _pImp->_pEdFileName->SetText( aDecodedName );
        aFolderURL.removeSegment();
    }

    INetURLObject aObj = aFolderURL;
    if ( aObj.GetProtocol() == INetProtocol::File )
    {
        // set folder as current directory
        aObj.setFinalSlash();
    }

    UpdateControls( aObj.GetMainURL( INetURLObject::NO_DECODE ) );

    // Somebody might want to enable some controls according to the current filter
    FilterSelect();

    OpenURL_Impl( aObj.GetMainURL( INetURLObject::NO_DECODE ) );

    _pFileView->Show();
    _pSplitter->Show();
    SvtDefModalDialogParent_Impl aDefParent( this );

    // if applicable read and set size from ini
    InitSize();

    return 1;
}


void SvtFileDialog::executeAsync( ::svt::AsyncPickerAction::Action _eAction,
                                    const OUString& _rURL, const OUString& _rFilter )
{
    DBG_ASSERT( !m_pCurrentAsyncAction.is(), "SvtFileDialog::executeAsync: previous async action not yet finished!" );

    m_pCurrentAsyncAction = new AsyncPickerAction( this, _pFileView, _eAction );

    bool bReallyAsync = true;
    m_aConfiguration.getNodeValue( OUString( "FillAsynchronously" ) ) >>= bReallyAsync;

    sal_Int32 nMinTimeout = 0;
    m_aConfiguration.getNodeValue( OUString( "Timeout/Min" ) ) >>= nMinTimeout;
    sal_Int32 nMaxTimeout = 0;
    m_aConfiguration.getNodeValue( OUString( "Timeout/Max" ) ) >>= nMaxTimeout;

    m_bInExecuteAsync = true;
    m_pCurrentAsyncAction->execute( _rURL, _rFilter, bReallyAsync ? nMinTimeout : -1, nMaxTimeout, GetBlackList() );
    m_bInExecuteAsync = false;
}



void SvtFileDialog::FileSelect()
{
    if ( _pFileNotifier )
        _pFileNotifier->notify( FILE_SELECTION_CHANGED, 0 );
}



void SvtFileDialog::FilterSelect()
{
    if ( _pFileNotifier )
        _pFileNotifier->notify( CTRL_STATE_CHANGED,
                                LISTBOX_FILTER );
}



void SvtFileDialog::SetStandardDir( const OUString& rStdDir )

/*  [Description]

   This method sets the path for the default button.
*/

{
    INetURLObject aObj( rStdDir );
    DBG_ASSERT( aObj.GetProtocol() != INetProtocol::NotValid, "Invalid protocol!" );
    aObj.setFinalSlash();
    _pImp->SetStandardDir( aObj.GetMainURL( INetURLObject::NO_DECODE ) );
}

void SvtFileDialog::SetBlackList( const css::uno::Sequence< OUString >& rBlackList )
{
    _pImp->SetBlackList( rBlackList );
}



const css::uno::Sequence< OUString >& SvtFileDialog::GetBlackList() const
{
    return _pImp->GetBlackList();
}


const OUString& SvtFileDialog::GetStandardDir() const

/*  [Description]

    This method returns the standard path.
*/

{
    return _pImp->GetStandardDir();
}



void SvtFileDialog::PrevLevel_Impl()
{
    _pFileView->EndInplaceEditing( false );

    OUString sDummy;
    executeAsync( AsyncPickerAction::ePrevLevel, sDummy, sDummy );
}



void SvtFileDialog::OpenURL_Impl( const OUString& _rURL )
{
    _pFileView->EndInplaceEditing( false );

    executeAsync( AsyncPickerAction::eOpenURL, _rURL, getMostCurrentFilter( _pImp ) );
}


SvtFileDialogFilter_Impl* SvtFileDialog::implAddFilter( const OUString& _rFilter, const OUString& _rType )
{
    SvtFileDialogFilter_Impl* pNewFilter = new SvtFileDialogFilter_Impl( _rFilter, _rType );
    _pImp->m_aFilter.push_front( std::unique_ptr<SvtFileDialogFilter_Impl>( pNewFilter ) );

    if ( !_pImp->GetCurFilter() )
        _pImp->SetCurFilter( pNewFilter, _rFilter );

    return pNewFilter;
}



void SvtFileDialog::AddFilter( const OUString& _rFilter, const OUString& _rType )
{
    DBG_ASSERT( !IsInExecute(), "SvtFileDialog::AddFilter: currently executing!" );
    implAddFilter ( _rFilter, _rType );
}


void SvtFileDialog::AddFilterGroup( const OUString& _rFilter, const Sequence< StringPair >& _rFilters )
{
    DBG_ASSERT( !IsInExecute(), "SvtFileDialog::AddFilter: currently executing!" );

    implAddFilter( _rFilter, OUString() );
    const StringPair* pSubFilters       =               _rFilters.getConstArray();
    const StringPair* pSubFiltersEnd    = pSubFilters + _rFilters.getLength();
    for ( ; pSubFilters != pSubFiltersEnd; ++pSubFilters )
        implAddFilter( pSubFilters->First, pSubFilters->Second );
}


void SvtFileDialog::SetCurFilter( const OUString& rFilter )
{
    DBG_ASSERT( !IsInExecute(), "SvtFileDialog::SetCurFilter: currently executing!" );

    // look for corresponding filter
    sal_uInt16 nPos = _pImp->m_aFilter.size();

    while ( nPos-- )
    {
        SvtFileDialogFilter_Impl* pFilter = _pImp->m_aFilter[ nPos ].get();
        if ( pFilter->GetName() == rFilter )
        {
            _pImp->SetCurFilter( pFilter, rFilter );
            break;
        }
    }
}



OUString SvtFileDialog::GetCurFilter() const
{
    OUString aFilter;

    const SvtFileDialogFilter_Impl* pCurrentFilter = _pImp->GetCurFilter();
    if ( pCurrentFilter )
        aFilter = pCurrentFilter->GetName();

    return aFilter;
}

OUString SvtFileDialog::getCurFilter( ) const
{
    return GetCurFilter();
}



sal_uInt16 SvtFileDialog::GetFilterCount() const
{
    return _pImp->m_aFilter.size();
}



const OUString& SvtFileDialog::GetFilterName( sal_uInt16 nPos ) const
{
    DBG_ASSERT( nPos < GetFilterCount(), "invalid index" );
    return _pImp->m_aFilter[ nPos ]->GetName();
}



void SvtFileDialog::InitSize()
{
    if ( _pImp->_aIniKey.isEmpty() )
        return;

    // initialize from config
    SvtViewOptions aDlgOpt( E_DIALOG, _pImp->_aIniKey );

    if ( aDlgOpt.Exists() )
    {
        SetWindowState(OUStringToOString(aDlgOpt.GetWindowState(), osl_getThreadTextEncoding()));

        Any aUserData = aDlgOpt.GetUserItem( OUString( "UserData" ));
        OUString sCfgStr;
        if ( aUserData >>= sCfgStr )
            _pFileView->SetConfigString( sCfgStr );
    }
}



std::vector<OUString> SvtFileDialog::GetPathList() const
{
    std::vector<OUString> aList;
    sal_uLong           nCount = _pFileView->GetSelectionCount();
    SvTreeListEntry*    pEntry = nCount ? _pFileView->FirstSelected() : NULL;

    if ( ! pEntry )
    {
        if ( !_pImp->_pEdFileName->GetText().isEmpty() && _bIsInExecute )
            aList.push_back(_pImp->_pEdFileName->GetURL());
        else
            aList.push_back(_aPath);
    }
    else
    {
        while ( pEntry )
        {
            aList.push_back(SvtFileView::GetURL(pEntry));
            pEntry = _pFileView->NextSelected( pEntry );
        }
    }

    return aList;
}


bool SvtFileDialog::IsolateFilterFromPath_Impl( OUString& rPath, OUString& rFilter )
{
    OUString aReversePath = comphelper::string::reverseString(rPath);
    sal_Int32 nQuestionMarkPos = rPath.indexOf( '?' );
    sal_Int32 nWildCardPos = rPath.indexOf( FILEDIALOG_DEF_WILDCARD );

    if ( nQuestionMarkPos != -1 )
    {
        // use question mark as wildcard only for files
        INetProtocol eProt = INetURLObject::CompareProtocolScheme( rPath );

        if ( INetProtocol::NotValid != eProt && INetProtocol::File != eProt )
            nQuestionMarkPos = -1;

        nWildCardPos = std::min( nWildCardPos, nQuestionMarkPos );
    }

    rFilter.clear();

    if ( nWildCardPos != -1 )
    {
        sal_Int32 nPathTokenPos = aReversePath.indexOf( '/' );

        if ( nPathTokenPos == -1 )
        {
            OUString aDelim(
#if defined(WNT)
                    '\\'
#else
                    '/'
#endif
            );

            nPathTokenPos = aReversePath.indexOf( aDelim );
#if !defined( UNX )
            if ( nPathTokenPos == -1 )
            {
                nPathTokenPos = aReversePath.indexOf( ':' );
            }
#endif
        }

        // check syntax
        if ( nPathTokenPos != -1 )
        {
            if ( nPathTokenPos < (rPath.getLength() - nWildCardPos - 1) )
            {
                ErrorHandler::HandleError( ERRCODE_SFX_INVALIDSYNTAX );
                return false;
            }

            // cut off filter
            rFilter = aReversePath;
            rFilter = rFilter.copy( 0, nPathTokenPos );
            rFilter = comphelper::string::reverseString(rFilter);

            // determine folder
            rPath = aReversePath;
            rPath = rPath.copy( nPathTokenPos );
            rPath = comphelper::string::reverseString(rPath);
        }
        else
        {
            rFilter = rPath;
            rPath.clear();
        }
    }

    return true;
}


void SvtFileDialog::implUpdateImages( )
{
    m_aImages = ImageList( SvtResId( RID_FILEPICKER_IMAGES ) );

    // set the appropriate images on the buttons
    if ( _pImp->_pBtnUp )
        _pImp->_pBtnUp->SetModeImage( GetButtonImage( IMG_FILEDLG_BTN_UP ) );

    if ( _pImp->_pBtnNewFolder )
        _pImp->_pBtnNewFolder->SetModeImage( GetButtonImage( IMG_FILEDLG_CREATEFOLDER ) );
}


void SvtFileDialog::DataChanged( const DataChangedEvent& _rDCEvt )
{
    if ( DataChangedEventType::SETTINGS == _rDCEvt.GetType() )
        implUpdateImages( );

    ModalDialog::DataChanged( _rDCEvt );
}


void SvtFileDialog::Resize()
{
    Dialog::Resize();

    if ( IsRollUp() )
        return;

    if ( _pFileNotifier )
        _pFileNotifier->notify( DIALOG_SIZE_CHANGED, 0 );
}

Control* SvtFileDialog::getControl( sal_Int16 _nControlId, bool _bLabelControl ) const
{
    Control* pReturn = NULL;

    switch ( _nControlId )
    {
        case CONTROL_FILEVIEW:
            pReturn = _bLabelControl ? NULL : static_cast< Control* >( _pFileView );
            break;

        case EDIT_FILEURL:
            pReturn =   _bLabelControl
                    ?   static_cast< Control* >( _pImp->_pFtFileName )
                    :   static_cast< Control* >( _pImp->_pEdFileName );
            break;

        case EDIT_FILEURL_LABEL:
            pReturn = static_cast< Control* >( _pImp->_pFtFileName );
            break;

        case CHECKBOX_AUTOEXTENSION:
            pReturn = _pImp->_pCbAutoExtension;
            break;

        case CHECKBOX_PASSWORD:
            pReturn = _pImp->_pCbPassword;
            break;

        case CHECKBOX_FILTEROPTIONS:
            pReturn = _pImp->_pCbOptions;
            break;

        case CHECKBOX_READONLY:
            pReturn = _pCbReadOnly;
            break;

        case CHECKBOX_LINK:
            pReturn = _pCbLinkBox;
            break;

        case CHECKBOX_PREVIEW:
            pReturn = _pCbPreviewBox;
            break;

        case CHECKBOX_SELECTION:
            pReturn = _pCbSelection;
            break;

        case LISTBOX_FILTER:
            pReturn = _bLabelControl ? _pImp->_pFtFileType : _pImp->GetFilterListControl();
            break;

        case LISTBOX_FILTER_LABEL:
            pReturn = _pImp->_pFtFileType;
            break;

        case FIXEDTEXT_CURRENTFOLDER:
            pReturn = _pImp->_pEdCurrentPath;
            break;

        case LISTBOX_VERSION:
            pReturn =   _bLabelControl
                    ?   static_cast< Control* >( _pImp->_pFtFileVersion )
                    :   static_cast< Control* >( _pImp->_pLbFileVersion );
            break;

        case LISTBOX_TEMPLATE:
            pReturn =   _bLabelControl
                    ?   static_cast< Control* >( _pImp->_pFtTemplates )
                    :   static_cast< Control* >( _pImp->_pLbTemplates );
            break;

        case LISTBOX_IMAGE_TEMPLATE:
            pReturn =   _bLabelControl
                    ?   static_cast< Control* >( _pImp->_pFtImageTemplates )
                    :   static_cast< Control* >( _pImp->_pLbImageTemplates );
            break;

        case LISTBOX_VERSION_LABEL:
            pReturn = _pImp->_pFtFileVersion;
            break;

        case LISTBOX_TEMPLATE_LABEL:
            pReturn = _pImp->_pFtTemplates;
            break;

        case LISTBOX_IMAGE_TEMPLATE_LABEL:
            pReturn = _pImp->_pFtImageTemplates;
            break;

        case PUSHBUTTON_OK:
            pReturn = _pImp->_pBtnFileOpen;
            break;

        case PUSHBUTTON_CANCEL:
            pReturn = _pImp->_pBtnCancel;
            break;

        case PUSHBUTTON_PLAY:
            pReturn = _pPbPlay;
            break;

        case PUSHBUTTON_HELP:
            pReturn = _pImp->_pBtnHelp;
            break;

        case TOOLBOXBUTOON_LEVEL_UP:
            pReturn = _pImp->_pBtnUp;
            break;

        case TOOLBOXBUTOON_NEW_FOLDER:
            pReturn = _pImp->_pBtnNewFolder;
            break;

        case LISTBOX_FILTER_SELECTOR:
            // only exists on SalGtkFilePicker
            break;

        default:
            SAL_WARN( "fpicker.office", "SvtFileDialog::getControl: invalid id!" );
    }
    return pReturn;
}


void SvtFileDialog::enableControl( sal_Int16 _nControlId, bool _bEnable )
{
    Control* pControl = getControl( _nControlId );
    if ( pControl )
        EnableControl( pControl, _bEnable );
    Control* pLabel = getControl( _nControlId, true );
    if ( pLabel )
        EnableControl( pLabel, _bEnable );
}


void SvtFileDialog::AddControls_Impl( )
{
    // create the "insert as link" checkbox, if needed
    if ( _nExtraBits & SFX_EXTRA_INSERTASLINK )
    {
        _pCbLinkBox ->SetText( SvtResId( STR_SVT_FILEPICKER_INSERT_AS_LINK ) );
        _pCbLinkBox ->SetHelpId( HID_FILEDLG_LINK_CB );
        _pCbLinkBox->SetClickHdl( LINK( this, SvtFileDialog, ClickHdl_Impl ) );
        _pCbLinkBox->Show();
    }

    // create the "show preview" checkbox ( and the preview window, too ), if needed
    if ( _nExtraBits & SFX_EXTRA_SHOWPREVIEW  )
    {
        _pImp->_aIniKey = "ImportGraphicDialog";
        // because the "<All Formats> (*.bmp,*...)" entry is to wide,
        // we need to disable the auto width feature of the filter box
        _pImp->DisableFilterBoxAutoWidth();

        // "preview"
        _pCbPreviewBox->SetText( SvtResId( STR_SVT_FILEPICKER_SHOW_PREVIEW ) );
        _pCbPreviewBox->SetHelpId( HID_FILEDLG_PREVIEW_CB );
        _pCbPreviewBox->SetClickHdl( LINK( this, SvtFileDialog, ClickHdl_Impl ) );
        _pCbPreviewBox->Show();

        // generate preview window just here
        _pPrevWin->SetOutputSizePixel(Size(200, 300));
        _pPrevWin->Show();

        _pPrevBmp = VclPtr<FixedBitmap>::Create( _pPrevWin, WinBits( WB_BORDER ) );
        _pPrevBmp->SetBackground( Wallpaper( Color( COL_WHITE ) ) );
        _pPrevBmp->SetSizePixel(_pPrevWin->GetSizePixel());
        _pPrevBmp->Show();
        _pPrevBmp->SetAccessibleName(SVT_RESSTR(STR_PREVIEW));
    }

    if ( _nExtraBits & SFX_EXTRA_AUTOEXTENSION )
    {
        _pImp->_pCbAutoExtension->SetText( SvtResId( STR_SVT_FILEPICKER_AUTO_EXTENSION ) );
        _pImp->_pCbAutoExtension->Check();
        _pImp->_pCbAutoExtension->SetClickHdl( LINK( this, SvtFileDialog, AutoExtensionHdl_Impl ) );
        _pImp->_pCbAutoExtension->Show();
    }

    if ( _nExtraBits & SFX_EXTRA_FILTEROPTIONS )
    {
        _pImp->_pCbOptions->SetText( SvtResId( STR_SVT_FILEPICKER_FILTER_OPTIONS ) );
        _pImp->_pCbOptions->SetClickHdl( LINK( this, SvtFileDialog, ClickHdl_Impl ) );
        _pImp->_pCbOptions->Show();
    }

    if ( _nExtraBits & SFX_EXTRA_SELECTION )
    {
        _pCbSelection->SetText( SvtResId( STR_SVT_FILEPICKER_SELECTION ) );
        _pCbSelection->SetClickHdl( LINK( this, SvtFileDialog, ClickHdl_Impl ) );
        _pCbSelection->Show();
    }

    if ( _nExtraBits & SFX_EXTRA_PLAYBUTTON )
    {
        _pPbPlay->SetText( SvtResId( STR_SVT_FILEPICKER_PLAY ) );
        _pPbPlay->SetHelpId( HID_FILESAVE_DOPLAY );
        _pPbPlay->SetClickHdl( LINK( this, SvtFileDialog, PlayButtonHdl_Impl ) );
        _pPbPlay->Show();
    }

    if ( _nExtraBits & SFX_EXTRA_SHOWVERSIONS )
    {
        _pImp->_pFtFileVersion->SetText( SvtResId( STR_SVT_FILEPICKER_VERSION ) );
        _pImp->_pFtFileVersion->Show();

        _pImp->_pLbFileVersion->SetHelpId( HID_FILEOPEN_VERSION );
        _pImp->_pLbFileVersion->Show();
    }
    else if ( _nExtraBits & SFX_EXTRA_TEMPLATES )
    {
        _pImp->_pFtTemplates->SetText( SvtResId( STR_SVT_FILEPICKER_TEMPLATES ) );
        _pImp->_pFtTemplates->Show();

        _pImp->_pLbTemplates->SetHelpId( HID_FILEOPEN_VERSION );
        _pImp->_pLbTemplates->Show();
            // This is strange. During the re-factoring during 96930, I discovered that this help id
            // is set in the "Templates mode". This was hidden in the previous implementation.
            // Shouldn't this be a more meaningfull help id.
    }
    else if ( _nExtraBits & SFX_EXTRA_IMAGE_TEMPLATE )
    {
        _pImp->_pFtImageTemplates->SetText( SvtResId( STR_SVT_FILEPICKER_IMAGE_TEMPLATE ) );
        _pImp->_pFtImageTemplates->Show();

        _pImp->_pLbImageTemplates->SetHelpId( HID_FILEOPEN_IMAGE_TEMPLATE );
        _pImp->_pLbImageTemplates->Show();
    }

    _pImp->_pPlaces = VclPtr<PlacesListBox>::Create(_pContainer, this, SVT_RESSTR(STR_PLACES_TITLE), WB_BORDER);
    _pImp->_pPlaces->SetHelpId("SVT_HID_FILESAVE_PLACES_LISTBOX");
    Size aSize(LogicToPixel(Size(50, 85), MAP_APPFONT));
    _pImp->_pPlaces->set_height_request(aSize.Height());
    _pImp->_pPlaces->set_width_request(aSize.Width());
    _pImp->_pPlaces->SetSizePixel(aSize);
    _pImp->_pPlaces->Show();

    sal_Int32 nPosX = _pImp->_pPlaces->GetSizePixel().Width();
    _pSplitter->SetPosPixel(Point(nPosX, 0));
    nPosX += _pSplitter->GetSizePixel().Width();
    _pFileView->SetPosPixel(Point(nPosX, 0));

    _pImp->_pPlaces->SetAddHdl( LINK ( this, SvtFileDialog, AddPlacePressed_Hdl ) );
    _pImp->_pPlaces->SetDelHdl( LINK ( this, SvtFileDialog, RemovePlacePressed_Hdl ) );

    initDefaultPlaces();
}


sal_Int32 SvtFileDialog::getTargetColorDepth()
{
    if ( _pPrevBmp )
        return _pPrevBmp->GetBitCount();
    else
        return 0;
}


sal_Int32 SvtFileDialog::getAvailableWidth()
{
    if ( _pPrevBmp )
        return _pPrevBmp->GetOutputSizePixel().Width();
    else
        return 0;
}


sal_Int32 SvtFileDialog::getAvailableHeight()
{
    if ( _pPrevBmp )
        return _pPrevBmp->GetOutputSizePixel().Height();
    else
        return 0;
}


void SvtFileDialog::setImage( sal_Int16 /*aImageFormat*/, const Any& rImage )
{
    if ( ! _pPrevBmp || ! _pPrevBmp->IsVisible() )
        return;

    Sequence < sal_Int8 > aBmpSequence;

    if ( rImage >>= aBmpSequence )
    {
        Bitmap          aBmp;
        SvMemoryStream  aData( aBmpSequence.getArray(),
                               aBmpSequence.getLength(),
                               StreamMode::READ );
        ReadDIB(aBmp, aData, true);

        _pPrevBmp->SetBitmap( aBmp );
    }
    else
    {
        Bitmap aEmpty;
        _pPrevBmp->SetBitmap( aEmpty );
    }
}


OUString SvtFileDialog::getCurrentFileText( ) const
{
    OUString sReturn;
    if ( _pImp && _pImp->_pEdFileName )
        sReturn = _pImp->_pEdFileName->GetText();
    return sReturn;
}


void SvtFileDialog::setCurrentFileText( const OUString& _rText, bool _bSelectAll )
{
    if ( _pImp && _pImp->_pEdFileName )
    {
        _pImp->_pEdFileName->SetText( _rText );
        if ( _bSelectAll )
            _pImp->_pEdFileName->SetSelection( Selection( 0, _rText.getLength() ) );
    }
}


bool SvtFileDialog::isAutoExtensionEnabled()
{
    return _pImp->_pCbAutoExtension && _pImp->_pCbAutoExtension->IsChecked();
}


bool SvtFileDialog::getShowState()
{
    if ( _pPrevBmp )
        return _pPrevBmp->IsVisible();
    else
        return false;
}

bool SvtFileDialog::ContentHasParentFolder( const OUString& rURL )
{
    m_aContent.bindTo( rURL );

    if ( m_aContent.isInvalid() )
        return false;

    return m_aContent.hasParentFolder( ) && m_aContent.isValid();
}

bool SvtFileDialog::ContentCanMakeFolder( const OUString& rURL )
{
    m_aContent.bindTo( rURL );

    if ( m_aContent.isInvalid() )
        return false;

    return m_aContent.canCreateFolder( ) && m_aContent.isValid();
}

bool SvtFileDialog::ContentGetTitle( const OUString& rURL, OUString& rTitle )
{
    m_aContent.bindTo( rURL );

    if ( m_aContent.isInvalid() )
        return false;

    OUString sTitle;
    m_aContent.getTitle( sTitle );
    rTitle = sTitle;

    return m_aContent.isValid();
}

void SvtFileDialog::appendDefaultExtension(OUString& _rFileName,
                                           const OUString& _rFilterDefaultExtension,
                                           const OUString& _rFilterExtensions)
{
    const OUString aType(_rFilterExtensions.toAsciiLowerCase());

    if ( aType != FILEDIALOG_FILTER_ALL )
    {
        const OUString aTemp(_rFileName.toAsciiLowerCase());
        sal_Int32 nPos = 0;

        do
        {
            if (nPos+1<aType.getLength() && aType[nPos]=='*') // take care of a leading *
                ++nPos;
            const OUString aExt(aType.getToken( 0, FILEDIALOG_DEF_EXTSEP, nPos ));
            if (aExt.isEmpty())
                continue;
            if (aTemp.endsWith(aExt))
                return;
        }
        while (nPos>=0);

        _rFileName += "." + _rFilterDefaultExtension;
    }
}

void SvtFileDialog::initDefaultPlaces( )
{
    PlacePtr pRootPlace( new Place( SVT_RESSTR(STR_DEFAULT_DIRECTORY), GetStandardDir() ) );
    _pImp->_pPlaces->AppendPlace( pRootPlace );

    // Load from user settings
    Sequence< OUString > placesUrlsList(officecfg::Office::Common::Misc::FilePickerPlacesUrls::get(m_context));
    Sequence< OUString > placesNamesList(officecfg::Office::Common::Misc::FilePickerPlacesNames::get(m_context));

    for(sal_Int32 nPlace = 0; nPlace < placesUrlsList.getLength() && nPlace < placesNamesList.getLength(); ++nPlace)
    {
        PlacePtr pPlace(new Place(placesNamesList[nPlace], placesUrlsList[nPlace], true));
        _pImp->_pPlaces->AppendPlace(pPlace);
    }

    // Reset the placesList "updated" state
    _pImp->_pPlaces->IsUpdated();
}

IMPL_LINK_NOARG_TYPED( SvtFileDialog, Split_Hdl, Splitter*, void )
{
    sal_Int32 nSplitPos = _pSplitter->GetSplitPosPixel();

    // Resize the places list
    sal_Int32 nPlaceX = _pImp->_pPlaces->GetPosPixel( ).X();
    Size placeSize = _pImp->_pPlaces->GetSizePixel( );
    placeSize.Width() = nSplitPos - nPlaceX;
    _pImp->_pPlaces->SetSizePixel( placeSize );

    // Change Pos and size of the fileview
    Point fileViewPos = _pFileView->GetPosPixel();
    sal_Int32 nOldX = fileViewPos.X();
    sal_Int32 nNewX = nSplitPos + _pSplitter->GetSizePixel().Width();
    fileViewPos.X() = nNewX;
    Size fileViewSize = _pFileView->GetSizePixel();
    fileViewSize.Width() -= ( nNewX - nOldX );
    _pFileView->SetPosSizePixel( fileViewPos, fileViewSize );

    _pSplitter->SetPosPixel( Point( placeSize.Width(), _pSplitter->GetPosPixel().Y() ) );
}

QueryFolderNameDialog::QueryFolderNameDialog(vcl::Window* _pParent,
    const OUString& rTitle, const OUString& rDefaultText, OUString* pGroupName)
    : ModalDialog(_pParent, "FolderNameDialog", "fps/ui/foldernamedialog.ui")
{
    get(m_pNameEdit, "entry");
    get(m_pNameLine, "frame");
    get(m_pOKBtn, "ok");

    SetText( rTitle );
    m_pNameEdit->SetText( rDefaultText );
    m_pNameEdit->SetSelection( Selection( 0, rDefaultText.getLength() ) );
    m_pOKBtn->SetClickHdl( LINK( this, QueryFolderNameDialog, OKHdl ) );
    m_pNameEdit->SetModifyHdl( LINK( this, QueryFolderNameDialog, NameHdl ) );

    if (pGroupName)
        m_pNameLine->set_label( *pGroupName );
};

QueryFolderNameDialog::~QueryFolderNameDialog()
{
    disposeOnce();
}

void QueryFolderNameDialog::dispose()
{
    m_pNameEdit.clear();
    m_pNameLine.clear();
    m_pOKBtn.clear();
    ModalDialog::dispose();
}

IMPL_LINK_NOARG_TYPED(QueryFolderNameDialog, OKHdl, Button*, void)
{
    // trim the strings
    m_pNameEdit->SetText(comphelper::string::strip(m_pNameEdit->GetText(), ' '));
    EndDialog( RET_OK );
}


IMPL_LINK_NOARG(QueryFolderNameDialog, NameHdl)
{
    // trim the strings
    OUString aName = comphelper::string::strip(m_pNameEdit->GetText(), ' ');
    if ( !aName.isEmpty() )
    {
        if ( !m_pOKBtn->IsEnabled() )
            m_pOKBtn->Enable( true );
    }
    else
    {
        if ( m_pOKBtn->IsEnabled() )
            m_pOKBtn->Enable( false );
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
