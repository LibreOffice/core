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

#include <accelerators/presethandler.hxx>

#include <classes/fwkresid.hxx>

#include "classes/resource.hrc"
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>
#include <services.h>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/configuration/CorruptedUIConfigurationException.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/FileSystemStorageFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/util/thePathSettings.hpp>

#include <vcl/svapp.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <rtl/ustrbuf.hxx>
#include <i18nlangtag/languagetag.hxx>


static const ::sal_Int32 ID_CORRUPT_UICONFIG_SHARE   = 1;
static const ::sal_Int32 ID_CORRUPT_UICONFIG_USER    = 2;
static const ::sal_Int32 ID_CORRUPT_UICONFIG_GENERAL = 3;

namespace framework
{


OUString PresetHandler::PRESET_DEFAULT()
{
    return OUString("default");
}


OUString PresetHandler::TARGET_CURRENT()
{
    return OUString("current");
}


OUString PresetHandler::RESOURCETYPE_MENUBAR()
{
    return OUString("menubar");
}


OUString PresetHandler::RESOURCETYPE_TOOLBAR()
{
    return OUString("toolbar");
}


OUString PresetHandler::RESOURCETYPE_ACCELERATOR()
{
    return OUString("accelerator");
}


OUString PresetHandler::RESOURCETYPE_STATUSBAR()
{
    return OUString("statusbar");
}


PresetHandler::PresetHandler(const css::uno::Reference< css::uno::XComponentContext >& xContext)
    : ThreadHelpBase     (&Application::GetSolarMutex()        )
    , m_xContext         (xContext                                )
    , m_aSharedStorages  (                                     )
    , m_lDocumentStorages()
    , m_aLanguageTag     (LANGUAGE_USER_PRIV_NOTRANSLATE)
{
}


PresetHandler::PresetHandler(const PresetHandler& rCopy)
    : ThreadHelpBase     (&Application::GetSolarMutex()        )
    , m_aLanguageTag( rCopy.m_aLanguageTag)
{
    m_xContext              = rCopy.m_xContext;
    m_eConfigType           = rCopy.m_eConfigType;
    m_sResourceType         = rCopy.m_sResourceType;
    m_sModule               = rCopy.m_sModule;
    m_aSharedStorages       = rCopy.m_aSharedStorages;
    m_xWorkingStorageShare  = rCopy.m_xWorkingStorageShare;
    m_xWorkingStorageNoLang = rCopy.m_xWorkingStorageNoLang;
    m_xWorkingStorageUser   = rCopy.m_xWorkingStorageUser;
    m_lPresets              = rCopy.m_lPresets;
    m_lTargets              = rCopy.m_lTargets;
    m_lDocumentStorages     = rCopy.m_lDocumentStorages;
    m_sRelPathShare         = rCopy.m_sRelPathShare;
    m_sRelPathNoLang        = rCopy.m_sRelPathNoLang;
    m_sRelPathUser          = rCopy.m_sRelPathUser;
}


PresetHandler::~PresetHandler()
{
    m_xWorkingStorageShare.clear();
    m_xWorkingStorageNoLang.clear();
    m_xWorkingStorageUser.clear();

    /* #i46497#
        Dont call forgetCachedStorages() here for shared storages.
        Because we opened different sub storages by using openPath().
        And every already open path was reused and referenced (means it's
        ref count was increased!)
        So now we have to release our ref counts to these shared storages
        only ... and not to free all used storages.
        Otherwise we will disconnect all other open configuration access
        objects which base on these storages.
     */
    m_aSharedStorages->m_lStoragesShare.closePath(m_sRelPathShare);
    m_aSharedStorages->m_lStoragesUser.closePath (m_sRelPathUser );

    /* On the other side closePath() is not needed for our special handled
       document storage. Because it's not shared with others ... so we can
       free it.
     */
    m_lDocumentStorages.forgetCachedStorages();
}


void PresetHandler::forgetCachedStorages()
{
    
    WriteGuard aWriteLock(m_aLock);

    if (m_eConfigType == E_DOCUMENT)
    {
        m_xWorkingStorageShare.clear();
        m_xWorkingStorageNoLang.clear();
        m_xWorkingStorageUser.clear();
    }

    m_lDocumentStorages.forgetCachedStorages();

    aWriteLock.unlock();
    
}

namespace {

OUString lcl_getLocalizedMessage(::sal_Int32 nID)
{
    OUString sMessage("Unknown error.");

    switch(nID)
    {
        case ID_CORRUPT_UICONFIG_SHARE :
                sMessage = FWK_RESSTR(STR_CORRUPT_UICFG_SHARE);

                break;

        case ID_CORRUPT_UICONFIG_USER :
                sMessage = FWK_RESSTR(STR_CORRUPT_UICFG_USER);
                break;

        case ID_CORRUPT_UICONFIG_GENERAL :
                sMessage = FWK_RESSTR(STR_CORRUPT_UICFG_GENERAL);
                break;
    }

    return sMessage;
}

void lcl_throwCorruptedUIConfigurationException(
    css::uno::Any const & exception, sal_Int32 id)
{
    css::uno::Exception e;
    bool ok = (exception >>= e);
    OSL_ASSERT(ok); (void) ok; 
    throw css::configuration::CorruptedUIConfigurationException(
        lcl_getLocalizedMessage(id),
        css::uno::Reference< css::uno::XInterface >(),
        (exception.getValueTypeName() +
         OUString(": \"") + e.Message +
         OUString("\"")));
}

}

css::uno::Reference< css::embed::XStorage > PresetHandler::getOrCreateRootStorageShare()
{
    css::uno::Reference< css::embed::XStorage > xRoot = m_aSharedStorages->m_lStoragesShare.getRootStorage();
    if (xRoot.is())
        return xRoot;

    
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;
    aReadLock.unlock();
    

    css::uno::Reference< css::util::XPathSettings > xPathSettings =
        css::util::thePathSettings::get( xContext );

    OUString sShareLayer = xPathSettings->getBasePathShareLayer();

    
    sal_Int32 nPos = sShareLayer.indexOf(';');
    if (nPos > 0)
        sShareLayer = sShareLayer.copy(0, nPos);

    
    nPos = sShareLayer.lastIndexOf('/');
    if (nPos != sShareLayer.getLength()-1)
        sShareLayer += "/";

    sShareLayer += "soffice.cfg";
    /*
    
    
    
    
    */
    css::uno::Sequence< css::uno::Any > lArgs(2);
    lArgs[0] <<= sShareLayer;
    lArgs[1] <<= css::embed::ElementModes::READ | css::embed::ElementModes::NOCREATE;

    css::uno::Reference< css::lang::XSingleServiceFactory > xStorageFactory = css::embed::FileSystemStorageFactory::create( xContext );
    css::uno::Reference< css::embed::XStorage >             xStorage;

    try
    {
        xStorage = css::uno::Reference< css::embed::XStorage >(xStorageFactory->createInstanceWithArguments(lArgs), css::uno::UNO_QUERY_THROW);
    }
    catch(const css::uno::Exception&)
    {
        css::uno::Any ex(cppu::getCaughtException());
        lcl_throwCorruptedUIConfigurationException(
            ex, ID_CORRUPT_UICONFIG_SHARE);
    }

    m_aSharedStorages->m_lStoragesShare.setRootStorage(xStorage);

    return xStorage;
}


css::uno::Reference< css::embed::XStorage > PresetHandler::getOrCreateRootStorageUser()
{
    css::uno::Reference< css::embed::XStorage > xRoot = m_aSharedStorages->m_lStoragesUser.getRootStorage();
    if (xRoot.is())
        return xRoot;

    
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;
    aReadLock.unlock();
    

    css::uno::Reference< css::util::XPathSettings > xPathSettings =
        css::util::thePathSettings::get( xContext );

    OUString sUserLayer = xPathSettings->getBasePathUserLayer();

    
    sal_Int32 nPos = sUserLayer.lastIndexOf('/');
    if (nPos != sUserLayer.getLength()-1)
        sUserLayer += "/";

    sUserLayer  += "soffice.cfg"; 

    css::uno::Sequence< css::uno::Any > lArgs(2);
    lArgs[0] <<= sUserLayer;
    lArgs[1] <<= css::embed::ElementModes::READWRITE;

    css::uno::Reference< css::lang::XSingleServiceFactory > xStorageFactory = css::embed::FileSystemStorageFactory::create( xContext );
    css::uno::Reference< css::embed::XStorage >             xStorage;

    try
    {
        xStorage = css::uno::Reference< css::embed::XStorage >(xStorageFactory->createInstanceWithArguments(lArgs), css::uno::UNO_QUERY_THROW);
    }
    catch(const css::uno::Exception&)
    {
        css::uno::Any ex(cppu::getCaughtException());
        lcl_throwCorruptedUIConfigurationException(
            ex, ID_CORRUPT_UICONFIG_USER);
    }

    m_aSharedStorages->m_lStoragesUser.setRootStorage(xStorage);

    return xStorage;
}


css::uno::Reference< css::embed::XStorage > PresetHandler::getWorkingStorageShare()
{
    
    ReadGuard aReadLock(m_aLock);
    return m_xWorkingStorageShare;
    
}


css::uno::Reference< css::embed::XStorage > PresetHandler::getWorkingStorageUser()
{
    
    ReadGuard aReadLock(m_aLock);
    return m_xWorkingStorageUser;
    
}


css::uno::Reference< css::embed::XStorage > PresetHandler::getParentStorageShare(const css::uno::Reference< css::embed::XStorage >& /*xChild*/)
{
    
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::embed::XStorage > xWorking = m_xWorkingStorageShare;
    aReadLock.unlock();
    

    return m_aSharedStorages->m_lStoragesShare.getParentStorage(xWorking);
}


css::uno::Reference< css::embed::XStorage > PresetHandler::getParentStorageUser(const css::uno::Reference< css::embed::XStorage >& /*xChild*/)
{
    
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::embed::XStorage > xWorking = m_xWorkingStorageUser;
    aReadLock.unlock();
    

    return m_aSharedStorages->m_lStoragesUser.getParentStorage(xWorking);
}


void PresetHandler::connectToResource(      PresetHandler::EConfigType                   eConfigType  ,
                                      const OUString&                             sResource    ,
                                      const OUString&                             sModule      ,
                                      const css::uno::Reference< css::embed::XStorage >& xDocumentRoot,
                                      const LanguageTag&                                 rLanguageTag )
{
    

    
    WriteGuard aWriteLock(m_aLock);

    m_eConfigType   = eConfigType  ;
    m_sResourceType = sResource    ;
    m_sModule       = sModule      ;
    m_aLanguageTag  = rLanguageTag ;

    aWriteLock.unlock();
    

    css::uno::Reference< css::embed::XStorage > xShare;
    css::uno::Reference< css::embed::XStorage > xNoLang;
    css::uno::Reference< css::embed::XStorage > xUser;

    
    
    if (eConfigType == E_DOCUMENT)
    {
        if (!xDocumentRoot.is())
            throw css::uno::RuntimeException(
                    OUString("There is valid root storage, where the UI configuration can work on."),
                    css::uno::Reference< css::uno::XInterface >());
        m_lDocumentStorages.setRootStorage(xDocumentRoot);
        xShare = xDocumentRoot;
        xUser  = xDocumentRoot;
    }
    else
    {
        xShare = getOrCreateRootStorageShare();
        xUser  = getOrCreateRootStorageUser();
    }

    
    try
    {

    
    
    
    
    sal_Int32 eShareMode = (css::embed::ElementModes::READ      | css::embed::ElementModes::NOCREATE);
    sal_Int32 eUserMode  = (css::embed::ElementModes::READWRITE                                     );

    OUStringBuffer sRelPathBuf(1024);
    OUString       sRelPathShare;
    OUString       sRelPathNoLang;
    OUString       sRelPathUser;
    switch(eConfigType)
    {
        case E_GLOBAL :
        {
            sRelPathBuf.append("global");
            sRelPathBuf.append("/");
            sRelPathBuf.append(sResource);
            sRelPathShare = sRelPathBuf.makeStringAndClear();
            sRelPathUser  = sRelPathShare;

            xShare = impl_openPathIgnoringErrors(sRelPathShare, eShareMode, sal_True );
            xUser  = impl_openPathIgnoringErrors(sRelPathUser , eUserMode , sal_False);
        }
        break;

        case E_MODULES :
        {
            sRelPathBuf.append("modules");
            sRelPathBuf.append("/");
            sRelPathBuf.append(sModule);
            sRelPathBuf.append("/");
            sRelPathBuf.append(sResource);
            sRelPathShare = sRelPathBuf.makeStringAndClear();
            sRelPathUser  = sRelPathShare;

            xShare = impl_openPathIgnoringErrors(sRelPathShare, eShareMode, sal_True );
            xUser  = impl_openPathIgnoringErrors(sRelPathUser , eUserMode , sal_False);
        }
        break;

        case E_DOCUMENT :
        {
            
            
            

            sRelPathBuf.append(sResource);
            sRelPathUser  = sRelPathBuf.makeStringAndClear();
            sRelPathShare = sRelPathUser;

            try
            {
                xUser  = m_lDocumentStorages.openPath(sRelPathUser , eUserMode );
                xShare = xUser;
            }
            catch(const css::uno::RuntimeException&)
                { throw; }
            catch(const css::uno::Exception&)
                { xShare.clear(); xUser.clear(); }
        }
        break;
    }

    
    xNoLang = xShare;
    sRelPathNoLang = sRelPathShare;

    if (
        (rLanguageTag != LanguageTag(LANGUAGE_USER_PRIV_NOTRANSLATE)) && 
        (eConfigType != E_DOCUMENT                           )    
       )
    {
        
        
        OUString             aShareLocale( rLanguageTag.getBcp47());
        OUString      sLocalizedSharePath(sRelPathShare);
        sal_Bool             bAllowFallbacks    = sal_True     ;
        xShare = impl_openLocalizedPathIgnoringErrors(sLocalizedSharePath, eShareMode, sal_True , aShareLocale, bAllowFallbacks);

        
        
        
        OUString             aUserLocale( rLanguageTag.getBcp47());
        OUString      sLocalizedUserPath(sRelPathUser);
                             bAllowFallbacks    = sal_False  ;
        xUser = impl_openLocalizedPathIgnoringErrors(sLocalizedUserPath, eUserMode , sal_False, aUserLocale, bAllowFallbacks);

        sRelPathShare = sLocalizedSharePath;
        sRelPathUser  = sLocalizedUserPath ;
    }

    
          css::uno::Reference< css::container::XNameAccess > xAccess ;
          css::uno::Sequence< OUString >              lNames  ;
    const OUString*                                   pNames  ;
          sal_Int32                                          c       ;
          sal_Int32                                          i       ;
          OUStringList                                       lPresets;
          OUStringList                                       lTargets;

    
    xAccess = css::uno::Reference< css::container::XNameAccess >(xShare, css::uno::UNO_QUERY);
    if (xAccess.is())
    {
        lNames  = xAccess->getElementNames();
        pNames  = lNames.getConstArray();
        c       = lNames.getLength();

        for (i=0; i<c; ++i)
        {
            OUString sTemp = pNames[i];
            sal_Int32       nPos  = sTemp.indexOf(".xml");
            if (nPos > -1)
                sTemp = sTemp.copy(0,nPos);
            lPresets.push_back(sTemp);
        }
    }

    
    xAccess = css::uno::Reference< css::container::XNameAccess >(xUser, css::uno::UNO_QUERY);
    if (xAccess.is())
    {
        lNames  = xAccess->getElementNames();
        pNames  = lNames.getConstArray();
        c       = lNames.getLength();

        for (i=0; i<c; ++i)
        {
            OUString sTemp = pNames[i];
            sal_Int32       nPos  = sTemp.indexOf(".xml");
            if (nPos > -1)
                sTemp = sTemp.copy(0,nPos);
            lTargets.push_back(sTemp);
        }
    }

    
    aWriteLock.lock();

    m_xWorkingStorageShare = xShare  ;
    m_xWorkingStorageNoLang= xNoLang;
    m_xWorkingStorageUser  = xUser   ;
    m_lPresets             = lPresets;
    m_lTargets             = lTargets;
    m_sRelPathShare        = sRelPathShare;
    m_sRelPathNoLang       = sRelPathNoLang;
    m_sRelPathUser         = sRelPathUser;

    aWriteLock.unlock();
    

    }
    catch(const css::uno::Exception&)
    {
        css::uno::Any ex(cppu::getCaughtException());
        lcl_throwCorruptedUIConfigurationException(
            ex, ID_CORRUPT_UICONFIG_GENERAL);
    }
}


void PresetHandler::copyPresetToTarget(const OUString& sPreset,
                                       const OUString& sTarget)
{
    
    

    
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::embed::XStorage > xWorkingShare = m_xWorkingStorageShare;
    css::uno::Reference< css::embed::XStorage > xWorkingNoLang= m_xWorkingStorageNoLang;
    css::uno::Reference< css::embed::XStorage > xWorkingUser  = m_xWorkingStorageUser ;
    aReadLock.unlock();
    

    
    if (
        (!xWorkingShare.is()) ||
        (!xWorkingUser.is() )
       )
    {
       return;
    }

    OUString sPresetFile(sPreset);
    sPresetFile += ".xml";

    OUString sTargetFile(sTarget);
    sTargetFile += ".xml";

    
    
    css::uno::Reference< css::container::XNameAccess > xCheckingUser(xWorkingUser, css::uno::UNO_QUERY_THROW);
    if (xCheckingUser->hasByName(sTargetFile))
        xWorkingUser->removeElement(sTargetFile);

    xWorkingShare->copyElementTo(sPresetFile, xWorkingUser, sTargetFile);

    
    
    commitUserChanges();
}


css::uno::Reference< css::io::XStream > PresetHandler::openPreset(const OUString& sPreset,
                                                                  sal_Bool bUseNoLangGlobal)
{
    
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::embed::XStorage > xFolder = bUseNoLangGlobal? m_xWorkingStorageNoLang: m_xWorkingStorageShare;
    aReadLock.unlock();
    

    
    if (!xFolder.is())
       return css::uno::Reference< css::io::XStream >();

    OUString sFile(sPreset);
    sFile += ".xml";

    
    css::uno::Reference< css::io::XStream > xStream = xFolder->openStreamElement(sFile, css::embed::ElementModes::READ);
    return xStream;
}


css::uno::Reference< css::io::XStream > PresetHandler::openTarget(const OUString& sTarget         ,
                                                                        sal_Bool         bCreateIfMissing)
{
    
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::embed::XStorage > xFolder = m_xWorkingStorageUser;
    aReadLock.unlock();
    

    
    if (!xFolder.is())
       return css::uno::Reference< css::io::XStream >();

    OUString sFile(sTarget);
    sFile += ".xml";

    sal_Int32 nOpenMode = css::embed::ElementModes::READWRITE;
    if (!bCreateIfMissing)
        nOpenMode |= css::embed::ElementModes::NOCREATE;

    
    css::uno::Reference< css::io::XStream > xStream;
    try
    {
        xStream = xFolder->openStreamElement(sFile, nOpenMode);
        return xStream;
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        { xStream.clear(); }

    
    
    nOpenMode &= ~css::embed::ElementModes::WRITE;
    xStream    = xFolder->openStreamElement(sFile, nOpenMode);

    return xStream;
}


void PresetHandler::commitUserChanges()
{
    
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::embed::XStorage > xWorking = m_xWorkingStorageUser;
    EConfigType                                 eCfgType = m_eConfigType;
    aReadLock.unlock();
    

    
    if (!xWorking.is())
       return;

    OUString sPath;

    switch(eCfgType)
    {
        case E_GLOBAL :
        case E_MODULES :
        {
            sPath = m_aSharedStorages->m_lStoragesUser.getPathOfStorage(xWorking);
            m_aSharedStorages->m_lStoragesUser.commitPath(sPath);
            m_aSharedStorages->m_lStoragesUser.notifyPath(sPath);
        }
        break;

        case E_DOCUMENT :
        {
            sPath = m_lDocumentStorages.getPathOfStorage(xWorking);
            m_lDocumentStorages.commitPath(sPath);
            m_lDocumentStorages.notifyPath(sPath);
        }
        break;
    }
}


void PresetHandler::addStorageListener(IStorageListener* pListener)
{
    
    ReadGuard aReadLock(m_aLock);
    OUString sRelPath = m_sRelPathUser; 
    EConfigType     eCfgType = m_eConfigType;
    aReadLock.unlock();
    

    if (sRelPath.isEmpty())
        return;

    switch(eCfgType)
    {
        case E_GLOBAL :
        case E_MODULES :
        {
            m_aSharedStorages->m_lStoragesUser.addStorageListener(pListener, sRelPath);
        }
        break;

        case E_DOCUMENT :
        {
            m_lDocumentStorages.addStorageListener(pListener, sRelPath);
        }
        break;
    }
}


void PresetHandler::removeStorageListener(IStorageListener* pListener)
{
    
    ReadGuard aReadLock(m_aLock);
    OUString sRelPath = m_sRelPathUser; 
    EConfigType     eCfgType = m_eConfigType;
    aReadLock.unlock();
    

    if (sRelPath.isEmpty())
        return;

    switch(eCfgType)
    {
        case E_GLOBAL :
        case E_MODULES :
        {
            m_aSharedStorages->m_lStoragesUser.removeStorageListener(pListener, sRelPath);
        }
        break;

        case E_DOCUMENT :
        {
            m_lDocumentStorages.removeStorageListener(pListener, sRelPath);
        }
        break;
    }
}


css::uno::Reference< css::embed::XStorage > PresetHandler::impl_openPathIgnoringErrors(const OUString& sPath ,
                                                                                             sal_Int32        eMode ,
                                                                                             sal_Bool         bShare)
{
    css::uno::Reference< css::embed::XStorage > xPath;
    try
    {
        if (bShare)
            xPath = m_aSharedStorages->m_lStoragesShare.openPath(sPath, eMode);
        else
            xPath = m_aSharedStorages->m_lStoragesUser.openPath(sPath, eMode);
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        { xPath.clear(); }
    return xPath;
}


::std::vector< OUString >::const_iterator PresetHandler::impl_findMatchingLocalizedValue(
        const ::std::vector< OUString >& lLocalizedValues,
        OUString& rLanguageTag,
        sal_Bool bAllowFallbacks )
{
    ::std::vector< OUString >::const_iterator pFound = lLocalizedValues.end();
    if (bAllowFallbacks)
    {
        pFound = LanguageTag::getFallback(lLocalizedValues, rLanguageTag);
        
        
        if (pFound != lLocalizedValues.end())
        {
            rLanguageTag = *pFound;
        }
    }
    else
    {
        for (  pFound  = lLocalizedValues.begin();
               pFound != lLocalizedValues.end()  ;
             ++pFound                            )
        {
            if (*pFound == rLanguageTag)
                break;
        }
    }

    return pFound;
}


css::uno::Reference< css::embed::XStorage > PresetHandler::impl_openLocalizedPathIgnoringErrors(
        OUString&      sPath         ,
        sal_Int32             eMode         ,
        sal_Bool              bShare        ,
        OUString&             rLanguageTag  ,
        sal_Bool              bAllowFallback)
{
    css::uno::Reference< css::embed::XStorage >      xPath         = impl_openPathIgnoringErrors(sPath, eMode, bShare);
    ::std::vector< OUString >                 lSubFolders   = impl_getSubFolderNames(xPath);
    ::std::vector< OUString >::const_iterator pLocaleFolder = impl_findMatchingLocalizedValue(lSubFolders, rLanguageTag, bAllowFallback);

    
    if (
        (pLocaleFolder == lSubFolders.end()                                                ) &&
        ((eMode & css::embed::ElementModes::NOCREATE) == css::embed::ElementModes::NOCREATE)
       )
        return css::uno::Reference< css::embed::XStorage >();

    
    
    
    OUString sLocalizedPath;
    sLocalizedPath  = sPath;
    sLocalizedPath += "/";
    if (pLocaleFolder != lSubFolders.end())
        sLocalizedPath += *pLocaleFolder;
    else
        sLocalizedPath += rLanguageTag;

    css::uno::Reference< css::embed::XStorage > xLocalePath = impl_openPathIgnoringErrors(sLocalizedPath, eMode, bShare);

    if (xLocalePath.is())
        sPath = sLocalizedPath;
    else
        sPath = OUString();

    return xLocalePath;
}


::std::vector< OUString > PresetHandler::impl_getSubFolderNames(const css::uno::Reference< css::embed::XStorage >& xFolder)
{
    css::uno::Reference< css::container::XNameAccess > xAccess(xFolder, css::uno::UNO_QUERY);
    if (!xAccess.is())
        return ::std::vector< OUString >();

          ::std::vector< OUString >      lSubFolders;
    const css::uno::Sequence< OUString > lNames = xAccess->getElementNames();
    const OUString*                      pNames = lNames.getConstArray();
          sal_Int32                             c      = lNames.getLength();
          sal_Int32                             i      = 0;

    for (i=0; i<c; ++i)
    {
        try
        {
            if (xFolder->isStorageElement(pNames[i]))
                lSubFolders.push_back(pNames[i]);
        }
        catch(const css::uno::RuntimeException&)
            { throw; }
        catch(const css::uno::Exception&)
            {}
    }

    return lSubFolders;
}


} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
