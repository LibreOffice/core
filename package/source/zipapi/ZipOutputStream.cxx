/*************************************************************************
 *
 *  $RCSfile: ZipOutputStream.cxx,v $
 *
 *  $Revision: 1.26 $
 *
 *  last change: $Author: mtg $ $Date: 2001-05-08 13:59:39 $
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
#include <ZipOutputStream.hxx>
#endif
#ifndef _VOS_DIAGNOSE_H_
#include <vos/diagnose.hxx>
#endif
#ifndef _VOS_REF_H_
#include <vos/ref.hxx>
#endif
#ifndef _COM_SUN_STAR_PACKAGES_ZIPCONSTANTS_HPP_
#include <com/sun/star/packages/ZipConstants.hpp>
#endif
#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif
#ifndef _ENCRYPTION_DATA_HXX_
#include <EncryptionData.hxx>
#endif

using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::packages::ZipConstants;

/** This class is used to write Zip files
 */
ZipOutputStream::ZipOutputStream( uno::Reference < io::XOutputStream > &xOStream, sal_Int32 nNewBufferSize)
: xStream(xOStream)
, aChucker(xOStream)
, nMethod(DEFLATED)
, pCurrentEntry(NULL)
, bFinished(sal_False)
, bEncryptCurrentEntry(sal_False)
, aBuffer(nNewBufferSize)
, aDeflater(DEFAULT_COMPRESSION, sal_True)
, nCurrentDataBegin ( 0 )
{
}

ZipOutputStream::~ZipOutputStream( void )
{
    for (sal_Int32 i = 0, nEnd = aZipList.size(); i < nEnd; i++)
        delete aZipList[i];
}

void SAL_CALL ZipOutputStream::setComment( const ::rtl::OUString& rComment )
    throw(uno::RuntimeException)
{
    sComment = rComment;
}
void SAL_CALL ZipOutputStream::setMethod( sal_Int32 nNewMethod )
    throw(uno::RuntimeException)
{
    nMethod = static_cast < sal_Int16 > (nNewMethod);
}
void SAL_CALL ZipOutputStream::setLevel( sal_Int32 nNewLevel )
    throw(uno::RuntimeException)
{
    aDeflater.setLevel( nNewLevel);
}

void SAL_CALL ZipOutputStream::putNextEntry( packages::ZipEntry& rEntry,
                        const vos::ORef < EncryptionData > &xEncryptData,
                        sal_Bool bEncrypt)
    throw(io::IOException, uno::RuntimeException)
{
    if (pCurrentEntry != NULL)
        closeEntry();
    if (rEntry.nTime == -1)
        rEntry.nTime = getCurrentDosTime();
    if (rEntry.nMethod == -1)
        rEntry.nMethod = nMethod;
    rEntry.nVersion = 20;
    if (rEntry.nSize == -1 || rEntry.nCompressedSize == -1 ||
        rEntry.nCrc == -1)
        rEntry.nFlag = 8;
    else if (rEntry.nSize != -1 && rEntry.nCompressedSize != -1 &&
        rEntry.nCrc != -1)
        rEntry.nFlag = 0;

    rEntry.nOffset = static_cast < sal_Int32 > (aChucker.getPosition());
    writeLOC(rEntry);
    nCurrentDataBegin = static_cast < sal_Int32 > (aChucker.getPosition());
    aZipList.push_back( &rEntry );
    pCurrentEntry = &rEntry;

    if (bEncrypt)
    {
        bEncryptCurrentEntry = sal_True;
        rtlCipherError aResult;

        aCipher = rtl_cipher_create ( rtl_Cipher_AlgorithmBF, rtl_Cipher_ModeStream);
        aResult = rtl_cipher_init( aCipher, rtl_Cipher_DirectionEncode,
                            reinterpret_cast < const sal_uInt8 * > (xEncryptData->aKey.getConstArray()),
                            xEncryptData->aKey.getLength(),
                            xEncryptData->aInitVector.getConstArray(),
                            xEncryptData->aInitVector.getLength());
        OSL_ASSERT( aResult == rtl_Cipher_E_None );
    }
}
void SAL_CALL ZipOutputStream::close(  )
    throw(io::IOException, uno::RuntimeException)
{
    finish();
}

