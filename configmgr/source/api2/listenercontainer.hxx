/*************************************************************************
 *
 *  $RCSfile: listenercontainer.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: jb $ $Date: 2000-11-20 01:38:18 $
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

        typedef uno::Type                       UnoType;
        typedef uno::XInterface                 UnoInterface;
        typedef uno::Reference<uno::XInterface> UnoInterfaceRef;
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
        class DisposeNotifier
        {
            typedef uno::Reference< lang::XEventListener > Listener;
            typedef std::vector< Listener > Listeners;
            lang::EventObject aEvent;
            Listeners aListeners;
        public:
            explicit
            DisposeNotifier(UnoInterfaceRef const& aInterface) : aEvent(aInterface) {}

            void appendAndClearContainer(ListenerContainer* pContainer);
            void notify();
        };
//-----------------------------------------------------------------------------
        template <class Key_, class KeyHash_, class KeyEq_, class KeyToIndex_>
        class SpecialListenerContainer
        {
        public:
            typedef cppu::OMultiTypeInterfaceContainerHelper    BasicContainerHelper;
            struct BasicContainerInfo
            {
                UnoInterface*           pInterface;
                BasicContainerHelper*   pContainer;
                BasicContainerInfo() : pInterface(0), pContainer(0) {}
            };
            typedef std::vector<BasicContainerInfo>         BasicContainerHelperArray;
            typedef BasicContainerHelperArray::size_type    Index;

            typedef Key_ Key;
            typedef cppu::OMultiTypeInterfaceContainerHelperVar< Key_,KeyHash_,KeyEq_ > SpecialContainerHelper;
            typedef cppu::OBroadcastHelperVar< SpecialContainerHelper, Key >            SpecialBroadcastHelper;
            typedef std::vector<Key> KeyList;

        public:
            /**
             * Create a container of interface containers.
             *
             * @param rMutex    the mutex to protect multi thread access.
             *                  The lifetime must be longer than the lifetime
             *                  of this object.
             */
            SpecialListenerContainer(osl::Mutex& rMutex, Index nCount, KeyToIndex_ aMapper)
            : m_aSpecialHelper(rMutex)
            , m_aContainers(nCount)
            , m_bDisposeLock(false)
            , m_aMapper(aMapper)
            {}

            ~SpecialListenerContainer()
            {
                OSL_ENSURE(isDisposed(), "ERROR: Object was not disposed properly");
                if (m_bDisposeLock) mutex().release();
            }
        public:
            /// get the mutex thatthis object uses
            osl::Mutex& mutex() const { return m_aSpecialHelper.rMutex; }

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

            /// return whether the object is present in this container
            bool isAvailable(Index nIndex)  const throw()
            {
                osl::MutexGuard aGuard(mutex());
                return nIndex < m_aContainers.size() && m_aContainers[nIndex].pInterface;
            }

            Index getSize() const
            {
                osl::MutexGuard aGuard(mutex());
                return m_aContainers.size();
            }

            /// return the interface associated with an index
            void setObjectAt(Index nIndex, UnoInterface* pInterface)
            {
                osl::MutexGuard aGuard(mutex());
                OSL_ENSHURE( !isDisposed(), "object is disposed" );

                if (isAlive())
                {
                    OSL_ENSURE( nIndex < m_aContainers.size(), " Invalid Index into Notifier");
                    OSL_ENSURE( pInterface, "Invalid NULL Interface passed into Notifier");

                    if ( nIndex < m_aContainers.size() && pInterface != NULL)
                    {
                        OSL_ENSURE( m_aContainers[nIndex].pInterface == NULL, "Interface already set");
                        if (m_aContainers[nIndex].pInterface == NULL)
                            m_aContainers[nIndex].pInterface = pInterface;
                    }
                }
            }


            /// return the interface associated with an index
            UnoInterfaceRef getObjectAt(Index nIndex) const
            {
                osl::MutexGuard aGuard(mutex());
                UnoInterfaceRef xRet( nIndex < m_aContainers.size() ? m_aContainers[nIndex].pInterface : 0 );
                return xRet;
            }

            /// return the interface associated with an index
            UnoInterfaceRef getObjectForKey(Key const& aKey ) const
            {
                osl::MutexGuard aGuard(mutex());
                Index nIndex = m_aMapper.findIndexForKey(aKey);
                UnoInterfaceRef xRet( nIndex < m_aContainers.size() ? m_aContainers[nIndex].pInterface : 0 );
                return xRet;
            }

            /**
             * Call disposing on all object in all the containers that
             * support XEventListener. Then clear the container.
             */
            bool disposeAll() throw(uno::RuntimeException);

            /**
             * Call disposing on all object in all the container for anIndex
             * and in the containers for the associated indices
             * support XEventListener. Then clear the container.
             */
            bool disposeOne( Index anIndex ) throw(uno::RuntimeException);

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
            void notifyDisposing() throw(uno::RuntimeException);

            /// mark the end of the dispose processing
            void endDisposing() throw();

        public:
            /**
             * Return the specuial container created under this key.
             * @return the container created under this key. If the container
             *          was not created, null was returned.
             */
            ListenerContainer *  getSpecialContainer( const Key_ & aKey) const
            { return m_aSpecialHelper.aLC.getContainer(aKey); }

            /**
             * Return the containerhelper created under this index.
             * @return the container helper created under this key. If the container helper
             *  was not created, null was returned.
             */
            BasicContainerHelper *  getContainerHelper( Index nIndex) const
            {
                osl::MutexGuard aGuard(mutex());
                return ((nIndex < m_aContainers.size()) ? m_aContainers[nIndex].pContainer : 0 );
            }
            /**
             * Return the container for the given type created under this index.
             * @return the container created under this key. If the container
             *          was not created, null was returned.
             */
            ListenerContainer *  getContainer( Index nIndex, const UnoType & aType) const
            {
                osl::MutexGuard aGuard(mutex());
                BasicContainerHelper* pContainer = (nIndex < m_aContainers.size()) ? m_aContainers[nIndex].pContainer : 0 ;

                return pContainer ? pContainer->getContainer(aType) : 0;
            }

            /**
             * Insert an element in the container specified with the index and type. The position is not specified.
             * The interface at the given index must be set already.
             * @param aKey      the id of the container.
             * @param xListener the added interface. It is allowed to insert null or
             *                  the same pointer more than once.
             * @return the new count of elements in the container (or 0 if the object is ready being disposed).
             */
            sal_Int32 addListener( Index nIndex, const UnoType& aType, uno::Reference< lang::XEventListener > const& xListener) throw(uno::RuntimeException);

            /**
             * Remove an element from the container specified with the index and type.
             * It uses the equal definition of uno objects to remove the interfaces.
             * @param aKey      the id of the container.
             * @param xListener the removed interface.
             * @return the new count of elements in the container (or 0 if the object is ready being disposed).
             */
            sal_Int32 removeListener( Index nIndex, const UnoType& aType, uno::Reference< lang::XEventListener > const& xListener) throw(uno::RuntimeException);


            /**
             * Insert an element in the special container specified with the key. The position is not specified.
             * The interface at the given index must be set already.
             * @param aKey      the id of the container.
             * @param xListener the added interface. It is allowed to insert null or
             *                  the same pointer more than once.
             * @return the new count of elements in the container (or 0 if the object is ready being disposed).
             */
            sal_Int32 addSpecialListener( const Key_& aKey, uno::Reference< lang::XEventListener > const& xListener) throw(uno::RuntimeException);

            /**
             * Remove an element from the container specified with the key.
             * It uses the equal definition of uno objects to remove the interfaces.
             * @param aKey      the id of the container.
             * @param xListener the removed interface.
             * @return the new count of elements in the container (or 0 if the object is ready being disposed).
             */
            sal_Int32 removeSpecialListener( const Key_& aKey, uno::Reference< lang::XEventListener > const& xListener) throw(uno::RuntimeException);

        private:
            void implFillDisposer(DisposeNotifier& aNotifier, Index nIndex);

            SpecialBroadcastHelper      m_aSpecialHelper;
            BasicContainerHelperArray   m_aContainers;
            KeyToIndex_                 m_aMapper;
            bool m_bDisposeLock;
        };
