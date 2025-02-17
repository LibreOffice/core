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

#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/unohelp.hxx>
#include <sal/log.hxx>
#include <tools/debug.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <svtools/valueset.hxx>
#include "valueimp.hxx"
#include <comphelper/servicehelper.hxx>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>

using namespace ::com::sun::star;


ValueSetItem::ValueSetItem( ValueSet& rParent )
    : mrParent(rParent)
    , mpData(nullptr)
    , mxAcc()
    , mnId(0)
    , meType(VALUESETITEM_NONE)
    , mbVisible(true)
{
}


ValueSetItem::~ValueSetItem()
{
    if( mxAcc.is() )
    {
        mxAcc->ValueSetItemDestroyed();
    }
}

const rtl::Reference<ValueItemAcc>& ValueSetItem::GetAccessible(bool bCreate)
{
    if (!mxAcc.is() && bCreate)
        mxAcc = new ValueItemAcc(this);

    return mxAcc;
}

ValueItemAcc::ValueItemAcc(ValueSetItem* pValueSetItem)
    : mpValueSetItem(pValueSetItem)
{
}

ValueItemAcc::~ValueItemAcc()
{
}

void ValueItemAcc::ValueSetItemDestroyed()
{
    const SolarMutexGuard aSolarGuard;
    mpValueSetItem = nullptr;
}

uno::Reference< accessibility::XAccessibleContext > SAL_CALL ValueItemAcc::getAccessibleContext()
{
    return this;
}


sal_Int64 SAL_CALL ValueItemAcc::getAccessibleChildCount()
{
    return 0;
}


uno::Reference< accessibility::XAccessible > SAL_CALL ValueItemAcc::getAccessibleChild( sal_Int64 )
{
    throw lang::IndexOutOfBoundsException();
}


uno::Reference< accessibility::XAccessible > SAL_CALL ValueItemAcc::getAccessibleParent()
{
    const SolarMutexGuard aSolarGuard;
    uno::Reference< accessibility::XAccessible >    xRet;

    if (mpValueSetItem)
        xRet = mpValueSetItem->mrParent.mxAccessible;

    return xRet;
}


sal_Int64 SAL_CALL ValueItemAcc::getAccessibleIndexInParent()
{
    const SolarMutexGuard aSolarGuard;
    // The index defaults to -1 to indicate the child does not belong to its
    // parent.
    sal_Int64 nIndexInParent = -1;

    if (mpValueSetItem)
    {
        bool bDone = false;

        sal_uInt16 nCount = mpValueSetItem->mrParent.ImplGetVisibleItemCount();
        ValueSetItem* pItem;
        for (sal_uInt16 i=0; i<nCount && !bDone; i++)
        {
            // Guard the retrieval of the i-th child with a try/catch block
            // just in case the number of children changes in the meantime.
            try
            {
                pItem = mpValueSetItem->mrParent.ImplGetItem(i);
            }
            catch (const lang::IndexOutOfBoundsException&)
            {
                pItem = nullptr;
            }

            if (pItem && pItem == mpValueSetItem)
            {
                nIndexInParent = i;
                bDone = true;
            }
        }
    }

    //if this valueset contain a none field(common value is default), then we should increase the real index and set the noitem index value equal 0.
    if (mpValueSetItem && ((mpValueSetItem->mrParent.GetStyle() & WB_NONEFIELD) != 0))
    {
        ValueSetItem* pFirstItem = mpValueSetItem->mrParent.ImplGetItem(VALUESET_ITEM_NONEITEM);
        if( pFirstItem && pFirstItem == mpValueSetItem)
            nIndexInParent = 0;
        else
            nIndexInParent++;
    }
    return nIndexInParent;
}


sal_Int16 SAL_CALL ValueItemAcc::getAccessibleRole()
{
    return accessibility::AccessibleRole::LIST_ITEM;
}


OUString SAL_CALL ValueItemAcc::getAccessibleDescription()
{
    return OUString();
}


OUString SAL_CALL ValueItemAcc::getAccessibleName()
{
    const SolarMutexGuard aSolarGuard;

    if (mpValueSetItem)
    {
        if (mpValueSetItem->maText.isEmpty())
            return "Item " + OUString::number(static_cast<sal_Int32>(mpValueSetItem->mnId));
        else
            return mpValueSetItem->maText;
    }

    return OUString();
}


