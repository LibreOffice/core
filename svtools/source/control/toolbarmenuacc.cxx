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


#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

#include <unotools/accessiblestatesethelper.hxx>

#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <tools/debug.hxx>

#include <svtools/framestatuslistener.hxx>
#include <svtools/toolbarmenu.hxx>

#include "toolbarmenuimp.hxx"


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;

namespace svtools {


ToolbarMenuAcc::ToolbarMenuAcc( ToolbarMenu_Impl& rParent )
: ToolbarMenuAccComponentBase(m_aMutex)
, mpParent( &rParent )
, mbIsFocused(false)
{
    mpParent->mrMenu.AddEventListener( LINK( this, ToolbarMenuAcc, WindowEventListener ) );
}


ToolbarMenuAcc::~ToolbarMenuAcc()
{
    if( mpParent )
        mpParent->mrMenu.RemoveEventListener( LINK( this, ToolbarMenuAcc, WindowEventListener ) );
}


IMPL_LINK( ToolbarMenuAcc, WindowEventListener, VclWindowEvent&, rEvent, void )
{
    /* Ignore VclEventId::WindowEndPopupMode, because the UNO accessibility wrapper
     * might have been destroyed by the previous VCLEventListener (if no AT tool
     * is running), e.g. sub-toolbars in impress.
     */
    if ( !mpParent || (rEvent.GetId() == VclEventId::WindowEndPopupMode) )
        return;
    DBG_ASSERT( rEvent.GetWindow(), "Window???" );
    if( rEvent.GetWindow()->IsAccessibilityEventsSuppressed() && ( rEvent.GetId() != VclEventId::ObjectDying ) )
        return;

    switch ( rEvent.GetId() )
    {
        case VclEventId::ObjectDying:
        {
            mpParent->mrMenu.RemoveEventListener( LINK( this, ToolbarMenuAcc, WindowEventListener ) );
            mpParent = nullptr;
        }
        break;

        case VclEventId::WindowGetFocus:
        {
            if( !mbIsFocused )
            {
                mpParent->notifyHighlightedEntry();
                mbIsFocused = true;
            }
        }
        break;
        case VclEventId::WindowLoseFocus:
        {
            if( mbIsFocused )
            {
                mbIsFocused = false;
            }
        }
        break;
        default:
        {
        }
        break;
    }
}


void ToolbarMenuAcc::FireAccessibleEvent( short nEventId, const Any& rOldValue, const Any& rNewValue )
{
    if( !nEventId )
        return;

    EventListenerVector                  aTmpListeners( mxEventListeners );
    AccessibleEventObject aEvtObject;

    aEvtObject.EventId = nEventId;
    aEvtObject.Source = static_cast<XWeak*>(this);
    aEvtObject.NewValue = rNewValue;
    aEvtObject.OldValue = rOldValue;

    for (auto const& tmpListener : aTmpListeners)
    {
        try
        {
            tmpListener->notifyEvent( aEvtObject );
        }
        catch( Exception& )
        {
        }
    }
}


Reference< XAccessibleContext > SAL_CALL ToolbarMenuAcc::getAccessibleContext()
{
    ThrowIfDisposed();
    return this;
}


sal_Int32 SAL_CALL ToolbarMenuAcc::getAccessibleChildCount()
{
    const SolarMutexGuard aSolarGuard;
    ThrowIfDisposed();

    return mpParent->getAccessibleChildCount();
}


Reference< XAccessible > SAL_CALL ToolbarMenuAcc::getAccessibleChild( sal_Int32 i )
{
    const SolarMutexGuard aSolarGuard;
    ThrowIfDisposed();

    return mpParent->getAccessibleChild(i);
}


Reference< XAccessible > SAL_CALL ToolbarMenuAcc::getAccessibleParent()
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;

    Reference< XAccessible > xRet;

    vcl::Window* pParent = mpParent->mrMenu.GetParent();
    if( pParent )
        xRet = pParent->GetAccessible();

    return xRet;
}


sal_Int32 SAL_CALL ToolbarMenuAcc::getAccessibleIndexInParent()
{
    const SolarMutexGuard aSolarGuard;
    ThrowIfDisposed();

    vcl::Window* pParent = mpParent->mrMenu.GetParent();
    if( pParent )
    {
        for( sal_uInt16 i = 0, nCount = pParent->GetChildCount(); i < nCount ; i++ )
        {
            if( pParent->GetChild( i ) == &mpParent->mrMenu )
                return i;
        }
    }

    return 0;
}