//-----------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////////////////////
        template <class Key_, class KeyHash_, class KeyEq_, class KeyToIndex_>
        bool SpecialListenerContainer<Key_,KeyHash_,KeyEq_, KeyToIndex_>::checkAlive(uno::XInterface* pObject) volatile const throw(lang::DisposedException)
        {
            bool bAlive = !m_aSpecialHelper.bInDispose;
            if (m_aSpecialHelper.bDisposed)
            {
                throw lang::DisposedException(OUString(RTL_CONSTASCII_USTRINGPARAM("The object has already been disposed")),pObject);
            }
            return bAlive;
        }
//-----------------------------------------------------------------------------
        template <class Key_, class KeyHash_, class KeyEq_, class KeyToIndex_>
        inline
        bool SpecialListenerContainer<Key_,KeyHash_,KeyEq_, KeyToIndex_>::isAlive() volatile const throw()
        {
            return !m_aSpecialHelper.bInDispose && !m_aSpecialHelper.bDisposed;
        }
//-----------------------------------------------------------------------------
        template <class Key_, class KeyHash_, class KeyEq_, class KeyToIndex_>
        inline
        bool SpecialListenerContainer<Key_,KeyHash_,KeyEq_, KeyToIndex_>::isDisposing() volatile const throw()
        {
            return !!m_aSpecialHelper.bInDispose;
        }
