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
#include <swerror.h>

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
                // Span area
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
            // Create stream
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
    // Is someone interested in our changes?
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
            // Recognize recursions and flag them
            IsLinkInServer( 0 );
            SvLinkSource::NotifyDataChanged();
        }
    }
}

void SwServerObject::SendDataChanged( const SwPaM& rRange )
{
    // Is someone interested in our changes?
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
            // Is the start area within the node area?
            bCall = pStt->nNode.GetIndex() <  pNd->EndOfSectionIndex() &&
                    pEnd->nNode.GetIndex() >= pNd->GetIndex();
        }

        if( bCall )
        {
            // Recognize recursions and flag them
            IsLinkInServer( 0 );
            SvLinkSource::NotifyDataChanged();
        }
    }
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
        // Get LinkManager
        const ::sfx2::SvBaseLinks& rLnks = pNds->GetDoc()->GetLinkManager().GetLinks();

// To avoid recursions: convert ServerType!
SwServerObject::ServerModes eSave = eType;
if( !pChkLnk )
    ((SwServerObject*)this)->eType = NONE_SERVER;
        for( sal_uInt16 n = rLnks.size(); n; )
        {
            const ::sfx2::SvBaseLink* pLnk = &(*rLnks[ --n ]);
            if( pLnk && OBJECT_CLIENT_GRF != pLnk->GetObjType() &&
                pLnk->ISA( SwBaseLink ) &&
                !((SwBaseLink*)pLnk)->IsNoDataFlag() &&
                ((SwBaseLink*)pLnk)->IsInRange( nSttNd, nEndNd, nStt, nEnd ))
            {
                if( pChkLnk )
                {
                    if( pLnk == pChkLnk ||
                        ((SwBaseLink*)pLnk)->IsRecursion( pChkLnk ) )
                        return sal_True;
                }
                else if( ((SwBaseLink*)pLnk)->IsRecursion( (SwBaseLink*)pLnk ) )
                    ((SwBaseLink*)pLnk)->SetNoDataFlag();
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
        OSL_FAIL("SwServerObject::SetNoServer(..)"
            " - setting an bookmark that is not DDE-capable");
}

SwDataChanged::SwDataChanged( const SwPaM& rPam )
    : pPam( &rPam ), pPos( 0 ), pDoc( rPam.GetDoc() )
{
    nNode = rPam.GetPoint()->nNode.GetIndex();
    nCntnt = rPam.GetPoint()->nContent.GetIndex();
}

SwDataChanged::SwDataChanged( SwDoc* pDc, const SwPosition& rPos )
    : pPam( 0 ), pPos( &rPos ), pDoc( pDc )
{
    nNode = rPos.nNode.GetIndex();
    nCntnt = rPos.nContent.GetIndex();
}

SwDataChanged::~SwDataChanged()
{
    // JP 09.04.96: Only if the Layout is available (thus during input)
    if( pDoc->GetCurrentViewShell() )   //swmod 071108//swmod 071225
    {
        const ::sfx2::SvLinkSources& rServers = pDoc->GetLinkManager().GetServers();

        ::sfx2::SvLinkSources aTemp(rServers);
        for( ::sfx2::SvLinkSources::const_iterator it = aTemp.begin(); it != aTemp.end(); ++it )
        {
            ::sfx2::SvLinkSourceRef refObj( *it );
            // Any one else interested in the Object?
            if( refObj->HasDataLinks() && refObj->ISA( SwServerObject ))
            {
                SwServerObject& rObj = *(SwServerObject*)&refObj;
                if( pPos )
                    rObj.SendDataChanged( *pPos );
                else
                    rObj.SendDataChanged( *pPam );
            }

            // We shouldn't have a connection anymore
            if( !refObj->HasDataLinks() )
            {
                // Then remove from the list
                pDoc->GetLinkManager().RemoveServer( *it );
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
