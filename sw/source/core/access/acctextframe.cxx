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

#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleRelation.hpp>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/accessibility/XAccessibleRelationSet.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <frmfmt.hxx>
#include <flyfrm.hxx>
#include <accmap.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <doc.hxx>
#include <hints.hxx>
#include "acctextframe.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

using utl::AccessibleRelationSetHelper;
using ::com::sun::star::accessibility::XAccessibleContext;

SwAccessibleTextFrame::SwAccessibleTextFrame(
        SwAccessibleMap* pInitMap,
        const SwFlyFrame& rFlyFrame  ) :
    SwAccessibleFrameBase( pInitMap, AccessibleRole::TEXT_FRAME, &rFlyFrame ),
    msTitle(),
    msDesc()
{
    const SwFlyFrameFormat* pFlyFrameFormat =
                    dynamic_cast<const SwFlyFrameFormat*>( rFlyFrame.GetFormat() );
    msTitle = pFlyFrameFormat->GetObjTitle();

    msDesc = pFlyFrameFormat->GetObjDescription();
    if ( msDesc.isEmpty() &&
         msTitle != GetName() )
    {
        msDesc = msTitle;
    }
}

SwAccessibleTextFrame::~SwAccessibleTextFrame()
{
}

void SwAccessibleTextFrame::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    const sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0 ;
    // #i73249# - suppress handling of RES_NAME_CHANGED
    // in case that attribute Title is used as the accessible name.
    if ( nWhich != RES_NAME_CHANGED ||
         msTitle.isEmpty() )
    {
        SwAccessibleFrameBase::Modify( pOld, pNew );
    }

    const SwFlyFrame *pFlyFrame = static_cast< const SwFlyFrame * >( GetFrame() );
    switch( nWhich )
    {
        // #i73249#
        case RES_TITLE_CHANGED:
        {
            OUString sOldTitle, sNewTitle;
            const SwStringMsgPoolItem *pOldItem = dynamic_cast<const SwStringMsgPoolItem*>(pOld);
            if (pOldItem)
                sOldTitle = pOldItem->GetString();
            const SwStringMsgPoolItem *pNewItem = dynamic_cast<const SwStringMsgPoolItem*>(pNew);
            if (pNewItem)
                sNewTitle = pNewItem->GetString();
            if (sOldTitle == sNewTitle)
            {
                break;
            }
            msTitle = sNewTitle;
            AccessibleEventObject aEvent;
            aEvent.EventId = AccessibleEventId::NAME_CHANGED;
            aEvent.OldValue <<= OUString( sOldTitle );
            aEvent.NewValue <<= msTitle;
            FireAccessibleEvent( aEvent );

            const SwFlyFrameFormat* pFlyFrameFormat =
                            dynamic_cast<const SwFlyFrameFormat*>( pFlyFrame->GetFormat() );
            if (!pFlyFrameFormat || !pFlyFrameFormat->GetObjDescription().isEmpty())
            {
                break;
            }
            SAL_FALLTHROUGH;
        }
        case RES_DESCRIPTION_CHANGED:
        {
            if ( pFlyFrame )
            {
                const OUString sOldDesc( msDesc );

                const SwFlyFrameFormat* pFlyFrameFormat =
                                dynamic_cast<const SwFlyFrameFormat*>( pFlyFrame->GetFormat() );
                const OUString& rDesc = pFlyFrameFormat->GetObjDescription();
                msDesc = rDesc;
                if ( msDesc.isEmpty() &&
                     msTitle != GetName() )
                {
                    msDesc = msTitle;
                }

                if ( msDesc != sOldDesc )
                {
                    AccessibleEventObject aEvent;
                    aEvent.EventId = AccessibleEventId::DESCRIPTION_CHANGED;
                    aEvent.OldValue <<= sOldDesc;
                    aEvent.NewValue <<= msDesc;
                    FireAccessibleEvent( aEvent );
                }
            }
        }
        break;
    }
}

// XInterface

css::uno::Any SAL_CALL
    SwAccessibleTextFrame::queryInterface (const css::uno::Type & rType)
    throw (css::uno::RuntimeException, std::exception)
{
    css::uno::Any aReturn = SwAccessibleContext::queryInterface (rType);
    if ( ! aReturn.hasValue())
        aReturn = ::cppu::queryInterface (rType,
            static_cast< css::accessibility::XAccessibleSelection* >(this)
            );
    return aReturn;
}

void SAL_CALL
    SwAccessibleTextFrame::acquire()
    throw ()
{
    SwAccessibleContext::acquire ();
}

void SAL_CALL
    SwAccessibleTextFrame::release()
    throw ()
{
    SwAccessibleContext::release ();
}

// XAccessibleSelection

void SAL_CALL SwAccessibleTextFrame::selectAccessibleChild( sal_Int32 )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception )
{
    SAL_WARN("sw.core", "<SwAccessibleTextFrame::selectAccessibleChild( sal_Int32 )> - missing implementation" );
}

