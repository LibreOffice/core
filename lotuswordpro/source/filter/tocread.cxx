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

#include <sal/config.h>
#include <sal/log.hxx>

#include <cstring>

#include "first.hxx"
#include <assert.h>
namespace OpenStormBento
{

BenError
CBenTOCReader::ReadLabelAndTOC()
{
    BenError Err;

    unsigned long TOCOffset;
    if ((Err = ReadLabel(&TOCOffset, &cTOCSize)) != BenErr_OK)
        return Err;

    sal_uLong nLength = cpContainer->GetSize();

    if (TOCOffset > nLength)
        return BenErr_ReadPastEndOfTOC;

    if (cTOCSize > nLength - TOCOffset)
        return BenErr_ReadPastEndOfTOC;

    cpContainer->SeekToPosition(TOCOffset);

    cpTOC.reset( new BenByte[cTOCSize] );
    if ((Err = cpContainer->ReadKnownSize(cpTOC.get(), cTOCSize)) != BenErr_OK)
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
    cpContainer->SeekFromEnd(-BEN_LABEL_SIZE);

    BenByte Label[BEN_LABEL_SIZE];
    if ((Err = cpContainer->ReadKnownSize(Label, BEN_LABEL_SIZE)) != BenErr_OK)
        return Err;

    if (memcmp(Label, gsBenMagicBytes, BEN_MAGIC_BYTES_SIZE) != 0)
        if ((Err = SearchForLabel(Label)) != BenErr_OK)
            return Err;

    BenByte * pCurrLabel = Label + BEN_MAGIC_BYTES_SIZE;

    BenWord Flags =
        UtGetIntelWord(pCurrLabel);
    pCurrLabel += 2; // Flags
    // Newer files are 0x0101--indicates if big or little endian.  Older
    // files are 0x0 for flags
    if (Flags != 0x0101 && Flags != 0x0)
        return BenErr_UnknownBentoFormatVersion;

    cBlockSize = UtGetIntelWord(pCurrLabel) * 1024; pCurrLabel += 2;
    if (cBlockSize == 0)
        return BenErr_NotBentoContainer;

    // Check major version
    if (UtGetIntelWord(pCurrLabel) != BEN_CURR_MAJOR_VERSION)
        return BenErr_UnknownBentoFormatVersion;
    pCurrLabel += 2;

    pCurrLabel += 2;    // Minor version

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

    sal_uLong Length = cpContainer->GetSize();

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

            cpContainer->SeekToPosition(CurrOffset - UsedBufferSize);

            if ((Err = cpContainer->ReadKnownSize(Buffer, UsedBufferSize)) !=
              BenErr_OK)
                return Err;

            BufferStartOffset = CurrOffset - UsedBufferSize;
        }

