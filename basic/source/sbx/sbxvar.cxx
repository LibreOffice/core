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

#include <tools/stream.hxx>
#include <svl/SfxBroadcaster.hxx>

#include <basic/sbx.hxx>
#include <runtime.hxx>
#include "sbxres.hxx"
#include "sbxconv.hxx"
#include <rtlproto.hxx>
#include <sbunoobj.hxx>
#include <rtl/character.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>

#include <com/sun/star/uno/XInterface.hpp>
using namespace com::sun::star::uno;

// SbxVariable

SbxVariable::SbxVariable()
{
}

SbxVariable::SbxVariable( const SbxVariable& r )
    : SvRefBase( r ),
      SbxValue( r ),
      m_aDeclareClassName( r.m_aDeclareClassName ),
      m_xComListener( r.m_xComListener),
      mpPar( r.mpPar ),
      pInfo( r.pInfo )
{
#if HAVE_FEATURE_SCRIPTING
    if( r.m_xComListener.is() )
    {
        registerComListenerVariableForBasic( this, r.m_pComListenerParentBasic );
    }
#endif
    if( r.CanRead() )
    {
        pParent = r.pParent;
        nUserData = r.nUserData;
        maName = r.maName;
        nHash = r.nHash;
    }
}

SbxEnsureParentVariable::SbxEnsureParentVariable(const SbxVariable& r)
    : SbxVariable(r)
    , xParent(const_cast<SbxVariable&>(r).GetParent())
{
    assert(GetParent() == xParent.get());
}

void SbxEnsureParentVariable::SetParent(SbxObject* p)
{
    assert(GetParent() == xParent.get());
    SbxVariable::SetParent(p);
    xParent = SbxObjectRef(p);
    assert(GetParent() == xParent.get());
}

SbxVariable::SbxVariable( SbxDataType t ) : SbxValue( t )
{
}

SbxVariable::~SbxVariable()
{
#if HAVE_FEATURE_SCRIPTING
    if( IsSet( SbxFlagBits::DimAsNew ))
    {
        removeDimAsNewRecoverItem( this );
    }
#endif
    mpBroadcaster.reset();
}

// Broadcasting

SfxBroadcaster& SbxVariable::GetBroadcaster()
{
    if( !mpBroadcaster )
    {
        mpBroadcaster.reset( new SfxBroadcaster );
    }
    return *mpBroadcaster;
}

SbxArray* SbxVariable::GetParameters() const
{
    return mpPar.get();
}


// Perhaps some day one could cut the parameter 0.
// Then the copying will be dropped...

void SbxVariable::Broadcast( SfxHintId nHintId )
{
    if( !mpBroadcaster || IsSet( SbxFlagBits::NoBroadcast ) )
        return;

    // Because the method could be called from outside, check the
    // rights here again
    if( nHintId == SfxHintId::BasicDataWanted )
    {
        if( !CanRead() )
        {
            return;
        }
    }
    if( nHintId == SfxHintId::BasicDataChanged )
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
    std::unique_ptr<SfxBroadcaster> pSave = std::move(mpBroadcaster);
    SbxFlagBits nSaveFlags = GetFlags();
    SetFlag( SbxFlagBits::ReadWrite );
    if( mpPar.is() )
    {
        // Register this as element 0, but don't change over the parent!
        mpPar->GetRef(0) = this;
    }
    pSave->Broadcast( SbxHint( nHintId, this ) );
    mpBroadcaster = std::move(pSave);
    SetFlags( nSaveFlags );
}

SbxInfo* SbxVariable::GetInfo()
{
    if( !pInfo.is() )
    {
        Broadcast( SfxHintId::BasicInfoWanted );
        if( pInfo.is() )
        {
            SetModified( true );
        }
    }
    return pInfo.get();
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
    if( t == SbxNameType::NONE )
    {
        return maName;
    }
    // Request parameter-information (not for objects)
    const_cast<SbxVariable*>(this)->GetInfo();
    // Append nothing, if it is a simple property (no empty brackets)
    if (!pInfo.is() || (pInfo->m_Params.empty() && GetClass() == SbxClassType::Property))
    {
        return maName;
    }
    sal_Unicode cType = ' ';
    OUStringBuffer aTmp( maName );
    // short type? Then fetch it, possible this is 0.
    SbxDataType et = GetType();
    if( t == SbxNameType::ShortTypes )
    {
        if( et <= SbxSTRING )
        {
            cType = cSuffixes[ et ];
        }
        if( cType != ' ' )
        {
            aTmp.append(cType);
        }
    }
    aTmp.append("(");

    for (SbxParams::const_iterator iter = pInfo->m_Params.begin(); iter != pInfo->m_Params.end(); ++iter)
    {
        auto const& i = *iter;
        int nt = i->eType & 0x0FFF;
        if (iter != pInfo->m_Params.begin())
        {
            aTmp.append(",");
        }
        if( i->nFlags & SbxFlagBits::Optional )
        {
            aTmp.append( GetSbxRes( StringId::Optional ) );
        }
        if( i->eType & SbxBYREF )
        {
            aTmp.append( GetSbxRes( StringId::ByRef ) );
        }
        aTmp.append( i->aName );
        cType = ' ';
        // short type? Then fetch it, possible this is 0.
        if( t == SbxNameType::ShortTypes )
        {
            if( nt <= SbxSTRING )
            {
                cType = cSuffixes[ nt ];
            }
        }
        if( cType != ' ' )
        {
            aTmp.append(cType);
            if( i->eType & SbxARRAY )
            {
                aTmp.append("()");
            }
        }
        else
        {
            if( i->eType & SbxARRAY )
            {
                aTmp.append("()");
            }
            // long type?
            aTmp.append(GetSbxRes( StringId::As ));
            if( nt < 32 )
            {
                aTmp.append(GetSbxRes( static_cast<StringId>( static_cast<int>( StringId::Types ) + nt ) ));
            }
            else
            {
                aTmp.append(GetSbxRes( StringId::Any ));
            }
        }
    }
    aTmp.append(")");
    const_cast<SbxVariable*>(this)->aToolString = aTmp.makeStringAndClear();
    return aToolString;
}

