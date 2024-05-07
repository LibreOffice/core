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
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <core_resource.hxx>
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
using namespace com::sun::star::beans;

// ODbAdminDialog
ODbAdminDialog::ODbAdminDialog(weld::Window* pParent,
                               SfxItemSet const * _pItems,
                               const Reference< XComponentContext >& _rxContext)
    : SfxTabDialogController(pParent, u"dbaccess/ui/admindialog.ui"_ustr, u"AdminDialog"_ustr, _pItems)
    , m_sMainPageID(u"advanced"_ustr)
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

void ODbAdminDialog::PageCreated(const OUString& rId, SfxTabPage& _rPage)
{
    // register ourself as modified listener
    static_cast<OGenericAdministrationPage&>(_rPage).SetServiceFactory( getORB() );
    static_cast<OGenericAdministrationPage&>(_rPage).SetAdminDialog(this,this);

    SfxTabDialogController::PageCreated(rId, _rPage);
}

void ODbAdminDialog::addDetailPage(const OUString& rPageId, TranslateId pTextId, CreateTabPage pCreateFunc)
{
    AddTabPage(rPageId, DBA_RES(pTextId), pCreateFunc);
}

void ODbAdminDialog::impl_selectDataSource(const css::uno::Any& _aDataSourceName)
{
    m_pImpl->setDataSourceOrName(_aDataSourceName);
    Reference< XPropertySet > xDatasource = m_pImpl->getCurrentDataSource();
    impl_resetPages( xDatasource );

    const DbuTypeCollectionItem* pCollectionItem = dynamic_cast<const DbuTypeCollectionItem*>(getOutputSet()->GetItem(DSID_TYPECOLLECTION));
    assert(pCollectionItem && "must exist");
    ::dbaccess::ODsnTypeCollection* pCollection = pCollectionItem->getCollection();
    ::dbaccess::DATASOURCE_TYPE eType = pCollection->determineType(getDatasourceType(*getOutputSet()));

    // and insert the new ones
    switch ( eType )
    {
        case  ::dbaccess::DST_DBASE:
            addDetailPage(u"dbase"_ustr, STR_PAGETITLE_ADVANCED, ODriversSettings::CreateDbase);
            break;

        case  ::dbaccess::DST_ADO:
            addDetailPage(u"ado"_ustr, STR_PAGETITLE_ADVANCED, ODriversSettings::CreateAdo);
            break;

        case  ::dbaccess::DST_FLAT:
            addDetailPage(u"text"_ustr, STR_PAGETITLE_ADVANCED, ODriversSettings::CreateText);
            break;

        case  ::dbaccess::DST_ODBC:
            addDetailPage(u"odbc"_ustr, STR_PAGETITLE_ADVANCED, ODriversSettings::CreateODBC);
            break;

        case  ::dbaccess::DST_MYSQL_ODBC:
            addDetailPage(u"mysqlodbc"_ustr, STR_PAGETITLE_ADVANCED, ODriversSettings::CreateMySQLODBC);
            break;

        case  ::dbaccess::DST_MYSQL_JDBC:
            addDetailPage(u"mysqljdbc"_ustr, STR_PAGETITLE_ADVANCED, ODriversSettings::CreateMySQLJDBC);
            break;

        case  ::dbaccess::DST_ORACLE_JDBC:
            addDetailPage(u"oraclejdbc"_ustr, STR_PAGETITLE_ADVANCED, ODriversSettings::CreateOracleJDBC);
            break;

        case  ::dbaccess::DST_LDAP:
            addDetailPage(u"ldap"_ustr,STR_PAGETITLE_ADVANCED,ODriversSettings::CreateLDAP);
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
                AddTabPage("user" + OUString::number(eType - dbaccess::DST_USERDEFINE1 + 1), aTitle, ODriversSettings::CreateUser);
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

    const DbuTypeCollectionItem* pCollectionItem = dynamic_cast<const DbuTypeCollectionItem*>(getOutputSet()->GetItem(DSID_TYPECOLLECTION));
    assert(pCollectionItem && "must exist");
    ::dbaccess::ODsnTypeCollection* pCollection = pCollectionItem->getCollection();
    if ( pCollection->determineType(getDatasourceType( *m_xExampleSet )) == ::dbaccess::DST_MYSQL_NATIVE )
    {
        OUString sMySqlNative(u"mysqlnative"_ustr);
        AddTabPage(sMySqlNative, DBA_RES(STR_PAGETITLE_CONNECTION), ODriversSettings::CreateMySQLNATIVE);
        RemoveTabPage(u"advanced"_ustr);
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

static ItemInfoPackage& getItemInfoPackageAdminDlg()
{
    class ItemInfoPackageAdminDlg : public ItemInfoPackage
    {
        typedef std::array<ItemInfoStatic, DSID_LAST_ITEM_ID - DSID_FIRST_ITEM_ID + 1> ItemInfoArrayAdminDlg;
        ItemInfoArrayAdminDlg maItemInfos {{
            // m_nWhich, m_pItem, m_nSlotID, m_nItemInfoFlags
            { DSID_NAME, new SfxStringItem(DSID_NAME, OUString()), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_ORIGINALNAME, new SfxStringItem(DSID_ORIGINALNAME, OUString()), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_CONNECTURL, new SfxStringItem(DSID_CONNECTURL, OUString()), 0, SFX_ITEMINFOFLAG_NONE },

            // gets added in constructor below once for LO runtime as static default
            { DSID_TABLEFILTER, nullptr, 0, SFX_ITEMINFOFLAG_NONE },

            // gets added by callback for each new Pool as dynamic default
            { DSID_TYPECOLLECTION, nullptr, 0, SFX_ITEMINFOFLAG_NONE },

            { DSID_INVALID_SELECTION, new SfxBoolItem(DSID_INVALID_SELECTION, false), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_READONLY, new SfxBoolItem(DSID_READONLY, false), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_USER, new SfxStringItem(DSID_USER, OUString()), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_PASSWORD, new SfxStringItem(DSID_PASSWORD, OUString()), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_ADDITIONALOPTIONS, new SfxStringItem(DSID_ADDITIONALOPTIONS, OUString()), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_CHARSET, new SfxStringItem(DSID_CHARSET, OUString()), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_PASSWORDREQUIRED, new SfxBoolItem(DSID_PASSWORDREQUIRED, false), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_SHOWDELETEDROWS, new SfxBoolItem(DSID_SHOWDELETEDROWS, false), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_ALLOWLONGTABLENAMES, new SfxBoolItem(DSID_ALLOWLONGTABLENAMES, false), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_JDBCDRIVERCLASS, new SfxStringItem(DSID_JDBCDRIVERCLASS, OUString()), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_FIELDDELIMITER, new SfxStringItem(DSID_FIELDDELIMITER, OUString(',')), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_TEXTDELIMITER, new SfxStringItem(DSID_TEXTDELIMITER, OUString('"')), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_DECIMALDELIMITER, new SfxStringItem(DSID_DECIMALDELIMITER, OUString('.')), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_THOUSANDSDELIMITER, new SfxStringItem(DSID_THOUSANDSDELIMITER, OUString()), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_TEXTFILEEXTENSION, new SfxStringItem(DSID_TEXTFILEEXTENSION, u"txt"_ustr), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_TEXTFILEHEADER, new SfxBoolItem(DSID_TEXTFILEHEADER, true), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_PARAMETERNAMESUBST, new SfxBoolItem(DSID_PARAMETERNAMESUBST, false), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_CONN_PORTNUMBER, new SfxInt32Item(DSID_CONN_PORTNUMBER, 8100), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_SUPPRESSVERSIONCL, new SfxBoolItem(DSID_SUPPRESSVERSIONCL, false), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_CONN_SHUTSERVICE, new SfxBoolItem(DSID_CONN_SHUTSERVICE, false), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_CONN_DATAINC, new SfxInt32Item(DSID_CONN_DATAINC, 20), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_CONN_CACHESIZE, new SfxInt32Item(DSID_CONN_CACHESIZE, 20), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_CONN_CTRLUSER, new SfxStringItem(DSID_CONN_CTRLUSER, OUString()), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_CONN_CTRLPWD, new SfxStringItem(DSID_CONN_CTRLPWD, OUString()), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_USECATALOG, new SfxBoolItem(DSID_USECATALOG, false), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_CONN_HOSTNAME, new SfxStringItem(DSID_CONN_HOSTNAME, OUString()), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_CONN_LDAP_BASEDN, new SfxStringItem(DSID_CONN_LDAP_BASEDN, OUString()), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_CONN_LDAP_PORTNUMBER, new SfxInt32Item(DSID_CONN_LDAP_PORTNUMBER, 389), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_CONN_LDAP_ROWCOUNT, new SfxInt32Item(DSID_CONN_LDAP_ROWCOUNT, 100), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_SQL92CHECK, new SfxBoolItem(DSID_SQL92CHECK, false), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_AUTOINCREMENTVALUE, new SfxStringItem(DSID_AUTOINCREMENTVALUE, OUString()), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_AUTORETRIEVEVALUE, new SfxStringItem(DSID_AUTORETRIEVEVALUE, OUString()), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_AUTORETRIEVEENABLED, new SfxBoolItem(DSID_AUTORETRIEVEENABLED, false), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_APPEND_TABLE_ALIAS, new SfxBoolItem(DSID_APPEND_TABLE_ALIAS, false), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_MYSQL_PORTNUMBER, new SfxInt32Item(DSID_MYSQL_PORTNUMBER, 3306), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_IGNOREDRIVER_PRIV, new SfxBoolItem(DSID_IGNOREDRIVER_PRIV, true), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_BOOLEANCOMPARISON, new SfxInt32Item(DSID_BOOLEANCOMPARISON, 0), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_ORACLE_PORTNUMBER, new SfxInt32Item(DSID_ORACLE_PORTNUMBER, 1521), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_ENABLEOUTERJOIN, new SfxBoolItem(DSID_ENABLEOUTERJOIN, true), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_CATALOG, new SfxBoolItem(DSID_CATALOG, true), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_SCHEMA, new SfxBoolItem(DSID_SCHEMA, true), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_INDEXAPPENDIX, new SfxBoolItem(DSID_INDEXAPPENDIX, true), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_CONN_LDAP_USESSL, new SfxBoolItem(DSID_CONN_LDAP_USESSL, false), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_DOCUMENT_URL, new SfxStringItem(DSID_DOCUMENT_URL, OUString()), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_DOSLINEENDS, new SfxBoolItem(DSID_DOSLINEENDS, false), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_DATABASENAME, new SfxStringItem(DSID_DATABASENAME, OUString()), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_AS_BEFORE_CORRNAME, new SfxBoolItem(DSID_AS_BEFORE_CORRNAME, false), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_CHECK_REQUIRED_FIELDS, new SfxBoolItem(DSID_CHECK_REQUIRED_FIELDS, true), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_IGNORECURRENCY, new SfxBoolItem(DSID_IGNORECURRENCY, false), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_CONN_SOCKET, new SfxStringItem(DSID_CONN_SOCKET, OUString()), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_ESCAPE_DATETIME, new SfxBoolItem(DSID_ESCAPE_DATETIME, true), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_NAMED_PIPE, new SfxStringItem(DSID_NAMED_PIPE, OUString()), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_PRIMARY_KEY_SUPPORT, new OptionalBoolItem( DSID_PRIMARY_KEY_SUPPORT ), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_MAX_ROW_SCAN, new SfxInt32Item(DSID_MAX_ROW_SCAN, 100), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_RESPECTRESULTSETTYPE, new SfxBoolItem( DSID_RESPECTRESULTSETTYPE,false ), 0, SFX_ITEMINFOFLAG_NONE },
            { DSID_POSTGRES_PORTNUMBER, new SfxInt32Item(DSID_POSTGRES_PORTNUMBER, 5432), 0, SFX_ITEMINFOFLAG_NONE }
        }};

        virtual const ItemInfoStatic& getItemInfoStatic(size_t nIndex) const override { return maItemInfos[nIndex]; }

    public:
        ItemInfoPackageAdminDlg()
        {
            static constexpr OUString sFilterAll( u"%"_ustr );
            setItemAtItemInfoStatic(
                new OStringListItem(DSID_TABLEFILTER, Sequence< OUString >{sFilterAll}),
                maItemInfos[DSID_TABLEFILTER - DSID_FIRST_ITEM_ID]);
        }

        virtual size_t size() const override { return maItemInfos.size(); }
        virtual const ItemInfo& getItemInfo(size_t nIndex, SfxItemPool& /*rPool*/) override { return maItemInfos[nIndex]; }
    };

    static std::unique_ptr<ItemInfoPackageAdminDlg> g_aItemInfoPackageAdminDlg;
    if (!g_aItemInfoPackageAdminDlg)
        g_aItemInfoPackageAdminDlg.reset(new ItemInfoPackageAdminDlg);
    return *g_aItemInfoPackageAdminDlg;
}

void ODbAdminDialog::createItemSet(std::unique_ptr<SfxItemSet>& _rpSet, rtl::Reference<SfxItemPool>& _rpPool, ::dbaccess::ODsnTypeCollection* _pTypeCollection)
{
    // just to be sure...
    _rpSet = nullptr;
    _rpPool = nullptr;
    _rpPool = new SfxItemPool(u"DSAItemPool"_ustr);

    // here we have to use the callback to create all needed default entries since
    // the DSID_TYPECOLLECTION needs the local given _pTypeCollection. Thus this will
    // be a ItemInfoDynamic created by SfxItemPool::registerItemInfoPackage. That
    // (and the contained Item) will be owned by the Pool and cleaned up when it goes
    // down (see SfxItemPool::cleanupItemInfos())
    _rpPool->registerItemInfoPackage(
        getItemInfoPackageAdminDlg(),
        [&_pTypeCollection](sal_uInt16 nWhich)
        {
            SfxPoolItem* pRetval(nullptr);
            if (DSID_TYPECOLLECTION == nWhich)
                pRetval = new DbuTypeCollectionItem(DSID_TYPECOLLECTION, _pTypeCollection);
            return pRetval;
        });

    // and, finally, the set
    _rpSet.reset(new SfxItemSet(*_rpPool));
}

void ODbAdminDialog::destroyItemSet(std::unique_ptr<SfxItemSet>& _rpSet, rtl::Reference<SfxItemPool>& _rpPool)
{
    // _first_ delete the set (referring the pool)
    _rpSet.reset();

    // delete the pool
    _rpPool = nullptr;
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
