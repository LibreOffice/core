/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef CONNECTIVITY_GLOBALREF_HXX
#define CONNECTIVITY_GLOBALREF_HXX

#include "java/LocalRef.hxx"
#include "java/lang/Object.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <jvmaccess/virtualmachine.hxx>

//........................................................................
namespace connectivity { namespace jdbc
{
//........................................................................

    //====================================================================
    //= GlobalRef
    //====================================================================
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


//........................................................................
} } // namespace connectivity::jdbc
//........................................................................

#endif // CONNECTIVITY_GLOBALREF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
