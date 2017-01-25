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
#include <IDocumentLinksAdministration.hxx>
#include <IDocumentLayoutAccess.hxx>
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

SwServerObject::~SwServerObject()
{
}

bool SwServerObject::GetData( uno::Any & rData,
                                const OUString & rMimeType, bool )
{
    bool bRet = false;
    WriterRef xWrt;
    switch( SotExchange::GetFormatIdFromMimeType( rMimeType ) )
    {
    case SotClipboardFormatId::STRING:
        ::GetASCWriter( OUString(), OUString(), xWrt );
        break;

    case SotClipboardFormatId::RTF:
    case SotClipboardFormatId::RICHTEXT:
        // mba: no BaseURL for data exchange
        ::GetRTFWriter( OUString(), OUString(), xWrt );
        break;
    default: break;
    }

    if( xWrt.is() )
    {
        SwPaM* pPam = nullptr;
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
            pPam = new SwPaM( *CNTNT_TYPE.pTableNd,
                            *CNTNT_TYPE.pTableNd->EndOfSectionNode() );
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
            SwWriter aWrt( aMemStm, *pPam, false );
            if( !IsError( aWrt.Write( xWrt )) )
            {
                aMemStm.WriteChar( '\0' );        // append a zero char
                rData <<= uno::Sequence< sal_Int8 >(
                                        static_cast<sal_Int8 const *>(aMemStm.GetData()),
                                        aMemStm.Seek( STREAM_SEEK_TO_END ) );
                bRet = true;
            }
            delete pPam;
        }
    }
    return bRet;
}

void SwServerObject::SendDataChanged( const SwPosition& rPos )
{
    // Is someone interested in our changes?
    if( HasDataLinks() )
    {
        bool bCall = false;
        const SwStartNode* pNd = nullptr;
        switch( eType )
        {
            case BOOKMARK_SERVER:
                if( CNTNT_TYPE.pBkmk->IsExpanded() )
                {
                    bCall = CNTNT_TYPE.pBkmk->GetMarkStart() <= rPos
                        && rPos < CNTNT_TYPE.pBkmk->GetMarkEnd();
                }
                break;

            case TABLE_SERVER:      pNd = CNTNT_TYPE.pTableNd;    break;
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
            IsLinkInServer( nullptr );
            SvLinkSource::NotifyDataChanged();
        }
    }
}

void SwServerObject::SendDataChanged( const SwPaM& rRange )
{
    // Is someone interested in our changes?
    if( HasDataLinks() )
    {
        bool bCall = false;
        const SwStartNode* pNd = nullptr;
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

        case TABLE_SERVER:      pNd = CNTNT_TYPE.pTableNd;    break;
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
            IsLinkInServer( nullptr );
            SvLinkSource::NotifyDataChanged();
        }
    }
}

bool SwServerObject::IsLinkInServer( const SwBaseLink* pChkLnk ) const
{
    sal_uLong nSttNd = 0, nEndNd = 0;
    sal_Int32 nStt = 0;
    sal_Int32 nEnd = 0;
    const SwNode* pNd = nullptr;
    const SwNodes* pNds = nullptr;

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

    case TABLE_SERVER:      pNd = CNTNT_TYPE.pTableNd;    break;
    case SECTION_SERVER:    pNd = CNTNT_TYPE.pSectNd;   break;

    case SECTION_SERVER+1:
        return true;
    }

    if( pNd )
    {
        nSttNd = pNd->GetIndex();
        nEndNd = pNd->EndOfSectionIndex();
        nStt = 0;
        nEnd = -1;
        pNds = &pNd->GetNodes();
    }

    if( nSttNd && nEndNd )
    {
        // Get LinkManager
        const ::sfx2::SvBaseLinks& rLnks = pNds->GetDoc()->getIDocumentLinksAdministration().GetLinkManager().GetLinks();

        // To avoid recursions: convert ServerType!
        SwServerObject::ServerModes eSave = eType;
        if( !pChkLnk )
            const_cast<SwServerObject*>(this)->eType = NONE_SERVER;
        for( size_t n = rLnks.size(); n; )
        {
            const ::sfx2::SvBaseLink* pLnk = &(*rLnks[ --n ]);
            if( pLnk && OBJECT_CLIENT_GRF != pLnk->GetObjType() &&
                dynamic_cast<const SwBaseLink*>( pLnk) !=  nullptr &&
                !static_cast<const SwBaseLink*>(pLnk)->IsNoDataFlag() &&
                static_cast<const SwBaseLink*>(pLnk)->IsInRange( nSttNd, nEndNd, nStt, nEnd ))
            {
                if( pChkLnk )
                {
                    if( pLnk == pChkLnk ||
                        static_cast<const SwBaseLink*>(pLnk)->IsRecursion( pChkLnk ) )
                        return true;
                }
                else if( static_cast<const SwBaseLink*>(pLnk)->IsRecursion( static_cast<const SwBaseLink*>(pLnk) ) )
                    const_cast<SwBaseLink*>(static_cast<const SwBaseLink*>(pLnk))->SetNoDataFlag();
            }
        }
        if( !pChkLnk )
            const_cast<SwServerObject*>(this)->eType = eSave;
    }

    return false;
}

void SwServerObject::SetNoServer()
{
    if(eType == BOOKMARK_SERVER && CNTNT_TYPE.pBkmk)
    {
        ::sw::mark::DdeBookmark* const pDdeBookmark = dynamic_cast< ::sw::mark::DdeBookmark* >(CNTNT_TYPE.pBkmk);
        if(pDdeBookmark)
        {
            CNTNT_TYPE.pBkmk = nullptr;
            eType = NONE_SERVER;
            pDdeBookmark->SetRefObject(nullptr);
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
    : pPam( &rPam ), pPos( nullptr ), pDoc( rPam.GetDoc() )
{
    nNode = rPam.GetPoint()->nNode.GetIndex();
    nContent = rPam.GetPoint()->nContent.GetIndex();
}

SwDataChanged::SwDataChanged( SwDoc* pDc, const SwPosition& rPos )
    : pPam( nullptr ), pPos( &rPos ), pDoc( pDc )
{
    nNode = rPos.nNode.GetIndex();
    nContent = rPos.nContent.GetIndex();
}

SwDataChanged::~SwDataChanged()
{
    // JP 09.04.96: Only if the Layout is available (thus during input)
    if( pDoc->getIDocumentLayoutAccess().GetCurrentViewShell() )
    {
        const ::sfx2::SvLinkSources& rServers = pDoc->getIDocumentLinksAdministration().GetLinkManager().GetServers();

        ::sfx2::SvLinkSources aTemp(rServers);
        for( ::sfx2::SvLinkSources::const_iterator it = aTemp.begin(); it != aTemp.end(); ++it )
        {
            ::sfx2::SvLinkSourceRef refObj( *it );
            // Any one else interested in the Object?
            if( refObj->HasDataLinks() && dynamic_cast<const SwServerObject*>( refObj.get() ) !=  nullptr)
            {
                SwServerObject& rObj = *static_cast<SwServerObject*>( refObj.get() );
                if( pPos )
                    rObj.SendDataChanged( *pPos );
                else
                    rObj.SendDataChanged( *pPam );
            }

            // We shouldn't have a connection anymore
            if( !refObj->HasDataLinks() )
            {
                // Then remove from the list
                pDoc->getIDocumentLinksAdministration().GetLinkManager().RemoveServer( *it );
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
