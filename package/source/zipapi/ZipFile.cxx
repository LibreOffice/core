/*************************************************************************
 *
 *  $RCSfile: ZipFile.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: mtg $ $Date: 2001-06-15 15:25:10 $
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
#ifndef _ZIP_FILE_HXX
#include <ZipFile.hxx>
#endif
#ifndef _ENTRY_INPUT_STREAM_HXX
#include <EntryInputStream.hxx>
#endif
#ifndef _ZIP_ENUMERATION_HXX
#include <ZipEnumeration.hxx>
#endif
#ifndef _VOS_DIAGNOSE_H_
#include <vos/diagnose.hxx>
#endif
#ifndef _COM_SUN_STAR_PACKAGES_ZIPCONSTANTS_HPP_
#include <com/sun/star/packages/ZipConstants.hpp>
#endif
#include <vector>


using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::packages;
using namespace com::sun::star::packages::ZipConstants;

/** This class is used to read entries from a zip file
 */
ZipFile::ZipFile( Reference < io::XInputStream > &xInput, sal_Bool bInitialise)
    throw(io::IOException, ZipException, RuntimeException)
: xStream(xInput)
, aGrabber(xInput)
, aInflater (sal_True)
{
    if (bInitialise)
    {
        if ( readCEN() == -1 )
            aEntries.clear();
    }
}
void ZipFile::setInputStream ( Reference < io::XInputStream > xNewStream )
{
    xStream = xNewStream;
    aGrabber.setInputStream ( xStream );
}

ZipFile::~ZipFile()
{
    aEntries.clear();
}

void SAL_CALL ZipFile::close(  )
    throw(io::IOException, RuntimeException)
{
}

ZipEnumeration * SAL_CALL ZipFile::entries(  )
{
    return new ZipEnumeration ( aEntries );
}

::rtl::OUString SAL_CALL ZipFile::getName(  )
    throw(RuntimeException)
{
    return sName;
}

sal_Int32 SAL_CALL ZipFile::getSize(  )
    throw(RuntimeException)
{
    return aEntries.size();
}

Type SAL_CALL ZipFile::getElementType(  )
    throw(RuntimeException)
{
    return ::getCppuType((ZipEntry *) 0);
}

sal_Bool SAL_CALL ZipFile::hasElements(  )
    throw(RuntimeException)
{
    return (aEntries.size()>0);
}

Any SAL_CALL ZipFile::getByName( const ::rtl::OUString& aName )
        throw(container::NoSuchElementException, lang::WrappedTargetException, RuntimeException)
{
    Any aAny;
    EntryHash::const_iterator aCI = aEntries.find(sName);
    if (aCI == aEntries.end())
        throw container::NoSuchElementException();
    aAny <<= (*aCI).second;
     return aAny;
}

Sequence< ::rtl::OUString > SAL_CALL ZipFile::getElementNames(  )
        throw(RuntimeException)
{
    sal_uInt32 i=0, nSize = aEntries.size();
    OUString *pNames = new OUString[aEntries.size()];
    for (EntryHash::const_iterator aIterator = aEntries.begin(); aIterator != aEntries.end(); aIterator++,i++)
        pNames[i] = (*aIterator).first;
    return Sequence<OUString> (pNames, nSize);
}

sal_Bool SAL_CALL ZipFile::hasByName( const ::rtl::OUString& aName )
        throw(RuntimeException)
{
    return aEntries.find(aName) != aEntries.end();
}

Reference< io::XInputStream > SAL_CALL ZipFile::getInputStream( ZipEntry& rEntry,
        const vos::ORef < EncryptionData > &rData)
    throw(io::IOException, ZipException, RuntimeException)
{
    if (rEntry.nOffset <= 0)
        readLOC(rEntry);
    Reference< io::XInputStream > xStreamRef = new EntryInputStream(xStream, rEntry, rData, sal_False );
    return xStreamRef;
}

Reference< io::XInputStream > SAL_CALL ZipFile::getRawStream( ZipEntry& rEntry,
        const vos::ORef < EncryptionData > &rData)
    throw(io::IOException, ZipException, RuntimeException)
{
    if (rEntry.nOffset <= 0)
        readLOC(rEntry);
    Reference< io::XInputStream > xStreamRef = new EntryInputStream(xStream, rEntry, rData, sal_True );
    return xStreamRef;
}

