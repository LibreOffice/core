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

#include <algorithm>
#include <iterator>
#include <map>
#include <new>
#include <set>

#include "migration.hxx"
#include "migration_impl.hxx"

#include <sal/log.hxx>
#include <unotools/textsearch.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <unotools/bootstrap.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/uri.hxx>
#include <i18nlangtag/lang.h>
#include <tools/urlobj.hxx>
#include <osl/file.hxx>
#include <osl/mutex.hxx>
#include <osl/security.hxx>
#include <unotools/configmgr.hxx>

#include <com/sun/star/configuration/Update.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/util/XStringSubstitution.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/FileSystemStorageFactory.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/ui/theModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/frame/theUICommandDescription.hpp>
#include <com/sun/star/ui/UIConfigurationManager.hpp>
#include <com/sun/star/ui/XUIConfiguration.hpp>
#include <com/sun/star/ui/XUIConfigurationStorage.hpp>
#include <com/sun/star/ui/XUIConfigurationPersistence.hpp>

using namespace osl;
using namespace std;
using namespace com::sun::star::task;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace com::sun::star::container;
using com::sun::star::uno::Exception;
using namespace com::sun::star;


namespace desktop {

static const char ITEM_DESCRIPTOR_COMMANDURL[] = "CommandURL";
static const char ITEM_DESCRIPTOR_CONTAINER[] = "ItemDescriptorContainer";
static const char ITEM_DESCRIPTOR_LABEL[] = "Label";

OUString retrieveLabelFromCommand(const OUString& sCommand, const OUString& sModuleIdentifier)
{
    OUString sLabel;

    uno::Reference< container::XNameAccess > xUICommands;
    uno::Reference< container::XNameAccess > const xNameAccess(
            frame::theUICommandDescription::get(
                ::comphelper::getProcessComponentContext()) );
    xNameAccess->getByName( sModuleIdentifier ) >>= xUICommands;
    if (xUICommands.is())
    {
        if ( !sCommand.isEmpty() )
        {
            OUString aStr;
            ::uno::Sequence< beans::PropertyValue > aPropSeq;
            try
            {
                uno::Any a( xUICommands->getByName( sCommand ));
                if ( a >>= aPropSeq )
                {
                    for ( sal_Int32 i = 0; i < aPropSeq.getLength(); i++ )
                    {
                        if ( aPropSeq[i].Name == "Label" )
                        {
                            aPropSeq[i].Value >>= aStr;
                            break;
                        }
                    }
                }

                sLabel = aStr;
            }
            catch (const container::NoSuchElementException&)
            {
                sLabel = sCommand;
                sal_Int32 nIndex = sLabel.indexOf(':');
                if (nIndex>=0 && nIndex <= sLabel.getLength()-1)
                    sLabel = sLabel.copy(nIndex+1);
            }

        }
    }

    return sLabel;
}

OUString mapModuleShortNameToIdentifier(const OUString& sShortName)
{
    OUString sIdentifier;

    if ( sShortName == "StartModule" )
        sIdentifier = "com.sun.star.frame.StartModule";

    else if ( sShortName == "swriter" )
        sIdentifier = "com.sun.star.text.TextDocument";

    else if ( sShortName == "scalc" )
        sIdentifier = "com.sun.star.sheet.SpreadsheetDocument";

    else if ( sShortName == "sdraw" )
        sIdentifier = "com.sun.star.drawing.DrawingDocument";

    else if ( sShortName == "simpress" )
        sIdentifier = "com.sun.star.presentation.PresentationDocument";

    else if ( sShortName == "smath" )
        sIdentifier = "com.sun.star.formula.FormulaProperties";

    else if ( sShortName == "schart" )
        sIdentifier = "com.sun.star.chart2.ChartDocument";

    else if ( sShortName == "BasicIDE" )
        sIdentifier = "com.sun.star.script.BasicIDE";

    else if ( sShortName == "dbapp" )
        sIdentifier = "com.sun.star.sdb.OfficeDatabaseDocument";

    else if ( sShortName == "sglobal" )
        sIdentifier = "com.sun.star.text.GlobalDocument";

    else if ( sShortName == "sweb" )
        sIdentifier = "com.sun.star.text.WebDocument";

    else if ( sShortName == "swxform" )
        sIdentifier = "com.sun.star.xforms.XMLFormDocument";

    else if ( sShortName == "sbibliography" )
        sIdentifier = "com.sun.star.frame.Bibliography";

    return sIdentifier;
}

bool MigrationImpl::alreadyMigrated()
{
    OUString MIGRATION_STAMP_NAME("/MIGRATED4");
    OUString aStr = m_aInfo.userdata + MIGRATION_STAMP_NAME;
    File aFile(aStr);
    // create migration stamp, and/or check its existence
    bool bRet = aFile.open (osl_File_OpenFlag_Write | osl_File_OpenFlag_Create | osl_File_OpenFlag_NoLock) == FileBase::E_EXIST;
    OSL_TRACE( "File '%s' exists? %d\n",
             OUStringToOString(aStr, RTL_TEXTENCODING_ASCII_US).getStr(),
             bRet );
    return bRet;
}

bool MigrationImpl::initializeMigration()
{
    bool bRet = false;

    if (!checkMigrationCompleted()) {
        readAvailableMigrations(m_vMigrationsAvailable);
        sal_Int32 nIndex = findPreferredMigrationProcess(m_vMigrationsAvailable);
        // m_aInfo is now set to the preferred migration source
        if ( nIndex >= 0 ) {
            if (alreadyMigrated())
                return false;
            m_vrMigrations = readMigrationSteps(m_vMigrationsAvailable[nIndex].name);
        }

        bRet = !m_aInfo.userdata.isEmpty();
    }

    OSL_TRACE( "Migration %s", bRet ? "needed" : "not required" );

    return bRet;
}

void Migration::migrateSettingsIfNecessary()
{
    MigrationImpl aImpl;

    if (! aImpl.initializeMigration() )
        return;

    bool bResult = false;
    try
    {
        bResult = aImpl.doMigration();
    }
    catch (const Exception& e)
    {
        OString aMsg("doMigration() exception: ");
        aMsg += OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US);
        OSL_FAIL(aMsg.getStr());
    }
    OSL_ENSURE(bResult, "Migration has not been successful");
    (void)bResult;
}

MigrationImpl::MigrationImpl()
    : m_vrVersions(new strings_v)
{
}

MigrationImpl::~MigrationImpl()
{
}