uno::Reference< accessibility::XAccessibleRelationSet > SAL_CALL ValueItemAcc::getAccessibleRelationSet()
{
    return uno::Reference< accessibility::XAccessibleRelationSet >();
}


sal_Int64 SAL_CALL ValueItemAcc::getAccessibleStateSet()
{
    const SolarMutexGuard aSolarGuard;
    sal_Int64 nStateSet = 0;

    if (mpValueSetItem)
    {
        nStateSet |= accessibility::AccessibleStateType::ENABLED;
        nStateSet |= accessibility::AccessibleStateType::SENSITIVE;
        nStateSet |= accessibility::AccessibleStateType::SHOWING;
        nStateSet |= accessibility::AccessibleStateType::VISIBLE;
        nStateSet |= accessibility::AccessibleStateType::TRANSIENT;
        nStateSet |= accessibility::AccessibleStateType::SELECTABLE;
        nStateSet |= accessibility::AccessibleStateType::FOCUSABLE;

        if (mpValueSetItem->mrParent.GetSelectedItemId() == mpValueSetItem->mnId)
        {

            nStateSet |= accessibility::AccessibleStateType::SELECTED;
            if (mpValueSetItem->mrParent.HasChildFocus())
                nStateSet |= accessibility::AccessibleStateType::FOCUSED;
        }
    }

    return nStateSet;
}


lang::Locale SAL_CALL ValueItemAcc::getLocale()
{
    const SolarMutexGuard aSolarGuard;
    uno::Reference< accessibility::XAccessible >    xParent( getAccessibleParent() );
    lang::Locale                                    aRet( u""_ustr, u""_ustr, u""_ustr );

    if( xParent.is() )
    {
        uno::Reference< accessibility::XAccessibleContext > xParentContext( xParent->getAccessibleContext() );

        if( xParentContext.is() )
            aRet = xParentContext->getLocale();
    }

    return aRet;
}

uno::Reference< accessibility::XAccessible > SAL_CALL ValueItemAcc::getAccessibleAtPoint( const awt::Point& )
{
    uno::Reference< accessibility::XAccessible > xRet;
    return xRet;
}

awt::Rectangle ValueItemAcc::implGetBounds()
{
    awt::Rectangle      aRet;

    if (mpValueSetItem)
    {
        tools::Rectangle aRect(mpValueSetItem->mrParent.GetItemRect(mpValueSetItem->mnId));
        tools::Rectangle aParentRect(Point(), mpValueSetItem->mrParent.GetOutputSizePixel());

        aRect.Intersection( aParentRect );

        aRet.X = aRect.Left();
        aRet.Y = aRect.Top();
        aRet.Width = aRect.GetWidth();
        aRet.Height = aRect.GetHeight();
    }

    return aRet;
}

void SAL_CALL ValueItemAcc::grabFocus()
{
    // nothing to do
}

sal_Int32 SAL_CALL ValueItemAcc::getForeground(  )
{
    Color nColor = Application::GetSettings().GetStyleSettings().GetWindowTextColor();
    return static_cast<sal_Int32>(nColor);
}

sal_Int32 SAL_CALL ValueItemAcc::getBackground(  )
{
    Color nColor;
    if (mpValueSetItem && mpValueSetItem->meType == VALUESETITEM_COLOR)
        nColor = mpValueSetItem->maColor;
    else
        nColor = Application::GetSettings().GetStyleSettings().GetWindowColor();
    return static_cast<sal_Int32>(nColor);
}

void ValueItemAcc::FireAccessibleEvent( short nEventId, const uno::Any& rOldValue, const uno::Any& rNewValue )
{
    NotifyAccessibleEvent(nEventId, rOldValue, rNewValue);
}

ValueSetAcc::ValueSetAcc(ValueSet* pValueSet) :
    mpValueSet(pValueSet),
    mbIsFocused(false)
{
}


ValueSetAcc::~ValueSetAcc()
{
}


