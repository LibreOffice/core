/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SBXCLASS_HXX
#define _SBXCLASS_HXX

#include "tools/ref.hxx"
#include "svl/svarray.hxx"
#include "svl/smplhint.hxx"
#include "svl/lstner.hxx"

#include <basic/sbxdef.hxx>
#include <basic/sbxform.hxx>
#include <basic/sbxobj.hxx>
#include <basic/sbxprop.hxx>
#include <basic/sbxmeth.hxx>

class String;
class UniString;
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
class SvDispatch;

///////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////

#ifndef __SBX_SBXPARAMINFO
#define __SBX_SBXPARAMINFO

// Parameter information
struct SbxParamInfo
{
    const String aName;          // Name of the parameter
    SbxBaseRef   aTypeRef;       // Object, if object type
    SbxDataType  eType;          // Data type
    UINT16       nFlags;         // Flag-Bits
    UINT32       nUserData;      // IDs etc.
    SbxParamInfo( const String& s, SbxDataType t, USHORT n, SbxBase* b = NULL )
    : aName( s ), aTypeRef( b ), eType( t ), nFlags( n ), nUserData( 0 ) {}
    ~SbxParamInfo() {}
};

SV_DECL_PTRARR_DEL(SbxParams,SbxParamInfo*,4,4)

#endif

#ifndef __SBX_SBXINFO
#define __SBX_SBXINFO

class SbxInfo : public SvRefBase
{
    friend class SbxVariable;
    friend class SbMethod;

    String          aComment;
    String          aHelpFile;
    UINT32          nHelpId;
    SbxParams       aParams;

protected:
    BOOL LoadData( SvStream&, USHORT );
    BOOL StoreData( SvStream& ) const;
    virtual ~SbxInfo();
public:
    SbxInfo();
    SbxInfo( const String&, UINT32 );

    void                AddParam( const String&, SbxDataType, USHORT=SBX_READ );
    void                AddParam( const SbxParamInfo& );
    const SbxParamInfo* GetParam( USHORT n ) const; // index starts with 1!
    const String&       GetComment() const              { return aComment; }
    const String&       GetHelpFile() const             { return aHelpFile; }
    UINT32              GetHelpId() const               { return nHelpId;   }

    void                SetComment( const String& r )   { aComment = r; }
    void                SetHelpFile( const String& r )  { aHelpFile = r; }
    void                SetHelpId( UINT32 nId )         { nHelpId = nId; }
};

#endif

#ifndef __SBX_SBXHINT_HXX
#define __SBX_SBXHINT_HXX

class SbxHint : public SfxSimpleHint
{
    SbxVariable* pVar;
public:
    TYPEINFO();
    SbxHint( ULONG n, SbxVariable* v ) : SfxSimpleHint( n ), pVar( v ) {}
    SbxVariable* GetVar() const { return pVar; }
};

#endif

#ifndef __SBX_SBXALIAS_HXX
#define __SBX_SBXALIAS_HXX

// SbxAlias is an alias for a var or object
class SbxAlias : public SbxVariable, public SfxListener
{
    SbxVariableRef xAlias;
    virtual ~SbxAlias();
    virtual void Broadcast( ULONG );
    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                             const SfxHint& rHint, const TypeId& rHintType );
public:
    SbxAlias( const String& rName, SbxVariable* pOriginal );
    SbxAlias( const SbxAlias& );
    SbxAlias& operator=( const SbxAlias& );
};

#endif

#ifndef __SBX_SBXARRAY
#define __SBX_SBXARRAY

// SbxArray is an unidimensional, dynamic Array
// The variables convert from SbxVariablen. Put()/Insert() into the
// declared datatype, if they are not SbxVARIANT.

class SbxVarRefs;
class SbxVariableRef;

class SbxArrayImpl;

class SbxArray : public SbxBase
{
// #100883 Method to set method directly to parameter array
    friend class SbMethod;
    friend class SbClassModuleObject;
    friend SbxObject* cloneTypeObjectImpl( const SbxObject& rTypeObj );
    void PutDirect( SbxVariable* pVar, UINT32 nIdx );

    SbxArrayImpl* mpSbxArrayImpl; // Impl data
    SbxVarRefs*   pData;          // The variables

protected:
    SbxDataType eType;            // Data type of the array
    virtual ~SbxArray();
    virtual BOOL LoadData( SvStream&, USHORT );
    virtual BOOL StoreData( SvStream& ) const;

public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_ARRAY,1);
    TYPEINFO();
    SbxArray( SbxDataType=SbxVARIANT );
    SbxArray( const SbxArray& );
    SbxArray& operator=( const SbxArray& );
    virtual void Clear();
    USHORT Count() const;
    virtual SbxDataType GetType() const;
    virtual SbxClassType GetClass() const;
    SbxVariableRef& GetRef( USHORT );
    SbxVariable* Get( USHORT );
    void Put( SbxVariable*, USHORT );
    void Insert( SbxVariable*, USHORT );
    void Remove( USHORT );
    void Remove( SbxVariable* );
    void Merge( SbxArray* );
    const String& GetAlias( USHORT );
    void PutAlias( const String&, USHORT );
    SbxVariable* FindUserData( UINT32 nUserData );
    virtual SbxVariable* Find( const String&, SbxClassType );

    // Additional methods for 32-bit indices
    UINT32 Count32() const;
    SbxVariableRef& GetRef32( UINT32 );
    SbxVariable* Get32( UINT32 );
    void Put32( SbxVariable*, UINT32 );
    void Insert32( SbxVariable*, UINT32 );
    void Remove32( UINT32 );
};

