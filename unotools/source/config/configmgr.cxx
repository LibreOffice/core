/*************************************************************************
 *
 *  $RCSfile: configmgr.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-20 16:38:51 $
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
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
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

static ::rtl::OUString aBrandName;
static ::rtl::OUString aProductVersion;
static ::rtl::OUString aProductExtension;

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
    sal_Bool                                bIsLocalProvider;
    ConfigItemList                          aItemList;
    Reference< XMultiServiceFactory >       xLocalConfigurationProvider;

};

/* -----------------------------28.08.00 15:35--------------------------------

 ---------------------------------------------------------------------------*/
ConfigManager::ConfigManager() :
    pMgrImpl(new utl::ConfigMgr_Impl)
{
    Reference< XMultiServiceFactory > xCfgProv = GetConfigurationProvider();
    Reference< XMultiServiceFactory > xLocalCfgProv = GetLocalConfigurationProvider();
    if(xCfgProv.get() && xCfgProv.get() == xLocalCfgProv.get())
        pMgrImpl->bIsLocalProvider = sal_False;
    else
        pMgrImpl->bIsLocalProvider = sal_True;
}
/* -----------------------------17.11.00 13:51--------------------------------

 ---------------------------------------------------------------------------*/
ConfigManager::ConfigManager(Reference< XMultiServiceFactory > xConfigProv) :
    pMgrImpl(new utl::ConfigMgr_Impl),
    xConfigurationProvider(xConfigProv)
{
    pMgrImpl->bIsLocalProvider = sal_False;
}
/* -----------------------------28.08.00 15:35--------------------------------

 ---------------------------------------------------------------------------*/
ConfigManager::~ConfigManager()
{
    //check list content -> should be empty!
    OSL_ENSURE(pMgrImpl->aItemList.empty(), "some ConfigItems are still alive");
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
        try
          {
            xConfigurationProvider = Reference< XMultiServiceFactory >
            (xMSF->createInstance(
                    C2U("com.sun.star.configuration.ConfigurationProvider")),
                        UNO_QUERY);
        }
#ifdef DBG_UTIL
    catch(Exception& rEx)
    {
        OString sMsg("CreateInstance with arguments exception: ");
        sMsg += OString(rEx.Message.getStr(),
                    rEx.Message.getLength(),
                     RTL_TEXTENCODING_ASCII_US);
        OSL_ENSURE(sal_False, sMsg.getStr());
    }
#else
    catch(Exception&){}
#endif
    }
    return xConfigurationProvider;
}
/* -----------------------------12.12.00 17:19--------------------------------

 ---------------------------------------------------------------------------*/
Reference< XMultiServiceFactory > ConfigManager::GetLocalConfigurationProvider()
{
    if(!pMgrImpl->xLocalConfigurationProvider.is())
    {
        Reference< XMultiServiceFactory > xMSF = ::utl::getProcessServiceFactory();
        Sequence <Any> aArgs(1);
        Any* pValues = aArgs.getArray();
        PropertyValue aPValue;
        aPValue.Name  = C2U("servertype");
        aPValue.Value <<= C2U("plugin");
        pValues[0] <<= aPValue;
        try
          {
            pMgrImpl->xLocalConfigurationProvider = Reference< XMultiServiceFactory >
            (xMSF->createInstanceWithArguments(
                    C2U("com.sun.star.configuration.ConfigurationProvider"), aArgs),
                        UNO_QUERY);
        }
#ifdef DBG_UTIL
    catch(Exception& rEx)
    {
        OString sMsg("CreateInstance with arguments exception: ");
        sMsg += OString(rEx.Message.getStr(),
                    rEx.Message.getLength(),
                     RTL_TEXTENCODING_ASCII_US);
        OSL_ENSURE(sal_False, sMsg.getStr());
    }
#else
    catch(Exception&){}
#endif
    }
    return pMgrImpl->xLocalConfigurationProvider;
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
            OSL_ENSURE(sal_False, "AddConfigItem: already inserted!");
    }
