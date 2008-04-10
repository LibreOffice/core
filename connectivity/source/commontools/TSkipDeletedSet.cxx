/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TSkipDeletedSet.cxx,v $
 * $Revision: 1.10 $
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
#include "precompiled_connectivity.hxx"
#include "TSkipDeletedSet.hxx"
#include <osl/diagnose.h>

using namespace connectivity;
// -----------------------------------------------------------------------------
OSkipDeletedSet::OSkipDeletedSet(IResultSetHelper* _pHelper)
    : m_pHelper(_pHelper)
{
}
// -----------------------------------------------------------------------------
OSkipDeletedSet::~OSkipDeletedSet()
{
    m_aBookmarksPositions.clear();
    m_aBookmarks.clear();
}
// -----------------------------------------------------------------------------
sal_Bool OSkipDeletedSet::skipDeleted(IResultSetHelper::Movement _eCursorPosition, sal_Int32 _nOffset, sal_Bool _bRetrieveData)
{
    OSL_ENSURE(_eCursorPosition != IResultSetHelper::BOOKMARK,"OSkipDeletedSet::SkipDeleted can't be called for BOOKMARK");

    IResultSetHelper::Movement eDelPosition = _eCursorPosition;
    sal_Int32 nDelOffset = abs(_nOffset);

    switch (_eCursorPosition)
    {
        case IResultSetHelper::ABSOLUTE:
        case IResultSetHelper::FIRST:                   // set the movement when positioning failed
            eDelPosition = IResultSetHelper::NEXT;
            nDelOffset = 1;
            break;
        case IResultSetHelper::LAST:
            eDelPosition = IResultSetHelper::PRIOR; // lsat row is invalid so position before
            nDelOffset = 1;
            break;
        case IResultSetHelper::RELATIVE:
            eDelPosition = (_nOffset >= 0) ? IResultSetHelper::NEXT : IResultSetHelper::PRIOR;
            break;
        default:
            break;
    }

    sal_Bool bDone          = sal_True;
    sal_Bool bDataFound     = sal_False;

    if (_eCursorPosition == IResultSetHelper::ABSOLUTE)
    {
        return moveAbsolute(_nOffset,_bRetrieveData);
    }
    else if (_eCursorPosition == IResultSetHelper::LAST)
    {
        sal_Int32 nBookmark = 0;
        sal_Int32 nCurPos = 1;
        // first position on the last known row
        if(m_aBookmarks.empty())
        {
            bDataFound = m_pHelper->move(IResultSetHelper::FIRST, 0, _bRetrieveData);
            if(bDataFound && (m_pHelper->deletedVisible() || !m_pHelper->isRowDeleted()))
                m_aBookmarksPositions.push_back(m_aBookmarks.insert(TInt2IntMap::value_type(m_pHelper->getDriverPos(),m_aBookmarksPositions.size()+1)).first);
        }
        else
        {
            // I already have a bookmark so we can positioned on that and look if it is the last one
            nBookmark = (*m_aBookmarksPositions.rbegin())->first;

            bDataFound = m_pHelper->move(IResultSetHelper::BOOKMARK, nBookmark, _bRetrieveData);
            OSL_ENSURE((m_pHelper->deletedVisible() || !m_pHelper->isRowDeleted()),"A bookmark should not be deleted!");
            nCurPos    = (*m_aBookmarksPositions.rbegin())->second;
        }


        // and than move forward until we are after the last row
        while(bDataFound)
        {
            bDataFound = m_pHelper->move(IResultSetHelper::NEXT, 1, sal_False); // we don't need the data here
            if(bDataFound && (m_pHelper->deletedVisible() || !m_pHelper->isRowDeleted()))
            {   // we weren't on the last row we remember it and move on
                ++nCurPos;
                m_aBookmarksPositions.push_back(m_aBookmarks.insert(TInt2IntMap::value_type(m_pHelper->getDriverPos(),m_aBookmarksPositions.size()+1)).first);
            }
            else if(!bDataFound && m_aBookmarks.size())
            {
                // i already know the last bookmark :-)
                // now we only have to repositioning us to the last row
                nBookmark = (*m_aBookmarksPositions.rbegin())->first;
                bDataFound = m_pHelper->move(IResultSetHelper::BOOKMARK, nBookmark, _bRetrieveData);
                break;
            }
        }
        return bDataFound;
    }
    else if (_eCursorPosition != IResultSetHelper::RELATIVE)
    {
        bDataFound = m_pHelper->move(_eCursorPosition, _nOffset, _bRetrieveData);
        bDone = bDataFound && (m_pHelper->deletedVisible() || !m_pHelper->isRowDeleted());
    }
    else
    {
        bDataFound = m_pHelper->move(eDelPosition, 1, _bRetrieveData);
        if (bDataFound && (m_pHelper->deletedVisible() || !m_pHelper->isRowDeleted()))
        {
            m_aBookmarksPositions.push_back(m_aBookmarks.insert(TInt2IntMap::value_type(m_pHelper->getDriverPos(),m_aBookmarksPositions.size()+1)).first);
            bDone = (--nDelOffset) == 0;
        }
        else
            bDone = sal_False;
    }

    while (bDataFound && !bDone)            // solange iterieren bis man auf einem gueltigen Satz ist
    {
        bDataFound = m_pHelper->move(eDelPosition, 1, _bRetrieveData);
        if (_eCursorPosition != IResultSetHelper::RELATIVE)
            bDone = bDataFound && (m_pHelper->deletedVisible() || !m_pHelper->isRowDeleted());
        else if (bDataFound && (m_pHelper->deletedVisible() || !m_pHelper->isRowDeleted()))
        {
            m_aBookmarksPositions.push_back(m_aBookmarks.insert(TInt2IntMap::value_type(m_pHelper->getDriverPos(),m_aBookmarksPositions.size()+1)).first);
            bDone = (--nDelOffset) == 0;
        }
        else
            bDone = sal_False;
    }


    if(bDataFound && bDone)
    {
        sal_Int32 nDriverPos = m_pHelper->getDriverPos();
        if(m_aBookmarks.find(nDriverPos) == m_aBookmarks.end())
            m_aBookmarksPositions.push_back(m_aBookmarks.insert(TInt2IntMap::value_type(nDriverPos,m_aBookmarksPositions.size()+1)).first);
    }

    return bDataFound;
}
// -------------------------------------------------------------------------
sal_Bool OSkipDeletedSet::moveAbsolute(sal_Int32 _nOffset,sal_Bool _bRetrieveData)
{
    sal_Bool bDataFound = sal_False;
    sal_Int32 nNewOffset = _nOffset;
    if(nNewOffset > 0)
    {
        if((sal_Int32)m_aBookmarks.size() < nNewOffset)
        {
            // bookmark isn't known yet
            // start at the last position
            sal_Int32 nCurPos = 0,nLastBookmark = 1;
            if(!m_aBookmarks.empty())
            {
                nLastBookmark   = (*m_aBookmarksPositions.rbegin())->first;
                nCurPos         = (*m_aBookmarksPositions.rbegin())->second;
                nNewOffset      = nNewOffset - nCurPos;
                bDataFound      = m_pHelper->move(IResultSetHelper::BOOKMARK, nLastBookmark, _bRetrieveData);
            }
            else
            {
                bDataFound = m_pHelper->move(IResultSetHelper::FIRST, 0, _bRetrieveData );
                if(bDataFound && (m_pHelper->deletedVisible() || !m_pHelper->isRowDeleted()))
                {
                    ++nCurPos;
                    m_aBookmarksPositions.push_back(m_aBookmarks.insert(TInt2IntMap::value_type(m_pHelper->getDriverPos(),m_aBookmarksPositions.size()+1)).first);
                    --nNewOffset;
                }
            }
            // now move to that row we need and don't count deleted rows
            while (bDataFound && nNewOffset)
            {
                bDataFound = m_pHelper->move(IResultSetHelper::NEXT, 1, _bRetrieveData);
                if(bDataFound && (m_pHelper->deletedVisible() || !m_pHelper->isRowDeleted()))
                {
                    ++nCurPos;
                    m_aBookmarksPositions.push_back(m_aBookmarks.insert(TInt2IntMap::value_type(m_pHelper->getDriverPos(),m_aBookmarksPositions.size()+1)).first);
                    --nNewOffset;
                }
            }
        }
        else
        {
            sal_Int32 nBookmark = m_aBookmarksPositions[nNewOffset-1]->first;
            bDataFound = m_pHelper->move(IResultSetHelper::BOOKMARK,nBookmark, _bRetrieveData);
            OSL_ENSURE((m_pHelper->deletedVisible() || !m_pHelper->isRowDeleted()),"moveAbsolute: row can't be deleted!");
        }
    }
    else
    {
        ++nNewOffset;
        bDataFound = skipDeleted(IResultSetHelper::LAST,0,nNewOffset == 0);

        for(sal_Int32 i=nNewOffset+1;bDataFound && i <= 0;++i)
            bDataFound = skipDeleted(IResultSetHelper::PRIOR,1,i == 0);

    }
    return bDataFound;
}
// -----------------------------------------------------------------------------
void OSkipDeletedSet::clear()
{
    ::std::vector<TInt2IntMap::iterator>().swap(m_aBookmarksPositions);
    TInt2IntMap().swap(m_aBookmarks);
}
// -----------------------------------------------------------------------------
sal_Int32 OSkipDeletedSet::getMappedPosition(sal_Int32 _nPos) const
{
    TInt2IntMap::const_iterator aFind = m_aBookmarks.find(_nPos);
    OSL_ENSURE(aFind != m_aBookmarks.end(),"OSkipDeletedSet::getMappedPosition() invalid bookmark!");
    return aFind->second;
}
// -----------------------------------------------------------------------------
void OSkipDeletedSet::insertNewPosition(sal_Int32 _nPos)
{
    OSL_ENSURE(m_aBookmarks.find(_nPos) == m_aBookmarks.end(),"OSkipDeletedSet::insertNewPosition: Invalid position");
    m_aBookmarksPositions.push_back(m_aBookmarks.insert(TInt2IntMap::value_type(_nPos,m_aBookmarksPositions.size()+1)).first);
}
// -----------------------------------------------------------------------------
void OSkipDeletedSet::deletePosition(sal_Int32 _nPos)
{
    TInt2IntMap::iterator aFind = m_aBookmarks.find(_nPos);
    OSL_ENSURE(aFind != m_aBookmarks.end(),"OSkipDeletedSet::deletePosition() bookmark not found!");
    TInt2IntMap::iterator aIter = aFind;
    ++aIter;
    for (; aIter != m_aBookmarks.end() ; ++aIter)
        --(aIter->second);
    m_aBookmarksPositions.erase(m_aBookmarksPositions.begin() + aFind->second-1);
    m_aBookmarks.erase(_nPos);
}
// -----------------------------------------------------------------------------