#endif

#ifndef __SBX_SBXDIMARRAY_HXX
#define __SBX_SBXDIMARRAY_HXX

// SbxDimArray is an array that can dimensioned using BASIC conventions.
struct SbxDim;

class SbxDimArrayImpl;

class SbxDimArray : public SbxArray
{
    SbxDimArrayImpl* mpSbxDimArrayImpl;   // Impl data

    SbxDim* pFirst, *pLast;               // Links to Dimension table
    short   nDim;                         // Number of dimensions
    void   AddDimImpl32( INT32, INT32, BOOL bAllowSize0 );
    bool mbHasFixedSize;
protected:
    USHORT  Offset( const short* );
    UINT32  Offset32( const INT32* );
    USHORT  Offset( SbxArray* );
    UINT32  Offset32( SbxArray* );
    virtual BOOL LoadData( SvStream&, USHORT );
    virtual BOOL StoreData( SvStream& ) const;
    virtual ~SbxDimArray();
public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_DIMARRAY,1);
    TYPEINFO();
    SbxDimArray( SbxDataType=SbxVARIANT );
    SbxDimArray( const SbxDimArray& );
    SbxDimArray& operator=( const SbxDimArray& );
    virtual void Clear();
    using SbxArray::GetRef;
    SbxVariableRef& GetRef( const short* );
    using SbxArray::Get;
    SbxVariable* Get( const short* );
    using SbxArray::Put;
    void Put( SbxVariable*, const short* );
    SbxVariableRef& GetRef( SbxArray* );
    SbxVariable* Get( SbxArray* );
    void Put( SbxVariable*, SbxArray* );

    short  GetDims() const { return nDim;  }
    void   AddDim( short, short );
    void   unoAddDim( short, short );
    BOOL   GetDim( short, short&, short& ) const;

    using SbxArray::GetRef32;
    SbxVariableRef& GetRef32( const INT32* );
    using SbxArray::Get32;
    SbxVariable* Get32( const INT32* );
    using SbxArray::Put32;
    void Put32( SbxVariable*, const INT32* );
    void   AddDim32( INT32, INT32 );
    void   unoAddDim32( INT32, INT32 );
    BOOL   GetDim32( INT32, INT32&, INT32& ) const;
        bool hasFixedSize() { return mbHasFixedSize; };
        void setHasFixedSize( bool bHasFixedSize ) {mbHasFixedSize = bHasFixedSize; };
};

#endif

#ifndef __SBX_SBXCOLLECTION_HXX
#define __SBX_SBXCOLLECTION_HXX

class SbxCollection : public SbxObject
{
    void Initialize();
protected:
    virtual ~SbxCollection();
    virtual BOOL LoadData( SvStream&, USHORT );
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
    virtual SbxVariable* FindUserData( UINT32 nUserData );
    virtual SbxVariable* Find( const String&, SbxClassType );
    virtual void Clear();
};

#endif

#ifndef __SBX_SBXSTDCOLLECTION_HXX
#define __SBX_SBXSTDCOLLECTION_HXX

class SbxStdCollection : public SbxCollection
{
protected:
    String aElemClass;
    BOOL   bAddRemoveOk;
    virtual ~SbxStdCollection();
    virtual BOOL LoadData( SvStream&, USHORT );
    virtual BOOL StoreData( SvStream& ) const;
    virtual void CollAdd( SbxArray* pPar );
    virtual void CollRemove( SbxArray* pPar );
public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_FIXCOLLECTION,1);
    TYPEINFO();
    SbxStdCollection
        ( const String& rClassname, const String& rElemClass, BOOL=TRUE );
    SbxStdCollection( const SbxStdCollection& );
    SbxStdCollection& operator=( const SbxStdCollection& );
    virtual void Insert( SbxVariable* );
    const String& GetElementClass() const { return aElemClass; }
};

#endif

#ifndef __SBX_SBXREFS_HXX
#define __SBX_SBXREFS_HXX

SV_IMPL_REF(SbxBase)

SV_IMPL_REF(SbxVariable)

#ifndef SBX_ARRAY_DECL_DEFINED
#define SBX_ARRAY_DECL_DEFINED
SV_DECL_REF(SbxArray)
#endif
#ifndef SBX_ARRAY_IMPL_DEFINED
#define SBX_ARRAY_IMPL_DEFINED
SV_IMPL_REF(SbxArray)
#endif

#ifndef SBX_INFO_DECL_DEFINED
#define SBX_INFO_DECL_DEFINED
SV_DECL_REF(SbxInfo)
#endif
#ifndef SBX_INFO_IMPL_DEFINED
#define SBX_INFO_IMPL_DEFINED
SV_IMPL_REF(SbxInfo)
#endif

#ifndef SBX_DIMARRAY_DECL_DEFINED
#define SBX_DIMARRAY_DECL_DEFINED
SV_DECL_REF(SbxDimArray)
#endif
SV_IMPL_REF(SbxDimArray)

#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
