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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

#include <tools/debug.hxx>
#include <tools/pstm.hxx>

#define STOR_NO_OPTIMIZE

/***********************************************************************/
/************************************************************************
|*    SvClassManager::Register()
*************************************************************************/
void SvClassManager::Register( USHORT nClassId, SvCreateInstancePersist pFunc )
{
#ifdef DBG_UTIL
    SvCreateInstancePersist p;
    p = Get( nClassId );
    DBG_ASSERT( !p || p == pFunc, "register class with same id" );
#endif
    aAssocTable.insert(Map::value_type(nClassId, pFunc));
}

/************************************************************************
|*    SvClassManager::Get()
*************************************************************************/
SvCreateInstancePersist SvClassManager::Get( USHORT nClassId )
{
    Map::const_iterator i(aAssocTable.find(nClassId));
    return i == aAssocTable.end() ? 0 : i->second;
}

/****************** SvRttiBase *******************************************/
TYPEINIT0( SvRttiBase );

/****************** SvPersistBaseMemberList ******************************/

SvPersistBaseMemberList::SvPersistBaseMemberList(){}
SvPersistBaseMemberList::SvPersistBaseMemberList(
    USHORT nInitSz, USHORT nResize )
    : SuperSvPersistBaseMemberList( nInitSz, nResize ){}

#define PERSIST_LIST_VER        (BYTE)0
#define PERSIST_LIST_DBGUTIL    (BYTE)0x80

/************************************************************************
|*    SvPersistBaseMemberList::WriteOnlyStreamedObjects()
*************************************************************************/
void SvPersistBaseMemberList::WriteObjects( SvPersistStream & rStm,
                                            BOOL bOnlyStreamed ) const
{
#ifdef STOR_NO_OPTIMIZE
    rStm << (BYTE)(PERSIST_LIST_VER | PERSIST_LIST_DBGUTIL);
    UINT32 nObjPos = rStm.WriteDummyLen();
#else
    BYTE bTmp = PERSIST_LIST_VER;
    rStm << bTmp;
#endif
    UINT32 nCountMember = Count();
    ULONG  nCountPos = rStm.Tell();
    UINT32 nWriteCount = 0;
    rStm << nCountMember;
    //bloss die Liste nicht veraendern,
    //wegen Seiteneffekten beim Save
    for( ULONG n = 0; n < nCountMember; n++ )
    {
        SvPersistBase * pObj = GetObject( n );
        if( !bOnlyStreamed || rStm.IsStreamed( pObj ) )
        { // Objekt soll geschrieben werden
            rStm << GetObject( n );
            nWriteCount++;
        }
    }
    if( nWriteCount != nCountMember )
    {
        // nicht alle Objekte geschrieben, Count anpassen
        ULONG nPos = rStm.Tell();
        rStm.Seek( nCountPos );
        rStm << nWriteCount;
        rStm.Seek( nPos );
    }
#ifdef STOR_NO_OPTIMIZE
    rStm.WriteLen( nObjPos );
#endif
}

/************************************************************************
|*    operator << ()
*************************************************************************/
SvPersistStream& operator << ( SvPersistStream & rStm,
                               const SvPersistBaseMemberList & rLst )
{
    rLst.WriteObjects( rStm );
    return rStm;
}

