/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <sot/storage.hxx>
#include <sfx2/linkmgr.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <doc.hxx>
#include <swtypes.hxx>
#include <swserv.hxx>
#include <swbaslnk.hxx>
#include <mvsave.hxx>
#include <IMark.hxx>
#include <bookmrk.hxx>
#include <pam.hxx>
#include <shellio.hxx>
#ifndef _SWERROR_H
#include <swerror.h>
#endif

using namespace ::com::sun::star;

SV_IMPL_REF( SwServerObject )

SwServerObject::~SwServerObject()
{
}


sal_Bool SwServerObject::GetData( uno::Any & rData,
                                const String & rMimeType, sal_Bool )
{
    sal_Bool bRet = sal_False;
    WriterRef xWrt;
    switch( SotExchange::GetFormatIdFromMimeType( rMimeType ) )
    {
    case FORMAT_STRING:
        ::GetASCWriter( aEmptyStr, String(), xWrt );
        break;

    case FORMAT_RTF:
        // mba: no BaseURL for data exchange
        ::GetRTFWriter( aEmptyStr, String(), xWrt );
        break;
    }

    if( xWrt.Is() )
    {
        SwPaM* pPam = 0;
        switch( eType )
        {
        case BOOKMARK_SERVER:
            if( CNTNT_TYPE.pBkmk->IsExpanded() )
            {
                // Bereich aufspannen
                pPam = new SwPaM( CNTNT_TYPE.pBkmk->GetMarkPos(),
                                CNTNT_TYPE.pBkmk->GetOtherMarkPos() );
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
        case NONE_SERVER: break;
        }

        if( pPam )
        {
            // Stream anlegen
            SvMemoryStream aMemStm( 65535, 65535 );
            SwWriter aWrt( aMemStm, *pPam, sal_False );
            if( !IsError( aWrt.Write( xWrt )) )
            {
                aMemStm << '\0';        // append a zero char
                rData <<= uno::Sequence< sal_Int8 >(
                                        (sal_Int8*)aMemStm.GetData(),
                                        aMemStm.Seek( STREAM_SEEK_TO_END ) );
                bRet = sal_True;
            }

            delete pPam;
        }
    }
    return bRet;
}


sal_Bool SwServerObject::SetData( const String & ,
                    const uno::Any& )
{
    // set new data into the "server" -> at first nothing to do
    return sal_False;
}


void SwServerObject::SendDataChanged( const SwPosition& rPos )
{
    // ist an unseren Aenderungen jemand interessiert ?
    if( HasDataLinks() )
    {
        int bCall = sal_False;
        const SwStartNode* pNd = 0;
        switch( eType )
        {
            case BOOKMARK_SERVER:
                if( CNTNT_TYPE.pBkmk->IsExpanded() )
                {
                    bCall = CNTNT_TYPE.pBkmk->GetMarkStart() <= rPos
                        && rPos < CNTNT_TYPE.pBkmk->GetMarkEnd();
                }
                break;

            case TABLE_SERVER:      pNd = CNTNT_TYPE.pTblNd;    break;
            case SECTION_SERVER:    pNd = CNTNT_TYPE.pSectNd;   break;
            case NONE_SERVER: break;
        }
        if( pNd )
        {
            sal_uLong nNd = rPos.nNode.GetIndex();
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
        int bCall = sal_False;
        const SwStartNode* pNd = 0;
        const SwPosition* pStt = rRange.Start(), *pEnd = rRange.End();
        switch( eType )
        {
        case BOOKMARK_SERVER:
            if(CNTNT_TYPE.pBkmk->IsExpanded())
            {
                bCall = *pStt <= CNTNT_TYPE.pBkmk->GetMarkEnd()
                    && *pEnd > CNTNT_TYPE.pBkmk->GetMarkStart();
            }
            break;

        case TABLE_SERVER:      pNd = CNTNT_TYPE.pTblNd;    break;
        case SECTION_SERVER:    pNd = CNTNT_TYPE.pSectNd;   break;
        case NONE_SERVER: break;
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


sal_Bool SwServerObject::IsLinkInServer( const SwBaseLink* pChkLnk ) const
{
    sal_uLong nSttNd = 0, nEndNd = 0;
    xub_StrLen nStt = 0;
    xub_StrLen nEnd = 0;
    const SwNode* pNd = 0;
    const SwNodes* pNds = 0;

    switch( eType )
    {
    case BOOKMARK_SERVER:
        if( CNTNT_TYPE.pBkmk->IsExpanded() )
        {
            const SwPosition* pStt = &CNTNT_TYPE.pBkmk->GetMarkStart(),
                            * pEnd = &CNTNT_TYPE.pBkmk->GetMarkEnd();

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
        return sal_True;
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
        const ::sfx2::SvBaseLinks& rLnks = pNds->GetDoc()->GetLinkManager().GetLinks();

// um Rekursionen zu Verhindern: ServerType umsetzen!
SwServerObject::ServerModes eSave = eType;
if( !pChkLnk )
// sowas sollte man nicht tun, wer weiss schon, wie gross ein enum ist
// ICC nimmt keinen int
// #41723#
//  *((int*)&eType) = SECTION_SERVER+1;
    ((SwServerObject*)this)->eType = NONE_SERVER;
        for( sal_uInt16 n = rLnks.Count(); n; )
        {
            const ::sfx2::SvBaseLink* pLnk = &(*rLnks[ --n ]);
            if( pLnk && OBJECT_CLIENT_GRF != pLnk->GetObjType())
            {
                const SwBaseLink* pSwBaseLink = dynamic_cast< const SwBaseLink* >(pLnk);

                if(pSwBaseLink &&
                    !pSwBaseLink->IsNoDataFlag() &&
                    pSwBaseLink->IsInRange( nSttNd, nEndNd, nStt, nEnd ))
            {
                if( pChkLnk )
                {
                        if( pLnk == pChkLnk || pSwBaseLink->IsRecursion( pChkLnk ) )
                        return sal_True;
                }
                    else if( pSwBaseLink->IsRecursion( (SwBaseLink*)pLnk ) )
                        const_cast< SwBaseLink* >(pSwBaseLink)->SetNoDataFlag();
                }
            }
        }
if( !pChkLnk )
    //  *((int*)&eType) = eSave;
    ((SwServerObject*)this)->eType = eSave;
    }

    return sal_False;
}

void SwServerObject::SetNoServer()
{
    if(eType == BOOKMARK_SERVER && CNTNT_TYPE.pBkmk)
    {
        ::sw::mark::DdeBookmark* const pDdeBookmark = dynamic_cast< ::sw::mark::DdeBookmark* >(CNTNT_TYPE.pBkmk);
        if(pDdeBookmark)
        {
            CNTNT_TYPE.pBkmk = 0, eType = NONE_SERVER;
            pDdeBookmark->SetRefObject(NULL);
        }
    }
}

void SwServerObject::SetDdeBookmark( ::sw::mark::IMark& rBookmark)
{
    ::sw::mark::DdeBookmark* const pDdeBookmark = dynamic_cast< ::sw::mark::DdeBookmark* >(&rBookmark);
    if(pDdeBookmark)
    {
        eType = BOOKMARK_SERVER;
        CNTNT_TYPE.pBkmk = &rBookmark;
        pDdeBookmark->SetRefObject(this);
    }
    else
        OSL_ENSURE(false,
            "SwServerObject::SetNoServer(..)"
            " - setting an bookmark that is not DDE-capable");
}

/*  */


SwDataChanged::SwDataChanged( const SwPaM& rPam, sal_uInt16 nTyp )
    : pPam( &rPam ), pPos( 0 ), pDoc( rPam.GetDoc() ), nType( nTyp )
{
    nNode = rPam.GetPoint()->nNode.GetIndex();
    nCntnt = rPam.GetPoint()->nContent.GetIndex();
}


SwDataChanged::SwDataChanged( SwDoc* pDc, const SwPosition& rPos, sal_uInt16 nTyp )
    : pPam( 0 ), pPos( &rPos ), pDoc( pDc ), nType( nTyp )
{
    nNode = rPos.nNode.GetIndex();
    nCntnt = rPos.nContent.GetIndex();
}

SwDataChanged::~SwDataChanged()
{
    // JP 09.04.96: nur wenn das Layout vorhanden ist ( also waehrend der
    //              Eingabe)
    if( pDoc->GetCurrentViewShell() )   //swmod 071108//swmod 071225
    {
        const ::sfx2::SvLinkSources& rServers = pDoc->GetLinkManager().GetServers();

        for( sal_uInt16 nCnt = rServers.Count(); nCnt; )
        {
            ::sfx2::SvLinkSourceRef refObj( rServers[ --nCnt ] );
            // noch jemand am Object interessiert ?
            if( refObj->HasDataLinks() )
            {
                SwServerObject* pObj = dynamic_cast< SwServerObject* >(&refObj);

                if(pObj)
                {
                    if( pPos )
                        pObj->SendDataChanged( *pPos );
                    else
                        pObj->SendDataChanged( *pPam );
                }
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
