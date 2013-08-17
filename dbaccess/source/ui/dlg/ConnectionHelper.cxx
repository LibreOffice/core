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
#include "AutoControls.hrc"
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

#if defined(WNT)
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

DBG_NAME(OConnectionHelper)

    OConnectionHelper::OConnectionHelper( Window* pParent, const ResId& _rId, const SfxItemSet& _rCoreAttrs)
        :OGenericAdministrationPage(pParent, _rId, _rCoreAttrs)
        ,m_aFT_Connection   ( this, ResId( FT_AUTOBROWSEURL, *_rId.GetResMgr() ) )
           ,m_aConnectionURL   ( this, ResId( ET_AUTOBROWSEURL, *_rId.GetResMgr() ) )
        ,m_aPB_Connection   ( this, ResId( PB_AUTOBROWSEURL, *_rId.GetResMgr() ) )
    {
        DBG_CTOR(OConnectionHelper,NULL);

        // extract the datasource type collection from the item set
        DbuTypeCollectionItem* pCollectionItem = PTR_CAST(DbuTypeCollectionItem, _rCoreAttrs.GetItem(DSID_TYPECOLLECTION));
        if (pCollectionItem)
            m_pCollection = pCollectionItem->getCollection();
        m_aPB_Connection.SetClickHdl(LINK(this, OConnectionHelper, OnBrowseConnections));
        OSL_ENSURE(m_pCollection, "OConnectionHelper::OConnectionHelper : really need a DSN type collection !");
        m_aConnectionURL.SetTypeCollection(m_pCollection);
    }

    OConnectionHelper::~OConnectionHelper()
    {

        DBG_DTOR(OConnectionHelper,NULL);
    }

    void OConnectionHelper::implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        sal_Bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        m_aFT_Connection.Show();
        m_aConnectionURL.Show();
        m_aConnectionURL.ShowPrefix( ::dbaccess::DST_JDBC == m_pCollection->determineType(m_eType) );

        sal_Bool bEnableBrowseButton = m_pCollection->supportsBrowsing( m_eType );
        m_aPB_Connection.Show( bEnableBrowseButton );

        SFX_ITEMSET_GET(_rSet, pUrlItem, SfxStringItem, DSID_CONNECTURL, sal_True);

        // forward the values to the controls
        if ( bValid )
        {
            String sUrl = pUrlItem->GetValue();
            setURL( sUrl );

            checkTestConnection();
            m_aConnectionURL.ClearModifyFlag();
        }

        OGenericAdministrationPage::implInitControls(_rSet, _bSaveValue);
    }

    void OConnectionHelper::implUpdateURLDependentStates() const
    {
        OSL_PRECOND( m_pAdminDialog, "OConnectionHelper::implUpdateURLDependentStates: no admin dialog!" );
        if ( !m_pAdminDialog )
            return;

        if ( m_pCollection->isFileSystemBased(m_eType) )
            m_pAdminDialog->enableConfirmSettings( getURLNoPrefix().Len() > 0 );
    }

    IMPL_LINK(OConnectionHelper, OnBrowseConnections, PushButton*, /*_pButton*/)
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

                    sal_Bool bDoBrowse = sal_False;
                    String sOldPath = getURLNoPrefix();
                    do
                    {
                        if (sOldPath.Len())
                            xFolderPicker->setDisplayDirectory(sOldPath);
                        if (0 == xFolderPicker->execute())
                            // cancelled by the user
                            return 0L;

                        sOldPath = xFolderPicker->getDirectory();
                        switch (checkPathExistence(sOldPath))
                        {
                            case RET_RETRY:
                                bDoBrowse = sal_True;
                                break;
                            case RET_CANCEL:
                                return 0L;
                            default:
                                break;
                        }
                    }
                    while (bDoBrowse);

                    String sSelectedDirectory = xFolderPicker->getDirectory();
                    INetURLObject aSelectedDirectory( sSelectedDirectory, INetURLObject::WAS_ENCODED, RTL_TEXTENCODING_UTF8 );

                    // for UI purpose, we don't want to have the path encoded
                    sSelectedDirectory = aSelectedDirectory.GetMainURL( INetURLObject::DECODE_WITH_CHARSET, RTL_TEXTENCODING_UTF8  );

                    setURLNoPrefix( sSelectedDirectory );
                    SetRoadmapStateValue(sal_True);
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
                    0,
                    aModule.GetFactoryEmptyDocumentURL(SvtModuleOptions::E_CALC)
                    ,SFX_FILTER_IMPORT);
                askForFileName(aFileDlg);
            }
            break;
            case  ::dbaccess::DST_MSACCESS:
            {
                const OUString sExt("*.mdb");
                String sFilterName(ModuleRes (STR_MSACCESS_FILTERNAME));
                ::sfx2::FileDialogHelper aFileDlg(
                    ui::dialogs::TemplateDescription::FILEOPEN_READONLY_VERSION,
                    0);
                aFileDlg.AddFilter(sFilterName,sExt);
                aFileDlg.SetCurrentFilter(sFilterName);
                askForFileName(aFileDlg);
            }
            break;
            case  ::dbaccess::DST_MSACCESS_2007:
            {
                const OUString sAccdb("*.accdb");
                String sFilterName2(ModuleRes (STR_MSACCESS_2007_FILTERNAME));
                ::sfx2::FileDialogHelper aFileDlg(
                    ui::dialogs::TemplateDescription::FILEOPEN_READONLY_VERSION,
                    0);
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
                    SetRoadmapStateValue(sal_True);
                    callModifiedHdl();
                }
                else
                    return 1L;
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
                    return 1L;
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
                ::com::sun::star::uno::Sequence< OUString > list;

                xMozillaBootstrap->getProfileList( profileType, list );
                const OUString * pArray = list.getConstArray();

                sal_Int32 count = list.getLength();

                StringBag aProfiles;
                for (sal_Int32 index=0; index < count; index++)
                    aProfiles.insert(pArray[index]);

                // execute the select dialog
                ODatasourceSelectDialog aSelector(GetParent(), aProfiles);
                OUString sOldProfile=getURLNoPrefix();

                if (!sOldProfile.isEmpty())
                    aSelector.Select(sOldProfile);
                else
                    aSelector.Select(xMozillaBootstrap->getDefaultProfile(profileType));

                if ( RET_OK == aSelector.Execute() )
                    setURLNoPrefix(aSelector.GetSelected());
                break;
            }
            default:
                break;
        }

        checkTestConnection();

        return 0L;
    }

    bool OConnectionHelper::checkTestConnection()
    {
        return true;
    }

    void OConnectionHelper::impl_setURL( const String& _rURL, sal_Bool _bPrefix )
    {
        String sURL( comphelper::string::stripEnd(_rURL, '*') );
        OSL_ENSURE( m_pCollection, "OConnectionHelper::impl_setURL: have no interpreter for the URLs!" );

        if ( m_pCollection && sURL.Len() )
        {
            if ( m_pCollection->isFileSystemBased( m_eType ) )
            {
                // get the two parts: prefix and file URL
                String sTypePrefix, sFileURLEncoded;
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
                if ( sFileURLEncoded.Len() )
                {
                    OFileNotation aFileNotation(sFileURLEncoded);
                    // set this decoded URL as text
                    sURL += String(aFileNotation.get(OFileNotation::N_SYSTEM));
                }
            }
        }

        if ( _bPrefix )
            m_aConnectionURL.SetText( sURL );
        else
            m_aConnectionURL.SetTextNoPrefix( sURL );

        implUpdateURLDependentStates();
    }

    String OConnectionHelper::impl_getURL( sal_Bool _bPrefix ) const
    {
        // get the pure text
        OUString sURL = _bPrefix ? m_aConnectionURL.GetText() : OUString(m_aConnectionURL.GetTextNoPrefix());

        OSL_ENSURE( m_pCollection, "OConnectionHelper::impl_getURL: have no interpreter for the URLs!" );

        if ( m_pCollection && !sURL.isEmpty() )
        {
            if ( m_pCollection->isFileSystemBased( m_eType ) )
            {
                // get the two parts: prefix and file URL
                OUString sTypePrefix, sFileURLDecoded;
                if ( _bPrefix )
                {
                    sTypePrefix = m_pCollection->getPrefix( m_eType );
                    sFileURLDecoded = m_pCollection->cutPrefix( sURL );
                }
                else
                {
                    sFileURLDecoded = sURL;
                }

                sURL = sTypePrefix;
                if ( !sFileURLDecoded.isEmpty() )
                {
                    OFileNotation aFileNotation( sFileURLDecoded, OFileNotation::N_SYSTEM );
                    sURL += String( aFileNotation.get( OFileNotation::N_URL ) );
                }

                // encode the URL
                INetURLObject aFileURL( sFileURLDecoded, INetURLObject::ENCODE_ALL, RTL_TEXTENCODING_UTF8 );
                sFileURLDecoded = aFileURL.GetMainURL( INetURLObject::NO_DECODE );
            }
        }
        return sURL;
    }

    void OConnectionHelper::setURL( const String& _rURL )
    {
        impl_setURL( _rURL, sal_True );
    }

    String OConnectionHelper::getURLNoPrefix( ) const
    {
        return impl_getURL( sal_False );
    }

    void OConnectionHelper::setURLNoPrefix( const String& _rURL )
    {
        impl_setURL( _rURL, sal_False );
    }

    sal_Int32 OConnectionHelper::checkPathExistence(const String& _rURL)
    {
        IS_PATH_EXIST e_exists = pathExists(_rURL, sal_False);
        if (( e_exists == PATH_NOT_EXIST) || ( e_exists == PATH_NOT_KNOWN))
        {
            String sQuery(ModuleRes(STR_ASK_FOR_DIRECTORY_CREATION));
            OFileNotation aTransformer(_rURL);
            sQuery.SearchAndReplaceAscii("$path$", aTransformer.get(OFileNotation::N_SYSTEM));

            m_bUserGrabFocus = sal_False;
            QueryBox aQuery(GetParent(), WB_YES_NO | WB_DEF_YES, sQuery);
            sal_Int32 nQueryResult = aQuery.Execute();
            m_bUserGrabFocus = sal_True;

            switch (nQueryResult)
            {
                case RET_YES:
                {
                    sal_Bool bTryCreate = sal_False;
                    do
                    {
                        if ( !createDirectoryDeep(_rURL) )
                        {   // could not create the directory
                            sQuery = String(ModuleRes(STR_COULD_NOT_CREATE_DIRECTORY));
                            sQuery.SearchAndReplaceAscii("$name$", aTransformer.get(OFileNotation::N_SYSTEM));

                            m_bUserGrabFocus = sal_False;
                            QueryBox aWhatToDo(GetParent(), WB_RETRY_CANCEL | WB_DEF_RETRY, sQuery);
                            nQueryResult = aWhatToDo.Execute();
                            m_bUserGrabFocus = sal_True;

                            if (RET_RETRY == nQueryResult)
                                bTryCreate = sal_True;
                            else
                            {
                                SetRoadmapStateValue(sal_False);
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
                    SetRoadmapStateValue(sal_False);
                    callModifiedHdl();
                    return RET_CANCEL;
            }
        }
/*        else
        {
            // TODO: error msg
            return RET_CANCEL;
        } */
        SetRoadmapStateValue(sal_True);
        callModifiedHdl();
        return RET_OK;
    }

    IS_PATH_EXIST OConnectionHelper::pathExists(const OUString& _rURL, sal_Bool bIsFile) const
    {
        ::ucbhelper::Content aCheckExistence;
        sal_Bool bExists = sal_False;
        IS_PATH_EXIST eExists = PATH_NOT_EXIST;
        Reference< ::com::sun::star::task::XInteractionHandler > xInteractionHandler(
            task::InteractionHandler::createWithParent(m_xORB, 0), UNO_QUERY );
        OFilePickerInteractionHandler* pHandler = new OFilePickerInteractionHandler(xInteractionHandler);
        xInteractionHandler = pHandler;

        Reference< XCommandEnvironment > xCmdEnv = new ::ucbhelper::CommandEnvironment( xInteractionHandler, Reference< XProgressHandler >() );
        try
        {
            aCheckExistence = ::ucbhelper::Content(_rURL, xCmdEnv, comphelper::getProcessComponentContext());
            bExists = bIsFile? aCheckExistence.isDocument(): aCheckExistence.isFolder();
            eExists = bExists? PATH_EXIST: PATH_NOT_EXIST;
        }
        catch(const Exception&)
        {
            eExists = ( pHandler && pHandler->isDoesNotExist() ) ? PATH_NOT_EXIST: (bIsFile ? PATH_NOT_EXIST : PATH_NOT_KNOWN);
           }
        return eExists;
    }
    long OConnectionHelper::PreNotify( NotifyEvent& _rNEvt )
    {
        if ( m_pCollection->isFileSystemBased(m_eType) )
        {
            switch (_rNEvt.GetType())
            {
                case EVENT_GETFOCUS:
                    if (m_aConnectionURL.IsWindowOrChild(_rNEvt.GetWindow()) && m_bUserGrabFocus)
                    {   // a descendant of the URL edit field got the focus
                        m_aConnectionURL.SaveValueNoPrefix();
                    }
                    break;

                case EVENT_LOSEFOCUS:
                    if (m_aConnectionURL.IsWindowOrChild(_rNEvt.GetWindow()) && m_bUserGrabFocus)
                    {   // a descendant of the URL edit field lost the focus
                        if (!commitURL())
                            return 1L;  // handled
                    }
                    break;
            }
        }

        return OGenericAdministrationPage::PreNotify( _rNEvt );
    }

    sal_Bool OConnectionHelper::createDirectoryDeep(const String& _rPathURL)
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
            eParentExists = pathExists(aParser.GetMainURL(INetURLObject::NO_DECODE), sal_False);
        }

        if (!aParser.getSegmentCount())
            return sal_False;

        // create all the missing levels
        try
        {
            // the parent content
            Reference< XCommandEnvironment > xEmptyEnv;
            ::ucbhelper::Content aParent(aParser.GetMainURL(INetURLObject::NO_DECODE), xEmptyEnv, comphelper::getProcessComponentContext());

            OUString sContentType;
            if ( INET_PROT_FILE == eProtocol )
            {
                sContentType = OUString("application/vnd.sun.staroffice.fsys-folder");
                // the file UCP currently does not support the ContentType property
            }
            else
            {
                Any aContentType = aParent.getPropertyValue("ContentType");
                aContentType >>= sContentType;
            }

            // the properties which need to be set on the new content
            Sequence< OUString > aNewDirectoryProperties(1);
            aNewDirectoryProperties[0] = OUString("Title");

            // the values to be set
            Sequence< Any > aNewDirectoryAttributes(1);

            // loop
            for (   ::std::vector< OUString >::reverse_iterator aLocalName = aToBeCreated.rbegin();
                    aLocalName != aToBeCreated.rend();
                    ++aLocalName
                )
            {
                aNewDirectoryAttributes[0] <<= *aLocalName;
                if (!aParent.insertNewContent(sContentType, aNewDirectoryProperties, aNewDirectoryAttributes, aParent))
                    return sal_False;
            }
        }
        catch ( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
            return sal_False;
        }

        return sal_True;
    }

    void OConnectionHelper::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aFT_Connection));
        _rControlList.push_back(new ODisableWrapper<PushButton>(&m_aPB_Connection));
    }

    void OConnectionHelper::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        _rControlList.push_back( new OSaveValueWrapper<Edit>( &m_aConnectionURL ) );
    }

    sal_Bool OConnectionHelper::commitURL()
    {
        String sURL;
        String sOldPath;
        sOldPath = m_aConnectionURL.GetSavedValueNoPrefix();
        sURL = m_aConnectionURL.GetTextNoPrefix();

        if ( m_pCollection->isFileSystemBased(m_eType) )
        {
            if ( ( sURL != sOldPath ) && ( 0 != sURL.Len() ) )
            {   // the text changed since entering the control

                // the path may be in system notation ....
                OFileNotation aTransformer(sURL);
                sURL = aTransformer.get(OFileNotation::N_URL);

                const ::dbaccess::DATASOURCE_TYPE eType = m_pCollection->determineType(m_eType);

                if ( ( ::dbaccess::DST_CALC == eType) || ( ::dbaccess::DST_MSACCESS == eType) || ( ::dbaccess::DST_MSACCESS_2007 == eType) )
                {
                    if( pathExists(sURL, sal_True) == PATH_NOT_EXIST )
                    {
                        String sFile = String( ModuleRes( STR_FILE_DOES_NOT_EXIST ) );
                        sFile.SearchAndReplaceAscii("$file$", aTransformer.get(OFileNotation::N_SYSTEM));
                        OSQLWarningBox( this, sFile ).Execute();
                        setURLNoPrefix(sOldPath);
                        SetRoadmapStateValue(sal_False);
                        callModifiedHdl();
                        return sal_False;
                    }
                }
                else
                {
                    switch (checkPathExistence(sURL))
                    {
                        case RET_RETRY:
                            m_bUserGrabFocus = sal_False;
                            m_aConnectionURL.GrabFocus();
                            m_bUserGrabFocus = sal_True;
                            return sal_False;

                        case RET_CANCEL:
                            setURLNoPrefix(sOldPath);
                            return sal_False;
                    }
                }
            }
        }

        setURLNoPrefix(sURL);
        m_aConnectionURL.SaveValueNoPrefix();
        return sal_True;
    }
    void OConnectionHelper::askForFileName(::sfx2::FileDialogHelper& _aFileOpen)
    {
        String sOldPath = getURLNoPrefix();
        if ( sOldPath.Len() )
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
