/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "unotools/configitem.hxx"
#include "unotools/configmgr.hxx"
#include "unotools/configpaths.hxx"
#include <comphelper/processfactory.hxx>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XChangesListener.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XHierarchicalName.hpp>
#include <com/sun/star/configuration/XTemplateContainer.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/awt/XRequestCallback.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/util/XStringEscape.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <osl/diagnose.h>
#include <tools/solarmutex.hxx>
#include <rtl/ustrbuf.hxx>

using namespace utl;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace com::sun::star::configuration;

#include <cppuhelper/implbase1.hxx>

#ifdef DBG_UTIL
static inline void lcl_CFG_DBG_EXCEPTION(const sal_Char* cText, const Exception& rEx)
{
    OString sMsg(cText);
    sMsg += OString(rEx.Message.getStr(), rEx.Message.getLength(), RTL_TEXTENCODING_ASCII_US);
    OSL_FAIL(sMsg.getStr());
}
#define CATCH_INFO(a) \
catch(const Exception& rEx)   \
{                       \
    lcl_CFG_DBG_EXCEPTION(a, rEx);\
}
#else
    #define CATCH_INFO(a) catch(const Exception&){}
#endif

/*
    The ConfigChangeListener_Impl receives notifications from the configuration about changes that
    have happened. It forwards this notification to the ConfigItem it knows a pParent by calling its
    "CallNotify" method. As ConfigItems are most probably not thread safe, the SolarMutex is acquired
    before doing so.
*/

namespace utl{
    class ConfigChangeListener_Impl : public cppu::WeakImplHelper1
    <
        com::sun::star::util::XChangesListener
    >
    {
        public:
            ConfigItem*                 pParent;
            const Sequence< OUString >  aPropertyNames;
            ConfigChangeListener_Impl(ConfigItem& rItem, const Sequence< OUString >& rNames);
            ~ConfigChangeListener_Impl();

        
        virtual void SAL_CALL changesOccurred( const ChangesEvent& Event ) throw(RuntimeException);

        
        virtual void SAL_CALL disposing( const EventObject& Source ) throw(RuntimeException);
    };
}

class ValueCounter_Impl
{
    sal_Int16& rCnt;
public:
    ValueCounter_Impl(sal_Int16& rCounter):
        rCnt(rCounter)
            {rCnt++;}
    ~ValueCounter_Impl()
            {
                OSL_ENSURE(rCnt>0, "RefCount < 0 ??");
                rCnt--;
            }
};

ConfigChangeListener_Impl::ConfigChangeListener_Impl(
             ConfigItem& rItem, const Sequence< OUString >& rNames) :
    pParent(&rItem),
    aPropertyNames(rNames)
{
}

ConfigChangeListener_Impl::~ConfigChangeListener_Impl()
{
}

static bool lcl_Find(
        const OUString& rTemp,
        const OUString* pCheckPropertyNames,
        sal_Int32 nLength)
{
    
    
    for(sal_Int32 nIndex = 0; nIndex < nLength; nIndex++)
        if( isPrefixOfConfigurationPath(rTemp, pCheckPropertyNames[nIndex]) )
            return true;
    return false;
}

void ConfigChangeListener_Impl::changesOccurred( const ChangesEvent& rEvent ) throw(RuntimeException)
{
    const ElementChange* pElementChanges = rEvent.Changes.getConstArray();

    Sequence<OUString>  aChangedNames(rEvent.Changes.getLength());
    OUString* pNames = aChangedNames.getArray();

    const OUString* pCheckPropertyNames = aPropertyNames.getConstArray();

    sal_Int32 nNotify = 0;
    for(int i = 0; i < aChangedNames.getLength(); i++)
    {
        OUString sTemp;
        pElementChanges[i].Accessor >>= sTemp;
        if(lcl_Find(sTemp, pCheckPropertyNames, aPropertyNames.getLength()))
            pNames[nNotify++] = sTemp;
    }
    if( nNotify )
    {
        if ( ::tools::SolarMutex::Acquire() )
        {
            aChangedNames.realloc(nNotify);
            pParent->CallNotify(aChangedNames);
            ::tools::SolarMutex::Release();
        }
    }
}


void ConfigChangeListener_Impl::disposing( const EventObject& /*rSource*/ ) throw(RuntimeException)
{
    pParent->RemoveChangesListener();
}

