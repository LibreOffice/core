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

#include <comphelper/servicehelper.hxx>
#include <sfx2/thumbnailview.hxx>
#include <sfx2/thumbnailviewitem.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <sal/log.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

using namespace ::com::sun::star;

ThumbnailViewAcc::ThumbnailViewAcc( ThumbnailView* pParent ) :
    ValueSetAccComponentBase (m_aMutex),
    mpParent( pParent )
{
}

ThumbnailViewAcc::~ThumbnailViewAcc()
{
}

namespace
{
    class theSfxValueSetAccUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSfxValueSetAccUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 >& ThumbnailViewAcc::getUnoTunnelId()
{
    return theSfxValueSetAccUnoTunnelId::get().getSeq();
}

ThumbnailViewAcc* ThumbnailViewAcc::getImplementation( const uno::Reference< uno::XInterface >& rxData )
    throw()
{
    try
    {
        return comphelper::getUnoTunnelImplementation<ThumbnailViewAcc>(rxData);
    }
    catch(const css::uno::Exception&)
    {
        return nullptr;
    }
}

uno::Reference< accessibility::XAccessibleContext > SAL_CALL ThumbnailViewAcc::getAccessibleContext()
{
    ThrowIfDisposed();
    return this;
}

sal_Int32 SAL_CALL ThumbnailViewAcc::getAccessibleChildCount()
{
    const SolarMutexGuard aSolarGuard;
    ThrowIfDisposed();

    sal_Int32 nCount = mpParent->ImplGetVisibleItemCount();
    return nCount;
}

uno::Reference< accessibility::XAccessible > SAL_CALL ThumbnailViewAcc::getAccessibleChild( sal_Int32 i )
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    ThumbnailViewItem* pItem = getItem (sal::static_int_cast< sal_uInt16 >(i));

    if( !pItem )
        throw lang::IndexOutOfBoundsException();

    uno::Reference< accessibility::XAccessible >  xRet = pItem->GetAccessible( /*bIsTransientChildrenDisabled*/false );
    return xRet;
}

uno::Reference< accessibility::XAccessible > SAL_CALL ThumbnailViewAcc::getAccessibleParent()
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    return mpParent->GetDrawingArea()->get_accessible_parent();
}

sal_Int32 SAL_CALL ThumbnailViewAcc::getAccessibleIndexInParent()
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;

    // -1 for child not found/no parent (according to specification)
    sal_Int32 nRet = -1;

    uno::Reference<accessibility::XAccessible> xParent(getAccessibleParent());
    if (!xParent)
        return nRet;

    try
    {
        uno::Reference<accessibility::XAccessibleContext> xParentContext(xParent->getAccessibleContext());

        //  iterate over parent's children and search for this object
        if ( xParentContext.is() )
        {
            sal_Int32 nChildCount = xParentContext->getAccessibleChildCount();
            for ( sal_Int32 nChild = 0; ( nChild < nChildCount ) && ( -1 == nRet ); ++nChild )
            {
                uno::Reference<XAccessible> xChild(xParentContext->getAccessibleChild(nChild));
                if ( xChild.get() == this )
                    nRet = nChild;
            }
        }
    }
    catch (const uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION( "sfx", "OAccessibleContextHelper::getAccessibleIndexInParent" );
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
    return "ThumbnailView";
}

OUString SAL_CALL ThumbnailViewAcc::getAccessibleName()
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    OUString              aRet;

    if (mpParent)
    {
        aRet = mpParent->GetAccessibleName();
    }

    return aRet;
}

uno::Reference< accessibility::XAccessibleRelationSet > SAL_CALL ThumbnailViewAcc::getAccessibleRelationSet()
{
    ThrowIfDisposed();
    return uno::Reference< accessibility::XAccessibleRelationSet >();
}

uno::Reference< accessibility::XAccessibleStateSet > SAL_CALL ThumbnailViewAcc::getAccessibleStateSet()
{
    ThrowIfDisposed();
    rtl::Reference<::utl::AccessibleStateSetHelper> pStateSet = new ::utl::AccessibleStateSetHelper();

    // Set some states.
    pStateSet->AddState (accessibility::AccessibleStateType::ENABLED);
    pStateSet->AddState (accessibility::AccessibleStateType::SENSITIVE);
    pStateSet->AddState (accessibility::AccessibleStateType::SHOWING);
    pStateSet->AddState (accessibility::AccessibleStateType::VISIBLE);
    pStateSet->AddState (accessibility::AccessibleStateType::MANAGES_DESCENDANTS);
    pStateSet->AddState (accessibility::AccessibleStateType::FOCUSABLE);

    return pStateSet;
}

