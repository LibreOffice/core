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

#include <sal/config.h>

#include <algorithm>
#include <cassert>

#include <rtl/ustring.hxx>

#include "node.hxx"
#include "nodemap.hxx"

namespace configmgr {

void NodeMap::cloneInto(NodeMap * target) const
{
    assert(target != nullptr && target->empty());
    NodeMapImpl clone(maImpl);
    for (NodeMapImpl::iterator i(clone.begin()); i != clone.end(); ++i) {
        i->second = i->second->clone(true);
    }
    std::swap(clone, target->maImpl);
    target->clearCache();
}

rtl::Reference< Node > NodeMap::findNode(int layer, OUString const & name) const
{
    const_iterator i;
    if (maCache == end() || maCache->first != name)
        maCache = const_cast< NodeMap *>(this)->maImpl.find(name);
    i = maCache;
    return i == end() || i->second->getLayer() > layer
        ? rtl::Reference< Node >() : i->second;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
