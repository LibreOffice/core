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

#include "connpoolconfig.hxx"
#include "connpoolsettings.hxx"

#include "connpooloptions.hxx"
#include <svl/itemset.hxx>
#include <unotools/confignode.hxx>
#include <comphelper/extract.hxx>
#include <svl/eitem.hxx>
#include <comphelper/processfactory.hxx>
#include "sdbcdriverenum.hxx"


namespace offapp
{


    using namespace ::utl;
    using namespace ::com::sun::star::uno;

    
    static const OUString& getConnectionPoolNodeName()
    {
        static OUString s_sNodeName("org.openoffice.Office.DataAccess/ConnectionPool" );
        return s_sNodeName;
    }

    
    static const OUString& getEnablePoolingNodeName()
    {
        static OUString s_sNodeName("EnablePooling");
        return s_sNodeName;
    }

    
    static const OUString& getDriverSettingsNodeName()
    {
        static OUString s_sNodeName("DriverSettings");
        return s_sNodeName;
    }

    
    static const OUString& getDriverNameNodeName()
    {
        static OUString s_sNodeName("DriverName");
        return s_sNodeName;
    }

    
    static const OUString& getEnableNodeName()
    {
        static OUString s_sNodeName("Enable");
        return s_sNodeName;
    }

    
    static const OUString& getTimeoutNodeName()
    {
        static OUString s_sNodeName("Timeout");
        return s_sNodeName;
    }

    
    
    
    
    void ConnectionPoolConfig::GetOptions(SfxItemSet& _rFillItems)
    {
        
        OConfigurationTreeRoot aConnectionPoolRoot = OConfigurationTreeRoot::createWithComponentContext(
            ::comphelper::getProcessComponentContext(), getConnectionPoolNodeName(), -1, OConfigurationTreeRoot::CM_READONLY);

        
        Any aEnabled = aConnectionPoolRoot.getNodeValue(getEnablePoolingNodeName());
        sal_Bool bEnabled = sal_True;
        aEnabled >>= bEnabled;
        _rFillItems.Put(SfxBoolItem(SID_SB_POOLING_ENABLED, bEnabled));

        
        DriverPoolingSettings aSettings;
        
        ODriverEnumeration aEnumDrivers;
        for (   ODriverEnumeration::const_iterator aLoopDrivers = aEnumDrivers.begin();
                aLoopDrivers != aEnumDrivers.end();
                ++aLoopDrivers
            )
        {
            aSettings.push_back(DriverPooling(*aLoopDrivers, sal_False, 120));
        }

        
        OConfigurationNode aDriverSettings = aConnectionPoolRoot.openNode(getDriverSettingsNodeName());

        Sequence< OUString > aDriverKeys = aDriverSettings.getNodeNames();
        const OUString* pDriverKeys = aDriverKeys.getConstArray();
        const OUString* pDriverKeysEnd = pDriverKeys + aDriverKeys.getLength();
        for (;pDriverKeys != pDriverKeysEnd; ++pDriverKeys)
        {
            
            OConfigurationNode aThisDriverSettings = aDriverSettings.openNode(*pDriverKeys);
            OUString sThisDriverName;
            aThisDriverSettings.getNodeValue(getDriverNameNodeName()) >>= sThisDriverName;

            
            
             DriverPoolingSettings::iterator aLookup;
             for    (   aLookup = aSettings.begin();
                    aLookup != aSettings.end();
                    ++aLookup
                )
                if (sThisDriverName.equals(aLookup->sName))
                    break;

            if (aLookup == aSettings.end())
            {   
                aSettings.push_back(DriverPooling(sThisDriverName, sal_False, 120));

                
                aLookup = aSettings.end();
                --aLookup;
            }

            
            aThisDriverSettings.getNodeValue(getEnableNodeName()) >>= aLookup->bEnabled;
            aThisDriverSettings.getNodeValue(getTimeoutNodeName()) >>= aLookup->nTimeoutSeconds;
        }

        _rFillItems.Put(DriverPoolingSettingsItem(SID_SB_DRIVER_TIMEOUTS, aSettings));
    }

    
    void ConnectionPoolConfig::SetOptions(const SfxItemSet& _rSourceItems)
    {
        
        OConfigurationTreeRoot aConnectionPoolRoot = OConfigurationTreeRoot::createWithComponentContext(
            ::comphelper::getProcessComponentContext(), getConnectionPoolNodeName(), -1, OConfigurationTreeRoot::CM_UPDATABLE);

        if (!aConnectionPoolRoot.isValid())
            
            return;

        sal_Bool bNeedCommit = sal_False;

        
        SFX_ITEMSET_GET( _rSourceItems, pEnabled, SfxBoolItem, SID_SB_POOLING_ENABLED, true );
        if (pEnabled)
        {
            sal_Bool bEnabled = pEnabled->GetValue();
            aConnectionPoolRoot.setNodeValue(getEnablePoolingNodeName(), Any(&bEnabled, ::getBooleanCppuType()));
            bNeedCommit = sal_True;
        }

        
        SFX_ITEMSET_GET( _rSourceItems, pDriverSettings, DriverPoolingSettingsItem, SID_SB_DRIVER_TIMEOUTS, true );
        if (pDriverSettings)
        {
            OConfigurationNode aDriverSettings = aConnectionPoolRoot.openNode(getDriverSettingsNodeName());
            if (!aDriverSettings.isValid())
                return;

            OUString sThisDriverName;
            OConfigurationNode aThisDriverSettings;

            const DriverPoolingSettings& rNewSettings = pDriverSettings->getSettings();
            for (   DriverPoolingSettings::const_iterator aLoop = rNewSettings.begin();
                    aLoop != rNewSettings.end();
                    ++aLoop
                )
            {
                
                sThisDriverName = aLoop->sName;

                
                if (aDriverSettings.hasByName(aLoop->sName))
                    aThisDriverSettings = aDriverSettings.openNode(aLoop->sName);
                else
                    aThisDriverSettings = aDriverSettings.createNode(aLoop->sName);

                
                aThisDriverSettings.setNodeValue(getDriverNameNodeName(), makeAny(sThisDriverName));
                aThisDriverSettings.setNodeValue(getEnableNodeName(), Any(&aLoop->bEnabled, ::getBooleanCppuType()));
                aThisDriverSettings.setNodeValue(getTimeoutNodeName(), makeAny(aLoop->nTimeoutSeconds));
            }
            bNeedCommit = sal_True;
        }
        if (bNeedCommit)
            aConnectionPoolRoot.commit();
    }


}   



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
