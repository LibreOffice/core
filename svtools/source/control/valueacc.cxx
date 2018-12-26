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
#include <sal/log.hxx>
#include <tools/debug.hxx>
#include <svtools/valueset.hxx>
#include "valueimp.hxx"
#include <comphelper/servicehelper.hxx>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
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

SvtValueSetItem::SvtValueSetItem( SvtValueSet& rParent )
    : mrParent(rParent)
    , mnId(0)
    , meType(VALUESETITEM_NONE)
    , mbVisible(true)
    , mxAcc()
{
}


SvtValueSetItem::~SvtValueSetItem()
{
    if( mxAcc.is() )
    {
        static_cast< ValueItemAcc* >( mxAcc.get() )->ParentDestroyed();
    }
}

uno::Reference< accessibility::XAccessible > const & SvtValueSetItem::GetAccessible( bool bIsTransientChildrenDisabled )
{
    if( !mxAcc.is() )
        mxAcc = new SvtValueItemAcc( this, bIsTransientChildrenDisabled );

    return mxAcc;
}

ValueSetAcc::ValueSetAcc( ValueSet* pParent ) :
    ValueSetAccComponentBase (m_aMutex),
    mpParent( pParent ),
    mbIsFocused(false)
{
}


ValueSetAcc::~ValueSetAcc()
{
}


void ValueSetAcc::FireAccessibleEvent( short nEventId, const uno::Any& rOldValue, const uno::Any& rNewValue )
{
    if( !nEventId )
        return;

    ::std::vector< uno::Reference< accessibility::XAccessibleEventListener > >                  aTmpListeners( mxEventListeners );
    accessibility::AccessibleEventObject                                                        aEvtObject;

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
{
    ThrowIfDisposed();
    return this;
}


sal_Int32 SAL_CALL ValueSetAcc::getAccessibleChildCount()
{
    const SolarMutexGuard aSolarGuard;
    ThrowIfDisposed();

    sal_Int32 nCount = mpParent->ImplGetVisibleItemCount();
    if (HasNoneField())
        nCount += 1;
    return nCount;
}


uno::Reference< accessibility::XAccessible > SAL_CALL ValueSetAcc::getAccessibleChild( sal_Int32 i )
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    ValueSetItem* pItem = getItem (sal::static_int_cast< sal_uInt16 >(i));

    if( !pItem )
        throw lang::IndexOutOfBoundsException();

    uno::Reference< accessibility::XAccessible >  xRet = pItem->GetAccessible( false/*bIsTransientChildrenDisabled*/ );
    return xRet;
}


uno::Reference< accessibility::XAccessible > SAL_CALL ValueSetAcc::getAccessibleParent()
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
{
    ThrowIfDisposed();
    return accessibility::AccessibleRole::LIST;
}


OUString SAL_CALL ValueSetAcc::getAccessibleDescription()
{
    ThrowIfDisposed();
    return OUString( "ValueSet" );
}


OUString SAL_CALL ValueSetAcc::getAccessibleName()
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
{
    ThrowIfDisposed();
    ::utl::AccessibleStateSetHelper* pStateSet = new ::utl::AccessibleStateSetHelper();

    // Set some states.
    pStateSet->AddState (accessibility::AccessibleStateType::ENABLED);
    pStateSet->AddState (accessibility::AccessibleStateType::SENSITIVE);
    pStateSet->AddState (accessibility::AccessibleStateType::SHOWING);
    pStateSet->AddState (accessibility::AccessibleStateType::VISIBLE);
    pStateSet->AddState (accessibility::AccessibleStateType::MANAGES_DESCENDANTS);
    pStateSet->AddState (accessibility::AccessibleStateType::FOCUSABLE);
    if (mbIsFocused)
        pStateSet->AddState (accessibility::AccessibleStateType::FOCUSED);

    return pStateSet;
}


lang::Locale SAL_CALL ValueSetAcc::getLocale()
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
{
    ThrowIfDisposed();
    ::osl::MutexGuard aGuard (m_aMutex);

    if( !rxListener.is() )
           return;

    bool bFound = false;

    for (auto const& eventListener : mxEventListeners)
    {
        if(eventListener == rxListener)
        {
            bFound = true;
            break;
        }
    }

    if (!bFound)
        mxEventListeners.push_back( rxListener );
}


