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

#ifndef INCLUDED_CODEMAKER_EXCEPTIONTREE_HXX
#define INCLUDED_CODEMAKER_EXCEPTIONTREE_HXX

#include "codemaker/global.hxx"
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
    ExceptionTreeNode * add(rtl::OString const & theName);

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
    void add(rtl::OString const & name, TypeManager const & manager)
        throw( CannotDumpException );

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
