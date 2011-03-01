/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_unotools.hxx"
#include "unotools/configmgr.hxx"
#include "unotools/configitem.hxx"
#include "unotools/configpathes.hxx"
#include <unotools/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <osl/diagnose.h>
#include <i18npool/mslangid.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/instance.hxx>
#if OSL_DEBUG_LEVEL > 0
#include <rtl/strbuf.hxx>
#endif

#include <list>

//-----------------------------------------------------------------------------

using namespace utl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;

using ::rtl::OUString;
#if OSL_DEBUG_LEVEL > 0
using ::rtl::OString;
#endif

#define UNISTRING(s) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(s))

//-----------------------------------------------------------------------------
const char* pConfigBaseURL = "/org.openoffice.";
const char* pAccessSrvc = "com.sun.star.configuration.ConfigurationUpdateAccess";

namespace
{
    struct BrandName
        : public rtl::Static< ::rtl::OUString, BrandName > {};
    struct ProductVersion
        : public rtl::Static< ::rtl::OUString, ProductVersion > {};
    struct AboutBoxProductVersion
        : public rtl::Static< ::rtl::OUString, AboutBoxProductVersion > {};
    struct OOOVendor
        : public rtl::Static< ::rtl::OUString, OOOVendor > {};
    struct ProductExtension
        : public rtl::Static< ::rtl::OUString, ProductExtension > {};
    struct XMLFileFormatName
        : public rtl::Static< ::rtl::OUString, XMLFileFormatName > {};
    struct XMLFileFormatVersion
        : public rtl::Static< ::rtl::OUString, XMLFileFormatVersion > {};
    struct WriterCompatibilityVersionOOo11
        : public rtl::Static< ::rtl::OUString, WriterCompatibilityVersionOOo11 > {};
    struct OpenSourceContext : public rtl::StaticWithInit< sal_Int32, OpenSourceContext >
    {
        sal_Int32 operator() () { return sal_Int32( -1 ); }
    };

}

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
    ConfigItemList                          aItemList;
};

ConfigManager::ConfigManager() :
    pMgrImpl(new utl::ConfigMgr_Impl)
{
    GetConfigurationProvider(); // attempt to create the provider early
}

ConfigManager::ConfigManager(Reference< XMultiServiceFactory > xConfigProv) :
    xConfigurationProvider(xConfigProv),
    pMgrImpl(new utl::ConfigMgr_Impl)
{
}

