/*************************************************************************
 *
 *  $RCSfile: ZipOutputStream.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: mtg $ $Date: 2000-11-28 10:50:54 $
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
#include "ZipOutputStream.hxx"
#endif

using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::package::ZipConstants;

/** This class is used to write Zip files
 */
ZipOutputStream::ZipOutputStream( uno::Reference < io::XOutputStream > &xOStream, sal_Int32 nNewBufferSize)
: xStream(xOStream)
, aChucker(xOStream)
, nMethod(DEFLATED)
, pCurrentEntry(NULL)
, bFinished(sal_False)
, aBuffer(nNewBufferSize)
, aDeflater(DEFAULT_COMPRESSION, sal_True)
{
}

ZipOutputStream::~ZipOutputStream( void )
{
}

void SAL_CALL ZipOutputStream::setComment( const ::rtl::OUString& rComment )
    throw(uno::RuntimeException)
{
    sComment = rComment;
}
void SAL_CALL ZipOutputStream::setMethod( sal_Int32 nNewMethod )
    throw(uno::RuntimeException)
{
    nMethod = nNewMethod;
}
void SAL_CALL ZipOutputStream::setLevel( sal_Int32 nNewLevel )
    throw(uno::RuntimeException)
{
    aDeflater.setLevel( nNewLevel);
}
void SAL_CALL ZipOutputStream::putNextEntry( const package::ZipEntry& rEntry )
    throw(io::IOException, uno::RuntimeException)
{
    package::ZipEntry *pNonConstEntry = const_cast < package::ZipEntry* >(&rEntry);
    if (pCurrentEntry != NULL)
        closeEntry();
    if (pNonConstEntry->nTime == -1)
    {
        time_t nTime =0;
        pNonConstEntry->nTime = tmDateToDosDate(*localtime(&nTime));
    }
    if (pNonConstEntry->nMethod == -1)
    {
        pNonConstEntry->nMethod = nMethod;
    }
    switch (pNonConstEntry->nMethod)
    {
        case DEFLATED:

            if (pNonConstEntry->nSize == -1 || pNonConstEntry->nCompressedSize == -1 ||
                pNonConstEntry->nCrc == -1)
                pNonConstEntry->nFlag = 8;
            else if (pNonConstEntry->nSize != -1 && pNonConstEntry->nCompressedSize != -1 &&
                pNonConstEntry->nCrc != -1)
                pNonConstEntry->nFlag = 0;
            pNonConstEntry->nVersion = 20;
            break;
        case STORED:
            if (pNonConstEntry->nSize == -1)
                pNonConstEntry->nSize = pNonConstEntry->nCompressedSize;
            else if (pNonConstEntry->nCompressedSize == -1 || pNonConstEntry->nCompressedSize == 0)
                pNonConstEntry->nCompressedSize = pNonConstEntry->nSize;
            pNonConstEntry->nVersion = 10;
            pNonConstEntry->nFlag = 0;
            break;
    }
    pNonConstEntry->nOffset = aChucker.getPosition();
    writeLOC(rEntry);
    aZipList.push_back(pNonConstEntry);
    pCurrentEntry=pNonConstEntry;
}
void SAL_CALL ZipOutputStream::close(  )
    throw(io::IOException, uno::RuntimeException)
{
    finish();
}

