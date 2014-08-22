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

#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <svl/SfxBroadcaster.hxx>

#include <basic/sbx.hxx>
#include "runtime.hxx"
#include "sbxres.hxx"
#include "sbxconv.hxx"
#include "sbunoobj.hxx"
#include <math.h>
#include <ctype.h>

#include <com/sun/star/uno/XInterface.hpp>
using namespace com::sun::star::uno;

///////////////////////////// SbxVariable

TYPEINIT1(SbxVariable,SbxValue)
TYPEINIT1(SbxHint,SfxSimpleHint)

///////////////////////////// SbxVariableImpl

class SbxVariableImpl
{
    friend class SbxVariable;
    OUString                    m_aDeclareClassName;
    Reference< XInterface >     m_xComListener;
    StarBASIC*                  m_pComListenerParentBasic;

    SbxVariableImpl( void )
        : m_pComListenerParentBasic( NULL )
    {}
    SbxVariableImpl( const SbxVariableImpl& r )
        : m_aDeclareClassName( r.m_aDeclareClassName )
        , m_xComListener( r.m_xComListener )
        , m_pComListenerParentBasic( r.m_pComListenerParentBasic )
    {
    }
};


///////////////////////////// Constructors

SbxVariable::SbxVariable() : SbxValue()
{
    mpSbxVariableImpl = NULL;
    pCst = NULL;
    pParent = NULL;
    nUserData = 0;
    nHash = 0;
}

SbxVariable::SbxVariable( const SbxVariable& r )
    : SvRefBase( r ),
      SbxValue( r ),
      mpPar( r.mpPar ),
      pInfo( r.pInfo )
{
    mpSbxVariableImpl = NULL;
    if( r.mpSbxVariableImpl != NULL )
    {
        mpSbxVariableImpl = new SbxVariableImpl( *r.mpSbxVariableImpl );
#ifndef DISABLE_SCRIPTING
        if( mpSbxVariableImpl->m_xComListener.is() )
        {
            registerComListenerVariableForBasic( this, mpSbxVariableImpl->m_pComListenerParentBasic );
        }
#endif
    }
    pCst = NULL;
    if( r.CanRead() )
    {
        pParent = r.pParent;
        nUserData = r.nUserData;
        maName = r.maName;
        nHash = r.nHash;
    }
    else
    {
        pParent = NULL;
        nUserData = 0;
        nHash = 0;
    }
}

SbxVariable::SbxVariable( SbxDataType t, void* p ) : SbxValue( t, p )
{
    mpSbxVariableImpl = NULL;
    pCst = NULL;
    pParent = NULL;
    nUserData = 0;
    nHash = 0;
}

SbxVariable::~SbxVariable()
{
#ifndef DISABLE_SCRIPTING
    if( IsSet( SBX_DIM_AS_NEW ))
    {
        removeDimAsNewRecoverItem( this );
    }
#endif
    delete mpSbxVariableImpl;
    delete pCst;
}

////////////////////////////// Broadcasting

SfxBroadcaster& SbxVariable::GetBroadcaster()
{
    if( !pCst )
    {
        pCst = new SfxBroadcaster;
    }
    return *pCst;
}

SbxArray* SbxVariable::GetParameters() const
{
    return mpPar;
}


// Perhaps some day one could cut the parameter 0.
// then the copying will be dropped ...

void SbxVariable::Broadcast( sal_uIntPtr nHintId )
{
    if( pCst && !IsSet( SBX_NO_BROADCAST ) )
    {
        // Because the method could be called from outside, check the
        // rights here again
        if( nHintId & SBX_HINT_DATAWANTED )
        {
            if( !CanRead() )
            {
                return;
            }
        }
        if( nHintId & SBX_HINT_DATACHANGED )
        {
            if( !CanWrite() )
            {
                return;
            }
        }
        // Avoid further broadcasting
        SfxBroadcaster* pSave = pCst;
        pCst = NULL;
        sal_uInt16 nSaveFlags = GetFlags();
        SetFlag( SBX_READWRITE );
        if( mpPar.Is() )
        {
            // Register this as element 0, but don't change over the parent!
            mpPar->GetRef( 0 ) = this;
        }
        pSave->Broadcast( SbxHint( nHintId, this ) );
        delete pCst; // who knows already, onto which thoughts someone comes?
        pCst = pSave;
        SetFlags( nSaveFlags );
    }
}

