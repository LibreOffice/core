/*************************************************************************
 *
 *  $RCSfile: ZipFile.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mtg $ $Date: 2000-11-13 13:38:01 $
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

#ifndef _ZIP_ENUMERATION_HXX
#include "ZipEnumeration.hxx"
#endif

#ifndef _COM_SUN_STAR_PACKAGE_ZIPCONSTANTS_HPP_
#include <com/sun/star/package/ZipConstants.hpp>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _RTL_BYTESEQ_HXX_
#include <rtl/byteseq.hxx>
#endif

#include <string.h>

using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::package::ZipConstants;

/** This class is used to read entries from a zip file
 */
ZipFile::ZipFile (uno::Reference < io::XInputStream > &xInput)
: xStream(xInput)
, aGrabber(xInput)
, pEntries(NULL)
, pTable(NULL)
, nTotal(0)
, nTableLen(0)
{
    readCEN();
}

/*
ZipFile::ZipFile( void )
:pEntries(NULL)
, pTable(NULL)
, nTotal(0)
, nTableLen(0)
{
}
void SAL_CALL ZipFile::setInputStream( const uno::Reference< io::XInputStream >& xInput )
    throw(io::IOException, package::ZipException, uno::RuntimeException)
{
    xStream = uno::Reference <io::XInputStream> (xInput);
    aGrabber.setInputStream ( xInput );
    readCEN();
}
*/

//ZipFile::ZipFile (const uno::Reference < lang::XMultiServiceFactory > &xFactory)
//{
//}
ZipFile::~ZipFile()
{
    if (pEntries != NULL)
        delete []pEntries;
    if (pTable != NULL)
    {
        ZipEntryImpl* pTmp = NULL;
        for (int i =0; i < nTableLen; i++)
        {
            while ((pTmp = pTable[i]) != NULL)
            {
                pTable[i] = pTmp->pNext;
                delete pTmp;
            }
        }
        delete []pTable;
    }

}
void ZipFile::close()
{
}

uno::Reference<container::XEnumeration> ZipFile::entries()
{
    uno::Reference< container::XEnumeration> xEnumRef;
    xEnumRef= new ZipEnumeration( uno::Sequence < package::ZipEntry > (pEntries, nTotal) );
    //xEnumRef = uno::Reference < container::XEnumeration>( (OWeakObject*) pEnum, uno::UNO_QUERY );
//  xEnumRef = uno::Reference < container::XEnumeration>( static_cast < container::XEnumeration *> (pEnum), uno::UNO_QUERY );
    return xEnumRef;
}
::rtl::OUString ZipFile::getName()
{
    return sName;
}
sal_Int32 ZipFile::getSize()
{
    return nTotal;
}

uno::Type ZipFile::getElementType()
{
    return ::getCppuType((package::ZipEntry *) 0);
}
sal_Bool ZipFile::hasElements()
{
    return (nTotal>0);
}

