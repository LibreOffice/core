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

#include "ring.hxx"
#include <boost/intrusive/circular_list_algorithms.hpp>

namespace
{
    struct Ring_node_traits
    {
        typedef Ring node;
        typedef Ring* node_ptr;
        typedef const Ring* const_node_ptr;
        static node_ptr get_next(const_node_ptr n) { return n->GetNext(); };
        static void set_next(node_ptr n, node_ptr next) { n->pNext = next; };
        static node_ptr get_previous(const_node_ptr n) { return n->GetPrev(); };
        static void set_previous(node_ptr n, node_ptr previous) { n->pPrev = previous; };
    };
    typedef boost::intrusive::circular_list_algorithms<Ring_node_traits> algo;
}


Ring::Ring()
{
    algo::init_header(this);
}

Ring::Ring( Ring *pObj )
{
    if( !pObj )
        algo::init_header(this);
    else
        algo::link_before(pObj, this);
}

Ring::~Ring()
{
    algo::unlink(this);
}

void Ring::MoveTo(Ring *pDestRing)
{
    // insert into "new"
    if( pDestRing )
        algo::transfer(pDestRing, this);
    else
        algo::unlink(this);

}

void Ring::MoveRingTo(Ring *pDestRing)
{
    algo::transfer(pDestRing, this, this);
}

sal_uInt32 Ring::numberOf() const
{
    return algo::count(this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
