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

#include <unotools/accessiblestatesethelper.hxx>

#include <vcl/svapp.hxx>

#include "svtools/toolbarmenu.hxx"

#include "toolbarmenuimp.hxx"


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;

namespace svtools {

// ------------------
// - ToolbarMenuAcc -
// ------------------

ToolbarMenuAcc::ToolbarMenuAcc( ToolbarMenu_Impl& rParent )
: ToolbarMenuAccComponentBase(m_aMutex)
, mpParent( &rParent )
, mbIsFocused(false)
{
    mpParent->mrMenu.AddEventListener( LINK( this, ToolbarMenuAcc, WindowEventListener ) );
}

// -----------------------------------------------------------------------------

ToolbarMenuAcc::~ToolbarMenuAcc()
{
    if( mpParent )
        mpParent->mrMenu.RemoveEventListener( LINK( this, ToolbarMenuAcc, WindowEventListener ) );
}

// -----------------------------------------------------------------------

IMPL_LINK( ToolbarMenuAcc, WindowEventListener, VclSimpleEvent*, pEvent )
{
    DBG_ASSERT( pEvent && pEvent->ISA( VclWindowEvent ), "Unknown WindowEvent!" );

    /* Ignore VCLEVENT_WINDOW_ENDPOPUPMODE, because the UNO accessibility wrapper
     * might have been destroyed by the previous VCLEventListener (if no AT tool
     * is running), e.g. sub-toolbars in impress.
     */
    if ( mpParent && pEvent && pEvent->ISA( VclWindowEvent ) && (pEvent->GetId() != VCLEVENT_WINDOW_ENDPOPUPMODE) )
    {
        DBG_ASSERT( ((VclWindowEvent*)pEvent)->GetWindow(), "Window???" );
        if( !((VclWindowEvent*)pEvent)->GetWindow()->IsAccessibilityEventsSuppressed() || ( pEvent->GetId() == VCLEVENT_OBJECT_DYING ) )
        {
            ProcessWindowEvent( *(VclWindowEvent*)pEvent );
        }
    }
    return 0;
}

// -----------------------------------------------------------------------

void ToolbarMenuAcc::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    Any aOldValue, aNewValue;

    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_OBJECT_DYING:
        {
            mpParent->mrMenu.RemoveEventListener( LINK( this, ToolbarMenuAcc, WindowEventListener ) );
            mpParent = 0;
        }
        break;