sal_Int16 SAL_CALL ToolbarMenuAcc::getAccessibleRole()
{
    ThrowIfDisposed();
    return AccessibleRole::LIST;
}


OUString SAL_CALL ToolbarMenuAcc::getAccessibleDescription()
{
    ThrowIfDisposed();
    return OUString( "ToolbarMenu" );
}


OUString SAL_CALL ToolbarMenuAcc::getAccessibleName()
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    OUString aRet;

    if( mpParent )
        aRet = mpParent->mrMenu.GetAccessibleName();

    if( aRet.isEmpty() )
    {
        vcl::Window* pLabel = mpParent->mrMenu.GetAccessibleRelationLabeledBy();
        if( pLabel && pLabel != &mpParent->mrMenu )
            aRet = OutputDevice::GetNonMnemonicString( pLabel->GetText() );
    }

    return aRet;
}


Reference< XAccessibleRelationSet > SAL_CALL ToolbarMenuAcc::getAccessibleRelationSet()
{
    ThrowIfDisposed();
    return Reference< XAccessibleRelationSet >();
}


Reference< XAccessibleStateSet > SAL_CALL ToolbarMenuAcc::getAccessibleStateSet()
{
    ThrowIfDisposed();
    ::utl::AccessibleStateSetHelper* pStateSet = new ::utl::AccessibleStateSetHelper();

    // Set some states.
    pStateSet->AddState (AccessibleStateType::ENABLED);
    pStateSet->AddState (AccessibleStateType::SENSITIVE);
    pStateSet->AddState (AccessibleStateType::SHOWING);
    pStateSet->AddState (AccessibleStateType::VISIBLE);
    pStateSet->AddState (AccessibleStateType::MANAGES_DESCENDANTS);
    pStateSet->AddState (AccessibleStateType::FOCUSABLE);
    if (mbIsFocused)
        pStateSet->AddState (AccessibleStateType::FOCUSED);

    return pStateSet;
}


Locale SAL_CALL ToolbarMenuAcc::getLocale()
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    Reference< XAccessible > xParent( getAccessibleParent() );
    Locale aRet( "", "", "" );

    if( xParent.is() )
    {
        Reference< XAccessibleContext > xParentContext( xParent->getAccessibleContext() );

        if( xParentContext.is() )
            aRet = xParentContext->getLocale ();
    }

    return aRet;
}


void SAL_CALL ToolbarMenuAcc::addAccessibleEventListener( const Reference< XAccessibleEventListener >& rxListener )
{
    ThrowIfDisposed();
    ::osl::MutexGuard aGuard(m_aMutex);

    if( !rxListener.is() )
           return;

    for (auto const& eventListener : mxEventListeners)
    {
        if( eventListener == rxListener )
            return;
    }

    mxEventListeners.push_back( rxListener );
}


void SAL_CALL ToolbarMenuAcc::removeAccessibleEventListener( const Reference< XAccessibleEventListener >& rxListener )
{
    ThrowIfDisposed();
    ::osl::MutexGuard aGuard(m_aMutex);

    if( rxListener.is() )
    {
        EventListenerVector::iterator aIter = std::find(mxEventListeners.begin(), mxEventListeners.end(), rxListener);
        if (aIter != mxEventListeners.end())
            mxEventListeners.erase(aIter);
    }
}


sal_Bool SAL_CALL ToolbarMenuAcc::containsPoint( const awt::Point& aPoint )
{
    ThrowIfDisposed();
    const awt::Rectangle aRect( getBounds() );
    const Point aSize( aRect.Width, aRect.Height );
    const Point aNullPoint, aTestPoint( aPoint.X, aPoint.Y );

    return tools::Rectangle( aNullPoint, aSize ).IsInside( aTestPoint );
}


Reference< XAccessible > SAL_CALL ToolbarMenuAcc::getAccessibleAtPoint( const awt::Point& aPoint )
{
    const SolarMutexGuard aSolarGuard;
    ThrowIfDisposed();

    Reference< XAccessible > xRet;

    const Point aVclPoint( aPoint.X, aPoint.Y );

    const int nEntryCount = mpParent->maEntryVector.size();
    for( int nEntry = 0; (nEntry < nEntryCount) && !xRet.is(); nEntry++ )
    {
        ToolbarMenuEntry* pEntry = mpParent->maEntryVector[nEntry].get();
        if( pEntry && pEntry->maRect.IsInside( aVclPoint ) )
        {
            if( pEntry->mpControl )
            {
                awt::Point aChildPoint( aPoint.X - pEntry->maRect.Left(), aPoint.Y - pEntry->maRect.Top() );
                Reference< XAccessibleComponent > xComp( pEntry->GetAccessible(), UNO_QUERY_THROW );
                xRet = xComp->getAccessibleAtPoint(aChildPoint);
            }
            else
            {
                xRet.set( pEntry->GetAccessible(), UNO_QUERY );
            }
        }
    }
    return xRet;
}