// The main entry point for migrating settings
bool MigrationImpl::doMigration()
{
    // compile file list for migration
    m_vrFileList = compileFileList();

    bool result = false;
    try
    {
        NewVersionUIInfo aNewVersionUIInfo;
        ::std::vector< MigrationModuleInfo > vModulesInfo = dectectUIChangesForAllModules();
        aNewVersionUIInfo.init(vModulesInfo);

        copyFiles();

        const OUString sMenubarResourceURL("private:resource/menubar/menubar");
        const OUString sToolbarResourcePre("private:resource/toolbar/");
        for (size_t i=0; i<vModulesInfo.size(); ++i)
        {
            OUString sModuleIdentifier = mapModuleShortNameToIdentifier(vModulesInfo[i].sModuleShortName);
            if (sModuleIdentifier.isEmpty())
                continue;

            uno::Sequence< uno::Any > lArgs(2);
            OUString aOldCfgDataPath = m_aInfo.userdata + "/user/config/soffice.cfg/modules/";
            lArgs[0] <<= aOldCfgDataPath + vModulesInfo[i].sModuleShortName;
            lArgs[1] <<= embed::ElementModes::READ;

            uno::Reference< uno::XComponentContext > xContext(comphelper::getProcessComponentContext());
            uno::Reference< lang::XSingleServiceFactory > xStorageFactory(embed::FileSystemStorageFactory::create(xContext));
            uno::Reference< embed::XStorage >             xModules(xStorageFactory->createInstanceWithArguments(lArgs), uno::UNO_QUERY);
            uno::Reference< ui::XUIConfigurationManager2 > xOldCfgManager = ui::UIConfigurationManager::create(xContext);

            if ( xModules.is() )
            {
                    xOldCfgManager->setStorage( xModules );
                    xOldCfgManager->reload();
            }

            uno::Reference< ui::XUIConfigurationManager > xCfgManager = aNewVersionUIInfo.getConfigManager(vModulesInfo[i].sModuleShortName);

            if (vModulesInfo[i].bHasMenubar)
            {
                uno::Reference< container::XIndexContainer > xOldVersionMenuSettings = uno::Reference< container::XIndexContainer >(xOldCfgManager->getSettings(sMenubarResourceURL, sal_True), uno::UNO_QUERY);
                uno::Reference< container::XIndexContainer > xNewVersionMenuSettings = aNewVersionUIInfo.getNewMenubarSettings(vModulesInfo[i].sModuleShortName);
                OUString sParent;
                compareOldAndNewConfig(sParent, xOldVersionMenuSettings, xNewVersionMenuSettings, sMenubarResourceURL);
                mergeOldToNewVersion(xCfgManager, xNewVersionMenuSettings, sModuleIdentifier, sMenubarResourceURL);
            }

            sal_Int32 nToolbars = vModulesInfo[i].m_vToolbars.size();
            if (nToolbars >0)
            {
                for (sal_Int32 j=0; j<nToolbars; ++j)
                {
                    OUString sToolbarName = vModulesInfo[i].m_vToolbars[j];
                    OUString sToolbarResourceURL = sToolbarResourcePre + sToolbarName;

                    uno::Reference< container::XIndexContainer > xOldVersionToolbarSettings = uno::Reference< container::XIndexContainer >(xOldCfgManager->getSettings(sToolbarResourceURL, sal_True), uno::UNO_QUERY);
                    uno::Reference< container::XIndexContainer > xNewVersionToolbarSettings = aNewVersionUIInfo.getNewToolbarSettings(vModulesInfo[i].sModuleShortName, sToolbarName);
                    OUString sParent;
                    compareOldAndNewConfig(sParent, xOldVersionToolbarSettings, xNewVersionToolbarSettings, sToolbarResourceURL);
                    mergeOldToNewVersion(xCfgManager, xNewVersionToolbarSettings, sModuleIdentifier, sToolbarResourceURL);
                }
            }

            m_aOldVersionItemsHashMap.clear();
        }

        // execute the migration items from Setup.xcu
        copyConfig();

        // execute custom migration services from Setup.xcu
        // and refresh the cache
        runServices();
        refresh();

        result = true;
    }
    catch (css::uno::Exception & e)
    {
        SAL_WARN(
            "desktop.migration",
            "ignored Exception \"" << e.Message
                << "\" while migrating from version \"" << m_aInfo.productname
                << "\" data \"" << m_aInfo.userdata << "\"");
    }

    // prevent running the migration multiple times
    setMigrationCompleted();
    return result;
}

void MigrationImpl::refresh()
{
    uno::Reference< XRefreshable >(
        configuration::theDefaultProvider::get(comphelper::getProcessComponentContext()),
        uno::UNO_QUERY_THROW)->refresh();
}

void MigrationImpl::setMigrationCompleted()
{
    try
    {
        uno::Reference< XPropertySet > aPropertySet(getConfigAccess("org.openoffice.Setup/Office", true), uno::UNO_QUERY_THROW);
        aPropertySet->setPropertyValue("MigrationCompleted", uno::makeAny(sal_True));
        uno::Reference< XChangesBatch >(aPropertySet, uno::UNO_QUERY_THROW)->commitChanges();
    }
    catch (...)
    {
        // fail silently
    }
}

bool MigrationImpl::checkMigrationCompleted()
{
    bool bMigrationCompleted = false;
    try {
        uno::Reference< XPropertySet > aPropertySet(
            getConfigAccess("org.openoffice.Setup/Office"), uno::UNO_QUERY_THROW);
        aPropertySet->getPropertyValue("MigrationCompleted") >>= bMigrationCompleted;

        if( !bMigrationCompleted && getenv("SAL_DISABLE_USERMIGRATION" ) )
        {
            // migration prevented - fake it's success
            setMigrationCompleted();
            bMigrationCompleted = true;
        }
    }
    catch (const Exception&)
    {
        // just return false...
    }
    OSL_TRACE( "Migration %s", bMigrationCompleted ? "already completed" : "not done" );

    return bMigrationCompleted;
}

static void insertSorted(migrations_available& rAvailableMigrations, supported_migration& aSupportedMigration)
{
    bool                           bInserted( false );
    migrations_available::iterator pIter = rAvailableMigrations.begin();
    while ( !bInserted && pIter != rAvailableMigrations.end())
    {
        if ( pIter->nPriority < aSupportedMigration.nPriority )
        {
            rAvailableMigrations.insert(pIter, aSupportedMigration );
            bInserted = true;
            break; // i111193: insert invalidates iterator!
        }
        ++pIter;
    }
    if ( !bInserted )
        rAvailableMigrations.push_back( aSupportedMigration );
}

