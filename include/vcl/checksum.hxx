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
#include <vcl/dllapi.h>

#define BITMAP_CHECKSUM_SIZE 4

typedef sal_uInt32   BitmapChecksum;
typedef sal_uInt8   BitmapChecksumOctetArray[BITMAP_CHECKSUM_SIZE];

template< sal_uInt8 N = 0 >
inline void BCToBCOA( BitmapChecksum n, BitmapChecksumOctetArray p )
{
  p[N] = static_cast<sal_uInt8>(n >> ( 8 * N ));
  return BCToBCOA< N + 1 >( n, p );
}

template<>
inline void BCToBCOA< BITMAP_CHECKSUM_SIZE >( BitmapChecksum, BitmapChecksumOctetArray )
{
    return;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
