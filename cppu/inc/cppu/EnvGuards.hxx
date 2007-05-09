/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: EnvGuards.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-09 13:32:54 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