//-----------------------------------------------------------------------------
        template <class Key_, class KeyHash_, class KeyEq_, class KeyToIndex_>
        inline
        bool SpecialListenerContainer<Key_,KeyHash_,KeyEq_, KeyToIndex_>::isDisposed() volatile const throw()
        {
            return !!m_aSpecialHelper.bDisposed;
        }
//-----------------------------------------------------------------------------
        template <class Key_, class KeyHash_, class KeyEq_, class KeyToIndex_>
        bool SpecialListenerContainer<Key_,KeyHash_,KeyEq_, KeyToIndex_>::disposeAll() throw(uno::RuntimeException)
        {
            if (beginDisposing())
            {
                notifyDisposing();
                endDisposing();
                return true;
            }
            else
                return false;
        }
//-----------------------------------------------------------------------------
        template <class Key_, class KeyHash_, class KeyEq_, class KeyToIndex_>
        bool SpecialListenerContainer<Key_,KeyHash_,KeyEq_, KeyToIndex_>::disposeOne(Index nIndex) throw(uno::RuntimeException)
        {
            OSL_ENSURE(!isDisposed(),"Object is already disposed in toto");

            osl::ClearableMutexGuard aGuard(mutex());

            if (isAlive())
            {
                if (nIndex < m_aContainers.size())
                {
                    if (UnoInterface* pObject = m_aContainers[nIndex].pInterface)
                    {
                        DisposeNotifier aNotifier(pObject);

                        implFillDisposer(aNotifier, nIndex);
                        m_aContainers[nIndex].pInterface = 0;
                        delete m_aContainers[nIndex].pContainer;

                        aGuard.clear();

                        aNotifier.notify();
                    }
                }
                return true;
            }
            else
                return false;
        }
//-----------------------------------------------------------------------------
        template <class Key_, class KeyHash_, class KeyEq_, class KeyToIndex_>
        bool SpecialListenerContainer<Key_,KeyHash_,KeyEq_, KeyToIndex_>::beginDisposing() throw()
        {
            osl::MutexGuard aGuard( mutex() );
            if (isAlive())
            {
                mutex().acquire();
                m_aSpecialHelper.bInDispose = sal_True;
                m_bDisposeLock = true;

                return true;
            }
            return false;
        }
