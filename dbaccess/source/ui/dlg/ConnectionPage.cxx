/*************************************************************************
 *
 *  $RCSfile: ConnectionPage.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:40:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Ocke Janssen
 *
 *
 ************************************************************************/

#ifndef DBAUI_CONNECTIONPAGE_HXX
#include "ConnectionPage.hxx"
#endif
#ifndef DBAUI_CONNECTIONPAGE_HRC
#include "ConnectionPage.hrc"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBU_DLG_HRC_
#include "dbu_dlg.hrc"
#endif
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
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
#include <svtools/filenotation.hxx>
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
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
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
#ifndef _SFX_DOCFILT_HACK_HXX
#include <sfx2/docfilt.hxx>
#endif
#if defined(WIN) || defined(WNT)
#define _ADO_DATALINK_BROWSE_
#endif

#ifdef _ADO_DATALINK_BROWSE_
typedef void*               HWND;
typedef void*               HMENU;
#ifndef _SV_SYSDATA_HXX
#include <vcl/sysdata.hxx>
#endif
#ifndef _DBAUI_ADO_DATALINK_HXX_
#include "adodatalinks.hxx"
#endif
#endif //_ADO_DATALINK_BROWSE_
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
    using namespace ::dbtools;
    using namespace ::svt;

    SfxTabPage* OConnectionTabPage::Create( Window* pParent,    const SfxItemSet& _rAttrSet )
    {
        return ( new OConnectionTabPage( pParent, _rAttrSet ) );
    }
    //========================================================================
    //= OConnectionTabPage
    //========================================================================
    DBG_NAME(OConnectionTabPage)
    OConnectionTabPage::OConnectionTabPage(Window* pParent, const SfxItemSet& _rCoreAttrs)

        :OGenericAdministrationPage(pParent, ModuleRes(PAGE_CONNECTION), _rCoreAttrs)
        ,m_aUserNameLabel(this, ResId(FT_USERNAME))
        ,m_aUserName(this, ResId(ET_USERNAME))
        ,m_aPasswordRequired(this, ResId(CB_PASSWORD_REQUIRED))
        ,m_aConnection(this, ResId(ET_CONNECTURL))
        ,m_aBrowseConnection(this, ResId(PB_BROWSECONNECTION))
        ,m_aJavaDriverLabel(this, ResId(FT_JDBCDRIVERCLASS))
        ,m_aJavaDriver(this, ResId(ET_JDBCDRIVERCLASS))
        ,m_aTestJavaDriver(this, ResId(PB_TESTDRIVERCLASS))
        ,m_aTestConnection(this, ResId(PB_TESTCONNECTION))
        ,m_aFL1(this, ResId(FL_SEPARATOR1))
        ,m_aFL2(this, ResId(FL_SEPARATOR2))
        ,m_aFL3(this, ResId(FL_SEPARATOR3))
        ,m_pCollection(NULL)
        ,m_pConnectionLabel(NULL)
        ,m_bUserGrabFocus(sal_True)
        ,m_eType(DST_JDBC)
    {
        DBG_CTOR(OConnectionTabPage,NULL);
        m_aConnection.SetModifyHdl(LINK(this, OConnectionTabPage, OnEditModified));
        //  m_aJavaDriver.SetModifyHdl(getControlModifiedLink());
        m_aJavaDriver.SetModifyHdl(LINK(this, OConnectionTabPage, OnEditModified));
        m_aUserName.SetModifyHdl(getControlModifiedLink());
        m_aPasswordRequired.SetClickHdl(getControlModifiedLink());

        m_aBrowseConnection.SetClickHdl(LINK(this, OConnectionTabPage, OnBrowseConnections));
        m_aTestConnection.SetClickHdl(LINK(this,OConnectionTabPage,OnTestConnectionClickHdl));
        m_aTestJavaDriver.SetClickHdl(LINK(this,OConnectionTabPage,OnTestJavaClickHdl));

        // extract the datasource type collection from the item set
        DbuTypeCollectionItem* pCollectionItem = PTR_CAST(DbuTypeCollectionItem, _rCoreAttrs.GetItem(DSID_TYPECOLLECTION));
        if (pCollectionItem)
            m_pCollection = pCollectionItem->getCollection();
        DBG_ASSERT(m_pCollection, "OConnectionTabPage::OConnectionTabPage : really need a DSN type collection !");

        FreeResource();
    }

    // -----------------------------------------------------------------------
    OConnectionTabPage::~OConnectionTabPage()
    {
        DBG_DTOR(OConnectionTabPage,NULL);
        DELETEZ(m_pConnectionLabel);
    }
    // -----------------------------------------------------------------------
    void OConnectionTabPage::implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        sal_Bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        m_eType = m_pAdminDialog->getDatasourceType(_rSet);

        BOOL bEnableBrowseButton = FALSE;
        delete m_pConnectionLabel;
        m_aConnection.Show();

        m_aConnection.ShowPrefix(FALSE);

        BOOL bShowUserAuthenfication = TRUE;

        OLocalResourceAccess aLocRes( PAGE_CONNECTION, RSC_TABPAGE );
        switch( m_eType )
        {
            case DST_DBASE:
                m_pConnectionLabel = new FixedText(this,ResId(FT_DBASE_PATH_OR_FILE));
                m_aConnection.SetHelpId(HID_DSADMIN_DBASE_PATH);
                bEnableBrowseButton = TRUE;
                bShowUserAuthenfication = FALSE;
                break;
            case DST_FLAT:
                m_pConnectionLabel = new FixedText(this,ResId(FT_FLAT_PATH_OR_FILE));
                m_aConnection.SetHelpId(HID_DSADMIN_FLAT_PATH);
                bEnableBrowseButton = TRUE;
                bShowUserAuthenfication = FALSE;
                break;
            case DST_CALC:
                m_pConnectionLabel = new FixedText(this,ResId(FT_CALC_PATH_OR_FILE));
                m_aConnection.SetHelpId(HID_DSADMIN_CALC_PATH);
                bEnableBrowseButton = TRUE;
                bShowUserAuthenfication = TRUE;
                break;
            case DST_ADABAS:
                m_pConnectionLabel = new FixedText(this,ResId(FT_ADABAS_DATABASE_NAME));
                m_aConnection.SetHelpId(HID_DSADMIN_ADABAS_DATABASE);
                bEnableBrowseButton = TRUE;
                break;
            case DST_ADO:
                m_pConnectionLabel = new FixedText(this,ResId(FT_CONNECTURL));
                bEnableBrowseButton = TRUE;
                break;
            case DST_MSACCESS:
                m_pConnectionLabel = new FixedText(this,ResId(FT_MSACCESS_MDB_FILE));
                m_aConnection.SetHelpId(HID_DSADMIN_MSACCESS_MDB_FILE);
                bEnableBrowseButton = TRUE;
                break;
            case DST_MYSQL_JDBC:
                m_pConnectionLabel = new FixedText(this,ResId(FT_MYSQL_DATABASE_NAME));
                m_aConnection.SetHelpId(HID_DSADMIN_MYSQL_DATABASE);
                break;
            case DST_ORACLE_JDBC:
                m_pConnectionLabel = new FixedText(this,ResId(FT_ORACLE_DATABASE_NAME));
                m_aConnection.SetHelpId(HID_DSADMIN_ORACLE_DATABASE);
                break;
            case DST_MYSQL_ODBC:
            case DST_ODBC:
                m_pConnectionLabel = new FixedText(this,ResId(FT_NAME_OF_ODBC_DATASOURCE));
                m_aConnection.SetHelpId( m_eType == DST_MYSQL_ODBC ? HID_DSADMIN_MYSQL_ODBC_DATASOURCE : HID_DSADMIN_ODBC_DATASOURCE);
                bEnableBrowseButton = TRUE;
                break;
            case DST_LDAP:
                m_pConnectionLabel = new FixedText(this,ResId(FT_HOSTNAME));
                m_aConnection.SetHelpId( HID_DSADMIN_LDAP_HOSTNAME );
                bShowUserAuthenfication = TRUE;
                break;
            case DST_MOZILLA:
            case DST_OUTLOOK:
            case DST_OUTLOOKEXP:
            case DST_EVOLUTION:
                m_pConnectionLabel = new FixedText(this,ResId(FT_NO_ADDITIONAL_SETTINGS));
                {
                    String sText = m_pConnectionLabel->GetText();
                    sText.SearchAndReplaceAscii("%test",m_aTestConnection.GetText());
                    String sTemp;
                    sText.SearchAndReplaceAscii("~",sTemp);
                    m_pConnectionLabel->SetText(sText);
                }
                m_aConnection.Hide();
                bShowUserAuthenfication = FALSE;
                break;
            case DST_JDBC:
                m_aConnection.ShowPrefix(TRUE);
                // run through
            default:
                m_pConnectionLabel = new FixedText(this,ResId(FT_CONNECTURL));
                break;
        }

        m_aFL2.Show(bShowUserAuthenfication);
        m_aUserNameLabel.Show(bShowUserAuthenfication);
        m_aUserName.Show(bShowUserAuthenfication);
        m_aPasswordRequired.Show(bShowUserAuthenfication);

        m_aBrowseConnection.Show(bEnableBrowseButton);

        OSL_ENSURE(m_pConnectionLabel,"Connectionlabel not set! ->GPF");
        m_pConnectionLabel->Show();
        // collect the items
        SFX_ITEMSET_GET(_rSet, pUidItem, SfxStringItem, DSID_USER, sal_True);
        SFX_ITEMSET_GET(_rSet, pPwdItem, SfxStringItem, DSID_PASSWORD, sal_True);
        SFX_ITEMSET_GET(_rSet, pJdbcDrvItem, SfxStringItem, DSID_JDBCDRIVERCLASS, sal_True);
        SFX_ITEMSET_GET(_rSet, pUrlItem, SfxStringItem, DSID_CONNECTURL, sal_True);
        SFX_ITEMSET_GET(_rSet, pAllowEmptyPwd, SfxBoolItem, DSID_PASSWORDREQUIRED, sal_True);
        //  SFX_ITEMSET_GET(_rSet, pHostName, SfxStringItem, DSID_CONN_HOSTNAME, sal_True);

        // forward the values to the controls
        if ( bValid )
        {
            m_aUserName.SetText(pUidItem->GetValue());
            m_aPasswordRequired.Check(pAllowEmptyPwd->GetValue());

            String sUrl = pUrlItem->GetValue();
            setURL( sUrl );

            BOOL bEnableJDBC = m_eType == DST_JDBC;
            m_aJavaDriver.SetText(pJdbcDrvItem->GetValue());

            m_aJavaDriverLabel.Show(bEnableJDBC);
            m_aJavaDriver.Show(bEnableJDBC);
            m_aTestJavaDriver.Show(bEnableJDBC);
            m_aTestJavaDriver.Enable( m_aJavaDriver.GetText().Len() != 0);
            m_aFL3.Show(bEnableJDBC);

            checkTestConnection();

            m_aUserName.ClearModifyFlag();
            m_aConnection.ClearModifyFlag();
            m_aJavaDriver.ClearModifyFlag();
        }

        OGenericAdministrationPage::implInitControls(_rSet, _bSaveValue);
    }
    // -----------------------------------------------------------------------
    void OConnectionTabPage::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        _rControlList.push_back(new ODisableWrapper<FixedLine>(&m_aFL1));
        _rControlList.push_back(new ODisableWrapper<FixedText>(m_pConnectionLabel));
        _rControlList.push_back(new ODisableWrapper<PushButton>(&m_aBrowseConnection));

        _rControlList.push_back(new ODisableWrapper<FixedLine>(&m_aFL2));
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aJavaDriverLabel));
        _rControlList.push_back(new ODisableWrapper<PushButton>(&m_aTestJavaDriver));

        _rControlList.push_back(new ODisableWrapper<FixedLine>(&m_aFL3));
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aUserNameLabel));
        _rControlList.push_back(new ODisableWrapper<PushButton>(&m_aTestConnection));
    }
    // -----------------------------------------------------------------------
    void OConnectionTabPage::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        _rControlList.push_back(new OSaveValueWrapper<Edit>(&m_aConnection));
        _rControlList.push_back(new OSaveValueWrapper<Edit>(&m_aJavaDriver));
        _rControlList.push_back(new OSaveValueWrapper<Edit>(&m_aUserName));
        _rControlList.push_back(new OSaveValueWrapper<CheckBox>(&m_aPasswordRequired));
    }

    // -----------------------------------------------------------------------
    sal_Bool OConnectionTabPage::FillItemSet(SfxItemSet& _rSet)
    {
        sal_Bool bChangedSomething = sal_False;

        if (m_aUserName.GetText() != m_aUserName.GetSavedValue())
        {
            _rSet.Put(SfxStringItem(DSID_USER, m_aUserName.GetText()));
            _rSet.Put(SfxStringItem(DSID_PASSWORD, String()));
            bChangedSomething = sal_True;
        }

        fillBool(_rSet,&m_aPasswordRequired,DSID_PASSWORDREQUIRED,bChangedSomething);

        if ( m_eType == DST_JDBC )
        {
            fillString(_rSet,&m_aJavaDriver, DSID_JDBCDRIVERCLASS, bChangedSomething);
        }
        fillString(_rSet,&m_aConnection, DSID_CONNECTURL, bChangedSomething);

        return bChangedSomething;
    }
    // -----------------------------------------------------------------------
    IMPL_LINK(OConnectionTabPage, OnTestConnectionClickHdl, PushButton*, _pButton)
    {
        OSL_ENSURE(m_pAdminDialog,"No Admin dialog set! ->GPF");
        sal_Bool bSuccess = sal_False;
        if ( m_pAdminDialog )
        {
            m_pAdminDialog->saveDatasource();
            OGenericAdministrationPage::implInitControls(*m_pItemSetHelper->getOutputSet(), sal_True);
            try
            {
                Reference< XConnection > xConnection = m_pAdminDialog->createConnection();
                bSuccess = xConnection.is();
                ::comphelper::disposeComponent(xConnection);
            }
            catch(Exception&)
            {
            }

            OSQLMessageBox::MessageType eImage = OSQLMessageBox::Info;
            String aMessage,sTitle;
            sTitle = String (ModuleRes(STR_CONNECTION_TEST));
            if ( bSuccess )
            {
                aMessage = String(ModuleRes(STR_CONNECTION_SUCCESS));
            }
            else
            {
                eImage = OSQLMessageBox::Error;
                aMessage = String(ModuleRes(STR_CONNECTION_NO_SUCCESS));
                m_pAdminDialog->clearPassword();
            }
            OSQLMessageBox aMsg(this,sTitle,aMessage);
            aMsg.Execute();
        }
        return 0L;
    }
    // -----------------------------------------------------------------------
    IMPL_LINK(OConnectionTabPage, OnTestJavaClickHdl, PushButton*, _pButton)
    {
        OSL_ENSURE(m_pAdminDialog,"No Admin dialog set! ->GPF");
        sal_Bool bSuccess = sal_False;
        try
        {
            if ( m_aJavaDriver.GetText().Len() )
            {
                ::rtl::Reference< jvmaccess::VirtualMachine > xJVM = ::connectivity::getJavaVM(m_pAdminDialog->getORB());
                bSuccess = ::connectivity::existsJavaClassByName(xJVM,m_aJavaDriver.GetText());
            }
        }
        catch(Exception&)
        {
        }

        USHORT nMessage = bSuccess ? STR_JDBCDRIVER_SUCCESS : STR_JDBCDRIVER_NO_SUCCESS;

        String aMessage = String(ModuleRes(nMessage));
        String sTitle(ModuleRes(STR_JDBCDRIVER_TEST));
        OSQLMessageBox aMsg(this,sTitle,aMessage);
        aMsg.Execute();
        return 0L;
    }
    // -----------------------------------------------------------------------
    void OConnectionTabPage::checkTestConnection()
    {
        OSL_ENSURE(m_pAdminDialog,"No Admin dialog set! ->GPF");
        BOOL bEnableTestConnection = !m_aConnection.IsVisible() || (m_aConnection.GetTextNoPrefix().Len() != 0);
        if ( m_eType == DST_JDBC )
            bEnableTestConnection = bEnableTestConnection && (m_aJavaDriver.GetText().Len() != 0);
        m_aTestConnection.Enable(bEnableTestConnection);
    }
    // -----------------------------------------------------------------------
    IMPL_LINK(OConnectionTabPage, OnEditModified, Edit*, _pEdit)
    {
        if ( _pEdit == &m_aJavaDriver )
            m_aTestJavaDriver.Enable( m_aJavaDriver.GetText().Len() != 0 );

        checkTestConnection();
        // tell the listener we were modified
        callModifiedHdl();
        return 0L;
    }
    // -----------------------------------------------------------------------
    IMPL_LINK(OConnectionTabPage, OnBrowseConnections, PushButton*, _pButton)
    {
        OSL_ENSURE(m_pAdminDialog,"No Admin dialog set! ->GPF");
        switch ( m_eType )
        {
            case DST_DBASE:
            case DST_FLAT:
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
                    callModifiedHdl();
                }
                catch(const Exception&)
                {
                    DBG_ERROR("OConnectionTabPage::OnBrowseConnections: caught an exception while browsing for the path!");
                }
            }
            break;
            case DST_CALC:
            {
                static const String s_sCalcType = String::CreateFromAscii("StarOffice XML (Calc)");
                const SfxFilter* pFilter = SfxFilter::GetFilterByName( s_sCalcType);
                OSL_ENSURE(pFilter,"Filter: StarOffice XML (Calc) could not be found!");
                askForFileName(pFilter->GetUIName(),pFilter->GetDefaultExtension());
            }
            break;
            case DST_MSACCESS:
            {
                ::rtl::OUString sExt(RTL_CONSTASCII_USTRINGPARAM("*.mdb"));
                String sFilterName(ModuleRes (STR_MSACCESS_FILTERNAME));
                askForFileName(sFilterName,sExt);
            }
            break;
            case DST_ADABAS:
            {
                // collect all names from the config dir
                // and all dir's of the DBWORK/wrk or DBROOT/wrk dir
                // compare the names

                // collect the names of the installed databases
                StringBag aInstalledDBs;
                ::rtl::OUString sAdabasConfigDir,sAdabasWorkDir,sRootDir;
                ::rtl::OUString sTemp(RTL_CONSTASCII_USTRINGPARAM("DBWORK"));
                rtl_uString* pDbVar = NULL;
                if(osl_getEnvironment(sTemp.pData,&pDbVar) == osl_Process_E_None && pDbVar)
                {
                    sAdabasWorkDir = pDbVar;
                    String sTemp;
                    sal_Bool bOk = utl::LocalFileHelper::ConvertPhysicalNameToURL(sAdabasWorkDir,sTemp);
                    sAdabasWorkDir = sTemp;
                    rtl_uString_release(pDbVar);
                    pDbVar = NULL;
                }

                sTemp = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DBCONFIG"));
                if(osl_getEnvironment(sTemp.pData,&pDbVar) == osl_Process_E_None && pDbVar)
                {
                    sAdabasConfigDir = pDbVar;
                    String sTemp;
                    sal_Bool bOk = utl::LocalFileHelper::ConvertPhysicalNameToURL(sAdabasConfigDir,sTemp);
                    sAdabasConfigDir = sTemp;
                    rtl_uString_release(pDbVar);
                    pDbVar = NULL;
                }

                sTemp = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DBROOT"));
                if(osl_getEnvironment(sTemp.pData,&pDbVar) == osl_Process_E_None && pDbVar)
                {
                    sRootDir = pDbVar;
                    String sTemp;
                    sal_Bool bOk = utl::LocalFileHelper::ConvertPhysicalNameToURL(sRootDir,sTemp);
                    sRootDir = sTemp;
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

                    ODatasourceSelectDialog aSelector(GetParent(), aInstalledDBs, m_eType,m_pItemSetHelper->getWriteOutputSet());
                    if (RET_OK == aSelector.Execute())
                    {
                        setURLNoPrefix(aSelector.GetSelected());
                        //  checkCreateDatabase(DST_ADABAS);
                        callModifiedHdl();
                    }
                }
                else
                {
                    OLocalResourceAccess aLocRes( PAGE_CONNECTION, RSC_TABPAGE );
                    String sError = String(ResId(STR_NO_ADABASE_DATASOURCES));
                    ErrorBox aBox(this, WB_OK, sError);
                    aBox.Execute();
                }
            }
            break;
            case DST_MYSQL_ODBC:
            case DST_ODBC:
            {
                // collect all ODBC data source names
                ::rtl::OUString sDataSource;
                if ( getSelectedDataSource(m_eType,sDataSource) && sDataSource.getLength() )
                {
                    setURLNoPrefix(sDataSource);
                    callModifiedHdl();
                }
                else
                    return 1L;
            }
            break;
