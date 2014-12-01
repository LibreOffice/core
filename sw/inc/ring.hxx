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

class Ring_node_traits;
class RingIterator;

class SW_DLLPUBLIC Ring
{
    friend class Ring_node_traits;
    typedef RingIterator iterator;
    typedef RingIterator const_iterator;
    Ring* pNext;
    Ring* pPrev;    ///< In order to speed up inserting and deleting.

protected:
    Ring();
    Ring( Ring * );
public:
    virtual ~Ring();
    void MoveTo( Ring *pDestRing );
    void MoveRingTo( Ring *pDestRing );

    Ring* GetNext() const       { return pNext; }
    Ring* GetPrev() const       { return pPrev; }

    sal_uInt32 numberOf() const;
};

class RingIterator : public boost::iterator_facade<
      RingIterator
    , Ring
    , boost::forward_traversal_tag
    >
{
    public:
        RingIterator()
            : m_pCurrent(nullptr)
            , m_pStart(nullptr)
        {}
        explicit RingIterator(Ring* pRing, bool bStart = true)
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
        Ring& dereference() const
            { return m_pCurrent ? *m_pCurrent : * m_pStart; }
        Ring* m_pCurrent;
        Ring* m_pStart;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