bool MigrationImpl::readAvailableMigrations(migrations_available& rAvailableMigrations)
{
    // get supported version names
    uno::Reference< XNameAccess > aMigrationAccess(getConfigAccess("org.openoffice.Setup/Migration/SupportedVersions"), uno::UNO_QUERY_THROW);
    uno::Sequence< OUString > seqSupportedVersions = aMigrationAccess->getElementNames();

    const OUString aVersionIdentifiers( "VersionIdentifiers" );
    const OUString aPriorityIdentifier( "Priority" );

    for (sal_Int32 i=0; i<seqSupportedVersions.getLength(); i++)
    {
        sal_Int32                 nPriority( 0 );
        uno::Sequence< OUString > seqVersions;
        uno::Reference< XNameAccess > xMigrationData( aMigrationAccess->getByName(seqSupportedVersions[i]), uno::UNO_QUERY_THROW );
        xMigrationData->getByName( aVersionIdentifiers ) >>= seqVersions;
        xMigrationData->getByName( aPriorityIdentifier ) >>= nPriority;

        supported_migration aSupportedMigration;
        aSupportedMigration.name      = seqSupportedVersions[i];
        aSupportedMigration.nPriority = nPriority;
        for (sal_Int32 j=0; j<seqVersions.getLength(); j++)
            aSupportedMigration.supported_versions.push_back(seqVersions[j].trim());
        insertSorted( rAvailableMigrations, aSupportedMigration );
        OSL_TRACE( " available migration '%s'\n",
                   OUStringToOString( aSupportedMigration.name, RTL_TEXTENCODING_ASCII_US ).getStr() );
    }

    return true;
}

migrations_vr MigrationImpl::readMigrationSteps(const OUString& rMigrationName)
{
    // get migration access
    uno::Reference< XNameAccess > aMigrationAccess(getConfigAccess("org.openoffice.Setup/Migration/SupportedVersions"), uno::UNO_QUERY_THROW);
    uno::Reference< XNameAccess > xMigrationData( aMigrationAccess->getByName(rMigrationName), uno::UNO_QUERY_THROW );

    // get migration description from org.openoffice.Setup/Migration
    // and build vector of migration steps
    OUString aMigrationSteps( "MigrationSteps" );
    uno::Reference< XNameAccess > theNameAccess(xMigrationData->getByName(aMigrationSteps), uno::UNO_QUERY_THROW);
    uno::Sequence< OUString > seqMigrations = theNameAccess->getElementNames();
    uno::Reference< XNameAccess > tmpAccess;
    uno::Reference< XNameAccess > tmpAccess2;
    uno::Sequence< OUString > tmpSeq;
    migrations_vr vrMigrations(new migrations_v);
    for (sal_Int32 i = 0; i < seqMigrations.getLength(); i++)
    {
        // get current migration step
        theNameAccess->getByName(seqMigrations[i]) >>= tmpAccess;
        migration_step tmpStep;
        tmpStep.name = seqMigrations[i];

        // read included files from current step description
        OUString aSeqEntry;
        if (tmpAccess->getByName("IncludedFiles") >>= tmpSeq)
        {
            for (sal_Int32 j=0; j<tmpSeq.getLength(); j++)
            {
                aSeqEntry = tmpSeq[j];
                tmpStep.includeFiles.push_back(aSeqEntry);
            }
        }

        // excluded files...
        if (tmpAccess->getByName("ExcludedFiles") >>= tmpSeq)
        {
            for (sal_Int32 j=0; j<tmpSeq.getLength(); j++)
                tmpStep.excludeFiles.push_back(tmpSeq[j]);
        }

        // included nodes...
        if (tmpAccess->getByName("IncludedNodes") >>= tmpSeq)
        {
            for (sal_Int32 j=0; j<tmpSeq.getLength(); j++)
                tmpStep.includeConfig.push_back(tmpSeq[j]);
        }

        // excluded nodes...
        if (tmpAccess->getByName("ExcludedNodes") >>= tmpSeq)
        {
            for (sal_Int32 j=0; j<tmpSeq.getLength(); j++)
                tmpStep.excludeConfig.push_back(tmpSeq[j]);
        }

        // included extensions...
        if (tmpAccess->getByName("IncludedExtensions") >>= tmpSeq)
        {
            for (sal_Int32 j=0; j<tmpSeq.getLength(); j++)
                tmpStep.includeExtensions.push_back(tmpSeq[j]);
        }

        // excluded extensions...
        if (tmpAccess->getByName("ExcludedExtensions") >>= tmpSeq)
        {
            for (sal_Int32 j=0; j<tmpSeq.getLength(); j++)
            {
                aSeqEntry = tmpSeq[j];
                tmpStep.excludeExtensions.push_back(aSeqEntry);
            }
        }

        // generic service
        tmpAccess->getByName("MigrationService") >>= tmpStep.service;

        vrMigrations->push_back(tmpStep);
    }
    return vrMigrations;
}

static FileBase::RC _checkAndCreateDirectory(INetURLObject& dirURL)
{
    FileBase::RC result = Directory::create(dirURL.GetMainURL(INetURLObject::DECODE_TO_IURI));
    if (result == FileBase::E_NOENT)
    {
        INetURLObject baseURL(dirURL);
        baseURL.removeSegment();
        _checkAndCreateDirectory(baseURL);
        return Directory::create(dirURL.GetMainURL(INetURLObject::DECODE_TO_IURI));
    } else
        return result;
}

#if defined UNX && ! defined MACOSX

static const char XDG_CONFIG_PART[] = "/.config/";

OUString MigrationImpl::preXDGConfigDir(const OUString& rConfigDir)
{
    OUString aPreXDGConfigPath;
    const char* pXDGCfgHome = getenv("XDG_CONFIG_HOME");

    // cater for XDG_CONFIG_HOME change
    // If XDG_CONFIG_HOME is set then we;
    // assume the user knows what they are doing ( room for improvement here, we could
    // of course search the default config dir etc. also  - but this is more complex,
    // we would need to weigh results from the current config dir against matches in
    // the 'old' config dir etc. ) - currently we just use the returned config dir.
    // If XDG_CONFIG_HOME is NOT set;
    // assume then we should now using the default $HOME/,config config location for
    // our user profiles, however *all* previous libreoffice and openoffice.org
    // configurations will be in the 'old' config directory and that's where we need
    // to search - we convert the returned config dir to the 'old' dir
    if ( !pXDGCfgHome && rConfigDir.endsWith( XDG_CONFIG_PART )  )
        // remove trailing '.config/' but leave the terminating '/'
        aPreXDGConfigPath = rConfigDir.copy( 0, rConfigDir.getLength() - sizeof(  XDG_CONFIG_PART ) + 2 );
    else
        aPreXDGConfigPath = rConfigDir;

    // the application-specific config dir is not longer prefixed by '.' because it is hidden under ".config"
    // we have to add the '.' for the pre-XDG directory names
    aPreXDGConfigPath += ".";

   return aPreXDGConfigPath;
}
#endif

