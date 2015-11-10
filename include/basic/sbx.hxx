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

#ifndef INCLUDED_BASIC_SBX_HXX
#define INCLUDED_BASIC_SBX_HXX

#include <tools/ref.hxx>
#include <svl/smplhint.hxx>
#include <svl/lstner.hxx>

#include <basic/sbxdef.hxx>
#include <basic/sbxform.hxx>
#include <basic/sbxobj.hxx>
#include <basic/sbxprop.hxx>
#include <basic/sbxmeth.hxx>
#include <basic/basicdllapi.h>

#include <vector>
#include <memory>

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
    const OUString aName;          // Name of the parameter
    SbxBaseRef     aTypeRef;       // Object, if object type
    SbxDataType    eType;          // Data type
    SbxFlagBits    nFlags;         // Flag-Bits
    sal_uInt32     nUserData;      // IDs etc.
    SbxParamInfo( const OUString& s, SbxDataType t, SbxFlagBits n, SbxBase* b = nullptr )
        : aName( s ), aTypeRef( b ), eType( t ), nFlags( n ), nUserData( 0 ) {}
    ~SbxParamInfo() {}
};

typedef std::vector<std::unique_ptr<SbxParamInfo>> SbxParams;

class BASIC_DLLPUBLIC SbxInfo : public SvRefBase
{
    friend class SbxVariable;
    friend class SbMethod;

    OUString        aComment;
    OUString        aHelpFile;
    sal_uInt32      nHelpId;
    SbxParams       m_Params;

    SbxInfo(SbxInfo const&) = delete;
    void operator=(SbxInfo const&) = delete;

protected:
    bool LoadData( SvStream&, sal_uInt16 );
    bool StoreData( SvStream& ) const;
    virtual ~SbxInfo();
public:
    SbxInfo();
    SbxInfo( const OUString&, sal_uInt32 );

    void                AddParam( const OUString&, SbxDataType, SbxFlagBits=SbxFlagBits::Read );
    const SbxParamInfo* GetParam( sal_uInt16 n ) const; // index starts with 1!
    const OUString&     GetComment() const              { return aComment; }
    const OUString&     GetHelpFile() const             { return aHelpFile; }
    sal_uInt32          GetHelpId() const               { return nHelpId;   }

    void                SetComment( const OUString& r )   { aComment = r; }
};

class BASIC_DLLPUBLIC SbxHint : public SfxSimpleHint
{
    SbxVariable* pVar;
public:
    SbxHint( sal_uInt32 n, SbxVariable* v ) : SfxSimpleHint( n ), pVar( v ) {}
    SbxVariable* GetVar() const { return pVar; }
};

// SbxAlias is an alias for a var or object
class BASIC_DLLPUBLIC SbxAlias : public SbxVariable, public SfxListener
{
    SbxVariableRef xAlias;
    virtual ~SbxAlias();
    virtual void Broadcast( sal_uInt32 ) override;
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;
public:
    SbxAlias( const SbxAlias& );
    SbxAlias& operator=( const SbxAlias& );
};

// SbxArray is an unidimensional, dynamic Array
// The variables convert from SbxVariablen. Put()/Insert() into the
// declared datatype, if they are not SbxVARIANT.

struct SbxVarEntry;

class BASIC_DLLPUBLIC SbxArray : public SbxBase
{
    typedef std::vector<SbxVarEntry*> VarEntriesType;

// #100883 Method to set method directly to parameter array
    friend class SbMethod;
    friend class SbClassModuleObject;
    friend SbxObject* cloneTypeObjectImpl( const SbxObject& rTypeObj );
    BASIC_DLLPRIVATE void PutDirect( SbxVariable* pVar, sal_uInt32 nIdx );

    VarEntriesType* mpVarEntries;          // The variables

protected:
    SbxDataType eType;            // Data type of the array
    virtual ~SbxArray();
    virtual bool LoadData( SvStream&, sal_uInt16 ) override;
    virtual bool StoreData( SvStream& ) const override;

public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_ARRAY,1);
    TYPEINFO_OVERRIDE();
    SbxArray( SbxDataType=SbxVARIANT );
    SbxArray( const SbxArray& );
    SbxArray& operator=( const SbxArray& );
    virtual void Clear() override;
    sal_uInt16           Count() const;
    virtual SbxDataType  GetType() const override;
    virtual SbxClassType GetClass() const override;
    SbxVariableRef&      GetRef( sal_uInt16 );
    SbxVariable*         Get( sal_uInt16 );
    void                 Put( SbxVariable*, sal_uInt16 );
    void                 Insert( SbxVariable*, sal_uInt16 );
    void                 Remove( sal_uInt16 );
    void                 Remove( SbxVariable* );
    void                 Merge( SbxArray* );
    OUString             GetAlias( sal_uInt16 );
    void                 PutAlias( const OUString&, sal_uInt16 );
    SbxVariable*         FindUserData( sal_uInt32 nUserData );
    SbxVariable* Find( const OUString&, SbxClassType );

    // Additional methods for 32-bit indices
    sal_uInt32           Count32() const;
    SbxVariableRef&      GetRef32( sal_uInt32 );
    SbxVariable*         Get32( sal_uInt32 );
    void                 Put32( SbxVariable*, sal_uInt32 );
    void                 Insert32( SbxVariable*, sal_uInt32 );
    void                 Remove32( sal_uInt32 );
};