lang::Locale SAL_CALL ThumbnailViewAcc::getLocale()
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    uno::Reference< accessibility::XAccessible >    xParent( getAccessibleParent() );
    lang::Locale                                    aRet( "", "", "" );

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
    ::osl::MutexGuard aGuard (m_aMutex);

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
    ::osl::MutexGuard aGuard (m_aMutex);

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

    return tools::Rectangle( aNullPoint, aSize ).IsInside( aTestPoint );
}

uno::Reference< accessibility::XAccessible > SAL_CALL ThumbnailViewAcc::getAccessibleAtPoint( const awt::Point& aPoint )
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    const sal_uInt16                                    nItemId = mpParent->GetItemId( Point( aPoint.X, aPoint.Y ) );
    uno::Reference< accessibility::XAccessible >    xRet;

    if ( nItemId )
    {
        const size_t nItemPos = mpParent->GetItemPos( nItemId );

        if( THUMBNAILVIEW_ITEM_NONEITEM != nItemPos )
        {
            ThumbnailViewItem *const pItem = mpParent->mFilteredItemList[nItemPos];
            xRet = pItem->GetAccessible( /*bIsTransientChildrenDisabled*/false );
        }
    }

    return xRet;
}

awt::Rectangle SAL_CALL ThumbnailViewAcc::getBounds()
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    const Point         aOutPos;
    const Size          aOutSize( mpParent->GetOutputSizePixel() );
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
    mpParent->GrabFocus();
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

void SAL_CALL ThumbnailViewAcc::selectAccessibleChild( sal_Int32 nChildIndex )
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    ThumbnailViewItem* pItem = getItem (sal::static_int_cast< sal_uInt16 >(nChildIndex));

    if(pItem == nullptr)
        throw lang::IndexOutOfBoundsException();

    mpParent->SelectItem( pItem->mnId );
}

sal_Bool SAL_CALL ThumbnailViewAcc::isAccessibleChildSelected( sal_Int32 nChildIndex )
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    ThumbnailViewItem* pItem = getItem (sal::static_int_cast< sal_uInt16 >(nChildIndex));

    if (pItem == nullptr)
        throw lang::IndexOutOfBoundsException();

    return mpParent->IsItemSelected( pItem->mnId );
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

sal_Int32 SAL_CALL ThumbnailViewAcc::getSelectedAccessibleChildCount()
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    sal_Int32           nRet = 0;

    for( sal_uInt16 i = 0, nCount = getItemCount(); i < nCount; i++ )
    {
        ThumbnailViewItem* pItem = getItem (i);

        if( pItem && mpParent->IsItemSelected( pItem->mnId ) )
            ++nRet;
    }

    return nRet;
}

uno::Reference< accessibility::XAccessible > SAL_CALL ThumbnailViewAcc::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    uno::Reference< accessibility::XAccessible >    xRet;

    for( sal_uInt16 i = 0, nCount = getItemCount(), nSel = 0; ( i < nCount ) && !xRet.is(); i++ )
    {
        ThumbnailViewItem* pItem = getItem(i);

        if( pItem && mpParent->IsItemSelected( pItem->mnId ) && ( nSelectedChildIndex == static_cast< sal_Int32 >( nSel++ ) ) )
            xRet = pItem->GetAccessible( /*bIsTransientChildrenDisabled*/false );
    }

    return xRet;
}

void SAL_CALL ThumbnailViewAcc::deselectAccessibleChild( sal_Int32 )
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    // Because of the single selection we can reset the whole selection when
    // the specified child is currently selected.
//FIXME TODO    if (isAccessibleChildSelected(nChildIndex))
//FIXME TODO        ;
}

