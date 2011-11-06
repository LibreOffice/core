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
