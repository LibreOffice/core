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

#include <sal/config.h>

#include <iomanip>

#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <basic/sbx.hxx>
#include <svl/SfxBroadcaster.hxx>
#include "sbxres.hxx"


static OUString pNameProp;          // Name-Property
static OUString pParentProp;        // Parent-Property

static sal_uInt16 nNameHash = 0, nParentHash = 0;


SbxObject::SbxObject( const OUString& rClass )
         : SbxVariable( SbxOBJECT ), aClassName( rClass )
{
    aData.pObj = this;
    if( !nNameHash )
    {
        pNameProp = GetSbxRes( StringId::NameProp );
        pParentProp = GetSbxRes( StringId::ParentProp );
        nNameHash = MakeHashCode( pNameProp );
        nParentHash = MakeHashCode( pParentProp );
    }
    SbxObject::Clear();
    SbxObject::SetName( rClass );
}

SbxObject::SbxObject( const SbxObject& rObj )
    : SvRefBase( rObj ), SbxVariable( rObj.GetType() ),
      SfxListener( rObj )
{
    *this = rObj;
}

SbxObject& SbxObject::operator=( const SbxObject& r )
{
    if( &r != this )
    {
        SbxVariable::operator=( r );
        aClassName = r.aClassName;
        pMethods   = new SbxArray;
        pProps     = new SbxArray;
        pObjs      = new SbxArray( SbxOBJECT );
        // The arrays were copied, the content taken over
        *pMethods  = *r.pMethods;
        *pProps    = *r.pProps;
        *pObjs     = *r.pObjs;
        // Because the variables were taken over, this is OK
        pDfltProp  = r.pDfltProp;
        SetName( r.GetName() );
        SetFlags( r.GetFlags() );
        SetModified( true );
    }
    return *this;
}

static void CheckParentsOnDelete( SbxObject* pObj, SbxArray* p )
{
    for( sal_uInt16 i = 0; i < p->Count(); i++ )
    {
        SbxVariableRef& rRef = p->GetRef( i );
        if( rRef->IsBroadcaster() )
        {
            pObj->EndListening( rRef->GetBroadcaster(), true );
        }
        // does the element have more than one reference and still a Listener?
        if( rRef->GetRefCount() > 1 )
        {
            rRef->SetParent( nullptr );
            DBG_ASSERT( !rRef->IsBroadcaster() || rRef->GetBroadcaster().GetListenerCount(), "Object element with dangling parent" );
        }
    }
}

SbxObject::~SbxObject()
{
    CheckParentsOnDelete( this, pProps.get() );
    CheckParentsOnDelete( this, pMethods.get() );
    CheckParentsOnDelete( this, pObjs.get() );

    // avoid handling in ~SbxVariable as SbxFlagBits::DimAsNew == SbxFlagBits::GlobalSearch
    ResetFlag( SbxFlagBits::DimAsNew );
}

SbxDataType SbxObject::GetType() const
{
    return SbxOBJECT;
}

SbxClassType SbxObject::GetClass() const
{
    return SbxClassType::Object;
}

void SbxObject::Clear()
{
    pMethods   = new SbxArray;
    pProps     = new SbxArray;
    pObjs      = new SbxArray( SbxOBJECT );
    SbxVariable* p;
    p = Make( pNameProp, SbxClassType::Property, SbxSTRING );
    p->SetFlag( SbxFlagBits::DontStore );
    p = Make( pParentProp, SbxClassType::Property, SbxOBJECT );
    p->ResetFlag( SbxFlagBits::Write );
    p->SetFlag( SbxFlagBits::DontStore );
    pDfltProp  = nullptr;
    SetModified( false );
}

void SbxObject::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    const SbxHint* p = dynamic_cast<const SbxHint*>(&rHint);
    if( p )
    {
        const SfxHintId nId = p->GetId();
        bool bRead  = ( nId == SfxHintId::BasicDataWanted );
        bool bWrite = ( nId == SfxHintId::BasicDataChanged );
        SbxVariable* pVar = p->GetVar();
        if( bRead || bWrite )
        {
            OUString aVarName( pVar->GetName() );
            sal_uInt16 nHash_ = MakeHashCode( aVarName );
            if( nHash_ == nNameHash && aVarName.equalsIgnoreAsciiCase( pNameProp ) )
            {
                if( bRead )
                {
                    pVar->PutString( GetName() );
                }
                else
                {
                    SetName( pVar->GetOUString() );
                }
            }
            else if( nHash_ == nParentHash && aVarName.equalsIgnoreAsciiCase( pParentProp ) )
            {
                SbxObject* p_ = GetParent();
                if( !p_ )
                {
                    p_ = this;
                }
                pVar->PutObject( p_ );
            }
        }
    }
}

