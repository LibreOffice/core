/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
