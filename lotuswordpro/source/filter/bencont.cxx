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
#include <osl/diagnose.h>
#include <sot/storinfo.hxx>
namespace OpenStormBento
{

// String definitions
const char gsBenMagicBytes[] = BEN_MAGIC_BYTES;

/**
*   New bento container from file stream
*   @param  pointer to length of bento file
*   @param  pointer to pointer of Bento Container object
*   @return error code
*/
sal_uLong BenOpenContainer(LwpSvStream * pStream, pLtcBenContainer * ppContainer)
{
    BenError Err;

    *ppContainer = nullptr;

    if (nullptr == pStream)
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

LtcBenContainer::~LtcBenContainer()
{
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

BenError
LtcBenContainer::RegisterPropertyName(const char * sPropertyName,
  pCBenPropertyName * ppPropertyName)
{
    pCUtListElmt pPrevNamedObjectListElmt;
    pCBenNamedObject pNamedObject = FindNamedObject(&cNamedObjects,
      sPropertyName, &pPrevNamedObjectListElmt);

    if (pNamedObject != nullptr)
    {
        if (! pNamedObject->IsPropertyName())
            return BenErr_NameConflict;
        else *ppPropertyName = static_cast<pCBenPropertyName>(pNamedObject);
    }
    else
    {
        pCUtListElmt pPrevObject;
        if (FindID(&cObjects, cNextAvailObjectID, &pPrevObject) != nullptr)
            return BenErr_DuplicateObjectID;

        *ppPropertyName = new CBenPropertyName(this, cNextAvailObjectID,
          static_cast<pCBenObject>(pPrevObject), sPropertyName, pPrevNamedObjectListElmt);
        ++cNextAvailObjectID;
    }

    return BenErr_OK;
}

pCBenObject
LtcBenContainer::GetNextObject(pCBenObject pCurrObject)
{
    return static_cast<pCBenObject>(cObjects.GetNextOrNULL(pCurrObject));
}

pCBenObject
LtcBenContainer::FindNextObjectWithProperty(pCBenObject pCurrObject,
  BenObjectID PropertyID)
{
    while ((pCurrObject = GetNextObject(pCurrObject)) != nullptr)
        if (pCurrObject->UseProperty(PropertyID) != nullptr)
            return pCurrObject;

    return nullptr;
}

/**
*   Construction
*   @param  Bento file stream pointer
*   @return
*/
LtcBenContainer::LtcBenContainer(LwpSvStream * pStream)
    : cNextAvailObjectID(0)
{
    cpStream = pStream;
    pStream->Seek(STREAM_SEEK_TO_END);
    m_ulLength = pStream->Tell();
    pStream->Seek(STREAM_SEEK_TO_BEGIN);
}

/**
*   Read buffer fro bento file with specified buffer
*   @param  buffer pointer
*   @param  buffer size
*   @param  number of bytes read
*   @return BenError
*/
BenError LtcBenContainer::Read(void * pBuffer, unsigned long MaxSize,
  unsigned long * pAmtRead)
{
    *pAmtRead = cpStream->Read(pBuffer, MaxSize);
    return BenErr_OK;
}
/**
*   Read buffer from bento file with specified size
*   @param  buffer pointer
*   @param  number of bytes to be read
*   @return BenError
*/
BenError LtcBenContainer::ReadKnownSize(void * pBuffer, unsigned long Amt)
{
    sal_uLong ulLength;
    ulLength = cpStream->Read(pBuffer, Amt);
    if(ulLength == Amt)
    {
        return BenErr_OK;
    }
    return BenErr_ReadPastEndOfContainer;
}
/**
*   Seek to position from the beginning of the bento file
*   @param  position in container file from beginning
*   @return BenError
*/
BenError LtcBenContainer::SeekToPosition(BenContainerPos Pos)
{
    cpStream->Seek(Pos);
    return BenErr_OK;
}
/**
*   Seek to position compare to end of bento file
*   @param  position in container file from end
*   @return BenError
*/
BenError LtcBenContainer::SeekFromEnd(long Offset)
{
    cpStream->Seek(STREAM_SEEK_TO_END);
    cpStream->SeekRel(Offset);

    return BenErr_OK;
}
/**
*   Find the next value stream with property name
*   @param  string of property name
*   @param  current value stream pointer with the property name
*   @return next value stream pointer with the property names
*/
LtcUtBenValueStream * LtcBenContainer::FindNextValueStreamWithPropertyName(const char * sPropertyName, LtcUtBenValueStream * pCurrentValueStream)
{
    CBenPropertyName * pPropertyName;
    RegisterPropertyName(sPropertyName, &pPropertyName);        // Get property name object

    if (nullptr == pPropertyName)
        return nullptr;                                            // Property not exist

    // Get current object
    CBenObject * pObj = nullptr;
    if (pCurrentValueStream != nullptr)
    {
        pObj = pCurrentValueStream->GetValue()->GetProperty()->GetBenObject();
    }

    pObj =FindNextObjectWithProperty(pObj, pPropertyName->GetID()); // Get next object with same property name
    if (nullptr == pObj)
        return nullptr;

    CBenValue * pValue;
    LtcUtBenValueStream * pValueStream;

    pValue = pObj->UseValue(pPropertyName->GetID());

    pValueStream = new LtcUtBenValueStream(pValue);

    return pValueStream;
}

/**
*   Find the unique value stream with property name
*   @param  string of property name
*   @return the only value stream pointer with the property names
*/
LtcUtBenValueStream * LtcBenContainer::FindValueStreamWithPropertyName(const char * sPropertyName)
{
    return FindNextValueStreamWithPropertyName(sPropertyName, nullptr);
}
/**
*   <description>
*   @param  pointer to length of bento file
*   @return BenError
*/
BenError LtcBenContainer::GetSize(sal_uLong * pLength)
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
*   Find hazily according to object ID
*   @param  pObjectname - format as "GrXX,XXXXXXXX" wherein XX is high part of object ID, and XXXXXXXX is low part
*   @return the value stream pointers  with the property names
*/
BenError LtcBenContainer::CreateGraphicStream(SvStream * &pStream, const char *pObjectName)
{
    if (!pObjectName)
    {
        pStream = nullptr;
        return BenErr_NamedObjectError;
    }
    // construct the string of property name
    char sSName[64]="";
    char sDName[64]="";

    sprintf(sSName, "%s-S", pObjectName);
    sprintf(sDName, "%s-D", pObjectName);

    /* traverse the found properties and construct the stream vectors */
    SvMemoryStream * pMemStream = nullptr;
    // get S&D's stream and merge them together
    SvStream *pD = nullptr, *pS = nullptr;

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

    OSL_ENSURE(nLen > 0, "expected a non-0 length");
    // the 'D' stream is NULL or it has invalid length
    if (nLen <= 0)
    {
        pStream = nullptr;
        return BenErr_NamedObjectError;
    }

    char * pBuf = new char[nLen];
    assert(pBuf != nullptr);
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

    pMemStream = new SvMemoryStream(pBuf, nLen, StreamMode::READ);
    assert(pMemStream != nullptr);

    pStream = pMemStream;
    return BenErr_OK;
}

}// end namespace OpenStormBento

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
