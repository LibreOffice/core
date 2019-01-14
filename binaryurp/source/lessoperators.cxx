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

#include <sal/config.h>

#include <algorithm>
#include <cassert>

#include <rtl/byteseq.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <typelib/typeclass.h>
#include <typelib/typedescription.hxx>

#include "lessoperators.hxx"

namespace com::sun::star::uno {

bool operator <(TypeDescription const & left, TypeDescription const & right) {
    assert(left.is() && right.is());
    typelib_TypeClass tc1 = left.get()->eTypeClass;
    typelib_TypeClass tc2 = right.get()->eTypeClass;
    return tc1 < tc2 ||
        (tc1 == tc2 &&
         (OUString::unacquired(&left.get()->pTypeName) <
          OUString::unacquired(&right.get()->pTypeName)));
}

}

namespace rtl {

bool operator <(ByteSequence const & left, ByteSequence const & right) {
    const sal_Int32 nLen = std::min( left.getLength(), right.getLength());
    for( sal_Int32 i = 0; i < nLen; ++i )
    {
        if (left[i] < right[i]) {
            return true;
        }
        if (right[i] < left[i]) {
            return false;
        }
    }
    return left.getLength() < right.getLength();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