ConfigItem::ConfigItem(const OUString &rSubTree, sal_Int16 nSetMode ) :
    sSubTree(rSubTree),
    m_nMode(nSetMode),
    m_bIsModified(false),
    m_bEnableInternalNotification(false),
    m_nInValueChange(0)
{
    if(0 != (nSetMode&CONFIG_MODE_RELEASE_TREE))
        ConfigManager::getConfigManager().addConfigItem(*this);
    else
        m_xHierarchyAccess = ConfigManager::getConfigManager().addConfigItem(*this);
}

ConfigItem::~ConfigItem()
{
    RemoveChangesListener();
    ConfigManager::getConfigManager().removeConfigItem(*this);
}

void ConfigItem::CallNotify( const com::sun::star::uno::Sequence<OUString>& rPropertyNames )
{
    
    
    
    if(!IsInValueChange() || m_bEnableInternalNotification)
        Notify(rPropertyNames);
}

void ConfigItem::impl_packLocalizedProperties(  const   Sequence< OUString >&   lInNames    ,
                                                const   Sequence< Any >&        lInValues   ,
                                                        Sequence< Any >&        lOutValues  )
{
    
    
    OSL_ENSURE( ((m_nMode & CONFIG_MODE_ALL_LOCALES ) == CONFIG_MODE_ALL_LOCALES), "ConfigItem::impl_packLocalizedProperties()\nWrong call of this method detected!\n" );

    sal_Int32                   nSourceCounter      ;   
    sal_Int32                   nSourceSize         ;   
    sal_Int32                   nDestinationCounter ;   
    sal_Int32                   nPropertyCounter    ;   
    sal_Int32                   nPropertiesSize     ;   
    Sequence< OUString >        lPropertyNames      ;   
    Sequence< PropertyValue >   lProperties         ;   
    Reference< XInterface >     xLocalizedNode      ;   

    
    
    
    
    
    
    
    nSourceSize = lInNames.getLength();
    lOutValues.realloc( nSourceSize );

    
    
    
    
    

    nDestinationCounter = 0;
    for( nSourceCounter=0; nSourceCounter<nSourceSize; ++nSourceCounter )
    {
        
        if( lInValues[nSourceCounter].getValueTypeName() == "com.sun.star.uno.XInterface" )
        {
            lInValues[nSourceCounter] >>= xLocalizedNode;
            Reference< XNameContainer > xSetAccess( xLocalizedNode, UNO_QUERY );
            if( xSetAccess.is() )
            {
                lPropertyNames  =   xSetAccess->getElementNames()   ;
                nPropertiesSize =   lPropertyNames.getLength()      ;
                lProperties.realloc( nPropertiesSize )              ;

                for( nPropertyCounter=0; nPropertyCounter<nPropertiesSize; ++nPropertyCounter )
                {
                    #if OSL_DEBUG_LEVEL > 1
                    
                    OUString sPropName   = lInNames[nSourceCounter];
                    OUString sLocaleName = lPropertyNames[nPropertyCounter];
                    #endif
                    lProperties[nPropertyCounter].Name  =   lPropertyNames[nPropertyCounter]                            ;
                    OUString sLocaleValue;
                    xSetAccess->getByName( lPropertyNames[nPropertyCounter] ) >>= sLocaleValue  ;
                    lProperties[nPropertyCounter].Value <<= sLocaleValue;
                }

                lOutValues[nDestinationCounter] <<= lProperties;
            }
        }
        
        else
        {
            lOutValues[nDestinationCounter] = lInValues[nSourceCounter];
        }
        ++nDestinationCounter;
    }
}