/************************************************************************
|*    operator >> ()
*************************************************************************/
SvPersistStream& operator >> ( SvPersistStream & rStm,
                               SvPersistBaseMemberList & rLst )
{
    BYTE nVer;
    rStm >> nVer;

    if( (nVer & ~PERSIST_LIST_DBGUTIL) != PERSIST_LIST_VER )
    {
        rStm.SetError( SVSTREAM_GENERALERROR );
        OSL_FAIL( "persist list, false version" );
    }

    UINT32 nObjLen(0), nObjPos(0);
    if( nVer & PERSIST_LIST_DBGUTIL )
        nObjLen = rStm.ReadLen( &nObjPos );

    sal_uInt32 nCount;
    rStm >> nCount;
    for( ULONG n = 0; n < nCount && rStm.GetError() == SVSTREAM_OK; n++ )
    {
        SvPersistBase * pObj;
        rStm >> pObj;
        if( pObj )
            rLst.Append( pObj );
    }
#ifdef DBG_UTIL
            if( nObjLen + nObjPos != rStm.Tell() )
            {
                ByteString aStr( "false list len: read = " );
                aStr += ByteString::CreateFromInt32( (long)(rStm.Tell() - nObjPos) );
                aStr += ", should = ";
                aStr += ByteString::CreateFromInt64(nObjLen);
                OSL_FAIL( aStr.GetBuffer() );
            }
#else
            (void)nObjLen;
#endif
    return rStm;
}

//=========================================================================
SvPersistStream::SvPersistStream
(
    SvClassManager & rMgr,  /* Alle Factorys, deren Objekt geladen und
                               gespeichert werdn k"onnen */
    SvStream * pStream,     /* Dieser Stream wird als Medium genommen, auf
                               dem der PersistStream arbeitet */
    UINT32 nStartIdxP       /* Ab diesem Index werden die Id's f"ur
                               die Objekte vergeben, er muss gr"osser
                               als Null sein. */
)
    : rClassMgr( rMgr )
    , pStm( pStream )
    , aPUIdx( nStartIdxP )
    , nStartIdx( nStartIdxP )
    , pRefStm( NULL )
    , nFlags( 0 )
/*  [Beschreibung]

    Der Konstruktor der Klasse SvPersistStream. Die Objekte rMgr und
    pStream d"urfen nicht ver"andert werden, solange sie in einem
    SvPersistStream eingesetzt sind. Eine Aussnahme gibt es f"ur
    pStream (siehe <SvPersistStream::SetStream>).
*/
{
    DBG_ASSERT( nStartIdx != 0, "zero index not allowed" );
    bIsWritable = TRUE;
    if( pStm )
    {
        SetVersion( pStm->GetVersion() );
        SetError( pStm->GetError() );
        SyncSvStream( pStm->Tell() );
    }
}

//=========================================================================
SvPersistStream::SvPersistStream
(
    SvClassManager & rMgr,  /* Alle Factorys, deren Objekt geladen und
                               gespeichert werdn k"onnen */
    SvStream * pStream,     /* Dieser Stream wird als Medium genommen, auf
                               dem der PersistStream arbeitet */
    const SvPersistStream & rPersStm
                            /* Wenn PersistStream's verschachtelt werden,
                               dann ist dies der Parent-Stream. */
)
    : rClassMgr( rMgr )
    , pStm( pStream )
    // Bereiche nicht ueberschneiden, deshalb nur groessere Indexe
    , aPUIdx( rPersStm.GetCurMaxIndex() +1 )
    , nStartIdx( rPersStm.GetCurMaxIndex() +1 )
    , pRefStm( &rPersStm )
    , nFlags( 0 )
/*  [Beschreibung]

    Der Konstruktor der Klasse SvPersistStream. Die Objekte rMgr und
    pStream d"urfen nicht ver"andert werden, solange sie in einem
    SvPersistStream eingesetzt sind. Eine Aussnahme gibt es f"ur
    pStream (siehe <SvPersistStream::SetStream>).
    Durch diesen Konstruktor wird eine Hierarchiebildung unterst"utzt.
    Alle Objekte aus einer Hierarchie m"ussen erst geladen werden,
    wenn das erste aus dieser Hierarchie benutzt werden soll.
*/
{
    bIsWritable = TRUE;
    if( pStm )
    {
        SetVersion( pStm->GetVersion() );
        SetError( pStm->GetError() );
        SyncSvStream( pStm->Tell() );
    }
}

//=========================================================================
SvPersistStream::~SvPersistStream()
/*  [Beschreibung]

    Der Detruktor ruft die Methode <SvPersistStream::SetStream>
    mit NULL.
*/
{
    SetStream( NULL );
}