void ValueSetAcc::FireAccessibleEvent( short nEventId, const uno::Any& rOldValue, const uno::Any& rNewValue )
{
    NotifyAccessibleEvent(nEventId, rOldValue, rNewValue);
}

bool ValueSetAcc::HasAccessibleListeners() const
{
    return comphelper::OAccessibleComponentHelper::hasAccessibleListeners();
}

void ValueSetAcc::GetFocus()
{
    mbIsFocused = true;

    // Broadcast the state change.
    css::uno::Any aOldState, aNewState;
    aNewState <<= css::accessibility::AccessibleStateType::FOCUSED;
    FireAccessibleEvent(
        css::accessibility::AccessibleEventId::STATE_CHANGED,
        aOldState, aNewState);
}


void ValueSetAcc::LoseFocus()
{
    mbIsFocused = false;

    // Broadcast the state change.
    css::uno::Any aOldState, aNewState;
    aOldState <<= css::accessibility::AccessibleStateType::FOCUSED;
    FireAccessibleEvent(
        css::accessibility::AccessibleEventId::STATE_CHANGED,
        aOldState, aNewState);
}


uno::Reference< accessibility::XAccessibleContext > SAL_CALL ValueSetAcc::getAccessibleContext()
{
    // still allow retrieving a11y context when not disposed yet, but ValueSet is unset
    ThrowIfDisposed(false);
    return this;
}


sal_Int64 SAL_CALL ValueSetAcc::getAccessibleChildCount()
{
    const SolarMutexGuard aSolarGuard;
    ThrowIfDisposed();

    sal_Int64 nCount = mpValueSet->ImplGetVisibleItemCount();
    if (HasNoneField())
        nCount += 1;
    return nCount;
}


uno::Reference< accessibility::XAccessible > SAL_CALL ValueSetAcc::getAccessibleChild( sal_Int64 i )
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;

    if (i < 0 || i >= getAccessibleChildCount())
        throw lang::IndexOutOfBoundsException();

    ValueSetItem* pItem = getItem (sal::static_int_cast< sal_uInt16 >(i));

    if( !pItem )
        throw lang::IndexOutOfBoundsException();

    rtl::Reference< ValueItemAcc > xRet = pItem->GetAccessible();
    return xRet;
}

uno::Reference< accessibility::XAccessible > SAL_CALL ValueSetAcc::getAccessibleParent()
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    return mpValueSet->GetDrawingArea()->get_accessible_parent();
}

sal_Int64 SAL_CALL ValueSetAcc::getAccessibleIndexInParent()
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;

    // -1 for child not found/no parent (according to specification)
    sal_Int64 nRet = -1;

    uno::Reference<accessibility::XAccessible> xParent(getAccessibleParent());
    if (!xParent)
        return nRet;

    try
    {
        uno::Reference<accessibility::XAccessibleContext> xParentContext(xParent->getAccessibleContext());

        //  iterate over parent's children and search for this object
        if ( xParentContext.is() )
        {
            sal_Int64 nChildCount = xParentContext->getAccessibleChildCount();
            for ( sal_Int64 nChild = 0; ( nChild < nChildCount ) && ( -1 == nRet ); ++nChild )
            {
                uno::Reference<XAccessible> xChild(xParentContext->getAccessibleChild(nChild));
                if ( xChild.get() == this )
                    nRet = nChild;
            }
        }
    }
    catch (const uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION( "svtools", "ValueSetAcc::getAccessibleIndexInParent" );
    }

    return nRet;
}

sal_Int16 SAL_CALL ValueSetAcc::getAccessibleRole()
{
    ThrowIfDisposed();
    return accessibility::AccessibleRole::LIST;
}


OUString SAL_CALL ValueSetAcc::getAccessibleDescription()
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    OUString              aRet;

    if (mpValueSet)
    {
        aRet = mpValueSet->GetAccessibleDescription();
    }

    return aRet;
}


OUString SAL_CALL ValueSetAcc::getAccessibleName()
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    OUString              aRet;

    if (mpValueSet)
    {
        aRet = mpValueSet->GetAccessibleName();
    }

    return aRet;
}

uno::Reference< accessibility::XAccessibleRelationSet > SAL_CALL ValueSetAcc::getAccessibleRelationSet()
{
    ThrowIfDisposed();
    SolarMutexGuard g;
    return mpValueSet->GetDrawingArea()->get_accessible_relation_set();
}

