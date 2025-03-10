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

#include "thumbnailviewacc.hxx"
#include "thumbnailviewitemacc.hxx"

#include <comphelper/servicehelper.hxx>
#include <sfx2/thumbnailview.hxx>
#include <sfx2/thumbnailviewitem.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <sal/log.hxx>
#include <tools/debug.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

using namespace ::com::sun::star;

ThumbnailViewAcc::ThumbnailViewAcc(ThumbnailView* pThumbnailView)
    : mpThumbnailView(pThumbnailView)
{
}

ThumbnailViewAcc::~ThumbnailViewAcc()
{
}

uno::Reference< accessibility::XAccessibleContext > SAL_CALL ThumbnailViewAcc::getAccessibleContext()
{
    ThrowIfDisposed();
    return this;
}

sal_Int64 SAL_CALL ThumbnailViewAcc::getAccessibleChildCount()
{
    const SolarMutexGuard aSolarGuard;
    ThrowIfDisposed();

    return mpThumbnailView->ImplGetVisibleItemCount();
}

uno::Reference< accessibility::XAccessible > SAL_CALL ThumbnailViewAcc::getAccessibleChild( sal_Int64 i )
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;

    if (i < 0 || i >= getAccessibleChildCount())
        throw lang::IndexOutOfBoundsException();

    ThumbnailViewItem* pItem = getItem (sal::static_int_cast< sal_uInt16 >(i));

    if( !pItem )
        throw lang::IndexOutOfBoundsException();

    rtl::Reference< ThumbnailViewItemAcc >  xRet = pItem->GetAccessible();
    return xRet;
}

uno::Reference< accessibility::XAccessible > SAL_CALL ThumbnailViewAcc::getAccessibleParent()
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    return mpThumbnailView->GetDrawingArea()->get_accessible_parent();
}

sal_Int64 SAL_CALL ThumbnailViewAcc::getAccessibleIndexInParent()
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
        TOOLS_WARN_EXCEPTION( "sfx", "ThumbnailViewAcc::getAccessibleIndexInParent" );
    }

    return nRet;
}

sal_Int16 SAL_CALL ThumbnailViewAcc::getAccessibleRole()
{
    ThrowIfDisposed();
    // #i73746# As the Java Access Bridge (v 2.0.1) uses "managesDescendants"
    // always if the role is LIST, we need a different role in this case
    return accessibility::AccessibleRole::LIST;
}

OUString SAL_CALL ThumbnailViewAcc::getAccessibleDescription()
{
    ThrowIfDisposed();
    return u"ThumbnailView"_ustr;
}

OUString SAL_CALL ThumbnailViewAcc::getAccessibleName()
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    OUString              aRet;

    if (mpThumbnailView)
    {
        aRet = mpThumbnailView->GetAccessibleName();
    }

    return aRet;
}

uno::Reference< accessibility::XAccessibleRelationSet > SAL_CALL ThumbnailViewAcc::getAccessibleRelationSet()
{
    ThrowIfDisposed();
    return uno::Reference< accessibility::XAccessibleRelationSet >();
}

sal_Int64 SAL_CALL ThumbnailViewAcc::getAccessibleStateSet()
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

    return nStateSet;
}

lang::Locale SAL_CALL ThumbnailViewAcc::getLocale()
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

uno::Reference< accessibility::XAccessible > SAL_CALL ThumbnailViewAcc::getAccessibleAtPoint( const awt::Point& aPoint )
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;

    const sal_uInt16 nItemId = mpThumbnailView->GetItemId(Point(aPoint.X, aPoint.Y));
    if ( !nItemId )
        return nullptr;

    const size_t nItemPos = mpThumbnailView->GetItemPos(nItemId);
    if( THUMBNAILVIEW_ITEM_NONEITEM == nItemPos )
        return nullptr;

    ThumbnailViewItem* const pItem = mpThumbnailView->mFilteredItemList[nItemPos];
    rtl::Reference<ThumbnailViewItemAcc> xRet = pItem->GetAccessible();

    return xRet;
}

awt::Rectangle ThumbnailViewAcc::implGetBounds()
{
    const Point         aOutPos;
    const Size aOutSize(mpThumbnailView->GetOutputSizePixel());
    awt::Rectangle      aRet;

    aRet.X = aOutPos.X();
    aRet.Y = aOutPos.Y();
    aRet.Width = aOutSize.Width();
    aRet.Height = aOutSize.Height();

    return aRet;
}

void SAL_CALL ThumbnailViewAcc::grabFocus()
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    mpThumbnailView->GrabFocus();
}

sal_Int32 SAL_CALL ThumbnailViewAcc::getForeground(  )
{
    ThrowIfDisposed();
    Color nColor = Application::GetSettings().GetStyleSettings().GetWindowTextColor();
    return static_cast<sal_Int32>(nColor);
}