awt::Rectangle SAL_CALL ToolbarMenuAcc::getBounds()
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    const Point         aOutPos( mpParent->mrMenu.GetPosPixel() );
    const Size          aOutSize( mpParent->mrMenu.GetOutputSizePixel() );
    awt::Rectangle      aRet;

    aRet.X = aOutPos.X();
    aRet.Y = aOutPos.Y();
    aRet.Width = aOutSize.Width();
    aRet.Height = aOutSize.Height();

    return aRet;
}


awt::Point SAL_CALL ToolbarMenuAcc::getLocation()
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    const Point aOutPos( mpParent->mrMenu.GetPosPixel() );
    return awt::Point( aOutPos.X(), aOutPos.Y() );
}


awt::Point SAL_CALL ToolbarMenuAcc::getLocationOnScreen()
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    const Point aScreenPos( mpParent->mrMenu.OutputToAbsoluteScreenPixel( Point() ) );
    return awt::Point( aScreenPos.X(), aScreenPos.Y() );
}


awt::Size SAL_CALL ToolbarMenuAcc::getSize()
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    const Size aOutSize( mpParent->mrMenu.GetOutputSizePixel() );
    return awt::Size( aOutSize.Width(), aOutSize.Height() );
}

void SAL_CALL ToolbarMenuAcc::grabFocus()
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    mpParent->mrMenu.GrabFocus();
}

sal_Int32 SAL_CALL ToolbarMenuAcc::getForeground()
{
    ThrowIfDisposed();
    Color nColor = Application::GetSettings().GetStyleSettings().GetMenuTextColor();
    return static_cast<sal_Int32>(nColor);
}

sal_Int32 SAL_CALL ToolbarMenuAcc::getBackground()
{
    ThrowIfDisposed();
    Color nColor = Application::GetSettings().GetStyleSettings().GetMenuColor();
    return static_cast<sal_Int32>(nColor);
}

void SAL_CALL ToolbarMenuAcc::selectAccessibleChild( sal_Int32 nChildIndex )
{
    const SolarMutexGuard aSolarGuard;
    ThrowIfDisposed();

    mpParent->selectAccessibleChild( nChildIndex );
}

sal_Bool SAL_CALL ToolbarMenuAcc::isAccessibleChildSelected( sal_Int32 nChildIndex )
{
    const SolarMutexGuard aSolarGuard;
    ThrowIfDisposed();
    return mpParent->isAccessibleChildSelected( nChildIndex );
}


void SAL_CALL ToolbarMenuAcc::clearAccessibleSelection()
{
    const SolarMutexGuard aSolarGuard;
    ThrowIfDisposed();
    mpParent->clearAccessibleSelection();
}


void SAL_CALL ToolbarMenuAcc::selectAllAccessibleChildren()
{
    ThrowIfDisposed();
    // unsupported due to single selection only
}


sal_Int32 SAL_CALL ToolbarMenuAcc::getSelectedAccessibleChildCount()
{
    const SolarMutexGuard aSolarGuard;
    ThrowIfDisposed();

    return mpParent->mnHighlightedEntry != -1 ? 1 : 0;
}


Reference< XAccessible > SAL_CALL ToolbarMenuAcc::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;

    if( (mpParent->mnHighlightedEntry != -1) && (nSelectedChildIndex == 0) )
    {
        ToolbarMenuEntry* pEntry = mpParent->maEntryVector[ mpParent->mnHighlightedEntry ].get();
        if( pEntry )
        {
            if( pEntry->mpControl )
            {
                Reference< XAccessibleSelection > xSel( pEntry->GetAccessible(), UNO_QUERY_THROW );
                return xSel->getSelectedAccessibleChild(0);
            }
            else
                return Reference< XAccessible >( pEntry->GetAccessible(), UNO_QUERY );
        }
    }

    throw IndexOutOfBoundsException();
}


void SAL_CALL ToolbarMenuAcc::deselectAccessibleChild( sal_Int32 nChildIndex )
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    // Because of the single selection we can reset the whole selection when
    // the specified child is currently selected.
    if (isAccessibleChildSelected(nChildIndex))
        mpParent->clearAccessibleSelection();
}