uno::Any ZipFile::getByName(const ::rtl::OUString& rName)
{
    uno::Any aAny;
    sal_Int32 nHash = abs(rName.hashCode() % nTableLen);
       ZipEntryImpl * pEntry = pTable[nHash];
    while (pEntry != NULL)
    {
        if (rName == pEntry->getName())
        {
            //uno::Reference < package::ZepEntry > xEntry = pEntry;
            aAny <<= *(pEntry->pEntry);
            return aAny;
        }
        pEntry = pEntry->pNext;
    }
    throw container::NoSuchElementException();
     return aAny;
}
uno::Sequence<rtl::OUString> ZipFile::getElementNames()
{
    OUString *pNames = new OUString[nTotal];
    for (int i = 0; i < nTotal; i++)
        pNames[i] = pEntries[i].sName;
    return uno::Sequence<OUString> (pNames, nTotal);
}
sal_Bool ZipFile::hasByName(const ::rtl::OUString& rName)
{
    sal_Int32 nHash = abs(rName.hashCode() % nTableLen);
       ZipEntryImpl * pEntry = pTable[nHash];
       while (pEntry != NULL)
    {
        if (rName == pEntry->getName())
            return sal_True;
        pEntry = pEntry->pNext;
    }
     return sal_False;
}
uno::Reference< io::XInputStream> ZipFile::getInputStream(const package::ZipEntry& rEntry)
{
    sal_Int64 nEnd = rEntry.nCompressedSize == 0 ? rEntry.nSize : rEntry.nCompressedSize;
    if (rEntry.nOffset <= 0)
        readLOC(rEntry);
    sal_Int64 nBegin = rEntry.nOffset;
    nEnd +=nBegin;

    uno::Reference< io::XInputStream > xStreamRef = new EntryInputStream(xStream, nBegin, nEnd, 1024);
    return xStreamRef;
}
sal_Bool ZipFile::readLOC(const package::ZipEntry &rEntry)
{
    sal_uInt32 nTestSig, nTime, nCRC, nSize, nCompressedSize;
    sal_uInt16 nVersion, nFlag, nHow, nNameLen, nExtraLen;
    sal_Int32 nPos = -rEntry.nOffset;

    aGrabber.seek(nPos);
    aGrabber >> nTestSig;

    if (nTestSig != LOCSIG)
    {
        DBG_ERROR ("Invalid LOC header (bad signature)");
        return sal_False;
    }
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
    pNonConstEntry->nOffset =  aGrabber.getPosition() + nNameLen + nExtraLen;
    return sal_True;

/*
    ZipEntryImpl *pEntry = new ZipEntryImpl();

    sal_Char * pTmpName = new sal_Char[nNameLen];
    xStream.Read(pTmpName, nNameLen);
    pEntry->sName = ByteSequence( pTmpName, nNameLen, RTL_TEXTENCODING_ASCII_US);
    delete [] pTmpName;

    sal_Char * pTmpExtra = new sal_Char[nExtraLen];
    xStream.Read(pTmpExtra, nExtraLen);
    pEntry->sExtra = ByteSequence(pTmpExtra, nExtraLen);
    delete [] pTmpExtra;

    pEntry->sComment = pComments[pCell->nIndex];
    pEntry->nSize = pCell->nSize;
    pEntry->nCompressedSize = pCell->nCompressedSize;
    pEntry->nCRC = pCell->nCRC;
    pEntry->nTime = nTime;
    pEntry->nMethod = nHow;
    pEntry->nPos = pCell->nPos + LOCHDR + nNameLen + nExtraLen;
    return pEntry;
*/
}


