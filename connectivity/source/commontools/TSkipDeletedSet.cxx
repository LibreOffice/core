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

#include <TSkipDeletedSet.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <algorithm>

using namespace connectivity;

OSkipDeletedSet::OSkipDeletedSet(IResultSetHelper* _pHelper)
    : m_pHelper(_pHelper)
    ,m_bDeletedVisible(false)
{
    m_aBookmarksPositions.reserve(256);
}

OSkipDeletedSet::~OSkipDeletedSet()
{
    m_aBookmarksPositions.clear();
    //m_aBookmarks.clear();
}

bool OSkipDeletedSet::skipDeleted(IResultSetHelper::Movement _eCursorPosition, sal_Int32 _nOffset, bool _bRetrieveData)
{
    OSL_ENSURE(_eCursorPosition != IResultSetHelper::BOOKMARK,"OSkipDeletedSet::SkipDeleted can't be called for BOOKMARK");

    IResultSetHelper::Movement eDelPosition = _eCursorPosition;
    sal_Int32 nDelOffset = abs(_nOffset);

    switch (_eCursorPosition)
    {
        case IResultSetHelper::ABSOLUTE1:
            return moveAbsolute(_nOffset,_bRetrieveData);
        case IResultSetHelper::FIRST:                   // set the movement when positioning failed
            eDelPosition = IResultSetHelper::NEXT;
            nDelOffset = 1;
            break;
        case IResultSetHelper::LAST:
            eDelPosition = IResultSetHelper::PRIOR; // last row is invalid so position before
            nDelOffset = 1;
            break;
        case IResultSetHelper::RELATIVE1:
            eDelPosition = (_nOffset >= 0) ? IResultSetHelper::NEXT : IResultSetHelper::PRIOR;
            break;
        default:
            break;
    }

    bool bDone          = true;
    bool bDataFound     = false;

    if (_eCursorPosition == IResultSetHelper::LAST)
    {
        SAL_INFO( "connectivity.commontools", "OSkipDeletedSet::skipDeleted: last" );
        sal_Int32 nBookmark = 0;
        // first position on the last known row
        if ( m_aBookmarksPositions.empty() )
        {
            bDataFound = m_pHelper->move(IResultSetHelper::FIRST, 0, _bRetrieveData);
            if(bDataFound && (m_bDeletedVisible || !m_pHelper->isRowDeleted()))
                //m_aBookmarksPositions.push_back(m_aBookmarks.emplace( m_pHelper->getDriverPos(),m_aBookmarksPositions.size()+1)).first);
                m_aBookmarksPositions.push_back(m_pHelper->getDriverPos());
        }
        else
        {
            // I already have a bookmark so we can positioned on that and look if it is the last one
            nBookmark = (*m_aBookmarksPositions.rbegin())/*->first*/;

            bDataFound = m_pHelper->move(IResultSetHelper::BOOKMARK, nBookmark, _bRetrieveData);
            OSL_ENSURE((m_bDeletedVisible || !m_pHelper->isRowDeleted()),"A bookmark should not be deleted!");
        }


        // and then move forward until we are after the last row
        while(bDataFound)
        {
            bDataFound = m_pHelper->move(IResultSetHelper::NEXT, 1, false); // we don't need the data here
            if( bDataFound && ( m_bDeletedVisible || !m_pHelper->isRowDeleted()) )
            {   // we weren't on the last row we remember it and move on
                m_aBookmarksPositions.push_back(m_pHelper->getDriverPos());
                //m_aBookmarksPositions.push_back(m_aBookmarks.emplace( m_pHelper->getDriverPos(),m_aBookmarksPositions.size()+1)).first);
            }
            else if(!bDataFound && !m_aBookmarksPositions.empty() )
            {
                // i already know the last bookmark :-)
                // now we only have to repositioning us to the last row
                nBookmark = (*m_aBookmarksPositions.rbegin())/*->first*/;
                bDataFound = m_pHelper->move(IResultSetHelper::BOOKMARK, nBookmark, _bRetrieveData);
                break;
            }
        }
        return bDataFound;
    }
    else if (_eCursorPosition != IResultSetHelper::RELATIVE1)
    {
        bDataFound = m_pHelper->move(_eCursorPosition, _nOffset, _bRetrieveData);
        bDone = bDataFound && (m_bDeletedVisible || !m_pHelper->isRowDeleted());
    }
    else
    {
        bDataFound = m_pHelper->move(eDelPosition, 1, _bRetrieveData);
        if (bDataFound && (m_bDeletedVisible || !m_pHelper->isRowDeleted()))
        {
            bDone = (--nDelOffset) == 0;
            if ( !bDone )
                m_aBookmarksPositions.push_back(m_pHelper->getDriverPos());
            //m_aBookmarksPositions.push_back(m_aBookmarks.emplace( m_pHelper->getDriverPos(),m_aBookmarksPositions.size()+1)).first);
        }
        else
            bDone = false;
    }

    while (bDataFound && !bDone)            // Iterate until we are at the valid set
    {
        bDataFound = m_pHelper->move(eDelPosition, 1, _bRetrieveData);
        if (_eCursorPosition != IResultSetHelper::RELATIVE1)
            bDone = bDataFound && (m_bDeletedVisible || !m_pHelper->isRowDeleted());
        else if (bDataFound && (m_bDeletedVisible || !m_pHelper->isRowDeleted()))
        {
            bDone = (--nDelOffset) == 0;
            if ( !bDone )
                m_aBookmarksPositions.push_back(m_pHelper->getDriverPos());
            //m_aBookmarksPositions.push_back(m_aBookmarks.emplace( m_pHelper->getDriverPos(),m_aBookmarksPositions.size()+1)).first);
        }
        else
            bDone = false;
    }

    if(bDataFound && bDone)
    {
        const sal_Int32 nDriverPos = m_pHelper->getDriverPos();
        if ( m_bDeletedVisible )
        {
            if ( nDriverPos > static_cast<sal_Int32>(m_aBookmarksPositions.size()) )
                m_aBookmarksPositions.push_back(nDriverPos);
        }
        else if ( std::find(m_aBookmarksPositions.begin(),m_aBookmarksPositions.end(),nDriverPos) == m_aBookmarksPositions.end() )
            m_aBookmarksPositions.push_back(nDriverPos);
        /*sal_Int32 nDriverPos = m_pHelper->getDriverPos();
        if(m_aBookmarks.find(nDriverPos) == m_aBookmarks.end())
            m_aBookmarksPositions.push_back(m_aBookmarks.emplace( nDriverPos,m_aBookmarksPositions.size()+1)).first);*/
    }

    return bDataFound;
}

