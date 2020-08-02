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

#include <sal/config.h>

#include <sal/log.hxx>
#include <unotools/configitem.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/configpaths.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XChangesListener.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/configuration/XTemplateContainer.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <osl/diagnose.h>
#include <comphelper/sequence.hxx>
#include <comphelper/solarmutex.hxx>
#include <rtl/ref.hxx>
#include <tools/diagnose_ex.h>

using namespace utl;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace com::sun::star::configuration;

#include <cppuhelper/implbase.hxx>

/*
    The ConfigChangeListener_Impl receives notifications from the configuration about changes that
    have happened. It forwards this notification to the ConfigItem it knows a pParent by calling its
    "CallNotify" method. As ConfigItems are most probably not thread safe, the SolarMutex is acquired
    before doing so.
*/

namespace utl{
    class ConfigChangeListener_Impl : public cppu::WeakImplHelper
    <
        css::util::XChangesListener
    >
    {
        public:
            ConfigItem*                 pParent;
            const Sequence< OUString >  aPropertyNames;
            ConfigChangeListener_Impl(ConfigItem& rItem, const Sequence< OUString >& rNames);

        //XChangesListener
        virtual void SAL_CALL changesOccurred( const ChangesEvent& Event ) override;

        //XEventListener
        virtual void SAL_CALL disposing( const EventObject& Source ) override;
    };
}

namespace {

class ValueCounter_Impl
{
    sal_Int16& rCnt;
public:
    explicit ValueCounter_Impl(sal_Int16& rCounter)
        : rCnt(rCounter)
    {
        rCnt++;
    }
    ~ValueCounter_Impl()
    {
        OSL_ENSURE(rCnt>0, "RefCount < 0 ??");
        rCnt--;
    }
};

}

ConfigChangeListener_Impl::ConfigChangeListener_Impl(
             ConfigItem& rItem, const Sequence< OUString >& rNames) :
    pParent(&rItem),
    aPropertyNames(rNames)
{
}

void ConfigChangeListener_Impl::changesOccurred( const ChangesEvent& rEvent )
{
    Sequence<OUString>  aChangedNames(rEvent.Changes.getLength());
    OUString* pNames = aChangedNames.getArray();

    sal_Int32 nNotify = 0;
    for(const auto& rElementChange : rEvent.Changes)
    {
        OUString sTemp;
        rElementChange.Accessor >>= sTemp;
        //true if the path is completely correct or if it is longer
        //i.e ...Print/Content/Graphic and .../Print
        bool bFound = std::any_of(aPropertyNames.begin(), aPropertyNames.end(),
            [&sTemp](const OUString& rCheckPropertyName) { return isPrefixOfConfigurationPath(sTemp, rCheckPropertyName); });
        if(bFound)
            pNames[nNotify++] = sTemp;
    }
    if( nNotify )
    {
        ::comphelper::SolarMutex *pMutex = ::comphelper::SolarMutex::get();
        if ( pMutex )
        {
            osl::Guard<comphelper::SolarMutex> aMutexGuard( pMutex );
            aChangedNames.realloc(nNotify);
            pParent->CallNotify(aChangedNames);
        }
    }
}

void ConfigChangeListener_Impl::disposing( const EventObject& /*rSource*/ )
{
    pParent->RemoveChangesListener();
}

ConfigItem::ConfigItem(const OUString &rSubTree, ConfigItemMode nSetMode ) :
    sSubTree(rSubTree),
    m_nMode(nSetMode),
    m_bIsModified(false),
    m_bEnableInternalNotification(false),
    m_nInValueChange(0)
{
    if (utl::ConfigManager::IsFuzzing())
        return;

    if (nSetMode & ConfigItemMode::ReleaseTree)
        ConfigManager::getConfigManager().addConfigItem(*this);
    else
        m_xHierarchyAccess = ConfigManager::getConfigManager().addConfigItem(*this);
}

ConfigItem::~ConfigItem()
{
    RemoveChangesListener();
    ConfigManager::getConfigManager().removeConfigItem(*this);
}

void ConfigItem::CallNotify( const css::uno::Sequence<OUString>& rPropertyNames )
{
    // the call is forwarded to the virtual Notify() method
    // it is pure virtual, so all classes deriving from ConfigItem have to decide how they
    // want to notify listeners
    if(m_nInValueChange <= 0 || m_bEnableInternalNotification)
        Notify(rPropertyNames);
}

