/*************************************************************************
 *
 *  $RCSfile: resmgr.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-11-06 08:17:58 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef _VOS_SIGNAL_HXX_
#include <vos/signal.hxx>
#endif

#ifndef _NEW_HXX
#include <new.hxx>
#endif
#ifndef _DEBUG_HXX
#include <debug.hxx>
#endif
#ifndef _TABLE_HXX
#include <table.hxx>
#endif
#ifndef _FSYS_HXX
#include <fsys.hxx>
#endif
#ifndef _STREAM_HXX
#include <stream.hxx>
#endif
#ifndef _INTN_HXX
#include <intn.hxx>
#endif
#ifndef _TOOLS_RESMGR_HXX
#include <resmgr.hxx>
#endif
#ifndef _TOOLS_RC_HXX
#include <rc.hxx>
#endif
#ifndef _TOOLS_RCID_H
#include <rcid.h>
#endif

#ifndef _TOOLS_SIMPLERESMGR_HXX_
#include "simplerm.hxx"
#endif

#pragma hdrstop

// =======================================================================

extern ImplSVResourceData aResData;
inline ImplSVResourceData* GetResData()
{
    return &aResData;
}

static List & GetResMgrList()
{
    ImplSVResourceData * pRD = GetResData();
    if ( !pRD->pInternalResMgrList )
        pRD->pInternalResMgrList = new List();
    return *pRD->pInternalResMgrList;
}

struct ImpContent
{
    ULONG   nTypeAndId;
    ULONG   nOffset;
};

#if defined( OS2 ) && defined( ICC )
static int _Optlink Compare( const void * pFirst, const void * pSecond )
#elif S390
extern "C" { int Compare( const void * pFirst, const void * pSecond )
#else
static int __LOADONCALLAPI Compare( const void * pFirst, const void * pSecond )
#endif
{
    if( ((ImpContent *)pFirst)->nTypeAndId > ((ImpContent *)pSecond)->nTypeAndId )
        return( 1 );
    else if( ((ImpContent *)pFirst)->nTypeAndId < ((ImpContent *)pSecond)->nTypeAndId )
        return( -1 );
    else
        return( 0 );
}

#ifdef S390
}
#endif

#if defined( OS2 ) && defined( ICC )
static int _Optlink Search( const void * nTypeAndId, const void * pSecond )
#elif S390
extern "C" { int Search( const void * nTypeAndId, const void * pSecond )
#else
static int __LOADONCALLAPI Search( const void * nTypeAndId, const void * pSecond )
#endif
{
    if( (ULONG)nTypeAndId > (((ImpContent *)pSecond)->nTypeAndId) )
        return( 1 );
    else if( (ULONG)nTypeAndId < (((ImpContent *)pSecond)->nTypeAndId) )
        return( -1 );
    else
        return( 0 );
}

#ifdef S390
}
#endif

// =======================================================================

SvStream * InternalResMgr::GetBitmapStream( USHORT nId )
{
    // Anfang der Strings suchen
    ImpContent * pFind = (ImpContent *)
        bsearch( (void *)((ULONG(RT_SYS_BITMAP) << 16) | nId), pContent, nEntries,
                sizeof( ImpContent ), Search );

    if ( pFind )
    {
        pStm->Seek( pFind->nOffset );
        return pStm;
    }
    return NULL;
}

// -----------------------------------------------------------------------

void InternalResMgr::GetResMgrPath( InternalResMgr* pThis,
                                    const UniString& rFileName,
                                    const UniString* pAppFileName,
                                    const UniString* pResourcePath )
{
    UniString aResFile;
    if ( rFileName.Len() )
    {
        UniString aResPath;
        if( pResourcePath )
            aResPath += *pResourcePath;
        if ( pAppFileName )
        {
            DirEntry aAppDir( *pAppFileName );
            aAppDir.CutName();
            UniString aAppPath = aAppDir.GetFull();
            DirEntry aResSubPath( UniString( RTL_CONSTASCII_USTRINGPARAM( "resource" ) ) );
            aAppDir += aResSubPath;
            UniString aAppResPath = aAppDir.GetFull();

            // Default resource path is bin\resource
            if ( aResPath.Len() )
                aResPath += DirEntry::GetSearchDelimiter();
            aResPath += aAppResPath;

            // we a search also in the bin path
            aResPath += DirEntry::GetSearchDelimiter();
            aResPath += aAppPath;
        }
        const sal_Char* pEnv = getenv( "STAR_RESOURCEPATH" );
        if( pEnv )
        {
            if ( aResPath.Len() )
                aResPath += DirEntry::GetSearchDelimiter();
            aResPath.AppendAscii( pEnv );
        }

        DirEntry aFullName( rFileName );
        if ( aFullName.Find( aResPath ) )
            aResFile = aFullName.GetFull();
        else
            aResFile = rFileName;
    }
    else if ( pAppFileName )
    {
        // Default Resourcefile ist die Anwendung
        aResFile = *pAppFileName;
#if defined( OS2 ) || defined( WIN ) || defined( WNT )
        aResFile.Erase( aResFile.Len() - 4 );
#endif
        aResFile.AppendAscii( ".res" );
    }

    if( aResFile.Len() )
    {
        DirEntry aEntry = aResFile;

        FileStat aStat( aEntry );
        if( aStat.IsKind( FSYS_KIND_FILE ) )
        {
            pThis->aFileName = aEntry.GetFull();
            pThis->aShortFileName = aEntry.GetName();
        }
    }
}

// -----------------------------------------------------------------------

InternalResMgr::InternalResMgr()
    : pContent( NULL )
    , pStringBlock( NULL )
    , pStm( NULL )
    , bEqual2Content( TRUE )
    , nEntries( 0 )
    , pResUseDump( 0 )
{
}

// -----------------------------------------------------------------------

InternalResMgr::~InternalResMgr()
{
    SvMemFree(pContent);
    SvMemFree(pStringBlock);
    delete pStm;

#ifdef DBG_UTIL
    if( pResUseDump )
    {
        const sal_Char* pLogFile = getenv( "STAR_RESOURCE_LOGGING" );
        if ( pLogFile )
        {
            SvFileStream aStm( UniString( pLogFile, RTL_TEXTENCODING_ASCII_US ), STREAM_WRITE );
            aStm.Seek( STREAM_SEEK_TO_END );
            ByteString aLine( "FileName: " );
            aLine.Append( ByteString( aFileName, RTL_TEXTENCODING_UTF8 ) );
            aStm.WriteLine( aLine );

            for( ULONG i = 0; i < pResUseDump->Count(); i++ )
            {
                ULONG nKeyId = pResUseDump->GetObjectKey( i );
                aLine.Assign( "Type/Id: " );
                aLine.Append( ByteString::CreateFromInt32( (nKeyId >> 16) & 0xFFFF ) );
                aLine.Append( '/' );
                aLine.Append( ByteString::CreateFromInt32( nKeyId & 0xFFFF ) );
                aStm.WriteLine( aLine );
            }
        }
    }
#endif

    delete pResUseDump;
}

// -----------------------------------------------------------------------

InternalResMgr* InternalResMgr::Create( const UniString& rName,
                                        const UniString* pAppName,
                                        const UniString* pResPath )
{
    InternalResMgr* pThis = new InternalResMgr();

    GetResMgrPath( pThis, rName, pAppName, pResPath );

    if ( pThis->aFileName.Len() && pThis->Create() )
        return pThis;

    delete pThis;
    return NULL;
}

// -----------------------------------------------------------------------

BOOL InternalResMgr::Create()
{
    BOOL bDone = FALSE;

    pStm = new SvFileStream( aFileName, (STREAM_READ | STREAM_SHARE_DENYWRITE | STREAM_NOCREATE) );
    if( pStm->GetError() == 0 )
    {
        INT32   lContLen = 0;

        pStm->Seek( STREAM_SEEK_TO_END );
        /*
        if( ( pInternalResMgr->pHead = (RSHEADER_TYPE *)mmap( 0, nResourceFileSize,
                                                        PROT_READ, MAP_PRIVATE,
                                                        fRes, 0 ) ) != (RSHEADER_TYPE *)-1)
                                                        */
        pStm->SeekRel( - (int)sizeof( lContLen ) );
        pStm->Read( &lContLen, sizeof( lContLen ) );
        // is bigendian, swab to the right endian
        lContLen = ResMgr::GetLong( &lContLen );
        pStm->SeekRel( -lContLen );
        pContent = (ImpContent *)SvMemAlloc( lContLen );
        pStm->Read( pContent, lContLen );
        // Auf die Anzahl der ImpContent kürzen
        nEntries = (UINT32)lContLen / sizeof( ImpContent );
        bEqual2Content = TRUE;  // Die Daten der Resourcen liegen
                                // genauso wie das Inhaltsverzeichnis
        BOOL bSorted = TRUE;
        if( nEntries )
        {
#ifdef DBG_UTIL
            const sal_Char* pLogFile = getenv( "STAR_RESOURCE_LOGGING" );
            if ( pLogFile )
            {
                pResUseDump = new Table();
                for( ULONG i = 0; i < nEntries; i++ )
                    pResUseDump->Insert( pContent[i].nTypeAndId, NULL );
            }
#endif
            // swap the content to the right endian
            pContent[0].nTypeAndId = ResMgr::GetLong( &pContent[0].nTypeAndId );
            pContent[0].nOffset = ResMgr::GetLong( &pContent[0].nOffset );
            for( ULONG i = 0; i < nEntries -1; i++ )
            {
                // swap the content to the right endian
                pContent[i+1].nTypeAndId = ResMgr::GetLong( &pContent[i+1].nTypeAndId );
                pContent[i+1].nOffset = ResMgr::GetLong( &pContent[i+1].nOffset );
                if( pContent[i].nTypeAndId >= pContent[i +1].nTypeAndId )
                    bSorted = FALSE;
                if( (pContent[i].nTypeAndId & 0xFFFF0000) == (pContent[i +1].nTypeAndId & 0xFFFF0000)
                  && pContent[i].nOffset >= pContent[i +1].nOffset )
                    bEqual2Content = FALSE;
            }
        }
        DBG_ASSERT( bSorted, "content not sorted" )
        DBG_ASSERT( bEqual2Content, "resource structure wrong" )
        if( !bSorted )
            qsort( pContent, nEntries, sizeof( ImpContent ), Compare );

        bDone = TRUE;
    }

    return bDone;
}

