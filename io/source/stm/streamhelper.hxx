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
                                                                IRingBuffer_OutOfBoundsException );
    virtual void    readAt( sal_Int32 nPos, Sequence<sal_Int8> & , sal_Int32 nBytesToRead ) const
                                                    throw( IRingBuffer_OutOfBoundsException );
    virtual sal_Int32   getSize() const throw(  );
    virtual void    forgetFromStart( sal_Int32 nBytesToForget ) throw(IRingBuffer_OutOfBoundsException);
    virtual void    forgetFromEnd( sal_Int32 nBytesToForget ) throw(IRingBuffer_OutOfBoundsException);

    virtual void shrink() throw();

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
                                                              I_FIFO_OutOfBoundsException );
    virtual void    read( Sequence<sal_Int8> & , sal_Int32 nBytesToRead )
                                                       throw( I_FIFO_OutOfBoundsException );
    virtual void    skip( sal_Int32 nBytesToSkip ) throw( I_FIFO_OutOfBoundsException );
    virtual sal_Int32   getSize()  const throw(  )
                        { return MemRingBuffer::getSize(); }
    virtual void    shrink() throw()
                        { MemRingBuffer::shrink(); }

};

// Restore NDEBUG state
#ifdef STREAMHELPER_HXX_HAD_NDEBUG
#define NDEBUG
#else
#undef NDEBUG
#endif

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
