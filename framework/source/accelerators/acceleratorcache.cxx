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

#include <com/sun/star/container/NoSuchElementException.hpp>

#include <vcl/svapp.hxx>

namespace framework
{
bool AcceleratorCache::hasKey(const css::awt::KeyEvent& aKey) const
{
    return (m_lKey2Commands.find(aKey) != m_lKey2Commands.end());
}

bool AcceleratorCache::hasCommand(const OUString& sCommand) const
{
    return (m_lCommand2Keys.find(sCommand) != m_lCommand2Keys.end());
}

AcceleratorCache::TKeyList AcceleratorCache::getAllKeys() const
{
    TKeyList lKeys;
    lKeys.reserve(m_lKey2Commands.size());

    for (auto const& key2Command : m_lKey2Commands)
    {
        lKeys.push_back(key2Command.first);
    }

    return lKeys;
}

void AcceleratorCache::setKeyCommandPair(const css::awt::KeyEvent& aKey, const OUString& sCommand)
{
    // register command for the specified key
    m_lKey2Commands[aKey] = sCommand;

    // update optimized structure to bind multiple keys to one command
    TKeyList& rKeyList = m_lCommand2Keys[sCommand];
    rKeyList.push_back(aKey);
}

AcceleratorCache::TKeyList AcceleratorCache::getKeysByCommand(const OUString& sCommand) const
{
    TCommand2Keys::const_iterator pCommand = m_lCommand2Keys.find(sCommand);
    if (pCommand == m_lCommand2Keys.end())
        throw css::container::NoSuchElementException();
    return pCommand->second;
}

OUString AcceleratorCache::getCommandByKey(const css::awt::KeyEvent& aKey) const
{
    TKey2Commands::const_iterator pKey = m_lKey2Commands.find(aKey);
    if (pKey == m_lKey2Commands.end())
        throw css::container::NoSuchElementException();
    return pKey->second;
}

void AcceleratorCache::removeKey(const css::awt::KeyEvent& aKey)
{
    // check if key exists
    TKey2Commands::const_iterator pKey = m_lKey2Commands.find(aKey);
    if (pKey == m_lKey2Commands.end())
        return;

    // get its registered command
    // Because we must know its place inside the optimized
    // structure, which bind keys to commands, too!
    OUString sCommand = pKey->second;
    pKey = m_lKey2Commands.end(); // nobody should use an undefined value .-)

    // remove key from primary list
    m_lKey2Commands.erase(aKey);

    // get keylist for that command
    TCommand2Keys::iterator pCommand = m_lCommand2Keys.find(sCommand);
    if (pCommand == m_lCommand2Keys.end())
        return;
    TKeyList& lKeys = pCommand->second;

    // one or more keys assign
    if (lKeys.size() == 1)
        // remove key from optimized command list
        m_lCommand2Keys.erase(sCommand);
    else // only remove this key from the keylist
    {
        auto pKeys = ::std::find(lKeys.begin(), lKeys.end(), aKey);

        if (pKeys != lKeys.end())
            lKeys.erase(pKeys);
    }
}

void AcceleratorCache::removeCommand(const OUString& sCommand)
{
    const TKeyList aKeys = getKeysByCommand(sCommand);
    for (auto const& lKey : aKeys)
    {
        removeKey(lKey);
    }
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
