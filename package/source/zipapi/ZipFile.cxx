/*************************************************************************
 *
 *  $RCSfile: ZipFile.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: mtg $ $Date: 2001-03-07 16:09:44 $
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
#include "ZipFile.hxx"
#endif


using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::package::ZipConstants;

/** This class is used to read entries from a zip file
 */
ZipFile::ZipFile (uno::Reference < io::XInputStream > &xInput)
    throw(io::IOException, package::ZipException, uno::RuntimeException)
: xStream(xInput)
, aGrabber(xInput)
{
    readCEN();
}
ZipFile::ZipFile( uno::Reference < io::XInputStream > &xInput, sal_Bool bInitialise)
    throw(io::IOException, package::ZipException, uno::RuntimeException)
: xStream(xInput)
, aGrabber(xInput)
{
    if (bInitialise)
        readCEN();
}
void ZipFile::setInputStream ( uno::Reference < io::XInputStream > xNewStream )
{
    xStream = xNewStream;
    aGrabber.setInputStream ( xStream );
}

void ZipFile::updateFromManList(std::vector < ManifestEntry * > &rManList)
{
    sal_Int32 i=0, nSize = rManList.size();
    aEntries.clear();

    // This is a bitwise copy, = is not an overloaded operator
    // I'm not sure how evil this is in this case...
    for (;i < nSize ; i++)
    {
        package::ZipEntry * pEntry = &rManList[i]->aEntry;
        aEntries[pEntry->sName] = *pEntry;
    }
}

ZipFile::~ZipFile()
{
    aEntries.clear();
}

void SAL_CALL ZipFile::close(  )
    throw(io::IOException, uno::RuntimeException)
{
}

uno::Reference< container::XEnumeration > SAL_CALL ZipFile::entries(  )
        throw(uno::RuntimeException)
{
    uno::Reference< container::XEnumeration> xEnumRef = new ZipEnumeration( aEntries );
    return xEnumRef;
}

::rtl::OUString SAL_CALL ZipFile::getName(  )
    throw(uno::RuntimeException)
{
    return sName;
}

sal_Int32 SAL_CALL ZipFile::getSize(  )
    throw(uno::RuntimeException)
{
    return aEntries.size();
}

uno::Type SAL_CALL ZipFile::getElementType(  )
    throw(uno::RuntimeException)
{
    return ::getCppuType((package::ZipEntry *) 0);
}

sal_Bool SAL_CALL ZipFile::hasElements(  )
    throw(uno::RuntimeException)
{
    return (aEntries.size()>0);
}

uno::Any SAL_CALL ZipFile::getByName( const ::rtl::OUString& aName )
        throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Any aAny;
    if (!aEntries.count(sName))
        throw container::NoSuchElementException();
    EntryHash::const_iterator aCI = aEntries.find(sName);
    aAny <<= (*aCI).second;
     return aAny;
}

uno::Sequence< ::rtl::OUString > SAL_CALL ZipFile::getElementNames(  )
        throw(uno::RuntimeException)
{
    sal_uInt32 i=0, nSize = aEntries.size();
    OUString *pNames = new OUString[aEntries.size()];
    for (EntryHash::const_iterator aIterator = aEntries.begin(); aIterator != aEntries.end(); aIterator++,i++)
        pNames[i] = (*aIterator).first;
    return uno::Sequence<OUString> (pNames, nSize);
}

sal_Bool SAL_CALL ZipFile::hasByName( const ::rtl::OUString& aName )
        throw(uno::RuntimeException)
{
    return aEntries.count(aName);
}

uno::Reference< io::XInputStream > SAL_CALL ZipFile::getInputStream( const package::ZipEntry& rEntry )
    throw(io::IOException, package::ZipException, uno::RuntimeException)
{
    sal_Int64 nEnd = rEntry.nCompressedSize == 0 ? rEntry.nSize : rEntry.nCompressedSize;
    if (rEntry.nOffset <= 0)
        readLOC(rEntry);
    sal_Int64 nBegin = rEntry.nOffset;
    nEnd +=nBegin;

    uno::Reference< io::XInputStream > xStreamRef = new EntryInputStream(xStream, nBegin, nEnd, 1024, rEntry.nSize, rEntry.nMethod == DEFLATED );
    return xStreamRef;
}

