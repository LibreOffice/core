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
#ifndef _DESKTOP_MIGRATION_IMPL_HXX_
#define _DESKTOP_MIGRATION_IMPL_HXX_

#include <vector>
#include <algorithm>
#include <memory>
#include <boost/unordered_map.hpp>

#include "migration.hxx"

#include <sal/types.h>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>

#include <com/sun/star/uno/Reference.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/ui/XUIConfigurationPersistence.hpp>

#define NS_CSS com::sun::star
#define NS_UNO com::sun::star::uno

namespace desktop
{

struct install_info
{
    rtl::OUString productname;  // human readeable product name
    rtl::OUString userdata;     // file: url for user installation
};

typedef std::vector< rtl::OUString > strings_v;
typedef std::auto_ptr< strings_v > strings_vr;

struct migration_step
{
    rtl::OUString name;
    strings_v includeFiles;
    strings_v excludeFiles;
    strings_v includeConfig;
    strings_v excludeConfig;
    strings_v includeExtensions;
    strings_v excludeExtensions;
    rtl::OUString service;
};

struct supported_migration
{
    rtl::OUString name;
    sal_Int32     nPriority;
    strings_v     supported_versions;
};

typedef std::vector< migration_step > migrations_v;
typedef std::auto_ptr< migrations_v > migrations_vr;
typedef std::vector< supported_migration > migrations_available;

//__________________________________________
/**
    define the item, e.g.:menuitem, toolbaritem, to be migrated. we keep the information
    of the command URL, the previous sibling node and the parent node of a item
*/
struct MigrationItem
{
    ::rtl::OUString m_sParentNodeName;
    ::rtl::OUString m_sPrevSibling;
    ::rtl::OUString m_sCommandURL;
    NS_UNO::Reference< NS_CSS::container::XIndexContainer > m_xPopupMenu;

    MigrationItem()
        :m_xPopupMenu(0)
    {
    }

    MigrationItem(const ::rtl::OUString& sParentNodeName,
        const ::rtl::OUString& sPrevSibling,
        const ::rtl::OUString& sCommandURL,
        const NS_UNO::Reference< NS_CSS::container::XIndexContainer > xPopupMenu)
    {
        m_sParentNodeName = sParentNodeName;
        m_sPrevSibling    = sPrevSibling;
        m_sCommandURL     = sCommandURL;
        m_xPopupMenu      = xPopupMenu;
    }

    MigrationItem& operator=(const MigrationItem& aMigrationItem)
    {
        m_sParentNodeName = aMigrationItem.m_sParentNodeName;
        m_sPrevSibling    = aMigrationItem.m_sPrevSibling;
        m_sCommandURL     = aMigrationItem.m_sCommandURL;
        m_xPopupMenu      = aMigrationItem.m_xPopupMenu;

        return *this;
    }

    sal_Bool operator==(const MigrationItem& aMigrationItem)
    {
        return ( aMigrationItem.m_sParentNodeName == m_sParentNodeName &&
            aMigrationItem.m_sPrevSibling    == m_sPrevSibling     &&
            aMigrationItem.m_sCommandURL     == m_sCommandURL      &&
            aMigrationItem.m_xPopupMenu.is() == m_xPopupMenu.is()    );
    }

    ::rtl::OUString GetPrevSibling() const { return m_sPrevSibling; }
};

typedef ::boost::unordered_map< ::rtl::OUString,
                         ::std::vector< MigrationItem >,
                         ::rtl::OUStringHash,
                         ::std::equal_to< ::rtl::OUString > > MigrationHashMap;

struct MigrationItemInfo
{
    ::rtl::OUString m_sResourceURL;
    MigrationItem m_aMigrationItem;

    MigrationItemInfo(){}

    MigrationItemInfo(const ::rtl::OUString& sResourceURL, const MigrationItem& aMigratiionItem)
    {
        m_sResourceURL = sResourceURL;
        m_aMigrationItem = aMigratiionItem;
    }
};

//__________________________________________
/**
    information for the UI elements to be migrated for one module
*/
struct MigrationModuleInfo
{
    ::rtl::OUString sModuleShortName;
    sal_Bool        bHasMenubar;
    ::std::vector< ::rtl::OUString > m_vToolbars;

