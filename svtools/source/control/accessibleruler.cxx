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
#include <svtools/accessibleruler.hxx>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/IllegalAccessibleComponentStateException.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <comphelper/accessibleeventnotifier.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <toolkit/helper/convert.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <rtl/uuid.h>
#include <tools/gen.hxx>

#include <svtools/ruler.hxx>

using namespace ::cppu;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;


//=====  internal  ============================================================

SvtRulerAccessible::SvtRulerAccessible(
    const uno::Reference< XAccessible >& rxParent, Ruler& rRepr, const OUString& rName ) :

    SvtRulerAccessible_Base( m_aMutex ),
    msName( rName ),
    mxParent( rxParent ),
    mpRepr( &rRepr ),
    mnClientId( 0 )
{
}

SvtRulerAccessible::~SvtRulerAccessible()
{

    if( IsAlive() )
    {
        osl_atomic_increment( &m_refCount );
        dispose();      // set mpRepr = NULL & release all children
    }
}

//=====  XAccessible  =========================================================

uno::Reference< XAccessibleContext > SAL_CALL SvtRulerAccessible::getAccessibleContext()
{
    return this;
}

//=====  XAccessibleComponent  ================================================

sal_Bool SAL_CALL SvtRulerAccessible::containsPoint( const awt::Point& rPoint )
{
    // no guard -> done in getBounds()
//  return GetBoundingBox().IsInside( VCLPoint( rPoint ) );
    return tools::Rectangle( Point( 0, 0 ), GetBoundingBox().GetSize() ).IsInside( VCLPoint( rPoint ) );
}

uno::Reference< XAccessible > SAL_CALL SvtRulerAccessible::getAccessibleAtPoint( const awt::Point& )
{
    ::osl::MutexGuard           aGuard( m_aMutex );

    ThrowExceptionIfNotAlive();

    uno::Reference< XAccessible >   xRet;


    return xRet;
}

awt::Rectangle SAL_CALL SvtRulerAccessible::getBounds()
{
    // no guard -> done in GetBoundingBox()
    return AWTRectangle( GetBoundingBox() );
}

awt::Point SAL_CALL SvtRulerAccessible::getLocation()
{
    // no guard -> done in GetBoundingBox()
    return AWTPoint( GetBoundingBox().TopLeft() );
}

awt::Point SAL_CALL SvtRulerAccessible::getLocationOnScreen()
{
    // no guard -> done in GetBoundingBoxOnScreen()
    return AWTPoint( GetBoundingBoxOnScreen().TopLeft() );
}

awt::Size SAL_CALL SvtRulerAccessible::getSize()
{
    // no guard -> done in GetBoundingBox()
    return AWTSize( GetBoundingBox().GetSize() );
}

bool SvtRulerAccessible::isVisible()
{
    ::osl::MutexGuard           aGuard( m_aMutex );

    ThrowExceptionIfNotAlive();

    return mpRepr->IsVisible();
}

//=====  XAccessibleContext  ==================================================
sal_Int32 SAL_CALL SvtRulerAccessible::getAccessibleChildCount()
{
    ::osl::MutexGuard   aGuard( m_aMutex );

    ThrowExceptionIfNotAlive();

    return 0;
}

uno::Reference< XAccessible > SAL_CALL SvtRulerAccessible::getAccessibleChild( sal_Int32 )
{
    uno::Reference< XAccessible >   xChild ;

    return xChild;
}

uno::Reference< XAccessible > SAL_CALL SvtRulerAccessible::getAccessibleParent()
{
    return mxParent;
}

sal_Int32 SAL_CALL SvtRulerAccessible::getAccessibleIndexInParent()
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    //  Use a simple but slow solution for now.  Optimize later.

    //  Iterate over all the parent's children and search for this object.
    if( mxParent.is() )
    {
        uno::Reference< XAccessibleContext >        xParentContext( mxParent->getAccessibleContext() );
        if( xParentContext.is() )
        {
            sal_Int32                       nChildCount = xParentContext->getAccessibleChildCount();
            for( sal_Int32 i = 0 ; i < nChildCount ; ++i )
            {
                uno::Reference< XAccessible >   xChild( xParentContext->getAccessibleChild( i ) );
                if( xChild.get() == static_cast<XAccessible*>(this) )
                    return i;
            }
        }
    }

    //   Return -1 to indicate that this object's parent does not know about the
    //   object.
    return -1;
}

sal_Int16 SAL_CALL SvtRulerAccessible::getAccessibleRole()
{
    return AccessibleRole::RULER;
}

OUString SAL_CALL SvtRulerAccessible::getAccessibleDescription()
{
    return OUString();
}

OUString SAL_CALL SvtRulerAccessible::getAccessibleName()
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    return msName;
}

/** Return empty uno::Reference to indicate that the relation set is not
    supported.
*/
uno::Reference< XAccessibleRelationSet > SAL_CALL SvtRulerAccessible::getAccessibleRelationSet()
{
    return uno::Reference< XAccessibleRelationSet >();
}


