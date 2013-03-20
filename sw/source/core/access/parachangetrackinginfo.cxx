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

#include <parachangetrackinginfo.hxx>

#include <wrong.hxx>
#include <com/sun/star/text/TextMarkupType.hpp>

#include <txtfrm.hxx>
#include <ndtxt.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <docary.hxx>
#include <redline.hxx>

namespace {
    void initChangeTrackTextMarkupLists( const SwTxtFrm& rTxtFrm,
                                         SwWrongList*& opChangeTrackInsertionTextMarkupList,
                                         SwWrongList*& opChangeTrackDeletionTextMarkupList,
                                         SwWrongList*& opChangeTrackFormatChangeTextMarkupList )
    {
        opChangeTrackInsertionTextMarkupList = new SwWrongList( WRONGLIST_CHANGETRACKING );
        opChangeTrackDeletionTextMarkupList = new SwWrongList( WRONGLIST_CHANGETRACKING );
        opChangeTrackFormatChangeTextMarkupList = new SwWrongList( WRONGLIST_CHANGETRACKING );

        if ( !rTxtFrm.GetTxtNode() )
        {
            OSL_FAIL( "<initChangeTrackTextMarkupLists(..) - missing <SwTxtNode> instance!" );
            return;
        }
        const SwTxtNode& rTxtNode( *(rTxtFrm.GetTxtNode()) );

        const IDocumentRedlineAccess* pIDocChangeTrack( rTxtNode.getIDocumentRedlineAccess() );
        if ( !pIDocChangeTrack )
        {
            OSL_FAIL( "<initChangeTrackTextMarkupLists(..) - missing <IDocumentRedlineAccess> instance!" );
            return;
        }

        if ( !IDocumentRedlineAccess::IsShowChanges( pIDocChangeTrack->GetRedlineMode() ) ||
             pIDocChangeTrack->GetRedlineTbl().empty() )
        {
            // nothing to do --> empty change track text markup lists.
            return;
        }

        const sal_uInt16 nIdxOfFirstRedlineForTxtNode =
                    pIDocChangeTrack->GetRedlinePos( rTxtNode, USHRT_MAX );
        if ( nIdxOfFirstRedlineForTxtNode == USHRT_MAX )
        {
            // nothing to do --> empty change track text markup lists.
            return;
        }

        const xub_StrLen nTxtFrmTextStartPos = rTxtFrm.IsFollow()
                                               ? rTxtFrm.GetOfst()
                                               : 0;
        const xub_StrLen nTxtFrmTextEndPos = rTxtFrm.HasFollow()
                                             ? rTxtFrm.GetFollow()->GetOfst()
                                             : rTxtFrm.GetTxt().Len();

        // iteration over the redlines which overlap with the text node.
        const SwRedlineTbl& rRedlineTbl = pIDocChangeTrack->GetRedlineTbl();
        const sal_uInt16 nRedlineCount( rRedlineTbl.size() );
        for ( sal_uInt16 nActRedline = nIdxOfFirstRedlineForTxtNode;
              nActRedline < nRedlineCount;
              ++nActRedline)
        {
            const SwRedline* pActRedline = rRedlineTbl[ nActRedline ];
            if ( pActRedline->Start()->nNode > rTxtNode.GetIndex() )
            {
                break;
            }

            xub_StrLen nTxtNodeChangeTrackStart( STRING_LEN );
            xub_StrLen nTxtNodeChangeTrackEnd( STRING_LEN );
            pActRedline->CalcStartEnd( rTxtNode.GetIndex(),
                                       nTxtNodeChangeTrackStart,
                                       nTxtNodeChangeTrackEnd );
            if ( nTxtNodeChangeTrackStart > nTxtFrmTextEndPos ||
                 nTxtNodeChangeTrackEnd < nTxtFrmTextStartPos )
            {
                // Consider only redlines which overlap with the text frame's text.
                continue;
            }

            SwWrongList* pMarkupList( 0 );
            switch ( pActRedline->GetType() )
            {
                case nsRedlineType_t::REDLINE_INSERT:
                {
                    pMarkupList = opChangeTrackInsertionTextMarkupList;
                }
                break;
                case nsRedlineType_t::REDLINE_DELETE:
                {
                    pMarkupList = opChangeTrackDeletionTextMarkupList;
                }
                break;
                case nsRedlineType_t::REDLINE_FORMAT:
                {
                    pMarkupList = opChangeTrackFormatChangeTextMarkupList;
                }
                break;
                default:
                {
                    // other types are not considered
                }
            }
            if ( pMarkupList )
            {
                const xub_StrLen nTxtFrmChangeTrackStart =
                                    nTxtNodeChangeTrackStart <= nTxtFrmTextStartPos
                                    ? nTxtFrmTextStartPos
                                    : nTxtNodeChangeTrackStart;

                const xub_StrLen nTxtFrmChangeTrackEnd =
                                    nTxtNodeChangeTrackEnd >= nTxtFrmTextEndPos
                                    ? nTxtFrmTextEndPos
                                    : nTxtNodeChangeTrackEnd;

                pMarkupList->Insert( OUString(), 0,
                                     nTxtFrmChangeTrackStart,
                                     nTxtFrmChangeTrackEnd - nTxtFrmChangeTrackStart,
                                     pMarkupList->Count() );
            }
        } // eof iteration over the redlines which overlap with the text node
    }
} // eof anonymous namespace

