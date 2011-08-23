/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "first.hxx"
#include "assert.h"
#include <stdio.h>
#include <sot/storinfo.hxx>
namespace OpenStormBento
{

// String definitions
const char gsBenMagicBytes[] = BEN_MAGIC_BYTES;

/**
*	New bento container from file stream
*	@param	pointer to length of bento file
*	@param	pointer to pointer of Bento Container object
*	@return	error code
*/
ULONG BenOpenContainer(LwpSvStream * pStream, pLtcBenContainer * ppContainer)
{
    BenError Err;

    *ppContainer = NULL;

    if (NULL == pStream)
    {
        return BenErr_ContainerWithNoObjects;
    }

    pLtcBenContainer pContainer = new LtcBenContainer(pStream);
    if ((Err = pContainer->Open()) != BenErr_OK) // delete two inputs
    {
        delete pContainer;
        return BenErr_InvalidTOC;
    }

    *ppContainer = pContainer;
    return BenErr_OK;
}
BenError
LtcBenContainer::Close()
{
    return BenErr_OK;
}

LtcBenContainer::~LtcBenContainer()
{
    Close();
}

BenError
LtcBenContainer::Open() // delete two inputs
{
    BenError Err;
    CBenTOCReader TOCReader(this);
    if ((Err = TOCReader.ReadLabelAndTOC()) != BenErr_OK)
    {
        return Err;
    }
    return BenErr_OK;
}

void
LtcBenContainer::Release()
{
    delete this;
}

BenError
LtcBenContainer::RegisterPropertyName(const char * sPropertyName,
  pCBenPropertyName * ppPropertyName)
{
    pCBenNamedObjectListElmt pPrevNamedObjectListElmt;
    pCBenNamedObject pNamedObject = FindNamedObject(&cNamedObjects,
      sPropertyName, &pPrevNamedObjectListElmt);

    if (pNamedObject != NULL)
    {
        if (! pNamedObject->IsPropertyName())
            return BenErr_NameConflict;
        else *ppPropertyName = (pCBenPropertyName) pNamedObject;
    }
    else
    {
        pCBenIDListElmt pPrevObject;
        if (FindID(&cObjects, cNextAvailObjectID, &pPrevObject) != NULL)
            return BenErr_DuplicateObjectID;

        *ppPropertyName = new CBenPropertyName(this, cNextAvailObjectID,
          (pCBenObject) pPrevObject, sPropertyName, pPrevNamedObjectListElmt);
        ++cNextAvailObjectID;
    }

    return BenErr_OK;
}

BenError
LtcBenContainer::RegisterTypeName(const char * sTypeName,
  pCBenTypeName * ppTypeName)
{
    pCBenNamedObjectListElmt pPrevNamedObjectListElmt;
    pCBenNamedObject pNamedObject = FindNamedObject(&cNamedObjects, sTypeName,
      &pPrevNamedObjectListElmt);

    if (pNamedObject != NULL)
    {
        if (! pNamedObject->IsTypeName())
            return BenErr_NameConflict;
        else *ppTypeName = (pCBenTypeName) pNamedObject;
    }
    else
    {
        pCBenIDListElmt pPrevObject;
        if (FindID(&cObjects, cNextAvailObjectID, &pPrevObject) != NULL)
            return BenErr_DuplicateObjectID;

        *ppTypeName = new CBenTypeName(this, cNextAvailObjectID,
          (pCBenObject) pPrevObject, sTypeName, pPrevNamedObjectListElmt);
        ++cNextAvailObjectID;
    }

    return BenErr_OK;
}

BenError
LtcBenContainer::NewObject(pCBenObject * ppBenObject)
{
    pCBenIDListElmt pPrev;
    if (FindID(&cObjects, cNextAvailObjectID, &pPrev) != NULL)
        return BenErr_DuplicateObjectID;

    *ppBenObject = new CBenObject(this, cNextAvailObjectID, pPrev);

    ++cNextAvailObjectID;
    return BenErr_OK;
}

pCBenObject
LtcBenContainer::GetNextObject(pCBenObject pCurrObject)
{
    return (pCBenObject) cObjects.GetNextOrNULL(pCurrObject);
}

pCBenObject
LtcBenContainer::FindNextObjectWithProperty(pCBenObject pCurrObject,
  BenObjectID PropertyID)
{
    while ((pCurrObject = GetNextObject(pCurrObject)) != NULL)
        if (pCurrObject->UseProperty(PropertyID) != NULL)
            return pCurrObject;

    return NULL;
}

pCBenObject
LtcBenContainer::FindObject(BenObjectID ObjectID)
{
    return (pCBenObject) FindID(&cObjects, ObjectID, NULL);
}

/**
*	Construction
*	@param	Bento file stream pointer
*	@return
*/
LtcBenContainer::LtcBenContainer(LwpSvStream * pStream)
{
    cpStream = pStream;
    pStream->Seek(STREAM_SEEK_TO_END);
    m_ulLength = pStream->Tell();
    pStream->Seek(STREAM_SEEK_TO_BEGIN);
}

/**
*	Read buffer fro bento file with specified buffer
*	@date	07/05/2004
*	@param	buffer pointer
*	@param  buffer size
*	@param	number of bytes read
*	@return	BenError
*/
BenError LtcBenContainer::Read(BenDataPtr pBuffer, unsigned long MaxSize,
  unsigned long * pAmtRead)
{
    *pAmtRead = cpStream->Read(pBuffer, MaxSize);
    return BenErr_OK;
}
/**
*	Read buffer from bento file with specified size
*	@date	07/05/2004
*	@param	buffer pointer
*	@param	number of bytes to be read
*	@return	BenError
*/
BenError LtcBenContainer::ReadKnownSize(BenDataPtr pBuffer, unsigned long Amt)
{
    ULONG ulLength;
    ulLength = cpStream->Read(pBuffer, Amt);
    if(ulLength == Amt)
    {
        return BenErr_OK;
    }
    return BenErr_ReadPastEndOfContainer;
}
/**
*	Seek to position from the beginning of the bento file
*	@date	07/05/2004
*	@param	position in container file from beginning
*	@return	BenError
*/
BenError LtcBenContainer::SeekToPosition(BenContainerPos Pos)
{
    cpStream->Seek(Pos);
    return BenErr_OK;
}
/**
*	Seek to position compare to end of bento file
*	@date	07/05/2004
*	@param	position in container file from end
*	@return	BenError
*/
BenError LtcBenContainer::SeekFromEnd(long Offset)
{
    cpStream->Seek(STREAM_SEEK_TO_END);
    cpStream->SeekRel(Offset);

    return BenErr_OK;
}
/**
*	Get position in the bento file
*	@date	07/05/2004
*	@param	pointer of current position in container file from end
*	@return	BenError
*/
BenError LtcBenContainer::GetPosition(BenContainerPos * pPosition)
{
    *pPosition = cpStream->Tell();
    return BenErr_OK;
}
/**
*	Find the next value stream with property name
*	@date	07/05/2004
*	@param	string of property name
*	@param	current value stream pointer with the property name
*	@return	next value stream pointer with the property names
*/
LtcUtBenValueStream * LtcBenContainer::FindNextValueStreamWithPropertyName(const char * sPropertyName, LtcUtBenValueStream * pCurrentValueStream)
{
    CBenPropertyName * pPropertyName;
    RegisterPropertyName(sPropertyName, &pPropertyName);		// Get property name object

    if (NULL == pPropertyName)
        return NULL;											// Property not exist

    // Get current object
    CBenObject * pObj = NULL;
    if (pCurrentValueStream != NULL)
    {
        pObj = pCurrentValueStream->GetValue()->GetProperty()->GetBenObject();
    }


    pObj =FindNextObjectWithProperty(pObj, pPropertyName->GetID());	// Get next object with same property name
    if (NULL == pObj)
        return NULL;

    CBenValue * pValue;
    LtcUtBenValueStream * pValueStream;

    pValue = pObj->UseValue(pPropertyName->GetID());

    pValueStream = new LtcUtBenValueStream(pValue);

    return pValueStream;
}

/**
*	Find the unique value stream with property name
*	@date	07/05/2004
*	@param	string of property name
*	@return	the only value stream pointer with the property names
*/
LtcUtBenValueStream * LtcBenContainer::FindValueStreamWithPropertyName(const char * sPropertyName)
{
    return FindNextValueStreamWithPropertyName(sPropertyName, NULL);
}
/**
*	Find the unique value stream with property name and Object ID
*	@date	10/24/2005
*	@param	object ID
*	@param	string of property name
*	@return	the only value stream pointer with the property names
*/
LtcUtBenValueStream * LtcBenContainer::FindObjectValueStreamWithObjectIDAndProperty(BenObjectID ObjectID, const char * sPropertyName)
{
    CBenPropertyName * pPropertyName;
    RegisterPropertyName(sPropertyName, &pPropertyName);		// Get property name object
    if (NULL == pPropertyName)
        return NULL;											// Property not exist
    // Get current object
    CBenObject * pObj = NULL;
    pObj = FindObject(ObjectID); // Get object with object ID
    if (NULL == pObj)
        return NULL;
    CBenValue * pValue;
    LtcUtBenValueStream * pValueStream;
    pValue = pObj->UseValue(pPropertyName->GetID());
    pValueStream = new LtcUtBenValueStream(pValue);
    return pValueStream;
}
/**
*	<description>
*	@date	07/05/2004
*	@param	pointer to length of bento file
*	@return	BenError
*/
BenError LtcBenContainer::GetSize(ULONG * pLength)
{
    *pLength = m_ulLength;
    return BenErr_OK;
}

sal_uInt32 GetSvStreamSize(SvStream * pStream)
{
    sal_uInt32 nCurPos = pStream->Tell();
    pStream->Seek(STREAM_SEEK_TO_END);
    sal_uInt32 ulLength = pStream->Tell();
    pStream->Seek(nCurPos);

    return ulLength;
}
/**
*	Find hazily according to part of property name
*	@date	01/31/2005
*	@param
*	@return	the value stream pointers vector with the property names
*/
BenError LtcBenContainer::CreateGraphicStreams(std::vector<SvStream *> * pStreamVector)
{
    /* traverse all named objects to find all the graphic property names */
    CBenObject * pObj= NULL;
    char *pName;
    std::vector<std::string> aGrPropertyVector ;
    while( (pObj = GetNextObject(pObj)) )
    {
        if(pObj->IsNamedObject())
        {
            CBenNamedObject * pNamedObj = static_cast<CBenNamedObject *>(pObj);
            if(!pNamedObj->IsPropertyName())
                continue;

            // Gr2E,41FDD458-S
            // Gr2E,41FDD458-D
            pName = const_cast<char *>(pNamedObj->GetName());
            if ( (pName[0] == 'G')
            && (pName[1] == 'r')
            &&( (pName[14] == 'D') || (pName[14] == 'S') ) )
            {
                aGrPropertyVector.push_back(std::string(pName));
            }
        }
    }

    /* traverse the found properties and construct the stream vectors */
    std::vector<std::string>::iterator itor;
    for (itor = aGrPropertyVector.begin(); itor != aGrPropertyVector.end(); itor ++)
    {
        // get property names with suffix of S&D
        char sSName[32], sDName[32];
        strcpy(sSName, itor->c_str());
        strcpy(sDName, itor->c_str());
        sSName[14] = 'S';
        sDName[14] = 'D';

        // erase strings in vector
        std::vector<std::string>::iterator itorSearch;
        for (itorSearch = aGrPropertyVector.begin(); itorSearch != aGrPropertyVector.end(); itorSearch ++)
        {
            if( (*itorSearch == sSName) || (*itorSearch == sDName) )
            {
                std::vector<std::string>::iterator itorDel = itorSearch;
                aGrPropertyVector.erase(itorDel);
            }
        }

        // get S&D's stream and merge them together
        {
            SvStream *pD = NULL, *pS = NULL;

            pS = FindValueStreamWithPropertyName(sSName);
            assert(pS != NULL);
            pD = FindValueStreamWithPropertyName(sDName);
            assert(pD != NULL);

            sal_uInt32 nDLen = GetSvStreamSize(pD);
            sal_uInt32 nLen = GetSvStreamSize(pS) + nDLen;
            assert(nLen > 0);
            char * pBuf = new char[nLen];
            assert(pBuf != NULL);
            char * pPointer = pBuf;
            pD->Read(pPointer, nDLen);
            pPointer += nDLen;
            pS->Read(pPointer, nLen - nDLen);

            delete pS;
            delete pD;

            SvMemoryStream * pStream = new SvMemoryStream(pBuf, nLen, STREAM_READ);
            assert(pStream != NULL);

            pStreamVector->push_back(pStream);
        }
    }

    aGrPropertyVector.clear();
    return BenErr_OK;
}

/**
*	Find hazily according to object ID
*	@date	01/31/2005
*	@param	pObjectname - format as "GrXX,XXXXXXXX" wherein XX is high part of object ID, and XXXXXXXX is low part
*	@return	the value stream pointers  with the property names
*/
BenError LtcBenContainer::CreateGraphicStream(SvStream * &pStream, const char *pObjectName)
{
    if (!pObjectName)
    {
        pStream = NULL;
        return BenErr_NamedObjectError;
    }
    // construct the string of property name
    char sSName[64]="";
    char sDName[64]="";

    sprintf(sSName, "%s-S", pObjectName);
    sprintf(sDName, "%s-D", pObjectName);

    /* traverse the found properties and construct the stream vectors */
    SvMemoryStream * pMemStream = NULL;
    // get S&D's stream and merge them together
    SvStream *pD = NULL, *pS = NULL;

    pS = FindValueStreamWithPropertyName(sSName);
    pD = FindValueStreamWithPropertyName(sDName);


    sal_uInt32 nDLen = 0;
    if(pD)
    {
        nDLen = GetSvStreamSize(pD);
    }
    sal_uInt32 nLen = nDLen;
    if(pS)
    {
        nLen += GetSvStreamSize(pS) ;
    }

    assert(nLen > 0);
    char * pBuf = new char[nLen];
    assert(pBuf != NULL);
    char * pPointer = pBuf;
    if(pD)
    {
        pD->Read(pPointer, nDLen);
        delete pD;
    }
    pPointer += nDLen;
    if(pS)
    {
        pS->Read(pPointer, nLen - nDLen);
        delete pS;
    }

    pMemStream = new SvMemoryStream(pBuf, nLen, STREAM_READ);
    assert(pMemStream != NULL);

    pStream = pMemStream;
    return BenErr_OK;
}
/**
*	Find ole object stream according to object name
*	@date	10/24/2005
*	@param
*	@return	the value ole storage stream pointers
*/
#include <tools/globname.hxx>
SotStorageStreamRef LtcBenContainer::ConvertAswStorageToOLE2Stream(const char * sObjectName)
{
    SotStorageStreamRef xOleObjStm;

    SvStream* pOleStorageStream = NULL;
    //Get Ole Storage stream
    AswEntry aEntry;
    pOleStorageStream = FindOLEStorageStreamWithObjectName(sObjectName, aEntry);

    if( !pOleStorageStream )
        return xOleObjStm;
    //Find it, create Ole stream
    //xOleObjStm = new SotStorageStream(String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "Ole-Object" ) ));
    xOleObjStm = new SotStorageStream(String());
    if( xOleObjStm->GetError() )
        return xOleObjStm;
    xOleObjStm->SetBufferSize( 0xff00 );

    SotStorageRef xOleObjStor = new SotStorage( *xOleObjStm );
    if( xOleObjStor->GetError() )
        return xOleObjStm;
    //Set class id
    ClsId aClsId = aEntry.GetClassId();
    SvGlobalName aGName( aClsId.n1, aClsId.n2, aClsId.n3, aClsId.n4,
                        aClsId.n5, aClsId.n6,aClsId.n7,aClsId.n8,
                        aClsId.n9,aClsId.n10,aClsId.n11);
    xOleObjStor->SetClass( aGName, 0, String());

    xOleObjStor->SetVersion( SOFFICE_FILEFORMAT_60 );
    std::string aOleStreamName("OleStream");
    sal_uInt32 nDLen = 0;
    nDLen = GetSvStreamSize(pOleStorageStream);
    for(sal_uInt32 nIndex = 0; nIndex < nDLen / ASWENTRY_SIZE; nIndex ++)
    {
        AswEntry aOleEntry;
        ReadAswEntry(pOleStorageStream, aOleEntry);
        if(aOleEntry.GetType()== BEN_STGTY_STREAM)
        {
            SvStream* pOleStream = FindObjectValueStreamWithObjectIDAndProperty(aOleEntry.GetObjectID(),  aOleStreamName.c_str());
            if(pOleStream)
            {
                String strName;
                aOleEntry.GetName(strName);
                SotStorageStreamRef xStm = xOleObjStor->OpenSotStream( strName );
                if( xStm->GetError() )
                    break;
                xStm->SetBufferSize( 8192 );
                (*pOleStream) >> (*xStm);
                xStm->Commit();
                delete pOleStream;
                pOleStream = NULL;
            }
        }
    }
    xOleObjStor->Commit();
    xOleObjStm->Commit();

    return xOleObjStm;
}
/**
*	Find ole object storage stream data according to object name
*	@date	10/24/2005
*	@param
*	@return	the value ole storage stream data pointers
*/
LtcUtBenValueStream* LtcBenContainer::FindOLEStorageStreamWithObjectName(const char * sObjectName, AswEntry& rEntry)
{
    if (!sObjectName)
        return NULL;

    //Find OleRootStorage stream
    std::string aRootStroageName("OleRootStorage");
    std::string aOleStroageName("OleStorage");
    LtcUtBenValueStream* pRootStream = NULL;
    pRootStream = FindValueStreamWithPropertyName(aRootStroageName.c_str());
    if(!pRootStream)
        return NULL;
    //Read root storage data and find the ole storage object id according to the object name
    sal_uInt32 nDLen = 0;
    nDLen = GetSvStreamSize(pRootStream);
    for(sal_uInt32 nIndex = 0; nIndex < nDLen / ASWENTRY_SIZE; nIndex ++)
    {
        //Get entry
        ReadAswEntry(pRootStream, rEntry);
        String sObjName;
        rEntry.GetName(sObjName);
        if(sObjName.EqualsAscii(sObjectName))
        {
            //Find it
            delete pRootStream;
            return FindObjectValueStreamWithObjectIDAndProperty(rEntry.GetObjectID(),  aOleStroageName.c_str());
        }
    }

    // Not find
    if(pRootStream)
    {
        delete pRootStream;
    }

    return NULL;
}
void LtcBenContainer::ReadAswEntry(SvStream * pStream, AswEntry & rEntry)
{
    char* pBuf = new char[ASWENTRY_SIZE];
    pStream->Read(pBuf, ASWENTRY_SIZE);
    rEntry.Load(pBuf);
    delete[] pBuf;
}
////////////////////////////////////////////////////////////////////
//classs AswEntry
AswEntry::AswEntry()
{
    Init();
}
void AswEntry::Init()
{
    memset( this, 0, sizeof (AswEntry));
}
void  AswEntry::SetName( const String& rName )
{
    int i;
    for( i = 0; i < rName.Len() && i < 68; i++ )
        nName[ i ] = rName.GetChar( i );
    while( i < 68 )
        nName[ i++ ] = 0;
}
void AswEntry::GetName(String & rName) const
{
    rName =  nName;
}
void AswEntry::Store( void* pTo )
{
    SvMemoryStream r( (sal_Char *)pTo, ASWENTRY_SIZE, STREAM_WRITE );
    for( short i = 0; i < 68; i++ )
        r << nName[ i ];			// 00 name as WCHAR
     r<< nMtime[ 0 ] 						// 42 entry type
      << nMtime[ 1 ]						// 43 0 or 1 (tree balance?)
      << nCtime[ 0 ]						// 44 left node entry
      << nCtime[ 1 ]						// 48 right node entry
      << nAtime[ 0 ]						// 44 left node entry
      << nAtime[ 1 ];						// 48 right node entry
      r.Write(&aClsId ,16);						// 50 class ID (optional)
      r<< nStatebits						// 60 state flags(?)
      << nType
      << nObjectIDRef				// 64 modification time
      << nMversion				// 6C creation and access time
      << nLversion 				// 6C creation and access time
      << nReserved[ 0 ]						// 74 starting block (either direct or translated)
      << nReserved[ 1 ];						// 78 file size
}
void AswEntry::Load( const void* pFrom )
{
    SvMemoryStream r( (sal_Char *)pFrom, ASWENTRY_SIZE, STREAM_WRITE );
    for( short i = 0; i < 68; i++ )
        r >> nName[ i ];			// 00 name as WCHAR
     r>> nMtime[ 0 ] 						// 42 entry type
      >> nMtime[ 1 ]						// 43 0 or 1 (tree balance?)
      >> nCtime[ 0 ]						// 44 left node entry
      >> nCtime[ 1 ]						// 48 right node entry
      >> nAtime[ 0 ]						// 44 left node entry
      >> nAtime[ 1 ];						// 48 right node entry
      r.Read(&aClsId,16);						// 50 class ID (optional)
      r>> nStatebits						// 60 state flags(?)
      >> nType
      >> nObjectIDRef				// 64 modification time
      >> nMversion				// 6C creation and access time
      >> nLversion 				// 6C creation and access time
      >> nReserved[ 0 ]						// 74 starting block (either direct or translated)
      >> nReserved[ 1 ];						// 78 file size
}
void AswEntry::SetClassId( const ClsId& r )
{
    memcpy( &aClsId, &r, sizeof( ClsId ) );
}
///////////////////////////////////////////////////////////////////
}// end namespace OpenStormBento

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
