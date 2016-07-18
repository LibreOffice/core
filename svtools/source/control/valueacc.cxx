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
#include <vcl/settings.hxx>
#include <svtools/valueset.hxx>
#include "valueimp.hxx"
#include <comphelper/servicehelper.hxx>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <unotools/accessiblerelationsethelper.hxx>

using namespace ::com::sun::star;


ValueSetItem::ValueSetItem( ValueSet& rParent )
    : mrParent(rParent)
    , mnId(0)
    , meType(VALUESETITEM_NONE)
    , mbVisible(true)
    , mpData(nullptr)
    , mxAcc()
{
}


ValueSetItem::~ValueSetItem()
{
    if( mxAcc.is() )
    {
        static_cast< ValueItemAcc* >( mxAcc.get() )->ParentDestroyed();
    }
}


uno::Reference< accessibility::XAccessible > const & ValueSetItem::GetAccessible( bool bIsTransientChildrenDisabled )
{
    if( !mxAcc.is() )
        mxAcc = new ValueItemAcc( this, bIsTransientChildrenDisabled );

    return mxAcc;
}


ValueSetAcc::ValueSetAcc( ValueSet* pParent, bool bIsTransientChildrenDisabled ) :
    ValueSetAccComponentBase (m_aMutex),
    mpParent( pParent ),
    mbIsTransientChildrenDisabled( bIsTransientChildrenDisabled ),
    mbIsFocused(false)
{
}


ValueSetAcc::~ValueSetAcc()
{
}


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


ValueSetAcc* ValueSetAcc::getImplementation( const uno::Reference< uno::XInterface >& rxData )
    throw()
{
    try
    {
        uno::Reference< lang::XUnoTunnel > xUnoTunnel( rxData, uno::UNO_QUERY );
        return( xUnoTunnel.is() ? reinterpret_cast<ValueSetAcc*>(sal::static_int_cast<sal_IntPtr>(xUnoTunnel->getSomething( ValueSetAcc::getUnoTunnelId() ))) : nullptr );
    }
    catch(const css::uno::Exception&)
    {
        return nullptr;
    }
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
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    return this;
}


sal_Int32 SAL_CALL ValueSetAcc::getAccessibleChildCount()
    throw (uno::RuntimeException, std::exception)
{
    const SolarMutexGuard aSolarGuard;
    ThrowIfDisposed();

    sal_Int32 nCount = mpParent->ImplGetVisibleItemCount();
    if (HasNoneField())
        nCount += 1;
    return nCount;
}


uno::Reference< accessibility::XAccessible > SAL_CALL ValueSetAcc::getAccessibleChild( sal_Int32 i )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
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


uno::Reference< accessibility::XAccessible > SAL_CALL ValueSetAcc::getAccessibleParent()
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


sal_Int32 SAL_CALL ValueSetAcc::getAccessibleIndexInParent()
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


sal_Int16 SAL_CALL ValueSetAcc::getAccessibleRole()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    // #i73746# As the Java Access Bridge (v 2.0.1) uses "managesDescendants"
    // always if the role is LIST, we need a different role in this case
    return (mbIsTransientChildrenDisabled
            ? accessibility::AccessibleRole::PANEL
            : accessibility::AccessibleRole::LIST );
}


OUString SAL_CALL ValueSetAcc::getAccessibleDescription()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    OUString              aRet( "ValueSet" );

    return aRet;
}


OUString SAL_CALL ValueSetAcc::getAccessibleName()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    OUString              aRet;

    if (mpParent)
    {
        aRet = mpParent->GetAccessibleName();

        if ( aRet.isEmpty() )
        {
            vcl::Window* pLabel = mpParent->GetAccessibleRelationLabeledBy();
            if ( pLabel && pLabel != mpParent )
                aRet = OutputDevice::GetNonMnemonicString( pLabel->GetText() );

            if (aRet.isEmpty())
                 aRet = mpParent->GetQuickHelpText();
        }
    }

    return aRet;
}


uno::Reference< accessibility::XAccessibleRelationSet > SAL_CALL ValueSetAcc::getAccessibleRelationSet()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    SolarMutexGuard g;
    uno::Reference< accessibility::XAccessibleRelationSet > xRelSet;
    vcl::Window* pWindow = static_cast<vcl::Window*>(mpParent);
    if ( pWindow )
    {
        utl::AccessibleRelationSetHelper* pRelationSet = new utl::AccessibleRelationSetHelper;
        xRelSet = pRelationSet;

        vcl::Window *pLabeledBy = pWindow->GetAccessibleRelationLabeledBy();
        if ( pLabeledBy && pLabeledBy != pWindow )
        {
            uno::Sequence< uno::Reference< uno::XInterface > > aSequence { pLabeledBy->GetAccessible() };
            pRelationSet->AddRelation( accessibility::AccessibleRelation( accessibility::AccessibleRelationType::LABELED_BY, aSequence ) );
        }

        vcl::Window* pMemberOf = pWindow->GetAccessibleRelationMemberOf();
        if ( pMemberOf && pMemberOf != pWindow )
        {
            uno::Sequence< uno::Reference< uno::XInterface > > aSequence { pMemberOf->GetAccessible() };
            pRelationSet->AddRelation( accessibility::AccessibleRelation( accessibility::AccessibleRelationType::MEMBER_OF, aSequence ) );
        }
    }
    return xRelSet;
}