SwParaChangeTrackingInfo::SwParaChangeTrackingInfo( const SwTxtFrm& rTxtFrm )
    : mrTxtFrm( rTxtFrm )
    , mpChangeTrackInsertionTextMarkupList( 0 )
    , mpChangeTrackDeletionTextMarkupList( 0 )
    , mpChangeTrackFormatChangeTextMarkupList( 0 )
{
}

SwParaChangeTrackingInfo::~SwParaChangeTrackingInfo()
{
    reset();
}

void SwParaChangeTrackingInfo::reset()
{
    delete mpChangeTrackInsertionTextMarkupList;
    mpChangeTrackInsertionTextMarkupList = 0;

    delete mpChangeTrackDeletionTextMarkupList;
    mpChangeTrackDeletionTextMarkupList = 0;

    delete mpChangeTrackFormatChangeTextMarkupList;
    mpChangeTrackFormatChangeTextMarkupList = 0;
}

const SwWrongList* SwParaChangeTrackingInfo::getChangeTrackingTextMarkupList( const sal_Int32 nTextMarkupType )
{
    SwWrongList* pChangeTrackingTextMarkupList = 0;

    if ( mpChangeTrackInsertionTextMarkupList == 0 )
    {
        OSL_ENSURE( mpChangeTrackDeletionTextMarkupList == 0,
                "<SwParaChangeTrackingInfo::getChangeTrackingTextMarkupList(..) - <mpChangeTrackDeletionTextMarkupList> expected to be NULL." );
        OSL_ENSURE( mpChangeTrackFormatChangeTextMarkupList == 0,
                "<SwParaChangeTrackingInfo::getChangeTrackingTextMarkupList(..) - <mpChangeTrackFormatChangeTextMarkupList> expected to be NULL." );
        initChangeTrackTextMarkupLists( mrTxtFrm,
                                        mpChangeTrackInsertionTextMarkupList,
                                        mpChangeTrackDeletionTextMarkupList,
                                        mpChangeTrackFormatChangeTextMarkupList );
    }

    switch ( nTextMarkupType )
    {
        case css::text::TextMarkupType::TRACK_CHANGE_INSERTION:
        {
            pChangeTrackingTextMarkupList = mpChangeTrackInsertionTextMarkupList;
        }
        break;
        case css::text::TextMarkupType::TRACK_CHANGE_DELETION:
        {
            pChangeTrackingTextMarkupList = mpChangeTrackDeletionTextMarkupList;
        }
        break;
        case css::text::TextMarkupType::TRACK_CHANGE_FORMATCHANGE:
        {
            pChangeTrackingTextMarkupList = mpChangeTrackFormatChangeTextMarkupList;
        }
        break;
        default:
        {
            OSL_FAIL( "<SwParaChangeTrackingInfo::getChangeTrackingTextMarkupList(..)> - misusage - unexpected text markup type for change tracking." );
        }
    }

    return pChangeTrackingTextMarkupList;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