sal_Bool SAL_CALL SwAccessibleTextFrame::isAccessibleChildSelected( sal_Int32 nChildIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard g;

    uno::Reference<XAccessible> xAcc = getAccessibleChild( nChildIndex );
    uno::Reference<XAccessibleContext> xContext;
    if( xAcc.is() )
        xContext = xAcc->getAccessibleContext();

    if( xContext.is() )
    {
        if( xContext->getAccessibleRole() == AccessibleRole::PARAGRAPH )
        {
            uno::Reference< css::accessibility::XAccessibleText >
                xText(xAcc, uno::UNO_QUERY);
            if( xText.is() )
            {
                if( xText->getSelectionStart() >= 0 ) return sal_True;
            }
        }
    }

    return sal_False;
}

void SAL_CALL SwAccessibleTextFrame::clearAccessibleSelection(  )
    throw ( uno::RuntimeException, std::exception )
{
    SAL_WARN("sw.core", "<SwAccessibleTextFrame::clearAccessibleSelection(  )> - missing implementation");
}

void SAL_CALL SwAccessibleTextFrame::selectAllAccessibleChildren(  )
    throw ( uno::RuntimeException, std::exception )
{
    SAL_WARN("sw.core", "<SwAccessibleTextFrame::selectAllAccessibleChildren(  )> - missing implementation");
}

sal_Int32 SAL_CALL SwAccessibleTextFrame::getSelectedAccessibleChildCount()
    throw ( uno::RuntimeException, std::exception )
{
    sal_Int32 nCount = 0;
    sal_Int32 TotalCount = getAccessibleChildCount();
    for( sal_Int32 i = 0; i < TotalCount; i++ )
        if( isAccessibleChildSelected(i) ) nCount++;

    return nCount;
}

uno::Reference<XAccessible> SAL_CALL SwAccessibleTextFrame::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    if ( nSelectedChildIndex > getSelectedAccessibleChildCount() )
        throw lang::IndexOutOfBoundsException();
    sal_Int32 i1, i2;
    for( i1 = 0, i2 = 0; i1 < getAccessibleChildCount(); i1++ )
        if( isAccessibleChildSelected(i1) )
        {
            if( i2 == nSelectedChildIndex )
                return getAccessibleChild( i1 );
            i2++;
        }
    return uno::Reference<XAccessible>();
}

void SAL_CALL SwAccessibleTextFrame::deselectAccessibleChild( sal_Int32 )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception )
{
    SAL_WARN("sw.core", "<SwAccessibleTextFrame::selectAllAccessibleChildren( sal_Int32 )> - missing implementation");
}

// #i73249#
OUString SAL_CALL SwAccessibleTextFrame::getAccessibleName()
        throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleContext )

    if ( !msTitle.isEmpty() )
    {
        return msTitle;
    }

    return SwAccessibleFrameBase::getAccessibleName();
}

OUString SAL_CALL SwAccessibleTextFrame::getAccessibleDescription()
        throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleContext )

    return msDesc;

}

OUString SAL_CALL SwAccessibleTextFrame::getImplementationName()
        throw( uno::RuntimeException, std::exception )
{
    return OUString("com.sun.star.comp.Writer.SwAccessibleTextFrameView");
}

sal_Bool SAL_CALL SwAccessibleTextFrame::supportsService(const OUString& sTestServiceName)
    throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, sTestServiceName);
}

uno::Sequence< OUString > SAL_CALL SwAccessibleTextFrame::getSupportedServiceNames()
        throw( uno::RuntimeException, std::exception )
{
    uno::Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.text.AccessibleTextFrameView";
    pArray[1] = sAccessibleServiceName;
    return aRet;
}

uno::Sequence< sal_Int8 > SAL_CALL SwAccessibleTextFrame::getImplementationId()
        throw(uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

// XAccessibleRelationSet

SwFlyFrame* SwAccessibleTextFrame::getFlyFrame() const
{
    SwFlyFrame* pFlyFrame = nullptr;

    const SwFrame* pFrame = GetFrame();
    assert(pFrame);
    if( pFrame->IsFlyFrame() )
    {
        pFlyFrame = static_cast<SwFlyFrame*>( const_cast<SwFrame*>( pFrame ) );
    }

    return pFlyFrame;
}

AccessibleRelation SwAccessibleTextFrame::makeRelation( sal_Int16 nType, const SwFlyFrame* pFrame )
{
    uno::Sequence<uno::Reference<XInterface> > aSequence { GetMap()->GetContext( pFrame ) };
    return AccessibleRelation( nType, aSequence );
}

uno::Reference<XAccessibleRelationSet> SAL_CALL SwAccessibleTextFrame::getAccessibleRelationSet( )
    throw ( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    CHECK_FOR_DEFUNC( XAccessibleContext );

    // get the frame, and insert prev/next relations into helper

    AccessibleRelationSetHelper* pHelper = new AccessibleRelationSetHelper();

    SwFlyFrame* pFlyFrame = getFlyFrame();
    assert(pFlyFrame);

    const SwFlyFrame* pPrevFrame = pFlyFrame->GetPrevLink();
    if( pPrevFrame != nullptr )
        pHelper->AddRelation( makeRelation(
            AccessibleRelationType::CONTENT_FLOWS_FROM, pPrevFrame ) );

    const SwFlyFrame* pNextFrame = pFlyFrame->GetNextLink();
    if( pNextFrame != nullptr )
        pHelper->AddRelation( makeRelation(
            AccessibleRelationType::CONTENT_FLOWS_TO, pNextFrame ) );

    return pHelper;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