    MigrationModuleInfo():bHasMenubar(sal_False){};
};

//__________________________________________
/**
    get the information before copying the ui configuration files of old version to new version
*/
class NewVersionUIInfo
{
public:

    NS_UNO::Reference< NS_CSS::ui::XUIConfigurationManager > getConfigManager(const ::rtl::OUString& sModuleShortName) const;
    NS_UNO::Reference< NS_CSS::container::XIndexContainer > getNewMenubarSettings(const ::rtl::OUString& sModuleShortName) const;
    NS_UNO::Reference< NS_CSS::container::XIndexContainer > getNewToolbarSettings(const ::rtl::OUString& sModuleShortName, const ::rtl::OUString& sToolbarName) const;
    void init(const ::std::vector< MigrationModuleInfo >& vModulesInfo);

private:

    NS_UNO::Sequence< NS_CSS::beans::PropertyValue > m_lCfgManagerSeq;
    NS_UNO::Sequence< NS_CSS::beans::PropertyValue > m_lNewVersionMenubarSettingsSeq;
    NS_UNO::Sequence< NS_CSS::beans::PropertyValue > m_lNewVersionToolbarSettingsSeq;
};

class MigrationImpl
{

private:
    strings_vr m_vrVersions;
    NS_UNO::Reference< NS_CSS::lang::XMultiServiceFactory > m_xFactory;

    migrations_available m_vMigrationsAvailable; // list of all available migrations
    migrations_vr        m_vrMigrations;         // list of all migration specs from config
    install_info         m_aInfo;                // info about the version being migrated
    strings_vr           m_vrFileList;           // final list of files to be copied
     MigrationHashMap     m_aOldVersionItemsHashMap;
     MigrationHashMap     m_aNewVersionItemsHashMap;
     ::rtl::OUString      m_sModuleIdentifier;

    // functions to control the migration process
    bool          readAvailableMigrations(migrations_available&);
    bool          alreadyMigrated();
    migrations_vr readMigrationSteps(const ::rtl::OUString& rMigrationName);
    sal_Int32     findPreferedMigrationProcess(const migrations_available&);
    install_info  findInstallation(const strings_v& rVersions);
    strings_vr    compileFileList();

    // helpers
    void subtract(strings_v& va, const strings_v& vb_c) const;
    strings_vr getAllFiles(const rtl::OUString& baseURL) const;
    strings_vr applyPatterns(const strings_v& vSet, const strings_v& vPatterns) const;
    NS_UNO::Reference< NS_CSS::container::XNameAccess > getConfigAccess(const sal_Char* path, sal_Bool rw=sal_False);

    ::std::vector< MigrationModuleInfo > dectectUIChangesForAllModules() const;
    void compareOldAndNewConfig(const ::rtl::OUString& sParentNodeName,
        const NS_UNO::Reference< NS_CSS::container::XIndexContainer >& xOldIndexContainer,
        const NS_UNO::Reference< NS_CSS::container::XIndexContainer >& xNewIndexContainer,
        const ::rtl::OUString& sToolbarName);
    void mergeOldToNewVersion(const NS_UNO::Reference< NS_CSS::ui::XUIConfigurationManager >& xCfgManager,
        const NS_UNO::Reference< NS_CSS::container::XIndexContainer>& xIndexContainer,
        const ::rtl::OUString& sModuleIdentifier,
        const ::rtl::OUString& sResourceURL);

    // actual processing function that perform the migration steps
    void copyFiles();
    void copyConfig();
    void runServices();
    void refresh();

    void setMigrationCompleted();
    bool checkMigrationCompleted();

public:
    MigrationImpl(const NS_UNO::Reference< NS_CSS::lang::XMultiServiceFactory >&);
    ~MigrationImpl();
    bool initializeMigration();
    sal_Bool doMigration();
    rtl::OUString getOldVersionName();
};
}
#undef NS_CSS
#undef NS_UNO

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
