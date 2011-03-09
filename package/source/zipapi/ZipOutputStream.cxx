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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_package.hxx"

#include <com/sun/star/packages/zip/ZipConstants.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <comphelper/storagehelper.hxx>

#include <osl/time.h>

#include <EncryptionData.hxx>
#include <PackageConstants.hxx>
#include <ZipEntry.hxx>
#include <ZipFile.hxx>
#include <ZipPackageStream.hxx>
#include <ZipOutputStream.hxx>

using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::io;
using namespace com::sun::star::uno;
using namespace com::sun::star::packages;
using namespace com::sun::star::packages::zip;
using namespace com::sun::star::packages::zip::ZipConstants;

/** This class is used to write Zip files
 */
ZipOutputStream::ZipOutputStream( uno::Reference < XOutputStream > &xOStream )
: xStream(xOStream)
, aBuffer(n_ConstBufferSize)
, aDeflater(DEFAULT_COMPRESSION, sal_True)
, aChucker(xOStream)
, pCurrentEntry(NULL)
, nMethod(DEFLATED)
, bFinished(sal_False)
, bEncryptCurrentEntry(sal_False)
, m_pCurrentStream(NULL)
{
}

ZipOutputStream::~ZipOutputStream( void )
{
    for (sal_Int32 i = 0, nEnd = aZipList.size(); i < nEnd; i++)
        delete aZipList[i];
}

void SAL_CALL ZipOutputStream::setMethod( sal_Int32 nNewMethod )
    throw(RuntimeException)
{
    nMethod = static_cast < sal_Int16 > (nNewMethod);
}
void SAL_CALL ZipOutputStream::setLevel( sal_Int32 nNewLevel )
    throw(RuntimeException)
{
    aDeflater.setLevel( nNewLevel);
}

void SAL_CALL ZipOutputStream::putNextEntry( ZipEntry& rEntry,
                        ZipPackageStream* pStream,
                        sal_Bool bEncrypt)
    throw(IOException, RuntimeException)
{
    if (pCurrentEntry != NULL)
        closeEntry();
    if (rEntry.nTime == -1)
        rEntry.nTime = getCurrentDosTime();
    if (rEntry.nMethod == -1)
        rEntry.nMethod = nMethod;
    rEntry.nVersion = 20;
    rEntry.nFlag = 1 << 11;
    if (rEntry.nSize == -1 || rEntry.nCompressedSize == -1 ||
        rEntry.nCrc == -1)
        rEntry.nFlag |= 8;

    if (bEncrypt)
    {
        bEncryptCurrentEntry = sal_True;

        ZipFile::StaticGetCipher( pStream->GetEncryptionData(), aCipher, sal_False );

        aDigest = rtl_digest_createSHA1();
        mnDigested = 0;
        rEntry.nFlag |= 1 << 4;
        m_pCurrentStream = pStream;
    }
    sal_Int32 nLOCLength = writeLOC(rEntry);
    rEntry.nOffset = static_cast < sal_Int32 > (aChucker.GetPosition()) - nLOCLength;
    aZipList.push_back( &rEntry );
    pCurrentEntry = &rEntry;
}

void SAL_CALL ZipOutputStream::closeEntry(  )
    throw(IOException, RuntimeException)
{
    ZipEntry *pEntry = pCurrentEntry;
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
                        OSL_ENSURE(false,"Invalid entry size");
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
                        OSL_ENSURE(false,"Invalid entry CRC-32");
                    }
                }
                else
                {
                    pEntry->nSize = aDeflater.getTotalIn();
                    pEntry->nCompressedSize = aDeflater.getTotalOut();
                    pEntry->nCrc = aCRC.getValue();
                    if ( bEncryptCurrentEntry )
                        pEntry->nSize = pEntry->nCompressedSize;
                    writeEXT(*pEntry);
                }
                aDeflater.reset();
                aCRC.reset();
                break;
            case STORED:
                if (!((pEntry->nFlag & 8) == 0))
                    OSL_ENSURE ( false, "Serious error, one of compressed size, size or CRC was -1 in a STORED stream");
                break;
            default:
                OSL_ENSURE(false,"Invalid compression method");
                break;
        }

        if (bEncryptCurrentEntry)
        {
            rtlDigestError aDigestResult;
            aEncryptionBuffer.realloc ( 0 );
            bEncryptCurrentEntry = sal_False;
            rtl_cipher_destroy ( aCipher );
            uno::Sequence< sal_uInt8 > aDigestSeq( RTL_DIGEST_LENGTH_SHA1 );
            aDigestResult = rtl_digest_getSHA1 ( aDigest,
                                                 aDigestSeq.getArray(),
                                                 RTL_DIGEST_LENGTH_SHA1 );
            OSL_ASSERT( aDigestResult == rtl_Digest_E_None );
            rtl_digest_destroySHA1 ( aDigest );
            if ( m_pCurrentStream )
                m_pCurrentStream->setDigest( aDigestSeq );
        }
        pCurrentEntry = NULL;
        m_pCurrentStream = NULL;
    }
}

