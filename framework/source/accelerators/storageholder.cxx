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

#include <accelerators/storageholder.hxx>
#include <accelerators/acceleratorconfiguration.hxx>
#include <sal/log.hxx>

#include <com/sun/star/embed/ElementModes.hpp>

#include <com/sun/star/embed/XTransactedObject.hpp>

#include <rtl/ustrbuf.hxx>

#include <algorithm>

#define PATH_SEPARATOR "/"
#define PATH_SEPARATOR_UNICODE      u'/'

namespace framework
{

StorageHolder::StorageHolder()
{
}

StorageHolder::~StorageHolder()
{
    // TODO implement me
    // dispose/clear etcpp.
}

void StorageHolder::forgetCachedStorages()
{
    osl::MutexGuard g(m_mutex);
    for (auto & lStorage : m_lStorages)
    {
        TStorageInfo& rInfo = lStorage.second;
        // TODO think about listener !
        rInfo.Storage.clear();
    }
    m_lStorages.clear();
}

void StorageHolder::setRootStorage(const css::uno::Reference< css::embed::XStorage >& xRoot)
{
    osl::MutexGuard g(m_mutex);
    m_xRoot = xRoot;
}

css::uno::Reference< css::embed::XStorage > StorageHolder::getRootStorage() const
{
    osl::MutexGuard g(m_mutex);
    return m_xRoot;
}

css::uno::Reference< css::embed::XStorage > StorageHolder::openPath(const OUString& sPath    ,
                                                                          sal_Int32        nOpenMode)
{
    OUString sNormedPath = StorageHolder::impl_st_normPath(sPath);
    std::vector<OUString> lFolders    = StorageHolder::impl_st_parsePath(sNormedPath);

    // SAFE -> ----------------------------------
    osl::ResettableMutexGuard aReadLock(m_mutex);
    css::uno::Reference< css::embed::XStorage > xParent = m_xRoot;
    aReadLock.clear();
    // <- SAFE ----------------------------------

    css::uno::Reference< css::embed::XStorage > xChild;
    OUString                             sRelPath;

    for (auto const& lFolder : lFolders)
    {
        OUString  sCheckPath (sRelPath + lFolder + PATH_SEPARATOR);

        // SAFE -> ------------------------------
        aReadLock.reset();

        // If we found an already open storage ... we must increase
        // its use count. Otherwise it will may be closed too early :-)
        TPath2StorageInfo::iterator pCheck = m_lStorages.find(sCheckPath);
        TStorageInfo*               pInfo  = nullptr;
        if (pCheck != m_lStorages.end())
        {
            pInfo = &(pCheck->second);
            ++(pInfo->UseCount);
            xChild = pInfo->Storage;

            aReadLock.clear();
            // <- SAFE ------------------------------
        }
        else
        {
            aReadLock.clear();
            // <- SAFE ------------------------------

            try
            {
                xChild = StorageHolder::openSubStorageWithFallback(xParent, lFolder, nOpenMode); // TODO think about delegating fallback decision to our own caller!
            }
            catch(const css::uno::RuntimeException&)
                { throw; }
            catch(const css::uno::Exception&)
                {
                    /* TODO URGENT!
                        in case we found some "already existing storages" on the path before and increased its UseCount ...
                        and now we will get an exception on creating a new sub storage ...
                        we must decrease all UseCounts, which was touched before. Otherwise these storages can't be closed!

                        Idea: Using of another structure member "PossibleUseCount" as vector of unique numbers.
                        Every thread use another unique number to identify all "owned candidates".
                        A flush method with the same unique number force increasing of the "UseCount" variable then
                        inside a synchronized block ...
                    */
                    throw;
                }

            osl::MutexGuard g(m_mutex);
            pInfo = &(m_lStorages[sCheckPath]);
            pInfo->Storage  = xChild;
            pInfo->UseCount = 1;
        }

        xParent   = xChild;
        sRelPath += lFolder + PATH_SEPARATOR;
    }

    // TODO think about return last storage as working storage ... but don't caching it inside this holder!
    // => otherwise the same storage is may be commit more than once.

    return xChild;
}

StorageHolder::TStorageList StorageHolder::getAllPathStorages(const OUString& sPath)
{
    OUString sNormedPath = StorageHolder::impl_st_normPath(sPath);
    std::vector<OUString> lFolders    = StorageHolder::impl_st_parsePath(sNormedPath);

    StorageHolder::TStorageList  lStoragesOfPath;
    OUString              sRelPath;

    osl::MutexGuard g(m_mutex);

    for (auto const& lFolder : lFolders)
    {
        OUString  sCheckPath (sRelPath + lFolder + PATH_SEPARATOR);

        TPath2StorageInfo::iterator pCheck = m_lStorages.find(sCheckPath);
        if (pCheck == m_lStorages.end())
        {
            // at least one path element was not found
            // Seems that this path isn't open ...
            lStoragesOfPath.clear();
            return lStoragesOfPath;
        }

        TStorageInfo& rInfo = pCheck->second;
        lStoragesOfPath.push_back(rInfo.Storage);

        sRelPath += lFolder + PATH_SEPARATOR;
    }

    return lStoragesOfPath;
}

void StorageHolder::commitPath(const OUString& sPath)
{
    StorageHolder::TStorageList lStorages = getAllPathStorages(sPath);

    css::uno::Reference< css::embed::XTransactedObject > xCommit;
    StorageHolder::TStorageList::reverse_iterator pIt;
    for (  pIt  = lStorages.rbegin(); // order of commit is important ... otherwise changes are not recognized!
           pIt != lStorages.rend();
         ++pIt                      )
    {
        xCommit.set(*pIt, css::uno::UNO_QUERY);
        if (!xCommit.is())
            continue;
        xCommit->commit();
    }

    // SAFE -> ------------------------------
    {
        osl::MutexGuard aReadLock(m_mutex);
        xCommit.set(m_xRoot, css::uno::UNO_QUERY);
    }
    // <- SAFE ------------------------------

    if (xCommit.is())
        xCommit->commit();
}

void StorageHolder::closePath(const OUString& rPath)
{
    OUString sNormedPath = StorageHolder::impl_st_normPath(rPath);
    std::vector<OUString> lFolders    = StorageHolder::impl_st_parsePath(sNormedPath);

    /* convert list of paths in the following way:
        [0] = "path_1" => "path_1
        [1] = "path_2" => "path_1/path_2"
        [2] = "path_3" => "path_1/path_2/path_3"
    */
    OUString        sParentPath;
    for (auto & lFolder : lFolders)
    {
        OUString sCurrentRelPath(sParentPath + lFolder + PATH_SEPARATOR);
        lFolder = sCurrentRelPath;
        sParentPath = sCurrentRelPath;
    }

    osl::MutexGuard g(m_mutex);

    std::vector<OUString>::reverse_iterator pIt2;
    for (  pIt2  = lFolders.rbegin();
           pIt2 != lFolders.rend();
         ++pIt2                     )
    {
        OUString             sPath = *pIt2;
        TPath2StorageInfo::iterator pPath = m_lStorages.find(sPath);
        if (pPath == m_lStorages.end())
            continue; // ???

        TStorageInfo& rInfo = pPath->second;
        --rInfo.UseCount;
        if (rInfo.UseCount < 1)
        {
            rInfo.Storage.clear();
            m_lStorages.erase(pPath);
        }
    }
}

void StorageHolder::notifyPath(const OUString& sPath)
{
    OUString sNormedPath = StorageHolder::impl_st_normPath(sPath);

    osl::MutexGuard g(m_mutex);

    TPath2StorageInfo::iterator pIt1 = m_lStorages.find(sNormedPath);
    if (pIt1 == m_lStorages.end())
        return;

    TStorageInfo& rInfo = pIt1->second;
    for (auto const& listener : rInfo.Listener)
    {
        if (listener)
            listener->changesOccurred();
    }
}

void StorageHolder::addStorageListener(      XMLBasedAcceleratorConfiguration* pListener,
                                       const OUString&  sPath    )
{
    OUString sNormedPath = StorageHolder::impl_st_normPath(sPath);

    osl::MutexGuard g(m_mutex);

    TPath2StorageInfo::iterator pIt1 = m_lStorages.find(sNormedPath);
    if (pIt1 == m_lStorages.end())
        return;

    TStorageInfo& rInfo = pIt1->second;
    TStorageListenerList::iterator pIt2 = ::std::find(rInfo.Listener.begin(), rInfo.Listener.end(), pListener);
    if (pIt2 == rInfo.Listener.end())
        rInfo.Listener.push_back(pListener);
}

void StorageHolder::removeStorageListener(      XMLBasedAcceleratorConfiguration* pListener,
                                          const OUString&  sPath    )
{
    OUString sNormedPath = StorageHolder::impl_st_normPath(sPath);

    osl::MutexGuard g(m_mutex);

    TPath2StorageInfo::iterator pIt1 = m_lStorages.find(sNormedPath);
    if (pIt1 == m_lStorages.end())
        return;

    TStorageInfo& rInfo = pIt1->second;
    TStorageListenerList::iterator pIt2 = ::std::find(rInfo.Listener.begin(), rInfo.Listener.end(), pListener);
    if (pIt2 != rInfo.Listener.end())
        rInfo.Listener.erase(pIt2);
}

OUString StorageHolder::getPathOfStorage(const css::uno::Reference< css::embed::XStorage >& xStorage)
{
    osl::MutexGuard g(m_mutex);

    for (auto const& lStorage : m_lStorages)
    {
        const TStorageInfo& rInfo = lStorage.second;
        if (rInfo.Storage == xStorage)
            return lStorage.first;
    }

    return OUString();
}

css::uno::Reference< css::embed::XStorage > StorageHolder::getParentStorage(const css::uno::Reference< css::embed::XStorage >& xChild)
{
    OUString sChildPath = getPathOfStorage(xChild);
    return getParentStorage(sChildPath);
}

css::uno::Reference< css::embed::XStorage > StorageHolder::getParentStorage(const OUString& sChildPath)
{
    // normed path = "a/b/c/" ... we search for "a/b/"
    OUString sNormedPath = StorageHolder::impl_st_normPath(sChildPath);
    std::vector<OUString> lFolders    = StorageHolder::impl_st_parsePath(sNormedPath);
    sal_Int32       c           = lFolders.size();

    // a) ""       => -       => no parent
    // b) "a/b/c/" => "a/b/"  => return storage "a/b/"
    // c) "a/"     => ""      => return root !

    // a)
    if (c < 1)
        return css::uno::Reference< css::embed::XStorage >();

    // SAFE -> ----------------------------------
    {
        osl::MutexGuard aReadLock(m_mutex);

        // b)
        if (c < 2)
            return m_xRoot;

        // c)
        OUStringBuffer sParentPath(64);
        sal_Int32       i = 0;
        for (i = 0; i < c - 1; ++i)
        {
            sParentPath.append(lFolders[i] + PATH_SEPARATOR);
        }

        auto pParent = m_lStorages.find(sParentPath.makeStringAndClear());
        if (pParent != m_lStorages.end())
            return pParent->second.Storage;
    }
    // <- SAFE ----------------------------------

    // ?
    SAL_INFO("fwk", "StorageHolder::getParentStorage(): Unexpected situation. Cached storage item seems to be wrong.");
    return css::uno::Reference< css::embed::XStorage >();
}

StorageHolder& StorageHolder::operator=(const StorageHolder& rCopy)
{
    osl::MutexGuard g(m_mutex);
    m_xRoot     = rCopy.m_xRoot;
    m_lStorages = rCopy.m_lStorages;
    return *this;
}

css::uno::Reference< css::embed::XStorage > StorageHolder::openSubStorageWithFallback(const css::uno::Reference< css::embed::XStorage >& xBaseStorage  ,
                                                                                      const OUString&                             sSubStorage   ,
                                                                                            sal_Int32                                    eOpenMode)
{
    // a) try it first with user specified open mode
    //    ignore errors ... but save it for later use!
    try
    {
        css::uno::Reference< css::embed::XStorage > xSubStorage = xBaseStorage->openStorageElement(sSubStorage, eOpenMode);
        if (xSubStorage.is())
            return xSubStorage;
    }
    catch(const css::uno::RuntimeException&)
    {
        throw;
    }
    catch(const css::uno::Exception&)
    {
        // b) readonly already tried? => forward last error!
        if ((eOpenMode & css::embed::ElementModes::WRITE) != css::embed::ElementModes::WRITE) // fallback possible ?
            throw;
    }

    // b) readonly already tried, throw error
    if ((eOpenMode & css::embed::ElementModes::WRITE) != css::embed::ElementModes::WRITE) // fallback possible ?
        throw css::uno::Exception();

    // c) try it readonly
    //    don't catch exception here! Outside code wish to know, if operation failed or not.
    //    Otherwise they work on NULL references ...
    sal_Int32 eNewMode = (eOpenMode & ~css::embed::ElementModes::WRITE);
    css::uno::Reference< css::embed::XStorage > xSubStorage = xBaseStorage->openStorageElement(sSubStorage, eNewMode);
    if (xSubStorage.is())
        return xSubStorage;

    // d) no chance!
    SAL_INFO("fwk", "openSubStorageWithFallback(): Unexpected situation! Got no exception for missing storage ...");
    return css::uno::Reference< css::embed::XStorage >();
}

OUString StorageHolder::impl_st_normPath(const OUString& sPath)
{
    // path must start without "/" but end with "/"!

    OUString sNormedPath = sPath;

    // "/bla" => "bla" && "/" => "" (!)
    (void)sNormedPath.startsWith(PATH_SEPARATOR, &sNormedPath);

    // "/" => "" || "" => "" ?
    if (sNormedPath.isEmpty())
        return OUString();

    // "bla" => "bla/"
    if (sNormedPath.lastIndexOf(PATH_SEPARATOR_UNICODE) != (sNormedPath.getLength()-1))
        sNormedPath += PATH_SEPARATOR;

    return sNormedPath;
}

std::vector<OUString> StorageHolder::impl_st_parsePath(const OUString& sPath)
{
    std::vector<OUString> lToken;
    sal_Int32    i  = 0;
    while (true)
    {
        OUString sToken = sPath.getToken(0, PATH_SEPARATOR_UNICODE, i);
        if (i < 0)
            break;
        lToken.push_back(sToken);
    }
    return lToken;
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
