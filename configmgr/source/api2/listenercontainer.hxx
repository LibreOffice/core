/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: listenercontainer.hxx,v $
 * $Revision: 1.19 $
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

#ifndef CONFIGMGR_API_LISTENERCONTAINER_HXX_
#define CONFIGMGR_API_LISTENERCONTAINER_HXX_

#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <cppuhelper/interfacecontainer.hxx>
#include "datalock.hxx"
#include "utility.hxx"

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
            ListenerContainerIterator(  cppu::OInterfaceContainerHelper& rCont )
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
            lang::EventObject aEvent;
            std::vector< uno::Reference< lang::XEventListener > > aListeners;
        public:
            explicit
            DisposeNotifier(uno::Reference<uno::XInterface> const& aInterface) : aEvent(aInterface) {}

            void appendAndClearContainer(cppu::OInterfaceContainerHelper* pContainer);
            void notify();
        };
//-----------------------------------------------------------------------------
        struct BasicContainerInfo
        {
            uno::XInterface*            pInterface;
            cppu::OMultiTypeInterfaceContainerHelper*   pContainer;
            BasicContainerInfo() : pInterface(0), pContainer(0) {}
        };

        template <class Key_, class KeyHash_, class KeyEq_, class KeyToIndex_>
        class SpecialListenerContainer
        {
        public:
            /**
             * Create a container of interface containers.
             */
            SpecialListenerContainer(std::vector<BasicContainerInfo>::size_type nCount, KeyToIndex_ aMapper)
            : m_aSpecialHelper(UnoApiLock::getLock())
            , m_aContainers(nCount)
            , m_aMapper(aMapper)
            , m_bDisposeLock(false)
            {}

            ~SpecialListenerContainer()
            {
                OSL_ENSURE(isDisposed(), "ERROR: Object was not disposed properly");
            }
        public:
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
            bool isAvailable(std::vector<BasicContainerInfo>::size_type nIndex)  const throw()
            {
                return nIndex < m_aContainers.size() && m_aContainers[nIndex].pInterface;
            }

            std::vector<BasicContainerInfo>::size_type getSize() const
            {
                return m_aContainers.size();
            }

            /// return the interface associated with an index
            void setObjectAt(std::vector<BasicContainerInfo>::size_type nIndex, uno::XInterface* pInterface)
            {
                OSL_ENSURE( !isDisposed(), "object is disposed" );

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
            uno::Reference<uno::XInterface> getObjectAt(std::vector<BasicContainerInfo>::size_type nIndex) const
            {
                uno::Reference<uno::XInterface> xRet( nIndex < m_aContainers.size() ? m_aContainers[nIndex].pInterface : 0 );
                return xRet;
            }

            /// return the interface associated with an index
            uno::Reference<uno::XInterface> getObjectForKey(Key_ const& aKey ) const
            {
                std::vector<BasicContainerInfo>::size_type nIndex = m_aMapper.findIndexForKey(aKey);
                uno::Reference<uno::XInterface> xRet( nIndex < m_aContainers.size() ? m_aContainers[nIndex].pInterface : 0 );
                return xRet;
            }

            /**
             * Call disposing on all object in all the container for anIndex
             * and in the containers for the associated indices
             * support XEventListener. Then clear the container.
             */
            bool disposeOne( std::vector<BasicContainerInfo>::size_type anIndex ) throw();

            /**
             * Start disposing this object
             * @return <TRUE/>
             *      if disposing has been started
             * @return <FALSE/>
             *      if disposing had already been started before
             */
            bool beginDisposing() throw();
            /**
             * Continue disposing this object
             * <p>  Call disposing on all object in all the containers that
             *      support XEventListener. Then clear the container.
             * </p>
             * @return <TRUE/>
             *      if disposing has been started
             * @return <FALSE/>
             *      if disposing had already been started before
             */
            void notifyDisposing() throw();

            /// mark the end of the dispose processing
            void endDisposing() throw();

        public:
            /**
             * Return the specuial container created under this key.
             * @return the container created under this key. If the container
             *          was not created, null was returned.
             */
            cppu::OInterfaceContainerHelper *  getSpecialContainer( const Key_ & aKey) const
            { return m_aSpecialHelper.aLC.getContainer(aKey); }

            /**
             * Return the containerhelper created under this index.
             * @return the container helper created under this key. If the container helper
             *  was not created, null was returned.
             */
            cppu::OMultiTypeInterfaceContainerHelper *  getContainerHelper( std::vector<BasicContainerInfo>::size_type nIndex) const
            {
                return ((nIndex < m_aContainers.size()) ? m_aContainers[nIndex].pContainer : 0 );
            }
            /**
             * Return the container for the given type created under this index.
             * @return the container created under this key. If the container
             *          was not created, null was returned.
             */
            cppu::OInterfaceContainerHelper *  getContainer( std::vector<BasicContainerInfo>::size_type nIndex, const uno::Type & aType) const
            {
                cppu::OMultiTypeInterfaceContainerHelper* pContainer = (nIndex < m_aContainers.size()) ? m_aContainers[nIndex].pContainer : 0 ;

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
            sal_Int32 addListener( std::vector<BasicContainerInfo>::size_type nIndex, const uno::Type& aType, uno::Reference< lang::XEventListener > const& xListener) throw();

            /**
             * Remove an element from the container specified with the index and type.
             * It uses the equal definition of uno objects to remove the interfaces.
             * @param aKey      the id of the container.
             * @param xListener the removed interface.
             * @return the new count of elements in the container (or 0 if the object is ready being disposed).
             */
            sal_Int32 removeListener( std::vector<BasicContainerInfo>::size_type nIndex, const uno::Type& aType, uno::Reference< lang::XEventListener > const& xListener) throw();


            /**
             * Insert an element in the special container specified with the key. The position is not specified.
             * The interface at the given index must be set already.
             * @param aKey      the id of the container.
             * @param xListener the added interface. It is allowed to insert null or
             *                  the same pointer more than once.
             * @return the new count of elements in the container (or 0 if the object is ready being disposed).
             */
            sal_Int32 addSpecialListener( const Key_& aKey, uno::Reference< lang::XEventListener > const& xListener) throw();

            /**
             * Remove an element from the container specified with the key.
             * It uses the equal definition of uno objects to remove the interfaces.
             * @param aKey      the id of the container.
             * @param xListener the removed interface.
             * @return the new count of elements in the container (or 0 if the object is ready being disposed).
             */
            sal_Int32 removeSpecialListener( const Key_& aKey, uno::Reference< lang::XEventListener > const& xListener) throw();

        private:
            void implFillDisposer(DisposeNotifier& aNotifier, std::vector<BasicContainerInfo>::size_type nIndex);

            cppu::OBroadcastHelperVar< cppu::OMultiTypeInterfaceContainerHelperVar< Key_,KeyHash_,KeyEq_ >, Key_ >      m_aSpecialHelper;
            std::vector<BasicContainerInfo> m_aContainers;
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
                throw lang::DisposedException(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("The object has already been disposed")),pObject);
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
        bool SpecialListenerContainer<Key_,KeyHash_,KeyEq_, KeyToIndex_>::disposeOne(std::vector<BasicContainerInfo>::size_type nIndex) throw()
        {
    //      OSL_ENSURE(!isDisposed(),"Object is already disposed in toto");
            if (isAlive())
            {
                if (nIndex < m_aContainers.size())
                {
                    if (uno::XInterface* pObject = m_aContainers[nIndex].pInterface)
                    {
                        DisposeNotifier aNotifier(pObject);

                        implFillDisposer(aNotifier, nIndex);
                        m_aContainers[nIndex].pInterface = 0;
                        delete m_aContainers[nIndex].pContainer;

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
            if (isAlive())
            {
                m_aSpecialHelper.bInDispose = sal_True;
                m_bDisposeLock = true;

                return true;
            }
            return false;
        }
//-----------------------------------------------------------------------------
        template <class Key_, class KeyHash_, class KeyEq_, class KeyToIndex_>
        void SpecialListenerContainer<Key_,KeyHash_,KeyEq_, KeyToIndex_>::notifyDisposing() throw()
        {
            OSL_ENSURE(isDisposing(),"Disposing isn't in progress on this object");
            OSL_ENSURE(m_bDisposeLock,"Duplicate call for dispose notification or disposing is not taking place");

            if (m_bDisposeLock)
            {
                OSL_ASSERT(m_aSpecialHelper.bInDispose);

                lang::EventObject aBaseEvt;
                std::vector<DisposeNotifier> aNotifiers;

                if (std::vector<BasicContainerInfo>::size_type size = m_aContainers.size())
                {
                    aNotifiers.reserve(m_aContainers.size());

                    aBaseEvt.Source = m_aContainers[0].pInterface;
                    for(std::vector<BasicContainerInfo>::size_type ix = 0; ix < size; ++ix)
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

                for(std::vector<BasicContainerInfo>::size_type jx = 0, count = aNotifiers.size(); jx < count; ++jx)
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
                }
            }
        }
//-----------------------------------------------------------------------------
        template <class Key_, class KeyHash_, class KeyEq_, class KeyToIndex_>
        sal_Int32 SpecialListenerContainer<Key_,KeyHash_,KeyEq_, KeyToIndex_>::addListener( std::vector<BasicContainerInfo>::size_type nIndex, const uno::Type& aType, const uno::Reference< lang::XEventListener > & xListener ) throw()
        {
            if ( nIndex < m_aContainers.size() && m_aContainers[nIndex].pInterface  )
            {
                if ( isAlive() )
                {
                    if (m_aContainers[nIndex].pContainer == 0)
                        m_aContainers[nIndex].pContainer = new cppu::OMultiTypeInterfaceContainerHelper(UnoApiLock::getLock());

                    return m_aContainers[nIndex].pContainer->addInterface(aType,xListener);
                }

                else if (xListener.is())
                {
                    lang::EventObject aEvent(m_aContainers[nIndex].pInterface);
                    try { xListener->disposing(aEvent); } catch (uno::Exception & ) {}
                }

            }
            else
                OSL_ENSURE(false, "Invalid index or interface not set");

            return 0;
        }
//-----------------------------------------------------------------------------
        template <class Key_, class KeyHash_, class KeyEq_, class KeyToIndex_>
        sal_Int32 SpecialListenerContainer<Key_,KeyHash_,KeyEq_, KeyToIndex_>::addSpecialListener( const Key_& aKey, const uno::Reference< lang::XEventListener > & xListener ) throw()
        {
            std::vector<BasicContainerInfo>::size_type nIndex = m_aMapper.findIndexForKey(aKey);
            if ( nIndex < m_aContainers.size() && m_aContainers[nIndex].pInterface  )
            {
                if ( isAlive() )
                {
                    return m_aSpecialHelper.aLC.addInterface(aKey,xListener);
                }

                else if (xListener.is())
                {
                    lang::EventObject aEvent(m_aContainers[nIndex].pInterface);
                    try { xListener->disposing(aEvent); } catch (uno::Exception & ) {}
                }
            }
            else
                OSL_ENSURE(false, "Invalid index or interface not set");

            return 0;
        }
//-----------------------------------------------------------------------------

        template <class Key_, class KeyHash_, class KeyEq_, class KeyToIndex_>
        sal_Int32 SpecialListenerContainer<Key_,KeyHash_,KeyEq_, KeyToIndex_>::removeListener( std::vector<BasicContainerInfo>::size_type nIndex, const uno::Type& aType, const uno::Reference< lang::XEventListener > & xListener ) throw()
        {
            OSL_ENSURE( !isDisposed(), "object is disposed" );

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
        sal_Int32 SpecialListenerContainer<Key_,KeyHash_,KeyEq_, KeyToIndex_>::removeSpecialListener( const Key_& aKey, const uno::Reference< lang::XEventListener > & xListener ) throw()
        {
            OSL_ENSURE( !isDisposed(), "object is disposed" );

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
        bool SpecialListenerContainer<Key_,KeyHash_,KeyEq_, KeyToIndex_>::findKeysForIndex(std::vector<BasicContainerInfo>::size_type nIndex, std::vector<Key_> & aKeys)
        {
            aKeys.clear();
            m_aMapper.findKeysForIndex(nIndex,aKeys);
            return !aKeys.empty();
        }
*///-----------------------------------------------------------------------------
    // relation function. Uses KeyToIndex
        template <class Key_, class KeyHash_, class KeyEq_, class KeyToIndex_>
        void SpecialListenerContainer<Key_,KeyHash_,KeyEq_, KeyToIndex_>::implFillDisposer(DisposeNotifier& aNotifier, std::vector<BasicContainerInfo>::size_type nIndex)
        {
            if (cppu::OMultiTypeInterfaceContainerHelper* pMultiContainer = m_aContainers[nIndex].pContainer)
            {
                uno::Sequence< uno::Type > aTypes(pMultiContainer->getContainedTypes());
                for (sal_Int32 ix = 0; ix < aTypes.getLength(); ++ix)
                {
                    cppu::OInterfaceContainerHelper* pContainer = pMultiContainer->getContainer(aTypes[ix]);
                    OSL_ENSURE(pContainer,"No container, but the type ?");
                    if (pContainer)
                        aNotifier.appendAndClearContainer(pContainer);
                }
            }
            std::vector<Key_> aKeys;
            if (m_aMapper.findKeysForIndex(nIndex,aKeys))
            {
                for(typename std::vector<Key_>::iterator it = aKeys.begin(); it != aKeys.end(); ++it)
                {
                    if (cppu::OInterfaceContainerHelper* pContainer = m_aSpecialHelper.aLC.getContainer(*it))
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


