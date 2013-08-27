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

#include <unotools/accessiblestatesethelper.hxx>
#include <vcl/svapp.hxx>
#include <svtools/valueset.hxx>
#include "valueimp.hxx"
#include <comphelper/servicehelper.hxx>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>

using namespace ::com::sun::star;

// ----------------
// - ValueSetItem -
// ----------------

ValueSetItem::ValueSetItem( ValueSet& rParent )
    : mrParent(rParent)
    , mnId(0)
    , meType(VALUESETITEM_NONE)
    , mbVisible(true)
    , mpData(NULL)
    , mpxAcc(NULL)
{
}

// -----------------------------------------------------------------------

ValueSetItem::~ValueSetItem()
{
    if( mpxAcc )
    {
        static_cast< ValueItemAcc* >( mpxAcc->get() )->ParentDestroyed();
        delete mpxAcc;
    }
}

// -----------------------------------------------------------------------

uno::Reference< accessibility::XAccessible > ValueSetItem::GetAccessible( bool bIsTransientChildrenDisabled )
{
    if( !mpxAcc )
        mpxAcc = new uno::Reference< accessibility::XAccessible >( new ValueItemAcc( this, bIsTransientChildrenDisabled ) );

    return *mpxAcc;
}

// ---------------
// - ValueSetAcc -
// ---------------

ValueSetAcc::ValueSetAcc( ValueSet* pParent, bool bIsTransientChildrenDisabled ) :
    ValueSetAccComponentBase (m_aMutex),
    mpParent( pParent ),
    mbIsTransientChildrenDisabled( bIsTransientChildrenDisabled ),
    mbIsFocused(false)
{
}

// -----------------------------------------------------------------------------

ValueSetAcc::~ValueSetAcc()
{
}

// -----------------------------------------------------------------------

void ValueSetAcc::FireAccessibleEvent( short nEventId, const uno::Any& rOldValue, const uno::Any& rNewValue )
{
    if( nEventId )
    {
        ::std::vector< uno::Reference< accessibility::XAccessibleEventListener > >                  aTmpListeners( mxEventListeners );
        accessibility::AccessibleEventObject                                                        aEvtObject;

        aEvtObject.EventId = nEventId;
        aEvtObject.Source = static_cast<uno::XWeak*>(this);
        aEvtObject.NewValue = rNewValue;
        aEvtObject.OldValue = rOldValue;

        for (::std::vector< uno::Reference< accessibility::XAccessibleEventListener > >::const_iterator aIter( aTmpListeners.begin() );
            aIter != aTmpListeners.end() ; ++aIter)
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

const uno::Sequence< sal_Int8 >& ValueSetAcc::getUnoTunnelId()
{
    return theValueSetAccUnoTunnelId::get().getSeq();
}

// -----------------------------------------------------------------------------

ValueSetAcc* ValueSetAcc::getImplementation( const uno::Reference< uno::XInterface >& rxData )
    throw()
{
    try
    {
        uno::Reference< lang::XUnoTunnel > xUnoTunnel( rxData, uno::UNO_QUERY );
        return( xUnoTunnel.is() ? reinterpret_cast<ValueSetAcc*>(sal::static_int_cast<sal_IntPtr>(xUnoTunnel->getSomething( ValueSetAcc::getUnoTunnelId() ))) : NULL );
    }
    catch(const ::com::sun::star::uno::Exception&)
    {
        return NULL;
    }
}


// -----------------------------------------------------------------------------

void ValueSetAcc::GetFocus (void)
{
    mbIsFocused = true;

    // Boradcast the state change.
    ::com::sun::star::uno::Any aOldState, aNewState;
    aNewState <<= ::com::sun::star::accessibility::AccessibleStateType::FOCUSED;
    FireAccessibleEvent(
        ::com::sun::star::accessibility::AccessibleEventId::STATE_CHANGED,
        aOldState, aNewState);
}

// -----------------------------------------------------------------------------

void ValueSetAcc::LoseFocus (void)
{
    mbIsFocused = false;

    // Boradcast the state change.
    ::com::sun::star::uno::Any aOldState, aNewState;
    aOldState <<= ::com::sun::star::accessibility::AccessibleStateType::FOCUSED;
    FireAccessibleEvent(
        ::com::sun::star::accessibility::AccessibleEventId::STATE_CHANGED,
        aOldState, aNewState);
}

// -----------------------------------------------------------------------------

uno::Reference< accessibility::XAccessibleContext > SAL_CALL ValueSetAcc::getAccessibleContext()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    return this;
}

// -----------------------------------------------------------------------------

sal_Int32 SAL_CALL ValueSetAcc::getAccessibleChildCount()
    throw (uno::RuntimeException)
{
    const SolarMutexGuard aSolarGuard;
    ThrowIfDisposed();

    sal_Int32 nCount = mpParent->ImplGetVisibleItemCount();
    if (HasNoneField())
        nCount += 1;
    return nCount;
}

// -----------------------------------------------------------------------------

uno::Reference< accessibility::XAccessible > SAL_CALL ValueSetAcc::getAccessibleChild( sal_Int32 i )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    uno::Reference< accessibility::XAccessible >    xRet;
    ValueSetItem* pItem = getItem (sal::static_int_cast< sal_uInt16 >(i));

    if( pItem )
        xRet = pItem->GetAccessible( mbIsTransientChildrenDisabled );
    else
        throw lang::IndexOutOfBoundsException();

    return xRet;
}

