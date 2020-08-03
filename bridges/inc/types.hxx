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

#include <typelib/typeclass.h>
#include <typelib/typedescription.h>

namespace bridges::cpp_uno::shared {

/**
 * Determines whether a type is a "simple" type (VOID, BOOLEAN, BYTE, SHORT,
 * UNSIGNED SHORT, LONG, UNSIGNED LONG, HYPER, UNSIGNED HYPER, FLOAT, DOUBLE,
 * CHAR, or an enum type).
 *
 * @param typeClass a type class
 * @return true if the given type is "simple"
 */
bool isSimpleType(typelib_TypeClass typeClass);

/**
 * Determines whether a type is a "simple" type (VOID, BOOLEAN, BYTE, SHORT,
 * UNSIGNED SHORT, LONG, UNSIGNED LONG, HYPER, UNSIGNED HYPER, FLOAT, DOUBLE,
 * CHAR, or an enum type).
 *
 * @param type a non-null pointer to a type description reference
 * @return true if the given type is "simple"
 */
bool isSimpleType(typelib_TypeDescriptionReference const * type);

/**
 * Determines whether a type is a "simple" type (VOID, BOOLEAN, BYTE, SHORT,
 * UNSIGNED SHORT, LONG, UNSIGNED LONG, HYPER, UNSIGNED HYPER, FLOAT, DOUBLE,
 * CHAR, or an enum type).
 *
 * @param type a non-null pointer to a type description
 * @return true if the given type is "simple"
 */
bool isSimpleType(typelib_TypeDescription const * type);

/**
 * Determines whether a type relates to an interface type (is itself an
 * interface type, or might contain entities of interface type).
 *
 * @param type a non-null pointer to a type description
 * @return true if the given type relates to an interface type
 */
bool relatesToInterfaceType(typelib_TypeDescription const * type);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