sal_Int32 SAL_CALL ThumbnailViewAcc::getBackground(  )
{
    ThrowIfDisposed();
    Color nColor = Application::GetSettings().GetStyleSettings().GetWindowColor();
    return static_cast<sal_Int32>(nColor);
}

void SAL_CALL ThumbnailViewAcc::selectAccessibleChild( sal_Int64 nChildIndex )
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;

    if (nChildIndex < 0 || nChildIndex >= getAccessibleChildCount())
        throw lang::IndexOutOfBoundsException();

    ThumbnailViewItem* pItem = getItem (sal::static_int_cast< sal_uInt16 >(nChildIndex));

    if(pItem == nullptr)
        throw lang::IndexOutOfBoundsException();

    mpThumbnailView->SelectItem(pItem->mnId);
}

sal_Bool SAL_CALL ThumbnailViewAcc::isAccessibleChildSelected( sal_Int64 nChildIndex )
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;

    if (nChildIndex < 0 || nChildIndex >= getAccessibleChildCount())
        throw lang::IndexOutOfBoundsException();

    ThumbnailViewItem* pItem = getItem (sal::static_int_cast< sal_uInt16 >(nChildIndex));

    if (pItem == nullptr)
        throw lang::IndexOutOfBoundsException();

    return mpThumbnailView->IsItemSelected(pItem->mnId);
}

void SAL_CALL ThumbnailViewAcc::clearAccessibleSelection()
{
    ThrowIfDisposed();
}

void SAL_CALL ThumbnailViewAcc::selectAllAccessibleChildren()
{
    ThrowIfDisposed();
    // unsupported due to single selection only
}

sal_Int64 SAL_CALL ThumbnailViewAcc::getSelectedAccessibleChildCount()
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    sal_Int64 nRet = 0;

    for( sal_uInt16 i = 0, nCount = getItemCount(); i < nCount; i++ )
    {
        ThumbnailViewItem* pItem = getItem (i);

        if (pItem && mpThumbnailView->IsItemSelected(pItem->mnId))
            ++nRet;
    }

    return nRet;
}

uno::Reference< accessibility::XAccessible > SAL_CALL ThumbnailViewAcc::getSelectedAccessibleChild( sal_Int64 nSelectedChildIndex )
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    rtl::Reference< ThumbnailViewItemAcc > xRet;

    for( sal_uInt16 i = 0, nCount = getItemCount(), nSel = 0; ( i < nCount ) && !xRet.is(); i++ )
    {
        ThumbnailViewItem* pItem = getItem(i);

        if (pItem && mpThumbnailView->IsItemSelected(pItem->mnId)
            && (nSelectedChildIndex == static_cast<sal_Int32>(nSel++)))
            xRet = pItem->GetAccessible();
    }

    return xRet;
}

void SAL_CALL ThumbnailViewAcc::deselectAccessibleChild( sal_Int64 nChildIndex)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;

    if (nChildIndex < 0 || nChildIndex >= getAccessibleChildCount())
        throw lang::IndexOutOfBoundsException();

    // Because of the single selection we can reset the whole selection when
    // the specified child is currently selected.
//FIXME TODO    if (isAccessibleChildSelected(nChildIndex))
//FIXME TODO        ;
}

sal_uInt16 ThumbnailViewAcc::getItemCount() const
{
    return mpThumbnailView->ImplGetVisibleItemCount();
}

ThumbnailViewItem* ThumbnailViewAcc::getItem (sal_uInt16 nIndex) const
{
    return mpThumbnailView->ImplGetVisibleItem(nIndex);
}

void ThumbnailViewAcc::ThrowIfDisposed()
{
    ensureAlive();

    DBG_ASSERT (mpThumbnailView!=nullptr, "ValueSetAcc not disposed but mpThumbnailView == NULL");
}

void ThumbnailViewAcc::FireAccessibleEvent( short nEventId, const uno::Any& rOldValue, const uno::Any& rNewValue )
{
    NotifyAccessibleEvent(nEventId, rOldValue, rNewValue);
}

bool ThumbnailViewAcc::HasAccessibleListeners() const
{
    return OAccessibleComponentHelper::hasAccessibleListeners();
}

void ThumbnailViewAcc::GetFocus()
{
    // Broadcast the state change.
    css::uno::Any aOldState, aNewState;
    aNewState <<= css::accessibility::AccessibleStateType::FOCUSED;
    FireAccessibleEvent(
        css::accessibility::AccessibleEventId::STATE_CHANGED,
        aOldState, aNewState);
}

void ThumbnailViewAcc::LoseFocus()
{
    // Broadcast the state change.
    css::uno::Any aOldState, aNewState;
    aOldState <<= css::accessibility::AccessibleStateType::FOCUSED;
    FireAccessibleEvent(
        css::accessibility::AccessibleEventId::STATE_CHANGED,
        aOldState, aNewState);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
