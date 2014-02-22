/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "TSkipDeletedSet.hxx"
#include <osl/diagnose.h>
#include "sal/log.hxx"

using namespace connectivity;

OSkipDeletedSet::OSkipDeletedSet(IResultSetHelper* _pHelper)
    : m_pHelper(_pHelper)
    ,m_bDeletedVisible(false)
{
    SAL_INFO( "connectivity.commontools", "commontools Ocke.Janssen@sun.com OSkipDeletedSet::OSkipDeletedSet" );
    m_aBookmarksPositions.reserve(256);
}

OSkipDeletedSet::~OSkipDeletedSet()
{
    m_aBookmarksPositions.clear();
    
}

sal_Bool OSkipDeletedSet::skipDeleted(IResultSetHelper::Movement _eCursorPosition, sal_Int32 _nOffset, sal_Bool _bRetrieveData)
{
    SAL_INFO( "connectivity.commontools", "commontools Ocke.Janssen@sun.com OSkipDeletedSet::skipDeleted" );
    OSL_ENSURE(_eCursorPosition != IResultSetHelper::BOOKMARK,"OSkipDeletedSet::SkipDeleted can't be called for BOOKMARK");

    IResultSetHelper::Movement eDelPosition = _eCursorPosition;
    sal_Int32 nDelOffset = abs(_nOffset);

    switch (_eCursorPosition)
    {
        case IResultSetHelper::ABSOLUTE:
            return moveAbsolute(_nOffset,_bRetrieveData);
        case IResultSetHelper::FIRST:                   
            eDelPosition = IResultSetHelper::NEXT;
            nDelOffset = 1;
            break;
        case IResultSetHelper::LAST:
            eDelPosition = IResultSetHelper::PRIOR; 
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
        SAL_INFO( "connectivity.commontools", "OSkipDeletedSet::skipDeleted: last" );
        sal_Int32 nBookmark = 0;
        
        if ( m_aBookmarksPositions.empty() )
        {
            bDataFound = m_pHelper->move(IResultSetHelper::FIRST, 0, _bRetrieveData);
            if(bDataFound && (m_bDeletedVisible || !m_pHelper->isRowDeleted()))
                
                m_aBookmarksPositions.push_back(m_pHelper->getDriverPos());
        }
        else
        {
            
            nBookmark = (*m_aBookmarksPositions.rbegin())/*->first*/;

            bDataFound = m_pHelper->move(IResultSetHelper::BOOKMARK, nBookmark, _bRetrieveData);
            OSL_ENSURE((m_bDeletedVisible || !m_pHelper->isRowDeleted()),"A bookmark should not be deleted!");
        }


        
        while(bDataFound)
        {
            bDataFound = m_pHelper->move(IResultSetHelper::NEXT, 1, sal_False); 
            if( bDataFound && ( m_bDeletedVisible || !m_pHelper->isRowDeleted()) )
            {   
                m_aBookmarksPositions.push_back(m_pHelper->getDriverPos());
                
            }
            else if(!bDataFound && !m_aBookmarksPositions.empty() )
            {
                
                
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
            
        }
        else
            bDone = sal_False;
    }

    while (bDataFound && !bDone)            
    {
        bDataFound = m_pHelper->move(eDelPosition, 1, _bRetrieveData);
        if (_eCursorPosition != IResultSetHelper::RELATIVE)
            bDone = bDataFound && (m_bDeletedVisible || !m_pHelper->isRowDeleted());
        else if (bDataFound && (m_bDeletedVisible || !m_pHelper->isRowDeleted()))
        {
            bDone = (--nDelOffset) == 0;
            if ( !bDone )
                m_aBookmarksPositions.push_back(m_pHelper->getDriverPos());
            
        }
        else
            bDone = sal_False;
    }

    if(bDataFound && bDone)
    {
        const sal_Int32 nDriverPos = m_pHelper->getDriverPos();
        if ( m_bDeletedVisible )
        {
            if ( nDriverPos > (sal_Int32)m_aBookmarksPositions.size() )
                m_aBookmarksPositions.push_back(nDriverPos);
        }
        else if ( ::std::find(m_aBookmarksPositions.begin(),m_aBookmarksPositions.end(),nDriverPos) == m_aBookmarksPositions.end() )
            m_aBookmarksPositions.push_back(nDriverPos);
        /*sal_Int32 nDriverPos = m_pHelper->getDriverPos();
        if(m_aBookmarks.find(nDriverPos) == m_aBookmarks.end())
            m_aBookmarksPositions.push_back(m_aBookmarks.insert(TInt2IntMap::value_type(nDriverPos,m_aBookmarksPositions.size()+1)).first);*/
    }

    return bDataFound;
}

sal_Bool OSkipDeletedSet::moveAbsolute(sal_Int32 _nPos,sal_Bool _bRetrieveData)
{
    SAL_INFO( "connectivity.commontools", "commontools Ocke.Janssen@sun.com OSkipDeletedSet::moveAbsolute" );
    sal_Bool bDataFound = sal_False;
    sal_Int32 nNewPos = _nPos;
    if(nNewPos > 0)
    {
        if((sal_Int32)m_aBookmarksPositions.size() < nNewPos)
        {
            
            
            sal_Int32 nCurPos = 0,nLastBookmark = 1;
            if ( m_aBookmarksPositions.empty() )
            {
                bDataFound = m_pHelper->move(IResultSetHelper::FIRST, 0, _bRetrieveData );
                if(bDataFound && (m_bDeletedVisible || !m_pHelper->isRowDeleted()))
                {
                    ++nCurPos;
                    m_aBookmarksPositions.push_back(m_pHelper->getDriverPos());
                    
                    --nNewPos;
                }
            } 
            else
            {
                nLastBookmark   = (*m_aBookmarksPositions.rbegin())/*->first*/;
                nCurPos         = /*(**/m_aBookmarksPositions.size()/*->second*/;
                nNewPos         = nNewPos - nCurPos;
                bDataFound      = m_pHelper->move(IResultSetHelper::BOOKMARK, nLastBookmark, _bRetrieveData);
            }

            
            while (bDataFound && nNewPos)
            {
                bDataFound = m_pHelper->move(IResultSetHelper::NEXT, 1, _bRetrieveData);
                if(bDataFound && (m_bDeletedVisible || !m_pHelper->isRowDeleted()))
                {
                    ++nCurPos;
                    m_aBookmarksPositions.push_back(m_pHelper->getDriverPos());
                    
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
    SAL_INFO( "connectivity.commontools", "commontools Ocke.Janssen@sun.com OSkipDeletedSet::clear" );
    ::std::vector<sal_Int32>().swap(m_aBookmarksPositions);
    
}

sal_Int32 OSkipDeletedSet::getMappedPosition(sal_Int32 _nPos) const
{
    SAL_INFO( "connectivity.commontools", "commontools Ocke.Janssen@sun.com OSkipDeletedSet::getMappedPosition" );
    ::std::vector<sal_Int32>::const_iterator aFind = ::std::find(m_aBookmarksPositions.begin(),m_aBookmarksPositions.end(),_nPos);
    if ( aFind !=  m_aBookmarksPositions.end() )
        return (aFind - m_aBookmarksPositions.begin()) + 1;
    /*TInt2IntMap::const_iterator aFind = m_aBookmarks.find(_nPos);
    OSL_ENSURE(aFind != m_aBookmarks.end(),"OSkipDeletedSet::getMappedPosition() invalid bookmark!");
    return aFind->second;*/
    OSL_FAIL("Why!");
    return -1;
}

void OSkipDeletedSet::insertNewPosition(sal_Int32 _nPos)
{
    SAL_INFO( "connectivity.commontools", "commontools Ocke.Janssen@sun.com OSkipDeletedSet::insertNewPosition" );
    
    
    
    m_aBookmarksPositions.push_back(_nPos);
}

void OSkipDeletedSet::deletePosition(sal_Int32 _nBookmark)
{
    SAL_INFO( "connectivity.commontools", "commontools Ocke.Janssen@sun.com OSkipDeletedSet::deletePosition" );
    ::std::vector<sal_Int32>::iterator aFind = ::std::find(m_aBookmarksPositions.begin(),m_aBookmarksPositions.end(),_nBookmark);
    if ( aFind !=  m_aBookmarksPositions.end() )
    {
    
    
    
        m_aBookmarksPositions.erase(aFind);
        
           
    } 
    
    
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
