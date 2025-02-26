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

void SAL_CALL ThumbnailViewAcc::addAccessibleEventListener( const uno::Reference< accessibility::XAccessibleEventListener >& rxListener )
{
    ThrowIfDisposed();
    std::unique_lock aGuard (m_aMutex);

    if( !rxListener.is() )
        return;

    bool bFound = false;

    for (auto const& eventListener : mxEventListeners)
    {
        if( eventListener == rxListener )
        {
            bFound = true;
            break;
        }
    }

    if (!bFound)
        mxEventListeners.push_back( rxListener );
}

void SAL_CALL ThumbnailViewAcc::removeAccessibleEventListener( const uno::Reference< accessibility::XAccessibleEventListener >& rxListener )
{
    ThrowIfDisposed();
    std::unique_lock aGuard (m_aMutex);

    if( rxListener.is() )
    {
        std::vector< uno::Reference< accessibility::XAccessibleEventListener > >::iterator aIter =
            std::find(mxEventListeners.begin(), mxEventListeners.end(), rxListener);

        if (aIter != mxEventListeners.end())
            mxEventListeners.erase( aIter );
    }
}

sal_Bool SAL_CALL ThumbnailViewAcc::containsPoint( const awt::Point& aPoint )
{
    ThrowIfDisposed();
    const awt::Rectangle    aRect( getBounds() );
    const Point             aSize( aRect.Width, aRect.Height );
    const Point             aNullPoint, aTestPoint( aPoint.X, aPoint.Y );

    return tools::Rectangle( aNullPoint, aSize ).Contains( aTestPoint );
}

uno::Reference< accessibility::XAccessible > SAL_CALL ThumbnailViewAcc::getAccessibleAtPoint( const awt::Point& aPoint )
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    const sal_uInt16 nItemId = mpThumbnailView->GetItemId(Point(aPoint.X, aPoint.Y));
    uno::Reference< accessibility::XAccessible >    xRet;

    if ( nItemId )
    {
        const size_t nItemPos = mpThumbnailView->GetItemPos(nItemId);

        if( THUMBNAILVIEW_ITEM_NONEITEM != nItemPos )
        {
            ThumbnailViewItem* const pItem = mpThumbnailView->mFilteredItemList[nItemPos];
            xRet = pItem->GetAccessible();
        }
    }

    return xRet;
}

awt::Rectangle SAL_CALL ThumbnailViewAcc::getBounds()
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    const Point         aOutPos;
    const Size aOutSize(mpThumbnailView->GetOutputSizePixel());
    awt::Rectangle      aRet;

    aRet.X = aOutPos.X();
    aRet.Y = aOutPos.Y();
    aRet.Width = aOutSize.Width();
    aRet.Height = aOutSize.Height();

    return aRet;
}

awt::Point SAL_CALL ThumbnailViewAcc::getLocation()
{
    ThrowIfDisposed();
    const awt::Rectangle    aRect( getBounds() );
    awt::Point              aRet;

    aRet.X = aRect.X;
    aRet.Y = aRect.Y;

    return aRet;
}

awt::Point SAL_CALL ThumbnailViewAcc::getLocationOnScreen()
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    awt::Point aScreenLoc(0, 0);

    uno::Reference<accessibility::XAccessible> xParent(getAccessibleParent());
    if (xParent)
    {
        uno::Reference<accessibility::XAccessibleContext> xParentContext(xParent->getAccessibleContext());
        uno::Reference<accessibility::XAccessibleComponent> xParentComponent(xParentContext, css::uno::UNO_QUERY);
        OSL_ENSURE( xParentComponent.is(), "ThumbnailViewAcc::getLocationOnScreen: no parent component!" );
        if ( xParentComponent.is() )
        {
            awt::Point aParentScreenLoc( xParentComponent->getLocationOnScreen() );
            awt::Point aOwnRelativeLoc( getLocation() );
            aScreenLoc.X = aParentScreenLoc.X + aOwnRelativeLoc.X;
            aScreenLoc.Y = aParentScreenLoc.Y + aOwnRelativeLoc.Y;
        }
    }

    return aScreenLoc;
}

awt::Size SAL_CALL ThumbnailViewAcc::getSize()
{
    ThrowIfDisposed();
    const awt::Rectangle    aRect( getBounds() );
    awt::Size               aRet;

    aRet.Width = aRect.Width;
    aRet.Height = aRect.Height;

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
    uno::Reference< accessibility::XAccessible >    xRet;

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

void ThumbnailViewAcc::disposing(std::unique_lock<std::mutex>& rGuard)
{
    ::std::vector<uno::Reference<accessibility::XAccessibleEventListener> > aListenerListCopy;

    // unlock because we need to take solar and the lock mutex in the correct order
    rGuard.unlock();
    {
        const SolarMutexGuard aSolarGuard;
        std::unique_lock aGuard (m_aMutex);

        // Reset the pointer to the parent.  It has to be the one who has
        // disposed us because he is dying.
        mpThumbnailView = nullptr;

        if (mxEventListeners.empty())
            return;

        // Make a copy of the list and clear the original.
        aListenerListCopy = std::move(mxEventListeners);
    }

    // Inform all listeners that this objects is disposing.
    lang::EventObject aEvent (static_cast<accessibility::XAccessible*>(this));
    for (auto const& listener : aListenerListCopy)
    {
        try
        {
            listener->disposing (aEvent);
        }
        catch(const uno::Exception&)
        {
            // Ignore exceptions.
        }
    }
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
    if (m_bDisposed)
    {
        SAL_WARN("sfx", "Calling disposed object. Throwing exception:");
        throw lang::DisposedException (
            u"object has been already disposed"_ustr,
            getXWeak());
    }
    else
    {
        DBG_ASSERT (mpThumbnailView!=nullptr, "ValueSetAcc not disposed but mpThumbnailView == NULL");
    }
}

void ThumbnailViewAcc::FireAccessibleEvent( short nEventId, const uno::Any& rOldValue, const uno::Any& rNewValue )
{
    if( !nEventId )
        return;

    std::unique_lock aGuard(m_aMutex);
    ::std::vector< uno::Reference< accessibility::XAccessibleEventListener > > aTmpListeners( mxEventListeners );
    aGuard.unlock();
    accessibility::AccessibleEventObject aEvtObject;

    aEvtObject.EventId = nEventId;
    aEvtObject.Source = getXWeak();
    aEvtObject.NewValue = rNewValue;
    aEvtObject.OldValue = rOldValue;
    aEvtObject.IndexHint = -1;

    for (auto const& tmpListener : aTmpListeners)
    {
        try
        {
            tmpListener->notifyEvent( aEvtObject );
        }
        catch(const uno::Exception&)
        {
        }
    }
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
