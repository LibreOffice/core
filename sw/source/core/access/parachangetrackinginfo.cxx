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

#include <algorithm>

namespace {
    void initChangeTrackTextMarkupLists( const SwTextFrm& rTextFrm,
                                         SwWrongList*& opChangeTrackInsertionTextMarkupList,
                                         SwWrongList*& opChangeTrackDeletionTextMarkupList,
                                         SwWrongList*& opChangeTrackFormatChangeTextMarkupList )
    {
        opChangeTrackInsertionTextMarkupList = new SwWrongList( WRONGLIST_CHANGETRACKING );
        opChangeTrackDeletionTextMarkupList = new SwWrongList( WRONGLIST_CHANGETRACKING );
        opChangeTrackFormatChangeTextMarkupList = new SwWrongList( WRONGLIST_CHANGETRACKING );

        if ( !rTextFrm.GetTextNode() )
        {
            OSL_FAIL( "<initChangeTrackTextMarkupLists(..) - missing <SwTextNode> instance!" );
            return;
        }
        const SwTextNode& rTextNode( *(rTextFrm.GetTextNode()) );

        const IDocumentRedlineAccess& rIDocChangeTrack( rTextNode.getIDocumentRedlineAccess() );

        if ( !IDocumentRedlineAccess::IsShowChanges( rIDocChangeTrack.GetRedlineMode() ) ||
             rIDocChangeTrack.GetRedlineTable().empty() )
        {
            // nothing to do --> empty change track text markup lists.
            return;
        }

        const sal_uInt16 nIdxOfFirstRedlineForTextNode =
                    rIDocChangeTrack.GetRedlinePos( rTextNode, USHRT_MAX );
        if ( nIdxOfFirstRedlineForTextNode == USHRT_MAX )
        {
            // nothing to do --> empty change track text markup lists.
            return;
        }

        const sal_Int32 nTextFrmTextStartPos = rTextFrm.IsFollow()
                                               ? rTextFrm.GetOfst()
                                               : 0;
        const sal_Int32 nTextFrmTextEndPos = rTextFrm.HasFollow()
                                             ? rTextFrm.GetFollow()->GetOfst()
                                             : rTextFrm.GetText().getLength();

        // iteration over the redlines which overlap with the text node.
        const SwRedlineTable& rRedlineTable = rIDocChangeTrack.GetRedlineTable();
        const sal_uInt16 nRedlineCount( rRedlineTable.size() );
        for ( sal_uInt16 nActRedline = nIdxOfFirstRedlineForTextNode;
              nActRedline < nRedlineCount;
              ++nActRedline)
        {
            const SwRangeRedline* pActRedline = rRedlineTable[ nActRedline ];
            if ( pActRedline->Start()->nNode > rTextNode.GetIndex() )
            {
                break;
            }

            sal_Int32 nTextNodeChangeTrackStart(COMPLETE_STRING);
            sal_Int32 nTextNodeChangeTrackEnd(COMPLETE_STRING);
            pActRedline->CalcStartEnd( rTextNode.GetIndex(),
                                       nTextNodeChangeTrackStart,
                                       nTextNodeChangeTrackEnd );
            if ( nTextNodeChangeTrackStart > nTextFrmTextEndPos ||
                 nTextNodeChangeTrackEnd < nTextFrmTextStartPos )
            {
                // Consider only redlines which overlap with the text frame's text.
                continue;
            }

            SwWrongList* pMarkupList( nullptr );
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
                const sal_Int32 nTextFrmChangeTrackStart =
                    std::max(nTextNodeChangeTrackStart, nTextFrmTextStartPos);

                const sal_Int32 nTextFrmChangeTrackEnd =
                    std::min(nTextNodeChangeTrackEnd, nTextFrmTextEndPos);

                pMarkupList->Insert( OUString(), nullptr,
                                     nTextFrmChangeTrackStart,
                                     nTextFrmChangeTrackEnd - nTextFrmChangeTrackStart,
                                     pMarkupList->Count() );
            }
        } // eof iteration over the redlines which overlap with the text node
    }
} // eof anonymous namespace

SwParaChangeTrackingInfo::SwParaChangeTrackingInfo( const SwTextFrm& rTextFrm )
    : mrTextFrm( rTextFrm )
    , mpChangeTrackInsertionTextMarkupList( nullptr )
    , mpChangeTrackDeletionTextMarkupList( nullptr )
    , mpChangeTrackFormatChangeTextMarkupList( nullptr )
{
}

SwParaChangeTrackingInfo::~SwParaChangeTrackingInfo()
{
    reset();
}

void SwParaChangeTrackingInfo::reset()
{
    delete mpChangeTrackInsertionTextMarkupList;
    mpChangeTrackInsertionTextMarkupList = nullptr;

    delete mpChangeTrackDeletionTextMarkupList;
    mpChangeTrackDeletionTextMarkupList = nullptr;

    delete mpChangeTrackFormatChangeTextMarkupList;
    mpChangeTrackFormatChangeTextMarkupList = nullptr;
}

const SwWrongList* SwParaChangeTrackingInfo::getChangeTrackingTextMarkupList( const sal_Int32 nTextMarkupType )
{
    SwWrongList* pChangeTrackingTextMarkupList = nullptr;

    if ( mpChangeTrackInsertionTextMarkupList == nullptr )
    {
        OSL_ENSURE( mpChangeTrackDeletionTextMarkupList == nullptr,
                "<SwParaChangeTrackingInfo::getChangeTrackingTextMarkupList(..) - <mpChangeTrackDeletionTextMarkupList> expected to be NULL." );
        OSL_ENSURE( mpChangeTrackFormatChangeTextMarkupList == nullptr,
                "<SwParaChangeTrackingInfo::getChangeTrackingTextMarkupList(..) - <mpChangeTrackFormatChangeTextMarkupList> expected to be NULL." );
        initChangeTrackTextMarkupLists( mrTextFrm,
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