//-----------------------------------------------------------------------------
        template <class Key_, class KeyHash_, class KeyEq_, class KeyToIndex_>
        void SpecialListenerContainer<Key_,KeyHash_,KeyEq_, KeyToIndex_>::notifyDisposing() throw(uno::RuntimeException)
        {
            OSL_ENSURE(isDisposing(),"Disposing isn't in progress on this object");
            OSL_ENSURE(m_bDisposeLock,"Duplicate call for dispose notification or disposing is not taking place");

            if (m_bDisposeLock)
            {
                OSL_ASSERT(m_aSpecialHelper.bInDispose);

                lang::EventObject aBaseEvt;
                std::vector<DisposeNotifier> aNotifiers;

                if (Index size = m_aContainers.size())
                {
                    aNotifiers.reserve(m_aContainers.size());

                    aBaseEvt.Source = m_aContainers[0].pInterface;
                    for(Index ix = 0; ix < size; ++ix)
                    {
                        if (m_aContainers[ix].pInterface)
                        {
                            aNotifiers.push_back(DisposeNotifier(m_aContainers[ix].pInterface));
                            implFillDisposer(aNotifiers.back(), ix);
                            m_aContainers[ix].pInterface = 0;
                            delete m_aContainers[ix].pContainer;
                        }
                    }
                }

                m_bDisposeLock = false;
                mutex().release();

                for(Index jx = 0, count = aNotifiers.size(); jx < count; ++jx)
                {
                    aNotifiers[jx].notify();
                }
                // in case we missed something
                m_aSpecialHelper.aLC.disposeAndClear( aBaseEvt );
            }
        }
//-----------------------------------------------------------------------------
        template <class Key_, class KeyHash_, class KeyEq_, class KeyToIndex_>
        void SpecialListenerContainer<Key_,KeyHash_,KeyEq_, KeyToIndex_>::endDisposing() throw()
        {
            OSL_ENSURE(isDisposing(),"Disposing isn't in progress on this object");

            if (!isAlive())
            {
                OSL_ENSURE(!m_bDisposeLock,"Did you forget to notify ?");

                m_aSpecialHelper.bDisposed = sal_True;
                m_aSpecialHelper.bInDispose = sal_False;

                if (m_bDisposeLock)
                {
                    m_bDisposeLock = false;
                    mutex().release();
                }
            }
        }
//-----------------------------------------------------------------------------
        template <class Key_, class KeyHash_, class KeyEq_, class KeyToIndex_>
        sal_Int32 SpecialListenerContainer<Key_,KeyHash_,KeyEq_, KeyToIndex_>::addListener( Index nIndex, const UnoType& aType, const uno::Reference< lang::XEventListener > & xListener ) throw(uno::RuntimeException)
        {
            osl::MutexGuard aGuard( mutex() );
            OSL_ENSHURE( !isDisposing(), "do not add listeners in the dispose call" );
            OSL_ENSHURE( !isDisposed(), "object is disposed" );

            if ( isAlive() )
            {
                if ( nIndex < m_aContainers.size() && m_aContainers[nIndex].pInterface  )
                {
                    if (m_aContainers[nIndex].pContainer == 0)
                        m_aContainers[nIndex].pContainer = new BasicContainerHelper(mutex());

                    return m_aContainers[nIndex].pContainer->addInterface(aType,xListener);
                }
                OSL_ENSURE(false, "Invalid index or interface not set");
            }
            return 0;
        }
//-----------------------------------------------------------------------------
        template <class Key_, class KeyHash_, class KeyEq_, class KeyToIndex_>
        sal_Int32 SpecialListenerContainer<Key_,KeyHash_,KeyEq_, KeyToIndex_>::addSpecialListener( const Key_& aKey, const uno::Reference< lang::XEventListener > & xListener ) throw(uno::RuntimeException)
        {
            osl::MutexGuard aGuard( mutex() );
            OSL_ENSHURE( !isDisposing(), "do not add listeners in the dispose call" );
            OSL_ENSHURE( !isDisposed(), "object is disposed" );

            if ( isAlive() )
            {
                Index nIndex = m_aMapper.findIndexForKey(aKey);
                if ( nIndex < m_aContainers.size() && m_aContainers[nIndex].pInterface  )
                {
                    return m_aSpecialHelper.aLC.addInterface(aKey,xListener);
                }
                OSL_ENSURE(false, "Invalid index or interface not set");
            }
            return 0;
        }
