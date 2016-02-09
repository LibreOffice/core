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
#include <unotools/accessiblestatesethelper.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <comphelper/accessibleeventnotifier.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <toolkit/helper/vclunohelper.hxx>
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

uno::Reference< XAccessibleContext > SAL_CALL SvtRulerAccessible::getAccessibleContext() throw( RuntimeException, std::exception )
{
    return this;
}

//=====  XAccessibleComponent  ================================================

sal_Bool SAL_CALL SvtRulerAccessible::containsPoint( const awt::Point& rPoint ) throw( RuntimeException, std::exception )
{
    // no guard -> done in getBounds()
//  return GetBoundingBox().IsInside( VCLPoint( rPoint ) );
    return Rectangle( Point( 0, 0 ), GetBoundingBox().GetSize() ).IsInside( VCLPoint( rPoint ) );
}

uno::Reference< XAccessible > SAL_CALL SvtRulerAccessible::getAccessibleAtPoint( const awt::Point& ) throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard           aGuard( m_aMutex );

    ThrowExceptionIfNotAlive();

    uno::Reference< XAccessible >   xRet;


    return xRet;
}

awt::Rectangle SAL_CALL SvtRulerAccessible::getBounds() throw( RuntimeException, std::exception )
{
    // no guard -> done in GetBoundingBox()
    return AWTRectangle( GetBoundingBox() );
}

awt::Point SAL_CALL SvtRulerAccessible::getLocation() throw( RuntimeException, std::exception )
{
    // no guard -> done in GetBoundingBox()
    return AWTPoint( GetBoundingBox().TopLeft() );
}

awt::Point SAL_CALL SvtRulerAccessible::getLocationOnScreen() throw( RuntimeException, std::exception )
{
    // no guard -> done in GetBoundingBoxOnScreen()
    return AWTPoint( GetBoundingBoxOnScreen().TopLeft() );
}

awt::Size SAL_CALL SvtRulerAccessible::getSize() throw( RuntimeException, std::exception )
{
    // no guard -> done in GetBoundingBox()
    return AWTSize( GetBoundingBox().GetSize() );
}

bool SAL_CALL SvtRulerAccessible::isVisible() throw( RuntimeException )
{
    ::osl::MutexGuard           aGuard( m_aMutex );

    ThrowExceptionIfNotAlive();

    return mpRepr->IsVisible();
}

//=====  XAccessibleContext  ==================================================
sal_Int32 SAL_CALL SvtRulerAccessible::getAccessibleChildCount() throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard   aGuard( m_aMutex );

    ThrowExceptionIfNotAlive();

    return 0;
}

uno::Reference< XAccessible > SAL_CALL SvtRulerAccessible::getAccessibleChild( sal_Int32 )
    throw( RuntimeException, lang::IndexOutOfBoundsException, std::exception )
{
    uno::Reference< XAccessible >   xChild ;

    return xChild;
}

uno::Reference< XAccessible > SAL_CALL SvtRulerAccessible::getAccessibleParent() throw( RuntimeException, std::exception )
{
    return mxParent;
}

sal_Int32 SAL_CALL SvtRulerAccessible::getAccessibleIndexInParent() throw( RuntimeException, std::exception )
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

sal_Int16 SAL_CALL SvtRulerAccessible::getAccessibleRole() throw( RuntimeException, std::exception )
{
    return AccessibleRole::RULER;
}

OUString SAL_CALL SvtRulerAccessible::getAccessibleDescription() throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    return msDescription;
}

OUString SAL_CALL SvtRulerAccessible::getAccessibleName() throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    return msName;
}

/** Return empty uno::Reference to indicate that the relation set is not
    supported.
*/
uno::Reference< XAccessibleRelationSet > SAL_CALL SvtRulerAccessible::getAccessibleRelationSet() throw( RuntimeException, std::exception )
{
    return uno::Reference< XAccessibleRelationSet >();
}


uno::Reference< XAccessibleStateSet > SAL_CALL SvtRulerAccessible::getAccessibleStateSet() throw( RuntimeException, std::exception )
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

lang::Locale SAL_CALL SvtRulerAccessible::getLocale() throw( IllegalAccessibleComponentStateException, RuntimeException, std::exception )
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
    throw( RuntimeException, std::exception )
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
    throw( RuntimeException, std::exception )
{
    if (xListener.is())
    {
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
}

void SAL_CALL SvtRulerAccessible::grabFocus() throw( RuntimeException, std::exception )
{
    SolarMutexGuard     aSolarGuard;
    ::osl::MutexGuard   aGuard( m_aMutex );

    ThrowExceptionIfNotAlive();

    mpRepr->GrabFocus();
}

sal_Int32 SvtRulerAccessible::getForeground(  )
        throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard     aSolarGuard;
    ::osl::MutexGuard   aGuard( m_aMutex );
    ThrowExceptionIfNotAlive();

    return mpRepr->GetControlForeground().GetColor();
}
sal_Int32 SvtRulerAccessible::getBackground(  )
        throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard     aSolarGuard;
    ::osl::MutexGuard   aGuard( m_aMutex );
    ThrowExceptionIfNotAlive();

    return mpRepr->GetControlBackground().GetColor();
}

// XServiceInfo
OUString SAL_CALL SvtRulerAccessible::getImplementationName() throw( RuntimeException, std::exception )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.ui.SvtRulerAccessible" ) );
}

sal_Bool SAL_CALL SvtRulerAccessible::supportsService( const OUString& sServiceName ) throw( RuntimeException, std::exception )
{
    return cppu::supportsService( this, sServiceName );
}

Sequence< OUString > SAL_CALL SvtRulerAccessible::getSupportedServiceNames() throw( RuntimeException, std::exception )
{
    const OUString sServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.accessibility.AccessibleContext" ) );
    return Sequence< OUString >( &sServiceName, 1 );
}

//=====  XTypeProvider  =======================================================
Sequence< sal_Int8 > SAL_CALL SvtRulerAccessible::getImplementationId() throw( RuntimeException, std::exception )
{
    return css::uno::Sequence<sal_Int8>();
}

void SAL_CALL SvtRulerAccessible::disposing()
{
    if( !rBHelper.bDisposed )
    {
        {
            ::osl::MutexGuard   aGuard( m_aMutex );
            mpRepr = nullptr;      // object dies with representation

        }

        {
            ::osl::MutexGuard   aGuard( m_aMutex );

            // Send a disposing to all listeners.
            if ( mnClientId )
            {
                comphelper::AccessibleEventNotifier::revokeClientNotifyDisposing( mnClientId, *this );
                mnClientId =  0;
            }
            mxParent.clear();
        }
    }
}

Rectangle SvtRulerAccessible::GetBoundingBoxOnScreen() throw( RuntimeException )
{
    SolarMutexGuard     aSolarGuard;
    ::osl::MutexGuard   aGuard( m_aMutex );

    ThrowExceptionIfNotAlive();
    return Rectangle( mpRepr->GetParent()->OutputToAbsoluteScreenPixel( mpRepr->GetPosPixel() ), mpRepr->GetSizePixel() );
}

Rectangle SvtRulerAccessible::GetBoundingBox() throw( RuntimeException )
{
    SolarMutexGuard     aSolarGuard;
    ::osl::MutexGuard   aGuard( m_aMutex );

    ThrowExceptionIfNotAlive();

    return Rectangle( mpRepr->GetPosPixel(), mpRepr->GetSizePixel() );
}

void SvtRulerAccessible::ThrowExceptionIfNotAlive() throw( lang::DisposedException )
{
    if( IsNotAlive() )
        throw lang::DisposedException();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
