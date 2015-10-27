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

#include <sal/config.h>

#include <memory>
#include <utility>

#include "scitems.hxx"
#include <editeng/eeitem.hxx>
#include <tools/gen.hxx>
#include "AccessibleText.hxx"
#include "editsrc.hxx"
#include "AccessiblePreviewCell.hxx"
#include "AccessibilityHints.hxx"
#include "prevwsh.hxx"
#include "prevloc.hxx"
#include "document.hxx"
#include <svx/AccessibleTextHelper.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <editeng/brushitem.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/helper/convert.hxx>
#include <comphelper/servicehelper.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

//=====  internal  ============================================================

ScAccessiblePreviewCell::ScAccessiblePreviewCell( const css::uno::Reference<css::accessibility::XAccessible>& rxParent,
                            ScPreviewShell* pViewShell,
                            /* const */ ScAddress& rCellAddress,
                            sal_Int32 nIndex ) :
    ScAccessibleCellBase( rxParent, ( pViewShell ? &pViewShell->GetDocument() : NULL ), rCellAddress, nIndex ),
    mpViewShell( pViewShell ),
    mpTextHelper(NULL)
{
    if (mpViewShell)
        mpViewShell->AddAccessibilityObject(*this);
}

ScAccessiblePreviewCell::~ScAccessiblePreviewCell()
{
    if (!ScAccessibleContextBase::IsDefunc() && !rBHelper.bInDispose)
    {
        // increment refcount to prevent double call off dtor
        osl_atomic_increment( &m_refCount );
        // call dispose to inform object which have a weak reference to this object
        dispose();
    }
}

void SAL_CALL ScAccessiblePreviewCell::disposing()
{
    SolarMutexGuard aGuard;
    if (mpViewShell)
    {
        mpViewShell->RemoveAccessibilityObject(*this);
        mpViewShell = NULL;
    }

    if (mpTextHelper)
        DELETEZ(mpTextHelper);

    ScAccessibleCellBase::disposing();
}

void ScAccessiblePreviewCell::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if (pSimpleHint && pSimpleHint->GetId() == SC_HINT_ACC_VISAREACHANGED)
    {
        if (mpTextHelper)
            mpTextHelper->UpdateChildren();
    }

    ScAccessibleContextBase::Notify(rBC, rHint);
}

//=====  XAccessibleComponent  ============================================

uno::Reference< XAccessible > SAL_CALL ScAccessiblePreviewCell::getAccessibleAtPoint( const awt::Point& rPoint )
                                throw (uno::RuntimeException, std::exception)
{
    uno::Reference<XAccessible> xRet;
    if (containsPoint(rPoint))
    {
         SolarMutexGuard aGuard;
        IsObjectValid();

        if(!mpTextHelper)
            CreateTextHelper();

        xRet = mpTextHelper->GetAt(rPoint);
    }

    return xRet;
}

void SAL_CALL ScAccessiblePreviewCell::grabFocus() throw (uno::RuntimeException, std::exception)
{
     SolarMutexGuard aGuard;
    IsObjectValid();
    if (getAccessibleParent().is())
    {
        uno::Reference<XAccessibleComponent> xAccessibleComponent(getAccessibleParent()->getAccessibleContext(), uno::UNO_QUERY);
        if (xAccessibleComponent.is())
            xAccessibleComponent->grabFocus();
    }
}

//=====  XAccessibleContext  ==============================================

sal_Int32 SAL_CALL ScAccessiblePreviewCell::getAccessibleChildCount() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    if (!mpTextHelper)
        CreateTextHelper();
    return mpTextHelper->GetChildCount();
}

uno::Reference< XAccessible > SAL_CALL ScAccessiblePreviewCell::getAccessibleChild(sal_Int32 nIndex)
                            throw (uno::RuntimeException, lang::IndexOutOfBoundsException, std::exception)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    if (!mpTextHelper)
        CreateTextHelper();
    return mpTextHelper->GetChild(nIndex);
}

uno::Reference<XAccessibleStateSet> SAL_CALL ScAccessiblePreviewCell::getAccessibleStateSet()
                            throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    uno::Reference<XAccessibleStateSet> xParentStates;
    if (getAccessibleParent().is())
    {
        uno::Reference<XAccessibleContext> xParentContext = getAccessibleParent()->getAccessibleContext();
        xParentStates = xParentContext->getAccessibleStateSet();
    }
    utl::AccessibleStateSetHelper* pStateSet = new utl::AccessibleStateSetHelper();
    if (IsDefunc(xParentStates))
        pStateSet->AddState(AccessibleStateType::DEFUNC);
    else
    {
        pStateSet->AddState(AccessibleStateType::ENABLED);
        pStateSet->AddState(AccessibleStateType::MULTI_LINE);
        if (IsOpaque(xParentStates))
            pStateSet->AddState(AccessibleStateType::OPAQUE);
        if (isShowing())
            pStateSet->AddState(AccessibleStateType::SHOWING);
        pStateSet->AddState(AccessibleStateType::TRANSIENT);
        if (isVisible())
            pStateSet->AddState(AccessibleStateType::VISIBLE);
        // MANAGES_DESCENDANTS (for paragraphs)
        pStateSet->AddState(AccessibleStateType::MANAGES_DESCENDANTS);
    }
    return pStateSet;
}