bool SbxObject::IsClass( const OUString& rName ) const
{
    return aClassName.equalsIgnoreAsciiCase( rName );
}

SbxVariable* SbxObject::FindUserData( sal_uInt32 nData )
{
    SbxVariable* pRes = pMethods->FindUserData( nData );
    if( !pRes )
    {
        pRes = pProps->FindUserData( nData );
    }
    if( !pRes )
    {
        pRes = pObjs->FindUserData( nData );
    }
    // Search in the parents?
    if( !pRes && IsSet( SbxFlagBits::GlobalSearch ) )
    {
        SbxObject* pCur = this;
        while( !pRes && pCur->pParent )
        {
            // I myself was already searched!
            SbxFlagBits nOwn = pCur->GetFlags();
            pCur->ResetFlag( SbxFlagBits::ExtSearch );
            // I search already global!
            SbxFlagBits nPar = pCur->pParent->GetFlags();
            pCur->pParent->ResetFlag( SbxFlagBits::GlobalSearch );
            pRes = pCur->pParent->FindUserData( nData );
            pCur->SetFlags( nOwn );
            pCur->pParent->SetFlags( nPar );
            pCur = pCur->pParent;
        }
    }
    return pRes;
}

SbxVariable* SbxObject::Find( const OUString& rName, SbxClassType t )
{
#ifdef DBG_UTIL
    static int nLvl = 1;
    static const char* pCls[] = { "DontCare","Array","Value","Variable","Method","Property","Object" };
    SAL_INFO(
        "basic.sbx",
        "search" << std::setw(nLvl) << " "
            << (t >= SbxClassType::DontCare && t <= SbxClassType::Object
                ? pCls[(int)t - 1] : "Unknown class")
            << " " << rName << " in " << SbxVariable::GetName());
    ++nLvl;
#endif

    SbxVariable* pRes = nullptr;
    pObjs->SetFlag( SbxFlagBits::ExtSearch );
    if( t == SbxClassType::DontCare )
    {
        pRes = pMethods->Find( rName, SbxClassType::Method );
        if( !pRes )
        {
            pRes = pProps->Find( rName, SbxClassType::Property );
        }
        if( !pRes )
        {
            pRes = pObjs->Find( rName, t );
        }
    }
    else
    {
        SbxArray* pArray = nullptr;
        switch( t )
        {
        case SbxClassType::Variable:
        case SbxClassType::Property: pArray = pProps.get();    break;
        case SbxClassType::Method:   pArray = pMethods.get();  break;
        case SbxClassType::Object:   pArray = pObjs.get();     break;
        default: SAL_WARN( "basic.sbx", "Invalid SBX-Class" ); break;
        }
        if( pArray )
        {
            pRes = pArray->Find( rName, t );
        }
    }
    // Extended Search in the Object-Array?
    // For objects and DontCare the array of objects has already been searched
    if( !pRes && ( t == SbxClassType::Method || t == SbxClassType::Property ) )
        pRes = pObjs->Find( rName, t );
    // Search in the parents?
    if( !pRes && IsSet( SbxFlagBits::GlobalSearch ) )
    {
        SbxObject* pCur = this;
        while( !pRes && pCur->pParent )
        {
            // I myself was already searched!
            SbxFlagBits nOwn = pCur->GetFlags();
            pCur->ResetFlag( SbxFlagBits::ExtSearch );
            // I search already global!
            SbxFlagBits nPar = pCur->pParent->GetFlags();
            pCur->pParent->ResetFlag( SbxFlagBits::GlobalSearch );
            pRes = pCur->pParent->Find( rName, t );
            pCur->SetFlags( nOwn );
            pCur->pParent->SetFlags( nPar );
            pCur = pCur->pParent;
        }
    }
#ifdef DBG_UTIL
    --nLvl;
    SAL_INFO_IF(
        pRes, "basic.sbx",
        "found" << std::setw(nLvl) << " " << rName << " in "
            << SbxVariable::GetName());
#endif
    return pRes;
}

