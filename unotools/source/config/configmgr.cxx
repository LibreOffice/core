/*************************************************************************
 *
 *  $RCSfile: configmgr.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: os $ $Date: 2000-10-13 13:58:29 $
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
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
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
    OSL_ENSHURE(pMgrImpl->aItemList.empty(), "some ConfigItems are still alive");
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
Reference< XHierarchicalNameAccess > ConfigManager::AddConfigItem(utl::ConfigItem& rCfgItem)
{
    ConfigItemList::iterator aListIter = pMgrImpl->aItemList.begin();
#ifdef DBG_UTIL
    for(aListIter = pMgrImpl->aItemList.begin(); aListIter != pMgrImpl->aItemList.end(); ++aListIter)
    {
        ConfigItemListEntry_Impl& rEntry = *aListIter;
        if(rEntry.pConfigItem == &rCfgItem)
            OSL_DEBUG_ONLY("AddConfigItem: already inserted!");
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
        OString sMsg("CreateInstance exception: ");
        sMsg += OString(rEx.Message.getStr(),
                    rEx.Message.getLength(),
                     RTL_TEXTENCODING_ASCII_US);
        OSL_DEBUG_ONLY(sMsg.getStr());
    }
#else
    catch(Exception&){}
#endif
    return Reference<XHierarchicalNameAccess>(xIFace, UNO_QUERY);
}
/* -----------------------------29.08.00 12:35--------------------------------

 ---------------------------------------------------------------------------*/
void ConfigManager::RemoveConfigItem(utl::ConfigItem& rCfgItem)
{
    OSL_ENSHURE(!pMgrImpl->aItemList.empty(), "no ConfigItems available");
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
/* -----------------------------25.09.00 16:34--------------------------------

 ---------------------------------------------------------------------------*/
Any ConfigManager::GetDirectConfigProperty(ConfigProperty eProp)
{
    Any aRet;
    OUString sPath = C2U(cConfigBaseURL);
    switch(eProp)
    {
        case INSTALLPATH:   sPath += C2U("UserProfile/Office"); break;
        case LOCALE:            sPath += C2U("UserProfile/International"); break;
        case OFFICEINSTALL: sPath += C2U("Office.Common/Path"); break;
    }
    Sequence< Any > aArgs(1);
    aArgs[0] <<= sPath;
    Reference< XMultiServiceFactory > xCfgProvider = GetConfigManager()->GetConfigurationProvider();
    Reference< XInterface > xIFace;
    try
    {
        xIFace = xCfgProvider->createInstanceWithArguments(
                C2U(cAccessSrvc),
                aArgs);

    }
    catch(Exception&){}
    Reference<XHierarchicalNameAccess> xHierarchyAccess(xIFace, UNO_QUERY);
    if(xHierarchyAccess.is())
    {
        OUString sProperty;
        switch(eProp)
        {
            case INSTALLPATH:   sProperty = C2U("InstallPath"); break;
            case LOCALE:        sProperty = C2U("Locale"); break;
            case OFFICEINSTALL: sProperty = C2U("OfficeInstall"); break;
        }
        try
        {
            aRet = xHierarchyAccess->getByHierarchicalName(sProperty);
        }
        catch(Exception&) {}
    }
    return aRet;
}

