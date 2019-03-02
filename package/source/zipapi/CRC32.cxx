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

#include <CRC32.hxx>
#include <PackageConstants.hxx>
#include <rtl/crc.h>
#include <com/sun/star/io/XInputStream.hpp>

using namespace com::sun::star::uno;
using namespace com::sun::star::io;

/** A class to compute the CRC32 value of a data stream
 */

CRC32::CRC32()
: nCRC(0)
{
}
CRC32::~CRC32()
{
}
void CRC32::reset()
{
    nCRC=0;
}
sal_Int32 CRC32::getValue()
{
    return nCRC;
}
/** Update CRC32 with specified sequence of bytes
 */
void CRC32::updateSegment(const Sequence< sal_Int8 > &b, sal_Int32 len)
{
    nCRC = rtl_crc32(nCRC, b.getConstArray(), len );
}
/** Update CRC32 with specified sequence of bytes
 */
void CRC32::update(const Sequence< sal_Int8 > &b)
{
    nCRC = rtl_crc32(nCRC, b.getConstArray(),b.getLength());
}

sal_Int64 CRC32::updateStream( Reference < XInputStream > const & xStream )
{
    sal_Int32 nLength;
    sal_Int64 nTotal = 0;
    Sequence < sal_Int8 > aSeq ( n_ConstBufferSize );
    do
    {
        nLength = xStream->readBytes ( aSeq, n_ConstBufferSize );
        updateSegment ( aSeq, nLength );
        nTotal += nLength;
    }
    while ( nLength == n_ConstBufferSize );

    return nTotal;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
