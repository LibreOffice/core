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
namespace OpenStormBento
{

BenError
CBenTOCReader::ReadLabelAndTOC()
{
    BenError Err;

    unsigned long TOCOffset;
    if ((Err = ReadLabel(&TOCOffset, &cTOCSize)) != BenErr_OK)
        return Err;

    if ((Err = cpContainer->SeekToPosition(TOCOffset)) != BenErr_OK)
        return Err;

    cpTOC = new BenByte[cTOCSize];
    if ((Err = cpContainer->ReadKnownSize(cpTOC, cTOCSize)) != BenErr_OK)
        return Err;

    if ((Err = ReadTOC()) != BenErr_OK)
        return Err;

    return BenErr_OK;
}

BenError
CBenTOCReader::ReadLabel(unsigned long * pTOCOffset, unsigned long * pTOCSize)
{
    // If seek fails, then probably because stream is smaller than
    // BEN_LABEL_SIZE and thus can't be Bento container
    BenError Err;
    if ((Err = cpContainer->SeekFromEnd(-BEN_LABEL_SIZE)) != BenErr_OK)
        return BenErr_NotBentoContainer;

    BenByte Label[BEN_LABEL_SIZE];
    if ((Err = cpContainer->ReadKnownSize(Label, BEN_LABEL_SIZE)) != BenErr_OK)
        return Err;

    if (memcmp(Label, gsBenMagicBytes, BEN_MAGIC_BYTES_SIZE) != 0)
        if ((Err = SearchForLabel(Label)) != BenErr_OK)
            return Err;

    BenByte * pCurrLabel = Label + BEN_MAGIC_BYTES_SIZE;

#ifndef NDEBUG
    BenWord Flags =
#endif
        UtGetIntelWord(pCurrLabel); pCurrLabel += 2; // Flags
    // Newer files are 0x0101--indicates if big or little endian.  Older
    // files are 0x0 for flags
    assert(Flags == 0x0101 || Flags == 0x0);

    cBlockSize = UtGetIntelWord(pCurrLabel) * 1024; pCurrLabel += 2;

    // Check major version
    if (UtGetIntelWord(pCurrLabel) != BEN_CURR_MAJOR_VERSION)
        return BenErr_UnknownBentoFormatVersion;
    pCurrLabel += 2;

    UtGetIntelWord(pCurrLabel); pCurrLabel += 2;    // Minor version

    *pTOCOffset = UtGetIntelDWord(pCurrLabel); pCurrLabel += 4;
    *pTOCSize = UtGetIntelDWord(pCurrLabel);

    assert(pCurrLabel + 4 == Label + BEN_LABEL_SIZE);

    return BenErr_OK;
}

#define LABEL_READ_BUFFER_SIZE 500
#define MAX_SEARCH_AMOUNT 1024 * 1024

BenError
CBenTOCReader::SearchForLabel(BenByte * pLabel)
{
    BenError Err;

    sal_uLong Length;
    if ((Err = cpContainer->GetSize(&Length)) != BenErr_OK)
        return Err;

    // Always ready to check for MagicBytes from
    // CurrOffset - BEN_MAGIC_BYTES_SIZE to CurrOffset - 1
    unsigned long CurrOffset = Length - BEN_LABEL_SIZE + BEN_MAGIC_BYTES_SIZE -
      1;

    char Buffer[LABEL_READ_BUFFER_SIZE] = {0};

    unsigned long BufferStartOffset = Length;   // Init to big value

    while (CurrOffset >= BEN_MAGIC_BYTES_SIZE)
    {
        // Don't search backwards more than 1 meg
        if (Length - CurrOffset > MAX_SEARCH_AMOUNT)
            break;


        // If before beginning of buffer
        if (CurrOffset - BEN_MAGIC_BYTES_SIZE < BufferStartOffset)
        {
            unsigned long UsedBufferSize;
            if (CurrOffset < LABEL_READ_BUFFER_SIZE)
                UsedBufferSize = CurrOffset;
            else UsedBufferSize = LABEL_READ_BUFFER_SIZE;

            if ((Err = cpContainer->SeekToPosition(CurrOffset - UsedBufferSize))
              != BenErr_OK)
                return Err;

            if ((Err = cpContainer->ReadKnownSize(Buffer, UsedBufferSize)) !=
              BenErr_OK)
                return Err;

            BufferStartOffset = CurrOffset - UsedBufferSize;
        }

        if (memcmp(Buffer + (CurrOffset - BEN_MAGIC_BYTES_SIZE -
          BufferStartOffset), gsBenMagicBytes, BEN_MAGIC_BYTES_SIZE) == 0)
        {
            if ((Err = cpContainer->SeekToPosition(CurrOffset -
              BEN_MAGIC_BYTES_SIZE)) != BenErr_OK)
                return Err;

            return cpContainer->ReadKnownSize(pLabel, BEN_LABEL_SIZE);
        }

        --CurrOffset;
    }

    return BenErr_NotBentoContainer;    // Didn't find magic bytes
}

BenError
CBenTOCReader::ReadTOC()
{
    BenError Err;
    BenByte LookAhead = GetCode();
    BenGeneration Generation = 0;

    // Read in all objects
    while (LookAhead == BEN_NEW_OBJECT)
    {
        BenObjectID ObjectID;
        if ((Err = GetDWord(&ObjectID)) != BenErr_OK)
            return Err;
        pCBenObject pObject = NULL;

        // Read in all properties for object
        do
        {
            BenObjectID PropertyID;

            if ((Err = GetDWord(&PropertyID)) != BenErr_OK)
                return Err;
            pCBenProperty pProperty = NULL;

            // Read in all values for property
            do
            {
                BenObjectID ReferencedListID = 0;

                BenObjectID TypeID;
                if ((Err = GetDWord(&TypeID)) != BenErr_OK)
                    return Err;
                LookAhead = GetCode();

                if (LookAhead == BEN_EXPLICIT_GEN)
                {
                    if ((Err = GetDWord(&Generation)) != BenErr_OK)
                        return Err;
                    LookAhead = GetCode();
                }

                if (LookAhead == BEN_REFERENCE_LIST_ID)
                {
                    if ((Err = GetDWord(&ReferencedListID)) != BenErr_OK)
                        return Err;
                    LookAhead = GetCode();
                }

                if (PropertyID == BEN_PROPID_GLOBAL_PROPERTY_NAME ||
                    PropertyID == BEN_PROPID_GLOBAL_TYPE_NAME)
                {
                    // Read property or type name

                    if (pObject != NULL || TypeID != BEN_TYPEID_7_BIT_ASCII ||
                      LookAhead != BEN_OFFSET4_LEN4)
                        return BenErr_NamedObjectError;

                    BenContainerPos Pos;
                    unsigned long Length;

                    if ((Err = GetDWord(&Pos)) != BenErr_OK)
                        return Err;
                    if ((Err = GetDWord(&Length)) != BenErr_OK)
                        return Err;
                    LookAhead = GetCode();

                    if ((Err = cpContainer->SeekToPosition(Pos)) != BenErr_OK)
                        return Err;

                    #define STACK_BUFFER_SIZE 256
                    char sStackBuffer[STACK_BUFFER_SIZE];
                    char * sAllocBuffer;
                    char * sBuffer;
                    if (Length > STACK_BUFFER_SIZE)
                    {
                        sBuffer = new char[Length];
                        sAllocBuffer = sBuffer;
                    }
                    else
                    {
                        sBuffer = sStackBuffer;
                        sAllocBuffer = NULL;
                    }

                    if ((Err = cpContainer->ReadKnownSize(sBuffer, Length)) !=
                      BenErr_OK)
                    {
                        delete[] sAllocBuffer;
                        return Err;
                    }

                    pCBenNamedObjectListElmt pPrevNamedObjectListElmt;
                    if (FindNamedObject(cpContainer->GetNamedObjects(),
                      sBuffer, &pPrevNamedObjectListElmt) != NULL)
                    {
                        delete[] sAllocBuffer;
                        return BenErr_DuplicateName;
                    }

                    pCBenObject pPrevObject = (pCBenObject) cpContainer->
                      GetObjects()->GetLast();

                    if (PropertyID == BEN_PROPID_GLOBAL_PROPERTY_NAME)
                        pObject = new CBenPropertyName(cpContainer, ObjectID,
                          pPrevObject, sBuffer, pPrevNamedObjectListElmt);
                    else pObject = new CBenTypeName(cpContainer, ObjectID,
                      pPrevObject, sBuffer, pPrevNamedObjectListElmt);

                    delete[] sAllocBuffer;
                }
                else if (PropertyID == BEN_PROPID_OBJ_REFERENCES)
                {
                    // Don't need to read in references object--we assume
                    // that all references use object ID as key
                    if ((Err = ReadSegments(NULL, &LookAhead)) != BenErr_OK)
                        return Err;
                }
                else if (ObjectID == BEN_OBJID_TOC)
                {
                    if (PropertyID == BEN_PROPID_TOC_SEED)
                    {
                        if (TypeID != BEN_TYPEID_TOC_TYPE ||
                          LookAhead !=  BEN_IMMEDIATE4)
                            return BenErr_TOCSeedError;

                        BenDWord Data;
                        if ((Err = GetDWord(&Data)) != BenErr_OK)
                            return Err;
                        LookAhead = GetCode();

                        cpContainer->SetNextAvailObjectID(Data);
                    }
                    else
                    {
                        // Ignore the other BEN_OBJID_TOC properties
                        if ((Err = ReadSegments(NULL, &LookAhead)) != BenErr_OK)
                            return Err;
                    }
                }
                else
                {
                    if (pProperty != NULL)
                        return BenErr_PropertyWithMoreThanOneValue;

                    if (pObject == NULL)
                        pObject = new CBenObject(cpContainer, ObjectID,
                          (pCBenObject) cpContainer->GetObjects()->GetLast());

                    pProperty = new CBenProperty(pObject, PropertyID, TypeID,
                      (pCBenProperty) pObject->GetProperties()->GetLast());

                    if ((Err = ReadSegments(pProperty->UseValue(),
                      &LookAhead)) != BenErr_OK)
                        return Err;
                }
            } while (LookAhead == BEN_NEW_TYPE);
        } while (LookAhead == BEN_NEW_PROPERTY);
    }

    if (LookAhead == BEN_READ_PAST_END_OF_TOC)
        return BenErr_OK;
    else return BenErr_InvalidTOC;
}

BenError
CBenTOCReader::ReadSegments(pCBenValue pValue, BenByte * pLookAhead)
{
    BenError Err;

    while (*pLookAhead >= BEN_SEGMENT_CODE_START &&
      *pLookAhead <= BEN_SEGMENT_CODE_END)
    {
        if ((Err = ReadSegment(pValue, pLookAhead)) !=
          BenErr_OK)
            return Err;
    }

    return BenErr_OK;
}

BenError
CBenTOCReader::ReadSegment(pCBenValue pValue, BenByte * pLookAhead)
{
    BenError Err;

    UtBool Immediate = UT_FALSE;
    UtBool EightByteOffset = UT_FALSE;
    unsigned long Offset(0), Length(0);

    switch (*pLookAhead)
    {
        case BEN_CONT_OFFSET4_LEN4:
        case BEN_OFFSET4_LEN4:
            if ((Err = GetDWord(&Offset))  != BenErr_OK)
                return Err;
            if ((Err = GetDWord(&Length))  != BenErr_OK)
                return Err;
            break;

        case BEN_IMMEDIATE0:
            Length = 0; Immediate = UT_TRUE;
            break;

        case BEN_IMMEDIATE1:
            Length = 1; Immediate = UT_TRUE;
            break;

        case BEN_IMMEDIATE2:
            Length = 2; Immediate = UT_TRUE;
            break;

        case BEN_IMMEDIATE3:
            Length = 3; Immediate = UT_TRUE;
            break;

        case BEN_CONT_IMMEDIATE4:
        case BEN_IMMEDIATE4:
            Length = 4; Immediate = UT_TRUE;
            break;

        case BEN_CONT_OFFSET8_LEN4:
        case BEN_OFFSET8_LEN4:
            EightByteOffset = UT_TRUE;
            break;

        default:
            return BenErr_OK;
    }

    BenByte ImmData[4];
    if (Immediate && Length != 0)
        if ((Err = GetData(ImmData, 4)) != BenErr_OK)
            return Err;

    *pLookAhead = GetCode();

    if (EightByteOffset)
        return BenErr_64BitOffsetNotSupported;

    if (pValue != NULL)
    {
        if (! Immediate)
            new CBenValueSegment(pValue, Offset, Length);
        else if (Length != 0)
        {
            assert(Length <= 4);
            new CBenValueSegment(pValue, ImmData, (unsigned short) Length);
        }
    }

    return BenErr_OK;
}

UtBool
CBenTOCReader::CanGetData(unsigned long Amt)
{
    return cCurr + Amt <= cTOCSize;
}

BenError
CBenTOCReader::GetByte(BenByte * pByte)
{
    if (! CanGetData(1))
        return BenErr_ReadPastEndOfTOC;

    *pByte = UtGetIntelByte(cpTOC + cCurr);
    ++cCurr;
    return BenErr_OK;
}

BenError
CBenTOCReader::GetDWord(BenDWord * pDWord)
{
    if (! CanGetData(4))
        return BenErr_ReadPastEndOfTOC;

    *pDWord = UtGetIntelDWord(cpTOC + cCurr);
    cCurr += 4;
    return BenErr_OK;
}

BenByte
CBenTOCReader::GetCode()
{
    BenByte Code;
    do
    {
        if (GetByte(&Code) != BenErr_OK)
            return BEN_READ_PAST_END_OF_TOC;

        if (Code == BEN_END_OF_BUFFER)
            // Advance to next block
            cCurr = cBlockSize * ((cCurr + (cBlockSize - 1)) /
              cBlockSize);
    }
    while (Code == BEN_NOOP || Code == BEN_END_OF_BUFFER);
    return Code;
}

BenError
CBenTOCReader::GetData(BenDataPtr pBuffer, unsigned long Amt)
{
    if (! CanGetData(Amt))
        return BenErr_ReadPastEndOfTOC;

    UtHugeMemcpy(pBuffer, cpTOC + cCurr, Amt);
    cCurr += Amt;
    return BenErr_OK;
}
}//end OpenStormBento namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
