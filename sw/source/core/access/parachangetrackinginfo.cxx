/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
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

#include "precompiled_sw.hxx"

#include <parachangetrackinginfo.hxx>

#include <wrong.hxx>
#include <com/sun/star/text/TextMarkupType.hpp>

#include <txtfrm.hxx>
#include <ndtxt.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <docary.hxx>
#include <redline.hxx>

namespace css = com::sun::star;

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
            OSL_ENSURE( false,
                    "<initChangeTrackTextMarkupLists(..) - missing <SwTxtNode> instance!" );
            return;
        }
        const SwTxtNode& rTxtNode( *(rTxtFrm.GetTxtNode()) );

        const IDocumentRedlineAccess* pIDocChangeTrack( rTxtNode.getIDocumentRedlineAccess() );
        if ( !pIDocChangeTrack )
        {
            OSL_ENSURE( false,
                    "<initChangeTrackTextMarkupLists(..) - missing <IDocumentRedlineAccess> instance!" );
            return;
        }

        if ( !IDocumentRedlineAccess::IsShowChanges( pIDocChangeTrack->GetRedlineMode() ) ||
             pIDocChangeTrack->GetRedlineTbl().Count() == 0 )
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
        const USHORT nRedlineCount( rRedlineTbl.Count() );
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

                pMarkupList->Insert( rtl::OUString(), 0,
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
            OSL_ENSURE( false,
                    "<SwParaChangeTrackingInfo::getChangeTrackingTextMarkupList(..)> - misusage - unexpected text markup type for change tracking." );
        }
    }

    return pChangeTrackingTextMarkupList;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
