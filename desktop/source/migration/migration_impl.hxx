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
#ifndef INCLUDED_DESKTOP_SOURCE_MIGRATION_MIGRATION_IMPL_HXX
#define INCLUDED_DESKTOP_SOURCE_MIGRATION_MIGRATION_IMPL_HXX

#include <algorithm>
#include <memory>
#include <unordered_map>
#include <vector>

#include <migration.hxx>

#include <sal/types.h>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>

#include <com/sun/star/uno/Reference.hxx>

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/ui/XUIConfigurationPersistence.hpp>

namespace desktop
{

struct install_info
{
    OUString productname;  // human readable product name
    OUString userdata;     // file: url for user installation
};

typedef std::vector< OUString > strings_v;
typedef std::unique_ptr< strings_v > strings_vr;

struct migration_step
{
    strings_v includeFiles;
    strings_v excludeFiles;
    strings_v includeConfig;
    strings_v excludeConfig;
    strings_v excludeExtensions;
    OUString service;
};

struct supported_migration
{
    OUString name;
    sal_Int32     nPriority;
    strings_v     supported_versions;
};

typedef std::vector< migration_step > migrations_v;
typedef std::unique_ptr< migrations_v > migrations_vr;
typedef std::vector< supported_migration > migrations_available;

inline bool areBothOpenFrom(OUString const & cmd1, OUString const & cmd2)
{
    return cmd1 == ".uno:Open" && cmd2.startsWith(".uno:OpenFrom");
}

/**
    define the item, e.g.:menuitem, toolbaritem, to be migrated. we keep the information
    of the command URL, the previous sibling node and the parent node of a item
*/
struct MigrationItem
{
    OUString m_sParentNodeName;
    OUString m_sPrevSibling;
    OUString m_sCommandURL;
    css::uno::Reference< css::container::XIndexContainer > m_xPopupMenu;

    MigrationItem()
    {
    }

    MigrationItem(const OUString& sParentNodeName,
        const OUString& sPrevSibling,
        const OUString& sCommandURL,
        const css::uno::Reference< css::container::XIndexContainer > & xPopupMenu)
    {
        m_sParentNodeName = sParentNodeName;
        m_sPrevSibling    = sPrevSibling;
        m_sCommandURL     = sCommandURL;
        m_xPopupMenu      = xPopupMenu;
    }

    bool operator==(const MigrationItem& aMigrationItem)
    {
        return
            (aMigrationItem.m_sCommandURL == m_sCommandURL
             || areBothOpenFrom(aMigrationItem.m_sCommandURL, m_sCommandURL)
             || areBothOpenFrom(m_sCommandURL, aMigrationItem.m_sCommandURL))
            && aMigrationItem.m_sParentNodeName == m_sParentNodeName
            && aMigrationItem.m_sPrevSibling    == m_sPrevSibling
            && aMigrationItem.m_xPopupMenu.is() == m_xPopupMenu.is();
    }
};

typedef std::unordered_map< OUString, std::vector< MigrationItem > > MigrationHashMap;

/**
    information for the UI elements to be migrated for one module
*/
struct MigrationModuleInfo
{
    OUString sModuleShortName;
    bool     bHasMenubar;
    std::vector< OUString > m_vToolbars;

    MigrationModuleInfo() : bHasMenubar(false) {};
};


/**
    get the information before copying the ui configuration files of old version to new version
*/
class NewVersionUIInfo
{
public:

    css::uno::Reference< css::ui::XUIConfigurationManager > getConfigManager(const OUString& sModuleShortName) const;
    css::uno::Reference< css::container::XIndexContainer > getNewMenubarSettings(const OUString& sModuleShortName) const;
    css::uno::Reference< css::container::XIndexContainer > getNewToolbarSettings(const OUString& sModuleShortName, const OUString& sToolbarName) const;
    void init(const std::vector< MigrationModuleInfo >& vModulesInfo);

private:

    std::vector< css::beans::PropertyValue > m_lCfgManagerSeq;
    css::uno::Sequence< css::beans::PropertyValue > m_lNewVersionMenubarSettingsSeq;
    css::uno::Sequence< css::beans::PropertyValue > m_lNewVersionToolbarSettingsSeq;
};

class MigrationImpl
{

private:
    migrations_available m_vMigrationsAvailable; // list of all available migrations
    migrations_vr        m_vrMigrations;         // list of all migration specs from config
    install_info         m_aInfo;                // info about the version being migrated
    strings_vr           m_vrFileList;           // final list of files to be copied
     MigrationHashMap     m_aOldVersionItemsHashMap;

    // functions to control the migration process
    static void   readAvailableMigrations(migrations_available&);
    bool          alreadyMigrated();
    static migrations_vr readMigrationSteps(const OUString& rMigrationName);
    sal_Int32     findPreferredMigrationProcess(const migrations_available&);
#if defined UNX && ! defined MACOSX
    static OUString preXDGConfigDir(const OUString& rConfigDir);
#endif
    static void   setInstallInfoIfExist(install_info& aInfo,  const OUString& rConfigDir, const OUString& rVersion);
    static install_info  findInstallation(const strings_v& rVersions);
    strings_vr    compileFileList();

    // helpers
    strings_vr getAllFiles(const OUString& baseURL) const;
    static strings_vr applyPatterns(const strings_v& vSet, const strings_v& vPatterns);
    static css::uno::Reference< css::container::XNameAccess > getConfigAccess(const sal_Char* path, bool rw=false);

    std::vector< MigrationModuleInfo > dectectUIChangesForAllModules() const;
    void compareOldAndNewConfig(const OUString& sParentNodeName,
        const css::uno::Reference< css::container::XIndexContainer >& xOldIndexContainer,
        const css::uno::Reference< css::container::XIndexContainer >& xNewIndexContainer,
        const OUString& sToolbarName);
    void mergeOldToNewVersion(const css::uno::Reference< css::ui::XUIConfigurationManager >& xCfgManager,
        const css::uno::Reference< css::container::XIndexContainer>& xIndexContainer,
        const OUString& sModuleIdentifier,
        const OUString& sResourceURL);

    // actual processing function that perform the migration steps
    void copyFiles();
    void copyConfig();
    void runServices();

    static void setMigrationCompleted();
    static bool checkMigrationCompleted();

public:
    MigrationImpl();
    ~MigrationImpl();
    bool initializeMigration();
    bool doMigration();
};
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
