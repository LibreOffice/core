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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_JAVA_LOCALREF_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_JAVA_LOCALREF_HXX

#include <jvmaccess/virtualmachine.hxx>


namespace connectivity { namespace jdbc
{

    /** helper class to hold a local ref to a JNI object

        Note that this class never actually calls NewLocalRef. It is assumed that all objects
        passed are already acquired with a local ref (as it usually is the case if you obtain
        the object from an JNI method).
    */
    template< typename T >
    class LocalRef
    {
    public:
        explicit LocalRef( JNIEnv& environment )
            :m_environment( environment )
            ,m_object( NULL )
        {
        }

        LocalRef( JNIEnv& environment, T object )
            :m_environment( environment )
            ,m_object( object )
        {
        }

        ~LocalRef()
        {
            reset();
        }

        T release()
        {
            T t = m_object;
            m_object = NULL;
            return t;
        }

        void set( T object ) { reset(); m_object = object; }

        void reset()
        {
            if ( m_object != NULL )
            {
                m_environment.DeleteLocalRef( m_object );
                m_object = NULL;
            }
        }

        JNIEnv& env() const { return m_environment; }
        T       get() const { return m_object; }
        bool    is()  const { return m_object != NULL; }

    private:
        LocalRef(LocalRef &) = delete;
        void operator =(LocalRef &) = delete;

    protected:
        JNIEnv& m_environment;
        T       m_object;
    };


} } // namespace connectivity::jdbc


#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_JAVA_LOCALREF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