// Operators

SbxVariable& SbxVariable::operator=( const SbxVariable& r )
{
    if (this != &r)
    {
        SbxValue::operator=( r );
        // tdf#144353 - copy information about a missing parameter. See SbiRuntime::SetIsMissing.
        if (r.pInfo && !dynamic_cast<const SbxMethod*>(&r))
            pInfo = r.pInfo;
        m_aDeclareClassName = r.m_aDeclareClassName;
        m_xComListener = r.m_xComListener;
        m_pComListenerParentBasic = r.m_pComListenerParentBasic;
#if HAVE_FEATURE_SCRIPTING
        if( m_xComListener.is() )
        {
            registerComListenerVariableForBasic( this, m_pComListenerParentBasic );
        }
#endif
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
    return SbxClassType::Variable;
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
            for (sal_uInt32 nIdx = 0; !bFound && nIdx < pChildren->Count(); ++nIdx)
            {
                bFound = (this == pChildren->Get(nIdx));
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

const OUString& SbxVariable::GetDeclareClassName() const
{
    return m_aDeclareClassName;
}

void SbxVariable::SetDeclareClassName( const OUString& rDeclareClassName )
{
    m_aDeclareClassName = rDeclareClassName;
}

void SbxVariable::SetComListener( const css::uno::Reference< css::uno::XInterface >& xComListener,
                                  StarBASIC* pParentBasic )
{
    m_xComListener = xComListener;
    m_pComListenerParentBasic = pParentBasic;
#if HAVE_FEATURE_SCRIPTING
    registerComListenerVariableForBasic( this, pParentBasic );
#endif
}

void SbxVariable::ClearComListener()
{
    m_xComListener.clear();
}


// Loading/Saving

bool SbxVariable::LoadData( SvStream& rStrm, sal_uInt16 nVer )
{
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
        sal_uInt16 nType;
        rStrm.SeekRel( -1 );
        rStrm.ReadUInt16( nType );
        maName = read_uInt16_lenPrefixed_uInt8s_ToOUString(rStrm,
                                                                RTL_TEXTENCODING_ASCII_US);
        sal_uInt32 nTemp;
        rStrm.ReadUInt32( nTemp );
        nUserData = nTemp;
        // correction: old methods have instead of SbxNULL now SbxEMPTY
        if( nType == SbxNULL && GetClass() == SbxClassType::Method )
        {
            nType = SbxEMPTY;
        }
        SbxValues aTmp;
        OUString aTmpString;
        OUString aVal;
        aTmp.eType = aData.eType = static_cast<SbxDataType>(nType);
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
            if( ImpScan( aTmpString, d, t, nullptr, !LibreOffice6FloatingPointMode() ) != ERRCODE_NONE || t == SbxDOUBLE )
            {
                aTmp.nSingle = 0;
                return false;
            }
            aTmp.nSingle = static_cast<float>(d);
            break;
        }
        case SbxDATE:
        case SbxDOUBLE:
        {
            // Floats as ASCII
            aTmpString = read_uInt16_lenPrefixed_uInt8s_ToOUString(rStrm,
                                                                        RTL_TEXTENCODING_ASCII_US);
            SbxDataType t;
            if( ImpScan( aTmpString, aTmp.nDouble, t, nullptr, !LibreOffice6FloatingPointMode() ) != ERRCODE_NONE )
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
            SAL_WARN( "basic.sbx", "Loaded a non-supported data type" );
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
        pInfo->LoadData( rStrm, static_cast<sal_uInt16>(cMark) );
    }
    Broadcast( SfxHintId::BasicDataChanged );
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
    if( pInfo.is() )
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
        :  nHelpId(0)
{}

SbxInfo::SbxInfo( const OUString& r, sal_uInt32 n )
       : aHelpFile( r ), nHelpId( n )
{}

void SbxVariable::Dump( SvStream& rStrm, bool bFill )
{
    OString aBNameStr(OUStringToOString(GetName( SbxNameType::ShortTypes ), RTL_TEXTENCODING_ASCII_US));
    rStrm.WriteCharPtr( "Variable( " )
         .WriteOString( OString::number(reinterpret_cast<sal_Int64>(this)) ).WriteCharPtr( "==" )
         .WriteOString( aBNameStr );
    OString aBParentNameStr(OUStringToOString(GetParent()->GetName(), RTL_TEXTENCODING_ASCII_US));
    if ( GetParent() )
    {
        rStrm.WriteCharPtr( " in parent '" ).WriteOString( aBParentNameStr ).WriteCharPtr( "'" );
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
