/*************************************************************************
 *
 *  $RCSfile: configmgr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:54 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _UTL_CONFIGMGR_HXX_
#include "unotools/configmgr.hxx"
#endif
#ifndef _UTL_CONFIGITEM_HXX_
#include "unotools/configitem.hxx"
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XMULTIPROPERTYSET_HPP_
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#include <list>

//-----------------------------------------------------------------------------

using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;

#define C2U(cChar) OUString::createFromAscii(cChar)
//-----------------------------------------------------------------------------
const char* cConfigBaseURL = "/org.openoffice.";
//const char* cConfigBaseURL = "/com.sun.star.";
const char* cAccessSrvc = "com.sun.star.configuration.ConfigurationUpdateAccess";

//-----------------------------------------------------------------------------
struct ConfigItemListEntry_Impl
{
    ConfigItem*                 pConfigItem;

    ConfigItemListEntry_Impl(ConfigItem*    pItem ) :
        pConfigItem(pItem){}
};
typedef std::list<ConfigItemListEntry_Impl> ConfigItemList;
struct utl::ConfigMgr_Impl
{
    ConfigItemList aItemList;
};

/* -----------------------------28.08.00 15:35--------------------------------

 ---------------------------------------------------------------------------*/
ConfigManager::ConfigManager() :
    pMgrImpl(new utl::ConfigMgr_Impl)
{
}
/* -----------------------------28.08.00 15:35--------------------------------

 ---------------------------------------------------------------------------*/
ConfigManager::~ConfigManager()
{
    //check list content -> should be empty!
    DBG_ASSERT(pMgrImpl->aItemList.empty(), "some ConfigItems are still alive")
    if(!pMgrImpl->aItemList.empty())
    {
        ConfigItemList::iterator aListIter;
        for(aListIter = pMgrImpl->aItemList.begin(); aListIter != pMgrImpl->aItemList.end(); ++aListIter)
        {
            ConfigItemListEntry_Impl& rEntry = *aListIter;
            rEntry.pConfigItem->ReleaseConfigMgr();
        }
        pMgrImpl->aItemList.erase(pMgrImpl->aItemList.begin(), pMgrImpl->aItemList.end());
    }
    delete pMgrImpl;

}
/* -----------------------------28.08.00 16:17--------------------------------

 ---------------------------------------------------------------------------*/
Reference< XMultiServiceFactory > ConfigManager::GetConfigurationProvider()
{
    if(!xConfigurationProvider.is())
    {
        Reference< XMultiServiceFactory > xMSF = ::utl::getProcessServiceFactory();
        xConfigurationProvider = Reference< XMultiServiceFactory >
            (xMSF->createInstance(
                    C2U("com.sun.star.configuration.ConfigurationProvider")),
                        UNO_QUERY);
    }
    return xConfigurationProvider;
}
/* -----------------------------29.08.00 12:35--------------------------------

 ---------------------------------------------------------------------------*/
Reference< XMultiPropertySet> ConfigManager::AddConfigItem(utl::ConfigItem& rCfgItem)
{
    ConfigItemList::iterator aListIter = pMgrImpl->aItemList.begin();
#ifdef DBG_UTIL
    for(aListIter = pMgrImpl->aItemList.begin(); aListIter != pMgrImpl->aItemList.end(); ++aListIter)
    {
        ConfigItemListEntry_Impl& rEntry = *aListIter;
        if(rEntry.pConfigItem == &rCfgItem)
            DBG_ERROR("AddConfigItem: already inserted!")
    }
#endif
    OUString sPath = C2U(cConfigBaseURL);
    sPath += rCfgItem.GetSubTreeName();
    Sequence< Any > aArgs(1);
    aArgs[0] <<= sPath;

    Reference< XMultiServiceFactory > xCfgProvider = GetConfigurationProvider();
    Reference< XInterface > xIFace;
    try
    {
        xIFace = xCfgProvider->createInstanceWithArguments(
                C2U(cAccessSrvc),
                aArgs);
        pMgrImpl->aItemList.insert(aListIter, ConfigItemListEntry_Impl(&rCfgItem));
#ifdef DEBUG
        Reference<XNameAccess> xNA(xIFace, UNO_QUERY);
        Sequence<OUString> aNames = xNA->getElementNames();
        const OUString* pNames = aNames.getConstArray();
#endif
    }
#ifdef DBG_UTIL
    catch(Exception& rEx)
    {
        ByteString sMsg("CreateInstance exception: ");
        sMsg += ByteString(String(rEx.Message), RTL_TEXTENCODING_ASCII_US);
        DBG_ERROR(sMsg.GetBuffer())
    }
#else
    catch(Exception&){}
#endif
    return Reference<XMultiPropertySet>(xIFace, UNO_QUERY);
}
/* -----------------------------29.08.00 12:35--------------------------------

 ---------------------------------------------------------------------------*/
void ConfigManager::RemoveConfigItem(utl::ConfigItem& rCfgItem)
{
    DBG_ASSERT(!pMgrImpl->aItemList.empty(), "no ConfigItems available")
    ConfigItemList::iterator aListIter = pMgrImpl->aItemList.begin();
    for(aListIter = pMgrImpl->aItemList.begin(); aListIter != pMgrImpl->aItemList.end(); ++aListIter)
    {
        ConfigItemListEntry_Impl& rEntry = *aListIter;
        if(rEntry.pConfigItem == &rCfgItem)
        {
            pMgrImpl->aItemList.erase(aListIter);
            break;
        }
    }
}
/* -----------------------------30.08.00 15:04--------------------------------

 ---------------------------------------------------------------------------*/
void ConfigManager::StoreConfigItems()
{
    if(!pMgrImpl->aItemList.empty())
    {
        ConfigItemList::iterator aListIter = pMgrImpl->aItemList.begin();
        for(aListIter = pMgrImpl->aItemList.begin(); aListIter != pMgrImpl->aItemList.end(); ++aListIter)
        {
            ConfigItemListEntry_Impl& rEntry = *aListIter;
            if(rEntry.pConfigItem->IsModified())
                rEntry.pConfigItem->Commit();
        }
    }
}
ConfigManager*   ConfigManager::pConfigManager = 0;
/* -----------------------------07.09.00 11:06--------------------------------

 ---------------------------------------------------------------------------*/
ConfigManager*  ConfigManager::GetConfigManager()
{
    if(!pConfigManager)
    {
        pConfigManager = new ConfigManager();
    }
    return pConfigManager;
}
/* -----------------------------07.09.00 11:06--------------------------------

 ---------------------------------------------------------------------------*/
void    ConfigManager::RemoveConfigManager()
{
    if(pConfigManager)
    {
        delete pConfigManager;
        pConfigManager = 0;
    }
}
/* -----------------------------08.09.00 13:22--------------------------------

 ---------------------------------------------------------------------------*/
rtl::OUString ConfigManager::GetConfigBaseURL()
{
    return C2U(cConfigBaseURL);
}

