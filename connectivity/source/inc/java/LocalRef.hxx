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

#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-attributes"
#endif
#include <jni.h>
#if defined __clang__
#pragma clang diagnostic pop
#endif

namespace connectivity::jdbc
{

    /** helper class to hold a local ref to a JNI object

        Note that this class never actually calls NewLocalRef. It is assumed that all objects
        passed are already acquired with a local ref (as it usually is the case if you obtain
        the object from a JNI method).
    */
    template< typename T >
    class LocalRef final
    {
    public:
        explicit LocalRef( JNIEnv& environment )
            :m_environment( environment )
            ,m_object( nullptr )
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
            m_object = nullptr;
            return t;
        }

        void set( T object ) { reset(); m_object = object; }

        void reset()
        {
            if ( m_object != nullptr )
            {
                m_environment.DeleteLocalRef( m_object );
                m_object = nullptr;
            }
        }

        JNIEnv& env() const { return m_environment; }
        T       get() const { return m_object; }
        bool    is()  const { return m_object != nullptr; }

    private:
        LocalRef(LocalRef const &) = delete;
        LocalRef& operator =(LocalRef const &) = delete;

        JNIEnv& m_environment;
        T       m_object;
    };


} // namespace connectivity::jdbc


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
