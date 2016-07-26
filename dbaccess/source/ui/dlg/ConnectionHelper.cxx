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

#include "dsnItem.hxx"
#include "ConnectionHelper.hxx"
#include "dbu_dlg.hrc"
#include "dbu_misc.hrc"
#include <svl/itemset.hxx>
#include <unotools/moduleoptions.hxx>
#include <sfx2/fcontnr.hxx>
#include <unotools/pathoptions.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include "dsitems.hxx"
#include "dbaccess_helpid.hrc"
#include "localresaccess.hxx"
#include <osl/process.h>
#include <osl/diagnose.h>
#include <vcl/msgbox.hxx>
#include <sfx2/filedlghelper.hxx>
#include "dbadmin.hxx"
#include <comphelper/types.hxx>
#include <vcl/stdtext.hxx>
#include "sqlmessage.hxx"
#include "odbcconfig.hxx"
#include "dsselect.hxx"
#include <svl/filenotation.hxx>
#include "dbustrings.hrc"
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/mozilla/MozillaBootstrap.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>
#include "UITools.hxx"
#include <unotools/localfilehelper.hxx>
#include <unotools/ucbhelper.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include "finteraction.hxx"
#include <connectivity/CommonTools.hxx>
#include <tools/urlobj.hxx>
#include <tools/diagnose_ex.h>
#include <sfx2/docfilt.hxx>

#if defined(_WIN32)
#define _ADO_DATALINK_BROWSE_
#endif

#ifdef _ADO_DATALINK_BROWSE_
#include <vcl/sysdata.hxx>
#include "adodatalinks.hxx"
#endif //_ADO_DATALINK_BROWSE_

#include <com/sun/star/mozilla/XMozillaBootstrap.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>

