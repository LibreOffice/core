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

#include <scitems.hxx>
#include <tools/gen.hxx>
#include <AccessibleText.hxx>
#include <editsrc.hxx>
#include <AccessiblePreviewCell.hxx>
#include <prevwsh.hxx>
#include <prevloc.hxx>
#include <document.hxx>
#include <svx/AccessibleTextHelper.hxx>
#include <editeng/brushitem.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/helper/convert.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <comphelper/sequence.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

//=====  internal  ============================================================

ScAccessiblePreviewCell::ScAccessiblePreviewCell( const css::uno::Reference<css::accessibility::XAccessible>& rxParent,
                            ScPreviewShell* pViewShell,
                            const ScAddress& rCellAddress,
                            sal_Int32 nIndex ) :
    ScAccessibleCellBase( rxParent, ( pViewShell ? &pViewShell->GetDocument() : nullptr ), rCellAddress, nIndex ),
    mpViewShell( pViewShell )
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
        mpViewShell = nullptr;
    }

    mpTextHelper.reset();

    ScAccessibleCellBase::disposing();
}

void ScAccessiblePreviewCell::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if (rHint.GetId() == SfxHintId::ScAccVisAreaChanged)
    {
        if (mpTextHelper)
            mpTextHelper->UpdateChildren();
    }

    ScAccessibleContextBase::Notify(rBC, rHint);
}

//=====  XAccessibleComponent  ============================================

uno::Reference< XAccessible > SAL_CALL ScAccessiblePreviewCell::getAccessibleAtPoint( const awt::Point& rPoint )
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

void SAL_CALL ScAccessiblePreviewCell::grabFocus()
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

sal_Int64 SAL_CALL ScAccessiblePreviewCell::getAccessibleChildCount()
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    if (!mpTextHelper)
        CreateTextHelper();
    return mpTextHelper->GetChildCount();
}

uno::Reference< XAccessible > SAL_CALL ScAccessiblePreviewCell::getAccessibleChild(sal_Int64 nIndex)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    if (!mpTextHelper)
        CreateTextHelper();
    return mpTextHelper->GetChild(nIndex);
}

sal_Int64 SAL_CALL ScAccessiblePreviewCell::getAccessibleStateSet()
{
    SolarMutexGuard aGuard;

    sal_Int64 nParentStates = 0;
    if (getAccessibleParent().is())
    {
        uno::Reference<XAccessibleContext> xParentContext = getAccessibleParent()->getAccessibleContext();
        nParentStates = xParentContext->getAccessibleStateSet();
    }
    sal_Int64 nStateSet = 0;
    if (IsDefunc(nParentStates))
        nStateSet |= AccessibleStateType::DEFUNC;
    else
    {
        nStateSet |= AccessibleStateType::ENABLED;
        nStateSet |= AccessibleStateType::MULTI_LINE;
        if (IsOpaque())
            nStateSet |= AccessibleStateType::OPAQUE;
        if (isShowing())
            nStateSet |= AccessibleStateType::SHOWING;
        nStateSet |= AccessibleStateType::TRANSIENT;
        if (isVisible())
            nStateSet |= AccessibleStateType::VISIBLE;
        // MANAGES_DESCENDANTS (for paragraphs)
        nStateSet |= AccessibleStateType::MANAGES_DESCENDANTS;
    }
    return nStateSet;
}

//=====  XServiceInfo  ====================================================

OUString SAL_CALL ScAccessiblePreviewCell::getImplementationName()
{
    return u"ScAccessiblePreviewCell"_ustr;
}

uno::Sequence<OUString> SAL_CALL ScAccessiblePreviewCell::getSupportedServiceNames()
{
    const css::uno::Sequence<OUString> vals { u"com.sun.star.table.AccessibleCellView"_ustr };
    return comphelper::concatSequences(ScAccessibleContextBase::getSupportedServiceNames(), vals);
}

//=====  XTypeProvider  =======================================================

uno::Sequence<sal_Int8> SAL_CALL
    ScAccessiblePreviewCell::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

//====  internal  =========================================================

AbsoluteScreenPixelRectangle ScAccessiblePreviewCell::GetBoundingBoxOnScreen() const
{
    tools::Rectangle aCellRect;
    if (mpViewShell)
    {
        mpViewShell->GetLocationData().GetCellPosition( maCellAddress, aCellRect );
        vcl::Window* pWindow = mpViewShell->GetWindow();
        if (pWindow)
        {
            AbsoluteScreenPixelRectangle aRect = pWindow->GetWindowExtentsAbsolute();
            aCellRect.Move(aRect.Left(), aRect.Top());
        }
    }
    return AbsoluteScreenPixelRectangle(aCellRect);
}

tools::Rectangle ScAccessiblePreviewCell::GetBoundingBox() const
{
    tools::Rectangle aCellRect;
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
                tools::Rectangle aParentRect (VCLRectangle(xAccParentComp->getBounds()));
                aCellRect.Move(-aParentRect.Left(), -aParentRect.Top());
            }
        }
    }
    return aCellRect;
}

bool ScAccessiblePreviewCell::IsDefunc(sal_Int64 nParentStates)
{
    return ScAccessibleContextBase::IsDefunc() || (mpDoc == nullptr) || (mpViewShell == nullptr) || !getAccessibleParent().is() ||
         (nParentStates & AccessibleStateType::DEFUNC);
}

bool ScAccessiblePreviewCell::IsEditable(sal_Int64 /* nParentStates */)
{
    return false;
}

bool ScAccessiblePreviewCell::IsOpaque() const
{
    // test whether there is a background color
    //! could be moved to ScAccessibleCellBase

    bool bOpaque(true);
    if (mpDoc)
    {
        const SvxBrushItem* pItem = mpDoc->GetAttr(maCellAddress, ATTR_BACKGROUND);
        if (pItem)
            bOpaque = pItem->GetColor() != COL_TRANSPARENT;
    }
    return bOpaque;
}

void ScAccessiblePreviewCell::CreateTextHelper()
{
    if (mpTextHelper)
        return;

    mpTextHelper.reset( new ::accessibility::AccessibleTextHelper(
        std::make_unique<ScAccessibilityEditSource>(
            std::make_unique<ScAccessiblePreviewCellTextData>(
                mpViewShell, maCellAddress))) );
    mpTextHelper->SetEventSource( this );

    // paragraphs in preview are transient
    mpTextHelper->SetAdditionalChildStates( AccessibleStateType::TRANSIENT );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