void ConfigItem::impl_unpackLocalizedProperties(    const   Sequence< OUString >&   lInNames    ,
                                                    const   Sequence< Any >&        lInValues   ,
                                                            Sequence< OUString >&   lOutNames   ,
                                                            Sequence< Any >&        lOutValues  )
{
    
    
    OSL_ENSURE( ((m_nMode & CONFIG_MODE_ALL_LOCALES ) == CONFIG_MODE_ALL_LOCALES), "ConfigItem::impl_unpackLocalizedProperties()\nWrong call of this method detected!\n" );

    sal_Int32                   nSourceCounter      ;   
    sal_Int32                   nSourceSize         ;   
    sal_Int32                   nDestinationCounter ;   
    sal_Int32                   nPropertyCounter    ;   
    sal_Int32                   nPropertiesSize     ;   
    OUString                    sNodeName           ;   
    Sequence< PropertyValue >   lProperties         ;   

    
    
    
    
    
    
    
    
    nSourceSize = lInNames.getLength();

    lOutNames.realloc   ( nSourceSize );
    lOutValues.realloc  ( nSourceSize );

    
    
    
    

    nDestinationCounter = 0;
    for( nSourceCounter=0; nSourceCounter<nSourceSize; ++nSourceCounter )
    {
        
        if( lInValues[nSourceCounter].getValueType() == ::getCppuType( (const Sequence< PropertyValue >*)NULL ) )
        {
            lInValues[nSourceCounter] >>= lProperties;
            nPropertiesSize = lProperties.getLength();

            sNodeName = lInNames[nSourceCounter] + "/";

            if( (nDestinationCounter+nPropertiesSize) > lOutNames.getLength() )
            {
                lOutNames.realloc   ( nDestinationCounter+nPropertiesSize );
                lOutValues.realloc  ( nDestinationCounter+nPropertiesSize );
            }

            for( nPropertyCounter=0; nPropertyCounter<nPropertiesSize; ++nPropertyCounter )
            {
                 lOutNames [nDestinationCounter] = sNodeName + lProperties[nPropertyCounter].Name   ;
                lOutValues[nDestinationCounter] = lProperties[nPropertyCounter].Value               ;
                ++nDestinationCounter;
            }
        }
        
        else
        {
            if( (nDestinationCounter+1) > lOutNames.getLength() )
            {
                lOutNames.realloc   ( nDestinationCounter+1 );
                lOutValues.realloc  ( nDestinationCounter+1 );
            }

            lOutNames [nDestinationCounter] = lInNames [nSourceCounter];
            lOutValues[nDestinationCounter] = lInValues[nSourceCounter];
            ++nDestinationCounter;
        }
    }
}

Sequence< sal_Bool > ConfigItem::GetReadOnlyStates(const com::sun::star::uno::Sequence< OUString >& rNames)
{
    sal_Int32 i;

    
    
    sal_Int32 nCount = rNames.getLength();
    Sequence< sal_Bool > lStates(nCount);

    
    
    for ( i=0; i<nCount; ++i)
        lStates[i] = sal_False;

    
    Reference< XHierarchicalNameAccess > xHierarchyAccess = GetTree();
    if (!xHierarchyAccess.is())
        return lStates;

    for (i=0; i<nCount; ++i)
    {
        try
        {
            OUString sName = rNames[i];
            OUString sPath;
            OUString sProperty;

            ::utl::splitLastFromConfigurationPath(sName,sPath,sProperty);
            if (sPath.isEmpty() && sProperty.isEmpty())
            {
                OSL_FAIL("ConfigItem::IsReadonly()\nsplitt failed\n");
                continue;
            }

            Reference< XInterface >       xNode;
            Reference< XPropertySet >     xSet ;
            Reference< XPropertySetInfo > xInfo;
            if (!sPath.isEmpty())
            {
                Any aNode = xHierarchyAccess->getByHierarchicalName(sPath);
                if (!(aNode >>= xNode) || !xNode.is())
                {
                    OSL_FAIL("ConfigItem::IsReadonly()\nno set available\n");
                    continue;
                }
            }
            else
            {
                xNode = xHierarchyAccess;
            }

        xSet = Reference< XPropertySet >(xNode, UNO_QUERY);
            if (xSet.is())
        {
            xInfo = xSet->getPropertySetInfo();
                OSL_ENSURE(xInfo.is(), "ConfigItem::IsReadonly()\ngetPropertySetInfo failed ...\n");
        }
            else
        {
               xInfo = Reference< XPropertySetInfo >(xNode, UNO_QUERY);
                OSL_ENSURE(xInfo.is(), "ConfigItem::IsReadonly()\nUNO_QUERY failed ...\n");
        }

            if (!xInfo.is())
            {
                OSL_FAIL("ConfigItem::IsReadonly()\nno prop info available\n");
                continue;
            }

            Property aProp = xInfo->getPropertyByName(sProperty);
            lStates[i] = ((aProp.Attributes & PropertyAttribute::READONLY) == PropertyAttribute::READONLY);
        }
        catch (const Exception&)
        {
        }
    }

    return lStates;
}

