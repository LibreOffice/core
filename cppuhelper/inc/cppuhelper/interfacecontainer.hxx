/*************************************************************************
 *
 *  $RCSfile: interfacecontainer.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:26:09 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#define _CPPUHELPER_INTERFACECONTAINER_HXX_

#include <cppuhelper/interfacecontainer.h>


#define CONT_HASHMAP ::std::hash_map< key , void* , hashImpl , equalImpl >

namespace cppu
{

template< class key , class hashImpl , class equalImpl >
inline OMultiTypeInterfaceContainerHelperVar< key , hashImpl , equalImpl >::OMultiTypeInterfaceContainerHelperVar( ::osl::Mutex & rMutex_ )
    : rMutex( rMutex_ )
{
    m_pMap = new CONT_HASHMAP;
}

//===================================================================
template< class key , class hashImpl , class equalImpl >
inline OMultiTypeInterfaceContainerHelperVar< key , hashImpl , equalImpl >::~OMultiTypeInterfaceContainerHelperVar()
{
    CONT_HASHMAP::iterator iter     = m_pMap->begin();
    CONT_HASHMAP::iterator end  = m_pMap->end();

    while( iter != end )
    {
        delete (OInterfaceContainerHelper*)(*iter).second;
        (*iter).second = 0;
        ++iter;
    }
    delete m_pMap;
}

//===================================================================
template< class key , class hashImpl , class equalImpl >
inline ::com::sun::star::uno::Sequence< key > OMultiTypeInterfaceContainerHelperVar< key , hashImpl , equalImpl >::getContainedTypes() const
{
    CONT_HASHMAP::size_type nSize;

    ::osl::MutexGuard aGuard( rMutex );
    if( nSize = m_pMap->size() )
    {
        ::com::sun::star::uno::Sequence< key > aInterfaceTypes( nSize );
        key * pArray = aInterfaceTypes.getArray();

        CONT_HASHMAP::iterator iter = m_pMap->begin();
        CONT_HASHMAP::iterator end = m_pMap->end();

        sal_Int32 i = 0;
        while( iter != end )
        {
            // are interfaces added to this container?
            if( ((OInterfaceContainerHelper*)(*iter).second)->getLength() )
                // yes, put the type in the array
                pArray[i++] = (*iter).first;
            iter++;
        }
        if( i != nSize ) {
            // may be empty container, reduce the sequence to the right size
            aInterfaceTypes = ::com::sun::star::uno::Sequence<key>( pArray, i );
        }
        return aInterfaceTypes;
    }
    return ::com::sun::star::uno::Sequence<key>();
}

//===================================================================
template< class key , class hashImpl , class equalImpl >
OInterfaceContainerHelper * OMultiTypeInterfaceContainerHelperVar< key , hashImpl , equalImpl >::getContainer
(
    const key & rKey
) const
{
    ::osl::MutexGuard aGuard( rMutex );

     CONT_HASHMAP::iterator iter = m_pMap->find( rKey );
    if( iter != m_pMap->end() )
            return (OInterfaceContainerHelper*) (*iter).second;
    return 0;
}

//===================================================================
template< class key , class hashImpl , class equalImpl >
sal_Int32 OMultiTypeInterfaceContainerHelperVar< key , hashImpl , equalImpl >::addInterface
(
 const key & rKey,
 const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & rListener
)
{
    ::osl::MutexGuard aGuard( rMutex );
    CONT_HASHMAP::iterator
             iter = m_pMap->find( rKey );
    if( iter == m_pMap->end() )
    {
        OInterfaceContainerHelper * pLC = new OInterfaceContainerHelper( rMutex );
        (*m_pMap)[rKey] = pLC;
        return pLC->addInterface( rListener );
    }
    else
        return ((OInterfaceContainerHelper*)(*iter).second)->addInterface( rListener );
}

//===================================================================
template< class key , class hashImpl , class equalImpl >
inline sal_Int32 OMultiTypeInterfaceContainerHelperVar< key , hashImpl , equalImpl >::removeInterface
(
    const key & rKey,
    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & rListener
)
{
    ::osl::MutexGuard aGuard( rMutex );

    // search container with id nUik
    CONT_HASHMAP::iterator iter = m_pMap->find( rKey );
        // container found?
    if( iter != m_pMap->end() )
            return ((OInterfaceContainerHelper*)(*iter).second)->removeInterface( rListener );

    // no container with this id. Always return 0
    return 0;
}

//===================================================================
template< class key , class hashImpl , class equalImpl >
void OMultiTypeInterfaceContainerHelperVar< key , hashImpl , equalImpl >::disposeAndClear( const ::com::sun::star::lang::EventObject & rEvt )
{
    CONT_HASHMAP::size_type nSize = 0;
    OInterfaceContainerHelper ** ppListenerContainers = NULL;
    {
        ::osl::MutexGuard aGuard( rMutex );
        if( nSize = m_pMap->size() )
        {
            typedef OInterfaceContainerHelper* ppp;
            ppListenerContainers = new ppp[nSize];
            //ppListenerContainers = new (ListenerContainer*)[nSize];

            CONT_HASHMAP::iterator iter = m_pMap->begin();
            CONT_HASHMAP::iterator end = m_pMap->end();

            CONT_HASHMAP::size_type i = 0;
            while( iter != end )
            {
                ppListenerContainers[i++] = (OInterfaceContainerHelper*)(*iter).second;
                ++iter;
            }
        }
    }

    // create a copy, because do not fire event in a guarded section
    for( CONT_HASHMAP::size_type i = 0;
            i < nSize; i++ )
    {
        if( ppListenerContainers[i] )
            ppListenerContainers[i]->disposeAndClear( rEvt );
    }

    delete [] ppListenerContainers;
}

//===================================================================
template< class key , class hashImpl , class equalImpl >
void OMultiTypeInterfaceContainerHelperVar< key , hashImpl , equalImpl >::clear()
{
    ::osl::MutexGuard aGuard( rMutex );
    CONT_HASHMAP::iterator iter = m_pMap->begin();
    CONT_HASHMAP::iterator end = m_pMap->end();

    while( iter != end )
    {
        ((OInterfaceContainerHelper*)(*iter)).second->clear();
        ++iter;
    }
}


}

#endif