// -----------------------------------------------------------------------------

uno::Reference< accessibility::XAccessible > SAL_CALL ValueSetAcc::getAccessibleParent()
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

// -----------------------------------------------------------------------------

sal_Int32 SAL_CALL ValueSetAcc::getAccessibleIndexInParent()
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

// -----------------------------------------------------------------------------

sal_Int16 SAL_CALL ValueSetAcc::getAccessibleRole()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    // #i73746# As the Java Access Bridge (v 2.0.1) uses "managesDescendants"
    // always if the role is LIST, we need a different role in this case
    return (mbIsTransientChildrenDisabled
            ? accessibility::AccessibleRole::PANEL
            : accessibility::AccessibleRole::LIST );
}

// -----------------------------------------------------------------------------

OUString SAL_CALL ValueSetAcc::getAccessibleDescription()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    OUString              aRet( "ValueSet" );

    return aRet;
}

// -----------------------------------------------------------------------------

OUString SAL_CALL ValueSetAcc::getAccessibleName()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    OUString              aRet;

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

// -----------------------------------------------------------------------------

uno::Reference< accessibility::XAccessibleRelationSet > SAL_CALL ValueSetAcc::getAccessibleRelationSet()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    return uno::Reference< accessibility::XAccessibleRelationSet >();
}

// -----------------------------------------------------------------------------

uno::Reference< accessibility::XAccessibleStateSet > SAL_CALL ValueSetAcc::getAccessibleStateSet()
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

// -----------------------------------------------------------------------------

lang::Locale SAL_CALL ValueSetAcc::getLocale()
    throw (accessibility::IllegalAccessibleComponentStateException, uno::RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    const OUString                           aEmptyStr;
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

// -----------------------------------------------------------------------------

void SAL_CALL ValueSetAcc::addAccessibleEventListener( const uno::Reference< accessibility::XAccessibleEventListener >& rxListener )
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    ::osl::MutexGuard aGuard (m_aMutex);

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

// -----------------------------------------------------------------------------

void SAL_CALL ValueSetAcc::removeAccessibleEventListener( const uno::Reference< accessibility::XAccessibleEventListener >& rxListener )
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    ::osl::MutexGuard aGuard (m_aMutex);

    if( rxListener.is() )
    {
        ::std::vector< uno::Reference< accessibility::XAccessibleEventListener > >::const_iterator aEnd = mxEventListeners.end();

        for(::std::vector< uno::Reference< accessibility::XAccessibleEventListener > >::iterator aIter = mxEventListeners.begin();
              aIter != aEnd;
              ++aIter)
        {
            if( *aIter == rxListener )
            {
                mxEventListeners.erase(aIter);
                break;
            }
        }
    }
}

// -----------------------------------------------------------------------------

sal_Bool SAL_CALL ValueSetAcc::containsPoint( const awt::Point& aPoint )
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const awt::Rectangle    aRect( getBounds() );
    const Point             aSize( aRect.Width, aRect.Height );
    const Point             aNullPoint, aTestPoint( aPoint.X, aPoint.Y );

    return Rectangle( aNullPoint, aSize ).IsInside( aTestPoint );
}

// -----------------------------------------------------------------------------

