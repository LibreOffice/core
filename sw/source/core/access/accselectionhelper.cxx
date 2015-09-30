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
#include <vcl/svapp.hxx>
#include <flyfrm.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/XAccessibleStateSet.hpp>
#include <fmtanchr.hxx>

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

using ::com::sun::star::accessibility::XAccessible;
using ::com::sun::star::accessibility::XAccessibleContext;
using ::com::sun::star::accessibility::XAccessibleSelection;

using namespace ::sw::access;

SwAccessibleSelectionHelper::SwAccessibleSelectionHelper(
    SwAccessibleContext& rContext ) :
        m_rContext( rContext )
{
}

SwAccessibleSelectionHelper::~SwAccessibleSelectionHelper()
{
}

SwFEShell* SwAccessibleSelectionHelper::GetFEShell()
{
    OSL_ENSURE( m_rContext.GetMap() != NULL, "no map?" );
    SwViewShell* pViewShell = m_rContext.GetMap()->GetShell();
    OSL_ENSURE( pViewShell != NULL,
                "No view shell? Then what are you looking at?" );

    SwFEShell* pFEShell = dynamic_cast<SwFEShell*>( pViewShell );

    return pFEShell;
}

void SwAccessibleSelectionHelper::throwIndexOutOfBoundsException()
        throw ( lang::IndexOutOfBoundsException )
{
    Reference < XAccessibleContext > xThis( &m_rContext );
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
    const SwAccessibleChild aChild = m_rContext.GetChild( *(m_rContext.GetMap()),
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
            m_rContext.Select( const_cast< SdrObject *>( pObj ), 0==aChild.GetSwFrm());
    }
    // no frame shell, or no frame, or no fly frame -> can't select
}

//When the selected state of the SwFrmOrObj is setted, return true.
static bool lcl_getSelectedState(const SwAccessibleChild& aChild,
                                     SwAccessibleContext* pContext,
                                     SwAccessibleMap* pMap)
{
    Reference< XAccessible > xAcc;
    if ( aChild.GetSwFrm() )
    {
        xAcc = pMap->GetContext( aChild.GetSwFrm(), false );
    }
    else if ( aChild.GetDrawObject() )
    {
        xAcc = pMap->GetContext( aChild.GetDrawObject(), pContext, false );
    }

    if( xAcc.is() )
    {
        Reference< XAccessibleContext > pRContext = xAcc->getAccessibleContext();
        if(!pRContext.is())
            return false;
        Reference<XAccessibleStateSet> pRStateSet = pRContext->getAccessibleStateSet();
        if( pRStateSet.is() )
        {
            Sequence<short> pStates = pRStateSet->getStates();
            sal_Int32 count = pStates.getLength();
            for( sal_Int32 i = 0; i < count; i++ )
            {
                if( pStates[i] == AccessibleStateType::SELECTED)
                    return true;
            }
        }
    }
    return false;
}

