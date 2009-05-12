/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: nodevisitor.hxx,v $
 * $Revision: 1.7 $
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

#ifndef INCLUDED_CONFIGMGR_SOURCE_INC_NODEVISITOR_HXX
#define INCLUDED_CONFIGMGR_SOURCE_INC_NODEVISITOR_HXX

#include "sal/config.h"

namespace configmgr { namespace sharable {
    struct GroupNode;
    struct SetNode;
    struct TreeFragment;
    struct ValueNode;
    union Node;
} }

namespace configmgr { namespace data {

class NodeVisitor {
public:
    virtual ~NodeVisitor() = 0;

    bool visitNode(sharable::Node * node);

    bool visitChildren(sharable::GroupNode * node);

protected:
    virtual bool handle(sharable::Node * node);

    virtual bool handle(sharable::ValueNode * node);

    virtual bool handle(sharable::GroupNode * node);

    virtual bool handle(sharable::SetNode * node);
};

class SetVisitor: public NodeVisitor {
public:
    virtual ~SetVisitor() = 0;

    bool visitTree(sharable::TreeFragment * tree);

    bool visitElements(sharable::SetNode * node);

protected:
    using NodeVisitor::handle;

    virtual bool handle(sharable::TreeFragment * tree);
};

} }

#endif