// Abbreviated version: The parent-string will be searched
// The whole thing recursive, because Call() might be overridden
// Qualified names are allowed

bool SbxObject::Call( const OUString& rName, SbxArray* pParam )
{
    SbxVariable* pMeth = FindQualified( rName, SbxClassType::DontCare);
    if( pMeth && nullptr != dynamic_cast<const SbxMethod*>( pMeth) )
    {
        // FindQualified() might have struck already!
        if( pParam )
        {
            pMeth->SetParameters( pParam );
        }
        pMeth->Broadcast( SfxHintId::BasicDataWanted );
        pMeth->SetParameters( nullptr );
        return true;
    }
    SetError( ERRCODE_SBX_NO_METHOD );
    return false;
}

SbxProperty* SbxObject::GetDfltProperty()
{
    if ( !pDfltProp && !aDfltPropName.isEmpty() )
    {
        pDfltProp = static_cast<SbxProperty*>( Find( aDfltPropName, SbxClassType::Property ) );
        if( !pDfltProp )
        {
            pDfltProp = static_cast<SbxProperty*>( Make( aDfltPropName, SbxClassType::Property, SbxVARIANT ) );
        }
    }
    return pDfltProp;
}
void SbxObject::SetDfltProperty( const OUString& rName )
{
    if ( rName != aDfltPropName )
    {
        pDfltProp = nullptr;
    }
    aDfltPropName = rName;
    SetModified( true );
}

// Search of an already available variable. If it was located,
// the index will be set, otherwise the Count of the Array will be returned.
// In any case the correct Array will be returned.

SbxArray* SbxObject::FindVar( SbxVariable* pVar, sal_uInt16& nArrayIdx )
{
    SbxArray* pArray = nullptr;
    if( pVar )
    {
        switch( pVar->GetClass() )
        {
        case SbxClassType::Variable:
        case SbxClassType::Property: pArray = pProps.get();    break;
        case SbxClassType::Method:   pArray = pMethods.get();  break;
        case SbxClassType::Object:   pArray = pObjs.get();     break;
        default: SAL_WARN( "basic.sbx", "Invalid SBX-Class" ); break;
        }
    }
    if( pArray )
    {
        nArrayIdx = pArray->Count();
        // Is the variable per name available?
        pArray->ResetFlag( SbxFlagBits::ExtSearch );
        SbxVariable* pOld = pArray->Find( pVar->GetName(), pVar->GetClass() );
        if( pOld )
        {
            for( sal_uInt16 i = 0; i < pArray->Count(); i++ )
            {
                SbxVariableRef& rRef = pArray->GetRef( i );
                if( rRef.get() == pOld )
                {
                    nArrayIdx = i; break;
                }
            }
        }
    }
    return pArray;
}

// If a new object will be established, this object will be indexed,
// if an object of this name exists already.

SbxVariable* SbxObject::Make( const OUString& rName, SbxClassType ct, SbxDataType dt, bool bIsRuntimeFunction )
{
    // Is the object already available?
    SbxArray* pArray = nullptr;
    switch( ct )
    {
    case SbxClassType::Variable:
    case SbxClassType::Property: pArray = pProps.get();    break;
    case SbxClassType::Method:   pArray = pMethods.get();  break;
    case SbxClassType::Object:   pArray = pObjs.get();     break;
    default: SAL_WARN( "basic.sbx", "Invalid SBX-Class" ); break;
    }
    if( !pArray )
    {
        return nullptr;
    }
    // Collections may contain objects of the same name
    if( !( ct == SbxClassType::Object && nullptr != dynamic_cast<const SbxCollection*>( this ) ) )
    {
        SbxVariable* pRes = pArray->Find( rName, ct );
        if( pRes )
        {
            return pRes;
        }
    }
    SbxVariable* pVar = nullptr;
    switch( ct )
    {
    case SbxClassType::Variable:
    case SbxClassType::Property:
        pVar = new SbxProperty( rName, dt );
        break;
    case SbxClassType::Method:
        pVar = new SbxMethod( rName, dt, bIsRuntimeFunction );
        break;
    case SbxClassType::Object:
        pVar = CreateObject( rName );
        break;
    default:
        break;
    }
    pVar->SetParent( this );
    pArray->Put( pVar, pArray->Count() );
    SetModified( true );
    // The object listen always
    StartListening( pVar->GetBroadcaster(), true );
    Broadcast( SfxHintId::BasicObjectChanged );
    return pVar;
}

