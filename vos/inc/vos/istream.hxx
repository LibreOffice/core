/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _VOS_ISTREAM_HXX_
#define _VOS_ISTREAM_HXX_

#include "sal/types.h"
#   include <vos/types.hxx>

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

