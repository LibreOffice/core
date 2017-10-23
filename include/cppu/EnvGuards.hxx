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

#ifndef INCLUDED_CPPU_ENVGUARDS_HXX
#define INCLUDED_CPPU_ENVGUARDS_HXX

#include "uno/environment.hxx"


namespace cppu
{
    /** Environment Guard
        The provided Environment becomes entered in the constructor and left
        in the destructor.
        (http://wiki.openoffice.org/wiki/Uno/Cpp/Spec/Environment_Guard)

        @since UDK 3.2.7
    */
    class EnvGuard
    {
        css::uno::Environment m_env;

    public:
        explicit EnvGuard(css::uno::Environment const & env)
        {
            if (env.is())
            {
                m_env = css::uno::Environment::getCurrent();
                env.enter();
            }
        }

        ~EnvGuard()
        {
            m_env.enter();
        }

        /** Checks if the associated environment is non empty.

            @return  0 == empty, 1 == non empty
        */
        bool SAL_CALL is() const
        {
            return m_env.is();
        }

        /** Leaves the associated environment and clears
            the reference.
        */
        void clear()
        {
            if (m_env.is())
            {
                m_env.enter();
                m_env.clear();
            }
        }
    };

    /** Environment Anti-Guard
        Any entered Environment becomes left in the constructor and re-entered
        in the destructor.
        (http://wiki.openoffice.org/wiki/Uno/Cpp/Spec/Environment_AntiGuard)

        @since UDK 3.2.7
    */
    class AntiEnvGuard
    {
        css::uno::Environment m_env;

    public:
        explicit AntiEnvGuard()
            : m_env(css::uno::Environment::getCurrent())
        {
            uno_Environment_enter(NULL);
        }

        ~AntiEnvGuard()
        {
            m_env.enter();
        }
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