sal_Int64 SAL_CALL ThumbnailViewAcc::getSomething( const uno::Sequence< sal_Int8 >& rId )
{
    sal_Int64 nRet;

    if( isUnoTunnelId<ThumbnailViewAcc>(rId) )
        nRet = reinterpret_cast< sal_Int64 >( this );
    else
        nRet = 0;

    return nRet;
}

void SAL_CALL ThumbnailViewAcc::disposing()
{
    ::std::vector<uno::Reference<accessibility::XAccessibleEventListener> > aListenerListCopy;

    {
        // Make a copy of the list and clear the original.
        const SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard (m_aMutex);
        aListenerListCopy = mxEventListeners;
        mxEventListeners.clear();

        // Reset the pointer to the parent.  It has to be the one who has
        // disposed us because he is dying.
        mpParent = nullptr;
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
    return mpParent->ImplGetVisibleItemCount();
}

ThumbnailViewItem* ThumbnailViewAcc::getItem (sal_uInt16 nIndex) const
{
    return mpParent->ImplGetVisibleItem (nIndex);
}

void ThumbnailViewAcc::ThrowIfDisposed()
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        SAL_WARN("sfx", "Calling disposed object. Throwing exception:");
        throw lang::DisposedException (
            "object has been already disposed",
            static_cast<uno::XWeak*>(this));
    }
    else
    {
        DBG_ASSERT (mpParent!=nullptr, "ValueSetAcc not disposed but mpParent == NULL");
    }
}

ThumbnailViewItemAcc::ThumbnailViewItemAcc( ThumbnailViewItem* pParent, bool bIsTransientChildrenDisabled ) :
    mpParent( pParent ),
    mbIsTransientChildrenDisabled( bIsTransientChildrenDisabled )
{
}

ThumbnailViewItemAcc::~ThumbnailViewItemAcc()
{
}

void ThumbnailViewItemAcc::ParentDestroyed()
{
    const ::osl::MutexGuard aGuard( maMutex );
    mpParent = nullptr;
}

