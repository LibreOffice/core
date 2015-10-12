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

#ifndef INCLUDED_CONFIGMGR_SOURCE_PARTIAL_HXX
#define INCLUDED_CONFIGMGR_SOURCE_PARTIAL_HXX

#include <sal/config.h>

#include <set>
#include <boost/unordered_map.hpp>

#include "path.hxx"
#include <rtl/ustring.hxx>

namespace configmgr {

class Partial {
public:
    enum Containment { CONTAINS_NOT, CONTAINS_SUBNODES, CONTAINS_NODE };

    Partial(
        std::set< OUString > const & includedPaths,
        std::set< OUString > const & excludedPaths);

    ~Partial();

    Containment contains(Path const & path) const;

private:
    Partial(const Partial&) = delete;
    Partial& operator=(const Partial&) = delete;

    struct Node {
        typedef boost::unordered_map< OUString, Node, OUStringHash > Children;

        Node(): startInclude(false) {}
        void clear() { startInclude=false; children.clear(); }

        Children children;
        bool startInclude;
    };

    Node root_;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
