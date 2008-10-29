/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: nodevisitor.cxx,v $
 * $Revision: 1.8 $
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

#include "precompiled_configmgr.hxx"
#include "sal/config.h"

#include "osl/diagnose.h"

#include "flags.hxx"
#include "nodevisitor.hxx"
#include "node.hxx"
#include "treefragment.hxx"

namespace configmgr { namespace data {

NodeVisitor::~NodeVisitor() {}

bool NodeVisitor::visitNode(sharable::Node * node) {
    switch (node->info.type & data::Type::mask_nodetype) {
    case data::Type::nodetype_value:
        return handle(&node->value);
    case data::Type::nodetype_group:
        return handle(&node->group);
    case data::Type::nodetype_set:
        return handle(&node->set);
    default:
        OSL_ASSERT(false);
        return false;
    }
}

bool NodeVisitor::visitChildren(sharable::GroupNode * node) {
    for (sharable::Node * child = node->getFirstChild(); child != 0;
         child = node->getNextChild(child))
    {
        if (visitNode(child)) {
            return true;
        }
    }
    return false;
}

bool NodeVisitor::handle(sharable::Node *) {
    return false;
}

bool NodeVisitor::handle(sharable::ValueNode * node) {
    return handle(sharable::node(node));
}

bool NodeVisitor::handle(sharable::GroupNode * node) {
    return handle(sharable::node(node));
}

bool NodeVisitor::handle(sharable::SetNode * node) {
    return handle(sharable::node(node));
}

SetVisitor::~SetVisitor() {}

bool SetVisitor::visitTree(sharable::TreeFragment * tree) {
    return handle(tree);
}

bool SetVisitor::visitElements(sharable::SetNode * node) {
    for (sharable::TreeFragment * element = node->getFirstElement();
         element != 0; element = node->getNextElement(element))
    {
        if (handle(element)) {
            return true;
        }
    }
    return false;
}

bool SetVisitor::handle(sharable::TreeFragment * tree) {
    return visitNode(tree->getRootNode());
}

} }
