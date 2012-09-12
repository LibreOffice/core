/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _SBXCLASS_HXX
#define _SBXCLASS_HXX

#include "tools/ref.hxx"
#include "svl/smplhint.hxx"
#include "svl/lstner.hxx"

#include <basic/sbxdef.hxx>
#include <basic/sbxform.hxx>
#include <basic/sbxobj.hxx>
#include <basic/sbxprop.hxx>
#include <basic/sbxmeth.hxx>
#include "basicdllapi.h"

#include <boost/ptr_container/ptr_vector.hpp>

class String;
class SvStream;
class SbxBase;
class SbxVariable;
class SbxProperty;
class SbxMethod;
class SbxObject;
class SbxArray;
class SbxDimArray;
class SbxFactory;

class SfxBroadcaster;

// Parameter information
struct SbxParamInfo
{
    const String aName;          // Name of the parameter
    SbxBaseRef   aTypeRef;       // Object, if object type
    SbxDataType  eType;          // Data type
    sal_uInt16       nFlags;         // Flag-Bits
    sal_uInt32       nUserData;      // IDs etc.
    SbxParamInfo( const String& s, SbxDataType t, sal_uInt16 n, SbxBase* b = NULL )
    : aName( s ), aTypeRef( b ), eType( t ), nFlags( n ), nUserData( 0 ) {}
    ~SbxParamInfo() {}
};

typedef boost::ptr_vector<SbxParamInfo> SbxParams;

class BASIC_DLLPUBLIC SbxInfo : public SvRefBase
{
    friend class SbxVariable;
    friend class SbMethod;

    String          aComment;
    String          aHelpFile;
    sal_uInt32          nHelpId;
    SbxParams       aParams;

protected:
    sal_Bool LoadData( SvStream&, sal_uInt16 );
    sal_Bool StoreData( SvStream& ) const;
    virtual ~SbxInfo();
public:
    SbxInfo();
    SbxInfo( const String&, sal_uInt32 );

    void                AddParam( const rtl::OUString&, SbxDataType, sal_uInt16=SBX_READ );
    const SbxParamInfo* GetParam( sal_uInt16 n ) const; // index starts with 1!
    const String&       GetComment() const              { return aComment; }
    const String&       GetHelpFile() const             { return aHelpFile; }
    sal_uInt32              GetHelpId() const               { return nHelpId;   }

    void                SetComment( const String& r )   { aComment = r; }
    void                SetHelpFile( const String& r )  { aHelpFile = r; }
    void                SetHelpId( sal_uInt32 nId )         { nHelpId = nId; }
};

class BASIC_DLLPUBLIC SbxHint : public SfxSimpleHint
{
    SbxVariable* pVar;
public:
    TYPEINFO();
    SbxHint( sal_uIntPtr n, SbxVariable* v ) : SfxSimpleHint( n ), pVar( v ) {}
    SbxVariable* GetVar() const { return pVar; }
};

// SbxAlias is an alias for a var or object
class BASIC_DLLPUBLIC SbxAlias : public SbxVariable, public SfxListener
{
    SbxVariableRef xAlias;
    virtual ~SbxAlias();
    virtual void Broadcast( sal_uIntPtr );
    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                             const SfxHint& rHint, const TypeId& rHintType );
public:
    SbxAlias( const SbxAlias& );
    SbxAlias& operator=( const SbxAlias& );
};

// SbxArray is an unidimensional, dynamic Array
// The variables convert from SbxVariablen. Put()/Insert() into the
// declared datatype, if they are not SbxVARIANT.

class SbxVarRefs;
class SbxVariableRef;

class BASIC_DLLPUBLIC SbxArray : public SbxBase
{
// #100883 Method to set method directly to parameter array
    friend class SbMethod;
    friend class SbClassModuleObject;
    friend SbxObject* cloneTypeObjectImpl( const SbxObject& rTypeObj );
    BASIC_DLLPRIVATE void PutDirect( SbxVariable* pVar, sal_uInt32 nIdx );

    SbxVarRefs*   pData;          // The variables

protected:
    SbxDataType eType;            // Data type of the array
    virtual ~SbxArray();
    virtual sal_Bool LoadData( SvStream&, sal_uInt16 );
    virtual sal_Bool StoreData( SvStream& ) const;

public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_ARRAY,1);
    TYPEINFO();
    SbxArray( SbxDataType=SbxVARIANT );
    SbxArray( const SbxArray& );
    SbxArray& operator=( const SbxArray& );
    virtual void Clear();
    sal_uInt16 Count() const;
    virtual SbxDataType GetType() const;
    virtual SbxClassType GetClass() const;
    SbxVariableRef& GetRef( sal_uInt16 );
    SbxVariable* Get( sal_uInt16 );
    void Put( SbxVariable*, sal_uInt16 );
    void Insert( SbxVariable*, sal_uInt16 );
    void Remove( sal_uInt16 );
    void Remove( SbxVariable* );
    void Merge( SbxArray* );
    const String& GetAlias( sal_uInt16 );
    void PutAlias( const String&, sal_uInt16 );
    SbxVariable* FindUserData( sal_uInt32 nUserData );
    virtual SbxVariable* Find( const rtl::OUString&, SbxClassType );

    // Additional methods for 32-bit indices
    sal_uInt32 Count32() const;
    SbxVariableRef& GetRef32( sal_uInt32 );
    SbxVariable* Get32( sal_uInt32 );
    void Put32( SbxVariable*, sal_uInt32 );
    void Insert32( SbxVariable*, sal_uInt32 );
    void Remove32( sal_uInt32 );
};

