/*************************************************************************
 *
 *  $RCSfile: dbadmin.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-11 11:31:03 $
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
//= ODatasourceMap
//=========================================================================
//-------------------------------------------------------------------------
ODatasourceMap::ODatasourceMap(const Reference< XMultiServiceFactory > _rxORB)
    :m_xORB(_rxORB)
{
    // create the DatabaseContext service
    DBG_ASSERT(m_xORB.is(), "ODatasourceMap::ODatasourceMap: need a service factory !");
    try
    {
        m_xDatabaseContext = Reference< XNameAccess >(m_xORB->createInstance(SERVICE_SDB_DATABASECONTEXT), UNO_QUERY);
    }
    catch(Exception&)
    {
    }

    // initialize our map
    if (m_xDatabaseContext.is())
    {
        Sequence< ::rtl::OUString > aDatasources = m_xDatabaseContext->getElementNames();
        const ::rtl::OUString* pDatasources = aDatasources.getConstArray();
        for (sal_Int32 i=0; i<aDatasources.getLength(); ++i, ++pDatasources)
            m_aDatasources[*pDatasources] = DatasourceInfo();
    }
}

//-------------------------------------------------------------------------
ODatasourceMap::Iterator ODatasourceMap::begin()
{
    return Iterator(this, m_aDatasources.begin());
}

//-------------------------------------------------------------------------
ODatasourceMap::Iterator ODatasourceMap::end()
{
    return Iterator(this, m_aDatasources.end());
}

//-------------------------------------------------------------------------
void ODatasourceMap::update(const ::rtl::OUString& _rName, const SfxItemSet& _rSet)
{
    DatasourceInfosIterator aPos = m_aDatasources.find(_rName);
    DBG_ASSERT(aPos != m_aDatasources.end(), "ODatasourceMap::update: invalid name!!");
    if (aPos == m_aDatasources.end())
        return;

    if (aPos->second.pModifications)
        // already had modifications for this datasource
        // -> simply update them
        aPos->second.pModifications->Put(_rSet);
    else
    {
        // this is the first time this data source is modified -> create a new set and initialize it
        aPos->second.pModifications = new SfxItemSet(_rSet);
        aPos->second.pModifications->Put(SfxStringItem(DSID_ORIGINALNAME, _rName));
        aPos->second.pModifications->Put(SfxBoolItem(DSID_NEWDATASOURCE, sal_False));
    }
}

//-------------------------------------------------------------------------
void ODatasourceMap::clearModified(const ::rtl::OUString& _rName)
{
    DatasourceInfosIterator aPos = m_aDatasources.find(_rName);
    DBG_ASSERT(aPos != m_aDatasources.end(), "ODatasourceMap::clearModified: invalid name!!");
    if (aPos == m_aDatasources.end())
        return;

    if (aPos->second.pModifications)
    {
        delete aPos->second.pModifications;
        aPos->second.pModifications = NULL;
    }
}

//-------------------------------------------------------------------------
void ODatasourceMap::clear()
{
    for (   ConstDatasourceInfosIterator aLoop = m_aDatasources.begin();
            aLoop != m_aDatasources.end();
            ++aLoop
        )
    {
        if (aLoop->second.pModifications)
            delete aLoop->second.pModifications;
    }
    m_aDatasources.clear();
}

//-------------------------------------------------------------------------
void ODatasourceMap::ensureObject(const ::rtl::OUString& _rName)
{
    DatasourceInfosIterator aPos = m_aDatasources.find(_rName);
    DBG_ASSERT(aPos != m_aDatasources.end(), "ODatasourceMap::ensureObject: invalid name!!");
    if (aPos == m_aDatasources.end())
        return;

    if (aPos->second.xDatasource.is())
        // nothing to do, the object already exists
        return;

    try
    {
        if (m_xDatabaseContext.is() && _rName.getLength())
            ::cppu::extractInterface(aPos->second.xDatasource, m_xDatabaseContext->getByName(_rName));
        DBG_ASSERT(aPos->second.xDatasource.is(), "ODatasourceMap::ensureObject: could not retrieve the object!");
    }
    catch(NoSuchElementException&)
    {
        DBG_ERROR("ODatasourceMap::ensureObject: did not find the element with the given name!");
    }
    catch(WrappedTargetException&)
    {
        DBG_ERROR("ODatasourceMap::ensureObject: caught a WrappedTargetException!");
    }
}

//-------------------------------------------------------------------------
::rtl::OUString ODatasourceMap::adjustRealName(const ::rtl::OUString& _rName)
{
    DatasourceInfosIterator aPos = m_aDatasources.find(_rName);
    DBG_ASSERT(aPos != m_aDatasources.end(), "ODatasourceMap::adjustRealName: invalid name!!");
    if (aPos == m_aDatasources.end())
        return _rName;

    if (!aPos->second.pModifications)
        return _rName;

    SFX_ITEMSET_GET(*aPos->second.pModifications, pRealName, SfxStringItem, DSID_NAME, sal_True);
    if (!pRealName)
        return _rName;

    ::rtl::OUString sRealName = pRealName->GetValue().GetBuffer();
    if (sRealName.equals(_rName))
        // all fine
        return _rName;

    DBG_ASSERT(m_aDatasources.end() == m_aDatasources.find(sRealName), "ODatasourceMap::adjustRealName: have an invalid real name!");

    renamed(_rName, sRealName);
    return sRealName;
}

//-------------------------------------------------------------------------
void ODatasourceMap::renamed(const ::rtl::OUString& _rOldName, const ::rtl::OUString& _rNewName)
{
    DatasourceInfosIterator aPos = m_aDatasources.find(_rOldName);
    DBG_ASSERT(aPos != m_aDatasources.end(), "ODatasourceMap::renamed: invalid name!!");
    if (aPos == m_aDatasources.end())
        return;

    // insert the DatasourceInfo under the new name
    DatasourceInfo aInfo = aPos->second;
    m_aDatasources.erase(aPos);
    m_aDatasources[_rNewName] = aInfo;
}

//-------------------------------------------------------------------------
sal_Bool ODatasourceMap::exists(const ::rtl::OUString& _rName) const
{
    return isValid() && (m_aDatasources.end() != m_aDatasources.find(_rName));
}

//-------------------------------------------------------------------------
Reference< XPropertySet > ODatasourceMap::createNew(const ::rtl::OUString& _rName, SfxItemPool* _pPool, const USHORT* _pRanges)
{
    Reference< XPropertySet > xReturn;
    try
    {
        xReturn = Reference< XPropertySet >(m_xORB->createInstance(SERVICE_SDB_DATASOURCE), UNO_QUERY);
    }
    catch(Exception&)
    {
    }

    if (xReturn.is())
    {
        // create a new item set
        SfxItemSet* pItems = new SfxItemSet(*_pPool, _pRanges);
        pItems->Put(SfxBoolItem(DSID_NEWDATASOURCE, sal_True));
        pItems->Put(SfxStringItem(DSID_NAME, _rName));
        m_aDatasources[_rName] = DatasourceInfo(xReturn, pItems);
    }
    return xReturn;
}

//-------------------------------------------------------------------------
ODatasourceMap::ODatasourceInfo ODatasourceMap::operator[](const ::rtl::OUString _rName)
{
    ConstDatasourceInfosIterator aPos = m_aDatasources.find(_rName);
    DBG_ASSERT(aPos != m_aDatasources.end(), "ODatasourceMap::operator[]: invalid name!!");
    if (aPos == m_aDatasources.end())
    {
        // just prevent crashes, the result here is completely nonsense
        static ODatasourceMap::DatasourceInfo aFallback;
        return ODatasourceInfo(NULL, ::rtl::OUString(), aFallback);
    }

    return ODatasourceInfo(this, aPos->first, aPos->second);
}

//-------------------------------------------------------------------------
//- ODatasourceMap::ODatasourceInfo
//-------------------------------------------------------------------------
Reference< XPropertySet > ODatasourceMap::ODatasourceInfo::getDatasource() const
{
    if (!m_rInfoImpl.xDatasource.is())
    {   // the object has not been accessed yet -> create it
        DBG_ASSERT(m_pOwner, "ODatasourceInfo::getDatasource: no owner which could provide the object!");
        DBG_ASSERT(!isNew(), "ODatasourceInfo::getDatasource: new object without property set!");

        if (m_pOwner)
            m_pOwner->ensureObject(getName());
    }
    return m_rInfoImpl.xDatasource;
}

//-------------------------------------------------------------------------
::rtl::OUString ODatasourceMap::ODatasourceInfo::getRealName() const
{
    if (!isModified())
        return getName();

    ::rtl::OUString sReturn;
    if (m_rInfoImpl.pModifications)
    {
        SFX_ITEMSET_GET(*m_rInfoImpl.pModifications, pRealName, SfxStringItem, DSID_NAME, sal_True);
        if (pRealName)
            sReturn = pRealName->GetValue().GetBuffer();
    }
    return sReturn;
}

//-------------------------------------------------------------------------
::rtl::OUString ODatasourceMap::ODatasourceInfo::getOriginalName() const
{
    if (!isModified())
        return getName();

    ::rtl::OUString sReturn;
    if (m_rInfoImpl.pModifications)
    {
        SFX_ITEMSET_GET(*m_rInfoImpl.pModifications, pOriginalName, SfxStringItem, DSID_ORIGINALNAME, sal_True);
        if (pOriginalName)
            sReturn = pOriginalName->GetValue().GetBuffer();
    }
    return sReturn;
}

//-------------------------------------------------------------------------
sal_Bool ODatasourceMap::ODatasourceInfo::isModified() const
{
    return NULL != m_rInfoImpl.pModifications;
}

//-------------------------------------------------------------------------
sal_Bool ODatasourceMap::ODatasourceInfo::isNew() const
{
    if (!m_rInfoImpl.pModifications)
        return sal_False;
    SFX_ITEMSET_GET(*m_rInfoImpl.pModifications, pIsNew, SfxBoolItem, DSID_NEWDATASOURCE, sal_True);
    return pIsNew && pIsNew->GetValue();
}

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
    m_aSelector.setNewHandler(LINK(this, ODbAdminDialog, OnNewDatasource));

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
            m_aAllDatasources.insert(*aDatasourceLoop);

            ++aDatasourceLoop;
        }

        if (!m_aDatasources.size())
        {
            WarningBox(_pParent, ModuleRes(ERR_NOREGISTEREDDATASOURCES)).Execute();
        }
        else
            sInitialSelection = *m_aDatasources.begin();
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
    m_sOriginalTitle = GetText();

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
        ::rtl::OUString sCurrentlySelected = m_aSelector.getSelected();
        if (m_aDatasources[sCurrentlySelected]->isModified())
        {
            m_aDatasources.update(sCurrentlySelected, *GetExampleSet());
            m_aDatasources.adjustRealName(sCurrentlySelected);
        }

        // We allowed the user to freely rename/create/delete datasources, without committing anything ('til now).
        // This could lead to conflicts: if datasource "A" was renamed to "B", and a new ds "A" created, then we
        // would have to do the renaming before the creation. This would require us to analyze the changes in
        // m_aDatasources for any such depencies, which could be difficult (okay, I'm not willing to do it :)
        // Instead we use another approach: If we encounter a conflict (a DS which can't be renamed or inserted),
        // we save this entry and continue with the next one. This way, the ds causing the conflict should be handled
        // first. After that, we do a new round, assuming that now the conflict is resolved.
        // A disadvantage is that this may require O(n^2) rounds, but this is not really expensive ....

        sal_Int32 nDelayed = 0;
        sal_Int32 nLastRoundDelayed = -1;
            // to ensure that we're not looping 'til death: If this doesn't change within one round, the DatabaseContext
            // is not in the state as this dialog started anymore. This means somebody else did some renamings
            // or insertings, causing us conflicts now.

        do
        {
            if (nLastRoundDelayed == nDelayed)
            {
                DBG_ERROR("ODbAdminDialog::Ok: somedoby tampered with the context!");
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
                        // put the remembered settings into the property set
                        translateProperties(*aLoop->getModifications(), xDatasource);

                        ::rtl::OUString sName = aLoop->getName();
                        DBG_ASSERT(sName.equals(aLoop->getRealName()), "ODbAdminDialog::Ok: invalid name/realname combination!");
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
                                DBG_ERROR("ODbAdminDialog::Ok: data source was renamed, but could not remove it (to insert it under a new name)!");
                                // TODO: an error message
                            }
                        }
                        if (aLoop->isRenamed() || aLoop->isNew())
                        {
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
                                DBG_ERROR("ODbAdminDialog::Ok: removed the entry, but could not re-insert it!");
                                // we're going to re-insert the object under it's old name
                                bOperationSuccess = sal_False;
                                try
                                {
                                    m_xDynamicContext->registerObject(sOriginalName, xDatasource.get());
                                    bOperationSuccess = sal_True;
                                }
                                catch(Exception&) { }
                                DBG_ASSERT(bOperationSuccess, "ODbAdminDialog::Ok: could not insert it under the old name, too ... no we have a data loss!");
                            }
                            // reset the item, so in case we need an extra round (because of delayed items) it
                            // won't be included anymore
                            m_aDatasources.clearModified(sName);
                            continue;
        //  | <------------ continue with the next data source
                        }

                        // We're here if the data source was not renamed, not deleted and is not new. Just flush it.
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
                        // reset the item, so in case we need an extra round (because of delayed items) it
                        // won't be included anymore
                        m_aDatasources.clearModified(sName);
                    }
                }
            }
        }
        while (nDelayed);
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
            static_cast<OGeneralPage&>(_rPage).SetNameModifyHandler(LINK(this, ODbAdminDialog, OnNameModified));
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
    *pCounter++ = new SfxUInt16Item(DSID_FIELDDELIMITER, ';');
    *pCounter++ = new SfxUInt16Item(DSID_TEXTDELIMITER, '"');
    *pCounter++ = new SfxUInt16Item(DSID_DECIMALDELIMITER, '.');
    *pCounter++ = new SfxUInt16Item(DSID_THOUSANDSDELIMITER, ',');
    *pCounter++ = new SfxStringItem(DSID_TEXTFILEEXTENSION, String::CreateFromAscii("txt"));
    *pCounter++ = new SfxBoolItem(DSID_TEXTFILEHEADER, sal_True);
    *pCounter++ = new SfxBoolItem(DSID_NEWDATASOURCE, sal_False);

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

    String sNewSelection = m_aSelector.getSelected();

    // remember the settings for this data source
    ODatasourceMap::ODatasourceInfo aPreviouslySelected = m_aDatasources[m_sCurrentDatasource];
    if (aPreviouslySelected.isModified())
        m_aDatasources.update(m_sCurrentDatasource, *GetExampleSet());
    // (The modified flag is set as soon as any UI element has any change (e.g. a single new character in a edit line).
    // But when this flag is set, and other changes occur, no items are transfered.
    // That's why the above statement "if (isModified()) update()" makes sense, though it may not seem so :)

    // need a special handling for the name property
    if (aPreviouslySelected.isModified())
    {
        String sName = aPreviouslySelected.getName().getStr();
        DBG_ASSERT(m_sCurrentDatasource.equals(sName.GetBuffer()), "ODbAdminDialog::OnDatasourceSelected: inconsistent names!");

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

    // switch the content of the pages
    implSelectDatasource(sNewSelection);

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
    m_aDatasources.update(sCurrentlySelected, *GetExampleSet());

    return 0L;
}

//-------------------------------------------------------------------------
IMPL_LINK(ODbAdminDialog, OnNameModified, OGeneralPage*, _pTabPage)
{
    ::rtl::OUString sNewStringSuggestion = _pTabPage->GetCurrentName();

    // check if there's already an data source with the suggested name
    ConstStringSetIterator aExistentPos = m_aAllDatasources.find(sNewStringSuggestion);
        // !! m_aAllDatasources contains _all_ data source names _except_ the currently selected one !!

    sal_Bool bValid = m_aAllDatasources.end() == aExistentPos;

    // the user is not allowed to leave the current data source (or to commit the dialog) as long
    // as the name is invalid
    m_aSelector.Enable(bValid && m_aDatasources.isValid());
    GetOKButton().Enable(bValid);

    // if this is the first modification did on this data source, we have to adjust the DS list accordingly
    if (!m_bResetting)
    {
        String sSelected = m_aSelector.getSelected();
        if (!m_aDatasources[sSelected].isModified())
        {   // (we could do it all the time here, but as this link is called every time a single character
            // of the name changes, this maybe would be too expensive.)
            m_aSelector.modified(sSelected);
            m_aDatasources.update(sSelected, *GetExampleSet());
        }
    }

    return bValid ? 1L : 0L;
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
    DBG_ASSERT(m_aDatasources.isValid(), "ODbAdminDialog::getDatasource : have no database context!");
    if (!m_aDatasources.exists(_rName))
        return Reference< XPropertySet >();

    return m_aDatasources[_rName]->getDatasource();
}

//-------------------------------------------------------------------------
void ODbAdminDialog::implSelectDatasource(const ::rtl::OUString& _rRegisteredName)
{
    Reference< XPropertySet > xDatasource = getDatasource(_rRegisteredName);
    if (!xDatasource.is())
        m_aSelector.select(m_aSelector.getSelected(), sal_False);
    else
        m_aSelector.select(_rRegisteredName);

    // insert the previously selected data source into our set
    if (m_sCurrentDatasource.getLength())
        m_aAllDatasources.insert(m_sCurrentDatasource);
    m_sCurrentDatasource = _rRegisteredName;
    // remove the now selected data source from our set
    m_aAllDatasources.erase(m_sCurrentDatasource);

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

    // reset some items in the input set which are for tracking the state of the current ds
    GetInputSetImpl()->Put(SfxBoolItem(DSID_NEWDATASOURCE, sal_False));

    // fill in the remembered settings for the data source
    if (m_aDatasources[m_sCurrentDatasource]->isModified())
        GetInputSetImpl()->Put(*m_aDatasources[m_sCurrentDatasource]->getModifications());

    // propagate this set as our new input set and reset the example set
    SetInputSet(GetInputSetImpl());
    delete pExampleSet;
    pExampleSet = new SfxItemSet(*GetInputSetImpl());

    // and again, add the non-details tab pages
    {
        OLocalResourceAccess aDummy(DLG_DATABASE_ADMINISTRATION, RSC_TABDIALOG);
        AddTabPage(PAGE_TABLESUBSCRIPTION, String(ResId(STR_PAGETITLE_TABLESUBSCRIPTION)), OTableSubscriptionPage::Create, NULL);
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
    }
    catch(Exception&)
    {
        DBG_ERROR("ODbAdminDialog::translateProperties : could not set all the relevant datasource properties!");
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
        if (m_aAllDatasources.end() != m_aAllDatasources.find(sCheck))
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
IMPL_LINK(ODbAdminDialog, OnNewDatasource, Button*, _pButton)
{
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

    // insert a new entry for the new DS
    m_aSelector.insertNew(sNewName);
    // update our "all-but the selected ds" structure
    m_aAllDatasources.insert(sNewName);

    // and select this new entry
    m_aSelector.select(sNewName);
    implSelectDatasource(sNewName);

    SfxTabPage* pGeneralPage = GetTabPage(PAGE_GENERAL);
    if (pGeneralPage)
        pGeneralPage->GrabFocus();

    return 0L;
}

//=========================================================================
//= ODatasourceSelector
//=========================================================================
#define DS_MODIFIED         1
#define DS_NEW              2
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
void ODatasourceSelector::modified(const String& _sName)
{
    sal_uInt16 nPos = m_aDatasourceList.GetEntryPos(_sName);
    sal_Int32 nFlags = reinterpret_cast<sal_Int32>(m_aDatasourceList.GetEntryData(nPos));
    if (nFlags & DS_MODIFIED)
        // nothing to do, the entry is already marked as modified
        return;

    sal_Bool bWasSelected = m_aDatasourceList.GetSelectEntryPos() == nPos;

    // reinsert the entry at it's old position with a new image
    m_aDatasourceList.SetUpdateMode(sal_False);
    String sName = m_aDatasourceList.GetEntry(nPos);
    m_aDatasourceList.RemoveEntry(nPos);
    nPos = m_aDatasourceList.InsertEntry(sName, Image(ModuleRes(BMP_DATABASE_MODIFIED)), nPos);
    m_aDatasourceList.SetUpdateMode(sal_True);

    if (bWasSelected)
        m_aDatasourceList.SelectEntryPos(nPos, sal_True);

    // set the "modified flag"
    m_aDatasourceList.SetEntryData(nPos, reinterpret_cast<void*>(nFlags | DS_MODIFIED));
}

//-------------------------------------------------------------------------
void ODatasourceSelector::renamed(const String& _rOldName, const String& _rNewName)
{
    sal_Int32 nPos = m_aDatasourceList.GetEntryPos(_rOldName);
    DBG_ASSERT((sal_uInt16)-1 != nPos, "ODatasourceSelector::renamed: invalid old name!");

    sal_Int32 nFlags = reinterpret_cast<sal_Int32>(m_aDatasourceList.GetEntryData(nPos));
    m_aDatasourceList.RemoveEntry(nPos);
    nPos = m_aDatasourceList.InsertEntry(_rNewName, Image(ModuleRes((nFlags & DS_NEW) ? BMP_DATABASE_NEW : BMP_DATABASE_MODIFIED)), nPos);
    m_aDatasourceList.SetEntryData(nPos, reinterpret_cast<void*>(nFlags));
}

//-------------------------------------------------------------------------
sal_Int32 ODatasourceSelector::insert(const String& _rName)
{
    sal_Int16 nPos = m_aDatasourceList.InsertEntry(_rName, Image(ModuleRes(BMP_DATABASE)));
    m_aDatasourceList.SetEntryData(nPos, reinterpret_cast<void*>(0));
    return nPos;
}

//-------------------------------------------------------------------------
void ODatasourceSelector::insertNew(const String& _rName)
{
    // insert the new entry with the appropriate image
    sal_Int16 nPos = m_aDatasourceList.InsertEntry(_rName, Image(ModuleRes(BMP_DATABASE_NEW)));
    m_aDatasourceList.SetEntryData(nPos, reinterpret_cast<void*>(DS_NEW));
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
 *  Revision 1.2  2000/10/09 12:39:29  fs
 *  some (a lot of) new imlpementations - still under development
 *
 *  Revision 1.1  2000/10/05 10:04:31  fs
 *  initial checkin
 *
 *
 *  Revision 1.0 20.09.00 10:55:58  fs
 ************************************************************************/

