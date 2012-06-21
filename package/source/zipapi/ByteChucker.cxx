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

#include <ByteChucker.hxx>
#include <PackageConstants.hxx>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XOutputStream.hpp>

using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

ByteChucker::ByteChucker(Reference<XOutputStream> xOstream)
: xStream(xOstream)
, xSeek (xOstream, UNO_QUERY )
, a1Sequence ( 1 )
, a2Sequence ( 2 )
, a4Sequence ( 4 )
, p1Sequence ( a1Sequence.getArray() )
, p2Sequence ( a2Sequence.getArray() )
, p4Sequence ( a4Sequence.getArray() )
{
}

ByteChucker::~ByteChucker()
{
}

void ByteChucker::WriteBytes( const Sequence< sal_Int8 >& aData )
    throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    xStream->writeBytes(aData);
}

sal_Int64 ByteChucker::GetPosition(  )
        throw(IOException, RuntimeException)
{
    return xSeek->getPosition();
}

ByteChucker& ByteChucker::operator << (sal_Int8 nInt8)
{
    p1Sequence[0] = nInt8  & 0xFF;
    WriteBytes( a1Sequence );
    return *this;
}

ByteChucker& ByteChucker::operator << (sal_Int16 nInt16)
{
    p2Sequence[0] = static_cast< sal_Int8 >((nInt16 >>  0 ) & 0xFF);
    p2Sequence[1] = static_cast< sal_Int8 >((nInt16 >>  8 ) & 0xFF);
    WriteBytes( a2Sequence );
    return *this;
}
ByteChucker& ByteChucker::operator << (sal_Int32 nInt32)
{
    p4Sequence[0] = static_cast< sal_Int8 >((nInt32 >>  0 ) & 0xFF);
    p4Sequence[1] = static_cast< sal_Int8 >((nInt32 >>  8 ) & 0xFF);
    p4Sequence[2] = static_cast< sal_Int8 >((nInt32 >> 16 ) & 0xFF);
    p4Sequence[3] = static_cast< sal_Int8 >((nInt32 >> 24 ) & 0xFF);
    WriteBytes( a4Sequence );
    return *this;
}

ByteChucker& ByteChucker::operator << (sal_uInt8 nuInt8)
{
    p1Sequence[0] = nuInt8  & 0xFF;
    WriteBytes( a1Sequence );
    return *this;
}
ByteChucker& ByteChucker::operator << (sal_uInt16 nuInt16)
{
    p2Sequence[0] = static_cast< sal_Int8 >((nuInt16 >>  0 ) & 0xFF);
    p2Sequence[1] = static_cast< sal_Int8 >((nuInt16 >>  8 ) & 0xFF);
    WriteBytes( a2Sequence );
    return *this;
}
ByteChucker& ByteChucker::operator << (sal_uInt32 nuInt32)
{
    p4Sequence[0] = static_cast < sal_Int8 > ((nuInt32 >>  0 ) & 0xFF);
    p4Sequence[1] = static_cast < sal_Int8 > ((nuInt32 >>  8 ) & 0xFF);
    p4Sequence[2] = static_cast < sal_Int8 > ((nuInt32 >> 16 ) & 0xFF);
    p4Sequence[3] = static_cast < sal_Int8 > ((nuInt32 >> 24 ) & 0xFF);
    WriteBytes( a4Sequence );
    return *this;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