// -----------------------------------------------------------------------

InternalResMgr* InternalResMgr::GetInternalResMgr( const UniString& rFileName,
                                                   const UniString* pAppName,
                                                   const UniString* pResPath )
{
    ImplSVResourceData* pSVInData = GetResData();

    // Nur InternalResMgr's mit FileNamen stehen in der Liste
    if ( rFileName.Len() )
    {
        List& rMgrList = GetResMgrList();

        InternalResMgr* pEle = (InternalResMgr*)rMgrList.First();
        while( pEle )
        {
            if ( rFileName.EqualsIgnoreCaseAscii( pEle->aFileName ) ||
                 rFileName.EqualsIgnoreCaseAscii( pEle->aShortFileName ) )
            {
                pEle->AddRef();
                return pEle;
            }
            pEle = (InternalResMgr*)rMgrList.Next();
        }

#ifdef DBG_UTIL
        ByteString aTraceStr( "Search/Load-RESDLL:" );
        aTraceStr += ByteString( rFileName, RTL_TEXTENCODING_UTF8 );
        DBG_TRACE( aTraceStr.GetBuffer() );
#endif

        pEle = Create( rFileName, pAppName, pResPath );

        if ( pEle )
        {
            pEle->AddRef();
            rMgrList.Insert( pEle );
        }

        return pEle;
    }

    return NULL;
}