void SbxObject::Insert( SbxVariable* pVar )
{
    sal_uInt16 nIdx;
    SbxArray* pArray = FindVar( pVar, nIdx );
    if( pArray )
    {
        // Into with it. But you should pay attention at the Pointer!
        if( nIdx < pArray->Count() )
        {
            // Then this element exists already
            // There are objects of the same name allowed at collections
            if( pArray == pObjs.get() && nullptr != dynamic_cast<const SbxCollection*>( this ) )
            {
                nIdx = pArray->Count();
            }
            else
            {
                SbxVariable* pOld = pArray->Get( nIdx );
                // already inside: overwrite
                if( pOld == pVar )
                {
                    return;
                }
                EndListening( pOld->GetBroadcaster(), true );
                if( pVar->GetClass() == SbxClassType::Property )
                {
                    if( pOld == pDfltProp )
                    {
                        pDfltProp = static_cast<SbxProperty*>(pVar);
                    }
                }
            }
        }
        StartListening( pVar->GetBroadcaster(), true );
        pArray->Put( pVar, nIdx );
        if( pVar->GetParent() != this )
        {
            pVar->SetParent( this );
        }
        SetModified( true );
        Broadcast( SfxHintId::BasicObjectChanged );
#ifdef DBG_UTIL
        static const char* pCls[] =
            { "DontCare","Array","Value","Variable","Method","Property","Object" };
        OUString aVarName( pVar->GetName() );
        if (const SbxObject *pSbxObj = aVarName.isEmpty() ? dynamic_cast<const SbxObject*>(pVar) : nullptr)
        {
            aVarName = pSbxObj->GetClassName();
        }
        SAL_INFO(
            "basic.sbx",
            "insert "
                << ((pVar->GetClass() >= SbxClassType::DontCare
                     && pVar->GetClass() <= SbxClassType::Object)
                    ? pCls[(int)pVar->GetClass() - 1] : "Unknown class")
                << " " << aVarName << " in " << SbxVariable::GetName());
#endif
    }
}

// Optimisation, Insertion without checking about
// double entry and without broadcasts, will only be used in SO2/auto.cxx
void SbxObject::QuickInsert( SbxVariable* pVar )
{
    SbxArray* pArray = nullptr;
    if( pVar )
    {
        switch( pVar->GetClass() )
        {
        case SbxClassType::Variable:
        case SbxClassType::Property: pArray = pProps.get();    break;
        case SbxClassType::Method:   pArray = pMethods.get();  break;
        case SbxClassType::Object:   pArray = pObjs.get();     break;
        default: SAL_WARN( "basic.sbx", "Invalid SBX-Class" ); break;
        }
    }
    if( pArray )
    {
        StartListening( pVar->GetBroadcaster(), true );
        pArray->Put( pVar, pArray->Count() );
        if( pVar->GetParent() != this )
        {
            pVar->SetParent( this );
        }
        SetModified( true );
#ifdef DBG_UTIL
        static const char* pCls[] =
            { "DontCare","Array","Value","Variable","Method","Property","Object" };
        OUString aVarName( pVar->GetName() );
        if (const SbxObject *pSbxObj = aVarName.isEmpty() ? dynamic_cast<const SbxObject*>(pVar) : nullptr)
        {
            aVarName = pSbxObj->GetClassName();
        }
        SAL_INFO(
            "basic.sbx",
            "insert "
                << ((pVar->GetClass() >= SbxClassType::DontCare
                     && pVar->GetClass() <= SbxClassType::Object)
                    ? pCls[(int)pVar->GetClass() - 1] : "Unknown class")
                << " " << aVarName << " in " << SbxVariable::GetName());
#endif
    }
}

void SbxObject::Remove( const OUString& rName, SbxClassType t )
{
    Remove( SbxObject::Find( rName, t ) );
}

