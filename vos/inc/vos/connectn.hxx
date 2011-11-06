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




#ifndef _VOS_CONNECTN_HXX_
#define _VOS_CONNECTN_HXX_

#   include <vos/ref.hxx>
#   include <rtl/ustring.hxx>

namespace vos
{
//---------------------------------------------------------------------------
/** abstract connection class
 */

class IConnection: public ::vos::IReference
{
public:

    //---------------------------------------------------------------------------
    /** reads the specified amount of bytes from the stream. The call will block
        until nBytesToRead bytes are available.

        @param pData
        pointer to the buffer to fill.

        @param nBytesToRead
        the number of bytes to read.

        @returns
        the number of bytes read;
    */

    virtual sal_Int32 SAL_CALL read( sal_Int8* pData, sal_Int32 nBytesToRead ) = 0;

    //---------------------------------------------------------------------------
    /** tries to read the specified amount of bytes from the stream.

        @param pData
        pointer to the buffer to fill.

        @param nBytesToRead
        the maximum number of bytes to read.

        @returns
        the number of bytes read;
    */

    virtual sal_Int32 SAL_CALL readSomeBytes( sal_Int8* pData, sal_Int32 nBytesToRead ) = 0;

    //---------------------------------------------------------------------------
    /** writes the specified number of bytes to the stream.

        @param pData
        pointer to the buffer to read from.

        @param nBytesToWrite
        the number of bytes to write.
    */

    virtual sal_Int32 SAL_CALL write( const sal_Int8* pData , sal_Int32 nBytesToWrite ) = 0;

    //---------------------------------------------------------------------------
    /** flushes all output data to the stream.
    */

    virtual sal_Int32 SAL_CALL flush() = 0;

    //---------------------------------------------------------------------------
    /** closes the stream of the connection.

        NOTE: implementation must be able to handle more than one close calls.
    */

    virtual sal_Int32 SAL_CALL close() = 0;

    //---------------------------------------------------------------------------
    /** gets the source address
    */
    virtual ::rtl::OUString SAL_CALL getSource() = 0;

    //---------------------------------------------------------------------------
    /** gets the destination address
    */
    virtual ::rtl::OUString SAL_CALL getDestination() = 0;
};

}

#endif // _VOS_CONNECTN_HXX_