// -----------------------------------------------------------------------

void InternalResMgr::FreeInternalResMgr( InternalResMgr* pFreeInternalResMgr )
{
    // Nur InternalResMgr's mit FileNamen stehen in der Liste und werden vor dem
    // Programmende freigegeben
    if( pFreeInternalResMgr->aFileName.Len() )
    {
        if( pFreeInternalResMgr->ReleaseRef() == 0 )
            GetResMgrList().Remove( pFreeInternalResMgr );
    }
}

// -----------------------------------------------------------------------

BOOL InternalResMgr::IsGlobalAvailable( RESOURCE_TYPE nRT, USHORT nId ) const
{
    // Anfang der Strings suchen
    ImpContent * pFind = (ImpContent *)
        bsearch( (void *)((ULONG(nRT) << 16) | nId), pContent, nEntries,
                sizeof( ImpContent ), Search );
    return pFind != NULL;
}

// -----------------------------------------------------------------------

void* InternalResMgr::LoadGlobalRes( RESOURCE_TYPE nRT, USHORT nId,
                                     void **pResHandle )
{
#ifdef DBG_UTIL
    if( pResUseDump )
        pResUseDump->Remove( (ULONG(nRT) << 16) | nId );
#endif
    // Anfang der Strings suchen
    ImpContent * pFind = (ImpContent *)
        bsearch( (void *)((ULONG(nRT) << 16) | nId), pContent, nEntries,
                sizeof( ImpContent ), Search );
    if( nRT == RSC_STRING && bEqual2Content && pFind )
    {
        // String Optimierung
        if( !pStringBlock )
        {
            // Anfang der Strings suchen
            ImpContent * pFirst = pFind;
            ImpContent * pLast = pFirst;
            while( pFirst > pContent && ((pFirst -1)->nTypeAndId >> 16) == RSC_STRING )
                pFirst--;
            while( pLast < (pContent + nEntries) && (pLast->nTypeAndId >> 16) == RSC_STRING )
                pLast++;
            nOffCorrection = pFirst->nOffset;
            UINT32 nSize;
            --pLast;
            pStm->Seek( pLast->nOffset );
            RSHEADER_TYPE aHdr;
            pStm->Read( &aHdr, sizeof( aHdr ) );
            nSize = pLast->nOffset + aHdr.GetGlobOff() - nOffCorrection;
            pStringBlock = (BYTE*)SvMemAlloc( nSize );
            pStm->Seek( pFirst->nOffset );
            pStm->Read( pStringBlock, nSize );
        }
        *pResHandle = pStringBlock;
        return (BYTE*)pStringBlock + pFind->nOffset - nOffCorrection;
    }
    *pResHandle = 0;
    if( pFind )
    {
        RSHEADER_TYPE aHeader;
        pStm->Seek( pFind->nOffset );
        pStm->Read( &aHeader, sizeof( RSHEADER_TYPE ) );
        void * pRes = new BYTE[ aHeader.GetGlobOff() ];
        memcpy( pRes, &aHeader, sizeof( RSHEADER_TYPE ) );
        pStm->Read( (BYTE*)pRes + sizeof( RSHEADER_TYPE ),
                    aHeader.GetGlobOff() - sizeof( RSHEADER_TYPE ) );
        return pRes;
    }
    //Resource holen
    return NULL;
}

// -----------------------------------------------------------------------

void InternalResMgr::FreeGlobalRes( void * pResHandle, void * pResource )
{
    if ( !pResHandle )
        // REsource wurde extra allokiert
        delete pResource;
}

// =======================================================================

#ifdef DBG_UTIL

UniString GetTypeRes_Impl( const ResId& rTypeId )
{
    // Funktion verlassen, falls Resourcefehler in dieser Funktion
    static bInUse = FALSE;
    UniString aTypStr( rTypeId.GetId() );

    if ( !bInUse )
    {
        bInUse = TRUE;

        ResId aResId( RSCVERSION_ID );
        aResId.SetRT( RSC_VERSIONCONTROL );

        if ( rTypeId.GetResMgr()->GetResource( aResId ) )
        {
            rTypeId.SetRT( RSC_STRING );
            if ( rTypeId.GetResMgr()->IsAvailable( rTypeId ) )
            {
                aTypStr = UniString( rTypeId );
                // Versions Resource Klassenzeiger ans Ende setzen
                Resource::GetResManager()->Increment( sizeof( RSHEADER_TYPE ) );
            }
        }
        bInUse = FALSE;
    }

    return aTypStr;
}

// -----------------------------------------------------------------------

