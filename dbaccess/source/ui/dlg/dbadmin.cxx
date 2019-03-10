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

#include "ConnectionPage.hxx"
#include "DbAdminImpl.hxx"
#include "DriverSettings.hxx"
#include "adminpages.hxx"
#include <dbadmin.hxx>
#include <dbu_dlg.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <core_resource.hxx>
#include <stringconstants.hxx>
#include <strings.hrc>
#include <dsitems.hxx>
#include "dsnItem.hxx"
#include "optionalboolitem.hxx"
#include <stringlistitem.hxx>

#include <unotools/confignode.hxx>

namespace dbaui
{
using namespace com::sun::star::uno;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::lang;
using namespace com::sun::star::util;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;

// ODbAdminDialog
ODbAdminDialog::ODbAdminDialog(weld::Window* pParent,
                               SfxItemSet const * _pItems,
                               const Reference< XComponentContext >& _rxContext)
    : SfxTabDialogController(pParent, "dbaccess/ui/admindialog.ui", "AdminDialog", _pItems)
    , m_sMainPageID("advanced")
{
    m_pImpl.reset(new ODbDataSourceAdministrationHelper(_rxContext, m_xDialog.get(), pParent, this));

    // add the initial tab page
    AddTabPage(m_sMainPageID, OConnectionTabPage::Create, nullptr);

    // remove the reset button - it's meaning is much too ambiguous in this dialog
    RemoveResetButton();
}

ODbAdminDialog::~ODbAdminDialog()
{
    SetInputSet(nullptr);
}

short ODbAdminDialog::Ok()
{
    SfxTabDialogController::Ok();
    return ( AR_LEAVE_MODIFIED == implApplyChanges() ) ? RET_OK : RET_CANCEL;
        // TODO : AR_ERROR is not handled correctly, we always close the dialog here
}

void ODbAdminDialog::PageCreated(const OString& rId, SfxTabPage& _rPage)
{
    // register ourself as modified listener
    static_cast<OGenericAdministrationPage&>(_rPage).SetServiceFactory( getORB() );
    static_cast<OGenericAdministrationPage&>(_rPage).SetAdminDialog(this,this);

    SfxTabDialogController::PageCreated(rId, _rPage);
}

void ODbAdminDialog::addDetailPage(const OString& rPageId, const char* pTextId, CreateTabPage pCreateFunc)
{
    AddTabPage(rPageId, DBA_RES(pTextId), pCreateFunc);
}

void ODbAdminDialog::impl_selectDataSource(const css::uno::Any& _aDataSourceName)
{
    m_pImpl->setDataSourceOrName(_aDataSourceName);
    Reference< XPropertySet > xDatasource = m_pImpl->getCurrentDataSource();
    impl_resetPages( xDatasource );

    const DbuTypeCollectionItem& rCollectionItem = dynamic_cast<const DbuTypeCollectionItem&>(*getOutputSet()->GetItem(DSID_TYPECOLLECTION));
    ::dbaccess::ODsnTypeCollection* pCollection = rCollectionItem.getCollection();
    ::dbaccess::DATASOURCE_TYPE eType = pCollection->determineType(getDatasourceType(*getOutputSet()));

    // and insert the new ones
    switch ( eType )
    {
        case  ::dbaccess::DST_DBASE:
            addDetailPage("dbase", STR_PAGETITLE_ADVANCED, ODriversSettings::CreateDbase);
            break;

        case  ::dbaccess::DST_ADO:
            addDetailPage("ado", STR_PAGETITLE_ADVANCED, ODriversSettings::CreateAdo);
            break;

        case  ::dbaccess::DST_FLAT:
            addDetailPage("text", STR_PAGETITLE_ADVANCED, ODriversSettings::CreateText);
            break;

        case  ::dbaccess::DST_ODBC:
            addDetailPage("odbc", STR_PAGETITLE_ADVANCED, ODriversSettings::CreateODBC);
            break;

        case  ::dbaccess::DST_MYSQL_ODBC:
            addDetailPage("mysqlodbc", STR_PAGETITLE_ADVANCED, ODriversSettings::CreateMySQLODBC);
            break;

        case  ::dbaccess::DST_MYSQL_JDBC:
            addDetailPage("mysqljdbc", STR_PAGETITLE_ADVANCED, ODriversSettings::CreateMySQLJDBC);
            break;

        case  ::dbaccess::DST_ORACLE_JDBC:
            addDetailPage("oraclejdbc", STR_PAGETITLE_ADVANCED, ODriversSettings::CreateOracleJDBC);
            break;

        case  ::dbaccess::DST_LDAP:
            addDetailPage("ldap",STR_PAGETITLE_ADVANCED,ODriversSettings::CreateLDAP);
            break;
        case  ::dbaccess::DST_USERDEFINE1:  /// first user defined driver
        case  ::dbaccess::DST_USERDEFINE2:
        case  ::dbaccess::DST_USERDEFINE3:
        case  ::dbaccess::DST_USERDEFINE4:
        case  ::dbaccess::DST_USERDEFINE5:
        case  ::dbaccess::DST_USERDEFINE6:
        case  ::dbaccess::DST_USERDEFINE7:
        case  ::dbaccess::DST_USERDEFINE8:
        case  ::dbaccess::DST_USERDEFINE9:
        case  ::dbaccess::DST_USERDEFINE10:
            {
                OUString aTitle(DBA_RES(STR_PAGETITLE_ADVANCED));
                AddTabPage("user" + OString(static_cast<int>(eType)), aTitle, ODriversSettings::CreateUser);
            }
            break;
        default:
            break;
    }
}

void ODbAdminDialog::impl_resetPages(const Reference< XPropertySet >& _rxDatasource)
{
    // the selection is valid if and only if we have a datasource now
    GetInputSetImpl()->Put(SfxBoolItem(DSID_INVALID_SELECTION, !_rxDatasource.is()));
        // (sal_False tells the tab pages to disable and reset all their controls, which is different
        // from "just set them to readonly")

    // reset the pages

    // prevent flicker
    m_xDialog->freeze();

    // remove all items which relate to indirect properties from the input set
    // (without this, the following may happen: select an arbitrary data source where some indirect properties
    // are set. Select another data source of the same type, where the indirect props are not set (yet). Then,
    // the indirect property values of the first ds are shown in the second ds ...)
    const ODbDataSourceAdministrationHelper::MapInt2String& rMap = m_pImpl->getIndirectProperties();
    for (auto const& elem : rMap)
        GetInputSetImpl()->ClearItem( static_cast<sal_uInt16>(elem.first) );

    // extract all relevant data from the property set of the data source
    m_pImpl->translateProperties(_rxDatasource, *GetInputSetImpl());

    // reset the example set
    m_xExampleSet.reset(new SfxItemSet(*GetInputSetImpl()));

    // special case: MySQL Native does not have the generic "advanced" page

    const DbuTypeCollectionItem& rCollectionItem = dynamic_cast<const DbuTypeCollectionItem&>(*getOutputSet()->GetItem(DSID_TYPECOLLECTION));
    ::dbaccess::ODsnTypeCollection* pCollection = rCollectionItem.getCollection();
    if ( pCollection->determineType(getDatasourceType( *m_xExampleSet )) == ::dbaccess::DST_MYSQL_NATIVE )
    {
        OString sMySqlNative("mysqlnative");
        AddTabPage(sMySqlNative, DBA_RES(STR_PAGETITLE_CONNECTION), ODriversSettings::CreateMySQLNATIVE);
        RemoveTabPage("advanced");
        m_sMainPageID = sMySqlNative;
    }

    SetCurPageId(m_sMainPageID);
    SfxTabPage* pConnectionPage = GetTabPage(m_sMainPageID);
    if ( pConnectionPage )
        pConnectionPage->Reset(GetInputSetImpl());
    // if this is NULL, the page has not been created yet, which means we're called before the
    // dialog was displayed (probably from inside the ctor)

    m_xDialog->thaw();
}

void ODbAdminDialog::setTitle(const OUString& rTitle)
{
    m_xDialog->set_title(rTitle);
}

void ODbAdminDialog::enableConfirmSettings( bool ) {}

void ODbAdminDialog::saveDatasource()
{
    PrepareLeaveCurrentPage();
}

ODbAdminDialog::ApplyResult ODbAdminDialog::implApplyChanges()
{
    if (!PrepareLeaveCurrentPage())
    {   // the page did not allow us to leave
        return AR_KEEP;
    }

    if ( !m_pImpl->saveChanges(*m_xExampleSet) )
        return AR_KEEP;

    return AR_LEAVE_MODIFIED;
}

void ODbAdminDialog::selectDataSource(const css::uno::Any& _aDataSourceName)
{
    impl_selectDataSource(_aDataSourceName);
}

const SfxItemSet* ODbAdminDialog::getOutputSet() const
{
    return GetExampleSet();
}

SfxItemSet* ODbAdminDialog::getWriteOutputSet()
{
    return m_xExampleSet.get();
}

std::pair< Reference<XConnection>,bool> ODbAdminDialog::createConnection()
{
    return m_pImpl->createConnection();
}

Reference< XComponentContext > ODbAdminDialog::getORB() const
{
    return m_pImpl->getORB();
}

Reference< XDriver > ODbAdminDialog::getDriver()
{
    return m_pImpl->getDriver();
}

OUString ODbAdminDialog::getDatasourceType(const SfxItemSet& _rSet) const
{
    return dbaui::ODbDataSourceAdministrationHelper::getDatasourceType(_rSet);
}

void ODbAdminDialog::clearPassword()
{
    m_pImpl->clearPassword();
}

void ODbAdminDialog::createItemSet(std::unique_ptr<SfxItemSet>& _rpSet, SfxItemPool*& _rpPool, std::vector<SfxPoolItem*>*& _rpDefaults, ::dbaccess::ODsnTypeCollection* _pTypeCollection)
{
    // just to be sure ....
    _rpSet = nullptr;
    _rpPool = nullptr;
    _rpDefaults = nullptr;

    const OUString sFilterAll( "%" );
    // create and initialize the defaults
    _rpDefaults = new std::vector<SfxPoolItem*>(DSID_LAST_ITEM_ID - DSID_FIRST_ITEM_ID + 1);
    SfxPoolItem** pCounter = _rpDefaults->data();  // want to modify this without affecting the out param _rppDefaults
    *pCounter++ = new SfxStringItem(DSID_NAME, OUString());
    *pCounter++ = new SfxStringItem(DSID_ORIGINALNAME, OUString());
    *pCounter++ = new SfxStringItem(DSID_CONNECTURL, OUString());
    *pCounter++ = new OStringListItem(DSID_TABLEFILTER, Sequence< OUString >(&sFilterAll, 1));
    *pCounter++ = new DbuTypeCollectionItem(DSID_TYPECOLLECTION, _pTypeCollection);
    *pCounter++ = new SfxBoolItem(DSID_INVALID_SELECTION, false);
    *pCounter++ = new SfxBoolItem(DSID_READONLY, false);
    *pCounter++ = new SfxStringItem(DSID_USER, OUString());
    *pCounter++ = new SfxStringItem(DSID_PASSWORD, OUString());
    *pCounter++ = new SfxStringItem(DSID_ADDITIONALOPTIONS, OUString());
    *pCounter++ = new SfxStringItem(DSID_CHARSET, OUString());
    *pCounter++ = new SfxBoolItem(DSID_PASSWORDREQUIRED, false);
    *pCounter++ = new SfxBoolItem(DSID_SHOWDELETEDROWS, false);
    *pCounter++ = new SfxBoolItem(DSID_ALLOWLONGTABLENAMES, false);
    *pCounter++ = new SfxStringItem(DSID_JDBCDRIVERCLASS, OUString());
    *pCounter++ = new SfxStringItem(DSID_FIELDDELIMITER, OUString(','));
    *pCounter++ = new SfxStringItem(DSID_TEXTDELIMITER, OUString('"'));
    *pCounter++ = new SfxStringItem(DSID_DECIMALDELIMITER, OUString('.'));
    *pCounter++ = new SfxStringItem(DSID_THOUSANDSDELIMITER, OUString());
    *pCounter++ = new SfxStringItem(DSID_TEXTFILEEXTENSION, OUString("txt"));
    *pCounter++ = new SfxBoolItem(DSID_TEXTFILEHEADER, true);
    *pCounter++ = new SfxBoolItem(DSID_PARAMETERNAMESUBST, false);
    *pCounter++ = new SfxInt32Item(DSID_CONN_PORTNUMBER, 8100);
    *pCounter++ = new SfxBoolItem(DSID_SUPPRESSVERSIONCL, false);
    *pCounter++ = new SfxBoolItem(DSID_CONN_SHUTSERVICE, false);
    *pCounter++ = new SfxInt32Item(DSID_CONN_DATAINC, 20);
    *pCounter++ = new SfxInt32Item(DSID_CONN_CACHESIZE, 20);
    *pCounter++ = new SfxStringItem(DSID_CONN_CTRLUSER, OUString());
    *pCounter++ = new SfxStringItem(DSID_CONN_CTRLPWD, OUString());
    *pCounter++ = new SfxBoolItem(DSID_USECATALOG, false);
    *pCounter++ = new SfxStringItem(DSID_CONN_HOSTNAME, OUString());
    *pCounter++ = new SfxStringItem(DSID_CONN_LDAP_BASEDN, OUString());
    *pCounter++ = new SfxInt32Item(DSID_CONN_LDAP_PORTNUMBER, 389);
    *pCounter++ = new SfxInt32Item(DSID_CONN_LDAP_ROWCOUNT, 100);
    *pCounter++ = new SfxBoolItem(DSID_SQL92CHECK, false);
    *pCounter++ = new SfxStringItem(DSID_AUTOINCREMENTVALUE, OUString());
    *pCounter++ = new SfxStringItem(DSID_AUTORETRIEVEVALUE, OUString());
    *pCounter++ = new SfxBoolItem(DSID_AUTORETRIEVEENABLED, false);
    *pCounter++ = new SfxBoolItem(DSID_APPEND_TABLE_ALIAS, false);
    *pCounter++ = new SfxInt32Item(DSID_MYSQL_PORTNUMBER, 3306);
    *pCounter++ = new SfxBoolItem(DSID_IGNOREDRIVER_PRIV, true);
    *pCounter++ = new SfxInt32Item(DSID_BOOLEANCOMPARISON, 0);
    *pCounter++ = new SfxInt32Item(DSID_ORACLE_PORTNUMBER, 1521);
    *pCounter++ = new SfxBoolItem(DSID_ENABLEOUTERJOIN, true);
    *pCounter++ = new SfxBoolItem(DSID_CATALOG, true);
    *pCounter++ = new SfxBoolItem(DSID_SCHEMA, true);
    *pCounter++ = new SfxBoolItem(DSID_INDEXAPPENDIX, true);
    *pCounter++ = new SfxBoolItem(DSID_CONN_LDAP_USESSL, false);
    *pCounter++ = new SfxStringItem(DSID_DOCUMENT_URL, OUString());
    *pCounter++ = new SfxBoolItem(DSID_DOSLINEENDS, false);
    *pCounter++ = new SfxStringItem(DSID_DATABASENAME, OUString());
    *pCounter++ = new SfxBoolItem(DSID_AS_BEFORE_CORRNAME, false);
    *pCounter++ = new SfxBoolItem(DSID_CHECK_REQUIRED_FIELDS, true);
    *pCounter++ = new SfxBoolItem(DSID_IGNORECURRENCY, false);
    *pCounter++ = new SfxStringItem(DSID_CONN_SOCKET, OUString());
    *pCounter++ = new SfxBoolItem(DSID_ESCAPE_DATETIME, true);
    *pCounter++ = new SfxStringItem(DSID_NAMED_PIPE, OUString());
    *pCounter++ = new OptionalBoolItem( DSID_PRIMARY_KEY_SUPPORT );
    *pCounter++ = new SfxInt32Item(DSID_MAX_ROW_SCAN, 100);
    *pCounter++ = new SfxBoolItem( DSID_RESPECTRESULTSETTYPE,false );

    // create the pool
    static SfxItemInfo const aItemInfos[DSID_LAST_ITEM_ID - DSID_FIRST_ITEM_ID + 1] =
    {
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
        {0,false},
    };

    OSL_ENSURE(SAL_N_ELEMENTS(aItemInfos) == DSID_LAST_ITEM_ID,"Invalid Ids!");
    _rpPool = new SfxItemPool("DSAItemPool", DSID_FIRST_ITEM_ID, DSID_LAST_ITEM_ID,
        aItemInfos, _rpDefaults);
    _rpPool->FreezeIdRanges();

    // and, finally, the set
    _rpSet.reset(new SfxItemSet(*_rpPool));
}

void ODbAdminDialog::destroyItemSet(std::unique_ptr<SfxItemSet>& _rpSet, SfxItemPool*& _rpPool, std::vector<SfxPoolItem*>*& _rpDefaults)
{
    // _first_ delete the set (referring the pool)
    _rpSet.reset();

    // delete the pool
    if (_rpPool)
    {
        _rpPool->ReleaseDefaults(true);
            // the "true" means delete the items, too
        SfxItemPool::Free(_rpPool);
        _rpPool = nullptr;
    }

    // reset the defaults ptr
    _rpDefaults = nullptr;
        // no need to explicitly delete the defaults, this has been done by the ReleaseDefaults
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