uno::Reference< accessibility::XAccessible > SAL_CALL ValueSetAcc::getAccessibleAtPoint( const awt::Point& aPoint )
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    const sal_uInt16                                    nItemId = mpParent->GetItemId( Point( aPoint.X, aPoint.Y ) );
    uno::Reference< accessibility::XAccessible >    xRet;

    if ( nItemId )
    {
        const size_t nItemPos = mpParent->GetItemPos( nItemId );

        if( VALUESET_ITEM_NONEITEM != nItemPos )
        {
            ValueSetItem *const pItem = mpParent->mItemList[nItemPos];
            xRet = pItem->GetAccessible( mbIsTransientChildrenDisabled );
        }
    }

    return xRet;
}

// -----------------------------------------------------------------------------

awt::Rectangle SAL_CALL ValueSetAcc::getBounds()
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

// -----------------------------------------------------------------------------

awt::Point SAL_CALL ValueSetAcc::getLocation()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const awt::Rectangle    aRect( getBounds() );
    awt::Point              aRet;

    aRet.X = aRect.X;
    aRet.Y = aRect.Y;

    return aRet;
}

// -----------------------------------------------------------------------------

awt::Point SAL_CALL ValueSetAcc::getLocationOnScreen()
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

// -----------------------------------------------------------------------------

awt::Size SAL_CALL ValueSetAcc::getSize()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const awt::Rectangle    aRect( getBounds() );
    awt::Size               aRet;

    aRet.Width = aRect.Width;
    aRet.Height = aRect.Height;

    return aRet;
}

// -----------------------------------------------------------------------------

void SAL_CALL ValueSetAcc::grabFocus()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    mpParent->GrabFocus();
}

// -----------------------------------------------------------------------------

uno::Any SAL_CALL ValueSetAcc::getAccessibleKeyBinding()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    return uno::Any();
}

// -----------------------------------------------------------------------------

sal_Int32 SAL_CALL ValueSetAcc::getForeground(  )
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    sal_uInt32 nColor = Application::GetSettings().GetStyleSettings().GetWindowTextColor().GetColor();
    return static_cast<sal_Int32>(nColor);
}

// -----------------------------------------------------------------------------

sal_Int32 SAL_CALL ValueSetAcc::getBackground(  )
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    sal_uInt32 nColor = Application::GetSettings().GetStyleSettings().GetWindowColor().GetColor();
    return static_cast<sal_Int32>(nColor);
}

// -----------------------------------------------------------------------------

void SAL_CALL ValueSetAcc::selectAccessibleChild( sal_Int32 nChildIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    ValueSetItem* pItem = getItem (sal::static_int_cast< sal_uInt16 >(nChildIndex));

    if(pItem != NULL)
    {
        mpParent->SelectItem( pItem->mnId );
        mpParent->Select ();
    }
    else
        throw lang::IndexOutOfBoundsException();
}

// -----------------------------------------------------------------------------

sal_Bool SAL_CALL ValueSetAcc::isAccessibleChildSelected( sal_Int32 nChildIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    ValueSetItem* pItem = getItem (sal::static_int_cast< sal_uInt16 >(nChildIndex));
    sal_Bool            bRet = sal_False;

    if (pItem != NULL)
        bRet = mpParent->IsItemSelected( pItem->mnId );
    else
        throw lang::IndexOutOfBoundsException();

    return bRet;
}

// -----------------------------------------------------------------------------

void SAL_CALL ValueSetAcc::clearAccessibleSelection()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    mpParent->SetNoSelection();
}

// -----------------------------------------------------------------------------

void SAL_CALL ValueSetAcc::selectAllAccessibleChildren()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    // unsupported due to single selection only
}

// -----------------------------------------------------------------------------

sal_Int32 SAL_CALL ValueSetAcc::getSelectedAccessibleChildCount()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    sal_Int32           nRet = 0;

    for( sal_uInt16 i = 0, nCount = getItemCount(); i < nCount; i++ )
    {
        ValueSetItem* pItem = getItem (i);

        if( pItem && mpParent->IsItemSelected( pItem->mnId ) )
            ++nRet;
    }

    return nRet;
}

// -----------------------------------------------------------------------------