void SAL_CALL ZipOutputStream::closeEntry(  )
    throw(io::IOException, uno::RuntimeException)
{
    package::ZipEntry *pEntry = pCurrentEntry;
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
                    writeEXT(*pEntry);
                }
                aDeflater.reset();
                break;
            case STORED:
                if (static_cast < sal_uInt32 > (pEntry->nCrc) != static_cast <sal_uInt32> (aCRC.getValue()))
                {
                    // boom

                    VOS_DEBUG_ONLY("Invalid entry crc32");
                }
                break;
            default:
                // boom;
                VOS_DEBUG_ONLY("Invalid compression method");
                break;
        }
        aCRC.reset();
        pCurrentEntry = NULL;
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
            }
            break;
        case STORED:
            sal_Int32 nOldLength = rBuffer.getLength();
            uno::Sequence < sal_Int8 > *pBuffer = const_cast < uno::Sequence < sal_Int8 > *> (&rBuffer);
            pBuffer->realloc(nNewLength);
            aChucker.writeBytes(*pBuffer);
            pBuffer->realloc(nOldLength);
            break;
    }
    aCRC.updateSegment(rBuffer, nNewOffset, nNewLength);
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
    sal_Int32 nOffset= aChucker.getPosition();
    for (int i =0, nEnd = aZipList.size(); i < nEnd; i++)
        writeCEN(*aZipList[i]);
    writeEND( nOffset, aChucker.getPosition() - nOffset);
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
        aChucker.writeBytes(aBuffer);
        aBuffer.realloc(nOldLength);
    }
}
void ZipOutputStream::writeEND(sal_uInt32 nOffset, sal_uInt32 nLength)
{
    sal_Int16 i=0, nCommentLength = sComment.getLength();
    const sal_Unicode *pChar = sComment.getStr();
    uno::Sequence < sal_Int8 > aSequence (nCommentLength);
    for ( ; i < nCommentLength; i++)
    {
        VOS_ENSURE (pChar[i] <127, "Non US ASCII character in zipfile comment!");
        aSequence[i] = static_cast < const sal_Int8 > (pChar[i]);
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
void ZipOutputStream::writeCEN( const package::ZipEntry &rEntry )
{
    sal_Int16 nNameLength = rEntry.sName.getLength(),
              nCommentLength = rEntry.sComment.getLength(),
              nExtraLength = rEntry.extra.getLength(), i = 0;

    aChucker << CENSIG;
    aChucker << rEntry.nVersion;
    aChucker << rEntry.nVersion;
    aChucker << rEntry.nFlag;
    aChucker << rEntry.nMethod;
    aChucker << rEntry.nTime;
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
    for ( ; i < nNameLength; i++)
    {
        VOS_ENSURE (pChar[i] <127, "Non US ASCII character in zipentry name!");
        aSequence[i] = static_cast < const sal_Int8 > (pChar[i]);
    }

    aChucker.writeBytes( aSequence );
    if (nExtraLength)
        aChucker.writeBytes( rEntry.extra);
    if (nCommentLength)
    {
        aSequence.realloc (nCommentLength);
        for (i=0, pChar = rEntry.sComment.getStr(); i < nCommentLength; i++)
        {
            VOS_ENSURE (pChar[i] <127, "Non US ASCII character in zipentry comment!");
            aSequence[i] = static_cast < const sal_Int8 > (pChar[i]);
        }
        aChucker.writeBytes( aSequence );
    }
}
void ZipOutputStream::writeEXT( const package::ZipEntry &rEntry )
{
    aChucker << EXTSIG;
    aChucker << rEntry.nCrc;
    aChucker << rEntry.nCompressedSize;
    aChucker << rEntry.nSize;
}

void ZipOutputStream::writeLOC( const package::ZipEntry &rEntry )
{
    sal_Int16 nNameLength = rEntry.sName.getLength(), i=0;
    aChucker << LOCSIG;
    aChucker << rEntry.nVersion;
    aChucker << rEntry.nFlag;
    aChucker << rEntry.nMethod;
    aChucker << rEntry.nTime;
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
    uno::Sequence < sal_Int8 > aSequence (nNameLength);
    for ( ; i < nNameLength; i++)
    {
        VOS_ENSURE (pChar[i] <127, "Non US ASCII character in zipentry name!");
        aSequence[i] = static_cast < const sal_Int8 > (pChar[i]);
    }
    aChucker.writeBytes( aSequence );
    if (rEntry.extra.getLength() != 0)
        aChucker.writeBytes( rEntry.extra );
}
sal_uInt32 ZipOutputStream::tmDateToDosDate ( tm &rTime)
{
    sal_uInt32 nYear = static_cast <sal_uInt32> (rTime.tm_year);

    if (nYear>1980)
        nYear-=1980;
    else if (nYear>80)
        nYear-=80;
    return static_cast < sal_uInt32>( ( ( ( rTime.tm_mday) +
                                          ( 32 * (rTime.tm_mon+1)) +
                                          ( 512 * nYear ) ) << 16) |
                                        ( ( rTime.tm_sec/2) +
                                            ( 32 * rTime.tm_min) +
                                          ( 2048 * static_cast <sal_uInt32 > (rTime.tm_hour) ) ) );
}
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

