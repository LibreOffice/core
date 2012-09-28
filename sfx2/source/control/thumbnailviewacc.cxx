/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "thumbnailviewacc.hxx"

#include <comphelper/servicehelper.hxx>
#include <sfx2/thumbnailview.hxx>
#include <sfx2/thumbnailviewitem.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <vcl/svapp.hxx>

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
        ::std::vector< uno::Reference< accessibility::XAccessibleEventListener > >::const_iterator aIter( aTmpListeners.begin() );
        accessibility::AccessibleEventObject aEvtObject;

        aEvtObject.EventId = nEventId;
        aEvtObject.Source = static_cast<uno::XWeak*>(this);
        aEvtObject.NewValue = rNewValue;
        aEvtObject.OldValue = rOldValue;

        while( aIter != aTmpListeners.end() )
        {
            try
            {
                (*aIter)->notifyEvent( aEvtObject );
            }
            catch(const uno::Exception&)
            {
            }

            aIter++;
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
        return( xUnoTunnel.is() ? reinterpret_cast<ThumbnailViewAcc*>(sal::static_int_cast<sal_IntPtr>(xUnoTunnel->getSomething( ThumbnailViewAcc::getUnoTunnelId() ))) : NULL );
    }
    catch(const ::com::sun::star::uno::Exception&)
    {
        return NULL;
    }
}

void ThumbnailViewAcc::GetFocus (void)
{
    mbIsFocused = true;

    // Boradcast the state change.
    ::com::sun::star::uno::Any aOldState, aNewState;
    aNewState <<= ::com::sun::star::accessibility::AccessibleStateType::FOCUSED;
    FireAccessibleEvent(
        ::com::sun::star::accessibility::AccessibleEventId::STATE_CHANGED,
        aOldState, aNewState);
}

void ThumbnailViewAcc::LoseFocus (void)
{
    mbIsFocused = false;

    // Boradcast the state change.
    ::com::sun::star::uno::Any aOldState, aNewState;
    aOldState <<= ::com::sun::star::accessibility::AccessibleStateType::FOCUSED;
    FireAccessibleEvent(
        ::com::sun::star::accessibility::AccessibleEventId::STATE_CHANGED,
        aOldState, aNewState);
}

uno::Reference< accessibility::XAccessibleContext > SAL_CALL ThumbnailViewAcc::getAccessibleContext()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    return this;
}

sal_Int32 SAL_CALL ThumbnailViewAcc::getAccessibleChildCount()
    throw (uno::RuntimeException)
{
    const SolarMutexGuard aSolarGuard;
    ThrowIfDisposed();

    sal_Int32 nCount = mpParent->ImplGetVisibleItemCount();
    return nCount;
}

uno::Reference< accessibility::XAccessible > SAL_CALL ThumbnailViewAcc::getAccessibleChild( sal_Int32 i )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
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
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    Window*                                         pParent = mpParent->GetParent();
    uno::Reference< accessibility::XAccessible >    xRet;

    if( pParent )
        xRet = pParent->GetAccessible();

    return xRet;
}

sal_Int32 SAL_CALL ThumbnailViewAcc::getAccessibleIndexInParent()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    Window*                 pParent = mpParent->GetParent();
    sal_Int32               nRet = 0;

    if( pParent )
    {
        sal_Bool bFound = sal_False;

        for( sal_uInt16 i = 0, nCount = pParent->GetChildCount(); ( i < nCount ) && !bFound; i++ )
        {
            if( pParent->GetChild( i ) == mpParent )
            {
                nRet = i;
                bFound = sal_True;
            }
        }
    }

    return nRet;
}

sal_Int16 SAL_CALL ThumbnailViewAcc::getAccessibleRole()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    // #i73746# As the Java Access Bridge (v 2.0.1) uses "managesDescendants"
    // always if the role is LIST, we need a different role in this case
    return (mbIsTransientChildrenDisabled
            ? accessibility::AccessibleRole::PANEL
            : accessibility::AccessibleRole::LIST );
}

