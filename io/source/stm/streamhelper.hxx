/*************************************************************************
 *
 *  $RCSfile: streamhelper.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:24:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#define Max( a, b )     (((a)>(b)) ? (a) : (b) )
#define Min( a, b )     (((a)<(b)) ? (a) : (b) )

namespace io_stm {

class IFIFO_OutOfBoundsException :
                public Exception
{};

class IFIFO_OutOfMemoryException :
                public Exception
{};

class IFIFO
{
public:


    virtual void    write( const Sequence<sal_Int8> &) throw( IFIFO_OutOfMemoryException,
                                                              IFIFO_OutOfBoundsException )=0;

    virtual void    read( Sequence<sal_Int8> & , sal_Int32 nBytesToRead )
                                                    throw( IFIFO_OutOfBoundsException )=0;
    virtual void    skip( sal_Int32 nBytesToSkip )
                                        throw( IFIFO_OutOfBoundsException )=0;
    virtual sal_Int32   getSize() const throw(  )  =0;
    virtual void    shrink() throw() = 0;

    virtual ~IFIFO() {};
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
    virtual void    forgetFromStart( sal_Int32 nBytesToForget ) throw()=0;
    virtual void    forgetFromEnd( sal_Int32 nBytesToForget ) throw()=0;
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
        public  IFIFO,
        private MemRingBuffer
{
public:
    virtual void    write( const Sequence<sal_Int8> &) throw( IFIFO_OutOfMemoryException,
                                                              IFIFO_OutOfBoundsException );
    virtual void    read( Sequence<sal_Int8> & , sal_Int32 nBytesToRead )
                                                    throw( IFIFO_OutOfBoundsException );
    virtual void    skip( sal_Int32 nBytesToSkip ) throw( IFIFO_OutOfBoundsException );
    virtual sal_Int32   getSize()  const throw(  )
                        { return MemRingBuffer::getSize(); }
    virtual void    shrink() throw()
                        { MemRingBuffer::shrink(); }

};

}