sal_uInt32 SAL_CALL ZipFile::getHeader(const package::ZipEntry& rEntry)
    throw(io::IOException, package::ZipException, uno::RuntimeException)
{
    uno::Sequence < sal_Int8 > aSequence (4);

    try
    {
        if (rEntry.nOffset <= 0)
            readLOC(rEntry);
    }
    catch (package::ZipException&)
    {
        VOS_ENSURE(0, "Zip file bug!");
        return 0;
    }

    aGrabber.seek(rEntry.nOffset);
    if (rEntry.nMethod == STORED)
    {
        if (xStream->readBytes(aSequence, 4) < 4)
            return 0;
    }
    else if (rEntry.nMethod == DEFLATED)
    {
        uno::Reference < io::XInputStream > xEntryStream = getInputStream (rEntry);
        if (xEntryStream->readBytes(aSequence, 4) < 4)
            return 0;
        /*
        Inflater aInflater ( sal_True );
        sal_Int32 nSize = rEntry.nCompressedSize < 50 ? rEntry.nCompressedSize : 50;
        uno::Sequence < sal_Int8 > aCompSeq (nSize );
        if (xStream->readBytes(aCompSeq, nSize) < nSize)
            return 0;
        aInflater.setInput(aCompSeq);
        aInflater.doInflate(aSequence);
        aInflater.end();
        */
    }
    return (static_cast < sal_uInt32 >
            (static_cast < sal_uInt8> (aSequence[0]& 0xFF)
           | static_cast < sal_uInt8> (aSequence[1]& 0xFF) << 8
           | static_cast < sal_uInt8> (aSequence[2]& 0xFF) << 16
           | static_cast < sal_uInt8> (aSequence[3]& 0xFF) << 24));
}

uno::Reference< io::XInputStream > SAL_CALL ZipFile::getRawStream( const package::ZipEntry& rEntry )
        throw(io::IOException, package::ZipException, uno::RuntimeException)
{
    sal_Int64 nSize = rEntry.nMethod == DEFLATED ? rEntry.nCompressedSize : rEntry.nSize;

    if (rEntry.nOffset <= 0)
        readLOC(rEntry);
    sal_Int64 nBegin = rEntry.nOffset;

    uno::Reference< io::XInputStream > xStreamRef = new EntryInputStream(xStream, nBegin, nSize+nBegin, 1024, nSize, sal_False);
    return xStreamRef;
}

sal_Bool ZipFile::readLOC(const package::ZipEntry &rEntry)
    throw(io::IOException, package::ZipException, uno::RuntimeException)
{
    sal_uInt32 nTestSig, nTime, nCRC, nSize, nCompressedSize;
    sal_uInt16 nVersion, nFlag, nHow, nNameLen, nExtraLen;
    sal_Int32 nPos = -rEntry.nOffset;

    aGrabber.seek(nPos);
    aGrabber >> nTestSig;

    if (nTestSig != LOCSIG)
        throw package::ZipException( OUString( RTL_CONSTASCII_USTRINGPARAM ( "Invalid LOC header (bad signature") ), uno::Reference < uno::XInterface > () );
    aGrabber >> nVersion;
    aGrabber >> nFlag;
    aGrabber >> nHow;
    aGrabber >> nTime;
    aGrabber >> nCRC;
    aGrabber >> nCompressedSize;
    aGrabber >> nSize;
    aGrabber >> nNameLen;
    aGrabber >> nExtraLen;
    package::ZipEntry *pNonConstEntry = const_cast < package::ZipEntry* > (&rEntry);
    pNonConstEntry->nOffset =  static_cast < sal_Int32 > (aGrabber.getPosition()) + nNameLen + nExtraLen;
    return sal_True;
}

sal_Int32 ZipFile::findEND( )
    throw(io::IOException, package::ZipException, uno::RuntimeException)
{
    sal_Int32 nLength=0, nPos=0;
    uno::Sequence < sal_Int8 > aByteSeq;
    nLength = nPos = static_cast <sal_Int32 > (aGrabber.getLength());

    if (nLength == 0)
        return -1;
        //throw (package::ZipException( OUString::createFromAscii("Trying to find Zip END signature in a zero length file!"), uno::Reference < uno::XInterface> () ));

    aGrabber.seek( nLength );

    while (nLength - nPos < 0xFFFF)
    {
        sal_uInt32 nCount = 0xFFFF - ( nLength - nPos);
        if (nCount > ENDHDR)
            nCount = ENDHDR;
        nPos -= nCount;

        for (sal_uInt16 i=0; i <nCount;i++)
        {
            sal_uInt32 nTest=0, nFoo=ENDSIG;
            aGrabber.seek (nPos+i);
            aGrabber >> nTest;
            if (nTest == ENDSIG)
            {
                sal_uInt16 nCommentLength;
                sal_Int32 nEndPos = nPos + i;
                aGrabber.seek(nEndPos+ENDCOM);
                aGrabber >> nCommentLength;
                if (nEndPos + ENDHDR + nCommentLength == nLength)
                {
                    if (nCommentLength>0)
                    {
                        aByteSeq.realloc(nCommentLength+1);
                        aGrabber.readBytes(uno::Sequence< sal_Int8>(aByteSeq.getArray(), nCommentLength), nCommentLength);
                        aByteSeq[nCommentLength]='\0';
                        sComment = OUString((sal_Char*)aByteSeq.getConstArray(), nCommentLength+1, RTL_TEXTENCODING_ASCII_US);

                    }
                    return nPos + i;
                }
            }
        }
    }
    throw package::ZipException( OUString( RTL_CONSTASCII_USTRINGPARAM ( "Zip END signature not found!") ), uno::Reference < uno::XInterface> () );
}