void SAL_CALL ValueSetAcc::removeAccessibleEventListener( const uno::Reference< accessibility::XAccessibleEventListener >& rxListener )
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
{
    ThrowIfDisposed();
    const awt::Rectangle    aRect( getBounds() );
    const Point             aSize( aRect.Width, aRect.Height );
    const Point             aNullPoint, aTestPoint( aPoint.X, aPoint.Y );

    return tools::Rectangle( aNullPoint, aSize ).IsInside( aTestPoint );
}


uno::Reference< accessibility::XAccessible > SAL_CALL ValueSetAcc::getAccessibleAtPoint( const awt::Point& aPoint )
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
            ValueSetItem *const pItem = mpParent->mItemList[nItemPos].get();
            xRet = pItem->GetAccessible( false/*bIsTransientChildrenDisabled*/ );
        }
    }

    return xRet;
}


awt::Rectangle SAL_CALL ValueSetAcc::getBounds()
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
{
    ThrowIfDisposed();
    const awt::Rectangle    aRect( getBounds() );
    awt::Point              aRet;

    aRet.X = aRect.X;
    aRet.Y = aRect.Y;

    return aRet;
}


awt::Point SAL_CALL ValueSetAcc::getLocationOnScreen()
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
{
    ThrowIfDisposed();
    const awt::Rectangle    aRect( getBounds() );
    awt::Size               aRet;

    aRet.Width = aRect.Width;
    aRet.Height = aRect.Height;

    return aRet;
}

void SAL_CALL ValueSetAcc::grabFocus()
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    mpParent->GrabFocus();
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

void SAL_CALL ValueSetAcc::selectAccessibleChild( sal_Int32 nChildIndex )
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    ValueSetItem* pItem = getItem (sal::static_int_cast< sal_uInt16 >(nChildIndex));

    if(pItem == nullptr)
        throw lang::IndexOutOfBoundsException();

    mpParent->SelectItem( pItem->mnId );
    mpParent->Select ();
}


sal_Bool SAL_CALL ValueSetAcc::isAccessibleChildSelected( sal_Int32 nChildIndex )
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    ValueSetItem* pItem = getItem (sal::static_int_cast< sal_uInt16 >(nChildIndex));

    if (pItem == nullptr)
        throw lang::IndexOutOfBoundsException();

    bool  bRet = mpParent->IsItemSelected( pItem->mnId );
    return bRet;
}


void SAL_CALL ValueSetAcc::clearAccessibleSelection()
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    mpParent->SetNoSelection();
}


void SAL_CALL ValueSetAcc::selectAllAccessibleChildren()
{
    ThrowIfDisposed();
    // unsupported due to single selection only
}


sal_Int32 SAL_CALL ValueSetAcc::getSelectedAccessibleChildCount()
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
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    uno::Reference< accessibility::XAccessible >    xRet;

    for( sal_uInt16 i = 0, nCount = getItemCount(), nSel = 0; ( i < nCount ) && !xRet.is(); i++ )
    {
        ValueSetItem* pItem = getItem(i);

        if( pItem && mpParent->IsItemSelected( pItem->mnId ) && ( nSelectedChildIndex == static_cast< sal_Int32 >( nSel++ ) ) )
            xRet = pItem->GetAccessible( false/*bIsTransientChildrenDisabled*/ );
    }

    return xRet;
}


void SAL_CALL ValueSetAcc::deselectAccessibleChild( sal_Int32 nChildIndex )
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    // Because of the single selection we can reset the whole selection when
    // the specified child is currently selected.
    if (isAccessibleChildSelected(nChildIndex))
        mpParent->SetNoSelection();
}


sal_Int64 SAL_CALL ValueSetAcc::getSomething( const uno::Sequence< sal_Int8 >& rId )
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
    lang::EventObject aEvent (static_cast<accessibility::XAccessible*>(this));
    for (auto const& listenerCopy : aListenerListCopy)
    {
        try
        {
            listenerCopy->disposing (aEvent);
        }
        catch(const uno::Exception&)
        {
            // Ignore exceptions.
        }
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
        pItem = mpParent->ImplGetItem (nIndex);

    return pItem;
}


