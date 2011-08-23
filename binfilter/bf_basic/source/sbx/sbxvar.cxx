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


#include <tools/stream.hxx>
#include "bf_svtools/brdcst.hxx"

#include "sbx.hxx"
#include "sbxbase.hxx"
#include "sbxres.hxx"
#include "sbxconv.hxx"
#include <math.h>
#include <ctype.h>

namespace binfilter {

///////////////////////////// SbxVariable //////////////////////////////

TYPEINIT1(SbxVariable,SbxValue)
TYPEINIT1(SbxHint,SfxSimpleHint)

extern UINT32 nVarCreator;			// in SBXBASE.CXX, fuer LoadData()
#ifdef DBG_UTIL
static ULONG nVar = 0;
#endif

///////////////////////////// Konstruktoren //////////////////////////////

SbxVariable::SbxVariable() : SbxValue()
{
    pCst = NULL;
    pParent = NULL;
    nUserData = 0;
    nHash = 0;
#ifdef DBG_UTIL
    DbgOutf( "SbxVariable::Ctor %lx=%ld", (void*)this, ++nVar );
    GetSbxData_Impl()->aVars.Insert( this, LIST_APPEND );
#endif
}

SbxVariable::SbxVariable( const SbxVariable& r )
           : SvRefBase( r ), SbxValue( r ), mpPar( r.mpPar ), pInfo( r.pInfo )
{
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
#ifdef DBG_UTIL
    static sal_Char const aCellsStr[] = "Cells";
    if ( maName.EqualsAscii( aCellsStr ) )
        maName.AssignAscii( aCellsStr, sizeof( aCellsStr )-1 );
    DbgOutf( "SbxVariable::Ctor %lx=%ld", (void*)this, ++nVar );
    GetSbxData_Impl()->aVars.Insert( this, LIST_APPEND );
#endif
}

SbxVariable::SbxVariable( SbxDataType t, void* p ) : SbxValue( t, p )
{
    pCst = NULL;
    pParent = NULL;
    nUserData = 0;
    nHash = 0;
#ifdef DBG_UTIL
    DbgOutf( "SbxVariable::Ctor %lx=%ld", (void*)this, ++nVar );
    GetSbxData_Impl()->aVars.Insert( this, LIST_APPEND );
#endif
}

SbxVariable::~SbxVariable()
{
#ifdef DBG_UTIL
    ByteString aBStr( (const UniString&)maName, RTL_TEXTENCODING_ASCII_US );
    DbgOutf( "SbxVariable::Dtor %lx (%s)", (void*)this, aBStr.GetBuffer() );
    static sal_Char const aCellsStr[] = "Cells";
    if ( maName.EqualsAscii( aCellsStr ) )
        maName.AssignAscii( aCellsStr, sizeof( aCellsStr )-1 );
    GetSbxData_Impl()->aVars.Remove( this );
#endif
    delete pCst;
}

////////////////////////////// Broadcasting //////////////////////////////

SfxBroadcaster& SbxVariable::GetBroadcaster()
{
    if( !pCst )
        pCst = new SfxBroadcaster;
    return *pCst;
}

// Eines Tages kann man vielleicht den Parameter 0 schleifen,
// dann entfaellt die Kopiererei...

void SbxVariable::Broadcast( ULONG nHintId )
{
    if( pCst && !IsSet( SBX_NO_BROADCAST ) && StaticIsEnabledBroadcasting() )
    {
        // Da die Methode von aussen aufrufbar ist, hier noch einmal
        // die Berechtigung testen
        if( nHintId & SBX_HINT_DATAWANTED )
            if( !CanRead() )
                return;
        if( nHintId & SBX_HINT_DATACHANGED )
            if( !CanWrite() )
                return;
        // Weitere Broadcasts verhindern
        SfxBroadcaster* pSave = pCst;
        pCst = NULL;
        USHORT nSaveFlags = GetFlags();
        SetFlag( SBX_READWRITE );
        if( mpPar.Is() )
            // this, als Element 0 eintragen, aber den Parent nicht umsetzen!
            mpPar->GetRef( 0 ) = this;
        pSave->Broadcast( SbxHint( nHintId, this ) );
        delete pCst; // wer weiss schon, auf welche Gedanken mancher kommt?
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
            SetModified( TRUE );
    }
    return pInfo;
}

void SbxVariable::SetParameters( SbxArray* p )
{
    mpPar = p;
}


/////////////////////////// Name der Variablen ///////////////////////////

void SbxVariable::SetName( const XubString& rName )
{
    maName = rName;
    nHash = MakeHashCode( rName );
}

const XubString& SbxVariable::GetName( SbxNameType t ) const
{
    static char cSuffixes[] = "  %&!#@ $";
    if( t == SbxNAME_NONE )
        return maName;
    // Parameter-Infos anfordern (nicht fuer Objekte)
    ((SbxVariable*)this)->GetInfo();
    // Nix anfuegen, wenn einfache Property (keine leeren Klammern)
    if( !pInfo
     || ( !pInfo->aParams.Count() && GetClass() == SbxCLASS_PROPERTY ) )
        return maName;
    xub_Unicode cType = ' ';
    XubString aTmp( maName );
    // Kurzer Typ? Dann holen, evtl. ist dieser 0.
    SbxDataType et = GetType();
    if( t == SbxNAME_SHORT_TYPES )
    {
        if( et <= SbxSTRING )
            cType = cSuffixes[ et ];
        if( cType != ' ' )
            aTmp += cType;
    }
    aTmp += '(';
    for( USHORT i = 0; i < pInfo->aParams.Count(); i++ )
    {
        const SbxParamInfo* q = pInfo->aParams.GetObject( i );
        int nt = q->eType & 0x0FFF;
        if( i )
            aTmp += ',';
        if( q->nFlags & SBX_OPTIONAL )
            aTmp += SbxRes( STRING_OPTIONAL );
        if( q->eType & SbxBYREF )
            aTmp += SbxRes( STRING_BYREF );
        aTmp += q->aName;
        cType = ' ';
        // Kurzer Typ? Dann holen, evtl. ist dieser 0.
        if( t == SbxNAME_SHORT_TYPES )
        {
            if( nt <= SbxSTRING )
                cType = cSuffixes[ nt ];
        }
        if( cType != ' ' )
        {
            aTmp += cType;
            if( q->eType & SbxARRAY )
                aTmp.AppendAscii( "()" );
        }
        else
        {
            if( q->eType & SbxARRAY )
                aTmp.AppendAscii( "()" );
            // langer Typ?
            if( t != SbxNAME_SHORT )
            {
                aTmp += SbxRes( STRING_AS );
                if( nt < 32 )
                    aTmp += SbxRes(
                        sal::static_int_cast< USHORT >( STRING_TYPES + nt ) );
                else
                    aTmp += SbxRes( STRING_ANY );
            }
        }
    }
    aTmp += ')';
    // Langer Typ? Dann holen
    if( t == SbxNAME_LONG_TYPES && et != SbxEMPTY )
    {
        aTmp += SbxRes( STRING_AS );
        if( et < 32 )
            aTmp += SbxRes(
                sal::static_int_cast< USHORT >( STRING_TYPES + et ) );
        else
            aTmp += SbxRes( STRING_ANY );
    }
    ((SbxVariable*) this)->aPic = aTmp;
    return aPic;
}

// Einen simplen Hashcode erzeugen: Es werden die ersten 6 Zeichen gewertet.

USHORT SbxVariable::MakeHashCode( const XubString& rName )
{
    USHORT n = 0;
    USHORT nLen = rName.Len();
    if( nLen > 6 )
        nLen = 6;
    const xub_Unicode* p = rName.GetBuffer();
    while( nLen-- )
    {
        BYTE c = (BYTE)*p;
        p++;
        // Falls wir ein Schweinezeichen haben, abbrechen!!
        if( c >= 0x80 )
            return 0;
        n = sal::static_int_cast< USHORT >( ( n << 3 ) + toupper( c ) );
    }
    return n;
}

////////////////////////////// Operatoren ////////////////////////////////

SbxVariable& SbxVariable::operator=( const SbxVariable& r )
{
    SbxValue::operator=( r );
    return *this;
}

//////////////////////////////// Konversion ////////////////////////////////

SbxDataType SbxVariable::GetType() const
{
    if( aData.eType == SbxOBJECT )
        return aData.pObj ? aData.pObj->GetType() : SbxOBJECT;
    else if( aData.eType == SbxVARIANT )
        return aData.pObj ? aData.pObj->GetType() : SbxVARIANT;
    else
        return aData.eType;
}

SbxClassType SbxVariable::GetClass() const
{
    return SbxCLASS_VARIABLE;
}

void SbxVariable::SetModified( BOOL b )
{
    if( IsSet( SBX_NO_MODIFY ) )
        return;
    SbxBase::SetModified( b );
    if( pParent && pParent != this ) //??? HotFix: Rekursion raus MM
        pParent->SetModified( b );
}

void SbxVariable::SetParent( SbxObject* p )
{
#ifdef DBG_UTIL
    // wird der Parent eines SbxObjects gesetzt?
    if ( p && ISA(SbxObject) )
    {
        // dann mu\s dieses auch Child vom neuen Parent sein
        BOOL bFound = FALSE;
        SbxArray *pChilds = p->GetObjects();
        if ( pChilds )
        {
            for ( USHORT nIdx = 0; !bFound && nIdx < pChilds->Count(); ++nIdx )
                bFound = ( this == pChilds->Get(nIdx) );
        }
        if ( !bFound )
        {
            String aMsg = String::CreateFromAscii( "dangling: [" );
            aMsg += GetName();
            aMsg.AppendAscii( "].SetParent([" );
            aMsg += p->GetName();
            aMsg.AppendAscii( "])" );
            ByteString aBStr( (const UniString&)aMsg, RTL_TEXTENCODING_ASCII_US );
            DbgOut( aBStr.GetBuffer(), DBG_OUT_WARNING, __FILE__, __LINE__);
        }
    }
#endif

    pParent = p;
}

////////////////////////////// Laden/Speichern /////////////////////////////

BOOL SbxVariable::LoadData( SvStream& rStrm, USHORT nVer )
{
    UINT16 nType;
    BYTE cMark;
    rStrm >> cMark;
    if( cMark == 0xFF )
    {
        if( !SbxValue::LoadData( rStrm, nVer ) )
            return FALSE;
        rStrm.ReadByteString( maName, RTL_TEXTENCODING_ASCII_US );
        rStrm >> nUserData;
    }
    else
    {
        rStrm.SeekRel( -1L );
        rStrm >> nType;
        rStrm.ReadByteString( maName, RTL_TEXTENCODING_ASCII_US );
        rStrm >> nUserData;
        // Korrektur: Alte Methoden haben statt SbxNULL jetzt SbxEMPTY
        if( nType == SbxNULL && GetClass() == SbxCLASS_METHOD )
            nType = SbxEMPTY;
        SbxValues aTmp;
        XubString aVal;
        aTmp.eType = aData.eType = (SbxDataType) nType;
        aTmp.pString = &aVal;
        switch( nType )
        {
            case SbxBOOL:
            case SbxERROR:
            case SbxINTEGER:
                rStrm >> aTmp.nInteger; break;
            case SbxLONG:
                rStrm >> aTmp.nLong; break;
            case SbxSINGLE:
            {
                // Floats als ASCII
                rStrm.ReadByteString( aVal, RTL_TEXTENCODING_ASCII_US );
                double d;
                SbxDataType t;
                if( ImpScan( aVal, d, t, NULL ) != SbxERR_OK || t == SbxDOUBLE )
                {
                    aTmp.nSingle = 0;
                    return FALSE;
                }
                aTmp.nSingle = (float) d;
                break;
            }
            case SbxDATE:
            case SbxDOUBLE:
            {
                // Floats als ASCII
                rStrm.ReadByteString( aVal, RTL_TEXTENCODING_ASCII_US );
                SbxDataType t;
                if( ImpScan( aVal, aTmp.nDouble, t, NULL ) != SbxERR_OK )
                {
                    aTmp.nDouble = 0;
                    return FALSE;
                }
                break;
            }
            case SbxSTRING:
                rStrm.ReadByteString( aVal, RTL_TEXTENCODING_ASCII_US );
                break;
            case SbxEMPTY:
            case SbxNULL:
                break;
            default:
                aData.eType = SbxNULL;
                DBG_ASSERT( !this, "Nicht unterstuetzer Datentyp geladen" );
                return FALSE;
        }
        // Wert putten
        if( nType != SbxNULL && nType != SbxEMPTY && !Put( aTmp ) )
            return FALSE;
    }
    rStrm >> cMark;
    // cMark ist auch eine Versionsnummer!
    // 1: initial version
    // 2: mit nUserData
    if( cMark )
    {
        if( cMark > 2 )
            return FALSE;
        pInfo = new SbxInfo;
        pInfo->LoadData( rStrm, (USHORT) cMark );
    }
    // Privatdaten nur laden, wenn es eine SbxVariable ist
    if( GetClass() == SbxCLASS_VARIABLE && !LoadPrivateData( rStrm, nVer ) )
        return FALSE;
    ((SbxVariable*) this)->Broadcast( SBX_HINT_DATACHANGED );
    nHash =  MakeHashCode( maName );
    SetModified( TRUE );
    return TRUE;
}

BOOL SbxVariable::StoreData( SvStream& rStrm ) const
{
    rStrm << (BYTE) 0xFF;		// Marker
    BOOL bValStore;
    if( this->IsA( TYPE(SbxMethod) ) )
    {
        // #50200 Verhindern, dass Objekte, die zur Laufzeit als Return-Wert
        // in der Methode als Value gespeichert sind, mit gespeichert werden
        SbxVariable* pThis = (SbxVariable*)this;
        USHORT nSaveFlags = GetFlags();
        pThis->SetFlag( SBX_WRITE );
        pThis->SbxValue::Clear();
        pThis->SetFlags( nSaveFlags );

        // Damit die Methode in keinem Fall ausgefuehrt wird!
        // CAST, um const zu umgehen!
        pThis->SetFlag( SBX_NO_BROADCAST );
        bValStore = SbxValue::StoreData( rStrm );
        pThis->ResetFlag( SBX_NO_BROADCAST );
    }
    else
        bValStore = SbxValue::StoreData( rStrm );
    if( !bValStore )
        return FALSE;
    // if( !SbxValue::StoreData( rStrm ) )
        // return FALSE;
    rStrm.WriteByteString( maName, RTL_TEXTENCODING_ASCII_US );
    rStrm << nUserData;
    if( pInfo.Is() )
    {
        rStrm << (BYTE) 2;		// Version 2: mit UserData!
        pInfo->StoreData( rStrm );
    }
    else
        rStrm << (BYTE) 0;
    // Privatdaten nur speichern, wenn es eine SbxVariable ist
    if( GetClass() == SbxCLASS_VARIABLE )
        return StorePrivateData( rStrm );
    else
        return TRUE;
}

////////////////////////////// SbxInfo ///////////////////////////////////

SbxInfo::SbxInfo() : aHelpFile(), nHelpId( 0 ), aParams()
{}

////////////////////////////// SbxAlias //////////////////////////////////

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
        EndListening( xAlias->GetBroadcaster() );
}

