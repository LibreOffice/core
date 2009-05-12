/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FreeReference.hxx,v $
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

#ifndef INCLUDED_cppu_FreeReference_hxx
#define INCLUDED_cppu_FreeReference_hxx

#include "uno/environment.hxx"
#include "cppu/Map.hxx"
#include "com/sun/star/uno/Reference.h"


namespace cssuno = com::sun::star::uno;


namespace cppu
{
    /** Freely (environment independent) usable Reference.
        (http://wiki.services.openoffice.org/wiki/Uno/Cpp/Spec/FreeReference)

        @since UDK 3.2.7
    */
    template< class T >
    class FreeReference
    {
        cssuno::Environment    m_env;
        T                   *  m_pObject;

    public:
        FreeReference() : m_pObject(NULL) {}

        FreeReference(T * pObject, __sal_NoAcquire)
            : m_env(cssuno::Environment::getCurrent()),
              m_pObject(pObject)
        {
        }

        FreeReference(T * pObject)
            : m_env(cssuno::Environment::getCurrent()),
              m_pObject(pObject)
        {
            if (m_pObject)
                m_env.get()->pExtEnv->acquireInterface(m_env.get()->pExtEnv, m_pObject);
        }

        explicit FreeReference(cssuno::Reference<T> const & xRef)
            : m_env(cssuno::Environment::getCurrent()),
              m_pObject(xRef.get())
        {
            if (m_pObject)
                m_env.get()->pExtEnv->acquireInterface(m_env.get()->pExtEnv, m_pObject);
        }

        FreeReference(FreeReference<T> const & rOther)
            : m_env    (rOther.m_env),
              m_pObject(rOther.m_pObject)
        {
            if (m_pObject)
                m_env.get()->pExtEnv->acquireInterface(m_env.get()->pExtEnv, m_pObject);
        }


        ~FreeReference()
        {
            clear();
        }

        cssuno::Environment getEnv() const throw (cssuno::RuntimeException)
        {
            return m_env;
        }

        cssuno::Reference<T> get() const throw (cssuno::RuntimeException)
        {
            return cssuno::Reference<T>(cppu::mapIn(m_pObject, m_env), SAL_NO_ACQUIRE);
        }

        operator cssuno::Reference<T> () const throw (cssuno::RuntimeException)
        {
            return get();
        }

        cssuno::Reference<T> operator -> () const throw (cssuno::RuntimeException)
        {
            return get();
        }

        bool is() const throw (cssuno::RuntimeException)
        {
            return m_pObject != NULL;
        }

        void clear()
        {
            if (m_pObject)
            {

                m_env.get()->pExtEnv->releaseInterface(m_env.get()->pExtEnv, m_pObject);
                m_pObject = NULL;
                m_env.clear();
            }
        }

        FreeReference<T> & operator = (FreeReference<T> const & rOther)
        {
            clear();

            m_pObject = rOther.m_pObject;
            if (m_pObject)
            {
                m_env     = rOther.m_env;
                m_env.get()->pExtEnv->acquireInterface(m_env.get()->pExtEnv, m_pObject);
            }

            return *this;
        }

        void set(cssuno::Reference<T> const & xRef)
        {
            clear();

            m_pObject = xRef.get();
            if (m_pObject)
            {
                m_env = cssuno::Environment::getCurrent();

                m_env.get()->pExtEnv->acquireInterface(m_env.get()->pExtEnv, m_pObject);
            }
        }

        bool operator == (FreeReference const & rOther) const
        {
            return get() == rOther.get();
        }

        bool operator != (FreeReference const & rOther) const
        {
            return !operator==(rOther);
        }
    };
}

#endif
