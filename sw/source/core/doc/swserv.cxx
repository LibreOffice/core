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

#include <sot/exchange.hxx>
#include <sfx2/linkmgr.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <doc.hxx>
#include <IDocumentLinksAdministration.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <swserv.hxx>
#include <swbaslnk.hxx>
#include <mvsave.hxx>
#include <IMark.hxx>
#include <bookmark.hxx>
#include <pam.hxx>
#include <shellio.hxx>

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
        ::GetASCWriter( std::u16string_view(), OUString(), xWrt );
        break;

    case SotClipboardFormatId::RTF:
    case SotClipboardFormatId::RICHTEXT:
        // mba: no BaseURL for data exchange
        ::GetRTFWriter( std::u16string_view(), OUString(), xWrt );
        break;
    default: break;
    }

    if( xWrt.is() )
    {
        SwPaM* pPam = nullptr;
        switch( m_eType )
        {
        case BOOKMARK_SERVER:
            if( m_CNTNT_TYPE.pBkmk->IsExpanded() )
            {
                // Span area
                pPam = new SwPaM( m_CNTNT_TYPE.pBkmk->GetMarkPos(),
                                m_CNTNT_TYPE.pBkmk->GetOtherMarkPos() );
            }
            break;

        case TABLE_SERVER:
            pPam = new SwPaM( *m_CNTNT_TYPE.pTableNd,
                            *m_CNTNT_TYPE.pTableNd->EndOfSectionNode() );
            break;

        case SECTION_SERVER:
            pPam = new SwPaM( SwPosition( *m_CNTNT_TYPE.pSectNd ) );
            pPam->Move( fnMoveForward );
            pPam->SetMark();
            pPam->GetPoint()->Assign( *m_CNTNT_TYPE.pSectNd->EndOfSectionNode() );
            pPam->Move( fnMoveBackward );
            break;
        case NONE_SERVER: break;
        }

        if( pPam )
        {
            // Create stream
            SvMemoryStream aMemStm( 65535, 65535 );
            SwWriter aWrt( aMemStm, *pPam, false );
            if( !aWrt.Write( xWrt ).IsError() )
            {
                aMemStm.WriteChar( '\0' );        // append a zero char
                rData <<= uno::Sequence< sal_Int8 >(
                                        static_cast<sal_Int8 const *>(aMemStm.GetData()),
                                        aMemStm.Tell() );
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
    if( !HasDataLinks() )
        return;

    bool bCall = false;
    const SwStartNode* pNd = nullptr;
    switch( m_eType )
    {
        case BOOKMARK_SERVER:
            if( m_CNTNT_TYPE.pBkmk->IsExpanded() )
            {
                bCall = m_CNTNT_TYPE.pBkmk->GetMarkStart() <= rPos
                    && rPos < m_CNTNT_TYPE.pBkmk->GetMarkEnd();
            }
            break;

        case TABLE_SERVER:      pNd = m_CNTNT_TYPE.pTableNd;    break;
        case SECTION_SERVER:    pNd = m_CNTNT_TYPE.pSectNd;   break;
        case NONE_SERVER: break;
    }
    if( pNd )
    {
        SwNodeOffset nNd = rPos.GetNodeIndex();
        bCall = pNd->GetIndex() < nNd && nNd < pNd->EndOfSectionIndex();
    }

    if( bCall )
    {
        // Recognize recursions and flag them
        IsLinkInServer( nullptr );
        SvLinkSource::NotifyDataChanged();
    }
}

void SwServerObject::SendDataChanged( const SwPaM& rRange )
{
    // Is someone interested in our changes?
    if( !HasDataLinks() )
        return;

    bool bCall = false;
    const SwStartNode* pNd = nullptr;
    auto [pStt, pEnd] = rRange.StartEnd(); // SwPosition*
    switch( m_eType )
    {
    case BOOKMARK_SERVER:
        if(m_CNTNT_TYPE.pBkmk->IsExpanded())
        {
            bCall = *pStt <= m_CNTNT_TYPE.pBkmk->GetMarkEnd()
                && *pEnd > m_CNTNT_TYPE.pBkmk->GetMarkStart();
        }
        break;

    case TABLE_SERVER:      pNd = m_CNTNT_TYPE.pTableNd;    break;
    case SECTION_SERVER:    pNd = m_CNTNT_TYPE.pSectNd;   break;
    case NONE_SERVER: break;
    }
    if( pNd )
    {
        // Is the start area within the node area?
        bCall = pStt->GetNodeIndex() <  pNd->EndOfSectionIndex() &&
                pEnd->GetNodeIndex() >= pNd->GetIndex();
    }

    if( bCall )
    {
        // Recognize recursions and flag them
        IsLinkInServer( nullptr );
        SvLinkSource::NotifyDataChanged();
    }
}

bool SwServerObject::IsLinkInServer( const SwBaseLink* pChkLnk ) const
{
    SwNodeOffset nSttNd(0), nEndNd(0);
    const SwNode* pNd = nullptr;
    const SwNodes* pNds = nullptr;

    switch( m_eType )
    {
    case BOOKMARK_SERVER:
        if( m_CNTNT_TYPE.pBkmk->IsExpanded() )
        {
            const SwPosition* pStt = &m_CNTNT_TYPE.pBkmk->GetMarkStart(),
                            * pEnd = &m_CNTNT_TYPE.pBkmk->GetMarkEnd();

            nSttNd = pStt->GetNodeIndex();
            nEndNd = pEnd->GetNodeIndex();
            pNds = &pStt->GetNodes();
        }
        break;

    case TABLE_SERVER:      pNd = m_CNTNT_TYPE.pTableNd;    break;
    case SECTION_SERVER:    pNd = m_CNTNT_TYPE.pSectNd;   break;

    case SECTION_SERVER+1:
        return true;
    }

    if( pNd )
    {
        nSttNd = pNd->GetIndex();
        nEndNd = pNd->EndOfSectionIndex();
        pNds = &pNd->GetNodes();
    }

    if( nSttNd && nEndNd )
    {
        assert(pNds);
        // Get LinkManager
        const ::sfx2::SvBaseLinks& rLnks = pNds->GetDoc().getIDocumentLinksAdministration().GetLinkManager().GetLinks();

        // To avoid recursions: convert ServerType!
        SwServerObject::ServerModes eSave = m_eType;
        if( !pChkLnk )
            const_cast<SwServerObject*>(this)->m_eType = NONE_SERVER;
        for( size_t n = rLnks.size(); n; )
        {
            const ::sfx2::SvBaseLink* pLnk = &(*rLnks[ --n ]);
            if (sfx2::SvBaseLinkObjectType::ClientGraphic != pLnk->GetObjType() &&
                dynamic_cast<const SwBaseLink*>( pLnk) !=  nullptr &&
                !static_cast<const SwBaseLink*>(pLnk)->IsNoDataFlag() &&
                static_cast<const SwBaseLink*>(pLnk)->IsInRange( nSttNd, nEndNd ))
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
            const_cast<SwServerObject*>(this)->m_eType = eSave;
    }

    return false;
}

void SwServerObject::SetNoServer()
{
    if(m_eType == BOOKMARK_SERVER && m_CNTNT_TYPE.pBkmk)
    {
        ::sw::mark::DdeBookmark* const pDdeBookmark = dynamic_cast< ::sw::mark::DdeBookmark* >(m_CNTNT_TYPE.pBkmk);
        if(pDdeBookmark)
        {
            m_CNTNT_TYPE.pBkmk = nullptr;
            m_eType = NONE_SERVER;
            pDdeBookmark->SetRefObject(nullptr);
        }
    }
}

void SwServerObject::SetDdeBookmark( ::sw::mark::IMark& rBookmark)
{
    ::sw::mark::DdeBookmark* const pDdeBookmark = dynamic_cast< ::sw::mark::DdeBookmark* >(&rBookmark);
    if(pDdeBookmark)
    {
        m_eType = BOOKMARK_SERVER;
        m_CNTNT_TYPE.pBkmk = &rBookmark;
        pDdeBookmark->SetRefObject(this);
    }
    else
        OSL_FAIL("SwServerObject::SetNoServer(..)"
            " - setting a bookmark that is not DDE-capable");
}

SwDataChanged::SwDataChanged( const SwPaM& rPam )
    : m_pPam( &rPam ), m_pPos( nullptr ), m_rDoc( rPam.GetDoc() )
{
    m_nContent = rPam.GetPoint()->GetContentIndex();
}

SwDataChanged::SwDataChanged( SwDoc& rDc, const SwPosition& rPos )
    : m_pPam( nullptr ), m_pPos( &rPos ), m_rDoc( rDc )
{
    m_nContent = rPos.GetContentIndex();
}

SwDataChanged::~SwDataChanged()
{
    // JP 09.04.96: Only if the Layout is available (thus during input)
    if( !m_rDoc.getIDocumentLayoutAccess().GetCurrentViewShell() )
        return;

    const ::sfx2::SvLinkSources& rServers = m_rDoc.getIDocumentLinksAdministration().GetLinkManager().GetServers();

    ::sfx2::SvLinkSources aTemp(rServers);
    for( const auto& rpLinkSrc : aTemp )
    {
        ::sfx2::SvLinkSourceRef refObj( rpLinkSrc );
        // Anyone else interested in the Object?
        if( refObj->HasDataLinks())
            if (auto pServerObj = dynamic_cast<SwServerObject*>( refObj.get() ))
            {
                if( m_pPos )
                    pServerObj->SendDataChanged( *m_pPos );
                else
                    pServerObj->SendDataChanged( *m_pPam );
            }

        // We shouldn't have a connection anymore
        if( !refObj->HasDataLinks() )
        {
            // Then remove from the list
            m_rDoc.getIDocumentLinksAdministration().GetLinkManager().RemoveServer( rpLinkSrc );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