::rtl::OUString SAL_CALL ThumbnailViewAcc::getAccessibleDescription()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    return rtl::OUString("ThumbnailView");
}

::rtl::OUString SAL_CALL ThumbnailViewAcc::getAccessibleName()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    rtl::OUString aRet;

    if ( mpParent )
        aRet = mpParent->GetAccessibleName();

    if ( aRet.isEmpty() )
    {
        Window* pLabel = mpParent->GetAccessibleRelationLabeledBy();
        if ( pLabel && pLabel != mpParent )
            aRet = OutputDevice::GetNonMnemonicString( pLabel->GetText() );
    }

    return aRet;
}

uno::Reference< accessibility::XAccessibleRelationSet > SAL_CALL ThumbnailViewAcc::getAccessibleRelationSet()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    return uno::Reference< accessibility::XAccessibleRelationSet >();
}

uno::Reference< accessibility::XAccessibleStateSet > SAL_CALL ThumbnailViewAcc::getAccessibleStateSet()
    throw (uno::RuntimeException)
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
    throw (accessibility::IllegalAccessibleComponentStateException, uno::RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    const ::rtl::OUString                           aEmptyStr;
    uno::Reference< accessibility::XAccessible >    xParent( getAccessibleParent() );
    lang::Locale                                    aRet( aEmptyStr, aEmptyStr, aEmptyStr );

    if( xParent.is() )
    {
        uno::Reference< accessibility::XAccessibleContext > xParentContext( xParent->getAccessibleContext() );

        if( xParentContext.is() )
            aRet = xParentContext->getLocale ();
    }

    return aRet;
}

void SAL_CALL ThumbnailViewAcc::addEventListener( const uno::Reference< accessibility::XAccessibleEventListener >& rxListener )
    throw (uno::RuntimeException)
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

void SAL_CALL ThumbnailViewAcc::removeEventListener( const uno::Reference< accessibility::XAccessibleEventListener >& rxListener )
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    ::osl::MutexGuard aGuard (m_aMutex);

    if( rxListener.is() )
    {
        std::vector< uno::Reference< accessibility::XAccessibleEventListener > >::iterator aIter = mxEventListeners.begin();
        bool bFound = false;

        while( !bFound && ( aIter != mxEventListeners.end() ) )
        {
            if( *aIter == rxListener )
            {
                mxEventListeners.erase( aIter );
                bFound = true;
            }
            else
                ++aIter;
        }
    }
}

sal_Bool SAL_CALL ThumbnailViewAcc::containsPoint( const awt::Point& aPoint )
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const awt::Rectangle    aRect( getBounds() );
    const Point             aSize( aRect.Width, aRect.Height );
    const Point             aNullPoint, aTestPoint( aPoint.X, aPoint.Y );

    return Rectangle( aNullPoint, aSize ).IsInside( aTestPoint );
}

uno::Reference< accessibility::XAccessible > SAL_CALL ThumbnailViewAcc::getAccessibleAtPoint( const awt::Point& aPoint )
    throw (uno::RuntimeException)
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
            ThumbnailViewItem *const pItem = mpParent->mItemList[nItemPos];
            xRet = pItem->GetAccessible( mbIsTransientChildrenDisabled );
        }
    }

    return xRet;
}

awt::Rectangle SAL_CALL ThumbnailViewAcc::getBounds()
    throw (uno::RuntimeException)
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
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const awt::Rectangle    aRect( getBounds() );
    awt::Point              aRet;

    aRet.X = aRect.X;
    aRet.Y = aRect.Y;

    return aRet;
}

awt::Point SAL_CALL ThumbnailViewAcc::getLocationOnScreen()
    throw (uno::RuntimeException)
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
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const awt::Rectangle    aRect( getBounds() );
    awt::Size               aRet;

    aRet.Width = aRect.Width;
    aRet.Height = aRect.Height;

    return aRet;
}

void SAL_CALL ThumbnailViewAcc::grabFocus()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    mpParent->GrabFocus();
}

uno::Any SAL_CALL ThumbnailViewAcc::getAccessibleKeyBinding()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    return uno::Any();
}

