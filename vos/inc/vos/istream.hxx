/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: istream.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 04:05:18 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _VOS_ISTREAM_HXX_
#define _VOS_ISTREAM_HXX_

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

#ifndef _VOS_TYPES_HXX_
#   include <vos/types.hxx>
#endif

namespace vos
{

/** Interface for a stream of data, on that you can read and write blocks of bytes.
*/
class IStream
{
public:

    IStream() { }
    virtual ~IStream() { }


    /** Retrieve n bytes from the stream and copy them into pBuffer.
        @param pBuffer receives the read data.
        @param n the number of bytes to read. pBuffer must be large enough
        to hold the n bytes!
        @return the number of read bytes
    */
    virtual sal_Int32 SAL_CALL read(void* pBuffer,
                         sal_uInt32 n) const= 0;

    /** Write n bytes from pBuffer to the stream.
        @param pBuffer contains the data to be written.
        @param n the number of bytes to write.
        @return the number of written bytes
    */
    virtual sal_Int32 SAL_CALL write(const void* pBuffer,
                          sal_uInt32 n)= 0;

    /** Checks if stream is closed for further reading.
        @return True is stream has ended (e.g. was closed).
    */
    virtual sal_Bool SAL_CALL isEof() const = 0;
};

}

#endif  // _VOS_ISTREAM_HXX_

