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

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>

using namespace ::com::sun::star;

ThumbnailViewAcc::ThumbnailViewAcc( ThumbnailView* pParent, bool bIsTransientChildrenDisabled ) :
    ValueSetAccComponentBase (m_aMutex),
    mpParent( pParent ),
    mbIsTransientChildrenDisabled( bIsTransientChildrenDisabled ),
    mbIsFocused(false)
{
}

ThumbnailViewAcc::~ThumbnailViewAcc()
{
}

void ThumbnailViewAcc::FireAccessibleEvent( short nEventId, const uno::Any& rOldValue, const uno::Any& rNewValue )
{
    if( nEventId )
    {
        ::std::vector< uno::Reference< accessibility::XAccessibleEventListener > > aTmpListeners( mxEventListeners );
        accessibility::AccessibleEventObject aEvtObject;

        aEvtObject.EventId = nEventId;
        aEvtObject.Source = static_cast<uno::XWeak*>(this);
        aEvtObject.NewValue = rNewValue;
        aEvtObject.OldValue = rOldValue;

        for (::std::vector< uno::Reference< accessibility::XAccessibleEventListener > >::const_iterator aIter( aTmpListeners.begin() ), aEnd( aTmpListeners.end() );
            aIter != aEnd ; ++aIter)
        {
            try
            {
                (*aIter)->notifyEvent( aEvtObject );
            }
            catch(const uno::Exception&)
            {
            }
        }
    }
}

namespace
{
    class theValueSetAccUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theValueSetAccUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 >& ThumbnailViewAcc::getUnoTunnelId()
{
    return theValueSetAccUnoTunnelId::get().getSeq();
}

ThumbnailViewAcc* ThumbnailViewAcc::getImplementation( const uno::Reference< uno::XInterface >& rxData )
    throw()
{
    try
    {
        uno::Reference< lang::XUnoTunnel > xUnoTunnel( rxData, uno::UNO_QUERY );
        return( xUnoTunnel.is() ? reinterpret_cast<ThumbnailViewAcc*>(sal::static_int_cast<sal_IntPtr>(xUnoTunnel->getSomething( ThumbnailViewAcc::getUnoTunnelId() ))) : nullptr );
    }
    catch(const css::uno::Exception&)
    {
        return nullptr;
    }
}

void ThumbnailViewAcc::GetFocus()
{
    mbIsFocused = true;

    // Broadcast the state change.
    css::uno::Any aOldState, aNewState;
    aNewState <<= css::accessibility::AccessibleStateType::FOCUSED;
    FireAccessibleEvent(
        css::accessibility::AccessibleEventId::STATE_CHANGED,
        aOldState, aNewState);
}

void ThumbnailViewAcc::LoseFocus()
{
    mbIsFocused = false;

    // Broadcast the state change.
    css::uno::Any aOldState, aNewState;
    aOldState <<= css::accessibility::AccessibleStateType::FOCUSED;
    FireAccessibleEvent(
        css::accessibility::AccessibleEventId::STATE_CHANGED,
        aOldState, aNewState);
}

uno::Reference< accessibility::XAccessibleContext > SAL_CALL ThumbnailViewAcc::getAccessibleContext()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    return this;
}

sal_Int32 SAL_CALL ThumbnailViewAcc::getAccessibleChildCount()
    throw (uno::RuntimeException, std::exception)
{
    const SolarMutexGuard aSolarGuard;
    ThrowIfDisposed();

    sal_Int32 nCount = mpParent->ImplGetVisibleItemCount();
    return nCount;
}

uno::Reference< accessibility::XAccessible > SAL_CALL ThumbnailViewAcc::getAccessibleChild( sal_Int32 i )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    uno::Reference< accessibility::XAccessible >    xRet;
    ThumbnailViewItem* pItem = getItem (sal::static_int_cast< sal_uInt16 >(i));

    if( pItem )
        xRet = pItem->GetAccessible( mbIsTransientChildrenDisabled );
    else
        throw lang::IndexOutOfBoundsException();

    return xRet;
}

uno::Reference< accessibility::XAccessible > SAL_CALL ThumbnailViewAcc::getAccessibleParent()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    vcl::Window*                                         pParent = mpParent->GetParent();
    uno::Reference< accessibility::XAccessible >    xRet;

    if( pParent )
        xRet = pParent->GetAccessible();

    return xRet;
}