SbxInfo* SbxVariable::GetInfo()
{
    if( !pInfo )
    {
        Broadcast( SBX_HINT_INFOWANTED );
        if( pInfo.Is() )
        {
            SetModified( true );
        }
    }
    return pInfo;
}

void SbxVariable::SetInfo( SbxInfo* p )
{
    pInfo = p;
}

void SbxVariable::SetParameters( SbxArray* p )
{
    mpPar = p;
}


/////////////////////////// Name of the variables

void SbxVariable::SetName( const OUString& rName )
{
    maName = rName;
    nHash = MakeHashCode( rName );
}

const OUString& SbxVariable::GetName( SbxNameType t ) const
{
    static const char cSuffixes[] = "  %&!#@ $";
    if( t == SbxNAME_NONE )
    {
        return maName;
    }
    // Request parameter-information (not for objects)
    ((SbxVariable*)this)->GetInfo();
    // Append nothing, if it is a simple property (no empty brackets)
    if( !pInfo || ( pInfo->aParams.empty() && GetClass() == SbxCLASS_PROPERTY ))
    {
        return maName;
    }
    sal_Unicode cType = ' ';
    OUString aTmp( maName );
    // short type? Then fetch it, posible this is 0.
    SbxDataType et = GetType();
    if( t == SbxNAME_SHORT_TYPES )
    {
        if( et <= SbxSTRING )
        {
            cType = cSuffixes[ et ];
        }
        if( cType != ' ' )
        {
            aTmp += OUString(sal_Unicode(cType));
        }
    }
    aTmp += "(";

    for(SbxParams::const_iterator i = pInfo->aParams.begin(); i != pInfo->aParams.end(); ++i)
    {
        int nt = i->eType & 0x0FFF;
        if( i != pInfo->aParams.begin() )
        {
            aTmp += ",";
        }
        if( i->nFlags & SBX_OPTIONAL )
        {
            aTmp += OUString( SbxRes( STRING_OPTIONAL ) );
        }
        if( i->eType & SbxBYREF )
        {
            aTmp += OUString( SbxRes( STRING_BYREF ) );
        }
        aTmp += i->aName;
        cType = ' ';
        // short type? Then fetch it, posible this is 0.
        if( t == SbxNAME_SHORT_TYPES )
        {
            if( nt <= SbxSTRING )
            {
                cType = cSuffixes[ nt ];
            }
        }
        if( cType != ' ' )
        {
            aTmp += OUString((sal_Unicode)cType);
            if( i->eType & SbxARRAY )
            {
                aTmp += "()";
            }
        }
        else
        {
            if( i->eType & SbxARRAY )
            {
                aTmp += "()";
            }
            // long type?
            if( t != SbxNAME_SHORT )
            {
                aTmp += OUString( SbxRes( STRING_AS ) );
                if( nt < 32 )
                {
                    aTmp += OUString( SbxRes( sal::static_int_cast< sal_uInt16 >( STRING_TYPES + nt ) ) );
                }
                else
                {
                    aTmp += OUString( SbxRes( STRING_ANY ) );
                }
            }
        }
    }
    aTmp += ")";
    // Long type? Then fetch it
    if( t == SbxNAME_LONG_TYPES && et != SbxEMPTY )
    {
        aTmp += OUString( SbxRes( STRING_AS ) );
        if( et < 32 )
        {
            aTmp += OUString( SbxRes( sal::static_int_cast< sal_uInt16 >( STRING_TYPES + et ) ) );
        }
        else
        {
            aTmp += OUString( SbxRes( STRING_ANY ) );
        }
    }
    ((SbxVariable*) this)->aToolString = aTmp;
    return aToolString;
}

// Create a simple hashcode: the first six characters were evaluated.

sal_uInt16 SbxVariable::MakeHashCode( const OUString& rName )
{
    sal_uInt16 n = 0;
    sal_Int32 i = 0;
    sal_uInt16 nLen = rName.getLength();
    if( nLen > 6 )
    {
        nLen = 6;
    }
     while( nLen-- )
    {
        sal_uInt8 c = (sal_uInt8)rName[i++];
        // If we have a commen sigen break!!
        if( c >= 0x80 )
        {
            return 0;
        }
        n = sal::static_int_cast< sal_uInt16 >( ( n << 3 ) + toupper( c ) );
    }
    return n;
}

