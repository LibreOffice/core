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

#ifndef INCLUDED_VCL_LAZYDELETE_HXX
#define INCLUDED_VCL_LAZYDELETE_HXX

#include <vcl/dllapi.h>
#include <vcl/vclptr.hxx>
#include <vcl/window.hxx>

#include <unordered_map>
#include <vector>
#include <algorithm>

#if OSL_DEBUG_LEVEL > 2
#include <typeinfo>
#include <stdio.h>
#endif

#include <com/sun/star/lang/XComponent.hpp>

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

    class VCL_DLLPUBLIC LazyDeletor : public LazyDeletorBase
    {
        static LazyDeletor*     s_pOneInstance;

        struct DeleteObjectEntry
        {
            VclPtr<vcl::Window> m_pObject;
            bool      m_bDeleted;

            DeleteObjectEntry() :
                m_pObject( nullptr ),
                m_bDeleted( false )
            {}

            DeleteObjectEntry( vcl::Window* i_pObject ) :
                m_pObject( i_pObject ),
                m_bDeleted( false )
            {}
        };

        std::vector< DeleteObjectEntry >    m_aObjects;
        typedef std::unordered_map< sal_IntPtr, unsigned int > PtrToIndexMap;
        PtrToIndexMap                       m_aPtrToIndex;

        /** strict weak ordering function to bring objects to be destroyed lazily
        in correct order, e.g. for Window objects children before parents
        */
        static bool is_less( vcl::Window* left, vcl::Window* right );

        LazyDeletor()  { LazyDelete::addDeletor( this ); }
        virtual ~LazyDeletor()
        {
            #if OSL_DEBUG_LEVEL > 2
            fprintf( stderr, "%s %p deleted\n",
                     typeid(*this).name(), this );
            #endif
            if( s_pOneInstance == this ) // sanity check
                s_pOneInstance = nullptr;

            // do the actual work
            unsigned int nCount = m_aObjects.size();
            std::vector< VclPtr < vcl::Window > > aRealDelete;
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
                if( ! m_aObjects[ m_aPtrToIndex[ reinterpret_cast<sal_IntPtr>(aRealDelete[n].get()) ] ].m_bDeleted )
                    aRealDelete[n].disposeAndClear();
            }
        }

        public:
        /** mark an object for lazy deletion
        */
        static void Delete( vcl::Window* i_pObject )
        {
            if( s_pOneInstance == nullptr )
                s_pOneInstance = new LazyDeletor();

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
        static void Undelete( vcl::Window* i_pObject )
        {
            if( s_pOneInstance )
            {
                PtrToIndexMap::const_iterator it = s_pOneInstance->m_aPtrToIndex.find( reinterpret_cast<sal_IntPtr>(i_pObject) );
                if( it != s_pOneInstance->m_aPtrToIndex.end() )
                    s_pOneInstance->m_aObjects[ it->second ].m_bDeleted = true;
            }
        }
    };

    /*
    class DeleteOnDeinit matches a similar need as LazyDelete for static objects:
    you may not access vcl objects after DeInitVCL has been called this includes their destruction
    therefore disallowing the existence of static vcl object like e.g. a static BitmapEx
    To work around this use DeleteOnDeinit<BitmapEx> which will allow you to have a static object container,
    that will have its contents destroyed on DeinitVCL. The single drawback is that you need to check on the
    container object whether it still contains content before actually accessing it.

    caveat: when constructing a vcl object, you certainly want to ensure that InitVCL has run already.
    However this is not necessarily the case when using a class static member or a file level static variable.
    In these cases make judicious use of the set() method of DeleteOnDeinit, but beware of the changing
    ownership.

    example use case: use a lazy initialized on call BitmapEx in a paint method. Of course a paint method
    would not normally be called after DeInitVCL anyway, so the check might not be necessary in a
    Window::Paint implementation, but always checking is a good idea.

    SomeWindow::Paint()
    {
        static vcl::DeleteOnDeinit< BitmapEx > aBmp( new BitmapEx( ResId( 1000, myResMgr ) ) );

        if( aBmp.get() ) // check whether DeInitVCL has been called already
            DrawBitmapEx( Point( 10, 10 ), *aBmp.get() );
    }
    */

    class VCL_DLLPUBLIC DeleteOnDeinitBase
    {
    public:
        static void SAL_DLLPRIVATE ImplDeleteOnDeInit();
        virtual ~DeleteOnDeinitBase();
    protected:
        static void addDeinitContainer( DeleteOnDeinitBase* i_pContainer );

        virtual void doCleanup() = 0;
    };

    template < typename T >
    class DeleteOnDeinit : public DeleteOnDeinitBase
    {
        T* m_pT;
        virtual void doCleanup() override { delete m_pT; m_pT = NULL; }
    public:
        DeleteOnDeinit( T* i_pT ) : m_pT( i_pT ) { addDeinitContainer( this ); }
        virtual ~DeleteOnDeinit() {}

        // get contents
        T* get() { return m_pT; }

        // set contents, returning old contents
        // ownership is transferred !
        T* set( T* i_pNew ) { T* pOld = m_pT; m_pT = i_pNew; return pOld; }

        // set contents, deleting old contents
        // ownership is transferred !
        void reset( T* i_pNew = NULL )
            { OSL_ASSERT( i_pNew != m_pT || i_pNew == NULL ); T* pOld = m_pT; m_pT = i_pNew; delete pOld; }
    };

    /** Similar to DeleteOnDeinit, the DeleteUnoReferenceOnDeinit
        template class makes sure that a static UNO object is disposed
        and released at the right time.

        Use like
            static DeleteUnoReferenceOnDeinit<lang::XMultiServiceFactory>
                xStaticFactory (\<create factory object>);
            Reference<lang::XMultiServiceFactory> xFactory (xStaticFactory.get());
            if (xFactory.is())
                \<do something with xFactory>
    */
    template <typename I>
    class DeleteUnoReferenceOnDeinit : public vcl::DeleteOnDeinitBase
    {
        css::uno::Reference<I> m_xI;
        virtual void doCleanup() override { set(NULL); }
    public:
        DeleteUnoReferenceOnDeinit(const css::uno::Reference<I>& r_xI ) : m_xI( r_xI ) {
            addDeinitContainer( this ); }
        virtual ~DeleteUnoReferenceOnDeinit() {}

        css::uno::Reference<I> get() { return m_xI; }

        void set (const css::uno::Reference<I>& r_xNew )
        {
            css::uno::Reference< css::lang::XComponent> xComponent (m_xI, css::uno::UNO_QUERY);
            m_xI = r_xNew;
            if (xComponent.is()) try
            {
                xComponent->dispose();
            }
            catch( css::uno::Exception& )
            {
            }
        }
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
