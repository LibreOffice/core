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

#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <accselectionhelper.hxx>

#include <acccontext.hxx>
#include <accmap.hxx>
#include <svx/AccessibleShape.hxx>
#include <viewsh.hxx>
#include <fesh.hxx>
#include <vcl/svapp.hxx>        // for SolarMutex
#include <flyfrm.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

using ::com::sun::star::accessibility::XAccessible;
using ::com::sun::star::accessibility::XAccessibleContext;
using ::com::sun::star::accessibility::XAccessibleSelection;

using namespace ::sw::access;

SwAccessibleSelectionHelper::SwAccessibleSelectionHelper(
    SwAccessibleContext& rCtxt ) :
        rContext( rCtxt )
{
}

SwAccessibleSelectionHelper::~SwAccessibleSelectionHelper()
{
}

SwFEShell* SwAccessibleSelectionHelper::GetFEShell()
{
    OSL_ENSURE( rContext.GetMap() != NULL, "no map?" );
    ViewShell* pViewShell = rContext.GetMap()->GetShell();
    OSL_ENSURE( pViewShell != NULL,
                "No view shell? Then what are you looking at?" );

    SwFEShell* pFEShell = NULL;
    if( pViewShell->ISA( SwFEShell ) )
    {
        pFEShell = static_cast<SwFEShell*>( pViewShell );
    }

    return pFEShell;
}

void SwAccessibleSelectionHelper::throwIndexOutOfBoundsException()
        throw ( lang::IndexOutOfBoundsException )
{
    Reference < XAccessibleContext > xThis( &rContext );
    Reference < XAccessibleSelection >xSelThis( xThis, UNO_QUERY );
    lang::IndexOutOfBoundsException aExcept(
                OUString( "index out of bounds" ),
                xSelThis );                                     \
    throw aExcept;
}


// XAccessibleSelection
void SwAccessibleSelectionHelper::selectAccessibleChild(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            RuntimeException )
{
    SolarMutexGuard aGuard;

    // Get the respective child as SwFrm (also do index checking), ...
    const SwAccessibleChild aChild = rContext.GetChild( *(rContext.GetMap()),
                                                        nChildIndex );
    if( !aChild.IsValid() )
        throwIndexOutOfBoundsException();

    // we can only select fly frames, so we ignore (should: return
    // false) all other attempts at child selection
    SwFEShell* pFEShell = GetFEShell();
    if( pFEShell != NULL )
    {
        const SdrObject *pObj = aChild.GetDrawObject();
        if( pObj )
            rContext.Select( const_cast< SdrObject *>( pObj ), 0==aChild.GetSwFrm());
    }
    // no frame shell, or no frame, or no fly frame -> can't select
}

sal_Bool SwAccessibleSelectionHelper::isAccessibleChildSelected(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            RuntimeException )
{
    SolarMutexGuard aGuard;

    // Get the respective child as SwFrm (also do index checking), ...
    const SwAccessibleChild aChild = rContext.GetChild( *(rContext.GetMap()),
                                                        nChildIndex );
    if( !aChild.IsValid() )
        throwIndexOutOfBoundsException();

    // ... and compare to the currently selected frame
    sal_Bool bRet = sal_False;
    SwFEShell* pFEShell = GetFEShell();
    if( pFEShell )
    {
        if ( aChild.GetSwFrm() != 0 )
        {
            bRet = (pFEShell->GetCurrFlyFrm() == aChild.GetSwFrm());
        }
        else if ( aChild.GetDrawObject() )
        {
            bRet = pFEShell->IsObjSelected( *aChild.GetDrawObject() );
        }
    }

    return bRet;
}

void SwAccessibleSelectionHelper::clearAccessibleSelection(  )
    throw ( RuntimeException )
{
    // return sal_False     // we can't deselect
}

void SwAccessibleSelectionHelper::selectAllAccessibleChildren(  )
    throw ( RuntimeException )
{
    SolarMutexGuard aGuard;

    // We can select only one. So iterate over the children to find
    // the first we can select, and select it.

    SwFEShell* pFEShell = GetFEShell();
    if( pFEShell )
    {
        ::std::list< SwAccessibleChild > aChildren;
        rContext.GetChildren( *(rContext.GetMap()), aChildren );

        ::std::list< SwAccessibleChild >::const_iterator aIter = aChildren.begin();
        ::std::list< SwAccessibleChild >::const_iterator aEndIter = aChildren.end();
        while( aIter != aEndIter )
        {
            const SwAccessibleChild& rChild = *aIter;
            const SdrObject* pObj = rChild.GetDrawObject();
            const SwFrm* pFrm = rChild.GetSwFrm();
            if( pObj && !(pFrm != 0 && pFEShell->IsObjSelected()) )
            {
                rContext.Select( const_cast< SdrObject *>( pObj ), 0==pFrm );
                if( pFrm )
                    break;
            }
            ++aIter;
        }
    }
}

