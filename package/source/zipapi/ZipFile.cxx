/*************************************************************************
 *
 *  $RCSfile: ZipFile.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: mtg $ $Date: 2000-12-08 13:40:03 $
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
ZipFile::ZipFile( com::sun::star::uno::Reference < com::sun::star::io::XInputStream > &xInput, sal_Bool bInitialise)
    throw(io::IOException, package::ZipException, uno::RuntimeException)
: xStream(xInput)
, aGrabber(xInput)
{
    if (bInitialise)
        readCEN();
}

void ZipFile::updateFromManList(std::vector < ManifestEntry * > &rManList)
{
    sal_Int32 i=0, nSize = rManList.size();
    aEntries.clear();

    for (;i < nSize ; i++)
    {
        package::ZipEntry *pEntry = new package::ZipEntry;
        pEntry->nVersion        = rManList[i]->pEntry->nVersion;
        pEntry->nFlag           = rManList[i]->pEntry->nFlag;
        pEntry->nMethod         = rManList[i]->pEntry->nMethod;
        pEntry->nTime           = rManList[i]->pEntry->nTime;
        pEntry->nCrc            = rManList[i]->pEntry->nCrc;
        pEntry->nCompressedSize = rManList[i]->pEntry->nCompressedSize;
        pEntry->nSize           = rManList[i]->pEntry->nSize;
        rManList[i]->pEntry->nOffset *=-1;
        pEntry->nOffset         = rManList[i]->pEntry->nOffset;
        pEntry->sName           = rManList[i]->pEntry->sName;
        pEntry->extra           = rManList[i]->pEntry->extra;
        pEntry->sComment        = rManList[i]->pEntry->sComment;
        aEntries[pEntry->sName] = *pEntry;
    }
}

ZipFile::~ZipFile()
{
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

sal_Bool ZipFile::readLOC(const package::ZipEntry &rEntry)
    throw(io::IOException, package::ZipException, uno::RuntimeException)
{
    sal_uInt32 nTestSig, nTime, nCRC, nSize, nCompressedSize;
    sal_uInt16 nVersion, nFlag, nHow, nNameLen, nExtraLen;
    sal_Int32 nPos = -rEntry.nOffset;

    aGrabber.seek(nPos);
    aGrabber >> nTestSig;

    if (nTestSig != LOCSIG)
        throw package::ZipException( OUString::createFromAscii("Invalid LOC header (bad signature"), uno::Reference < uno::XInterface > () );
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
    throw package::ZipException( OUString::createFromAscii("Zip END signature not found!"), uno::Reference < uno::XInterface> () );
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
        throw package::ZipException(OUString::createFromAscii("invalid END header (bad entry count)"), uno::Reference < uno::XInterface > ());

    if (nTotal > ZIP_MAXENTRIES)
        throw package::ZipException(OUString::createFromAscii("too many entries in ZIP File"), uno::Reference < uno::XInterface > ());

    if (nCenLen < 0 || nCenLen > nEndPos)
        throw package::ZipException(OUString::createFromAscii("Invalid END header (bad central directory size)"), uno::Reference < uno::XInterface > ());

    nCenPos = nEndPos - nCenLen;

    if (nCenOff < 0 || nCenOff > nCenPos)
        throw package::ZipException(OUString::createFromAscii("Invalid END header (bad central directory size)"), uno::Reference < uno::XInterface > ());

    nLocPos = nCenPos - nCenOff;
    aGrabber.seek(nCenPos);

    for (nCount = 0 ; nCount < nTotal; nCount++)
    {
        package::ZipEntry *pEntry = new package::ZipEntry;
        sal_Int32 nTestSig, nCRC, nCompressedSize, nTime, nSize, nExtAttr, nOffset;
        sal_Int16 nVerMade, nVersion, nFlag, nHow, nNameLen, nExtraLen, nCommentLen;
        sal_Int16 nDisk, nIntAttr;

        if (aGrabber.getPosition() - nCenPos + CENHDR > nCenLen)
            throw package::ZipException(OUString::createFromAscii("Invalid CEN header (bad header size check 1)"), uno::Reference < uno::XInterface > ());

        aGrabber >> nTestSig;
        if (nTestSig != CENSIG)
            throw package::ZipException(OUString::createFromAscii("Invalid CEN header (bad signature)"), uno::Reference < uno::XInterface > ());

        aGrabber >> nVerMade;
        aGrabber >> nVersion;
        if ((nVersion & 1) == 1)
            throw package::ZipException(OUString::createFromAscii("Invalid CEN header (encrypted entry)"), uno::Reference < uno::XInterface > ());

        aGrabber >> nFlag;
        aGrabber >> nHow;
        if (nHow != STORED && nHow != DEFLATED)
            throw package::ZipException(OUString::createFromAscii("Invalid CEN header (bad compression method)"), uno::Reference < uno::XInterface > ());

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
            throw package::ZipException(OUString::createFromAscii("Invalid CEN header (bad header siez check 2)"), uno::Reference < uno::XInterface > ());

        if (nNameLen > ZIP_MAXNAMELEN)
            throw package::ZipException(OUString::createFromAscii("name length exceeds 512 bytes"), uno::Reference < uno::XInterface > ());

        if (nExtraLen > ZIP_MAXEXTRA)
            throw package::ZipException(OUString::createFromAscii("extra header info exceeds 256 bytes"), uno::Reference < uno::XInterface > ());

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

    if (nCount != nTotal)
        throw package::ZipException(OUString::createFromAscii("Count != Total"), uno::Reference < uno::XInterface > ());

    return nCenPos;
}
