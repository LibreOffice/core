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

#ifndef INCLUDED_CONFIGMGR_SOURCE_NODEMAP_HXX
#define INCLUDED_CONFIGMGR_SOURCE_NODEMAP_HXX

#include <sal/config.h>
#include <map>
#include <rtl/ref.hxx>
#include <node.hxx>

namespace configmgr {

typedef std::map< OUString, rtl::Reference< Node > > NodeMapImpl;
class NodeMap
{
    NodeMapImpl aImpl;
  public:
    typedef NodeMapImpl::iterator iterator;
    typedef NodeMapImpl::const_iterator const_iterator;
    typedef NodeMapImpl::value_type value_type;

     NodeMap() {}
    ~NodeMap() {}
    void clear() { aImpl.clear(); }
    bool empty() { return aImpl.empty(); }
    void erase(const iterator &it) { aImpl.erase(it); }
    void erase(const OUString &aStr) { aImpl.erase(aStr); }
    iterator find(const OUString &aStr) { return aImpl.find( aStr ); }

    const_iterator find(const OUString &aStr) const { return aImpl.find( aStr ); }
    rtl::Reference<Node> &operator[](const OUString &aStr) { return aImpl[aStr]; }
    iterator begin() { return aImpl.begin(); }
    const_iterator begin() const { return aImpl.begin(); }

    iterator end() { return aImpl.end(); }
    const_iterator end() const { return aImpl.end(); }
    std::pair<iterator,bool> insert(const value_type &vt) { return aImpl.insert(vt); }
};

void cloneNodeMap(NodeMap const & source, NodeMap * target);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