uno::Reference< accessibility::XAccessible > SAL_CALL ValueSetAcc::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    uno::Reference< accessibility::XAccessible >    xRet;

    for( sal_uInt16 i = 0, nCount = getItemCount(), nSel = 0; ( i < nCount ) && !xRet.is(); i++ )
    {
        ValueSetItem* pItem = getItem(i);

        if( pItem && mpParent->IsItemSelected( pItem->mnId ) && ( nSelectedChildIndex == static_cast< sal_Int32 >( nSel++ ) ) )
            xRet = pItem->GetAccessible( mbIsTransientChildrenDisabled );
    }

    return xRet;
}

// -----------------------------------------------------------------------------

void SAL_CALL ValueSetAcc::deselectAccessibleChild( sal_Int32 nChildIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    // Because of the single selection we can reset the whole selection when
    // the specified child is currently selected.
    if (isAccessibleChildSelected(nChildIndex))
        mpParent->SetNoSelection();
}

// -----------------------------------------------------------------------------

sal_Int64 SAL_CALL ValueSetAcc::getSomething( const uno::Sequence< sal_Int8 >& rId ) throw( uno::RuntimeException )
{
    sal_Int64 nRet;

    if( ( rId.getLength() == 16 ) && ( 0 == memcmp( ValueSetAcc::getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) ) )
        nRet = reinterpret_cast< sal_Int64 >( this );
    else
        nRet = 0;

    return nRet;
}




void SAL_CALL ValueSetAcc::disposing (void)
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


sal_uInt16 ValueSetAcc::getItemCount (void) const
{
    sal_uInt16 nCount = mpParent->ImplGetVisibleItemCount();
    // When the None-Item is visible then increase the number of items by
    // one.
    if (HasNoneField())
        nCount += 1;
    return nCount;
}


ValueSetItem* ValueSetAcc::getItem (sal_uInt16 nIndex) const
{
    ValueSetItem* pItem = NULL;

    if (HasNoneField())
    {
        if (nIndex == 0)
            // When present the first item is the then always visible none field.
            pItem = mpParent->ImplGetItem (VALUESET_ITEM_NONEITEM);
        else
            // Shift down the index to compensate for the none field.
            nIndex -= 1;
    }
    if (pItem == NULL)
        pItem = mpParent->ImplGetVisibleItem (static_cast<sal_uInt16>(nIndex));

    return pItem;
}




void ValueSetAcc::ThrowIfDisposed (void)
    throw (::com::sun::star::lang::DisposedException)
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
        DBG_ASSERT (mpParent!=NULL, "ValueSetAcc not disposed but mpParent == NULL");
    }
}



bool ValueSetAcc::HasNoneField (void) const
{
    DBG_ASSERT (mpParent!=NULL, "ValueSetAcc::HasNoneField called with mpParent==NULL");
    return ((mpParent->GetStyle() & WB_NONEFIELD) != 0);
}




// ----------------
// - ValueItemAcc -
// ----------------

ValueItemAcc::ValueItemAcc( ValueSetItem* pParent, bool bIsTransientChildrenDisabled ) :
    mpParent( pParent ),
    mbIsTransientChildrenDisabled( bIsTransientChildrenDisabled )
{
}

// -----------------------------------------------------------------------------

ValueItemAcc::~ValueItemAcc()
{
}

// -----------------------------------------------------------------------

void ValueItemAcc::FireAccessibleEvent( short nEventId, const uno::Any& rOldValue, const uno::Any& rNewValue )
{
    if( nEventId )
    {
        ::std::vector< uno::Reference< accessibility::XAccessibleEventListener > >                  aTmpListeners( mxEventListeners );
        accessibility::AccessibleEventObject                                                        aEvtObject;

        aEvtObject.EventId = nEventId;
        aEvtObject.Source = static_cast<uno::XWeak*>(this);
        aEvtObject.NewValue = rNewValue;
        aEvtObject.OldValue = rOldValue;

        for (::std::vector< uno::Reference< accessibility::XAccessibleEventListener > >::const_iterator aIter( aTmpListeners.begin() );
             aIter != aTmpListeners.end() ; ++aIter)
        {
            (*aIter)->notifyEvent( aEvtObject );
        }
    }
}

// -----------------------------------------------------------------------------

void ValueItemAcc::ParentDestroyed()
{
    const ::osl::MutexGuard aGuard( maMutex );
    mpParent = NULL;
}