void SbxAlias::Broadcast( ULONG nHt )
{
    if( xAlias.Is() && StaticIsEnabledBroadcasting() )
    {
        xAlias->SetParameters( GetParameters() );
        if( nHt == SBX_HINT_DATAWANTED )
            SbxVariable::operator=( *xAlias );
        else if( nHt == SBX_HINT_DATACHANGED || nHt == SBX_HINT_CONVERTED )
            *xAlias = *this;
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
        // Alias loeschen?
        if( pParent )
            pParent->Remove( this );
    }
}

void SbxVariable::Dump( SvStream& rStrm, BOOL bFill )
{
    ByteString aBNameStr( (const UniString&)GetName( SbxNAME_SHORT_TYPES ), RTL_TEXTENCODING_ASCII_US );
    rStrm << "Variable( "
          << ByteString::CreateFromInt64( (ULONG) this ).GetBuffer() << "=="
          << aBNameStr.GetBuffer();
    ByteString aBParentNameStr( (const UniString&)GetParent()->GetName(), RTL_TEXTENCODING_ASCII_US );
    if ( GetParent() )
        rStrm << " in parent '" << aBParentNameStr.GetBuffer() << "'";
    else
        rStrm << " no parent";
    rStrm << " ) ";

    // bei Object-Vars auch das Object ausgeben
    if ( GetValues_Impl().eType == SbxOBJECT &&
            GetValues_Impl().pObj &&
            GetValues_Impl().pObj != this &&
            GetValues_Impl().pObj != GetParent() )
    {
        rStrm << " contains ";
        ((SbxObject*) GetValues_Impl().pObj)->Dump( rStrm, bFill );
    }
    else
        rStrm << endl;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