static void RscError_Impl( const sal_Char* pMessage, ResMgr* pResMgr,
                           RESOURCE_TYPE nRT, USHORT nId,
                           ImpRCStack* pResStack, short nStackTop )
{
    // neuen ResourceMgr erzeugen
    ResMgr* pNewResMgr = new ResMgr( pResMgr->GetFileName() );

    ByteString aStr = ByteString( pResMgr->GetFileName(), RTL_TEXTENCODING_UTF8 );
    if ( aStr.Len() )
        aStr += '\n';

    aStr.Append( "Class: " );
    aStr.Append( ByteString( GetTypeRes_Impl( ResId( nRT, pNewResMgr ) ), RTL_TEXTENCODING_UTF8 ) );
    aStr.Append( ", Id: " );
    aStr.Append( ByteString::CreateFromInt32( (long)nId ) );
    aStr.Append( ". " );
    aStr.Append( pMessage );

    aStr.Append( "\nResource Stack\n" );
    while( nStackTop > 0 )
    {
        aStr.Append( "Class: " );
        aStr.Append( ByteString( GetTypeRes_Impl( ResId( (pResStack + nStackTop)->pResource->GetRT(), pNewResMgr ) ), RTL_TEXTENCODING_UTF8 ) );
        aStr.Append( ", Id: " );
        aStr.Append( ByteString::CreateFromInt32( (long)(pResStack + nStackTop)->pResource->GetId() ) );
        nStackTop--;
    }

    delete pNewResMgr;

    DBG_ERROR( aStr.GetBuffer() );
}

#endif

// =======================================================================

static void RscException_Impl()
{
    switch ( NAMESPACE_VOS(OSignalHandler)::raise( OSL_SIGNAL_USER_RESOURCEFAILURE, (void*)"" ) )
    {
        case NAMESPACE_VOS(OSignalHandler)::TAction_CallNextHandler:
            abort();
            break;

        case NAMESPACE_VOS(OSignalHandler)::TAction_Ignore:
            return;
            break;

        case NAMESPACE_VOS(OSignalHandler)::TAction_AbortApplication:
            abort();
            break;

        case NAMESPACE_VOS(OSignalHandler)::TAction_KillApplication:
            exit(-1);
            break;
    }
}

// =======================================================================

void ImpRCStack::Init( ResMgr* pMgr, const Resource* pObj, USHORT Id )
{
    pResource   = NULL;
    pClassRes   = NULL;
    Flags       = RC_NOTYPE;
    aResHandle  = NULL;
    pResObj     = pObj;
    nId         = Id & ~RSC_DONTRELEASE; //TLX: Besser Init aendern
    pResMgr     = pMgr;
    if ( !(Id & RSC_DONTRELEASE) )
        Flags |= RC_AUTORELEASE;
}

// -----------------------------------------------------------------------

void ImpRCStack::Clear()
{
    pResource   = NULL;
    pClassRes   = NULL;
    Flags       = RC_NOTYPE;
    aResHandle  = NULL;
    pResObj     = NULL;
    nId         = 0;
    pResMgr     = NULL;
}

// -----------------------------------------------------------------------

static RSHEADER_TYPE* LocalResource( const ImpRCStack* pStack,
                                     RESOURCE_TYPE nRTType,
                                     USHORT nId )
{
    // Gibt die Position der Resource zurueck, wenn sie gefunden wurde.
    // Ansonsten gibt die Funktion Null zurueck.
    RSHEADER_TYPE*  pTmp;   // Zeiger auf Kind-Resourceobjekte
    RSHEADER_TYPE*  pEnd;   // Zeiger auf das Ende der Resource

    if ( pStack->pResource && pStack->pClassRes )
    {
        pTmp = (RSHEADER_TYPE*)
               ((BYTE*)pStack->pResource + pStack->pResource->GetLocalOff());
        pEnd = (RSHEADER_TYPE*)
               ((BYTE*)pStack->pResource + pStack->pResource->GetGlobOff());
        while ( pTmp != pEnd )
        {
            if ( pTmp->GetRT() == nRTType && pTmp->GetId() == nId )
                return pTmp;
            pTmp = (RSHEADER_TYPE*)((BYTE*)pTmp + pTmp->GetGlobOff());
        }
    }

    return NULL;
}

// =======================================================================

void ResMgr::DestroyAllResMgr()
{
    ImplSVResourceData* pSVInData = GetResData();

    // Da auch von Abort gerufen werden kann, geben wir alle
    // ResMgr's und alle InternalResMgr's hier frei
    List* pMgrList = pSVInData->pInternalResMgrList;
    if ( pMgrList )
    {
        InternalResMgr* pEle = (InternalResMgr*)pMgrList->First();
        while ( pEle )
        {
            DBG_WARNING1( "ResMgr's not destroyed: %s",
                          ByteString( pEle->aFileName, RTL_TEXTENCODING_UTF8 ).GetBuffer() );
            pEle->ReleaseReference();
            pEle = (InternalResMgr*)pMgrList->Next();
        }
        delete pMgrList;
    }
}

// -----------------------------------------------------------------------

void ResMgr::Init( const UniString& rFileName )
{
    if ( !pImpRes )
    {
#ifdef DBG_UTIL
        ByteString aStr( "Resourcefile not found:\n" );
        aStr += ByteString( rFileName, RTL_TEXTENCODING_UTF8 );
        DBG_ERROR( aStr.GetBuffer() );
#endif
        RscException_Impl();
    }
#ifdef DBG_UTIL
    else
    {
        void* aResHandle = 0;     // Hilfvariable fuer Resource
        void* pVoid;              // Zeiger auf die Resource

        pVoid = pImpRes->LoadGlobalRes( RSC_VERSIONCONTROL, RSCVERSION_ID,
                                        &aResHandle );
        if ( pVoid )
            pImpRes->FreeGlobalRes( aResHandle, pVoid );
        else
        {
            ByteString aStr( "Wrong version:\n" );
            aStr += ByteString( pImpRes->aFileName, RTL_TEXTENCODING_UTF8 );
            DbgError( aStr.GetBuffer() );
        }
    }
#endif

    nTopRes = 0;
    aStack[0].Clear();
}

