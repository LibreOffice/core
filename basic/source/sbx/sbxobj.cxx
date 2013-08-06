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

#include <tools/stream.hxx>
#include <basic/sbx.hxx>
#include "sbxres.hxx"
#include <svl/brdcst.hxx>

TYPEINIT1(SbxMethod,SbxVariable)
TYPEINIT1(SbxProperty,SbxVariable)
TYPEINIT2(SbxObject,SbxVariable,SfxListener)

static OUString pNameProp;          // Name-Property
static OUString pParentProp;        // Parent-Property

static sal_uInt16 nNameHash = 0, nParentHash = 0;



SbxObject::SbxObject( const OUString& rClass )
         : SbxVariable( SbxOBJECT ), aClassName( rClass )
{
    aData.pObj = this;
    if( !nNameHash )
    {
        pNameProp = OUString::createFromAscii(GetSbxRes( STRING_NAMEPROP ));
        pParentProp = OUString::createFromAscii(GetSbxRes( STRING_PARENTPROP ));
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
        SetModified( sal_True );
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
            pObj->EndListening( rRef->GetBroadcaster(), sal_True );
        }
        // does the element have more then one reference and still a Listener?
        if( rRef->GetRefCount() > 1 )
        {
            rRef->SetParent( NULL );
            DBG_ASSERT( !rRef->IsBroadcaster() || rRef->GetBroadcaster().GetListenerCount(), "Object element with dangling parent" );
        }
    }
}

SbxObject::~SbxObject()
{
    CheckParentsOnDelete( this, pProps );
    CheckParentsOnDelete( this, pMethods );
    CheckParentsOnDelete( this, pObjs );

    // avoid handling in ~SbxVariable as SBX_DIM_AS_NEW == SBX_GBLSEARCH
    ResetFlag( SBX_DIM_AS_NEW );
}

SbxDataType SbxObject::GetType() const
{
    return SbxOBJECT;
}

SbxClassType SbxObject::GetClass() const
{
    return SbxCLASS_OBJECT;
}

void SbxObject::Clear()
{
    pMethods   = new SbxArray;
    pProps     = new SbxArray;
    pObjs      = new SbxArray( SbxOBJECT );
    SbxVariable* p;
    p = Make( pNameProp, SbxCLASS_PROPERTY, SbxSTRING );
    p->SetFlag( SBX_DONTSTORE );
    p = Make( pParentProp, SbxCLASS_PROPERTY, SbxOBJECT );
    p->ResetFlag( SBX_WRITE );
    p->SetFlag( SBX_DONTSTORE );
    pDfltProp  = NULL;
    SetModified( sal_False );
}

