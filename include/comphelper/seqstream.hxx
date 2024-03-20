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
#ifndef INCLUDED_COMPHELPER_SEQSTREAM_HXX
#define INCLUDED_COMPHELPER_SEQSTREAM_HXX

#include <config_options.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/comphelperdllapi.h>
#include <comphelper/bytereader.hxx>
#include <mutex>

namespace comphelper
{

/** Base class for wrappers around memory data that want to be exposed as an XInputStream */
class COMPHELPER_DLLPUBLIC MemoryInputStream
    : public ::cppu::WeakImplHelper< css::io::XInputStream, css::io::XSeekable >,
      public comphelper::ByteReader
{
    std::mutex    m_aMutex;
    const sal_Int8* m_pMemoryData;
    sal_Int32       m_nMemoryDataLength;
    sal_Int32       m_nPos;

public:
    MemoryInputStream(const sal_Int8* pData, sal_Int32 nDataLength);

// css::io::XInputStream
    virtual sal_Int32 SAL_CALL readBytes( css::uno::Sequence<sal_Int8>& aData, sal_Int32 nBytesToRead ) override final;

    virtual sal_Int32 SAL_CALL readSomeBytes( css::uno::Sequence<sal_Int8>& aData, sal_Int32 nMaxBytesToRead ) override final;

    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip ) override final;

    virtual sal_Int32 SAL_CALL available(  ) override final;

    virtual void SAL_CALL closeInput(  ) override final;

    virtual void SAL_CALL seek( sal_Int64 location ) override final;
    virtual sal_Int64 SAL_CALL getPosition(  ) override final;
    virtual sal_Int64 SAL_CALL getLength(  ) override final;

// comphelper::ByteReader
    virtual sal_Int32 readSomeBytes( sal_Int8* pData, sal_Int32 nBytesToRead ) override final;

private:
    sal_Int32 avail();
};


// Stream for reading data from a sequence of bytes
class COMPHELPER_DLLPUBLIC SequenceInputStream final
    : public MemoryInputStream
{
    css::uno::Sequence<sal_Int8> const m_aData;

public:
    SequenceInputStream(css::uno::Sequence<sal_Int8> const & rData);
};

// don't export to avoid duplicate WeakImplHelper definitions with MSVC
class SAL_DLLPUBLIC_TEMPLATE OSequenceOutputStream_Base
    : public ::cppu::WeakImplHelper< css::io::XOutputStream >
{};

class UNLESS_MERGELIBS(COMPHELPER_DLLPUBLIC) OSequenceOutputStream final : public OSequenceOutputStream_Base
{
private:
    css::uno::Sequence< sal_Int8 >&                 m_rSequence;
    double                                          m_nResizeFactor;
    sal_Int32 const                                 m_nMinimumResize;
    sal_Int32                                       m_nSize;
        // the size of the virtual stream. This is not the size of the sequence, but the number of bytes written
        // into the stream at a given moment.

    bool                                            m_bConnected;
        // closeOutput has been called ?

    std::mutex                                      m_aMutex;

    void finalizeOutput();
    virtual ~OSequenceOutputStream() override { if (m_bConnected) finalizeOutput(); }

public:
    /** constructs the object. Everything written into the stream through the XOutputStream methods will be forwarded
        to the sequence, reallocating it if necessary. Writing will start at offset 0 within the sequence.
        @param      _rSeq               a reference to the sequence which will be used for output.
                                        The caller is responsible for taking care of the lifetime of the stream
                                        object and the sequence. If you're in doubt about this, use <code>closeOutput</code>
                                        before destroying the sequence
        @param      _nResizeFactor      the factor which is used for resizing the sequence when necessary. In every
                                        resize step, the new sequence size will be calculated by multiplying the current
                                        size with this factor, rounded off to the next multiple of 4.
        @param      _nMinimumResize     the minimal number of bytes which is additionally allocated on resizing
        @see        closeOutput
    */
    OSequenceOutputStream(
        css::uno::Sequence< sal_Int8 >& _rSeq,
        double _nResizeFactor = 1.3,
        sal_Int32 _nMinimumResize = 128
        );

    /// same as XOutputStream::writeBytes (as expected :)
    virtual void SAL_CALL writeBytes( const css::uno::Sequence< sal_Int8 >& aData ) override;
    /// this is a dummy in this implementation, no buffering is used
    virtual void SAL_CALL flush(  ) override;
    /** closes the output stream. In the case of this class, this means that the sequence used for writing is
        resized to the really used size and not used any further, every subsequent call to one of the XOutputStream
        methods will throw a <code>NotConnectedException</code>.
    */
    virtual void SAL_CALL closeOutput(  ) override;
};

} // namespace comphelper

#endif // INCLUDED_COMPHELPER_SEQSTREAM_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
