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

#include <frmsidebarwincontainer.hxx>

#include <map>
#include <fmtfld.hxx>
#include <txtfld.hxx>
#include <SidebarWin.hxx>

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

    struct SidebarWinOrder
    {
        bool operator()( const SidebarWinKey& rSidebarWinKeyA,
                             const SidebarWinKey& rSidebarWinKeyB ) const
        {
            return rSidebarWinKeyA < rSidebarWinKeyB;
        }
    };

    typedef ::std::map < SidebarWinKey, VclPtr<sw::sidebarwindows::SwSidebarWin>, SidebarWinOrder > SidebarWinContainer;

    struct FrmKey
    {
        const SwFrm* mpFrm;

        explicit FrmKey( const SwFrm* pFrm )
            : mpFrm( pFrm )
        {}

        bool operator < ( const FrmKey& rFrmKey ) const
        {
            return mpFrm < rFrmKey.mpFrm;
        }
    };

    struct FrmOrder
    {
        bool operator()( const FrmKey& rFrmKeyA,
                             const FrmKey& rFrmKeyB ) const
        {
            return rFrmKeyA < rFrmKeyB;
        }
    };

    typedef ::std::map < FrmKey, SidebarWinContainer, FrmOrder > _FrmSidebarWinContainer;
}

namespace sw { namespace sidebarwindows {

class FrmSidebarWinContainer : public _FrmSidebarWinContainer
{
};

SwFrmSidebarWinContainer::SwFrmSidebarWinContainer()
    : mpFrmSidebarWinContainer( new FrmSidebarWinContainer() )
{}

SwFrmSidebarWinContainer::~SwFrmSidebarWinContainer()
{
    mpFrmSidebarWinContainer->clear();
    delete mpFrmSidebarWinContainer;
}

bool SwFrmSidebarWinContainer::insert( const SwFrm& rFrm,
                                       const SwFormatField& rFormatField,
                                       SwSidebarWin& rSidebarWin )
{
    bool bInserted( false );

    FrmKey aFrmKey( &rFrm );
    SidebarWinContainer& rSidebarWinContainer = (*mpFrmSidebarWinContainer)[ aFrmKey ];

    SidebarWinKey aSidebarWinKey( rFormatField.GetTextField()->GetStart() );
    if ( rSidebarWinContainer.empty() ||
         rSidebarWinContainer.find( aSidebarWinKey) == rSidebarWinContainer.end() )
    {
        rSidebarWinContainer[ aSidebarWinKey ] = &rSidebarWin;
        bInserted = true;
    }

    return bInserted;
}

bool SwFrmSidebarWinContainer::remove( const SwFrm& rFrm,
                                       const SwSidebarWin& rSidebarWin )
{
    bool bRemoved( false );

    FrmKey aFrmKey( &rFrm );
    FrmSidebarWinContainer::iterator aFrmIter = mpFrmSidebarWinContainer->find( aFrmKey );
    if ( aFrmIter != mpFrmSidebarWinContainer->end() )
    {
        SidebarWinContainer& rSidebarWinContainer = (*aFrmIter).second;
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

bool SwFrmSidebarWinContainer::empty( const SwFrm& rFrm )
{
    bool bEmpty( true );

    FrmKey aFrmKey( &rFrm );
    FrmSidebarWinContainer::iterator aFrmIter = mpFrmSidebarWinContainer->find( aFrmKey );
    if ( aFrmIter != mpFrmSidebarWinContainer->end() )
    {
        bEmpty = (*aFrmIter).second.empty();
    }

    return bEmpty;
}

SwSidebarWin* SwFrmSidebarWinContainer::get( const SwFrm& rFrm,
                                             const sal_Int32 nIndex )
{
    SwSidebarWin* pRet( nullptr );

    FrmKey aFrmKey( &rFrm );
    FrmSidebarWinContainer::iterator aFrmIter = mpFrmSidebarWinContainer->find( aFrmKey );
    if ( aFrmIter != mpFrmSidebarWinContainer->end() )
    {
        SidebarWinContainer& rSidebarWinContainer = (*aFrmIter).second;
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

void SwFrmSidebarWinContainer::getAll( const SwFrm& rFrm,
                                       std::vector< vcl::Window* >* pSidebarWins )
{
    pSidebarWins->clear();

    FrmKey aFrmKey( &rFrm );
    FrmSidebarWinContainer::iterator aFrmIter = mpFrmSidebarWinContainer->find( aFrmKey );
    if ( aFrmIter != mpFrmSidebarWinContainer->end() )
    {
        SidebarWinContainer& rSidebarWinContainer = (*aFrmIter).second;
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
