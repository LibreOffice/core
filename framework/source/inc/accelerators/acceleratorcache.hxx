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

#pragma once

#include <stdtypes.h>

#include <com/sun/star/awt/KeyEvent.hpp>

#include <unordered_map>
#include <vector>

// definition

namespace framework
{

/**
    @short  implements a cache for any accelerator configuration.

    @descr  It's implemented threadsafe, supports copy-on-write pattern
            and a flush mechanism to support concurrent access to the same
            configuration.

            copy-on-write ... How? Do the following:
 */
class AcceleratorCache
{

    // const, types

    public:

        /** TODO document me
            commands -> keys
        */
        typedef ::std::vector< css::awt::KeyEvent > TKeyList;
        typedef std::unordered_map<OUString, TKeyList> TCommand2Keys;

        /** TODO document me
            keys -> commands
        */
        typedef std::unordered_map< css::awt::KeyEvent ,
                                    OUString    ,
                                    KeyEventHashCode   ,
                                    KeyEventEqualsFunc > TKey2Commands;

    // member

    private:

        /** map commands to keys in relation 1:n.
            First key is interpreted as preferred one! */
        TCommand2Keys m_lCommand2Keys;

        /** map keys to commands in relation 1:1. */
        TKey2Commands m_lKey2Commands;

    // interface

    public:
        /** @short  checks if the specified key exists.

            @param  aKey
                    the key, which should be checked.

            @return [bool]
                    sal_True if the specified key exists inside this container.
         */
        bool hasKey(const css::awt::KeyEvent& aKey) const;
        bool hasCommand(const OUString& sCommand) const;

        /** TODO document me */
        TKeyList getAllKeys() const;

        /** @short  add a new or change an existing key-command pair
                    of this container.

            @param  aKey
                    describe the key.

            @param  sCommand
                    describe the command.
          */
        void setKeyCommandPair(const css::awt::KeyEvent& aKey    ,
                                       const OUString&    sCommand);

        /** @short  returns the list of keys, which are registered
                    for this command.

            @param  sCommand
                    describe the command.

            @return [TKeyList]
                    the list of registered keys. Can be empty!
          */
        TKeyList getKeysByCommand(const OUString& sCommand) const;

        /** TODO */
        OUString getCommandByKey(const css::awt::KeyEvent& aKey) const;

        /** TODO */
        void removeKey(const css::awt::KeyEvent& aKey);
        void removeCommand(const OUString& sCommand);
};

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
