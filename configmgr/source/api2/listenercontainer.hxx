/*************************************************************************
 *
 *  $RCSfile: listenercontainer.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jb $ $Date: 2000-11-10 12:22:55 $
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

#ifndef CONFIGMGR_API_LISTENERCONTAINER_HXX_
#define CONFIGMGR_API_LISTENERCONTAINER_HXX_

#include "apitypes.hxx"

#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif

#include <osl/diagnose.h>

namespace configmgr
{
    namespace configapi
    {
/////////////////////////////////////////////////////////////////////////////////////////////

        namespace css   = ::com::sun::star;
        namespace uno       = css::uno;
        namespace lang      = css::lang;

//-----------------------------------------------------------------------------
        typedef cppu::OInterfaceContainerHelper ListenerContainer;

        template <class Listener>
        class ListenerContainerIterator
        {
        public:
            /**
             * Create an iterator over the elements of the container. The iterator
             * copies the elements of the conatainer. A change to the container does not
             * affect the iterator.<BR>
             * Remark: The copy is on demand. The iterator copy the elements only if the container
             * change the contens. It is not allowed to destroy the container if a iterator exist.
             *
             * @param rCont the container of the elements.
             */
            ListenerContainerIterator(  ListenerContainer& rCont )
            : m_aIter(rCont)
            , m_xNext()
            { advance(); }

            /**
             * Release the connection to the container.
             */
             ~ListenerContainerIterator() {}

            /** Return true, if there are more elements in the iterator. */
            sal_Bool hasMoreElements() const { return m_xNext.is() != 0; }

            /** Return the next element of the iterator. Call this method if
             *  hasMoreElements return false, is an error.
             */
            uno::Reference<Listener> next();

        private:
            void advance();

            cppu::OInterfaceIteratorHelper m_aIter;
            uno::Reference<Listener> m_xNext;
        };
//-----------------------------------------------------------------------------

        template <class Key_, class KeyHash_, class KeyEq_>
        class MultiListenerContainer
        {
        public:
            /**
             * Create a container of interface containers.
             *
             * @param rMutex    the mutex to protect multi thread access.
             *                  The lifetime must be longer than the lifetime
             *                  of this object.
             */
            MultiListenerContainer(osl::Mutex& rMutex)
            : m_aBroadcastHelper(rMutex)
            , m_bDisposeLock(false)
            {}

            ~MultiListenerContainer()
            {
                OSL_ENSURE(isDisposed(), "ERROR: Object was not disposed properly");
                if (m_bDisposeLock) mutex().release();
            }
        public:
            /// get the mutex thatthis object uses
            osl::Mutex& mutex() const { return m_aBroadcastHelper.rMutex; }

            /**
             * check whether this is disposed or still alive
             * @param pObject
             *      an interface on the object on which's behalf the operation was started
             * @return <FALSE/>
             *      if the object is being disposed
             * @throw com::sun::star::lang::DisposedException
             *      if the object was disposed completely
             */
            bool checkAlive(uno::XInterface* pObject = 0) volatile const throw(lang::DisposedException);

            /// return whether the object is completely alive
            bool isAlive() volatile const throw();
            /// return whether the object is currently being disposed
            bool isDisposing()volatile  const throw();
            /// return whether the object is completely disposed
            bool isDisposed()volatile  const throw();

            /**
             * Call disposing on all object in all the containers that
             * support XEventListener. Then clear the container.
             */
            void dispose( const lang::EventObject & rEvt ) throw(uno::RuntimeException);

            /**
             * Start disposing this object, leave the mutex locked for dispose processing
             * @return <TRUE/>
             *      if disposing has been started
             * @return <FALSE/>
             *      if disposing had already been started before
             */
            bool beginDisposing() throw();
            /**
             * Continue disposing this object leave the mutex unlocked
             * <p>  Call disposing on all object in all the containers that
             *      support XEventListener. Then clear the container.
             * </p>
             * @return <TRUE/>
             *      if disposing has been started
             * @return <FALSE/>
             *      if disposing had already been started before
             */
            void notifyDisposing( const lang::EventObject & rEvt ) throw(uno::RuntimeException);

            /// mark the end of the dispose processing
            void endDisposing() throw();

        public:
            /**
             * Return the container created under this key.
             * @return the container created under this key. If the container
             *          was not created, null was returned.
             */
            ListenerContainer *  getContainer( const Key_ & aKey) const
            { return m_aBroadcastHelper.aLC.getContainer(aKey); }

            /**
             * Insert an element in the container specified with the key. The position is not specified.
             * @param aKey      the id of the container.
             * @param xListener the added interface. It is allowed to insert null or
             *                  the same pointer more than once.
             * @return the new count of elements in the container (or 0 if the object is ready being disposed).
             */
            sal_Int32 addListener( const Key_& aKey, uno::Reference< lang::XEventListener > const& xListener) throw(uno::RuntimeException);

            /**
             * Remove an element from the container specified with the key.
             * It uses the equal definition of uno objects to remove the interfaces.
             * @param aKey      the id of the container.
             * @param xListener the removed interface.
             * @return the new count of elements in the container (or 0 if the object is ready being disposed).
             */
            sal_Int32 removeListener( const Key_& aKey, uno::Reference< lang::XEventListener > const& xListener) throw(uno::RuntimeException);

        public:
            typedef Key_        Key;
            typedef KeyHash_    KeyHash;
            typedef KeyEq_      KeyEq;
            typedef cppu::OMultiTypeInterfaceContainerHelperVar< Key_, KeyHash_, KeyEq_ >   ContainerHelper;
            typedef cppu::OBroadcastHelperVar< ContainerHelper, Key >                           BroadcastHelper;
        private:
            BroadcastHelper m_aBroadcastHelper;
            bool m_bDisposeLock;
        };
