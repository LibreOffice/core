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

#pragma once

#include <com/sun/star/uno/Sequence.hxx>

#include <assert.h>

using namespace com::sun::star::uno;

namespace io_stm
{

class MemRingBuffer
{
public:
    MemRingBuffer();
    virtual ~MemRingBuffer();

    /***
    * overwrites data at given position. Size is automatically extended, when
    * data is written beyond end.
    ***/
    /// @throws css::io::BufferSizeExceededException
    void    writeAt( sal_Int32 nPos, const Sequence<sal_Int8> &);
    /// @throws css::io::BufferSizeExceededException
    void    readAt( sal_Int32 nPos, Sequence<sal_Int8> & , sal_Int32 nBytesToRead ) const;
    sal_Int32   getSize() const throw();
    /// @throws css::io::BufferSizeExceededException
    void    forgetFromStart(sal_Int32 nBytesToForget);

private:
    /// @throws css::io::BufferSizeExceededException
    void resizeBuffer(sal_Int32 nMinSize);
    void checkInvariants() const {
        assert( m_nBufferLen >= 0 );
        assert( m_nOccupiedBuffer >= 0 );
        assert( m_nOccupiedBuffer <= m_nBufferLen );
        assert( m_nStart >= 0 );
        assert( 0 == m_nStart || m_nStart < m_nBufferLen );
        (void) this; // avoid loplugin:staticmethods
    }

    sal_Int8    *m_p;
    sal_Int32   m_nBufferLen;
    sal_Int32   m_nStart;
    sal_Int32   m_nOccupiedBuffer;
};


class MemFIFO :
    private MemRingBuffer
{
public:
    /// @throws css::io::BufferSizeExceededException
    void          write( const Sequence<sal_Int8> &);
    /// @throws css::io::BufferSizeExceededException
    void          read( Sequence<sal_Int8> & , sal_Int32 nBytesToRead );
    /// @throws css::io::BufferSizeExceededException
    void          skip( sal_Int32 nBytesToSkip );
    sal_Int32     getSize() const throw()
                  { return MemRingBuffer::getSize(); }

};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