void SAL_CALL ZipOutputStream::closeEntry(  )
    throw(io::IOException, uno::RuntimeException)
{
    packages::ZipEntry *pEntry = pCurrentEntry;
    if (pEntry)
    {
        switch (pEntry->nMethod)
        {
            case DEFLATED:
                aDeflater.finish();
                while (!aDeflater.finished())
                    doDeflate();
                if ((pEntry->nFlag & 8) == 0)
                {
                    if (pEntry->nSize != aDeflater.getTotalIn())
                    {
                        VOS_DEBUG_ONLY("Invalid entry size");
                    }
                    if (pEntry->nCompressedSize != aDeflater.getTotalOut())
                    {
                        //VOS_DEBUG_ONLY("Invalid entry compressed size");
                        // Different compression strategies make the merit of this
                        // test somewhat dubious
                        pEntry->nCompressedSize = aDeflater.getTotalOut();
                    }
                    if (pEntry->nCrc != aCRC.getValue())
                    {
                        VOS_DEBUG_ONLY("Invalid entry CRC-32");
                    }
                }
                else
                {
                    pEntry->nSize = aDeflater.getTotalIn();
                    pEntry->nCompressedSize = aDeflater.getTotalOut();
                    pEntry->nCrc = aCRC.getValue();
                    // writeEXT(*pEntry);
                    // Let's seek back and re-write the LOC header correctly
                    // Also, if it's an encrypted stream, we need to flag
                    // the method type as STORED instead of DEFLATED or
                    // no standard tools will be able to read them!
                    // - mtg

                    pEntry->nFlag    =  0;
                    pEntry->nVersion = 10;
                    if ( bEncryptCurrentEntry )
                    {
                        pEntry->nMethod = STORED;
                        pEntry->nSize = pEntry->nCompressedSize;
                    }
                    sal_Int64 nPos = aChucker.getPosition();

                    aChucker.seek( pEntry->nOffset );
                    aChucker << LOCSIG;
                    aChucker << pEntry->nVersion;
                    aChucker << pEntry->nFlag;
                    aChucker << pEntry->nMethod;
                    aChucker << static_cast < sal_uInt32 > (pEntry->nTime);
                    aChucker << static_cast < sal_uInt32 > (pEntry->nCrc);
                    aChucker << pEntry->nCompressedSize;
                    aChucker << pEntry->nSize;

                    aChucker.seek( nPos );
                }
                aDeflater.reset();
                break;
            case STORED:

                pEntry->nCrc = aCRC.getValue();
                if (!((pEntry->nFlag & 8) == 0))
                {
                    // writeEXT(*pEntry);
                    // instead of writing a data descriptor (due to the fact
                    // that the 'jar' tool doesn't like data descriptors
                    // for STORED streams), we seek back and update the LOC
                    // header.

                    pEntry->nFlag    =  0;
                    pEntry->nVersion = 10;
                    sal_Int64 nPos = aChucker.getPosition(), nSize = aChucker.getPosition() - nCurrentDataBegin;
                    pEntry->nCompressedSize = pEntry->nSize = static_cast < sal_Int32 > (nSize);

                    aChucker.seek( pEntry->nOffset );
                    aChucker << LOCSIG;
                    aChucker << pEntry->nVersion;
                    aChucker << pEntry->nFlag;
                    aChucker << pEntry->nMethod;
                    aChucker << static_cast < sal_uInt32 > (pEntry->nTime);
                    aChucker << static_cast < sal_uInt32 > (pEntry->nCrc);
                    aChucker << pEntry->nCompressedSize;
                    aChucker << pEntry->nSize;

                    aChucker.seek( nPos );
                }

                /*
                if (static_cast < sal_uInt32 > (pEntry->nCrc) != static_cast <sal_uInt32> (aCRC.getValue()))
                {
                    // boom

                    VOS_DEBUG_ONLY("Invalid entry crc32");
                }
                */
                break;
            default:
                // boom;
                VOS_DEBUG_ONLY("Invalid compression method");
                break;
        }
        aCRC.reset();
        if (bEncryptCurrentEntry)
        {
            aEncryptionBuffer.realloc ( 0 );
            bEncryptCurrentEntry = sal_False;
            rtl_cipher_destroy ( aCipher );
        }
        pCurrentEntry = NULL;
        nCurrentDataBegin = 0;
    }
}

