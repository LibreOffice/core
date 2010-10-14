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
#include "precompiled_sw.hxx"

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
                ::vos::ORef < SwAccessibleContext > xAccImpl =
                                    rAccMap.GetContextImpl( &rFrm, sal_False );
                if( xAccImpl.isValid() )
                {
                    SwAccessibleContext* pAccImpl = xAccImpl.getBodyPtr();
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
                ::vos::ORef < SwAccessibleContext > xAccImpl =
                                    rList.GetAccMap().GetContextImpl( &rFrm, sal_False );
                if( xAccImpl.isValid() )
                {
                    SwAccessibleContext* pAccImpl = xAccImpl.getBodyPtr();
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
