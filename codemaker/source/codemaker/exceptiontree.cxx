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


#include "codemaker/exceptiontree.hxx"
#include "codemaker/typemanager.hxx"

#include "osl/diagnose.h"
#include "rtl/ref.hxx"
#include "rtl/string.hxx"
#include "rtl/textenc.h"
#include "rtl/ustring.hxx"
#include "unoidl/unoidl.hxx"

#include <memory>
#include <vector>

using codemaker::ExceptionTree;
using codemaker::ExceptionTreeNode;

ExceptionTreeNode * ExceptionTreeNode::add(rtl::OString const & theName) {
    std::unique_ptr< ExceptionTreeNode > node(new ExceptionTreeNode(theName));
    children.push_back(node.get());
    return node.release();
}

void ExceptionTreeNode::clearChildren() {
    for (Children::iterator i(children.begin()); i != children.end(); ++i) {
        delete *i;
    }
    children.clear();
}

void ExceptionTree::add(
    rtl::OString const & name, rtl::Reference< TypeManager > const & manager)
{
    std::vector< rtl::OString > list;
    bool bRuntimeException = false;
    for (rtl::OString n(name); n != "com.sun.star.uno.Exception";) {
        if (n == "com.sun.star.uno.RuntimeException") {
            bRuntimeException = true;
            break;
        }
        list.push_back(n);
        rtl::Reference< unoidl::Entity > ent;
        codemaker::UnoType::Sort s = manager->getSort(b2u(n), &ent);
        (void) s; // WaE: unused variable
        assert(s == codemaker::UnoType::SORT_EXCEPTION_TYPE);
        n = u2b(
            static_cast< unoidl::ExceptionTypeEntity * >(ent.get())->
            getDirectBase());
        assert(!n.isEmpty());
    }
    if (!bRuntimeException) {
        ExceptionTreeNode * node = &m_root;
        for (std::vector< rtl::OString >::reverse_iterator i(list.rbegin());
             !node->present; ++i)
        {
            if (i == list.rend()) {
                node->setPresent();
                break;
            }
            for (ExceptionTreeNode::Children::iterator j(
                     node->children.begin());;
                 ++j)
            {
                if (j == node->children.end()) {
                    node = node->add(*i);
                    break;
                }
                if ((*j)->name == *i) {
                    node = *j;
                    break;
                }
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