uno::Reference< accessibility::XAccessibleStateSet > SAL_CALL ValueSetAcc::getAccessibleStateSet()
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


lang::Locale SAL_CALL ValueSetAcc::getLocale()
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


void SAL_CALL ValueSetAcc::addAccessibleEventListener( const uno::Reference< accessibility::XAccessibleEventListener >& rxListener )
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    ::osl::MutexGuard aGuard (m_aMutex);

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


void SAL_CALL ValueSetAcc::removeAccessibleEventListener( const uno::Reference< accessibility::XAccessibleEventListener >& rxListener )
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    ::osl::MutexGuard aGuard (m_aMutex);

    if( rxListener.is() )
    {
        ::std::vector< uno::Reference< accessibility::XAccessibleEventListener > >::iterator aIter =
            std::find(mxEventListeners.begin(), mxEventListeners.end(), rxListener);

        if (aIter != mxEventListeners.end())
            mxEventListeners.erase(aIter);
    }
}


sal_Bool SAL_CALL ValueSetAcc::containsPoint( const awt::Point& aPoint )
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const awt::Rectangle    aRect( getBounds() );
    const Point             aSize( aRect.Width, aRect.Height );
    const Point             aNullPoint, aTestPoint( aPoint.X, aPoint.Y );

    return Rectangle( aNullPoint, aSize ).IsInside( aTestPoint );
}


uno::Reference< accessibility::XAccessible > SAL_CALL ValueSetAcc::getAccessibleAtPoint( const awt::Point& aPoint )
    throw (uno::RuntimeException, std::exception)
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


awt::Rectangle SAL_CALL ValueSetAcc::getBounds()
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


awt::Point SAL_CALL ValueSetAcc::getLocation()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const awt::Rectangle    aRect( getBounds() );
    awt::Point              aRet;

    aRet.X = aRect.X;
    aRet.Y = aRect.Y;

    return aRet;
}


awt::Point SAL_CALL ValueSetAcc::getLocationOnScreen()
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


awt::Size SAL_CALL ValueSetAcc::getSize()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const awt::Rectangle    aRect( getBounds() );
    awt::Size               aRet;

    aRet.Width = aRect.Width;
    aRet.Height = aRect.Height;

    return aRet;
}

void SAL_CALL ValueSetAcc::grabFocus()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    mpParent->GrabFocus();
}

sal_Int32 SAL_CALL ValueSetAcc::getForeground(  )
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    sal_uInt32 nColor = Application::GetSettings().GetStyleSettings().GetWindowTextColor().GetColor();
    return static_cast<sal_Int32>(nColor);
}

sal_Int32 SAL_CALL ValueSetAcc::getBackground(  )
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    sal_uInt32 nColor = Application::GetSettings().GetStyleSettings().GetWindowColor().GetColor();
    return static_cast<sal_Int32>(nColor);
}

void SAL_CALL ValueSetAcc::selectAccessibleChild( sal_Int32 nChildIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    ValueSetItem* pItem = getItem (sal::static_int_cast< sal_uInt16 >(nChildIndex));

    if(pItem != nullptr)
    {
        mpParent->SelectItem( pItem->mnId );
        mpParent->Select ();
    }
    else
        throw lang::IndexOutOfBoundsException();
}


sal_Bool SAL_CALL ValueSetAcc::isAccessibleChildSelected( sal_Int32 nChildIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    ValueSetItem* pItem = getItem (sal::static_int_cast< sal_uInt16 >(nChildIndex));
    bool            bRet = false;

    if (pItem != nullptr)
        bRet = mpParent->IsItemSelected( pItem->mnId );
    else
        throw lang::IndexOutOfBoundsException();

    return bRet;
}


void SAL_CALL ValueSetAcc::clearAccessibleSelection()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    mpParent->SetNoSelection();
}


void SAL_CALL ValueSetAcc::selectAllAccessibleChildren()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    // unsupported due to single selection only
}


sal_Int32 SAL_CALL ValueSetAcc::getSelectedAccessibleChildCount()
    throw (uno::RuntimeException, std::exception)
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


uno::Reference< accessibility::XAccessible > SAL_CALL ValueSetAcc::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
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


