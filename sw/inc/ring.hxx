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
#include <boost/iterator/iterator_facade.hpp>
#include <boost/intrusive/circular_list_algorithms.hpp>

namespace sw
{
    class Ring_node_traits;
    template <class T> class RingIterator;

    template <class T>
    class SW_DLLPUBLIC Ring
    {
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
        friend class Ring_node_traits;
        typedef boost::intrusive::circular_list_algorithms<Ring_node_traits> algo;
        T* pNext;
        T* pPrev;    ///< In order to speed up inserting and deleting.

    protected:
        Ring()
            { algo::init_header(static_cast< T* >(this)); }
        Ring( T* );
    public:
        typedef RingIterator<T> iterator;
        typedef RingIterator<T> const_iterator;
        virtual ~Ring()
            { algo::unlink(static_cast< T* >(this)); };
        void MoveTo( T* pDestRing );
        void MoveRingTo( T* pDestRing );

        T* GetNext() const       { return pNext; }
        T* GetPrev() const       { return pPrev; }
        // unfortunately we cant name these STL-conforming, as some derived classes
        // also derive from other STL containers (which is bad anyway, but ...)
        iterator beginRing();
        iterator endRing();

        sal_uInt32 numberOf() const
            { return algo::count(static_cast< const T* >(this)); }
    };

    template <class T>
    inline Ring<T>::Ring( T* pObj )
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

    template <class T>
    inline void Ring<T>::MoveRingTo(T* pDestRing)
    {
        std::swap(*(&pPrev->pNext), *(&pDestRing->pPrev->pNext));
        std::swap(*(&pPrev), *(&pDestRing->pPrev));
    }

    template <class T>
    class RingIterator : public boost::iterator_facade<
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
                { return m_pCurrent == other.m_pCurrent && m_pStart == m_pStart; }
            T& dereference() const
                { return m_pCurrent ? *m_pCurrent : * m_pStart; }
            T* m_pCurrent;
            T* m_pStart;
    };

    template <class T>
    inline typename Ring<T>::iterator Ring<T>::beginRing()
        { return Ring<T>::iterator(static_cast< T* >(this)); };

    template <class T>
    inline typename Ring<T>::iterator Ring<T>::endRing()
        { return Ring<T>::iterator(static_cast< T* >(this), false); };

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