Sequence< Any > ConfigItem::GetProperties(const Sequence< OUString >& rNames)
{
    Sequence< Any > aRet(rNames.getLength());
    const OUString* pNames = rNames.getConstArray();
    Any* pRet = aRet.getArray();
    Reference<XHierarchicalNameAccess> xHierarchyAccess = GetTree();
    if(xHierarchyAccess.is())
    {
        for(int i = 0; i < rNames.getLength(); i++)
        {
            try
            {
                pRet[i] = xHierarchyAccess->getByHierarchicalName(pNames[i]);
            }
            catch (const Exception& rEx)
            {
                SAL_WARN(
                    "unotools.config",
                    "ignoring XHierarchicalNameAccess to /org.openoffice."
                        << sSubTree << "/" << pNames[i] << " Exception: "
                        << rEx.Message);
            }
        }

        
        if((m_nMode & CONFIG_MODE_ALL_LOCALES ) == CONFIG_MODE_ALL_LOCALES)
        {
            Sequence< Any > lValues;
            impl_packLocalizedProperties( rNames, aRet, lValues );
            aRet = lValues;
        }
    }
    return aRet;
}

bool ConfigItem::PutProperties( const Sequence< OUString >& rNames,
                                                const Sequence< Any>& rValues)
{
    ValueCounter_Impl aCounter(m_nInValueChange);
    Reference<XHierarchicalNameAccess> xHierarchyAccess = GetTree();
    Reference<XNameReplace> xTopNodeReplace(xHierarchyAccess, UNO_QUERY);
    bool bRet = xHierarchyAccess.is() && xTopNodeReplace.is();
    if(bRet)
    {
        Sequence< OUString >    lNames          ;
        Sequence< Any >         lValues         ;
        const OUString*         pNames  = NULL  ;
        const Any*              pValues = NULL  ;
        sal_Int32               nNameCount      ;
        if(( m_nMode & CONFIG_MODE_ALL_LOCALES ) == CONFIG_MODE_ALL_LOCALES )
        {
            
            
            
            
            impl_unpackLocalizedProperties( rNames, rValues, lNames, lValues );
            pNames      = lNames.getConstArray  ();
            pValues     = lValues.getConstArray ();
            nNameCount  = lNames.getLength      ();
        }
        else
        {
            
            
            pNames      = rNames.getConstArray  ();
            pValues     = rValues.getConstArray ();
            nNameCount  = rNames.getLength      ();
        }
        for(int i = 0; i < nNameCount; i++)
        {
            try
            {
                OUString sNode, sProperty;
                if (splitLastFromConfigurationPath(pNames[i],sNode, sProperty))
                {
                    Any aNode = xHierarchyAccess->getByHierarchicalName(sNode);

                    Reference<XNameAccess> xNodeAcc;
                    aNode >>= xNodeAcc;
                    Reference<XNameReplace>   xNodeReplace(xNodeAcc, UNO_QUERY);
                    Reference<XNameContainer> xNodeCont   (xNodeAcc, UNO_QUERY);

                    bool bExist = (xNodeAcc.is() && xNodeAcc->hasByName(sProperty));
                    if (bExist && xNodeReplace.is())
                        xNodeReplace->replaceByName(sProperty, pValues[i]);
                    else
                        if (!bExist && xNodeCont.is())
                            xNodeCont->insertByName(sProperty, pValues[i]);
                        else
                            bRet = false;
                }
                else 
                {
                    xTopNodeReplace->replaceByName(sProperty, pValues[i]);
                }
            }
            CATCH_INFO("Exception from PutProperties: ");
        }
        try
        {
            Reference<XChangesBatch> xBatch(xHierarchyAccess, UNO_QUERY);
            xBatch->commitChanges();
        }
        CATCH_INFO("Exception from commitChanges(): ")
    }

    return bRet;
}

void ConfigItem::DisableNotification()
{
    OSL_ENSURE( xChangeLstnr.is(), "ConfigItem::DisableNotification: notifications not enabled currently!" );
    RemoveChangesListener();
}

bool    ConfigItem::EnableNotification(const Sequence< OUString >& rNames,
                bool bEnableInternalNotification )

{
    OSL_ENSURE(0 == (m_nMode&CONFIG_MODE_RELEASE_TREE), "notification in CONFIG_MODE_RELEASE_TREE mode not possible");
    m_bEnableInternalNotification = bEnableInternalNotification;
    Reference<XHierarchicalNameAccess> xHierarchyAccess = GetTree();
    Reference<XChangesNotifier> xChgNot(xHierarchyAccess, UNO_QUERY);
    if(!xChgNot.is())
        return false;

    OSL_ENSURE(!xChangeLstnr.is(), "EnableNotification already called");
    if(xChangeLstnr.is())
        xChgNot->removeChangesListener( xChangeLstnr );
    bool bRet = true;

    try
    {
        xChangeLstnr = new ConfigChangeListener_Impl(*this, rNames);
        xChgNot->addChangesListener( xChangeLstnr );
    }
    catch (const RuntimeException&)
    {
        bRet = false;
    }
    return bRet;
}