void SAL_CALL ZipOutputStream::write( const uno::Sequence< sal_Int8 >& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength )
    throw(io::IOException, uno::RuntimeException)
{
    switch (pCurrentEntry->nMethod)
    {
        case DEFLATED:
            if (!aDeflater.finished())
            {
                aDeflater.setInputSegment(rBuffer, nNewOffset, nNewLength);
                 while (!aDeflater.needsInput())
                    doDeflate();
                if (!bEncryptCurrentEntry)
                    aCRC.updateSegment(rBuffer, nNewOffset, nNewLength);
            }
            break;
        case STORED:
            sal_Int32 nOldLength = rBuffer.getLength();
            uno::Sequence < sal_Int8 > *pBuffer = const_cast < uno::Sequence < sal_Int8 > *> (&rBuffer);
            pBuffer->realloc(nNewLength);
            if (bEncryptCurrentEntry)
            {
                rtlCipherError aResult;
                aEncryptionBuffer.realloc ( nNewLength );
                aResult = rtl_cipher_encode ( aCipher, static_cast < const void * > (pBuffer->getConstArray()),
                                              nNewLength, reinterpret_cast < sal_uInt8 * > (aEncryptionBuffer.getArray()),  nNewLength );
                aChucker.writeBytes ( aEncryptionBuffer );
                aCRC.updateSegment( aEncryptionBuffer, nNewOffset, nNewLength);
                aEncryptionBuffer.realloc ( nOldLength );
            }
            else
            {
                aChucker.writeBytes( *pBuffer );
                aCRC.updateSegment(rBuffer, nNewOffset, nNewLength);
            }
            pBuffer->realloc(nOldLength);
            break;
    }

}
void SAL_CALL ZipOutputStream::rawWrite( const uno::Sequence< sal_Int8 >& rBuffer)
    throw(io::IOException, uno::RuntimeException)
{
    aChucker.writeBytes(rBuffer);
}
void SAL_CALL ZipOutputStream::rawCloseEntry(  )
    throw(io::IOException, uno::RuntimeException)
{
    writeEXT(*pCurrentEntry);
    aCRC.reset();
    pCurrentEntry = NULL;
}
void SAL_CALL ZipOutputStream::finish(  )
    throw(io::IOException, uno::RuntimeException)
{
    if (bFinished)
        return;
    if (pCurrentEntry != NULL)
        closeEntry();
    if (aZipList.size() < 1)
    {
        // boom
        VOS_DEBUG_ONLY("Zip file must have at least one entry!\n");
    }
    sal_Int32 nOffset= static_cast < sal_Int32 > (aChucker.getPosition());
    for (sal_Int32 i =0, nEnd = aZipList.size(); i < nEnd; i++)
        writeCEN( *aZipList[i] );
    writeEND( nOffset, static_cast < sal_Int32 > (aChucker.getPosition()) - nOffset);
    bFinished = sal_True;
}
void ZipOutputStream::doDeflate()
{
    //sal_Int32 nOldOut = aDeflater.getTotalOut();
    sal_Int32 nLength = aDeflater.doDeflateSegment(aBuffer, 0, aBuffer.getLength());
    sal_Int32 nOldLength = aBuffer.getLength();
    //sal_Int32 nNewOut = aDeflater.getTotalOut() - nOldOut;
    if (nLength> 0 )
    {
        aBuffer.realloc(nLength);
        if (bEncryptCurrentEntry)
        {
            rtlCipherError aResult;
            aEncryptionBuffer.realloc ( nLength );
            aResult = rtl_cipher_encode ( aCipher, static_cast < const void * > (aBuffer.getConstArray()),
                                            nLength, reinterpret_cast < sal_uInt8 * > (aEncryptionBuffer.getArray()),  nLength );
            aChucker.writeBytes ( aEncryptionBuffer );
            aCRC.update ( aEncryptionBuffer );
            aEncryptionBuffer.realloc ( nOldLength );
        }
        else
            aChucker.writeBytes(aBuffer);
        aBuffer.realloc(nOldLength);
    }
}
void ZipOutputStream::writeEND(sal_uInt32 nOffset, sal_uInt32 nLength)
    throw(io::IOException, uno::RuntimeException)
{
    sal_Int16 nCommentLength = static_cast < sal_Int16 > (sComment.getLength());
    uno::Sequence < sal_Int8 > aSequence (nCommentLength);
    sal_Int8 *pArray = aSequence.getArray();

    const sal_Unicode *pChar = sComment.getStr();
    for ( sal_Int16 i = 0; i < nCommentLength; i++)
    {
        VOS_ENSURE (pChar[i] <127, "Non US ASCII character in zipfile comment!");
        *(pArray+i) = static_cast < const sal_Int8 > (pChar[i]);
    }
    aChucker << ENDSIG;
    aChucker << static_cast < sal_Int16 > ( 0 );
    aChucker << static_cast < sal_Int16 > ( 0 );
    aChucker << static_cast < sal_Int16 > ( aZipList.size() );
    aChucker << static_cast < sal_Int16 > ( aZipList.size() );
    aChucker << nLength;
    aChucker << nOffset;
    aChucker << nCommentLength;
    if (nCommentLength)
        aChucker.writeBytes(aSequence);
}
void ZipOutputStream::writeCEN( const packages::ZipEntry &rEntry )
    throw(io::IOException, uno::RuntimeException)
{
    sal_Int16 nNameLength       = static_cast < sal_Int16 > ( rEntry.sName.getLength() ) ,
              nCommentLength    = static_cast < sal_Int16 > ( rEntry.sComment.getLength() ) ,
              nExtraLength      = static_cast < sal_Int16 > ( rEntry.extra.getLength() );

    aChucker << CENSIG;
    aChucker << rEntry.nVersion;
    aChucker << rEntry.nVersion;
    aChucker << rEntry.nFlag;
    aChucker << rEntry.nMethod;
    aChucker << static_cast < sal_uInt32> (rEntry.nTime);
    aChucker << static_cast < sal_uInt32> (rEntry.nCrc);
    aChucker << rEntry.nCompressedSize;
    aChucker << rEntry.nSize;
    aChucker << nNameLength;
    aChucker << nExtraLength;
    aChucker << nCommentLength;
    aChucker << static_cast < sal_Int16> (0);
    aChucker << static_cast < sal_Int16> (0);
    aChucker << static_cast < sal_Int32> (0);
    aChucker << rEntry.nOffset;
/*
    sal_uInt64 nCurrent = aChucker.getPosition();
    aChucker.seek(rEntry.nOffset+16);
    aChucker << static_cast < sal_uInt32> (rEntry.nCrc);
    aChucker << rEntry.nCompressedSize;
    aChucker << rEntry.nSize;
    aChucker.seek(nCurrent);
*/
    const sal_Unicode *pChar = rEntry.sName.getStr();
    uno::Sequence < sal_Int8 > aSequence (nNameLength);
    sal_Int8 *pArray = aSequence.getArray();

    for ( sal_Int16 i = 0; i < nNameLength; i++)
    {
        VOS_ENSURE (pChar[i] <127, "Non US ASCII character in zipentry name!");
        *(pArray+i) = static_cast < const sal_Int8 > (pChar[i]);
    }

    aChucker.writeBytes( aSequence );
    if (nExtraLength)
        aChucker.writeBytes( rEntry.extra);
    if (nCommentLength)
    {
        if (nNameLength != nCommentLength)
        {
            aSequence.realloc (nCommentLength);
            pArray = aSequence.getArray();
        }
        for (i=0, pChar = rEntry.sComment.getStr(); i < nCommentLength; i++)
        {
            VOS_ENSURE (pChar[i] <127, "Non US ASCII character in zipentry comment!");
            *(pArray+i) = static_cast < const sal_Int8 > (pChar[i]);
        }
        aChucker.writeBytes( aSequence );
    }
}
void ZipOutputStream::writeEXT( const packages::ZipEntry &rEntry )
    throw(io::IOException, uno::RuntimeException)
{
    aChucker << EXTSIG;
    aChucker << rEntry.nCrc;
    aChucker << rEntry.nCompressedSize;
    aChucker << rEntry.nSize;
}

