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
#include "accselectionhelper.hxx"

#include "acccontext.hxx"
#include <accmap.hxx>
#include <o3tl/safeint.hxx>
#include <svx/AccessibleShape.hxx>
#include <viewsh.hxx>
#include <fesh.hxx>
#include <vcl/svapp.hxx>
#include <flyfrm.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
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

SwFEShell* SwAccessibleSelectionHelper::GetFEShell()
{
    OSL_ENSURE( m_rContext.GetMap() != nullptr, "no map?" );
    SwViewShell* pViewShell = m_rContext.GetMap()->GetShell();
    OSL_ENSURE( pViewShell != nullptr,
                "No view shell? Then what are you looking at?" );

    SwFEShell* pFEShell = dynamic_cast<SwFEShell*>( pViewShell );

    return pFEShell;
}

void SwAccessibleSelectionHelper::throwIndexOutOfBoundsException()
{
    Reference < XAccessibleContext > xThis( &m_rContext );
    Reference < XAccessibleSelection >xSelThis( xThis, UNO_QUERY );
    lang::IndexOutOfBoundsException aExcept(
                u"index out of bounds"_ustr,
                xSelThis );
    throw aExcept;
}

// XAccessibleSelection
void SwAccessibleSelectionHelper::selectAccessibleChild(
    sal_Int64 nChildIndex )
{
    SolarMutexGuard aGuard;

    if (nChildIndex < 0 || nChildIndex >= m_rContext.GetChildCount(*(m_rContext.GetMap())))
        throwIndexOutOfBoundsException();

    // Get the respective child as SwFrame (also do index checking), ...
    const SwAccessibleChild aChild = m_rContext.GetChild( *(m_rContext.GetMap()),
                                                        nChildIndex );
    if( !aChild.IsValid() )
        throwIndexOutOfBoundsException();

    // we can only select fly frames, so we ignore (should: return
    // false) all other attempts at child selection
    if (GetFEShell())
    {
        const SdrObject *pObj = aChild.GetDrawObject();
        if( pObj )
            m_rContext.Select( const_cast< SdrObject *>( pObj ), nullptr==aChild.GetSwFrame());
    }
    // no frame shell, or no frame, or no fly frame -> can't select
}

//When the selected state of the SwFrameOrObj is set, return true.
static bool lcl_getSelectedState(const SwAccessibleChild& aChild,
                                     SwAccessibleContext* pContext,
                                     SwAccessibleMap* pMap)
{
    Reference< XAccessible > xAcc;
    if ( aChild.GetSwFrame() )
    {
        xAcc = pMap->GetContext( aChild.GetSwFrame(), false );
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
        sal_Int64 nRStateSet = pRContext->getAccessibleStateSet();
        if(nRStateSet & AccessibleStateType::SELECTED)
            return true;
    }
    return false;
}

bool SwAccessibleSelectionHelper::isAccessibleChildSelected(
    sal_Int64 nChildIndex )
{
    SolarMutexGuard aGuard;

    if (nChildIndex < 0 || nChildIndex >= m_rContext.GetChildCount(*(m_rContext.GetMap())))
        throwIndexOutOfBoundsException();

    // Get the respective child as SwFrame (also do index checking), ...
    const SwAccessibleChild aChild = m_rContext.GetChild( *(m_rContext.GetMap()),
                                                        nChildIndex );
    if( !aChild.IsValid() )
        throwIndexOutOfBoundsException();

    // ... and compare to the currently selected frame
    bool bRet = false;
    if (const SwFEShell* pFEShell = GetFEShell())
    {
        if ( aChild.GetSwFrame() != nullptr )
        {
            bRet = (pFEShell->GetSelectedFlyFrame() == aChild.GetSwFrame());
        }
        else if ( aChild.GetDrawObject() )
        {
            bRet = pFEShell->IsObjSelected( *aChild.GetDrawObject() );
        }
        //If the SwFrameOrObj is not selected directly in the UI, we should check whether it is selected in the selection cursor.
        if( !bRet )
        {
            if( lcl_getSelectedState( aChild, &m_rContext, m_rContext.GetMap() ) )
                bRet = true;
        }
    }

    return bRet;
}

void SwAccessibleSelectionHelper::selectAllAccessibleChildren(  )
{
    SolarMutexGuard aGuard;

    // We can select only one. So iterate over the children to find
    // the first we can select, and select it.

    SwFEShell* pFEShell = GetFEShell();
    if (!pFEShell)
        return;

    std::list< SwAccessibleChild > aChildren;
    m_rContext.GetChildren( *(m_rContext.GetMap()), aChildren );

    for( const SwAccessibleChild& rChild : aChildren )
    {
        const SdrObject* pObj = rChild.GetDrawObject();
        const SwFrame* pFrame = rChild.GetSwFrame();
        if( pObj && !(pFrame != nullptr && pFEShell->IsObjSelected()) )
        {
            m_rContext.Select( const_cast< SdrObject *>( pObj ), nullptr==pFrame );
            if( pFrame )
                break;
        }
    }
}

