/*************************************************************************
*
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2000, 2011 Oracle and/or its affiliates.
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

#include "sal/config.h"

#include <algorithm>

#include "osl/diagnose.h"
#include "rtl/byteseq.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "typelib/typeclass.h"
#include "typelib/typedescription.hxx"

#include "lessoperators.hxx"

namespace com { namespace sun { namespace star { namespace uno {

bool operator <(TypeDescription const & left, TypeDescription const & right) {
    OSL_ASSERT(left.is() && right.is());
    typelib_TypeClass tc1 = left.get()->eTypeClass;
    typelib_TypeClass tc2 = right.get()->eTypeClass;
    return tc1 < tc2 ||
        (tc1 == tc2 &&
         (rtl::OUString(left.get()->pTypeName) <
          rtl::OUString(right.get()->pTypeName)));
}

} } } }

namespace rtl {

bool operator <(ByteSequence const & left, ByteSequence const & right) {
    for (sal_Int32 i = 0; i != std::min(left.getLength(), right.getLength());
         ++i)
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