void SAL_CALL ToolbarMenuAcc::disposing()
{
    EventListenerVector aListenerListCopy;

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
    EventObject aEvent (static_cast<XAccessible*>(this));
    for (auto const& listenerCopy : aListenerListCopy)
    {
        try
        {
            listenerCopy->disposing (aEvent);
        }
        catch( Exception& )
        {
            // Ignore exceptions.
        }

    }
}

void ToolbarMenuAcc::ThrowIfDisposed()
{
    if(rBHelper.bDisposed || rBHelper.bInDispose || !mpParent)
    {
        throw DisposedException ("object has been already disposed", static_cast<XWeak*>(this));
    }
}


ToolbarMenuEntryAcc::ToolbarMenuEntryAcc( ToolbarMenuEntry* pParent )
: ToolbarMenuEntryAccBase( m_aMutex )
, mpParent( pParent )
{
}


ToolbarMenuEntryAcc::~ToolbarMenuEntryAcc()
{
}


void SAL_CALL ToolbarMenuEntryAcc::disposing()
{
    EventListenerVector aListenerListCopy;

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
    EventObject aEvent (static_cast<XAccessible*>(this));
    for (auto const& listenerCopy : aListenerListCopy)
    {
        try
        {
            listenerCopy->disposing (aEvent);
        }
        catch( Exception& )
        {
            // Ignore exceptions.
        }
    }
}


Reference< XAccessibleContext > SAL_CALL ToolbarMenuEntryAcc::getAccessibleContext()
{
    return this;
}


sal_Int32 SAL_CALL ToolbarMenuEntryAcc::getAccessibleChildCount()
{
    return 0;
}


Reference< XAccessible > SAL_CALL ToolbarMenuEntryAcc::getAccessibleChild( sal_Int32 )
{
    throw IndexOutOfBoundsException();
}


Reference< XAccessible > SAL_CALL ToolbarMenuEntryAcc::getAccessibleParent()
{
    const SolarMutexGuard aSolarGuard;
    Reference< XAccessible > xRet;

    if( mpParent )
        xRet = mpParent->mrMenu.GetAccessible();

    return xRet;
}


sal_Int32 SAL_CALL ToolbarMenuEntryAcc::getAccessibleIndexInParent()
{
    const SolarMutexGuard aSolarGuard;
    // The index defaults to -1 to indicate the child does not belong to its
    // parent.
    sal_Int32 nIndexInParent = -1;

    if( mpParent )
    {
        Reference< XAccessibleContext > xParent( mpParent->mrMenu.GetAccessible(), UNO_QUERY );

        if( xParent.is() )
        {
            Reference< XAccessible > xThis( this );

            const sal_Int32 nCount = xParent->getAccessibleChildCount();
            for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
            {
                if( xParent->getAccessibleChild(nIndex) == xThis )
                {
                    nIndexInParent = nIndex;
                    break;
                }
            }
        }
    }

    return nIndexInParent;
}


sal_Int16 SAL_CALL ToolbarMenuEntryAcc::getAccessibleRole()
{
    return AccessibleRole::LIST_ITEM;
}


OUString SAL_CALL ToolbarMenuEntryAcc::getAccessibleDescription()
{
    return OUString();
}


OUString SAL_CALL ToolbarMenuEntryAcc::getAccessibleName()
{
    const SolarMutexGuard aSolarGuard;
    OUString              aRet;

    if( mpParent )
    {
        aRet = mpParent->maText;

        if( aRet.isEmpty() )
        {
            aRet = "Item ";
            aRet += OUString::number( mpParent->mnEntryId );
        }
    }

    return aRet;
}


Reference< XAccessibleRelationSet > SAL_CALL ToolbarMenuEntryAcc::getAccessibleRelationSet()
{
    return Reference< XAccessibleRelationSet >();
}


Reference< XAccessibleStateSet > SAL_CALL ToolbarMenuEntryAcc::getAccessibleStateSet()
{
    const SolarMutexGuard aSolarGuard;
    ::utl::AccessibleStateSetHelper*    pStateSet = new ::utl::AccessibleStateSetHelper;

    if( mpParent )
    {
        pStateSet->AddState (AccessibleStateType::ENABLED);
        pStateSet->AddState (AccessibleStateType::SENSITIVE);
        pStateSet->AddState (AccessibleStateType::SHOWING);
        pStateSet->AddState (AccessibleStateType::VISIBLE);
        pStateSet->AddState (AccessibleStateType::TRANSIENT);
        if( mpParent->mnEntryId != TITLE_ID )
        {
            pStateSet->AddState( AccessibleStateType::SELECTABLE );

            // SELECTED
            if( mpParent->mrMenu.getHighlightedEntryId() == mpParent->mnEntryId )
                pStateSet->AddState( AccessibleStateType::SELECTED );
        }
    }

    return pStateSet;
}


