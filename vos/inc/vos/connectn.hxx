/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: connectn.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 08:58:26 $
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


#ifndef _VOS_CONNECTN_HXX_
#define _VOS_CONNECTN_HXX_

#ifndef _VOS_REF_HXX_
#   include <vos/ref.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#   include <rtl/ustring.hxx>
#endif

#ifdef _USE_NAMESPACE
namespace vos
{
#endif
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

#ifdef _USE_NAMESPACE
}
#endif

#endif // _VOS_CONNECTN_HXX_

