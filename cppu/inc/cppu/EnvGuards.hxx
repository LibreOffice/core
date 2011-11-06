/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef INCLUDED_cppu_EnvGuards_hxx
#define INCLUDED_cppu_EnvGuards_hxx

#include "uno/environment.hxx"
#include "uno/mapping.hxx"


namespace cssuno = com::sun::star::uno;


namespace cppu
{
    /** Environment Guard
        The provided Environment becomes entered in the constructor and left
        in the destructor.
        (http://wiki.services.openoffice.org/wiki/Uno/Cpp/Spec/Environment_Guard)

        @since UDK 3.2.7
    */
    class EnvGuard
    {
        cssuno::Environment m_env;

    public:
        explicit EnvGuard(cssuno::Environment const & env)
        {
            if (env.is())
            {
                m_env = cssuno::Environment::getCurrent();
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
        sal_Bool SAL_CALL is() const SAL_THROW( () )
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
        (http://wiki.services.openoffice.org/wiki/Uno/Cpp/Spec/Environment_AntiGuard)

        @since UDK 3.2.7
    */
    class AntiEnvGuard
    {
        cssuno::Environment m_env;

    public:
        explicit AntiEnvGuard()
            : m_env(cssuno::Environment::getCurrent())
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
