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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"
#include <accelerators/acceleratorcache.hxx>

//_______________________________________________
// own includes

#include <xml/acceleratorconfigurationreader.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>

//_______________________________________________
// interface includes

#include <com/sun/star/container/ElementExistException.hpp>

#include <com/sun/star/container/NoSuchElementException.hpp>

//_______________________________________________
// other includes
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