namespace
{
    class theValueItemAccUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theValueItemAccUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 >& ValueItemAcc::getUnoTunnelId()
{
    return theValueItemAccUnoTunnelId::get().getSeq();
}

// -----------------------------------------------------------------------------

ValueItemAcc* ValueItemAcc::getImplementation( const uno::Reference< uno::XInterface >& rxData )
    throw()
{
    try
    {
        uno::Reference< lang::XUnoTunnel > xUnoTunnel( rxData, uno::UNO_QUERY );
        return( xUnoTunnel.is() ? reinterpret_cast<ValueItemAcc*>(sal::static_int_cast<sal_IntPtr>(xUnoTunnel->getSomething( ValueItemAcc::getUnoTunnelId() ))) : NULL );
    }
    catch(const ::com::sun::star::uno::Exception&)
    {
        return NULL;
    }
}

// -----------------------------------------------------------------------------

uno::Reference< accessibility::XAccessibleContext > SAL_CALL ValueItemAcc::getAccessibleContext()
    throw (uno::RuntimeException)
{
    return this;
}

// -----------------------------------------------------------------------------

sal_Int32 SAL_CALL ValueItemAcc::getAccessibleChildCount()
    throw (uno::RuntimeException)
{
    return 0;
}

// -----------------------------------------------------------------------------

uno::Reference< accessibility::XAccessible > SAL_CALL ValueItemAcc::getAccessibleChild( sal_Int32 )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    throw lang::IndexOutOfBoundsException();
}

// -----------------------------------------------------------------------------

uno::Reference< accessibility::XAccessible > SAL_CALL ValueItemAcc::getAccessibleParent()
    throw (uno::RuntimeException)
{
    const SolarMutexGuard aSolarGuard;
    uno::Reference< accessibility::XAccessible >    xRet;

    if( mpParent )
        xRet = mpParent->mrParent.GetAccessible();

    return xRet;
}

// -----------------------------------------------------------------------------

sal_Int32 SAL_CALL ValueItemAcc::getAccessibleIndexInParent()
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
        ValueSetItem* pItem;
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

// -----------------------------------------------------------------------------

sal_Int16 SAL_CALL ValueItemAcc::getAccessibleRole()
    throw (uno::RuntimeException)
{
    return accessibility::AccessibleRole::LIST_ITEM;
}

// -----------------------------------------------------------------------------

OUString SAL_CALL ValueItemAcc::getAccessibleDescription()
    throw (uno::RuntimeException)
{
    return OUString();
}

// -----------------------------------------------------------------------------

OUString SAL_CALL ValueItemAcc::getAccessibleName()
    throw (uno::RuntimeException)
{
    const SolarMutexGuard aSolarGuard;
    OUString aRet;

    if( mpParent )
    {
        aRet = mpParent->maText;

        if( aRet.isEmpty() )
        {
            OUStringBuffer aBuffer("Item ");
            aBuffer.append(static_cast<sal_Int32>(mpParent->mnId));
            aRet = aBuffer.makeStringAndClear();
        }
    }

    return aRet;
}

// -----------------------------------------------------------------------------

uno::Reference< accessibility::XAccessibleRelationSet > SAL_CALL ValueItemAcc::getAccessibleRelationSet()
    throw (uno::RuntimeException)
{
    return uno::Reference< accessibility::XAccessibleRelationSet >();
}

// -----------------------------------------------------------------------------

uno::Reference< accessibility::XAccessibleStateSet > SAL_CALL ValueItemAcc::getAccessibleStateSet()
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

// -----------------------------------------------------------------------------

