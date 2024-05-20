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
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <vcl/svapp.hxx>
#include <sal/log.hxx>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleRelation.hpp>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/accessibility/XAccessibleRelationSet.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <frmfmt.hxx>
#include <flyfrm.hxx>
#include <accmap.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <hints.hxx>
#include "acctextframe.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

using utl::AccessibleRelationSetHelper;
using ::com::sun::star::accessibility::XAccessibleContext;

SwAccessibleTextFrame::SwAccessibleTextFrame(
        std::shared_ptr<SwAccessibleMap> const& pInitMap,
        const SwFlyFrame& rFlyFrame  ) :
    SwAccessibleFrameBase( pInitMap, AccessibleRole::TEXT_FRAME, &rFlyFrame )
{
    const SwFlyFrameFormat* pFlyFrameFormat = rFlyFrame.GetFormat();
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

void SwAccessibleTextFrame::Notify(const SfxHint& rHint)
{
    const SwFlyFrame* pFlyFrame = static_cast<const SwFlyFrame*>(GetFrame());
    const SwFlyFrameFormat* pFlyFrameFormat = pFlyFrame ? pFlyFrame->GetFormat() : nullptr;
    switch(rHint.GetId())
    {
        case SfxHintId::Dying:
            EndListeningAll();
            return;
        default:
            return;
        // #i73249#
        case SfxHintId::SwTitleChanged:
        {
            auto rTitleChanged = static_cast<const sw::TitleChanged&>(rHint);
            msTitle = rTitleChanged.m_sNew;
            AccessibleEventObject aEvent;
            aEvent.EventId = AccessibleEventId::NAME_CHANGED;
            aEvent.OldValue <<= rTitleChanged.m_sOld;
            aEvent.NewValue <<= msTitle;
            FireAccessibleEvent( aEvent );
            if(!pFlyFrameFormat || !pFlyFrameFormat->GetObjDescription().isEmpty())
                break;
            [[fallthrough]];
        }
        case SfxHintId::SwDescriptionChanged:
        {
            if(!pFlyFrame)
                return;
            const OUString sOldDesc(msDesc);
            msDesc = pFlyFrameFormat->GetObjDescription();
            if(msDesc.isEmpty() && msTitle != GetName())
                msDesc = msTitle;
            if(msDesc == sOldDesc)
                return;
            AccessibleEventObject aEvent;
            aEvent.EventId = AccessibleEventId::DESCRIPTION_CHANGED;
            aEvent.OldValue <<= sOldDesc;
            aEvent.NewValue <<= msDesc;
            FireAccessibleEvent(aEvent);
            return;
        }
    }
}

// XInterface

css::uno::Any SAL_CALL
    SwAccessibleTextFrame::queryInterface (const css::uno::Type & rType)
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
    noexcept
{
    SwAccessibleContext::acquire ();
}

void SAL_CALL
    SwAccessibleTextFrame::release()
    noexcept
{
    SwAccessibleContext::release ();
}

// XAccessibleSelection

void SAL_CALL SwAccessibleTextFrame::selectAccessibleChild( sal_Int64 )
{
    SAL_WARN("sw.a11y", "SwAccessibleTextFrame::selectAccessibleChild - missing implementation");
}

sal_Bool SAL_CALL SwAccessibleTextFrame::isAccessibleChildSelected( sal_Int64 nChildIndex )
{
    SolarMutexGuard g;

    uno::Reference<XAccessible> xAcc = getAccessibleChild( nChildIndex );
    uno::Reference<XAccessibleContext> xContext;
    if( xAcc.is() )
        xContext = xAcc->getAccessibleContext();

    if( xContext.is() )
    {
        const sal_Int16 nRole = xContext->getAccessibleRole();
        if (nRole  == AccessibleRole::PARAGRAPH || nRole == AccessibleRole::BLOCK_QUOTE)
        {
            uno::Reference< css::accessibility::XAccessibleText >
                xText(xAcc, uno::UNO_QUERY);
            if( xText.is() )
            {
                if( xText->getSelectionStart() >= 0 ) return true;
            }
        }
    }

    return false;
}

void SAL_CALL SwAccessibleTextFrame::clearAccessibleSelection(  )
{
    SAL_WARN("sw.a11y", "<SwAccessibleTextFrame::clearAccessibleSelection()> - missing implementation");
}

void SAL_CALL SwAccessibleTextFrame::selectAllAccessibleChildren(  )
{
    SAL_WARN("sw.a11y", "<SwAccessibleTextFrame::selectAllAccessibleChildren()> - missing implementation");
}

sal_Int64 SAL_CALL SwAccessibleTextFrame::getSelectedAccessibleChildCount()
{
    sal_Int64 nCount = 0;
    sal_Int64 TotalCount = getAccessibleChildCount();
    for( sal_Int64 i = 0; i < TotalCount; i++ )
        if( isAccessibleChildSelected(i) ) nCount++;

    return nCount;
}

uno::Reference<XAccessible> SAL_CALL SwAccessibleTextFrame::getSelectedAccessibleChild( sal_Int64 nSelectedChildIndex )
{
    SolarMutexGuard g;

    if ( nSelectedChildIndex > getSelectedAccessibleChildCount() )
        throw lang::IndexOutOfBoundsException();

    for(sal_Int64 i1 = 0, i2 = 0; i1 < getAccessibleChildCount(); i1++ )
        if( isAccessibleChildSelected(i1) )
        {
            if( i2 == nSelectedChildIndex )
                return getAccessibleChild( i1 );
            i2++;
        }
    return uno::Reference<XAccessible>();
}

void SAL_CALL SwAccessibleTextFrame::deselectAccessibleChild( sal_Int64 )
{
    SAL_WARN("sw.a11y", "SwAccessibleTextFrame::selectAllAccessibleChildren - missing implementation");
}

// #i73249#
OUString SAL_CALL SwAccessibleTextFrame::getAccessibleName()
{
    SolarMutexGuard aGuard;

    ThrowIfDisposed();

    if ( !msTitle.isEmpty() )
    {
        return msTitle;
    }

    return SwAccessibleFrameBase::getAccessibleName();
}

OUString SAL_CALL SwAccessibleTextFrame::getAccessibleDescription()
{
    SolarMutexGuard aGuard;

    ThrowIfDisposed();

    return msDesc;

}

OUString SAL_CALL SwAccessibleTextFrame::getImplementationName()
{
    return u"com.sun.star.comp.Writer.SwAccessibleTextFrameView"_ustr;
}

sal_Bool SAL_CALL SwAccessibleTextFrame::supportsService(const OUString& sTestServiceName)
{
    return cppu::supportsService(this, sTestServiceName);
}

uno::Sequence< OUString > SAL_CALL SwAccessibleTextFrame::getSupportedServiceNames()
{
    return { u"com.sun.star.text.AccessibleTextFrameView"_ustr, sAccessibleServiceName };
}

uno::Sequence< sal_Int8 > SAL_CALL SwAccessibleTextFrame::getImplementationId()
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
    uno::Sequence<uno::Reference<XAccessible>> aSequence { GetMap()->GetContext(pFrame) };
    return AccessibleRelation( nType, aSequence );
}

uno::Reference<XAccessibleRelationSet> SAL_CALL SwAccessibleTextFrame::getAccessibleRelationSet( )
{
    SolarMutexGuard aGuard;

    ThrowIfDisposed();

    // get the frame, and insert prev/next relations into helper

    rtl::Reference<AccessibleRelationSetHelper> pHelper = new AccessibleRelationSetHelper();

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
