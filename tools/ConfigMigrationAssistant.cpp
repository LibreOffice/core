/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Configuration file migration utility for upgrading coolwsd.xml.
 * Functions: migrate(), diffConfig(), help()
 */

#include <config.h>

#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <Poco/Util/XMLConfiguration.h>
#include <Poco/Util/AbstractConfiguration.h>
#include <Poco/AutoPtr.h>

#include <tools/ConfigMigrationAssistant.hpp>

using Poco::Util::XMLConfiguration;

const std::string NET_POST_ALLOW_HOST = ".net.post_allow.host";
const std::string STORAGE_WOPI_HOST = ".storage.wopi.host";
const std::set<std::string> multiElems {NET_POST_ALLOW_HOST, STORAGE_WOPI_HOST, ".logging.file.property", ".ssl.hpkp.pins.pin"};
const std::set<std::string> migrateIfNoTarget {".admin_console.secure_password", ".support_key"};
const std::map<std::string, std::string> renamedElems { {"loleaflet_logging", "browser_logging"} };
const std::map<std::string, std::string> specialDefault {
                    {".ssl.cert_file_path", "/etc/loolwsd/cert.pem"},
                    {".ssl.key_file_path", "/etc/loolwsd/key.pem"},
                    {".ssl.ca_file_path", "/etc/loolwsd/ca-chain.cert.pem"},
                    {".ssl.termination", "false"},
                    {".logging.file.property[@name=path]", "/var/log/loolwsd.log"} };
const std::map<std::string, std::string> specialAttribute {
                    {".logging.file", "enable"},
                    {".trace_event", "enable"},
                    {".trace", "enable"},
                    {".trace.path","compress"},
                    {".trace.path","snapshot"},
                    {".net.post_allow","allow"},
                    {".ssl.hpkp","enable"},
                    {".ssl.hpkp","report_only"},
                    {".ssl.hpkp.max_age","enable"},
                    {".ssl.hpkp.report_uri","enable"} };
//                    {".storage.filesystem","allow"}, // don't migrate this
//                    {".storage.wopi","allow"}, // and this
static std::vector<std::string> netPostAllow, netPostAllowDesc, wopiHost, wopiHostDesc, wopiHostAllow;
static bool netPostAllowAdded, wopiHostAdded;

static std::string getTargetKey(int id, const std::string& commonKeyPart)
{
    if (id == 0)
        return commonKeyPart;
    return commonKeyPart + "[" + std::to_string(id) + "]";
}