// -----------------------------------------------------------------------

ResMgr::ResMgr( const UniString& rFileName,
                const UniString* pAppName,
                const UniString* pResPath )
{
    pImpRes = InternalResMgr::GetInternalResMgr( rFileName, pAppName, pResPath );
    Init( pImpRes ? (const UniString&)pImpRes->aFileName : rFileName );
}

// -----------------------------------------------------------------------

ResMgr::ResMgr( InternalResMgr * pImpMgr )
{
    pImpRes = pImpMgr;
    Init( pImpMgr->aFileName );
}

// -----------------------------------------------------------------------

ResMgr::~ResMgr()
{
    InternalResMgr::FreeInternalResMgr( pImpRes );
}

// -----------------------------------------------------------------------

#ifdef DBG_UTIL

void ResMgr::TestStack( const Resource* pResObj )
{
    if ( DbgIsResource() )
    {
        for( short i = 1; i <= nTopRes; i++ )
        {
            if ( aStack[i].pResObj == pResObj )
            {
#ifdef DBG_UTIL
                RscError_Impl( "Resource not freed! ", this,
                               aStack[i].pResource->GetRT(),
                               aStack[i].pResource->GetId(),
                               aStack, i -1 );
#endif
            }
        }
    }
}

#else

void ResMgr::TestStack( const Resource* )
{
}

#endif

// -----------------------------------------------------------------------

BOOL ResMgr::IsAvailable( const ResId& rId, const Resource* pResObj ) const
{
    BOOL            bAvailable = FALSE;
    RSHEADER_TYPE*  pClassRes = rId.GetpResource();
    RESOURCE_TYPE   nRT = rId.GetRT2();
    USHORT          nId = rId.GetId();
    const ResMgr*   pMgr = rId.GetResMgr();

    if ( !pMgr )
        pMgr = this;

    if ( !pResObj || pResObj == pMgr->aStack[pMgr->nTopRes].pResObj )
    {
        if ( !pClassRes )
            pClassRes = LocalResource( &pMgr->aStack[pMgr->nTopRes], nRT, nId );
        if ( pClassRes )
        {
            if ( pClassRes->GetRT() == nRT )
                bAvailable = TRUE;
        }
    }

    // vieleicht globale Resource
    if ( !pClassRes )
        bAvailable = pMgr->pImpRes->IsGlobalAvailable( nRT, nId );

    return bAvailable;
}

// -----------------------------------------------------------------------

inline ResMgr* GetActualResMgr()
{
    return GetResData()->pAppResMgr;
}

// -----------------------------------------------------------------------

BOOL ResMgr::GetResource( const ResId& rId, const Resource* pResObj )
{
    DBG_TESTSOLARMUTEX();

    ResMgr* pMgr = rId.GetResMgr();
    if ( pMgr && (this != pMgr) )
        return pMgr->GetResource( rId, pResObj );

    RSHEADER_TYPE*  pClassRes = rId.GetpResource();
    RESOURCE_TYPE   nRT = rId.GetRT2();
    USHORT          nId = rId.GetId();

    ResMgr* pLastMgr = GetActualResMgr();
    if ( pLastMgr != this )
        Resource::SetResManager( this );

    nTopRes++;      // Stackzeiger erhoehen
    ImpRCStack* pTop = &aStack[nTopRes];
    pTop->Init( pLastMgr, pResObj, nId |
                (rId.IsAutoRelease() ? 0 : RSC_DONTRELEASE) );

    if ( pClassRes )
    {
        if ( pClassRes->GetRT() == nRT )
            pTop->pClassRes = pClassRes;
        else
        {
#ifdef DBG_UTIL
            RscError_Impl( "Different class and resource type!",
                           this, nRT, nId, aStack, nTopRes -1 );
#endif
            RscException_Impl();
            nTopRes--;
            return FALSE;
        }
    }
    else
        pTop->pClassRes = LocalResource( pTop -1, nRT, nId );

    if ( pTop->pClassRes )
        // lokale Resource, nicht system Resource
        pTop->pResource = (RSHEADER_TYPE *)pTop->pClassRes;
    else
    {
        pTop->Flags |= RC_GLOBAL;
        pTop->pClassRes = pImpRes->LoadGlobalRes( nRT, nId, &pTop->aResHandle );
        if ( pTop->pClassRes )
            pTop->pResource = (RSHEADER_TYPE *)pTop->pClassRes;
        else
        {
#ifdef DBG_UTIL
            RscError_Impl( "Cannot load resource! ",
                           this, nRT, nId, aStack, nTopRes -1 );
#endif
            RscException_Impl();
            ImplSVResourceData * pRD = GetResData();
            nTopRes--;
            return FALSE;
        }
    }

    return TRUE;
}

// -----------------------------------------------------------------------

void * ResMgr::GetResourceSkipHeader( const ResId& rResId, ResMgr ** ppResMgr )
{
    if ( rResId.GetResMgr() )
        *ppResMgr = rResId.GetResMgr();
    else
        *ppResMgr = Resource::GetResManager();
    (*ppResMgr)->GetResource( rResId );
    (*ppResMgr)->Increment( sizeof( RSHEADER_TYPE ) );
    return (*ppResMgr)->GetClass();
}