Locale SAL_CALL ToolbarMenuEntryAcc::getLocale()
{
    Locale aRet( "", "", "" );

    Reference< XAccessible > xParent( getAccessibleParent() );
    if( xParent.is() )
    {
        Reference< XAccessibleContext > xParentContext( xParent->getAccessibleContext() );

        if( xParentContext.is() )
            aRet = xParentContext->getLocale();
    }

    return aRet;
}


void SAL_CALL ToolbarMenuEntryAcc::addAccessibleEventListener( const Reference< XAccessibleEventListener >& rxListener )
{
    const ::osl::MutexGuard aGuard( maMutex );

    if( rxListener.is() )
    {
        for (auto const& eventListener : mxEventListeners)
        {
            if (eventListener == rxListener)
                return;
        }
        // listener not found so add it
        mxEventListeners.push_back( rxListener );
    }
}


void SAL_CALL ToolbarMenuEntryAcc::removeAccessibleEventListener( const Reference< XAccessibleEventListener >& rxListener )
{
    const ::osl::MutexGuard aGuard( maMutex );

    if( rxListener.is() )
    {
        EventListenerVector::iterator aIter = std::find(mxEventListeners.begin(), mxEventListeners.end(), rxListener);
        if (aIter != mxEventListeners.end())
            mxEventListeners.erase(aIter);
    }
}


sal_Bool SAL_CALL ToolbarMenuEntryAcc::containsPoint( const awt::Point& aPoint )
{
    const awt::Rectangle    aRect( getBounds() );
    const Point             aSize( aRect.Width, aRect.Height );
    const Point             aNullPoint, aTestPoint( aPoint.X, aPoint.Y );

    return tools::Rectangle( aNullPoint, aSize ).IsInside( aTestPoint );
}


Reference< XAccessible > SAL_CALL ToolbarMenuEntryAcc::getAccessibleAtPoint( const awt::Point& )
{
    Reference< XAccessible > xRet;
    return xRet;
}


awt::Rectangle SAL_CALL ToolbarMenuEntryAcc::getBounds()
{
    const SolarMutexGuard aSolarGuard;
    awt::Rectangle      aRet;

    if( mpParent )
    {
        tools::Rectangle   aRect( mpParent->maRect );
        tools::Rectangle   aParentRect( Point(), mpParent->mrMenu.GetOutputSizePixel() );

        aRect.Intersection( aParentRect );

        aRet.X = aRect.Left();
        aRet.Y = aRect.Top();
        aRet.Width = aRect.GetWidth();
        aRet.Height = aRect.GetHeight();
    }

    return aRet;
}


awt::Point SAL_CALL ToolbarMenuEntryAcc::getLocation()
{
    const awt::Rectangle aRect( getBounds() );
    return awt::Point( aRect.X, aRect.Y );
}


awt::Point SAL_CALL ToolbarMenuEntryAcc::getLocationOnScreen()
{
    const SolarMutexGuard aSolarGuard;
    awt::Point aRet;

    if( mpParent )
    {
        const Point aScreenPos( mpParent->mrMenu.OutputToAbsoluteScreenPixel( mpParent->maRect.TopLeft() ) );

        aRet.X = aScreenPos.X();
        aRet.Y = aScreenPos.Y();
    }

    return aRet;
}


awt::Size SAL_CALL ToolbarMenuEntryAcc::getSize()
{
    const awt::Rectangle aRect( getBounds() );
    awt::Size aRet;

    aRet.Width = aRect.Width;
    aRet.Height = aRect.Height;

    return aRet;
}

void SAL_CALL ToolbarMenuEntryAcc::grabFocus()
{
    // nothing to do
}

sal_Int32 SAL_CALL ToolbarMenuEntryAcc::getForeground(  )
{
    return static_cast<sal_Int32>(Application::GetSettings().GetStyleSettings().GetMenuTextColor());
}

sal_Int32 SAL_CALL ToolbarMenuEntryAcc::getBackground(  )
{
    return static_cast<sal_Int32>(Application::GetSettings().GetStyleSettings().GetMenuColor());
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
