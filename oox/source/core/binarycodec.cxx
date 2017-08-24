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

#include "oox/core/binarycodec.hxx"

#include <algorithm>
#include <string.h>
#include "oox/helper/attributelist.hxx"

#include <osl/diagnose.h>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/docpasswordhelper.hxx>

using namespace ::com::sun::star;

namespace oox {
namespace core {

sal_uInt16 CodecHelper::getPasswordHash( const AttributeList& rAttribs, sal_Int32 nElement )
{
    sal_Int32 nPasswordHash = rAttribs.getIntegerHex( nElement, 0 );
    OSL_ENSURE( (0 <= nPasswordHash) && (nPasswordHash <= SAL_MAX_UINT16), "CodecHelper::getPasswordHash - invalid password hash" );
    return static_cast< sal_uInt16 >( ((0 <= nPasswordHash) && (nPasswordHash <= SAL_MAX_UINT16)) ? nPasswordHash : 0 );
}



} // namespace core
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
