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

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Any.hxx>

namespace http_dav_ucp
{

enum Depth { DAVZERO = 0, DAVONE = 1, DAVINFINITY = -1 };

enum ProppatchOperation { PROPSET = 0, PROPREMOVE = 1 };

struct ProppatchValue
{
    ProppatchOperation       operation;
    OUString                 name;
    css::uno::Any            value;

    ProppatchValue( const ProppatchOperation o,
                    const OUString & n,
                    const css::uno::Any & v )
    : operation( o ), name( n ), value( v ) {}
};

} // namespace http_dav_ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