// -----------------------------------------------------------------------

#ifdef DBG_UTIL
void ResMgr::PopContext( const Resource* pResObj )
#else
void ResMgr::PopContext( const Resource* )
#endif
{
#ifdef DBG_UTIL
    if ( DbgIsResource() )
    {
        if ( (aStack[nTopRes].pResObj != pResObj) || !nTopRes )
        {
            RscError_Impl( "Cannot free resource! ", this,
                           RSC_NOTYPE, 0, aStack, nTopRes );
        }
    }
#endif

    if ( nTopRes )
    {
        ImpRCStack* pTop = &aStack[nTopRes];
#ifdef DBG_UTIL
        if ( DbgIsResource() )
        {
            void* pRes = (BYTE*)pTop->pResource +
                         pTop->pResource->GetLocalOff();

            if ( pTop->pClassRes != pRes )
            {
                RscError_Impl( "Classpointer not at the end!",
                               this, pTop->pResource->GetRT(),
                               pTop->pResource->GetId(),
                               aStack, nTopRes -1 );
            }
        }
#endif

        // Resource freigeben
        if ( pTop->Flags & RC_GLOBAL )
            // kann auch Fremd-Ressource sein
            pImpRes->FreeGlobalRes( pTop->aResHandle, pTop->pResource );
        if ( pTop->pResMgr != this )
            // wurde durch ResId gesetzt, automatisch zuruecksetzen
            Resource::SetResManager( pTop->pResMgr );
        nTopRes--;
    }
}

// -----------------------------------------------------------------------

RSHEADER_TYPE* ResMgr::CreateBlock( const ResId& rId )
{
    RSHEADER_TYPE* pHeader = NULL;
    if ( GetResource( rId ) )
    {
        // Der Zeiger steht am Anfang, deswegen zeigt der Klassen-Pointer
        // auf den Header und die restliche Groesse ist die Gesammte.
        pHeader = (RSHEADER_TYPE*)new BYTE[ GetRemainSize() ];
        memcpy( pHeader, GetClass(), GetRemainSize() );
        Increment( pHeader->GetLocalOff() ); //ans Ende setzen
        if ( pHeader->GetLocalOff() != pHeader->GetGlobOff() )
            // Hat Sub-Ressourcen, deshalb extra freigeben
            PopContext();
    }

    return pHeader;
}

// ------------------------------------------------------------------

INT16 ResMgr::GetShort( void * pShort )
{
#ifdef __BIGENDIAN
    return *(UINT16*)pShort;
#else
    return SWAPSHORT( *(UINT16*)pShort );
#endif
}

// ------------------------------------------------------------------

INT32 ResMgr::GetLong( void * pLong )
{
#ifdef __BIGENDIAN
    return (long)(((INT32)(*(UINT16*)pLong) << 16) | *(((UINT16*)pLong) + 1));
#else
    return ((INT32)(*(BYTE*)pLong) << 24)
            | ((INT32)(*((BYTE*)pLong +1)) << 16)
            | ((INT32)(*((BYTE*)pLong +2)) << 8)
            | (INT32)(*((BYTE*)pLong +3));
#endif
}

// -----------------------------------------------------------------------

USHORT ResMgr::GetString( UniString& rStr, const BYTE* pStr )
{
    UniString aString( (sal_Char*)pStr, RTL_TEXTENCODING_UTF8,
                       RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_MAPTOPRIVATE |
                       RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT |
                       RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT );
    rStr = aString;
    return GetStringSize( pStr );
}

// ------------------------------------------------------------------

USHORT ResMgr::GetStringSize( const BYTE* pStr )
{
    return GetStringSize( strlen( (const char*)pStr ) );
}

// -----------------------------------------------------------------------

USHORT ResMgr::GetRemainSize()
{
    return  (USHORT)((long)(BYTE *)aStack[nTopRes].pResource +
                     aStack[nTopRes].pResource->GetLocalOff() -
                     (long)(BYTE *)aStack[nTopRes].pClassRes);
}

// -----------------------------------------------------------------------

void* ResMgr::Increment( USHORT nSize )
{
    BYTE* pClassRes = (BYTE*)aStack[nTopRes].pClassRes + nSize;

    aStack[nTopRes].pClassRes = pClassRes;

    RSHEADER_TYPE* pRes = aStack[nTopRes].pResource;

    if ( (pRes->GetGlobOff() == pRes->GetLocalOff()) &&
         (((char*)pRes + pRes->GetLocalOff()) == aStack[nTopRes].pClassRes) &&
         (aStack[nTopRes].Flags & RC_AUTORELEASE))
    {
        PopContext( aStack[nTopRes].pResObj );
    }

    return pClassRes;
}

// -----------------------------------------------------------------------

