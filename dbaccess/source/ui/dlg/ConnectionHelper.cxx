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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "dsnItem.hxx"
#ifndef DBAUI_CONNECTIONHELPER_HXX
#include "ConnectionHelper.hxx"
#endif
#ifndef _DBAUI_AUTOCONTROLS_HRC_
#include "AutoControls.hrc"
#endif
#ifndef _DBU_DLG_HRC_
#include "dbu_dlg.hrc"
#endif
#ifndef _DBU_MISC_HRC_
#include "dbu_misc.hrc"
#endif
#ifndef _SFXITEMSET_HXX
#include <svl/itemset.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <unotools/moduleoptions.hxx>
#endif
#ifndef _SFX_FCONTNR_HXX
#include <sfx2/fcontnr.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <unotools/pathoptions.hxx>
#endif
#ifndef _SFXSTRITEM_HXX
#include <svl/stritem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svl/eitem.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <svl/intitem.hxx>
#endif
#ifndef _DBAUI_DATASOURCEITEMS_HXX_
#include "dsitems.hxx"
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
#endif
#ifndef _DBAUI_LOCALRESACCESS_HXX_
#include "localresaccess.hxx"
#endif
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif
#ifndef _DBAUI_DBADMIN_HXX_
#include "dbadmin.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _VCL_STDTEXT_HXX
#include <vcl/stdtext.hxx>
#endif
#ifndef _DBAUI_SQLMESSAGE_HXX_
#include "sqlmessage.hxx"
#endif
#ifndef _DBAUI_ODBC_CONFIG_HXX_
#include "odbcconfig.hxx"
#endif
#ifndef _DBAUI_DSSELECT_HXX_
#include "dsselect.hxx"
#endif
#ifndef SVTOOLS_FILENOTATION_HXX_
#include <svl/filenotation.hxx>
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XFOLDERPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFolderPicker.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
// #106016# ------------------------------------
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XPROGRESSHANDLER_HPP_
#include <com/sun/star/ucb/XProgressHandler.hpp>
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif
#ifndef _UNOTOOLS_UCBHELPER_HXX
#include <unotools/ucbhelper.hxx>
#endif
#ifndef _UCBHELPER_COMMANDENVIRONMENT_HXX
#include <ucbhelper/commandenvironment.hxx>
#endif
#ifndef DBAUI_FILEPICKER_INTERACTION_HXX
#include "finteraction.hxx"
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include <connectivity/CommonTools.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX
#include <sfx2/docfilt.hxx>
#endif
#if !defined(WINDOWS_VISTA_PSDK) && defined(WNT)
#define _ADO_DATALINK_BROWSE_
#endif

#ifdef _ADO_DATALINK_BROWSE_
#if defined( WNT )
    #include <tools/prewin.h>
    #include <windows.h>
    #include <tools/postwin.h>
#endif
#ifndef _SV_SYSDATA_HXX
#include <vcl/sysdata.hxx>
#endif
#ifndef _DBAUI_ADO_DATALINK_HXX_
#include "adodatalinks.hxx"
#endif
#endif //_ADO_DATALINK_BROWSE_

#ifndef _COM_SUN_STAR_MOZILLA_XMOZILLABOOTSTRAP_HPP_
#include <com/sun/star/mozilla/XMozillaBootstrap.hpp>
#endif
#include <unotools/processfactory.hxx>



