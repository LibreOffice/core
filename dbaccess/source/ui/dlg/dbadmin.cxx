/*************************************************************************
 *
 *  $RCSfile: dbadmin.cxx,v $
 *
 *  $Revision: 1.89 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:44:16 $
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
#ifndef _DBU_DLG_HRC_
#include "dbu_dlg.hrc"
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
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _DBAUI_ADMINPAGES_HXX_
#include "adminpages.hxx"
#endif
#ifndef _DBAUI_LOCALRESACCESS_HXX_
#include "localresaccess.hxx"
#endif
#ifndef _DBAUI_STRINGLISTITEM_HXX_
#include "stringlistitem.hxx"
#endif
#ifndef _DBAUI_PROPERTYSETITEM_HXX_
#include "propertysetitem.hxx"
#endif
#ifndef _UNOTOOLS_CONFIGNODE_HXX_
#include <unotools/confignode.hxx>
#endif
#ifndef DBAUI_CONNECTIONPAGE_HXX
#include "ConnectionPage.hxx"
#endif
#ifndef DBAUI_ADVANCEDPAGEDLG_HXX
#include "AdvancedPageDlg.hxx"
#endif
#ifndef DBAUI_DRIVERSETTINGS_HXX
#include "DriverSettings.hxx"
#endif
#ifndef _DBAUI_DBADMINIMPL_HXX_
#include "DbAdminImpl.hxx"
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................
using namespace dbtools;
using namespace com::sun::star::uno;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::lang;
using namespace com::sun::star::util;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;

//=========================================================================
//= ODbAdminDialog
//=========================================================================
//-------------------------------------------------------------------------
ODbAdminDialog::ODbAdminDialog(Window* _pParent
                               , SfxItemSet* _pItems
                               , const Reference< XMultiServiceFactory >& _rxORB
                               )
    :SfxTabDialog(_pParent, ModuleRes(DLG_DATABASE_ADMINISTRATION), _pItems)
    ,m_bResetting(sal_False)
    ,m_bApplied(sal_False)
    ,m_bUIEnabled( sal_True )
{
    m_pImpl = ::std::auto_ptr<ODbDataSourceAdministrationHelper>(new ODbDataSourceAdministrationHelper(_rxORB,this,this));
    // add the initial tab page
    AddTabPage(PAGE_CONNECTION, String(ResId(STR_PAGETITLE_GENERAL)), OConnectionTabPage::Create, NULL);

    // remove the reset button - it's meaning is much too ambiguous in this dialog
    RemoveResetButton();
    // no local resources needed anymore
    FreeResource();
}

//-------------------------------------------------------------------------
ODbAdminDialog::~ODbAdminDialog()
{
    SetInputSet(NULL);
    DELETEZ(pExampleSet);
}
//-------------------------------------------------------------------------
short ODbAdminDialog::Ok()
{
    SfxTabDialog::Ok();
    disabledUI();
    return ( AR_LEAVE_MODIFIED == implApplyChanges() ) ? RET_OK : RET_CANCEL;
        // TODO : AR_ERROR is not handled correctly, we always close the dialog here
}

//-------------------------------------------------------------------------
void ODbAdminDialog::PageCreated(USHORT _nId, SfxTabPage& _rPage)
{
    // register ourself as modified listener
    static_cast<OGenericAdministrationPage&>(_rPage).SetServiceFactory(m_pImpl->getORB());
    static_cast<OGenericAdministrationPage&>(_rPage).SetAdminDialog(this,this);

    AdjustLayout();
    Window *pWin = GetViewWindow();
    if(pWin)
        pWin->Invalidate();

    SfxTabDialog::PageCreated(_nId, _rPage);
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

    AddTabPage(_nPageId, String(ResId(_nTextId)), _pCreateFunc, NULL);
    m_aCurrentDetailPages.push(_nPageId);
}

//-------------------------------------------------------------------------
void ODbAdminDialog::implSelectDatasource(const ::com::sun::star::uno::Any& _aDataSourceName)
{
    m_pImpl->setCurrentDataSourceName(_aDataSourceName);

    // reset the tag pages
    Reference< XPropertySet > xDatasource = m_pImpl->getCurrentDataSource();
    resetPages(xDatasource);

    DATASOURCE_TYPE eType = getDatasourceType(*getOutputSet());

    // and insert the new ones
    switch ( eType )
    {
        case DST_DBASE:
            addDetailPage(PAGE_DBASE, STR_PAGETITLE_ADVANCED, ODriversSettings::CreateDbase);
            //  bResetPasswordRequired = sal_True;
            break;

        case DST_ADO:
            addDetailPage(PAGE_ADO, STR_PAGETITLE_ADVANCED, ODriversSettings::CreateAdo);
            break;

        case DST_FLAT:
            addDetailPage(PAGE_TEXT, STR_PAGETITLE_ADVANCED, ODriversSettings::CreateText);
            //  bResetPasswordRequired = sal_True;
            break;

        case DST_ODBC:
            addDetailPage(PAGE_ODBC, STR_PAGETITLE_ADVANCED, ODriversSettings::CreateODBC);
            break;

        case DST_MYSQL_ODBC:
            addDetailPage(PAGE_MYSQL_ODBC, STR_PAGETITLE_ADVANCED, ODriversSettings::CreateMySQLODBC);
            break;
        case DST_MYSQL_JDBC:
            addDetailPage(PAGE_MYSQL_JDBC, STR_PAGETITLE_ADVANCED, ODriversSettings::CreateMySQLJDBC);
            break;
        case DST_ORACLE_JDBC:
            addDetailPage(PAGE_ORACLE_JDBC, STR_PAGETITLE_ADVANCED, ODriversSettings::CreateOracleJDBC);
            break;

        case DST_ADABAS:
            // for adabas we have more than one page
            // CAUTION: the order of inserting pages matters.
            // the major detail page should be inserted last always (thus, it becomes the first page after
            // the general page)
            addDetailPage(PAGE_ADABAS, STR_PAGETITLE_ADVANCED, ODriversSettings::CreateAdabas);
            break;

        case DST_LDAP       : addDetailPage(PAGE_LDAP,STR_PAGETITLE_ADVANCED,ODriversSettings::CreateLDAP);break;       /// LDAP
        case DST_USERDEFINE1:   /// first user defined driver
        case DST_USERDEFINE2:
        case DST_USERDEFINE3:
        case DST_USERDEFINE4:
        case DST_USERDEFINE5:
        case DST_USERDEFINE6:
        case DST_USERDEFINE7:
        case DST_USERDEFINE8:
        case DST_USERDEFINE9:
        case DST_USERDEFINE10:
            {
                OLocalResourceAccess aDummy(DLG_DATABASE_ADMINISTRATION, RSC_TABDIALOG);
                String aTitle(ResId(STR_PAGETITLE_ADVANCED));
                AddTabPage(PAGE_USERDRIVER, aTitle, ODriversSettings::CreateUser, 0, sal_False, 1);
                m_aCurrentDetailPages.push(PAGE_USERDRIVER);
            }
            break;
    }
}

//-------------------------------------------------------------------------
void ODbAdminDialog::resetPages(const Reference< XPropertySet >& _rxDatasource)
{
    // the selection is valid if and only if we have a datasource now
    GetInputSetImpl()->Put(SfxBoolItem(DSID_INVALID_SELECTION, !_rxDatasource.is()));
        // (sal_False tells the tab pages to disable and reset all their controls, which is different
        // from "just set them to readonly")

    // reset the pages

    sal_uInt16 nOldSelectedPage = GetCurPageId();

    // prevent flicker
    SetUpdateMode(sal_False);

    m_bResetting = sal_True;
    ShowPage(PAGE_CONNECTION);
    m_bResetting = sal_False;

    // remove all tab pages (except the general one)
    // remove all current detail pages
    removeDetailPages();

    // remove all items which relate to indirect properties from the input set
    // (without this, the following may happen: select an arbitrary data source where some indirect properties
    // are set. Select another data source of the same type, where the indirect props are not set (yet). Then,
    // the indirect property values of the first ds are shown in the second ds ...)
    const ODbDataSourceAdministrationHelper::MapInt2String& rMap = m_pImpl->getIndirectProperties();
    for (   ODbDataSourceAdministrationHelper::ConstMapInt2StringIterator aIndirect = rMap.begin();
            aIndirect != rMap.end();
            ++aIndirect
        )
        GetInputSetImpl()->ClearItem( (sal_uInt16)aIndirect->first );

    // extract all relevant data from the property set of the data source
    m_pImpl->translateProperties(_rxDatasource, *GetInputSetImpl());

    // propagate this set as our new input set and reset the example set
    SetInputSet(GetInputSetImpl());
    delete pExampleSet;
    pExampleSet = new SfxItemSet(*GetInputSetImpl());

    m_bResetting = sal_True;

    // unfortunately, I have no chance if a page with ID nOldSelectedPage still exists
    // So we first select the general page (which is always available) and the the old page (which may not be there)

    ShowPage( PAGE_CONNECTION );
    SfxTabPage* pConnectionPage = GetTabPage(PAGE_CONNECTION);
    if ( pConnectionPage )
        pConnectionPage->Reset(*GetInputSetImpl());
    // if this is NULL, the page has not been created yet, which means we're called before the
    // dialog was displayed (probably from inside the ctor)

    if ( isUIEnabled() )
    {
        ShowPage( nOldSelectedPage );
        // same for the previously selected page, if it is still there
        SfxTabPage* pOldPage = GetTabPage( nOldSelectedPage );
        if (pOldPage)
            pOldPage->Reset(*GetInputSetImpl());
    }

    SetUpdateMode(sal_True);

    m_bResetting = sal_False;
}
// -----------------------------------------------------------------------------
void ODbAdminDialog::setTitle(const ::rtl::OUString& _sTitle)
{
    SetText(_sTitle);
}
//-------------------------------------------------------------------------
sal_Bool ODbAdminDialog::saveDatasource()
{
    return PrepareLeaveCurrentPage();
}
//-------------------------------------------------------------------------
ODbAdminDialog::ApplyResult ODbAdminDialog::implApplyChanges()
{
    if (!PrepareLeaveCurrentPage())
    {   // the page did not allow us to leave
        return AR_KEEP;
    }

    if ( !m_pImpl->saveChanges(*pExampleSet) )
        return AR_KEEP;

    if ( isUIEnabled() )
        ShowPage(GetCurPageId());
        // This does the usual ActivatePage, so the pages can save their current status.
        // This way, next time they're asked what has changed since now and here, they really
        // can compare with the status they have _now_ (not the one they had before this apply call).

    m_bApplied = sal_True;

    return AR_LEAVE_MODIFIED;
}
//-------------------------------------------------------------------------
void ODbAdminDialog::selectDataSource(const ::com::sun::star::uno::Any& _aDataSourceName)
{
    implSelectDatasource(_aDataSourceName);
}

// -----------------------------------------------------------------------------
const SfxItemSet* ODbAdminDialog::getOutputSet() const
{
    return GetExampleSet();
}
// -----------------------------------------------------------------------------
SfxItemSet* ODbAdminDialog::getWriteOutputSet()
{
    return pExampleSet;
}
// -----------------------------------------------------------------------------
Reference< XConnection > ODbAdminDialog::createConnection()
{
    return m_pImpl->createConnection();
}
// -----------------------------------------------------------------------------
Reference< XMultiServiceFactory > ODbAdminDialog::getORB()
{
    return m_pImpl->getORB();
}
// -----------------------------------------------------------------------------
Reference< XDriver > ODbAdminDialog::getDriver()
{
    return m_pImpl->getDriver();
}
// -----------------------------------------------------------------------------
DATASOURCE_TYPE ODbAdminDialog::getDatasourceType(const SfxItemSet& _rSet) const
{
    return m_pImpl->getDatasourceType(_rSet);
}
// -----------------------------------------------------------------------------
void ODbAdminDialog::clearPassword()
{
    m_pImpl->clearPassword();
}
//-------------------------------------------------------------------------
SfxItemSet* ODbAdminDialog::createItemSet(SfxItemSet*& _rpSet, SfxItemPool*& _rpPool, SfxPoolItem**& _rppDefaults, ODsnTypeCollection* _pTypeCollection)
{
    // just to be sure ....
    _rpSet = NULL;
    _rpPool = NULL;
    _rppDefaults = NULL;

    const ::rtl::OUString sFilterAll( "%", 1, RTL_TEXTENCODING_ASCII_US );
    // create and initialize the defaults
    _rppDefaults = new SfxPoolItem*[DSID_LAST_ITEM_ID - DSID_FIRST_ITEM_ID + 1];
    SfxPoolItem** pCounter = _rppDefaults;  // want to modify this without affecting the out param _rppDefaults
    *pCounter++ = new SfxStringItem(DSID_NAME, String());
    *pCounter++ = new SfxStringItem(DSID_ORIGINALNAME, String());
    *pCounter++ = new SfxStringItem(DSID_CONNECTURL, _pTypeCollection ? _pTypeCollection->getDatasourcePrefix(DST_JDBC) : String());
    *pCounter++ = new OStringListItem(DSID_TABLEFILTER, Sequence< ::rtl::OUString >(&sFilterAll, 1));
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
    *pCounter++ = new SfxBoolItem(DSID_PARAMETERNAMESUBST, sal_True);
    *pCounter++ = new SfxInt32Item(DSID_CONN_PORTNUMBER, 8100);
    *pCounter++ = new SfxBoolItem(DSID_SUPPRESSVERSIONCL, sal_False);
    *pCounter++ = new OPropertySetItem(DSID_DATASOURCE_UNO);
    *pCounter++ = new SfxBoolItem(DSID_CONN_SHUTSERVICE, sal_False);
    *pCounter++ = new SfxInt32Item(DSID_CONN_DATAINC, 20);
    *pCounter++ = new SfxInt32Item(DSID_CONN_CACHESIZE, 20);
    *pCounter++ = new SfxStringItem(DSID_CONN_CTRLUSER, String());
    *pCounter++ = new SfxStringItem(DSID_CONN_CTRLPWD, String());
    *pCounter++ = new SfxBoolItem(DSID_USECATALOG, sal_False);
    *pCounter++ = new SfxStringItem(DSID_CONN_HOSTNAME, String());
    *pCounter++ = new SfxStringItem(DSID_CONN_LDAP_BASEDN, String());
    *pCounter++ = new SfxInt32Item(DSID_CONN_LDAP_PORTNUMBER, 389);
    *pCounter++ = new SfxInt32Item(DSID_CONN_LDAP_ROWCOUNT, 100);
    *pCounter++ = new SfxBoolItem(DSID_SQL92CHECK, sal_False);
    *pCounter++ = new SfxStringItem(DSID_AUTOINCREMENTVALUE, String());
    *pCounter++ = new SfxStringItem(DSID_AUTORETRIEVEVALUE, String());
    *pCounter++ = new SfxBoolItem(DSID_AUTORETRIEVEENABLED, sal_False);
    *pCounter++ = new SfxBoolItem(DSID_APPEND_TABLE_ALIAS, sal_True);
    *pCounter++ = new SfxInt32Item(DSID_MYSQL_PORTNUMBER, 3306);
    *pCounter++ = new SfxBoolItem(DSID_IGNOREDRIVER_PRIV, sal_True);
    *pCounter++ = new SfxInt32Item(DSID_BOOLEANCOMPARISON, 0);
    *pCounter++ = new SfxInt32Item(DSID_ORACLE_PORTNUMBER, 1521);
    *pCounter++ = new SfxBoolItem(DSID_ENABLEOUTERJOIN, sal_True);
    *pCounter++ = new SfxBoolItem(DSID_CATALOG, sal_True);
    *pCounter++ = new SfxBoolItem(DSID_SCHEMA, sal_True);
    *pCounter++ = new SfxBoolItem(DSID_INDEXAPPENDIX, sal_True);
    *pCounter++ = new SfxBoolItem(DSID_CONN_LDAP_USESSL, sal_False);
    *pCounter++ = new SfxBoolItem(DSID_DOSLINEENDS, sal_False);


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

    OSL_ENSURE(sizeof(aItemInfos)/sizeof(aItemInfos[0]) == DSID_LAST_ITEM_ID,"Invlaid Ids!");
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
//.........................................................................
}   // namespace dbaui
//.........................................................................

