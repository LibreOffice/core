/*************************************************************************
 *
 *  $RCSfile: swserv.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2001-03-08 21:19:07 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _SVXLINKMGR_HXX
#include <svx/linkmgr.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _SWSERV_HXX
#include <swserv.hxx>
#endif
#ifndef _SWBASLNK_HXX
#include <swbaslnk.hxx>
#endif
#ifndef _MVSAVE_HXX
#include <mvsave.hxx>
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _NODE_HXX
#include <node.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif

#ifndef _SWERROR_H
#include <swerror.h>
#endif

SV_IMPL_REF( SwServerObject )

SwServerObject::~SwServerObject()
{
}


BOOL SwServerObject::GetData( ::com::sun::star::uno::Any & rData,
                                 const String & rMimeType, BOOL )
{
    BOOL bRet = FALSE;
    WriterRef xWrt;
    switch( SotExchange::GetFormatIdFromMimeType( rMimeType ) )
    {
    case FORMAT_STRING:
        ::GetASCWriter( aEmptyStr, xWrt );
        break;

    case FORMAT_RTF:
        ::GetRTFWriter( aEmptyStr, xWrt );
        break;
    }

    if( xWrt.Is() )
    {
        SwPaM* pPam = 0;
        switch( eType )
        {
        case BOOKMARK_SERVER:
            if( CNTNT_TYPE.pBkmk->GetOtherPos() )
            {
                // Bereich aufspannen
                pPam = new SwPaM( CNTNT_TYPE.pBkmk->GetPos(),
                                *CNTNT_TYPE.pBkmk->GetOtherPos() );
            }
            break;

        case TABLE_SERVER:
            pPam = new SwPaM( *CNTNT_TYPE.pTblNd,
                             *CNTNT_TYPE.pTblNd->EndOfSectionNode() );
            break;

        case SECTION_SERVER:
            pPam = new SwPaM( SwPosition( *CNTNT_TYPE.pSectNd ) );
            pPam->Move( fnMoveForward );
            pPam->SetMark();
            pPam->GetPoint()->nNode = *CNTNT_TYPE.pSectNd->EndOfSectionNode();
            pPam->Move( fnMoveBackward );
            break;
        }

        if( pPam )
        {
            // Stream anlegen
            SvMemoryStream aMemStm( 65535, 65535 );
            SwWriter aWrt( aMemStm, *pPam, FALSE );
            if( !IsError( aWrt.Write( xWrt )) )
            {
                aMemStm << '\0';        // append a zero char
                rData <<= ::com::sun::star::uno::Sequence< sal_Int8 >(
                                        (sal_Int8*)aMemStm.GetData(),
                                        aMemStm.Seek( STREAM_SEEK_TO_END ) );
                bRet = TRUE;
            }

            delete pPam;
        }
    }
    return bRet;
}


BOOL SwServerObject::SetData( const String & rMimeType,
                    const ::com::sun::star::uno::Any& rData )
{
    // set new data into the "server" -> at first nothing to do
    return FALSE;
}


void SwServerObject::SendDataChanged( const SwPosition& rPos )
{
    // ist an unseren Aenderungen jemand interessiert ?
    if( HasDataLinks() )
    {
        int bCall = FALSE;
        const SwStartNode* pNd = 0;
        switch( eType )
        {
        case BOOKMARK_SERVER:
            if( CNTNT_TYPE.pBkmk->GetOtherPos() )
            {
                SwBookmark& rBkmk = *CNTNT_TYPE.pBkmk;
                bCall = rBkmk.GetPos() < *rBkmk.GetOtherPos()
                    ? ( rBkmk.GetPos() <= rPos && rPos < *rBkmk.GetOtherPos() )
                    : ( *rBkmk.GetOtherPos() <= rPos && rPos < rBkmk.GetPos() );
            }
            break;

        case TABLE_SERVER:      pNd = CNTNT_TYPE.pTblNd;    break;
        case SECTION_SERVER:    pNd = CNTNT_TYPE.pSectNd;   break;
        }
        if( pNd )
        {
            register ULONG nNd = rPos.nNode.GetIndex();
            bCall = pNd->GetIndex() < nNd && nNd < pNd->EndOfSectionIndex();
        }

        if( bCall )
        {
            // Recursionen erkennen und flaggen
            IsLinkInServer( 0 );
            SvLinkSource::NotifyDataChanged();
        }
    }
    // sonst melden wir uns ab !!
// ????? JP 27.06.95: geht das so ????
//  else
//      Closed();
}


void SwServerObject::SendDataChanged( const SwPaM& rRange )
{
    // ist an unseren Aenderungen jemand interessiert ?
    if( HasDataLinks() )
    {
        int bCall = FALSE;
        const SwStartNode* pNd = 0;
        const SwPosition* pStt = rRange.Start(), *pEnd = rRange.End();
        switch( eType )
        {
        case BOOKMARK_SERVER:
            if( CNTNT_TYPE.pBkmk->GetOtherPos() )
            {
                SwBookmark& rBkmk = *CNTNT_TYPE.pBkmk;
                const SwPosition* pBkStt = &rBkmk.GetPos(),
                                * pBkEnd = rBkmk.GetOtherPos();
                if( *pBkStt > *pBkEnd )
                {
                    const SwPosition* pTmp = pBkStt;
                    pBkStt = pBkEnd;
                    pBkEnd = pTmp;
                }
                bCall = *pStt <= *pBkEnd && *pEnd > *pBkStt;
            }
            break;

        case TABLE_SERVER:      pNd = CNTNT_TYPE.pTblNd;    break;
        case SECTION_SERVER:    pNd = CNTNT_TYPE.pSectNd;   break;
        }
        if( pNd )
        {
            // liegt der Start-Bereich im Node Bereich ?
            bCall = pStt->nNode.GetIndex() <  pNd->EndOfSectionIndex() &&
                    pEnd->nNode.GetIndex() >= pNd->GetIndex();
        }

        if( bCall )
        {
            // Recursionen erkennen und flaggen
            IsLinkInServer( 0 );
            SvLinkSource::NotifyDataChanged();
        }
    }
    // sonst melden wir uns ab !!
// ????? JP 27.06.95: geht das so ????
//  else
//      Closed();
}


BOOL SwServerObject::IsLinkInServer( const SwBaseLink* pChkLnk ) const
{
    ULONG nSttNd = 0, nEndNd = 0;
    xub_StrLen nStt, nEnd;
    const SwNode* pNd = 0;
    const SwNodes* pNds = 0;

    switch( eType )
    {
    case BOOKMARK_SERVER:
        if( CNTNT_TYPE.pBkmk->GetOtherPos() )
        {
            SwBookmark& rBkmk = *CNTNT_TYPE.pBkmk;
            const SwPosition* pStt = &rBkmk.GetPos(),
                            * pEnd = rBkmk.GetOtherPos();
            if( *pStt > *pEnd )
            {
                const SwPosition* pTmp = pStt;
                pStt = pEnd;
                pEnd = pTmp;
            }

            nSttNd = pStt->nNode.GetIndex();
            nStt = pStt->nContent.GetIndex();
            nEndNd = pEnd->nNode.GetIndex();
            nEnd = pEnd->nContent.GetIndex();
            pNds = &pStt->nNode.GetNodes();
        }
        break;

    case TABLE_SERVER:      pNd = CNTNT_TYPE.pTblNd;    break;
    case SECTION_SERVER:    pNd = CNTNT_TYPE.pSectNd;   break;

    case SECTION_SERVER+1:
        return TRUE;
    }

    if( pNd )
    {
        nSttNd = pNd->GetIndex();
        nEndNd = pNd->EndOfSectionIndex();
        nStt = 0, nEnd = USHRT_MAX;
        pNds = &pNd->GetNodes();
    }

    if( nSttNd && nEndNd )
    {
        // LinkManager besorgen:
        const ::so3::SvBaseLinks& rLnks = pNds->GetDoc()->GetLinkManager().GetLinks();

// um Rekursionen zu Verhindern: ServerType umsetzen!
SwServerObject::ServerModes eSave = eType;
if( !pChkLnk )
// sowas sollte man nicht tun, wer weiss schon, wie gross ein enum ist
// ICC nimmt keinen int
// #41723#
//  *((int*)&eType) = SECTION_SERVER+1;
    ((SwServerObject*)this)->eType = NONE_SERVER;
        for( USHORT n = rLnks.Count(); n; )
        {
            const ::so3::SvBaseLink* pLnk = &(*rLnks[ --n ]);
            if( pLnk && OBJECT_CLIENT_GRF != pLnk->GetObjType() &&
                pLnk->ISA( SwBaseLink ) &&
                !((SwBaseLink*)pLnk)->IsNoDataFlag() &&
                ((SwBaseLink*)pLnk)->IsInRange( nSttNd, nEndNd, nStt, nEnd ))
            {
                if( pChkLnk )
                {
                    if( pLnk == pChkLnk ||
                        ((SwBaseLink*)pLnk)->IsRecursion( pChkLnk ) )
                        return TRUE;
                }
                else if( ((SwBaseLink*)pLnk)->IsRecursion( (SwBaseLink*)pLnk ) )
                    ((SwBaseLink*)pLnk)->SetNoDataFlag();
            }
        }
if( !pChkLnk )
    //  *((int*)&eType) = eSave;
    ((SwServerObject*)this)->eType = eSave;
    }

    return FALSE;
}

/*  */


