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


#include <tools/debug.hxx>
#include "bf_so3/linkmgr.hxx"

#include <vcl/msgbox.hxx>
#include "bf_so3/lnkbase.hxx"
#include "impldde.hxx"
#include "svuidlg.hrc"
#include <bf_so3/iface.hxx>

#define _SVSTDARR_STRINGSDTOR

#include <bf_svtools/svstdarr.hxx>


namespace binfilter
{

SV_IMPL_PTRARR( SvBaseLinks, SvBaseLinkRefPtr )

SvLinkManager::SvLinkManager()
    : pPersist( 0 )
{
}


SvLinkManager::~SvLinkManager()
{
    SvBaseLinkRef** ppRef = (SvBaseLinkRef**)aLinkTbl.GetData();
    for( USHORT n = aLinkTbl.Count(); n; --n, ++ppRef )
    {
        if( (*ppRef)->Is() )
        {
            (*(*ppRef))->Disconnect();
            (*(*ppRef))->pLinkMgr = 0;
        }
        delete *ppRef;
    }
}


/************************************************************************
|*    SvLinkManager::Remove()
|*
|*    Beschreibung
*************************************************************************/

void SvLinkManager::Remove( SvBaseLink *pLink )
{
    // keine Links doppelt einfuegen
    int bFound = FALSE;
    SvBaseLinkRef** ppRef = (SvBaseLinkRef**)aLinkTbl.GetData();
    for( USHORT n = aLinkTbl.Count(); n; --n, ++ppRef )
    {
        if( pLink == *(*ppRef) )
        {
            (*(*ppRef))->Disconnect();
            (*(*ppRef))->pLinkMgr = 0;
            (*(*ppRef)).Clear();
            bFound = TRUE;
        }

        // falls noch leere rum stehen sollten, weg damit
        if( !(*ppRef)->Is() )
        {
            delete *ppRef;
            aLinkTbl.Remove( aLinkTbl.Count() - n, 1 );
            if( bFound )
                return ;
            --ppRef;
        }
    }
}


void SvLinkManager::Remove( USHORT nPos, USHORT nCnt )
{
    if( nCnt && nPos < aLinkTbl.Count() )
    {
        if( nPos + nCnt > aLinkTbl.Count() )
            nCnt = aLinkTbl.Count() - nPos;

        SvBaseLinkRef** ppRef = (SvBaseLinkRef**)aLinkTbl.GetData() + nPos;
        for( USHORT n = nCnt; n; --n, ++ppRef )
        {
            if( (*ppRef)->Is() )
            {
                (*(*ppRef))->Disconnect();
                (*(*ppRef))->pLinkMgr = 0;
            }
            delete *ppRef;
        }
        aLinkTbl.Remove( nPos, nCnt );
    }
}


BOOL SvLinkManager::Insert( SvBaseLink* pLink )
{
    // keine Links doppelt einfuegen
    for( USHORT n = 0; n < aLinkTbl.Count(); ++n )
    {
        SvBaseLinkRef* pTmp = aLinkTbl[ n ];
        if( !pTmp->Is() )
            aLinkTbl.DeleteAndDestroy( n-- );

        if( pLink == *pTmp )
            return FALSE;
    }

    SvBaseLinkRef* pTmp = new SvBaseLinkRef( pLink );
    pLink->pLinkMgr = this;
    aLinkTbl.Insert( pTmp, aLinkTbl.Count() );
    return TRUE;
}


BOOL SvLinkManager::InsertLink( SvBaseLink * pLink,
                                USHORT nObjType,
                                USHORT nUpdateMode,
                                const String* pName )
{
    // unbedingt zuerst
    pLink->SetObjType( nObjType );
    if( pName )
        pLink->SetName( *pName );
    pLink->SetUpdateMode( nUpdateMode );
    return Insert( pLink );
}


BOOL SvLinkManager::InsertDDELink( SvBaseLink * pLink,
                                    const String& rServer,
                                    const String& rTopic,
                                    const String& rItem )
{
    if( !( OBJECT_CLIENT_SO & pLink->GetObjType() ) )
        return FALSE;

    String sCmd;
    ::binfilter::MakeLnkName( sCmd, &rServer, rTopic, rItem );

    pLink->SetObjType( OBJECT_CLIENT_DDE );
    pLink->SetName( sCmd );
    return Insert( pLink );
}


BOOL SvLinkManager::InsertDDELink( SvBaseLink * pLink )
{
    DBG_ASSERT( OBJECT_CLIENT_SO & pLink->GetObjType(), "no OBJECT_CLIENT_SO" );
    if( !( OBJECT_CLIENT_SO & pLink->GetObjType() ) )
        return FALSE;

    if( pLink->GetObjType() == OBJECT_CLIENT_SO )
        pLink->SetObjType( OBJECT_CLIENT_DDE );

    return Insert( pLink );
}


// erfrage die Strings fuer den Dialog
BOOL SvLinkManager::GetDisplayNames( const SvBaseLink * pLink,
                                        String* pType,
                                        String* pFile,
                                        String* pLinkStr,
                                        String* pFilter ) const
{
    (void)pFilter;

    BOOL bRet = FALSE;
    String aLN = pLink->GetLinkSourceName();
    if( aLN.Len() != 0 && pLink->GetObjType() == OBJECT_CLIENT_DDE )
    {
        USHORT nTmp = 0;
        String sCmd( aLN );
        String sServer(
            sCmd.GetToken(
                0,
                sal::static_int_cast< sal_Unicode >(cTokenSeperator), nTmp ) );
        String sTopic(
            sCmd.GetToken(
                0,
                sal::static_int_cast< sal_Unicode >(cTokenSeperator), nTmp ) );

        if( pType )
            *pType = sServer;
        if( pFile )
            *pFile = sTopic;
        if( pLinkStr )
            *pLinkStr = sCmd.Copy( nTmp );
        bRet = TRUE;
    }
    return bRet;
}


void SvLinkManager::UpdateAllLinks( BOOL bCallErrHdl,
                                    BOOL bUpdateGrfLinks )
{
    (void)bCallErrHdl;

    SvStringsDtor aApps, aTopics, aItems;
    String sApp, sTopic, sItem;

    // erstmal eine Kopie vom Array machen, damit sich updatende Links in
    // Links in ... nicht dazwischen funken!!
    SvPtrarr aTmpArr( 255, 50 );
    USHORT n;
    for( n = 0; n < aLinkTbl.Count(); ++n )
    {
        SvBaseLink* pLink = *aLinkTbl[ n ];
        if( !pLink )
        {
            Remove( n-- );
            continue;
        }
        aTmpArr.Insert( pLink, aTmpArr.Count() );
    }

    for( n = 0; n < aTmpArr.Count(); ++n )
    {
        SvBaseLink* pLink = (SvBaseLink*)aTmpArr[ n ];

        // suche erstmal im Array nach dem Eintrag
        USHORT nFndPos = USHRT_MAX;
        for( USHORT i = 0; i < aLinkTbl.Count(); ++i )
            if( pLink == *aLinkTbl[ i ] )
            {
                nFndPos = i;
                break;
            }

        if( USHRT_MAX == nFndPos )
            continue;					// war noch nicht vorhanden!

        // Graphic-Links noch nicht updaten
        if( !pLink->IsVisible() ||
            ( !bUpdateGrfLinks && OBJECT_CLIENT_GRF == pLink->GetObjType() ))
            continue;

        pLink->Update();
    }
}

/************************************************************************
|*    SvBaseLink::CreateObject()
|*
|*    Beschreibung
*************************************************************************/

SvLinkSourceRef SvLinkManager::CreateObj( SvBaseLink * pLink )
{
    if( OBJECT_CLIENT_DDE == pLink->GetObjType() )
        return new SvDDEObject();
    return SvLinkSourceRef();
}

BOOL SvLinkManager::InsertServer( SvLinkSource* pObj )
{
    // keine doppelt einfuegen
    if( !pObj || USHRT_MAX != aServerTbl.GetPos( pObj ) )
        return FALSE;

    aServerTbl.Insert( pObj, aServerTbl.Count() );
    return TRUE;
}


void SvLinkManager::RemoveServer( SvLinkSource* pObj )
{
    USHORT nPos = aServerTbl.GetPos( pObj );
    if( USHRT_MAX != nPos )
        aServerTbl.Remove( nPos, 1 );
}


void MakeLnkName( String& rName, const String* pType, const String& rFile,
                    const String& rLink, const String* pFilter )
{
    if( pType )
        (rName = *pType).EraseLeadingChars().EraseTrailingChars() += cTokenSeperator;
    else if( rName.Len() )
        rName.Erase();

    ((rName += rFile).EraseLeadingChars().EraseTrailingChars() +=
        cTokenSeperator ).EraseLeadingChars().EraseTrailingChars() += rLink;
    if( pFilter )
        ((rName += cTokenSeperator ) += *pFilter).EraseLeadingChars().EraseTrailingChars();
}

}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
