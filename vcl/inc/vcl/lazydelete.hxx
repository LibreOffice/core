/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: lazydelete.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _VCL_LAZYDELETE_HXX
#define _VCL_LAZYDELETE_HXX

#include "dllapi.h"

#include <vector>
#include <hash_map>
#include <algorithm>

#if OSL_DEBUG_LEVEL > 2
#include <typeinfo>
#include <stdio.h>
#endif

namespace vcl
{
    /* Helpers for lazy object deletion

    With vcl it is often necessary to delete objects (especially Windows)
    in the right order as well as in a way ensuring that the deleted objects
    are not still on the stack (e.g. deleting a Window in its key handler). To
    make this easier a helper class is given here which takes care of both
    sorting as well as lazy deletion.

    The grisly details:
    LazyDelete is a class that LazyDeletor register to. When vcl's event
    loop (that is Application::Yield or Application::Reschedule) comes out
    of the last level, the LazyDelete::flush is called. This will cause
    LazyDelete to delete all registered LazyDeletor objects.

    LazyDeletor<T> is a one instance object that contains a list of
    <T> objects to be deleted in sorted order. It is derived from
    LazyDeletorBase as to be able to register itself in LazyDelete.

    The user calls the static method LazyDeletor<T>::Delete( T* ) with the
    object to be destroyed lazy. The static method creates the LazyDeletor<T>
    (which in turn registers itself in LazyDelete) if this is the first time
    a T* is to be destroyed lazy. It then inserts the object. When the LazyDeletor<T>
    gets delte it will delete the stored objects in a fashion
    that will ensure the correct order of deletion via the specialized is_less method
    (e.g. if a Window is a child of another Window and therefore should be destroyed
    first it is "less" in this sense)

    LazyDelete::flush will be called when the top of the nested event loop is
    reached again and will then destroy each registered LazyDeletor<T> which
    in turn destroys the objects needed to be destroyed lazily. After this
    the state is as before entering the event loop.

    Preconditions:
    - The class <T> of which objects are to be destroyed needs a virtual
    destructor or must be final, else the wrong type will be destroyed.
    - The destructor of <T> should call LazyDeletor<T>::Undelete( this ). This
    prevents duplicate deletionin case someone destroys the object prematurely.
    */

    class LazyDeletorBase;
    class VCL_DLLPUBLIC LazyDelete
    {
        public:
        /** flush all registered object lists
        */
        static void flush();
        /** register an object list to be destroyed
        */
        static void addDeletor( LazyDeletorBase* pDeletor );
    };

    class VCL_DLLPUBLIC LazyDeletorBase
    {
        friend void LazyDelete::flush();
        protected:
        LazyDeletorBase();
        virtual ~LazyDeletorBase();
    };

    template < typename T >
    class VCL_DLLPUBLIC LazyDeletor : public LazyDeletorBase
    {
        static LazyDeletor< T >*     s_pOneInstance;

        struct DeleteObjectEntry
        {
            T*      m_pObject;
            bool    m_bDeleted;

            DeleteObjectEntry() :
                m_pObject( NULL ),
                m_bDeleted( false )
            {}

            DeleteObjectEntry( T* i_pObject ) :
                m_pObject( i_pObject ),
                m_bDeleted( false )
            {}
        };

        std::vector< DeleteObjectEntry >    m_aObjects;
        typedef std::hash_map< sal_IntPtr, unsigned int > PtrToIndexMap;
        PtrToIndexMap                       m_aPtrToIndex;

        /** strict weak ordering funtion to bring objects to be destroyed lazily
        in correct order, e.g. for Window objects children before parents
        */
        static bool is_less( T* left, T* right );

        LazyDeletor()  { LazyDelete::addDeletor( this ); }
        virtual ~LazyDeletor()
        {
            #if OSL_DEBUG_LEVEL > 2
            fprintf( stderr, "%s %p deleted\n",
                     typeid(*this).name(), this );
            #endif
            if( s_pOneInstance == this ) // sanity check
                s_pOneInstance = NULL;

            // do the actual work
            unsigned int nCount = m_aObjects.size();
            std::vector<T*> aRealDelete;
            aRealDelete.reserve( nCount );
            for( unsigned int i = 0; i < nCount; i++ )
            {
                if( ! m_aObjects[i].m_bDeleted )
                {
                    aRealDelete.push_back( m_aObjects[i].m_pObject );
                }
            }
            // sort the vector of objects to be destroyed
            std::sort( aRealDelete.begin(), aRealDelete.end(), is_less );
            nCount = aRealDelete.size();
            for( unsigned int n = 0; n < nCount; n++ )
            {
                #if OSL_DEBUG_LEVEL > 2
                fprintf( stderr, "%s deletes object %p of type %s\n",
                         typeid(*this).name(),
                         aRealDelete[n],
                         typeid(*aRealDelete[n]).name() );
                #endif
                // check if the object to be deleted is not already destroyed
                // as a side effect of a previous lazily destroyed object
                if( ! m_aObjects[ m_aPtrToIndex[ reinterpret_cast<sal_IntPtr>(aRealDelete[n]) ] ].m_bDeleted )
                    delete aRealDelete[n];
            }
        }

        public:
        /** mark an object for lazy deletion
        */
        static void Delete( T* i_pObject )
        {
            if( s_pOneInstance == NULL )
                s_pOneInstance = new LazyDeletor<T>();

            // is this object already in the list ?
            // if so mark it as not to be deleted; else insert it
            PtrToIndexMap::const_iterator dup = s_pOneInstance->m_aPtrToIndex.find( reinterpret_cast<sal_IntPtr>(i_pObject) );
            if( dup != s_pOneInstance->m_aPtrToIndex.end() )
            {
                s_pOneInstance->m_aObjects[ dup->second ].m_bDeleted = false;
            }
            else
            {
                s_pOneInstance->m_aPtrToIndex[ reinterpret_cast<sal_IntPtr>(i_pObject) ] = s_pOneInstance->m_aObjects.size();
                s_pOneInstance->m_aObjects.push_back( DeleteObjectEntry( i_pObject ) );
            }
        }
        /** unmark an object already marked for lazy deletion
        */
        static void Undelete( T* i_pObject )
        {
            if( s_pOneInstance )
            {
                PtrToIndexMap::const_iterator it = s_pOneInstance->m_aPtrToIndex.find( reinterpret_cast<sal_IntPtr>(i_pObject) );
                if( it != s_pOneInstance->m_aPtrToIndex.end() )
                    s_pOneInstance->m_aObjects[ it->second ].m_bDeleted = true;
            }
        }
    };
}

#endif