void SbxObject::Remove( SbxVariable* pVar )
{
    sal_uInt16 nIdx;
    SbxArray* pArray = FindVar( pVar, nIdx );
    if( pArray && nIdx < pArray->Count() )
    {
#ifdef DBG_UTIL
        OUString aVarName( pVar->GetName() );
        if (const SbxObject *pSbxObj = aVarName.isEmpty() ? dynamic_cast<const SbxObject*>(pVar) : nullptr)
        {
            aVarName = pSbxObj->GetClassName();
        }
        SAL_INFO(
            "basic.sbx",
            "remove " << aVarName << " in " << SbxVariable::GetName());
#endif
        SbxVariableRef pVar_ = pArray->Get( nIdx );
        if( pVar_->IsBroadcaster() )
        {
            EndListening( pVar_->GetBroadcaster(), true );
        }
        if( pVar_.get() == pDfltProp )
        {
            pDfltProp = nullptr;
        }
        pArray->Remove( nIdx );
        if( pVar_->GetParent() == this )
        {
            pVar_->SetParent( nullptr );
        }
        SetModified( true );
        Broadcast( SfxHintId::BasicObjectChanged );
    }
}

static bool LoadArray( SvStream& rStrm, SbxObject* pThis, SbxArray* pArray )
{
    SbxArrayRef p = static_cast<SbxArray*>( SbxBase::Load( rStrm ) );
    if( !p.is() )
    {
        return false;
    }
    for( sal_uInt16 i = 0; i < p->Count(); i++ )
    {
        SbxVariableRef& r = p->GetRef( i );
        SbxVariable* pVar = r.get();
        if( pVar )
        {
            pVar->SetParent( pThis );
            pThis->StartListening( pVar->GetBroadcaster(), true );
        }
    }
    pArray->Merge( p.get() );
    return true;
}

// The load of an object is additive!

bool SbxObject::LoadData( SvStream& rStrm, sal_uInt16 nVer )
{
    // Help for the read in of old objects: just return TRUE,
    // LoadPrivateData() has to set the default status up
    if( !nVer )
    {
        return true;
    }
    pDfltProp = nullptr;
    if( !SbxVariable::LoadData( rStrm, nVer ) )
    {
        return false;
    }
    // If it contains no alien object, insert ourselves
    if( aData.eType == SbxOBJECT && !aData.pObj )
    {
        aData.pObj = this;
    }
    sal_uInt32 nSize;
    OUString aDfltProp;
    aClassName = read_uInt16_lenPrefixed_uInt8s_ToOUString(rStrm, RTL_TEXTENCODING_ASCII_US);
    aDfltProp = read_uInt16_lenPrefixed_uInt8s_ToOUString(rStrm, RTL_TEXTENCODING_ASCII_US);
    sal_uInt64 nPos = rStrm.Tell();
    rStrm.ReadUInt32( nSize );
    sal_uInt64 const nNewPos = rStrm.Tell();
    nPos += nSize;
    DBG_ASSERT( nPos >= nNewPos, "SBX: Loaded too much data" );
    if( nPos != nNewPos )
    {
        rStrm.Seek( nPos );
    }
    if( !LoadArray( rStrm, this, pMethods.get() ) ||
        !LoadArray( rStrm, this, pProps.get() ) ||
        !LoadArray( rStrm, this, pObjs.get() ) )
    {
        return false;
    }
    // Set properties
    if( !aDfltProp.isEmpty() )
    {
        pDfltProp = static_cast<SbxProperty*>( pProps->Find( aDfltProp, SbxClassType::Property ) );
    }
    SetModified( false );
    return true;
}

bool SbxObject::StoreData( SvStream& rStrm ) const
{
    if( !SbxVariable::StoreData( rStrm ) )
    {
        return false;
    }
    OUString aDfltProp;
    if( pDfltProp )
    {
        aDfltProp = pDfltProp->GetName();
    }
    write_uInt16_lenPrefixed_uInt8s_FromOUString(rStrm, aClassName, RTL_TEXTENCODING_ASCII_US);
    write_uInt16_lenPrefixed_uInt8s_FromOUString(rStrm, aDfltProp, RTL_TEXTENCODING_ASCII_US);
    sal_uInt64 const nPos = rStrm.Tell();
    rStrm.WriteUInt32( 0 );
    sal_uInt64 const nNew = rStrm.Tell();
    rStrm.Seek( nPos );
    rStrm.WriteUInt32( nNew - nPos );
    rStrm.Seek( nNew );
    if( !pMethods->Store( rStrm ) )
    {
        return false;
    }
    if( !pProps->Store( rStrm ) )
    {
        return false;
    }
    if( !pObjs->Store( rStrm ) )
    {
        return false;
    }
    const_cast<SbxObject*>(this)->SetModified( false );
    return true;
}