void ValueSetAcc::ThrowIfDisposed()
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        SAL_WARN("svx", "Calling disposed object. Throwing exception:");
        throw lang::DisposedException (
            "object has been already disposed",
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
    if( !nEventId )
        return;

    ::std::vector< uno::Reference< accessibility::XAccessibleEventListener > >                  aTmpListeners( mxEventListeners );
    accessibility::AccessibleEventObject                                                        aEvtObject;

    aEvtObject.EventId = nEventId;
    aEvtObject.Source = static_cast<uno::XWeak*>(this);
    aEvtObject.NewValue = rNewValue;
    aEvtObject.OldValue = rOldValue;

    for (auto const& tmpListener : aTmpListeners)
    {
        tmpListener->notifyEvent( aEvtObject );
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
{
    return this;
}


sal_Int32 SAL_CALL ValueItemAcc::getAccessibleChildCount()
{
    return 0;
}


uno::Reference< accessibility::XAccessible > SAL_CALL ValueItemAcc::getAccessibleChild( sal_Int32 )
{
    throw lang::IndexOutOfBoundsException();
}


uno::Reference< accessibility::XAccessible > SAL_CALL ValueItemAcc::getAccessibleParent()
{
    const SolarMutexGuard aSolarGuard;
    uno::Reference< accessibility::XAccessible >    xRet;

    if( mpParent )
        xRet = mpParent->mrParent.GetAccessible();

    return xRet;
}


sal_Int32 SAL_CALL ValueItemAcc::getAccessibleIndexInParent()
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

    if( mpParent )
    {
        if (mpParent->maText.isEmpty())
            return "Item " +  OUString::number(static_cast<sal_Int32>(mpParent->mnId));
        else
            return mpParent->maText;
    }

    return OUString();
}


uno::Reference< accessibility::XAccessibleRelationSet > SAL_CALL ValueItemAcc::getAccessibleRelationSet()
{
    return uno::Reference< accessibility::XAccessibleRelationSet >();
}


uno::Reference< accessibility::XAccessibleStateSet > SAL_CALL ValueItemAcc::getAccessibleStateSet()
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
        if( mpParent->mrParent.GetSelectedItemId() == mpParent->mnId )
        {
            pStateSet->AddState( accessibility::AccessibleStateType::SELECTED );
            //              pStateSet->AddState( accessibility::AccessibleStateType::FOCUSED );
        }
    }

    return pStateSet;
}


lang::Locale SAL_CALL ValueItemAcc::getLocale()
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
{
    const ::osl::MutexGuard aGuard( maMutex );

    if( !rxListener.is() )
           return;

    bool bFound = false;

    for (auto const& eventListener : mxEventListeners)
    {
        if(eventListener == rxListener)
        {
            bFound = true;
            break;
        }
    }

    if (!bFound)
        mxEventListeners.push_back( rxListener );
}


void SAL_CALL ValueItemAcc::removeAccessibleEventListener( const uno::Reference< accessibility::XAccessibleEventListener >& rxListener )
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
{
    const awt::Rectangle    aRect( getBounds() );
    const Point             aSize( aRect.Width, aRect.Height );
    const Point             aNullPoint, aTestPoint( aPoint.X, aPoint.Y );

    return tools::Rectangle( aNullPoint, aSize ).IsInside( aTestPoint );
}

uno::Reference< accessibility::XAccessible > SAL_CALL ValueItemAcc::getAccessibleAtPoint( const awt::Point& )
{
    uno::Reference< accessibility::XAccessible > xRet;
    return xRet;
}

awt::Rectangle SAL_CALL ValueItemAcc::getBounds()
{
    const SolarMutexGuard aSolarGuard;
    awt::Rectangle      aRet;

    if( mpParent )
    {
        tools::Rectangle   aRect( mpParent->mrParent.GetItemRect(mpParent->mnId) );
        tools::Rectangle   aParentRect( Point(), mpParent->mrParent.GetOutputSizePixel() );

        aRect.Intersection( aParentRect );

        aRet.X = aRect.Left();
        aRet.Y = aRect.Top();
        aRet.Width = aRect.GetWidth();
        aRet.Height = aRect.GetHeight();
    }

    return aRet;
}

awt::Point SAL_CALL ValueItemAcc::getLocation()
{
    const awt::Rectangle    aRect( getBounds() );
    awt::Point              aRet;

    aRet.X = aRect.X;
    aRet.Y = aRect.Y;

    return aRet;
}

awt::Point SAL_CALL ValueItemAcc::getLocationOnScreen()
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
{
    const awt::Rectangle    aRect( getBounds() );
    awt::Size               aRet;

    aRet.Width = aRect.Width;
    aRet.Height = aRect.Height;

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
    if (mpParent && mpParent->meType == VALUESETITEM_COLOR)
        nColor = mpParent->maColor;
    else
        nColor = Application::GetSettings().GetStyleSettings().GetWindowColor();
    return static_cast<sal_Int32>(nColor);
}

