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

#pragma once

#include <sal/config.h>
#include "config_map.hxx"
#include <rtl/ref.hxx>
#include "node.hxx"

namespace configmgr {

typedef config_map< rtl::Reference< Node > > NodeMapImpl;
class NodeMap
{
    NodeMapImpl maImpl;

    NodeMap(const NodeMap &rMap) = delete;

  public:
    typedef NodeMapImpl::iterator iterator;
    typedef NodeMapImpl::const_iterator const_iterator;
    typedef NodeMapImpl::value_type value_type;

    NodeMap() { clearCache(); }
    bool empty() const { return maImpl.empty(); }
    iterator find(const OUString &aStr) { return maImpl.find( aStr ); }

    const_iterator find(const OUString &aStr) const { return maImpl.find( aStr ); }
    iterator begin() { return maImpl.begin(); }
    const_iterator begin() const { return maImpl.begin(); }

    iterator end() { return maImpl.end(); }
    const_iterator end() const { return maImpl.end(); }

    rtl::Reference<Node> &operator[](const OUString &aStr) { clearCache(); return maImpl[aStr]; }
    std::pair<iterator,bool> insert(const value_type &vt) { clearCache(); return maImpl.insert(vt); }
    void erase(const iterator &it) { maImpl.erase(it); clearCache(); }
    void erase(const OUString &aStr) { maImpl.erase(aStr); clearCache(); }

    rtl::Reference< Node > findNode(int layer, OUString const & name) const;
    void cloneInto(NodeMap * target) const;

private:
    // We get a large number of repeated identical lookups.
    mutable const_iterator maCache;
    void clearCache() { maCache = maImpl.end(); }
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
