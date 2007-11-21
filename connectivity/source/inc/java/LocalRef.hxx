/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: LocalRef.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-21 15:07:06 $
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