const char* ResMgr::GetLang( LanguageType& nType, USHORT nPrio )
{
    static sal_Char const aDefEng[]         = "44";
    static sal_Char const aDefUSEng[]       = "01";
    static sal_Char const aDefGerman[]      = "49";
    static sal_Char const aDefFrench[]      = "33";
    static sal_Char const aDefPortuguese[]  = "03";

    if ( nType == LANGUAGE_SYSTEM || nType == LANGUAGE_DONTKNOW )
        nType = ::GetSystemLanguage();

    if ( nPrio == 0 )
    {
        switch ( nType )
        {
            case LANGUAGE_DANISH:
                return "45";

            case LANGUAGE_DUTCH:
            case LANGUAGE_DUTCH_BELGIAN:
                return "31";

            case LANGUAGE_ENGLISH:
            case LANGUAGE_ENGLISH_UK:
            case LANGUAGE_ENGLISH_EIRE:
            case LANGUAGE_ENGLISH_SAFRICA:
            case LANGUAGE_ENGLISH_JAMAICA:
            case LANGUAGE_ENGLISH_BELIZE:
            case LANGUAGE_ENGLISH_TRINIDAD:
            case LANGUAGE_ENGLISH_ZIMBABWE:
            case LANGUAGE_ENGLISH_PHILIPPINES:
                return aDefEng;

            case LANGUAGE_ENGLISH_US:
            case LANGUAGE_ENGLISH_CAN:
                return aDefUSEng;

            case LANGUAGE_ENGLISH_AUS:
            case LANGUAGE_ENGLISH_NZ:
                return "61";

            case LANGUAGE_FINNISH:
                return "05";

            case LANGUAGE_FRENCH_CANADIAN:
                return "02";

            case LANGUAGE_FRENCH:
            case LANGUAGE_FRENCH_BELGIAN:
            case LANGUAGE_FRENCH_SWISS:
            case LANGUAGE_FRENCH_LUXEMBOURG:
            case LANGUAGE_FRENCH_MONACO:
                return aDefFrench;

            case LANGUAGE_GERMAN:
            case LANGUAGE_GERMAN_SWISS:
            case LANGUAGE_GERMAN_AUSTRIAN:
            case LANGUAGE_GERMAN_LUXEMBOURG:
            case LANGUAGE_GERMAN_LIECHTENSTEIN:
                return aDefGerman;

            case LANGUAGE_ITALIAN:
            case LANGUAGE_ITALIAN_SWISS:
                return "39";

            case LANGUAGE_NORWEGIAN:
            case LANGUAGE_NORWEGIAN_BOKMAL:
                return "47";

            case LANGUAGE_PORTUGUESE:
                return aDefPortuguese;

            case LANGUAGE_PORTUGUESE_BRAZILIAN:
                return "55";

            case LANGUAGE_SPANISH:
            case LANGUAGE_SPANISH_MEXICAN:
            case LANGUAGE_SPANISH_MODERN:
            case LANGUAGE_SPANISH_GUATEMALA:
            case LANGUAGE_SPANISH_COSTARICA:
            case LANGUAGE_SPANISH_PANAMA:
            case LANGUAGE_SPANISH_DOMINICAN_REPUBLIC:
            case LANGUAGE_SPANISH_VENEZUELA:
            case LANGUAGE_SPANISH_COLOMBIA:
            case LANGUAGE_SPANISH_PERU:
            case LANGUAGE_SPANISH_ARGENTINA:
            case LANGUAGE_SPANISH_ECUADOR:
            case LANGUAGE_SPANISH_CHILE:
            case LANGUAGE_SPANISH_URUGUAY:
            case LANGUAGE_SPANISH_PARAGUAY:
            case LANGUAGE_SPANISH_BOLIVIA:
                return "34";

            case LANGUAGE_SWEDISH:
                return "46";

            case LANGUAGE_POLISH:
                return "48";
            case LANGUAGE_CZECH:
                return "42";
            case LANGUAGE_HUNGARIAN:
                return "36";
            case LANGUAGE_RUSSIAN:
                return "07";
            case LANGUAGE_SLOVAK:
                return "04";
            case LANGUAGE_GREEK:
                return "30";
            case LANGUAGE_TURKISH:
                return "90";

            case LANGUAGE_CHINESE_SIMPLIFIED:
                return "86";
            case LANGUAGE_CHINESE_TRADITIONAL:
                return "88";
            case LANGUAGE_JAPANESE:
                return "81";
            case LANGUAGE_KOREAN:
            case LANGUAGE_KOREAN_JOHAB:
                return "82";

            case LANGUAGE_ARABIC:
            case LANGUAGE_ARABIC_IRAQ:
            case LANGUAGE_ARABIC_EGYPT:
            case LANGUAGE_ARABIC_LIBYA:
            case LANGUAGE_ARABIC_ALGERIA:
            case LANGUAGE_ARABIC_MOROCCO:
            case LANGUAGE_ARABIC_TUNISIA:
            case LANGUAGE_ARABIC_OMAN:
            case LANGUAGE_ARABIC_YEMEN:
            case LANGUAGE_ARABIC_SYRIA:
            case LANGUAGE_ARABIC_JORDAN:
            case LANGUAGE_ARABIC_LEBANON:
            case LANGUAGE_ARABIC_KUWAIT:
            case LANGUAGE_ARABIC_UAE:
            case LANGUAGE_ARABIC_BAHRAIN:
            case LANGUAGE_ARABIC_QATAR:
                return "96";

            default:
                return aDefUSEng;
        }
    }
    else if ( nPrio == 1 )
    {
        switch ( nType )
        {
            case LANGUAGE_FRENCH_CANADIAN:
                return aDefFrench;

            case LANGUAGE_PORTUGUESE_BRAZILIAN:
                return aDefPortuguese;

            default:
                return NULL;
        }
    }
    else if ( nPrio == 2 )
        return aDefUSEng;
    else if ( nPrio == 3 )
        return aDefEng;
    else
        return aDefGerman;
}

// -----------------------------------------------------------------------

