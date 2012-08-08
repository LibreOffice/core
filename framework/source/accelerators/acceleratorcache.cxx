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

#include <accelerators/acceleratorcache.hxx>

#include <xml/acceleratorconfigurationreader.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>

#include <com/sun/star/container/ElementExistException.hpp>

#include <com/sun/star/container/NoSuchElementException.hpp>

#include <vcl/svapp.hxx>

namespace framework
{

//-----------------------------------------------
AcceleratorCache::AcceleratorCache()
    : ThreadHelpBase(&Application::GetSolarMutex())
{
}

//-----------------------------------------------
AcceleratorCache::AcceleratorCache(const AcceleratorCache& rCopy)
    : ThreadHelpBase(&Application::GetSolarMutex())
{
    m_lCommand2Keys = rCopy.m_lCommand2Keys;
    m_lKey2Commands = rCopy.m_lKey2Commands;
}

//-----------------------------------------------
AcceleratorCache::~AcceleratorCache()
{
    // Dont save anything automaticly here.
    // The user has to do that explicitly!
}

//-----------------------------------------------
void AcceleratorCache::takeOver(const AcceleratorCache& rCopy)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    m_lCommand2Keys = rCopy.m_lCommand2Keys;
    m_lKey2Commands = rCopy.m_lKey2Commands;

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
AcceleratorCache& AcceleratorCache::operator=(const AcceleratorCache& rCopy)
{
    takeOver(rCopy);
    return *this;
}

//-----------------------------------------------
sal_Bool AcceleratorCache::hasKey(const css::awt::KeyEvent& aKey) const
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    return (m_lKey2Commands.find(aKey) != m_lKey2Commands.end());
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
sal_Bool AcceleratorCache::hasCommand(const ::rtl::OUString& sCommand) const
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    return (m_lCommand2Keys.find(sCommand) != m_lCommand2Keys.end());
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
AcceleratorCache::TKeyList AcceleratorCache::getAllKeys() const
{
    TKeyList lKeys;

    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    lKeys.reserve(m_lKey2Commands.size());

    TKey2Commands::const_iterator pIt;
    TKey2Commands::const_iterator pEnd = m_lKey2Commands.end();
    for (  pIt  = m_lKey2Commands.begin();
           pIt != pEnd  ;
         ++pIt                           )
    {
        lKeys.push_back(pIt->first);
    }

    aReadLock.unlock();
    // <- SAFE ----------------------------------

    return lKeys;
}

//-----------------------------------------------
void AcceleratorCache::setKeyCommandPair(const css::awt::KeyEvent& aKey    ,
                                         const ::rtl::OUString&    sCommand)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    // register command for the specified key
    m_lKey2Commands[aKey] = sCommand;

    // update optimized structure to bind multiple keys to one command
    TKeyList& rKeyList = m_lCommand2Keys[sCommand];
    rKeyList.push_back(aKey);

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
AcceleratorCache::TKeyList AcceleratorCache::getKeysByCommand(const ::rtl::OUString& sCommand) const
{
    TKeyList lKeys;

    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    TCommand2Keys::const_iterator pCommand = m_lCommand2Keys.find(sCommand);
    if (pCommand == m_lCommand2Keys.end())
        throw css::container::NoSuchElementException(
                ::rtl::OUString(), css::uno::Reference< css::uno::XInterface >());
    lKeys = pCommand->second;

    aReadLock.unlock();
    // <- SAFE ----------------------------------

    return lKeys;
}

//-----------------------------------------------
::rtl::OUString AcceleratorCache::getCommandByKey(const css::awt::KeyEvent& aKey) const
{
    ::rtl::OUString sCommand;

    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    TKey2Commands::const_iterator pKey = m_lKey2Commands.find(aKey);
    if (pKey == m_lKey2Commands.end())
        throw css::container::NoSuchElementException(
                ::rtl::OUString(), css::uno::Reference< css::uno::XInterface >());
    sCommand = pKey->second;

    aReadLock.unlock();
    // <- SAFE ----------------------------------

    return sCommand;
}

//-----------------------------------------------
void AcceleratorCache::removeKey(const css::awt::KeyEvent& aKey)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    // check if key exists
    TKey2Commands::const_iterator pKey = m_lKey2Commands.find(aKey);
    if (pKey == m_lKey2Commands.end())
        return;

    // get its registered command
    // Because we must know its place inside the optimized
    // structure, which bind keys to commands, too!
    ::rtl::OUString sCommand = pKey->second;
    pKey = m_lKey2Commands.end(); // nobody should use an undefined value .-)

    // remove key from primary list
    m_lKey2Commands.erase(aKey);

    // remove key from optimized command list
    m_lCommand2Keys.erase(sCommand);

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
void AcceleratorCache::removeCommand(const ::rtl::OUString& sCommand)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    const TKeyList&                            lKeys = getKeysByCommand(sCommand);
    AcceleratorCache::TKeyList::const_iterator pKey ;
    for (  pKey  = lKeys.begin();
           pKey != lKeys.end()  ;
         ++pKey                 )
    {
        const css::awt::KeyEvent& rKey = *pKey;
        removeKey(rKey);
    }
    m_lCommand2Keys.erase(sCommand);

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