sal_Int32 SAL_CALL ThumbnailViewAcc::getAccessibleIndexInParent()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    vcl::Window*                 pParent = mpParent->GetParent();
    sal_Int32               nRet = 0;

    if( pParent )
    {
        bool bFound = false;

        for( sal_uInt16 i = 0, nCount = pParent->GetChildCount(); ( i < nCount ) && !bFound; i++ )
        {
            if( pParent->GetChild( i ) == mpParent )
            {
                nRet = i;
                bFound = true;
            }
        }
    }

    return nRet;
}

sal_Int16 SAL_CALL ThumbnailViewAcc::getAccessibleRole()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    // #i73746# As the Java Access Bridge (v 2.0.1) uses "managesDescendants"
    // always if the role is LIST, we need a different role in this case
    return (mbIsTransientChildrenDisabled
            ? accessibility::AccessibleRole::PANEL
            : accessibility::AccessibleRole::LIST );
}

OUString SAL_CALL ThumbnailViewAcc::getAccessibleDescription()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    return OUString("ThumbnailView");
}

OUString SAL_CALL ThumbnailViewAcc::getAccessibleName()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    OUString aRet;

    if ( mpParent )
    {
        aRet = mpParent->GetAccessibleName();
        if (aRet.isEmpty())
        {
            vcl::Window* pLabel = mpParent->GetAccessibleRelationLabeledBy();
            if (pLabel && pLabel != mpParent)
                aRet = OutputDevice::GetNonMnemonicString( pLabel->GetText() );
        }
    }

    return aRet;
}

uno::Reference< accessibility::XAccessibleRelationSet > SAL_CALL ThumbnailViewAcc::getAccessibleRelationSet()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    return uno::Reference< accessibility::XAccessibleRelationSet >();
}

uno::Reference< accessibility::XAccessibleStateSet > SAL_CALL ThumbnailViewAcc::getAccessibleStateSet()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    ::utl::AccessibleStateSetHelper* pStateSet = new ::utl::AccessibleStateSetHelper();

    // Set some states.
    pStateSet->AddState (accessibility::AccessibleStateType::ENABLED);
    pStateSet->AddState (accessibility::AccessibleStateType::SENSITIVE);
    pStateSet->AddState (accessibility::AccessibleStateType::SHOWING);
    pStateSet->AddState (accessibility::AccessibleStateType::VISIBLE);
    if ( !mbIsTransientChildrenDisabled )
        pStateSet->AddState (accessibility::AccessibleStateType::MANAGES_DESCENDANTS);
    pStateSet->AddState (accessibility::AccessibleStateType::FOCUSABLE);
    if (mbIsFocused)
        pStateSet->AddState (accessibility::AccessibleStateType::FOCUSED);

    return pStateSet;
}

lang::Locale SAL_CALL ThumbnailViewAcc::getLocale()
    throw (accessibility::IllegalAccessibleComponentStateException, uno::RuntimeException, std::exception)
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
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    ::osl::MutexGuard aGuard (m_aMutex);

    if( rxListener.is() )
    {
        std::vector< uno::Reference< accessibility::XAccessibleEventListener > >::const_iterator aIter = mxEventListeners.begin();
        bool bFound = false;

        while( !bFound && ( aIter != mxEventListeners.end() ) )
        {
            if( *aIter == rxListener )
                bFound = true;
            else
                ++aIter;
        }

        if (!bFound)
            mxEventListeners.push_back( rxListener );
    }
}

void SAL_CALL ThumbnailViewAcc::removeAccessibleEventListener( const uno::Reference< accessibility::XAccessibleEventListener >& rxListener )
    throw (uno::RuntimeException, std::exception)
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
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const awt::Rectangle    aRect( getBounds() );
    const Point             aSize( aRect.Width, aRect.Height );
    const Point             aNullPoint, aTestPoint( aPoint.X, aPoint.Y );

    return Rectangle( aNullPoint, aSize ).IsInside( aTestPoint );
}

uno::Reference< accessibility::XAccessible > SAL_CALL ThumbnailViewAcc::getAccessibleAtPoint( const awt::Point& aPoint )
    throw (uno::RuntimeException, std::exception)
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
            xRet = pItem->GetAccessible( mbIsTransientChildrenDisabled );
        }
    }

    return xRet;
}

awt::Rectangle SAL_CALL ThumbnailViewAcc::getBounds()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    const Point         aOutPos( mpParent->GetPosPixel() );
    const Size          aOutSize( mpParent->GetOutputSizePixel() );
    awt::Rectangle      aRet;

    aRet.X = aOutPos.X();
    aRet.Y = aOutPos.Y();
    aRet.Width = aOutSize.Width();
    aRet.Height = aOutSize.Height();

    return aRet;
}

