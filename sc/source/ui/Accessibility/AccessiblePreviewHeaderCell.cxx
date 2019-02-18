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

#include <tools/gen.hxx>
#include <AccessibleText.hxx>
#include <editsrc.hxx>
#include <svx/AccessibleTextHelper.hxx>
#include <AccessiblePreviewHeaderCell.hxx>
#include <prevwsh.hxx>
#include <prevloc.hxx>
#include <strings.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>

#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <svl/hint.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <comphelper/sequence.hxx>
#include <toolkit/helper/convert.hxx>

#ifdef indices
#undef indices
#endif

#ifdef extents
#undef extents
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

//=====  internal  ============================================================

ScAccessiblePreviewHeaderCell::ScAccessiblePreviewHeaderCell( const css::uno::Reference<css::accessibility::XAccessible>& rxParent,
                            ScPreviewShell* pViewShell,
                            const ScAddress& rCellPos, bool bIsColHdr, bool bIsRowHdr,
                            sal_Int32 nIndex ) :
    ScAccessibleContextBase( rxParent, AccessibleRole::TABLE_CELL ),
    mpViewShell( pViewShell ),
    mpTextHelper( nullptr ),
    mnIndex( nIndex ),
    maCellPos( rCellPos ),
    mbColumnHeader( bIsColHdr ),
    mbRowHeader( bIsRowHdr )
{
    if (mpViewShell)
        mpViewShell->AddAccessibilityObject(*this);
}

ScAccessiblePreviewHeaderCell::~ScAccessiblePreviewHeaderCell()
{
    if (mpViewShell)
        mpViewShell->RemoveAccessibilityObject(*this);
}

void SAL_CALL ScAccessiblePreviewHeaderCell::disposing()
{
    SolarMutexGuard aGuard;
    if (mpViewShell)
    {
        mpViewShell->RemoveAccessibilityObject(*this);
        mpViewShell = nullptr;
    }

    mpTableInfo.reset();

    ScAccessibleContextBase::disposing();
}

//=====  SfxListener  =====================================================

void ScAccessiblePreviewHeaderCell::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    const SfxHintId nId = rHint.GetId();
    if (nId == SfxHintId::ScAccVisAreaChanged)
    {
        if (mpTextHelper)
            mpTextHelper->UpdateChildren();
    }
    else if ( nId == SfxHintId::DataChanged )
    {
        //  column / row layout may change with any document change,
        //  so it must be invalidated
        mpTableInfo.reset();
    }

    ScAccessibleContextBase::Notify(rBC, rHint);
}

//=====  XInterface  =====================================================

uno::Any SAL_CALL ScAccessiblePreviewHeaderCell::queryInterface( uno::Type const & rType )
{
    uno::Any aAny (ScAccessiblePreviewHeaderCellImpl::queryInterface(rType));
    return aAny.hasValue() ? aAny : ScAccessibleContextBase::queryInterface(rType);
}

void SAL_CALL ScAccessiblePreviewHeaderCell::acquire()
    throw ()
{
    ScAccessibleContextBase::acquire();
}

void SAL_CALL ScAccessiblePreviewHeaderCell::release()
    throw ()
{
    ScAccessibleContextBase::release();
}

//=====  XAccessibleValue  ================================================

uno::Any SAL_CALL ScAccessiblePreviewHeaderCell::getCurrentValue()
{
    SolarMutexGuard aGuard;
    IsObjectValid();

    double fValue(0.0);
    if (mbColumnHeader)
        fValue = maCellPos.Col();
    else
        fValue = maCellPos.Row();

    return uno::Any(fValue);
}

sal_Bool SAL_CALL ScAccessiblePreviewHeaderCell::setCurrentValue( const uno::Any& /* aNumber */ )
{
    //  it is not possible to set a value
    return false;
}

uno::Any SAL_CALL ScAccessiblePreviewHeaderCell::getMaximumValue()
{
    SolarMutexGuard aGuard;
    IsObjectValid();

    double fValue(0.0);
    if (mbColumnHeader)
        fValue = MAXCOL;
    else
        fValue = MAXROW;
    return uno::Any(fValue);
}

uno::Any SAL_CALL ScAccessiblePreviewHeaderCell::getMinimumValue()
{
    return uno::Any(0.0);
}

//=====  XAccessibleComponent  ============================================

uno::Reference< XAccessible > SAL_CALL ScAccessiblePreviewHeaderCell::getAccessibleAtPoint( const awt::Point& rPoint )
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

void SAL_CALL ScAccessiblePreviewHeaderCell::grabFocus()
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

sal_Int32 SAL_CALL ScAccessiblePreviewHeaderCell::getAccessibleChildCount()
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    if (!mpTextHelper)
        CreateTextHelper();
    return mpTextHelper->GetChildCount();
}

uno::Reference< XAccessible > SAL_CALL ScAccessiblePreviewHeaderCell::getAccessibleChild(sal_Int32 nIndex)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    if (!mpTextHelper)
        CreateTextHelper();
    return mpTextHelper->GetChild(nIndex);
}

sal_Int32 SAL_CALL ScAccessiblePreviewHeaderCell::getAccessibleIndexInParent()
{
    return mnIndex;
}

uno::Reference<XAccessibleStateSet> SAL_CALL ScAccessiblePreviewHeaderCell::getAccessibleStateSet()
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
        if (isShowing())
            pStateSet->AddState(AccessibleStateType::SHOWING);
        pStateSet->AddState(AccessibleStateType::TRANSIENT);
        if (isVisible())
            pStateSet->AddState(AccessibleStateType::VISIBLE);
    }
    return pStateSet;
}

//=====  XServiceInfo  ====================================================