sal_Int64 SAL_CALL ValueItemAcc::getSomething( const uno::Sequence< sal_Int8 >& rId )
{
    sal_Int64 nRet;

    if( ( rId.getLength() == 16 ) && ( 0 == memcmp( ValueItemAcc::getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) ) )
        nRet = reinterpret_cast< sal_Int64 >( this );
    else
        nRet = 0;

    return nRet;
}

SvtValueItemAcc::SvtValueItemAcc( SvtValueSetItem* pParent, bool bIsTransientChildrenDisabled ) :
    mpParent( pParent ),
    mbIsTransientChildrenDisabled( bIsTransientChildrenDisabled )
{
}

SvtValueItemAcc::~SvtValueItemAcc()
{
}

namespace
{
    class theSvtValueItemAccUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSvtValueItemAccUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 >& SvtValueItemAcc::getUnoTunnelId()
{
    return theSvtValueItemAccUnoTunnelId::get().getSeq();
}


SvtValueItemAcc* SvtValueItemAcc::getImplementation( const uno::Reference< uno::XInterface >& rxData )
    throw()
{
    try
    {
        uno::Reference< lang::XUnoTunnel > xUnoTunnel( rxData, uno::UNO_QUERY );
        return( xUnoTunnel.is() ? reinterpret_cast<SvtValueItemAcc*>(sal::static_int_cast<sal_IntPtr>(xUnoTunnel->getSomething( SvtValueItemAcc::getUnoTunnelId() ))) : nullptr );
    }
    catch(const css::uno::Exception&)
    {
        return nullptr;
    }
}


uno::Reference< accessibility::XAccessibleContext > SAL_CALL SvtValueItemAcc::getAccessibleContext()
{
    return this;
}


sal_Int32 SAL_CALL SvtValueItemAcc::getAccessibleChildCount()
{
    return 0;
}


uno::Reference< accessibility::XAccessible > SAL_CALL SvtValueItemAcc::getAccessibleChild( sal_Int32 )
{
    throw lang::IndexOutOfBoundsException();
}


uno::Reference< accessibility::XAccessible > SAL_CALL SvtValueItemAcc::getAccessibleParent()
{
    const SolarMutexGuard aSolarGuard;
    uno::Reference< accessibility::XAccessible >    xRet;

    if( mpParent )
        xRet = mpParent->mrParent.mxAccessible;

    return xRet;
}


sal_Int32 SAL_CALL SvtValueItemAcc::getAccessibleIndexInParent()
{
    const SolarMutexGuard aSolarGuard;
    // The index defaults to -1 to indicate the child does not belong to its
    // parent.
    sal_Int32 nIndexInParent = -1;

    if( mpParent )
    {
        bool bDone = false;

        sal_uInt16 nCount = mpParent->mrParent.ImplGetVisibleItemCount();
        SvtValueSetItem* pItem;
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
        SvtValueSetItem* pFirstItem = mpParent->mrParent.ImplGetItem (VALUESET_ITEM_NONEITEM);
        if( pFirstItem && pFirstItem ->GetAccessible(mbIsTransientChildrenDisabled).get() == this )
            nIndexInParent = 0;
        else
            nIndexInParent++;
    }
    return nIndexInParent;
}


sal_Int16 SAL_CALL SvtValueItemAcc::getAccessibleRole()
{
    return accessibility::AccessibleRole::LIST_ITEM;
}


OUString SAL_CALL SvtValueItemAcc::getAccessibleDescription()
{
    return OUString();
}


OUString SAL_CALL SvtValueItemAcc::getAccessibleName()
{
    const SolarMutexGuard aSolarGuard;

    if( mpParent )
    {
        if (mpParent->maText.isEmpty())
            return "Item " +  OUString::number(static_cast<sal_Int32>(mpParent->mnId));
        else
            return mpParent->maText;
    }

    return OUString();
}


uno::Reference< accessibility::XAccessibleRelationSet > SAL_CALL SvtValueItemAcc::getAccessibleRelationSet()
{
    return uno::Reference< accessibility::XAccessibleRelationSet >();
}


uno::Reference< accessibility::XAccessibleStateSet > SAL_CALL SvtValueItemAcc::getAccessibleStateSet()
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
        if( mpParent->mrParent.GetSelectedItemId() == mpParent->mnId )
        {
            pStateSet->AddState( accessibility::AccessibleStateType::SELECTED );
            //              pStateSet->AddState( accessibility::AccessibleStateType::FOCUSED );
        }
    }

    return pStateSet;
}


lang::Locale SAL_CALL SvtValueItemAcc::getLocale()
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