#endif
    OUString sPath = C2U(cConfigBaseURL);
    sPath += rCfgItem.GetSubTreeName();
    Sequence< Any > aArgs(1);
    PropertyValue aPath;
    aPath.Name = C2U("nodepath");
    aPath.Value <<= sPath;
    aArgs[0] <<= aPath;
    if(rCfgItem.GetMode()&CONFIG_MODE_DELAYED_UPDATE)
    {
        aArgs.realloc(2);
        PropertyValue aUpdate;
        aUpdate.Name = C2U("lazywrite");
        sal_Bool bTrue = sal_True;
        aUpdate.Value.setValue(&bTrue, ::getBooleanCppuType());
        aArgs.getArray()[1] <<= aUpdate;
    }

    Reference< XMultiServiceFactory > xCfgProvider = GetConfigurationProvider();
    Reference< XInterface > xIFace;
    if(xCfgProvider.is())
    {
        try
        {
            xIFace = xCfgProvider->createInstanceWithArguments(
                    C2U(cAccessSrvc),
                    aArgs);
            pMgrImpl->aItemList.insert(aListIter, ConfigItemListEntry_Impl(&rCfgItem));
        }
#ifdef DBG_UTIL
        catch(Exception& rEx)
        {
            OString sMsg("CreateInstance exception: ");
            sMsg += OString(rEx.Message.getStr(),
                        rEx.Message.getLength(),
                         RTL_TEXTENCODING_ASCII_US);
            OSL_ENSURE(sal_False, sMsg.getStr());
        }
#else
        catch(Exception&){}
#endif
    }
    return Reference<XHierarchicalNameAccess>(xIFace, UNO_QUERY);
}
/* -----------------------------29.08.00 12:35--------------------------------

 ---------------------------------------------------------------------------*/
void ConfigManager::RemoveConfigItem(utl::ConfigItem& rCfgItem)
{
    OSL_ENSURE(!pMgrImpl->aItemList.empty(), "no ConfigItems available");
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
    ConfigManager * pTheConfigManager = GetConfigManager();
    if (pTheConfigManager->IsLocalConfigProvider())
        switch (eProp)
        {
            case INSTALLPATH:
                return
                    pTheConfigManager->
                        GetLocalProperty(
                            rtl::OUString::createFromAscii(
                                "UserProfile/Office/InstallPath"));

            case OFFICEINSTALL:
                return
                    pTheConfigManager->
                        GetLocalProperty(
                            rtl::OUString::createFromAscii(
                                "Office.Common/Path/Current/OfficeInstall"));
        }

    Any aRet;
    if ( eProp == PRODUCTNAME && aBrandName.getLength() )
    {
        aRet <<= aBrandName;
        return aRet;
    }

    if ( eProp == PRODUCTVERSION && aProductVersion.getLength() )
    {
        aRet <<= aProductVersion;
        return aRet;
    }

    if ( eProp == PRODUCTEXTENSION && aProductExtension.getLength() )
    {
        aRet <<= aProductExtension;
        return aRet;
    }

    OUString sPath = C2U(cConfigBaseURL);
    switch(eProp)
    {
        case INSTALLPATH:
        case USERINSTALLURL:    sPath += C2U("UserProfile/Office"); break;
        case LOCALE:            sPath += C2U("UserProfile/International"); break;
        case PRODUCTNAME:
        case PRODUCTVERSION:
        case PRODUCTEXTENSION:  sPath += C2U("Setup/Product"); break;
        case OFFICEINSTALL:
        case OFFICEINSTALLURL:  sPath += C2U("Office.Common/Path/Current"); break;
    }
    Sequence< Any > aArgs(1);
    aArgs[0] <<= sPath;
    Reference< XMultiServiceFactory > xCfgProvider = GetConfigManager()->GetConfigurationProvider();
    if(!xCfgProvider.is())
        return aRet;
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
            case INSTALLPATH:       sProperty = C2U("InstallPath"); break;
            case LOCALE:            sProperty = C2U("Locale"); break;
            case PRODUCTNAME:       sProperty = C2U("Name"); break;
            case PRODUCTVERSION:    sProperty = C2U("Version"); break;
            case PRODUCTEXTENSION:  sProperty = C2U("Extension"); break;
            case OFFICEINSTALL:     sProperty = C2U("OfficeInstall"); break;
            case USERINSTALLURL:    sProperty = C2U("InstallURL"); break;
            case OFFICEINSTALLURL:  sProperty = C2U("OfficeInstallURL"); break;
        }
        try
        {
            aRet = xHierarchyAccess->getByHierarchicalName(sProperty);
        }
        catch(Exception&)
        {
        }
    }

    if ( eProp == PRODUCTNAME )
        aRet >>= aBrandName;

    if ( eProp == PRODUCTVERSION )
        aRet >>= aProductVersion;

    if ( eProp == PRODUCTEXTENSION )
        aRet >>= aProductExtension;

    return aRet;
}
/* -----------------------------12.12.00 17:22--------------------------------

 ---------------------------------------------------------------------------*/