void SAL_CALL ValueSetAcc::deselectAccessibleChild( sal_Int32 nChildIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    // Because of the single selection we can reset the whole selection when
    // the specified child is currently selected.
    if (isAccessibleChildSelected(nChildIndex))
        mpParent->SetNoSelection();
}


sal_Int64 SAL_CALL ValueSetAcc::getSomething( const uno::Sequence< sal_Int8 >& rId ) throw( uno::RuntimeException, std::exception )
{
    sal_Int64 nRet;

    if( ( rId.getLength() == 16 ) && ( 0 == memcmp( ValueSetAcc::getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) ) )
        nRet = reinterpret_cast< sal_Int64 >( this );
    else
        nRet = 0;

    return nRet;
}


void SAL_CALL ValueSetAcc::disposing()
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


sal_uInt16 ValueSetAcc::getItemCount() const
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
    ValueSetItem* pItem = nullptr;

    if (HasNoneField())
    {
        if (nIndex == 0)
            // When present the first item is the then always visible none field.
            pItem = mpParent->ImplGetItem (VALUESET_ITEM_NONEITEM);
        else
            // Shift down the index to compensate for the none field.
            nIndex -= 1;
    }
    if (pItem == nullptr)
        pItem = mpParent->ImplGetItem (static_cast<sal_uInt16>(nIndex));

    return pItem;
}


void ValueSetAcc::ThrowIfDisposed()
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

bool ValueSetAcc::HasNoneField() const
{
    assert(mpParent && "ValueSetAcc::HasNoneField called with mpParent==NULL");
    return ((mpParent->GetStyle() & WB_NONEFIELD) != 0);
}

ValueItemAcc::ValueItemAcc( ValueSetItem* pParent, bool bIsTransientChildrenDisabled ) :
    mpParent( pParent ),
    mbIsTransientChildrenDisabled( bIsTransientChildrenDisabled )
{
}

ValueItemAcc::~ValueItemAcc()
{
}

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


void ValueItemAcc::ParentDestroyed()
{
    const ::osl::MutexGuard aGuard( maMutex );
    mpParent = nullptr;
}

namespace
{
    class theValueItemAccUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theValueItemAccUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 >& ValueItemAcc::getUnoTunnelId()
{
    return theValueItemAccUnoTunnelId::get().getSeq();
}


ValueItemAcc* ValueItemAcc::getImplementation( const uno::Reference< uno::XInterface >& rxData )
    throw()
{
    try
    {
        uno::Reference< lang::XUnoTunnel > xUnoTunnel( rxData, uno::UNO_QUERY );
        return( xUnoTunnel.is() ? reinterpret_cast<ValueItemAcc*>(sal::static_int_cast<sal_IntPtr>(xUnoTunnel->getSomething( ValueItemAcc::getUnoTunnelId() ))) : nullptr );
    }
    catch(const css::uno::Exception&)
    {
        return nullptr;
    }
}


uno::Reference< accessibility::XAccessibleContext > SAL_CALL ValueItemAcc::getAccessibleContext()
    throw (uno::RuntimeException, std::exception)
{
    return this;
}


sal_Int32 SAL_CALL ValueItemAcc::getAccessibleChildCount()
    throw (uno::RuntimeException, std::exception)
{
    return 0;
}


uno::Reference< accessibility::XAccessible > SAL_CALL ValueItemAcc::getAccessibleChild( sal_Int32 )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    throw lang::IndexOutOfBoundsException();
}


uno::Reference< accessibility::XAccessible > SAL_CALL ValueItemAcc::getAccessibleParent()
    throw (uno::RuntimeException, std::exception)
{
    const SolarMutexGuard aSolarGuard;
    uno::Reference< accessibility::XAccessible >    xRet;

    if( mpParent )
        xRet = mpParent->mrParent.GetAccessible();

    return xRet;
}


sal_Int32 SAL_CALL ValueItemAcc::getAccessibleIndexInParent()
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
        ValueSetItem* pItem;
        for (sal_uInt16 i=0; i<nCount && !bDone; i++)
        {
            // Guard the retrieval of the i-th child with a try/catch block
            // just in case the number of children changes in the mean time.
            try
            {
                pItem = mpParent->mrParent.ImplGetItem(i);
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

    //if this valueset contain a none field(common value is default), then we should increase the real index and set the noitem index value equal 0.
    if ( mpParent && ( (mpParent->mrParent.GetStyle() & WB_NONEFIELD) != 0 ) )
    {
        ValueSetItem* pFirstItem = mpParent->mrParent.ImplGetItem (VALUESET_ITEM_NONEITEM);
        if( pFirstItem && pFirstItem ->GetAccessible(mbIsTransientChildrenDisabled).get() == this )
            nIndexInParent = 0;
        else
            nIndexInParent++;
    }
    return nIndexInParent;
}


sal_Int16 SAL_CALL ValueItemAcc::getAccessibleRole()
    throw (uno::RuntimeException, std::exception)
{
    return accessibility::AccessibleRole::LIST_ITEM;
}


OUString SAL_CALL ValueItemAcc::getAccessibleDescription()
    throw (uno::RuntimeException, std::exception)
{
    return OUString();
}


OUString SAL_CALL ValueItemAcc::getAccessibleName()
    throw (uno::RuntimeException, std::exception)
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


uno::Reference< accessibility::XAccessibleRelationSet > SAL_CALL ValueItemAcc::getAccessibleRelationSet()
    throw (uno::RuntimeException, std::exception)
{
    return uno::Reference< accessibility::XAccessibleRelationSet >();
}


uno::Reference< accessibility::XAccessibleStateSet > SAL_CALL ValueItemAcc::getAccessibleStateSet()
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
        if( mpParent->mrParent.GetSelectItemId() == mpParent->mnId )
        {
            pStateSet->AddState( accessibility::AccessibleStateType::SELECTED );
            //              pStateSet->AddState( accessibility::AccessibleStateType::FOCUSED );
        }
    }

    return pStateSet;
}