void SAL_CALL SvtValueItemAcc::addAccessibleEventListener( const uno::Reference< accessibility::XAccessibleEventListener >& rxListener )
{
    const ::osl::MutexGuard aGuard( maMutex );

    if( !rxListener.is() )
           return;

    bool bFound = false;

    for (auto const& eventListener : mxEventListeners)
    {
        if(eventListener == rxListener)
        {
            bFound = true;
            break;
        }
    }

    if (!bFound)
        mxEventListeners.push_back( rxListener );
}


void SAL_CALL SvtValueItemAcc::removeAccessibleEventListener( const uno::Reference< accessibility::XAccessibleEventListener >& rxListener )
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


sal_Bool SAL_CALL SvtValueItemAcc::containsPoint( const awt::Point& aPoint )
{
    const awt::Rectangle    aRect( getBounds() );
    const Point             aSize( aRect.Width, aRect.Height );
    const Point             aNullPoint, aTestPoint( aPoint.X, aPoint.Y );

    return tools::Rectangle( aNullPoint, aSize ).IsInside( aTestPoint );
}

uno::Reference< accessibility::XAccessible > SAL_CALL SvtValueItemAcc::getAccessibleAtPoint( const awt::Point& )
{
    uno::Reference< accessibility::XAccessible > xRet;
    return xRet;
}

awt::Rectangle SAL_CALL SvtValueItemAcc::getBounds()
{
    const SolarMutexGuard aSolarGuard;
    awt::Rectangle      aRet;

    if( mpParent )
    {
        tools::Rectangle   aRect( mpParent->mrParent.GetItemRect(mpParent->mnId) );
        tools::Rectangle   aParentRect( Point(), mpParent->mrParent.GetOutputSizePixel() );

        aRect.Intersection( aParentRect );

        aRet.X = aRect.Left();
        aRet.Y = aRect.Top();
        aRet.Width = aRect.GetWidth();
        aRet.Height = aRect.GetHeight();
    }

    return aRet;
}

awt::Point SAL_CALL SvtValueItemAcc::getLocation()
{
    const awt::Rectangle    aRect( getBounds() );
    awt::Point              aRet;

    aRet.X = aRect.X;
    aRet.Y = aRect.Y;

    return aRet;
}

awt::Point SAL_CALL SvtValueItemAcc::getLocationOnScreen()
{
    const SolarMutexGuard aSolarGuard;
    awt::Point          aRet;

    if( mpParent )
    {
        const Point aPos = mpParent->mrParent.GetItemRect(mpParent->mnId).TopLeft();
        const Point aScreenPos( mpParent->mrParent.GetDrawingArea()->get_accessible_location() );

        aRet.X = aPos.X() + aScreenPos.X();
        aRet.Y = aPos.Y() + aScreenPos.Y();
    }

    return aRet;
}

awt::Size SAL_CALL SvtValueItemAcc::getSize()
{
    const awt::Rectangle    aRect( getBounds() );
    awt::Size               aRet;

    aRet.Width = aRect.Width;
    aRet.Height = aRect.Height;

    return aRet;
}

void SAL_CALL SvtValueItemAcc::grabFocus()
{
    // nothing to do
}

sal_Int32 SAL_CALL SvtValueItemAcc::getForeground(  )
{
    Color nColor = Application::GetSettings().GetStyleSettings().GetWindowTextColor();
    return static_cast<sal_Int32>(nColor);
}

sal_Int32 SAL_CALL SvtValueItemAcc::getBackground(  )
{
    Color nColor;
    if (mpParent && mpParent->meType == VALUESETITEM_COLOR)
        nColor = mpParent->maColor;
    else
        nColor = Application::GetSettings().GetStyleSettings().GetWindowColor();
    return static_cast<sal_Int32>(nColor);
}

sal_Int64 SAL_CALL SvtValueItemAcc::getSomething( const uno::Sequence< sal_Int8 >& rId )
{
    sal_Int64 nRet;

    if( ( rId.getLength() == 16 ) && ( 0 == memcmp( SvtValueItemAcc::getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) ) )
        nRet = reinterpret_cast< sal_Int64 >( this );
    else
        nRet = 0;

    return nRet;
}

void SvtValueItemAcc::FireAccessibleEvent( short nEventId, const uno::Any& rOldValue, const uno::Any& rNewValue )
{
    if( !nEventId )
        return;

    ::std::vector< uno::Reference< accessibility::XAccessibleEventListener > >                  aTmpListeners( mxEventListeners );
    accessibility::AccessibleEventObject                                                        aEvtObject;

    aEvtObject.EventId = nEventId;
    aEvtObject.Source = static_cast<uno::XWeak*>(this);
    aEvtObject.NewValue = rNewValue;
    aEvtObject.OldValue = rOldValue;

    for (auto const& tmpListener : aTmpListeners)
    {
        tmpListener->notifyEvent( aEvtObject );
    }
}

