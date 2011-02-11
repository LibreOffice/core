/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
#include "TSkipDeletedSet.hxx"
#include <osl/diagnose.h>
#include <rtl/logfile.hxx>

using namespace connectivity;
// -----------------------------------------------------------------------------
OSkipDeletedSet::OSkipDeletedSet(IResultSetHelper* _pHelper)
    : m_pHelper(_pHelper)
    ,m_bDeletedVisible(false)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "commontools", "Ocke.Janssen@sun.com", "OSkipDeletedSet::OSkipDeletedSet" );
    m_aBookmarksPositions.reserve(256);
}
// -----------------------------------------------------------------------------
OSkipDeletedSet::~OSkipDeletedSet()
{
    m_aBookmarksPositions.clear();
    //m_aBookmarks.clear();
}
// -----------------------------------------------------------------------------
sal_Bool OSkipDeletedSet::skipDeleted(IResultSetHelper::Movement _eCursorPosition, sal_Int32 _nOffset, sal_Bool _bRetrieveData)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "commontools", "Ocke.Janssen@sun.com", "OSkipDeletedSet::skipDeleted" );
    OSL_ENSURE(_eCursorPosition != IResultSetHelper::BOOKMARK,"OSkipDeletedSet::SkipDeleted can't be called for BOOKMARK");

    IResultSetHelper::Movement eDelPosition = _eCursorPosition;
    sal_Int32 nDelOffset = abs(_nOffset);

    switch (_eCursorPosition)
    {
        case IResultSetHelper::ABSOLUTE:
            return moveAbsolute(_nOffset,_bRetrieveData);
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

    if (_eCursorPosition == IResultSetHelper::LAST)
    {
        RTL_LOGFILE_CONTEXT_TRACE( aLogger, "OSkipDeletedSet::skipDeleted: last" );
        sal_Int32 nBookmark = 0;
        // first position on the last known row
        if ( m_aBookmarksPositions.empty() )
        {
            bDataFound = m_pHelper->move(IResultSetHelper::FIRST, 0, _bRetrieveData);
            if(bDataFound && (m_bDeletedVisible || !m_pHelper->isRowDeleted()))
                //m_aBookmarksPositions.push_back(m_aBookmarks.insert(TInt2IntMap::value_type(m_pHelper->getDriverPos(),m_aBookmarksPositions.size()+1)).first);
                m_aBookmarksPositions.push_back(m_pHelper->getDriverPos());
        }
        else
        {
            // I already have a bookmark so we can positioned on that and look if it is the last one
            nBookmark = (*m_aBookmarksPositions.rbegin())/*->first*/;

            bDataFound = m_pHelper->move(IResultSetHelper::BOOKMARK, nBookmark, _bRetrieveData);
            OSL_ENSURE((m_bDeletedVisible || !m_pHelper->isRowDeleted()),"A bookmark should not be deleted!");
        }


        // and than move forward until we are after the last row
        while(bDataFound)
        {
            bDataFound = m_pHelper->move(IResultSetHelper::NEXT, 1, sal_False); // we don't need the data here
            if( bDataFound && ( m_bDeletedVisible || !m_pHelper->isRowDeleted()) )
            {   // we weren't on the last row we remember it and move on
                m_aBookmarksPositions.push_back(m_pHelper->getDriverPos());
                //m_aBookmarksPositions.push_back(m_aBookmarks.insert(TInt2IntMap::value_type(m_pHelper->getDriverPos(),m_aBookmarksPositions.size()+1)).first);
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
    else if (_eCursorPosition != IResultSetHelper::RELATIVE)
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
            //m_aBookmarksPositions.push_back(m_aBookmarks.insert(TInt2IntMap::value_type(m_pHelper->getDriverPos(),m_aBookmarksPositions.size()+1)).first);
        }
        else
            bDone = sal_False;
    }

    while (bDataFound && !bDone)            // Iterate until we are at the valid set
    {
        bDataFound = m_pHelper->move(eDelPosition, 1, _bRetrieveData);
        if (_eCursorPosition != IResultSetHelper::RELATIVE)
            bDone = bDataFound && (m_bDeletedVisible || !m_pHelper->isRowDeleted());
        else if (bDataFound && (m_bDeletedVisible || !m_pHelper->isRowDeleted()))
        {
            bDone = (--nDelOffset) == 0;
            if ( !bDone )
                m_aBookmarksPositions.push_back(m_pHelper->getDriverPos());
            //m_aBookmarksPositions.push_back(m_aBookmarks.insert(TInt2IntMap::value_type(m_pHelper->getDriverPos(),m_aBookmarksPositions.size()+1)).first);
        }
        else
            bDone = sal_False;
    }

    if(bDataFound && bDone )
    {
        const sal_Int32 nDriverPos = m_pHelper->getDriverPos();
        if ( ::std::find(m_aBookmarksPositions.begin(),m_aBookmarksPositions.end(),nDriverPos) == m_aBookmarksPositions.end() )
            m_aBookmarksPositions.push_back(nDriverPos);
        /*sal_Int32 nDriverPos = m_pHelper->getDriverPos();
        if(m_aBookmarks.find(nDriverPos) == m_aBookmarks.end())
            m_aBookmarksPositions.push_back(m_aBookmarks.insert(TInt2IntMap::value_type(nDriverPos,m_aBookmarksPositions.size()+1)).first);*/
    }

    return bDataFound;
}
// -------------------------------------------------------------------------
sal_Bool OSkipDeletedSet::moveAbsolute(sal_Int32 _nPos,sal_Bool _bRetrieveData)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "commontools", "Ocke.Janssen@sun.com", "OSkipDeletedSet::moveAbsolute" );
    sal_Bool bDataFound = sal_False;
    sal_Int32 nNewPos = _nPos;
    if(nNewPos > 0)
    {
        if((sal_Int32)m_aBookmarksPositions.size() < nNewPos)
        {
            // bookmark isn't known yet
            // start at the last known position
            sal_Int32 nCurPos = 0,nLastBookmark = 1;
            if ( m_aBookmarksPositions.empty() )
            {
                bDataFound = m_pHelper->move(IResultSetHelper::FIRST, 0, _bRetrieveData );
                if(bDataFound && (m_bDeletedVisible || !m_pHelper->isRowDeleted()))
                {
                    ++nCurPos;
                    m_aBookmarksPositions.push_back(m_pHelper->getDriverPos());
                    //m_aBookmarksPositions.push_back(m_aBookmarks.insert(TInt2IntMap::value_type(m_pHelper->getDriverPos(),m_aBookmarksPositions.size()+1)).first);
                    --nNewPos;
                }
            } // if ( m_aBookmarksPositions.empty() )
            else
            {
                nLastBookmark   = (*m_aBookmarksPositions.rbegin())/*->first*/;
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
                    //m_aBookmarksPositions.push_back(m_aBookmarks.insert(TInt2IntMap::value_type(m_pHelper->getDriverPos(),m_aBookmarksPositions.size()+1)).first);
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
// -----------------------------------------------------------------------------
void OSkipDeletedSet::clear()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "commontools", "Ocke.Janssen@sun.com", "OSkipDeletedSet::clear" );
    ::std::vector<sal_Int32>().swap(m_aBookmarksPositions);
    //TInt2IntMap().swap(m_aBookmarks);
}
// -----------------------------------------------------------------------------
sal_Int32 OSkipDeletedSet::getMappedPosition(sal_Int32 _nPos) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "commontools", "Ocke.Janssen@sun.com", "OSkipDeletedSet::getMappedPosition" );
    ::std::vector<sal_Int32>::const_iterator aFind = ::std::find(m_aBookmarksPositions.begin(),m_aBookmarksPositions.end(),_nPos);
    if ( aFind !=  m_aBookmarksPositions.end() )
        return (aFind - m_aBookmarksPositions.begin()) + 1;
    /*TInt2IntMap::const_iterator aFind = m_aBookmarks.find(_nPos);
    OSL_ENSURE(aFind != m_aBookmarks.end(),"OSkipDeletedSet::getMappedPosition() invalid bookmark!");
    return aFind->second;*/
    OSL_ENSURE(0,"Why!");
    return -1;
}
// -----------------------------------------------------------------------------
void OSkipDeletedSet::insertNewPosition(sal_Int32 _nPos)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "commontools", "Ocke.Janssen@sun.com", "OSkipDeletedSet::insertNewPosition" );
    //OSL_ENSURE(m_aBookmarks.find(_nPos) == m_aBookmarks.end(),"OSkipDeletedSet::insertNewPosition: Invalid position");
    //m_aBookmarksPositions.push_back(m_aBookmarks.insert(TInt2IntMap::value_type(_nPos,m_aBookmarksPositions.size()+1)).first);
    //OSL_ENSURE(::std::find(m_aBookmarksPositions.begin(),m_aBookmarksPositions.end(),_nPos) == m_aBookmarksPositions.end(),"Invalid driver pos");
    m_aBookmarksPositions.push_back(_nPos);
}
// -----------------------------------------------------------------------------
void OSkipDeletedSet::deletePosition(sal_Int32 _nBookmark)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "commontools", "Ocke.Janssen@sun.com", "OSkipDeletedSet::deletePosition" );
    ::std::vector<sal_Int32>::iterator aFind = ::std::find(m_aBookmarksPositions.begin(),m_aBookmarksPositions.end(),_nBookmark);
    if ( aFind !=  m_aBookmarksPositions.end() )
    {
    //TInt2IntMap::iterator aFind = m_aBookmarks.find(_nPos);
    //OSL_ENSURE(aFind != m_aBookmarks.end(),"OSkipDeletedSet::deletePosition() bookmark not found!");
    //TInt2IntMap::iterator aIter = aFind;
        m_aBookmarksPositions.erase(aFind);
        //for (; aFind != m_aBookmarksPositions.end() ; ++aIter)
           // --(aFind->second);
    } // if ( aFind !=  m_aBookmarksPositions.end() )
    //m_aBookmarksPositions.erase(m_aBookmarksPositions.begin() + aFind->second-1);
    //m_aBookmarks.erase(_nPos);
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
