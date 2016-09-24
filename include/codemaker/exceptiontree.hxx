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

#ifndef INCLUDED_CODEMAKER_EXCEPTIONTREE_HXX
#define INCLUDED_CODEMAKER_EXCEPTIONTREE_HXX

#include <codemaker/global.hxx>
#include <rtl/ref.hxx>
#include <rtl/string.hxx>

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
    ExceptionTreeNode(ExceptionTreeNode &) = delete;
    void operator =(ExceptionTreeNode) = delete;

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
   an instance of ExceptionTreeNode, where name gives the name of the UNO
   exception type, present is true iff the given exception type is a member of
   the set S, and children contains all the relevant direct subtypes of the
   given exception type, in no particular order (for nodes other than the root
   node it holds that children is non-empty iff present is false).
 */
class ExceptionTree {
public:
    ExceptionTree(): m_root("com.sun.star.uno.Exception") {}

    ~ExceptionTree() {}

    /**
       Builds the exception hierarchy, by adding one exception type at a time.

       This function can be called more than once for the same exception name.

       @param name the name of a UNO exception type; it is an error if the given
       name does not represent a UNO exception type

       @param manager a type manager, used to resolve type names; it is an error
       if different calls to this member function use different, incompatible
       type managers
     */
    void add(
        rtl::OString const & name,
        rtl::Reference< TypeManager > const & manager);

    /**
       Gives access to the resultant exception hierarchy.

       @return a reference to the root of the exception hierarchy, as
       formed by all previous calls to add; it is an error if any calls to add
       follow the first call to getRoot
     */
    ExceptionTreeNode const & getRoot() const { return m_root; }

private:
    ExceptionTree(ExceptionTree &) = delete;
    void operator =(const ExceptionTree&) = delete;

    ExceptionTreeNode m_root;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
