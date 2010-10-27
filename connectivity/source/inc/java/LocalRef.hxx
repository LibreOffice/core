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

#ifndef CONNECTIVITY_LOCALREF_HXX
#define CONNECTIVITY_LOCALREF_HXX

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <jvmaccess/virtualmachine.hxx>

//........................................................................
namespace connectivity { namespace jdbc
{
//........................................................................

    //====================================================================
    //= LocalRef
    //====================================================================
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
        LocalRef(LocalRef &); // not defined
        void operator =(LocalRef &); // not defined

    protected:
        JNIEnv& m_environment;
        T       m_object;
    };

//........................................................................
} } // namespace connectivity::jdbc
//........................................................................

#endif // CONNECTIVITY_LOCALREF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
