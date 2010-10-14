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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