// SbxDimArray is an array that can dimensioned using BASIC conventions.
struct SbxDim;

class BASIC_DLLPUBLIC SbxDimArray : public SbxArray
{
    SbxDim* pFirst, *pLast;               // Links to Dimension table
    short   nDim;                         // Number of dimensions
    BASIC_DLLPRIVATE void AddDimImpl32( sal_Int32, sal_Int32, sal_Bool bAllowSize0 );
    bool mbHasFixedSize;
protected:
    sal_uInt16  Offset( const short* );
    sal_uInt32  Offset32( const sal_Int32* );
    sal_uInt32  Offset32( SbxArray* );
    virtual sal_Bool LoadData( SvStream&, sal_uInt16 );
    virtual sal_Bool StoreData( SvStream& ) const;
    virtual ~SbxDimArray();
public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_DIMARRAY,1);
    TYPEINFO();
    SbxDimArray( SbxDataType=SbxVARIANT );
    SbxDimArray( const SbxDimArray& );
    SbxDimArray& operator=( const SbxDimArray& );
    virtual void Clear();
    using SbxArray::GetRef;
    using SbxArray::Get;
    SbxVariable* Get( const short* );
    using SbxArray::Put;
    void Put( SbxVariable*, const short* );
    SbxVariable* Get( SbxArray* );

    short  GetDims() const;
    void   AddDim( short, short );
    void   unoAddDim( short, short );
    sal_Bool   GetDim( short, short&, short& ) const;

    using SbxArray::GetRef32;
    using SbxArray::Get32;
    SbxVariable* Get32( const sal_Int32* );
    using SbxArray::Put32;
    void Put32( SbxVariable*, const sal_Int32* );
    void   AddDim32( sal_Int32, sal_Int32 );
    void   unoAddDim32( sal_Int32, sal_Int32 );
    sal_Bool   GetDim32( sal_Int32, sal_Int32&, sal_Int32& ) const;
        bool hasFixedSize() { return mbHasFixedSize; };
        void setHasFixedSize( bool bHasFixedSize ) {mbHasFixedSize = bHasFixedSize; };
};

class BASIC_DLLPUBLIC SbxCollection : public SbxObject
{
    BASIC_DLLPRIVATE void Initialize();
protected:
    virtual ~SbxCollection();
    virtual sal_Bool LoadData( SvStream&, sal_uInt16 );
    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                             const SfxHint& rHint, const TypeId& rHintType );
    // Overridable methods (why not pure virtual?):
    virtual void CollAdd( SbxArray* pPar );
    virtual void CollItem( SbxArray* pPar );
    virtual void CollRemove( SbxArray* pPar );

public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_COLLECTION,1);
    TYPEINFO();
    SbxCollection( const String& rClassname );
    SbxCollection( const SbxCollection& );
    SbxCollection& operator=( const SbxCollection& );
    virtual SbxVariable* FindUserData( sal_uInt32 nUserData );
    virtual SbxVariable* Find( const rtl::OUString&, SbxClassType );
    virtual void Clear();
};

class BASIC_DLLPUBLIC SbxStdCollection : public SbxCollection
{
protected:
    String aElemClass;
    sal_Bool   bAddRemoveOk;
    virtual ~SbxStdCollection();
    virtual sal_Bool LoadData( SvStream&, sal_uInt16 );
    virtual sal_Bool StoreData( SvStream& ) const;
    virtual void CollAdd( SbxArray* pPar );
    virtual void CollRemove( SbxArray* pPar );
public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_FIXCOLLECTION,1);
    TYPEINFO();
    SbxStdCollection
        ( const String& rClassname, const String& rElemClass, sal_Bool=sal_True );
    SbxStdCollection( const SbxStdCollection& );
    SbxStdCollection& operator=( const SbxStdCollection& );
    virtual void Insert( SbxVariable* );
    const String& GetElementClass() const { return aElemClass; }
};

SV_IMPL_REF(SbxBase)

SV_IMPL_REF(SbxVariable)

#ifndef SBX_ARRAY_DECL_DEFINED
#define SBX_ARRAY_DECL_DEFINED
SV_DECL_REF(SbxArray)
#endif
SV_IMPL_REF(SbxArray)

#ifndef SBX_INFO_DECL_DEFINED
#define SBX_INFO_DECL_DEFINED
SV_DECL_REF(SbxInfo)
#endif
SV_IMPL_REF(SbxInfo)

SV_DECL_REF(SbxDimArray)
SV_IMPL_REF(SbxDimArray)

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