//=========================================================================
void SvPersistStream::SetStream
(
    SvStream * pStream  /* auf diesem Stream arbeitet der PersistStream */

)
/*  [Beschreibung]

    Es wird ein Medium (pStream) eingesetzt, auf dem PersistStream arbeitet.
    Dieses darf nicht von aussen modifiziert werden, solange es
     eingesetzt ist. Es sei denn, w"ahrend auf dem Medium gearbeitet
    wird, wird keine Methode von SvPersistStream gerufen, bevor
    nicht <SvPersistStream::SetStream> mit demselben Medium gerufen
    wurde.
*/
{
    if( pStm != pStream )
    {
        if( pStm )
        {
            SyncSysStream();
            pStm->SetError( GetError() );
        }
        pStm = pStream;
    }
    if( pStm )
    {
        SetVersion( pStm->GetVersion() );
        SetError( pStm->GetError() );
        SyncSvStream( pStm->Tell() );
    }
}

//=========================================================================
USHORT SvPersistStream::IsA() const
/*  [Beschreibung]

    Gibt den Identifier dieses Streamklasse zur"uck.

    [R"uckgabewert]

    USHORT      ID_PERSISTSTREAM wird zur"uckgegeben.


    [Querverweise]

    <SvStream::IsA>
*/
{
    return ID_PERSISTSTREAM;
}


/*************************************************************************
|*    SvPersistStream::ResetError()
*************************************************************************/
void SvPersistStream::ResetError()
{
    SvStream::ResetError();
    DBG_ASSERT( pStm, "stream not set" );
    pStm->ResetError();
}

/*************************************************************************
|*    SvPersistStream::GetData()
*************************************************************************/
ULONG SvPersistStream::GetData( void* pData, ULONG nSize )
{
    DBG_ASSERT( pStm, "stream not set" );
    ULONG nRet = pStm->Read( pData, nSize );
    SetError( pStm->GetError() );
    return nRet;
}

/*************************************************************************
|*    SvPersistStream::PutData()
*************************************************************************/
ULONG SvPersistStream::PutData( const void* pData, ULONG nSize )
{
    DBG_ASSERT( pStm, "stream not set" );
    ULONG nRet = pStm->Write( pData, nSize );
    SetError( pStm->GetError() );
    return nRet;
}

/*************************************************************************
|*    SvPersistStream::Seek()
*************************************************************************/
ULONG SvPersistStream::SeekPos( ULONG nPos )
{
    DBG_ASSERT( pStm, "stream not set" );
    ULONG nRet = pStm->Seek( nPos );
    SetError( pStm->GetError() );
    return nRet;
}

/*************************************************************************
|*    SvPersistStream::FlushData()
*************************************************************************/
void SvPersistStream::FlushData()
{
}

/*************************************************************************
|*    SvPersistStream::GetCurMaxIndex()
*************************************************************************/
ULONG SvPersistStream::GetCurMaxIndex( const SvPersistUIdx & rIdx ) const
{
    // const  bekomme ich nicht den hoechsten Index
    SvPersistUIdx * p = (SvPersistUIdx *)&rIdx;
    // alten merken
    ULONG nCurIdx = p->GetCurIndex();
    p->Last();
    // Bereiche nicht ueberschneiden, deshalb nur groessere Indexe
    ULONG nMaxIdx = p->GetCurIndex();
    // wieder herstellen
    p->Seek( nCurIdx );
    return nMaxIdx;
}

/*************************************************************************
|*    SvPersistStream::GetIndex()
*************************************************************************/
ULONG SvPersistStream::GetIndex( SvPersistBase * pObj ) const
{
    ULONG nId = (ULONG)aPTable.Get( (ULONG)pObj );
    if( !nId && pRefStm )
        return pRefStm->GetIndex( pObj );
    return nId;
}