void ConfigItem::impl_packLocalizedProperties(  const   Sequence< OUString >&   lInNames    ,
                                                const   Sequence< Any >&        lInValues   ,
                                                        Sequence< Any >&        lOutValues  )
{
    // Safe impossible cases.
    // This method should be called for special ConfigItem-mode only!
    OSL_ENSURE( ((m_nMode & ConfigItemMode::AllLocales ) == ConfigItemMode::AllLocales), "ConfigItem::impl_packLocalizedProperties() Wrong call of this method detected!" );

    sal_Int32                   nSourceCounter;      // used to step during input lists
    sal_Int32                   nSourceSize;         // marks end of loop over input lists
    sal_Int32                   nDestinationCounter; // actual position in output lists
    sal_Int32                   nPropertyCounter;    // counter of inner loop for Sequence< PropertyValue >
    sal_Int32                   nPropertiesSize;     // marks end of inner loop
    Sequence< OUString >        lPropertyNames;      // list of all locales for localized entry
    Sequence< PropertyValue >   lProperties;         // localized values of a configuration entry packed for return
    Reference< XInterface >     xLocalizedNode;      // if cfg entry is localized ... lInValues contains an XInterface!

    // Optimise follow algorithm ... A LITTLE BIT :-)
    // There exist two different possibilities:
    //  i ) There exist no localized entries ...                        =>  size of lOutValues will be the same like lInNames/lInValues!
    //  ii) There exist some (mostly one or two) localized entries ...  =>  size of lOutValues will be the same like lInNames/lInValues!
    //  ... Why? If a localized value exist - the any is filled with an XInterface object (is a SetNode-service).
    //      We read all his child nodes and pack it into Sequence< PropertyValue >.
    //      The result list we pack into the return any. We never change size of lists!
    nSourceSize = lInNames.getLength();
    lOutValues.realloc( nSourceSize );

    // Algorithm:
    // Copy all names and values from in to out lists.
    // Look for special localized entries ... You can detect it as "XInterface" packed into an Any.
    // Use this XInterface-object to read all localized values and pack it into Sequence< PropertValue >.
    // Add this list to out lists then.

    nDestinationCounter = 0;
    for( nSourceCounter=0; nSourceCounter<nSourceSize; ++nSourceCounter )
    {
        // If item a special localized one ... convert and pack it ...
        if( lInValues[nSourceCounter].getValueTypeName() == "com.sun.star.uno.XInterface" )
        {
            lInValues[nSourceCounter] >>= xLocalizedNode;
            Reference< XNameContainer > xSetAccess( xLocalizedNode, UNO_QUERY );
            if( xSetAccess.is() )
            {
                lPropertyNames  =   xSetAccess->getElementNames();
                nPropertiesSize =   lPropertyNames.getLength();
                lProperties.realloc( nPropertiesSize );

                for( nPropertyCounter=0; nPropertyCounter<nPropertiesSize; ++nPropertyCounter )
                {
                    lProperties[nPropertyCounter].Name  =   lPropertyNames[nPropertyCounter];
                    OUString sLocaleValue;
                    xSetAccess->getByName( lPropertyNames[nPropertyCounter] ) >>= sLocaleValue;
                    lProperties[nPropertyCounter].Value <<= sLocaleValue;
                }

                lOutValues[nDestinationCounter] <<= lProperties;
            }
        }
        // ... or copy normal items to return lists directly.
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
    // Safe impossible cases.
    // This method should be called for special ConfigItem-mode only!
    OSL_ENSURE( ((m_nMode & ConfigItemMode::AllLocales ) == ConfigItemMode::AllLocales), "ConfigItem::impl_unpackLocalizedProperties() Wrong call of this method detected!" );

    sal_Int32                   nSourceSize;         // marks end of loop over input lists
    sal_Int32                   nDestinationCounter; // actual position in output lists
    sal_Int32                   nPropertiesSize;     // marks end of inner loop
    OUString                    sNodeName;           // base name of node ( e.g. "UIName/" ) ... expand to locale ( e.g. "UIName/de" )
    Sequence< PropertyValue >   lProperties;         // localized values of a configuration entry gotten from lInValues-Any

    // Optimise follow algorithm ... A LITTLE BIT :-)
    // There exist two different possibilities:
    //  i ) There exist no localized entries ...                        =>  size of lOutNames/lOutValues will be the same like lInNames/lInValues!
    //  ii) There exist some (mostly one or two) localized entries ...  =>  size of lOutNames/lOutValues will be some bytes greater then lInNames/lInValues.
    //  =>  I think we should make it fast for i). ii) is a special case and mustn't be SOOOO... fast.
    //      We should reserve same space for output list like input ones first.
    //      Follow algorithm looks for these borders and change it for ii) only!
    //      It will be faster then a "realloc()" call in every loop ...
    nSourceSize = lInNames.getLength();

    lOutNames.realloc   ( nSourceSize );
    lOutValues.realloc  ( nSourceSize );

    // Algorithm:
    // Copy all names and values from const to return lists.
    // Look for special localized entries ... You can detect it as Sequence< PropertyValue > packed into an Any.
    // Split it ... insert PropertyValue.Name to lOutNames and PropertyValue.Value to lOutValues.

    nDestinationCounter = 0;
    for( sal_Int32 nSourceCounter=0; nSourceCounter<nSourceSize; ++nSourceCounter )
    {
        // If item a special localized one ... split it and insert his parts to output lists ...
        if( lInValues[nSourceCounter].getValueType() == cppu::UnoType<Sequence<PropertyValue>>::get() )
        {
            lInValues[nSourceCounter] >>= lProperties;
            nPropertiesSize = lProperties.getLength();

            sNodeName = lInNames[nSourceCounter] + "/";

            if( (nDestinationCounter+nPropertiesSize) > lOutNames.getLength() )
            {
                lOutNames.realloc   ( nDestinationCounter+nPropertiesSize );
                lOutValues.realloc  ( nDestinationCounter+nPropertiesSize );
            }

            for( const auto& rProperty : std::as_const(lProperties) )
            {
                lOutNames [nDestinationCounter] = sNodeName + rProperty.Name;
                lOutValues[nDestinationCounter] = rProperty.Value;
                ++nDestinationCounter;
            }
        }
        // ... or copy normal items to return lists directly.
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

Sequence< sal_Bool > ConfigItem::GetReadOnlyStates(const css::uno::Sequence< OUString >& rNames)
{
    sal_Int32 i;

    // size of return list is fix!
    // Every item must match to length of incoming name list.
    sal_Int32 nCount = rNames.getLength();
    Sequence< sal_Bool > lStates(nCount);

    // We must be sure to return a valid information every time!
    // Set default to non readonly... similar to the configuration handling of this property.
    std::fill(lStates.begin(), lStates.end(), false);

    // no access - no information...
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

            (void)::utl::splitLastFromConfigurationPath(sName,sPath,sProperty);
            if (sPath.isEmpty() && sProperty.isEmpty())
            {
                OSL_FAIL("ConfigItem::IsReadonly() split failed");
                continue;
            }

            Reference< XInterface >       xNode;
            Reference< XPropertySet >     xSet;
            Reference< XPropertySetInfo > xInfo;
            if (!sPath.isEmpty())
            {
                Any aNode = xHierarchyAccess->getByHierarchicalName(sPath);
                if (!(aNode >>= xNode) || !xNode.is())
                {
                    OSL_FAIL("ConfigItem::IsReadonly() no set available");
                    continue;
                }
            }
            else
            {
                xNode = xHierarchyAccess;
            }

            xSet.set(xNode, UNO_QUERY);
            if (xSet.is())
            {
                xInfo = xSet->getPropertySetInfo();
                OSL_ENSURE(xInfo.is(), "ConfigItem::IsReadonly() getPropertySetInfo failed ...");
            }
            else
            {
                xInfo.set(xNode, UNO_QUERY);
                OSL_ENSURE(xInfo.is(), "ConfigItem::IsReadonly() UNO_QUERY failed ...");
            }

            if (!xInfo.is())
            {
                OSL_FAIL("ConfigItem::IsReadonly() no prop info available");
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
            catch (const Exception&)
            {
                TOOLS_WARN_EXCEPTION(
                    "unotools.config",
                    "ignoring XHierarchicalNameAccess to /org.openoffice."
                        << sSubTree << "/" << pNames[i]);
            }
        }

        // In special mode "ALL_LOCALES" we must convert localized values to Sequence< PropertyValue >.
        if((m_nMode & ConfigItemMode::AllLocales ) == ConfigItemMode::AllLocales)
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
        Sequence< OUString >    lNames;
        Sequence< Any >         lValues;
        const OUString*         pNames  = nullptr;
        const Any*              pValues = nullptr;
        sal_Int32               nNameCount;
        if(( m_nMode & ConfigItemMode::AllLocales ) == ConfigItemMode::AllLocales )
        {
            // If ConfigItem works in "ALL_LOCALES"-mode ... we must support a Sequence< PropertyValue >
            // as value of a localized configuration entry!
            // How we can do that?
            // We must split all PropertyValues to "Sequence< OUString >" AND "Sequence< Any >"!
            impl_unpackLocalizedProperties( rNames, rValues, lNames, lValues );
            pNames      = lNames.getConstArray  ();
            pValues     = lValues.getConstArray ();
            nNameCount  = lNames.getLength      ();
        }
        else
        {
            // This is the normal mode ...
            // Use given input lists directly.
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
                else //direct value
                {
                    xTopNodeReplace->replaceByName(sProperty, pValues[i]);
                }
            }
            catch (css::uno::Exception &)
            {
                TOOLS_WARN_EXCEPTION("unotools.config", "Exception from PutProperties");
            }
        }
        try
        {
            Reference<XChangesBatch> xBatch(xHierarchyAccess, UNO_QUERY);
            xBatch->commitChanges();
        }
        catch (css::uno::Exception &)
        {
            TOOLS_WARN_EXCEPTION("unotools.config", "Exception from commitChanges");
        }
    }

    return bRet;
}

void ConfigItem::DisableNotification()
{
    OSL_ENSURE( xChangeLstnr.is(), "ConfigItem::DisableNotification: notifications not enabled currently!" );
    RemoveChangesListener();
}

bool ConfigItem::EnableNotification(const Sequence< OUString >& rNames,
                                    bool bEnableInternalNotification )
{
    OSL_ENSURE(!(m_nMode & ConfigItemMode::ReleaseTree), "notification in ConfigItemMode::ReleaseTree mode not possible");
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
    Reference<XHierarchicalNameAccess> xHierarchyAccess = GetTree();
    if(!xHierarchyAccess.is())
        return;

    Reference<XChangesNotifier> xChgNot(xHierarchyAccess, UNO_QUERY);
    if(xChgNot.is() && xChangeLstnr.is())
    {
        try
        {
            xChgNot->removeChangesListener( xChangeLstnr );
            xChangeLstnr = nullptr;
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
    case ConfigNameFormat::LocalNode:
        // unaltered - this is our input format
        break;

    case ConfigNameFormat::LocalPath:
        {
            Reference<XTemplateContainer> xTypeContainer(_xParentNode, UNO_QUERY);
            if (xTypeContainer.is())
            {
                OUString sTypeName = xTypeContainer->getElementTemplateName();
                sTypeName = sTypeName.copy(sTypeName.lastIndexOf('/')+1);

                std::transform(_rNames.begin(), _rNames.end(), _rNames.begin(),
                    [&sTypeName](const OUString& rName) -> OUString { return wrapConfigurationElementName(rName,sTypeName); });
            }
            else
            {
                Reference<XServiceInfo> xSVI(_xParentNode, UNO_QUERY);
                if (xSVI.is() && xSVI->supportsService("com.sun.star.configuration.SetAccess"))
                {
                    std::transform(_rNames.begin(), _rNames.end(), _rNames.begin(),
                        [](const OUString& rName) -> OUString { return wrapConfigurationElementName(rName); });
                }
            }
        }
        break;

    }
}

Sequence< OUString > ConfigItem::GetNodeNames(const OUString& rNode)
{
    ConfigNameFormat const eDefaultFormat = ConfigNameFormat::LocalNode; // CONFIG_NAME_DEFAULT;

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
                xCont.set(xHierarchyAccess, UNO_QUERY);
            if(xCont.is())
            {
                aRet = xCont->getElementNames();
                lcl_normalizeLocalNames(aRet,eFormat,xCont);
            }

        }
        catch (css::uno::Exception &)
        {
            TOOLS_WARN_EXCEPTION("unotools.config", "Exception from GetNodeNames");
        }
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
                xCont.set(xHierarchyAccess, UNO_QUERY);
            if(!xCont.is())
                return false;
            const Sequence< OUString > aNames = xCont->getElementNames();
            Reference<XChangesBatch> xBatch(xHierarchyAccess, UNO_QUERY);
            for(const OUString& rName : aNames)
            {
                try
                {
                    xCont->removeByName(rName);
                }
                catch (css::uno::Exception &)
                {
                    TOOLS_WARN_EXCEPTION("unotools.config", "Exception from removeByName");
                }
            }
            xBatch->commitChanges();
            bRet = true;
        }
        catch (css::uno::Exception &)
        {
            TOOLS_WARN_EXCEPTION("unotools.config", "Exception from ClearNodeSet");
        }
    }
    return bRet;
}