uno::Reference< XAccessibleStateSet > SAL_CALL SvtRulerAccessible::getAccessibleStateSet()
{
    ::osl::MutexGuard                       aGuard( m_aMutex );
    utl::AccessibleStateSetHelper*          pStateSetHelper = new utl::AccessibleStateSetHelper;

    if( IsAlive() )
    {
        pStateSetHelper->AddState( AccessibleStateType::ENABLED );

        pStateSetHelper->AddState( AccessibleStateType::SHOWING );

        if( isVisible() )
            pStateSetHelper->AddState( AccessibleStateType::VISIBLE );

        if ( mpRepr->GetStyle() & WB_HORZ )
            pStateSetHelper->AddState( AccessibleStateType::HORIZONTAL );
        else
            pStateSetHelper->AddState( AccessibleStateType::VERTICAL );

        if(pStateSetHelper->contains(AccessibleStateType::FOCUSABLE))
        {
            pStateSetHelper->RemoveState( AccessibleStateType::FOCUSABLE );
        }

    }


    return pStateSetHelper;
}

lang::Locale SAL_CALL SvtRulerAccessible::getLocale()
{
    ::osl::MutexGuard                           aGuard( m_aMutex );
    if( mxParent.is() )
    {
        uno::Reference< XAccessibleContext >    xParentContext( mxParent->getAccessibleContext() );
        if( xParentContext.is() )
            return xParentContext->getLocale();
    }

    //  No parent.  Therefore throw exception to indicate this cluelessness.
    throw IllegalAccessibleComponentStateException();
}

void SAL_CALL SvtRulerAccessible::addAccessibleEventListener( const uno::Reference< XAccessibleEventListener >& xListener )
{
    if (xListener.is())
    {
        ::osl::MutexGuard   aGuard( m_aMutex );
        if (!mnClientId)
            mnClientId = comphelper::AccessibleEventNotifier::registerClient( );
        comphelper::AccessibleEventNotifier::addEventListener( mnClientId, xListener );
    }
}

void SAL_CALL SvtRulerAccessible::removeAccessibleEventListener( const uno::Reference< XAccessibleEventListener >& xListener )
{
    if (!(xListener.is() && mnClientId))
        return;

    ::osl::MutexGuard   aGuard( m_aMutex );

    sal_Int32 nListenerCount = comphelper::AccessibleEventNotifier::removeEventListener( mnClientId, xListener );
    if ( !nListenerCount )
    {
        // no listeners anymore
        // -> revoke ourself. This may lead to the notifier thread dying (if we were the last client),
        // and at least to us not firing any events anymore, in case somebody calls
        // NotifyAccessibleEvent, again
        comphelper::AccessibleEventNotifier::revokeClient( mnClientId );
        mnClientId = 0;
    }
}

void SAL_CALL SvtRulerAccessible::grabFocus()
{
    SolarMutexGuard     aSolarGuard;
    ::osl::MutexGuard   aGuard( m_aMutex );

    ThrowExceptionIfNotAlive();

    mpRepr->GrabFocus();
}

sal_Int32 SvtRulerAccessible::getForeground(  )
{
    SolarMutexGuard     aSolarGuard;
    ::osl::MutexGuard   aGuard( m_aMutex );
    ThrowExceptionIfNotAlive();

    return sal_Int32(mpRepr->GetControlForeground());
}
sal_Int32 SvtRulerAccessible::getBackground(  )
{
    SolarMutexGuard     aSolarGuard;
    ::osl::MutexGuard   aGuard( m_aMutex );
    ThrowExceptionIfNotAlive();

    return sal_Int32(mpRepr->GetControlBackground());
}

// XServiceInfo
OUString SAL_CALL SvtRulerAccessible::getImplementationName()
{
    return OUString( "com.sun.star.comp.ui.SvtRulerAccessible" );
}

sal_Bool SAL_CALL SvtRulerAccessible::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService( this, sServiceName );
}

Sequence< OUString > SAL_CALL SvtRulerAccessible::getSupportedServiceNames()
{
    return Sequence< OUString > { OUString("com.sun.star.accessibility.AccessibleContext") };
}

//=====  XTypeProvider  =======================================================
Sequence< sal_Int8 > SAL_CALL SvtRulerAccessible::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

void SAL_CALL SvtRulerAccessible::disposing()
{
    if( rBHelper.bDisposed )
        return;

    ::osl::MutexGuard   aGuard( m_aMutex );
    mpRepr = nullptr;      // object dies with representation

    // Send a disposing to all listeners.
    if ( mnClientId )
    {
        comphelper::AccessibleEventNotifier::revokeClientNotifyDisposing( mnClientId, *this );
        mnClientId =  0;
    }
    mxParent.clear();
}

tools::Rectangle SvtRulerAccessible::GetBoundingBoxOnScreen()
{
    SolarMutexGuard     aSolarGuard;
    ::osl::MutexGuard   aGuard( m_aMutex );

    ThrowExceptionIfNotAlive();
    return tools::Rectangle( mpRepr->GetParent()->OutputToAbsoluteScreenPixel( mpRepr->GetPosPixel() ), mpRepr->GetSizePixel() );
}

tools::Rectangle SvtRulerAccessible::GetBoundingBox()
{
    SolarMutexGuard     aSolarGuard;
    ::osl::MutexGuard   aGuard( m_aMutex );

    ThrowExceptionIfNotAlive();

    return tools::Rectangle( mpRepr->GetPosPixel(), mpRepr->GetSizePixel() );
}

void SvtRulerAccessible::ThrowExceptionIfNotAlive()
{
    if( rBHelper.bDisposed || rBHelper.bInDispose )
        throw lang::DisposedException();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
