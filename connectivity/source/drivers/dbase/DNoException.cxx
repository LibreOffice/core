/*************************************************************************
 *
 *  $RCSfile: DNoException.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:00:49 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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

#ifndef _CONNECTIVITY_DBASE_TABLE_HXX_
#include "dbase/DTable.hxx"
#endif
#ifndef _CONNECTIVITY_DBASE_INDEX_HXX_
#include "dbase/DIndex.hxx"
#endif
#ifndef _CONNECTIVITY_DBASE_INDEXNODE_HXX_
#include "dbase/dindexnode.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#include <sal/types.h>
#include <algorithm>

using namespace connectivity;
using namespace connectivity::dbase;
using namespace com::sun::star::uno;
using namespace com::sun::star::sdbc;
//------------------------------------------------------------------
sal_Bool ODbaseTable::seekRow(IResultSetHelper::Movement eCursorPosition, sal_Int32 nOffset, sal_Int32& nCurPos)
{
    // ----------------------------------------------------------
    // Positionierung vorbereiten:
    OSL_ENSURE(m_pFileStream,"ODbaseTable::seekRow: FileStream is NULL!");

    sal_uInt32  nNumberOfRecords = (sal_uInt32)m_aHeader.db_anz;
    sal_uInt32 nTempPos = m_nFilePos;
    m_nFilePos = nCurPos;

    switch(eCursorPosition)
    {
        case IResultSetHelper::NEXT:
            ++m_nFilePos;
            break;
        case IResultSetHelper::PRIOR:
            if (m_nFilePos > 0)
                --m_nFilePos;
            break;
        case IResultSetHelper::FIRST:
            m_nFilePos = 1;
            break;
        case IResultSetHelper::LAST:
            m_nFilePos = nNumberOfRecords;
            break;
        case IResultSetHelper::RELATIVE:
            m_nFilePos = (((sal_Int32)m_nFilePos) + nOffset < 0) ? 0L
                            : (sal_uInt32)(((sal_Int32)m_nFilePos) + nOffset);
            break;
        case IResultSetHelper::ABSOLUTE:
        case IResultSetHelper::BOOKMARK:
            m_nFilePos = (sal_uInt32)nOffset;
            break;
    }

    if (m_nFilePos > (sal_Int32)nNumberOfRecords)
        m_nFilePos = (sal_Int32)nNumberOfRecords + 1;

    if (m_nFilePos == 0 || m_nFilePos == (sal_Int32)nNumberOfRecords + 1)
        goto Error;
    else
    {
        sal_uInt16 nEntryLen = m_aHeader.db_slng;

        OSL_ENSURE(m_nFilePos >= 1,"SdbDBFCursor::FileFetchRow: ungueltige Record-Position");
        sal_Int32 nPos = m_aHeader.db_kopf + (sal_Int32)(m_nFilePos-1) * nEntryLen;

        ULONG nLen = m_pFileStream->Seek(nPos);
        if (m_pFileStream->GetError() != ERRCODE_NONE)
            goto Error;

        nLen = m_pFileStream->Read((char*)m_pBuffer, nEntryLen);
        if (m_pFileStream->GetError() != ERRCODE_NONE)
            goto Error;
    }
    goto End;

Error:
    switch(eCursorPosition)
    {
        case IResultSetHelper::PRIOR:
        case IResultSetHelper::FIRST:
            m_nFilePos = 0;
            break;
        case IResultSetHelper::LAST:
        case IResultSetHelper::NEXT:
        case IResultSetHelper::ABSOLUTE:
        case IResultSetHelper::RELATIVE:
            if (nOffset > 0)
                m_nFilePos = nNumberOfRecords + 1;
            else if (nOffset < 0)
                m_nFilePos = 0;
            break;
        case IResultSetHelper::BOOKMARK:
            m_nFilePos = nTempPos;   // vorherige Position
    }
    //  aStatus.Set(SDB_STAT_NO_DATA_FOUND);
    return sal_False;

End:
    nCurPos = m_nFilePos;
    return sal_True;
}
// -----------------------------------------------------------------------------
BOOL ODbaseTable::ReadMemo(ULONG nBlockNo, ORowSetValue& aVariable)
{
    BOOL bIsText = TRUE;
    //  SdbConnection* pConnection = GetConnection();

    m_pMemoStream->Seek(nBlockNo * m_aMemoHeader.db_size);
    switch (m_aMemoHeader.db_typ)
    {
        case MemodBaseIII: // dBase III-Memofeld, endet mit Ctrl-Z
        {
            const char cEOF = (char) 0x1a;
            ByteString aBStr;
            static char aBuf[514];
            aBuf[512] = 0;          // sonst kann der Zufall uebel mitspielen
            BOOL bReady = sal_False;

            do
            {
                m_pMemoStream->Read(&aBuf,512);

                USHORT i = 0;
                while (aBuf[i] != cEOF && ++i < 512)
                    ;
                bReady = aBuf[i] == cEOF;

                aBuf[i] = 0;
                aBStr += aBuf;

            } while (!bReady && !m_pMemoStream->IsEof() && aBStr.Len() < STRING_MAXLEN);

            ::rtl::OUString aStr(aBStr.GetBuffer(), aBStr.Len(),getConnection()->getTextEncoding());
            aVariable = aStr;

        } break;
        case MemoFoxPro:
        case MemodBaseIV: // dBase IV-Memofeld mit Laengenangabe
        {
            char sHeader[4];
            m_pMemoStream->Read(sHeader,4);
            // Foxpro stores text and binary data
            if (m_aMemoHeader.db_typ == MemoFoxPro)
            {
                if (((BYTE)sHeader[0]) != 0 || ((BYTE)sHeader[1]) != 0 || ((BYTE)sHeader[2]) != 0)
                {
//                  String aText = String(SdbResId(STR_STAT_IResultSetHelper::INVALID));
//                  aText.SearchAndReplace(String::CreateFromAscii("%%d"),m_pMemoStream->GetFileName());
//                  aText.SearchAndReplace(String::CreateFromAscii("%%t"),aStatus.TypeToString(MEMO));
//                  aStatus.Set(SDB_STAT_ERROR,
//                          String::CreateFromAscii("01000"),
//                          aStatus.CreateErrorMessage(aText),
//                          0, String() );
                    return sal_False;
                }

                bIsText = sHeader[3] != 0;
            }
            else if (((BYTE)sHeader[0]) != 0xFF || ((BYTE)sHeader[1]) != 0xFF || ((BYTE)sHeader[2]) != 0x08)
            {
//              String aText = String(SdbResId(STR_STAT_IResultSetHelper::INVALID));
//              aText.SearchAndReplace(String::CreateFromAscii("%%d"),m_pMemoStream->GetFileName());
//              aText.SearchAndReplace(String::CreateFromAscii("%%t"),aStatus.TypeToString(MEMO));
//              aStatus.Set(SDB_STAT_ERROR,
//                      String::CreateFromAscii("01000"),
//                      aStatus.CreateErrorMessage(aText),
//                      0, String() );
                return sal_False;
            }

            ULONG nLength;
            (*m_pMemoStream) >> nLength;

            if (m_aMemoHeader.db_typ == MemodBaseIV)
                nLength -= 8;

            //  char cChar;
            ::rtl::OUString aStr;
            while ( nLength > STRING_MAXLEN )
            {
                ByteString aBStr;
                aBStr.Expand(STRING_MAXLEN);
                m_pMemoStream->Read(aBStr.AllocBuffer(STRING_MAXLEN),STRING_MAXLEN);
                aStr += ::rtl::OUString(aBStr.GetBuffer(),aBStr.Len(), getConnection()->getTextEncoding());
                nLength -= STRING_MAXLEN;
            }
            if ( nLength > 0 )
            {
                ByteString aBStr;
                aBStr.Expand(static_cast<xub_StrLen>(nLength));
                m_pMemoStream->Read(aBStr.AllocBuffer(static_cast<xub_StrLen>(nLength)),nLength);
                //  aBStr.ReleaseBufferAccess();

                aStr += ::rtl::OUString(aBStr.GetBuffer(),aBStr.Len(), getConnection()->getTextEncoding());

            }
            if ( aStr.getLength() )
                aVariable = aStr;
        }
    }
    return sal_True;
}
// -----------------------------------------------------------------------------
BOOL ODbaseTable::WriteMemo(ORowSetValue& aVariable, ULONG& rBlockNr)
{
    // wird die BlockNr 0 vorgegeben, wird der block ans Ende gehaengt
    char cChar = 0;
    BOOL bIsText = TRUE;
    //  SdbConnection* pConnection = GetConnection();

    ULONG nSize = 0;
    ULONG nStreamSize;
    BYTE nHeader[4];

    ByteString aStr;
        //      ::Sequence<sal_Int8>* pData = NULL;
//  if (aVariable.getValueType() == ::getCppuType((const ::com::sun::star::uno::Sequence< sal_Int8 > *)0))
//  {
//              pData = (::Sequence<sal_Int8>*)aVariable.get();
//      nSize = pData->getLength();
//  }
//  else
//  {
        aStr = ByteString(String(aVariable.getString()), getConnection()->getTextEncoding());
        nSize = aStr.Len();
    //  }

    // Anhaengen oder ueberschreiben
    BOOL bAppend = rBlockNr == 0;

    if (!bAppend)
    {
        switch (m_aMemoHeader.db_typ)
        {
            case MemodBaseIII: // dBase III-Memofeld, endet mit 2 * Ctrl-Z
                bAppend = nSize > (512 - 2);
                break;
            case MemoFoxPro:
            case MemodBaseIV: // dBase IV-Memofeld mit Laengenangabe
            {
                char sHeader[4];
                m_pMemoStream->Seek(rBlockNr * m_aMemoHeader.db_size);
                m_pMemoStream->SeekRel(4L);
                m_pMemoStream->Read(sHeader,4);

                ULONG nOldSize;
                if (m_aMemoHeader.db_typ == MemoFoxPro)
                    nOldSize = ((((unsigned char)sHeader[0]) * 256 +
                                 (unsigned char)sHeader[1]) * 256 +
                                 (unsigned char)sHeader[2]) * 256 +
                                 (unsigned char)sHeader[3];
                else
                    nOldSize = ((((unsigned char)sHeader[3]) * 256 +
                                 (unsigned char)sHeader[2]) * 256 +
                                 (unsigned char)sHeader[1]) * 256 +
                                 (unsigned char)sHeader[0]  - 8;

                // passt die neue Laenge in die belegten Bloecke
                ULONG nUsedBlocks = ((nSize + 8) / m_aMemoHeader.db_size) + (((nSize + 8) % m_aMemoHeader.db_size > 0) ? 1 : 0),
                      nOldUsedBlocks = ((nOldSize + 8) / m_aMemoHeader.db_size) + (((nOldSize + 8) % m_aMemoHeader.db_size > 0) ? 1 : 0);
                bAppend = nUsedBlocks > nOldUsedBlocks;
            }
        }
    }

    if (bAppend)
    {
        ULONG nStreamSize;
        nStreamSize = m_pMemoStream->Seek(STREAM_SEEK_TO_END);
        // letzten block auffuellen
        rBlockNr = (nStreamSize / m_aMemoHeader.db_size) + ((nStreamSize % m_aMemoHeader.db_size) > 0 ? 1 : 0);

        m_pMemoStream->SetStreamSize(rBlockNr * m_aMemoHeader.db_size);
        m_pMemoStream->Seek(STREAM_SEEK_TO_END);
    }
    else
    {
        m_pMemoStream->Seek(rBlockNr * m_aMemoHeader.db_size);
    }

    switch (m_aMemoHeader.db_typ)
    {
        case MemodBaseIII: // dBase III-Memofeld, endet mit Ctrl-Z
        {
            const char cEOF = (char) 0x1a;
            nSize++;

//          if (pData)
//          {
//              m_pMemoStream->Write((const char*) pData->getConstArray(), pData->getLength());
//          }
//          else
//          {
                m_pMemoStream->Write(aStr.GetBuffer(), aStr.Len());
            //  }

            (*m_pMemoStream) << cEOF << cEOF;
        } break;
        case MemoFoxPro:
        case MemodBaseIV: // dBase IV-Memofeld mit Laengenangabe
        {
            (*m_pMemoStream) << (BYTE)0xFF
                                         << (BYTE)0xFF
                                         << (BYTE)0x08;

            UINT32 nWriteSize = nSize;
            if (m_aMemoHeader.db_typ == MemoFoxPro)
            {
                (*m_pMemoStream) << (BYTE) 0x01; // ((pData = NULL) ? 0x01 : 0x00);
                for (int i = 4; i > 0; nWriteSize >>= 8)
                    nHeader[--i] = (BYTE) (nWriteSize % 256);
            }
            else
            {
                (*m_pMemoStream) << (BYTE) 0x00;
                nWriteSize += 8;
                for (int i = 0; i < 4; nWriteSize >>= 8)
                    nHeader[i++] = (BYTE) (nWriteSize % 256);
            }

            m_pMemoStream->Write(nHeader,4);
//          if (pData)
//          {
//              m_pMemoStream->Write((const char*) pData->getConstArray(), pData->getLength());
//          }
//          else
//          {
                m_pMemoStream->Write(aStr.GetBuffer(), aStr.Len());
            //  }
            m_pMemoStream->Flush();
        }
    }


    // Schreiben der neuen Blocknummer
    if (bAppend)
    {
        nStreamSize = m_pMemoStream->Seek(STREAM_SEEK_TO_END);
        m_aMemoHeader.db_next = (nStreamSize / m_aMemoHeader.db_size) + ((nStreamSize % m_aMemoHeader.db_size) > 0 ? 1 : 0);

        // Schreiben der neuen Blocknummer
        m_pMemoStream->Seek(0L);
        (*m_pMemoStream) << m_aMemoHeader.db_next;
        m_pMemoStream->Flush();
    }
    return sal_True;
}
// -----------------------------------------------------------------------------
void ODbaseTable::AllocBuffer()
{
    UINT16 nSize = m_aHeader.db_slng;
    OSL_ENSURE(nSize > 0, "Size too small");

    if (m_nBufferSize != nSize)
    {
        delete m_pBuffer;
        m_pBuffer = NULL;
    }

    // Falls noch kein Puffer vorhanden: allozieren:
    if (m_pBuffer == NULL && nSize)
    {
        m_nBufferSize = nSize;
        m_pBuffer       = new BYTE[m_nBufferSize+1];
    }
}
// -----------------------------------------------------------------------------
BOOL ODbaseTable::WriteBuffer()
{
    OSL_ENSURE(m_nFilePos >= 1,"SdbDBFCursor::FileFetchRow: ungueltige Record-Position");

    // Auf gewuenschten Record positionieren:
    long nPos = m_aHeader.db_kopf + (long)(m_nFilePos-1) * m_aHeader.db_slng;
    m_pFileStream->Seek(nPos);
    return m_pFileStream->Write((char*) m_pBuffer, m_aHeader.db_slng) > 0;
}
// -----------------------------------------------------------------------------
sal_Int32 ODbaseTable::getCurrentLastPos() const
{
    return m_aHeader.db_anz;
}
// -----------------------------------------------------------------------------
//==================================================================
// ONDXNode
//==================================================================

//------------------------------------------------------------------
void ONDXNode::Read(SvStream &rStream, ODbaseIndex& rIndex)
{
    rStream >> aKey.nRecord; // schluessel

    if (rIndex.getHeader().db_keytype)
    {
        double aDbl;
        rStream >> aDbl;
        aKey = ONDXKey(aDbl,aKey.nRecord);
    }
    else
    {
        ByteString aBuf;
        USHORT nLen = rIndex.getHeader().db_keylen;
        char* pStr = aBuf.AllocBuffer(nLen+1);

        rStream.Read(pStr,nLen);
        pStr[nLen] = 0;
        aBuf.ReleaseBufferAccess();
        aBuf.EraseTrailingChars();

        //  aKey = ONDXKey((aBuf,rIndex.GetDBFConnection()->GetCharacterSet()) ,aKey.nRecord);
        aKey = ONDXKey(::rtl::OUString(aBuf.GetBuffer(),aBuf.Len(),rIndex.m_pTable->getConnection()->getTextEncoding()) ,aKey.nRecord);
    }
    rStream >> aChild;
}

union
{
    double aDbl;
    char   aData[128];
} aNodeData;
//------------------------------------------------------------------
void ONDXNode::Write(SvStream &rStream, const ONDXPage& rPage) const
{
    const ODbaseIndex& rIndex = rPage.GetIndex();
    if (!rIndex.isUnique() || rPage.IsLeaf())
        rStream << (sal_uInt32)aKey.nRecord; // schluessel
    else
        rStream << (sal_uInt32)0;   // schluessel

    if (rIndex.getHeader().db_keytype) // double
    {
        if (aKey.getValue().isNull())
        {
            memset(aNodeData.aData,0,rIndex.getHeader().db_keylen);
            rStream.Write((BYTE*)aNodeData.aData,rIndex.getHeader().db_keylen);
        }
        else
            rStream << (double) aKey.getValue();
    }
    else
    {
        memset(aNodeData.aData,0x20,rIndex.getHeader().db_keylen);
        if (!aKey.getValue().isNull())
        {
            ::rtl::OUString sValue = aKey.getValue();
            ByteString aText(sValue.getStr(), rIndex.m_pTable->getConnection()->getTextEncoding());
            strncpy(aNodeData.aData,aText.GetBuffer(),std::min(rIndex.getHeader().db_keylen, aText.Len()));
        }
        rStream.Write((BYTE*)aNodeData.aData,rIndex.getHeader().db_keylen);
    }
    rStream << aChild;
}


//------------------------------------------------------------------
ONDXPagePtr& ONDXNode::GetChild(ODbaseIndex* pIndex, ONDXPage* pParent)
{
    if (!aChild.Is() && pIndex)
    {
        aChild = pIndex->CreatePage(aChild.GetPagePos(),pParent,aChild.HasPage());
    }
    return aChild;
}

//==================================================================
// ONDXKey
//==================================================================
//------------------------------------------------------------------
BOOL ONDXKey::IsText(sal_Int32 eType)
{
    return eType == DataType::VARCHAR || eType == DataType::CHAR;
}

//------------------------------------------------------------------
StringCompare ONDXKey::Compare(const ONDXKey& rKey) const
{
    //  DBG_ASSERT(is(), "Falscher Indexzugriff");
    StringCompare eResult;

    if (getValue().isNull())
    {
        if (rKey.getValue().isNull() || (rKey.IsText(getDBType()) && !rKey.getValue().getString().getLength()))
            eResult = COMPARE_EQUAL;
        else
            eResult = COMPARE_LESS;
    }
    else if (rKey.getValue().isNull())
    {
        if (getValue().isNull() || (IsText(getDBType()) && !getValue().getString().getLength()))
            eResult = COMPARE_EQUAL;
        else
            eResult = COMPARE_GREATER;
    }
    else if (IsText(getDBType()))
    {
        INT32 nRes = getValue().getString().compareTo(rKey.getValue());
        eResult = (nRes > 0) ? COMPARE_GREATER : (nRes == 0) ? COMPARE_EQUAL : COMPARE_LESS;
    }
    else
    {
        double m = getValue(),n = rKey.getValue();
        eResult = (m > n) ? COMPARE_GREATER : (n == m) ? COMPARE_EQUAL : COMPARE_LESS;
    }

    // Record vergleich, wenn Index !Unique
    if (eResult == COMPARE_EQUAL && nRecord && rKey.nRecord)
        eResult = (nRecord > rKey.nRecord) ? COMPARE_GREATER :
                  (nRecord == rKey.nRecord) ? COMPARE_EQUAL : COMPARE_LESS;

    return eResult;
}
// -----------------------------------------------------------------------------
void ONDXKey::setValue(const ORowSetValue& _rVal)
{
    xValue = _rVal;
}
// -----------------------------------------------------------------------------
const ORowSetValue& ONDXKey::getValue() const
{
    return xValue;
}
// -----------------------------------------------------------------------------
SvStream& connectivity::dbase::operator >> (SvStream &rStream, ONDXPagePtr& rPage)
{
    rStream >> rPage.nPagePos;
    return rStream;
}
// -----------------------------------------------------------------------------
SvStream& connectivity::dbase::operator << (SvStream &rStream, const ONDXPagePtr& rPage)
{
    rStream << rPage.nPagePos;
    return rStream;
}
// -----------------------------------------------------------------------------
//==================================================================
// ONDXPagePtr
//==================================================================
//------------------------------------------------------------------
ONDXPagePtr::ONDXPagePtr(const ONDXPagePtr& rRef)
              :ONDXPageRef(rRef)
              ,nPagePos(rRef.nPagePos)
{
}

//------------------------------------------------------------------
ONDXPagePtr::ONDXPagePtr(ONDXPage* pRefPage)
              :ONDXPageRef(pRefPage)
              ,nPagePos(0)
{
    if (pRefPage)
        nPagePos = pRefPage->GetPagePos();
}
//------------------------------------------------------------------
ONDXPagePtr& ONDXPagePtr::operator=(const ONDXPagePtr& rRef)
{
    ONDXPageRef::operator=(rRef);
    nPagePos = rRef.nPagePos;
    return *this;
}

//------------------------------------------------------------------
ONDXPagePtr& ONDXPagePtr::operator= (ONDXPage* pRef)
{
    ONDXPageRef::operator=(pRef);
    nPagePos = (pRef) ? pRef->GetPagePos() : 0;
    return *this;
}
// -----------------------------------------------------------------------------
static UINT32 nValue;
//------------------------------------------------------------------
SvStream& connectivity::dbase::operator >> (SvStream &rStream, ONDXPage& rPage)
{
    rStream.Seek(rPage.GetPagePos() * 512);
    rStream >> nValue >> rPage.aChild;
    rPage.nCount = USHORT(nValue);

//  DBG_ASSERT(rPage.nCount && rPage.nCount < rPage.GetIndex().GetMaxNodes(), "Falscher Count");
    for (USHORT i = 0; i < rPage.nCount; i++)
        rPage[i].Read(rStream, rPage.GetIndex());
    return rStream;
}

//------------------------------------------------------------------
SvStream& connectivity::dbase::operator << (SvStream &rStream, const ONDXPage& rPage)
{
    // Seite existiert noch nicht
    ULONG nSize = (rPage.GetPagePos() + 1) * 512;
    if (nSize > rStream.Seek(STREAM_SEEK_TO_END))
    {
        rStream.SetStreamSize(nSize);
        rStream.Seek(rPage.GetPagePos() * 512);

        char aEmptyData[512];
        memset(aEmptyData,0x00,512);
        rStream.Write((BYTE*)aEmptyData,512);
    }
    ULONG nCurrentPos = rStream.Seek(rPage.GetPagePos() * 512);

    nValue = rPage.nCount;
    rStream << nValue << rPage.aChild;

    USHORT i = 0;
    for (; i < rPage.nCount; i++)
        rPage[i].Write(rStream, rPage);

    // check if we have to fill the stream with '\0'
    if(i < rPage.rIndex.getHeader().db_maxkeys)
    {
        ULONG nTell = rStream.Tell() % 512;
        USHORT nBufferSize = rStream.GetBufferSize();
        ULONG nSize = nBufferSize - nTell;
        char* pEmptyData = new char[nSize];
        memset(pEmptyData,0x00,nSize);
        rStream.Write((BYTE*)pEmptyData,nSize);
        rStream.Seek(nTell);
        delete [] pEmptyData;
    }
    return rStream;
}
// -----------------------------------------------------------------------------
#if OSL_DEBUG_LEVEL > 1
//------------------------------------------------------------------
void ONDXPage::PrintPage()
{
    DBG_TRACE4("\nSDB: -----------Page: %d  Parent: %d  Count: %d  Child: %d-----",
        nPagePos, HasParent() ? aParent->GetPagePos() : 0 ,nCount, aChild.GetPagePos());

    for (USHORT i = 0; i < nCount; i++)
    {
        ONDXNode rNode = (*this)[i];
        ONDXKey&  rKey = rNode.GetKey();
        if (!IsLeaf())
            rNode.GetChild(&rIndex, this);

        if (rKey.getValue().isNull())
        {
            DBG_TRACE2("SDB: [%d,NULL,%d]",rKey.GetRecord(), rNode.GetChild().GetPagePos());
        }
        else if (rIndex.getHeader().db_keytype)
        {
            DBG_TRACE3("SDB: [%d,%f,%d]",rKey.GetRecord(), rKey.getValue().getDouble(),rNode.GetChild().GetPagePos());
        }
        else
        {
            DBG_TRACE3("SDB: [%d,%s,%d]",rKey.GetRecord(), (const char* )ByteString(rKey.getValue().getString().getStr(), rIndex.m_pTable->getConnection()->getTextEncoding()).GetBuffer(),rNode.GetChild().GetPagePos());
        }
    }
    DBG_TRACE("SDB: -----------------------------------------------\n");
    if (!IsLeaf())
    {
#if OSL_DEBUG_LEVEL > 1
        GetChild(&rIndex)->PrintPage();
        for (USHORT i = 0; i < nCount; i++)
        {
            ONDXNode rNode = (*this)[i];
            rNode.GetChild(&rIndex,this)->PrintPage();
        }
#endif
    }
    DBG_TRACE("SDB: ===============================================\n");
}
#endif
// -----------------------------------------------------------------------------
BOOL ONDXPage::IsFull() const
{
    return Count() == rIndex.getHeader().db_maxkeys;
}
// -----------------------------------------------------------------------------
//------------------------------------------------------------------
USHORT ONDXPage::Search(const ONDXKey& rSearch)
{
    // binare Suche spaeter
    USHORT i = 0xFFFF;
    while (++i < Count())
        if ((*this)[i].GetKey() == rSearch)
            break;

    return (i < Count()) ? i : NODE_NOTFOUND;
}

//------------------------------------------------------------------
USHORT ONDXPage::Search(const ONDXPage* pPage)
{
    USHORT i = 0xFFFF;
    while (++i < Count())
        if (((*this)[i]).GetChild() == pPage)
            break;

    // wenn nicht gefunden, dann wird davon ausgegangen, dass die Seite selbst
    // auf die Page zeigt
    return (i < Count()) ? i : NODE_NOTFOUND;
}
// -----------------------------------------------------------------------------
// laeuft rekursiv
void ONDXPage::SearchAndReplace(const ONDXKey& rSearch,
                                  ONDXKey& rReplace)
{
    OSL_ENSURE(rSearch != rReplace,"Invalid here:rSearch == rReplace");
    if (rSearch != rReplace)
    {
        USHORT nPos = NODE_NOTFOUND;
        ONDXPage* pPage = this;

        while (pPage && (nPos = pPage->Search(rSearch)) == NODE_NOTFOUND)
            pPage = pPage->aParent;

        if (pPage)
        {
            (*pPage)[nPos].GetKey() = rReplace;
            pPage->SetModified(TRUE);
        }
    }
}
// -----------------------------------------------------------------------------
ONDXNode& ONDXPage::operator[] (USHORT nPos)
{
    DBG_ASSERT(nCount > nPos, "falscher Indexzugriff");
    return ppNodes[nPos];
}

//------------------------------------------------------------------
const ONDXNode& ONDXPage::operator[] (USHORT nPos) const
{
    DBG_ASSERT(nCount > nPos, "falscher Indexzugriff");
    return ppNodes[nPos];
}
// -----------------------------------------------------------------------------
void ONDXPage::Remove(USHORT nPos)
{
    DBG_ASSERT(nCount > nPos, "falscher Indexzugriff");

    for (USHORT i = nPos; i < (nCount-1); i++)
        (*this)[i] = (*this)[i+1];

    nCount--;
    bModified = TRUE;
}
// -----------------------------------------------------------------------------











