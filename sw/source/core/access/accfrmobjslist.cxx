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

#include "accfrmobjslist.hxx"
#include <accmap.hxx>
#include "acccontext.hxx"

#include <pagefrm.hxx>
#include <sortedobjs.hxx>
#include <anchoredobject.hxx>

using namespace ::sw::access;

SwAccessibleChildSList_const_iterator::SwAccessibleChildSList_const_iterator(
    const SwAccessibleChildSList& rLst,
    SwAccessibleMap& rAccMap )
    : m_rList( rLst ),
      m_aCurr( m_rList.GetFrame().GetLower() ),
      m_nNextObj( 0 )
{
    if( !m_aCurr.GetSwFrame() )
    {
        const SwFrame& rFrame = m_rList.GetFrame();
        if( rFrame.IsPageFrame() )
        {
            const SwPageFrame& rPgFrame = static_cast< const SwPageFrame& >( rFrame );
            const SwSortedObjs *pObjs = rPgFrame.GetSortedObjs();
            if( pObjs && pObjs->size() )
            {
                m_aCurr = (*pObjs)[m_nNextObj++]->GetDrawObj();
            }
        }
        else if( rFrame.IsTextFrame() )
        {
            const SwSortedObjs *pObjs = rFrame.GetDrawObjs();
            if ( pObjs && pObjs->size() )
            {
                m_aCurr = (*pObjs)[m_nNextObj++]->GetDrawObj();
                while( m_aCurr.IsValid() && !m_aCurr.IsBoundAsChar() )
                {
                    m_aCurr = (m_nNextObj < pObjs->size())
                            ? (*pObjs)[m_nNextObj++]->GetDrawObj()
                            : static_cast< const SdrObject *>( nullptr );
                }
            }
            if ( !m_aCurr.IsValid() )
            {
                ::rtl::Reference < SwAccessibleContext > xAccImpl =
                                    rAccMap.GetContextImpl( &rFrame, false );
                if( xAccImpl.is() )
                {
                    SwAccessibleContext* pAccImpl = xAccImpl.get();
                    m_aCurr = SwAccessibleChild( pAccImpl->GetAdditionalAccessibleChild( 0 ) );
                    ++m_nNextObj;
                }
            }
        }
    }

    if( m_rList.IsVisibleChildrenOnly() )
    {
        // Find the first visible
        while( m_aCurr.IsValid() &&
               !m_aCurr.AlwaysIncludeAsChild() &&
               !m_aCurr.GetBox( rAccMap ).Overlaps( m_rList.GetVisArea() ) )
        {
            next();
        }
    }
}

SwAccessibleChildSList_const_iterator& SwAccessibleChildSList_const_iterator::next()
{
    bool bNextTaken( true );
    if( m_aCurr.GetDrawObject() || m_aCurr.GetWindow() )
    {
        bNextTaken = false;
    }
    else if( m_aCurr.GetSwFrame() )
    {
        m_aCurr = m_aCurr.GetSwFrame()->GetNext();
        if( !m_aCurr.GetSwFrame() )
        {
            bNextTaken = false;
        }
    }

    if( !bNextTaken )
    {
        const SwFrame& rFrame = m_rList.GetFrame();
        if( rFrame.IsPageFrame() )
        {
            const SwPageFrame& rPgFrame = static_cast< const SwPageFrame& >( rFrame );
            const SwSortedObjs *pObjs = rPgFrame.GetSortedObjs();
            m_aCurr = ( pObjs && m_nNextObj < pObjs->size() )
                    ? (*pObjs)[m_nNextObj++]->GetDrawObj()
                    : static_cast< const SdrObject *>( nullptr );
        }
        else if( rFrame.IsTextFrame() )
        {
            const SwSortedObjs* pObjs = rFrame.GetDrawObjs();
            const size_t nObjsCount = pObjs ? pObjs->size() : 0;
            m_aCurr = ( pObjs && m_nNextObj < nObjsCount )
                    ? (*pObjs)[m_nNextObj++]->GetDrawObj()
                    : static_cast< const SdrObject *>( nullptr );
            while( m_aCurr.IsValid() && !m_aCurr.IsBoundAsChar() )
            {
                m_aCurr = ( m_nNextObj < nObjsCount )
                        ? (*pObjs)[m_nNextObj++]->GetDrawObj()
                        : static_cast< const SdrObject *>( nullptr );
            }
            if ( !m_aCurr.IsValid() )
            {
                ::rtl::Reference < SwAccessibleContext > xAccImpl =
                                    m_rList.GetAccMap().GetContextImpl( &rFrame, false );
                if( xAccImpl.is() )
                {
                    m_aCurr = SwAccessibleChild(xAccImpl->GetAdditionalAccessibleChild(m_nNextObj - nObjsCount));
                    ++m_nNextObj;
                }
            }
        }
    }

    return *this;
}

SwAccessibleChildSList_const_iterator& SwAccessibleChildSList_const_iterator::next_visible()
{
    next();
    while( m_aCurr.IsValid() &&
           !m_aCurr.AlwaysIncludeAsChild() &&
           !m_aCurr.GetBox( m_rList.GetAccMap() ).Overlaps( m_rList.GetVisArea() ) )
    {
        next();
    }

    return *this;
}

SwAccessibleChildSList_const_iterator& SwAccessibleChildSList_const_iterator::operator++()
{
    return m_rList.IsVisibleChildrenOnly() ? next_visible() : next();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