bool ConfigItem::ClearNodeElements(const OUString& rNode, Sequence< OUString > const & rElements)
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
                xCont.set(xHierarchyAccess, UNO_QUERY);
            if(!xCont.is())
                return false;
            try
            {
                for(const OUString& rElement : rElements)
                {
                    xCont->removeByName(rElement);
                }
                Reference<XChangesBatch> xBatch(xHierarchyAccess, UNO_QUERY);
                xBatch->commitChanges();
            }
            catch (css::uno::Exception &)
            {
                TOOLS_WARN_EXCEPTION("unotools.config", "Exception from commitChanges()");
            }
            bRet = true;
        }
        catch (css::uno::Exception &)
        {
            TOOLS_WARN_EXCEPTION("unotools.config", "Exception from GetNodeNames()");
        }
    }
    return bRet;
}

static OUString lcl_extractSetPropertyName( const OUString& rInPath, const OUString& rPrefix )
{
    OUString const sSubPath = dropPrefixFromConfigurationPath( rInPath, rPrefix);
    return extractFirstFromConfigurationPath( sSubPath );
}

static
Sequence< OUString > lcl_extractSetPropertyNames( const Sequence< PropertyValue >& rValues, const OUString& rPrefix )
{
    Sequence< OUString > aSubNodeNames(rValues.getLength());
    OUString* pSubNodeNames = aSubNodeNames.getArray();

    OUString sLastSubNode;
    sal_Int32 nSubIndex = 0;

    for(const PropertyValue& rProperty : rValues)
    {
        OUString const sSubPath = dropPrefixFromConfigurationPath( rProperty.Name, rPrefix);
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

// Add or change properties
bool ConfigItem::SetSetProperties(
    const OUString& rNode, const Sequence< PropertyValue >& rValues)
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
                xCont.set(xHierarchyAccess, UNO_QUERY);
            if(!xCont.is())
                return false;

            Reference<XSingleServiceFactory> xFac(xCont, UNO_QUERY);

            if(xFac.is())
            {
                const Sequence< OUString > aSubNodeNames = lcl_extractSetPropertyNames(rValues, rNode);

                for(const auto& rSubNodeName : aSubNodeNames)
                {
                    if(!xCont->hasByName(rSubNodeName))
                    {
                        Reference<XInterface> xInst = xFac->createInstance();
                        Any aVal; aVal <<= xInst;
                        xCont->insertByName(rSubNodeName, aVal);
                    }
                    //set values
                }
                try
                {
                    xBatch->commitChanges();
                }
                catch (css::uno::Exception &)
                {
                    TOOLS_WARN_EXCEPTION("unotools.config", "Exception from commitChanges()");
                }

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
                //if no factory is available then the node contains basic data elements
                for(const PropertyValue& rValue : rValues)
                {
                    try
                    {
                        OUString sSubNode = lcl_extractSetPropertyName( rValue.Name, rNode );

                        if(xCont->hasByName(sSubNode))
                            xCont->replaceByName(sSubNode, rValue.Value);
                        else
                            xCont->insertByName(sSubNode, rValue.Value);

                        OSL_ENSURE( xHierarchyAccess->hasByHierarchicalName(rValue.Name),
                            "Invalid config path" );
                    }
                    catch (css::uno::Exception &)
                    {
                        TOOLS_WARN_EXCEPTION("unotools.config", "Exception from insert/replaceByName()");
                    }
                }
                xBatch->commitChanges();
            }
        }
        catch (const Exception&)
        {
            TOOLS_WARN_EXCEPTION("unotools.config", "Exception from SetSetProperties");
            bRet = false;
        }
    }
    return bRet;
}