void MigrationImpl::setInstallInfoIfExist(
    install_info& aInfo,
    const OUString& rConfigDir,
    const OUString& rVersion)
{
    OUString url(INetURLObject(rConfigDir).GetMainURL(INetURLObject::NO_DECODE));
    osl::DirectoryItem item;
    osl::FileStatus stat(osl_FileStatus_Mask_Type);

    if (osl::DirectoryItem::get(url, item) == osl::FileBase::E_None
            && item.getFileStatus(stat) == osl::FileBase::E_None
            && stat.getFileType() == osl::FileStatus::Directory)
    {
        aInfo.userdata = url;
        aInfo.productname = rVersion;
    }
}

install_info MigrationImpl::findInstallation(const strings_v& rVersions)
{

    OUString aTopConfigDir;
    osl::Security().getConfigDir( aTopConfigDir );
    if ( !aTopConfigDir.isEmpty() && aTopConfigDir[ aTopConfigDir.getLength()-1 ] != '/' )
        aTopConfigDir += "/";

#if defined UNX && ! defined MACOSX
    OUString aPreXDGTopConfigDir = preXDGConfigDir(aTopConfigDir);
#endif

    install_info aInfo;
    strings_v::const_iterator i_ver = rVersions.begin();
    while (i_ver != rVersions.end())
    {
        OUString aVersion, aProfileName;
        sal_Int32 nSeparatorIndex = (*i_ver).indexOf('=');
        if ( nSeparatorIndex != -1 )
        {
            aVersion = (*i_ver).copy( 0, nSeparatorIndex );
            aProfileName = (*i_ver).copy( nSeparatorIndex+1 );
        }

        if ( !aVersion.isEmpty() && !aProfileName.isEmpty() &&
             ( aInfo.userdata.isEmpty() ||
               aProfileName.equalsIgnoreAsciiCase(
                   utl::ConfigManager::getProductName() ) ) )
        {
            setInstallInfoIfExist(aInfo, aTopConfigDir + aProfileName, aVersion);
#if defined UNX && ! defined MACOSX
            //try preXDG path if the new one does not exist
            if ( aInfo.userdata.isEmpty())
                setInstallInfoIfExist(aInfo, aPreXDGTopConfigDir + aProfileName, aVersion);
#endif
        }
        ++i_ver;
    }

    return aInfo;
}

sal_Int32 MigrationImpl::findPreferredMigrationProcess(const migrations_available& rAvailableMigrations)
{
    sal_Int32    nIndex( -1 );
    sal_Int32    i( 0 );

    migrations_available::const_iterator rIter = rAvailableMigrations.begin();
    while ( rIter != rAvailableMigrations.end() )
    {
        install_info aInstallInfo = findInstallation(rIter->supported_versions);
        if (!aInstallInfo.productname.isEmpty() )
        {
            m_aInfo = aInstallInfo;
            nIndex  = i;
            break;
        }
        ++i;
        ++rIter;
    }

    OSL_TRACE( " preferred migration is from product '%s'\n",
               OUStringToOString( m_aInfo.productname, RTL_TEXTENCODING_ASCII_US ).getStr() );
    OSL_TRACE( " and settings directory '%s'\n",
               OUStringToOString( m_aInfo.userdata, RTL_TEXTENCODING_ASCII_US ).getStr() );

    return nIndex;
}

strings_vr MigrationImpl::applyPatterns(const strings_v& vSet, const strings_v& vPatterns)
{
    using namespace utl;
    strings_vr vrResult(new strings_v);
    strings_v::const_iterator i_set;
    strings_v::const_iterator i_pat = vPatterns.begin();
    while (i_pat != vPatterns.end())
    {
        // find matches for this pattern in input set
        // and copy them to the result
        SearchParam param(*i_pat, SearchParam::SRCH_REGEXP);
        TextSearch ts(param, LANGUAGE_DONTKNOW);
        i_set = vSet.begin();
        sal_Int32 start = 0;
        sal_Int32 end = 0;
        while (i_set != vSet.end())
        {
            end = i_set->getLength();
            if (ts.SearchForward(*i_set, &start, &end))
                vrResult->push_back(*i_set);
            ++i_set;
        }
        ++i_pat;
    }
    return vrResult;
}

strings_vr MigrationImpl::getAllFiles(const OUString& baseURL) const
{
    using namespace osl;
    strings_vr vrResult(new strings_v);

    // get sub dirs
    Directory dir(baseURL);
    if (dir.open() == FileBase::E_None)
    {
        strings_v vSubDirs;
        strings_vr vrSubResult;

        // work through directory contents...
        DirectoryItem item;
        FileStatus fs(osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileURL);
        while (dir.getNextItem(item) == FileBase::E_None)
        {
            if (item.getFileStatus(fs) == FileBase::E_None)
            {
                if (fs.getFileType() == FileStatus::Directory)
                    vSubDirs.push_back(fs.getFileURL());
                else
                    vrResult->push_back(fs.getFileURL());
            }
        }

        // recurse subfolders
        strings_v::const_iterator i = vSubDirs.begin();
        while (i != vSubDirs.end())
        {
            vrSubResult = getAllFiles(*i);
            vrResult->insert(vrResult->end(), vrSubResult->begin(), vrSubResult->end());
            ++i;
        }
    }
    return vrResult;
}

namespace {

// removes elements of vector 2 in vector 1
strings_v subtract(strings_v const & va, strings_v const & vb) {
    strings_v a(va);
    std::sort(a.begin(), a.end());
    strings_v::iterator ae(std::unique(a.begin(), a.end()));
    strings_v b(vb);
    std::sort(b.begin(), b.end());
    strings_v::iterator be(std::unique(b.begin(), b.end()));
    strings_v c;
    std::set_difference(a.begin(), ae, b.begin(), be, std::back_inserter(c));
    return c;
}

}