OUString SAL_CALL ScAccessiblePreviewHeaderCell::getImplementationName()
{
    return OUString("ScAccessiblePreviewHeaderCell");
}

uno::Sequence<OUString> SAL_CALL ScAccessiblePreviewHeaderCell::getSupportedServiceNames()
{
    uno::Sequence< OUString > aSequence = ScAccessibleContextBase::getSupportedServiceNames();
    sal_Int32 nOldSize(aSequence.getLength());
    aSequence.realloc(nOldSize + 1);

    aSequence[nOldSize] = "com.sun.star.table.AccessibleCellView";

    return aSequence;
}

//=====  XTypeProvider  =======================================================

uno::Sequence< uno::Type > SAL_CALL ScAccessiblePreviewHeaderCell::getTypes()
{
    return comphelper::concatSequences(ScAccessiblePreviewHeaderCellImpl::getTypes(), ScAccessibleContextBase::getTypes());
}

uno::Sequence<sal_Int8> SAL_CALL
    ScAccessiblePreviewHeaderCell::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

//====  internal  =========================================================

tools::Rectangle ScAccessiblePreviewHeaderCell::GetBoundingBoxOnScreen() const
{
    tools::Rectangle aCellRect;

    FillTableInfo();

    if (mpTableInfo)
    {
        const ScPreviewColRowInfo& rColInfo = mpTableInfo->GetColInfo()[maCellPos.Col()];
        const ScPreviewColRowInfo& rRowInfo = mpTableInfo->GetRowInfo()[maCellPos.Row()];

        aCellRect = tools::Rectangle( rColInfo.nPixelStart, rRowInfo.nPixelStart, rColInfo.nPixelEnd, rRowInfo.nPixelEnd );
    }

    if (mpViewShell)
    {
        vcl::Window* pWindow = mpViewShell->GetWindow();
        if (pWindow)
        {
            tools::Rectangle aRect = pWindow->GetWindowExtentsRelative(nullptr);
            aCellRect.setX(aCellRect.getX() + aRect.getX());
            aCellRect.setY(aCellRect.getY() + aRect.getY());
        }
    }
    return aCellRect;
}

tools::Rectangle ScAccessiblePreviewHeaderCell::GetBoundingBox() const
{
    FillTableInfo();

    if (mpTableInfo)
    {
        const ScPreviewColRowInfo& rColInfo = mpTableInfo->GetColInfo()[maCellPos.Col()];
        const ScPreviewColRowInfo& rRowInfo = mpTableInfo->GetRowInfo()[maCellPos.Row()];

        tools::Rectangle aCellRect( rColInfo.nPixelStart, rRowInfo.nPixelStart, rColInfo.nPixelEnd, rRowInfo.nPixelEnd );
        uno::Reference<XAccessible> xAccParent = const_cast<ScAccessiblePreviewHeaderCell*>(this)->getAccessibleParent();
        if (xAccParent.is())
        {
            uno::Reference<XAccessibleContext> xAccParentContext = xAccParent->getAccessibleContext();
            uno::Reference<XAccessibleComponent> xAccParentComp (xAccParentContext, uno::UNO_QUERY);
            if (xAccParentComp.is())
            {
                tools::Rectangle aParentRect (VCLRectangle(xAccParentComp->getBounds()));
                aCellRect.setX(aCellRect.getX() - aParentRect.getX());
                aCellRect.setY(aCellRect.getY() - aParentRect.getY());
            }
        }
        return aCellRect;
    }
    return tools::Rectangle();
}

OUString ScAccessiblePreviewHeaderCell::createAccessibleDescription()
{
    OUString sDescription = STR_ACC_HEADERCELL_DESCR;
    return sDescription;
}

OUString ScAccessiblePreviewHeaderCell::createAccessibleName()
{
    OUString sName = STR_ACC_HEADERCELL_NAME;

    if ( mbColumnHeader )
    {
        if ( mbRowHeader )
        {
            //! name for corner cell?

//          sName = "Column/Row Header";
        }
        else
        {
            // name of column header
            sName += ScColToAlpha( maCellPos.Col() );
        }
    }
    else
    {
        // name of row header
        sName += OUString::number(  ( maCellPos.Row() + 1 ) );
    }

    return sName;
}

bool ScAccessiblePreviewHeaderCell::IsDefunc( const uno::Reference<XAccessibleStateSet>& rxParentStates )
{
    return ScAccessibleContextBase::IsDefunc() || (mpViewShell == nullptr) || !getAccessibleParent().is() ||
        (rxParentStates.is() && rxParentStates->contains(AccessibleStateType::DEFUNC));
}

void ScAccessiblePreviewHeaderCell::CreateTextHelper()
{
    if (!mpTextHelper)
    {
        mpTextHelper = new ::accessibility::AccessibleTextHelper(
            std::make_unique<ScAccessibilityEditSource>(
                std::make_unique<ScAccessiblePreviewHeaderCellTextData>(
                    mpViewShell, getAccessibleName(), maCellPos,
                    mbColumnHeader, mbRowHeader)));
        mpTextHelper->SetEventSource(this);
    }
}

void ScAccessiblePreviewHeaderCell::FillTableInfo() const
{
    if ( mpViewShell && !mpTableInfo )
    {
        Size aOutputSize;
        vcl::Window* pWindow = mpViewShell->GetWindow();
        if ( pWindow )
            aOutputSize = pWindow->GetOutputSizePixel();
        tools::Rectangle aVisRect( Point(), aOutputSize );

        mpTableInfo.reset( new ScPreviewTableInfo );
        mpViewShell->GetLocationData().GetTableInfo( aVisRect, *mpTableInfo );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