SvtValueSetAcc::SvtValueSetAcc( SvtValueSet* pParent ) :
    ValueSetAccComponentBase (m_aMutex),
    mpParent( pParent ),
    mbIsFocused(false)
{
}


SvtValueSetAcc::~SvtValueSetAcc()
{
}


void SvtValueSetAcc::FireAccessibleEvent( short nEventId, const uno::Any& rOldValue, const uno::Any& rNewValue )
{
    if( !nEventId )
        return;

    ::std::vector< uno::Reference< accessibility::XAccessibleEventListener > >                  aTmpListeners( mxEventListeners );
    accessibility::AccessibleEventObject                                                        aEvtObject;

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
    class theSvtValueSetAccUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSvtValueSetAccUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 >& SvtValueSetAcc::getUnoTunnelId()
{
    return theSvtValueSetAccUnoTunnelId::get().getSeq();
}


SvtValueSetAcc* SvtValueSetAcc::getImplementation( const uno::Reference< uno::XInterface >& rxData )
    throw()
{
    try
    {
        uno::Reference< lang::XUnoTunnel > xUnoTunnel( rxData, uno::UNO_QUERY );
        return( xUnoTunnel.is() ? reinterpret_cast<SvtValueSetAcc*>(sal::static_int_cast<sal_IntPtr>(xUnoTunnel->getSomething( SvtValueSetAcc::getUnoTunnelId() ))) : nullptr );
    }
    catch(const css::uno::Exception&)
    {
        return nullptr;
    }
}


void SvtValueSetAcc::GetFocus()
{
    mbIsFocused = true;

    // Broadcast the state change.
    css::uno::Any aOldState, aNewState;
    aNewState <<= css::accessibility::AccessibleStateType::FOCUSED;
    FireAccessibleEvent(
        css::accessibility::AccessibleEventId::STATE_CHANGED,
        aOldState, aNewState);
}


void SvtValueSetAcc::LoseFocus()
{
    mbIsFocused = false;

    // Broadcast the state change.
    css::uno::Any aOldState, aNewState;
    aOldState <<= css::accessibility::AccessibleStateType::FOCUSED;
    FireAccessibleEvent(
        css::accessibility::AccessibleEventId::STATE_CHANGED,
        aOldState, aNewState);
}


uno::Reference< accessibility::XAccessibleContext > SAL_CALL SvtValueSetAcc::getAccessibleContext()
{
    ThrowIfDisposed();
    return this;
}


sal_Int32 SAL_CALL SvtValueSetAcc::getAccessibleChildCount()
{
    const SolarMutexGuard aSolarGuard;
    ThrowIfDisposed();

    sal_Int32 nCount = mpParent->ImplGetVisibleItemCount();
    if (HasNoneField())
        nCount += 1;
    return nCount;
}


uno::Reference< accessibility::XAccessible > SAL_CALL SvtValueSetAcc::getAccessibleChild( sal_Int32 i )
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    SvtValueSetItem* pItem = getItem (sal::static_int_cast< sal_uInt16 >(i));

    if( !pItem )
        throw lang::IndexOutOfBoundsException();

    uno::Reference< accessibility::XAccessible >  xRet = pItem->GetAccessible( false/*bIsTransientChildrenDisabled*/ );
    return xRet;
}

uno::Reference< accessibility::XAccessible > SAL_CALL SvtValueSetAcc::getAccessibleParent()
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    return mpParent->GetDrawingArea()->get_accessible_parent();
}

sal_Int32 SAL_CALL SvtValueSetAcc::getAccessibleIndexInParent()
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
        OSL_FAIL( "OAccessibleContextHelper::getAccessibleIndexInParent: caught an exception!" );
    }

    return nRet;
}

sal_Int16 SAL_CALL SvtValueSetAcc::getAccessibleRole()
{
    ThrowIfDisposed();
    return accessibility::AccessibleRole::LIST;
}


OUString SAL_CALL SvtValueSetAcc::getAccessibleDescription()
{
    ThrowIfDisposed();
    return OUString( "ValueSet" );
}


OUString SAL_CALL SvtValueSetAcc::getAccessibleName()
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