strings_vr MigrationImpl::compileFileList()
{

    strings_vr vrResult(new strings_v);
    strings_vr vrInclude;
    strings_vr vrExclude;

    // get a list of all files:
    strings_vr vrFiles = getAllFiles(m_aInfo.userdata);

    // get a file list result for each migration step
    migrations_v::const_iterator i_migr = m_vrMigrations->begin();
    while (i_migr != m_vrMigrations->end())
    {
        vrInclude = applyPatterns(*vrFiles, i_migr->includeFiles);
        vrExclude = applyPatterns(*vrFiles, i_migr->excludeFiles);
        strings_v sub(subtract(*vrInclude, *vrExclude));
        vrResult->insert(vrResult->end(), sub.begin(), sub.end());
        ++i_migr;
    }
    return vrResult;
}

namespace {

struct componentParts {
    std::set< OUString > includedPaths;
    std::set< OUString > excludedPaths;
};

typedef std::map< OUString, componentParts > Components;

bool getComponent(OUString const & path, OUString * component) {
    OSL_ASSERT(component != 0);
    if (path.isEmpty() || path[0] != '/') {
        OSL_TRACE(
            ("configuration migration in/exclude path %s ignored (does not"
             " start with slash)"),
            OUStringToOString(path, RTL_TEXTENCODING_UTF8).getStr());
        return false;
    }
    sal_Int32 i = path.indexOf('/', 1);
    *component = i < 0 ? path.copy(1) : path.copy(1, i - 1);
    return true;
}

uno::Sequence< OUString > setToSeq(std::set< OUString > const & set) {
    std::set< OUString >::size_type n = set.size();
    if (n > SAL_MAX_INT32) {
        throw std::bad_alloc();
    }
    uno::Sequence< OUString > seq(static_cast< sal_Int32 >(n));
    sal_Int32 i = 0;
    for (std::set< OUString >::const_iterator j(set.begin());
         j != set.end(); ++j)
    {
        seq[i++] = *j;
    }
    return seq;
}

}

void MigrationImpl::copyConfig() {
    Components comps;
    for (migrations_v::const_iterator i(m_vrMigrations->begin());
         i != m_vrMigrations->end(); ++i)
    {
        for (strings_v::const_iterator j(i->includeConfig.begin());
             j != i->includeConfig.end(); ++j)
        {
            OUString comp;
            if (getComponent(*j, &comp)) {
                comps[comp].includedPaths.insert(*j);
            }
        }
        for (strings_v::const_iterator j(i->excludeConfig.begin());
             j != i->excludeConfig.end(); ++j)
        {
            OUString comp;
            if (getComponent(*j, &comp)) {
                comps[comp].excludedPaths.insert(*j);
            }
        }
    }

    // check if the shared registrymodifications.xcu file exists
    bool bRegistryModificationsXcuExists = false;
    OUString regFilePath(m_aInfo.userdata);
    regFilePath += "/user/registrymodifications.xcu";
    File regFile(regFilePath);
    ::osl::FileBase::RC nError = regFile.open(osl_File_OpenFlag_Read);
    if ( nError == ::osl::FileBase::E_None ) {
        bRegistryModificationsXcuExists = true;
        regFile.close();
    }

    for (Components::const_iterator i(comps.begin()); i != comps.end(); ++i) {
        if (!i->second.includedPaths.empty()) {
            if (!bRegistryModificationsXcuExists) {
                // shared registrymodifications.xcu does not exists
                // the configuration is split in many registry files
                // determine the file names from the first element in included paths
                OUStringBuffer buf(m_aInfo.userdata);
                buf.append("/user/registry/data");
                sal_Int32 n = 0;
                do {
                    OUString seg(i->first.getToken(0, '.', n));
                    OUString enc(
                        rtl::Uri::encode(
                            seg, rtl_UriCharClassPchar, rtl_UriEncodeStrict,
                            RTL_TEXTENCODING_UTF8));
                    if (enc.isEmpty() && !seg.isEmpty()) {
                        OSL_TRACE(
                            ("configuration migration component %s ignored (cannot"
                            " be encoded as file path)"),
                            OUStringToOString(
                                i->first, RTL_TEXTENCODING_UTF8).getStr());
                        goto next;
                    }
                    buf.append('/');
                    buf.append(enc);
                } while (n >= 0);
                buf.append(".xcu");
                regFilePath = buf.toString();
            }
            configuration::Update::get(
                comphelper::getProcessComponentContext())->
                insertModificationXcuFile(
                    regFilePath, setToSeq(i->second.includedPaths),
                    setToSeq(i->second.excludedPaths));
        } else {
            OSL_TRACE(
                ("configuration migration component %s ignored (only excludes,"
                 " no includes)"),
                OUStringToOString(
                    i->first, RTL_TEXTENCODING_UTF8).getStr());
        }
    next:;
    }
}

uno::Reference< XNameAccess > MigrationImpl::getConfigAccess(const sal_Char* pPath, bool bUpdate)
{
    uno::Reference< XNameAccess > xNameAccess;
    try{
        OUString sAccessSrvc;
        if (bUpdate)
            sAccessSrvc = "com.sun.star.configuration.ConfigurationUpdateAccess";
        else
            sAccessSrvc = "com.sun.star.configuration.ConfigurationAccess";

        OUString sConfigURL = OUString::createFromAscii(pPath);

        uno::Reference< XMultiServiceFactory > theConfigProvider(
            configuration::theDefaultProvider::get(
                comphelper::getProcessComponentContext()));

        // access the provider
        uno::Sequence< uno::Any > theArgs(1);
        theArgs[ 0 ] <<= sConfigURL;
        xNameAccess = uno::Reference< XNameAccess > (
                theConfigProvider->createInstanceWithArguments(
                sAccessSrvc, theArgs ), uno::UNO_QUERY_THROW );
    }
    catch (const css::uno::Exception& e)
    {
        SAL_WARN(
            "desktop.migration", "ignoring Exception \"" << e.Message << "\"");
    }
    return xNameAccess;
}