/*************************************************************************
|*    SvPersistStream::GetObject)
*************************************************************************/
SvPersistBase * SvPersistStream::GetObject( ULONG nIdx ) const
{
    if( nIdx >= nStartIdx )
        return aPUIdx.Get( nIdx );
    else if( pRefStm )
        return pRefStm->GetObject( nIdx );
    return NULL;
}

//=========================================================================
#define LEN_1           0x80
#define LEN_2           0x40
#define LEN_4           0x20
#define LEN_5           0x10
UINT32 SvPersistStream::ReadCompressed
(
    SvStream & rStm /* Aus diesem Stream werden die komprimierten Daten
                       gelesen */
)
/*  [Beschreibung]

    Ein im Stream komprimiert abgelegtes Wort wird gelesen. In welchem
    Format komprimiert wird, siehe <SvPersistStream::WriteCompressed>.

    [R"uckgabewert]

    UINT32      Das nicht komprimierte Wort wird zur"uckgegeben.

    [Querverweise]

*/
{
    UINT32 nRet(0);
    BYTE    nMask;
    rStm >> nMask;
    if( nMask & LEN_1 )
        nRet = ~LEN_1 & nMask;
    else if( nMask & LEN_2 )
    {
        nRet = ~LEN_2 & nMask;
        nRet <<= 8;
        rStm >> nMask;
        nRet |= nMask;
    }
    else if( nMask & LEN_4 )
    {
        nRet = ~LEN_4 & nMask;
        nRet <<= 8;
        rStm >> nMask;
        nRet |= nMask;
        nRet <<= 16;
        USHORT n;
        rStm >> n;
        nRet |= n;
    }
    else if( nMask & LEN_5 )
    {
        if( nMask & 0x0F )
        {
            rStm.SetError( SVSTREAM_FILEFORMAT_ERROR );
            OSL_FAIL( "format error" );
        }
        rStm >> nRet;
    }
    else
    {
        rStm.SetError( SVSTREAM_FILEFORMAT_ERROR );
        OSL_FAIL( "format error" );
    }
    return nRet;
}

//=========================================================================
void SvPersistStream::WriteCompressed
(
    SvStream & rStm,/* Aus diesem Stream werden die komprimierten Daten
                       gelesen */
    UINT32 nVal     /* Dieser Wert wird komprimiert geschrieben */
)
/*  [Beschreibung]

    Das "ubergebene Wort wird komprimiert und in den Stream
     geschrieben. Folgendermassen wir komprimiert.
    nVal < 0x80         =>  0x80        + nVal ist 1 Byte gross.
    nVal < 0x4000       =>  0x4000      + nVal ist 2 Byte gross.
    nVal < 0x20000000   =>  0x20000000  + nVal ist 4 Byte gross.
    nVal > 0x1FFFFFFF   =>  0x1000000000+ nVal ist 5 Byte gross.

    [Querverweise]

    <SvPersistStream::ReadCompressed>
*/
{
#ifdef STOR_NO_OPTIMIZE
    if( nVal < 0x80 )
        rStm << (BYTE)(LEN_1 | nVal);
    else if( nVal < 0x4000 )
    {
        rStm << (BYTE)(LEN_2 | (nVal >> 8));
        rStm << (BYTE)nVal;
    }
    else if( nVal < 0x20000000 )
    {
        // hoechstes BYTE
        rStm << (BYTE)(LEN_4 | (nVal >> 24));
        // 2. hoechstes BYTE
        rStm << (BYTE)(nVal >> 16);
        rStm << (USHORT)(nVal);
    }
    else
#endif
    {
        rStm << (BYTE)LEN_5;
        rStm << nVal;
    }
}