ResMgr* ResMgr::CreateResMgr( const sal_Char* pPrefixName,
                              LanguageType nType,
                              const UniString* pAppName,
                              const UniString* pResPath )
{
    // Suchreihenfolge festlegen
    const sal_Char* pLang[5];

    // Resourcefile suchen
    UniString aName;
    InternalResMgr* pInternalResMgr = NULL;
    for ( int i = 0; i < 5; i++ )
    {
        pLang[i] = GetLang( nType, i );

        if ( pLang[i] && (i == 0 || pLang[i] != pLang[0]) )
        {
            aName.AssignAscii( pPrefixName );
            aName.AppendAscii( pLang[i] );
            aName.AppendAscii( ".res" );
            pInternalResMgr = InternalResMgr::GetInternalResMgr( aName, pAppName, pResPath );
            if ( pInternalResMgr )
                break;
        }
    }

    if ( pInternalResMgr )
        return new ResMgr( pInternalResMgr );

    return NULL;
}

// -----------------------------------------------------------------------

INT16 ResMgr::ReadShort()
{
    INT16 n = GetShort( GetClass() );
    Increment( sizeof( INT16 ) );
    return n;
}

// -----------------------------------------------------------------------

INT32 ResMgr::ReadLong()
{
    INT32 n = GetLong( GetClass() );
    Increment( sizeof( INT32 ) );
    return n;
}

// -----------------------------------------------------------------------

UniString ResMgr::ReadString()
{
    UniString aRet;
    Increment( GetString( aRet, (const BYTE*)GetClass() ) );
    return aRet;
}

// =======================================================================

SimpleResMgr::SimpleResMgr( const sal_Char* pPrefixName,
                            LanguageType nType,
                            const UniString* pAppName,
                            const UniString* pResPath )
{
    // Suchreihenfolge festlegen
    const sal_Char* pLang[5];

    // Resourcefile suchen
    UniString aName;
    for ( int i = 0; i < 5; i++ )
    {
        pLang[i] = ResMgr::GetLang( nType, i );

        if ( pLang[i] && (i == 0 || pLang[i] != pLang[0]) )
        {
            aName.AssignAscii( pPrefixName );
            aName.AppendAscii( pLang[i] );
            aName.AppendAscii( ".res" );
            m_pResImpl = InternalResMgr::Create( aName, pAppName, pResPath );
            if ( m_pResImpl )
            {
                m_pResImpl->AddRef();
                break;
            }
        }
    }
}

// -----------------------------------------------------------------------

SimpleResMgr::~SimpleResMgr()
{
#ifdef DBG_UTIL
    sal_Int32 nRefCount =
#endif
        m_pResImpl->ReleaseRef();
    DBG_ASSERT(0 == nRefCount, "SimpleResMgr::~SimpleResMgr: invalid impl ref count!");
        // our impl class is not expected to be shared, and only we ourself should have added a ref
}

// -----------------------------------------------------------------------

UniString SimpleResMgr::ReadString( USHORT nId )
{
    NAMESPACE_VOS(OGuard) aGuard(m_aAccessSafety);

    DBG_ASSERT( m_pResImpl, "SimpleResMgr::ReadString : have no impl class !" );
    // perhaps constructed with an invalid filename ?

    UniString sReturn;
    if ( !m_pResImpl )
        return sReturn;

    void* pResHandle = NULL;
    RSHEADER_TYPE* pResHeader = (RSHEADER_TYPE*)m_pResImpl->LoadGlobalRes( RSC_STRING, nId, &pResHandle );
    if ( !pResHeader )
        // no such resource
        return sReturn;

    USHORT nLen = pResHeader->GetLocalOff() - sizeof(RSHEADER_TYPE);
    ResMgr::GetString( sReturn, (const BYTE*)(pResHeader+1) );

    // not neccessary with te current implementation which holds the string table permanently, but to be sure ....
    m_pResImpl->FreeGlobalRes( pResHeader, pResHandle );
    return sReturn;
}

// -----------------------------------------------------------------------

USHORT SimpleResMgr::ReadBlob( USHORT nId, void** pBuffer )
{
    NAMESPACE_VOS(OGuard) aGuard(m_aAccessSafety);

    DBG_ASSERT( m_pResImpl, "SimpleResMgr::ReadBlob : have no impl class !" );

    // perhaps constructed with an invalid filename ?
    DBG_ASSERT( pBuffer, "SimpleResMgr::ReadBlob : invalid argument !" );
    *pBuffer = NULL;

    void* pResHandle = NULL;
    RSHEADER_TYPE* pResHeader = (RSHEADER_TYPE*)m_pResImpl->LoadGlobalRes( RSC_RESOURCE, nId, &pResHandle );
    DBG_ASSERT( pResHeader, "SimpleResMgr::ReadBlob : couldn't find the resource with the given id !" );

    // no exception handling, this would require the locking of the solar mutex which isn't allowed within this class
    if ( !pResHeader )
        return 0;

    DBG_ASSERT( pResHandle == NULL, "SimpleResMgr::ReadBlob : behaviour of LoadGlobalRes changed !" );
    // if pResHandle is not NULL the FreeBlob wouldn't have to delete the pointer given as pBuffer, but
    // FreeBlob doesn't know that so it would probably crash ....

    USHORT nRemaining = pResHeader->GetLocalOff() - sizeof(RSHEADER_TYPE);
    *pBuffer = (void*)(((BYTE*)pResHeader) + sizeof(RSHEADER_TYPE));
    return nRemaining;
}

// -----------------------------------------------------------------------

void SimpleResMgr::FreeBlob( void* pBuffer )
{
    void* pCompleteBuffer = (void*)(((BYTE*)pBuffer) - sizeof(RSHEADER_TYPE));
    delete pCompleteBuffer;
}