void MigrationImpl::copyFiles()
{
    strings_v::const_iterator i_file = m_vrFileList->begin();
    OUString localName;
    OUString destName;
    OUString userInstall;
    utl::Bootstrap::PathStatus aStatus;
    aStatus = utl::Bootstrap::locateUserInstallation(userInstall);
    if (aStatus == utl::Bootstrap::PATH_EXISTS)
    {
        while (i_file != m_vrFileList->end())
        {
            // remove installation prefix from file
            localName = i_file->copy(m_aInfo.userdata.getLength());
            if (localName.endsWith( "/autocorr/acor_.dat"))
            {
                // Previous versions used an empty language tag for
                // LANGUAGE_DONTKNOW with the "[All]" autocorrection entry.
                // As of LibreOffice 4.0 it is 'und' for LANGUAGE_UNDETERMINED
                // so the file name is "acor_und.dat".
                localName = localName.copy( 0, localName.getLength() - 4) + "und.dat";
            }
            destName = userInstall + localName;
            INetURLObject aURL(destName);
            // check whether destination directory exists
            aURL.removeSegment();
            _checkAndCreateDirectory(aURL);
            FileBase::RC copyResult = File::copy(*i_file, destName);
            if (copyResult != FileBase::E_None)
            {
                OString msg("Cannot copy ");
                msg += OUStringToOString(*i_file, RTL_TEXTENCODING_UTF8) + " to "
                    +  OUStringToOString(destName, RTL_TEXTENCODING_UTF8);
                OSL_FAIL(msg.getStr());
            }
            ++i_file;
        }
    }
    else
    {
        OSL_FAIL("copyFiles: UserInstall does not exist");
    }
}

void MigrationImpl::runServices()
{
    // Build argument array
    uno::Sequence< uno::Any > seqArguments(3);
    seqArguments[0] = uno::makeAny(NamedValue("Productname",
        uno::makeAny(m_aInfo.productname)));
    seqArguments[1] = uno::makeAny(NamedValue("UserData",
        uno::makeAny(m_aInfo.userdata)));


    // create an instance of every migration service
    // and execute the migration job
    uno::Reference< XJob > xMigrationJob;

    uno::Reference< uno::XComponentContext > xContext(comphelper::getProcessComponentContext());
    migrations_v::const_iterator i_mig  = m_vrMigrations->begin();
    while (i_mig != m_vrMigrations->end())
    {
        if( !i_mig->service.isEmpty())
        {

            try
            {
                // set black list for extension migration
                uno::Sequence< OUString > seqExtBlackList;
                sal_uInt32 nSize = i_mig->excludeExtensions.size();
                if ( nSize > 0 )
                    seqExtBlackList = comphelper::arrayToSequence< OUString >(
                        &i_mig->excludeExtensions[0], nSize );
                seqArguments[2] = uno::makeAny(NamedValue("ExtensionBlackList",
                    uno::makeAny( seqExtBlackList )));

                xMigrationJob = uno::Reference< XJob >(
                    xContext->getServiceManager()->createInstanceWithArgumentsAndContext(i_mig->service, seqArguments, xContext),
                    uno::UNO_QUERY_THROW);

                xMigrationJob->execute(uno::Sequence< NamedValue >());


            }
            catch (const Exception& e)
            {
                OString aMsg("Execution of migration service failed (Exception caught).\nService: ");
                aMsg += OUStringToOString(i_mig->service, RTL_TEXTENCODING_ASCII_US) + "\nMessage: ";
                aMsg += OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US);
                OSL_FAIL(aMsg.getStr());
            }
            catch (...)
            {
                OString aMsg("Execution of migration service failed (Exception caught).\nService: ");
                aMsg += OUStringToOString(i_mig->service, RTL_TEXTENCODING_ASCII_US) +
                    "\nNo message available";
                OSL_FAIL(aMsg.getStr());
            }

        }
        ++i_mig;
    }
}

::std::vector< MigrationModuleInfo > MigrationImpl::dectectUIChangesForAllModules() const
{
    ::std::vector< MigrationModuleInfo > vModulesInfo;
    const OUString MENUBAR("menubar");
    const OUString TOOLBAR("toolbar");

    uno::Sequence< uno::Any > lArgs(2);
    lArgs[0] <<= m_aInfo.userdata + "/user/config/soffice.cfg/modules";
    lArgs[1] <<= embed::ElementModes::READ;

    uno::Reference< lang::XSingleServiceFactory > xStorageFactory(
                     embed::FileSystemStorageFactory::create(comphelper::getProcessComponentContext()));
    uno::Reference< embed::XStorage >             xModules;

    xModules = uno::Reference< embed::XStorage >(xStorageFactory->createInstanceWithArguments(lArgs), uno::UNO_QUERY);
    if (!xModules.is())
        return vModulesInfo;

    uno::Reference< container::XNameAccess > xAccess = uno::Reference< container::XNameAccess >(xModules, uno::UNO_QUERY);
    uno::Sequence< OUString > lNames = xAccess->getElementNames();
    sal_Int32 nLength = lNames.getLength();
    for (sal_Int32 i=0; i<nLength; ++i)
    {
        OUString sModuleShortName = lNames[i];
        uno::Reference< embed::XStorage > xModule = xModules->openStorageElement(sModuleShortName, embed::ElementModes::READ);
        if (xModule.is())
        {
            MigrationModuleInfo aModuleInfo;

            uno::Reference< embed::XStorage > xMenubar = xModule->openStorageElement(MENUBAR, embed::ElementModes::READ);
            if (xMenubar.is())
            {
                uno::Reference< container::XNameAccess > xNameAccess = uno::Reference< container::XNameAccess >(xMenubar, uno::UNO_QUERY);
                if (xNameAccess->getElementNames().getLength() > 0)
                {
                    aModuleInfo.sModuleShortName = sModuleShortName;
                    aModuleInfo.bHasMenubar = true;
                }
            }

            uno::Reference< embed::XStorage > xToolbar = xModule->openStorageElement(TOOLBAR, embed::ElementModes::READ);
            if (xToolbar.is())
            {
                const OUString RESOURCEURL_CUSTOM_ELEMENT("custom_");
                sal_Int32 nCustomLen = 7;

                uno::Reference< container::XNameAccess > xNameAccess = uno::Reference< container::XNameAccess >(xToolbar, uno::UNO_QUERY);
                ::uno::Sequence< OUString > lToolbars = xNameAccess->getElementNames();
                for (sal_Int32 j=0; j<lToolbars.getLength(); ++j)
                {
                    OUString sToolbarName = lToolbars[j];
                    if (sToolbarName.getLength()>=nCustomLen &&
                        sToolbarName.copy(0, nCustomLen).equals(RESOURCEURL_CUSTOM_ELEMENT))
                        continue;

                    aModuleInfo.sModuleShortName = sModuleShortName;
                    sal_Int32 nIndex = sToolbarName.lastIndexOf('.');
                    if (nIndex > 0)
                    {
                        OUString sExtension(sToolbarName.copy(nIndex));
                        OUString sToolbarResourceName(sToolbarName.copy(0, nIndex));
                        if (!sToolbarResourceName.isEmpty() && sExtension == ".xml")
                            aModuleInfo.m_vToolbars.push_back(sToolbarResourceName);
                    }
                }
            }

            if (!aModuleInfo.sModuleShortName.isEmpty())
                vModulesInfo.push_back(aModuleInfo);
        }
    }

    return vModulesInfo;
}