awt::Point SAL_CALL ThumbnailViewAcc::getLocation()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const awt::Rectangle    aRect( getBounds() );
    awt::Point              aRet;

    aRet.X = aRect.X;
    aRet.Y = aRect.Y;

    return aRet;
}

awt::Point SAL_CALL ThumbnailViewAcc::getLocationOnScreen()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    const Point         aScreenPos( mpParent->OutputToAbsoluteScreenPixel( Point() ) );
    awt::Point          aRet;

    aRet.X = aScreenPos.X();
    aRet.Y = aScreenPos.Y();

    return aRet;
}

awt::Size SAL_CALL ThumbnailViewAcc::getSize()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const awt::Rectangle    aRect( getBounds() );
    awt::Size               aRet;

    aRet.Width = aRect.Width;
    aRet.Height = aRect.Height;

    return aRet;
}

void SAL_CALL ThumbnailViewAcc::grabFocus()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    mpParent->GrabFocus();
}

sal_Int32 SAL_CALL ThumbnailViewAcc::getForeground(  )
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    sal_uInt32 nColor = Application::GetSettings().GetStyleSettings().GetWindowTextColor().GetColor();
    return static_cast<sal_Int32>(nColor);
}

sal_Int32 SAL_CALL ThumbnailViewAcc::getBackground(  )
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    sal_uInt32 nColor = Application::GetSettings().GetStyleSettings().GetWindowColor().GetColor();
    return static_cast<sal_Int32>(nColor);
}

void SAL_CALL ThumbnailViewAcc::selectAccessibleChild( sal_Int32 nChildIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    ThumbnailViewItem* pItem = getItem (sal::static_int_cast< sal_uInt16 >(nChildIndex));

    if(pItem != nullptr)
    {
        mpParent->SelectItem( pItem->mnId );
    }
    else
        throw lang::IndexOutOfBoundsException();
}

sal_Bool SAL_CALL ThumbnailViewAcc::isAccessibleChildSelected( sal_Int32 nChildIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    ThumbnailViewItem* pItem = getItem (sal::static_int_cast< sal_uInt16 >(nChildIndex));
    bool            bRet = false;

    if (pItem != nullptr)
        bRet = mpParent->IsItemSelected( pItem->mnId );
    else
        throw lang::IndexOutOfBoundsException();

    return bRet;
}

void SAL_CALL ThumbnailViewAcc::clearAccessibleSelection()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
}

void SAL_CALL ThumbnailViewAcc::selectAllAccessibleChildren()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    // unsupported due to single selection only
}

sal_Int32 SAL_CALL ThumbnailViewAcc::getSelectedAccessibleChildCount()
    throw (uno::RuntimeException, std::exception)
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
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    uno::Reference< accessibility::XAccessible >    xRet;

    for( sal_uInt16 i = 0, nCount = getItemCount(), nSel = 0; ( i < nCount ) && !xRet.is(); i++ )
    {
        ThumbnailViewItem* pItem = getItem(i);

        if( pItem && mpParent->IsItemSelected( pItem->mnId ) && ( nSelectedChildIndex == static_cast< sal_Int32 >( nSel++ ) ) )
            xRet = pItem->GetAccessible( mbIsTransientChildrenDisabled );
    }

    return xRet;
}

void SAL_CALL ThumbnailViewAcc::deselectAccessibleChild( sal_Int32 nChildIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    // Because of the single selection we can reset the whole selection when
    // the specified child is currently selected.
//FIXME TODO    if (isAccessibleChildSelected(nChildIndex))
//FIXME TODO        ;
    (void) nChildIndex;
}

sal_Int64 SAL_CALL ThumbnailViewAcc::getSomething( const uno::Sequence< sal_Int8 >& rId ) throw( uno::RuntimeException, std::exception )
{
    sal_Int64 nRet;

    if( ( rId.getLength() == 16 ) && ( 0 == memcmp( ThumbnailViewAcc::getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) ) )
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
    ::std::vector<uno::Reference<accessibility::XAccessibleEventListener> >::const_iterator
          aListenerIterator (aListenerListCopy.begin());
    lang::EventObject aEvent (static_cast<accessibility::XAccessible*>(this));
    while (aListenerIterator != aListenerListCopy.end())
    {
        try
        {
            (*aListenerIterator)->disposing (aEvent);
        }
        catch(const uno::Exception&)
        {
            // Ignore exceptions.
        }

        ++aListenerIterator;
    }
}

sal_uInt16 ThumbnailViewAcc::getItemCount() const
{
    return mpParent->ImplGetVisibleItemCount();
}

ThumbnailViewItem* ThumbnailViewAcc::getItem (sal_uInt16 nIndex) const
{
    return mpParent->ImplGetVisibleItem (static_cast<sal_uInt16>(nIndex));
}

