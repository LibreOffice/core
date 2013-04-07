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

#include "sal/types.h"
#include <rtl/ustring.hxx>

#include <vector>

namespace codemaker {

namespace UnoType {
    /**
       An enumeration of all the sorts of UNO types.

       All complex UNO types are subsumed under SORT_COMPLEX.
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
        SORT_COMPLEX
    };

    /**
       Maps from a binary UNO type name or UNO type registry name to its type
       sort.

       @param type a binary UNO type name or UNO type registry name

       @return the sort of the UNO type denoted by the given type; the detection
       of the correct sort is purely syntactical (especially, if the given input
       is a UNO type registry name that denotes something other than a UNO type,
       SORT_COMPLEX is returned)
     */
    Sort getSort(OString const & type);

    /**
       Determines whether a UNO type name or UNO type registry name denotes a
       UNO sequence type.

       @param type a binary UNO type name or UNO type registry name

       @return true iff the given type denotes a UNO sequence type; the
       detection is purely syntactical
     */
    bool isSequenceType(OString const & type);

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
    OString decompose(
        OString const & type, sal_Int32 * rank = 0,
        std::vector< OString > * arguments = 0);
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
