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

#include <config_features.h>

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

// SbxVariable


// SbxVariableImpl

class SbxVariableImpl
{
    friend class SbxVariable;
    OUString                    m_aDeclareClassName;
    Reference< XInterface >     m_xComListener;
    StarBASIC*                  m_pComListenerParentBasic;

    SbxVariableImpl()
        : m_pComListenerParentBasic( nullptr )
    {}
    SbxVariableImpl( const SbxVariableImpl& r )
        : m_aDeclareClassName( r.m_aDeclareClassName )
        , m_xComListener( r.m_xComListener )
        , m_pComListenerParentBasic( r.m_pComListenerParentBasic )
    {
    }
};


// Constructors

SbxVariable::SbxVariable() : SbxValue()
{
    mpSbxVariableImpl = nullptr;
    pCst = nullptr;
    pParent = nullptr;
    nUserData = 0;
    nHash = 0;
}

SbxVariable::SbxVariable( const SbxVariable& r )
    : SvRefBase( r ),
      SbxValue( r ),
      mpPar( r.mpPar ),
      pInfo( r.pInfo )
{
    mpSbxVariableImpl = nullptr;
    if( r.mpSbxVariableImpl != nullptr )
    {
        mpSbxVariableImpl = new SbxVariableImpl( *r.mpSbxVariableImpl );
#if HAVE_FEATURE_SCRIPTING
        if( mpSbxVariableImpl->m_xComListener.is() )
        {
            registerComListenerVariableForBasic( this, mpSbxVariableImpl->m_pComListenerParentBasic );
        }
#endif
    }
    pCst = nullptr;
    if( r.CanRead() )
    {
        pParent = r.pParent;
        nUserData = r.nUserData;
        maName = r.maName;
        nHash = r.nHash;
    }
    else
    {
        pParent = nullptr;
        nUserData = 0;
        nHash = 0;
    }
}

SbxVariable::SbxVariable( SbxDataType t, void* p ) : SbxValue( t, p )
{
    mpSbxVariableImpl = nullptr;
    pCst = nullptr;
    pParent = nullptr;
    nUserData = 0;
    nHash = 0;
}

SbxVariable::~SbxVariable()
{
#if HAVE_FEATURE_SCRIPTING
    if( IsSet( SbxFlagBits::DimAsNew ))
    {
        removeDimAsNewRecoverItem( this );
    }
#endif
    delete mpSbxVariableImpl;
    delete pCst;
}

// Broadcasting

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
// Then the copying will be dropped...