void SAL_CALL ZipOutputStream::write( const Sequence< sal_Int8 >& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength )
    throw(IOException, RuntimeException)
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
            {
                Sequence < sal_Int8 > aTmpBuffer ( rBuffer.getConstArray(), nNewLength );
                aChucker.WriteBytes( aTmpBuffer );
            }
            break;
    }
}

void SAL_CALL ZipOutputStream::rawWrite( Sequence< sal_Int8 >& rBuffer, sal_Int32 /*nNewOffset*/, sal_Int32 nNewLength )
    throw(IOException, RuntimeException)
{
    Sequence < sal_Int8 > aTmpBuffer ( rBuffer.getConstArray(), nNewLength );
    aChucker.WriteBytes( aTmpBuffer );
}

void SAL_CALL ZipOutputStream::rawCloseEntry(  )
    throw(IOException, RuntimeException)
{
    if ( pCurrentEntry->nMethod == DEFLATED && ( pCurrentEntry->nFlag & 8 ) )
        writeEXT(*pCurrentEntry);
    pCurrentEntry = NULL;
}

void SAL_CALL ZipOutputStream::finish(  )
    throw(IOException, RuntimeException)
{
    if (bFinished)
        return;

    if (pCurrentEntry != NULL)
        closeEntry();

    if (aZipList.size() < 1)
        OSL_ENSURE(false,"Zip file must have at least one entry!\n");

    sal_Int32 nOffset= static_cast < sal_Int32 > (aChucker.GetPosition());
    for (sal_Int32 i =0, nEnd = aZipList.size(); i < nEnd; i++)
        writeCEN( *aZipList[i] );
    writeEND( nOffset, static_cast < sal_Int32 > (aChucker.GetPosition()) - nOffset);
    bFinished = sal_True;
    xStream->flush();
}