//-----------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////////////////////
        template <class Key_, class KeyHash_, class KeyEq_>
        bool MultiListenerContainer<Key_,KeyHash_,KeyEq_>::checkAlive(uno::XInterface* pObject) volatile const throw(lang::DisposedException)
        {
            bool bAlive = !m_aBroadcastHelper.bInDispose;
            if (m_aBroadcastHelper.bDisposed)
            {
                throw lang::DisposedException(OUString(RTL_CONSTASCII_USTRINGPARAM("The object has already been disposed")),pObject);
            }
            return bAlive;
        }
//-----------------------------------------------------------------------------
        template <class Key_, class KeyHash_, class KeyEq_>
        inline
        bool MultiListenerContainer<Key_,KeyHash_,KeyEq_>::isAlive() volatile const throw()
        {
            return !m_aBroadcastHelper.bInDispose && !m_aBroadcastHelper.bDisposed;
        }
//-----------------------------------------------------------------------------
        template <class Key_, class KeyHash_, class KeyEq_>
        inline
        bool MultiListenerContainer<Key_,KeyHash_,KeyEq_>::isDisposing() volatile const throw()
        {
            return !!m_aBroadcastHelper.bInDispose;
        }
//-----------------------------------------------------------------------------
        template <class Key_, class KeyHash_, class KeyEq_>
        inline
        bool MultiListenerContainer<Key_,KeyHash_,KeyEq_>::isDisposed() volatile const throw()
        {
            return !!m_aBroadcastHelper.bDisposed;
        }
//-----------------------------------------------------------------------------
        template <class Key_, class KeyHash_, class KeyEq_>
        void MultiListenerContainer<Key_,KeyHash_,KeyEq_>::dispose(const lang::EventObject & rEvt) throw(uno::RuntimeException)
        {
            if (beginDisposing())
            {
                notifyDisposing( rEvt );
                endDisposing();
            }
        }
//-----------------------------------------------------------------------------
        template <class Key_, class KeyHash_, class KeyEq_>
        bool MultiListenerContainer<Key_,KeyHash_,KeyEq_>::beginDisposing() throw()
        {
            osl::MutexGuard aGuard( mutex() );
            if (isAlive())
            {
                mutex().acquire();
                m_aBroadcastHelper.bInDispose = sal_True;
                m_bDisposeLock = true;

                return true;
            }
            return false;
        }
//-----------------------------------------------------------------------------
        template <class Key_, class KeyHash_, class KeyEq_>
        void MultiListenerContainer<Key_,KeyHash_,KeyEq_>::notifyDisposing(const lang::EventObject & rEvt) throw(uno::RuntimeException)
        {
            OSL_ENSURE(isDisposing(),"Disposing isn't in progress on this object");
            OSL_ENSURE(m_bDisposeLock,"Duplicate call for dispose notification or disposing is not taking place");

            if (m_bDisposeLock)
            {
                OSL_ASSERT(m_aBroadcastHelper.bInDispose);
                m_bDisposeLock = false;
                mutex().release();

                m_aBroadcastHelper.aLC.disposeAndClear( rEvt );
            }
        }
//-----------------------------------------------------------------------------
        template <class Key_, class KeyHash_, class KeyEq_>
        void MultiListenerContainer<Key_,KeyHash_,KeyEq_>::endDisposing() throw()
        {
            OSL_ENSURE(isDisposing(),"Disposing isn't in progress on this object");

            if (!isAlive())
            {
                OSL_ENSURE(!m_bDisposeLock,"Did you forget to notify ?");

                m_aBroadcastHelper.bDisposed = sal_True;
                m_aBroadcastHelper.bInDispose = sal_False;

                if (m_bDisposeLock)
                {
                    m_bDisposeLock = false;
                    mutex().release();
                }
            }
        }
//-----------------------------------------------------------------------------
        template <class Key_, class KeyHash_, class KeyEq_>
        sal_Int32 MultiListenerContainer<Key_,KeyHash_,KeyEq_>::addListener( const Key_& aKey, const uno::Reference< lang::XEventListener > & xListener ) throw(uno::RuntimeException)
        {
            osl::MutexGuard aGuard( mutex() );
            OSL_ENSHURE( !isDisposing(), "do not add listeners in the dispose call" );
            OSL_ENSHURE( !isDisposed(), "object is disposed" );

            if ( isAlive() )
                return m_aBroadcastHelper.aLC.addInterface(aKey,xListener);

            else
                return 0;
        }
//-----------------------------------------------------------------------------

        template <class Key_, class KeyHash_, class KeyEq_>
        sal_Int32 MultiListenerContainer<Key_,KeyHash_,KeyEq_>::removeListener( const Key_& aKey, const uno::Reference< lang::XEventListener > & xListener ) throw(uno::RuntimeException)
        {
            osl::MutexGuard aGuard( mutex() );
            OSL_ENSHURE( !isDisposed(), "object is disposed" );

            if ( isAlive() )
                m_aBroadcastHelper.aLC.removeInterface(aKey, xListener );

            else
                return 0;
        }
//-----------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////////////////////

        template <class Listener>
        inline
        void ListenerContainerIterator<Listener>::advance()
        {
            while (!m_xNext.is() && m_aIter.hasMoreElements())
            {
                m_xNext = m_xNext.query( m_aIter.next() );
            }
        }
//-----------------------------------------------------------------------------

        template <class Listener>
        uno::Reference<Listener> ListenerContainerIterator<Listener>::next()
        {
            uno::Reference<Listener> xRet(m_xNext);
            m_xNext.clear();
            advance();
            return xRet;
        }
//-----------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////////////////////
    }
}
#endif // CONFIGMGR_API_LISTENERCONTAINER_HXX_


