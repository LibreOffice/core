/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_codemaker.hxx"

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

void ExceptionTree::add(rtl::OString const & name, TypeManager const & manager)
    throw( CannotDumpException )
{
    typedef std::vector< rtl::OString > List;
    List list;
    bool runtimeException = false;
    for (rtl::OString n(name); n != "com/sun/star/uno/Exception";) {
        if (n == "com/sun/star/uno/RuntimeException") {
            runtimeException = true;
            break;
        }
        list.push_back(n);
        typereg::Reader reader(manager.getTypeReader(n));
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
        for (List::reverse_iterator i(list.rbegin()); !node->present; ++i) {
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