void MigrationImpl::compareOldAndNewConfig(const OUString& sParent,
                                           const uno::Reference< container::XIndexContainer >& xIndexOld,
                                           const uno::Reference< container::XIndexContainer >& xIndexNew,
                                           const OUString& sResourceURL)
{
    const OUString MENU_SEPARATOR(" | ");

    ::std::vector< MigrationItem > vOldItems;
    ::std::vector< MigrationItem > vNewItems;
    uno::Sequence< beans::PropertyValue > aProp;
    sal_Int32 nOldCount = xIndexOld->getCount();
    sal_Int32 nNewCount = xIndexNew->getCount();

    for (int n=0; n<nOldCount; ++n)
    {
        MigrationItem aMigrationItem;
        if (xIndexOld->getByIndex(n) >>= aProp)
        {
            for(int i=0; i<aProp.getLength(); ++i)
            {
                if ( aProp[i].Name == ITEM_DESCRIPTOR_COMMANDURL )
                    aProp[i].Value >>= aMigrationItem.m_sCommandURL;
                else if ( aProp[i].Name == ITEM_DESCRIPTOR_CONTAINER )
                    aProp[i].Value >>= aMigrationItem.m_xPopupMenu;
            }

            if (!aMigrationItem.m_sCommandURL.isEmpty())
                vOldItems.push_back(aMigrationItem);
        }
    }

    for (int n=0; n<nNewCount; ++n)
    {
        MigrationItem aMigrationItem;
        if (xIndexNew->getByIndex(n) >>= aProp)
        {
            for(int i=0; i<aProp.getLength(); ++i)
            {
                if ( aProp[i].Name == ITEM_DESCRIPTOR_COMMANDURL )
                    aProp[i].Value >>= aMigrationItem.m_sCommandURL;
                else if ( aProp[i].Name == ITEM_DESCRIPTOR_CONTAINER )
                    aProp[i].Value >>= aMigrationItem.m_xPopupMenu;
            }

            if (!aMigrationItem.m_sCommandURL.isEmpty())
                vNewItems.push_back(aMigrationItem);
        }
    }

    ::std::vector< MigrationItem >::iterator it;

    OUString sSibling;
    for (it = vOldItems.begin(); it!=vOldItems.end(); ++it)
    {
        ::std::vector< MigrationItem >::iterator pFound = ::std::find(vNewItems.begin(), vNewItems.end(), *it);
        if (pFound != vNewItems.end() && it->m_xPopupMenu.is())
        {
            OUString sName;
            if (!sParent.isEmpty())
                sName = sParent + MENU_SEPARATOR + it->m_sCommandURL;
            else
                sName = it->m_sCommandURL;
            compareOldAndNewConfig(sName, it->m_xPopupMenu, pFound->m_xPopupMenu, sResourceURL);
        }
        else if (pFound == vNewItems.end())
        {
            MigrationItem aMigrationItem(sParent, sSibling, it->m_sCommandURL, it->m_xPopupMenu);
            if (m_aOldVersionItemsHashMap.find(sResourceURL)==m_aOldVersionItemsHashMap.end())
            {
                ::std::vector< MigrationItem > vMigrationItems;
                m_aOldVersionItemsHashMap.insert(MigrationHashMap::value_type(sResourceURL, vMigrationItems));
                m_aOldVersionItemsHashMap[sResourceURL].push_back(aMigrationItem);
            }
            else
            {
                if (::std::find(m_aOldVersionItemsHashMap[sResourceURL].begin(), m_aOldVersionItemsHashMap[sResourceURL].end(), aMigrationItem)==m_aOldVersionItemsHashMap[sResourceURL].end())
                    m_aOldVersionItemsHashMap[sResourceURL].push_back(aMigrationItem);
            }
        }

        sSibling = it->m_sCommandURL;
    }
}

void MigrationImpl::mergeOldToNewVersion(const uno::Reference< ui::XUIConfigurationManager >& xCfgManager,
                                         const uno::Reference< container::XIndexContainer>& xIndexContainer,
                                         const OUString& sModuleIdentifier,
                                         const OUString& sResourceURL)
{
    MigrationHashMap::iterator pFound = m_aOldVersionItemsHashMap.find(sResourceURL);
    if (pFound==m_aOldVersionItemsHashMap.end())
        return;

    ::std::vector< MigrationItem >::iterator it;
    for (it=pFound->second.begin(); it!=pFound->second.end(); ++it)
    {
        uno::Reference< container::XIndexContainer > xTemp = xIndexContainer;

        OUString sParentNodeName = it->m_sParentNodeName;
        sal_Int32 nIndex = 0;
        do
        {
            OUString sToken = sParentNodeName.getToken(0, '|', nIndex).trim();
            if (sToken.isEmpty())
                break;

            sal_Int32 nCount = xTemp->getCount();
            for (sal_Int32 i=0; i<nCount; ++i)
            {
                OUString sCommandURL;
                OUString sLabel;
                uno::Reference< container::XIndexContainer > xChild;

                uno::Sequence< beans::PropertyValue > aPropSeq;
                xTemp->getByIndex(i) >>= aPropSeq;
                for (sal_Int32 j=0; j<aPropSeq.getLength(); ++j)
                {
                    OUString sPropName = aPropSeq[j].Name;
                    if ( sPropName == ITEM_DESCRIPTOR_COMMANDURL )
                        aPropSeq[j].Value >>= sCommandURL;
                    else if ( sPropName == ITEM_DESCRIPTOR_LABEL )
                        aPropSeq[j].Value >>= sLabel;
                    else if ( sPropName == ITEM_DESCRIPTOR_CONTAINER )
                        aPropSeq[j].Value >>= xChild;
                }

                if (sCommandURL == sToken)
                {
                    xTemp = xChild;
                    break;
                }
            }

        } while (nIndex>=0);

        if (nIndex == -1)
        {
            uno::Sequence< beans::PropertyValue > aPropSeq(3);

            aPropSeq[0].Name = ITEM_DESCRIPTOR_COMMANDURL;
            aPropSeq[0].Value <<= it->m_sCommandURL;
            aPropSeq[1].Name = ITEM_DESCRIPTOR_LABEL;
            aPropSeq[1].Value <<= retrieveLabelFromCommand(it->m_sCommandURL, sModuleIdentifier);
            aPropSeq[2].Name = ITEM_DESCRIPTOR_CONTAINER;
            aPropSeq[2].Value <<= it->m_xPopupMenu;

            if (it->m_sPrevSibling.isEmpty())
                xTemp->insertByIndex(0, uno::makeAny(aPropSeq));
            else
            {
                sal_Int32 nCount = xTemp->getCount();
                sal_Int32 i = 0;
                for (; i<nCount; ++i)
                {
                    OUString sCmd;
                    uno::Sequence< beans::PropertyValue > aTempPropSeq;
                    xTemp->getByIndex(i) >>= aTempPropSeq;
                    for (sal_Int32 j=0; j<aTempPropSeq.getLength(); ++j)
                    {
                        if ( aTempPropSeq[j].Name == ITEM_DESCRIPTOR_COMMANDURL )
                        {
                            aTempPropSeq[j].Value >>= sCmd;
                            break;
                        }
                    }

                    if (sCmd.equals(it->m_sPrevSibling))
                        break;
                }

                xTemp->insertByIndex(i+1, uno::makeAny(aPropSeq));
            }
        }
    }

    uno::Reference< container::XIndexAccess > xIndexAccess(xIndexContainer, uno::UNO_QUERY);
    if (xIndexAccess.is())
        xCfgManager->replaceSettings(sResourceURL, xIndexAccess);

    uno::Reference< ui::XUIConfigurationPersistence > xUIConfigurationPersistence(xCfgManager, uno::UNO_QUERY);
    if (xUIConfigurationPersistence.is())
        xUIConfigurationPersistence->store();
}

