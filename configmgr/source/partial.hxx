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

#ifndef INCLUDED_CONFIGMGR_SOURCE_PARTIAL_HXX
#define INCLUDED_CONFIGMGR_SOURCE_PARTIAL_HXX

#include "sal/config.h"

#include <map>
#include <set>

#include "boost/noncopyable.hpp"

#include "path.hxx"

namespace rtl { class OUString; }

namespace configmgr {

class Partial: private boost::noncopyable {
public:
    enum Containment { CONTAINS_NOT, CONTAINS_SUBNODES, CONTAINS_NODE };

    Partial(
        std::set< rtl::OUString > const & includedPaths,
        std::set< rtl::OUString > const & excludedPaths);

    ~Partial();

    Containment contains(Path const & path) const;

private:
    struct Node {
        typedef std::map< rtl::OUString, Node > Children;

        Node(): startInclude(false) {}

        Children children;
        bool startInclude;
    };

    Node root_;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
