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
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>

using namespace com::sun::star;
using namespace com::sun::star::uno;


namespace xmlscript
{

namespace {

class BSeqInputStream
    : public ::cppu::WeakImplHelper< io::XInputStream >
{
    std::vector<sal_Int8> _seq;
    sal_Int32 _nPos;

public:
    explicit BSeqInputStream( std::vector<sal_Int8>&& rSeq )
        : _seq( std::move(rSeq) )
        , _nPos( 0 )
        {}

    // XInputStream
    virtual sal_Int32 SAL_CALL readBytes(
        Sequence< sal_Int8 > & rData, sal_Int32 nBytesToRead ) override;
    virtual sal_Int32 SAL_CALL readSomeBytes(
        Sequence< sal_Int8 > & rData, sal_Int32 nMaxBytesToRead ) override;
    virtual void SAL_CALL skipBytes(
        sal_Int32 nBytesToSkip ) override;
    virtual sal_Int32 SAL_CALL available() override;
    virtual void SAL_CALL closeInput() override;
};

}

sal_Int32 BSeqInputStream::readBytes(
    Sequence< sal_Int8 > & rData, sal_Int32 nBytesToRead )
{
    nBytesToRead = ((nBytesToRead > static_cast<sal_Int32>(_seq.size()) - _nPos)
                    ? _seq.size() - _nPos
                    : nBytesToRead);

    if (rData.getLength() != nBytesToRead)
        rData.realloc( nBytesToRead );
    if (nBytesToRead != 0) {
        memcpy(rData.getArray(), &_seq[_nPos], nBytesToRead);
    }
    _nPos += nBytesToRead;
    return nBytesToRead;
}

sal_Int32 BSeqInputStream::readSomeBytes(
    Sequence< sal_Int8 > & rData, sal_Int32 nMaxBytesToRead )
{
    return readBytes( rData, nMaxBytesToRead );
}

void BSeqInputStream::skipBytes(
    sal_Int32 /*nBytesToSkip*/ )
{
}

sal_Int32 BSeqInputStream::available()
{
    return _seq.size() - _nPos;
}

void BSeqInputStream::closeInput()
{
}

namespace {

class BSeqOutputStream
    : public ::cppu::WeakImplHelper< io::XOutputStream >
{
    std::vector<sal_Int8> * _seq;

public:
    explicit BSeqOutputStream( std::vector<sal_Int8> * seq )
        : _seq( seq )
        {}

    // XOutputStream
    virtual void SAL_CALL writeBytes(
        Sequence< sal_Int8 > const & rData ) override;
    virtual void SAL_CALL flush() override;
    virtual void SAL_CALL closeOutput() override;
};

}

void BSeqOutputStream::writeBytes( Sequence< sal_Int8 > const & rData )
{
    sal_Int32 nPos = _seq->size();
    _seq->resize( nPos + rData.getLength() );
    if (rData.getLength() != 0) {
        memcpy( _seq->data() + nPos,
                rData.getConstArray(),
                rData.getLength() );
    }
}
void BSeqOutputStream::flush()
{
}

void BSeqOutputStream::closeOutput()
{
}

Reference< io::XInputStream > createInputStream( std::vector<sal_Int8>&& rInData )
{
    return new BSeqInputStream( std::move(rInData) );
}

Reference< io::XInputStream > createInputStream( const sal_Int8* pData, int len )
{
    std::vector<sal_Int8> rInData(len);
    if (len != 0) {
        memcpy( rInData.data(), pData, len);
    }
    return new BSeqInputStream( std::move(rInData) );
}

Reference< io::XOutputStream > createOutputStream( std::vector<sal_Int8> * pOutData )
{
    return new BSeqOutputStream( pOutData );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