uno::Reference< ui::XUIConfigurationManager > NewVersionUIInfo::getConfigManager(const OUString& sModuleShortName) const
{
    uno::Reference< ui::XUIConfigurationManager > xCfgManager;

    for (sal_Int32 i=0; i<m_lCfgManagerSeq.getLength(); ++i)
    {
        if (m_lCfgManagerSeq[i].Name.equals(sModuleShortName))
        {
            m_lCfgManagerSeq[i].Value >>= xCfgManager;
            break;
        }
    }

    return xCfgManager;
}

uno::Reference< container::XIndexContainer > NewVersionUIInfo::getNewMenubarSettings(const OUString& sModuleShortName) const
{
    uno::Reference< container::XIndexContainer > xNewMenuSettings;

    for (sal_Int32 i=0; i<m_lNewVersionMenubarSettingsSeq.getLength(); ++i)
    {
        if (m_lNewVersionMenubarSettingsSeq[i].Name.equals(sModuleShortName))
        {
            m_lNewVersionMenubarSettingsSeq[i].Value >>= xNewMenuSettings;
            break;
        }
    }

    return xNewMenuSettings;
}

uno::Reference< container::XIndexContainer > NewVersionUIInfo::getNewToolbarSettings(const OUString& sModuleShortName, const OUString& sToolbarName) const
{
    uno::Reference< container::XIndexContainer > xNewToolbarSettings;

    for (sal_Int32 i=0; i<m_lNewVersionToolbarSettingsSeq.getLength(); ++i)
    {
        if (m_lNewVersionToolbarSettingsSeq[i].Name.equals(sModuleShortName))
        {
            uno::Sequence< beans::PropertyValue > lToolbarSettingsSeq;
            m_lNewVersionToolbarSettingsSeq[i].Value >>= lToolbarSettingsSeq;
            for (sal_Int32 j=0; j<lToolbarSettingsSeq.getLength(); ++j)
            {
                if (lToolbarSettingsSeq[j].Name.equals(sToolbarName))
                {
                    lToolbarSettingsSeq[j].Value >>= xNewToolbarSettings;
                    break;
                }
            }

            break;
        }
    }

    return xNewToolbarSettings;
}

void NewVersionUIInfo::init(const ::std::vector< MigrationModuleInfo >& vModulesInfo)
{
    m_lCfgManagerSeq.realloc(vModulesInfo.size());
    m_lNewVersionMenubarSettingsSeq.realloc(vModulesInfo.size());
    m_lNewVersionToolbarSettingsSeq.realloc(vModulesInfo.size());

    const OUString sMenubarResourceURL("private:resource/menubar/menubar");
    const OUString sToolbarResourcePre("private:resource/toolbar/");

    uno::Reference< ui::XModuleUIConfigurationManagerSupplier > xModuleCfgSupplier = ui::theModuleUIConfigurationManagerSupplier::get( ::comphelper::getProcessComponentContext() );

    for (size_t i=0; i<vModulesInfo.size(); ++i)
    {
        OUString sModuleIdentifier = mapModuleShortNameToIdentifier(vModulesInfo[i].sModuleShortName);
        if (!sModuleIdentifier.isEmpty())
        {
            uno::Reference< ui::XUIConfigurationManager > xCfgManager = xModuleCfgSupplier->getUIConfigurationManager(sModuleIdentifier);
            m_lCfgManagerSeq[i].Name = vModulesInfo[i].sModuleShortName;
            m_lCfgManagerSeq[i].Value <<= xCfgManager;

            if (vModulesInfo[i].bHasMenubar)
            {
                m_lNewVersionMenubarSettingsSeq[i].Name = vModulesInfo[i].sModuleShortName;
                m_lNewVersionMenubarSettingsSeq[i].Value <<= xCfgManager->getSettings(sMenubarResourceURL, sal_True);
            }

            sal_Int32 nToolbars = vModulesInfo[i].m_vToolbars.size();
            if (nToolbars > 0)
            {
                uno::Sequence< beans::PropertyValue > lPropSeq(nToolbars);
                for (sal_Int32 j=0; j<nToolbars; ++j)
                {
                    OUString sToolbarName = vModulesInfo[i].m_vToolbars[j];
                    OUString sToolbarResourceURL = sToolbarResourcePre + sToolbarName;

                    lPropSeq[j].Name = sToolbarName;
                    lPropSeq[j].Value <<= xCfgManager->getSettings(sToolbarResourceURL, sal_True);
                }

                m_lNewVersionToolbarSettingsSeq[i].Name = vModulesInfo[i].sModuleShortName;
                m_lNewVersionToolbarSettingsSeq[i].Value <<= lPropSeq;
            }
        }
    }
}

} // namespace desktop

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
