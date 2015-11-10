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

#ifndef INCLUDED_CODEMAKER_UNOTYPE_HXX
#define INCLUDED_CODEMAKER_UNOTYPE_HXX

#include <sal/types.h>

#include <vector>

namespace rtl { class OString; }

namespace codemaker {

namespace UnoType {
    /**
       An enumeration of all the sorts of relevant UNOIDL entities.
     */
    enum Sort {
        SORT_VOID,
        SORT_BOOLEAN,
        SORT_BYTE,
        SORT_SHORT,
        SORT_UNSIGNED_SHORT,
        SORT_LONG,
        SORT_UNSIGNED_LONG,
        SORT_HYPER,
        SORT_UNSIGNED_HYPER,
        SORT_FLOAT,
        SORT_DOUBLE,
        SORT_CHAR,
        SORT_STRING,
        SORT_TYPE,
        SORT_ANY,
        SORT_SEQUENCE_TYPE,
        SORT_MODULE,
        SORT_ENUM_TYPE,
        SORT_PLAIN_STRUCT_TYPE,
        SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE,
        SORT_INSTANTIATED_POLYMORPHIC_STRUCT_TYPE,
        SORT_EXCEPTION_TYPE,
        SORT_INTERFACE_TYPE,
        SORT_TYPEDEF,
        SORT_CONSTANT_GROUP,
        SORT_SINGLE_INTERFACE_BASED_SERVICE,
        SORT_ACCUMULATION_BASED_SERVICE,
        SORT_INTERFACE_BASED_SINGLETON,
        SORT_SERVICE_BASED_SINGLETON
    };

    /**
       Decomposes a UNO type name or UNO type registry name.

       @param type a binary UNO type name or UNO type registry name

       @param rank if non-null, returns the rank of the denoted UNO type (which
       is zero for any given type that does not denote a UNO sequence type)

       @param arguments if non-null, the type arguments are stripped from an
       instantiated polymorphic struct type and returned via this parameter (in
       the correct order); if null, type arguments are not stripped from
       instantiated polymorphic struct types

       @return the base part of the given type
     */
    rtl::OString decompose(
        rtl::OString const & type, sal_Int32 * rank = nullptr,
        std::vector< rtl::OString > * arguments = nullptr);
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