sal_Bool ZipFile::readLOC( ZipEntry &rEntry )
    throw(io::IOException, ZipException, RuntimeException)
{
    sal_uInt32 nTestSig, nTime, nCRC, nSize, nCompressedSize;
    sal_uInt16 nVersion, nFlag, nHow, nNameLen, nExtraLen;
    sal_Int32 nPos = -rEntry.nOffset;

    aGrabber.seek(nPos);
    aGrabber >> nTestSig;

    if (nTestSig != LOCSIG)
        throw ZipException( OUString( RTL_CONSTASCII_USTRINGPARAM ( "Invalid LOC header (bad signature") ), Reference < XInterface > () );
    aGrabber >> nVersion;
    aGrabber >> nFlag;
    aGrabber >> nHow;
    aGrabber >> nTime;
    aGrabber >> nCRC;
    aGrabber >> nCompressedSize;
    aGrabber >> nSize;
    aGrabber >> nNameLen;
    aGrabber >> nExtraLen;
    rEntry.nOffset = static_cast < sal_Int32 > (aGrabber.getPosition()) + nNameLen + nExtraLen;
    return sal_True;
}

sal_Int32 ZipFile::findEND( )
    throw(io::IOException, ZipException, RuntimeException)
{
    sal_Int32 nLength=0, nPos=0;
    Sequence < sal_Int8 > aByteSeq;
    try
    {
        nLength = nPos = static_cast <sal_Int32 > (aGrabber.getLength());

        if (nLength == 0)
            return -1;
            //throw (ZipException( OUString::createFromAscii("Trying to find Zip END signature in a zero length file!"), Reference < XInterface> () ));

        aGrabber.seek( nLength );

        while (nLength - nPos < 0xFFFF)
        {
            sal_uInt32 nCount = 0xFFFF - ( nLength - nPos);
            if (nCount > ENDHDR)
                nCount = ENDHDR;
            nPos -= nCount;

            for (sal_uInt16 i=0; i <nCount;i++)
            {
                sal_uInt32 nTest;
                aGrabber.seek (nPos+i);
                aGrabber >> nTest;
                if (nTest == ENDSIG)
                {
                    sal_uInt16 nCommentLength;
                    sal_Int32 nEndPos = nPos + i;
                    aGrabber.seek(nEndPos+ENDCOM);
                    aGrabber >> nCommentLength;
                    /*
                    if (nEndPos + ENDHDR + nCommentLength == nLength)
                    {
                        Since we don't actually use the comment ourselves, we'll just ignore it, and also skip the check
                        below. Neither WinZip nor InfoZip's unzip perform such checking and happily open files with
                        garbage bytes on the end, so we should do so too! I will 'assert' however that the file has garbage
                        at the end, and hope that the file hasn't suffered any other abuse

                        if (nCommentLength>0)
                        {
                            aByteSeq.realloc(nCommentLength+1);
                            aGrabber.readBytes(Sequence< sal_Int8>(aByteSeq.getArray(), nCommentLength), nCommentLength);
                            aByteSeq[nCommentLength]='\0';
                            sComment = OUString((sal_Char*)aByteSeq.getConstArray(), nCommentLength+1, RTL_TEXTENCODING_ASCII_US);

                        }
                    }
                    */
                    VOS_ENSURE ( nEndPos + ENDHDR + nCommentLength == nLength, "This Zip File is potentially corrupt - it has garbage after the END descriptor! Hoping for the best...!");
                    return nPos + i;
                }
            }
        }
    }
    catch ( IllegalArgumentException& )
    {
        throw ZipException( OUString( RTL_CONSTASCII_USTRINGPARAM ( "Zip END signature not found!") ), Reference < XInterface> () );
    }
    throw ZipException( OUString( RTL_CONSTASCII_USTRINGPARAM ( "Zip END signature not found!") ), Reference < XInterface> () );
}

