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
#include <type_traits>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/intrusive/circular_list_algorithms.hpp>

namespace sw
{
    template <typename value_type> class RingContainer;
    template <typename value_type> class RingIterator;
    /**
     * An intrusive container class double linking the contained nodes
     * @example sw/qa/core/uwriter.cxx
     */
    template <typename value_type>
    class Ring
    {
        public:
            typedef typename std::add_const<value_type>::type const_value_type;
            typedef RingContainer<value_type> ring_container;
            typedef RingContainer<const_value_type> const_ring_container;
            virtual ~Ring()
                { unlink(); };
            /** algo::unlink is buggy! don't call it directly! */
            void unlink()
            {
                algo::unlink(this);
                pNext = this; // don't leave pointers to old list behind!
                pPrev = this;
            }
            /**
             * Removes this item from its current ring container and adds it to
             * another ring container. If the item was not alone in the original
             * ring container, the other items in the ring will stay in the old
             * ring container.
             * Note: the newly created item will be inserted just before item pDestRing.
             * @param pDestRing the container to add this item to
             */
            void MoveTo( value_type* pDestRing );
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
                : pNext(this)
                , pPrev(this)
                { }
            /**
             * Creates a new item and add it to an existing ring container.
             * Note: the newly created item will be inserted just before item pRing.
             * @param pRing ring container to add the created item to
             */
            Ring( value_type* pRing );
            /** @return the next item in the ring container */
            value_type* GetNextInRing()
                { return static_cast<value_type *>(pNext); }
            /** @return the previous item in the ring container */
            value_type* GetPrevInRing()
                { return static_cast<value_type *>(pPrev); }
            /** @return the next item in the ring container */
            const_value_type* GetNextInRing() const
                { return static_cast<value_type *>(pNext); }
            /** @return the previous item in the ring container */
            const_value_type* GetPrevInRing() const
                { return static_cast<value_type *>(pPrev); }
            /** @return true if and only if this item is alone in its ring */
            bool unique() const
                { return algo::unique(static_cast< const_value_type* >(this)); }

        private:
            /** internal implementation class -- not for external use */
            struct Ring_node_traits
            {
                typedef Ring node;
                typedef Ring* node_ptr;
                typedef const Ring* const_node_ptr;
                static node_ptr get_next(const_node_ptr n) { return const_cast<node_ptr>(n)->pNext; };
                static void set_next(node_ptr n, node_ptr next) { n->pNext = next; };
                static node_ptr get_previous(const_node_ptr n) { return const_cast<node_ptr>(n)->pPrev; };
                static void set_previous(node_ptr n, node_ptr previous) { n->pPrev = previous; };
            };
            friend ring_container;
            friend const_ring_container;
            friend typename ring_container::iterator;
            friend typename ring_container::const_iterator;
            friend typename const_ring_container::iterator;
            friend typename const_ring_container::const_iterator;
            friend class boost::iterator_core_access;
            typedef boost::intrusive::circular_list_algorithms<Ring_node_traits> algo;
            Ring* pNext;
            Ring* pPrev;
    };

    template <typename value_type>
    inline Ring<value_type>::Ring( value_type* pObj )
        : pNext(this)
        , pPrev(this)
    {
        if( pObj )
        {
            algo::link_before(pObj, this);
        }
    }

    template <typename value_type>
    inline void Ring<value_type>::MoveTo(value_type* pDestRing)
    {
        value_type* pThis = static_cast< value_type* >(this);
        unlink();
        // insert into "new"
        if (pDestRing)
        {
            algo::link_before(pDestRing, pThis);
        }
    }

    /**
     * helper class that provides Svalue_typeL-style container iteration to the ring
     */
    template <typename value_type>
    class RingContainer final
    {
        private:
            /** the item in the ring where iteration starts */
            value_type* m_pStart;
            typedef typename std::remove_const<value_type>::type nonconst_value_type;

        public:
            RingContainer( value_type* pRing ) : m_pStart(pRing) {};
            typedef RingIterator<value_type> iterator;
            typedef RingIterator<const value_type> const_iterator;
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
            /**
             * Merges two ring containers. All item from both ring containers will
             * be in the same ring container in the end.
             * Note: The items of this ring container will be inserted just before
             * item pDestRing
             * @param pDestRing the container to merge this container with
             */
            void merge( RingContainer< value_type > aDestRing )
            {
                // first check that we aren't merged already, swapping would
                // actually un-merge in this case!
                assert(m_pStart->pPrev != aDestRing.m_pStart);
                assert(m_pStart != aDestRing.m_pStart->pPrev);
                std::swap(*(&m_pStart->pPrev->pNext), *(&aDestRing.m_pStart->pPrev->pNext));
                std::swap(*(&m_pStart->pPrev), *(&aDestRing.m_pStart->pPrev));
            }
    };

    template <typename value_type>
    class RingIterator final : public boost::iterator_facade<
          RingIterator<value_type>
        , value_type
        , boost::forward_traversal_tag
        >
    {
        private:
            typedef typename std::remove_const<value_type>::type nonconst_value_type;
        public:
            RingIterator()
                : m_pCurrent(nullptr)
                , m_pStart(nullptr)
            {}
            explicit RingIterator(nonconst_value_type* pRing, bool bStart = true)
                : m_pCurrent(nullptr)
                , m_pStart(pRing)
            {
                if(!bStart)
                    m_pCurrent = m_pStart;
            }

        private:
            friend class boost::iterator_core_access;
            void increment()
                { m_pCurrent = m_pCurrent ? m_pCurrent->GetNextInRing() : m_pStart->GetNextInRing(); }
            bool equal(RingIterator const& other) const
            {
                // we never want to compare iterators from
                // different rings or starting points
                assert(m_pStart == other.m_pStart);
                return m_pCurrent == other.m_pCurrent;
            }
            value_type& dereference() const
                { return m_pCurrent ? *m_pCurrent : * m_pStart; }
            /**
             * value_type is:
             * - pointing to the current item in the iteration in general
             * - nullptr if on the first item (begin())
             * - m_pStart when beyond the last item (end())
             */
            nonconst_value_type* m_pCurrent;
            /** the first item of the iteration */
            nonconst_value_type* m_pStart;
    };

    template <typename value_type>
    inline typename Ring<value_type>::ring_container Ring<value_type>::GetRingContainer()
        { return Ring<value_type>::ring_container(static_cast< value_type* >(this)); };

    template <typename value_type>
    inline typename Ring<value_type>::const_ring_container Ring<value_type>::GetRingContainer() const
        { return Ring<value_type>::const_ring_container(static_cast< const_value_type* >(this)); };

    template <typename value_type>
    inline typename RingContainer<value_type>::iterator RingContainer<value_type>::begin()
        { return RingContainer<value_type>::iterator(const_cast< nonconst_value_type* >(m_pStart)); };

    template <typename value_type>
    inline typename RingContainer<value_type>::iterator RingContainer<value_type>::end()
        { return RingContainer<value_type>::iterator(const_cast< nonconst_value_type* >(m_pStart), false); };

    template <typename value_type>
    inline typename RingContainer<value_type>::const_iterator RingContainer<value_type>::begin() const
        { return RingContainer<value_type>::const_iterator(const_cast< nonconst_value_type* >(m_pStart)); };

    template <typename value_type>
    inline typename RingContainer<value_type>::const_iterator RingContainer<value_type>::end() const
        { return RingContainer<value_type>::const_iterator(const_cast< nonconst_value_type* >(m_pStart), false); };
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