//-----------------------------------------------------------------------------

        template <class Key_, class KeyHash_, class KeyEq_, class KeyToIndex_>
        sal_Int32 SpecialListenerContainer<Key_,KeyHash_,KeyEq_, KeyToIndex_>::removeListener( Index nIndex, const UnoType& aType, const uno::Reference< lang::XEventListener > & xListener ) throw(uno::RuntimeException)
        {
            osl::MutexGuard aGuard( mutex() );
            OSL_ENSHURE( !isDisposed(), "object is disposed" );

            if ( isAlive() )
            {
                if ( nIndex < m_aContainers.size() && m_aContainers[nIndex].pContainer  )
                {
                    return m_aContainers[nIndex].pContainer->removeInterface(aType,xListener);
                }
            }
            return 0;
        }
//-----------------------------------------------------------------------------

        template <class Key_, class KeyHash_, class KeyEq_, class KeyToIndex_>
        sal_Int32 SpecialListenerContainer<Key_,KeyHash_,KeyEq_, KeyToIndex_>::removeSpecialListener( const Key_& aKey, const uno::Reference< lang::XEventListener > & xListener ) throw(uno::RuntimeException)
        {
            osl::MutexGuard aGuard( mutex() );
            OSL_ENSHURE( !isDisposed(), "object is disposed" );

            if ( isAlive() )
                return m_aSpecialHelper.aLC.removeInterface(aKey, xListener );

            else
                return 0;
        }
//-----------------------------------------------------------------------------
    // relation function. Uses KeyToIndex
/*      template <class Key_, class KeyHash_, class KeyEq_, class KeyToIndex_>
        SpecialListenerContainer<Key_,KeyHash_,KeyEq_, KeyToIndex_>::Index
            SpecialListenerContainer<Key_,KeyHash_,KeyEq_, KeyToIndex_>::findIndexForKey(Key const& aKey)
        {
            m_aMapper.findIndexForKey(aKey);
        }
//-----------------------------------------------------------------------------
    // relation function. Uses KeyToIndex
        template <class Key_, class KeyHash_, class KeyEq_, class KeyToIndex_>
        bool SpecialListenerContainer<Key_,KeyHash_,KeyEq_, KeyToIndex_>::findKeysForIndex(Index nIndex, KeyList & aKeys)
        {
            aKeys.clear();
            m_aMapper.findKeysForIndex(nIndex,aKeys);
            return !aKeys.empty();
        }
*///-----------------------------------------------------------------------------
    // relation function. Uses KeyToIndex
        template <class Key_, class KeyHash_, class KeyEq_, class KeyToIndex_>
        void SpecialListenerContainer<Key_,KeyHash_,KeyEq_, KeyToIndex_>::implFillDisposer(DisposeNotifier& aNotifier, Index nIndex)
        {
            if (BasicContainerHelper* pMultiContainer = m_aContainers[nIndex].pContainer)
            {
                uno::Sequence< UnoType > aTypes(pMultiContainer->getContainedTypes());
                for (sal_Int32 ix = 0; ix < aTypes.getLength(); ++ix)
                {
                    ListenerContainer* pContainer = pMultiContainer->getContainer(aTypes[ix]);
                    OSL_ENSURE(pContainer,"No container, but the type ?");
                    if (pContainer)
                        aNotifier.appendAndClearContainer(pContainer);
                }
            }
            KeyList aKeys;
            if (m_aMapper.findKeysForIndex(nIndex,aKeys))
            {
                for(KeyList::iterator it = aKeys.begin(); it != aKeys.end(); ++it)
                {
                    if (ListenerContainer* pContainer = m_aSpecialHelper.aLC.getContainer(*it))
                    {
                        aNotifier.appendAndClearContainer(pContainer);
                    }
                }
            }
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