lang::Locale SAL_CALL ValueItemAcc::getLocale()
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


void SAL_CALL ValueItemAcc::addAccessibleEventListener( const uno::Reference< accessibility::XAccessibleEventListener >& rxListener )
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


void SAL_CALL ValueItemAcc::removeAccessibleEventListener( const uno::Reference< accessibility::XAccessibleEventListener >& rxListener )
    throw (uno::RuntimeException, std::exception)
{
    const ::osl::MutexGuard aGuard( maMutex );

    if( rxListener.is() )
    {
        ::std::vector< uno::Reference< accessibility::XAccessibleEventListener > >::iterator aIter =
            std::find(mxEventListeners.begin(), mxEventListeners.end(), rxListener);

        if (aIter != mxEventListeners.end())
            mxEventListeners.erase(aIter);
    }
}


sal_Bool SAL_CALL ValueItemAcc::containsPoint( const awt::Point& aPoint )
    throw (uno::RuntimeException, std::exception)
{
    const awt::Rectangle    aRect( getBounds() );
    const Point             aSize( aRect.Width, aRect.Height );
    const Point             aNullPoint, aTestPoint( aPoint.X, aPoint.Y );

    return Rectangle( aNullPoint, aSize ).IsInside( aTestPoint );
}

uno::Reference< accessibility::XAccessible > SAL_CALL ValueItemAcc::getAccessibleAtPoint( const awt::Point& )
    throw (uno::RuntimeException, std::exception)
{
    uno::Reference< accessibility::XAccessible > xRet;
    return xRet;
}

awt::Rectangle SAL_CALL ValueItemAcc::getBounds()
    throw (uno::RuntimeException, std::exception)
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

awt::Point SAL_CALL ValueItemAcc::getLocation()
    throw (uno::RuntimeException, std::exception)
{
    const awt::Rectangle    aRect( getBounds() );
    awt::Point              aRet;

    aRet.X = aRect.X;
    aRet.Y = aRect.Y;

    return aRet;
}

awt::Point SAL_CALL ValueItemAcc::getLocationOnScreen()
    throw (uno::RuntimeException, std::exception)
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

awt::Size SAL_CALL ValueItemAcc::getSize()
    throw (uno::RuntimeException, std::exception)
{
    const awt::Rectangle    aRect( getBounds() );
    awt::Size               aRet;

    aRet.Width = aRect.Width;
    aRet.Height = aRect.Height;

    return aRet;
}

void SAL_CALL ValueItemAcc::grabFocus()
    throw (uno::RuntimeException, std::exception)
{
    // nothing to do
}

sal_Int32 SAL_CALL ValueItemAcc::getForeground(  )
    throw (uno::RuntimeException, std::exception)
{
    sal_uInt32 nColor = Application::GetSettings().GetStyleSettings().GetWindowTextColor().GetColor();
    return static_cast<sal_Int32>(nColor);
}

sal_Int32 SAL_CALL ValueItemAcc::getBackground(  )
    throw (uno::RuntimeException, std::exception)
{
    sal_uInt32 nColor;
    if (mpParent && mpParent->meType == VALUESETITEM_COLOR)
        nColor = mpParent->maColor.GetColor();
    else
        nColor = Application::GetSettings().GetStyleSettings().GetWindowColor().GetColor();
    return static_cast<sal_Int32>(nColor);
}

sal_Int64 SAL_CALL ValueItemAcc::getSomething( const uno::Sequence< sal_Int8 >& rId ) throw( uno::RuntimeException, std::exception )
{
    sal_Int64 nRet;

    if( ( rId.getLength() == 16 ) && ( 0 == memcmp( ValueItemAcc::getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) ) )
        nRet = reinterpret_cast< sal_Int64 >( this );
    else
        nRet = 0;

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
