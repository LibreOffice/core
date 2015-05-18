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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_JAVA_GLOBALREF_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_JAVA_GLOBALREF_HXX

#include "java/LocalRef.hxx"
#include "java/lang/Object.hxx"

#include <jvmaccess/virtualmachine.hxx>


namespace connectivity { namespace jdbc
{
    /** helper class to hold a local ref to a JNI object
    */
    template< typename T >
    class GlobalRef
    {
    public:
        GlobalRef()
            :m_object( NULL )
        {
        }

        GlobalRef( const GlobalRef& _source )
            :m_object( NULL )
        {
            *this = _source;
        }

        GlobalRef& operator=( const GlobalRef& _source )
        {
            if ( &_source == this )
                return *this;

            SDBThreadAttach t;
            set( t.env(), _source.get() );
            return *this;
        }

        ~GlobalRef()
        {
            reset();
        }

        void reset()
        {
            if ( m_object != NULL )
            {
                SDBThreadAttach t;
                t.env().DeleteGlobalRef( m_object );
                m_object = NULL;
            }
        }

        void set( JNIEnv& _environment, T _object )
        {
            if ( m_object != NULL )
                _environment.DeleteGlobalRef( m_object );
            m_object = _object;
            if ( m_object )
                m_object = static_cast< T >( _environment.NewGlobalRef( m_object ) );
        }

        void set( LocalRef< T >& _object )
        {
            set( _object.env(), _object.release() );
        }

        T get() const
        {
            return m_object;
        }

        bool is() const
        {
            return m_object != NULL;
        }

    private:
        T   m_object;
    };



} }


#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_JAVA_GLOBALREF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