sal_Int32 SAL_CALL ThumbnailViewAcc::getForeground(  )
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    sal_uInt32 nColor = Application::GetSettings().GetStyleSettings().GetWindowTextColor().GetColor();
    return static_cast<sal_Int32>(nColor);
}

sal_Int32 SAL_CALL ThumbnailViewAcc::getBackground(  )
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    sal_uInt32 nColor = Application::GetSettings().GetStyleSettings().GetWindowColor().GetColor();
    return static_cast<sal_Int32>(nColor);
}

void SAL_CALL ThumbnailViewAcc::selectAccessibleChild( sal_Int32 nChildIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    ThumbnailViewItem* pItem = getItem (sal::static_int_cast< sal_uInt16 >(nChildIndex));

    if(pItem != NULL)
    {
        mpParent->SelectItem( pItem->mnId );
    }
    else
        throw lang::IndexOutOfBoundsException();
}

sal_Bool SAL_CALL ThumbnailViewAcc::isAccessibleChildSelected( sal_Int32 nChildIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    ThumbnailViewItem* pItem = getItem (sal::static_int_cast< sal_uInt16 >(nChildIndex));
    sal_Bool            bRet = sal_False;

    if (pItem != NULL)
        bRet = mpParent->IsItemSelected( pItem->mnId );
    else
        throw lang::IndexOutOfBoundsException();

    return bRet;
}

void SAL_CALL ThumbnailViewAcc::clearAccessibleSelection()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
}

void SAL_CALL ThumbnailViewAcc::selectAllAccessibleChildren()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    // unsupported due to single selection only
}

sal_Int32 SAL_CALL ThumbnailViewAcc::getSelectedAccessibleChildCount()
    throw (uno::RuntimeException)
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
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
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
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    // Because of the single selection we can reset the whole selection when
    // the specified child is currently selected.
//FIXME TODO    if (isAccessibleChildSelected(nChildIndex))
//FIXME TODO        ;
    (void) nChildIndex;
}

sal_Int64 SAL_CALL ThumbnailViewAcc::getSomething( const uno::Sequence< sal_Int8 >& rId ) throw( uno::RuntimeException )
{
    sal_Int64 nRet;

    if( ( rId.getLength() == 16 ) && ( 0 == rtl_compareMemory( ThumbnailViewAcc::getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) ) )
        nRet = reinterpret_cast< sal_Int64 >( this );
    else
        nRet = 0;

    return nRet;
}

void SAL_CALL ThumbnailViewAcc::disposing (void)
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
        mpParent = NULL;
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

sal_uInt16 ThumbnailViewAcc::getItemCount (void) const
{
    return mpParent->ImplGetVisibleItemCount();
}

ThumbnailViewItem* ThumbnailViewAcc::getItem (sal_uInt16 nIndex) const
{
    return mpParent->ImplGetVisibleItem (static_cast<sal_uInt16>(nIndex));
}

void ThumbnailViewAcc::ThrowIfDisposed (void)
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        OSL_TRACE ("Calling disposed object. Throwing exception:");
        throw lang::DisposedException (
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("object has been already disposed")),
            static_cast<uno::XWeak*>(this));
    }
    else
    {
        DBG_ASSERT (mpParent!=NULL, "ValueSetAcc not disposed but mpParent == NULL");
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
        ::std::vector< uno::Reference< accessibility::XAccessibleEventListener > >                  aTmpListeners( mxEventListeners );
        ::std::vector< uno::Reference< accessibility::XAccessibleEventListener > >::const_iterator  aIter( aTmpListeners.begin() );
        accessibility::AccessibleEventObject                                                        aEvtObject;

        aEvtObject.EventId = nEventId;
        aEvtObject.Source = static_cast<uno::XWeak*>(this);
        aEvtObject.NewValue = rNewValue;
        aEvtObject.OldValue = rOldValue;

        while( aIter != aTmpListeners.end() )
        {
            (*aIter)->notifyEvent( aEvtObject );
            aIter++;
        }
    }
}