static void MigrateLevel(const XMLConfiguration &sourceConfig, XMLConfiguration &targetConfig, const std::string& sourceLevel)
{
    Poco::Util::AbstractConfiguration::Keys subKeys;
    sourceConfig.keys(sourceLevel, subKeys);
    for (const auto& key: subKeys)
    {
        const std::string fullKey = sourceLevel + "." + key;
        MigrateLevel(sourceConfig, targetConfig, fullKey);
    }
    if (subKeys.empty())
    {
        std::string sourceElement = sourceConfig.getString(sourceLevel);
        // Need to handle keys pointing to multiple elements separately, refer to multiElems
        const std::string commonKeyPart = sourceLevel.find('[') != std::string::npos
                                              ? sourceLevel.substr(0, sourceLevel.find('['))
                                              : sourceLevel;
        if (multiElems.find(commonKeyPart) != multiElems.end())
        {
            if (commonKeyPart == ".logging.file.property")
            {
                const std::string propName = sourceConfig.getString(sourceLevel + "[@name]");
                const std::string targetKey = commonKeyPart + "[@name=" + propName + "]";
                const std::string targetElement = targetConfig.getString(targetKey);
                const bool hasSpecialDefault = specialDefault.find(targetKey) != specialDefault.end();
                if (targetElement != sourceElement && (!hasSpecialDefault || sourceElement != specialDefault.at(targetKey)))
                {
                    std::cout << targetKey << ": replaced \"" << targetElement << "\" with \"" << sourceElement << "\"." << std::endl;

                    targetConfig.setString(targetKey, sourceElement);
                }
            }
            else if (commonKeyPart == NET_POST_ALLOW_HOST || commonKeyPart == STORAGE_WOPI_HOST)
            {
                bool foundKey = false;
                int id = 0;
                while (!foundKey)
                {
                    const std::string targetKey(getTargetKey(id, commonKeyPart));
                    if (!targetConfig.has(targetKey))
                    {
                        break;
                    }
                    if (targetConfig.getString(targetKey) == sourceElement)
                    {
                        foundKey = true;
                    }
                    id++;
                }
                if (!foundKey)
                {
                    if (commonKeyPart == NET_POST_ALLOW_HOST)
                        netPostAllowAdded = true;
                    else if (commonKeyPart == STORAGE_WOPI_HOST)
                        wopiHostAdded = true;
                }
                // Keep record of these configs for post processing
                if (commonKeyPart == NET_POST_ALLOW_HOST)
                {
                    netPostAllow.push_back(std::move(sourceElement));
                    netPostAllowDesc.push_back(sourceConfig.getString(sourceLevel + "[@desc]"));
                }
                else if (commonKeyPart == STORAGE_WOPI_HOST)
                {
                    wopiHost.push_back(std::move(sourceElement));
                    wopiHostDesc.push_back(sourceConfig.getString(sourceLevel + "[@desc]"));
                    wopiHostAllow.push_back(sourceConfig.getString(sourceLevel + "[@allow]"));
                }
            }
            // Generic handling of lists that are shipped empty
            else if (commonKeyPart == ".ssl.hpkp.pins.pin" || commonKeyPart == ".monitors.monitor")
            {
                // Shipped empty, no need to check for existing, append new ones
                // Except there's an empty <pin> entry, let's replace empty entries
                int id = 0;
                while (true)
                {
                    const std::string targetKey(id == 0 ? commonKeyPart : commonKeyPart + "[" + std::to_string(id) + "]");
                    if (!targetConfig.has(targetKey) || targetConfig.getString(targetKey) == "")
                    {
                        break;
                    }
                    id++;
                }
                const std::string targetKey(getTargetKey(id, commonKeyPart));
                std::cout << targetKey << ": added \"" << sourceElement << "\"." << std::endl;

                targetConfig.setString(targetKey, sourceElement);
            }
            else
            {
                std::cerr << "Unhandled key with multiples: " << sourceLevel << std::endl;
            }
            return;
        }

        bool moveOn = false;
        // Don't migrate empty elements
        if (sourceElement == "")
        {
            moveOn = true;
        }
        const std::string targetKey = renamedElems.find(sourceLevel) != renamedElems.end() ? renamedElems.at(sourceLevel) : sourceLevel;
        // Special default and normal default cases
        if (!moveOn && specialDefault.find(sourceLevel) != specialDefault.end())
        {
            if (sourceElement == specialDefault.at(sourceLevel))
            {
                moveOn = true;
            }
        }
        else if (!moveOn)
        {
            const std::string defaultAttr = sourceLevel + "[@default]";
            // Nothing to migrate if the config is the same as the default (if a default exists)
            if (sourceConfig.has(defaultAttr) && (sourceElement == sourceConfig.getString(defaultAttr)))
            {
                moveOn = true;
            }
        }
        // If new config doesn't have the element anymore, disregard
        if (!moveOn && !targetConfig.has(targetKey) && migrateIfNoTarget.find(targetKey) == migrateIfNoTarget.end())
        {
            std::cout << targetKey << " does not exist, and is not relevant anymore." << std::endl;
            moveOn = true;
        }
        // Finally, migrate if the source and target values are different
        if (!moveOn && (!targetConfig.has(targetKey) || sourceElement != targetConfig.getString(targetKey)))
        {
            const std::string targetElement = targetConfig.has(targetKey) ? targetConfig.getString(targetKey) : "";
            // Don't log password
            if (sourceLevel == ".admin_console.password")
                std::cout << targetKey << ": replaced \"" << targetElement << "\" with \"******\"." << std::endl;
            else
                std::cout << targetKey << ": replaced \"" << targetElement << "\" with \"" << sourceElement << "\"." << std::endl;

            targetConfig.setString(targetKey, sourceElement);
            moveOn = true;
        }
    }
    // Handle special attributes, can exist both for leaf nodes and elements with subnodes
    if (specialAttribute.find(sourceLevel) != specialAttribute.end())
    {
        const std::string targetAttrKey =
                (renamedElems.find(sourceLevel) != renamedElems.end() ? renamedElems.at(sourceLevel) : sourceLevel) +
                "[@" + specialAttribute.at(sourceLevel) + "]";
        const std::string sourceAttrKey = sourceLevel+ "[@" + specialAttribute.at(sourceLevel) + "]";
        const std::string sourceAttribute = sourceConfig.getString(sourceAttrKey);
        const std::string targetAttribute = targetConfig.getString(targetAttrKey);
        if (sourceAttribute != targetAttribute)
        {
            std::cout << sourceAttrKey << ": replaced attribute \"" << targetAttribute << "\" with \""<< sourceAttribute << "\"." << std::endl;

            targetConfig.setString(targetAttrKey, sourceAttribute);
        }
    }
}