uno::Reference< accessibility::XAccessibleRelationSet > SAL_CALL SvtValueSetAcc::getAccessibleRelationSet()
{
    ThrowIfDisposed();
    SolarMutexGuard g;
    return mpParent->GetDrawingArea()->get_accessible_relation_set();
}

uno::Reference< accessibility::XAccessibleStateSet > SAL_CALL SvtValueSetAcc::getAccessibleStateSet()
{
    ThrowIfDisposed();
    ::utl::AccessibleStateSetHelper* pStateSet = new ::utl::AccessibleStateSetHelper();

    // Set some states.
    pStateSet->AddState (accessibility::AccessibleStateType::ENABLED);
    pStateSet->AddState (accessibility::AccessibleStateType::SENSITIVE);
    pStateSet->AddState (accessibility::AccessibleStateType::SHOWING);
    pStateSet->AddState (accessibility::AccessibleStateType::VISIBLE);
    pStateSet->AddState (accessibility::AccessibleStateType::MANAGES_DESCENDANTS);
    pStateSet->AddState (accessibility::AccessibleStateType::FOCUSABLE);
    if (mbIsFocused)
        pStateSet->AddState (accessibility::AccessibleStateType::FOCUSED);

    return pStateSet;
}


lang::Locale SAL_CALL SvtValueSetAcc::getLocale()
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


void SAL_CALL SvtValueSetAcc::addAccessibleEventListener( const uno::Reference< accessibility::XAccessibleEventListener >& rxListener )
{
    ThrowIfDisposed();
    ::osl::MutexGuard aGuard (m_aMutex);

    if( !rxListener.is() )
           return;

    bool bFound = false;

    for (auto const& eventListener : mxEventListeners)
    {
        if(eventListener == rxListener)
        {
            bFound = true;
            break;
        }
    }

    if (!bFound)
        mxEventListeners.push_back( rxListener );
}


void SAL_CALL SvtValueSetAcc::removeAccessibleEventListener( const uno::Reference< accessibility::XAccessibleEventListener >& rxListener )
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


sal_Bool SAL_CALL SvtValueSetAcc::containsPoint( const awt::Point& aPoint )
{
    ThrowIfDisposed();
    const awt::Rectangle    aRect( getBounds() );
    const Point             aSize( aRect.Width, aRect.Height );
    const Point             aNullPoint, aTestPoint( aPoint.X, aPoint.Y );

    return tools::Rectangle( aNullPoint, aSize ).IsInside( aTestPoint );
}


uno::Reference< accessibility::XAccessible > SAL_CALL SvtValueSetAcc::getAccessibleAtPoint( const awt::Point& aPoint )
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
            SvtValueSetItem *const pItem = mpParent->mItemList[nItemPos].get();
            xRet = pItem->GetAccessible( false/*bIsTransientChildrenDisabled*/ );
        }
    }

    return xRet;
}


awt::Rectangle SAL_CALL SvtValueSetAcc::getBounds()
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


awt::Point SAL_CALL SvtValueSetAcc::getLocation()
{
    ThrowIfDisposed();
    const awt::Rectangle    aRect( getBounds() );
    awt::Point              aRet;

    aRet.X = aRect.X;
    aRet.Y = aRect.Y;

    return aRet;
}


awt::Point SAL_CALL SvtValueSetAcc::getLocationOnScreen()
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    awt::Point aScreenLoc(0, 0);

    uno::Reference<accessibility::XAccessible> xParent(getAccessibleParent());
    if (xParent)
    {
        uno::Reference<accessibility::XAccessibleContext> xParentContext(xParent->getAccessibleContext());
        uno::Reference<accessibility::XAccessibleComponent> xParentComponent(xParentContext, css::uno::UNO_QUERY);
        OSL_ENSURE( xParentComponent.is(), "SvtValueSetAcc::getLocationOnScreen: no parent component!" );
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


awt::Size SAL_CALL SvtValueSetAcc::getSize()
{
    ThrowIfDisposed();
    const awt::Rectangle    aRect( getBounds() );
    awt::Size               aRet;

    aRet.Width = aRect.Width;
    aRet.Height = aRect.Height;

    return aRet;
}

void SAL_CALL SvtValueSetAcc::grabFocus()
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    mpParent->GrabFocus();
}

sal_Int32 SAL_CALL SvtValueSetAcc::getForeground(  )
{
    ThrowIfDisposed();
    Color nColor = Application::GetSettings().GetStyleSettings().GetWindowTextColor();
    return static_cast<sal_Int32>(nColor);
}