void ThumbnailViewItemAcc::ParentDestroyed()
{
    const ::osl::MutexGuard aGuard( maMutex );
    mpParent = NULL;
}

namespace
{
    class theValueItemAccUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theValueItemAccUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 >& ThumbnailViewItemAcc::getUnoTunnelId()
{
    return theValueItemAccUnoTunnelId::get().getSeq();
}

uno::Reference< accessibility::XAccessibleContext > SAL_CALL ThumbnailViewItemAcc::getAccessibleContext()
    throw (uno::RuntimeException)
{
    return this;
}

sal_Int32 SAL_CALL ThumbnailViewItemAcc::getAccessibleChildCount()
    throw (uno::RuntimeException)
{
    return 0;
}

uno::Reference< accessibility::XAccessible > SAL_CALL ThumbnailViewItemAcc::getAccessibleChild( sal_Int32 )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    throw lang::IndexOutOfBoundsException();
}

uno::Reference< accessibility::XAccessible > SAL_CALL ThumbnailViewItemAcc::getAccessibleParent()
    throw (uno::RuntimeException)
{
    const SolarMutexGuard aSolarGuard;
    uno::Reference< accessibility::XAccessible >    xRet;

    if( mpParent )
        xRet = mpParent->mrParent.GetAccessible();

    return xRet;
}

sal_Int32 SAL_CALL ThumbnailViewItemAcc::getAccessibleIndexInParent()
    throw (uno::RuntimeException)
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
                pItem = NULL;
            }

            // Do not create an accessible object for the test.
            if (pItem != NULL && pItem->mpxAcc != NULL)
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
    throw (uno::RuntimeException)
{
    return accessibility::AccessibleRole::LIST_ITEM;
}

::rtl::OUString SAL_CALL ThumbnailViewItemAcc::getAccessibleDescription()
    throw (uno::RuntimeException)
{
    return ::rtl::OUString();
}

::rtl::OUString SAL_CALL ThumbnailViewItemAcc::getAccessibleName()
    throw (uno::RuntimeException)
{
    const SolarMutexGuard aSolarGuard;
    rtl::OUString aRet;

    if( mpParent )
    {
        aRet = mpParent->maTitle;

        if( aRet.isEmpty() )
        {
            rtl::OUStringBuffer aBuffer("Item ");
            aBuffer.append(static_cast<sal_Int32>(mpParent->mnId));
            aRet = aBuffer.makeStringAndClear();
        }
    }

    return aRet;
}

uno::Reference< accessibility::XAccessibleRelationSet > SAL_CALL ThumbnailViewItemAcc::getAccessibleRelationSet()
    throw (uno::RuntimeException)
{
    return uno::Reference< accessibility::XAccessibleRelationSet >();
}

uno::Reference< accessibility::XAccessibleStateSet > SAL_CALL ThumbnailViewItemAcc::getAccessibleStateSet()
    throw (uno::RuntimeException)
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
        if( mpParent->mrParent.GetSelectItemId() == mpParent->mnId )
        {
            pStateSet->AddState( accessibility::AccessibleStateType::SELECTED );
            //              pStateSet->AddState( accessibility::AccessibleStateType::FOCUSED );
        }
    }

    return pStateSet;
}

lang::Locale SAL_CALL ThumbnailViewItemAcc::getLocale()
    throw (accessibility::IllegalAccessibleComponentStateException, uno::RuntimeException)
{
    const SolarMutexGuard aSolarGuard;
    const ::rtl::OUString                           aEmptyStr;
    uno::Reference< accessibility::XAccessible >    xParent( getAccessibleParent() );
    lang::Locale                                    aRet( aEmptyStr, aEmptyStr, aEmptyStr );

    if( xParent.is() )
    {
        uno::Reference< accessibility::XAccessibleContext > xParentContext( xParent->getAccessibleContext() );

        if( xParentContext.is() )
            aRet = xParentContext->getLocale();
    }

    return aRet;
}