void ConfigItem::RemoveChangesListener()
{
    Reference<XChangesNotifier> xChgNot(m_xHierarchyAccess, UNO_QUERY);
    if(xChgNot.is() && xChangeLstnr.is())
    {
        try
        {
            xChgNot->removeChangesListener( xChangeLstnr );
            xChangeLstnr = 0;
        }
        catch (const Exception&)
        {
        }
    }
}

static void lcl_normalizeLocalNames(Sequence< OUString >& _rNames, ConfigNameFormat _eFormat, Reference<XInterface> const& _xParentNode)
{
    switch (_eFormat)
    {
    case CONFIG_NAME_LOCAL_NAME:
        
        break;

    case CONFIG_NAME_FULL_PATH:
        {
            Reference<XHierarchicalName> xFormatter(_xParentNode, UNO_QUERY);
            if (xFormatter.is())
            {
                OUString * pNames = _rNames.getArray();
                for(int i = 0; i<_rNames.getLength(); ++i)
                {
                    try
                    {
                        pNames[i] = xFormatter->composeHierarchicalName(pNames[i]);
                    }
                    CATCH_INFO("Exception from composeHierarchicalName(): ")
                }
                break;
            }
        }
        OSL_FAIL("Cannot create absolute paths: missing interface");
        

    case CONFIG_NAME_LOCAL_PATH:
        {
            Reference<XTemplateContainer> xTypeContainer(_xParentNode, UNO_QUERY);
            if (xTypeContainer.is())
            {
                OUString sTypeName = xTypeContainer->getElementTemplateName();
                sTypeName = sTypeName.copy(sTypeName.lastIndexOf('/')+1);

                OUString * pNames = _rNames.getArray();
                for(int i = 0; i<_rNames.getLength(); ++i)
                {
                    pNames[i] = wrapConfigurationElementName(pNames[i],sTypeName);
                }
            }
            else
            {
                Reference<XServiceInfo> xSVI(_xParentNode, UNO_QUERY);
                if (xSVI.is() && xSVI->supportsService("com.sun.star.configuration.SetAccess"))
                {
                    OUString * pNames = _rNames.getArray();
                    for(int i = 0; i<_rNames.getLength(); ++i)
                    {
                        pNames[i] = wrapConfigurationElementName(pNames[i]);
                    }
                }
            }
        }
        break;

    case CONFIG_NAME_PLAINTEXT_NAME:
        {
            Reference<XStringEscape> xEscaper(_xParentNode, UNO_QUERY);
            if (xEscaper.is())
            {
                OUString * pNames = _rNames.getArray();
                for(int i = 0; i<_rNames.getLength(); ++i)
                try
                {
                    pNames[i] = xEscaper->unescapeString(pNames[i]);
                }
                CATCH_INFO("Exception from unescapeString(): ")
            }
        }
        break;

    }
}

Sequence< OUString > ConfigItem::GetNodeNames(const OUString& rNode)
{
    ConfigNameFormat const eDefaultFormat = CONFIG_NAME_LOCAL_NAME; 

    return GetNodeNames(rNode, eDefaultFormat);
}

Sequence< OUString > ConfigItem::GetNodeNames(const OUString& rNode, ConfigNameFormat eFormat)
{
    Sequence< OUString > aRet;
    Reference<XHierarchicalNameAccess> xHierarchyAccess = GetTree();
    if(xHierarchyAccess.is())
    {
        try
        {
            Reference<XNameAccess> xCont;
            if(!rNode.isEmpty())
            {
                Any aNode = xHierarchyAccess->getByHierarchicalName(rNode);
                aNode >>= xCont;
            }
            else
                xCont = Reference<XNameAccess> (xHierarchyAccess, UNO_QUERY);
            if(xCont.is())
            {
                aRet = xCont->getElementNames();
                lcl_normalizeLocalNames(aRet,eFormat,xCont);
            }

        }
        CATCH_INFO("Exception from GetNodeNames: ");
    }
    return aRet;
}

