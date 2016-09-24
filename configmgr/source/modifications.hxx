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

#ifndef INCLUDED_CONFIGMGR_SOURCE_MODIFICATIONS_HXX
#define INCLUDED_CONFIGMGR_SOURCE_MODIFICATIONS_HXX

#include <sal/config.h>

#include <vector>

#include <boost/unordered_map.hpp>
#include <config_dconf.h>

namespace configmgr {

class Modifications {
public:
    struct Node {
        typedef boost::unordered_map<OUString, Node, OUStringHash> Children;

        Children children;
    };

    Modifications();

    ~Modifications();

    void add(std::vector<OUString> const & path);

    void remove(std::vector<OUString> const & path);

#if ENABLE_DCONF
    void clear() { root_.children.clear(); }
#endif

    bool empty() const { return root_.children.empty(); }

    Node const & getRoot() const { return root_;}

private:
    Modifications(const Modifications&) = delete;
    Modifications& operator=(const Modifications&) = delete;

    Node root_;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
