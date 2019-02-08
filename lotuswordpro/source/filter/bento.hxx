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
#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_BENTO_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_BENTO_HXX

#include <sal/config.h>

#include <cstring>
#include <memory>
#include <string>
#include <vector>
#include <lwpsvstream.hxx>

#define BEN_CC __stdcall
#ifdef COMP_BENTO
#define BEN_EXPORT __declspec(dllexport)
#else
#define BEN_EXPORT
#endif

#include "ut.hxx"
#include "utlist.hxx"
#include <tools/stream.hxx>
#include <sot/storage.hxx>

namespace OpenStormBento
{

#define BEN_CURR_MAJOR_VERSION 2
#define BEN_CURR_MINOR_VERSION 0
#define BEN_MAGIC_BYTES_SIZE 8
#define BEN_LABEL_SIZE 24

#define BEN_MAGIC_BYTES "\xA4""CM""\xA5""Hdr""\xD7"

enum BenError
{
    BenErr_OK = 0,
    BenErr_NameConflict = 1,
    BenErr_DuplicateObjectID = 2,
    BenErr_UnknownBentoFormatVersion = 3,
    BenErr_NamedObjectError = 4,
    BenErr_NamedObjectIllegalValue = 5,
    BenErr_InvalidTOC = 6,
    BenErr_64BitOffsetNotSupported = 7,
    BenErr_ReadPastEndOfTOC = 8,
    BenErr_ContainerWithNoObjects = 9,
    BenErr_ObjectWithNoProperties = 10,
    BenErr_PropertyWithNoValues = 11,
    BenErr_IllegalInMemoryTOC = 12,
    BenErr_PropertyAlreadyExists = 13,
    BenErr_UnexpectedEndOfFile = 14,
    BenErr_InvalidWriteOffset = 15,
    BenErr_InvalidImmediateWrite = 16,
    BenErr_TOCSeedError = 17,
    BenErr_ReadPastEndOfContainer = 18,
    BenErr_DuplicateName = 19,
    BenErr_BadReferencedList = 20,
    BenErr_IllegalContinuedImmediate = 21,
    BenErr_NotBentoContainer = 22,
    BenErr_PropertyWithMoreThanOneValue = 23

    // IMPORTANT - UtStream errors (UtErr_....) are also valid Bento
    // errors.  They have codes of 100 and higher.  When, say, a Bento
    // container open fails due to an access violation (somebody else, say,
    // has it open exclusive), will get a UtErr code.  If define own
    // subclass of UtStream (custom handler), can define own error codes--
    // those should start at 200
};

class LtcBenContainer;
class CBenObject;
class CBenProperty;
class CBenValue;
class CBenValueSegment;
class CBenNamedObject;
class CBenPropertyName;

typedef sal_uInt8 BenByte;
typedef sal_uInt16 BenWord;
typedef sal_uInt32 BenDWord;

typedef sal_uInt32 BenContainerPos;
typedef sal_uInt32 BenObjectID;
typedef sal_uInt32 BenGeneration;

sal_uLong BenOpenContainer(LwpSvStream * pStream, std::unique_ptr<LtcBenContainer>* ppContainer);

class CBenIDListElmt : public CUtListElmt
{
public: // Internal methods
    CBenIDListElmt(BenObjectID ID, CUtListElmt * pPrev) : CUtListElmt(pPrev)
      { cID = ID; }
    explicit CBenIDListElmt(BenObjectID ID) { cID = ID; }
    BenObjectID GetID() { return cID; }

private: // Data
    BenObjectID cID;
};

class CBenNamedObjectListElmt : public CUtListElmt
{
public: // Methods
    // added to remove warning
    explicit CBenNamedObjectListElmt(CUtListElmt * pPrev) : CUtListElmt(pPrev)
      { cpNamedObject = nullptr; }
    void SetNamedObject(CBenNamedObject * pObj)
    {
        cpNamedObject = pObj;
    }

    CBenNamedObject * GetNamedObject() { return cpNamedObject; }

private: // Data
    CBenNamedObject * cpNamedObject;
};

class LtcUtBenValueStream : public SvStream
{
public:
    explicit LtcUtBenValueStream(CBenValue * pValue);
    virtual ~LtcUtBenValueStream() override;

public: // Overridden methods

    /* added by  */
    sal_uLong GetSize() { return m_ulValueLength; };
protected: // Overridden methods

    virtual std::size_t GetData(void* pData, std::size_t nSize) override;
    virtual std::size_t PutData(const void* pData, std::size_t nSize) override;
    virtual sal_uInt64   SeekPos( sal_uInt64 nPos ) override;
    virtual void    SetSize( sal_uInt64 nSize ) override;
    virtual void    FlushData() override;

private: // Data
    CBenValue * cpValue;
    size_t cCurrentPosition;

    sal_uLong m_ulValueLength;      // Added by , sum of length of all sub-valuesegments
};

class LtcBenContainer
{
public:
    BenError Open();
    void RegisterPropertyName(const char * sPropertyName,
      CBenPropertyName ** ppPropertyName);
    // Pass NULL to begin iteration.  Done when returns NULL.
    // Objects are returned in order of increasing ID
    CBenObject * GetNextObject(CBenObject const * pCurrObject);
    CBenObject * FindNextObjectWithProperty(CBenObject * pCurrObject,
      BenObjectID PropertyID);

public: // Internal methods
    explicit LtcBenContainer(LwpSvStream * pStream);
    ~LtcBenContainer();

    sal_uLong remainingSize() const;
    void Read(void * pBuffer, size_t MaxSize,
      size_t* pAmtRead);
    BenError ReadKnownSize(void * pBuffer, size_t Amt);
    void SeekToPosition(BenContainerPos Pos);
    void SeekFromEnd(long Offset);