// SbxDimArray is an array that can dimensioned using BASIC conventions.
struct SbxDim {                 // an array-dimension:
    sal_Int32 nLbound, nUbound; // Limitations
    sal_Int32 nSize;            // Number of elements
};

class BASIC_DLLPUBLIC SbxDimArray : public SbxArray
{
    std::vector<SbxDim> m_vDimensions;     // Dimension table
    BASIC_DLLPRIVATE void AddDimImpl32( sal_Int32, sal_Int32, bool bAllowSize0 );
    bool mbHasFixedSize;
protected:
    sal_uInt16  Offset( const short* );
    sal_uInt32  Offset32( const sal_Int32* );
    sal_uInt32  Offset32( SbxArray* );
    virtual bool LoadData( SvStream&, sal_uInt16 ) override;
    virtual bool StoreData( SvStream& ) const override;
    virtual ~SbxDimArray();
public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_DIMARRAY,1);
    TYPEINFO_OVERRIDE();
    SbxDimArray( SbxDataType=SbxVARIANT );
    SbxDimArray( const SbxDimArray& );
    SbxDimArray& operator=( const SbxDimArray& );
    virtual void Clear() override;
    using SbxArray::GetRef;
    using SbxArray::Get;
    SbxVariable* Get( const short* );
    using SbxArray::Put;
    void Put( SbxVariable*, const short* );
    SbxVariable* Get( SbxArray* );

    short  GetDims() const { return m_vDimensions.size();}
    void   AddDim( short, short );
    void   unoAddDim( short, short );
    bool   GetDim( short, short&, short& ) const;

    using SbxArray::GetRef32;
    using SbxArray::Get32;
    SbxVariable* Get32( const sal_Int32* );
    using SbxArray::Put32;
    void Put32( SbxVariable*, const sal_Int32* );
    void   AddDim32( sal_Int32, sal_Int32 );
    void   unoAddDim32( sal_Int32, sal_Int32 );
    bool   GetDim32( sal_Int32, sal_Int32&, sal_Int32& ) const;
    bool hasFixedSize() { return mbHasFixedSize; };
    void setHasFixedSize( bool bHasFixedSize ) {mbHasFixedSize = bHasFixedSize; };
};

class BASIC_DLLPUBLIC SbxCollection : public SbxObject
{
    BASIC_DLLPRIVATE void Initialize();
protected:
    virtual ~SbxCollection();
    virtual bool LoadData( SvStream&, sal_uInt16 ) override;
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;
    // Overridable methods (why not pure virtual?):
    virtual void CollAdd( SbxArray* pPar );
    void CollItem( SbxArray* pPar );
    virtual void CollRemove( SbxArray* pPar );

public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_COLLECTION,1);
    TYPEINFO_OVERRIDE();
    SbxCollection( const OUString& rClassname );
    SbxCollection( const SbxCollection& );
    SbxCollection& operator=( const SbxCollection& );
    virtual SbxVariable* FindUserData( sal_uInt32 nUserData ) override;
    virtual SbxVariable* Find( const OUString&, SbxClassType ) override;
    virtual void Clear() override;
};

class BASIC_DLLPUBLIC SbxStdCollection : public SbxCollection
{
protected:
    OUString aElemClass;
    bool bAddRemoveOk;
    virtual ~SbxStdCollection();
    virtual bool LoadData( SvStream&, sal_uInt16 ) override;
    virtual bool StoreData( SvStream& ) const override;
    virtual void CollAdd( SbxArray* pPar ) override;
    virtual void CollRemove( SbxArray* pPar ) override;
public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_FIXCOLLECTION,1);
    TYPEINFO_OVERRIDE();
    SbxStdCollection( const OUString& rClassname, const OUString& rElemClass, bool=true );
    SbxStdCollection( const SbxStdCollection& );
    SbxStdCollection& operator=( const SbxStdCollection& );
    virtual void Insert( SbxVariable* ) override;
};

#ifndef SBX_ARRAY_DECL_DEFINED
#define SBX_ARRAY_DECL_DEFINED
typedef tools::SvRef<SbxArray> SbxArrayRef;
#endif

#ifndef SBX_INFO_DECL_DEFINED
#define SBX_INFO_DECL_DEFINED
typedef tools::SvRef<SbxInfo> SbxInfoRef;
#endif

typedef tools::SvRef<SbxDimArray> SbxDimArrayRef;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
