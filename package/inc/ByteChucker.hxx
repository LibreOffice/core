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
#include <com/sun/star/io/BufferSizeExceededException.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/NotConnectedException.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

namespace com { namespace sun { namespace star {
    namespace io { class XSeekable; class XOutputStream; }
} } }
class ByteChucker
{
protected:
    css::uno::Reference < css::io::XOutputStream > xStream;
    css::uno::Reference < css::io::XSeekable > xSeek;
    css::uno::Sequence < sal_Int8 > a1Sequence, a2Sequence, a4Sequence;
    sal_Int8 * const p1Sequence, * const p2Sequence, * const p4Sequence;

public:
    ByteChucker (css::uno::Reference<css::io::XOutputStream> xOstream);
    ~ByteChucker();

    void WriteBytes( const css::uno::Sequence< sal_Int8 >& aData )
        throw(css::io::NotConnectedException, css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException);

    sal_Int64 GetPosition()
        throw(css::io::IOException, css::uno::RuntimeException);

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
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