Reference< XHierarchicalNameAccess> ConfigManager::GetHierarchyAccess(const OUString& rFullPath)
{
    Sequence< Any > aArgs(1);
    aArgs[0] <<= rFullPath;
    Reference< XMultiServiceFactory > xCfgProvider = GetLocalConfigurationProvider();
    Reference< XInterface > xIFace;
    if(xCfgProvider.is())
    {
        try
        {
            xIFace = xCfgProvider->createInstanceWithArguments(
                    C2U(cAccessSrvc),
                    aArgs);
        }
#ifdef DBG_UTIL
        catch(Exception& rEx)
        {
            OString sMsg("CreateInstance exception: ");
            sMsg += OString(rEx.Message.getStr(),
                        rEx.Message.getLength(),
                         RTL_TEXTENCODING_ASCII_US);
            OSL_ENSURE(sal_False, sMsg.getStr());
        }
#else
        catch(Exception&){}
#endif
    }
    return Reference<XHierarchicalNameAccess>(xIFace, UNO_QUERY);
}
/* -----------------------------12.12.00 17:17--------------------------------

 ---------------------------------------------------------------------------*/
Any ConfigManager::GetLocalProperty(const OUString& rProperty)
{
    OUString sPath = C2U(cConfigBaseURL);
    sPath += rProperty;

    sal_Int32 nLastIndex = sPath.lastIndexOf( '/',  sPath.getLength());
    OUString sNode =    sPath.copy( 0, nLastIndex );
    OUString sProperty =    sPath.copy( nLastIndex + 1, sPath.getLength() - nLastIndex - 1 );

    Reference< XHierarchicalNameAccess> xAccess = GetHierarchyAccess(sNode);
    Any aRet;
    try
    {
        if(xAccess.is())
            aRet = xAccess->getByHierarchicalName(sProperty);
    }
#ifdef DBG_UTIL
    catch(Exception& rEx)
    {
        OString sMsg("GetLocalProperty: ");
        sMsg += OString(rEx.Message.getStr(),
                    rEx.Message.getLength(),
                     RTL_TEXTENCODING_ASCII_US);
        OSL_ENSURE(sal_False, sMsg.getStr());
    }
#else
    catch(Exception&){}
#endif
    return aRet;
}
/* -----------------------------12.12.00 17:17--------------------------------

 ---------------------------------------------------------------------------*/
void ConfigManager::PutLocalProperty(const OUString& rProperty, const Any& rValue)
{
    OUString sPath = C2U(cConfigBaseURL);
    sPath += rProperty;

    sal_Int32 nLastIndex = sPath.lastIndexOf( '/',  sPath.getLength());
    OUString sNode =    sPath.copy( 0, nLastIndex );
    OUString sProperty =    sPath.copy( nLastIndex + 1, sPath.getLength() - nLastIndex - 1 );

    Reference< XHierarchicalNameAccess> xHierarchyAccess = GetHierarchyAccess(sNode);

    Any aNode = xHierarchyAccess->getByHierarchicalName(sNode);
    Reference<XNameAccess> xNodeAcc;
    aNode >>= xNodeAcc;
    Reference<XNameReplace> xNodeReplace(xNodeAcc, UNO_QUERY);
    if(xNodeReplace.is())
    {
        try
        {
            xNodeReplace->replaceByName(sProperty, rValue);
        }
#ifdef DBG_UTIL
        catch(Exception& rEx)
        {
            OString sMsg("PutLocalProperty: ");
            sMsg += OString(rEx.Message.getStr(),
                        rEx.Message.getLength(),
                         RTL_TEXTENCODING_ASCII_US);
            OSL_ENSURE(sal_False, sMsg.getStr());
        }
#else
        catch(Exception& ){}
#endif
    }
}
/* -----------------------------13.12.00 08:47--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool    ConfigManager::IsLocalConfigProvider()
{
    return pMgrImpl->bIsLocalProvider;
}

