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

#include <accfrmobjslist.hxx>
#include <accmap.hxx>
#include <acccontext.hxx>

#include <pagefrm.hxx>
#include <sortedobjs.hxx>
#include <anchoredobject.hxx>

using namespace ::sw::access;

SwAccessibleChildSList_const_iterator::SwAccessibleChildSList_const_iterator(
    const SwAccessibleChildSList& rLst,
    SwAccessibleMap& rAccMap )
    : rList( rLst ),
      aCurr( rList.GetFrame().GetLower() ),
      nNextObj( 0 )
{
    if( !aCurr.GetSwFrame() )
    {
        const SwFrame& rFrame = rList.GetFrame();
        if( rFrame.IsPageFrame() )
        {
            const SwPageFrame& rPgFrame = static_cast< const SwPageFrame& >( rFrame );
            const SwSortedObjs *pObjs = rPgFrame.GetSortedObjs();
            if( pObjs && pObjs->size() )
            {
                aCurr = (*pObjs)[nNextObj++]->GetDrawObj();
            }
        }
        else if( rFrame.IsTextFrame() )
        {
            const SwSortedObjs *pObjs = rFrame.GetDrawObjs();
            if ( pObjs && pObjs->size() )
            {
                aCurr = (*pObjs)[nNextObj++]->GetDrawObj();
                while( aCurr.IsValid() && !aCurr.IsBoundAsChar() )
                {
                    aCurr = (nNextObj < pObjs->size())
                            ? (*pObjs)[nNextObj++]->GetDrawObj()
                            : static_cast< const SdrObject *>( nullptr );
                }
            }
            if ( !aCurr.IsValid() )
            {
                ::rtl::Reference < SwAccessibleContext > xAccImpl =
                                    rAccMap.GetContextImpl( &rFrame, false );
                if( xAccImpl.is() )
                {
                    SwAccessibleContext* pAccImpl = xAccImpl.get();
                    aCurr = SwAccessibleChild( pAccImpl->GetAdditionalAccessibleChild( 0 ) );
                    ++nNextObj;
                }
            }
        }
    }

    if( rList.IsVisibleChildrenOnly() )
    {
        // Find the first visible
        while( aCurr.IsValid() &&
               !aCurr.AlwaysIncludeAsChild() &&
               !aCurr.GetBox( rAccMap ).IsOver( rList.GetVisArea() ) )
        {
            next();
        }
    }
}

SwAccessibleChildSList_const_iterator& SwAccessibleChildSList_const_iterator::next()
{
    bool bNextTaken( true );
    if( aCurr.GetDrawObject() || aCurr.GetWindow() )
    {
        bNextTaken = false;
    }
    else if( aCurr.GetSwFrame() )
    {
        aCurr = aCurr.GetSwFrame()->GetNext();
        if( !aCurr.GetSwFrame() )
        {
            bNextTaken = false;
        }
    }

    if( !bNextTaken )
    {
        const SwFrame& rFrame = rList.GetFrame();
        if( rFrame.IsPageFrame() )
        {
            const SwPageFrame& rPgFrame = static_cast< const SwPageFrame& >( rFrame );
            const SwSortedObjs *pObjs = rPgFrame.GetSortedObjs();
            aCurr = ( pObjs && nNextObj < pObjs->size() )
                    ? (*pObjs)[nNextObj++]->GetDrawObj()
                    : static_cast< const SdrObject *>( nullptr );
        }
        else if( rFrame.IsTextFrame() )
        {
            const SwSortedObjs* pObjs = rFrame.GetDrawObjs();
            const size_t nObjsCount = pObjs ? pObjs->size() : 0;
            aCurr = ( pObjs && nNextObj < nObjsCount )
                    ? (*pObjs)[nNextObj++]->GetDrawObj()
                    : static_cast< const SdrObject *>( nullptr );
            while( aCurr.IsValid() && !aCurr.IsBoundAsChar() )
            {
                aCurr = ( nNextObj < nObjsCount )
                        ? (*pObjs)[nNextObj++]->GetDrawObj()
                        : static_cast< const SdrObject *>( nullptr );
            }
            if ( !aCurr.IsValid() )
            {
                ::rtl::Reference < SwAccessibleContext > xAccImpl =
                                    rList.GetAccMap().GetContextImpl( &rFrame, false );
                if( xAccImpl.is() )
                {
                    SwAccessibleContext* pAccImpl = xAccImpl.get();
                    aCurr = SwAccessibleChild( pAccImpl->GetAdditionalAccessibleChild( nNextObj - nObjsCount ) );
                    ++nNextObj;
                }
            }
        }
    }

    return *this;
}

SwAccessibleChildSList_const_iterator& SwAccessibleChildSList_const_iterator::next_visible()
{
    next();
    while( aCurr.IsValid() &&
           !aCurr.AlwaysIncludeAsChild() &&
           !aCurr.GetBox( rList.GetAccMap() ).IsOver( rList.GetVisArea() ) )
    {
        next();
    }

    return *this;
}

SwAccessibleChildSList_const_iterator& SwAccessibleChildSList_const_iterator::operator++()
{
    return rList.IsVisibleChildrenOnly() ? next_visible() : next();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
