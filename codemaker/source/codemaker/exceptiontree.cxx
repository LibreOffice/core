/*************************************************************************
 *
 *  $RCSfile: exceptiontree.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 03:10:27 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

ExceptionTreeNode * ExceptionTreeNode::add(rtl::OString const & name) {
    std::auto_ptr< ExceptionTreeNode > node(new ExceptionTreeNode(name));
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
