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

#include <string.h>

#include <osl/mutex.hxx>
#include <comphelper/servicehelper.hxx>

#include "StyleMap.hxx"

using namespace osl;
using namespace css::uno;
using namespace css::lang;

namespace
{

class theStyleMapUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theStyleMapUnoTunnelId>
{};

} // end anonymous namespace

StyleMap::StyleMap()
{
}

StyleMap::~StyleMap()
{
}

// XUnoTunnel & co
const Sequence<sal_Int8>& StyleMap::getUnoTunnelId() throw()
{
    return theStyleMapUnoTunnelId::get().getSeq();
}

StyleMap* StyleMap::getImplementation(Reference<XInterface> xInterface) throw()
{
    Reference<XUnoTunnel> xUnoTunnel(xInterface, UNO_QUERY);
    if (xUnoTunnel.is())
    {
        return reinterpret_cast<StyleMap*>(xUnoTunnel->getSomething(StyleMap::getUnoTunnelId()));
    }

    return nullptr;
}

// XUnoTunnel
sal_Int64 SAL_CALL StyleMap::getSomething(const Sequence<sal_Int8>& rId)
    throw(RuntimeException, std::exception)
{
    if (rId.getLength() == 16 &&
        memcmp(getUnoTunnelId().getConstArray(), rId.getConstArray(), 16) == 0)
    {
        return reinterpret_cast<sal_Int64>(this);
    }

    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