        if (memcmp(Buffer + (CurrOffset - BEN_MAGIC_BYTES_SIZE -
          BufferStartOffset), gsBenMagicBytes, BEN_MAGIC_BYTES_SIZE) == 0)
        {
            cpContainer->SeekToPosition(CurrOffset -
              BEN_MAGIC_BYTES_SIZE);

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
        CBenObject * pObject = nullptr;

        // Read in all properties for object
        do
        {
            BenObjectID PropertyID;

            if ((Err = GetDWord(&PropertyID)) != BenErr_OK)
                return Err;
            CBenProperty * pProperty = nullptr;

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

                    if (pObject != nullptr || TypeID != BEN_TYPEID_7_BIT_ASCII ||
                      LookAhead != BEN_OFFSET4_LEN4)
                        return BenErr_NamedObjectError;

                    BenContainerPos Pos;
                    sal_uInt32 Length;

                    if ((Err = GetDWord(&Pos)) != BenErr_OK)
                        return Err;
                    if ((Err = GetDWord(&Length)) != BenErr_OK)
                        return Err;
                    LookAhead = GetCode();

                    cpContainer->SeekToPosition(Pos);

                    const auto nRemainingSize = cpContainer->remainingSize();
                    if (Length > nRemainingSize)
                    {
                        SAL_WARN("lwp", "stream too short for claimed no of records");
                        Length = nRemainingSize;
                    }

                    #define STACK_BUFFER_SIZE 256
                    char sStackBuffer[STACK_BUFFER_SIZE];
                    std::unique_ptr<char[]> sAllocBuffer;
                    char * sBuffer;
                    if (Length > STACK_BUFFER_SIZE)
                    {
                        sAllocBuffer.reset(new char[Length]);
                        sBuffer = sAllocBuffer.get();
                    }
                    else
                    {
                        sBuffer = sStackBuffer;
                    }

                    if ((Err = cpContainer->ReadKnownSize(sBuffer, Length)) !=
                      BenErr_OK)
                    {
                        return Err;
                    }

                    OString sName;
                    if (Length)
                        sName = OString(sBuffer, Length - 1);

                    CUtListElmt * pPrevNamedObjectListElmt;
                    if (FindNamedObject(&cpContainer->GetNamedObjects(),
                      sName, &pPrevNamedObjectListElmt) != nullptr)
                    {
                        return BenErr_DuplicateName;
                    }

                    CUtListElmt* pPrevObject = cpContainer->GetObjects().GetLast();

                    if (PropertyID == BEN_PROPID_GLOBAL_PROPERTY_NAME)
                        pObject = new CBenPropertyName(cpContainer, ObjectID,
                           pPrevObject, sName, pPrevNamedObjectListElmt);
                    else
                        pObject = new CBenTypeName(cpContainer, ObjectID,
                           pPrevObject, sName, pPrevNamedObjectListElmt);
                }
                else if (PropertyID == BEN_PROPID_OBJ_REFERENCES)
                {
                    // Don't need to read in references object--we assume
                    // that all references use object ID as key
                    if ((Err = ReadSegments(nullptr, &LookAhead)) != BenErr_OK)
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
                        if ((Err = ReadSegments(nullptr, &LookAhead)) != BenErr_OK)
                            return Err;
                    }
                }
                else
                {
                    if (pProperty != nullptr)
                        return BenErr_PropertyWithMoreThanOneValue;

                    if (pObject == nullptr)
                        pObject = new CBenObject(cpContainer, ObjectID,
                          cpContainer->GetObjects().GetLast());

                    pProperty = new CBenProperty(pObject, PropertyID, TypeID,
                      pObject->GetProperties().GetLast());

                    if ((Err = ReadSegments(&pProperty->UseValue(),
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
CBenTOCReader::ReadSegments(CBenValue * pValue, BenByte * pLookAhead)
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
CBenTOCReader::ReadSegment(CBenValue * pValue, BenByte * pLookAhead)
{
    BenError Err;

    bool Immediate = false;
    bool EightByteOffset = false;
    sal_uInt32 Offset(0), Length(0);

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
            Length = 0; Immediate = true;
            break;

        case BEN_IMMEDIATE1:
            Length = 1; Immediate = true;
            break;

        case BEN_IMMEDIATE2:
            Length = 2; Immediate = true;
            break;

        case BEN_IMMEDIATE3:
            Length = 3; Immediate = true;
            break;

        case BEN_CONT_IMMEDIATE4:
        case BEN_IMMEDIATE4:
            Length = 4; Immediate = true;
            break;

        case BEN_CONT_OFFSET8_LEN4:
        case BEN_OFFSET8_LEN4:
            EightByteOffset = true;
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

    if (pValue != nullptr)
    {
        if (! Immediate)
            new CBenValueSegment(pValue, Offset, Length);
        else if (Length != 0)
        {
            assert(Length <= 4);
            new CBenValueSegment(pValue, ImmData, static_cast<unsigned short>(Length));
        }
    }

    return BenErr_OK;
}

bool
CBenTOCReader::CanGetData(unsigned long Amt)
{
    return cCurr + Amt <= cTOCSize;
}

BenError
CBenTOCReader::GetByte(BenByte * pByte)
{
    if (! CanGetData(1))
        return BenErr_ReadPastEndOfTOC;

    *pByte = UtGetIntelByte(cpTOC.get() + cCurr);
    ++cCurr;
    return BenErr_OK;
}

BenError
CBenTOCReader::GetDWord(BenDWord * pDWord)
{
    if (! CanGetData(4))
        return BenErr_ReadPastEndOfTOC;

    *pDWord = UtGetIntelDWord(cpTOC.get() + cCurr);
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
CBenTOCReader::GetData(void * pBuffer, unsigned long Amt)
{
    if (! CanGetData(Amt))
        return BenErr_ReadPastEndOfTOC;

    std::memcpy(pBuffer, cpTOC.get() + cCurr, Amt);
    cCurr += Amt;
    return BenErr_OK;
}
}//end OpenStormBento namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