lang::Locale SAL_CALL ValueItemAcc::getLocale()
    throw (accessibility::IllegalAccessibleComponentStateException, uno::RuntimeException)
{
    const SolarMutexGuard aSolarGuard;
    const OUString                           aEmptyStr;
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

// -----------------------------------------------------------------------------

void SAL_CALL ValueItemAcc::addAccessibleEventListener( const uno::Reference< accessibility::XAccessibleEventListener >& rxListener )
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

// -----------------------------------------------------------------------------

void SAL_CALL ValueItemAcc::removeAccessibleEventListener( const uno::Reference< accessibility::XAccessibleEventListener >& rxListener )
    throw (uno::RuntimeException)
{
    const ::osl::MutexGuard aGuard( maMutex );

    if( rxListener.is() )
    {
        ::std::vector< uno::Reference< accessibility::XAccessibleEventListener > >::const_iterator aEnd = mxEventListeners.end();

        for(::std::vector< uno::Reference< accessibility::XAccessibleEventListener > >::iterator aIter = mxEventListeners.begin();
              aIter != aEnd;
              ++aIter)
        {
            if( *aIter == rxListener )
            {
                mxEventListeners.erase(aIter);
                break;
            }
        }
    }
}

// -----------------------------------------------------------------------------

sal_Bool SAL_CALL ValueItemAcc::containsPoint( const awt::Point& aPoint )
    throw (uno::RuntimeException)
{
    const awt::Rectangle    aRect( getBounds() );
    const Point             aSize( aRect.Width, aRect.Height );
    const Point             aNullPoint, aTestPoint( aPoint.X, aPoint.Y );

    return Rectangle( aNullPoint, aSize ).IsInside( aTestPoint );
}

// -----------------------------------------------------------------------------

uno::Reference< accessibility::XAccessible > SAL_CALL ValueItemAcc::getAccessibleAtPoint( const awt::Point& )
    throw (uno::RuntimeException)
{
    uno::Reference< accessibility::XAccessible > xRet;
    return xRet;
}

// -----------------------------------------------------------------------------

awt::Rectangle SAL_CALL ValueItemAcc::getBounds()
    throw (uno::RuntimeException)
{
    const SolarMutexGuard aSolarGuard;
    awt::Rectangle      aRet;

    if( mpParent )
    {
        Rectangle   aRect( mpParent->mrParent.GetItemRect(mpParent->mnId) );
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

// -----------------------------------------------------------------------------

awt::Point SAL_CALL ValueItemAcc::getLocation()
    throw (uno::RuntimeException)
{
    const awt::Rectangle    aRect( getBounds() );
    awt::Point              aRet;

    aRet.X = aRect.X;
    aRet.Y = aRect.Y;

    return aRet;
}

// -----------------------------------------------------------------------------

awt::Point SAL_CALL ValueItemAcc::getLocationOnScreen()
    throw (uno::RuntimeException)
{
    const SolarMutexGuard aSolarGuard;
    awt::Point          aRet;

    if( mpParent )
    {
        const Point aPos = mpParent->mrParent.GetItemRect(mpParent->mnId).TopLeft();
        const Point aScreenPos( mpParent->mrParent.OutputToAbsoluteScreenPixel( aPos ) );

        aRet.X = aScreenPos.X();
        aRet.Y = aScreenPos.Y();
    }

    return aRet;
}

// -----------------------------------------------------------------------------

awt::Size SAL_CALL ValueItemAcc::getSize()
    throw (uno::RuntimeException)
{
    const awt::Rectangle    aRect( getBounds() );
    awt::Size               aRet;

    aRet.Width = aRect.Width;
    aRet.Height = aRect.Height;

    return aRet;
}

// -----------------------------------------------------------------------------

void SAL_CALL ValueItemAcc::grabFocus()
    throw (uno::RuntimeException)
{
    // nothing to do
}

// -----------------------------------------------------------------------------

uno::Any SAL_CALL ValueItemAcc::getAccessibleKeyBinding()
    throw (uno::RuntimeException)
{
    return uno::Any();
}

// -----------------------------------------------------------------------------

sal_Int32 SAL_CALL ValueItemAcc::getForeground(  )
    throw (uno::RuntimeException)
{
    sal_uInt32 nColor = Application::GetSettings().GetStyleSettings().GetWindowTextColor().GetColor();
    return static_cast<sal_Int32>(nColor);
}

// -----------------------------------------------------------------------------

sal_Int32 SAL_CALL ValueItemAcc::getBackground(  )
    throw (uno::RuntimeException)
{
    sal_uInt32 nColor;
    if (mpParent && mpParent->meType == VALUESETITEM_COLOR)
        nColor = mpParent->maColor.GetColor();
    else
        nColor = Application::GetSettings().GetStyleSettings().GetWindowColor().GetColor();
    return static_cast<sal_Int32>(nColor);
}

// -----------------------------------------------------------------------------

sal_Int64 SAL_CALL ValueItemAcc::getSomething( const uno::Sequence< sal_Int8 >& rId ) throw( uno::RuntimeException )
{
    sal_Int64 nRet;

    if( ( rId.getLength() == 16 ) && ( 0 == memcmp( ValueItemAcc::getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) ) )
        nRet = reinterpret_cast< sal_Int64 >( this );
    else
        nRet = 0;

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
