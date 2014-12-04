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
#ifndef INCLUDED_SW_INC_RING_HXX
#define INCLUDED_SW_INC_RING_HXX

#include <swdllapi.h>
#include <swtypes.hxx>
#include <utility>
#include <iterator>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/intrusive/circular_list_algorithms.hpp>

namespace sw
{
    template <class T> class RingContainer;
    /**
     * An intrusive container class double linking the contained nodes
     * @example sw/qa/core/uwriter.cxx
     */
    template <class T>
    class Ring
    {
        public:
            typedef RingContainer<T> ring_container;
            typedef RingContainer<const T> const_ring_container;
            virtual ~Ring()
                { algo::unlink(static_cast< T* >(this)); };
            /**
             * Removes this item from its current ring container and adds it to
             * another ring container. If the item was not alone in the original
             * ring container, the other items in the ring will stay in the old
             * ring container.
             * Note: the newly created item will be inserted just before item pDestRing.
             * @param pDestRing the container to add this item to
             */
            void MoveTo( T* pDestRing );
            /**
             * Merges two ring containers. All item from both ring containers will
             * be in the same ring container in the end.
             * Note: The items of this ring container will be inserted just before
             * item pDestRing
             * @param pDestRing the container to merge this container with
             */
            void MoveRingTo( T* pDestRing )
            {
                std::swap(*(&pPrev->pNext), *(&pDestRing->pPrev->pNext));
                std::swap(*(&pPrev), *(&pDestRing->pPrev));
            }
            /** @return the next item in the ring container */
            T* GetNext() const
                { return pNext; }
            /** @return the previous item in the ring container */
            T* GetPrev() const
                { return pPrev; }
            /** @return a stl-like container with begin()/end() for iteration */
            ring_container GetRingContainer();
            /** @return a stl-like container with begin()/end() for const iteration */
            const_ring_container GetRingContainer() const;

        protected:
            /**
             * Creates a new item in a ring container all by itself.
             * Note: Ring instances can newer be outside a container. At most, they
             * are alone in one.
             */
            Ring()
                : pPrev(static_cast< T* >(this))
                { algo::init_header(static_cast< T* >(this)); }
            /**
             * Creates a new item and add it to an existing ring container.
             * Note: the newly created item will be inserted just before item pRing.
             * @param pRing ring container to add the created item to
             */
            Ring( T* pRing );

        private:
            /** internal implementation class -- not for external use */
            struct Ring_node_traits
            {
                typedef T node;
                typedef T* node_ptr;
                typedef const T* const_node_ptr;
                static node_ptr get_next(const_node_ptr n) { return n->GetNext(); };
                static void set_next(node_ptr n, node_ptr next) { n->pNext = next; };
                static node_ptr get_previous(const_node_ptr n) { return n->GetPrev(); };
                static void set_previous(node_ptr n, node_ptr previous) { n->pPrev = previous; };
            };
            friend struct Ring_node_traits;
            typedef boost::intrusive::circular_list_algorithms<Ring_node_traits> algo;
            T* pNext;
            T* pPrev;
    };

    template <class T>
    inline Ring<T>::Ring( T* pObj )
        : pPrev(static_cast< T* >(this))
    {
        T* pThis = static_cast< T* >(this);
        if( !pObj )
            algo::init_header(pThis);
        else
            algo::link_before(pObj, pThis);
    }

    template <class T>
    inline void Ring<T>::MoveTo(T* pDestRing)
    {
        T* pThis = static_cast< T* >(this);
        // insert into "new"
        if( pDestRing )
        {
            if(algo::unique(pThis))
                algo::link_before(pDestRing, pThis);
            else
                algo::transfer(pDestRing, pThis);
        }
        else
            algo::unlink(pThis);
    }

    template <class T> class RingIterator;
    /**
     * helper class that provides STL-style container iteration to the ring
     */
    template <class T>
    class RingContainer SAL_FINAL
    {
        private:
            /** the item in the ring where iteration starts */
            T* m_pStart;

        public:
            RingContainer( T* pRing ) : m_pStart(pRing) {};
            typedef RingIterator<T> iterator;
            typedef RingIterator<const T> const_iterator;
            /**
             * iterator access
             * @code
             * for(SwPaM& rCurrentPaM : pPaM->GetRingContainer())
             *     do_stuff(rCurrentPaM); // this gets called on every SwPaM in the same ring as pPaM
             * @endcode
             */
            iterator begin();
            iterator end();
            const_iterator begin() const;
            const_iterator end() const;
            /** @return the number of elements in the container */
            size_t size() const
                { return std::distance(begin(), end()); }
    };

    template <class T>
    class RingIterator SAL_FINAL : public boost::iterator_facade<
          RingIterator<T>
        , T
        , boost::forward_traversal_tag
        >
    {
        public:
            RingIterator()
                : m_pCurrent(nullptr)
                , m_pStart(nullptr)
            {}
            explicit RingIterator(T* pRing, bool bStart = true)
                : m_pCurrent(nullptr)
                , m_pStart(pRing)
            {
                if(!bStart)
                    m_pCurrent = m_pStart;
            }

        private:
            friend class boost::iterator_core_access;
            void increment()
                { m_pCurrent = m_pCurrent ? m_pCurrent->GetNext() : m_pStart->GetNext(); }
            bool equal(RingIterator const& other) const
            {
                // we never want to compare iterators from
                // different rings or starting points
                assert(m_pStart == other.m_pStart);
                return m_pCurrent == other.m_pCurrent;
            }
            T& dereference() const
                { return m_pCurrent ? *m_pCurrent : * m_pStart; }
            /**
             * This is:
             * - pointing to the current item in the iteration in general
             * - nullptr if on the first item (begin())
             * - m_pStart when beyond the last item (end())
             */
            T* m_pCurrent;
            /** the first item of the iteration */
            T* m_pStart;
    };

    template <class T>
    inline typename Ring<T>::ring_container Ring<T>::GetRingContainer()
        { return Ring<T>::ring_container(static_cast< T* >(this)); };

    template <class T>
    inline typename Ring<T>::const_ring_container Ring<T>::GetRingContainer() const
        { return Ring<T>::const_ring_container(static_cast< const T* >(this)); };

    template <class T>
    inline typename RingContainer<T>::iterator RingContainer<T>::begin()
        { return RingContainer<T>::iterator(m_pStart); };

    template <class T>
    inline typename RingContainer<T>::iterator RingContainer<T>::end()
        { return RingContainer<T>::iterator(m_pStart, false); };

    template <class T>
    inline typename RingContainer<T>::const_iterator RingContainer<T>::begin() const
        { return RingContainer<T>::const_iterator(m_pStart); };

    template <class T>
    inline typename RingContainer<T>::const_iterator RingContainer<T>::end() const
        { return RingContainer<T>::const_iterator(m_pStart, false); };
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
