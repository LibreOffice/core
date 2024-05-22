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
#include <comphelper/sequence.hxx>

#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <svl/hint.hxx>
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
        if (mxTextHelper)
            mxTextHelper->UpdateChildren();
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
    noexcept
{
    ScAccessibleContextBase::acquire();
}

void SAL_CALL ScAccessiblePreviewHeaderCell::release()
    noexcept
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
    ScDocument& rDoc = mpViewShell->GetDocument();
    if (mbColumnHeader)
        fValue = rDoc.MaxCol();
    else
        fValue = rDoc.MaxRow();
    return uno::Any(fValue);
}

uno::Any SAL_CALL ScAccessiblePreviewHeaderCell::getMinimumValue()
{
    return uno::Any(0.0);
}

uno::Any SAL_CALL ScAccessiblePreviewHeaderCell::getMinimumIncrement()
{
    // value can't be changed, s. 'setCurrentValue'
    return uno::Any();
}

//=====  XAccessibleComponent  ============================================

uno::Reference< XAccessible > SAL_CALL ScAccessiblePreviewHeaderCell::getAccessibleAtPoint( const awt::Point& rPoint )
{
    uno::Reference<XAccessible> xRet;
    if (containsPoint(rPoint))
    {
        SolarMutexGuard aGuard;
        IsObjectValid();

        if(!mxTextHelper)
            CreateTextHelper();

        xRet = mxTextHelper->GetAt(rPoint);
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

sal_Int64 SAL_CALL ScAccessiblePreviewHeaderCell::getAccessibleChildCount()
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    if (!mxTextHelper)
        CreateTextHelper();
    return mxTextHelper->GetChildCount();
}

uno::Reference< XAccessible > SAL_CALL ScAccessiblePreviewHeaderCell::getAccessibleChild(sal_Int64 nIndex)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    if (!mxTextHelper)
        CreateTextHelper();
    return mxTextHelper->GetChild(nIndex);
}

sal_Int64 SAL_CALL ScAccessiblePreviewHeaderCell::getAccessibleIndexInParent()
{
    return mnIndex;
}

sal_Int64 SAL_CALL ScAccessiblePreviewHeaderCell::getAccessibleStateSet()
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
        if (isShowing())
            nStateSet |= AccessibleStateType::SHOWING;
        nStateSet |= AccessibleStateType::TRANSIENT;
        if (isVisible())
            nStateSet |= AccessibleStateType::VISIBLE;
    }
    return nStateSet;
}

//=====  XServiceInfo  ====================================================

OUString SAL_CALL ScAccessiblePreviewHeaderCell::getImplementationName()
{
    return u"ScAccessiblePreviewHeaderCell"_ustr;
}

uno::Sequence<OUString> SAL_CALL ScAccessiblePreviewHeaderCell::getSupportedServiceNames()
{
    const css::uno::Sequence<OUString> vals { u"com.sun.star.table.AccessibleCellView"_ustr };
    return comphelper::concatSequences(ScAccessibleContextBase::getSupportedServiceNames(), vals);
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

AbsoluteScreenPixelRectangle ScAccessiblePreviewHeaderCell::GetBoundingBoxOnScreen() const
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
            AbsoluteScreenPixelRectangle aRect = pWindow->GetWindowExtentsAbsolute();
            aCellRect.Move(aRect.Left(), aRect.Top());
        }
    }
    return AbsoluteScreenPixelRectangle(aCellRect);
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
                aCellRect.Move(-aParentRect.Left(), -aParentRect.Top());
            }
        }
        return aCellRect;
    }
    return tools::Rectangle();
}

OUString ScAccessiblePreviewHeaderCell::createAccessibleDescription()
{
    return STR_ACC_HEADERCELL_DESCR;
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
        sName += OUString::number(  maCellPos.Row() + 1 );
    }

    return sName;
}

bool ScAccessiblePreviewHeaderCell::IsDefunc( sal_Int64 nParentStates )
{
    return ScAccessibleContextBase::IsDefunc() || (mpViewShell == nullptr) || !getAccessibleParent().is() ||
        (nParentStates & AccessibleStateType::DEFUNC);
}

void ScAccessiblePreviewHeaderCell::CreateTextHelper()
{
    if (!mxTextHelper)
    {
        mxTextHelper.reset( new ::accessibility::AccessibleTextHelper(
            std::make_unique<ScAccessibilityEditSource>(
                std::make_unique<ScAccessiblePreviewHeaderCellTextData>(
                    mpViewShell, getAccessibleName(), maCellPos,
                    mbColumnHeader, mbRowHeader))) );
        mxTextHelper->SetEventSource(this);
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