void SAL_CALL ThumbnailViewItemAcc::addEventListener( const uno::Reference< accessibility::XAccessibleEventListener >& rxListener )
    throw (uno::RuntimeException)
{
    const ::osl::MutexGuard aGuard( maMutex );

    if( rxListener.is() )
    {
           ::std::vector< uno::Reference< accessibility::XAccessibleEventListener > >::const_iterator aIter = mxEventListeners.begin();
        sal_Bool bFound = sal_False;

        while( !bFound && ( aIter != mxEventListeners.end() ) )
        {
            if( *aIter == rxListener )
                bFound = sal_True;
            else
                ++aIter;
        }

        if (!bFound)
            mxEventListeners.push_back( rxListener );
    }
}

void SAL_CALL ThumbnailViewItemAcc::removeEventListener( const uno::Reference< accessibility::XAccessibleEventListener >& rxListener )
    throw (uno::RuntimeException)
{
    const ::osl::MutexGuard aGuard( maMutex );

    if( rxListener.is() )
    {
           ::std::vector< uno::Reference< accessibility::XAccessibleEventListener > >::iterator aIter = mxEventListeners.begin();
        sal_Bool bFound = sal_False;

        while( !bFound && ( aIter != mxEventListeners.end() ) )
        {
            if( *aIter == rxListener )
            {
                mxEventListeners.erase( aIter );
                bFound = sal_True;
            }
            else
                ++aIter;
        }
    }
}

sal_Bool SAL_CALL ThumbnailViewItemAcc::containsPoint( const awt::Point& aPoint )
    throw (uno::RuntimeException)
{
    const awt::Rectangle    aRect( getBounds() );
    const Point             aSize( aRect.Width, aRect.Height );
    const Point             aNullPoint, aTestPoint( aPoint.X, aPoint.Y );

    return Rectangle( aNullPoint, aSize ).IsInside( aTestPoint );
}

uno::Reference< accessibility::XAccessible > SAL_CALL ThumbnailViewItemAcc::getAccessibleAtPoint( const awt::Point& )
    throw (uno::RuntimeException)
{
    uno::Reference< accessibility::XAccessible > xRet;
    return xRet;
}

awt::Rectangle SAL_CALL ThumbnailViewItemAcc::getBounds()
    throw (uno::RuntimeException)
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
    throw (uno::RuntimeException)
{
    const awt::Rectangle    aRect( getBounds() );
    awt::Point              aRet;

    aRet.X = aRect.X;
    aRet.Y = aRect.Y;

    return aRet;
}

awt::Point SAL_CALL ThumbnailViewItemAcc::getLocationOnScreen()
    throw (uno::RuntimeException)
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
    throw (uno::RuntimeException)
{
    const awt::Rectangle    aRect( getBounds() );
    awt::Size               aRet;

    aRet.Width = aRect.Width;
    aRet.Height = aRect.Height;

    return aRet;
}

void SAL_CALL ThumbnailViewItemAcc::grabFocus()
    throw (uno::RuntimeException)
{
    // nothing to do
}

uno::Any SAL_CALL ThumbnailViewItemAcc::getAccessibleKeyBinding()
    throw (uno::RuntimeException)
{
    return uno::Any();
}

sal_Int32 SAL_CALL ThumbnailViewItemAcc::getForeground(  )
    throw (uno::RuntimeException)
{
    sal_uInt32 nColor = Application::GetSettings().GetStyleSettings().GetWindowTextColor().GetColor();
    return static_cast<sal_Int32>(nColor);
}

sal_Int32 SAL_CALL ThumbnailViewItemAcc::getBackground(  )
    throw (uno::RuntimeException)
{
    return static_cast<sal_Int32>(Application::GetSettings().GetStyleSettings().GetWindowColor().GetColor());
}

sal_Int64 SAL_CALL ThumbnailViewItemAcc::getSomething( const uno::Sequence< sal_Int8 >& rId ) throw( uno::RuntimeException )
{
    sal_Int64 nRet;

    if( ( rId.getLength() == 16 ) && ( 0 == rtl_compareMemory( ThumbnailViewItemAcc::getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) ) )
        nRet = reinterpret_cast< sal_Int64 >( this );
    else
        nRet = 0;

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */


