/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_package.hxx"
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