void ZipOutputStream::doDeflate()
{
    sal_Int32 nLength = aDeflater.doDeflateSegment(aBuffer, 0, aBuffer.getLength());
    sal_Int32 nOldLength = aBuffer.getLength();

    if ( nLength > 0 )
    {
        Sequence < sal_Int8 > aTmpBuffer ( aBuffer.getConstArray(), nLength );
        const void *pTmpBuffer = static_cast < const void * > ( aTmpBuffer.getConstArray() );
        if (bEncryptCurrentEntry)
        {
            // Need to update our digest before encryption...
            rtlDigestError aDigestResult = rtl_Digest_E_None;
            sal_Int16 nDiff = n_ConstDigestLength - mnDigested;
            if ( nDiff )
            {
                sal_Int16 nEat = static_cast < sal_Int16 > ( nDiff > nLength ? nLength : nDiff );
                aDigestResult = rtl_digest_updateSHA1 ( aDigest, pTmpBuffer, nEat );
                mnDigested = mnDigested + nEat;
            }
            OSL_ASSERT( aDigestResult == rtl_Digest_E_None );

            aEncryptionBuffer.realloc ( nLength );

            rtlCipherError aCipherResult;
            aCipherResult = rtl_cipher_encode ( aCipher, pTmpBuffer,
                                            nLength, reinterpret_cast < sal_uInt8 * > (aEncryptionBuffer.getArray()),  nLength );
            OSL_ASSERT( aCipherResult == rtl_Cipher_E_None );

            aChucker.WriteBytes( aEncryptionBuffer );
            aCRC.update ( aEncryptionBuffer );
            aEncryptionBuffer.realloc ( nOldLength );
        }
        else
            aChucker.WriteBytes ( aTmpBuffer );
    }
}
void ZipOutputStream::writeEND(sal_uInt32 nOffset, sal_uInt32 nLength)
    throw(IOException, RuntimeException)
{
    aChucker << ENDSIG;
    aChucker << static_cast < sal_Int16 > ( 0 );
    aChucker << static_cast < sal_Int16 > ( 0 );
    aChucker << static_cast < sal_Int16 > ( aZipList.size() );
    aChucker << static_cast < sal_Int16 > ( aZipList.size() );
    aChucker << nLength;
    aChucker << nOffset;
    aChucker << static_cast < sal_Int16 > ( 0 );
}
void ZipOutputStream::writeCEN( const ZipEntry &rEntry )
    throw(IOException, RuntimeException)
{
    if ( !::comphelper::OStorageHelper::IsValidZipEntryFileName( rEntry.sPath, sal_True ) )
        throw IOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Unexpected character is used in file name." ) ), uno::Reference< XInterface >() );

    ::rtl::OString sUTF8Name = ::rtl::OUStringToOString( rEntry.sPath, RTL_TEXTENCODING_UTF8 );
    sal_Int16 nNameLength       = static_cast < sal_Int16 > ( sUTF8Name.getLength() );

    aChucker << CENSIG;
    aChucker << rEntry.nVersion;
    aChucker << rEntry.nVersion;
    if (rEntry.nFlag & (1 << 4) )
    {
        // If it's an encrypted entry, we pretend its stored plain text
        ZipEntry *pEntry = const_cast < ZipEntry * > ( &rEntry );
        pEntry->nFlag &= ~(1 <<4 );
        aChucker << rEntry.nFlag;
        aChucker << static_cast < sal_Int16 > ( STORED );
    }
    else
    {
        aChucker << rEntry.nFlag;
        aChucker << rEntry.nMethod;
    }
    aChucker << static_cast < sal_uInt32> ( rEntry.nTime );
    aChucker << static_cast < sal_uInt32> ( rEntry.nCrc );
    aChucker << rEntry.nCompressedSize;
    aChucker << rEntry.nSize;
    aChucker << nNameLength;
    aChucker << static_cast < sal_Int16> (0);
    aChucker << static_cast < sal_Int16> (0);
    aChucker << static_cast < sal_Int16> (0);
    aChucker << static_cast < sal_Int16> (0);
    aChucker << static_cast < sal_Int32> (0);
    aChucker << rEntry.nOffset;

    Sequence < sal_Int8 > aSequence( (sal_Int8*)sUTF8Name.getStr(), sUTF8Name.getLength() );
    aChucker.WriteBytes( aSequence );
}
void ZipOutputStream::writeEXT( const ZipEntry &rEntry )
    throw(IOException, RuntimeException)
{
    aChucker << EXTSIG;
    aChucker << static_cast < sal_uInt32> ( rEntry.nCrc );
    aChucker << rEntry.nCompressedSize;
    aChucker << rEntry.nSize;
}

sal_Int32 ZipOutputStream::writeLOC( const ZipEntry &rEntry )
    throw(IOException, RuntimeException)
{
    if ( !::comphelper::OStorageHelper::IsValidZipEntryFileName( rEntry.sPath, sal_True ) )
        throw IOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Unexpected character is used in file name." ) ), uno::Reference< XInterface >() );

    ::rtl::OString sUTF8Name = ::rtl::OUStringToOString( rEntry.sPath, RTL_TEXTENCODING_UTF8 );
    sal_Int16 nNameLength       = static_cast < sal_Int16 > ( sUTF8Name.getLength() );

    aChucker << LOCSIG;
    aChucker << rEntry.nVersion;

    if (rEntry.nFlag & (1 << 4) )
    {
        // If it's an encrypted entry, we pretend its stored plain text
        sal_Int16 nTmpFlag = rEntry.nFlag;
        nTmpFlag &= ~(1 <<4 );
        aChucker << nTmpFlag;
        aChucker << static_cast < sal_Int16 > ( STORED );
    }
    else
    {
        aChucker << rEntry.nFlag;
        aChucker << rEntry.nMethod;
    }

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
    aChucker << static_cast < sal_Int16 > (0);

    Sequence < sal_Int8 > aSequence( (sal_Int8*)sUTF8Name.getStr(), sUTF8Name.getLength() );
    aChucker.WriteBytes( aSequence );

    return LOCHDR + nNameLength;
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

