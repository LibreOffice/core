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

#ifndef INCLUDED_OOX_TOKEN_NAMESPACEMAP_HXX
#define INCLUDED_OOX_TOKEN_NAMESPACEMAP_HXX

#include <map>
#include <rtl/instance.hxx>
#include <rtl/ustring.hxx>

namespace oox {



/** A map that contains all XML namespace URLs used in the filters. */
struct NamespaceMap
{
    std::map< sal_Int32, OUString > maTransitionalNamespaceMap;
    std::map< sal_Int32, OUString > maStrictNamespaceMap;

    NamespaceMap();

    typedef std::map< sal_Int32, OUString >::iterator iterator;
    typedef std::map< sal_Int32, OUString >::const_iterator const_iterator;
};

/** Thread-save singleton of a map of all supported XML namespace URLs. */
struct StaticNamespaceMap : public ::rtl::Static< NamespaceMap, StaticNamespaceMap > {};



}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
