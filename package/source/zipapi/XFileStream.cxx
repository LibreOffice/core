/*************************************************************************
 *
 *  $RCSfile: XFileStream.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mtg $ $Date: 2001-07-04 14:56:24 $
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
 *  Contributor(s): Martin Gallwey (gallwey@sun.com)
 *
 *
 ************************************************************************/
#ifndef _XFILE_STREAM_HXX
#include <XFileStream.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#include <memory.h> // for memcpy

using namespace osl;
using namespace com::sun::star::io;
using namespace com::sun::star::uno;

XFileStream::XFileStream( File * pNewFile )
: pFile (pNewFile )
{
}
XFileStream::~XFileStream(void)
{
    delete pFile;
}
Any SAL_CALL  XFileStream::queryInterface( const Type& rType )
        throw(RuntimeException)
{
    return ::cppu::queryInterface ( rType                                       ,
                                        // OWeakObject interfaces
                                        reinterpret_cast< XInterface*       > ( this )  ,
                                        static_cast< XWeak*         > ( this )  ,
                                        // my interfaces
                                        static_cast< XInputStream*      > ( this )  ,
                                        static_cast< XSeekable*     > ( this ) );

}
void SAL_CALL  XFileStream::acquire(void)
    throw()
{
    OWeakObject::acquire();
}
void SAL_CALL  XFileStream::release(void)
    throw()
{
    OWeakObject::release();
}
sal_Int32 SAL_CALL XFileStream::readBytes( Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    if (nBytesToRead < 0)
        throw BufferSizeExceededException(::rtl::OUString(),*this);

    sal_uInt64 nBytesRead = 0;
    aData.realloc ( nBytesToRead );
    FileBase::RC nError = pFile->read ( aData.getArray(), nBytesToRead, nBytesRead );

    if ( nError != FileBase::E_None )
        throw IOException ();

    return static_cast < sal_Int32 > (nBytesRead);
}

sal_Int32 SAL_CALL XFileStream::readSomeBytes( Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
        throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    return readBytes(aData, nMaxBytesToRead);
}
void SAL_CALL XFileStream::skipBytes( sal_Int32 nBytesToSkip )
        throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    if (nBytesToSkip < 0)
        throw BufferSizeExceededException(::rtl::OUString(),*this);

    FileBase::RC nError = pFile->setPos ( osl_Pos_Current, nBytesToSkip );
    if ( nError != FileBase::E_None )
        throw IOException ();
}

sal_Int32 SAL_CALL XFileStream::available(  )
        throw(NotConnectedException, IOException, RuntimeException)
{
    sal_uInt64 nPos, nEndPos;
    sal_Int32 nResult = 0;
    FileBase::RC nError = pFile->getPos ( nPos );
    if ( nError != FileBase::E_None )
        throw IOException ();
    nError = pFile->setPos ( osl_Pos_End, 0 );
    if ( nError != FileBase::E_None )
        throw IOException ();
    nError = pFile->getPos ( nEndPos );
    if ( nError != FileBase::E_None )
        throw IOException ();
    nResult = static_cast < sal_Int32 > ( nEndPos - nPos );
    nError = pFile->setPos ( osl_Pos_Absolut, nPos );
    if ( nError != FileBase::E_None )
        throw IOException ();
    return nResult;
}

void SAL_CALL XFileStream::closeInput(  )
        throw(NotConnectedException, IOException, RuntimeException)
{
    pFile->close();
}
void SAL_CALL XFileStream::seek( sal_Int64 location )
        throw(::com::sun::star::lang::IllegalArgumentException, IOException, RuntimeException)
{
    FileBase::RC nError = pFile->setPos ( osl_Pos_Absolut, location );
    if ( nError != FileBase::E_None )
        throw IOException ();
}
sal_Int64 SAL_CALL XFileStream::getPosition(  )
        throw(IOException, RuntimeException)
{
    sal_uInt64 nPos;
    FileBase::RC nError = pFile->getPos ( nPos );
    if ( nError != FileBase::E_None )
        throw IOException ();
    return nPos;
}
sal_Int64 SAL_CALL XFileStream::getLength(  )
        throw(IOException, RuntimeException)
{
    sal_uInt64 nPos, nEndPos;
    FileBase::RC nError = pFile->getPos ( nPos );
    if ( nError != FileBase::E_None )
        throw IOException ();
    nError = pFile->setPos ( osl_Pos_End, 0 );
    if ( nError != FileBase::E_None )
        throw IOException ();
    nError = pFile->getPos ( nEndPos );
    if ( nError != FileBase::E_None )
        throw IOException ();
    nError = pFile->setPos ( osl_Pos_Absolut, nPos );
    if ( nError != FileBase::E_None )
        throw IOException ();
    return nEndPos;
}
