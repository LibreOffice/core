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
#ifndef BENTO_H
#define BENTO_H

#include <string>
#include	<vector>
#include "lwpsvstream.hxx"

#define BEN_CC __stdcall
#ifdef COMP_BENTO
#define BEN_EXPORT __declspec(dllexport)
#else
#define BEN_EXPORT
#endif

using namespace std;

#include "ut.hxx"
#include "utlist.hxx"
#include <tools/stream.hxx>	// SvStream definition
#include <sot/storage.hxx>	// SotStorageStream definition, add by  10/24/2005

namespace OpenStormBento
{

#define BEN_CURR_MAJOR_VERSION 2
#define BEN_CURR_MINOR_VERSION 0
#define BEN_MAGIC_BYTES_SIZE 8
#define BEN_LABEL_SIZE 24

#define BEN_MAGIC_BYTES "\xA4""CM""\xA5""Hdr""\xD7"
//For Ole2DirectoryStruct, Add by  10/24/2005
#define BEN_STGTY_STORAGE    1
#define BEN_STGTY_STREAM   2
#define ASWENTRY_SIZE	204
struct ClsId
{
    INT32	n1;
    INT16	n2, n3;
    UINT8	n4, n5, n6, n7, n8, n9, n10, n11;
};
class AswEntry	//total length: 204
{
    UINT16	nName[ 68 ];		//Name of IStorage or IStream referenced by this entry, length = 136
    INT32	nMtime[ 2 ];
    INT32	nCtime[ 2 ];
    INT32	nAtime[ 2 ];
    ClsId	aClsId;			//CLSID from OLE 2 IStorage::SetClass call
    UINT32	nStatebits;		//State bits from OLE 2 IStorage::SetStateBits call
    UINT32	nType;			// STGTY_STORAGE: 1 or STGTY_STREAM:2,
    UINT32	nObjectIDRef;		//Persistent Bento reference to Bento object for this IStorage or IStream
    UINT32	nMversion;
    UINT32	nLversion;
    UINT32	nReserved[2];		//skip 16 char
public:
    AswEntry();
    void	Init();						// initialize the data
    void SetName( const String& );	// store a name (ASCII, up to 32 chars)
    void	GetName( String& rName ) const;
    void	Load( const void* );
    void Store( void* );
    UINT32 GetType() const		{ return  nType;	}
    void	SetType( UINT32 t )	{ nType = t;}
    const   ClsId& GetClassId() const  	{ return aClsId;}
    void	SetClassId( const ClsId& );
    void SetObjectID(UINT32 id) { nObjectIDRef = id;}
    UINT32 GetObjectID() const { return nObjectIDRef;}
};
//End by

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
/*
 * These two functions are useless in SODC
 *
inline UtError BenToUtError(BenError Err)
{
    if (Err == 0 || Err >= 100)
        return (UtError) Err;
    else return UtErr_Fail;
}

inline BenError UtToBenError(UtError Err)
{ UT_ASSERT(Err == 0 || Err >= 100); return (BenError) Err; }
enum BenSeekMode {
    BenSeek_FromStart = 1,
    BenSeek_FromCurr = 2,
    BenSeek_FromEnd = 3
};
*/

UtDefClassP(LtcBenContainer);
UtDefClassP(CBenIDListElmt);
UtDefClassP(CBenObject);
UtDefClassP(CBenProperty);
UtDefClassP(CBenReference);
UtDefClassP(CBenValue);
UtDefClassP(CBenValueSegment);
UtDefClassP(CBenNamedObjectListElmt);
UtDefClassP(CBenNamedObject);
UtDefClassP(CBenPropertyName);
UtDefClassP(CBenTypeName);

typedef unsigned char BenByte;
typedef unsigned short BenWord;
typedef unsigned long BenDWord;
typedef void * BenDataPtr;
typedef const void  * BenConstDataPtr;

typedef unsigned long BenContainerPos;
typedef unsigned long BenObjectID;
typedef unsigned long BenGeneration;

ULONG BenOpenContainer(LwpSvStream * pStream, pLtcBenContainer * ppContainer);

class CBenIDListElmt : public CUtListElmt
{
public: // Internal methods
    CBenIDListElmt(BenObjectID ID, pCBenIDListElmt pPrev) : CUtListElmt(pPrev)
      { cID = ID; }
    CBenIDListElmt(BenObjectID ID) { cID = ID; }
    BenObjectID GetID() { return cID; }

private: // Data
    BenObjectID cID;
};

class CBenNamedObjectListElmt : public CUtListElmt
{
public: // Methods
    // added by  to remove warning 2004-06-24
    CBenNamedObjectListElmt(pCBenNamedObjectListElmt pPrev) : CUtListElmt(pPrev)
      { cpNamedObject = NULL; }
    void SetNamedObject(pCBenNamedObject pObj)
    {
        cpNamedObject = pObj;
    }

