/*************************************************************************
 *
 *  $RCSfile: dbadmin.cxx,v $
 *
 *  $Revision: 1.65 $
 *
 *  last change: $Author: oj $ $Date: 2001-07-17 07:30:50 $
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
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBAUI_DBADMIN_HXX_
#include "dbadmin.hxx"
#endif
#ifndef _DBAUI_DBADMIN_HRC_
#include "dbadmin.hrc"
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBAUI_DATASOURCEITEMS_HXX_
#include "dsitems.hxx"
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
#ifndef _VCL_STDTEXT_HXX
#include <vcl/stdtext.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVTOOLS_LOGINDLG_HXX_
#include <svtools/logindlg.hxx>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XNAMINGSERVICE_HPP_
#include <com/sun/star/uno/XNamingService.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XFLUSHABLE_HPP_
#include <com/sun/star/util/XFlushable.hpp>
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _DBAUI_ADMINPAGES_HXX_
#include "adminpages.hxx"
#endif
#ifndef _DBAUI_DETAILPAGES_HXX_
#include "detailpages.hxx"
#endif
#ifndef _DBAUI_COMMONPAGES_HXX_
#include "commonpages.hxx"
#endif
#ifndef _DBAUI_TABLESPAGE_HXX_
#include "tablespage.hxx"
#endif
#ifndef _DBAUI_GENERALPAGE_HXX_
#include "generalpage.hxx"
#endif
#ifndef _DBAUI_LOCALRESACCESS_HXX_
#include "localresaccess.hxx"
#endif
#ifndef _DBAUI_STRINGLISTITEM_HXX_
#include "stringlistitem.hxx"
#endif
#ifndef _TYPELIB_TYPEDESCRIPTION_HXX_
#include <typelib/typedescription.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _DBAUI_PROPERTYSETITEM_HXX_
#include "propertysetitem.hxx"
#endif
#ifndef DBAUI_ADABASPAGE_HXX
#include "AdabasPage.hxx"
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDRIVERACCESS_HPP_
#include <com/sun/star/sdbc/XDriverAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDRIVER_HPP_
#include <com/sun/star/sdbc/XDriver.hpp>
#endif
#ifndef DBAUI_USERADMIN_HXX
#include "UserAdmin.hxx"
#endif
//.........................................................................
namespace dbaui
{
//.........................................................................
using namespace dbtools;
using namespace com::sun::star::uno;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdb;
using namespace com::sun::star::lang;
using namespace com::sun::star::util;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;

//=========================================================================
//= ODbAdminDialog
//=========================================================================
//-------------------------------------------------------------------------
ODbAdminDialog::ODbAdminDialog(Window* _pParent, SfxItemSet* _pItems, const Reference< XMultiServiceFactory >& _rxORB)
    :SfxTabDialog(_pParent, ModuleRes(DLG_DATABASE_ADMINISTRATION), _pItems)
    ,m_aSelector(this, ResId(WND_DATASOURCESELECTOR))
    ,m_bResetting(sal_False)
    ,m_aDatasources(_rxORB)
    ,m_xORB(_rxORB)
    ,m_nPostApplyPage(0)
    ,m_pPostApplyPageSettings(NULL)
{
    // add the initial tab pages
    AddTabPage(PAGE_GENERAL, String(ResId(STR_PAGETITLE_GENERAL)), OGeneralPage::Create, NULL);
    AddTabPage(PAGE_TABLESUBSCRIPTION, String(ResId(STR_PAGETITLE_TABLESUBSCRIPTION)), OTableSubscriptionPage::Create, NULL);
    AddTabPage(PAGE_QUERYADMINISTRATION, String(ResId(STR_PAGETITLE_QUERIES)), OQueryAdministrationPage::Create, NULL);
    AddTabPage(PAGE_DOCUMENTLINKS, String(ResId(STR_PAGETITLE_DOCUMENTS)), ODocumentLinksPage::Create, NULL);
    // no local resources needed anymore
    FreeResource();

    /// initialize the property translation map
    // direct properties of a data source
    m_aDirectPropTranslator.insert(MapInt2String::value_type(DSID_CONNECTURL, PROPERTY_URL));
    m_aDirectPropTranslator.insert(MapInt2String::value_type(DSID_NAME, PROPERTY_NAME));
    m_aDirectPropTranslator.insert(MapInt2String::value_type(DSID_USER, PROPERTY_USER));
    m_aDirectPropTranslator.insert(MapInt2String::value_type(DSID_PASSWORD, PROPERTY_PASSWORD));
    m_aDirectPropTranslator.insert(MapInt2String::value_type(DSID_PASSWORDREQUIRED, PROPERTY_ISPASSWORDREQUIRED));
    m_aDirectPropTranslator.insert(MapInt2String::value_type(DSID_TABLEFILTER, PROPERTY_TABLEFILTER));
    m_aDirectPropTranslator.insert(MapInt2String::value_type(DSID_READONLY, PROPERTY_ISREADONLY));
    m_aDirectPropTranslator.insert(MapInt2String::value_type(DSID_SUPPRESSVERSIONCL, PROPERTY_SUPPRESSVERSIONCL));

    // implicit properties, to be found in the direct property "Info"
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_JDBCDRIVERCLASS, ::rtl::OUString::createFromAscii("JavaDriverClass")));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_TEXTFILEEXTENSION, ::rtl::OUString::createFromAscii("Extension")));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_CHARSET, ::rtl::OUString::createFromAscii("CharSet")));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_TEXTFILEHEADER, ::rtl::OUString::createFromAscii("HeaderLine")));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_FIELDDELIMITER, ::rtl::OUString::createFromAscii("FieldDelimiter")));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_TEXTDELIMITER, ::rtl::OUString::createFromAscii("StringDelimiter")));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_DECIMALDELIMITER, ::rtl::OUString::createFromAscii("DecimalDelimiter")));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_THOUSANDSDELIMITER, ::rtl::OUString::createFromAscii("ThousandDelimiter")));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_SHOWDELETEDROWS, ::rtl::OUString::createFromAscii("ShowDeleted")));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_ALLOWLONGTABLENAMES, ::rtl::OUString::createFromAscii("NoNameLengthLimit")));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_ADDITIONALOPTIONS, ::rtl::OUString::createFromAscii("SystemDriverSettings")));

    // special settings for adabas
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_CONN_SHUTSERVICE, ::rtl::OUString::createFromAscii("ShutdownDatabase")));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_CONN_DATAINC, ::rtl::OUString::createFromAscii("DataCacheSizeIncrement")));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_CONN_CACHESIZE, ::rtl::OUString::createFromAscii("DataCacheSize")));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_CONN_CTRLUSER, ::rtl::OUString::createFromAscii("ControlUser")));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_CONN_CTRLPWD, ::rtl::OUString::createFromAscii("ControlPassword")));
    // extra settings for odbc
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_USECATALOG, ::rtl::OUString::createFromAscii("UseCatalog")));
    // extra settings for a ldap address book
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_CONN_LDAP_HOSTNAME, ::rtl::OUString::createFromAscii("HostName")));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_CONN_LDAP_BASEDN, ::rtl::OUString::createFromAscii("BaseDN")));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_CONN_LDAP_PORTNUMBER, ::rtl::OUString::createFromAscii("PortNumber")));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_CONN_LDAP_ROWCOUNT, ::rtl::OUString::createFromAscii("MaxRowCount")));

    // remove the reset button - it's meaning is much too ambiguous in this dialog
    RemoveResetButton();

    // enable an apply button
    EnableApplyButton(sal_True);
    SetApplyHandler(LINK(this, ODbAdminDialog, OnApplyChanges));
    // disable the apply button
    GetApplyButton()->Enable(sal_False);

    // register the view window
    SetViewWindow(&m_aSelector);
    SetViewAlign(WINDOWALIGN_LEFT);
    AdjustLayout();

    // do some knittings
    m_aSelector.setSelectHandler(LINK(this, ODbAdminDialog, OnDatasourceSelected));
    m_aSelector.setNewHandler(LINK(this, ODbAdminDialog, OnNewDatasource));
    m_aSelector.setDeleteHandler(LINK(this, ODbAdminDialog, OnDeleteDatasource));
    m_aSelector.setRestoreHandler(LINK(this, ODbAdminDialog, OnRestoreDatasource));

    ::rtl::OUString sInitialSelection;  // will be the initial selection

    if (!m_aDatasources.isValid())
    {
        ShowServiceNotAvailableError(_pParent, String(SERVICE_SDB_DATABASECONTEXT), sal_True);
        m_aSelector.Disable();
    }
    else
    {
        m_xDatabaseContext = m_aDatasources.getContext();
        m_xDynamicContext = Reference< XNamingService >(m_xDatabaseContext, UNO_QUERY);
        DBG_ASSERT(m_xDynamicContext.is(), "ODbAdminDialog::ODbAdminDialog : no XNamingService interface !");

        // fill the listbox with the names of the registered datasources
        ODatasourceMap::Iterator aDatasourceLoop = m_aDatasources.begin();
        while (aDatasourceLoop != m_aDatasources.end())
        {
            m_aSelector.insert(*aDatasourceLoop);
            m_aValidDatasources.insert(*aDatasourceLoop);

            ++aDatasourceLoop;
        }

        if (!m_aDatasources.size())
        {
            WarningBox(_pParent, ModuleRes(ERR_NOREGISTEREDDATASOURCES)).Execute();
        }
        else
            sInitialSelection = *m_aDatasources.begin();
    }

    implSelectDatasource(sInitialSelection);

    GetApplyButton()->Enable(sal_False);
        // nothing modified 'til now -> now apply
}

//-------------------------------------------------------------------------
ODbAdminDialog::~ODbAdminDialog()
{
    SetInputSet(NULL);
    DELETEZ(pExampleSet);
}
// -----------------------------------------------------------------------------
String ODbAdminDialog::getConnectionURL() const
{
    SFX_ITEMSET_GET(*GetExampleSet(), pUrlItem, SfxStringItem, DSID_CONNECTURL, sal_True);
    return pUrlItem->GetValue();
}
// -----------------------------------------------------------------------------
Reference< XDriver > ODbAdminDialog::getDriver()
{
    // get the global DriverManager
    Reference< XDriverAccess > xDriverManager;
    String sCurrentActionError = String(ModuleRes(STR_COULDNOTCREATE_DRIVERMANAGER));
        // in case an error occures
    sCurrentActionError.SearchAndReplaceAscii("#servicename#", (::rtl::OUString)SERVICE_SDBC_CONNECTIONPOOL);
    try
    {
        xDriverManager = Reference< XDriverAccess >(getORB()->createInstance(SERVICE_SDBC_CONNECTIONPOOL), UNO_QUERY);
        DBG_ASSERT(xDriverManager.is(), "ODbAdminDialog::getDriver: could not instantiate the driver manager, or it does not provide the necessary interface!");
    }
    catch (Exception& e)
    {
        // wrap the exception into an SQLException
        SQLException aSQLWrapper(e.Message, getORB(), ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("S1000")), 0, Any());
        throw SQLException(sCurrentActionError, getORB(), ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("S1000")), 0, makeAny(aSQLWrapper));
    }
    if (!xDriverManager.is())
        throw SQLException(sCurrentActionError, getORB(), ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("S1000")), 0, Any());


    sCurrentActionError = String(ModuleRes(STR_NOREGISTEREDDRIVER));

    Reference< XDriver > xDriver = xDriverManager->getDriverByURL(getConnectionURL());
    if (!xDriver.is())
        // will be caught and translated into an SQLContext exception
        throw SQLException(sCurrentActionError, getORB(), ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("S1000")), 0, Any());
    return xDriver;
}
// -----------------------------------------------------------------------------
Reference<XConnection> ODbAdminDialog::createConnection()
{
    Reference<XConnection> xConnection;
//  //  if (bValid)
    {   // get the current table list from the connection for the current settings
        // the PropertyValues for the current dialog settings
        Sequence< PropertyValue > aConnectionParams;
        if (getCurrentSettings(aConnectionParams))
        {
            // the current DSN
            // fill the table list with this connection information
            SQLExceptionInfo aErrorInfo;
            try
            {
                WaitObject aWaitCursor(this);
                xConnection = getDriver()->connect(getConnectionURL(), aConnectionParams);
            }
            catch (::com::sun::star::sdb::SQLContext& e) { aErrorInfo = SQLExceptionInfo(e); }
            catch (::com::sun::star::sdbc::SQLWarning& e) { aErrorInfo = SQLExceptionInfo(e); }
            catch (::com::sun::star::sdbc::SQLException& e) { aErrorInfo = SQLExceptionInfo(e); }

            showError(aErrorInfo,this,getORB());
        }
    }
    if(xConnection.is())
        successfullyConnected();// notify the admindlg to save the password

    return xConnection;
}
//-------------------------------------------------------------------------
short ODbAdminDialog::Execute()
{
    short nResult = SfxTabDialog::Execute();

    // within it's dtor, the SfxTabDialog saves (amongst others) the currently selected tab page and
    // reads it upon the next Execute (dependent on the resource id, which thus has to be globally unique,
    // though our's isn't)
    // As this is not wanted if e.g. the table subscription page is selected, we show the GeneralPage here
    ShowPage(PAGE_GENERAL);

    // clear the temporary SfxItemSets we created
    m_aDatasources.clear();

    return nResult;
}

//-------------------------------------------------------------------------
void ODbAdminDialog::selectDataSource(const ::rtl::OUString& _rName)
{
    if (m_aDatasources.exists(_rName))
        implSelectDatasource(_rName);
}

//-------------------------------------------------------------------------
void ODbAdminDialog::clearPassword()
{
    if (pExampleSet)
        pExampleSet->ClearItem(DSID_PASSWORD);
}

//-------------------------------------------------------------------------
void ODbAdminDialog::successfullyConnected()
{
    DBG_ASSERT(GetExampleSet(), "ODbAdminDialog::successfullyConnected: not to be called without an example set!");
    if (!GetExampleSet())
        return;

    if (hasAuthentication(*GetExampleSet()))
    {
        SFX_ITEMSET_GET(*GetExampleSet(), pPassword, SfxStringItem, DSID_PASSWORD, sal_True);
        if (pPassword && (0 != pPassword->GetValue().Len()))
        {
            ::rtl::OUString sPassword = pPassword->GetValue();

            ODatasourceMap::ODatasourceInfo aDatasourceInfo = m_aDatasources[m_sCurrentDatasource];
            Reference< XPropertySet > xCurrentDatasource = aDatasourceInfo.getDatasource();
            DBG_ASSERT(xCurrentDatasource.is(), "ODbAdminDialog::successfullyConnected: no data source!");
            if (xCurrentDatasource.is())
            {
                try
                {
                    xCurrentDatasource->setPropertyValue(m_aDirectPropTranslator[DSID_PASSWORD], makeAny(sPassword));
                }
                catch(const Exception&)
                {
                    DBG_ERROR("ODbAdminDialog::successfullyConnected: caught an exception!");
                }
            }
        }
    }
}

//-------------------------------------------------------------------------
sal_Bool ODbAdminDialog::getCurrentSettings(Sequence< PropertyValue >& _rDriverParam)
{
    DBG_ASSERT(GetExampleSet(), "ODbAdminDialog::getCurrentSettings : not to be called without an example set!");
    if (!GetExampleSet())
        return sal_False;

    ::std::vector< PropertyValue > aReturn;
        // collecting this in a vector because it has a push_back, in opposite to sequences

    // user: DSID_USER -> "user"
    SFX_ITEMSET_GET(*GetExampleSet(), pUser, SfxStringItem, DSID_USER, sal_True);
    if (pUser && pUser->GetValue().Len())
        aReturn.push_back(
            PropertyValue(  ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("user")), 0,
                            makeAny(::rtl::OUString(pUser->GetValue())), PropertyState_DIRECT_VALUE));

    // check if the connection type requires a password
    if (hasAuthentication(*GetExampleSet()))
    {
        // password: DSID_PASSWORD -> "password"
        SFX_ITEMSET_GET(*GetExampleSet(), pPassword, SfxStringItem, DSID_PASSWORD, sal_True);
        String sPassword = pPassword ? pPassword->GetValue() : String();
        SFX_ITEMSET_GET(*GetExampleSet(), pPasswordRequired, SfxBoolItem, DSID_PASSWORDREQUIRED, sal_True);
        // if the set does not contain a password, but the item set says it requires one, ask the user
        if ((!pPassword || !pPassword->GetValue().Len()) && (pPasswordRequired && pPasswordRequired->GetValue()))
        {
            SFX_ITEMSET_GET(*GetExampleSet(), pName, SfxStringItem, DSID_NAME, sal_True);

            ::svt::LoginDialog aDlg(this,
                LF_NO_PATH | LF_NO_ACCOUNT | LF_NO_ERRORTEXT | LF_USERNAME_READONLY,
                String(), NULL);

            aDlg.SetName(pUser ? pUser->GetValue() : String());
            aDlg.ClearPassword();  // this will give the password field the focus

            String sLoginRequest(ModuleRes(STR_ENTER_CONNECTION_PASSWORD));
            sLoginRequest.SearchAndReplaceAscii("$name$", pName ? pName->GetValue() : String()),
            aDlg.SetLoginRequestText(sLoginRequest);
            aDlg.SetSavePasswordText(ModuleRes(STR_REMEMBERPASSWORD_SESSION));
            aDlg.SetSavePassword(sal_True);

            sal_Int32 nResult = aDlg.Execute();
            if (nResult != RET_OK)
                return sal_False;

            sPassword = aDlg.GetPassword();
            if (aDlg.IsSavePassword())
                pExampleSet->Put(SfxStringItem(DSID_PASSWORD, sPassword));
        }

        if (sPassword.Len())
            aReturn.push_back(
                PropertyValue(  ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("password")), 0,
                                makeAny(::rtl::OUString(sPassword)), PropertyState_DIRECT_VALUE));
    }

    _rDriverParam = Sequence< PropertyValue >(aReturn.begin(), aReturn.size());

    // append all the other stuff (charset etc.)
    fillDatasourceInfo(*GetExampleSet(), _rDriverParam);

    return sal_True;
}

//-------------------------------------------------------------------------
sal_Bool ODbAdminDialog::isCurrentModified() const
{
    if (0 == m_aSelector.count())
        return sal_False;

    String sCurrentlySelected = m_aSelector.getSelected();
    return const_cast<ODbAdminDialog*>(this)->m_aDatasources[sCurrentlySelected].isModified();
}

//-------------------------------------------------------------------------
sal_Bool ODbAdminDialog::isApplyable() const
{
    return GetApplyButton()->IsEnabled();
}

//-------------------------------------------------------------------------
void ODbAdminDialog::applyChangesAsync(const OPageSettings* _pUseTheseSettings)
{
    DBG_ASSERT(isApplyable(), "ODbAdminDialog::applyChangesAsync: invalid call!");
    DBG_ASSERT((0 == m_nPostApplyPage) && !m_pPostApplyPageSettings, "ODbAdminDialog::applyChangesAsync: already doing this!");

    sal_uInt16 nCurrentPageId = GetCurPageId();

    // get the view settings
    if (!_pUseTheseSettings)
    {
        OGenericAdministrationPage* pCurrentPage = static_cast<OGenericAdministrationPage*>(GetTabPage(nCurrentPageId));
        OPageSettings* pViewSettings = NULL;
        if (pCurrentPage)
        {   // get the pages current view settings
            pViewSettings = pCurrentPage->createViewSettings();
            pCurrentPage->fillViewSettings(pViewSettings);
        }
        m_pPostApplyPageSettings = pViewSettings;
    }
    else
        m_pPostApplyPageSettings = _pUseTheseSettings;

    // remember the page id
    m_nPostApplyPage = nCurrentPageId;

    PostUserEvent(LINK(this, ODbAdminDialog, OnAsyncApplyChanges));
}

//-------------------------------------------------------------------------
short ODbAdminDialog::Ok()
{
    short nResult = SfxTabDialog::Ok();
    return (AR_LEAVE_MODIFIED == implApplyChanges(sal_False)) ? RET_OK : RET_CANCEL;
        // TODO : AR_ERROR is not handled correctly, we always close the dialog here
}

//-------------------------------------------------------------------------
void ODbAdminDialog::PageCreated(USHORT _nId, SfxTabPage& _rPage)
{
    // register ourself as modified listener
    static_cast<OGenericAdministrationPage&>(_rPage).SetModifiedHandler(LINK(this, ODbAdminDialog, OnDatasourceModifed));

    // some registrations which depend on the type of the page
    switch (_nId)
    {
        case PAGE_GENERAL:
            static_cast<OGeneralPage&>(_rPage).SetTypeSelectHandler(LINK(this, ODbAdminDialog, OnTypeSelected));
            static_cast<OGeneralPage&>(_rPage).SetNameModifyHandler(LINK(this, ODbAdminDialog, OnNameModified));
            static_cast<OGeneralPage&>(_rPage).setServiceFactory(m_xORB);
            static_cast<OGeneralPage&>(_rPage).SetAdminDialog(this);
            break;
        case PAGE_TABLESUBSCRIPTION:
            static_cast<OTableSubscriptionPage&>(_rPage).setServiceFactory(m_xORB);
            static_cast<OTableSubscriptionPage&>(_rPage).SetAdminDialog(this);
            break;
        case PAGE_DOCUMENTLINKS:
        case PAGE_QUERYADMINISTRATION:
            static_cast<OCollectionPage&>(_rPage).setServiceFactory(m_xORB);
            static_cast<OCollectionPage&>(_rPage).SetAdminDialog(this);
            break;
        case TAB_PAG_ADABAS_SETTINGS:
            static_cast<OAdabasAdminSettings&>(_rPage).SetAdminDialog(this);
            break;
        case TAB_PAGE_USERADMIN:
            static_cast<OUserAdmin&>(_rPage).setServiceFactory(m_xORB);
            static_cast<OUserAdmin&>(_rPage).SetAdminDialog(this);
            break;
    }

    AdjustLayout();
    Window *pWin = GetViewWindow();
    if(pWin)
        pWin->Invalidate();

    SfxTabDialog::PageCreated(_nId, _rPage);
}

//-------------------------------------------------------------------------
SfxItemSet* ODbAdminDialog::createItemSet(SfxItemSet*& _rpSet, SfxItemPool*& _rpPool, SfxPoolItem**& _rppDefaults, ODsnTypeCollection* _pTypeCollection)
{
    // just to be sure ....
    _rpSet = NULL;
    _rpPool = NULL;
    _rppDefaults = NULL;

    // create and initialize the defaults
    _rppDefaults = new SfxPoolItem*[DSID_LAST_ITEM_ID - DSID_FIRST_ITEM_ID + 1];
    SfxPoolItem** pCounter = _rppDefaults;  // want to modify this without affecting the out param _rppDefaults
    *pCounter++ = new SfxStringItem(DSID_NAME, String());
    *pCounter++ = new SfxStringItem(DSID_ORIGINALNAME, String());
    *pCounter++ = new SfxStringItem(DSID_CONNECTURL, _pTypeCollection ? _pTypeCollection->getDatasourcePrefix(DST_JDBC) : String());
    *pCounter++ = new OStringListItem(DSID_TABLEFILTER, Sequence< ::rtl::OUString >(&::rtl::OUString("%", 1, RTL_TEXTENCODING_ASCII_US), 1));
    *pCounter++ = new DbuTypeCollectionItem(DSID_TYPECOLLECTION, _pTypeCollection);
    *pCounter++ = new SfxBoolItem(DSID_INVALID_SELECTION, sal_False);
    *pCounter++ = new SfxBoolItem(DSID_READONLY, sal_False);
    *pCounter++ = new SfxStringItem(DSID_USER, String());
    *pCounter++ = new SfxStringItem(DSID_PASSWORD, String());
    *pCounter++ = new SfxStringItem(DSID_ADDITIONALOPTIONS, String());
    *pCounter++ = new SfxStringItem(DSID_CHARSET, String());
    *pCounter++ = new SfxBoolItem(DSID_PASSWORDREQUIRED, sal_False);
    *pCounter++ = new SfxBoolItem(DSID_SHOWDELETEDROWS, sal_False);
    *pCounter++ = new SfxBoolItem(DSID_ALLOWLONGTABLENAMES, sal_False);
    *pCounter++ = new SfxStringItem(DSID_JDBCDRIVERCLASS, String());
    *pCounter++ = new SfxStringItem(DSID_FIELDDELIMITER, ';');
    *pCounter++ = new SfxStringItem(DSID_TEXTDELIMITER, '"');
    *pCounter++ = new SfxStringItem(DSID_DECIMALDELIMITER, '.');
    *pCounter++ = new SfxStringItem(DSID_THOUSANDSDELIMITER, ',');
    *pCounter++ = new SfxStringItem(DSID_TEXTFILEEXTENSION, String::CreateFromAscii("txt"));
    *pCounter++ = new SfxBoolItem(DSID_TEXTFILEHEADER, sal_True);
    *pCounter++ = new SfxBoolItem(DSID_NEWDATASOURCE, sal_False);
    *pCounter++ = new SfxBoolItem(DSID_DELETEDDATASOURCE, sal_False);
    *pCounter++ = new SfxBoolItem(DSID_SUPPRESSVERSIONCL, sal_True);
    *pCounter++ = new OPropertySetItem(DSID_DATASOURCE_UNO);
    *pCounter++ = new SfxBoolItem(DSID_CONN_SHUTSERVICE, sal_False);
    *pCounter++ = new SfxInt32Item(DSID_CONN_DATAINC, 20);
    *pCounter++ = new SfxInt32Item(DSID_CONN_CACHESIZE, 20);
    *pCounter++ = new SfxStringItem(DSID_CONN_CTRLUSER, String());
    *pCounter++ = new SfxStringItem(DSID_CONN_CTRLPWD, String());
    *pCounter++ = new SfxBoolItem(DSID_USECATALOG, sal_False);
    *pCounter++ = new SfxStringItem(DSID_CONN_LDAP_HOSTNAME, String());
    *pCounter++ = new SfxStringItem(DSID_CONN_LDAP_BASEDN, String());
    *pCounter++ = new SfxInt32Item(DSID_CONN_LDAP_PORTNUMBER, 389);
    *pCounter++ = new SfxInt32Item(DSID_CONN_LDAP_ROWCOUNT, 100);



    // create the pool
    static SfxItemInfo __READONLY_DATA aItemInfos[DSID_LAST_ITEM_ID - DSID_FIRST_ITEM_ID + 1] =
    {
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
    };

    _rpPool = new SfxItemPool(String::CreateFromAscii("DSAItemPool"), DSID_FIRST_ITEM_ID, DSID_LAST_ITEM_ID,
        aItemInfos, _rppDefaults);
    _rpPool->FreezeIdRanges();

    // and, finally, the set
    _rpSet = new SfxItemSet(*_rpPool, sal_True);

    return _rpSet;
}

//-------------------------------------------------------------------------
void ODbAdminDialog::destroyItemSet(SfxItemSet*& _rpSet, SfxItemPool*& _rpPool, SfxPoolItem**& _rppDefaults)
{
    // _first_ delete the set (refering the pool)
    if (_rpSet)
    {
        delete _rpSet;
        _rpSet = NULL;
    }

    // delete the pool
    if (_rpPool)
    {
        _rpPool->ReleaseDefaults(sal_True);
            // the "true" means delete the items, too
        delete _rpPool;
        _rpPool = NULL;
    }

    // reset the defaults ptr
    _rppDefaults = NULL;
        // no need to explicitly delete the defaults, this has been done by the ReleaseDefaults
}

//-------------------------------------------------------------------------
IMPL_LINK(ODbAdminDialog, OnDatasourceSelected, ListBox*, _pBox)
{
    if (!prepareSwitchDatasource())
    {   // restore the old selection
        if (m_sCurrentDatasource.getLength())
            m_aSelector.select(m_sCurrentDatasource);
        else
            m_aSelector.select(m_nCurrentDeletedDataSource);
    }

    // switch the content of the pages
    if (DELETED == m_aSelector.getSelectedState())
        implSelectDeleted(m_aSelector.getSelectedAccessKey());
    else
        implSelectDatasource(m_aSelector.getSelected());

    return 0L;
}

//-------------------------------------------------------------------------
IMPL_LINK(ODbAdminDialog, OnDatasourceModifed, SfxTabPage*, _pTabPage)
{
    // check if the currently selected entry is already marked as modified
    String sCurrentlySelected = m_aSelector.getSelected();
    if (m_aDatasources[sCurrentlySelected].isModified())
        // yes -> nothing to do
        return 0L;

    // no -> mark the item as modified
    m_aSelector.modified(sCurrentlySelected);
    m_aDatasources.update(sCurrentlySelected, *pExampleSet);

    // enable the apply button
    GetApplyButton()->Enable(sal_True);

    return 0L;
}

//-------------------------------------------------------------------------
IMPL_LINK(ODbAdminDialog, OnNameModified, OGeneralPage*, _pTabPage)
{
    if (!m_bResetting)
    {
        ::rtl::OUString sNewStringSuggestion = _pTabPage->GetCurrentName();

        // check if there's already a data source with the suggested name
        ConstStringSetIterator aExistentPos = m_aValidDatasources.find(sNewStringSuggestion);
            // !! m_aValidDatasources contains _all_ data source names _except_ the currently selected one !!

        sal_Bool bValid = m_aValidDatasources.end() == aExistentPos;

        // the user is not allowed to leave the current data source (or to commit the dialog) as long
        // as the name is invalid
        m_aSelector.Enable(bValid && m_aDatasources.isValid());
        GetOKButton().Enable(bValid);
        GetApplyButton()->Enable(bValid);

        // if this is the first modification for this data source, we have to adjust the DS list accordingly
        String sSelected = m_aSelector.getSelected();
        if (!m_aDatasources[sSelected].isModified())
        {   // (we could do it all the time here, but as this link is called every time a single character
            // of the name changes, this maybe would be too expensive.)
            m_aSelector.modified(sSelected);
            m_aDatasources.update(sSelected, *pExampleSet);
        }

        // enable the apply button
        GetApplyButton()->Enable(sal_True && bValid);

        return bValid ? 1L : 0L;
    }
    return 1L;
}
// -----------------------------------------------------------------------------
void ODbAdminDialog::removeDetailPages()
{
    // remove all current detail pages
    while (m_aCurrentDetailPages.size())
    {
        RemoveTabPage((USHORT)m_aCurrentDetailPages.top());
        m_aCurrentDetailPages.pop();
    }
}

// -----------------------------------------------------------------------------
void ODbAdminDialog::addDetailPage(USHORT _nPageId, USHORT _nTextId, CreateTabPage _pCreateFunc)
{
    // open our own resource block, as the page titles are strings local to this block
    OLocalResourceAccess aDummy(DLG_DATABASE_ADMINISTRATION, RSC_TABDIALOG);

    AddTabPage(_nPageId, String(ResId(_nTextId)), _pCreateFunc, 0, sal_False, 1);
    m_aCurrentDetailPages.push(_nPageId);
}

//-------------------------------------------------------------------------
IMPL_LINK(ODbAdminDialog, OnTypeSelected, OGeneralPage*, _pTabPage)
{
    // doe have to reset the "password required" flag to false? (in case the datasource does not support passwords)
    sal_Bool bResetPasswordRequired = sal_False;
    _pTabPage->enableConnectionURL();

    // remove all current detail pages
    removeDetailPages();

    // and insert the new ones
    switch (_pTabPage->GetSelectedType())
    {
        case DST_DBASE:
            addDetailPage(PAGE_DBASE, STR_PAGETITLE_DBASE, ODbaseDetailsPage::Create);
            bResetPasswordRequired = sal_True;
            break;

        case DST_JDBC:
            addDetailPage(PAGE_JDBC, STR_PAGETITLE_JDBC, OJdbcDetailsPage::Create);
            break;

        case DST_ADO:
            addDetailPage(PAGE_ADO, STR_PAGETITLE_ADO, OAdoDetailsPage::Create);
            break;

        case DST_TEXT:
            addDetailPage(PAGE_TEXT, STR_PAGETITLE_TEXT, OTextDetailsPage::Create);
            bResetPasswordRequired = sal_True;
            break;

        case DST_ODBC:
            addDetailPage(PAGE_ODBC, STR_PAGETITLE_ODBC, OOdbcDetailsPage::Create);
            break;

        case DST_ADABAS:
            // for adabas we have more than one page
            // CAUTION: the order of inserting pages matters.
            // the major detail page should be inserted last always (thus, it becomes the first page after
            // the general page)
            addDetailPage(TAB_PAGE_USERADMIN, STR_PAGETITLE_USERADMIN, OUserAdmin::Create);
            addDetailPage(TAB_PAG_ADABAS_SETTINGS, STR_PAGETITLE_ADABAS_STATISTIC, OAdabasAdminSettings::Create);
            addDetailPage(PAGE_ADABAS, STR_PAGETITLE_ADABAS, OAdabasDetailsPage::Create);
            {
//              Size aOldSize = pTabControl->GetSizePixel();
//              pTabControl->SetTabPageSizePixel( ... );
//              Size aNewSize = pTabControl->GetSizePixel();
//              if ( aOldSize != aNewSize )
//                  pTabDialog->AdjustLayout();
            }
            break;

        case DST_ADDRESSBOOK:
            if(getDatasourceType(*GetExampleSet()) == DST_ADDRESSBOOK)
            {
                String sConnectionURL;
                SFX_ITEMSET_GET(*GetExampleSet(), pUrlItem, SfxStringItem, DSID_CONNECTURL, sal_True);
                sConnectionURL = pUrlItem->GetValue();
                if (0 == sConnectionURL.CompareToAscii("sdbc:address:ldap:"))
                    addDetailPage(PAGE_LDAP,STR_PAGETITLE_LDAP,OLDAPDetailsPage::Create);
            }
            _pTabPage->disableConnectionURL();
            break;
    }

    if (bResetPasswordRequired)
    {
        GetInputSetImpl()->Put(SfxBoolItem(DSID_PASSWORDREQUIRED, sal_False));
        if (pExampleSet)
            pExampleSet->Put(SfxBoolItem(DSID_PASSWORDREQUIRED, sal_False));
    }

    return 0L;
}

//-------------------------------------------------------------------------
Reference< XPropertySet > ODbAdminDialog::getDatasource(const ::rtl::OUString& _rName)
{
    DBG_ASSERT(m_aDatasources.isValid(), "ODbAdminDialog::getDatasource : have no database context!");
    if (!m_aDatasources.exists(_rName))
        return Reference< XPropertySet >();

    return m_aDatasources[_rName]->getDatasource();
}

//-------------------------------------------------------------------------
void ODbAdminDialog::implSelectDeleted(sal_Int32 _nKey)
{
    m_aSelector.select(_nKey);

    // insert the previously selected data source into our "all valid datasources" set
    if (m_sCurrentDatasource.getLength())   // previous selection was not on a deleted data source
        m_aValidDatasources.insert(m_sCurrentDatasource);
    m_sCurrentDatasource = ::rtl::OUString();
    m_nCurrentDeletedDataSource = _nKey;

    // reset the tag pages
    resetPages(Reference< XPropertySet >(), sal_True);

    // disallow reset for deleted pages
    //  GetResetButton().Enable(sal_False);
}

//-------------------------------------------------------------------------
void ODbAdminDialog::implSelectDatasource(const ::rtl::OUString& _rRegisteredName)
{
    m_aSelector.select(_rRegisteredName);

    // insert the previously selected data source into our set
    if (m_sCurrentDatasource.getLength())   // previous selection was not on a deleted data source
        m_aValidDatasources.insert(m_sCurrentDatasource);
    m_sCurrentDatasource = _rRegisteredName;
    m_nCurrentDeletedDataSource = -1;
    // remove the now selected data source from our set
    m_aValidDatasources.erase(m_sCurrentDatasource);

    // reset the tag pages
    Reference< XPropertySet > xDatasource = getDatasource(_rRegisteredName);
    resetPages(xDatasource, sal_False);

    // allow reset for non-deleted pages
    //  GetResetButton().Enable(sal_True);
}

//-------------------------------------------------------------------------
void ODbAdminDialog::resetPages(const Reference< XPropertySet >& _rxDatasource, sal_Bool _bDeleted)
{
    // the selection is valid if and only if we have a datasource now
    GetInputSetImpl()->Put(SfxBoolItem(DSID_INVALID_SELECTION, !_rxDatasource.is()));
        // (sal_False tells the tab pages to disable and reset all their controls, which is different
        // from "just set them to readonly")

    // reset the pages

    // prevent flicker
    SetUpdateMode(sal_False);

    m_bResetting = sal_True;
    ShowPage(PAGE_GENERAL);
    m_bResetting = sal_False;

    // remove all tab pages (except the general one)
    // remove all current detail pages
    while (m_aCurrentDetailPages.size())
    {
        RemoveTabPage((USHORT)m_aCurrentDetailPages.top());
        m_aCurrentDetailPages.pop();
    }
    // remove the table/query tab pages
    RemoveTabPage(PAGE_TABLESUBSCRIPTION);
    RemoveTabPage(PAGE_QUERYADMINISTRATION);
    RemoveTabPage(PAGE_DOCUMENTLINKS);

    // extract all relevant data from the property set of the data source
    translateProperties(_rxDatasource, *GetInputSetImpl());

    // reset some meta data items in the input set which are for tracking the state of the current ds
    GetInputSetImpl()->Put(SfxBoolItem(DSID_NEWDATASOURCE, sal_False));
    GetInputSetImpl()->Put(SfxBoolItem(DSID_DELETEDDATASOURCE, _bDeleted));
    GetInputSetImpl()->Put(OPropertySetItem(DSID_DATASOURCE_UNO, _rxDatasource));

    // fill in the remembered settings for the data source
    if (m_sCurrentDatasource.getLength())   // the current datasource is not deleted
        if (m_aDatasources[m_sCurrentDatasource]->isModified()) // the current data source was modified before
            GetInputSetImpl()->Put(*m_aDatasources[m_sCurrentDatasource]->getModifications());

    // propagate this set as our new input set and reset the example set
    SetInputSet(GetInputSetImpl());
    delete pExampleSet;
    pExampleSet = new SfxItemSet(*GetInputSetImpl());

    // and again, add the non-details tab pages
    if (!_bDeleted)
    {
        OLocalResourceAccess aDummy(DLG_DATABASE_ADMINISTRATION, RSC_TABDIALOG);
        AddTabPage(PAGE_TABLESUBSCRIPTION, String(ResId(STR_PAGETITLE_TABLESUBSCRIPTION)), OTableSubscriptionPage::Create, NULL);
        AddTabPage(PAGE_QUERYADMINISTRATION, String(ResId(STR_PAGETITLE_QUERIES)), OQueryAdministrationPage::Create, NULL);
        AddTabPage(PAGE_DOCUMENTLINKS, String(ResId(STR_PAGETITLE_DOCUMENTS)), ODocumentLinksPage::Create, NULL);
    }

    m_bResetting = sal_True;
    ShowPage(PAGE_GENERAL);
    SetUpdateMode(sal_True);

    // propagate the new data to the general tab page the general tab page
    SfxTabPage* pGeneralPage = GetTabPage(PAGE_GENERAL);
    if (pGeneralPage)
        pGeneralPage->Reset(*GetInputSetImpl());
    // if this is NULL, the page has not been created yet, which means we're called before the
    // dialog was displayed (probably from inside the ctor)
    m_bResetting = sal_False;
}

//-------------------------------------------------------------------------
Any ODbAdminDialog::implTranslateProperty(const SfxPoolItem* _pItem)
{
    // translate the SfxPoolItem
    Any aValue;
    if (_pItem->ISA(SfxStringItem))
        aValue <<= ::rtl::OUString(PTR_CAST(SfxStringItem, _pItem)->GetValue().GetBuffer());
    else if (_pItem->ISA(SfxBoolItem))
        aValue = ::cppu::bool2any(PTR_CAST(SfxBoolItem, _pItem)->GetValue());
    else if (_pItem->ISA(SfxInt32Item))
        aValue <<= PTR_CAST(SfxInt32Item, _pItem)->GetValue();
    else if (_pItem->ISA(OStringListItem))
        aValue <<= PTR_CAST(OStringListItem, _pItem)->getList();
    else
    {
        DBG_ERROR("ODbAdminDialog::implTranslateProperty: unsupported item type!");
        return aValue;
    }

    return aValue;
}

//-------------------------------------------------------------------------
void ODbAdminDialog::implTranslateProperty(const Reference< XPropertySet >& _rxSet, const ::rtl::OUString& _rName, const SfxPoolItem* _pItem)
{
    Any aValue = implTranslateProperty(_pItem);
    try
    {
        _rxSet->setPropertyValue(_rName, aValue);
    }
    catch(Exception&)
    {
#ifdef DBG_UTIL
        ::rtl::OString sMessage("ODbAdminDialog::implTranslateProperty: could not set the property ");
        sMessage += ::rtl::OString(_rName.getStr(), _rName.getLength(), RTL_TEXTENCODING_ASCII_US);
        sMessage += ::rtl::OString("!");
        DBG_ERROR(sMessage.getStr());
#endif
    }
}

//-------------------------------------------------------------------------
void ODbAdminDialog::implTranslateProperty(SfxItemSet& _rSet, sal_Int32  _nId, const Any& _rValue)
{
    switch (_rValue.getValueType().getTypeClass())
    {
        case TypeClass_STRING:
        {
            ::rtl::OUString sValue;
            _rValue >>= sValue;
            _rSet.Put(SfxStringItem((USHORT)_nId, sValue.getStr()));
        }
        break;
        case TypeClass_BOOLEAN:
            _rSet.Put(SfxBoolItem((USHORT)_nId, ::cppu::any2bool(_rValue)));
            break;
        case TypeClass_LONG:
            {
                sal_Int32 nValue = 0;
                _rValue >>= nValue;
                _rSet.Put(SfxInt32Item((USHORT)_nId, nValue));
            }
            break;
        case TypeClass_SEQUENCE:
        {
            // determine the element type
            TypeDescription aTD(_rValue.getValueType());
            typelib_IndirectTypeDescription* pSequenceTD =
                reinterpret_cast< typelib_IndirectTypeDescription* >(aTD.get());
            DBG_ASSERT(pSequenceTD && pSequenceTD->pType, "ODbAdminDialog::implTranslateProperty: invalid sequence type!");

            Type aElementType(pSequenceTD->pType);
            switch (aElementType.getTypeClass())
            {
                case TypeClass_STRING:
                {
                    Sequence< ::rtl::OUString > aStringList;
                    _rValue >>= aStringList;
                    _rSet.Put(OStringListItem((USHORT)_nId, aStringList));
                }
                break;
                default:
                    DBG_ERROR("ODbAdminDialog::implTranslateProperty: unsupported property value type!");
            }
        }
        break;
        case TypeClass_VOID:
            _rSet.ClearItem((USHORT)_nId);
            break;
        default:
            DBG_ERROR("ODbAdminDialog::implTranslateProperty: unsupported property value type!");
    }
}

//-------------------------------------------------------------------------
struct PropertyValueLess
{
    bool operator() (const PropertyValue& x, const PropertyValue& y) const
        { return x.Name < y.Name ? true : false; }      // construct prevents a MSVC6 warning
};
DECLARE_STL_SET( PropertyValue, PropertyValueLess, PropertyValueSet);

//........................................................................
void ODbAdminDialog::translateProperties(const Reference< XPropertySet >& _rxSource, SfxItemSet& _rDest)
{
    ::rtl::OUString sNewConnectURL, sName, sUid, sPwd;
    Sequence< ::rtl::OUString > aTableFitler;
    sal_Bool bPasswordRequired = sal_False;
    sal_Bool bReadOnly = sal_True;

    if (_rxSource.is())
    {
        for (   ConstMapInt2StringIterator aDirect = m_aDirectPropTranslator.begin();
                aDirect != m_aDirectPropTranslator.end();
                ++aDirect
            )
        {
            // get the property value
            Any aValue;
            try
            {
                aValue = _rxSource->getPropertyValue(aDirect->second);
            }
            catch(Exception&)
            {
#if DBG_UTIL
                ::rtl::OString aMessage("ODbAdminDialog::translateProperties: could not extract the property ");
                aMessage += ::rtl::OString(aDirect->second.getStr(), aDirect->second.getLength(), RTL_TEXTENCODING_ASCII_US);
                aMessage += ::rtl::OString("!");
                DBG_ERROR(aMessage.getStr());
#endif
            }
            // transfer it into an item
            implTranslateProperty(_rDest, aDirect->first, aValue);
        }

        // get the additional informations
        Sequence< PropertyValue > aAdditionalInfo;
        try
        {
            _rxSource->getPropertyValue(PROPERTY_INFO) >>= aAdditionalInfo;
        }
        catch(Exception&) { }

        // collect the names of the additional settings
        const PropertyValue* pAdditionalInfo = aAdditionalInfo.getConstArray();
        PropertyValueSet aInfos;
        for (sal_Int32 i=0; i<aAdditionalInfo.getLength(); ++i, ++pAdditionalInfo)
        {
            if (0 == pAdditionalInfo->Name.compareToAscii("JDBCDRV"))
            {   // compatibility
                PropertyValue aCompatibility(*pAdditionalInfo);
                aCompatibility.Name = ::rtl::OUString::createFromAscii("JavaDriverClass");
                aInfos.insert(aCompatibility);
            }
            else
                aInfos.insert(*pAdditionalInfo);
        }

        // go through all known translations and check if we have such a setting
        PropertyValue aSearchFor;
        for (   ConstMapInt2StringIterator aIndirect = m_aIndirectPropTranslator.begin();
                aIndirect != m_aIndirectPropTranslator.end();
                ++aIndirect
            )
        {
            aSearchFor.Name = aIndirect->second;
            ConstPropertyValueSetIterator aInfoPos = aInfos.find(aSearchFor);
            if (aInfos.end() != aInfoPos)
                // the property is contained in the info sequence
                // -> transfer it into an item
                implTranslateProperty(_rDest, aIndirect->first, aInfoPos->Value);
        }
    }
}

//-------------------------------------------------------------------------
void ODbAdminDialog::translateProperties(const SfxItemSet& _rSource, const Reference< XPropertySet >& _rxDest)
{
    DBG_ASSERT(_rxDest.is(), "ODbAdminDialog::translateProperties: invalid property set!");
    if (!_rxDest.is())
        return;

    // the property set info
    Reference< XPropertySetInfo > xInfo;
    try { xInfo = _rxDest->getPropertySetInfo(); }
    catch(Exception&) { }

    // -----------------------------
    // transfer the direct propertis
    for (   ConstMapInt2StringIterator aDirect = m_aDirectPropTranslator.begin();
            aDirect != m_aDirectPropTranslator.end();
            ++aDirect
        )
    {
        const SfxPoolItem* pCurrentItem = _rSource.GetItem((USHORT)aDirect->first);
        if (pCurrentItem)
        {
            sal_Int16 nAttributes = PropertyAttribute::READONLY;
            if (xInfo.is())
            {
                try { nAttributes = xInfo->getPropertyByName(aDirect->second).Attributes; }
                catch(Exception&) { }
            }
            if ((nAttributes & PropertyAttribute::READONLY) == 0)
                implTranslateProperty(_rxDest, aDirect->second, pCurrentItem);
        }
    }

    // -------------------------------
    // now for the indirect properties

    Sequence< PropertyValue > aInfo;
    // the original properties
    try
    {
        _rxDest->getPropertyValue(PROPERTY_INFO) >>= aInfo;
    }
    catch(Exception&) { }

    // overwrite and extend them
    fillDatasourceInfo(_rSource, aInfo);

    // and propagate the (newly composed) sequence to the set
    try
    {
        _rxDest->setPropertyValue(PROPERTY_INFO, makeAny(aInfo));
    }
    catch(Exception&)
    {
        DBG_ERROR("ODbAdminDialog::translateProperties: could not propagate the composed info sequence to the property set!");
    }
}

//-------------------------------------------------------------------------
DATASOURCE_TYPE ODbAdminDialog::getDatasourceType(const SfxItemSet& _rSet) const
{
    SFX_ITEMSET_GET(_rSet, pConnectURL, SfxStringItem, DSID_CONNECTURL, sal_True);
    SFX_ITEMSET_GET(_rSet, pTypeCollection, DbuTypeCollectionItem, DSID_TYPECOLLECTION, sal_True);
    DBG_ASSERT(pConnectURL && pTypeCollection, "ODbAdminDialog::getDatasourceType: invalid items in the source set!");
    String sConnectURL = pConnectURL->GetValue();
    ODsnTypeCollection* pCollection = pTypeCollection->getCollection();
    DBG_ASSERT(pCollection, "ODbAdminDialog::getDatasourceType: invalid type collection!");
    return pCollection->getType(sConnectURL);
}

//-------------------------------------------------------------------------
sal_Bool ODbAdminDialog::hasAuthentication(const SfxItemSet& _rSet) const
{
    DATASOURCE_TYPE eType = getDatasourceType(_rSet);
    SFX_ITEMSET_GET(_rSet, pTypeCollection, DbuTypeCollectionItem, DSID_TYPECOLLECTION, sal_True);
    return pTypeCollection->getCollection()->hasAuthentication(eType);
}

//-------------------------------------------------------------------------
const sal_Int32* ODbAdminDialog::getRelevantItems(const SfxItemSet& _rSet) const
{
    DATASOURCE_TYPE eType = getDatasourceType(_rSet);
    const sal_Int32* pRelevantItems = NULL;
    switch (eType)
    {
        case DST_ADABAS:
            {
                static sal_Int32* pAdabasItems = NULL;
                if(!pAdabasItems)
                {
                    const sal_Int32* pFirstRelevantItems = OAdabasDetailsPage::getDetailIds();
                    const sal_Int32* pSecondRelevantItems = OAdabasAdminSettings::getDetailIds();
                    sal_Int32 nFLen = 0;
                    sal_Int32 nSLen = 0;

                    for(pRelevantItems = pFirstRelevantItems;pRelevantItems && *pRelevantItems;++pRelevantItems)
                        ++nFLen;

                    for(pRelevantItems = pSecondRelevantItems;pRelevantItems && *pRelevantItems;++pRelevantItems)
                        ++nSLen;

                    pAdabasItems = new sal_Int32[nFLen + nSLen + 1];
                    nFLen = 0;
                    for(pRelevantItems = pFirstRelevantItems;pRelevantItems && *pRelevantItems;++pRelevantItems)
                        pAdabasItems[nFLen++] = *pRelevantItems;

                    for(pRelevantItems = pSecondRelevantItems;pRelevantItems && *pRelevantItems;++pRelevantItems)
                        pAdabasItems[nFLen++] = *pRelevantItems;
                    pAdabasItems[nFLen] = 0;

                }
                pRelevantItems = pAdabasItems;
            }
            break;
        case DST_JDBC:          pRelevantItems = OJdbcDetailsPage::getDetailIds(); break;
        case DST_ADO:           pRelevantItems = OAdoDetailsPage::getDetailIds(); break;
        case DST_ODBC:          pRelevantItems = OOdbcDetailsPage::getDetailIds(); break;
        case DST_ADDRESSBOOK:
            {
                String sConnectionURL;
                SFX_ITEMSET_GET(*GetExampleSet(), pUrlItem, SfxStringItem, DSID_CONNECTURL, sal_True);
                sConnectionURL = pUrlItem->GetValue();
                if(String::CreateFromAscii("sdbc:address:ldap:") == sConnectionURL)
                    pRelevantItems = OLDAPDetailsPage::getDetailIds();
                else
                {
                    static sal_Int32 nRelevantIds[] = { 0 };
                    pRelevantItems = nRelevantIds;
                }
                break;
            }
        case DST_DBASE:         pRelevantItems = ODbaseDetailsPage::getDetailIds(); break;
        case DST_TEXT:          pRelevantItems = OTextDetailsPage::getDetailIds(); break;
        case DST_CALC:
            {
                // spreadsheet currently has no options page
                static sal_Int32 nRelevantIds[] = { 0 };
                pRelevantItems = nRelevantIds;
            }
            break;
    }
    return pRelevantItems;
}

//-------------------------------------------------------------------------
void ODbAdminDialog::fillDatasourceInfo(const SfxItemSet& _rSource, ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rInfo)
{
    // within the current "Info" sequence, replace the ones we can examine from the item set
    // (we don't just fill a completely new sequence with our own items, but we preserve any properties unknown to
    // us)

    // first determine which of all the items are relevant for the data source (depends on the connection url)
    const sal_Int32* pRelevantItems = getRelevantItems(_rSource);
    DBG_ASSERT(pRelevantItems, "ODbAdminDialog::translateProperties: invalid item ids got from the page!");

    // collect the translated property values for the relevant items
    PropertyValueSet aRelevantSettings;
    ConstMapInt2StringIterator aTranslation;
    while (pRelevantItems && *pRelevantItems)
    {
        const SfxPoolItem* pCurrent = _rSource.GetItem((USHORT)*pRelevantItems);
        aTranslation = m_aIndirectPropTranslator.find(*pRelevantItems);
        if (pCurrent && (m_aIndirectPropTranslator.end() != aTranslation))
            aRelevantSettings.insert(PropertyValue(aTranslation->second, 0, implTranslateProperty(pCurrent), PropertyState_DIRECT_VALUE));

        ++pRelevantItems;
    }

    // settings to preserve
    MapInt2String   aPreservedSettings;

    // now aRelevantSettings contains all the property values relevant for the current data source type,
    // check the original sequence if it already contains any of these values (which have to be overwritten, then)
    PropertyValue* pInfo = _rInfo.getArray();
    PropertyValue aSearchFor;
    sal_Int32 nObsoleteSetting = -1;
    for (sal_Int32 i=0; i<_rInfo.getLength(); ++i, ++pInfo)
    {
        aSearchFor.Name = pInfo->Name;
        PropertyValueSetIterator aOverwrittenSetting = aRelevantSettings.find(aSearchFor);
        if (aRelevantSettings.end() != aOverwrittenSetting)
        {   // the setting was present in the original sequence, and it is to be overwritten -> replace it
            *pInfo = *aOverwrittenSetting;
            aRelevantSettings.erase(aOverwrittenSetting);
        }
        else if (0 == pInfo->Name.compareToAscii("JDBCDRV"))
        {   // this is a compatibility setting, remove it from the sequence (it's replaced by JavaDriverClass)
            nObsoleteSetting = i;
        }
        else
            aPreservedSettings[i] = pInfo->Name;
    }
    if (-1 != nObsoleteSetting)
        ::comphelper::removeElementAt(_rInfo, nObsoleteSetting);

    if (aPreservedSettings.size())
    {   // check if there are settings which
        // * are known as indirect properties
        // * but not relevant for the current data source type
        // These settings have to be removed: If they're not relevant, we have no UI for changing them.
        // 25.06.2001 - 88004/87182 - frank.schoenheit@sun.com

        // for this, we need a string-controlled quick access to m_aIndirectPropTranslator
        StringSet aIndirectProps;
        for (   ConstMapInt2StringIterator aIPLoop = m_aIndirectPropTranslator.begin();
                aIPLoop != m_aIndirectPropTranslator.end();
                ++aIPLoop
            )
        {
            aIndirectProps.insert(aIPLoop->second);
        }

        // now check the to-be-preserved props
        ::std::vector< sal_Int32 > aRemoveIndexes;
        sal_Int32 nPositionCorrector = 0;
        for (   ConstMapInt2StringIterator aPreserved = aPreservedSettings.begin();
                aPreserved != aPreservedSettings.end();
                ++aPreserved
            )
        {
            if (aIndirectProps.end() != aIndirectProps.find(aPreserved->second))
            {
#ifdef DBG_UTIL
                const ::rtl::OUString sName = aPreserved->second;
#endif
                aRemoveIndexes.push_back(aPreserved->first - nPositionCorrector);
                ++nPositionCorrector;
            }
        }
        // now finally remove all such props
        for (   ::std::vector< sal_Int32 >::const_iterator aRemoveIndex = aRemoveIndexes.begin();
                aRemoveIndex != aRemoveIndexes.end();
                ++aRemoveIndex
            )
            ::comphelper::removeElementAt(_rInfo, *aRemoveIndex);
#ifdef DBG_UTIL
        const PropertyValue* pWhatsLeft = _rInfo.getConstArray();
        const PropertyValue* pWhatsLeftEnd = pWhatsLeft + _rInfo.getLength();
        for (; pWhatsLeft != pWhatsLeftEnd; ++pWhatsLeft)
        {
            ::rtl::OUString sLookAtIt = pWhatsLeft->Name;
        }
#endif
    }

    // check which values are still left ('cause they were not present in the original sequence, but are to be set)
    sal_Int32 nOldLength = _rInfo.getLength();
    _rInfo.realloc(nOldLength + aRelevantSettings.size());
    PropertyValue* pAppendValues = _rInfo.getArray() + nOldLength;
    for (   ConstPropertyValueSetIterator aLoop = aRelevantSettings.begin();
            aLoop != aRelevantSettings.end();
            ++aLoop, ++pAppendValues
        )
    {
        *pAppendValues = *aLoop;
    }
}

//-------------------------------------------------------------------------
::rtl::OUString ODbAdminDialog::getUniqueName() const
{
    ::rtl::OUString sBase = String(ModuleRes(STR_DATASOURCE_DEFAULTNAME)).GetBuffer();
    sBase += ::rtl::OUString(" ", 1, RTL_TEXTENCODING_ASCII_US);
    for (sal_Int32 i=1; i<65635; ++i)
    {
        ::rtl::OUString sCheck(sBase);
        sCheck += ::rtl::OUString::valueOf(i);

        // check if "all but the current" datasources aleady contain the to-be-checked name
        if (m_aValidDatasources.end() != m_aValidDatasources.find(sCheck))
            continue;
        // check if the currently selected data source allows the new name
        if (m_sCurrentDatasource.equals(sCheck))
            continue;

        // have a valid new name
        return sCheck;
    }

    DBG_ERROR("ODbAdminDialog::getUniqueName: no free names!");
    return ::rtl::OUString();
}

//-------------------------------------------------------------------------
sal_Bool ODbAdminDialog::prepareSwitchDatasource()
{
    // first ask the current page if it is allowed to leave
    if (!PrepareLeaveCurrentPage())
        // the page did not allow us to leave -> outta here
        return sal_False;

    // if the old data source is not a to-be-deleted one, save the modifications made in the tabpages
    if (m_sCurrentDatasource.getLength())
    {
        // remember the settings for this data source
        ODatasourceMap::ODatasourceInfo aPreviouslySelected = m_aDatasources[m_sCurrentDatasource];
        if (aPreviouslySelected.isModified())
            m_aDatasources.update(m_sCurrentDatasource, *pExampleSet);
        // (The modified flag is set as soon as any UI element has any change (e.g. a single new character in a edit line).
        // But when this flag is set, and other changes occur, no items are transfered.
        // That's why the above statement "if (isModified()) update()" makes sense, though it may not seem so :)

        // need a special handling for the name property
        if (aPreviouslySelected.isModified())
        {
            String sName = aPreviouslySelected.getName().getStr();
            DBG_ASSERT(m_sCurrentDatasource.equals(sName.GetBuffer()), "ODbAdminDialog::prepareSwitchDatasource: inconsistent names!");

            // first adjust the name which the datasource is stored under in our map.
            String sNewName = m_aDatasources.adjustRealName(sName);
            // if this was a real change ('cause the ds was stored under name "A", but the modifications set already contained
            // an DSID_NAME item "B", which has been corrected by the previous call), tell the selector window that
            // something changed
            if (!sNewName.Equals(sName))
            {
                // tell our selector window that the name has changed
                m_aSelector.renamed(sName, sNewName);
                // update our "current database"
                m_sCurrentDatasource = sNewName;
            }
        }
    }

    return sal_True;
}

//-------------------------------------------------------------------------
IMPL_LINK(ODbAdminDialog, OnNewDatasource, Window*, _pWindow)
{
    if (!prepareSwitchDatasource())
        return 1L;

    ::rtl::OUString sNewName = getUniqueName();
    if (0 == sNewName.getLength())
        return 1L;  // no free names

    // create a new datasource (not belonging to a context, yet)
    Reference< XPropertySet > xFloatingDatasource = m_aDatasources.createNew(sNewName, GetInputSetImpl()->GetPool(), GetInputSetImpl()->GetRanges());
    if (!xFloatingDatasource.is())
    {
        ShowServiceNotAvailableError(this, String(SERVICE_SDB_DATASOURCE), sal_True);
        return 1L;
    }

    GetInputSetImpl()->ClearItem();

    // insert a new entry for the new DS
    m_aSelector.insertNew(sNewName);
    // update our "all-but the selected ds" structure
    m_aValidDatasources.insert(sNewName);

    // and select this new entry
    m_aSelector.select(sNewName);
    implSelectDatasource(sNewName);

    // enable the apply button
    GetApplyButton()->Enable(sal_True);

    SfxTabPage* pGeneralPage = GetTabPage(PAGE_GENERAL);
    if (pGeneralPage)
        pGeneralPage->GrabFocus();

    return 0L;
}

//-------------------------------------------------------------------------
IMPL_LINK(ODbAdminDialog, OnDeleteDatasource, Window*, _pWindow)
{
    ::rtl::OUString sDeleteWhich = m_aSelector.getSelected();

    if (NEW == m_aSelector.getSelectedState())
    {
        // insert the previously selected data source into our "all valid datasources" set
        if (m_sCurrentDatasource.getLength())   // previous selection was not on a deleted data source
            m_aValidDatasources.insert(m_sCurrentDatasource);
        m_sCurrentDatasource = ::rtl::OUString();

        m_aDatasources.deleted(sDeleteWhich);
        m_aSelector.deleted(sDeleteWhich);
    }
    else
    {
        sal_Int32 nAccessKey = m_aDatasources.markDeleted(sDeleteWhich);
        if (-1 == nAccessKey)
            return 0L;

        // mark it as deleted
        m_aSelector.markDeleted(sDeleteWhich, nAccessKey);
        // re-select it (to reset the pages so they reflect the new state)
        implSelectDeleted(nAccessKey);
    }

    // mark the name as "available"
    m_aValidDatasources.erase(sDeleteWhich);

    // enable the apply button
    GetApplyButton()->Enable(sal_True);

    return 1L;
}

//-------------------------------------------------------------------------
IMPL_LINK(ODbAdminDialog, OnRestoreDatasource, Window*, _pWindow)
{
    sal_Int32 nAccessKey = m_aSelector.getSelectedAccessKey();
    ::rtl::OUString sName;
    if (m_aDatasources.restoreDeleted(nAccessKey, sName))
    {   // successfully restore the item in the map
        // -> restore it in the view, too
        ODatasourceMap::ODatasourceInfo aInfo(m_aDatasources[sName]);
        m_aSelector.restoreDeleted(nAccessKey, aInfo.isModified() ? MODIFIED : aInfo.isNew() ? NEW : CLEAN);

        implSelectDatasource(sName);
    }
    else
    {
        ErrorBox aError(this, ModuleRes(ERR_COULDNOTRESTOREDS));
        aError.Execute();
    }

    // enable the apply button
    GetApplyButton()->Enable(sal_True);

    return 0L;
}

//-------------------------------------------------------------------------
ODbAdminDialog::ApplyResult ODbAdminDialog::implApplyChanges(const sal_Bool _bActivateOnSuccess)
{
    if (!PrepareLeaveCurrentPage())
    {   // the page did not allow us to leave
        return AR_KEEP;
    }

    ApplyResult eResult = AR_LEAVE_UNCHANGED;

    // save the settings for the currently selected data source
    if (m_aSelector.count() && (DELETED != m_aSelector.getSelectedState()))
    {
        ::rtl::OUString sCurrentlySelected = m_aSelector.getSelected();
        if (m_aDatasources[sCurrentlySelected]->isModified())
        {
            m_aDatasources.update(sCurrentlySelected, *pExampleSet);
            String sNewName = m_aDatasources.adjustRealName(sCurrentlySelected);
            String sOldName = sCurrentlySelected;
            // the data source has not only been modified, but renamed, too
            // -> adjust the selector and m_sCurrentDatasource
            // (we are allowed to do this here, this is no part of the committment of the changes, it just
            // leaves our structures in a consistent state for the real commitment)
            if (!sNewName.Equals(sOldName))
            {
                // tell our selector window that the name has changed
                m_aSelector.renamed(sOldName, sNewName);
                // update our "current database"
                m_sCurrentDatasource = sNewName;

                // adjust the selection
                implSelectDatasource(m_sCurrentDatasource);
            }
        }
    }

    // We allowed the user to freely rename/create/delete datasources, without committing anything ('til now).
    // This could lead to conflicts: if datasource "A" was renamed to "B", and a new ds "A" created, then we
    // would have to do the renaming before the creation. This would require us to analyze the changes in
    // m_aDatasources for any such dependencies, which could be difficult (okay, I'm not willing to do it :)
    // Instead we use another approach: If we encounter a conflict (a DS which can't be renamed or inserted),
    // we save this entry and continue with the next one. This way, the ds causing the conflict should be handled
    // first. After that, we do a new round, assuming that now the conflict is resolved.
    // A disadvantage is that this may require O(n^2) rounds, but this is not really expensive ....

    // first delete all datasources which were scheduled for deletion
    for (   ODatasourceMap::Iterator aLoopDeleted = m_aDatasources.beginDeleted();
            aLoopDeleted != m_aDatasources.endDeleted();
            ++aLoopDeleted
        )
    {
        ::rtl::OUString sDeleteWhich = aLoopDeleted->getOriginalName();
        sal_Bool bOperationSuccess = sal_False;
        try
        {
            m_xDynamicContext->revokeObject(sDeleteWhich);
            bOperationSuccess = sal_True;
        }
        catch(Exception&) { }
        if (bOperationSuccess)
        {
            eResult = AR_LEAVE_MODIFIED;
            m_aSelector.deleted(aLoopDeleted->getAccessKey());
        }
        else
        {
            DBG_ERROR("ODbAdminDialog::implApplyChanges: could not delete a data source!");
            // TODO: an error message
        }
    }
    m_aDatasources.clearDeleted();

    sal_Int32 nDelayed = 0;
    sal_Int32 nLastRoundDelayed = -1;
        // to ensure that we're not looping 'til death: If this doesn't change within one round, the DatabaseContext
        // is not in the state as this dialog started anymore. This means somebody else did some renamings
        // or insertings, causing us conflicts now.

    do
    {
        if (nLastRoundDelayed == nDelayed)
        {
            DBG_ERROR("ODbAdminDialog::implApplyChanges: somebody tampered with the context!");
            // TODO: error handling
            break;
        }

        // reset the counter
        nLastRoundDelayed = nDelayed;
        nDelayed = 0;

        // propagate all the settings made to the appropriate data source, and add/drop/rename data sources
        for (   ODatasourceMap::Iterator aLoop = m_aDatasources.begin();
                aLoop != m_aDatasources.end();
                ++aLoop
            )
        {
            // nothing to do if no modifications were done
            if (aLoop->isModified())
            {
                Reference< XPropertySet > xDatasource = aLoop->getDatasource();
                if (xDatasource.is())
                {
                    eResult = AR_LEAVE_MODIFIED;
                        // we changes something

                    // put the remembered settings into the property set
                    translateProperties(*aLoop->getModifications(), xDatasource);

                    ::rtl::OUString sName = aLoop->getName();
                    DBG_ASSERT(sName.equals(aLoop->getRealName()), "ODbAdminDialog::implApplyChanges: invalid name/realname combination!");
                        // these both names shouldn't be diefferent here anymore
                    ::rtl::OUString sOriginalName = aLoop->getOriginalName();

                    // if we need a new name, check for conflicts
                    if (aLoop->isRenamed() || aLoop->isNew())
                    {
                        sal_Bool bAlreadyHaveNewName = sal_True;
                        try
                        {
                            bAlreadyHaveNewName = m_xDatabaseContext->hasByName(sName);
                        }
                        catch(RuntimeException&) { }
                        if (bAlreadyHaveNewName)
                        {
                            ++nDelayed;
                            continue;
    //  | <---------------- continue with the next data source
                        }

                        if (aLoop->isRenamed())
                        {
                            // remove the object
                            sal_Bool bOperationSuccess = sal_False;
                            try
                            {
                                m_xDynamicContext->revokeObject(sOriginalName);
                                bOperationSuccess = sal_True;
                            }
                            catch(Exception&) { }
                            if (!bOperationSuccess)
                            {
                                DBG_ERROR("ODbAdminDialog::implApplyChanges: data source was renamed, but could not remove it (to insert it under a new name)!");
                                // TODO: an error message
                            }
                        }

                        // (re)insert the object under the new name
                        sal_Bool bOperationSuccess = sal_False;
                        try
                        {
                            m_xDynamicContext->registerObject(sName, xDatasource.get());
                            bOperationSuccess = sal_True;
                        }
                        catch(Exception&) { }
                        if (bOperationSuccess)
                        {
                            // everything's ok ...
                            // no need to flush the object anymore, this is done automatically upon insertion
                        }
                        else if (aLoop->isRenamed())
                        {
                            // oops ... we removed the ds, but could not re-insert it
                            // try to prevent data loss
                            DBG_ERROR("ODbAdminDialog::implApplyChanges: removed the entry, but could not re-insert it!");
                            // we're going to re-insert the object under it's old name
                            bOperationSuccess = sal_False;
                            try
                            {
                                m_xDynamicContext->registerObject(sOriginalName, xDatasource.get());
                                bOperationSuccess = sal_True;
                            }
                            catch(Exception&) { }
                            DBG_ASSERT(bOperationSuccess, "ODbAdminDialog::implApplyChanges: could not insert it under the old name, too ... no we have a data loss!");
                        }

                        // reset the item, so in case we need an extra round (because of delayed items) it
                        // won't be included anymore
                        m_aDatasources.clearModifiedFlag(sName);
                        // and tell the selector the new state
                        m_aSelector.flushed(sName);

                        continue;
    //  | <------------ continue with the next data source
                    }

                    // We're here if the data source was not renamed, not deleted and is not new. Just flush it.
                    Reference< XFlushable > xFlushDatasource(xDatasource, UNO_QUERY);
                    if (!xFlushDatasource.is())
                    {
                        DBG_ERROR("ODbAdminDialog::implApplyChanges: the datasource should be flushable!");
                        continue;
                    }

                    try
                    {
                        xFlushDatasource->flush();
                    }
                    catch(RuntimeException&)
                    {
                        DBG_ERROR("ODbAdminDialog::implApplyChanges: caught an exception whild flushing the data source's data!");
                    }
                    // reset the item, so in case we need an extra round (because of delayed items) it
                    // won't be included anymore
                    m_aDatasources.clearModifiedFlag(sName);
                    // and tell the selector the new state
                    m_aSelector.flushed(sName);
                }
            }
        }
    }
    while (nDelayed);

    // reset some meta-data-items in the the example set
    // 00/11/10 - 80185 - FS
    pExampleSet->Put(SfxBoolItem(DSID_NEWDATASOURCE, sal_False));
    pExampleSet->Put(SfxBoolItem(DSID_DELETEDDATASOURCE, sal_False));

    // disable the apply button
    GetApplyButton()->Enable(sal_False);


    if (_bActivateOnSuccess)
        ShowPage(GetCurPageId());
        // This does the usual ActivatePage, so the pages can save their current status.
        // This way, next time they're asked what has changed since now and here, they really
        // can compare with the status they have _now_ (not the one they had before this apply call).

    return eResult;
}

//-------------------------------------------------------------------------
IMPL_LINK(ODbAdminDialog, OnAsyncApplyChanges, void*, _OnErrorResId)
{
    SfxTabDialog::Ok();
    if (AR_KEEP != implApplyChanges())
    {
        // show the page
        if (GetCurPageId() != m_nPostApplyPage)
            ShowPage(m_nPostApplyPage);

        // restore the view settings
        if (m_pPostApplyPageSettings)
        {
            SfxTabPage* pPage = GetTabPage(m_nPostApplyPage);
            if (pPage)
                static_cast<OGenericAdministrationPage*>(pPage)->restoreViewSettings(m_pPostApplyPageSettings);

            delete m_pPostApplyPageSettings;
            m_pPostApplyPageSettings = NULL;
        }

        m_nPostApplyPage = 0;

        return 1L;
    }
    return 0L;
}

//-------------------------------------------------------------------------
IMPL_LINK(ODbAdminDialog, OnApplyChanges, PushButton*, EMPTYARG)
{
    const sal_uInt16 nOldPageId = GetCurPageId();

    // get the view settings of the current page
    SfxTabPage* pCurrentPage = GetTabPage(nOldPageId);
    OPageSettings* pViewSettings = NULL;
    if (pCurrentPage)
    {
        pViewSettings = static_cast<OGenericAdministrationPage *>(pCurrentPage)->createViewSettings();
        static_cast<OGenericAdministrationPage *>(pCurrentPage)->fillViewSettings(pViewSettings);
    }

    // really apply the changes
    implApplyChanges();

    // select the old page, again (if possible)
    const sal_uInt16 nNewPageId = GetCurPageId();

    pCurrentPage = GetTabPage(nOldPageId);
    if (pCurrentPage)
    {
        if (nNewPageId != nOldPageId)
            ShowPage(nOldPageId);

        static_cast<OGenericAdministrationPage *>(pCurrentPage)->restoreViewSettings(pViewSettings);
    }

    delete pViewSettings;
    return 0L;
}

//.........................................................................
}   // namespace dbaui
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.64  2001/07/11 10:10:30  oj
 *  #87257# change GetUILanguage
 *
 *  Revision 1.63  2001/07/06 11:33:29  oj
 *  #89359# now dialog saves password temp
 *
 *  Revision 1.62  2001/06/25 16:04:40  fs
 *  #88004# outsourced ODataSourceMap and ODataSourceSelector / adjusted fillDatasourceInfo so that settings without and UI are do not survive the method
 *
 *  Revision 1.61  2001/06/25 08:27:18  oj
 *  #88699# new control for ldap rowcount
 *
 *  Revision 1.60  2001/06/20 13:43:42  fs
 *  #88447# corrected order of detail pages
 *
 *  Revision 1.59  2001/06/20 07:08:33  oj
 *  #88434# new page for user admin
 *
 *  Revision 1.58  2001/06/14 14:18:10  fs
 *  #88242# corrected adding/removing detail pages
 *
 *  Revision 1.57  2001/06/07 15:12:36  fs
 *  #87934# removed a wrong assertion
 *
 *  Revision 1.56  2001/06/01 08:41:31  oj
 *  #87149# changed order for tabpages
 *
 *  Revision 1.55  2001/05/31 11:37:57  oj
 *  #87149# correct ldap protocol
 *
 *  Revision 1.54  2001/05/31 11:09:07  oj
 *  #87149# change subprotocol and Propertynames
 *
 *  Revision 1.53  2001/05/29 13:33:12  oj
 *  #87149# addressbook ui impl
 *
 *  Revision 1.52  2001/05/29 10:18:26  fs
 *  #86082# set the service factory on the general page
 *
 *  Revision 1.51  2001/05/23 14:16:42  oj
 *  #87149# new helpids
 *
 *  Revision 1.50  2001/05/15 15:07:06  fs
 *  #86991# save the current (modified) settings when inserting a new data source
 *
 *  Revision 1.49  2001/05/15 11:25:35  fs
 *  #86996# use the connection pool instead of the driver manager
 *
 *  Revision 1.48  2001/05/10 13:37:04  fs
 *  #86223# restore view settings after applying (no matter if syncronously or asynchronously / +successfullyConnected to make the password persistent
 *
 *  Revision 1.47  2001/04/27 15:47:03  fs
 *  resetPages: do a ShowPage(GENERAL) before removing pages
 *
 *  Revision 1.46  2001/04/26 11:40:21  fs
 *  file is alive, again - added support for data source associated bookmarks
 *
 *  Revision 1.45  2001/04/20 13:38:06  oj
 *  #85736# new checkbox for odbc
 *
 *  Revision 1.44  2001/04/04 10:38:43  oj
 *  reading uninitialized memory
 *
 *  Revision 1.43  2001/03/30 11:54:34  fs
 *  #65293# missing include
 *
 *  Revision 1.42  2001/03/29 07:44:43  fs
 *  #84826# +clearPassword
 *
 *  Revision 1.41  2001/03/29 07:34:00  oj
 *  dispose connection in dtor and type casts
 *
 *  Revision 1.0 20.09.00 10:55:58  fs
 ************************************************************************/

