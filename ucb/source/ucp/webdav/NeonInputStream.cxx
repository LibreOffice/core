/*************************************************************************
 *
 *  $RCSfile: NeonInputStream.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kso $ $Date: 2000-11-10 14:36:08 $
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
#include "NeonInputStream.hxx"

using namespace cppu;
using namespace rtl;
using namespace com::sun::star::io;
using namespace com::sun::star::uno;
using namespace webdav_ucp;


// -------------------------------------------------------------------
// Constructor
// -------------------------------------------------------------------
NeonInputStream::NeonInputStream( void )
: mLen( 0 ),
  mPos( 0 )
{
}

// -------------------------------------------------------------------
// Destructor
// -------------------------------------------------------------------
NeonInputStream::~NeonInputStream( void )
{
}

// -------------------------------------------------------------------
// AddToStream
// Allows the caller to add some data to the "end" of the stream
// -------------------------------------------------------------------
void NeonInputStream::AddToStream( const char * inBuf, sal_Int32 inLen )
{
    sal_Int32 nOldLen = mLen;
    mLen += inLen;

    mInputBuffer.realloc( mLen );
    for ( sal_Int32 n = nOldLen; n < mLen; ++n )
    {
        mInputBuffer[ n ] = *inBuf;
        inBuf++;
    }


}

// -------------------------------------------------------------------
// queryInterface
// -------------------------------------------------------------------
Any NeonInputStream::queryInterface( const Type &type )
                        throw( RuntimeException )
{
    Any aRet = ::cppu::queryInterface( type,
                                       static_cast< XInputStream * >( this ),
                                       static_cast< XSeekable * >( this ) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( type );
}

// -------------------------------------------------------------------
// readBytes
// "Reads" the specified number of bytes from the stream
// -------------------------------------------------------------------
sal_Int32 SAL_CALL NeonInputStream::readBytes(
  ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw( ::com::sun::star::io::NotConnectedException,
               ::com::sun::star::io::BufferSizeExceededException,
               ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException )
{
    // Work out how much we're actually going to write
    sal_Int32 theBytes2Read = nBytesToRead;
    sal_Int32 theBytesLeft  = mLen - mPos;
    if ( theBytes2Read > theBytesLeft )
        theBytes2Read = theBytesLeft;

    // Realloc buffer.
    aData.realloc( theBytes2Read );

    sal_Int8 * theBuffer = aData.getArray();

    // Write the data
    for ( sal_Int32 theIndex = 0; theIndex < theBytes2Read; theIndex ++ )
        theBuffer[ theIndex ] = mInputBuffer[ mPos + theIndex ];

    // Update our stream position for next time
    mPos += theBytes2Read;

    return theBytes2Read;
}

// -------------------------------------------------------------------
// readSomeBytes
// -------------------------------------------------------------------
sal_Int32 SAL_CALL NeonInputStream::readSomeBytes(
 ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
        throw( ::com::sun::star::io::NotConnectedException,
               ::com::sun::star::io::BufferSizeExceededException,
               ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException )
{
    // Warning: What should this be doing ?
    return readBytes( aData, nMaxBytesToRead );
}

// -------------------------------------------------------------------
// skipBytes
// Moves the current stream position forward
// -------------------------------------------------------------------
void SAL_CALL NeonInputStream::skipBytes( sal_Int32 nBytesToSkip )
        throw( ::com::sun::star::io::NotConnectedException,
               ::com::sun::star::io::BufferSizeExceededException,
               ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException )
{
    mPos += nBytesToSkip;
    if ( mPos >= mLen )
        mPos = mLen;
}

// -------------------------------------------------------------------
// available
// Returns the number of unread bytes currently remaining on the stream
// -------------------------------------------------------------------
sal_Int32 SAL_CALL NeonInputStream::available(  )
        throw( ::com::sun::star::io::NotConnectedException,
               ::com::sun::star::io::BufferSizeExceededException,
               ::com::sun::star::uno::RuntimeException )
{
    return mLen - mPos;
}

// -------------------------------------------------------------------
// closeInput
// -------------------------------------------------------------------
void SAL_CALL NeonInputStream::closeInput( void )
         throw( ::com::sun::star::io::NotConnectedException,
                  ::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException )
{
}

// -------------------------------------------------------------------
// seek
// -------------------------------------------------------------------
void SAL_CALL NeonInputStream::seek( sal_Int64 location )
        throw( ::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException )
{
    if ( location < 0 )
        throw ::com::sun::star::lang::IllegalArgumentException();

    if ( location >= mLen )
        throw IOException();

    mPos = location;
}

// -------------------------------------------------------------------
// getPosition
// -------------------------------------------------------------------
sal_Int64 SAL_CALL NeonInputStream::getPosition()
        throw( ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException )
{
    return mPos;
}

// -------------------------------------------------------------------
// getLength
// -------------------------------------------------------------------
sal_Int64 SAL_CALL NeonInputStream::getLength()
        throw( ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException )
{
    return mLen;
}