void ThumbnailViewAcc::ThrowIfDisposed()
    throw (css::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        OSL_TRACE ("Calling disposed object. Throwing exception:");
        throw lang::DisposedException (
            OUString("object has been already disposed"),
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

void ThumbnailViewItemAcc::FireAccessibleEvent( short nEventId, const uno::Any& rOldValue, const uno::Any& rNewValue )
{
    if( nEventId )
    {
        ::std::vector< uno::Reference< accessibility::XAccessibleEventListener > > aTmpListeners( mxEventListeners );
        accessibility::AccessibleEventObject aEvtObject;

        aEvtObject.EventId = nEventId;
        aEvtObject.Source = static_cast<uno::XWeak*>(this);
        aEvtObject.NewValue = rNewValue;
        aEvtObject.OldValue = rOldValue;

        for (::std::vector< uno::Reference< accessibility::XAccessibleEventListener > >::const_iterator aIter( aTmpListeners.begin() ), aEnd( aTmpListeners.end() );
            aIter != aEnd ; ++aIter)
        {
            try
            {
                (*aIter)->notifyEvent( aEvtObject );
            }
            catch(const uno::Exception&)
            {
            }
        }
    }
}

void ThumbnailViewItemAcc::ParentDestroyed()
{
    const ::osl::MutexGuard aGuard( maMutex );
    mpParent = nullptr;
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
        uno::Reference< lang::XUnoTunnel > xUnoTunnel( rxData, uno::UNO_QUERY );
        return( xUnoTunnel.is() ?
                reinterpret_cast<ThumbnailViewItemAcc*>(sal::static_int_cast<sal_IntPtr>(
                        xUnoTunnel->getSomething( ThumbnailViewItemAcc::getUnoTunnelId() ))) :
                nullptr );
    }
    catch(const css::uno::Exception&)
    {
        return nullptr;
    }
}

uno::Reference< accessibility::XAccessibleContext > SAL_CALL ThumbnailViewItemAcc::getAccessibleContext()
    throw (uno::RuntimeException, std::exception)
{
    return this;
}

sal_Int32 SAL_CALL ThumbnailViewItemAcc::getAccessibleChildCount()
    throw (uno::RuntimeException, std::exception)
{
    return 0;
}

uno::Reference< accessibility::XAccessible > SAL_CALL ThumbnailViewItemAcc::getAccessibleChild( sal_Int32 )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    throw lang::IndexOutOfBoundsException();
}

uno::Reference< accessibility::XAccessible > SAL_CALL ThumbnailViewItemAcc::getAccessibleParent()
    throw (uno::RuntimeException, std::exception)
{
    const SolarMutexGuard aSolarGuard;
    uno::Reference< accessibility::XAccessible >    xRet;

    if( mpParent )
        xRet = mpParent->mrParent.GetAccessible();

    return xRet;
}

sal_Int32 SAL_CALL ThumbnailViewItemAcc::getAccessibleIndexInParent()
    throw (uno::RuntimeException, std::exception)
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
            // just in case the number of children changes in the mean time.
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
    throw (uno::RuntimeException, std::exception)
{
    return accessibility::AccessibleRole::LIST_ITEM;
}

OUString SAL_CALL ThumbnailViewItemAcc::getAccessibleDescription()
    throw (uno::RuntimeException, std::exception)
{
    return OUString();
}

OUString SAL_CALL ThumbnailViewItemAcc::getAccessibleName()
    throw (uno::RuntimeException, std::exception)
{
    const SolarMutexGuard aSolarGuard;
    OUString aRet;

    if( mpParent )
    {
        aRet = mpParent->maTitle;

        if( aRet.isEmpty() )
        {
            OUStringBuffer aBuffer("Item ");
            aBuffer.append(static_cast<sal_Int32>(mpParent->mnId));
            aRet = aBuffer.makeStringAndClear();
        }
    }

    return aRet;
}

uno::Reference< accessibility::XAccessibleRelationSet > SAL_CALL ThumbnailViewItemAcc::getAccessibleRelationSet()
    throw (uno::RuntimeException, std::exception)
{
    return uno::Reference< accessibility::XAccessibleRelationSet >();
}