sal_Int32 ZipFile::findEND( )
{
    ULONG nLength=0, nPos=0;
    ByteSequence aByteSeq;
    nLength = nPos = aGrabber.getLength();
    aGrabber.seek( nLength );

    while (nLength - nPos < 0xFFFF)
    {
        ULONG nCount = 0xFFFF - ( nLength - nPos);
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
                sal_uInt32 nEndPos = nPos + i;
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
    return -1;
}

#if 0
sal_Bool ZipFile::isMetaName( ByteSequence &rName)
{
    ByteSequence sTemp = rName.toUpperCase();
    if (sTemp.compareToAscii( "META_INF/", 9) == 0)
        return sal_True;
    return sal_False;

}
void ZipFile::addMetaName( ByteSequence &rName )
{
    aMetaNames.Insert (new String(rName));
}


void ZipFile::addEntryComment( int nIndex, ByteSequence &rComment)
{
    if (pComments == NULL)
        pComments = new ByteSequence[nTotal];
    pComments[nIndex]=rComment;
}
#endif

sal_Int32 ZipFile::readCEN()
{
    sal_uInt32 nEndPos, nLocPos;
    sal_Int16  nCount;
    sal_uInt32 nCenLen, nCenPos, nCenOff;

    nEndPos = findEND();
    aGrabber.seek(nEndPos + ENDTOT);
    aGrabber >> nTotal;
    aGrabber >> nCenLen;
    aGrabber >> nCenOff;

    if (nTotal<0 || nTotal * CENHDR > nCenLen)
    {
        DBG_ERROR("invalid END header (bad entry count)");
        return -1;
    }
    if (nTotal > ZIP_MAXENTRIES)
    {
        DBG_ERROR("too many entries in ZIP File");
        return -1;
    }

    //aGrabber.seek(nEndPos+ENDSIZ);


    if (nCenLen < 0 || nCenLen > nEndPos)
    {
        DBG_ERROR ("invalid END header (bad central directory size)");
        return -1;
    }
    nCenPos = nEndPos - nCenLen;

    if (nCenOff < 0 || nCenOff > nCenPos)
    {
        DBG_ERROR("invalid END header (bad central directory size)");
        return -1;
    }
    nLocPos = nCenPos - nCenOff;

    aGrabber.seek(nCenPos);

    pEntries = new package::ZipEntry[nTotal];
    nTableLen = nTotal * 2;
    pTable = new ZipEntryImpl*[nTableLen];
    memset(pTable, 0, sizeof (ZipEntryImpl*) * nTableLen);

    for (nCount = 0 ; nCount < nTotal; nCount++)
    {
        package::ZipEntry *pEntry = &pEntries[nCount];
        sal_Int32 nTestSig, nCRC, nCompressedSize, nTime, nSize, nExtAttr, nOffset;
        sal_Int16 nVerMade, nVersion, nFlag, nHow, nNameLen, nExtraLen, nCommentLen;
        sal_Int16 nDisk, nIntAttr;
        if (aGrabber.getPosition() - nCenPos + CENHDR > nCenLen)
        {
            DBG_ERROR("invalid CEN header (bad header size check 1");
            break;
        }
        aGrabber >> nTestSig;
        if (nTestSig != CENSIG)
        {
            DBG_ERROR ("invalid CEN header (bad signature)");
            break;
        }
        aGrabber >> nVerMade;
        aGrabber >> nVersion;
        if ((nVersion & 1) == 1)
        {
            DBG_ERROR ( "invalid CEN header (encrypted entry)");
            break;
        }
        aGrabber >> nFlag;
        aGrabber >> nHow;
        if (nHow != STORED && nHow != DEFLATED)
        {
            DBG_ERROR ( "invalid CEN header (bad compression method)");
            break;
        }
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
        {
            DBG_ERROR ( "invalid CEN header (bad header size check 2)");
            break;
        }
        if (nNameLen > ZIP_MAXNAMELEN)
        {
            DBG_ERROR ( "name length exceeds 512 bytes");
            break;
        }
        if (nExtraLen > ZIP_MAXEXTRA)
        {
            DBG_ERROR ( "extra header info exceeds 256 bytes");
            break;
        }

        pEntry->nTime   = nTime;
        pEntry->nCrc    = nCRC;
        pEntry->nSize   = nSize;
        pEntry->nCompressedSize = nCompressedSize;
        pEntry->nMethod = nHow;
        pEntry->nFlag   = nFlag;
        pEntry->nVersion= nVersion;
        pEntry->nOffset = nOffset + nLocPos;

        pEntry->nOffset *= -1;

        if (nHow == STORED)
            pEntry->nCompressedSize = 0;

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
        sal_Int32 nHash = abs(pEntry->sName.hashCode() % nTableLen);
         ZipEntryImpl* pTmp = new ZipEntryImpl();
        pTmp->pEntry = pEntry;
        pTmp->pNext = pTable[nHash];
        pTable[nHash] = pTmp;
        /*pEntry->pNext = pTable[nHash];
        pTable[nHash] = pEntry;*/
    }

    if (nCount != nTotal)
    {
        DBG_ERROR("Count != total!");
        return -1;
    }
    return nCenPos;
}