bool OSkipDeletedSet::moveAbsolute(sal_Int32 _nPos,bool _bRetrieveData)
{
    bool bDataFound = false;
    sal_Int32 nNewPos = _nPos;
    if(nNewPos > 0)
    {
        if(static_cast<sal_Int32>(m_aBookmarksPositions.size()) < nNewPos)
        {
            // bookmark isn't known yet
            // start at the last known position
            sal_Int32 nCurPos = 0;
            if ( m_aBookmarksPositions.empty() )
            {
                bDataFound = m_pHelper->move(IResultSetHelper::FIRST, 0, _bRetrieveData );
                if(bDataFound && (m_bDeletedVisible || !m_pHelper->isRowDeleted()))
                {
                    ++nCurPos;
                    m_aBookmarksPositions.push_back(m_pHelper->getDriverPos());
                    //m_aBookmarksPositions.push_back(m_aBookmarks.emplace( m_pHelper->getDriverPos(),m_aBookmarksPositions.size()+1)).first);
                    --nNewPos;
                }
            } // if ( m_aBookmarksPositions.empty() )
            else
            {
                sal_Int32 nLastBookmark = *m_aBookmarksPositions.rbegin()/*->first*/;
                nCurPos         = /*(**/m_aBookmarksPositions.size()/*->second*/;
                nNewPos         = nNewPos - nCurPos;
                bDataFound      = m_pHelper->move(IResultSetHelper::BOOKMARK, nLastBookmark, _bRetrieveData);
            }

            // now move to that row we need and don't count deleted rows
            while (bDataFound && nNewPos)
            {
                bDataFound = m_pHelper->move(IResultSetHelper::NEXT, 1, _bRetrieveData);
                if(bDataFound && (m_bDeletedVisible || !m_pHelper->isRowDeleted()))
                {
                    ++nCurPos;
                    m_aBookmarksPositions.push_back(m_pHelper->getDriverPos());
                    //m_aBookmarksPositions.push_back(m_aBookmarks.emplace( m_pHelper->getDriverPos(),m_aBookmarksPositions.size()+1)).first);
                    --nNewPos;
                }
            }
        }
        else
        {
            const sal_Int32 nBookmark = m_aBookmarksPositions[nNewPos-1]/*->first*/;
            bDataFound = m_pHelper->move(IResultSetHelper::BOOKMARK,nBookmark, _bRetrieveData);
            OSL_ENSURE((m_bDeletedVisible || !m_pHelper->isRowDeleted()),"moveAbsolute: row can't be deleted!");
        }
    }
    else
    {
        ++nNewPos;
        bDataFound = skipDeleted(IResultSetHelper::LAST,0,nNewPos == 0);

        for(sal_Int32 i=nNewPos+1;bDataFound && i <= 0;++i)
            bDataFound = skipDeleted(IResultSetHelper::PRIOR,1,i == 0);

    }
    return bDataFound;
}

void OSkipDeletedSet::clear()
{
    std::vector<sal_Int32>().swap(m_aBookmarksPositions);
}

sal_Int32 OSkipDeletedSet::getMappedPosition(sal_Int32 _nPos) const
{
    std::vector<sal_Int32>::const_iterator aFind = std::find(m_aBookmarksPositions.begin(),m_aBookmarksPositions.end(),_nPos);
    if ( aFind !=  m_aBookmarksPositions.end() )
        return (aFind - m_aBookmarksPositions.begin()) + 1;
    OSL_FAIL("Why!");
    return -1;
}

void OSkipDeletedSet::insertNewPosition(sal_Int32 _nPos)
{
    m_aBookmarksPositions.push_back(_nPos);
}

void OSkipDeletedSet::deletePosition(sal_Int32 _nBookmark)
{
    std::vector<sal_Int32>::iterator aFind = std::find(m_aBookmarksPositions.begin(),m_aBookmarksPositions.end(),_nBookmark);
    if ( aFind !=  m_aBookmarksPositions.end() )
    {
        m_aBookmarksPositions.erase(aFind);
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