bool ConfigItem::ReplaceSetProperties(
    const OUString& rNode, const Sequence< PropertyValue >& rValues)
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
                xCont.set(xHierarchyAccess, UNO_QUERY);
            if(!xCont.is())
                return false;

            // JB: Change: now the same name handling for sets of simple values
            const Sequence< OUString > aSubNodeNames = lcl_extractSetPropertyNames(rValues, rNode);

            Reference<XSingleServiceFactory> xFac(xCont, UNO_QUERY);
            const bool isSimpleValueSet = !xFac.is();

            //remove unknown members first
            {
                const Sequence<OUString> aContainerSubNodes = xCont->getElementNames();

                for(const OUString& rContainerSubNode : aContainerSubNodes)
                {
                    bool bFound = comphelper::findValue(aSubNodeNames, rContainerSubNode) != -1;
                    if(!bFound)
                        try
                        {
                            xCont->removeByName(rContainerSubNode);
                        }
                        catch (const Exception&)
                        {
                            if (isSimpleValueSet)
                            {
                                try
                                {
                                    // #i37322#: fallback action: replace with <void/>
                                    xCont->replaceByName(rContainerSubNode, Any());
                                    // fallback successful: continue looping
                                    continue;
                                }
                                catch (Exception &)
                                {} // propagate original exception, if fallback fails
                            }
                            throw;
                        }
                }
                try { xBatch->commitChanges(); }
                catch (css::uno::Exception &)
                {
                    TOOLS_WARN_EXCEPTION("unotools.config", "Exception from commitChanges");
                }
            }

            if(xFac.is()) // !isSimpleValueSet
            {
                for(const OUString& rSubNodeName : aSubNodeNames)
                {
                    if(!xCont->hasByName(rSubNodeName))
                    {
                        //create if not available
                        Reference<XInterface> xInst = xFac->createInstance();
                        Any aVal; aVal <<= xInst;
                        xCont->insertByName(rSubNodeName, aVal);
                    }
                }
                try { xBatch->commitChanges(); }
                catch (css::uno::Exception &)
                {
                    TOOLS_WARN_EXCEPTION("unotools.config", "Exception from commitChanges");
                }

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
                //if no factory is available then the node contains basic data elements
                for(const PropertyValue& rValue : rValues)
                {
                    try
                    {
                        OUString sSubNode = lcl_extractSetPropertyName( rValue.Name, rNode );

                        if(xCont->hasByName(sSubNode))
                            xCont->replaceByName(sSubNode, rValue.Value);
                        else
                            xCont->insertByName(sSubNode, rValue.Value);
                    }
                    catch (css::uno::Exception &)
                    {
                        TOOLS_WARN_EXCEPTION("unotools.config", "Exception from insert/replaceByName");
                    }
                }
                xBatch->commitChanges();
            }
        }
        catch (const Exception& )
        {
            TOOLS_WARN_EXCEPTION("unotools.config", "Exception from ReplaceSetProperties");
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
                xCont.set(xHierarchyAccess, UNO_QUERY);
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
                catch (css::uno::Exception &)
                {
                    TOOLS_WARN_EXCEPTION("unotools.config", "Exception from commitChanges");
                }
            }
            else
            {
                //if no factory is available then the node contains basic data elements
                try
                {
                    if(!xCont->hasByName(rNewNode))
                        xCont->insertByName(rNewNode, Any());
                }
                catch (css::uno::Exception &)
                {
                    TOOLS_WARN_EXCEPTION("unotools.config", "Exception from AddNode");
                }
            }
            xBatch->commitChanges();
        }
        catch (const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("unotools.config");
            bRet = false;
        }
    }
    return bRet;
}


void    ConfigItem::SetModified()
{
    m_bIsModified = true;
}

void    ConfigItem::ClearModified()
{
    m_bIsModified = false;
}

Reference< XHierarchicalNameAccess> ConfigItem::GetTree()
{
    Reference< XHierarchicalNameAccess> xRet;
    if (utl::ConfigManager::IsFuzzing())
        return xRet;
    if(!m_xHierarchyAccess.is())
        xRet = ConfigManager::acquireTree(*this);
    else
        xRet = m_xHierarchyAccess;
    return xRet;
}

void ConfigItem::Commit()
{
    ImplCommit();
    ClearModified();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