sal_Int32 ZipFile::readCEN()
    throw(io::IOException, package::ZipException, uno::RuntimeException)
{
    sal_Int32 nEndPos, nLocPos;
    sal_Int16  nCount, nTotal;
    sal_Int32 nCenLen, nCenPos, nCenOff;

    nEndPos = findEND();
    if (nEndPos == -1)
        return -1;
    aGrabber.seek(nEndPos + ENDTOT);
    aGrabber >> nTotal;
    aGrabber >> nCenLen;
    aGrabber >> nCenOff;

    if (nTotal<0 || nTotal * CENHDR > nCenLen)
        throw package::ZipException(OUString( RTL_CONSTASCII_USTRINGPARAM ( "invalid END header (bad entry count)") ), uno::Reference < uno::XInterface > ());

    if (nTotal > ZIP_MAXENTRIES)
        throw package::ZipException(OUString( RTL_CONSTASCII_USTRINGPARAM ( "too many entries in ZIP File") ), uno::Reference < uno::XInterface > ());

    if (nCenLen < 0 || nCenLen > nEndPos)
        throw package::ZipException(OUString( RTL_CONSTASCII_USTRINGPARAM ( "Invalid END header (bad central directory size)") ), uno::Reference < uno::XInterface > ());

    nCenPos = nEndPos - nCenLen;

    if (nCenOff < 0 || nCenOff > nCenPos)
        throw package::ZipException(OUString( RTL_CONSTASCII_USTRINGPARAM ( "Invalid END header (bad central directory size)") ), uno::Reference < uno::XInterface > ());

    nLocPos = nCenPos - nCenOff;
    aGrabber.seek(nCenPos);

    package::ZipEntry *pEntry = new package::ZipEntry;
    for (nCount = 0 ; nCount < nTotal; nCount++)
    {
        sal_Int32 nTestSig, nCRC, nCompressedSize, nTime, nSize, nExtAttr, nOffset;
        sal_Int16 nVerMade, nVersion, nFlag, nHow, nNameLen, nExtraLen, nCommentLen;
        sal_Int16 nDisk, nIntAttr;

        if (aGrabber.getPosition() - nCenPos + CENHDR > nCenLen)
            throw package::ZipException(OUString( RTL_CONSTASCII_USTRINGPARAM ( "Invalid CEN header (bad header size check 1)") ), uno::Reference < uno::XInterface > ());

        aGrabber >> nTestSig;
        if (nTestSig != CENSIG)
            throw package::ZipException(OUString( RTL_CONSTASCII_USTRINGPARAM ( "Invalid CEN header (bad signature)") ), uno::Reference < uno::XInterface > ());

        aGrabber >> nVerMade;
        aGrabber >> nVersion;
        if ((nVersion & 1) == 1)
            throw package::ZipException(OUString( RTL_CONSTASCII_USTRINGPARAM ( "Invalid CEN header (encrypted entry)") ), uno::Reference < uno::XInterface > ());

        aGrabber >> nFlag;
        aGrabber >> nHow;
        if (nHow != STORED && nHow != DEFLATED)
            throw package::ZipException(OUString( RTL_CONSTASCII_USTRINGPARAM ( "Invalid CEN header (bad compression method)") ), uno::Reference < uno::XInterface > ());

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
            throw package::ZipException(OUString( RTL_CONSTASCII_USTRINGPARAM ( "Invalid CEN header (bad header size check 2)") ), uno::Reference < uno::XInterface > ());

        if (nNameLen > ZIP_MAXNAMELEN)
            throw package::ZipException(OUString( RTL_CONSTASCII_USTRINGPARAM ( "name length exceeds 512 bytes" ) ), uno::Reference < uno::XInterface > ());

        if (nExtraLen > ZIP_MAXEXTRA)
            throw package::ZipException(OUString( RTL_CONSTASCII_USTRINGPARAM ( "extra header info exceeds 256 bytes") ), uno::Reference < uno::XInterface > ());

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
        uno::Sequence < sal_Int8> aSequence (nNameLen);
        aGrabber.readBytes(aSequence, nNameLen);
        pEntry->sName = OUString((sal_Char*)aSequence.getConstArray(), nNameLen, RTL_TEXTENCODING_ASCII_US);

        aGrabber.seek(aGrabber.getPosition() + nExtraLen);
        if (nCommentLen>0)
        {
            uno::Sequence < sal_Int8 > aCommentSeq( nCommentLen );
            aGrabber.readBytes(aCommentSeq, nCommentLen);
            pEntry->sComment = OUString((sal_Char*)aCommentSeq.getConstArray(), nNameLen, RTL_TEXTENCODING_ASCII_US);
        }
        aEntries[pEntry->sName] = *pEntry;
    }
    delete pEntry;

    if (nCount != nTotal)
        throw package::ZipException(OUString( RTL_CONSTASCII_USTRINGPARAM ( "Count != Total") ), uno::Reference < uno::XInterface > ());

    return nCenPos;
}
