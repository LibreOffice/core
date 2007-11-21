/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: GlobalRef.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-21 15:06:54 $
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
