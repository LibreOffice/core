/*************************************************************************
 *
 *  $RCSfile: exceptiontree.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 03:08:45 $
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

#ifndef INCLUDED_codemaker_exceptiontree_hxx
#define INCLUDED_codemaker_exceptiontree_hxx

#include "rtl/string.hxx"

#include <vector>

class TypeManager;

namespace codemaker {

/**
   Represents a node of the hierarchy from the ExceptionTree class.
 */
struct ExceptionTreeNode {
    typedef std::vector< ExceptionTreeNode * > Children;

    // Internally used by ExceptionTree:
    ExceptionTreeNode(rtl::OString const & theName):
        name(theName), present(false) {}

    // Internally used by ExceptionTree:
    ~ExceptionTreeNode() { clearChildren(); }

    // Internally used by ExceptionTree:
    void setPresent() { present = true; clearChildren(); }

    // Internally used by ExceptionTree:
    ExceptionTreeNode * add(rtl::OString const & name);

    rtl::OString name;
    bool present;
    Children children;

private:
    ExceptionTreeNode(ExceptionTreeNode &); // not implemented
    void operator =(ExceptionTreeNode); // not implemented

    void clearChildren();
};

/**
   Represents the hierarchy formed by a set of UNO exception types.

   The hierarchy is rooted at com.sun.star.uno.Exception.  For each exception E
   from the given set S, the hierarchy from com.sun.star.uno.Exception to the
   first supertype E' of E which is itself a member of S is represented (i.e.,
   subtypes that are hidden by supertypes are pruned from the hierarchy).  The
   exception com.sun.star.uno.RuntimeException and its subtypes are pruned
   completely from the hierarchy.  Each node of the hierarchy is represented by
   an instance of ExceptionTreeNode, where name gives the slashified name of
   the UNO exception type, present is true iff the given exception type is a
   member of the set S, and children contains all the relevant direct subtypes
   of the given exception type, in no particular order (for nodes other than the
   root node it holds that children is non-empty iff present is false).
 */
class ExceptionTree {
public:
    ExceptionTree(): m_root("com/sun/star/uno/Exception") {}

    ~ExceptionTree() {}

    /**
       Builds the exception hierarchy, by adding one exception type at a time.

       This function can be called more than once for the same exception name.

       @param name the name of a UNO exception type, in slashified form; it is
       an error if the given name does not represent a UNO exception type

       @param manager a type manager, used to resolve type names; it is an error
       if different calls to this member function use different, incompatible
       type managers
     */
    void add(rtl::OString const & name, TypeManager const & manager);

    /**
       Gives access to the resultant exception hierarchy.

       @return a non-null pointer to the root of the exception hierarchy, as
       formed by all previous calls to add; it is an error if any calls to add
       follow the first call to getRoot
     */
    ExceptionTreeNode const * getRoot() const { return &m_root; }

private:
    ExceptionTree(ExceptionTree &); // not implemented
    void operator =(ExceptionTree); // not implemented

    ExceptionTreeNode m_root;
};

}

#endif