    CBenNamedObjectListElmt(pCBenNamedObject pNamedObject,
      pCBenNamedObjectListElmt pPrev) : CUtListElmt(pPrev)
      { cpNamedObject = pNamedObject; }
    pCBenNamedObject GetNamedObject() { return cpNamedObject; }

private: // Data
    pCBenNamedObject cpNamedObject;
};

class LtcUtBenValueStream : public SvStream
{
public:
    LtcUtBenValueStream(pCBenValue pValue);
    ~LtcUtBenValueStream();

public: // Overridden methods

    /* added by  */
    CBenValue * GetValue(){ return cpValue; };
    ULONG GetSize() { return m_ulValueLength; };
protected: // Overridden methods

    virtual ULONG	GetData( void* pData, ULONG nSize );
    virtual ULONG	PutData( const void* pData, ULONG nSize );
    virtual ULONG	SeekPos( ULONG nPos );
    virtual void	SetSize( ULONG nSize );
    virtual void	FlushData();

private: // Data
    pCBenValue cpValue;
    unsigned long cCurrentPosition;

    ULONG m_ulValueLength;		// Added by , sum of length of all sub-valuesegments
//	void	GetAmountLeft(ULONG * pAmtLeft);	useless in SODC
};

class LtcBenContainer
{
public:
    BenError Open();
    void Release(); // Deletes container object--last call
    BenError Close();
    BenError RegisterTypeName(const char * sTypeName,
      pCBenTypeName * ppTypeName);
    BenError RegisterPropertyName(const char * sPropertyName,
      pCBenPropertyName * ppPropertyName);
    // Pass NULL to begin iteration.  Done when returns NULL.
    // Objects are returned in order of increasing ID
    pCBenObject GetNextObject(pCBenObject pCurrObject);
    pCBenObject FindObject(BenObjectID ObjectID);
    pCBenObject FindNextObjectWithProperty(pCBenObject pCurrObject,
      BenObjectID PropertyID);
    BenError BEN_EXPORT NewObject(pCBenObject * ppBenObject);

public: // Internal methods
    LtcBenContainer(LwpSvStream * pStream);
    ~LtcBenContainer();

    BenError Read(BenDataPtr pBuffer, unsigned long MaxSize,
      unsigned long * pAmtRead);
    BenError ReadKnownSize(BenDataPtr pBuffer, unsigned long Amt);
    BenError SeekToPosition(BenContainerPos Pos);
    BenError SeekFromEnd(long Offset);

    BenError GetPosition(BenContainerPos * pPosition);

    BenObjectID GetNextAvailObjectID() { return cNextAvailObjectID; }
    void SetNextAvailObjectID(BenObjectID ID) { cNextAvailObjectID = ID; }
    pCUtList GetObjects() { return &cObjects; }
    pCUtList GetNamedObjects() { return &cNamedObjects; }

    // Added by  2004-06-10
    LtcUtBenValueStream * FindNextValueStreamWithPropertyName(const char * sPropertyName, LtcUtBenValueStream * pCurrentValueStream);
    LtcUtBenValueStream * FindValueStreamWithPropertyName(const char * sPropertyName);
    LtcUtBenValueStream * FindObjectValueStreamWithObjectIDAndProperty(BenObjectID ObjectID, const char * sPropertyName);
    BenError CreateGraphicStreams(std::vector<SvStream *> * pStreamVector)	;
    BenError CreateGraphicStream(SvStream * &pStream,  const char *pObjectName);
    //Add by  10/24/2005
    SotStorageStreamRef ConvertAswStorageToOLE2Stream(const char * sObjectName);
    LtcUtBenValueStream * FindOLEStorageStreamWithObjectName(const char * sObjectName, AswEntry& rDirStruct);
    void ReadAswEntry(SvStream * pStream, AswEntry& rEntry);