//.........................................................................
namespace dbaui
{
//.........................................................................
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
        DBG_ASSERT(m_pCollection, "OConnectionHelper::OConnectionHelper : really need a DSN type collection !");
        m_aConnectionURL.SetTypeCollection(m_pCollection);
    }


    OConnectionHelper::~OConnectionHelper()
    {

        DBG_DTOR(OConnectionHelper,NULL);
    }


    // -----------------------------------------------------------------------
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

    // -----------------------------------------------------------------------
    void OConnectionHelper::implUpdateURLDependentStates() const
    {
        OSL_PRECOND( m_pAdminDialog, "OConnectionHelper::implUpdateURLDependentStates: no admin dialog!" );
        if ( !m_pAdminDialog )
            return;

        if ( m_pCollection->isFileSystemBased(m_eType) )
            m_pAdminDialog->enableConfirmSettings( getURLNoPrefix().Len() > 0 );
    }

    // -----------------------------------------------------------------------
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
                    ::rtl::OUString sFolderPickerService = ::rtl::OUString::createFromAscii(SERVICE_UI_FOLDERPICKER);
                    Reference< XFolderPicker > xFolderPicker(m_xORB->createInstance(sFolderPickerService), UNO_QUERY);
                    if (!xFolderPicker.is())
                    {
                        ShowServiceNotAvailableError(GetParent(), sFolderPickerService, sal_True);
                        break;
                    }

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
                ::sfx2::FileDialogHelper aFileDlg(WB_3DLOOK | WB_STDMODAL | WB_OPEN
                                                ,aModule.GetFactoryEmptyDocumentURL(SvtModuleOptions::E_CALC)
                                                ,SFX_FILTER_IMPORT);
                askForFileName(aFileDlg);
            }
            break;
            case  ::dbaccess::DST_MSACCESS:
            {
                const ::rtl::OUString sExt(RTL_CONSTASCII_USTRINGPARAM("*.mdb"));
                String sFilterName(ModuleRes (STR_MSACCESS_FILTERNAME));
                ::sfx2::FileDialogHelper aFileDlg(WB_3DLOOK | WB_STDMODAL | WB_OPEN);
                aFileDlg.AddFilter(sFilterName,sExt);
                aFileDlg.SetCurrentFilter(sFilterName);
                askForFileName(aFileDlg);
            }
            break;
            case  ::dbaccess::DST_MSACCESS_2007:
            {
                const ::rtl::OUString sAccdb(RTL_CONSTASCII_USTRINGPARAM("*.accdb"));
                String sFilterName2(ModuleRes (STR_MSACCESS_2007_FILTERNAME));
                ::sfx2::FileDialogHelper aFileDlg(WB_3DLOOK | WB_STDMODAL | WB_OPEN);
                aFileDlg.AddFilter(sFilterName2,sAccdb);
                aFileDlg.SetCurrentFilter(sFilterName2);
                askForFileName(aFileDlg);
            }
            break;
            case  ::dbaccess::DST_ADABAS:
            {
                // collect all names from the config dir
                // and all dir's of the DBWORK/wrk or DBROOT/wrk dir
                // compare the names

                // collect the names of the installed databases
                StringBag aInstalledDBs;
                ::rtl::OUString sAdabasConfigDir,sAdabasWorkDir,sRootDir;
                ::rtl::OUString sEnvVarName(RTL_CONSTASCII_USTRINGPARAM("DBWORK"));
                rtl_uString* pDbVar = NULL;
                if(osl_getEnvironment(sEnvVarName.pData,&pDbVar) == osl_Process_E_None && pDbVar)
                {
                    sAdabasWorkDir = pDbVar;
                    String sURL;
                    utl::LocalFileHelper::ConvertPhysicalNameToURL(sAdabasWorkDir,sURL);
                    sAdabasWorkDir = sURL;
                    rtl_uString_release(pDbVar);
                    pDbVar = NULL;
                }

                sEnvVarName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DBCONFIG"));
                if(osl_getEnvironment(sEnvVarName.pData,&pDbVar) == osl_Process_E_None && pDbVar)
                {
                    sAdabasConfigDir = pDbVar;
                    String sURL;
                    utl::LocalFileHelper::ConvertPhysicalNameToURL(sAdabasConfigDir,sURL);
                    sAdabasConfigDir = sURL;
                    rtl_uString_release(pDbVar);
                    pDbVar = NULL;
                }

                sEnvVarName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DBROOT"));
                if(osl_getEnvironment(sEnvVarName.pData,&pDbVar) == osl_Process_E_None && pDbVar)
                {
                    sRootDir = pDbVar;
                    String sURL;
                    utl::LocalFileHelper::ConvertPhysicalNameToURL(sRootDir,sURL);
                    sRootDir = sURL;
                    rtl_uString_release(pDbVar);
                    pDbVar = NULL;
                }

                sal_Bool bOldFashion = sAdabasConfigDir.getLength() && sAdabasWorkDir.getLength();

                if(!bOldFashion) // we have a normal adabas installation
                {    // so we check the local database names in $DBROOT/config
                    sAdabasConfigDir    = sRootDir;
                    sAdabasWorkDir      = sRootDir;
                }

                if(sAdabasConfigDir.getLength() && sAdabasWorkDir.getLength() && sRootDir.getLength())
                {

                    aInstalledDBs   = getInstalledAdabasDBs(sAdabasConfigDir,sAdabasWorkDir);

                    if(!aInstalledDBs.size() && bOldFashion)
                    {
                        sAdabasConfigDir    = sRootDir;
                        sAdabasWorkDir      = sRootDir;
                        aInstalledDBs       = getInstalledAdabasDBs(sAdabasConfigDir,sAdabasWorkDir);
                    }

                    ODatasourceSelectDialog aSelector(GetParent(), aInstalledDBs, true,m_pItemSetHelper->getWriteOutputSet());
                    if (RET_OK == aSelector.Execute())
                    {
                        setURLNoPrefix(aSelector.GetSelected());
                        //  checkCreateDatabase( ::dbaccess::DST_ADABAS);
                        SetRoadmapStateValue(sal_True);
                        callModifiedHdl();
                    }
                }
                else
                {
                    LocalResourceAccess aLocRes( PAGE_CONNECTION, RSC_TABPAGE );
                    String sError = String(ModuleRes(STR_NO_ADABASE_DATASOURCES));
                    ErrorBox aBox(this, WB_OK, sError);
                    aBox.Execute();
                }
            }
            break;
            case  ::dbaccess::DST_MYSQL_ODBC:
            case  ::dbaccess::DST_ODBC:
            {
                // collect all ODBC data source names
                ::rtl::OUString sCurrDatasource = getURLNoPrefix();
                ::rtl::OUString sDataSource;
                if ( getSelectedDataSource(sDataSource,sCurrDatasource) && sDataSource.getLength() )
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
                ::rtl::OUString sOldDataSource=getURLNoPrefix();
                ::rtl::OUString sNewDataSource;
                HWND hWnd = GetParent()->GetSystemData()->hWnd;
                sNewDataSource = getAdoDatalink((long)hWnd,sOldDataSource);
                if ( sNewDataSource.getLength() )
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

                Reference<XMultiServiceFactory> xFactory = ::comphelper::getProcessServiceFactory();
                OSL_ENSURE( xFactory.is(), "can't get service factory" );

                Reference<XInterface> xInstance = xFactory->createInstance(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.mozilla.MozillaBootstrap")) );
                OSL_ENSURE( xInstance.is(), "failed to create instance" );
                Reference<XMozillaBootstrap> xMozillaBootstrap =  Reference<XMozillaBootstrap>(xInstance,UNO_QUERY);
                OSL_ENSURE( xMozillaBootstrap.is(), "failed to create instance" );

                if (xMozillaBootstrap.is())
                {
                    // collect all Mozilla Profiles
                    ::com::sun::star::uno::Sequence< ::rtl::OUString > list;

                    xMozillaBootstrap->getProfileList( profileType, list );
                    const ::rtl::OUString * pArray = list.getConstArray();

                    sal_Int32 count = list.getLength();

                    StringBag aProfiles;
                    for (sal_Int32 index=0; index < count; index++)
                        aProfiles.insert(pArray[index]);


                    // excute the select dialog
                    ODatasourceSelectDialog aSelector(GetParent(), aProfiles, eType);
                    ::rtl::OUString sOldProfile=getURLNoPrefix();

                    if (sOldProfile.getLength())
                        aSelector.Select(sOldProfile);
                    else
                        aSelector.Select(xMozillaBootstrap->getDefaultProfile(profileType));

                    if ( RET_OK == aSelector.Execute() )
                        setURLNoPrefix(aSelector.GetSelected());
                    break;
                }
            }
            default:
                break;
        }

        checkTestConnection();

        return 0L;
    }

    //-------------------------------------------------------------------------

    bool OConnectionHelper::checkTestConnection()
    {
        return true;
    }

    //-------------------------------------------------------------------------
    void OConnectionHelper::impl_setURL( const String& _rURL, sal_Bool _bPrefix )
    {
        String sURL( _rURL );
        DBG_ASSERT( m_pCollection, "OConnectionHelper::impl_setURL: have no interpreter for the URLs!" );

        if ( m_pCollection && sURL.Len() )
        {
            if ( m_pCollection->isFileSystemBased( m_eType ) )
            {
                // get the tow parts: prefix and file URL
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

    //-------------------------------------------------------------------------
    String OConnectionHelper::impl_getURL( sal_Bool _bPrefix ) const
    {
        // get the pure text
        String sURL = _bPrefix ? m_aConnectionURL.GetText() : m_aConnectionURL.GetTextNoPrefix();

        DBG_ASSERT( m_pCollection, "OConnectionHelper::impl_getURL: have no interpreter for the URLs!" );

        if ( m_pCollection && sURL.Len() )
        {
            if ( m_pCollection->isFileSystemBased( m_eType ) )
            {
                // get the tow parts: prefix and file URL
                String sTypePrefix, sFileURLDecoded;
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
                if ( sFileURLDecoded.Len() )
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

    //-------------------------------------------------------------------------
    void OConnectionHelper::setURL( const String& _rURL )
    {
        impl_setURL( _rURL, sal_True );
    }

    //-------------------------------------------------------------------------
    String OConnectionHelper::getURLNoPrefix( ) const
    {
        return impl_getURL( sal_False );
    }

    //-------------------------------------------------------------------------
    void OConnectionHelper::setURLNoPrefix( const String& _rURL )
    {
        impl_setURL( _rURL, sal_False );
    }

    //-------------------------------------------------------------------------
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
                     // SetRoadmapStateValue(sal_False);
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


    //-------------------------------------------------------------------------
    StringBag OConnectionHelper::getInstalledAdabasDBDirs(const String& _rPath,const ::ucbhelper::ResultSetInclude& _reResultSetInclude)
    {
        INetURLObject aNormalizer;
        aNormalizer.SetSmartProtocol(INET_PROT_FILE);
        aNormalizer.SetSmartURL(_rPath);
        String sAdabasConfigDir = aNormalizer.GetMainURL(INetURLObject::NO_DECODE);

        ::ucbhelper::Content aAdabasConfigDir;
        try
        {
            aAdabasConfigDir = ::ucbhelper::Content(sAdabasConfigDir, Reference< ::com::sun::star::ucb::XCommandEnvironment >());
        }
        catch(::com::sun::star::ucb::ContentCreationException&)
        {
            return StringBag();
        }

        StringBag aInstalledDBs;
        sal_Bool bIsFolder = sal_False;
        try
        {
            bIsFolder = aAdabasConfigDir.isFolder();
        }
        catch(Exception&) // the exception is thrown when the path doesn't exists
        {
        }
        if (bIsFolder && aAdabasConfigDir.get().is())
        {   // we have a content for the directory, loop through all entries
            Sequence< ::rtl::OUString > aProperties(1);
            aProperties[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Title"));

            try
            {
                Reference< XResultSet > xFiles = aAdabasConfigDir.createCursor(aProperties, _reResultSetInclude);
                Reference< XRow > xRow(xFiles, UNO_QUERY);
                xFiles->beforeFirst();
                while (xFiles->next())
                {
#ifdef DBG_UTIL
                    ::rtl::OUString sName = xRow->getString(1);
#endif
                    aInstalledDBs.insert(xRow->getString(1));
                }
            }
            catch(Exception&)
            {
                DBG_ERROR("OConnectionHelper::getInstalledAdabasDBDirs: could not enumerate the adabas config files!");
            }
        }


        return aInstalledDBs;
    }
    // -----------------------------------------------------------------------------
    StringBag OConnectionHelper::getInstalledAdabasDBs(const String &_rConfigDir,const String &_rWorkDir)
    {
        String sAdabasConfigDir(_rConfigDir),sAdabasWorkDir(_rWorkDir);

        if (sAdabasConfigDir.Len() && ('/' == sAdabasConfigDir.GetBuffer()[sAdabasConfigDir.Len() - 1]))
            sAdabasConfigDir.AppendAscii("config");
        else
            sAdabasConfigDir.AppendAscii("/config");

        if (sAdabasWorkDir.Len() && ('/' == sAdabasWorkDir.GetBuffer()[sAdabasWorkDir.Len() - 1]))
            sAdabasWorkDir.AppendAscii("wrk");
        else
            sAdabasWorkDir.AppendAscii("/wrk");
        // collect the names of the installed databases
        StringBag aInstalledDBs;
        // collect the names of the installed databases
        StringBag aConfigDBs,aWrkDBs;
        aConfigDBs  = getInstalledAdabasDBDirs(sAdabasConfigDir,::ucbhelper::INCLUDE_DOCUMENTS_ONLY);
        aWrkDBs     = getInstalledAdabasDBDirs(sAdabasWorkDir,::ucbhelper::INCLUDE_FOLDERS_ONLY);
        ConstStringBagIterator aOuter = aConfigDBs.begin();
        ConstStringBagIterator aOuterEnd = aConfigDBs.end();
        for(;aOuter != aOuterEnd;++aOuter)
        {
            ConstStringBagIterator aInner = aWrkDBs.begin();
            ConstStringBagIterator aInnerEnd = aWrkDBs.end();
            for (;aInner != aInnerEnd; ++aInner)
            {
                if (aInner->equalsIgnoreAsciiCase(*aOuter))
                {
                    aInstalledDBs.insert(*aInner);
                    break;
                }
            }
        }
        return aInstalledDBs;
    }
    // #106016# -------------------------------------------------------------------
    IS_PATH_EXIST OConnectionHelper::pathExists(const ::rtl::OUString& _rURL, sal_Bool bIsFile) const
    {
        ::ucbhelper::Content aCheckExistence;
        sal_Bool bExists = sal_False;
        IS_PATH_EXIST eExists = PATH_NOT_EXIST;
        Reference< ::com::sun::star::task::XInteractionHandler > xInteractionHandler = Reference< ::com::sun::star::task::XInteractionHandler >(
            m_xORB->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.task.InteractionHandler") ) ), UNO_QUERY );
        OFilePickerInteractionHandler* pHandler = new OFilePickerInteractionHandler(xInteractionHandler);
        xInteractionHandler = pHandler;

        Reference< XCommandEnvironment > xCmdEnv = new ::ucbhelper::CommandEnvironment( xInteractionHandler, Reference< XProgressHandler >() );
        try
        {
            aCheckExistence = ::ucbhelper::Content(_rURL, xCmdEnv );
            bExists = bIsFile? aCheckExistence.isDocument(): aCheckExistence.isFolder();
            eExists = bExists? PATH_EXIST: PATH_NOT_EXIST;
        }
        catch(const Exception&)
        {
            eExists = ( pHandler && pHandler->isDoesNotExist() ) ? PATH_NOT_EXIST: (bIsFile ? PATH_NOT_EXIST : PATH_NOT_KNOWN);
           }
        return eExists;
    }
    //-------------------------------------------------------------------------
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
            } // switch (_rNEvt.GetType())
        }

        return OGenericAdministrationPage::PreNotify( _rNEvt );
    }

    //-------------------------------------------------------------------------

    sal_Bool OConnectionHelper::createDirectoryDeep(const String& _rPathURL)
    {
        ::rtl::OUString sPath(_rPathURL);

        // get an URL object analyzing the URL for us ...
        INetURLObject aParser;
        aParser.SetURL(_rPathURL);

        INetProtocol eProtocol = aParser.GetProtocol();

        ::std::vector< ::rtl::OUString > aToBeCreated;  // the to-be-created levels

        // search a level which exists
        // #106016# ---------------------
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
            ::ucbhelper::Content aParent(aParser.GetMainURL(INetURLObject::NO_DECODE), xEmptyEnv);

            ::rtl::OUString sContentType;
            if ( INET_PROT_FILE == eProtocol )
            {
                sContentType = ::rtl::OUString::createFromAscii( "application/vnd.sun.staroffice.fsys-folder" );
                // the file UCP currently does not support the ContentType property
            }
            else
            {
                Any aContentType = aParent.getPropertyValue( ::rtl::OUString::createFromAscii( "ContentType" ) );
                aContentType >>= sContentType;
            }

            // the properties which need to be set on the new content
            Sequence< ::rtl::OUString > aNewDirectoryProperties(1);
            aNewDirectoryProperties[0] = ::rtl::OUString::createFromAscii("Title");

            // the values to be set
            Sequence< Any > aNewDirectoryAttributes(1);

            // loop
            for (   ::std::vector< ::rtl::OUString >::reverse_iterator aLocalName = aToBeCreated.rbegin();
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


    // -----------------------------------------------------------------------
    void OConnectionHelper::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aFT_Connection));
        _rControlList.push_back(new ODisableWrapper<PushButton>(&m_aPB_Connection));
    }


    // -----------------------------------------------------------------------
    void OConnectionHelper::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        _rControlList.push_back( new OSaveValueWrapper<Edit>( &m_aConnectionURL ) );
    }

    //-------------------------------------------------------------------------
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
                { // #106016# --------------------------
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
    //-------------------------------------------------------------------------
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

//.........................................................................
}   // namespace dbaui
//.........................................................................
