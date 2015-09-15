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


#ifndef INCLUDED_VCL_INC_CHECKSUM_HXX
#define INCLUDED_VCL_INC_CHECKSUM_HXX

#include <sal/config.h>
#include <sal/types.h>
#include <tools/solar.h>
#include <vcl/dllapi.h>

#define BITMAP_CHECKSUM_SIZE 8
#define BITMAP_CHECKSUM_BITS BITMAP_CHECKSUM_SIZE * 8

typedef sal_uInt64   BitmapChecksum;
typedef sal_uInt8   BitmapChecksumOctetArray[BITMAP_CHECKSUM_SIZE];

template< sal_uInt8 N = 0 >
inline void BCToBCOA( BitmapChecksum n, BitmapChecksumOctetArray p )
{
  p[N] = (sal_uInt8)(n >> ( 8 * N ));
  return BCToBCOA< N + 1 >( n, p );
}

template<>
inline void BCToBCOA< BITMAP_CHECKSUM_SIZE >( BitmapChecksum, BitmapChecksumOctetArray )
{
    return;
}

#ifdef __cplusplus
extern "C" {
#endif

/*========================================================================
 *
 * vcl_crc64 interface.
 *
 *======================================================================*/
/** Evaluate CRC64 over given data.

    This function evaluates the CRC polynomial 0xEDB88320.

    @param  Crc    [in] CRC64 over previous data or zero.
    @param  Data   [in] data buffer.
    @param  DatLen [in] data buffer length.
    @return new CRC64 value.
 */
VCL_DLLPUBLIC sal_uInt64 SAL_CALL vcl_crc64 (
    sal_uInt64  Crc,
    const void *Data, sal_uInt32 DatLen
)   SAL_THROW_EXTERN_C();



VCL_DLLPUBLIC const sal_uInt64* vcl_get_crc64_table();


#ifdef __cplusplus
}
#endif

inline BitmapChecksum vcl_get_checksum (
    BitmapChecksum  Checksum,
    const void *Data,
    sal_uInt32 DatLen
)
{
    return (BitmapChecksum)(vcl_crc64( Checksum, Data, DatLen ));
}


#endif // INCLUDED_VCL_INC_CHECKSUM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