sal_Int64 SwAccessibleSelectionHelper::getSelectedAccessibleChildCount(  )
{
    SolarMutexGuard aGuard;

    sal_Int64 nCount = 0;
    // Only one frame can be selected at a time, and we only frames
    // for selectable children.
    if (const SwFEShell* pFEShell = GetFEShell())
    {
        const SwFlyFrame* pFlyFrame = pFEShell->GetSelectedFlyFrame();
        if( pFlyFrame )
        {
            nCount = 1;
        }
        else
        {
            const size_t nSelObjs = pFEShell->IsObjSelected();
            if( nSelObjs > 0 )
            {
                std::list< SwAccessibleChild > aChildren;
                m_rContext.GetChildren( *(m_rContext.GetMap()), aChildren );

                for( const SwAccessibleChild& rChild : aChildren )
                {
                    if( rChild.GetDrawObject() && !rChild.GetSwFrame() &&
                        SwAccessibleFrame::GetParent(rChild, m_rContext.IsInPagePreview())
                           == m_rContext.GetFrame() &&
                        pFEShell->IsObjSelected( *rChild.GetDrawObject() ) )
                    {
                        nCount++;
                    }
                    if (o3tl::make_unsigned(nCount) >= nSelObjs)
                        break;
                }
            }
        }
        //If the SwFrameOrObj is not selected directly in the UI,
        //we should check whether it is selected in the selection cursor.
        if( nCount == 0 )
        {
            std::list< SwAccessibleChild > aChildren;
            m_rContext.GetChildren( *(m_rContext.GetMap()), aChildren );
            nCount = static_cast<sal_Int32>(std::count_if(aChildren.begin(), aChildren.end(),
                [this](const SwAccessibleChild& aChild) { return lcl_getSelectedState(aChild, &m_rContext, m_rContext.GetMap()); }));
        }
    }
    return nCount;
}

Reference<XAccessible> SwAccessibleSelectionHelper::getSelectedAccessibleChild(
    sal_Int64 nSelectedChildIndex )
{
    SolarMutexGuard aGuard;

    // Since the index is relative to the selected children, and since
    // there can be at most one selected frame child, the index must
    // be 0, and a selection must exist, otherwise we have to throw an
    // lang::IndexOutOfBoundsException
    SwFEShell* pFEShell = GetFEShell();
    if (!pFEShell)
        throwIndexOutOfBoundsException();

    SwAccessibleChild aChild;
    const SwFlyFrame *pFlyFrame = pFEShell->GetSelectedFlyFrame();
    if( pFlyFrame )
    {
        if( 0 == nSelectedChildIndex )
        {
            if(SwAccessibleFrame::GetParent( SwAccessibleChild(pFlyFrame), m_rContext.IsInPagePreview()) == m_rContext.GetFrame() )
            {
                aChild = pFlyFrame;
            }
            else
            {
                const SwFrameFormat *pFrameFormat = pFlyFrame->GetFormat();
                if (pFrameFormat)
                {
                    const SwFormatAnchor& rAnchor = pFrameFormat->GetAnchor();
                    if( rAnchor.GetAnchorId() == RndStdIds::FLY_AS_CHAR )
                    {
                        const SwFrame *pParaFrame = SwAccessibleFrame::GetParent( SwAccessibleChild(pFlyFrame), m_rContext.IsInPagePreview() );
                        aChild = pParaFrame;
                    }
                }
            }
        }
    }
    else
    {
        const size_t nSelObjs = pFEShell->IsObjSelected();
        if( 0 == nSelObjs || o3tl::make_unsigned(nSelectedChildIndex) >= nSelObjs )
            throwIndexOutOfBoundsException();

        std::list< SwAccessibleChild > aChildren;
        m_rContext.GetChildren( *(m_rContext.GetMap()), aChildren );

        for( const SwAccessibleChild& rChild : aChildren )
        {
            if( rChild.GetDrawObject() && !rChild.GetSwFrame() &&
                SwAccessibleFrame::GetParent(rChild, m_rContext.IsInPagePreview()) ==
                    m_rContext.GetFrame() &&
                pFEShell->IsObjSelected( *rChild.GetDrawObject() ) )
            {
                if( 0 == nSelectedChildIndex )
                    aChild = rChild;
                else
                    --nSelectedChildIndex;
            }
            if (aChild.IsValid())
                break;
        }
    }

    if( !aChild.IsValid() )
        throwIndexOutOfBoundsException();

    OSL_ENSURE( m_rContext.GetMap() != nullptr, "We need the map." );
    Reference< XAccessible > xChild;
    if( aChild.GetSwFrame() )
    {
        ::rtl::Reference < SwAccessibleContext > xChildImpl(
                m_rContext.GetMap()->GetContextImpl( aChild.GetSwFrame() ) );
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
    sal_Int64 nChildIndex )
{
    SolarMutexGuard g;

    if( nChildIndex < 0 ||
        nChildIndex >= m_rContext.GetChildCount( *(m_rContext.GetMap()) ) )
        throwIndexOutOfBoundsException();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