//=====  XServiceInfo  ====================================================

OUString SAL_CALL ScAccessiblePreviewCell::getImplementationName() throw(uno::RuntimeException, std::exception)
{
    return OUString("ScAccessiblePreviewCell");
}

uno::Sequence<OUString> SAL_CALL ScAccessiblePreviewCell::getSupportedServiceNames()
                                                    throw(uno::RuntimeException, std::exception)
{
    uno::Sequence< OUString > aSequence = ScAccessibleContextBase::getSupportedServiceNames();
    sal_Int32 nOldSize(aSequence.getLength());
    aSequence.realloc(nOldSize + 1);

    aSequence[nOldSize] = "com.sun.star.table.AccessibleCellView";

    return aSequence;
}

//=====  XTypeProvider  =======================================================

uno::Sequence<sal_Int8> SAL_CALL
    ScAccessiblePreviewCell::getImplementationId()
    throw (uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

//====  internal  =========================================================

Rectangle ScAccessiblePreviewCell::GetBoundingBoxOnScreen() const throw (uno::RuntimeException, std::exception)
{
    Rectangle aCellRect;
    if (mpViewShell)
    {
        mpViewShell->GetLocationData().GetCellPosition( maCellAddress, aCellRect );
        vcl::Window* pWindow = mpViewShell->GetWindow();
        if (pWindow)
        {
            Rectangle aRect = pWindow->GetWindowExtentsRelative(NULL);
            aCellRect.setX(aCellRect.getX() + aRect.getX());
            aCellRect.setY(aCellRect.getY() + aRect.getY());
        }
    }
    return aCellRect;
}

Rectangle ScAccessiblePreviewCell::GetBoundingBox() const throw (uno::RuntimeException, std::exception)
{
    Rectangle aCellRect;
    if (mpViewShell)
    {
        mpViewShell->GetLocationData().GetCellPosition( maCellAddress, aCellRect );
        uno::Reference<XAccessible> xAccParent = const_cast<ScAccessiblePreviewCell*>(this)->getAccessibleParent();
        if (xAccParent.is())
        {
            uno::Reference<XAccessibleContext> xAccParentContext = xAccParent->getAccessibleContext();
            uno::Reference<XAccessibleComponent> xAccParentComp (xAccParentContext, uno::UNO_QUERY);
            if (xAccParentComp.is())
            {
                Rectangle aParentRect (VCLRectangle(xAccParentComp->getBounds()));
                aCellRect.setX(aCellRect.getX() - aParentRect.getX());
                aCellRect.setY(aCellRect.getY() - aParentRect.getY());
            }
        }
    }
    return aCellRect;
}

bool ScAccessiblePreviewCell::IsDefunc(
    const uno::Reference<XAccessibleStateSet>& rxParentStates)
{
    return ScAccessibleContextBase::IsDefunc() || (mpDoc == NULL) || (mpViewShell == NULL) || !getAccessibleParent().is() ||
         (rxParentStates.is() && rxParentStates->contains(AccessibleStateType::DEFUNC));
}

bool ScAccessiblePreviewCell::IsEditable(
    const uno::Reference<XAccessibleStateSet>& /* rxParentStates */)
{
    return false;
}

bool ScAccessiblePreviewCell::IsOpaque(
    const uno::Reference<XAccessibleStateSet>& /* rxParentStates */)
{
    // test whether there is a background color
    //! could be moved to ScAccessibleCellBase

    bool bOpaque(true);
    if (mpDoc)
    {
        const SvxBrushItem* pItem = static_cast<const SvxBrushItem*>(mpDoc->GetAttr(
            maCellAddress.Col(), maCellAddress.Row(), maCellAddress.Tab(), ATTR_BACKGROUND));
        if (pItem)
            bOpaque = pItem->GetColor() != COL_TRANSPARENT;
    }
    return bOpaque;
}

void ScAccessiblePreviewCell::CreateTextHelper()
{
    if (!mpTextHelper)
    {
        ::std::unique_ptr < ScAccessibleTextData > pAccessiblePreviewCellTextData
            (new ScAccessiblePreviewCellTextData(mpViewShell, maCellAddress));
        ::std::unique_ptr< SvxEditSource > pEditSource (new ScAccessibilityEditSource(std::move(pAccessiblePreviewCellTextData)));

        mpTextHelper = new ::accessibility::AccessibleTextHelper( std::move(pEditSource) );
        mpTextHelper->SetEventSource( this );

        // paragraphs in preview are transient
        ::accessibility::AccessibleTextHelper::VectorOfStates aChildStates;
        aChildStates.push_back( AccessibleStateType::TRANSIENT );
        mpTextHelper->SetAdditionalChildStates( aChildStates );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
