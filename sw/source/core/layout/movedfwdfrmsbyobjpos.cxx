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

#include <movedfwdfrmsbyobjpos.hxx>
#include <txtfrm.hxx>
#include <rowfrm.hxx>
#include <pagefrm.hxx>
#include <calbck.hxx>
#include <ndtxt.hxx>

SwMovedFwdFramesByObjPos::SwMovedFwdFramesByObjPos()
{
}

SwMovedFwdFramesByObjPos::~SwMovedFwdFramesByObjPos()
{
    Clear();
}

void SwMovedFwdFramesByObjPos::Insert( const SwTextFrame& _rMovedFwdFrameByObjPos,
                                     const sal_uInt32 _nToPageNum )
{
    maMovedFwdFrames.emplace(_rMovedFwdFrameByObjPos.GetTextNodeFirst(), _nToPageNum);
}

void SwMovedFwdFramesByObjPos::Remove( const SwTextFrame& _rTextFrame )
{
    maMovedFwdFrames.erase(_rTextFrame.GetTextNodeFirst());
}

bool SwMovedFwdFramesByObjPos::FrameMovedFwdByObjPos( const SwTextFrame& _rTextFrame,
                                                  sal_uInt32& _ornToPageNum ) const
{
    // sw_redlinehide: assumption: this wants to uniquely identify all
    // SwTextFrame belonging to the same paragraph, so just use first one as key
    auto aIter = maMovedFwdFrames.find( _rTextFrame.GetTextNodeFirst() );
    if ( maMovedFwdFrames.end() != aIter )
    {
        _ornToPageNum = (*aIter).second;
        return true;
    }

    return false;
}

// #i26945#
bool SwMovedFwdFramesByObjPos::DoesRowContainMovedFwdFrame( const SwRowFrame& _rRowFrame ) const
{
    bool bDoesRowContainMovedFwdFrame( false );

    const sal_uInt32 nPageNumOfRow = _rRowFrame.FindPageFrame()->GetPhyPageNum();

    for ( const auto & rEntry : maMovedFwdFrames )
    {
        if ( rEntry.second >= nPageNumOfRow )
        {
            SwIterator<SwTextFrame, SwTextNode, sw::IteratorMode::UnwrapMulti> aFrameIter(*rEntry.first);
            for( SwTextFrame* pTextFrame = aFrameIter.First(); pTextFrame; pTextFrame = aFrameIter.Next() )
            {
                // #115759# - assure that found text frame
                // is the first one.
                if ( _rRowFrame.IsAnLower( pTextFrame ) && !pTextFrame->GetIndPrev() )
                {
                    bDoesRowContainMovedFwdFrame = true;
                    break;
                }
            }
        }
    }

    return bDoesRowContainMovedFwdFrame;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