////////////////////////////// Operators

SbxVariable& SbxVariable::operator=( const SbxVariable& r )
{
    SbxValue::operator=( r );
    delete mpSbxVariableImpl;
    if( r.mpSbxVariableImpl != NULL )
    {
        mpSbxVariableImpl = new SbxVariableImpl( *r.mpSbxVariableImpl );
#ifndef DISABLE_SCRIPTING
        if( mpSbxVariableImpl->m_xComListener.is() )
        {
            registerComListenerVariableForBasic( this, mpSbxVariableImpl->m_pComListenerParentBasic );
        }
#endif
    }
    else
    {
        mpSbxVariableImpl = NULL;
    }
    return *this;
}

//////////////////////////////// Conversion

SbxDataType SbxVariable::GetType() const
{
    if( aData.eType == SbxOBJECT )
    {
        return aData.pObj ? aData.pObj->GetType() : SbxOBJECT;
    }
    else if( aData.eType == SbxVARIANT )
    {
        return aData.pObj ? aData.pObj->GetType() : SbxVARIANT;
    }
    else
    {
        return aData.eType;
    }
}

SbxClassType SbxVariable::GetClass() const
{
    return SbxCLASS_VARIABLE;
}

void SbxVariable::SetModified( bool b )
{
    if( IsSet( SBX_NO_MODIFY ) )
    {
        return;
    }
    SbxBase::SetModified( b );
    if( pParent && pParent != this ) //??? HotFix: Recursion out here MM
    {
        pParent->SetModified( b );
    }
}

void SbxVariable::SetParent( SbxObject* p )
{
#ifdef DBG_UTIL
    // Will the parent of a SbxObject be set?
    if ( p && ISA(SbxObject) )
    {
        // then this had to be a child of the new parent
        bool bFound = false;
        SbxArray *pChildren = p->GetObjects();
        if ( pChildren )
        {
            for ( sal_uInt16 nIdx = 0; !bFound && nIdx < pChildren->Count(); ++nIdx )
            {
                bFound = ( this == pChildren->Get(nIdx) );
            }
        }
        SAL_INFO_IF(
            !bFound, "basic.sbx",
            "dangling: [" << GetName() << "].SetParent([" << p->GetName()
                << "])");
    }
#endif

    pParent = p;
}

SbxVariableImpl* SbxVariable::getImpl( void )
{
    if( mpSbxVariableImpl == NULL )
    {
        mpSbxVariableImpl = new SbxVariableImpl();
    }
    return mpSbxVariableImpl;
}

const OUString& SbxVariable::GetDeclareClassName( void )
{
    SbxVariableImpl* pImpl = getImpl();
    return pImpl->m_aDeclareClassName;
}

void SbxVariable::SetDeclareClassName( const OUString& rDeclareClassName )
{
    SbxVariableImpl* pImpl = getImpl();
    pImpl->m_aDeclareClassName = rDeclareClassName;
}

void SbxVariable::SetComListener( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xComListener,
                                  StarBASIC* pParentBasic )
{
    SbxVariableImpl* pImpl = getImpl();
    pImpl->m_xComListener = xComListener;
    pImpl->m_pComListenerParentBasic = pParentBasic;
#ifndef DISABLE_SCRIPTING
    registerComListenerVariableForBasic( this, pParentBasic );
#endif
}

void SbxVariable::ClearComListener( void )
{
    SbxVariableImpl* pImpl = getImpl();
    pImpl->m_xComListener.clear();
}


////////////////////////////// Loading/Saving

