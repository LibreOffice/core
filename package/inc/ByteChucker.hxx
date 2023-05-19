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
#ifndef INCLUDED_PACKAGE_INC_BYTECHUCKER_HXX
#define INCLUDED_PACKAGE_INC_BYTECHUCKER_HXX

#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/uno/Reference.h>

namespace com::sun::star {
    namespace io { class XSeekable; class XOutputStream; }
}
class ByteChucker final
{
    css::uno::Reference < css::io::XOutputStream > xStream;
    css::uno::Reference < css::io::XSeekable > xSeek;
    css::uno::Sequence < sal_Int8 > a2Sequence, a4Sequence, a8Sequence;
    sal_Int8 * const p2Sequence, * const p4Sequence, * const p8Sequence;

public:
    ByteChucker (css::uno::Reference<css::io::XOutputStream> const & xOstream);
    ~ByteChucker();

    /// @throws css::io::NotConnectedException
    /// @throws css::io::BufferSizeExceededException
    /// @throws css::io::IOException
    /// @throws css::uno::RuntimeException
    void WriteBytes( const css::uno::Sequence< sal_Int8 >& aData );

    /// @throws css::io::IOException
    /// @throws css::uno::RuntimeException
    sal_Int64 GetPosition();

    void WriteInt16(sal_Int16 nInt16)
    {
        p2Sequence[0] = static_cast< sal_Int8 >((nInt16 >>  0 ) & 0xFF);
        p2Sequence[1] = static_cast< sal_Int8 >((nInt16 >>  8 ) & 0xFF);
        WriteBytes( a2Sequence );
    }

    void WriteInt32(sal_Int32 nInt32)
    {
        p4Sequence[0] = static_cast< sal_Int8 >((nInt32 >>  0 ) & 0xFF);
        p4Sequence[1] = static_cast< sal_Int8 >((nInt32 >>  8 ) & 0xFF);
        p4Sequence[2] = static_cast< sal_Int8 >((nInt32 >> 16 ) & 0xFF);
        p4Sequence[3] = static_cast< sal_Int8 >((nInt32 >> 24 ) & 0xFF);
        WriteBytes( a4Sequence );
    }

    void WriteUInt32(sal_uInt32 nuInt32)
    {
        p4Sequence[0] = static_cast < sal_Int8 > ((nuInt32 >>  0 ) & 0xFF);
        p4Sequence[1] = static_cast < sal_Int8 > ((nuInt32 >>  8 ) & 0xFF);
        p4Sequence[2] = static_cast < sal_Int8 > ((nuInt32 >> 16 ) & 0xFF);
        p4Sequence[3] = static_cast < sal_Int8 > ((nuInt32 >> 24 ) & 0xFF);
        WriteBytes( a4Sequence );
    }

    void WriteUInt64(sal_uInt64 nuInt64)
    {
        p8Sequence[0] = static_cast<sal_Int8>((nuInt64 >>  0) & 0xFF);
        p8Sequence[1] = static_cast<sal_Int8>((nuInt64 >>  8) & 0xFF);
        p8Sequence[2] = static_cast<sal_Int8>((nuInt64 >> 16) & 0xFF);
        p8Sequence[3] = static_cast<sal_Int8>((nuInt64 >> 24) & 0xFF);
        p8Sequence[4] = static_cast<sal_Int8>((nuInt64 >> 32) & 0xFF);
        p8Sequence[5] = static_cast<sal_Int8>((nuInt64 >> 40) & 0xFF);
        p8Sequence[6] = static_cast<sal_Int8>((nuInt64 >> 48) & 0xFF);
        p8Sequence[7] = static_cast<sal_Int8>((nuInt64 >> 56) & 0xFF);
        WriteBytes( a8Sequence );
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