    BenError GetSize(ULONG * pLength);
    LwpSvStream * GetStream()
    {
        return cpStream;
    };
private: // Data
    CUtOwningList cObjects;
    CUtList cNamedObjects;
    LwpSvStream * cpStream;
    ULONG m_ulLength; 	// Added for TOCRead.cpp
    BenObjectID cNextAvailObjectID; // for new object
};

class CBenObject : public CBenIDListElmt
{
public:
    pCBenProperty GetNextProperty(pCBenProperty pCurrProperty);
    pCBenProperty UseProperty(BenObjectID PropertyID);
    void DeleteProperty(pCBenProperty pProperty);
    pCBenValue UseValue(BenObjectID PropertyID);
    pCBenValue UseSingleValue();
    // Inefficient to use this method if will use property multiple times--
    // instead register property and call UseProperty with property ID
    pCBenValue UseValueWithPropertyName(const char * sPropertyName);
    virtual UtBool IsNamedObject();
    pLtcBenContainer GetContainer() { return cpContainer; }
    BenObjectID GetObjectID() { return GetID(); }
    BenError BEN_EXPORT NewValue(BenObjectID PropertyID, BenObjectID TypeID,
      pCBenValue * ppValue);
public: // Internal methods
    CBenObject(pLtcBenContainer pContainer, BenObjectID ObjectID,
      pCBenIDListElmt pPrev) : CBenIDListElmt(ObjectID, pPrev)
      { cpContainer = pContainer; }
    pCUtList GetProperties() { return &cProperties; }

private: // Data
    pLtcBenContainer cpContainer;
    CUtOwningList cProperties;
};

class CBenValue : public CBenIDListElmt
{
public:
    unsigned long GetValueSize();
    BenError ReadValueData(BenDataPtr pBuffer,
      unsigned long Offset, unsigned long MaxSize, unsigned long * pAmtRead);
    BenError ReadValueDataKnownSize(BenDataPtr pBuffer,
      unsigned long Offset, unsigned long Amt);

    BenError BEN_EXPORT WriteValueData(BenConstDataPtr pBuffer,
      unsigned long Offset, unsigned long Size);
    BenError BEN_EXPORT WriteValueData(BenConstDataPtr pBuffer,
      unsigned long Offset, unsigned long Size, unsigned long * pAmtWritten);
    BenError BEN_EXPORT WriteImmediateValueData(BenConstDataPtr pBuffer,
      unsigned short Size);
    BenError BEN_EXPORT TruncateValueSize(unsigned long NewSize);
    BenError BEN_EXPORT NewReference(BenObjectID ReferencedObjectID,
      pCBenReference pReference);
    BenObjectID BEN_EXPORT GetReferencedObject(pCBenReference pReference);


    pCBenProperty BEN_EXPORT GetProperty() { return cpProperty; }

public: // Internal methods
    // added by  to remove WARNING 2004-06-24
    CBenValue(BenObjectID TypeID):CBenIDListElmt(TypeID)
    {
        cpProperty = NULL;
        cpReferencedList = NULL;
    }

    void SetProperty(pCBenProperty pProperty)
    {
        cpProperty = pProperty;
    }

    CBenValue(pCBenProperty pProperty, BenObjectID TypeID) :
      CBenIDListElmt(TypeID)
    {
        cpProperty = pProperty;
        cpReferencedList = NULL;
    }
    BenObjectID GetTypeNameID() { return GetID(); }
    inline pCBenValueSegment GetNextValueSegment(pCBenValueSegment
      pCurrValueSegment);
    inline pLtcBenContainer GetContainer();
    pCUtList GetValueSegments() { return &cValueSegments; }
    BenObjectID GetReferencedListID();
    // Currently, no generation support
    BenGeneration GetGeneration() { return 1; }

private: // Data
    pCBenProperty cpProperty;
    CUtOwningList cValueSegments;
    pCBenValue cpReferencedList;
    unsigned long cReferencedObjectsSize;
};

class CBenProperty : public CBenIDListElmt
{
public:
    // In the Apple Bento, a property can have multiple values, each of a
    // different type.  But we never use it that way, so in this code a
    // property has exactly one value

