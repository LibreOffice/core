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
#ifndef INCLUDED_PACKAGE_INC_BYTEGRABBER_HXX
#define INCLUDED_PACKAGE_INC_BYTEGRABBER_HXX

#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/io/BufferSizeExceededException.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/NotConnectedException.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <osl/mutex.hxx>

namespace com { namespace sun { namespace star {
    namespace io { class XSeekable; class XInputStream; }
} } }
class ByteGrabber
{
protected:
    ::osl::Mutex m_aMutex;

    css::uno::Reference < css::io::XInputStream > xStream;
    css::uno::Reference < css::io::XSeekable > xSeek;
    css::uno::Sequence < sal_Int8 > aSequence;
    const sal_Int8 *pSequence;

public:
    ByteGrabber (css::uno::Reference < css::io::XInputStream > xIstream);
    ~ByteGrabber();

    void setInputStream (css::uno::Reference < css::io::XInputStream > xNewStream);
    // XInputStream
    sal_Int32 SAL_CALL readBytes( css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw(css::io::NotConnectedException, css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException);
    // XSeekable
    sal_Int64 SAL_CALL seek( sal_Int64 location )
        throw(css::lang::IllegalArgumentException, css::io::IOException, css::uno::RuntimeException);
    sal_Int64 SAL_CALL getPosition(  )
        throw(css::io::IOException, css::uno::RuntimeException);
    sal_Int64 SAL_CALL getLength(  )
        throw(css::io::IOException, css::uno::RuntimeException);

    sal_uInt16 ReadUInt16();
    sal_uInt32 ReadUInt32();
    sal_Int16 ReadInt16()
    {
        return static_cast<sal_Int16>(ReadUInt16());
    }
    sal_Int32 ReadInt32()
    {
        return static_cast<sal_Int32>(ReadUInt32());
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