void ZipOutputStream::writeLOC( const packages::ZipEntry &rEntry )
    throw(io::IOException, uno::RuntimeException)
{
    sal_Int16 nNameLength = static_cast < sal_Int16 > (rEntry.sName.getLength());
    uno::Sequence < sal_Int8 > aSequence(nNameLength);
    sal_Int8 *pArray = aSequence.getArray();

    aChucker << LOCSIG;
    aChucker << rEntry.nVersion;
    aChucker << rEntry.nFlag;
    aChucker << rEntry.nMethod;
    aChucker << static_cast < sal_uInt32 > (rEntry.nTime);
    if ((rEntry.nFlag & 8) == 8 )
    {
        aChucker << static_cast < sal_Int32 > (0);
        aChucker << static_cast < sal_Int32 > (0);
        aChucker << static_cast < sal_Int32 > (0);
    }
    else
    {
        aChucker << static_cast < sal_uInt32 > (rEntry.nCrc);
        aChucker << rEntry.nCompressedSize;
        aChucker << rEntry.nSize;
    }
    aChucker << nNameLength;
    aChucker << static_cast <sal_Int16 > ( rEntry.extra.getLength());

    const sal_Unicode *pChar = rEntry.sName.getStr();
    for ( sal_Int16 i = 0; i < nNameLength; i++)
    {
        VOS_ENSURE (pChar[i] <127, "Non US ASCII character in zipentry name!");
        *(pArray+i) = static_cast < const sal_Int8 > (pChar[i]);
    }
    aChucker.writeBytes( aSequence );
    if (rEntry.extra.getLength() != 0)
        aChucker.writeBytes( rEntry.extra );
}
sal_uInt32 ZipOutputStream::getCurrentDosTime( )
{
    oslDateTime aDateTime;
    TimeValue aTimeValue;
    osl_getSystemTime ( &aTimeValue );
    osl_getDateTimeFromTimeValue( &aTimeValue, &aDateTime);

    sal_uInt32 nYear = static_cast <sal_uInt32> (aDateTime.Year);

    if (nYear>1980)
        nYear-=1980;
    else if (nYear>80)
        nYear-=80;
    sal_uInt32 nResult = static_cast < sal_uInt32>( ( ( ( aDateTime.Day) +
                                          ( 32 * (aDateTime.Month)) +
                                          ( 512 * nYear ) ) << 16) |
                                        ( ( aDateTime.Seconds/2) +
                                            ( 32 * aDateTime.Minutes) +
                                          ( 2048 * static_cast <sal_uInt32 > (aDateTime.Hours) ) ) );
    return nResult;
}
/*

   This is actually never used, so I removed it, but thought that the
   implementation details may be useful in the future...mtg 20010307

   I stopped using the time library and used the OSL version instead, but
   it might still be useful to have this code here..

void ZipOutputStream::dosDateToTMDate ( tm &rTime, sal_uInt32 nDosDate)
{
    sal_uInt32 nDate = static_cast < sal_uInt32 > (nDosDate >> 16);
    rTime.tm_mday = static_cast < sal_uInt32 > ( nDate & 0x1F);
    rTime.tm_mon  = static_cast < sal_uInt32 > ( ( ( (nDate) & 0x1E0)/0x20)-1);
    rTime.tm_year = static_cast < sal_uInt32 > ( ( (nDate & 0x0FE00)/0x0200)+1980);

    rTime.tm_hour = static_cast < sal_uInt32 > ( (nDosDate & 0xF800)/0x800);
    rTime.tm_min  = static_cast < sal_uInt32 > ( (nDosDate & 0x7E0)/0x20);
    rTime.tm_sec  = static_cast < sal_uInt32 > ( 2 * (nDosDate & 0x1F) );
}
*/