uno::Reference< accessibility::XAccessibleStateSet > SAL_CALL ThumbnailViewItemAcc::getAccessibleStateSet()
    throw (uno::RuntimeException, std::exception)
{
    const SolarMutexGuard aSolarGuard;
    ::utl::AccessibleStateSetHelper*    pStateSet = new ::utl::AccessibleStateSetHelper;

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
    throw (accessibility::IllegalAccessibleComponentStateException, uno::RuntimeException, std::exception)
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
    throw (uno::RuntimeException, std::exception)
{
    const ::osl::MutexGuard aGuard( maMutex );

    if( rxListener.is() )
    {
           ::std::vector< uno::Reference< accessibility::XAccessibleEventListener > >::const_iterator aIter = mxEventListeners.begin();
        bool bFound = false;

        while( !bFound && ( aIter != mxEventListeners.end() ) )
        {
            if( *aIter == rxListener )
                bFound = true;
            else
                ++aIter;
        }

        if (!bFound)
            mxEventListeners.push_back( rxListener );
    }
}

void SAL_CALL ThumbnailViewItemAcc::removeAccessibleEventListener( const uno::Reference< accessibility::XAccessibleEventListener >& rxListener )
    throw (uno::RuntimeException, std::exception)
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
    throw (uno::RuntimeException, std::exception)
{
    const awt::Rectangle    aRect( getBounds() );
    const Point             aSize( aRect.Width, aRect.Height );
    const Point             aNullPoint, aTestPoint( aPoint.X, aPoint.Y );

    return Rectangle( aNullPoint, aSize ).IsInside( aTestPoint );
}

uno::Reference< accessibility::XAccessible > SAL_CALL ThumbnailViewItemAcc::getAccessibleAtPoint( const awt::Point& )
    throw (uno::RuntimeException, std::exception)
{
    uno::Reference< accessibility::XAccessible > xRet;
    return xRet;
}

awt::Rectangle SAL_CALL ThumbnailViewItemAcc::getBounds()
    throw (uno::RuntimeException, std::exception)
{
    const SolarMutexGuard aSolarGuard;
    awt::Rectangle      aRet;

    if( mpParent )
    {
        Rectangle   aRect( mpParent->getDrawArea() );
        Point       aOrigin;
        Rectangle   aParentRect( aOrigin, mpParent->mrParent.GetOutputSizePixel() );

        aRect.Intersection( aParentRect );

        aRet.X = aRect.Left();
        aRet.Y = aRect.Top();
        aRet.Width = aRect.GetWidth();
        aRet.Height = aRect.GetHeight();
    }

    return aRet;
}

awt::Point SAL_CALL ThumbnailViewItemAcc::getLocation()
    throw (uno::RuntimeException, std::exception)
{
    const awt::Rectangle    aRect( getBounds() );
    awt::Point              aRet;

    aRet.X = aRect.X;
    aRet.Y = aRect.Y;

    return aRet;
}

awt::Point SAL_CALL ThumbnailViewItemAcc::getLocationOnScreen()
    throw (uno::RuntimeException, std::exception)
{
    const SolarMutexGuard aSolarGuard;
    awt::Point          aRet;

    if( mpParent )
    {
        const Point aPos = mpParent->getDrawArea().TopLeft();
        const Point aScreenPos( mpParent->mrParent.OutputToAbsoluteScreenPixel( aPos ) );

        aRet.X = aScreenPos.X();
        aRet.Y = aScreenPos.Y();
    }

    return aRet;
}

awt::Size SAL_CALL ThumbnailViewItemAcc::getSize()
    throw (uno::RuntimeException, std::exception)
{
    const awt::Rectangle    aRect( getBounds() );
    awt::Size               aRet;

    aRet.Width = aRect.Width;
    aRet.Height = aRect.Height;

    return aRet;
}

void SAL_CALL ThumbnailViewItemAcc::grabFocus()
    throw (uno::RuntimeException, std::exception)
{
    // nothing to do
}

sal_Int32 SAL_CALL ThumbnailViewItemAcc::getForeground(  )
    throw (uno::RuntimeException, std::exception)
{
    sal_uInt32 nColor = Application::GetSettings().GetStyleSettings().GetWindowTextColor().GetColor();
    return static_cast<sal_Int32>(nColor);
}

sal_Int32 SAL_CALL ThumbnailViewItemAcc::getBackground(  )
    throw (uno::RuntimeException, std::exception)
{
    return static_cast<sal_Int32>(Application::GetSettings().GetStyleSettings().GetWindowColor().GetColor());
}

sal_Int64 SAL_CALL ThumbnailViewItemAcc::getSomething( const uno::Sequence< sal_Int8 >& rId ) throw( uno::RuntimeException, std::exception )
{
    sal_Int64 nRet;

    if( ( rId.getLength() == 16 ) && ( 0 == memcmp( ThumbnailViewItemAcc::getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) ) )
        nRet = reinterpret_cast< sal_Int64 >( this );
    else
        nRet = 0;

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
