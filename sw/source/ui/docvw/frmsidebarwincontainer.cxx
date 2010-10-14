/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
 /*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: $
 * $Revision: $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <frmsidebarwincontainer.hxx>

#include <map>
#include <fmtfld.hxx>
#include <txtfld.hxx>
#include <SidebarWin.hxx>

namespace {
    struct SidebarWinKey
    {
        const xub_StrLen mnIndex;

        explicit SidebarWinKey( const xub_StrLen nIndex )
            : mnIndex( nIndex )
        {}

        bool operator < ( const SidebarWinKey& rSidebarWinKey ) const
        {
            return mnIndex < rSidebarWinKey.mnIndex;
        }
    };

    struct SidebarWinOrder
    {
        sal_Bool operator()( const SidebarWinKey& rSidebarWinKeyA,
                             const SidebarWinKey& rSidebarWinKeyB ) const
        {
            return rSidebarWinKeyA < rSidebarWinKeyB;
        }
    };

    typedef ::std::map < SidebarWinKey, sw::sidebarwindows::SwSidebarWin*, SidebarWinOrder > SidebarWinContainer;

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
        sal_Bool operator()( const FrmKey& rFrmKeyA,
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
                                       const SwFmtFld& rFmtFld,
                                       SwSidebarWin& rSidebarWin )
{
    bool bInserted( false );

    FrmKey aFrmKey( &rFrm );
    SidebarWinContainer& rSidebarWinContainer = (*mpFrmSidebarWinContainer)[ aFrmKey ];

    SidebarWinKey aSidebarWinKey( *(rFmtFld.GetTxtFld()->GetStart()) );
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
    SwSidebarWin* pRet( 0 );

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
                                       std::vector< Window* >* pSidebarWins )
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
