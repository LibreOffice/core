/*************************************************************************
 *
 *  $RCSfile: dbadmin.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-09 12:39:29 $
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
#ifndef _COM_SUN_STAR_UNO_XNAMINGSERVICE_HPP_
#include <com/sun/star/uno/XNamingService.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XFLUSHABLE_HPP_
#include <com/sun/star/util/XFlushable.hpp>
#endif
#ifndef _DBASHARED_STRINGCONSTANTS_HRC_
#include "stringconstants.hrc"
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
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

#define MODIFIED_MARKER     " (*)"

//.........................................................................
namespace dbaui
{
//.........................................................................

using namespace dbaccess;
using namespace com::sun::star::uno;
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
    ,m_xORB(_rxORB)
{
    // add the initial tab pages
    AddTabPage(PAGE_GENERAL, String(ResId(STR_PAGETITLE_GENERAL)), OGeneralPage::Create, NULL);
    AddTabPage(PAGE_TABLESUBSCRIPTION, String(ResId(STR_PAGETITLE_TABLESUBSCRIPTION)), OTableSubscriptionPage::Create, NULL);

    // no local resources needed anymore
    FreeResource();

    // register the view window
    SetViewWindow(&m_aSelector);
    SetViewAlign(WINDOWALIGN_LEFT);

    // do some knittings
    m_aSelector.setSelectHandler(LINK(this, ODbAdminDialog, OnDatasourceSelected));

    // create the DatabaseContext service
    DBG_ASSERT(m_xORB.is(), "ODbAdminDialog::ODbAdminDialog : need a service factory !");
    try
    {
        m_xDatabaseContext = Reference< XNameAccess >(m_xORB->createInstance(SERVICE_SDB_DATABASECONTEXT), UNO_QUERY);
    }
    catch(Exception&)
    {
    }

    ::rtl::OUString sInitialSelection;  // will be the initial selection

    if (!m_xDatabaseContext.is())
    {
        ShowServiceNotAvailableError(_pParent, String(SERVICE_SDB_DATABASECONTEXT), sal_True);
    }
    else
    {
        // fill the listbox with the names of the registered datasources
        Sequence< ::rtl::OUString > aDatasources = m_xDatabaseContext->getElementNames();
        const ::rtl::OUString* pDatasources = aDatasources.getConstArray();
        for (sal_Int32 i=0; i<aDatasources.getLength(); ++i, ++pDatasources)
            m_aSelector.insert(*pDatasources);

        if (!aDatasources.getLength())
        {
            WarningBox(_pParent, ModuleRes(ERR_NOREGISTEREDDATASOURCES)).Execute();
        }
        else
            sInitialSelection = aDatasources[0];
    }
    // TODO : get the initial selection from the configuration

    implSelectDatasource(sInitialSelection);
}

//-------------------------------------------------------------------------
ODbAdminDialog::~ODbAdminDialog()
{
    SetInputSet(NULL);
    DELETEZ(pExampleSet);
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
    m_aSelector.clearAllModified();

    return nResult;
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
    if (pUser)
        aReturn.push_back(
            PropertyValue(  ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("user")), 0,
                            makeAny(::rtl::OUString(pUser->GetValue())), PropertyState_DIRECT_VALUE));

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

        sal_uInt16 nResult = aDlg.Execute();
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

    // TODO: all the other stuff (charset etc.)

    _rDriverParam = Sequence< PropertyValue >(aReturn.begin(), aReturn.size());
    return sal_True;
}

//-------------------------------------------------------------------------
short ODbAdminDialog::Ok()
{
    short nResult = SfxTabDialog::Ok();
    if (RET_OK == nResult)
    {
        // save the settings for the currently selected data source
        sal_Int32 nCurrentlySelected = m_aSelector.getSelected();
        if (m_aSelector.isModified(nCurrentlySelected))
            m_aSelector.update(nCurrentlySelected, *GetExampleSet());

        // propagate all the settings made to the appropriate data source
        for (sal_Int32 i=0; i<m_aSelector.getCount(); ++i)
        {
            // nothing to do if no modifications were done
            if (m_aSelector.isModified(i))
            {
                String sDSName = m_aSelector.getOriginalName(i);
                Reference< XPropertySet > xDatasource = getDatasource(sDSName);
                if (xDatasource.is())
                {
                    // put the remembered settings into the property set
                    translateProperties(*m_aSelector.getItems(i), xDatasource);
                    // flush the data source
                    Reference< XFlushable > xFlushDatasource(xDatasource, UNO_QUERY);
                    if (!xFlushDatasource.is())
                    {
                        DBG_ERROR("ODbAdminDialog::Ok: the datasource should be flushable!");
                        continue;
                    }

                    try
                    {
                        xFlushDatasource->flush();
                    }
                    catch(RuntimeException&)
                    {
                        DBG_ERROR("ODbAdminDialog::Ok: caught an exception whild flushing the data source's data!");
                    }
                }
            }
        }
    }

    return nResult;
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
            break;
        case PAGE_TABLESUBSCRIPTION:
            static_cast<OTableSubscriptionPage&>(_rPage).setServiceFactory(m_xORB);
            static_cast<OTableSubscriptionPage&>(_rPage).SetAdminDialog(this);
            break;
    }

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
    *pCounter++ = new SfxStringItem(DSID_CONNECTURL, String());
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
    *pCounter++ = new SfxUInt16Item(DSID_FIELDDELIMITER, ';');
    *pCounter++ = new SfxUInt16Item(DSID_TEXTDELIMITER, '"');
    *pCounter++ = new SfxUInt16Item(DSID_DECIMALDELIMITER, '.');
    *pCounter++ = new SfxUInt16Item(DSID_THOUSANDSDELIMITER, ',');
    *pCounter++ = new SfxStringItem(DSID_TEXTFILEEXTENSION, String::CreateFromAscii("txt"));
    *pCounter++ = new SfxBoolItem(DSID_TEXTFILEHEADER, sal_True);

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
    // first ask the current page if it is allowed to leave
    if (!PrepareLeaveCurrentPage())
    {   // the page did not allow us to leave -> restore the old selection
        m_aSelector.select(m_sCurrentDatasource);
        return 1L;
    }

    // TODO: additionally, check if the name (which may have been modified by the user) is unique within the data source
    // collection

    // remember the settings for this data source
    sal_Int32 nOldPos = m_aSelector.getPos(m_sCurrentDatasource);
    if (m_aSelector.isModified(nOldPos))
        m_aSelector.update(nOldPos, *GetExampleSet());

    sal_Int32 nNewPos = m_aSelector.getSelected();
    // switch the content of the pages
    implSelectDatasource(m_aSelector.getOriginalName(nNewPos));

    return 0L;
}

//-------------------------------------------------------------------------
IMPL_LINK(ODbAdminDialog, OnDatasourceModifed, SfxTabPage*, _pTabPage)
{
    // check if the currently selected entry is already marked as modified
    sal_Int16 nSelected = m_aSelector.getSelected();
    if (m_aSelector.isModified(nSelected))
        // yes -> nothing to do
        return 0L;

    // no -> append the modified marker to the text
    m_aSelector.modified(nSelected, GetExampleSet() ? *GetExampleSet() : *GetInputSetImpl());

    return 0L;
}

//-------------------------------------------------------------------------
IMPL_LINK(ODbAdminDialog, OnTypeSelected, OGeneralPage*, _pTabPage)
{
    // remove all current detail pages
    while (m_aCurrentDetailPages.size())
    {
        RemoveTabPage(m_aCurrentDetailPages.top());
        m_aCurrentDetailPages.pop();
    }

    // open our own resource block, as the page titles are strings local to this block
    OLocalResourceAccess aDummy(DLG_DATABASE_ADMINISTRATION, RSC_TABDIALOG);

    // and insert the new ones
    switch (_pTabPage->GetSelectedType())
    {
        case DST_DBASE:
            AddTabPage(PAGE_DBASE, String(ResId(STR_PAGETITLE_DBASE)), ODbaseDetailsPage::Create, 0, sal_False, 1);
            m_aCurrentDetailPages.push(PAGE_DBASE);
            break;
        case DST_JDBC:
            AddTabPage(PAGE_JDBC, String(ResId(STR_PAGETITLE_JDBC)), OJdbcDetailsPage::Create, 0, sal_False, 1);
            m_aCurrentDetailPages.push(PAGE_JDBC);
            break;
        case DST_TEXT:
            AddTabPage(PAGE_TEXT, String(ResId(STR_PAGETITLE_TEXT)), OTextDetailsPage::Create, 0, sal_False, 1);
            m_aCurrentDetailPages.push(PAGE_TEXT);
            break;
        case DST_ODBC:
            AddTabPage(PAGE_ODBC, String(ResId(STR_PAGETITLE_ODBC)), OOdbcDetailsPage::Create, 0, sal_False, 1);
            m_aCurrentDetailPages.push(PAGE_ODBC);
            break;
        case DST_ADABAS:
            AddTabPage(PAGE_ADABAS, String(ResId(STR_PAGETITLE_ADABAS)), OAdabasDetailsPage::Create, 0, sal_False, 1);
            m_aCurrentDetailPages.push(PAGE_ADABAS);
            break;
    }

    return 0L;
}

//-------------------------------------------------------------------------
Reference< XPropertySet > ODbAdminDialog::getDatasource(const ::rtl::OUString& _rName)
{
    DBG_ASSERT(m_xDatabaseContext.is(), "ODbAdminDialog::getDatasource : have no database context!");
    Reference< XPropertySet > xDatasource;
    try
    {
        if (m_xDatabaseContext.is() && _rName.getLength())
            ::cppu::extractInterface(xDatasource, m_xDatabaseContext->getByName(_rName));
    }
    catch(NoSuchElementException&)
    {
        DBG_ERROR("ODbAdminDialog::getDatasource : did not find the element with the given name!");
    }
    catch(WrappedTargetException&)
    {
        DBG_ERROR("ODbAdminDialog::getDatasource : caught a WrappedTargetException!");
    }
    return xDatasource;
}

//-------------------------------------------------------------------------
void ODbAdminDialog::implSelectDatasource(const ::rtl::OUString& _rRegisteredName)
{
    Reference< XPropertySet > xDatasource = getDatasource(_rRegisteredName);
    if (!xDatasource.is())
        m_aSelector.select(m_aSelector.getSelected(), sal_False);
    else
        m_aSelector.select(_rRegisteredName);

    m_sCurrentDatasource = _rRegisteredName;

    // the selection is valid if and only if we have a datasource now
    GetInputSetImpl()->Put(SfxBoolItem(DSID_INVALID_SELECTION, !xDatasource.is()));
        // (sal_False tells the tab pages to disable and reset all their controls, which is different
        // from "just set them to readonly")

    // reset the pages

    // prevent flicker
    SetUpdateMode(sal_False);

    // remove all tab pages (except the general one)
    // remove all current detail pages
    while (m_aCurrentDetailPages.size())
    {
        RemoveTabPage(m_aCurrentDetailPages.top());
        m_aCurrentDetailPages.pop();
    }
    // remove the table/query tab pages
    RemoveTabPage(PAGE_TABLESUBSCRIPTION);

    // extract all relevant data from the property set of the data source
    translateProperties(xDatasource, *GetInputSetImpl());

    // fill in the remembered settings for the data source
    m_aSelector.getSettings(m_aSelector.getSelected(), *GetInputSetImpl());

    // propagate this set as our new input set and reset the example set
    SetInputSet(GetInputSetImpl());
    delete pExampleSet;
    pExampleSet = new SfxItemSet(*GetInputSetImpl());

    // and again, add the non-details tab pages
    {
        OLocalResourceAccess aDummy(DLG_DATABASE_ADMINISTRATION, RSC_TABDIALOG);
        AddTabPage(PAGE_TABLESUBSCRIPTION, String(ResId(STR_PAGETITLE_TABLESUBSCRIPTION)), OTableSubscriptionPage::Create, NULL);
    }

    ShowPage(PAGE_GENERAL);
    SetUpdateMode(sal_True);

    // propagate the new data to the general tab page the general tab page
    SfxTabPage* pGeneralPage = GetTabPage(PAGE_GENERAL);
    if (pGeneralPage)
        pGeneralPage->Reset(*GetInputSetImpl());
    // if this is NULL, the page has not been created yet, which means we're called before the
    // dialog was displayed (probably from inside the ctor)
}

//-------------------------------------------------------------------------
void ODbAdminDialog::translateProperties(const Reference< XPropertySet >& _rxSource, SfxItemSet& _rDest)
{
    ::rtl::OUString sNewConnectURL, sName, sUid, sPwd;
    Sequence< ::rtl::OUString > aTableFitler;
    sal_Bool bPasswordRequired = sal_False;
    sal_Bool bReadOnly = sal_True;
    try
    {
        if (_rxSource.is())
        {
            _rxSource->getPropertyValue(PROPERTY_URL) >>= sNewConnectURL;
            _rxSource->getPropertyValue(PROPERTY_NAME) >>= sName;
            _rxSource->getPropertyValue(PROPERTY_USER) >>= sUid;
            _rxSource->getPropertyValue(PROPERTY_PASSWORD) >>= sPwd;
            _rxSource->getPropertyValue(PROPERTY_TABLEFILTER) >>= aTableFitler;
            _rxSource->getPropertyValue(PROPERTY_ISPASSWORDREQUIRED) >>= bPasswordRequired;
            _rxSource->getPropertyValue(PROPERTY_ISREADONLY) >>= bReadOnly;
        }
    }
    catch(Exception&)
    {
        DBG_ERROR("ODbAdminDialog::translateProperties : could not extract all the relevant datasource properties!");
    }

    _rDest.Put(SfxStringItem(DSID_CONNECTURL, sNewConnectURL));
    _rDest.Put(SfxStringItem(DSID_NAME, sName));
    _rDest.Put(SfxStringItem(DSID_USER, sUid));
    _rDest.Put(SfxStringItem(DSID_PASSWORD, sPwd));
    _rDest.Put(OStringListItem(DSID_TABLEFILTER, aTableFitler));
    _rDest.Put(SfxBoolItem(DSID_PASSWORDREQUIRED, bPasswordRequired));
    _rDest.Put(SfxBoolItem(DSID_READONLY, bReadOnly));
}

//-------------------------------------------------------------------------
void ODbAdminDialog::translateProperties(const SfxItemSet& _rSource, const Reference< XPropertySet >& _rxDest)
{
    DBG_ASSERT(_rxDest.is(), "ODbAdminDialog::translateProperties : invalid property set!");
    if (!_rxDest.is())
        return;

    // get the items
    SFX_ITEMSET_GET(_rSource, pConnectURL, SfxStringItem, DSID_CONNECTURL, sal_True);
    SFX_ITEMSET_GET(_rSource, pName, SfxStringItem, DSID_NAME, sal_True);
    SFX_ITEMSET_GET(_rSource, pUser, SfxStringItem, DSID_USER, sal_True);
    SFX_ITEMSET_GET(_rSource, pPassword, SfxStringItem, DSID_PASSWORD, sal_True);
    SFX_ITEMSET_GET(_rSource, pTableFilter, OStringListItem, DSID_TABLEFILTER, sal_True);
    SFX_ITEMSET_GET(_rSource, pPasswordRequired, SfxBoolItem, DSID_PASSWORDREQUIRED, sal_True);

    // set the values
    try
    {
        if (pConnectURL)
            _rxDest->setPropertyValue(PROPERTY_URL, makeAny(::rtl::OUString(pConnectURL->GetValue().GetBuffer())));
        if (pUser)
            _rxDest->setPropertyValue(PROPERTY_USER, makeAny(::rtl::OUString(pUser->GetValue().GetBuffer())));
        if (pPassword)
            _rxDest->setPropertyValue(PROPERTY_PASSWORD, makeAny(::rtl::OUString(pPassword->GetValue().GetBuffer())));
        if (pTableFilter)
            _rxDest->setPropertyValue(PROPERTY_TABLEFILTER, makeAny(pTableFilter->getList()));
        if (pPasswordRequired)
            _rxDest->setPropertyValue(PROPERTY_ISPASSWORDREQUIRED, makeAny(pPasswordRequired->GetValue()));

//      if (pName)
//          _rxDest->setPropertyValue(PROPERTY_NAME, makeAny(::rtl::OUString(pName->GetValue().GetBuffer())));
        // TODO: a changed name requires an removeByName and insertByName
    }
    catch(Exception&)
    {
        DBG_ERROR("ODbAdminDialog::translateProperties : could not set all the relevant datasource properties!");
    }
}

//=========================================================================
//= ODatasourceSelector
//=========================================================================
//-------------------------------------------------------------------------
ODatasourceSelector::ODatasourceSelector(Window* _pParent, const ResId& _rResId)
    :Window(_pParent, _rResId)
    ,m_aNewDatasource   (this, ResId(PB_NEW_DATASOURCE))
    ,m_aDatasourceList  (this, ResId(LB_DATASOURCES))
{
    FreeResource();

    m_aDatasourceList.SetZOrder(NULL, WINDOW_ZORDER_FIRST);

    m_aDatasourceList.EnableClipSiblings(sal_True);
}

//-------------------------------------------------------------------------
ODatasourceSelector::~ODatasourceSelector()
{
}

//-------------------------------------------------------------------------
sal_Bool ODatasourceSelector::isModified(sal_Int32 _nPos) const
{
    return NULL != m_aDatasourceList.GetEntryData(_nPos);
}

//-------------------------------------------------------------------------
void ODatasourceSelector::clearAllModified()
{
    // delete all SfxItemSets which are bound to the entries
    for (sal_Int32 nLoop = 0; nLoop < m_aDatasourceList.GetEntryCount(); ++nLoop)
    {
        SfxItemSet* pItems = reinterpret_cast<SfxItemSet*>(m_aDatasourceList.GetEntryData(nLoop));
        if (pItems)
        {
            delete pItems;
            m_aDatasourceList.SetEntryData(nLoop, reinterpret_cast<SfxItemSet*>(NULL));
        }
    }
}

//-------------------------------------------------------------------------
void ODatasourceSelector::modified(sal_Int32 _nPos, const SfxItemSet& _rSet)
{
    SfxItemSet* pItems = reinterpret_cast<SfxItemSet*>(m_aDatasourceList.GetEntryData(_nPos));
    if (pItems)
        pItems->Put(_rSet);
    else
    {
        pItems = new SfxItemSet(_rSet);
        // the entry was not modified before. Add the modified marker
        // (unfortunately the ListBox does not have an operation such as SetEntryText ...)
        m_aDatasourceList.SetUpdateMode(sal_False);
        String sText = m_aDatasourceList.GetEntry(_nPos);
        // remember the original name
        pItems->Put(SfxStringItem(DSID_ORIGINALNAME, sText));

        m_aDatasourceList.RemoveEntry(_nPos);
        sText.AppendAscii(MODIFIED_MARKER);
        _nPos = m_aDatasourceList.InsertEntry(sText, _nPos);
        m_aDatasourceList.SelectEntryPos (_nPos, sal_True);
        m_aDatasourceList.SetUpdateMode(sal_True);
    }

    m_aDatasourceList.SetEntryData(_nPos, reinterpret_cast<void*>(pItems));
}

//-------------------------------------------------------------------------
void ODatasourceSelector::getSettings(sal_Int32 _nPos, SfxItemSet& _rSet)
{
    SfxItemSet* pItems = reinterpret_cast<SfxItemSet*>(m_aDatasourceList.GetEntryData(_nPos));
    if (pItems)
        _rSet.Put(*pItems);
}

//-------------------------------------------------------------------------
sal_Int32 ODatasourceSelector::getPos(const String& _rName) const
{
    sal_uInt16 nPos = m_aDatasourceList.GetEntryPos(_rName);
    if ((sal_uInt16)-1 == nPos)
        nPos = m_aDatasourceList.GetEntryPos(String(_rName).AppendAscii(MODIFIED_MARKER));
    if ((sal_uInt16)-1 == nPos)
        nPos = -1;

    return nPos;
}

//-------------------------------------------------------------------------
String ODatasourceSelector::getOriginalName(sal_Int32 _nPos) const
{
    // calc the name of the newly selected data source (we may have to cut the modified marker)
    String sText = m_aDatasourceList.GetEntry(_nPos);
    if (!isModified(_nPos))
        return sText;

    const SfxItemSet* pItems = getItems(_nPos);
    DBG_ASSERT(pItems, "ODatasourceSelector::getOriginalName: not modified, but no items?");
    SFX_ITEMSET_GET(*pItems, pOriginal, SfxStringItem, DSID_ORIGINALNAME, sal_True);
    DBG_ASSERT(pOriginal, "ODatasourceSelector::getOriginalName: a modified set should always have an OriginalName entry!");
    return pOriginal->GetValue();
}

//-------------------------------------------------------------------------
const SfxItemSet* ODatasourceSelector::getItems(sal_Int32 _nPos) const
{
    return reinterpret_cast<const SfxItemSet*>(m_aDatasourceList.GetEntryData(_nPos));
}

//-------------------------------------------------------------------------
sal_Int32 ODatasourceSelector::insert(const String& _rName)
{
    sal_Int16 nPos = m_aDatasourceList.InsertEntry(_rName);
    m_aDatasourceList.SetEntryData(nPos, reinterpret_cast<void*>(NULL));
    return nPos;
}

//-------------------------------------------------------------------------
void ODatasourceSelector::Resize()
{
    Window::Resize();
    Size aSize = GetSizePixel();

    // adjust the width of the button
    sal_Int32 nButtonHeight = m_aNewDatasource.GetSizePixel().Height();
    m_aNewDatasource.SetSizePixel(Size(aSize.Width(), nButtonHeight));

    // adjust width/height of the listbox
    m_aDatasourceList.SetPosPixel(Point(0, nButtonHeight));
    m_aDatasourceList.SetSizePixel(Size(aSize.Width(), aSize.Height() - nButtonHeight));
}

//.........................................................................
}   // namespace dbaui
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  2000/10/05 10:04:31  fs
 *  initial checkin
 *
 *
 *  Revision 1.0 20.09.00 10:55:58  fs
 ************************************************************************/

