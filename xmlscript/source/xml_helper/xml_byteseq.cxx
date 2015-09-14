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

#include <string.h>

#include <cppuhelper/implbase.hxx>
#include <xmlscript/xml_helper.hxx>

using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::uno;

using ::rtl::ByteSequence;

namespace xmlscript
{

class BSeqInputStream
    : public ::cppu::WeakImplHelper< io::XInputStream >
{
    ByteSequence _seq;
    sal_Int32 _nPos;

public:
    explicit BSeqInputStream( ByteSequence const & rSeq )
        : _seq( rSeq )
        , _nPos( 0 )
        {}

    // XInputStream
    virtual sal_Int32 SAL_CALL readBytes(
        Sequence< sal_Int8 > & rData, sal_Int32 nBytesToRead )
        throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL readSomeBytes(
        Sequence< sal_Int8 > & rData, sal_Int32 nMaxBytesToRead )
        throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL skipBytes(
        sal_Int32 nBytesToSkip )
        throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL available()
        throw (io::NotConnectedException, io::IOException, RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL closeInput()
        throw (io::NotConnectedException, io::IOException, RuntimeException, std::exception) SAL_OVERRIDE;
};

sal_Int32 BSeqInputStream::readBytes(
    Sequence< sal_Int8 > & rData, sal_Int32 nBytesToRead )
    throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, RuntimeException, std::exception)
{
    nBytesToRead = ((nBytesToRead > _seq.getLength() - _nPos)
                    ? _seq.getLength() - _nPos
                    : nBytesToRead);

    ByteSequence aBytes( _seq.getConstArray() + _nPos, nBytesToRead );
    rData = toUnoSequence( aBytes );
    _nPos += nBytesToRead;
    return nBytesToRead;
}

sal_Int32 BSeqInputStream::readSomeBytes(
    Sequence< sal_Int8 > & rData, sal_Int32 nMaxBytesToRead )
    throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, RuntimeException, std::exception)
{
    return readBytes( rData, nMaxBytesToRead );
}

void BSeqInputStream::skipBytes(
    sal_Int32 /*nBytesToSkip*/ )
    throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, RuntimeException, std::exception)
{
}

sal_Int32 BSeqInputStream::available()
    throw (io::NotConnectedException, io::IOException, RuntimeException, std::exception)
{
    return (_seq.getLength() - _nPos);
}

void BSeqInputStream::closeInput()
    throw (io::NotConnectedException, io::IOException, RuntimeException, std::exception)
{
}

class BSeqOutputStream
    : public ::cppu::WeakImplHelper< io::XOutputStream >
{
    ByteSequence * _seq;

public:
    explicit BSeqOutputStream( ByteSequence * seq )
        : _seq( seq )
        {}

    // XOutputStream
    virtual void SAL_CALL writeBytes(
        Sequence< sal_Int8 > const & rData )
        throw (io::NotConnectedException, io::BufferSizeExceededException, RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL flush()
        throw (io::NotConnectedException, io::BufferSizeExceededException, RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL closeOutput()
        throw (io::NotConnectedException, io::BufferSizeExceededException, RuntimeException, std::exception) SAL_OVERRIDE;
};

void BSeqOutputStream::writeBytes( Sequence< sal_Int8 > const & rData )
    throw (io::NotConnectedException, io::BufferSizeExceededException, RuntimeException, std::exception)
{
    sal_Int32 nPos = _seq->getLength();
    _seq->realloc( nPos + rData.getLength() );
    memcpy( _seq->getArray() + nPos,
                      rData.getConstArray(),
                      rData.getLength() );
}
void BSeqOutputStream::flush()
    throw (io::NotConnectedException, io::BufferSizeExceededException, RuntimeException, std::exception)
{
}

void BSeqOutputStream::closeOutput()
    throw (io::NotConnectedException, io::BufferSizeExceededException, RuntimeException, std::exception)
{
}

Reference< io::XInputStream > SAL_CALL createInputStream( ByteSequence const & rInData )
{
    return new BSeqInputStream( rInData );
}

Reference< io::XOutputStream > SAL_CALL createOutputStream( ByteSequence * pOutData )
{
    return new BSeqOutputStream( pOutData );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
