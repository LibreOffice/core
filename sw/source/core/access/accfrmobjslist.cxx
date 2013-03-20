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
      aCurr( rList.GetFrm().GetLower() ),
      nNextObj( 0 )
{
    if( !aCurr.GetSwFrm() )
    {
        const SwFrm& rFrm = rList.GetFrm();
        if( rFrm.IsPageFrm() )
        {
            const SwPageFrm& rPgFrm = static_cast< const SwPageFrm& >( rFrm );
            const SwSortedObjs *pObjs = rPgFrm.GetSortedObjs();
            if( pObjs && pObjs->Count() )
            {
                aCurr = (*pObjs)[nNextObj++]->GetDrawObj();
            }
        }
        else if( rFrm.IsTxtFrm() )
        {
            const SwSortedObjs *pObjs = rFrm.GetDrawObjs();
            if ( pObjs && pObjs->Count() )
            {
                aCurr = (*pObjs)[nNextObj++]->GetDrawObj();
                while( aCurr.IsValid() && !aCurr.IsBoundAsChar() )
                {
                    aCurr = (nNextObj < pObjs->Count())
                            ? (*pObjs)[nNextObj++]->GetDrawObj()
                            : static_cast< const SdrObject *>( 0 );
                }
            }
            if ( !aCurr.IsValid() )
            {
                ::rtl::Reference < SwAccessibleContext > xAccImpl =
                                    rAccMap.GetContextImpl( &rFrm, sal_False );
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
    else if( aCurr.GetSwFrm() )
    {
        aCurr = aCurr.GetSwFrm()->GetNext();
        if( !aCurr.GetSwFrm() )
        {
            bNextTaken = false;
        }
    }

    if( !bNextTaken )
    {
        const SwFrm& rFrm = rList.GetFrm();
        if( rFrm.IsPageFrm() )
        {
            const SwPageFrm& rPgFrm = static_cast< const SwPageFrm& >( rFrm );
            const SwSortedObjs *pObjs = rPgFrm.GetSortedObjs();
            aCurr = ( pObjs && nNextObj < pObjs->Count() )
                    ? (*pObjs)[nNextObj++]->GetDrawObj()
                    : static_cast< const SdrObject *>( 0 );
        }
        else if( rFrm.IsTxtFrm() )
        {
            const SwSortedObjs* pObjs = rFrm.GetDrawObjs();
            const sal_uInt32 nObjsCount = pObjs ? pObjs->Count() : 0;
            aCurr = ( pObjs && nNextObj < nObjsCount )
                    ? (*pObjs)[nNextObj++]->GetDrawObj()
                    : static_cast< const SdrObject *>( 0 );
            while( aCurr.IsValid() && !aCurr.IsBoundAsChar() )
            {
                aCurr = ( nNextObj < nObjsCount )
                        ? (*pObjs)[nNextObj++]->GetDrawObj()
                        : static_cast< const SdrObject *>( 0 );
            }
            if ( !aCurr.IsValid() )
            {
                ::rtl::Reference < SwAccessibleContext > xAccImpl =
                                    rList.GetAccMap().GetContextImpl( &rFrm, sal_False );
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