//=========================================================================
UINT32 SvPersistStream::WriteDummyLen()
/*  [Beschreibung]

    Die Methode schreibt 4 Byte in den Stream und gibt die Streamposition
    zur"uck.

    [R"uckgabewert]

    UINT32      Die Position hinter der L"angenangabe wird zur"uckgegeben.

    [Beispiel]

    UINT32 nObjPos = rStm.WriteDummyLen();
    ...
    // Daten schreiben
    ...
    rStm.WriteLen( nObjPos );

    [Querverweise]

    <SvPersistStream::ReadLen>, <SvPersistStream::WriteLen>

*/
{
#ifdef DBG_UTIL
    UINT32 nPos = Tell();
#endif
    UINT32 n0 = 0;
    *this << n0; // wegen Sun sp
    // keine Assertion bei Streamfehler
    DBG_ASSERT( GetError() != SVSTREAM_OK
                  || (sizeof( UINT32 ) == Tell() -nPos),
                "keine 4-Byte fuer Langenangabe" );
    return Tell();
}

//=========================================================================
void SvPersistStream::WriteLen
(
    UINT32 nObjPos  /* die Position + 4, an der die L"ange geschrieben
                       wird. */
)
/*  [Beschreibung]

    Die Methode schreibt die Differenz zwischen der aktuellen und
     nObjPos als UINT32 an die Position nObjPos -4 im Stream. Danach
    wird der Stream wieder auf die alte Position gestellt.

    [Beispiel]

    Die Differenz enth"alt nicht die L"angenangabe.

    UINT32 nObjPos = rStm.WriteDummyLen();
    ...
    // Daten schreiben
    ...
    rStm.WriteLen( nObjPos );
    // weitere Daten schreiben

    [Querverweise]

    <SvPersistStream::ReadLen>, <SvPersistStream::WriteDummyLen>
*/
{
    UINT32 nPos = Tell();
    UINT32 nLen = nPos - nObjPos;
    // die Laenge mu� im stream 4-Byte betragen
    Seek( nObjPos - sizeof( UINT32 ) );
    // Laenge schreiben
    *this << nLen;
    Seek( nPos );
}

//=========================================================================
UINT32 SvPersistStream::ReadLen
(
    UINT32 * pTestPos   /* Die Position des Streams, nach dem Lesen der
                           L"ange, wird zur"uckgegeben. Es darf auch NULL
                           "ubergeben werden. */
)
/*  [Beschreibung]

    Liest die L"ange die vorher mit <SvPersistStream::WriteDummyLen>
    und <SvPersistStream::WriteLen> geschrieben wurde.
*/
{
    UINT32 nLen;
    *this >> nLen;
    if( pTestPos )
        *pTestPos = Tell();
    return nLen;
}

//=========================================================================
// Dateirormat abw"arts kompatibel
#ifdef STOR_NO_OPTIMIZE
#define P_VER       (BYTE)0x00
#else
#define P_VER       (BYTE)0x01
#endif
#define P_VER_MASK  (BYTE)0x0F
#define P_ID_0      (BYTE)0x80
#define P_OBJ       (BYTE)0x40
#define P_DBGUTIL   (BYTE)0x20
#define P_ID        (BYTE)0x10
#ifdef STOR_NO_OPTIMIZE
#define P_STD   P_DBGUTIL
#else
#define P_STD   0
#endif

static void WriteId
(
    SvStream & rStm,
    BYTE nHdr,
    UINT32 nId,
    USHORT nClassId
)
{
#ifdef STOR_NO_OPTIMIZE
    nHdr |= P_ID;
#endif
    nHdr |= P_VER;
    if( nHdr & P_ID )
    {
        if( (nHdr & P_OBJ) || nId != 0 )
        { // Id nur bei Zeiger, oder DBGUTIL
            rStm << (BYTE)(nHdr);
            SvPersistStream::WriteCompressed( rStm, nId );
        }
        else
        { // NULL Pointer
            rStm << (BYTE)(nHdr | P_ID_0);
            return;
        }
    }
    else
        rStm << nHdr;

    if( (nHdr & P_DBGUTIL) || (nHdr & P_OBJ) )
        // Objekte haben immer eine Klasse,
        // Pointer nur bei DBG_UTIL und != NULL
        SvPersistStream::WriteCompressed( rStm, nClassId );
}

