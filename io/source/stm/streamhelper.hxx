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

#ifndef INCLUDED_IO_SOURCE_STM_STREAMHELPER_HXX
#define INCLUDED_IO_SOURCE_STM_STREAMHELPER_HXX

// Save NDEBUG state
#ifdef NDEBUG
#define STREAMHELPER_HXX_HAD_NDEBUG
#undef NDEBUG
#endif

#if OSL_DEBUG_LEVEL == 0
#define NDEBUG
#endif
#include <assert.h>

#define Max( a, b )     (((a)>(b)) ? (a) : (b) )
#define Min( a, b )     (((a)<(b)) ? (a) : (b) )

namespace io_stm {

class I_FIFO_OutOfBoundsException :
                public Exception
{};

class I_FIFO_OutOfMemoryException :
                public Exception
{};

class I_FIFO
{
public:


    virtual void    write( const Sequence<sal_Int8> &) throw( I_FIFO_OutOfMemoryException,
                                                              I_FIFO_OutOfBoundsException )=0;

    virtual void    read( Sequence<sal_Int8> & , sal_Int32 nBytesToRead )
                                                       throw( I_FIFO_OutOfBoundsException )=0;
    virtual void    skip( sal_Int32 nBytesToSkip )
                                                       throw( I_FIFO_OutOfBoundsException )=0;
    virtual sal_Int32   getSize() const throw(  )  =0;
    virtual void    shrink() throw() = 0;

    virtual ~I_FIFO() {};
};


class IRingBuffer_OutOfBoundsException :
                public Exception
{};

class IRingBuffer_OutOfMemoryException :
                public Exception
{};

class IRingBuffer
{
public:
    /***
    * overwrites data at given position. Size is automatically extended, when
    * data is written beyond end.
    *
    ***/

    virtual void    writeAt( sal_Int32 nPos, const Sequence<sal_Int8> &)
        throw( IRingBuffer_OutOfMemoryException,
               IRingBuffer_OutOfBoundsException )=0;
    virtual void    readAt( sal_Int32 nPos, Sequence<sal_Int8> & , sal_Int32 nBytesToRead ) const
        throw( IRingBuffer_OutOfBoundsException )=0;
    virtual sal_Int32   getSize() const throw(  )  =0;
    virtual void    forgetFromStart( sal_Int32 nBytesToForget ) throw(IRingBuffer_OutOfBoundsException)=0;
    virtual void    forgetFromEnd( sal_Int32 nBytesToForget ) throw(IRingBuffer_OutOfBoundsException)=0;
    virtual void    shrink() throw() = 0;
    virtual ~IRingBuffer() {};
};


class MemRingBuffer :
        public IRingBuffer
{
public:
    MemRingBuffer();
    virtual ~MemRingBuffer();

    virtual void    writeAt( sal_Int32 nPos, const Sequence<sal_Int8> &)
                                                    throw(  IRingBuffer_OutOfMemoryException,
                                                                IRingBuffer_OutOfBoundsException ) SAL_OVERRIDE;
    virtual void    readAt( sal_Int32 nPos, Sequence<sal_Int8> & , sal_Int32 nBytesToRead ) const
                                                    throw( IRingBuffer_OutOfBoundsException ) SAL_OVERRIDE;
    virtual sal_Int32   getSize() const throw(  ) SAL_OVERRIDE;
    virtual void    forgetFromStart( sal_Int32 nBytesToForget ) throw(IRingBuffer_OutOfBoundsException) SAL_OVERRIDE;
    virtual void    forgetFromEnd( sal_Int32 nBytesToForget ) throw(IRingBuffer_OutOfBoundsException) SAL_OVERRIDE;

    virtual void shrink() throw() SAL_OVERRIDE;

private:

    void resizeBuffer( sal_Int32 nMinSize ) throw( IRingBuffer_OutOfMemoryException );
    inline void checkInvariants()
    {
        assert( m_nBufferLen >= 0 );
        assert( m_nOccupiedBuffer >= 0 );
        assert( m_nOccupiedBuffer <= m_nBufferLen );
        assert( m_nStart >= 0 );
        assert( 0 == m_nStart || m_nStart < m_nBufferLen );
    }

    sal_Int8    *m_p;
    sal_Int32   m_nBufferLen;
    sal_Int32   m_nStart;
    sal_Int32   m_nOccupiedBuffer;
};


class MemFIFO :
    public  I_FIFO,
    private MemRingBuffer
{
public:
    virtual void    write( const Sequence<sal_Int8> &) throw( I_FIFO_OutOfMemoryException,
                                                              I_FIFO_OutOfBoundsException ) SAL_OVERRIDE;
    virtual void    read( Sequence<sal_Int8> & , sal_Int32 nBytesToRead )
                                                       throw( I_FIFO_OutOfBoundsException ) SAL_OVERRIDE;
    virtual void    skip( sal_Int32 nBytesToSkip ) throw( I_FIFO_OutOfBoundsException ) SAL_OVERRIDE;
    virtual sal_Int32   getSize()  const throw(  ) SAL_OVERRIDE
                        { return MemRingBuffer::getSize(); }
    virtual void    shrink() throw() SAL_OVERRIDE
                        { MemRingBuffer::shrink(); }

};

// Restore NDEBUG state
#ifdef STREAMHELPER_HXX_HAD_NDEBUG
#define NDEBUG
#else
#undef NDEBUG
#endif

}

#endif // INCLUDED_IO_SOURCE_STM_STREAMHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