static void PreProcess()
{
    netPostAllow.clear();
    netPostAllowDesc.clear();
    wopiHost.clear();
    wopiHostDesc.clear();
    wopiHostAllow.clear();
    netPostAllowAdded = false;
    wopiHostAdded = false;
}

static void PostProcess(XMLConfiguration &targetConfig)
{
    assert(netPostAllow.size() == netPostAllowDesc.size());
    assert(wopiHost.size() == wopiHostDesc.size());
    assert(wopiHost.size() == wopiHostAllow.size());
    if (netPostAllowAdded)
    {
        // Clear entries in new config first, then replace with migrated entries
        while (targetConfig.has(NET_POST_ALLOW_HOST))
        {
            targetConfig.remove(NET_POST_ALLOW_HOST);
        }
        for (size_t i(0); i < netPostAllow.size(); ++i)
        {
            const std::string targetKeyRoot(NET_POST_ALLOW_HOST + "[" + std::to_string(i) + "]");
            std::cout << targetKeyRoot << ": added \"" << netPostAllow[i] << "\"." << std::endl;

            targetConfig.setString(targetKeyRoot, netPostAllow[i]);
            targetConfig.setString(targetKeyRoot + "[@desc]", netPostAllowDesc[i]);
        }
    }
    if (wopiHostAdded)
    {
        // Clear entries in new config first, then replace with migrated entries
        while (targetConfig.has(STORAGE_WOPI_HOST))
        {
            targetConfig.remove(STORAGE_WOPI_HOST);
        }
        for (size_t i(0); i < wopiHost.size(); ++i)
        {
            const std::string targetKeyRoot(STORAGE_WOPI_HOST + "[" + std::to_string(i) + "]");
            std::cout << targetKeyRoot << ": added \"" << wopiHost[i] << "\" (allow: " <<
                         wopiHostAllow[i] << ")." << std::endl;

            targetConfig.setString(targetKeyRoot, wopiHost[i]);
            targetConfig.setString(targetKeyRoot + "[@desc]", wopiHostDesc[i]);
            targetConfig.setString(targetKeyRoot + "[@allow]", wopiHostAllow[i]);
        }
    }
}

int MigrateConfig(const std::string& oldConfigFile, const std::string& newConfigFile, bool write)
{
    PreProcess();
    Poco::AutoPtr<XMLConfiguration> oldXMLConfig(new XMLConfiguration(oldConfigFile));
    Poco::AutoPtr<XMLConfiguration> newXMLConfig(new XMLConfiguration(newConfigFile));
    MigrateLevel(*oldXMLConfig, *newXMLConfig, "");
    PostProcess(*newXMLConfig);
    if (write)
        newXMLConfig->save(newConfigFile);
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
