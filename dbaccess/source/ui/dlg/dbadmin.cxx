/*************************************************************************
 *
 *  $RCSfile: dbadmin.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-26 15:02:16 $
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
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif
#ifndef _TYPELIB_TYPEDESCRIPTION_HXX_
#include <typelib/typedescription.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................

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
ODatasourceMap::Iterator ODatasourceMap::beginDeleted()
{
    return Iterator(this, m_aDeletedDatasources.begin());
}

//-------------------------------------------------------------------------
ODatasourceMap::Iterator ODatasourceMap::endDeleted()
{
    return Iterator(this, m_aDeletedDatasources.end());
}

//-------------------------------------------------------------------------
void ODatasourceMap::update(const ::rtl::OUString& _rName, SfxItemSet& _rSet)
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
        aPos->second.pModifications->Put(SfxBoolItem(DSID_DELETEDDATASOURCE, sal_False));

        // and reset the original name in the source set, it may contain an old one
        _rSet.Put(SfxStringItem(DSID_ORIGINALNAME, _rName));
    }
}

//-------------------------------------------------------------------------
void ODatasourceMap::deleted(const ::rtl::OUString& _rName)
{
    DatasourceInfosIterator aPos = m_aDatasources.find(_rName);
    DBG_ASSERT(aPos != m_aDeletedDatasources.end(), "ODatasourceMap::deleted: invalid access key!");
    if (aPos == m_aDeletedDatasources.end())
        return;

    delete aPos->second.pModifications;
    m_aDatasources.erase(aPos);
}

//-------------------------------------------------------------------------
sal_Bool ODatasourceMap::restoreDeleted(sal_Int32 _nAccessId, ::rtl::OUString& _rName)
{
    MapInt2InfoIterator aPos = m_aDeletedDatasources.find(_nAccessId);
    if (m_aDeletedDatasources.end() == aPos)
    {
        DBG_ERROR("ODatasourceSelector::restoreDeleted: invalid access id!");
        return sal_False;
    }

    // the name (not the original name) of the data source
    Iterator aEasyNameAccess(this, aPos);
    ::rtl::OUString sName = aEasyNameAccess->getName();

    // check if we have a not-deleted data source with that name
    ConstDatasourceInfosIterator aExistentPos = m_aDatasources.find(sName);
    if (m_aDatasources.end() != aExistentPos)
        // yes -> too bad ...
        return sal_False;

    m_aDatasources[sName] = aPos->second;
    m_aDeletedDatasources.erase(aPos);
    _rName = sName;
    return sal_True;
}

//-------------------------------------------------------------------------
sal_Int32 ODatasourceMap::markDeleted(const ::rtl::OUString& _rName)
{
    DatasourceInfosIterator aPos = m_aDatasources.find(_rName);
    DBG_ASSERT(aPos != m_aDatasources.end(), "ODatasourceMap::markDeleted: invalid name!!");
    if (aPos == m_aDatasources.end())
        return -1;

    // after the DatasourceInfo moved from m_aDatasources to m_aDeletedDatasources, we loose the info
    // of the name of the ds (which in m_aDatasources is the key). So later we need an modifications item set
    // or the real object itself to obtain that name.
    if (NULL == aPos->second.pModifications)
        ensureObject(_rName);

    // search a free access key
    const sal_uInt32 nPrime = 65521;
    sal_uInt32 nAccessKey = rand() % nPrime;    // the engendering

    for (sal_uInt32 nLoop=0; nLoop<nPrime; ++nLoop, nAccessKey = (nAccessKey * nAccessKey) % nPrime)
    {
        ::std::pair< MapInt2InfoIterator, bool > aInsertPos
            = m_aDeletedDatasources.insert(MapInt2Info::value_type(nAccessKey, aPos->second));
        if (aInsertPos.second)
            // a new entry was inserted into m_aDeletedDatasources, which means the key wasn't used yet
            break;
    }
    if (nAccessKey<nPrime)
    {   // we found a free key ...
        // delete the DatasourceInfo from the other map
        m_aDatasources.erase(aPos);
        return nAccessKey;
    }

    DBG_ERROR("ODatasourceMap::markDeleted: could not find a free key (tried 65521 different ones ...)!");
    return -1;
}

//-------------------------------------------------------------------------
void ODatasourceMap::clearModifiedFlag(const ::rtl::OUString& _rName)
{
    DatasourceInfosIterator aPos = m_aDatasources.find(_rName);
    DBG_ASSERT(aPos != m_aDatasources.end(), "ODatasourceMap::clearModifiedFlag: invalid name!!");
    if (aPos == m_aDatasources.end())
        return;

    if (aPos->second.pModifications)
    {
        delete aPos->second.pModifications;
        aPos->second.pModifications = NULL;
    }
}

//-------------------------------------------------------------------------
void ODatasourceMap::clearDeleted()
{
    for (   ConstMapInt2InfoIterator aLoopDeleted = m_aDeletedDatasources.begin();
            aLoopDeleted != m_aDeletedDatasources.end();
            ++aLoopDeleted
        )
    {
        if (aLoopDeleted->second.pModifications)
            delete aLoopDeleted->second.pModifications;
    }
    m_aDeletedDatasources.clear();
}

//-------------------------------------------------------------------------
void ODatasourceMap::clear()
{
    // the "ordinary" data sources
    for (   ConstDatasourceInfosIterator aLoop = m_aDatasources.begin();
            aLoop != m_aDatasources.end();
            ++aLoop
        )
    {
        if (aLoop->second.pModifications)
            delete aLoop->second.pModifications;
    }
    m_aDatasources.clear();

    // the deleted ones
    clearDeleted();
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
        return ODatasourceInfo(NULL, ::rtl::OUString(), aFallback, -1);
    }

    return ODatasourceInfo(this, aPos->first, aPos->second, -1);
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

//-------------------------------------------------------------------------
//- ODatasourceMap::Iterator
//-------------------------------------------------------------------------
ODatasourceMap::Iterator::Iterator(ODatasourceMap* _pOwner, ODatasourceMap::ConstDatasourceInfosIterator _rPos)
    :m_pOwner(_pOwner)
    ,m_aPos(_rPos)
    ,m_aPosDeleted(_pOwner->m_aDeletedDatasources.end())
    ,m_bLoopingDeleted(sal_False)
{
}

//-------------------------------------------------------------------------
ODatasourceMap::Iterator::Iterator(ODatasourceMap* _pOwner, ODatasourceMap::ConstMapInt2InfoIterator _rPos)
    :m_pOwner(_pOwner)
    ,m_aPos(_pOwner->m_aDatasources.end())
    ,m_aPosDeleted(_rPos)
    ,m_bLoopingDeleted(sal_True)
{
}

//-------------------------------------------------------------------------
ODatasourceMap::Iterator::Iterator(const ODatasourceMap::Iterator& _rSource)
    :m_pOwner(_rSource.m_pOwner)
    ,m_aPos(_rSource.m_aPos)
    ,m_aPosDeleted(_rSource.m_aPosDeleted)
    ,m_bLoopingDeleted(_rSource.m_bLoopingDeleted)
{
}

//-------------------------------------------------------------------------
const ODatasourceMap::Iterator& ODatasourceMap::Iterator::operator++()
{
    if (m_bLoopingDeleted)
        ++m_aPosDeleted;
    else
        ++m_aPos;
    return *this;
}

//-------------------------------------------------------------------------
const ODatasourceMap::Iterator& ODatasourceMap::Iterator::operator--()
{
    if (m_bLoopingDeleted)
        --m_aPosDeleted;
    else
        --m_aPos;
    return *this;
}

//-------------------------------------------------------------------------
::rtl::OUString ODatasourceMap::Iterator::implGetName(const ODatasourceMap::DatasourceInfo& _rInfo) const
{
    ::rtl::OUString sName;
    if (_rInfo.pModifications)
    {
        SFX_ITEMSET_GET(*_rInfo.pModifications, pName, SfxStringItem, DSID_NAME, sal_True);
        sName = pName->GetValue();
    }
    else
    {
        DBG_ASSERT(_rInfo.xDatasource.is(), "ODatasourceMap::Iterator::implGetName: no modifications, no object ... How am I supposed to get the name?");
        if (_rInfo.xDatasource.is())
        {
            try
            {
                _rInfo.xDatasource->getPropertyValue(PROPERTY_NAME) >>= sName;
            }
            catch(Exception&)
            {
                DBG_ERROR("ODatasourceMap::operator->: could not obtain the data source name!");
            }
        }
    }
    return sName;
}

//-------------------------------------------------------------------------
ODatasourceMap::ODatasourceInfo ODatasourceMap::Iterator::operator->() const
{
    if (!m_bLoopingDeleted)
        return ODatasourceInfo(m_pOwner, m_aPos->first, m_aPos->second, -1);
    else
        return ODatasourceInfo(m_pOwner, implGetName(m_aPosDeleted->second), m_aPosDeleted->second, m_aPosDeleted->first);
}

//-------------------------------------------------------------------------
ODatasourceMap::ODatasourceInfo ODatasourceMap::Iterator::operator*() const
{
    if (!m_bLoopingDeleted)
        return ODatasourceInfo(m_pOwner, m_aPos->first, m_aPos->second, -1);
    else
        return ODatasourceInfo(m_pOwner, implGetName(m_aPosDeleted->second), m_aPosDeleted->second, m_aPosDeleted->first);
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
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_JDBCDRIVERCLASS, ::rtl::OUString::createFromAscii("JDBCDRV")));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_TEXTFILEEXTENSION, ::rtl::OUString::createFromAscii("Extension")));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_CHARSET, ::rtl::OUString::createFromAscii("CharSet")));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_TEXTFILEHEADER, ::rtl::OUString::createFromAscii("HeaderLine")));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_FIELDDELIMITER, ::rtl::OUString::createFromAscii("FieldDelimiter")));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_TEXTDELIMITER, ::rtl::OUString::createFromAscii("StringDelimiter")));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_DECIMALDELIMITER, ::rtl::OUString::createFromAscii("DecimalDelimiter")));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_SHOWDELETEDROWS, ::rtl::OUString::createFromAscii("ShowDeleted")));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_ALLOWLONGTABLENAMES, ::rtl::OUString::createFromAscii("NoNameLengthLimit")));

    // enable an apply button
    EnableApplyButton(sal_True);
    SetApplyHandler(LINK(this, ODbAdminDialog, OnApplyChanges));
    // disable the apply button
    GetApplyButton()->Enable(sal_False);

    // register the view window
    SetViewWindow(&m_aSelector);
    SetViewAlign(WINDOWALIGN_LEFT);

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

    _rDriverParam = Sequence< PropertyValue >(aReturn.begin(), aReturn.size());

    // append all the other stuff (charset etc.)
    fillDatasourceInfo(*GetExampleSet(), _rDriverParam);

    return sal_True;
}

//-------------------------------------------------------------------------
short ODbAdminDialog::Ok()
{
    short nResult = SfxTabDialog::Ok();
    return (AR_LEAVE_MODIFIED == implApplyChanges()) ? RET_OK : RET_CANCEL;
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
    *pCounter++ = new SfxStringItem(DSID_FIELDDELIMITER, ';');
    *pCounter++ = new SfxStringItem(DSID_TEXTDELIMITER, '"');
    *pCounter++ = new SfxStringItem(DSID_DECIMALDELIMITER, '.');
    *pCounter++ = new SfxStringItem(DSID_THOUSANDSDELIMITER, ',');
    *pCounter++ = new SfxStringItem(DSID_TEXTFILEEXTENSION, String::CreateFromAscii("txt"));
    *pCounter++ = new SfxBoolItem(DSID_TEXTFILEHEADER, sal_True);
    *pCounter++ = new SfxBoolItem(DSID_NEWDATASOURCE, sal_False);
    *pCounter++ = new SfxBoolItem(DSID_DELETEDDATASOURCE, sal_False);
    *pCounter++ = new SfxBoolItem(DSID_SUPPRESSVERSIONCL, sal_True);

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
    ::rtl::OUString sNewStringSuggestion = _pTabPage->GetCurrentName();

    // check if there's already an data source with the suggested name
    ConstStringSetIterator aExistentPos = m_aValidDatasources.find(sNewStringSuggestion);
        // !! m_aValidDatasources contains _all_ data source names _except_ the currently selected one !!

    sal_Bool bValid = m_aValidDatasources.end() == aExistentPos;

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
            m_aDatasources.update(sSelected, *pExampleSet);
        }

        // enable the apply button
        GetApplyButton()->Enable(sal_True);

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
void ODbAdminDialog::implSelectDeleted(sal_Int32 _nKey)
{
    m_aSelector.select(_nKey);

    // insert the previously selected data source into our "all valid datasources" set
    if (m_sCurrentDatasource.getLength())   // previous selection was not on a deleted data source
        m_aValidDatasources.insert(m_sCurrentDatasource);
    m_sCurrentDatasource = ::rtl::OUString();

    // reset the tag pages
    resetPages(Reference< XPropertySet >(), sal_True);
}

//-------------------------------------------------------------------------
void ODbAdminDialog::implSelectDatasource(const ::rtl::OUString& _rRegisteredName)
{
    m_aSelector.select(_rRegisteredName);

    // insert the previously selected data source into our set
    if (m_sCurrentDatasource.getLength())   // previous selection was not on a deleted data source
        m_aValidDatasources.insert(m_sCurrentDatasource);
    m_sCurrentDatasource = _rRegisteredName;
    // remove the now selected data source from our set
    m_aValidDatasources.erase(m_sCurrentDatasource);

    // reset the tag pages
    Reference< XPropertySet > xDatasource = getDatasource(_rRegisteredName);
    resetPages(xDatasource, sal_False);
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
    translateProperties(_rxDatasource, *GetInputSetImpl());

    // reset some meta data items in the input set which are for tracking the state of the current ds
    GetInputSetImpl()->Put(SfxBoolItem(DSID_NEWDATASOURCE, sal_False));
    GetInputSetImpl()->Put(SfxBoolItem(DSID_DELETEDDATASOURCE, _bDeleted));

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
            _rSet.Put(SfxStringItem(_nId, sValue.getStr()));
        }
        break;
        case TypeClass_BOOLEAN:
            _rSet.Put(SfxBoolItem(_nId, ::cppu::any2bool(_rValue)));
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
                    _rSet.Put(OStringListItem(_nId, aStringList));
                }
                break;
                default:
                    DBG_ERROR("ODbAdminDialog::implTranslateProperty: unsupported property value type!");
            }
        }
        break;
        case TypeClass_VOID:
            _rSet.ClearItem(_nId);
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
            aInfos.insert(*pAdditionalInfo);

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
        const SfxPoolItem* pCurrentItem = _rSource.GetItem(aDirect->first);
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
void ODbAdminDialog::fillDatasourceInfo(const SfxItemSet& _rSource, ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rInfo)
{
    // within the current "Info" sequence, replace the ones we can examine from the item set
    // (we don't just fill a completely new sequence with our own items, but we preserve any properties unknown to
    // us)

    // first determine which of all the items are relevant for the data source (depends on the connection url)
    sal_Int32* pRelevantItems = NULL;

    SFX_ITEMSET_GET(_rSource, pConnectURL, SfxStringItem, DSID_CONNECTURL, sal_True);
    SFX_ITEMSET_GET(_rSource, pTypeCollection, DbuTypeCollectionItem, DSID_TYPECOLLECTION, sal_True);
    DBG_ASSERT(pConnectURL && pTypeCollection, "ODbAdminDialog::translateProperties: invalid items in the source set!");
    String sConnectURL = pConnectURL->GetValue();
    ODsnTypeCollection* pCollection = pTypeCollection->getCollection();
    DBG_ASSERT(pCollection, "ODbAdminDialog::translateProperties: invalid type collection!");
    DATASOURCE_TYPE eType = pCollection->getType(sConnectURL);
    switch (eType)
    {
        case DST_ADABAS: pRelevantItems = OAdabasDetailsPage::getDetailIds(); break;
        case DST_JDBC: pRelevantItems = OJdbcDetailsPage::getDetailIds(); break;
        case DST_ODBC: pRelevantItems = OOdbcDetailsPage::getDetailIds(); break;
        case DST_DBASE: pRelevantItems = ODbaseDetailsPage::getDetailIds(); break;
        case DST_TEXT: pRelevantItems = OTextDetailsPage::getDetailIds(); break;
    }
    DBG_ASSERT(pRelevantItems, "ODbAdminDialog::translateProperties: invalid item ids got from the page!");

    // collect the translated property values for the relevant items
    PropertyValueSet aRelevantSettings;
    ConstMapInt2StringIterator aTranslation;
    while (*pRelevantItems)
    {
        const SfxPoolItem* pCurrent = _rSource.GetItem(*pRelevantItems);
        aTranslation = m_aIndirectPropTranslator.find(*pRelevantItems);
        if (pCurrent && (m_aIndirectPropTranslator.end() != aTranslation))
            aRelevantSettings.insert(PropertyValue(aTranslation->second, 0, implTranslateProperty(pCurrent), PropertyState_DIRECT_VALUE));

        ++pRelevantItems;
    }

    // now aRelevantSettings contains all the property values relevant for the current data source type,
    // check the original sequence if it already contains any of these values (which have to be overwritten, then)
    PropertyValue* pInfo = _rInfo.getArray();
    PropertyValue aSearchFor;
    for (sal_Int32 i=0; i<_rInfo.getLength(); ++i, ++pInfo)
    {
        aSearchFor.Name = pInfo->Name;
        PropertyValueSetIterator aOverwrittenSetting = aRelevantSettings.find(aSearchFor);
        if (aRelevantSettings.end() != aOverwrittenSetting)
        {   // the setting was present in the original sequence, and it is to be overwritten -> replace it
            *pInfo = *aOverwrittenSetting;
            aRelevantSettings.erase(aOverwrittenSetting);
        }
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
IMPL_LINK(ODbAdminDialog, OnNewDatasource, Window*, _pWindow)
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
ODbAdminDialog::ApplyResult ODbAdminDialog::implApplyChanges()
{
    if (!PrepareLeaveCurrentPage())
    {   // the page did not allow us to leave
        return AR_KEEP;
    }

    ApplyResult eResult = AR_LEAVE_UNCHANGED;

    // save the settings for the currently selected data source
    if (DELETED != m_aSelector.getSelectedState())
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

    // disable the apply button
    GetApplyButton()->Enable(sal_False);

    return eResult;
}

//-------------------------------------------------------------------------
IMPL_LINK(ODbAdminDialog, OnApplyChanges, PushButton*, EMPTYARG)
{
    implApplyChanges();
    return 0L;
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

    m_aNewDatasource.SetClickHdl(LINK(this, ODatasourceSelector, OnButtonPressed));
}

//-------------------------------------------------------------------------
ODatasourceSelector::~ODatasourceSelector()
{
    for (sal_Int32 i=0; i<m_aDatasourceList.GetEntryCount(); ++i)
        delete reinterpret_cast<EntryData*>(m_aDatasourceList.GetEntryData(i));
}

//-------------------------------------------------------------------------
sal_Int32 ODatasourceSelector::getImageId(DatasourceState _eState)
{
    switch (_eState)
    {
        case CLEAN: return IMG_DATABASE;
        case MODIFIED:  return IMG_DATABASE_MODIFIED;
        case NEW:       return IMG_DATABASE_NEW;
        case DELETED:   return IMG_DATABASE_DELETED;
    }
    DBG_ERROR("ODatasourceSelector::getImage: invalid state");
    return 0;
}

//-------------------------------------------------------------------------
DatasourceState ODatasourceSelector::getEntryState(sal_Int32 _nPos) const
{
    EntryData* pData = static_cast<EntryData*>(m_aDatasourceList.GetEntryData(_nPos));
    if (!pData)
        return CLEAN;
    return pData->eState;
}

//-------------------------------------------------------------------------
void ODatasourceSelector::setEntryState(sal_Int32 _nPos, DatasourceState _eState)
{
    EntryData* pData = static_cast<EntryData*>(m_aDatasourceList.GetEntryData(_nPos));
    if (pData ? _eState == pData->eState : CLEAN == _eState)
        // nothing changed
        return;

    // to restore the selection afterwards ...
    sal_Bool bWasSelected = m_aDatasourceList.GetSelectEntryPos() == _nPos;

    String sName = m_aDatasourceList.GetEntry(_nPos);
    m_aDatasourceList.RemoveEntry(_nPos);
    _nPos = m_aDatasourceList.InsertEntry(sName, Image(ModuleRes(getImageId(_eState))), _nPos);

    if (!pData)
        pData = new EntryData;
    pData->eState = _eState;
    m_aDatasourceList.SetEntryData(_nPos, static_cast<void*>(pData));

    if (bWasSelected)
        m_aDatasourceList.SelectEntryPos(_nPos, sal_True);
}

//-------------------------------------------------------------------------
sal_Int32 ODatasourceSelector::getAccessKey(sal_Int32 _nPos) const
{
    EntryData* pData = static_cast<EntryData*>(m_aDatasourceList.GetEntryData(_nPos));
    if (!pData)
        return NULL;    // entry is in default state
    return pData->nAccessKey;
}

//-------------------------------------------------------------------------
void ODatasourceSelector::setAccessKey(sal_Int32 _nPos, sal_Int32 _nAccessKey)
{
    EntryData* pData = static_cast<EntryData*>(m_aDatasourceList.GetEntryData(_nPos));
    DBG_ASSERT(pData, "ODatasourceSelector::setAccessKey: to be called for entry in DELETED state only!");
    if (!pData)
    {
        pData = new EntryData(DELETED);
        m_aDatasourceList.SetEntryData(_nPos, static_cast<void*>(pData));
    }
    pData->nAccessKey = _nAccessKey;
}

//-------------------------------------------------------------------------
void ODatasourceSelector::implDeleted(sal_Int32 _nPos)
{
    // remove the entry
    m_aDatasourceList.RemoveEntry(_nPos);

    // select the one below (or above, if it was the last one)
    if (_nPos >= m_aDatasourceList.GetEntryCount())
        _nPos = m_aDatasourceList.GetEntryCount() - 1;
    m_aDatasourceList.SelectEntryPos(_nPos);
    // call the select handler to propagate the new selection
    m_aDatasourceList.GetSelectHdl().Call(&m_aDatasourceList);
}

//-------------------------------------------------------------------------
void ODatasourceSelector::deleted(sal_Int32 _nAccessKey)
{
    sal_Int32 nPos = getDeletedEntryPos(_nAccessKey);
    if (-1 != nPos)
        implDeleted(nPos);
    else
        DBG_ERROR("ODatasourceSelector::deleted: no deleted entry wiht that name!");
}

//-------------------------------------------------------------------------
void ODatasourceSelector::deleted(const String& _rName)
{
    sal_Int32 nPos = getValidEntryPos(_rName);
    if (-1 != nPos)
        implDeleted(nPos);
    else
        DBG_ERROR("ODatasourceSelector::deleted: no non-deleted entry wiht that name!");
}

//-------------------------------------------------------------------------
void ODatasourceSelector::restoreDeleted(sal_Int32 _nAccessKey, DatasourceState _eState)
{
    DBG_ASSERT(DELETED != _eState, "ODatasourceSelector::restoreDeleted: invalid datasource state!");
    sal_Int32 nPos = getDeletedEntryPos(_nAccessKey);
    if (-1 == nPos)
    {
        DBG_ERROR("ODatasourceSelector::restoreDeleted: invalid access key!");
        return;
    }
    DBG_ASSERT(-1 == getValidEntryPos(m_aDatasourceList.GetEntry(nPos)),
        "ODatasourceSelector::restoreDeleted: already have a not-deleted entry with that name!");

    setEntryState(nPos, _eState);
    setAccessKey(nPos, 0);
}

//-------------------------------------------------------------------------
void ODatasourceSelector::markDeleted(const String& _rName, sal_Int32 _nAccessKey)
{
    sal_Int32 nPos = getValidEntryPos(_rName);
    DBG_ASSERT(-1 != nPos, "ODatasourceSelector::markDeleted: no non-deleted entry wiht that name!");
    setEntryState(nPos, DELETED);
    DBG_ASSERT(_nAccessKey, "ODatasourceSelector::markDeleted: invalid access key!");
    DBG_ASSERT(-1 == getDeletedEntryPos(_nAccessKey), "ODatasourceSelector::modified: already have a deleted entry with this access key!");
    setAccessKey(nPos, _nAccessKey);
}

//-------------------------------------------------------------------------
void ODatasourceSelector::modified(const String& _sName)
{
    sal_Int32 nPos = getValidEntryPos(_sName);
    DBG_ASSERT(-1 != nPos, "ODatasourceSelector::modified: invalid name!");
    setEntryState(nPos, MODIFIED);
}

//-------------------------------------------------------------------------
sal_Int32 ODatasourceSelector::getDeletedEntryPos(sal_Int32 _nAccessKey)
{
    for (sal_Int32 i=0; i<m_aDatasourceList.GetEntryCount(); ++i)
        if ((DELETED == getEntryState(i)) && (_nAccessKey == getAccessKey(i)))
            return i;

    return -1;
}

//-------------------------------------------------------------------------
sal_Int32 ODatasourceSelector::getValidEntryPos(const String& _rName)
{
    // first guess
    sal_Int32 nPos = m_aDatasourceList.GetEntryPos(_rName);
    while (DELETED == getEntryState(nPos))
    {
        // unfortunally we no such thing as "GetEntryPos(<name>, <startindex>) ... -> search manually
        while ((++nPos<m_aDatasourceList.GetEntryCount()))
            if (m_aDatasourceList.GetEntry(nPos).Equals(_rName))
                // have a candidate
                break;
        if (nPos>=m_aDatasourceList.GetEntryCount())
            // did not find another candidate
            return -1;
    }
    return nPos;
}

//-------------------------------------------------------------------------
void ODatasourceSelector::renamed(const String& _rOldName, const String& _rNewName)
{
    sal_Int32 nPos = getValidEntryPos(_rOldName);
    DBG_ASSERT(-1 != nPos, "ODatasourceSelector::renamed: invalid old name!");

    // save the info about the entry
    DatasourceState eState = getEntryState(nPos);
    EntryData* pData = static_cast<EntryData*>(m_aDatasourceList.GetEntryData(nPos));

    m_aDatasourceList.RemoveEntry(nPos);
    nPos = m_aDatasourceList.InsertEntry(_rNewName, Image(ModuleRes(getImageId(eState))), nPos);

    // restore the info about the entry
    m_aDatasourceList.SetEntryData(nPos, static_cast<void*>(pData));
}

//-------------------------------------------------------------------------
sal_Int32 ODatasourceSelector::insert(const String& _rName)
{
    sal_Int16 nPos = m_aDatasourceList.InsertEntry(_rName, Image(ModuleRes(getImageId(CLEAN))));
    m_aDatasourceList.SetEntryData(nPos, static_cast<void*>(NULL));
    return nPos;
}

//-------------------------------------------------------------------------
void ODatasourceSelector::flushed(const String& _rName)
{
    sal_Int32 nPos = getValidEntryPos(_rName);
    DBG_ASSERT(-1 != nPos, "ODatasourceSelector::flushed: invalid data source name!");

    setEntryState(nPos, CLEAN);
}

//-------------------------------------------------------------------------
void ODatasourceSelector::insertNew(const String& _rName)
{
    // insert the new entry with the appropriate image
    sal_Int16 nPos = m_aDatasourceList.InsertEntry(_rName, Image(ModuleRes(getImageId(NEW))));
    m_aDatasourceList.SetEntryData(nPos, static_cast<void*>(new EntryData(NEW)));
}

//-------------------------------------------------------------------------
void ODatasourceSelector::select(const String& _rName)
{
    sal_Int32 nPos = getValidEntryPos(_rName);
    DBG_ASSERT(-1 != nPos, "ODatasourceSelector::select: invalid data source name (maybe deleted or not existent)!");

    m_aDatasourceList.SelectEntryPos(nPos);
}

//-------------------------------------------------------------------------
void ODatasourceSelector::select(sal_Int32 _nAccessKey)
{
    sal_Int32 nPos = getDeletedEntryPos(_nAccessKey);
    DBG_ASSERT(-1 != nPos, "ODatasourceSelector::select: invalid access key (have no such entry)!");

    m_aDatasourceList.SelectEntryPos(nPos);
}

//-------------------------------------------------------------------------
long ODatasourceSelector::Notify(NotifyEvent& _rNEvt)
{
    sal_Bool bHandled = sal_False;

    switch (_rNEvt.GetType())
    {
        case EVENT_COMMAND:
        {
            const CommandEvent* pCommand = _rNEvt.GetCommandEvent();
            if (COMMAND_CONTEXTMENU == pCommand->GetCommand())
            {
                // check if the context menu request occured in the listbox
                if  (m_aDatasourceList.IsChild(_rNEvt.GetWindow()))
                {
                    // calc the pos where to open the menu
                    Point aWhere;
                    if (pCommand->IsMouseEvent())
                    {
                        aWhere = pCommand->GetMousePosPixel();
                    }
                    else
                    {   // context menu via keyboard -> assume the center of the currently selected data source
                        // TODO: use another class instead of the listbox (e.g. a SvTreeListBox). We have no change
                        // to get an item rect or the item height or something like that from a listbox ...
                        aWhere = Point(0, 0);
                    }
                    PopupMenu aMenu(ModuleRes(MENU_DATASOURCELIST_POPUP));
                    aMenu.SetMenuFlags(aMenu.GetMenuFlags() | MENU_FLAG_HIDEDISABLEDENTRIES);

                    // we don't really know if adding datasources is allowed currently. But an other instance does
                    // (the dialog), and this instance en- or disables our "new datasource" button
                    aMenu.EnableItem(MID_NEW_DATASOURCE, m_aNewDatasource.IsEnabled());

                    DatasourceState eState = getEntryState(m_aDatasourceList.GetSelectEntryPos());
                    aMenu.EnableItem(MID_DELETE_DATASOURCE, DELETED != eState);
                    aMenu.EnableItem(MID_RESTORE_DATASOURCE, DELETED == eState);

                    switch (aMenu.Execute(_rNEvt.GetWindow(), aWhere))
                    {
                        case MID_NEW_DATASOURCE:
                            m_aNewHandler.Call(this);
                            break;
                        case MID_DELETE_DATASOURCE:
                            m_aDeleteHandler.Call(this);
                            break;
                        case MID_RESTORE_DATASOURCE:
                            m_aRestoreHandler.Call(this);
                            break;
                    }
                    bHandled = sal_True;
                }
            }
        }
        break;
        case EVENT_KEYINPUT:
        {
            const KeyEvent* pKeyEvent = _rNEvt.GetKeyEvent();
            const KeyCode& rKeyCode = pKeyEvent->GetKeyCode();
            if ((0 == rKeyCode.GetAllModifier()) && ((sal_uInt16)-1 != m_aDatasourceList.GetSelectEntryPos()))
                switch (rKeyCode.GetCode())
                {
                    case KEY_DELETE:
                        m_aDeleteHandler.Call(this);
                        bHandled = sal_True;
                        break;
                    case KEY_INSERT:
                        m_aNewHandler.Call(this);
                        bHandled = sal_True;
                        break;
                }
        }
        break;
    }

    return bHandled ? 1 : Window::Notify(_rNEvt);
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

//-------------------------------------------------------------------------
IMPL_LINK(ODatasourceSelector, OnButtonPressed, Button*, EMPTYARG)
{
    return m_aNewHandler.Call(this);
}

//.........................................................................
}   // namespace dbaui
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.9  2000/10/26 07:31:30  fs
 *  introduced fillDatasourceInfo, with this now getCurrentSettings collects _all_ relevant properties
 *
 *  Revision 1.8  2000/10/24 12:12:26  fs
 *  ODatasourceMap::update takes a non-constant set (to reset the ORIGINALNAME item)
 *
 *  Revision 1.7  2000/10/23 12:56:50  fs
 *  added apply functionality
 *
 *  Revision 1.6  2000/10/20 09:53:17  fs
 *  handling for the SuppresVersionColumns property of a data source
 *
 *  Revision 1.5  2000/10/13 16:06:20  fs
 *  implemented the usage if the 'Info' property of the data sources / allow key usage in the data source list
 *
 *  Revision 1.4  2000/10/12 16:20:42  fs
 *  new implementations ... still under construction
 *
 *  Revision 1.3  2000/10/11 11:31:03  fs
 *  new implementations - still under construction
 *
 *  Revision 1.2  2000/10/09 12:39:29  fs
 *  some (a lot of) new imlpementations - still under development
 *
 *  Revision 1.1  2000/10/05 10:04:31  fs
 *  initial checkin
 *
 *
 *  Revision 1.0 20.09.00 10:55:58  fs
 ************************************************************************/

