/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: EnvGuards.hxx,v $
 * $Revision: 1.3 $
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