sal_Int32 SwAccessibleSelectionHelper::getSelectedAccessibleChildCount(  )
    throw ( RuntimeException )
{
    SolarMutexGuard aGuard;

    sal_Int32 nCount = 0;
    // Only one frame can be selected at a time, and we only frames
    // for selectable children.
    SwFEShell* pFEShell = GetFEShell();
    if( pFEShell != 0 )
    {
        const SwFlyFrm* pFlyFrm = pFEShell->GetCurrFlyFrm();
        if( pFlyFrm )
        {
            if( rContext.GetParent( SwAccessibleChild(pFlyFrm), rContext.IsInPagePreview()) ==
                    rContext.GetFrm() )
            {
                nCount = 1;
            }
        }
        else
        {
            sal_uInt16 nSelObjs = pFEShell->IsObjSelected();
            if( nSelObjs > 0 )
            {
                ::std::list< SwAccessibleChild > aChildren;
                rContext.GetChildren( *(rContext.GetMap()), aChildren );

                ::std::list< SwAccessibleChild >::const_iterator aIter =
                    aChildren.begin();
                ::std::list< SwAccessibleChild >::const_iterator aEndIter =
                    aChildren.end();
                while( aIter != aEndIter && nCount < nSelObjs )
                {
                    const SwAccessibleChild& rChild = *aIter;
                    if( rChild.GetDrawObject() && !rChild.GetSwFrm() &&
                        rContext.GetParent(rChild, rContext.IsInPagePreview())
                           == rContext.GetFrm() &&
                        pFEShell->IsObjSelected( *rChild.GetDrawObject() ) )
                    {
                        nCount++;
                    }
                    ++aIter;
                }
            }
        }
    }
    return nCount;
}

Reference<XAccessible> SwAccessibleSelectionHelper::getSelectedAccessibleChild(
    sal_Int32 nSelectedChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            RuntimeException)
{
    SolarMutexGuard aGuard;

    // Since the index is relative to the selected children, and since
    // there can be at most one selected frame child, the index must
    // be 0, and a selection must exist, otherwise we have to throw an
    // lang::IndexOutOfBoundsException
    SwFEShell* pFEShell = GetFEShell();
    if( 0 == pFEShell )
        throwIndexOutOfBoundsException();

    SwAccessibleChild aChild;
    const SwFlyFrm *pFlyFrm = pFEShell->GetCurrFlyFrm();
    if( pFlyFrm )
    {
        if( 0 == nSelectedChildIndex &&
            rContext.GetParent( SwAccessibleChild(pFlyFrm), rContext.IsInPagePreview()) ==
                rContext.GetFrm() )
        {
            aChild = pFlyFrm;
        }
    }
    else
    {
        sal_uInt16 nSelObjs = pFEShell->IsObjSelected();
        if( 0 == nSelObjs || nSelectedChildIndex >= nSelObjs )
            throwIndexOutOfBoundsException();

        ::std::list< SwAccessibleChild > aChildren;
        rContext.GetChildren( *(rContext.GetMap()), aChildren );

        ::std::list< SwAccessibleChild >::const_iterator aIter = aChildren.begin();
        ::std::list< SwAccessibleChild >::const_iterator aEndIter = aChildren.end();
        while( aIter != aEndIter && !aChild.IsValid() )
        {
            const SwAccessibleChild& rChild = *aIter;
            if( rChild.GetDrawObject() && !rChild.GetSwFrm() &&
                rContext.GetParent(rChild, rContext.IsInPagePreview()) ==
                    rContext.GetFrm() &&
                pFEShell->IsObjSelected( *rChild.GetDrawObject() ) )
            {
                if( 0 == nSelectedChildIndex )
                    aChild = rChild;
                else
                    --nSelectedChildIndex;
            }
            ++aIter;
        }
    }

    if( !aChild.IsValid() )
        throwIndexOutOfBoundsException();

    OSL_ENSURE( rContext.GetMap() != NULL, "We need the map." );
    Reference< XAccessible > xChild;
    if( aChild.GetSwFrm() )
    {
        ::rtl::Reference < SwAccessibleContext > xChildImpl(
                rContext.GetMap()->GetContextImpl( aChild.GetSwFrm(),
                sal_True ) );
        if( xChildImpl.is() )
        {
            xChildImpl->SetParent( &rContext );
            xChild = xChildImpl.get();
        }
    }
    else if ( aChild.GetDrawObject() )
    {
        ::rtl::Reference < ::accessibility::AccessibleShape > xChildImpl(
                rContext.GetMap()->GetContextImpl( aChild.GetDrawObject(),
                                          &rContext, sal_True )  );
        if( xChildImpl.is() )
            xChild = xChildImpl.get();
    }
    return xChild;
}

// index has to be treated as global child index.
void SwAccessibleSelectionHelper::deselectAccessibleChild(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            RuntimeException )
{
    if( nChildIndex < 0 ||
        nChildIndex >= rContext.GetChildCount( *(rContext.GetMap()) ) )
        throwIndexOutOfBoundsException();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