    pCBenValue UseValue() { return &cValue; }
    pCBenObject GetBenObject() { return cpObject; }
    pLtcBenContainer GetContainer() { return GetBenObject()->GetContainer(); }

public: // Internal methods
    // changed by  to remove WARNING here 2004-06-24
    CBenProperty(pCBenObject pObject, BenObjectID PropertyID,
      BenObjectID TypeID, pCBenIDListElmt pPrevProperty) :
      CBenIDListElmt(PropertyID, pPrevProperty), cValue(TypeID)
    {
        cpObject = pObject;
        cValue.SetProperty(this);
    }

BenObjectID GetPropertyNameID() { return GetID(); }

private: // Data
    pCBenObject cpObject;
    CBenValue cValue;
};

// In our implementation, reference is always just the object ID for the
// object referenced (but you shouldn't assume that)
#define BEN_REFERENCE_SIZE 4

class CBenReference
{
public: // Methods
    BenByte * GetData() { return cData; }

private: // Data
    BenByte cData[BEN_REFERENCE_SIZE];
};

class CBenValueSegment : public CUtListElmt
{
public: // Internal methods
    CBenValueSegment(pCBenValue pValue, BenContainerPos Pos,
      unsigned long Size) : CUtListElmt(pValue->GetValueSegments())
      { cpValue = pValue; cImmediate = UT_FALSE; cPos = Pos;
      cSize = Size; }
    CBenValueSegment(pCBenValue pValue, BenConstDataPtr pImmData,
      unsigned short Size) : CUtListElmt(pValue->GetValueSegments())
      { cpValue = pValue; cImmediate = UT_TRUE;
      UtHugeMemcpy(cImmData, pImmData, Size); cSize = Size; }
    CBenValueSegment(BenContainerPos Pos, unsigned long Size)
      { cpValue = NULL; cImmediate = UT_FALSE; cPos = Pos;
      cSize = Size; }
    CBenValueSegment(BenConstDataPtr pImmData, unsigned short Size)
      { cpValue = NULL; cImmediate = UT_TRUE;
      UtHugeMemcpy(cImmData, pImmData, Size); cSize = Size; }
    UtBool IsLast()
    {
        return cpValue == NULL || cpValue->GetValueSegments()->GetLast() ==
          this;
    }
    UtBool IsImmediate() { return cImmediate; }
    BenContainerPos GetPosition() { return cPos; }
    unsigned long GetSize() { return cSize; }
    void SetSize(unsigned long Size) { cSize = Size; }
    BenByte * GetImmediateData() { return cImmData; }

private: // Data
    pCBenValue cpValue;
    UtBool cImmediate;
    union
    {
        BenContainerPos cPos;
        BenByte cImmData[4];
    };
    unsigned long cSize;
};

inline pCBenValueSegment CBenValue::GetNextValueSegment(pCBenValueSegment
  pCurrValueSegment)
{ return (pCBenValueSegment) cValueSegments.GetNextOrNULL(pCurrValueSegment); }

inline pLtcBenContainer CBenValue::GetContainer()
{ return GetProperty()->GetContainer(); }

class CBenNamedObject : public CBenObject
{
public: // Methods
    virtual UtBool IsNamedObject();
    virtual UtBool IsPropertyName();
    virtual UtBool IsTypeName();

public: // Internal methods
    CBenNamedObject(pLtcBenContainer pContainer, BenObjectID ObjectID,
    pCBenObject pPrevObject, const char * sName,
    pCBenNamedObjectListElmt pPrevNamedObjectListElmt);
    const char * GetName() { return csName.data(); }


    const char * GetNameCStr() { return csName.c_str(); }


    void SetPosition(BenContainerPos Pos) { cPos = Pos; }
    BenContainerPos GetPosition(void) { return cPos; }
    size_t GetLength(void) { return csName.length()+ 1; }
    pCBenNamedObjectListElmt GetNameListElmt() { return &cNameListElmt; }

private: // Data
    string csName;
    CBenNamedObjectListElmt cNameListElmt;
    BenContainerPos cPos;
};

class CBenPropertyName : public CBenNamedObject
{
public: // Internal methods
    CBenPropertyName(pLtcBenContainer pContainer, BenObjectID ObjectID,
    pCBenObject pPrevObject, const char * sName,
    pCBenNamedObjectListElmt pPrevNamedObjectListElmt) :
    CBenNamedObject(pContainer, ObjectID, pPrevObject, sName,
    pPrevNamedObjectListElmt) { ; }
    virtual UtBool IsPropertyName();
};

class CBenTypeName : public CBenNamedObject
{
public: // Internal methods
    CBenTypeName(pLtcBenContainer pContainer, BenObjectID ObjectID,
    pCBenObject pPrevObject, const char * sName,
    pCBenNamedObjectListElmt pPrevNamedObjectListElmt) :
    CBenNamedObject(pContainer, ObjectID, pPrevObject, sName,
    pPrevNamedObjectListElmt) { ; }
    virtual UtBool IsTypeName();
};


} // end namespace OpenStormBento
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
