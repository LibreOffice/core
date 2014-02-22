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

#include <accelerators/acceleratorcache.hxx>

#include <xml/acceleratorconfigurationreader.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>

#include <com/sun/star/container/ElementExistException.hpp>

#include <com/sun/star/container/NoSuchElementException.hpp>

#include <vcl/svapp.hxx>

namespace framework
{


AcceleratorCache::AcceleratorCache()
    : ThreadHelpBase(&Application::GetSolarMutex())
{
}


AcceleratorCache::AcceleratorCache(const AcceleratorCache& rCopy)
    : ThreadHelpBase(&Application::GetSolarMutex())
{
    m_lCommand2Keys = rCopy.m_lCommand2Keys;
    m_lKey2Commands = rCopy.m_lKey2Commands;
}


AcceleratorCache::~AcceleratorCache()
{
    
    
}


void AcceleratorCache::takeOver(const AcceleratorCache& rCopy)
{
    
    WriteGuard aWriteLock(m_aLock);

    m_lCommand2Keys = rCopy.m_lCommand2Keys;
    m_lKey2Commands = rCopy.m_lKey2Commands;

    aWriteLock.unlock();
    
}


AcceleratorCache& AcceleratorCache::operator=(const AcceleratorCache& rCopy)
{
    takeOver(rCopy);
    return *this;
}


sal_Bool AcceleratorCache::hasKey(const css::awt::KeyEvent& aKey) const
{
    
    ReadGuard aReadLock(m_aLock);

    return (m_lKey2Commands.find(aKey) != m_lKey2Commands.end());
    
}


sal_Bool AcceleratorCache::hasCommand(const OUString& sCommand) const
{
    
    ReadGuard aReadLock(m_aLock);

    return (m_lCommand2Keys.find(sCommand) != m_lCommand2Keys.end());
    
}


AcceleratorCache::TKeyList AcceleratorCache::getAllKeys() const
{
    TKeyList lKeys;

    
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
    

    return lKeys;
}


void AcceleratorCache::setKeyCommandPair(const css::awt::KeyEvent& aKey    ,
                                         const OUString&    sCommand)
{
    
    WriteGuard aWriteLock(m_aLock);

    
    m_lKey2Commands[aKey] = sCommand;

    
    TKeyList& rKeyList = m_lCommand2Keys[sCommand];
    rKeyList.push_back(aKey);

    aWriteLock.unlock();
    
}


AcceleratorCache::TKeyList AcceleratorCache::getKeysByCommand(const OUString& sCommand) const
{
    TKeyList lKeys;

    
    ReadGuard aReadLock(m_aLock);

    TCommand2Keys::const_iterator pCommand = m_lCommand2Keys.find(sCommand);
    if (pCommand == m_lCommand2Keys.end())
        throw css::container::NoSuchElementException(
                OUString(), css::uno::Reference< css::uno::XInterface >());
    lKeys = pCommand->second;

    aReadLock.unlock();
    

    return lKeys;
}


OUString AcceleratorCache::getCommandByKey(const css::awt::KeyEvent& aKey) const
{
    OUString sCommand;

    
    ReadGuard aReadLock(m_aLock);

    TKey2Commands::const_iterator pKey = m_lKey2Commands.find(aKey);
    if (pKey == m_lKey2Commands.end())
        throw css::container::NoSuchElementException(
                OUString(), css::uno::Reference< css::uno::XInterface >());
    sCommand = pKey->second;

    aReadLock.unlock();
    

    return sCommand;
}


void AcceleratorCache::removeKey(const css::awt::KeyEvent& aKey)
{
    
    WriteGuard aWriteLock(m_aLock);

    
    TKey2Commands::const_iterator pKey = m_lKey2Commands.find(aKey);
    if (pKey == m_lKey2Commands.end())
        return;

    
    
    
    OUString sCommand = pKey->second;
    pKey = m_lKey2Commands.end(); 

    
    m_lKey2Commands.erase(aKey);

    
    m_lCommand2Keys.erase(sCommand);

    aWriteLock.unlock();
    
}


void AcceleratorCache::removeCommand(const OUString& sCommand)
{
    
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
    
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