bool SbxVariable::LoadData( SvStream& rStrm, sal_uInt16 nVer )
{
    sal_uInt16 nType;
    sal_uInt8 cMark;
    rStrm.ReadUChar( cMark );
    if( cMark == 0xFF )
    {
        if( !SbxValue::LoadData( rStrm, nVer ) )
        {
            return false;
        }
        maName = read_uInt16_lenPrefixed_uInt8s_ToOUString(rStrm,
                                                                RTL_TEXTENCODING_ASCII_US);
        sal_uInt32 nTemp;
        rStrm.ReadUInt32( nTemp );
        nUserData = nTemp;
    }
    else
    {
        rStrm.SeekRel( -1L );
        rStrm.ReadUInt16( nType );
        maName = read_uInt16_lenPrefixed_uInt8s_ToOUString(rStrm,
                                                                RTL_TEXTENCODING_ASCII_US);
        sal_uInt32 nTemp;
        rStrm.ReadUInt32( nTemp );
        nUserData = nTemp;
        // correction: old methods have instead of SbxNULL now SbxEMPTY
        if( nType == SbxNULL && GetClass() == SbxCLASS_METHOD )
        {
            nType = SbxEMPTY;
        }
        SbxValues aTmp;
        OUString aTmpString;
        OUString aVal;
        aTmp.eType = aData.eType = (SbxDataType) nType;
        aTmp.pOUString = &aVal;
        switch( nType )
        {
        case SbxBOOL:
        case SbxERROR:
        case SbxINTEGER:
            rStrm.ReadInt16( aTmp.nInteger ); break;
        case SbxLONG:
            rStrm.ReadInt32( aTmp.nLong ); break;
        case SbxSINGLE:
        {
            // Floats as ASCII
            aTmpString = read_uInt16_lenPrefixed_uInt8s_ToOUString(
                    rStrm, RTL_TEXTENCODING_ASCII_US);
            double d;
            SbxDataType t;
            if( ImpScan( aTmpString, d, t, NULL ) != SbxERR_OK || t == SbxDOUBLE )
            {
                aTmp.nSingle = 0;
                return false;
            }
            aTmp.nSingle = (float) d;
            break;
        }
        case SbxDATE:
        case SbxDOUBLE:
        {
            // Floats as ASCII
            aTmpString = read_uInt16_lenPrefixed_uInt8s_ToOUString(rStrm,
                                                                        RTL_TEXTENCODING_ASCII_US);
            SbxDataType t;
            if( ImpScan( aTmpString, aTmp.nDouble, t, NULL ) != SbxERR_OK )
            {
                aTmp.nDouble = 0;
                return false;
            }
            break;
        }
        case SbxSTRING:
            aVal = read_uInt16_lenPrefixed_uInt8s_ToOUString(rStrm,
                                                                  RTL_TEXTENCODING_ASCII_US);
            break;
        case SbxEMPTY:
        case SbxNULL:
            break;
        default:
            aData.eType = SbxNULL;
            DBG_ASSERT( false, "Loaded a non-supported data type" );
            return false;
        }
        // putt value
        if( nType != SbxNULL && nType != SbxEMPTY && !Put( aTmp ) )
        {
            return false;
        }
    }
    rStrm.ReadUChar( cMark );
    // cMark is also a version number!
    // 1: initial version
    // 2: with nUserData
    if( cMark )
    {
        if( cMark > 2 )
        {
            return false;
        }
        pInfo = new SbxInfo;
        pInfo->LoadData( rStrm, (sal_uInt16) cMark );
    }
    // Load private data only, if it is a SbxVariable
    if( GetClass() == SbxCLASS_VARIABLE && !LoadPrivateData( rStrm, nVer ) )
    {
        return false;
    }
    ((SbxVariable*) this)->Broadcast( SBX_HINT_DATACHANGED );
    nHash =  MakeHashCode( maName );
    SetModified( true );
    return true;
}

