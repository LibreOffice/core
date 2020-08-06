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
#include <sal/types.h>

namespace com::sun::star::uno {
    class Any;
    class Type;
}

namespace configmgr {

enum Type {
    TYPE_ERROR, TYPE_NIL, TYPE_ANY, TYPE_BOOLEAN, TYPE_SHORT, TYPE_INT,
    TYPE_LONG, TYPE_DOUBLE, TYPE_STRING, TYPE_HEXBINARY, TYPE_BOOLEAN_LIST,
    TYPE_SHORT_LIST, TYPE_INT_LIST, TYPE_LONG_LIST, TYPE_DOUBLE_LIST,
    TYPE_STRING_LIST, TYPE_HEXBINARY_LIST };

bool isListType(Type type);

Type elementType(Type type);

css::uno::Type const & mapType(Type type);

Type getDynamicType(css::uno::Any const & value);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