bool SwAccessibleSelectionHelper::isAccessibleChildSelected(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            RuntimeException )
{
    SolarMutexGuard aGuard;

    // Get the respective child as SwFrm (also do index checking), ...
    const SwAccessibleChild aChild = m_rContext.GetChild( *(m_rContext.GetMap()),
                                                        nChildIndex );
    if( !aChild.IsValid() )
        throwIndexOutOfBoundsException();

    // ... and compare to the currently selected frame
    bool bRet = false;
    const SwFEShell* pFEShell = GetFEShell();
    if( pFEShell )
    {
        if ( aChild.GetSwFrm() != 0 )
        {
            bRet = (pFEShell->GetSelectedFlyFrm() == aChild.GetSwFrm());
        }
        else if ( aChild.GetDrawObject() )
        {
            bRet = pFEShell->IsObjSelected( *aChild.GetDrawObject() );
        }
        //If the SwFrmOrObj is not selected directly in the UI, we should check whether it is selected in the selection cursor.
        if( !bRet )
        {
            if( lcl_getSelectedState( aChild, &m_rContext, m_rContext.GetMap() ) )
                bRet = true;
        }
    }

    return bRet;
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
        m_rContext.GetChildren( *(m_rContext.GetMap()), aChildren );

        ::std::list< SwAccessibleChild >::const_iterator aIter = aChildren.begin();
        ::std::list< SwAccessibleChild >::const_iterator aEndIter = aChildren.end();
        while( aIter != aEndIter )
        {
            const SwAccessibleChild& rChild = *aIter;
            const SdrObject* pObj = rChild.GetDrawObject();
            const SwFrm* pFrm = rChild.GetSwFrm();
            if( pObj && !(pFrm != 0 && pFEShell->IsObjSelected()) )
            {
                m_rContext.Select( const_cast< SdrObject *>( pObj ), 0==pFrm );
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
    const SwFEShell* pFEShell = GetFEShell();
    if( pFEShell != 0 )
    {
        const SwFlyFrm* pFlyFrm = pFEShell->GetSelectedFlyFrm();
        if( pFlyFrm )
        {
            nCount = 1;
        }
        else
        {
            const size_t nSelObjs = pFEShell->IsObjSelected();
            if( nSelObjs > 0 )
            {
                ::std::list< SwAccessibleChild > aChildren;
                m_rContext.GetChildren( *(m_rContext.GetMap()), aChildren );

                ::std::list< SwAccessibleChild >::const_iterator aIter =
                    aChildren.begin();
                ::std::list< SwAccessibleChild >::const_iterator aEndIter =
                    aChildren.end();
                while( aIter != aEndIter && static_cast<size_t>(nCount) < nSelObjs )
                {
                    const SwAccessibleChild& rChild = *aIter;
                    if( rChild.GetDrawObject() && !rChild.GetSwFrm() &&
                        SwAccessibleFrame::GetParent(rChild, m_rContext.IsInPagePreview())
                           == m_rContext.GetFrm() &&
                        pFEShell->IsObjSelected( *rChild.GetDrawObject() ) )
                    {
                        nCount++;
                    }
                    ++aIter;
                }
            }
        }
        //If the SwFrmOrObj is not selected directly in the UI,
        //we should check whether it is selected in the selection cursor.
        if( nCount == 0 )
        {
            ::std::list< SwAccessibleChild > aChildren;
            m_rContext.GetChildren( *(m_rContext.GetMap()), aChildren );
            ::std::list< SwAccessibleChild >::const_iterator aIter =
                aChildren.begin();
            ::std::list< SwAccessibleChild >::const_iterator aEndIter =
                aChildren.end();
            while( aIter != aEndIter )
            {
                const SwAccessibleChild& aChild = *aIter;
                if( lcl_getSelectedState( aChild, &m_rContext, m_rContext.GetMap() ) )
                    nCount++;
                ++aIter;
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
    const SwFlyFrm *pFlyFrm = pFEShell->GetSelectedFlyFrm();
    if( pFlyFrm )
    {
        if( 0 == nSelectedChildIndex )
        {
            if(SwAccessibleFrame::GetParent( SwAccessibleChild(pFlyFrm), m_rContext.IsInPagePreview()) == m_rContext.GetFrm() )
            {
                aChild = pFlyFrm;
            }
            else
            {
                const SwFrameFormat *pFrameFormat = pFlyFrm->GetFormat();
                if (pFrameFormat)
                {
                    const SwFormatAnchor& pAnchor = pFrameFormat->GetAnchor();
                    if( pAnchor.GetAnchorId() == FLY_AS_CHAR )
                    {
                        const SwFrm  *pParaFrm =  SwAccessibleFrame::GetParent( SwAccessibleChild(pFlyFrm), m_rContext.IsInPagePreview() );
                        aChild  = pParaFrm;
                    }
                }
            }
        }
    }
    else
    {
        const size_t nSelObjs = pFEShell->IsObjSelected();
        if( 0 == nSelObjs || static_cast<size_t>(nSelectedChildIndex) >= nSelObjs )
            throwIndexOutOfBoundsException();

        ::std::list< SwAccessibleChild > aChildren;
        m_rContext.GetChildren( *(m_rContext.GetMap()), aChildren );

        ::std::list< SwAccessibleChild >::const_iterator aIter = aChildren.begin();
        ::std::list< SwAccessibleChild >::const_iterator aEndIter = aChildren.end();
        while( aIter != aEndIter && !aChild.IsValid() )
        {
            const SwAccessibleChild& rChild = *aIter;
            if( rChild.GetDrawObject() && !rChild.GetSwFrm() &&
                SwAccessibleFrame::GetParent(rChild, m_rContext.IsInPagePreview()) ==
                    m_rContext.GetFrm() &&
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

    OSL_ENSURE( m_rContext.GetMap() != NULL, "We need the map." );
    Reference< XAccessible > xChild;
    if( aChild.GetSwFrm() )
    {
        ::rtl::Reference < SwAccessibleContext > xChildImpl(
                m_rContext.GetMap()->GetContextImpl( aChild.GetSwFrm() ) );
        if( xChildImpl.is() )
        {
            xChildImpl->SetParent( &m_rContext );
            xChild = xChildImpl.get();
        }
    }
    else if ( aChild.GetDrawObject() )
    {
        ::rtl::Reference < ::accessibility::AccessibleShape > xChildImpl(
                m_rContext.GetMap()->GetContextImpl( aChild.GetDrawObject(),
                                          &m_rContext )  );
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
    SolarMutexGuard g;

    if( nChildIndex < 0 ||
        nChildIndex >= m_rContext.GetChildCount( *(m_rContext.GetMap()) ) )
        throwIndexOutOfBoundsException();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