bool SbxVariable::StoreData( SvStream& rStrm ) const
{
    rStrm.WriteUChar( (sal_uInt8) 0xFF );      // Marker
    bool bValStore;
    if( this->IsA( TYPE(SbxMethod) ) )
    {
        // #50200 Avoid that objects , which during the runtime
        // as return-value are saved in the method as a value were saved
        SbxVariable* pThis = (SbxVariable*)this;
        sal_uInt16 nSaveFlags = GetFlags();
        pThis->SetFlag( SBX_WRITE );
        pThis->SbxValue::Clear();
        pThis->SetFlags( nSaveFlags );

        // So that the method will not be executed in any case!
        // CAST, to avoid const!
        pThis->SetFlag( SBX_NO_BROADCAST );
        bValStore = SbxValue::StoreData( rStrm );
        pThis->ResetFlag( SBX_NO_BROADCAST );
    }
    else
    {
        bValStore = SbxValue::StoreData( rStrm );
    }
    if( !bValStore )
    {
        return false;
    }
    write_uInt16_lenPrefixed_uInt8s_FromOUString(rStrm, maName,
                                                      RTL_TEXTENCODING_ASCII_US);
    rStrm.WriteUInt32( (sal_uInt32)nUserData );
    if( pInfo.Is() )
    {
        rStrm.WriteUChar( (sal_uInt8) 2 );     // Version 2: with UserData!
        pInfo->StoreData( rStrm );
    }
    else
    {
        rStrm.WriteUChar( (sal_uInt8) 0 );
    }
    // Save private data only, if it is a SbxVariable
    if( GetClass() == SbxCLASS_VARIABLE )
    {
        return StorePrivateData( rStrm );
    }
    else
    {
        return true;
    }
}

////////////////////////////// SbxInfo

SbxInfo::SbxInfo() : aHelpFile(), nHelpId( 0 ), aParams()
{}

SbxInfo::SbxInfo( const OUString& r, sal_uInt32 n )
       : aHelpFile( r ), nHelpId( n ), aParams()
{}

////////////////////////////// SbxAlias

SbxAlias::SbxAlias( const SbxAlias& r )
        : SvRefBase( r ), SbxVariable( r ),
          SfxListener( r ), xAlias( r.xAlias )
{}

SbxAlias& SbxAlias::operator=( const SbxAlias& r )
{
    xAlias = r.xAlias;
    return *this;
}

SbxAlias::~SbxAlias()
{
    if( xAlias.Is() )
    {
        EndListening( xAlias->GetBroadcaster() );
    }
}

void SbxAlias::Broadcast( sal_uIntPtr nHt )
{
    if( xAlias.Is() )
    {
        xAlias->SetParameters( GetParameters() );
        if( nHt == SBX_HINT_DATAWANTED )
        {
            SbxVariable::operator=( *xAlias );
        }
        else if( nHt == SBX_HINT_DATACHANGED || nHt == SBX_HINT_CONVERTED )
        {
            *xAlias = *this;
        }
        else if( nHt == SBX_HINT_INFOWANTED )
        {
            xAlias->Broadcast( nHt );
            pInfo = xAlias->GetInfo();
        }
    }
}

void SbxAlias::SFX_NOTIFY( SfxBroadcaster&, const TypeId&,
                           const SfxHint& rHint, const TypeId& )
{
    const SbxHint* p = PTR_CAST(SbxHint,&rHint);
    if( p && p->GetId() == SBX_HINT_DYING )
    {
        xAlias.Clear();
        // delete the alias?
        if( pParent )
        {
            pParent->Remove( this );
        }
    }
}

void SbxVariable::Dump( SvStream& rStrm, bool bFill )
{
    OString aBNameStr(OUStringToOString(GetName( SbxNAME_SHORT_TYPES ), RTL_TEXTENCODING_ASCII_US));
    rStrm.WriteCharPtr( "Variable( " )
         .WriteCharPtr( OString::number(reinterpret_cast<sal_Int64>(this)).getStr() ).WriteCharPtr( "==" )
         .WriteCharPtr( aBNameStr.getStr() );
    OString aBParentNameStr(OUStringToOString(GetParent()->GetName(), RTL_TEXTENCODING_ASCII_US));
    if ( GetParent() )
    {
        rStrm.WriteCharPtr( " in parent '" ).WriteCharPtr( aBParentNameStr.getStr() ).WriteCharPtr( "'" );
    }
    else
    {
        rStrm.WriteCharPtr( " no parent" );
    }
    rStrm.WriteCharPtr( " ) " );

    // output also the object at object-vars
    if ( GetValues_Impl().eType == SbxOBJECT &&
            GetValues_Impl().pObj &&
            GetValues_Impl().pObj != this &&
            GetValues_Impl().pObj != GetParent() )
    {
        rStrm.WriteCharPtr( " contains " );
        ((SbxObject*) GetValues_Impl().pObj)->Dump( rStrm, bFill );
    }
    else
    {
        rStrm << endl;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
