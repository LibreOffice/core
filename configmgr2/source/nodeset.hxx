/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* $RCSfile: code,v $
*
* $Revision: 1.4 $
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
************************************************************************/

#ifndef INCLUDED_CONFIGMGR_NODESET_HXX
#define INCLUDED_CONFIGMGR_NODESET_HXX

#include "sal/config.h"

#include <cstddef>

#include "boost/noncopyable.hpp"
#include "osl/diagnose.h"
#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"
#include "stl/hash_set"

#include "node.hxx"

namespace configmgr {

struct NodeHash {
    std::size_t operator ()(rtl::Reference< Node > const & node) {
        OSL_ASSERT(node.is());
        return node->getName().hashCode();
    }
};

struct NodeEqual {
    bool operator ()(
        rtl::Reference< Node > const & node1,
        rtl::Reference< Node > const & node2)
    {
        OSL_ASSERT(node1.is() && node2.is());
        return node1->getName() == node2->getName();
    }
};

class NodeSet:
    public std::hash_set< rtl::Reference< Node >, NodeHash, NodeEqual >,
    private boost::noncopyable
{
public:
    NodeSet();

    ~NodeSet();

    void clone(Node * parent, NodeSet * target) const;
};

}

#endif