ConfigManager::~ConfigManager()
{
    //check list content -> should be empty!
#if OSL_DEBUG_LEVEL > 1
    // I think this is a pointless assertion, the code seems to cope
    // fine with it being empty, no need to crash in a dbglevel=1
    // build.
    OSL_ENSURE(pMgrImpl->aItemList.empty(), "some ConfigItems are still alive");
#endif
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

Reference< XMultiServiceFactory > ConfigManager::GetConfigurationProvider()
{
    if(!xConfigurationProvider.is())
    {
        Reference< XMultiServiceFactory > xMSF = ::utl::getProcessServiceFactory();
        if ( xMSF.is() )
        {
            try
            {
                xConfigurationProvider = Reference< XMultiServiceFactory >
                    (xMSF->createInstance(
                        UNISTRING("com.sun.star.configuration.ConfigurationProvider")),
                     UNO_QUERY);
            }
#ifdef DBG_UTIL
    catch(Exception& rEx)
    {
        static sal_Bool bMessage = sal_True;
        if(bMessage)
        {
            bMessage = sal_False;
            OString sMsg("CreateInstance with arguments exception: ");
            sMsg += OString(rEx.Message.getStr(),
                        rEx.Message.getLength(),
                        RTL_TEXTENCODING_ASCII_US);
            OSL_ENSURE(sal_False, sMsg.getStr());
        }
    }
#else
    catch(Exception&){}
#endif
        }
    }
    return xConfigurationProvider;
}

namespace
{
    // helper to achieve exception - safe registration of a ConfigItem under construction
    class RegisterConfigItemHelper // : Noncopyable
    {
        utl::ConfigManager & rCfgMgr;
        utl::ConfigItem* pCfgItem;
    public:
        RegisterConfigItemHelper(utl::ConfigManager & rMgr, utl::ConfigItem& rCfgItem)
        : rCfgMgr(rMgr)
        , pCfgItem(&rCfgItem)
        {
            rCfgMgr.RegisterConfigItem(rCfgItem);
        }

        ~RegisterConfigItemHelper()
        {
            if (pCfgItem) rCfgMgr.RemoveConfigItem(*pCfgItem);
        }

        void keep() { pCfgItem = 0; }
    };
}

Reference< XMultiServiceFactory > ConfigManager::GetLocalConfigurationProvider()
{
    return GetConfigurationProvider();
}

Reference< XHierarchicalNameAccess > ConfigManager::AddConfigItem(utl::ConfigItem& rCfgItem)
{
    RegisterConfigItemHelper registeredItem(*this,rCfgItem);
    Reference< XHierarchicalNameAccess > xTree = AcquireTree(rCfgItem);
    registeredItem.keep();
    return xTree;
}

void    ConfigManager::RegisterConfigItem(utl::ConfigItem& rCfgItem)
{
    ConfigItemList::iterator aListIter = pMgrImpl->aItemList.begin();
#ifdef DBG_UTIL
    for(aListIter = pMgrImpl->aItemList.begin(); aListIter != pMgrImpl->aItemList.end(); ++aListIter)
    {
        ConfigItemListEntry_Impl& rEntry = *aListIter;
        if(rEntry.pConfigItem == &rCfgItem)
            OSL_ENSURE(sal_False, "RegisterConfigItem: already inserted!");
    }
#endif
    pMgrImpl->aItemList.insert(aListIter, ConfigItemListEntry_Impl(&rCfgItem));
}

Reference< XHierarchicalNameAccess> ConfigManager::AcquireTree(utl::ConfigItem& rCfgItem)
{
#ifdef DBG_UTIL
    sal_Bool bFound = sal_False;
    ConfigItemList::iterator aListIter = pMgrImpl->aItemList.begin();
    for(aListIter = pMgrImpl->aItemList.begin(); aListIter != pMgrImpl->aItemList.end(); ++aListIter)
    {
        ConfigItemListEntry_Impl& rEntry = *aListIter;
        if(rEntry.pConfigItem == &rCfgItem)
        {
            bFound = sal_True;
            break;
        }
    }
    OSL_ENSURE(bFound, "AcquireTree: ConfigItem unknown!");
#endif
    OUString sPath(OUString::createFromAscii(pConfigBaseURL));
    sPath += rCfgItem.GetSubTreeName();
    Sequence< Any > aArgs(2);
    Any* pArgs = aArgs.getArray();
    PropertyValue aPath;
    aPath.Name = UNISTRING("nodepath");
    aPath.Value <<= sPath;
    pArgs[0] <<= aPath;
    sal_Bool bLazy = 0 != (rCfgItem.GetMode()&CONFIG_MODE_DELAYED_UPDATE);
    PropertyValue aUpdate;
    aUpdate.Name = UNISTRING("lazywrite");
    aUpdate.Value.setValue(&bLazy, ::getBooleanCppuType());
    pArgs[1] <<= aUpdate;

    // Initialize item with support for reading/writing more then one locales at same time!
    // It's neccessary for creation of a complete configuration entry without changing office locale
    // at runtime.
    if((rCfgItem.GetMode() & CONFIG_MODE_ALL_LOCALES) == CONFIG_MODE_ALL_LOCALES)
    {
        sal_Int32 nCount = aArgs.getLength();
        aArgs.realloc(nCount+1);

        PropertyValue aAllLocale;
        aAllLocale.Name  =   UNISTRING("locale");
        aAllLocale.Value <<= UNISTRING("*"     );
        aArgs[nCount]    <<= aAllLocale;
    }

    Reference< XMultiServiceFactory > xCfgProvider = GetConfigurationProvider();
    Reference< XInterface > xIFace;
    if(xCfgProvider.is())
    {
        try
        {
            xIFace = xCfgProvider->createInstanceWithArguments(
                    OUString::createFromAscii(pAccessSrvc),
                    aArgs);
        }
        catch(Exception& rEx)
        {
            if (CONFIG_MODE_PROPAGATE_ERRORS & rCfgItem.GetMode())
            {
                OSL_TRACE("ConfigItem: Propagating creation error: %s\n",
                            OUStringToOString(rEx.Message,RTL_TEXTENCODING_ASCII_US).getStr());

                throw;
            }
#ifdef DBG_UTIL
            if(0 == (CONFIG_MODE_IGNORE_ERRORS & rCfgItem.GetMode()))
            {
                OString sMsg("CreateInstance exception: ");
                sMsg += OString(rEx.Message.getStr(),
                            rEx.Message.getLength(),
                            RTL_TEXTENCODING_ASCII_US);
                OSL_ENSURE(sal_False, sMsg.getStr());
            }
#endif
        }
    }
    return Reference<XHierarchicalNameAccess>(xIFace, UNO_QUERY);
}

void ConfigManager::RemoveConfigItem(utl::ConfigItem& rCfgItem)
{
    if( !pMgrImpl->aItemList.empty() )
    {
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
}

void ConfigManager::StoreConfigItems()
{
    if(!pMgrImpl->aItemList.empty())
    {
        ConfigItemList::iterator aListIter = pMgrImpl->aItemList.begin();
        for(aListIter = pMgrImpl->aItemList.begin(); aListIter != pMgrImpl->aItemList.end(); ++aListIter)
        {
            ConfigItemListEntry_Impl& rEntry = *aListIter;
            if(rEntry.pConfigItem->IsModified())
            {
                rEntry.pConfigItem->Commit();
                rEntry.pConfigItem->ClearModified();
            }
        }
    }
}

struct theConfigManager : public rtl::Static<ConfigManager, theConfigManager> {};

ConfigManager& ConfigManager::GetConfigManager()
{
    return theConfigManager::get();
}

rtl::OUString ConfigManager::GetConfigBaseURL()
{
    return OUString::createFromAscii(pConfigBaseURL);
}

Any ConfigManager::GetDirectConfigProperty(ConfigProperty eProp)
{
    switch(eProp)
    {
        case INSTALLPATH:
            OSL_ENSURE( false,
                        "ConfigManager::GetDirectConfigProperty: "
                        "INSTALLPATH no longer supported." );
            return Any();
        case USERINSTALLURL:
            OSL_ENSURE( false,
                        "ConfigManager::GetDirectConfigProperty: "
                        "USERINSTALLURL no longer supported." );
            return Any();
        case OFFICEINSTALL:
            OSL_ENSURE( false,
                        "ConfigManager::GetDirectConfigProperty: "
                        "OFFICEINSTALL no longer supported." );
            return Any();
        case OFFICEINSTALLURL:
            OSL_ENSURE( false,
                        "ConfigManager::GetDirectConfigProperty: "
                        "OFFICEINSTALLURL no longer supported." );
            return Any();
        default:
            break;
    }

    Any aRet;

    ::rtl::OUString sBrandName;
#ifdef ENABLE_BROFFICE
    LanguageType nType = MsLangId::getSystemUILanguage();
    if ( nType == LANGUAGE_PORTUGUESE_BRAZILIAN )
        sBrandName = OUString(RTL_CONSTASCII_USTRINGPARAM("BrOffice"));
    else
#endif
        sBrandName = BrandName::get();

    if ( eProp == PRODUCTNAME && sBrandName.getLength() )
    {
        aRet <<= sBrandName;
        return aRet;
    }

    rtl::OUString &rProductVersion = ProductVersion::get();
    if ( eProp == PRODUCTVERSION && rProductVersion.getLength() )
    {
        aRet <<= rProductVersion;
        return aRet;
    }

    rtl::OUString &rAboutBoxProductVersion = AboutBoxProductVersion::get();
    if ( eProp == ABOUTBOXPRODUCTVERSION && rAboutBoxProductVersion.getLength() )
    {
        aRet <<= rAboutBoxProductVersion;
        return aRet;
    }

    rtl::OUString &rOOOVendor = OOOVendor::get();
    if ( eProp == OOOVENDOR && rOOOVendor.getLength() )
    {
        aRet <<= rOOOVendor;
        return aRet;
    }


    rtl::OUString &rProductExtension = ProductExtension::get();
    if ( eProp == PRODUCTEXTENSION && rProductExtension.getLength() )
    {
        aRet <<= rProductExtension;
        return aRet;
    }

    rtl::OUString &rXMLFileFormatName = XMLFileFormatName::get();
    if ( eProp == PRODUCTXMLFILEFORMATNAME && rXMLFileFormatName.getLength() )
    {
        aRet <<= rXMLFileFormatName;
        return aRet;
    }

    rtl::OUString &rXMLFileFormatVersion = XMLFileFormatVersion::get();
    if ( eProp == PRODUCTXMLFILEFORMATVERSION && rXMLFileFormatVersion.getLength() )
    {
        aRet <<= rXMLFileFormatVersion;
        return aRet;
    }

    sal_Int32 &rOpenSourceContext = OpenSourceContext::get();
    if ( eProp == OPENSOURCECONTEXT && ( rOpenSourceContext >= 0 ) )
    {
        aRet <<= rOpenSourceContext;
        return aRet;
    }

    rtl::OUString &rWriterCompatibilityVersionOOo11 = WriterCompatibilityVersionOOo11::get();
    if ( eProp == WRITERCOMPATIBILITYVERSIONOOO11 && rWriterCompatibilityVersionOOo11.getLength() )
    {
        aRet <<= rWriterCompatibilityVersionOOo11;
        return aRet;
    }

    if (eProp == PRODUCTEXTENSION) {
        rtl::OUString name(
                RTL_CONSTASCII_USTRINGPARAM(
                    "${BRAND_BASE_DIR}/program/edition/edition.ini"));
        rtl::Bootstrap::expandMacros(name);
        if (rtl::Bootstrap(name).getFrom(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("EDITIONNAME")),
                rProductExtension))
        {
            return com::sun::star::uno::Any(rProductExtension);
        }
    }

    OUString sPath = OUString::createFromAscii(pConfigBaseURL);
    switch(eProp)
    {
        case LOCALE:                        sPath += UNISTRING("Setup/L10N"); break;

        case PRODUCTNAME:
        case PRODUCTVERSION:
        case PRODUCTEXTENSION:
        case PRODUCTXMLFILEFORMATNAME :
        case PRODUCTXMLFILEFORMATVERSION:
        case OPENSOURCECONTEXT:
        case OOOVENDOR:
        case ABOUTBOXPRODUCTVERSION:        sPath += UNISTRING("Setup/Product"); break;

        case DEFAULTCURRENCY:               sPath += UNISTRING("Setup/L10N"); break;

        case WRITERCOMPATIBILITYVERSIONOOO11:
            sPath += UNISTRING("Office.Compatibility/WriterCompatibilityVersion"); break;
        default:
            break;
    }
    Sequence< Any > aArgs(1);
    aArgs[0] <<= sPath;
    Reference< XMultiServiceFactory > xCfgProvider = GetConfigManager().GetConfigurationProvider();
    if(!xCfgProvider.is())
        return aRet;
    Reference< XInterface > xIFace;
    try
    {
        xIFace = xCfgProvider->createInstanceWithArguments(
                OUString::createFromAscii(pAccessSrvc),
                aArgs);

    }
    catch(Exception&){}
    Reference<XNameAccess> xDirectAccess(xIFace, UNO_QUERY);
    if(xDirectAccess.is())
    {
        OUString sProperty;
        switch(eProp)
        {
            case LOCALE:                            sProperty = UNISTRING("ooLocale"); break;
            case PRODUCTNAME:                       sProperty = UNISTRING("ooName"); break;
            case PRODUCTVERSION:                    sProperty = UNISTRING("ooSetupVersion"); break;
            case ABOUTBOXPRODUCTVERSION:            sProperty = UNISTRING("ooSetupVersionAboutBox"); break;
            case OOOVENDOR:                         sProperty = UNISTRING("ooVendor"); break;
            case PRODUCTEXTENSION:                  sProperty = UNISTRING("ooSetupExtension"); break;
            case PRODUCTXMLFILEFORMATNAME:          sProperty = UNISTRING("ooXMLFileFormatName"); break;
            case PRODUCTXMLFILEFORMATVERSION:       sProperty = UNISTRING("ooXMLFileFormatVersion"); break;
            case OPENSOURCECONTEXT:                 sProperty = UNISTRING("ooOpenSourceContext"); break;
            case DEFAULTCURRENCY:                   sProperty = UNISTRING("ooSetupCurrency"); break;
            case WRITERCOMPATIBILITYVERSIONOOO11:   sProperty = UNISTRING("OOo11"); break;
            default:
                break;
        }
        try
        {
            aRet = xDirectAccess->getByName(sProperty);
        }
        catch(Exception&)
        {
            #if OSL_DEBUG_LEVEL > 0
            rtl::OStringBuffer aBuf(256);
            aBuf.append( "ConfigManager::GetDirectConfigProperty: could not retrieve the property \"" );
            aBuf.append( rtl::OUStringToOString( sProperty, RTL_TEXTENCODING_ASCII_US ) );
            aBuf.append( "\" under \"" );
            aBuf.append( rtl::OUStringToOString( sPath, RTL_TEXTENCODING_ASCII_US ) );
            aBuf.append( "\" (caught an exception)!" );
            OSL_ENSURE( sal_False, aBuf.getStr() );
            #endif
        }
    }

    if ( eProp == PRODUCTNAME )
        aRet >>= sBrandName;

    if ( eProp == PRODUCTXMLFILEFORMATNAME )
        aRet >>= rXMLFileFormatName;

    if ( eProp == PRODUCTXMLFILEFORMATVERSION )
        aRet >>= rXMLFileFormatVersion;

    if ( eProp == PRODUCTVERSION )
        aRet >>= rProductVersion;

    if( eProp == OOOVENDOR )
        aRet >>= rOOOVendor;

    if ( eProp == ABOUTBOXPRODUCTVERSION )
    {
        aRet >>= rAboutBoxProductVersion;
        getBasisAboutBoxProductVersion( rAboutBoxProductVersion );
        aRet <<= rAboutBoxProductVersion;
    }

    if ( eProp == PRODUCTEXTENSION )
        aRet >>= rProductExtension;

    if ( eProp == WRITERCOMPATIBILITYVERSIONOOO11 )
        aRet >>= rWriterCompatibilityVersionOOo11;

    if ( eProp == OPENSOURCECONTEXT )
        aRet >>= rOpenSourceContext;

    return aRet;
}

/*---------------------------------------------------------------------------*/
void ConfigManager::getBasisAboutBoxProductVersion( OUString& rVersion )
{
    rtl::OUString aPackageVersion = UNISTRING( "${$OOO_BASE_DIR/program/" SAL_CONFIGFILE("version") ":OOOPackageVersion}" );
    rtl::Bootstrap::expandMacros( aPackageVersion );

    if ( aPackageVersion.getLength() )
    {
        sal_Int32 nTokIndex = 0;
        rtl::OUString aVersionMinor = aPackageVersion.getToken( 1, '.', nTokIndex );
        rtl::OUString aVersionMicro;

        if ( nTokIndex > 0 )
            aVersionMicro = aPackageVersion.getToken( 0, '.', nTokIndex );

        if ( aVersionMinor.getLength() == 0 )
            aVersionMinor = UNISTRING( "0" );
        if ( aVersionMicro.getLength() == 0 )
            aVersionMicro = UNISTRING( "0" );

        sal_Int32 nIndex = rVersion.indexOf( '.' );
        if ( nIndex == -1 )
        {
            rVersion += UNISTRING( "." );
            rVersion += aVersionMinor;
        }
        else
        {
            nIndex = rVersion.indexOf( '.', nIndex+1 );
        }
        if ( nIndex == -1 )
        {
            rVersion += UNISTRING( "." );
            rVersion += aVersionMicro;
        }
        else
        {
            rVersion = rVersion.replaceAt( nIndex+1, rVersion.getLength()-nIndex-1, aVersionMicro );
        }
    }
}

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
                    OUString::createFromAscii(pAccessSrvc),
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

Any ConfigManager::GetLocalProperty(const OUString& rProperty)
{
    OUString sPath(OUString::createFromAscii(pConfigBaseURL));
    sPath += rProperty;

    OUString sNode, sProperty;
    OSL_VERIFY( splitLastFromConfigurationPath(sPath, sNode, sProperty) );

    Reference< XNameAccess> xAccess( GetHierarchyAccess(sNode), UNO_QUERY );
    Any aRet;
    try
    {
        if(xAccess.is())
            aRet = xAccess->getByName(sProperty);
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

void ConfigManager::PutLocalProperty(const OUString& rProperty, const Any& rValue)
{
    OUString sPath(OUString::createFromAscii(pConfigBaseURL));
    sPath += rProperty;

    OUString sNode, sProperty;
    OSL_VERIFY( splitLastFromConfigurationPath(sPath, sNode, sProperty) );

    Reference<XNameReplace> xNodeReplace(GetHierarchyAccess(sNode), UNO_QUERY);
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

sal_Bool    ConfigManager::IsLocalConfigProvider()
{
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