        case VCLEVENT_WINDOW_GETFOCUS:
        {
            if( !mbIsFocused )
            {
                mpParent->notifyHighlightedEntry();
                mbIsFocused = true;
            }
        }
        break;
        case VCLEVENT_WINDOW_LOSEFOCUS:
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

// -----------------------------------------------------------------------

void ToolbarMenuAcc::FireAccessibleEvent( short nEventId, const Any& rOldValue, const Any& rNewValue )
{
    if( nEventId )
    {
        EventListenerVector                  aTmpListeners( mxEventListeners );
        AccessibleEventObject aEvtObject;

        aEvtObject.EventId = nEventId;
        aEvtObject.Source = static_cast<XWeak*>(this);
        aEvtObject.NewValue = rNewValue;
        aEvtObject.OldValue = rOldValue;

        for (EventListenerVector::const_iterator aIter( aTmpListeners.begin() ), aEnd( aTmpListeners.end() );
            aIter != aEnd ; ++aIter)
        {
            try
            {
                (*aIter)->notifyEvent( aEvtObject );
            }
            catch( Exception& )
            {
            }
        }
    }
}

// -----------------------------------------------------------------------------

Reference< XAccessibleContext > SAL_CALL ToolbarMenuAcc::getAccessibleContext() throw (RuntimeException)
{
    ThrowIfDisposed();
    return this;
}

// -----------------------------------------------------------------------------

sal_Int32 SAL_CALL ToolbarMenuAcc::getAccessibleChildCount() throw (RuntimeException)
{
    const SolarMutexGuard aSolarGuard;
    ThrowIfDisposed();

    return mpParent->getAccessibleChildCount();
}

// -----------------------------------------------------------------------------

Reference< XAccessible > SAL_CALL ToolbarMenuAcc::getAccessibleChild( sal_Int32 i ) throw (IndexOutOfBoundsException, RuntimeException)
{
    const SolarMutexGuard aSolarGuard;
    ThrowIfDisposed();

    return mpParent->getAccessibleChild(i);
}

// -----------------------------------------------------------------------------

Reference< XAccessible > SAL_CALL ToolbarMenuAcc::getAccessibleParent() throw (RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;

    Reference< XAccessible > xRet;

    Window* pParent = mpParent->mrMenu.GetParent();
    if( pParent )
        xRet = pParent->GetAccessible();

    return xRet;
}

// -----------------------------------------------------------------------------

sal_Int32 SAL_CALL ToolbarMenuAcc::getAccessibleIndexInParent() throw (RuntimeException)
{
    const SolarMutexGuard aSolarGuard;
    ThrowIfDisposed();

    Window* pParent = mpParent->mrMenu.GetParent();
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

// -----------------------------------------------------------------------------

sal_Int16 SAL_CALL ToolbarMenuAcc::getAccessibleRole() throw (RuntimeException)
{
    ThrowIfDisposed();
    return AccessibleRole::LIST;
}

// -----------------------------------------------------------------------------

OUString SAL_CALL ToolbarMenuAcc::getAccessibleDescription() throw (RuntimeException)
{
    ThrowIfDisposed();
    return OUString( "ToolbarMenu" );
}

// -----------------------------------------------------------------------------

OUString SAL_CALL ToolbarMenuAcc::getAccessibleName() throw (RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    OUString aRet;

    if( mpParent )
        aRet = mpParent->mrMenu.GetAccessibleName();

    if( aRet.isEmpty() )
    {
        Window* pLabel = mpParent->mrMenu.GetAccessibleRelationLabeledBy();
        if( pLabel && pLabel != &mpParent->mrMenu )
            aRet = OutputDevice::GetNonMnemonicString( pLabel->GetText() );
    }

    return aRet;
}

// -----------------------------------------------------------------------------

Reference< XAccessibleRelationSet > SAL_CALL ToolbarMenuAcc::getAccessibleRelationSet() throw (RuntimeException)
{
    ThrowIfDisposed();
    return Reference< XAccessibleRelationSet >();
}

// -----------------------------------------------------------------------------

Reference< XAccessibleStateSet > SAL_CALL ToolbarMenuAcc::getAccessibleStateSet() throw (RuntimeException)
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

// -----------------------------------------------------------------------------

Locale SAL_CALL ToolbarMenuAcc::getLocale() throw (IllegalAccessibleComponentStateException, RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    const OUString aEmptyStr;
    Reference< XAccessible > xParent( getAccessibleParent() );
    Locale aRet( aEmptyStr, aEmptyStr, aEmptyStr );

    if( xParent.is() )
    {
        Reference< XAccessibleContext > xParentContext( xParent->getAccessibleContext() );

        if( xParentContext.is() )
            aRet = xParentContext->getLocale ();
    }

    return aRet;
}

// -----------------------------------------------------------------------------

void SAL_CALL ToolbarMenuAcc::addAccessibleEventListener( const Reference< XAccessibleEventListener >& rxListener ) throw (RuntimeException)
{
    ThrowIfDisposed();
    ::osl::MutexGuard aGuard(m_aMutex);

    if( rxListener.is() )
    {
           EventListenerVector::const_iterator aIter = mxEventListeners.begin();
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

// -----------------------------------------------------------------------------

void SAL_CALL ToolbarMenuAcc::removeAccessibleEventListener( const Reference< XAccessibleEventListener >& rxListener ) throw (RuntimeException)
{
    ThrowIfDisposed();
    ::osl::MutexGuard aGuard(m_aMutex);

    if( rxListener.is() )
    {
        EventListenerVector::const_iterator aEnd = mxEventListeners.end();

        for(EventListenerVector::iterator aIter = mxEventListeners.begin();
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

sal_Bool SAL_CALL ToolbarMenuAcc::containsPoint( const awt::Point& aPoint ) throw (RuntimeException)
{
    ThrowIfDisposed();
    const awt::Rectangle aRect( getBounds() );
    const Point aSize( aRect.Width, aRect.Height );
    const Point aNullPoint, aTestPoint( aPoint.X, aPoint.Y );

    return Rectangle( aNullPoint, aSize ).IsInside( aTestPoint );
}

// -----------------------------------------------------------------------------

Reference< XAccessible > SAL_CALL ToolbarMenuAcc::getAccessibleAtPoint( const awt::Point& aPoint ) throw (RuntimeException)
{
    const SolarMutexGuard aSolarGuard;
    ThrowIfDisposed();

    Reference< XAccessible > xRet;

    const Point aVclPoint( aPoint.X, aPoint.Y );

    const int nEntryCount = mpParent->maEntryVector.size();
    for( int nEntry = 0; (nEntry < nEntryCount) && !xRet.is(); nEntry++ )
    {
        ToolbarMenuEntry* pEntry = mpParent->maEntryVector[nEntry];
        if( pEntry && pEntry->maRect.IsInside( aVclPoint ) )
        {
            if( pEntry->mpControl )
            {
                awt::Point aChildPoint( aPoint.X - pEntry->maRect.Left(), aPoint.Y - pEntry->maRect.Top() );
                Reference< XAccessibleComponent > xComp( pEntry->GetAccessible(true), UNO_QUERY_THROW );
                xRet = xComp->getAccessibleAtPoint(aChildPoint);
            }
            else
            {
                xRet = Reference< XAccessible >( pEntry->GetAccessible(true), UNO_QUERY );
            }
        }
    }
    return xRet;
}

// -----------------------------------------------------------------------------

awt::Rectangle SAL_CALL ToolbarMenuAcc::getBounds() throw (RuntimeException)
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

// -----------------------------------------------------------------------------

awt::Point SAL_CALL ToolbarMenuAcc::getLocation() throw (RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    const Point aOutPos( mpParent->mrMenu.GetPosPixel() );
    return awt::Point( aOutPos.X(), aOutPos.Y() );
}

// -----------------------------------------------------------------------------

awt::Point SAL_CALL ToolbarMenuAcc::getLocationOnScreen()  throw (RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    const Point aScreenPos( mpParent->mrMenu.OutputToAbsoluteScreenPixel( Point() ) );
    return awt::Point( aScreenPos.X(), aScreenPos.Y() );
}

// -----------------------------------------------------------------------------

awt::Size SAL_CALL ToolbarMenuAcc::getSize() throw (RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    const Size aOutSize( mpParent->mrMenu.GetOutputSizePixel() );
    return awt::Size( aOutSize.Width(), aOutSize.Height() );
}

// -----------------------------------------------------------------------------

void SAL_CALL ToolbarMenuAcc::grabFocus() throw (RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    mpParent->mrMenu.GrabFocus();
}

// -----------------------------------------------------------------------------

Any SAL_CALL ToolbarMenuAcc::getAccessibleKeyBinding() throw (RuntimeException)
{
    ThrowIfDisposed();
    return Any();
}

// -----------------------------------------------------------------------------

sal_Int32 SAL_CALL ToolbarMenuAcc::getForeground() throw (RuntimeException)
{
    ThrowIfDisposed();
    sal_uInt32 nColor = Application::GetSettings().GetStyleSettings().GetMenuTextColor().GetColor();
    return static_cast<sal_Int32>(nColor);
}

// -----------------------------------------------------------------------------

sal_Int32 SAL_CALL ToolbarMenuAcc::getBackground() throw (RuntimeException)
{
    ThrowIfDisposed();
    sal_uInt32 nColor = Application::GetSettings().GetStyleSettings().GetMenuColor().GetColor();
    return static_cast<sal_Int32>(nColor);
}

// -----------------------------------------------------------------------------

void SAL_CALL ToolbarMenuAcc::selectAccessibleChild( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    const SolarMutexGuard aSolarGuard;
    ThrowIfDisposed();

    mpParent->selectAccessibleChild( nChildIndex );
}

// -----------------------------------------------------------------------------

sal_Bool SAL_CALL ToolbarMenuAcc::isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    const SolarMutexGuard aSolarGuard;
    ThrowIfDisposed();
    return mpParent->isAccessibleChildSelected( nChildIndex );
}

// -----------------------------------------------------------------------------

void SAL_CALL ToolbarMenuAcc::clearAccessibleSelection() throw (RuntimeException)
{
    const SolarMutexGuard aSolarGuard;
    ThrowIfDisposed();
    mpParent->clearAccessibleSelection();
}

// -----------------------------------------------------------------------------

void SAL_CALL ToolbarMenuAcc::selectAllAccessibleChildren() throw (RuntimeException)
{
    ThrowIfDisposed();
    // unsupported due to single selection only
}

// -----------------------------------------------------------------------------

sal_Int32 SAL_CALL ToolbarMenuAcc::getSelectedAccessibleChildCount() throw (RuntimeException)
{
    const SolarMutexGuard aSolarGuard;
    ThrowIfDisposed();

    return mpParent->mnHighlightedEntry != -1 ? 1 : 0;
}

// -----------------------------------------------------------------------------

Reference< XAccessible > SAL_CALL ToolbarMenuAcc::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;

    if( (mpParent->mnHighlightedEntry != -1) && (nSelectedChildIndex == 0) )
    {
        ToolbarMenuEntry* pEntry = mpParent->maEntryVector[ mpParent->mnHighlightedEntry ];
        if( pEntry )
        {
            if( pEntry->mpControl )
            {
                Reference< XAccessibleSelection > xSel( pEntry->GetAccessible(true), UNO_QUERY_THROW );
                return xSel->getSelectedAccessibleChild(0);
            }
            else
                return Reference< XAccessible >( pEntry->GetAccessible(true), UNO_QUERY );
        }
    }

    throw IndexOutOfBoundsException();
}

// -----------------------------------------------------------------------------

void SAL_CALL ToolbarMenuAcc::deselectAccessibleChild( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    // Because of the single selection we can reset the whole selection when
    // the specified child is currently selected.
    if (isAccessibleChildSelected(nChildIndex))
        mpParent->clearAccessibleSelection();
}

// -----------------------------------------------------------------------------

void SAL_CALL ToolbarMenuAcc::disposing (void)
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
        mpParent = NULL;
    }

    // Inform all listeners that this objects is disposing.
    EventListenerVector::const_iterator aListenerIterator (aListenerListCopy.begin());
    EventObject aEvent (static_cast<XAccessible*>(this));
    while(aListenerIterator != aListenerListCopy.end())
    {
        try
        {
            (*aListenerIterator)->disposing (aEvent);
        }
        catch( Exception& )
        {
            // Ignore exceptions.
        }

        ++aListenerIterator;
    }
}

void ToolbarMenuAcc::ThrowIfDisposed (void) throw (DisposedException)
{
    if(rBHelper.bDisposed || rBHelper.bInDispose || !mpParent)
    {
        throw DisposedException ("object has been already disposed", static_cast<XWeak*>(this));
    }
}

// -----------------------
// - ToolbarMenuEntryAcc -
// -----------------------

ToolbarMenuEntryAcc::ToolbarMenuEntryAcc( ToolbarMenuEntry* pParent )
: ToolbarMenuEntryAccBase( m_aMutex )
, mpParent( pParent )
{
}

// -----------------------------------------------------------------------------

ToolbarMenuEntryAcc::~ToolbarMenuEntryAcc()
{
}

// -----------------------------------------------------------------------

void SAL_CALL ToolbarMenuEntryAcc::disposing (void)
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
        mpParent = NULL;
    }

    // Inform all listeners that this objects is disposing.
    EventListenerVector::const_iterator aListenerIterator (aListenerListCopy.begin());
    EventObject aEvent (static_cast<XAccessible*>(this));
    while(aListenerIterator != aListenerListCopy.end())
    {
        try
        {
            (*aListenerIterator)->disposing (aEvent);
        }
        catch( Exception& )
        {
            // Ignore exceptions.
        }

        ++aListenerIterator;
    }
}
// -----------------------------------------------------------------------------

Reference< XAccessibleContext > SAL_CALL ToolbarMenuEntryAcc::getAccessibleContext() throw (RuntimeException)
{
    return this;
}

// -----------------------------------------------------------------------------

sal_Int32 SAL_CALL ToolbarMenuEntryAcc::getAccessibleChildCount() throw (RuntimeException)
{
    return 0;
}

// -----------------------------------------------------------------------------

Reference< XAccessible > SAL_CALL ToolbarMenuEntryAcc::getAccessibleChild( sal_Int32 ) throw (IndexOutOfBoundsException, RuntimeException)
{
    throw IndexOutOfBoundsException();
}

// -----------------------------------------------------------------------------

Reference< XAccessible > SAL_CALL ToolbarMenuEntryAcc::getAccessibleParent() throw (RuntimeException)
{
    const SolarMutexGuard aSolarGuard;
    Reference< XAccessible > xRet;

    if( mpParent )
        xRet = mpParent->mrMenu.GetAccessible();

    return xRet;
}

// -----------------------------------------------------------------------------

sal_Int32 SAL_CALL ToolbarMenuEntryAcc::getAccessibleIndexInParent() throw (RuntimeException)
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

// -----------------------------------------------------------------------------

sal_Int16 SAL_CALL ToolbarMenuEntryAcc::getAccessibleRole() throw (RuntimeException)
{
    return AccessibleRole::LIST_ITEM;
}

// -----------------------------------------------------------------------------

OUString SAL_CALL ToolbarMenuEntryAcc::getAccessibleDescription() throw (RuntimeException)
{
    return OUString();
}

// -----------------------------------------------------------------------------

OUString SAL_CALL ToolbarMenuEntryAcc::getAccessibleName() throw (RuntimeException)
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

// -----------------------------------------------------------------------------

Reference< XAccessibleRelationSet > SAL_CALL ToolbarMenuEntryAcc::getAccessibleRelationSet() throw (RuntimeException)
{
    return Reference< XAccessibleRelationSet >();
}

// -----------------------------------------------------------------------------

Reference< XAccessibleStateSet > SAL_CALL ToolbarMenuEntryAcc::getAccessibleStateSet() throw (RuntimeException)
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

// -----------------------------------------------------------------------------

Locale SAL_CALL ToolbarMenuEntryAcc::getLocale() throw (IllegalAccessibleComponentStateException, RuntimeException)
{
    const OUString aEmptyStr;
    Locale aRet( aEmptyStr, aEmptyStr, aEmptyStr );

    Reference< XAccessible > xParent( getAccessibleParent() );
    if( xParent.is() )
    {
        Reference< XAccessibleContext > xParentContext( xParent->getAccessibleContext() );

        if( xParentContext.is() )
            aRet = xParentContext->getLocale();
    }

    return aRet;
}

// -----------------------------------------------------------------------------

void SAL_CALL ToolbarMenuEntryAcc::addAccessibleEventListener( const Reference< XAccessibleEventListener >& rxListener ) throw (RuntimeException)
{
    const ::osl::MutexGuard aGuard( maMutex );

    if( rxListener.is() )
    {
        for (EventListenerVector::const_iterator aIter( mxEventListeners.begin() ), aEnd( mxEventListeners.end() );
            aIter != aEnd; ++aIter )
        {
            if( *aIter == rxListener )
                return;
        }
        // listener not found so add it
        mxEventListeners.push_back( rxListener );
    }
}

// -----------------------------------------------------------------------------

void SAL_CALL ToolbarMenuEntryAcc::removeAccessibleEventListener( const Reference< XAccessibleEventListener >& rxListener ) throw (RuntimeException)
{
    const ::osl::MutexGuard aGuard( maMutex );

    if( rxListener.is() )
    {
        EventListenerVector::const_iterator aEnd = mxEventListeners.end();

        for (EventListenerVector::iterator aIter = mxEventListeners.begin();
               aIter != aEnd;
               ++aIter)
        {
            if( *aIter == rxListener )
            {
                mxEventListeners.erase( aIter );
                break;
            }
        }
    }
}

// -----------------------------------------------------------------------------

sal_Bool SAL_CALL ToolbarMenuEntryAcc::containsPoint( const awt::Point& aPoint ) throw (RuntimeException)
{
    const awt::Rectangle    aRect( getBounds() );
    const Point             aSize( aRect.Width, aRect.Height );
    const Point             aNullPoint, aTestPoint( aPoint.X, aPoint.Y );

    return Rectangle( aNullPoint, aSize ).IsInside( aTestPoint );
}

// -----------------------------------------------------------------------------

Reference< XAccessible > SAL_CALL ToolbarMenuEntryAcc::getAccessibleAtPoint( const awt::Point& ) throw (RuntimeException)
{
    Reference< XAccessible > xRet;
    return xRet;
}

// -----------------------------------------------------------------------------

awt::Rectangle SAL_CALL ToolbarMenuEntryAcc::getBounds() throw (RuntimeException)
{
    const SolarMutexGuard aSolarGuard;
    awt::Rectangle      aRet;

    if( mpParent )
    {
        Rectangle   aRect( mpParent->maRect );
        Point       aOrigin;
        Rectangle   aParentRect( aOrigin, mpParent->mrMenu.GetOutputSizePixel() );

        aRect.Intersection( aParentRect );

        aRet.X = aRect.Left();
        aRet.Y = aRect.Top();
        aRet.Width = aRect.GetWidth();
        aRet.Height = aRect.GetHeight();
    }

    return aRet;
}

// -----------------------------------------------------------------------------

awt::Point SAL_CALL ToolbarMenuEntryAcc::getLocation() throw (RuntimeException)
{
    const awt::Rectangle aRect( getBounds() );
    return awt::Point( aRect.X, aRect.Y );
}

// -----------------------------------------------------------------------------

awt::Point SAL_CALL ToolbarMenuEntryAcc::getLocationOnScreen() throw (RuntimeException)
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

// -----------------------------------------------------------------------------

awt::Size SAL_CALL ToolbarMenuEntryAcc::getSize() throw (RuntimeException)
{
    const awt::Rectangle aRect( getBounds() );
    awt::Size aRet;

    aRet.Width = aRect.Width;
    aRet.Height = aRect.Height;

    return aRet;
}

// -----------------------------------------------------------------------------

void SAL_CALL ToolbarMenuEntryAcc::grabFocus() throw (RuntimeException)
{
    // nothing to do
}

// -----------------------------------------------------------------------------

Any SAL_CALL ToolbarMenuEntryAcc::getAccessibleKeyBinding() throw (RuntimeException)
{
    return Any();
}

// -----------------------------------------------------------------------------

sal_Int32 SAL_CALL ToolbarMenuEntryAcc::getForeground(  ) throw (RuntimeException)
{
    return static_cast<sal_Int32>(Application::GetSettings().GetStyleSettings().GetMenuTextColor().GetColor());
}

// -----------------------------------------------------------------------------

sal_Int32 SAL_CALL ToolbarMenuEntryAcc::getBackground(  )  throw (RuntimeException)
{
    return static_cast<sal_Int32>(Application::GetSettings().GetStyleSettings().GetMenuColor().GetColor());
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