static bool CollectAttrs( const SbxBase* p, OUString& rRes )
{
    OUString aAttrs;
    if( p->IsHidden() )
    {
        aAttrs = "Hidden";
    }
    if( p->IsSet( SbxFlagBits::ExtSearch ) )
    {
        if( !aAttrs.isEmpty() )
        {
            aAttrs += ",";
        }
        aAttrs += "ExtSearch";
    }
    if( !p->IsVisible() )
    {
        if( !aAttrs.isEmpty() )
        {
            aAttrs += ",";
        }
        aAttrs += "Invisible";
    }
    if( p->IsSet( SbxFlagBits::DontStore ) )
    {
        if( !aAttrs.isEmpty() )
        {
            aAttrs += ",";
        }
        aAttrs += "DontStore";
    }
    if( !aAttrs.isEmpty() )
    {
        rRes = " (" + aAttrs + ")";
        return true;
    }
    else
    {
        rRes.clear();
        return false;
    }
}

void SbxObject::Dump( SvStream& rStrm, bool bFill )
{
    // Shifting
    static sal_uInt16 nLevel = 0;
    if ( nLevel > 10 )
    {
        rStrm.WriteCharPtr( "<too deep>" ) << endl;
        return;
    }
    ++nLevel;
    OUString aIndent("");
    for ( sal_uInt16 n = 1; n < nLevel; ++n )
    {
        aIndent += "    ";
    }
    // Output the data of the object itself
    OString aNameStr(OUStringToOString(GetName(), RTL_TEXTENCODING_ASCII_US));
    OString aClassNameStr(OUStringToOString(aClassName, RTL_TEXTENCODING_ASCII_US));
    rStrm.WriteCharPtr( "Object( " )
         .WriteCharPtr( OString::number(reinterpret_cast<sal_Int64>(this)).getStr() ).WriteCharPtr( "=='" )
         .WriteCharPtr( aNameStr.isEmpty() ?  "<unnamed>" : aNameStr.getStr()  ).WriteCharPtr( "', " )
         .WriteCharPtr( "of class '" ).WriteCharPtr( aClassNameStr.getStr() ).WriteCharPtr( "', " )
         .WriteCharPtr( "counts " )
         .WriteCharPtr( OString::number(GetRefCount()).getStr() )
         .WriteCharPtr( " refs, " );
    if ( GetParent() )
    {
        OString aParentNameStr(OUStringToOString(GetName(), RTL_TEXTENCODING_ASCII_US));
        rStrm.WriteCharPtr( "in parent " )
             .WriteCharPtr( OString::number(reinterpret_cast<sal_Int64>(GetParent())).getStr() )
             .WriteCharPtr( "=='" ).WriteCharPtr( aParentNameStr.isEmpty() ? "<unnamed>" : aParentNameStr.getStr()   ).WriteCharPtr( "'" );
    }
    else
    {
        rStrm.WriteCharPtr( "no parent " );
    }
    rStrm.WriteCharPtr( " )" ) << endl;
    OString aIndentNameStr(OUStringToOString(aIndent, RTL_TEXTENCODING_ASCII_US));
    rStrm.WriteCharPtr( aIndentNameStr.getStr() ).WriteCharPtr( "{" ) << endl;

    // Flags
    OUString aAttrs;
    if( CollectAttrs( this, aAttrs ) )
    {
        OString aAttrStr(OUStringToOString(aAttrs, RTL_TEXTENCODING_ASCII_US));
        rStrm.WriteCharPtr( aIndentNameStr.getStr() ).WriteCharPtr( "- Flags: " ).WriteCharPtr( aAttrStr.getStr() ) << endl;
    }

    // Methods
    rStrm.WriteCharPtr( aIndentNameStr.getStr() ).WriteCharPtr( "- Methods:" ) << endl;
    for( sal_uInt16 i = 0; i < pMethods->Count(); i++ )
    {
        SbxVariableRef& r = pMethods->GetRef( i );
        SbxVariable* pVar = r.get();
        if( pVar )
        {
            OUString aLine = aIndent + "  - " + pVar->GetName( SbxNAME_SHORT_TYPES );
            OUString aAttrs2;
            if( CollectAttrs( pVar, aAttrs2 ) )
            {
                aLine += aAttrs2;
            }
            if( dynamic_cast<const SbxMethod *>(pVar) == nullptr )
            {
                aLine += "  !! Not a Method !!";
            }
            write_uInt16_lenPrefixed_uInt8s_FromOUString(rStrm, aLine, RTL_TEXTENCODING_ASCII_US);

            // Output also the object at object-methods
            if ( pVar->GetValues_Impl().eType == SbxOBJECT &&
                    pVar->GetValues_Impl().pObj &&
                    pVar->GetValues_Impl().pObj != this &&
                    pVar->GetValues_Impl().pObj != GetParent() )
            {
                rStrm.WriteCharPtr( " contains " );
                static_cast<SbxObject*>(pVar->GetValues_Impl().pObj)->Dump( rStrm, bFill );
            }
            else
            {
                rStrm << endl;
            }
        }
    }

    // Properties
    rStrm.WriteCharPtr( aIndentNameStr.getStr() ).WriteCharPtr( "- Properties:" ) << endl;
    {
        for( sal_uInt16 i = 0; i < pProps->Count(); i++ )
        {
            SbxVariableRef& r = pProps->GetRef( i );
            SbxVariable* pVar = r.get();
            if( pVar )
            {
                OUString aLine = aIndent + "  - " + pVar->GetName( SbxNAME_SHORT_TYPES );
                OUString aAttrs3;
                if( CollectAttrs( pVar, aAttrs3 ) )
                {
                    aLine += aAttrs3;
                }
                if( dynamic_cast<const SbxProperty *>(pVar) == nullptr )
                {
                    aLine += "  !! Not a Property !!";
                }
                write_uInt16_lenPrefixed_uInt8s_FromOUString(rStrm, aLine, RTL_TEXTENCODING_ASCII_US);

                // output also the object at object properties
                if ( pVar->GetValues_Impl().eType == SbxOBJECT &&
                        pVar->GetValues_Impl().pObj &&
                        pVar->GetValues_Impl().pObj != this &&
                        pVar->GetValues_Impl().pObj != GetParent() )
                {
                    rStrm.WriteCharPtr( " contains " );
                    static_cast<SbxObject*>(pVar->GetValues_Impl().pObj)->Dump( rStrm, bFill );
                }
                else
                {
                    rStrm << endl;
                }
            }
        }
    }

    // Objects
    rStrm.WriteCharPtr( aIndentNameStr.getStr() ).WriteCharPtr( "- Objects:" ) << endl;
    {
        for( sal_uInt16 i = 0; i < pObjs->Count(); i++ )
        {
            SbxVariableRef& r = pObjs->GetRef( i );
            SbxVariable* pVar = r.get();
            if ( pVar )
            {
                rStrm.WriteCharPtr( aIndentNameStr.getStr() ).WriteCharPtr( "  - Sub" );
                if (SbxObject *pSbxObj = dynamic_cast<SbxObject*>(pVar))
                {
                    pSbxObj->Dump(rStrm, bFill);
                }
                else if (SbxVariable *pSbxVar = dynamic_cast<SbxVariable*>(pVar))
                {
                    pSbxVar->Dump(rStrm, bFill);
                }
            }
        }
    }

    rStrm.WriteCharPtr( aIndentNameStr.getStr() ).WriteCharPtr( "}" ) << endl << endl;
    --nLevel;
}

SbxMethod::SbxMethod( const OUString& r, SbxDataType t, bool bIsRuntimeFunction )
    : SbxVariable(t)
    , mbIsRuntimeFunction(bIsRuntimeFunction)
    , mbRuntimeFunctionReturnType(t)
{
    SetName(r);
}

SbxMethod::SbxMethod( const SbxMethod& r )
    : SvRefBase(r)
    , SbxVariable(r)
    , mbIsRuntimeFunction(r.IsRuntimeFunction())
    , mbRuntimeFunctionReturnType(r.GetRuntimeFunctionReturnType())
{
}

SbxMethod::~SbxMethod()
{
}

SbxClassType SbxMethod::GetClass() const
{
    return SbxClassType::Method;
}

SbxProperty::SbxProperty( const OUString& r, SbxDataType t )
    : SbxVariable( t )
{
    SetName( r );
}

SbxProperty::~SbxProperty()
{
}

SbxClassType SbxProperty::GetClass() const
{
    return SbxClassType::Property;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
