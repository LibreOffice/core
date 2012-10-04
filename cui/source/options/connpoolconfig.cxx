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

#include "connpoolconfig.hxx"
#include "connpoolsettings.hxx"

#include "connpooloptions.hxx"
#include <svl/itemset.hxx>
#include <unotools/confignode.hxx>
#include <comphelper/extract.hxx>
#include <svl/eitem.hxx>
#include <comphelper/processfactory.hxx>
#include "sdbcdriverenum.hxx"

//........................................................................
namespace offapp
{
//........................................................................

    using namespace ::utl;
    using namespace ::com::sun::star::uno;

    //--------------------------------------------------------------------
    static const ::rtl::OUString& getConnectionPoolNodeName()
    {
        static ::rtl::OUString s_sNodeName(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.Office.DataAccess/ConnectionPool") );
        return s_sNodeName;
    }

    //--------------------------------------------------------------------
    static const ::rtl::OUString& getEnablePoolingNodeName()
    {
        static ::rtl::OUString s_sNodeName(RTL_CONSTASCII_USTRINGPARAM("EnablePooling"));
        return s_sNodeName;
    }

    //--------------------------------------------------------------------
    static const ::rtl::OUString& getDriverSettingsNodeName()
    {
        static ::rtl::OUString s_sNodeName(RTL_CONSTASCII_USTRINGPARAM("DriverSettings"));
        return s_sNodeName;
    }

    //--------------------------------------------------------------------
    static const ::rtl::OUString& getDriverNameNodeName()
    {
        static ::rtl::OUString s_sNodeName(RTL_CONSTASCII_USTRINGPARAM("DriverName"));
        return s_sNodeName;
    }

    //--------------------------------------------------------------------
    static const ::rtl::OUString& getEnableNodeName()
    {
        static ::rtl::OUString s_sNodeName(RTL_CONSTASCII_USTRINGPARAM("Enable"));
        return s_sNodeName;
    }

    //--------------------------------------------------------------------
    static const ::rtl::OUString& getTimeoutNodeName()
    {
        static ::rtl::OUString s_sNodeName(RTL_CONSTASCII_USTRINGPARAM("Timeout"));
        return s_sNodeName;
    }

    //====================================================================
    //= ConnectionPoolConfig
    //====================================================================
    //--------------------------------------------------------------------
    void ConnectionPoolConfig::GetOptions(SfxItemSet& _rFillItems)
    {
        // the config node where all pooling relevant info are stored under
        OConfigurationTreeRoot aConnectionPoolRoot = OConfigurationTreeRoot::createWithServiceFactory(
            ::comphelper::getProcessServiceFactory(), getConnectionPoolNodeName(), -1, OConfigurationTreeRoot::CM_READONLY);

        // the global "enabled" flag
        Any aEnabled = aConnectionPoolRoot.getNodeValue(getEnablePoolingNodeName());
        sal_Bool bEnabled = sal_True;
        aEnabled >>= bEnabled;
        _rFillItems.Put(SfxBoolItem(SID_SB_POOLING_ENABLED, bEnabled));

        // the settings for the single drivers
        DriverPoolingSettings aSettings;
        // first get all the drivers register at the driver manager
        ODriverEnumeration aEnumDrivers;
        for (   ODriverEnumeration::const_iterator aLoopDrivers = aEnumDrivers.begin();
                aLoopDrivers != aEnumDrivers.end();
                ++aLoopDrivers
            )
        {
            aSettings.push_back(DriverPooling(*aLoopDrivers, sal_False, 120));
        }

        // then look for which of them settings are stored in the configuration
        OConfigurationNode aDriverSettings = aConnectionPoolRoot.openNode(getDriverSettingsNodeName());

        Sequence< ::rtl::OUString > aDriverKeys = aDriverSettings.getNodeNames();
        const ::rtl::OUString* pDriverKeys = aDriverKeys.getConstArray();
        const ::rtl::OUString* pDriverKeysEnd = pDriverKeys + aDriverKeys.getLength();
        for (;pDriverKeys != pDriverKeysEnd; ++pDriverKeys)
        {
            // the name of the driver in this round
            OConfigurationNode aThisDriverSettings = aDriverSettings.openNode(*pDriverKeys);
            ::rtl::OUString sThisDriverName;
            aThisDriverSettings.getNodeValue(getDriverNameNodeName()) >>= sThisDriverName;

            // look if we (resp. the driver manager) know this driver
            // doing O(n) search here, which is expensive, but this doesn't matter in this small case ...
             DriverPoolingSettings::iterator aLookup;
             for    (   aLookup = aSettings.begin();
                    aLookup != aSettings.end();
                    ++aLookup
                )
                if (sThisDriverName.equals(aLookup->sName))
                    break;

            if (aLookup == aSettings.end())
            {   // do not know the driver - add it
                aSettings.push_back(DriverPooling(sThisDriverName, sal_False, 120));

                // and the position of the new entry
                aLookup = aSettings.end();
                --aLookup;
            }

            // now fill this entry with the settings from the configuration
            aThisDriverSettings.getNodeValue(getEnableNodeName()) >>= aLookup->bEnabled;
            aThisDriverSettings.getNodeValue(getTimeoutNodeName()) >>= aLookup->nTimeoutSeconds;
        }

        _rFillItems.Put(DriverPoolingSettingsItem(SID_SB_DRIVER_TIMEOUTS, aSettings));
    }

    //--------------------------------------------------------------------
    void ConnectionPoolConfig::SetOptions(const SfxItemSet& _rSourceItems)
    {
        // the config node where all pooling relevant info are stored under
        OConfigurationTreeRoot aConnectionPoolRoot = OConfigurationTreeRoot::createWithServiceFactory(
            ::comphelper::getProcessServiceFactory(), getConnectionPoolNodeName(), -1, OConfigurationTreeRoot::CM_UPDATABLE);

        if (!aConnectionPoolRoot.isValid())
            // already asserted by the OConfigurationTreeRoot
            return;

        sal_Bool bNeedCommit = sal_False;

        // the global "enabled" flag
        SFX_ITEMSET_GET( _rSourceItems, pEnabled, SfxBoolItem, SID_SB_POOLING_ENABLED, sal_True );
        if (pEnabled)
        {
            sal_Bool bEnabled = pEnabled->GetValue();
            aConnectionPoolRoot.setNodeValue(getEnablePoolingNodeName(), Any(&bEnabled, ::getBooleanCppuType()));
            bNeedCommit = sal_True;
        }

        // the settings for the single drivers
        SFX_ITEMSET_GET( _rSourceItems, pDriverSettings, DriverPoolingSettingsItem, SID_SB_DRIVER_TIMEOUTS, sal_True );
        if (pDriverSettings)
        {
            OConfigurationNode aDriverSettings = aConnectionPoolRoot.openNode(getDriverSettingsNodeName());
            if (!aDriverSettings.isValid())
                return;

            ::rtl::OUString sThisDriverName;
            OConfigurationNode aThisDriverSettings;

            const DriverPoolingSettings& rNewSettings = pDriverSettings->getSettings();
            for (   DriverPoolingSettings::const_iterator aLoop = rNewSettings.begin();
                    aLoop != rNewSettings.end();
                    ++aLoop
                )
            {
                // need the name as ::rtl::OUString
                sThisDriverName = aLoop->sName;

                // the sub-node for this driver
                if (aDriverSettings.hasByName(aLoop->sName))
                    aThisDriverSettings = aDriverSettings.openNode(aLoop->sName);
                else
                    aThisDriverSettings = aDriverSettings.createNode(aLoop->sName);

                // set the values
                aThisDriverSettings.setNodeValue(getDriverNameNodeName(), makeAny(sThisDriverName));
                aThisDriverSettings.setNodeValue(getEnableNodeName(), Any(&aLoop->bEnabled, ::getBooleanCppuType()));
                aThisDriverSettings.setNodeValue(getTimeoutNodeName(), makeAny(aLoop->nTimeoutSeconds));
            }
            bNeedCommit = sal_True;
        }
        if (bNeedCommit)
            aConnectionPoolRoot.commit();
    }

//........................................................................
}   // namespace offapp
//........................................................................


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
