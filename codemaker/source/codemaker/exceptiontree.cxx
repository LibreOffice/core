/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: exceptiontree.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:23:01 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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