#ifdef _ADO_DATALINK_BROWSE_
            case DST_ADO:
            {
                ::rtl::OUString sOldDataSource=getURLNoPrefix();
                ::rtl::OUString sNewDataSource;
                HWND hWnd = GetParent()->GetSystemData()->hWnd;
                sNewDataSource = getAdoDatalink((long)hWnd,sOldDataSource);
                if ( sNewDataSource.getLength() )
                {
                    setURLNoPrefix(sNewDataSource);
                    callModifiedHdl();
                }
                else
                    return 1L;
            }
            break;
#endif

        }

        checkTestConnection();

        return 0L;
    }
    //-------------------------------------------------------------------------
    void OConnectionTabPage::implSetURL( const String& _rURL, sal_Bool _bPrefix )
    {
        String sURL( _rURL );
        DBG_ASSERT( m_pCollection, "OConnectionTabPage::setURL: have no interpreter for the URLs!" );

        if ( m_pCollection && sURL.Len() )
        {
            if ( m_pCollection->isFileSystemBased( m_eType ) )
            {
                // get the tow parts: prefix and file URL
                String sTypePrefix, sFileURLEncoded;
                if ( _bPrefix )
                {
                    sTypePrefix = m_pCollection->getDatasourcePrefix( m_eType );
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
            m_aConnection.SetText( sURL );
        else
            m_aConnection.SetTextNoPrefix( sURL );
    }

    //-------------------------------------------------------------------------
    String OConnectionTabPage::implGetURL( sal_Bool _bPrefix ) const
    {
        // get the pure text
        String sURL = _bPrefix ? m_aConnection.GetText() : m_aConnection.GetTextNoPrefix();

        DBG_ASSERT( m_pCollection, "OConnectionTabPage::implGetURL: have no interpreter for the URLs!" );

        if ( m_pCollection && sURL.Len() )
        {
            if ( m_pCollection->isFileSystemBased( m_eType ) )
            {
                // get the tow parts: prefix and file URL
                String sTypePrefix, sFileURLDecoded;
                if ( _bPrefix )
                {
                    sTypePrefix = m_pCollection->getDatasourcePrefix( m_eType );
                    sFileURLDecoded = m_pCollection->cutPrefix( sURL );
                }
                else
                {
                    sFileURLDecoded = sURL;
                }

                // encode the URL
                INetURLObject aFileURL( sFileURLDecoded, INetURLObject::ENCODE_ALL, RTL_TEXTENCODING_UTF8 );
                sFileURLDecoded = aFileURL.GetMainURL( INetURLObject::NO_DECODE );
                sURL = sTypePrefix;
                if ( sFileURLDecoded.Len() )
                {
                    OFileNotation aFileNotation(sFileURLDecoded,OFileNotation::N_SYSTEM);

                    // set this decoded URL as text
                    sURL += String(aFileNotation.get(OFileNotation::N_URL));
                }
            }
        }
        return sURL;
    }
    //-------------------------------------------------------------------------
    String OConnectionTabPage::getURL( ) const
    {
        return implGetURL( sal_True );
    }

    //-------------------------------------------------------------------------
    void OConnectionTabPage::setURL( const String& _rURL )
    {
        implSetURL( _rURL, sal_True );
    }

    //-------------------------------------------------------------------------
    String OConnectionTabPage::getURLNoPrefix( ) const
    {
        return implGetURL( sal_False );
    }

    //-------------------------------------------------------------------------
    void OConnectionTabPage::setURLNoPrefix( const String& _rURL )
    {
        implSetURL( _rURL, sal_False );
    }

    //-------------------------------------------------------------------------
    String OConnectionTabPage::getConnectionURL( ) const
    {
        return getURL( );
    }

    //-------------------------------------------------------------------------
    void OConnectionTabPage::changeConnectionURL( const String& _rNewDSN )
    {
        setURL( _rNewDSN );
    }
    //-------------------------------------------------------------------------
    sal_Int32 OConnectionTabPage::checkPathExistence(const String& _rURL)
    {
        // #106016# ----------------
        if ( pathExists(_rURL, sal_False) == PATH_NOT_EXIST )
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
                                return RET_RETRY;
                        }
                    }
                    while (bTryCreate);
                }
                break;

                case RET_NO:
                    return RET_OK;

                default:
                    // cancelled
                    return RET_CANCEL;
            }
        }
        return RET_OK;
    }
    //-------------------------------------------------------------------------
    StringBag OConnectionTabPage::getInstalledAdabasDBDirs(const String& _rPath,const ::ucb::ResultSetInclude& _reResultSetInclude)
    {
        INetURLObject aNormalizer;
        aNormalizer.SetSmartProtocol(INET_PROT_FILE);
        aNormalizer.SetSmartURL(_rPath);
        String sAdabasConfigDir = aNormalizer.GetMainURL(INetURLObject::NO_DECODE);

        ::ucb::Content aAdabasConfigDir;
        try
        {
            aAdabasConfigDir = ::ucb::Content(sAdabasConfigDir, Reference< ::com::sun::star::ucb::XCommandEnvironment >());
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
                DBG_ERROR("OConnectionTabPage::getInstalledAdabasDBDirs: could not enumerate the adabas config files!");
            }
        }


        return aInstalledDBs;
    }
    // -----------------------------------------------------------------------------
    StringBag OConnectionTabPage::getInstalledAdabasDBs(const String &_rConfigDir,const String &_rWorkDir)
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
        aConfigDBs  = getInstalledAdabasDBDirs(sAdabasConfigDir,::ucb::INCLUDE_DOCUMENTS_ONLY);
        aWrkDBs     = getInstalledAdabasDBDirs(sAdabasWorkDir,::ucb::INCLUDE_FOLDERS_ONLY);
        ConstStringBagIterator aOuter = aConfigDBs.begin();
        for(;aOuter != aConfigDBs.end();++aOuter)
        {
            ConstStringBagIterator aInner = aWrkDBs.begin();
            for (;aInner != aWrkDBs.end(); ++aInner)
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
    IS_PATH_EXIST OConnectionTabPage::pathExists(const ::rtl::OUString& _rURL, sal_Bool bIsFile) const
    {
        ::ucb::Content aCheckExistence;
        sal_Bool bExists = sal_False;
        IS_PATH_EXIST eExists = PATH_NOT_EXIST;
        Reference< ::com::sun::star::task::XInteractionHandler > xInteractionHandler = Reference< ::com::sun::star::task::XInteractionHandler >(
            m_xORB->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.task.InteractionHandler") ) ), UNO_QUERY );
        OFilePickerInteractionHandler* pHandler = new OFilePickerInteractionHandler(xInteractionHandler);
        xInteractionHandler = pHandler;

        Reference< XCommandEnvironment > xCmdEnv = new ::ucb::CommandEnvironment( xInteractionHandler, Reference< XProgressHandler >() );
        try
        {
            aCheckExistence = ::ucb::Content(_rURL, xCmdEnv );
            bExists = bIsFile? aCheckExistence.isDocument(): aCheckExistence.isFolder();
            eExists = bExists? PATH_EXIST: PATH_NOT_EXIST;
        }
        catch(const Exception&)
        {
            eExists = ( pHandler && pHandler->isDoesNotExist() ) ? PATH_NOT_EXIST: PATH_NOT_KNOWN;
           }
        return eExists;
    }
    //-------------------------------------------------------------------------
    long OConnectionTabPage::PreNotify( NotifyEvent& _rNEvt )
    {
        if (    (DST_DBASE == m_eType)
            ||  (DST_FLAT == m_eType)
            ||  (DST_CALC == m_eType) )
            switch (_rNEvt.GetType())
            {
                case EVENT_GETFOCUS:
                    if (m_aConnection.IsWindowOrChild(_rNEvt.GetWindow()) && m_bUserGrabFocus)
                    {   // a descendant of the URL edit field got the focus
                        m_aConnection.SaveValueNoPrefix();
                    }
                    break;

                case EVENT_LOSEFOCUS:
                    if (m_aConnection.IsWindowOrChild(_rNEvt.GetWindow()) && m_bUserGrabFocus)
                    {   // a descendant of the URL edit field lost the focus
                        if (!commitURL())
                            return 1L;  // handled
                    }
                    break;
            }

        return OGenericAdministrationPage::PreNotify( _rNEvt );
    }
    //-------------------------------------------------------------------------
    sal_Bool OConnectionTabPage::createDirectoryDeep(const String& _rPathURL)
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
            ::ucb::Content aParent(aParser.GetMainURL(INetURLObject::NO_DECODE), xEmptyEnv);

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
        catch (const Exception& e)
        {
            OSL_ENSURE( sal_False, "" );
            e; // make compiler happy
            return sal_False;
        }

        return sal_True;
    }
    //-------------------------------------------------------------------------
    sal_Bool OConnectionTabPage::commitURL()
    {
        if (    (DST_DBASE == m_eType)
            ||  (DST_FLAT == m_eType)
            ||  (DST_CALC == m_eType) )
        {
            String sOldPath = m_aConnection.GetSavedValueNoPrefix();
            String sURL = m_aConnection.GetTextNoPrefix();
            if ((sURL != sOldPath) && (0 != sURL.Len()))
            {   // the text changed since entering the control

                // the path may be in system notation ....
                OFileNotation aTransformer(sURL);
                sURL = aTransformer.get(OFileNotation::N_URL);

                if ( DST_CALC == m_eType )
                { // #106016# --------------------------
                    if( pathExists(sURL, sal_True) == PATH_NOT_EXIST )
                    {
                        String sFile = String(ModuleRes(STR_CALCDOC_DOESNOTEXIST));
                        sFile.SearchAndReplaceAscii("$file$", aTransformer.get(OFileNotation::N_SYSTEM));
                        OSQLMessageBox(this,String(ModuleRes(STR_STAT_WARNING)),sFile).Execute();
                        setURLNoPrefix(sOldPath);
                        return sal_False;
                    }
                    else
                    {
                        setURLNoPrefix(sURL);
                        m_aConnection.SaveValueNoPrefix();
                    }
                }
                else
                {
                    switch (checkPathExistence(sURL))
                    {
                        case RET_RETRY:
                            m_bUserGrabFocus = sal_False;
                            m_aConnection.GrabFocus();
                            m_bUserGrabFocus = sal_True;
                            return sal_False;

                        case RET_CANCEL:
                            setURLNoPrefix(sOldPath);
                            return sal_False;

                        default:
                            // accept the input
                            setURLNoPrefix(sURL);
                            m_aConnection.SaveValueNoPrefix();
                            break;
                    }
                }
            }
        }

        return sal_True;
    }
    //-------------------------------------------------------------------------
    void OConnectionTabPage::askForFileName(const ::rtl::OUString& _sFilterName, const ::rtl::OUString& _sExtension)
    {
        ::sfx2::FileDialogHelper aFileDlg(WB_3DLOOK | WB_STDMODAL | WB_OPEN);

        aFileDlg.AddFilter(_sFilterName,_sExtension);
        aFileDlg.SetCurrentFilter(_sFilterName);

        String sOldPath = getURLNoPrefix();
        if ( sOldPath.Len() )
            aFileDlg.SetDisplayDirectory(sOldPath);
        else
            aFileDlg.SetDisplayDirectory( SvtPathOptions().GetWorkPath() );
        if (0 == aFileDlg.Execute())
        {
            setURLNoPrefix(aFileDlg.GetPath());
            callModifiedHdl();
        }
    }
//.........................................................................
}   // namespace dbaui
//.........................................................................