sal_Int32 SAL_CALL SvtValueSetAcc::getBackground(  )
{
    ThrowIfDisposed();
    Color nColor = Application::GetSettings().GetStyleSettings().GetWindowColor();
    return static_cast<sal_Int32>(nColor);
}

void SAL_CALL SvtValueSetAcc::selectAccessibleChild( sal_Int32 nChildIndex )
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    SvtValueSetItem* pItem = getItem (sal::static_int_cast< sal_uInt16 >(nChildIndex));

    if(pItem == nullptr)
        throw lang::IndexOutOfBoundsException();

    mpParent->SelectItem( pItem->mnId );
}


sal_Bool SAL_CALL SvtValueSetAcc::isAccessibleChildSelected( sal_Int32 nChildIndex )
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    SvtValueSetItem* pItem = getItem (sal::static_int_cast< sal_uInt16 >(nChildIndex));

    if (pItem == nullptr)
        throw lang::IndexOutOfBoundsException();

    bool  bRet = mpParent->IsItemSelected( pItem->mnId );
    return bRet;
}


void SAL_CALL SvtValueSetAcc::clearAccessibleSelection()
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    mpParent->SetNoSelection();
}


void SAL_CALL SvtValueSetAcc::selectAllAccessibleChildren()
{
    ThrowIfDisposed();
    // unsupported due to single selection only
}


sal_Int32 SAL_CALL SvtValueSetAcc::getSelectedAccessibleChildCount()
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    sal_Int32           nRet = 0;

    for( sal_uInt16 i = 0, nCount = getItemCount(); i < nCount; i++ )
    {
        SvtValueSetItem* pItem = getItem (i);

        if( pItem && mpParent->IsItemSelected( pItem->mnId ) )
            ++nRet;
    }

    return nRet;
}


uno::Reference< accessibility::XAccessible > SAL_CALL SvtValueSetAcc::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    uno::Reference< accessibility::XAccessible >    xRet;

    for( sal_uInt16 i = 0, nCount = getItemCount(), nSel = 0; ( i < nCount ) && !xRet.is(); i++ )
    {
        SvtValueSetItem* pItem = getItem(i);

        if( pItem && mpParent->IsItemSelected( pItem->mnId ) && ( nSelectedChildIndex == static_cast< sal_Int32 >( nSel++ ) ) )
            xRet = pItem->GetAccessible( false/*bIsTransientChildrenDisabled*/ );
    }

    return xRet;
}


void SAL_CALL SvtValueSetAcc::deselectAccessibleChild( sal_Int32 nChildIndex )
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    // Because of the single selection we can reset the whole selection when
    // the specified child is currently selected.
    if (isAccessibleChildSelected(nChildIndex))
        mpParent->SetNoSelection();
}


sal_Int64 SAL_CALL SvtValueSetAcc::getSomething( const uno::Sequence< sal_Int8 >& rId )
{
    sal_Int64 nRet;

    if( ( rId.getLength() == 16 ) && ( 0 == memcmp( SvtValueSetAcc::getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) ) )
        nRet = reinterpret_cast< sal_Int64 >( this );
    else
        nRet = 0;

    return nRet;
}


void SAL_CALL SvtValueSetAcc::disposing()
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
    for (auto const& listenerCopy : aListenerListCopy)
    {
        try
        {
            listenerCopy->disposing (aEvent);
        }
        catch(const uno::Exception&)
        {
            // Ignore exceptions.
        }
    }
}


sal_uInt16 SvtValueSetAcc::getItemCount() const
{
    sal_uInt16 nCount = mpParent->ImplGetVisibleItemCount();
    // When the None-Item is visible then increase the number of items by
    // one.
    if (HasNoneField())
        nCount += 1;
    return nCount;
}

SvtValueSetItem* SvtValueSetAcc::getItem (sal_uInt16 nIndex) const
{
    SvtValueSetItem* pItem = nullptr;

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
        pItem = mpParent->ImplGetItem (nIndex);

    return pItem;
}


void SvtValueSetAcc::ThrowIfDisposed()
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        SAL_WARN("svx", "Calling disposed object. Throwing exception:");
        throw lang::DisposedException (
            "object has been already disposed",
            static_cast<uno::XWeak*>(this));
    }
    else
    {
        DBG_ASSERT (mpParent!=nullptr, "SvtValueSetAcc not disposed but mpParent == NULL");
    }
}

bool SvtValueSetAcc::HasNoneField() const
{
    assert(mpParent && "SvtValueSetAcc::HasNoneField called with mpParent==NULL");
    return ((mpParent->GetStyle() & WB_NONEFIELD) != 0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