SwDataChanged::SwDataChanged( const SwPaM& rPam, USHORT nTyp )
    : pPam( &rPam ), nType( nTyp ), pDoc( rPam.GetDoc() ), pPos( 0 )
{
    nNode = rPam.GetPoint()->nNode.GetIndex();
    nCntnt = rPam.GetPoint()->nContent.GetIndex();
}


SwDataChanged::SwDataChanged( SwDoc* pDoc, const SwPosition& rPos, USHORT nTyp )
    : pPam( 0 ), nType( nTyp ), pDoc( pDoc ), pPos( &rPos )
{
    nNode = rPos.nNode.GetIndex();
    nCntnt = rPos.nContent.GetIndex();
}


SwDataChanged::~SwDataChanged()
{
    // JP 09.04.96: nur wenn das Layout vorhanden ist ( also waehrend der
    //              Eingabe)
    if( pDoc->GetRootFrm() )
    {
        const ::so3::SvLinkSources& rServers = pDoc->GetLinkManager().GetServers();

        for( USHORT nCnt = rServers.Count(); nCnt; )
        {
            ::so3::SvLinkSourceRef refObj( rServers[ --nCnt ] );
            // noch jemand am Object interessiert ?
            if( refObj->HasDataLinks() && refObj->ISA( SwServerObject ))
            {
                SwServerObject& rObj = *(SwServerObject*)&refObj;
                if( pPos )
                    rObj.SendDataChanged( *pPos );
                else
                    rObj.SendDataChanged( *pPam );
            }

            // sollte jetzt gar keine Verbindung mehr bestehen
            if( !refObj->HasDataLinks() )
            {
                // dann raus aus der Liste (Object bleibt aber bestehen!)
                // falls es noch da ist !!
                if( nCnt < rServers.Count() && &refObj == rServers[ nCnt ] )
                    pDoc->GetLinkManager().RemoveServer( nCnt, 1 );
            }
        }
    }
}