void ThumbnailViewAcc::FireAccessibleEvent( short nEventId, const uno::Any& rOldValue, const uno::Any& rNewValue )
{
    if( !nEventId )
        return;

    ::std::vector< uno::Reference< accessibility::XAccessibleEventListener > > aTmpListeners( mxEventListeners );
    accessibility::AccessibleEventObject aEvtObject;

    aEvtObject.EventId = nEventId;
    aEvtObject.Source = static_cast<uno::XWeak*>(this);
    aEvtObject.NewValue = rNewValue;
    aEvtObject.OldValue = rOldValue;

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

namespace
{
    class theValueItemAccUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theValueItemAccUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 >& ThumbnailViewItemAcc::getUnoTunnelId()
{
    return theValueItemAccUnoTunnelId::get().getSeq();
}

ThumbnailViewItemAcc* ThumbnailViewItemAcc::getImplementation( const uno::Reference< uno::XInterface >& rxData )
    throw()
{
    try
    {
        return comphelper::getUnoTunnelImplementation<ThumbnailViewItemAcc>(rxData);
    }
    catch(const css::uno::Exception&)
    {
        return nullptr;
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

uno::Reference< accessibility::XAccessibleContext > SAL_CALL ThumbnailViewItemAcc::getAccessibleContext()
{
    return this;
}

sal_Int32 SAL_CALL ThumbnailViewItemAcc::getAccessibleChildCount()
{
    return 0;
}

uno::Reference< accessibility::XAccessible > SAL_CALL ThumbnailViewItemAcc::getAccessibleChild( sal_Int32 )
{
    throw lang::IndexOutOfBoundsException();
}

uno::Reference< accessibility::XAccessible > SAL_CALL ThumbnailViewItemAcc::getAccessibleParent()
{
    const SolarMutexGuard aSolarGuard;
    uno::Reference< accessibility::XAccessible >    xRet;

    if( mpParent )
        xRet = mpParent->mrParent.getAccessible();

    return xRet;
}

sal_Int32 SAL_CALL ThumbnailViewItemAcc::getAccessibleIndexInParent()
{
    const SolarMutexGuard aSolarGuard;
    // The index defaults to -1 to indicate the child does not belong to its
    // parent.
    sal_Int32 nIndexInParent = -1;

    if( mpParent )
    {
        bool bDone = false;

        sal_uInt16 nCount = mpParent->mrParent.ImplGetVisibleItemCount();
        ThumbnailViewItem* pItem;
        for (sal_uInt16 i=0; i<nCount && !bDone; i++)
        {
            // Guard the retrieval of the i-th child with a try/catch block
            // just in case the number of children changes in the meantime.
            try
            {
                pItem = mpParent->mrParent.ImplGetVisibleItem (i);
            }
            catch (const lang::IndexOutOfBoundsException&)
            {
                pItem = nullptr;
            }

            // Do not create an accessible object for the test.
            if (pItem != nullptr && pItem->mxAcc.is())
                if (pItem->GetAccessible( mbIsTransientChildrenDisabled ).get() == this )
                {
                    nIndexInParent = i;
                    bDone = true;
                }
        }
    }

    return nIndexInParent;
}

sal_Int16 SAL_CALL ThumbnailViewItemAcc::getAccessibleRole()
{
    return accessibility::AccessibleRole::LIST_ITEM;
}

OUString SAL_CALL ThumbnailViewItemAcc::getAccessibleDescription()
{
    return OUString();
}

OUString SAL_CALL ThumbnailViewItemAcc::getAccessibleName()
{
    const SolarMutexGuard aSolarGuard;
    OUString aRet;

    if( mpParent )
    {
        aRet = mpParent->maTitle;

        if( aRet.isEmpty() )
        {
            aRet = "Item " + OUString::number(static_cast<sal_Int32>(mpParent->mnId));
        }
    }

    return aRet;
}

uno::Reference< accessibility::XAccessibleRelationSet > SAL_CALL ThumbnailViewItemAcc::getAccessibleRelationSet()
{
    return uno::Reference< accessibility::XAccessibleRelationSet >();
}

uno::Reference< accessibility::XAccessibleStateSet > SAL_CALL ThumbnailViewItemAcc::getAccessibleStateSet()
{
    const SolarMutexGuard aSolarGuard;
    rtl::Reference<::utl::AccessibleStateSetHelper> pStateSet = new ::utl::AccessibleStateSetHelper;

    if( mpParent )
    {
        pStateSet->AddState (accessibility::AccessibleStateType::ENABLED);
        pStateSet->AddState (accessibility::AccessibleStateType::SENSITIVE);
        pStateSet->AddState (accessibility::AccessibleStateType::SHOWING);
        pStateSet->AddState (accessibility::AccessibleStateType::VISIBLE);
        if ( !mbIsTransientChildrenDisabled )
            pStateSet->AddState (accessibility::AccessibleStateType::TRANSIENT);

        // SELECTABLE
        pStateSet->AddState( accessibility::AccessibleStateType::SELECTABLE );
        //      pStateSet->AddState( accessibility::AccessibleStateType::FOCUSABLE );

        // SELECTED
        if( mpParent->isSelected() )
        {
            pStateSet->AddState( accessibility::AccessibleStateType::SELECTED );
            //              pStateSet->AddState( accessibility::AccessibleStateType::FOCUSED );
        }
    }

    return pStateSet;
}

lang::Locale SAL_CALL ThumbnailViewItemAcc::getLocale()
{
    const SolarMutexGuard aSolarGuard;
    uno::Reference< accessibility::XAccessible >    xParent( getAccessibleParent() );
    lang::Locale                                    aRet( "", "", "" );

    if( xParent.is() )
    {
        uno::Reference< accessibility::XAccessibleContext > xParentContext( xParent->getAccessibleContext() );

        if( xParentContext.is() )
            aRet = xParentContext->getLocale();
    }

    return aRet;
}

void SAL_CALL ThumbnailViewItemAcc::addAccessibleEventListener( const uno::Reference< accessibility::XAccessibleEventListener >& rxListener )
{
    const ::osl::MutexGuard aGuard( maMutex );

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

void SAL_CALL ThumbnailViewItemAcc::removeAccessibleEventListener( const uno::Reference< accessibility::XAccessibleEventListener >& rxListener )
{
    const ::osl::MutexGuard aGuard( maMutex );

    if( rxListener.is() )
    {
        std::vector< uno::Reference< accessibility::XAccessibleEventListener > >::iterator aIter =
            std::find(mxEventListeners.begin(), mxEventListeners.end(), rxListener);

        if (aIter != mxEventListeners.end())
            mxEventListeners.erase( aIter );
    }
}

sal_Bool SAL_CALL ThumbnailViewItemAcc::containsPoint( const awt::Point& aPoint )
{
    const awt::Rectangle    aRect( getBounds() );
    const Point             aSize( aRect.Width, aRect.Height );
    const Point             aNullPoint, aTestPoint( aPoint.X, aPoint.Y );

    return tools::Rectangle( aNullPoint, aSize ).IsInside( aTestPoint );
}

uno::Reference< accessibility::XAccessible > SAL_CALL ThumbnailViewItemAcc::getAccessibleAtPoint( const awt::Point& )
{
    uno::Reference< accessibility::XAccessible > xRet;
    return xRet;
}

awt::Rectangle SAL_CALL ThumbnailViewItemAcc::getBounds()
{
    const SolarMutexGuard aSolarGuard;
    awt::Rectangle      aRet;

    if( mpParent )
    {
        tools::Rectangle   aRect( mpParent->getDrawArea() );
        tools::Rectangle   aParentRect;

        // get position of the accessible parent in screen coordinates
        uno::Reference< XAccessible > xParent = getAccessibleParent();
        if ( xParent.is() )
        {
            uno::Reference<XAccessibleComponent> xParentComponent(xParent->getAccessibleContext(), uno::UNO_QUERY);
            if (xParentComponent.is())
            {
                awt::Size aParentSize = xParentComponent->getSize();
                aParentRect = tools::Rectangle(0, 0, aParentSize.Width, aParentSize.Height);
            }
        }

        aRect.Intersection( aParentRect );

        aRet.X = aRect.Left();
        aRet.Y = aRect.Top();
        aRet.Width = aRect.GetWidth();
        aRet.Height = aRect.GetHeight();
    }

    return aRet;
}

awt::Point SAL_CALL ThumbnailViewItemAcc::getLocation()
{
    const awt::Rectangle    aRect( getBounds() );
    awt::Point              aRet;

    aRet.X = aRect.X;
    aRet.Y = aRect.Y;

    return aRet;
}

awt::Point SAL_CALL ThumbnailViewItemAcc::getLocationOnScreen()
{
    const SolarMutexGuard aSolarGuard;
    awt::Point          aRet;

    if( mpParent )
    {
        const Point aPos = mpParent->getDrawArea().TopLeft();

        aRet.X = aPos.X();
        aRet.Y = aPos.Y();

        // get position of the accessible parent in screen coordinates
        uno::Reference< XAccessible > xParent = getAccessibleParent();
        if ( xParent.is() )
        {
            uno::Reference<XAccessibleComponent> xParentComponent(xParent->getAccessibleContext(), uno::UNO_QUERY);
            if (xParentComponent.is())
            {
                awt::Point aParentScreenLoc = xParentComponent->getLocationOnScreen();
                aRet.X += aParentScreenLoc.X;
                aRet.Y += aParentScreenLoc.Y;
            }
        }
    }

    return aRet;
}

awt::Size SAL_CALL ThumbnailViewItemAcc::getSize()
{
    const awt::Rectangle    aRect( getBounds() );
    awt::Size               aRet;

    aRet.Width = aRect.Width;
    aRet.Height = aRect.Height;

    return aRet;
}

void SAL_CALL ThumbnailViewItemAcc::grabFocus()
{
    // nothing to do
}

sal_Int32 SAL_CALL ThumbnailViewItemAcc::getForeground(  )
{
    Color nColor = Application::GetSettings().GetStyleSettings().GetWindowTextColor();
    return static_cast<sal_Int32>(nColor);
}

sal_Int32 SAL_CALL ThumbnailViewItemAcc::getBackground(  )
{
    return static_cast<sal_Int32>(Application::GetSettings().GetStyleSettings().GetWindowColor());
}

sal_Int64 SAL_CALL ThumbnailViewItemAcc::getSomething( const uno::Sequence< sal_Int8 >& rId )
{
    sal_Int64 nRet;

    if( isUnoTunnelId<ThumbnailViewItemAcc>(rId) )
        nRet = reinterpret_cast< sal_Int64 >( this );
    else
        nRet = 0;

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