bool ConfigItem::ClearNodeSet(const OUString& rNode)
{
    ValueCounter_Impl aCounter(m_nInValueChange);
    bool bRet = false;
    Reference<XHierarchicalNameAccess> xHierarchyAccess = GetTree();
    if(xHierarchyAccess.is())
    {
        try
        {
            Reference<XNameContainer> xCont;
            if(!rNode.isEmpty())
            {
                Any aNode = xHierarchyAccess->getByHierarchicalName(rNode);
                aNode >>= xCont;
            }
            else
                xCont = Reference<XNameContainer> (xHierarchyAccess, UNO_QUERY);
            if(!xCont.is())
                return false;
            Sequence< OUString > aNames = xCont->getElementNames();
            const OUString* pNames = aNames.getConstArray();
            Reference<XChangesBatch> xBatch(xHierarchyAccess, UNO_QUERY);
            for(sal_Int32 i = 0; i < aNames.getLength(); i++)
            {
                try
                {
                    xCont->removeByName(pNames[i]);
                }
                CATCH_INFO("Exception from removeByName(): ")
            }
            xBatch->commitChanges();
            bRet = true;
        }
        CATCH_INFO("Exception from ClearNodeSet")
    }
    return bRet;
}

bool ConfigItem::ClearNodeElements(const OUString& rNode, Sequence< OUString >& rElements)
{
    ValueCounter_Impl aCounter(m_nInValueChange);
    bool bRet = false;
    Reference<XHierarchicalNameAccess> xHierarchyAccess = GetTree();
    if(xHierarchyAccess.is())
    {
        const OUString* pElements = rElements.getConstArray();
        try
        {
            Reference<XNameContainer> xCont;
            if(!rNode.isEmpty())
            {
                Any aNode = xHierarchyAccess->getByHierarchicalName(rNode);
                aNode >>= xCont;
            }
            else
                xCont = Reference<XNameContainer> (xHierarchyAccess, UNO_QUERY);
            if(!xCont.is())
                return false;
            try
            {
                for(sal_Int32 nElement = 0; nElement < rElements.getLength(); nElement++)
                {
                    xCont->removeByName(pElements[nElement]);
                }
                Reference<XChangesBatch> xBatch(xHierarchyAccess, UNO_QUERY);
                xBatch->commitChanges();
            }
            CATCH_INFO("Exception from commitChanges(): ")
            bRet = true;
        }
        CATCH_INFO("Exception from GetNodeNames: ")
    }
    return bRet;
}

static inline
OUString lcl_extractSetPropertyName( const OUString& rInPath, const OUString& rPrefix )
{
    OUString const sSubPath = dropPrefixFromConfigurationPath( rInPath, rPrefix);
    return extractFirstFromConfigurationPath( sSubPath );
}

static
Sequence< OUString > lcl_extractSetPropertyNames( const Sequence< PropertyValue >& rValues, const OUString& rPrefix )
{
    const PropertyValue* pProperties = rValues.getConstArray();

    Sequence< OUString > aSubNodeNames(rValues.getLength());
    OUString* pSubNodeNames = aSubNodeNames.getArray();

    OUString sLastSubNode;
    sal_Int32 nSubIndex = 0;

    for(sal_Int32 i = 0; i < rValues.getLength(); i++)
    {
        OUString const sSubPath = dropPrefixFromConfigurationPath( pProperties[i].Name, rPrefix);
        OUString const sSubNode = extractFirstFromConfigurationPath( sSubPath );

        if(sLastSubNode != sSubNode)
        {
            pSubNodeNames[nSubIndex++] = sSubNode;
        }

        sLastSubNode = sSubNode;
    }
    aSubNodeNames.realloc(nSubIndex);

    return aSubNodeNames;
}


