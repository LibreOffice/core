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

#include "sal/config.h"

#include <algorithm>
#include <cassert>

#include "rtl/byteseq.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "typelib/typeclass.h"
#include "typelib/typedescription.hxx"

#include "lessoperators.hxx"

namespace com { namespace sun { namespace star { namespace uno {

bool operator<( const TypeDescription& rLeft, const TypeDescription& rRight) {
	assert( rLeft.is() && rRight.is());
	const typelib_TypeDescription& rA = *rLeft.get();
	const typelib_TypeDescription& rB = *rRight.get();
        if( rA.eTypeClass != rB.eTypeClass)
		return (rA.eTypeClass < rB.eTypeClass);
	const sal_Int32 nCmp = rtl_ustr_compare_WithLength(
			rA.pTypeName->buffer, rA.pTypeName->length,
			rB.pTypeName->buffer, rB.pTypeName->length);
	return (nCmp < 0);
}

bool TypeDescEqual::operator()( const TypeDescription& rLeft, const TypeDescription& rRight) const
{
	assert( rLeft.is() && rRight.is());
	const typelib_TypeDescription& rA = *rLeft.get();
	const typelib_TypeDescription& rB = *rRight.get();
	if( rA.eTypeClass != rB.eTypeClass)
		return false;
	const sal_Int32 nCmp = rtl_ustr_compare_WithLength(
			rA.pTypeName->buffer, rA.pTypeName->length,
			rB.pTypeName->buffer, rB.pTypeName->length);
	return (nCmp == 0);
}

sal_Int32 TypeDescHash::operator()( const TypeDescription& rTD) const
{
	assert( rTD.is());
	const typelib_TypeDescription& rA = *rTD.get();
	sal_Int32 h = rtl_ustr_hashCode_WithLength( rA.pTypeName->buffer, rA.pTypeName->length);
	h ^= static_cast<sal_Int32>(rA.eTypeClass);
	return h;
}

} } } }

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