void SbxVariable::Broadcast( sal_uInt32 nHintId )
{
    if( pCst && !IsSet( SbxFlagBits::NoBroadcast ) )
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

        //fdo#86843 Add a ref during the following block to guard against
        //getting deleted before completing this method
        SbxVariableRef aBroadcastGuard(this);

        // Avoid further broadcasting
        SfxBroadcaster* pSave = pCst;
        pCst = nullptr;
        SbxFlagBits nSaveFlags = GetFlags();
        SetFlag( SbxFlagBits::ReadWrite );
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


// Name of the variables

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
    const_cast<SbxVariable*>(this)->GetInfo();
    // Append nothing, if it is a simple property (no empty brackets)
    if (!pInfo || (pInfo->m_Params.empty() && GetClass() == SbxCLASS_PROPERTY))
    {
        return maName;
    }
    sal_Unicode cType = ' ';
    OUString aTmp( maName );
    // short type? Then fetch it, possible this is 0.
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

    for (SbxParams::const_iterator iter = pInfo->m_Params.begin(); iter != pInfo->m_Params.end(); ++iter)
    {
        auto const& i = *iter;
        int nt = i->eType & 0x0FFF;
        if (iter != pInfo->m_Params.begin())
        {
            aTmp += ",";
        }
        if( i->nFlags & SbxFlagBits::Optional )
        {
            aTmp += OUString( SbxRes( STRING_OPTIONAL ) );
        }
        if( i->eType & SbxBYREF )
        {
            aTmp += OUString( SbxRes( STRING_BYREF ) );
        }
        aTmp += i->aName;
        cType = ' ';
        // short type? Then fetch it, possible this is 0.
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
    const_cast<SbxVariable*>(this)->aToolString = aTmp;
    return aToolString;
}

// Create a simple hashcode: the first six characters are evaluated.

sal_uInt16 SbxVariable::MakeHashCode( const OUString& rName )
{
    sal_uInt16 n = 0;
    sal_Int32 nLen = rName.getLength();
    if( nLen > 6 )
    {
        nLen = 6;
    }
    for( sal_Int32 i=0; i<nLen; ++i )
    {
        sal_uInt8 c = static_cast<sal_uInt8>(rName[i]);
        // If we have a comment sign break!!
        if( c >= 0x80 )
        {
            return 0;
        }
        n = sal::static_int_cast< sal_uInt16 >( ( n << 3 ) + toupper( c ) );
    }
    return n;
}

// Operators

SbxVariable& SbxVariable::operator=( const SbxVariable& r )
{
    SbxValue::operator=( r );
    delete mpSbxVariableImpl;
    if( r.mpSbxVariableImpl != nullptr )
    {
        mpSbxVariableImpl = new SbxVariableImpl( *r.mpSbxVariableImpl );
#if HAVE_FEATURE_SCRIPTING
        if( mpSbxVariableImpl->m_xComListener.is() )
        {
            registerComListenerVariableForBasic( this, mpSbxVariableImpl->m_pComListenerParentBasic );
        }
#endif
    }
    else
    {
        mpSbxVariableImpl = nullptr;
    }
    return *this;
}

// Conversion

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
    if( IsSet( SbxFlagBits::NoModify ) )
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
    if (p && dynamic_cast<SbxObject*>(this))
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

SbxVariableImpl* SbxVariable::getImpl()
{
    if( mpSbxVariableImpl == nullptr )
    {
        mpSbxVariableImpl = new SbxVariableImpl();
    }
    return mpSbxVariableImpl;
}

const OUString& SbxVariable::GetDeclareClassName()
{
    SbxVariableImpl* pImpl = getImpl();
    return pImpl->m_aDeclareClassName;
}

void SbxVariable::SetDeclareClassName( const OUString& rDeclareClassName )
{
    SbxVariableImpl* pImpl = getImpl();
    pImpl->m_aDeclareClassName = rDeclareClassName;
}

void SbxVariable::SetComListener( css::uno::Reference< css::uno::XInterface > xComListener,
                                  StarBASIC* pParentBasic )
{
    SbxVariableImpl* pImpl = getImpl();
    pImpl->m_xComListener = xComListener;
    pImpl->m_pComListenerParentBasic = pParentBasic;
#if HAVE_FEATURE_SCRIPTING
    registerComListenerVariableForBasic( this, pParentBasic );
#endif
}

void SbxVariable::ClearComListener()
{
    SbxVariableImpl* pImpl = getImpl();
    pImpl->m_xComListener.clear();
}


// Loading/Saving

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
            if( ImpScan( aTmpString, d, t, nullptr ) != ERRCODE_SBX_OK || t == SbxDOUBLE )
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
            if( ImpScan( aTmpString, aTmp.nDouble, t, nullptr ) != ERRCODE_SBX_OK )
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
            SAL_WARN( "basic", "Loaded a non-supported data type" );
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
    Broadcast( SBX_HINT_DATACHANGED );
    nHash =  MakeHashCode( maName );
    SetModified( true );
    return true;
}

bool SbxVariable::StoreData( SvStream& rStrm ) const
{
    rStrm.WriteUChar( 0xFF );      // Marker
    bool bValStore;
    if( dynamic_cast<const SbxMethod *>(this) != nullptr )
    {
        // #50200 Avoid that objects , which during the runtime
        // as return-value are saved in the method as a value were saved
        SbxVariable* pThis = const_cast<SbxVariable*>(this);
        SbxFlagBits nSaveFlags = GetFlags();
        pThis->SetFlag( SbxFlagBits::Write );
        pThis->SbxValue::Clear();
        pThis->SetFlags( nSaveFlags );

        // So that the method will not be executed in any case!
        // CAST, to avoid const!
        pThis->SetFlag( SbxFlagBits::NoBroadcast );
        bValStore = SbxValue::StoreData( rStrm );
        pThis->ResetFlag( SbxFlagBits::NoBroadcast );
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
    rStrm.WriteUInt32( nUserData );
    if( pInfo.Is() )
    {
        rStrm.WriteUChar( 2 );     // Version 2: with UserData!
        pInfo->StoreData( rStrm );
    }
    else
    {
        rStrm.WriteUChar( 0 );
    }
    return true;
}

// SbxInfo

SbxInfo::SbxInfo()
        : aHelpFile(), nHelpId(0)
{}

SbxInfo::SbxInfo( const OUString& r, sal_uInt32 n )
       : aHelpFile( r ), nHelpId( n )
{}

// SbxAlias

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

void SbxAlias::Broadcast( sal_uInt32 nHt )
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

void SbxAlias::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    const SbxHint* p = dynamic_cast<const SbxHint*>(&rHint);
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
        static_cast<SbxObject*>(GetValues_Impl().pObj)->Dump( rStrm, bFill );
    }
    else
    {
        rStrm << endl;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