    void SetNextAvailObjectID(BenObjectID ID) { cNextAvailObjectID = ID; }
    CUtList& GetObjects() { return cObjects; }
    CUtList& GetNamedObjects() { return cNamedObjects; }

    LtcUtBenValueStream * FindNextValueStreamWithPropertyName(const char * sPropertyName);
    LtcUtBenValueStream * FindValueStreamWithPropertyName(const char * sPropertyName);
    std::vector<sal_uInt8> GetGraphicData(const char *pObjectName);

    sal_uLong GetSize() const { return m_ulLength; }
private: // Data
    CUtOwningList cObjects;
    CUtList cNamedObjects;
    LwpSvStream * cpStream;
    sal_uLong m_ulLength;   // Added for TOCRead.cpp
    BenObjectID cNextAvailObjectID; // for new object
};

class CBenObject : public CBenIDListElmt
{
public:
    CBenProperty * UseProperty(BenObjectID PropertyID);
    CBenValue * UseValue(BenObjectID PropertyID);
    LtcBenContainer * GetContainer() { return cpContainer; }
public: // Internal methods
    CBenObject(LtcBenContainer * pContainer, BenObjectID ObjectID,
      CUtListElmt * pPrev) : CBenIDListElmt(ObjectID, pPrev)
      { cpContainer = pContainer; }
    CUtList& GetProperties() { return cProperties; }

private: // Data
    LtcBenContainer * cpContainer;
    CUtOwningList cProperties;
};

class CBenValue : public CBenIDListElmt
{
public:
    size_t GetValueSize();
    void ReadValueData(void * pBuffer,
      size_t Offset, size_t MaxSize, size_t* pAmtRead);

    CBenProperty * BEN_EXPORT GetProperty() { return cpProperty; }

public: // Internal methods
    // added to remove WARNING
    explicit CBenValue(BenObjectID TypeID):CBenIDListElmt(TypeID)
    {
        cpProperty = nullptr;
    }

    void SetProperty(CBenProperty * pProperty)
    {
        cpProperty = pProperty;
    }

    inline CBenValueSegment * GetNextValueSegment(CBenValueSegment const *
      pCurrValueSegment);
    CUtList& GetValueSegments() { return cValueSegments; }

private: // Data
    CBenProperty * cpProperty;
    CUtOwningList cValueSegments;
};

class CBenProperty : public CBenIDListElmt
{
public:
    // In the Apple Bento, a property can have multiple values, each of a
    // different type.  But we never use it that way, so in this code a
    // property has exactly one value

    CBenValue& UseValue() { return cValue; }
    CBenObject * GetBenObject() { return cpObject; }
    LtcBenContainer * GetContainer() { return GetBenObject()->GetContainer(); }

public: // Internal methods
    // changed to remove WARNING here
    CBenProperty(CBenObject * pObject, BenObjectID PropertyID,
      BenObjectID TypeID, CUtListElmt * pPrevProperty) :
      CBenIDListElmt(PropertyID, pPrevProperty), cValue(TypeID)
    {
        cpObject = pObject;
        cValue.SetProperty(this);
    }
private: // Data
    CBenObject * cpObject;
    CBenValue cValue;
};

class CBenValueSegment : public CUtListElmt
{
public: // Internal methods
    CBenValueSegment(CBenValue * pValue, BenContainerPos Pos,
      size_t Size) : CUtListElmt(&pValue->GetValueSegments())
      { cImmediate = false; cPos = Pos;
        cSize = Size; }
    CBenValueSegment(CBenValue * pValue, const void  * pImmData,
      unsigned short Size) : CUtListElmt(&pValue->GetValueSegments())
      { cImmediate = true;
        std::memcpy(cImmData, pImmData, Size); cSize = Size; }
    bool IsImmediate() { return cImmediate; }
    BenContainerPos GetPosition() { return cPos; }
    size_t GetSize() { return cSize; }
    BenByte * GetImmediateData() { return cImmData; }

private: // Data
    bool cImmediate;
    union
    {
        BenContainerPos cPos;
        BenByte cImmData[4];
    };
    size_t cSize;
};

inline CBenValueSegment * CBenValue::GetNextValueSegment(CBenValueSegment const *
  pCurrValueSegment)
{ return static_cast<CBenValueSegment *>( cValueSegments.GetNextOrNULL(pCurrValueSegment) ); }


class CBenNamedObject : public CBenObject
{
public: // Methods
    virtual bool IsPropertyName();

public: // Internal methods
    CBenNamedObject(LtcBenContainer * pContainer, BenObjectID ObjectID,
    CUtListElmt * pPrevObject, const OString& rName,
    CUtListElmt * pPrevNamedObjectListElmt);

    const OString& GetName() { return csName; }

private: // Data
    OString csName;
    CBenNamedObjectListElmt cNameListElmt;
};

class CBenPropertyName : public CBenNamedObject
{
public: // Internal methods
    CBenPropertyName(LtcBenContainer * pContainer, BenObjectID ObjectID,
    CUtListElmt * pPrevObject, const OString& rName,
    CUtListElmt * pPrevNamedObjectListElmt) :
    CBenNamedObject(pContainer, ObjectID, pPrevObject, rName,
    pPrevNamedObjectListElmt) { ; }
    virtual bool IsPropertyName() override;
};

class CBenTypeName : public CBenNamedObject
{
public: // Internal methods
    CBenTypeName(LtcBenContainer * pContainer, BenObjectID ObjectID,
    CUtListElmt * pPrevObject, const OString& rName,
    CUtListElmt * pPrevNamedObjectListElmt) :
    CBenNamedObject(pContainer, ObjectID, pPrevObject, rName,
    pPrevNamedObjectListElmt) { ; }
};

} // end namespace OpenStormBento
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
