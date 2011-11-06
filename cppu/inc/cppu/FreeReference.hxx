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
