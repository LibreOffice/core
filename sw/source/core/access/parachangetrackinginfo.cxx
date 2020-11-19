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

#include "parachangetrackinginfo.hxx"

#include <wrong.hxx>
#include <com/sun/star/text/TextMarkupType.hpp>
#include <osl/diagnose.h>

#include <txtfrm.hxx>
#include <rootfrm.hxx>
#include <ndtxt.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <docary.hxx>
#include <redline.hxx>

#include <algorithm>

namespace {
    void initChangeTrackTextMarkupLists( const SwTextFrame& rTextFrame,
                                         std::unique_ptr<SwWrongList>& opChangeTrackInsertionTextMarkupList,
                                         std::unique_ptr<SwWrongList>& opChangeTrackDeletionTextMarkupList,
                                         std::unique_ptr<SwWrongList>& opChangeTrackFormatChangeTextMarkupList )
    {
        opChangeTrackInsertionTextMarkupList.reset( new SwWrongList( WRONGLIST_CHANGETRACKING ) );
        opChangeTrackDeletionTextMarkupList.reset( new SwWrongList( WRONGLIST_CHANGETRACKING ) );
        opChangeTrackFormatChangeTextMarkupList.reset( new SwWrongList( WRONGLIST_CHANGETRACKING ) );

        if (!rTextFrame.GetTextNodeFirst())
        {
            OSL_FAIL( "<initChangeTrackTextMarkupLists(..) - missing <SwTextNode> instance!" );
            return;
        }
        // sw_redlinehide: the first node is sufficient - there are only
        // multiple ones in Hide case and the code below returns early then
        const SwTextNode& rTextNode(*(rTextFrame.GetTextNodeFirst()));

        const IDocumentRedlineAccess& rIDocChangeTrack( rTextNode.getIDocumentRedlineAccess() );

        if (!IDocumentRedlineAccess::IsShowChanges(rIDocChangeTrack.GetRedlineFlags())
           || rTextFrame.getRootFrame()->IsHideRedlines()
           || rIDocChangeTrack.GetRedlineTable().empty())
        {
            // nothing to do --> empty change track text markup lists.
            return;
        }

        const SwRedlineTable::size_type nIdxOfFirstRedlineForTextNode =
                    rIDocChangeTrack.GetRedlinePos( rTextNode, RedlineType::Any );
        if ( nIdxOfFirstRedlineForTextNode == SwRedlineTable::npos )
        {
            // nothing to do --> empty change track text markup lists.
            return;
        }

        // sw_redlinehide: rely on the Hide early return above & cast
        // TextFrameIndex to SwIndex directly
        const sal_Int32 nTextFrameTextStartPos = rTextFrame.IsFollow()
            ? sal_Int32(rTextFrame.GetOffset())
            : 0;
        const sal_Int32 nTextFrameTextEndPos = rTextFrame.HasFollow()
            ? sal_Int32(rTextFrame.GetFollow()->GetOffset())
            : rTextFrame.GetText().getLength();

        // iteration over the redlines which overlap with the text node.
        const SwRedlineTable& rRedlineTable = rIDocChangeTrack.GetRedlineTable();
        const SwRedlineTable::size_type nRedlineCount( rRedlineTable.size() );
        for ( SwRedlineTable::size_type nActRedline = nIdxOfFirstRedlineForTextNode;
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
            if ( nTextNodeChangeTrackStart > nTextFrameTextEndPos ||
                 nTextNodeChangeTrackEnd < nTextFrameTextStartPos )
            {
                // Consider only redlines which overlap with the text frame's text.
                continue;
            }

            SwWrongList* pMarkupList( nullptr );
            switch ( pActRedline->GetType() )
            {
                case RedlineType::Insert:
                {
                    pMarkupList = opChangeTrackInsertionTextMarkupList.get();
                }
                break;
                case RedlineType::Delete:
                {
                    pMarkupList = opChangeTrackDeletionTextMarkupList.get();
                }
                break;
                case RedlineType::Format:
                {
                    pMarkupList = opChangeTrackFormatChangeTextMarkupList.get();
                }
                break;
                default:
                {
                    // other types are not considered
                }
            }
            if ( pMarkupList )
            {
                const sal_Int32 nTextFrameChangeTrackStart =
                    std::max(nTextNodeChangeTrackStart, nTextFrameTextStartPos);

                const sal_Int32 nTextFrameChangeTrackEnd =
                    std::min(nTextNodeChangeTrackEnd, nTextFrameTextEndPos);

                pMarkupList->Insert( OUString(), nullptr,
                                     nTextFrameChangeTrackStart,
                                     nTextFrameChangeTrackEnd - nTextFrameChangeTrackStart,
                                     pMarkupList->Count() );
            }
        } // eof iteration over the redlines which overlap with the text node
    }
} // eof anonymous namespace

SwParaChangeTrackingInfo::SwParaChangeTrackingInfo( const SwTextFrame& rTextFrame )
    : mrTextFrame( rTextFrame )
{
}

SwParaChangeTrackingInfo::~SwParaChangeTrackingInfo()
{
    reset();
}

void SwParaChangeTrackingInfo::reset()
{
    mpChangeTrackInsertionTextMarkupList.reset();
    mpChangeTrackDeletionTextMarkupList.reset();
    mpChangeTrackFormatChangeTextMarkupList.reset();
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
        initChangeTrackTextMarkupLists( mrTextFrame,
                                        mpChangeTrackInsertionTextMarkupList,
                                        mpChangeTrackDeletionTextMarkupList,
                                        mpChangeTrackFormatChangeTextMarkupList );
    }

    switch ( nTextMarkupType )
    {
        case css::text::TextMarkupType::TRACK_CHANGE_INSERTION:
        {
            pChangeTrackingTextMarkupList = mpChangeTrackInsertionTextMarkupList.get();
        }
        break;
        case css::text::TextMarkupType::TRACK_CHANGE_DELETION:
        {
            pChangeTrackingTextMarkupList = mpChangeTrackDeletionTextMarkupList.get();
        }
        break;
        case css::text::TextMarkupType::TRACK_CHANGE_FORMATCHANGE:
        {
            pChangeTrackingTextMarkupList = mpChangeTrackFormatChangeTextMarkupList.get();
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