sal_Int64 SAL_CALL ValueSetAcc::getAccessibleStateSet()
{
    ThrowIfDisposed();
    sal_Int64 nStateSet = 0;

    // Set some states.
    nStateSet |= accessibility::AccessibleStateType::ENABLED;
    nStateSet |= accessibility::AccessibleStateType::SENSITIVE;
    nStateSet |= accessibility::AccessibleStateType::SHOWING;
    nStateSet |= accessibility::AccessibleStateType::VISIBLE;
    nStateSet |= accessibility::AccessibleStateType::MANAGES_DESCENDANTS;
    nStateSet |= accessibility::AccessibleStateType::FOCUSABLE;
    if (mbIsFocused)
        nStateSet |= accessibility::AccessibleStateType::FOCUSED;

    return nStateSet;
}


lang::Locale SAL_CALL ValueSetAcc::getLocale()
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    uno::Reference< accessibility::XAccessible >    xParent( getAccessibleParent() );
    lang::Locale                                    aRet( u""_ustr, u""_ustr, u""_ustr );

    if( xParent.is() )
    {
        uno::Reference< accessibility::XAccessibleContext > xParentContext( xParent->getAccessibleContext() );

        if( xParentContext.is() )
            aRet = xParentContext->getLocale ();
    }

    return aRet;
}

uno::Reference< accessibility::XAccessible > SAL_CALL ValueSetAcc::getAccessibleAtPoint( const awt::Point& aPoint )
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    const sal_uInt16 nItemId = mpValueSet->GetItemId(Point(aPoint.X, aPoint.Y));
    uno::Reference< accessibility::XAccessible >    xRet;

    if ( nItemId )
    {
        const size_t nItemPos = mpValueSet->GetItemPos(nItemId);

        if( VALUESET_ITEM_NONEITEM != nItemPos )
        {
            ValueSetItem* const pItem = mpValueSet->mItemList[nItemPos].get();
            xRet = pItem->GetAccessible();
        }
    }

    return xRet;
}

awt::Rectangle ValueSetAcc::implGetBounds()
{
    weld::DrawingArea* pDrawingArea = mpValueSet->GetDrawingArea();
    if (!pDrawingArea)
        return css::awt::Rectangle();

    const AbsoluteScreenPixelPoint aOutPos = pDrawingArea->get_accessible_location_on_screen();
    const Size aOutSize(mpValueSet->GetOutputSizePixel());
    tools::Rectangle aBounds(aOutPos, aOutSize);

    // subtract absolute parent pos to get relative pos in parent
    uno::Reference<accessibility::XAccessible> xParent(getAccessibleParent());
    if (xParent)
    {
        uno::Reference<accessibility::XAccessibleContext> xParentContext(xParent->getAccessibleContext());
        uno::Reference<accessibility::XAccessibleComponent> xParentComponent(xParent->getAccessibleContext(), css::uno::UNO_QUERY);
        if (xParentComponent.is())
        {
            awt::Point aParentPos = xParentComponent->getLocationOnScreen();
            aBounds.Move(-aParentPos.X, - aParentPos.Y);
        }
    }

    return vcl::unohelper::ConvertToAWTRect(aBounds);
}

void SAL_CALL ValueSetAcc::grabFocus()
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    mpValueSet->GrabFocus();
}

sal_Int32 SAL_CALL ValueSetAcc::getForeground(  )
{
    ThrowIfDisposed();
    Color nColor = Application::GetSettings().GetStyleSettings().GetWindowTextColor();
    return static_cast<sal_Int32>(nColor);
}

sal_Int32 SAL_CALL ValueSetAcc::getBackground(  )
{
    ThrowIfDisposed();
    Color nColor = Application::GetSettings().GetStyleSettings().GetWindowColor();
    return static_cast<sal_Int32>(nColor);
}

void SAL_CALL ValueSetAcc::selectAccessibleChild( sal_Int64 nChildIndex )
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;

    if (nChildIndex < 0 || nChildIndex >= getAccessibleChildCount())
        throw lang::IndexOutOfBoundsException();

    ValueSetItem* pItem = getItem (sal::static_int_cast< sal_uInt16 >(nChildIndex));

    if(pItem == nullptr)
        throw lang::IndexOutOfBoundsException();

    mpValueSet->SelectItem(pItem->mnId);
}