bool ConfigItem::SetSetProperties(
    const OUString& rNode, Sequence< PropertyValue > rValues)
{
    ValueCounter_Impl aCounter(m_nInValueChange);
    bool bRet = true;
    Reference<XHierarchicalNameAccess> xHierarchyAccess = GetTree();
    if(xHierarchyAccess.is())
    {
        Reference<XChangesBatch> xBatch(xHierarchyAccess, UNO_QUERY);
        try
        {
            Reference<XNameContainer> xCont;
            if(!rNode.isEmpty())
            {
                Any aNode = xHierarchyAccess->getByHierarchicalName(rNode);
                aNode >>= xCont;
            }
            else
                xCont = Reference<XNameContainer> (xHierarchyAccess, UNO_QUERY);
            if(!xCont.is())
                return false;

            Reference<XSingleServiceFactory> xFac(xCont, UNO_QUERY);

            if(xFac.is())
            {
                const Sequence< OUString > aSubNodeNames = lcl_extractSetPropertyNames(rValues, rNode);

                const sal_Int32 nSubNodeCount = aSubNodeNames.getLength();

                for(sal_Int32 j = 0; j <nSubNodeCount ; j++)
                {
                    if(!xCont->hasByName(aSubNodeNames[j]))
                    {
                        Reference<XInterface> xInst = xFac->createInstance();
                        Any aVal; aVal <<= xInst;
                        xCont->insertByName(aSubNodeNames[j], aVal);
                    }
                    
                }
                try
                {
                    xBatch->commitChanges();
                }
                CATCH_INFO("Exception from commitChanges(): ")

                const PropertyValue* pProperties = rValues.getConstArray();

                Sequence< OUString > aSetNames(rValues.getLength());
                OUString* pSetNames = aSetNames.getArray();

                Sequence< Any> aSetValues(rValues.getLength());
                Any* pSetValues = aSetValues.getArray();

                bool bEmptyNode = rNode.isEmpty();
                for(sal_Int32 k = 0; k < rValues.getLength(); k++)
                {
                    pSetNames[k] =  pProperties[k].Name.copy( bEmptyNode ? 1 : 0);
                    pSetValues[k] = pProperties[k].Value;
                }
                bRet = PutProperties(aSetNames, aSetValues);
            }
            else
            {
                
                const PropertyValue* pValues = rValues.getConstArray();
                for(int nValue = 0; nValue < rValues.getLength();nValue++)
                {
                    try
                    {
                        OUString sSubNode = lcl_extractSetPropertyName( pValues[nValue].Name, rNode );

                        if(xCont->hasByName(sSubNode))
                            xCont->replaceByName(sSubNode, pValues[nValue].Value);
                        else
                            xCont->insertByName(sSubNode, pValues[nValue].Value);

                        OSL_ENSURE( xHierarchyAccess->hasByHierarchicalName(pValues[nValue].Name),
                            "Invalid config path" );
                    }
                    CATCH_INFO("Exception form insert/replaceByName(): ")
                }
                xBatch->commitChanges();
            }
        }
#ifdef DBG_UTIL
        catch (const Exception& rEx)
        {
            lcl_CFG_DBG_EXCEPTION("Exception from SetSetProperties: ", rEx);
#else
        catch (const Exception&)
        {
#endif
            bRet = false;
        }
    }
    return bRet;
}

bool ConfigItem::ReplaceSetProperties(
    const OUString& rNode, Sequence< PropertyValue > rValues)
{
    ValueCounter_Impl aCounter(m_nInValueChange);
    bool bRet = true;
    Reference<XHierarchicalNameAccess> xHierarchyAccess = GetTree();
    if(xHierarchyAccess.is())
    {
        Reference<XChangesBatch> xBatch(xHierarchyAccess, UNO_QUERY);
        try
        {
            Reference<XNameContainer> xCont;
            if(!rNode.isEmpty())
            {
                Any aNode = xHierarchyAccess->getByHierarchicalName(rNode);
                aNode >>= xCont;
            }
            else
                xCont = Reference<XNameContainer> (xHierarchyAccess, UNO_QUERY);
            if(!xCont.is())
                return false;

            
            const Sequence< OUString > aSubNodeNames = lcl_extractSetPropertyNames(rValues, rNode);
            const OUString* pSubNodeNames = aSubNodeNames.getConstArray();
            const sal_Int32 nSubNodeCount = aSubNodeNames.getLength();

            Reference<XSingleServiceFactory> xFac(xCont, UNO_QUERY);
            const bool isSimpleValueSet = !xFac.is();

            
            {
                const Sequence<OUString> aContainerSubNodes = xCont->getElementNames();
                const OUString* pContainerSubNodes = aContainerSubNodes.getConstArray();

                for(sal_Int32 nContSub = 0; nContSub < aContainerSubNodes.getLength(); nContSub++)
                {
                    bool bFound = false;
                    for(sal_Int32 j = 0; j < nSubNodeCount; j++)
                    {
                        if(pSubNodeNames[j] == pContainerSubNodes[nContSub])
                        {
                            bFound = true;
                            break;
                        }
                    }
                    if(!bFound)
                    try
                    {
                        xCont->removeByName(pContainerSubNodes[nContSub]);
                    }
                    catch (const Exception&)
                    {
                        if (isSimpleValueSet)
                        {
                            try
                            {
                                
                                xCont->replaceByName(pContainerSubNodes[nContSub], Any());
                                
                                continue;
                            }
                            catch (Exception &)
                            {} 
                        }
                        throw;
                    }
                }
                try { xBatch->commitChanges(); }
                CATCH_INFO("Exception from commitChanges(): ")
            }

            if(xFac.is()) 
            {
                for(sal_Int32 j = 0; j < nSubNodeCount; j++)
                {
                    if(!xCont->hasByName(pSubNodeNames[j]))
                    {
                        
                        Reference<XInterface> xInst = xFac->createInstance();
                        Any aVal; aVal <<= xInst;
                        xCont->insertByName(pSubNodeNames[j], aVal);
                    }
                }
                try { xBatch->commitChanges(); }
                CATCH_INFO("Exception from commitChanges(): ")

                const PropertyValue* pProperties = rValues.getConstArray();

                Sequence< OUString > aSetNames(rValues.getLength());
                OUString* pSetNames = aSetNames.getArray();

                Sequence< Any> aSetValues(rValues.getLength());
                Any* pSetValues = aSetValues.getArray();

                bool bEmptyNode = rNode.isEmpty();
                for(sal_Int32 k = 0; k < rValues.getLength(); k++)
                {
                    pSetNames[k] =  pProperties[k].Name.copy( bEmptyNode ? 1 : 0);
                    pSetValues[k] = pProperties[k].Value;
                }
                bRet = PutProperties(aSetNames, aSetValues);
            }
            else
            {
                const PropertyValue* pValues = rValues.getConstArray();

                
                for(int nValue = 0; nValue < rValues.getLength();nValue++)
                {
                    try
                    {
                        OUString sSubNode = lcl_extractSetPropertyName( pValues[nValue].Name, rNode );

                        if(xCont->hasByName(sSubNode))
                            xCont->replaceByName(sSubNode, pValues[nValue].Value);
                        else
                            xCont->insertByName(sSubNode, pValues[nValue].Value);
                    }
                    CATCH_INFO("Exception from insert/replaceByName(): ");
                }
                xBatch->commitChanges();
            }
        }
#ifdef DBG_UTIL
        catch (const Exception& rEx)
        {
            lcl_CFG_DBG_EXCEPTION("Exception from ReplaceSetProperties: ", rEx);
#else
        catch (const Exception&)
        {
#endif
            bRet = false;
        }
    }
    return bRet;
}

bool ConfigItem::AddNode(const OUString& rNode, const OUString& rNewNode)
{
    ValueCounter_Impl aCounter(m_nInValueChange);
    bool bRet = true;
    Reference<XHierarchicalNameAccess> xHierarchyAccess = GetTree();
    if(xHierarchyAccess.is())
    {
        Reference<XChangesBatch> xBatch(xHierarchyAccess, UNO_QUERY);
        try
        {
            Reference<XNameContainer> xCont;
            if(!rNode.isEmpty())
            {
                Any aNode = xHierarchyAccess->getByHierarchicalName(rNode);
                aNode >>= xCont;
            }
            else
                xCont = Reference<XNameContainer> (xHierarchyAccess, UNO_QUERY);
            if(!xCont.is())
                return false;

            Reference<XSingleServiceFactory> xFac(xCont, UNO_QUERY);

            if(xFac.is())
            {
                if(!xCont->hasByName(rNewNode))
                {
                    Reference<XInterface> xInst = xFac->createInstance();
                    Any aVal; aVal <<= xInst;
                    xCont->insertByName(rNewNode, aVal);
                }
                try
                {
                    xBatch->commitChanges();
                }
                CATCH_INFO("Exception from commitChanges(): ")
            }
            else
            {
                
                try
                {
                    if(!xCont->hasByName(rNewNode))
                        xCont->insertByName(rNewNode, Any());
                }
                CATCH_INFO("Exception from AddNode(): ")
            }
            xBatch->commitChanges();
        }
#ifdef DBG_UTIL
        catch (const Exception& rEx)
        {
            lcl_CFG_DBG_EXCEPTION("Exception from AddNode(): ", rEx);
#else
        catch (const Exception&)
        {
#endif
            bRet = false;
        }
    }
    return bRet;
}

sal_Int16   ConfigItem::GetMode() const
{
    return m_nMode;
}

void    ConfigItem::SetModified()
{
    m_bIsModified = true;
}

void    ConfigItem::ClearModified()
{
    m_bIsModified = false;
}

bool ConfigItem::IsModified() const
{
    return m_bIsModified;
}

bool ConfigItem::IsInValueChange() const
{
    return m_nInValueChange > 0;
}

Reference< XHierarchicalNameAccess> ConfigItem::GetTree()
{
    Reference< XHierarchicalNameAccess> xRet;
    if(!m_xHierarchyAccess.is())
        xRet = ConfigManager::acquireTree(*this);
    else
        xRet = m_xHierarchyAccess;
    return xRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