//=========================================================================
static void ReadId
(
    SvStream & rStm,
    BYTE & nHdr,
    UINT32 & nId,
    USHORT & nClassId
)
{
    nClassId = 0;
    rStm >> nHdr;
    if( nHdr & P_ID_0 )
        nId = 0;
    else
    {
        if( (nHdr & P_VER_MASK) == 0 )
        {
            if( (nHdr & P_DBGUTIL) || !(nHdr & P_OBJ) )
                nId = SvPersistStream::ReadCompressed( rStm );
            else
                nId = 0;
        }
        else if( nHdr & P_ID )
            nId = SvPersistStream::ReadCompressed( rStm );

        if( (nHdr & P_DBGUTIL) || (nHdr & P_OBJ) )
            nClassId = (USHORT)SvPersistStream::ReadCompressed( rStm );
    }
}

//=========================================================================
void SvPersistStream::WriteObj
(
    BYTE nHdr,
    SvPersistBase * pObj
)
{
#ifdef STOR_NO_OPTIMIZE
    UINT32 nObjPos = 0;
    if( nHdr & P_DBGUTIL )
        // Position fuer Laenge merken
        nObjPos = WriteDummyLen();
#endif
    pObj->Save( *this );
#ifdef STOR_NO_OPTIMIZE
    if( nHdr & P_DBGUTIL )
        WriteLen( nObjPos );
#endif
}

//=========================================================================
SvPersistStream& SvPersistStream::WritePointer
(
    SvPersistBase * pObj
)
{
    BYTE nP = P_STD;

    if( pObj )
    {
        ULONG nId = GetIndex( pObj );
        if( nId )
            nP |= P_ID;
        else
        {
            nId = aPUIdx.Insert( pObj );
            aPTable.Insert( (ULONG)pObj, (void *)nId );
            nP |= P_OBJ;
        }
        WriteId( *this, nP, nId, pObj->GetClassId() );
        if( nP & P_OBJ )
            WriteObj( nP, pObj );
    }
    else
    { // NULL Pointer
        WriteId( *this, nP | P_ID, 0, 0 );
    }
    return *this;
}

//=========================================================================
UINT32 SvPersistStream::ReadObj
(
    SvPersistBase * &   rpObj,
    BOOL                bRegister
)
{
    BYTE    nHdr;
    UINT32  nId = 0;
    USHORT  nClassId;

    rpObj = NULL;   // Spezifikation: Im Fehlerfall 0.
    ReadId( *this, nHdr, nId, nClassId );

    // reine Versionsnummer durch maskieren
    if( P_VER < (nHdr & P_VER_MASK) )
    {
        SetError( SVSTREAM_FILEFORMAT_ERROR );
        OSL_FAIL( "false version" );
    }

    if( !(nHdr & P_ID_0) && GetError() == SVSTREAM_OK )
    {
        if( P_OBJ & nHdr )
        { // read object, nId nur bei P_DBGUTIL gesetzt
            DBG_ASSERT( !(nHdr & P_DBGUTIL) || NULL == aPUIdx.Get( nId ),
                        "object already exist" );
            SvCreateInstancePersist pFunc = rClassMgr.Get( nClassId );

            UINT32 nObjLen(0), nObjPos(0);
            if( nHdr & P_DBGUTIL )
                nObjLen = ReadLen( &nObjPos );
            if( !pFunc )
            {
#ifdef DBG_UTIL
                ByteString aStr( "no class with id: " );
                aStr += ByteString::CreateFromInt32( nClassId );
                aStr += " registered";
                DBG_WARNING( aStr.GetBuffer() );
#else
                (void)nObjLen;
#endif
                SetError( ERRCODE_IO_NOFACTORY );
                return 0;
            }
            pFunc( &rpObj );
            // Sichern
            rpObj->AddRef();

            if( bRegister )
            {
                // unbedingt erst in Tabelle eintragen
                ULONG nNewId = aPUIdx.Insert( rpObj );
                // um den gleichen Zustand, wie nach dem Speichern herzustellen
                aPTable.Insert( (ULONG)rpObj, (void *)nNewId );
                DBG_ASSERT( !(nHdr & P_DBGUTIL) || nId == nNewId,
                            "read write id conflict: not the same" );
            }
            // und dann Laden
            rpObj->Load( *this );
#ifdef DBG_UTIL
            if( nObjLen + nObjPos != Tell() )
            {
                ByteString aStr( "false object len: read = " );
                aStr += ByteString::CreateFromInt32( (long)(Tell() - nObjPos) );
                aStr += ", should = ";
                aStr += ByteString::CreateFromInt32( nObjLen );
                OSL_FAIL( aStr.GetBuffer() );
            }
#endif
            rpObj->RestoreNoDelete();
            rpObj->ReleaseRef();
        }
        else
        {
            rpObj = GetObject( nId );
            DBG_ASSERT( rpObj != NULL, "object does not exist" );
            DBG_ASSERT( rpObj->GetClassId() == nClassId, "class mismatch" );
        }
    }
    return nId;
}