sal_Bool SAL_CALL ValueSetAcc::isAccessibleChildSelected( sal_Int64 nChildIndex )
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;

    if (nChildIndex < 0 || nChildIndex >= getAccessibleChildCount())
        throw lang::IndexOutOfBoundsException();

    ValueSetItem* pItem = getItem (sal::static_int_cast< sal_uInt16 >(nChildIndex));

    if (pItem == nullptr)
        throw lang::IndexOutOfBoundsException();

    bool bRet = mpValueSet->IsItemSelected(pItem->mnId);
    return bRet;
}


void SAL_CALL ValueSetAcc::clearAccessibleSelection()
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    mpValueSet->SetNoSelection();
}


void SAL_CALL ValueSetAcc::selectAllAccessibleChildren()
{
    ThrowIfDisposed();
    // unsupported due to single selection only
}


sal_Int64 SAL_CALL ValueSetAcc::getSelectedAccessibleChildCount()
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    sal_Int64 nRet = 0;

    for( sal_uInt16 i = 0, nCount = getItemCount(); i < nCount; i++ )
    {
        ValueSetItem* pItem = getItem (i);

        if (pItem && mpValueSet->IsItemSelected(pItem->mnId))
            ++nRet;
    }

    return nRet;
}


uno::Reference< accessibility::XAccessible > SAL_CALL ValueSetAcc::getSelectedAccessibleChild( sal_Int64 nSelectedChildIndex )
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    uno::Reference< accessibility::XAccessible >    xRet;

    for( sal_uInt16 i = 0, nCount = getItemCount(), nSel = 0; ( i < nCount ) && !xRet.is(); i++ )
    {
        ValueSetItem* pItem = getItem(i);

        if (pItem && mpValueSet->IsItemSelected(pItem->mnId) && (nSelectedChildIndex == static_cast< sal_Int64 >(nSel++)))
            xRet = pItem->GetAccessible();
    }

    return xRet;
}


void SAL_CALL ValueSetAcc::deselectAccessibleChild( sal_Int64 nChildIndex )
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;

    if (nChildIndex < 0 || nChildIndex >= getAccessibleChildCount())
        throw lang::IndexOutOfBoundsException();

    // Because of the single selection we can reset the whole selection when
    // the specified child is currently selected.
    if (isAccessibleChildSelected(nChildIndex))
        mpValueSet->SetNoSelection();
}

void ValueSetAcc::Invalidate()
{
    mpValueSet = nullptr;
}

sal_uInt16 ValueSetAcc::getItemCount() const
{
    sal_uInt16 nCount = mpValueSet->ImplGetVisibleItemCount();
    // When the None-Item is visible then increase the number of items by
    // one.
    if (HasNoneField())
        nCount += 1;
    return nCount;
}

ValueSetItem* ValueSetAcc::getItem (sal_uInt16 nIndex) const
{
    ValueSetItem* pItem = nullptr;

    if (HasNoneField())
    {
        if (nIndex == 0)
            // When present the first item is the then always visible none field.
            pItem = mpValueSet->ImplGetItem(VALUESET_ITEM_NONEITEM);
        else
            // Shift down the index to compensate for the none field.
            nIndex -= 1;
    }
    if (pItem == nullptr)
        pItem = mpValueSet->ImplGetItem(nIndex);

    return pItem;
}


void ValueSetAcc::ThrowIfDisposed(bool bCheckValueSet)
{
    ensureAlive();

    if (bCheckValueSet && !mpValueSet)
    {
        assert(false && "ValueSetAcc not disposed but mpValueSet  == NULL");
        throw css::uno::RuntimeException(u"ValueSetAcc not disposed but mpValueSet == NULL"_ustr);
    }
}

bool ValueSetAcc::HasNoneField() const
{
    assert(mpValueSet && "ValueSetAcc::HasNoneField called with mpValueSet==NULL");
    return ((mpValueSet->GetStyle() & WB_NONEFIELD) != 0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
