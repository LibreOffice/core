/*************************************************************************
 *
 *  $RCSfile: ZipOutputStream.hxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: mtg $ $Date: 2001-09-06 12:10:27 $
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
#ifndef _ZIP_OUTPUT_STREAM_HXX
#define _ZIP_OUTPUT_STREAM_HXX

#ifndef _BYTE_CHUCKER_HXX_
#include <ByteChucker.hxx>
#endif
#ifndef _DEFLATER_HXX
#include <Deflater.hxx>
#endif
#ifndef _CRC32_HXX
#include <CRC32.hxx>
#endif
#ifndef __SGI_STL_VECTOR
#include <vector>
#endif
#ifndef _RTL_CIPHER_H_
#include <rtl/cipher.h>
#endif
#ifndef _COM_SUN_STAR_PACKAGES_ZIP_ZIPENTRY_HPP_
#include <com/sun/star/packages/zip/ZipEntry.hpp>
#endif
#ifndef _VOS_REF_H_
#include <vos/ref.hxx>
#endif
#ifndef RTL_DIGEST_H_
#include <rtl/digest.h>
#endif

class EncryptionData;

class ZipOutputStream
{
protected:
    com::sun::star::uno::Reference < com::sun::star::io::XOutputStream > xStream;
    ::std::vector < ::com::sun::star::packages::zip::ZipEntry *>            aZipList;
    com::sun::star::uno::Sequence < sal_Int8 > aBuffer;
    com::sun::star::uno::Sequence < sal_Int8 > aEncryptionBuffer;
    ::rtl::OUString     sComment;
    Deflater            aDeflater;
    rtlCipher           aCipher;
    rtlDigest           aDigest;
    CRC32               aCRC;
    ByteChucker         aChucker;
    com::sun::star::packages::zip::ZipEntry             *pCurrentEntry;
    sal_Int16           nMethod, nLevel;
    sal_Bool            bFinished, bEncryptCurrentEntry, bSpanning;
    sal_Int16           nCurrentDiskNumber;
    ::vos::ORef < EncryptionData >  xCurrentEncryptData;

public:
    ZipOutputStream( com::sun::star::uno::Reference < com::sun::star::io::XOutputStream > &xOStream, sal_Bool bNewSpanning );
    ~ZipOutputStream(void);
    void setDiskNumber ( sal_Int16 nNewDiskNumber ) { nCurrentDiskNumber = nNewDiskNumber; }

    // rawWrite to support a direct write to the output stream
    void SAL_CALL rawWrite( ::com::sun::star::uno::Sequence< sal_Int8 >& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    void SAL_CALL rawCloseEntry(  )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    // XZipOutputStream interfaces
    void SAL_CALL setComment( const ::rtl::OUString& rComment )
        throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setMethod( sal_Int32 nNewMethod )
        throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setLevel( sal_Int32 nNewLevel )
        throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL putNextEntry( ::com::sun::star::packages::zip::ZipEntry& rEntry,
            vos::ORef < EncryptionData > &rData,
            sal_Bool bEncrypt = sal_False )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    void SAL_CALL closeEntry(  )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    void SAL_CALL write( const ::com::sun::star::uno::Sequence< sal_Int8 >& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    void SAL_CALL finish(  )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    void SAL_CALL close(  )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    static sal_uInt32 getCurrentDosTime ( );
protected:
    void doDeflate();
    void writeEND(sal_uInt32 nOffset, sal_uInt32 nLength)
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    void writeCEN( const com::sun::star::packages::zip::ZipEntry &rEntry )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    void writeEXT( const com::sun::star::packages::zip::ZipEntry &rEntry )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    sal_Int32 writeLOC( const com::sun::star::packages::zip::ZipEntry &rEntry )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
};

#endif