//=========================================================================
SvPersistStream& SvPersistStream::ReadPointer
(
    SvPersistBase * & rpObj
)
{
    ReadObj( rpObj, TRUE );
    return *this;
}

//=========================================================================
SvPersistStream& operator <<
(
    SvPersistStream & rStm,
    SvPersistBase * pObj
)
{
    return rStm.WritePointer( pObj );
}

//=========================================================================
SvPersistStream& operator >>
(
    SvPersistStream & rStm,
    SvPersistBase * & rpObj
)
{
    return rStm.ReadPointer( rpObj );
}

//=========================================================================
SvStream& operator <<
(
    SvStream & rStm,
    SvPersistStream & rThis
)
{
    SvStream * pOldStm = rThis.GetStream();
    rThis.SetStream( &rStm );

    BYTE bTmp = 0;
    rThis << bTmp;    // Version
    UINT32 nCount = (UINT32)rThis.aPUIdx.Count();
    rThis << nCount;
    SvPersistBase * pEle = rThis.aPUIdx.First();
    for( UINT32 i = 0; i < nCount; i++ )
    {
        BYTE nP = P_OBJ | P_ID | P_STD;
        WriteId( rThis, nP, rThis.aPUIdx.GetCurIndex(),
                        pEle->GetClassId() );
        rThis.WriteObj( nP, pEle );
        pEle = rThis.aPUIdx.Next();
    }
    rThis.SetStream( pOldStm );
    return rStm;
}

//=========================================================================
SvStream& operator >>
(
    SvStream & rStm,
    SvPersistStream & rThis
)
{
    SvStream * pOldStm = rThis.GetStream();
    rThis.SetStream( &rStm );

    BYTE nVers;
    rThis >> nVers;    // Version
    if( 0 == nVers )
    {
        UINT32 nCount = 0;
        rThis >> nCount;
        for( UINT32 i = 0; i < nCount; i++ )
        {
            SvPersistBase * pEle;
            // Lesen, ohne in die Tabellen einzutragen
            sal_uIntPtr nId = rThis.ReadObj( pEle, FALSE );
            if( rThis.GetError() )
                break;

            // Die Id eines Objektes wird nie modifiziert
            rThis.aPUIdx.Insert( nId, pEle );
            rThis.aPTable.Insert( (ULONG)pEle, (void *)nId );
        }
    }
    else
        rThis.SetError( SVSTREAM_FILEFORMAT_ERROR );

    rThis.SetStream( pOldStm );
    return rStm;
}

//=========================================================================
ULONG SvPersistStream::InsertObj( SvPersistBase * pObj )
{
    ULONG nId = aPUIdx.Insert( pObj );
    aPTable.Insert( (ULONG)pObj, (void *)nId );
    return nId;
}

//=========================================================================
ULONG SvPersistStream::RemoveObj( SvPersistBase * pObj )
{
    ULONG nIdx = GetIndex( pObj );
    aPUIdx.Remove( nIdx );
    aPTable.Remove( (ULONG)pObj );
    return nIdx;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