sal_Int32 ZipFile::readCEN()
    throw(io::IOException, ZipException, RuntimeException)
{
    sal_Int32 nEndPos, nLocPos;
    sal_Int16  nCount, nTotal;
    sal_Int32 nCenLen, nCenPos = -1, nCenOff;

    try
    {
        nEndPos = findEND();
        if (nEndPos == -1)
            return -1;
        aGrabber.seek(nEndPos + ENDTOT);
        aGrabber >> nTotal;
        aGrabber >> nCenLen;
        aGrabber >> nCenOff;

        if (nTotal<0 || nTotal * CENHDR > nCenLen)
            throw ZipException(OUString( RTL_CONSTASCII_USTRINGPARAM ( "invalid END header (bad entry count)") ), Reference < XInterface > ());

        if (nTotal > ZIP_MAXENTRIES)
            throw ZipException(OUString( RTL_CONSTASCII_USTRINGPARAM ( "too many entries in ZIP File") ), Reference < XInterface > ());

        if (nCenLen < 0 || nCenLen > nEndPos)
            throw ZipException(OUString( RTL_CONSTASCII_USTRINGPARAM ( "Invalid END header (bad central directory size)") ), Reference < XInterface > ());

        nCenPos = nEndPos - nCenLen;

        if (nCenOff < 0 || nCenOff > nCenPos)
            throw ZipException(OUString( RTL_CONSTASCII_USTRINGPARAM ( "Invalid END header (bad central directory size)") ), Reference < XInterface > ());

        nLocPos = nCenPos - nCenOff;
        aGrabber.seek(nCenPos);

        ZipEntry *pEntry = new ZipEntry;
        for (nCount = 0 ; nCount < nTotal; nCount++)
        {
            sal_Int32 nTestSig, nCRC, nCompressedSize, nTime, nSize, nExtAttr, nOffset;
            sal_Int16 nVerMade, nVersion, nFlag, nHow, nNameLen, nExtraLen, nCommentLen;
            sal_Int16 nDisk, nIntAttr;

            if (aGrabber.getPosition() - nCenPos + CENHDR > nCenLen)
                throw ZipException(OUString( RTL_CONSTASCII_USTRINGPARAM ( "Invalid CEN header (bad header size check 1)") ), Reference < XInterface > ());

            aGrabber >> nTestSig;
            if (nTestSig != CENSIG)
                throw ZipException(OUString( RTL_CONSTASCII_USTRINGPARAM ( "Invalid CEN header (bad signature)") ), Reference < XInterface > ());

            aGrabber >> nVerMade;
            aGrabber >> nVersion;
            if ((nVersion & 1) == 1)
                throw ZipException(OUString( RTL_CONSTASCII_USTRINGPARAM ( "Invalid CEN header (encrypted entry)") ), Reference < XInterface > ());

            aGrabber >> nFlag;
            aGrabber >> nHow;
            if (nHow != STORED && nHow != DEFLATED)
                throw ZipException(OUString( RTL_CONSTASCII_USTRINGPARAM ( "Invalid CEN header (bad compression method)") ), Reference < XInterface > ());

            aGrabber >> nTime;
            aGrabber >> nCRC;
            aGrabber >> nCompressedSize;
            aGrabber >> nSize;
            aGrabber >> nNameLen;
            aGrabber >> nExtraLen;
            aGrabber >> nCommentLen;
            aGrabber >> nDisk;
            aGrabber >> nIntAttr;
            aGrabber >> nExtAttr;
            aGrabber >> nOffset;

            if (aGrabber.getPosition() - nCenPos + nNameLen + nExtraLen + nCommentLen > nCenLen)
                throw ZipException(OUString( RTL_CONSTASCII_USTRINGPARAM ( "Invalid CEN header (bad header size check 2)") ), Reference < XInterface > ());

            if (nNameLen > ZIP_MAXNAMELEN)
                throw ZipException(OUString( RTL_CONSTASCII_USTRINGPARAM ( "name length exceeds 512 bytes" ) ), Reference < XInterface > ());

            if (nExtraLen > ZIP_MAXEXTRA)
                throw ZipException(OUString( RTL_CONSTASCII_USTRINGPARAM ( "extra header info exceeds 256 bytes") ), Reference < XInterface > ());

            pEntry->nTime   = nTime;
            pEntry->nCrc    = nCRC;
            pEntry->nSize   = nSize;
            pEntry->nCompressedSize = nCompressedSize;
            pEntry->nMethod = nHow;
            pEntry->nFlag   = nFlag;
            pEntry->nVersion= nVersion;
            pEntry->nOffset = nOffset + nLocPos;

            pEntry->nOffset *= -1;
            /*
            if (nHow == STORED)
                pEntry->nCompressedSize = 0;
            */
            Sequence < sal_Int8> aSequence (nNameLen);
            aGrabber.readBytes(aSequence, nNameLen);
            pEntry->sName = OUString((sal_Char*)aSequence.getConstArray(), nNameLen, RTL_TEXTENCODING_ASCII_US);

            aGrabber.seek(aGrabber.getPosition() + nExtraLen);
            if (nCommentLen>0)
            {
                Sequence < sal_Int8 > aCommentSeq( nCommentLen );
                aGrabber.readBytes(aCommentSeq, nCommentLen);
                pEntry->sComment = OUString((sal_Char*)aCommentSeq.getConstArray(), nNameLen, RTL_TEXTENCODING_ASCII_US);
            }
            aEntries[pEntry->sName] = *pEntry;
        }
        delete pEntry;

        if (nCount != nTotal)
            throw ZipException(OUString( RTL_CONSTASCII_USTRINGPARAM ( "Count != Total") ), Reference < XInterface > ());
    }
    catch ( IllegalArgumentException & )
    {
        // seek can throw this...
        nCenPos = -1; // make sure we return -1 to indicate an error
    }
    return nCenPos;
}
