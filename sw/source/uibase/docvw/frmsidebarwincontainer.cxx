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

#include "frmsidebarwincontainer.hxx"

#include <map>
#include <fmtfld.hxx>
#include <txtfld.hxx>
#include <AnnotationWin.hxx>

namespace {
    struct SidebarWinKey
    {
        const sal_Int32 mnIndex;

        explicit SidebarWinKey( const sal_Int32 nIndex )
            : mnIndex( nIndex )
        {}

        bool operator < ( const SidebarWinKey& rSidebarWinKey ) const
        {
            return mnIndex < rSidebarWinKey.mnIndex;
        }
    };

    typedef std::map < SidebarWinKey, VclPtr<sw::annotation::SwAnnotationWin> > SidebarWinContainer;

    struct FrameKey
    {
        const SwFrame* mpFrame;

        explicit FrameKey( const SwFrame* pFrame )
            : mpFrame( pFrame )
        {}

        bool operator < ( const FrameKey& rFrameKey ) const
        {
            return mpFrame < rFrameKey.mpFrame;
        }
    };

    typedef std::map < FrameKey, SidebarWinContainer > FrameSidebarWinContainer_;
}

namespace sw { namespace sidebarwindows {

class FrameSidebarWinContainer : public FrameSidebarWinContainer_
{
};

SwFrameSidebarWinContainer::SwFrameSidebarWinContainer()
    : mpFrameSidebarWinContainer( new FrameSidebarWinContainer )
{}

SwFrameSidebarWinContainer::~SwFrameSidebarWinContainer()
{
    mpFrameSidebarWinContainer->clear();
    mpFrameSidebarWinContainer.reset();
}

bool SwFrameSidebarWinContainer::insert( const SwFrame& rFrame,
                                       const SwFormatField& rFormatField,
                                       sw::annotation::SwAnnotationWin& rSidebarWin )
{
    bool bInserted( false );

    FrameKey aFrameKey( &rFrame );
    SidebarWinContainer& rSidebarWinContainer = (*mpFrameSidebarWinContainer)[ aFrameKey ];

    SidebarWinKey aSidebarWinKey( rFormatField.GetTextField()->GetStart() );
    if ( rSidebarWinContainer.empty() ||
         rSidebarWinContainer.find( aSidebarWinKey) == rSidebarWinContainer.end() )
    {
        rSidebarWinContainer[ aSidebarWinKey ] = &rSidebarWin;
        bInserted = true;
    }

    return bInserted;
}

bool SwFrameSidebarWinContainer::remove( const SwFrame& rFrame,
                                         const sw::annotation::SwAnnotationWin & rSidebarWin )
{
    bool bRemoved( false );

    FrameKey aFrameKey( &rFrame );
    FrameSidebarWinContainer::iterator aFrameIter = mpFrameSidebarWinContainer->find( aFrameKey );
    if ( aFrameIter != mpFrameSidebarWinContainer->end() )
    {
        SidebarWinContainer& rSidebarWinContainer = (*aFrameIter).second;
        for ( SidebarWinContainer::iterator aIter = rSidebarWinContainer.begin();
              aIter != rSidebarWinContainer.end();
              ++aIter )
        {
            if ( (*aIter).second == &rSidebarWin )
            {
                rSidebarWinContainer.erase( aIter );
                bRemoved = true;
                break;
            }
        }
    }

    return bRemoved;
}

bool SwFrameSidebarWinContainer::empty( const SwFrame& rFrame )
{
    bool bEmpty( true );

    FrameKey aFrameKey( &rFrame );
    FrameSidebarWinContainer::iterator aFrameIter = mpFrameSidebarWinContainer->find( aFrameKey );
    if ( aFrameIter != mpFrameSidebarWinContainer->end() )
    {
        bEmpty = (*aFrameIter).second.empty();
    }

    return bEmpty;
}

sw::annotation::SwAnnotationWin* SwFrameSidebarWinContainer::get( const SwFrame& rFrame,
                                             const sal_Int32 nIndex )
{
    sw::annotation::SwAnnotationWin* pRet( nullptr );

    FrameKey aFrameKey( &rFrame );
    FrameSidebarWinContainer::iterator aFrameIter = mpFrameSidebarWinContainer->find( aFrameKey );
    if ( aFrameIter != mpFrameSidebarWinContainer->end() )
    {
        SidebarWinContainer& rSidebarWinContainer = (*aFrameIter).second;
        sal_Int32 nCounter( nIndex );
        for ( SidebarWinContainer::iterator aIter = rSidebarWinContainer.begin();
              nCounter >= 0 && aIter != rSidebarWinContainer.end();
              ++aIter )
        {
            if ( nCounter == 0 )
            {
                pRet = (*aIter).second;
                break;
            }

            --nCounter;
        }
    }

    return pRet;
}

void SwFrameSidebarWinContainer::getAll( const SwFrame& rFrame,
                                       std::vector< vcl::Window* >* pSidebarWins )
{
    pSidebarWins->clear();

    FrameKey aFrameKey( &rFrame );
    FrameSidebarWinContainer::iterator aFrameIter = mpFrameSidebarWinContainer->find( aFrameKey );
    if ( aFrameIter != mpFrameSidebarWinContainer->end() )
    {
        SidebarWinContainer& rSidebarWinContainer = (*aFrameIter).second;
        for ( SidebarWinContainer::iterator aIter = rSidebarWinContainer.begin();
              aIter != rSidebarWinContainer.end();
              ++aIter )
        {
            pSidebarWins->push_back( (*aIter).second );
        }
    }
}

} } // eof of namespace sw::sidebarwindows::

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
