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
#include "registry/reader.hxx"
#include "registry/types.h"
#include "rtl/string.hxx"
#include "rtl/textenc.h"
#include "rtl/ustring.hxx"

#include <memory>
#include <vector>

using codemaker::ExceptionTree;
using codemaker::ExceptionTreeNode;

ExceptionTreeNode * ExceptionTreeNode::add(rtl::OString const & theName) {
    std::auto_ptr< ExceptionTreeNode > node(new ExceptionTreeNode(theName));
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
    throw( CannotDumpException )
{
    typedef std::vector< rtl::OString > OStringList;
    OStringList stringlist;
    bool runtimeException = false;
    for (rtl::OString n(name); n != "com/sun/star/uno/Exception";) {
        if (n == "com/sun/star/uno/RuntimeException") {
            runtimeException = true;
            break;
        }
        stringlist.push_back(n);
        typereg::Reader reader(manager->getTypeReader(n));
        if (!reader.isValid())
            throw CannotDumpException(
                ::rtl::OString("Unknown type '" + n.replace('/', '.')
                               + "', incomplete type library."));

        OSL_ASSERT(
            reader.getTypeClass() == RT_TYPE_EXCEPTION
            && reader.getSuperTypeCount() == 1);
        n = rtl::OUStringToOString(
            reader.getSuperTypeName(0), RTL_TEXTENCODING_UTF8);
    }
    if (!runtimeException) {
        ExceptionTreeNode * node = &m_root;
        for (OStringList::reverse_iterator i(stringlist.rbegin()); !node->present; ++i) {
            if (i == stringlist.rend()) {
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
