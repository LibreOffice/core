/*************************************************************************
 *
 *  $RCSfile: ZipOutputStream.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mtg $ $Date: 2000-11-16 11:55:51 $
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
#ifndef _ZIP_OUTPUT_STREAM_HXX
#define _ZIP_OUTPUT_STREAM_HXX

#ifndef _BYTE_CHUCKER_HXX_
#include "ByteChucker.hxx"
#endif

#ifndef _ZIP_ENTRY_IMPL_HXX
#include "ZipEntryImpl.hxx"
#endif

#ifndef _ZIP_FILE_HXX
#include "ZipFile.hxx"
#endif

#ifndef _DEFLATER_HXX
#include "Deflater.hxx"
#endif

#ifndef _CRC32_HXX
#include "CRC32.hxx"
#endif

#ifndef _TOOLS_TIME_HXX
#include <tools/time.hxx>
#endif

#ifndef _COM_SUN_STAR_PACKAGE_XZIPOUTPUTSTREAM_HPP_
#include <com/sun/star/package/XZipOutputStream.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx> // helper for implementations
#endif

#ifndef __SGI_STL_VECTOR
#include <stl/vector>
#endif


class ZipOutputStream : public cppu::WeakImplHelper1< com::sun::star::package::XZipOutputStream >
{
private:
    com::sun::star::uno::Reference < com::sun::star::io::XOutputStream > xStream;
    Deflater            aDeflater;
    com::sun::star::uno::Sequence < sal_Int8 > aBuffer;
    ::rtl::OUString     sComment;
    sal_Int16           nMethod;
    sal_Int16           nLevel;
    com::sun::star::package::ZipEntry           *pCurrentEntry;
    CRC32               aCRC;
    sal_Bool            bFinished;
    ByteChucker         aChucker;
    ::std::vector <::com::sun::star::package::ZipEntry>         aZipList;

public:
    ZipOutputStream( com::sun::star::uno::Reference < com::sun::star::io::XOutputStream > &xOStream, sal_Int32 nNewBufferSize);
    virtual ~ZipOutputStream(void);
    virtual void SAL_CALL setComment( const ::rtl::OUString& rComment )
        throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setMethod( sal_Int32 nNewMethod )
        throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setLevel( sal_Int32 nNewLevel )
        throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL putNextEntry( const ::com::sun::star::package::ZipEntry& rEntry )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL closeEntry(  )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL write( const ::com::sun::star::uno::Sequence< sal_Int8 >& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL finish(  )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL close(  )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
#if 0
    ZipOutputStream (SvStream& aOStream);
    void                close();
    void                closeEntry();
    void                finish();
    void                putNextEntry(ZipEntry &rEntry);
    void                setComment(::rtl::ByteSequence &nComment);
    void                setLevel(sal_Int16 nNewLevel);
    void                setMethod(sal_Int16 nNewMethod);
    void                write (::rtl::ByteSequence &rBuffer, sal_uInt16 nOff, sal_uInt16 nLen);
    static const sal_Int16 STORED   = ZipEntry::STORED;
    static const sal_Int16 DEFLATED = ZipEntry::DEFLATED;
#endif
private:
    void doDeflate();
    void writeEND(sal_uInt32 nOffset, sal_uInt32 nLength);
    void writeCEN( const com::sun::star::package::ZipEntry &rEntry );
    void writeEXT( const com::sun::star::package::ZipEntry &rEntry );
    void writeLOC( const com::sun::star::package::ZipEntry &rEntry );

};

#endif