void SbxObject::SFX_NOTIFY( SfxBroadcaster&, const TypeId&,
                            const SfxHint& rHint, const TypeId& )
{
    const SbxHint* p = PTR_CAST(SbxHint,&rHint);
    if( p )
    {
        sal_uIntPtr nId = p->GetId();
        bool bRead  = ( nId == SBX_HINT_DATAWANTED );
        bool bWrite = ( nId == SBX_HINT_DATACHANGED );
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

sal_Bool SbxObject::IsClass( const OUString& rName ) const
{
    return sal_Bool( aClassName.equalsIgnoreAsciiCase( rName ) );
}

SbxVariable* SbxObject::FindUserData( sal_uInt32 nData )
{
    if( !GetAll( SbxCLASS_DONTCARE ) )
    {
        return NULL;
    }
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
    if( !pRes && IsSet( SBX_GBLSEARCH ) )
    {
        SbxObject* pCur = this;
        while( !pRes && pCur->pParent )
        {
            // I myself was already searched!
            sal_uInt16 nOwn = pCur->GetFlags();
            pCur->ResetFlag( SBX_EXTSEARCH );
            // I search already global!
            sal_uInt16 nPar = pCur->pParent->GetFlags();
            pCur->pParent->ResetFlag( SBX_GBLSEARCH );
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
    static sal_uInt16 nLvl = 0;
    static const char* pCls[] = { "DontCare","Array","Value","Variable","Method","Property","Object" };
    OString aNameStr1(OUStringToOString(rName, RTL_TEXTENCODING_ASCII_US));
    OString aNameStr2(OUStringToOString(SbxVariable::GetName(), RTL_TEXTENCODING_ASCII_US));
    DbgOutf( "SBX: Search %.*s %s %s in %s",
             nLvl++, "                              ",
             ( t >= SbxCLASS_DONTCARE && t <= SbxCLASS_OBJECT )
             ? pCls[ t-1 ] : "Unknown class", aNameStr1.getStr(), aNameStr2.getStr() );
#endif

    if( !GetAll( t ) )
    {
        return NULL;
    }
    SbxVariable* pRes = NULL;
    pObjs->SetFlag( SBX_EXTSEARCH );
    if( t == SbxCLASS_DONTCARE )
    {
        pRes = pMethods->Find( rName, SbxCLASS_METHOD );
        if( !pRes )
        {
            pRes = pProps->Find( rName, SbxCLASS_PROPERTY );
        }
        if( !pRes )
        {
            pRes = pObjs->Find( rName, t );
        }
    }
    else
    {
        SbxArray* pArray = NULL;
        switch( t )
        {
        case SbxCLASS_VARIABLE:
        case SbxCLASS_PROPERTY: pArray = pProps;    break;
        case SbxCLASS_METHOD:   pArray = pMethods;  break;
        case SbxCLASS_OBJECT:   pArray = pObjs;     break;
        default: DBG_ASSERT( !this, "Invalid SBX-Class" ); break;
        }
        if( pArray )
        {
            pRes = pArray->Find( rName, t );
        }
    }
    // Extended Search in the Object-Array?
    // For objects and DontCare the array of objects has already been searched
    if( !pRes && ( t == SbxCLASS_METHOD || t == SbxCLASS_PROPERTY ) )
        pRes = pObjs->Find( rName, t );
    // Search in the parents?
    if( !pRes && IsSet( SBX_GBLSEARCH ) )
    {
        SbxObject* pCur = this;
        while( !pRes && pCur->pParent )
        {
            // I myself was already searched!
            sal_uInt16 nOwn = pCur->GetFlags();
            pCur->ResetFlag( SBX_EXTSEARCH );
            // I search already global!
            sal_uInt16 nPar = pCur->pParent->GetFlags();
            pCur->pParent->ResetFlag( SBX_GBLSEARCH );
            pRes = pCur->pParent->Find( rName, t );
            pCur->SetFlags( nOwn );
            pCur->pParent->SetFlags( nPar );
            pCur = pCur->pParent;
        }
    }
#ifdef DBG_UTIL
    nLvl--;
    if( pRes )
    {
        OString aNameStr3(OUStringToOString(rName, RTL_TEXTENCODING_ASCII_US));
        OString aNameStr4(OUStringToOString(SbxVariable::GetName(), RTL_TEXTENCODING_ASCII_US));
        DbgOutf( "SBX: Found %.*s %s in %s",
            nLvl, "                              ", aNameStr3.getStr(), aNameStr4.getStr() );
    }
#endif
    return pRes;
}

// Abbreviated version: The parent-string will be searched
// The whole thing recursive, because Call() might be overloaded
// Qualified names are allowed

sal_Bool SbxObject::Call( const OUString& rName, SbxArray* pParam )
{
    SbxVariable* pMeth = FindQualified( rName, SbxCLASS_DONTCARE);
    if( pMeth && pMeth->ISA(SbxMethod) )
    {
        // FindQualified() might have struck already!
        if( pParam )
        {
            pMeth->SetParameters( pParam );
        }
        pMeth->Broadcast( SBX_HINT_DATAWANTED );
        pMeth->SetParameters( NULL );
        return sal_True;
    }
    SetError( SbxERR_NO_METHOD );
    return sal_False;
}

SbxProperty* SbxObject::GetDfltProperty()
{
    if ( !pDfltProp && !aDfltPropName.isEmpty() )
    {
        pDfltProp = (SbxProperty*) Find( aDfltPropName, SbxCLASS_PROPERTY );
        if( !pDfltProp )
        {
            pDfltProp = (SbxProperty*) Make( aDfltPropName, SbxCLASS_PROPERTY, SbxVARIANT );
        }
    }
    return pDfltProp;
}
void SbxObject::SetDfltProperty( const OUString& rName )
{
    if ( rName != aDfltPropName )
    {
        pDfltProp = NULL;
    }
    aDfltPropName = rName;
    SetModified( sal_True );
}

// Search of an already available variable. If it was located,
// the index will be set, otherwise the Count of the Array will be returned.
// In any case the correct Array will be returned.

SbxArray* SbxObject::FindVar( SbxVariable* pVar, sal_uInt16& nArrayIdx )
{
    SbxArray* pArray = NULL;
    if( pVar ) switch( pVar->GetClass() )
    {
    case SbxCLASS_VARIABLE:
    case SbxCLASS_PROPERTY: pArray = pProps;    break;
    case SbxCLASS_METHOD:   pArray = pMethods;  break;
    case SbxCLASS_OBJECT:   pArray = pObjs;     break;
    default: DBG_ASSERT( !this, "Invalid SBX-Class" ); break;
    }
    if( pArray )
    {
        nArrayIdx = pArray->Count();
        // Is the variable per name available?
        pArray->ResetFlag( SBX_EXTSEARCH );
        SbxVariable* pOld = pArray->Find( pVar->GetName(), pVar->GetClass() );
        if( pOld )
        {
            for( sal_uInt16 i = 0; i < pArray->Count(); i++ )
            {
                SbxVariableRef& rRef = pArray->GetRef( i );
                if( (SbxVariable*) rRef == pOld )
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

SbxVariable* SbxObject::Make( const OUString& rName, SbxClassType ct, SbxDataType dt )
{
    // Is the object already available?
    SbxArray* pArray = NULL;
    switch( ct )
    {
    case SbxCLASS_VARIABLE:
    case SbxCLASS_PROPERTY: pArray = pProps;    break;
    case SbxCLASS_METHOD:   pArray = pMethods;  break;
    case SbxCLASS_OBJECT:   pArray = pObjs;     break;
    default: DBG_ASSERT( !this, "Invalid SBX-Class" ); break;
    }
    if( !pArray )
    {
        return NULL;
    }
    // Collections may contain objects of the same name
    if( !( ct == SbxCLASS_OBJECT && ISA(SbxCollection) ) )
    {
        SbxVariable* pRes = pArray->Find( rName, ct );
        if( pRes )
        {
            return pRes;
        }
    }
    SbxVariable* pVar = NULL;
    switch( ct )
    {
    case SbxCLASS_VARIABLE:
    case SbxCLASS_PROPERTY:
        pVar = new SbxProperty( rName, dt );
        break;
    case SbxCLASS_METHOD:
        pVar = new SbxMethod( rName, dt );
        break;
    case SbxCLASS_OBJECT:
        pVar = CreateObject( rName );
        break;
    default:
        break;
    }
    pVar->SetParent( this );
    pArray->Put( pVar, pArray->Count() );
    SetModified( sal_True );
    // The object listen always
    StartListening( pVar->GetBroadcaster(), sal_True );
    Broadcast( SBX_HINT_OBJECTCHANGED );
    return pVar;
}

SbxObject* SbxObject::MakeObject( const OUString& rName, const OUString& rClass )
{
    // Is the object already available?
    if( !ISA(SbxCollection) )
    {
        SbxVariable* pRes = pObjs->Find( rName, SbxCLASS_OBJECT );
        if( pRes )
        {
            return PTR_CAST(SbxObject,pRes);
        }
    }
    SbxObject* pVar = CreateObject( rClass );
    if( pVar )
    {
        pVar->SetName( rName );
        pVar->SetParent( this );
        pObjs->Put( pVar, pObjs->Count() );
        SetModified( sal_True );
        // The object listen always
        StartListening( pVar->GetBroadcaster(), sal_True );
        Broadcast( SBX_HINT_OBJECTCHANGED );
    }
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
            if( pArray == pObjs && ISA(SbxCollection) )
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
                EndListening( pOld->GetBroadcaster(), sal_True );
                if( pVar->GetClass() == SbxCLASS_PROPERTY )
                {
                    if( pOld == pDfltProp )
                    {
                        pDfltProp = (SbxProperty*) pVar;
                    }
                }
            }
        }
        StartListening( pVar->GetBroadcaster(), sal_True );
        pArray->Put( pVar, nIdx );
        if( pVar->GetParent() != this )
        {
            pVar->SetParent( this );
        }
        SetModified( sal_True );
        Broadcast( SBX_HINT_OBJECTCHANGED );
#ifdef DBG_UTIL
        static const char* pCls[] =
            { "DontCare","Array","Value","Variable","Method","Property","Object" };
        OUString aVarName( pVar->GetName() );
        if ( aVarName.isEmpty() && pVar->ISA(SbxObject) )
        {
            aVarName = PTR_CAST(SbxObject,pVar)->GetClassName();
        }
        OString aNameStr1(OUStringToOString(aVarName, RTL_TEXTENCODING_ASCII_US));
        OString aNameStr2(OUStringToOString(SbxVariable::GetName(), RTL_TEXTENCODING_ASCII_US));
        DbgOutf( "SBX: Insert %s %s in %s",
                 ( pVar->GetClass() >= SbxCLASS_DONTCARE &&
                   pVar->GetClass() <= SbxCLASS_OBJECT )
                 ? pCls[ pVar->GetClass()-1 ] : "Unknown class", aNameStr1.getStr(), aNameStr2.getStr() );
#endif
    }
}

// Optimisation, Insertion without checking about
// double entry and without broadcasts, will only be used in SO2/auto.cxx
void SbxObject::QuickInsert( SbxVariable* pVar )
{
    SbxArray* pArray = NULL;
    if( pVar )
    {
        switch( pVar->GetClass() )
        {
        case SbxCLASS_VARIABLE:
        case SbxCLASS_PROPERTY: pArray = pProps;    break;
        case SbxCLASS_METHOD:   pArray = pMethods;  break;
        case SbxCLASS_OBJECT:   pArray = pObjs;     break;
        default: DBG_ASSERT( !this, "Invalid SBX-Class" ); break;
        }
    }
    if( pArray )
    {
        StartListening( pVar->GetBroadcaster(), sal_True );
        pArray->Put( pVar, pArray->Count() );
        if( pVar->GetParent() != this )
        {
            pVar->SetParent( this );
        }
        SetModified( sal_True );
#ifdef DBG_UTIL
        static const char* pCls[] =
            { "DontCare","Array","Value","Variable","Method","Property","Object" };
        OUString aVarName( pVar->GetName() );
        if ( aVarName.isEmpty() && pVar->ISA(SbxObject) )
        {
            aVarName = PTR_CAST(SbxObject,pVar)->GetClassName();
        }
        OString aNameStr1(OUStringToOString(aVarName, RTL_TEXTENCODING_ASCII_US));
        OString aNameStr2(OUStringToOString(SbxVariable::GetName(), RTL_TEXTENCODING_ASCII_US));
        DbgOutf( "SBX: Insert %s %s in %s",
                 ( pVar->GetClass() >= SbxCLASS_DONTCARE &&
                   pVar->GetClass() <= SbxCLASS_OBJECT )
                 ? pCls[ pVar->GetClass()-1 ] : "Unknown class", aNameStr1.getStr(), aNameStr2.getStr() );
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
        if ( aVarName.isEmpty() && pVar->ISA(SbxObject) )
        {
            aVarName = PTR_CAST(SbxObject,pVar)->GetClassName();
        }
        OString aNameStr1(OUStringToOString(aVarName, RTL_TEXTENCODING_ASCII_US));
        OString aNameStr2(OUStringToOString(SbxVariable::GetName(), RTL_TEXTENCODING_ASCII_US));
        DbgOutf( "SBX: Remove %s in %s",
            aNameStr1.getStr(), aNameStr2.getStr() );
#endif
        SbxVariableRef pVar_ = pArray->Get( nIdx );
        if( pVar_->IsBroadcaster() )
        {
            EndListening( pVar_->GetBroadcaster(), sal_True );
        }
        if( (SbxVariable*) pVar_ == pDfltProp )
        {
            pDfltProp = NULL;
        }
        pArray->Remove( nIdx );
        if( pVar_->GetParent() == this )
        {
            pVar_->SetParent( NULL );
        }
        SetModified( sal_True );
        Broadcast( SBX_HINT_OBJECTCHANGED );
    }
}

static sal_Bool LoadArray( SvStream& rStrm, SbxObject* pThis, SbxArray* pArray )
{
    SbxArrayRef p = (SbxArray*) SbxBase::Load( rStrm );
    if( !p.Is() )
    {
        return sal_False;
    }
    for( sal_uInt16 i = 0; i < p->Count(); i++ )
    {
        SbxVariableRef& r = p->GetRef( i );
        SbxVariable* pVar = r;
        if( pVar )
        {
            pVar->SetParent( pThis );
            pThis->StartListening( pVar->GetBroadcaster(), sal_True );
        }
    }
    pArray->Merge( p );
    return sal_True;
}

// The load of an object is additive!

sal_Bool SbxObject::LoadData( SvStream& rStrm, sal_uInt16 nVer )
{
    // Help for the read in of old objects: just return TRUE,
    // LoadPrivateData() has to set the default status up
    if( !nVer )
    {
        return sal_True;
    }
    pDfltProp = NULL;
    if( !SbxVariable::LoadData( rStrm, nVer ) )
    {
        return sal_False;
    }
    // If it contains no alien object, insert ourselves
    if( aData.eType == SbxOBJECT && !aData.pObj )
    {
        aData.pObj = this;
    }
    sal_uInt32 nSize;
    OUString aDfltProp;
    aClassName = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rStrm, RTL_TEXTENCODING_ASCII_US);
    aDfltProp = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rStrm, RTL_TEXTENCODING_ASCII_US);
    sal_uIntPtr nPos = rStrm.Tell();
    rStrm >> nSize;
    if( !LoadPrivateData( rStrm, nVer ) )
    {
        return sal_False;
    }
    sal_uIntPtr nNewPos = rStrm.Tell();
    nPos += nSize;
    DBG_ASSERT( nPos >= nNewPos, "SBX: Loaded too much data" );
    if( nPos != nNewPos )
    {
        rStrm.Seek( nPos );
    }
    if( !LoadArray( rStrm, this, pMethods ) ||
        !LoadArray( rStrm, this, pProps ) ||
        !LoadArray( rStrm, this, pObjs ) )
    {
        return sal_False;
    }
    // Set properties
    if( !aDfltProp.isEmpty() )
    {
        pDfltProp = (SbxProperty*) pProps->Find( aDfltProp, SbxCLASS_PROPERTY );
    }
    SetModified( sal_False );
    return sal_True;
}

sal_Bool SbxObject::StoreData( SvStream& rStrm ) const
{
    if( !SbxVariable::StoreData( rStrm ) )
    {
        return sal_False;
    }
    OUString aDfltProp;
    if( pDfltProp )
    {
        aDfltProp = pDfltProp->GetName();
    }
    write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rStrm, aClassName, RTL_TEXTENCODING_ASCII_US);
    write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rStrm, aDfltProp, RTL_TEXTENCODING_ASCII_US);
    sal_uIntPtr nPos = rStrm.Tell();
    rStrm << (sal_uInt32) 0L;
    if( !StorePrivateData( rStrm ) )
    {
        return sal_False;
    }
    sal_uIntPtr nNew = rStrm.Tell();
    rStrm.Seek( nPos );
    rStrm << (sal_uInt32) ( nNew - nPos );
    rStrm.Seek( nNew );
    if( !pMethods->Store( rStrm ) )
    {
        return sal_False;
    }
    if( !pProps->Store( rStrm ) )
    {
        return sal_False;
    }
    if( !pObjs->Store( rStrm ) )
    {
        return sal_False;
    }
    ((SbxObject*) this)->SetModified( sal_False );
    return sal_True;
}

OUString SbxObject::GenerateSource( const OUString &rLinePrefix,
                                    const SbxObject* )
{
    // Collect the properties in a String
    OUString aSource;
    SbxArrayRef xProps( GetProperties() );
    bool bLineFeed = false;
    for ( sal_uInt16 nProp = 0; nProp < xProps->Count(); ++nProp )
    {
        SbxPropertyRef xProp = (SbxProperty*) xProps->Get(nProp);
        OUString aPropName( xProp->GetName() );
        if ( xProp->CanWrite() &&
             !( xProp->GetHashCode() == nNameHash &&
                aPropName.equalsIgnoreAsciiCase(pNameProp)))
        {
            // Insert a break except in front of the first property
            if ( bLineFeed )
            {
                aSource += "\n";
            }
            else
            {
                bLineFeed = true;
            }
            aSource += rLinePrefix;
            aSource += ".";
            aSource += aPropName;
            aSource += " = ";

            // convert the property value to text
            switch ( xProp->GetType() )
            {
            case SbxEMPTY:
            case SbxNULL:
                // no value
                break;

            case SbxSTRING:
                // Strings in quotation mark
                aSource += "\"";
                aSource += xProp->GetOUString();
                aSource += "\"";
                break;

            default:
                // miscellaneous, such as e.g. numbers directly
                aSource += xProp->GetOUString();
                break;
            }
        }
    }
    return aSource;
}

static sal_Bool CollectAttrs( const SbxBase* p, OUString& rRes )
{
    OUString aAttrs;
    if( p->IsHidden() )
    {
        aAttrs = "Hidden";
    }
    if( p->IsSet( SBX_EXTSEARCH ) )
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
    if( p->IsSet( SBX_DONTSTORE ) )
    {
        if( !aAttrs.isEmpty() )
        {
            aAttrs += ",";
        }
        aAttrs += "DontStore";
    }
    if( !aAttrs.isEmpty() )
    {
        rRes = " (";
        rRes += aAttrs;
        rRes += ")";
        return sal_True;
    }
    else
    {
        rRes = "";
        return sal_False;
    }
}

void SbxObject::Dump( SvStream& rStrm, sal_Bool bFill )
{
    // Shifting
    static sal_uInt16 nLevel = 0;
    if ( nLevel > 10 )
    {
        rStrm << "<too deep>" << endl;
        return;
    }
    ++nLevel;
    OUString aIndent("");
    for ( sal_uInt16 n = 1; n < nLevel; ++n )
    {
        aIndent += "    ";
    }
    // if necessary complete the object
    if ( bFill )
    {
        GetAll( SbxCLASS_DONTCARE );
    }
    // Output the data of the object itself
    OString aNameStr(OUStringToOString(GetName(), RTL_TEXTENCODING_ASCII_US));
    OString aClassNameStr(OUStringToOString(aClassName, RTL_TEXTENCODING_ASCII_US));
    rStrm << "Object( "
          << OString::valueOf(reinterpret_cast<sal_Int64>(this)).getStr()<< "=='"
          << ( aNameStr.isEmpty() ?  "<unnamed>" : aNameStr.getStr() ) << "', "
          << "of class '" << aClassNameStr.getStr() << "', "
          << "counts "
          << OString::number(GetRefCount()).getStr()
          << " refs, ";
    if ( GetParent() )
    {
        OString aParentNameStr(OUStringToOString(GetName(), RTL_TEXTENCODING_ASCII_US));
        rStrm << "in parent "
              << OString::valueOf(reinterpret_cast<sal_Int64>(GetParent())).getStr()
              << "=='" << ( aParentNameStr.isEmpty() ? "<unnamed>" : aParentNameStr.getStr()  ) << "'";
    }
    else
    {
        rStrm << "no parent ";
    }
    rStrm << " )" << endl;
    OString aIndentNameStr(OUStringToOString(aIndent, RTL_TEXTENCODING_ASCII_US));
    rStrm << aIndentNameStr.getStr() << "{" << endl;

    // Flags
    OUString aAttrs;
    if( CollectAttrs( this, aAttrs ) )
    {
        OString aAttrStr(OUStringToOString(aAttrs, RTL_TEXTENCODING_ASCII_US));
        rStrm << aIndentNameStr.getStr() << "- Flags: " << aAttrStr.getStr() << endl;
    }

    // Methods
    rStrm << aIndentNameStr.getStr() << "- Methods:" << endl;
    for( sal_uInt16 i = 0; i < pMethods->Count(); i++ )
    {
        SbxVariableRef& r = pMethods->GetRef( i );
        SbxVariable* pVar = r;
        if( pVar )
        {
            OUString aLine( aIndent );
            aLine += "  - ";
            aLine += pVar->GetName( SbxNAME_SHORT_TYPES );
            OUString aAttrs2;
            if( CollectAttrs( pVar, aAttrs2 ) )
            {
                aLine += aAttrs2;
            }
            if( !pVar->IsA( TYPE(SbxMethod) ) )
            {
                aLine += "  !! Not a Method !!";
            }
            write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rStrm, aLine, RTL_TEXTENCODING_ASCII_US);

            // Output also the object at object-methods
            if ( pVar->GetValues_Impl().eType == SbxOBJECT &&
                    pVar->GetValues_Impl().pObj &&
                    pVar->GetValues_Impl().pObj != this &&
                    pVar->GetValues_Impl().pObj != GetParent() )
            {
                rStrm << " contains ";
                ((SbxObject*) pVar->GetValues_Impl().pObj)->Dump( rStrm, bFill );
            }
            else
            {
                rStrm << endl;
            }
        }
    }

    // Properties
    rStrm << aIndentNameStr.getStr() << "- Properties:" << endl;
    {
        for( sal_uInt16 i = 0; i < pProps->Count(); i++ )
        {
            SbxVariableRef& r = pProps->GetRef( i );
            SbxVariable* pVar = r;
            if( pVar )
            {
                OUString aLine( aIndent );
                aLine += "  - ";
                aLine += pVar->GetName( SbxNAME_SHORT_TYPES );
                OUString aAttrs3;
                if( CollectAttrs( pVar, aAttrs3 ) )
                {
                    aLine += aAttrs3;
                }
                if( !pVar->IsA( TYPE(SbxProperty) ) )
                {
                    aLine += "  !! Not a Property !!";
                }
                write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rStrm, aLine, RTL_TEXTENCODING_ASCII_US);

                // output also the object at object properties
                if ( pVar->GetValues_Impl().eType == SbxOBJECT &&
                        pVar->GetValues_Impl().pObj &&
                        pVar->GetValues_Impl().pObj != this &&
                        pVar->GetValues_Impl().pObj != GetParent() )
                {
                    rStrm << " contains ";
                    ((SbxObject*) pVar->GetValues_Impl().pObj)->Dump( rStrm, bFill );
                }
                else
                {
                    rStrm << endl;
                }
            }
        }
    }

    // Objects
    rStrm << aIndentNameStr.getStr() << "- Objects:" << endl;
    {
        for( sal_uInt16 i = 0; i < pObjs->Count(); i++ )
        {
            SbxVariableRef& r = pObjs->GetRef( i );
            SbxVariable* pVar = r;
            if ( pVar )
            {
                rStrm << aIndentNameStr.getStr() << "  - Sub";
                if ( pVar->ISA(SbxObject) )
                {
                    ((SbxObject*) pVar)->Dump( rStrm, bFill );
                }
                else if ( pVar->ISA(SbxVariable) )
                {
                    ((SbxVariable*) pVar)->Dump( rStrm, bFill );
                }
            }
        }
    }

    rStrm << aIndentNameStr.getStr() << "}" << endl << endl;
    --nLevel;
}

SbxMethod::SbxMethod( const OUString& r, SbxDataType t )
    : SbxVariable( t )
{
    SetName( r );
}

SbxMethod::SbxMethod( const SbxMethod& r )
    : SvRefBase( r ), SbxVariable( r )
{
}

SbxMethod::~SbxMethod()
{
}

SbxClassType SbxMethod::GetClass() const
{
    return SbxCLASS_METHOD;
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
    return SbxCLASS_PROPERTY;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