namespace dbaui
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::ucb;
    using namespace ::com::sun::star::ui::dialogs;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::mozilla;
    using namespace ::dbtools;
    using namespace ::svt;

    OConnectionHelper::OConnectionHelper( vcl::Window* pParent, const OString& _rId, const OUString& _rUIXMLDescription, const SfxItemSet& _rCoreAttrs)
        : OGenericAdministrationPage(pParent, _rId, _rUIXMLDescription, _rCoreAttrs)
        , m_bUserGrabFocus(false)
        , m_pCollection(nullptr)
    {
        get(m_pFT_Connection, "browseurllabel");
        get(m_pConnectionURL, "browseurl");
        get(m_pPB_Connection, "browse");
        get(m_pPB_CreateDB, "create");

        // extract the datasource type collection from the item set
        const DbuTypeCollectionItem* pCollectionItem = dynamic_cast<const DbuTypeCollectionItem*>( _rCoreAttrs.GetItem(DSID_TYPECOLLECTION) );
        if (pCollectionItem)
            m_pCollection = pCollectionItem->getCollection();
        m_pPB_Connection->SetClickHdl(LINK(this, OConnectionHelper, OnBrowseConnections));
        m_pPB_CreateDB->SetClickHdl(LINK(this, OConnectionHelper, OnCreateDatabase));
        OSL_ENSURE(m_pCollection, "OConnectionHelper::OConnectionHelper : really need a DSN type collection !");
        m_pConnectionURL->SetTypeCollection(m_pCollection);
    }

    OConnectionHelper::~OConnectionHelper()
    {
        disposeOnce();
    }

    void OConnectionHelper::dispose()
    {
        // FIXME: used to have an if (m_bDelete) ...
        m_pFT_Connection.disposeAndClear();
        m_pConnectionURL.disposeAndClear();
        m_pPB_Connection.disposeAndClear();
        m_pPB_CreateDB.disposeAndClear();
        OGenericAdministrationPage::dispose();
    }

    void OConnectionHelper::implInitControls(const SfxItemSet& _rSet, bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        m_pFT_Connection->Show();
        m_pConnectionURL->Show();
        m_pConnectionURL->Resize();
        m_pConnectionURL->ShowPrefix( ::dbaccess::DST_JDBC == m_pCollection->determineType(m_eType) );

        bool bEnableBrowseButton = m_pCollection->supportsBrowsing( m_eType );
        m_pPB_Connection->Show( bEnableBrowseButton );

        bool bEnableCreateButton = m_pCollection->supportsDBCreation( m_eType );
        m_pPB_CreateDB->Show( bEnableCreateButton );

        const SfxStringItem* pUrlItem = _rSet.GetItem<SfxStringItem>(DSID_CONNECTURL);

        // forward the values to the controls
        if ( bValid )
        {
            OUString sUrl = pUrlItem->GetValue();
            setURL( sUrl );

            checkTestConnection();
            m_pConnectionURL->ClearModifyFlag();
        }

        OGenericAdministrationPage::implInitControls(_rSet, _bSaveValue);
    }

    void OConnectionHelper::implUpdateURLDependentStates() const
    {
        OSL_PRECOND( m_pAdminDialog && m_pCollection, "OConnectionHelper::implUpdateURLDependentStates: no admin dialog!" );
        if ( !m_pAdminDialog || !m_pCollection )
            return;

        if ( m_pCollection->isFileSystemBased(m_eType) )
            m_pAdminDialog->enableConfirmSettings( !getURLNoPrefix().isEmpty() );
    }

    IMPL_LINK_NOARG_TYPED(OConnectionHelper, OnBrowseConnections, Button*, void)
    {
        OSL_ENSURE(m_pAdminDialog,"No Admin dialog set! ->GPF");
        const ::dbaccess::DATASOURCE_TYPE eType = m_pCollection->determineType(m_eType);
        switch ( eType )
        {
            case  ::dbaccess::DST_DBASE:
            case  ::dbaccess::DST_FLAT:
            {
                try
                {
                    Reference< XFolderPicker2 > xFolderPicker = FolderPicker::create(m_xORB);

                    bool bDoBrowse = false;
                    OUString sOldPath = getURLNoPrefix();
                    do
                    {
                        if (!sOldPath.isEmpty())
                            xFolderPicker->setDisplayDirectory(sOldPath);
                        if (0 == xFolderPicker->execute())
                            // cancelled by the user
                            return;

                        sOldPath = xFolderPicker->getDirectory();
                        switch (checkPathExistence(sOldPath))
                        {
                            case RET_RETRY:
                                bDoBrowse = true;
                                break;
                            case RET_CANCEL:
                                return;
                            default:
                                break;
                        }
                    }
                    while (bDoBrowse);

                    OUString sSelectedDirectory = xFolderPicker->getDirectory();
                    INetURLObject aSelectedDirectory( sSelectedDirectory, INetURLObject::WAS_ENCODED, RTL_TEXTENCODING_UTF8 );

                    // for UI purpose, we don't want to have the path encoded
                    sSelectedDirectory = aSelectedDirectory.GetMainURL( INetURLObject::DECODE_WITH_CHARSET );

                    setURLNoPrefix( sSelectedDirectory );
                    SetRoadmapStateValue(true);
                    callModifiedHdl();
                }
                catch( const Exception& )
                {
                    DBG_UNHANDLED_EXCEPTION();
                }
            }
            break;
            case  ::dbaccess::DST_CALC:
            {
                SvtModuleOptions aModule;
                ::sfx2::FileDialogHelper aFileDlg(
                    ui::dialogs::TemplateDescription::FILEOPEN_READONLY_VERSION,
                    FileDialogFlags::NONE,
                    aModule.GetFactoryEmptyDocumentURL(SvtModuleOptions::EFactory::CALC)
                    ,SfxFilterFlags::IMPORT);
                askForFileName(aFileDlg);
            }
            break;
            case  ::dbaccess::DST_MSACCESS:
            {
                const OUString sExt("*.mdb;*.mde");
                OUString sFilterName(ModuleRes (STR_MSACCESS_FILTERNAME));
                ::sfx2::FileDialogHelper aFileDlg(
                    ui::dialogs::TemplateDescription::FILEOPEN_READONLY_VERSION);
                aFileDlg.AddFilter(sFilterName,sExt);
                aFileDlg.SetCurrentFilter(sFilterName);
                askForFileName(aFileDlg);
            }
            break;
            case  ::dbaccess::DST_MSACCESS_2007:
            {
                const OUString sAccdb("*.accdb;*.accde");
                OUString sFilterName2(ModuleRes (STR_MSACCESS_2007_FILTERNAME));
                ::sfx2::FileDialogHelper aFileDlg(
                    ui::dialogs::TemplateDescription::FILEOPEN_READONLY_VERSION);
                aFileDlg.AddFilter(sFilterName2,sAccdb);
                aFileDlg.SetCurrentFilter(sFilterName2);
                askForFileName(aFileDlg);
            }
            break;
            case  ::dbaccess::DST_MYSQL_ODBC:
            case  ::dbaccess::DST_ODBC:
            {
                // collect all ODBC data source names
                OUString sCurrDatasource = getURLNoPrefix();
                OUString sDataSource;
                if ( getSelectedDataSource(sDataSource,sCurrDatasource) && !sDataSource.isEmpty() )
                {
                    setURLNoPrefix(sDataSource);
                    SetRoadmapStateValue(true);
                    callModifiedHdl();
                }
                else
                    return;
            }
            break;
#ifdef _ADO_DATALINK_BROWSE_
            case  ::dbaccess::DST_ADO:
            {
                OUString sOldDataSource=getURLNoPrefix();
                OUString sNewDataSource;
                HWND hWnd = GetParent()->GetSystemData()->hWnd;
                sNewDataSource = getAdoDatalink((LONG_PTR)hWnd,sOldDataSource);
                if ( !sNewDataSource.isEmpty() )
                {
                    setURLNoPrefix(sNewDataSource);
                    SetRoadmapStateValue(sal_True);
                    callModifiedHdl();
                }
                else
                    return;
            }
            break;
#endif
            case  ::dbaccess::DST_MOZILLA:
            case  ::dbaccess::DST_THUNDERBIRD:
            {
                MozillaProductType profileType = MozillaProductType_Mozilla;
                if (eType ==  ::dbaccess::DST_THUNDERBIRD)
                    profileType = MozillaProductType_Thunderbird;

                Reference<XComponentContext> xContext = ::comphelper::getProcessComponentContext();
                Reference<XMozillaBootstrap> xMozillaBootstrap = MozillaBootstrap::create(xContext);

                // collect all Mozilla Profiles
                css::uno::Sequence< OUString > list;

                xMozillaBootstrap->getProfileList( profileType, list );
                const OUString * pArray = list.getConstArray();

                sal_Int32 count = list.getLength();

                StringBag aProfiles;
                for (sal_Int32 index=0; index < count; index++)
                    aProfiles.insert(pArray[index]);

                // execute the select dialog
                ScopedVclPtrInstance< ODatasourceSelectDialog > aSelector(GetParent(), aProfiles);
                OUString sOldProfile=getURLNoPrefix();

                if (!sOldProfile.isEmpty())
                    aSelector->Select(sOldProfile);
                else
                    aSelector->Select(xMozillaBootstrap->getDefaultProfile(profileType));

                if ( RET_OK == aSelector->Execute() )
                    setURLNoPrefix(aSelector->GetSelected());
                break;
            }
            case ::dbaccess::DST_FIREBIRD:
            {
                const OUString sExt("*.fdb");
                OUString sFilterName(ModuleRes (STR_FIREBIRD_FILTERNAME));
                ::sfx2::FileDialogHelper aFileDlg(
                    ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE);
                aFileDlg.AddFilter(sFilterName,sExt);
                aFileDlg.SetCurrentFilter(sFilterName);
                askForFileName(aFileDlg);
                break;
            }
            default:
                break;
        }

        checkTestConnection();
    }

    IMPL_LINK_NOARG_TYPED(OConnectionHelper, OnCreateDatabase, Button*, void)
    {
        OSL_ENSURE(m_pAdminDialog,"No Admin dialog set! ->GPF");
        const ::dbaccess::DATASOURCE_TYPE eType = m_pCollection->determineType(m_eType);
        switch ( eType )
        {
        case ::dbaccess::DST_FIREBIRD:
            {
                const OUString sExt("*.fdb");
                OUString sFilterName(ModuleRes (STR_FIREBIRD_FILTERNAME));
                ::sfx2::FileDialogHelper aFileDlg(
                    ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION);
                aFileDlg.AddFilter(sFilterName,sExt);
                aFileDlg.SetCurrentFilter(sFilterName);
                askForFileName(aFileDlg);
                break;
            }
            default:
                break;
        }

        checkTestConnection();
    }

    bool OConnectionHelper::checkTestConnection()
    {
        return true;
    }

    void OConnectionHelper::impl_setURL( const OUString& _rURL, bool _bPrefix )
    {
        OUString sURL( comphelper::string::stripEnd(_rURL, '*') );
        OSL_ENSURE( m_pCollection, "OConnectionHelper::impl_setURL: have no interpreter for the URLs!" );

        if ( m_pCollection && !sURL.isEmpty() )
        {
            if ( m_pCollection->isFileSystemBased( m_eType ) )
            {
                // get the two parts: prefix and file URL
                OUString sTypePrefix, sFileURLEncoded;
                if ( _bPrefix )
                {
                    sTypePrefix = m_pCollection->getPrefix( m_eType );
                    sFileURLEncoded = m_pCollection->cutPrefix( sURL );
                }
                else
                {
                    sFileURLEncoded = sURL;
                }

                // substitute any variables
                sFileURLEncoded = SvtPathOptions().SubstituteVariable( sFileURLEncoded );

                // decode the URL
                sURL = sTypePrefix;
                if ( !sFileURLEncoded.isEmpty() )
                {
                    OFileNotation aFileNotation(sFileURLEncoded);
                    // set this decoded URL as text
                    sURL += aFileNotation.get(OFileNotation::N_SYSTEM);
                }
            }
        }

        if ( _bPrefix )
            m_pConnectionURL->SetText( sURL );
        else
            m_pConnectionURL->SetTextNoPrefix( sURL );

        implUpdateURLDependentStates();
    }

    OUString OConnectionHelper::impl_getURL() const
    {
        // get the pure text
        OUString sURL = m_pConnectionURL->GetTextNoPrefix();

        OSL_ENSURE( m_pCollection, "OConnectionHelper::impl_getURL: have no interpreter for the URLs!" );

        if ( m_pCollection && !sURL.isEmpty() )
        {
            if ( m_pCollection->isFileSystemBased( m_eType ) )
            {
                // get the two parts: prefix and file URL
                OUString sTypePrefix, sFileURLDecoded;
                sFileURLDecoded = sURL;

                sURL = sTypePrefix;
                if ( !sFileURLDecoded.isEmpty() )
                {
                    OFileNotation aFileNotation( sFileURLDecoded, OFileNotation::N_SYSTEM );
                    sURL += aFileNotation.get( OFileNotation::N_URL );
                }

                // encode the URL
                INetURLObject aFileURL( sFileURLDecoded, INetURLObject::ENCODE_ALL, RTL_TEXTENCODING_UTF8 );
                sFileURLDecoded = aFileURL.GetMainURL( INetURLObject::NO_DECODE );
            }
        }
        return sURL;
    }

    void OConnectionHelper::setURL( const OUString& _rURL )
    {
        impl_setURL( _rURL, true );
    }

    OUString OConnectionHelper::getURLNoPrefix( ) const
    {
        return impl_getURL();
    }

    void OConnectionHelper::setURLNoPrefix( const OUString& _rURL )
    {
        impl_setURL( _rURL, false );
    }

    sal_Int32 OConnectionHelper::checkPathExistence(const OUString& _rURL)
    {
        IS_PATH_EXIST e_exists = pathExists(_rURL, false);
        if (!m_pCollection->supportsDBCreation(m_eType) &&
            (( e_exists == PATH_NOT_EXIST) || ( e_exists == PATH_NOT_KNOWN)))
        {
            OUString sQuery(ModuleRes(STR_ASK_FOR_DIRECTORY_CREATION));
            OFileNotation aTransformer(_rURL);
            sQuery = sQuery.replaceFirst("$path$", aTransformer.get(OFileNotation::N_SYSTEM));

            m_bUserGrabFocus = false;
            ScopedVclPtrInstance< QueryBox > aQuery(GetParent(), WB_YES_NO | WB_DEF_YES, sQuery);
            sal_Int32 nQueryResult = aQuery->Execute();
            m_bUserGrabFocus = true;

            switch (nQueryResult)
            {
                case RET_YES:
                {
                    bool bTryCreate = false;
                    do
                    {
                        if ( !createDirectoryDeep(_rURL) )
                        {   // could not create the directory
                            sQuery = ModuleRes(STR_COULD_NOT_CREATE_DIRECTORY);
                            sQuery = sQuery.replaceFirst("$name$", aTransformer.get(OFileNotation::N_SYSTEM));

                            m_bUserGrabFocus = false;
                            ScopedVclPtrInstance< QueryBox > aWhatToDo(GetParent(), WB_RETRY_CANCEL | WB_DEF_RETRY, sQuery);
                            nQueryResult = aWhatToDo->Execute();
                            m_bUserGrabFocus = true;

                            if (RET_RETRY == nQueryResult)
                                bTryCreate = true;
                            else
                            {
                                SetRoadmapStateValue(false);
                                callModifiedHdl();
                                return RET_RETRY;
                            }
                        }
                    }
                    while (bTryCreate);
                }
                break;

                case RET_NO:
                    callModifiedHdl();
                    return RET_OK;

                default:
                    // cancelled
                    SetRoadmapStateValue(false);
                    callModifiedHdl();
                    return RET_CANCEL;
            }
        }
/*        else
        {
            // TODO: error msg
            return RET_CANCEL;
        } */
        SetRoadmapStateValue(true);
        callModifiedHdl();
        return RET_OK;
    }

    IS_PATH_EXIST OConnectionHelper::pathExists(const OUString& _rURL, bool bIsFile) const
    {
        ::ucbhelper::Content aCheckExistence;
        IS_PATH_EXIST eExists = PATH_NOT_EXIST;
        Reference< css::task::XInteractionHandler > xInteractionHandler(
            task::InteractionHandler::createWithParent(m_xORB, nullptr), UNO_QUERY );
        OFilePickerInteractionHandler* pHandler = new OFilePickerInteractionHandler(xInteractionHandler);
        xInteractionHandler = pHandler;

        Reference< XCommandEnvironment > xCmdEnv = new ::ucbhelper::CommandEnvironment( xInteractionHandler, Reference< XProgressHandler >() );
        try
        {
            aCheckExistence = ::ucbhelper::Content(_rURL, xCmdEnv, comphelper::getProcessComponentContext());
            const bool bExists = bIsFile? aCheckExistence.isDocument(): aCheckExistence.isFolder();
            eExists = bExists? PATH_EXIST: PATH_NOT_EXIST;
        }
        catch (const Exception&)
        {
            eExists = pHandler->isDoesNotExist() ? PATH_NOT_EXIST : (bIsFile ? PATH_NOT_EXIST : PATH_NOT_KNOWN);
        }
        return eExists;
    }
    bool OConnectionHelper::PreNotify( NotifyEvent& _rNEvt )
    {
        if ( m_pCollection->isFileSystemBased(m_eType) )
        {
            switch (_rNEvt.GetType())
            {
                case MouseNotifyEvent::GETFOCUS:
                    if (m_pConnectionURL->IsWindowOrChild(_rNEvt.GetWindow()) && m_bUserGrabFocus)
                    {   // a descendant of the URL edit field got the focus
                        m_pConnectionURL->SaveValueNoPrefix();
                    }
                    break;

                case MouseNotifyEvent::LOSEFOCUS:
                    if (m_pConnectionURL->IsWindowOrChild(_rNEvt.GetWindow()) && m_bUserGrabFocus)
                    {   // a descendant of the URL edit field lost the focus
                        if (!commitURL())
                            return true;  // handled
                    }
                    break;
                default:
                    break;
            }
        }

        return OGenericAdministrationPage::PreNotify( _rNEvt );
    }

    bool OConnectionHelper::createDirectoryDeep(const OUString& _rPathURL)
    {
        // get an URL object analyzing the URL for us ...
        INetURLObject aParser;
        aParser.SetURL(_rPathURL);

        INetProtocol eProtocol = aParser.GetProtocol();

        ::std::vector< OUString > aToBeCreated;  // the to-be-created levels

        // search a level which exists
        IS_PATH_EXIST eParentExists = PATH_NOT_EXIST;
        while ( eParentExists == PATH_NOT_EXIST && aParser.getSegmentCount())
        {
            aToBeCreated.push_back(aParser.getName());  // remember the local name for creation
            aParser.removeSegment();                    // cut the local name
            eParentExists = pathExists(aParser.GetMainURL(INetURLObject::NO_DECODE), false);
        }

        if (!aParser.getSegmentCount())
            return false;

        // create all the missing levels
        try
        {
            // the parent content
            Reference< XCommandEnvironment > xEmptyEnv;
            ::ucbhelper::Content aParent(aParser.GetMainURL(INetURLObject::NO_DECODE), xEmptyEnv, comphelper::getProcessComponentContext());

            OUString sContentType;
            if ( INetProtocol::File == eProtocol )
            {
                sContentType = "application/vnd.sun.staroffice.fsys-folder";
                // the file UCP currently does not support the ContentType property
            }
            else
            {
                Any aContentType = aParent.getPropertyValue("ContentType");
                aContentType >>= sContentType;
            }

            // the properties which need to be set on the new content
            Sequence< OUString > aNewDirectoryProperties { "Title" };

            // the values to be set
            Sequence< Any > aNewDirectoryAttributes(1);

            // loop
            for (   ::std::vector< OUString >::const_reverse_iterator aLocalName = aToBeCreated.rbegin();
                    aLocalName != aToBeCreated.rend();
                    ++aLocalName
                )
            {
                aNewDirectoryAttributes[0] <<= *aLocalName;
                if (!aParent.insertNewContent(sContentType, aNewDirectoryProperties, aNewDirectoryAttributes, aParent))
                    return false;
            }
        }
        catch ( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
            return false;
        }

        return true;
    }

    void OConnectionHelper::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        _rControlList.push_back(new ODisableWrapper<FixedText>(m_pFT_Connection));
        _rControlList.push_back(new ODisableWrapper<PushButton>(m_pPB_Connection));
        _rControlList.push_back(new ODisableWrapper<PushButton>(m_pPB_CreateDB));
    }

    void OConnectionHelper::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        _rControlList.push_back( new OSaveValueWrapper<Edit>( m_pConnectionURL ) );
    }

    bool OConnectionHelper::commitURL()
    {
        OUString sURL;
        OUString sOldPath;
        sOldPath = m_pConnectionURL->GetSavedValueNoPrefix();
        sURL = m_pConnectionURL->GetTextNoPrefix();

        if ( m_pCollection->isFileSystemBased(m_eType) )
        {
            if ( ( sURL != sOldPath ) && !sURL.isEmpty() )
            {   // the text changed since entering the control

                // the path may be in system notation ....
                OFileNotation aTransformer(sURL);
                sURL = aTransformer.get(OFileNotation::N_URL);

                const ::dbaccess::DATASOURCE_TYPE eType = m_pCollection->determineType(m_eType);

                if ( ( ::dbaccess::DST_CALC == eType) || ( ::dbaccess::DST_MSACCESS == eType) || ( ::dbaccess::DST_MSACCESS_2007 == eType) )
                {
                    if( pathExists(sURL, true) == PATH_NOT_EXIST )
                    {
                        OUString sFile = ModuleRes( STR_FILE_DOES_NOT_EXIST );
                        sFile = sFile.replaceFirst("$file$", aTransformer.get(OFileNotation::N_SYSTEM));
                        ScopedVclPtrInstance<OSQLWarningBox>(this, sFile)->Execute();
                        setURLNoPrefix(sOldPath);
                        SetRoadmapStateValue(false);
                        callModifiedHdl();
                        return false;
                    }
                }
                else
                {
                    switch (checkPathExistence(sURL))
                    {
                        case RET_RETRY:
                            m_bUserGrabFocus = false;
                            m_pConnectionURL->GrabFocus();
                            m_bUserGrabFocus = true;
                            return false;

                        case RET_CANCEL:
                            setURLNoPrefix(sOldPath);
                            return false;
                    }
                }
            }
        }

        setURLNoPrefix(sURL);
        m_pConnectionURL->SaveValueNoPrefix();
        return true;
    }
    void OConnectionHelper::askForFileName(::sfx2::FileDialogHelper& _aFileOpen)
    {
        OUString sOldPath = getURLNoPrefix();
        if ( !sOldPath.isEmpty() )
            _aFileOpen.SetDisplayDirectory(sOldPath);
        else
            _aFileOpen.SetDisplayDirectory( SvtPathOptions().GetWorkPath() );
        if (0 == _aFileOpen.Execute())
        {
            setURLNoPrefix(_aFileOpen.GetPath());
            SetRoadmapStateValue(checkTestConnection());
            callModifiedHdl();
        }
    }

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
