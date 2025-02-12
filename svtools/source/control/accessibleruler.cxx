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
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/IllegalAccessibleComponentStateException.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <comphelper/accessibleeventnotifier.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <utility>
#include <vcl/svapp.hxx>
#include <vcl/unohelp.hxx>
#include <osl/mutex.hxx>
#include <tools/gen.hxx>

#include <svtools/ruler.hxx>
#include "accessibleruler.hxx"

using namespace ::cppu;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;


//=====  internal  ============================================================

SvtRulerAccessible::SvtRulerAccessible(
    uno::Reference< XAccessible > xParent, Ruler& rRepr, OUString aName ) :

    msName(std::move( aName )),
    mxParent(std::move( xParent )),
    mpRepr( &rRepr ),
    mnClientId( 0 )
{
}

SvtRulerAccessible::~SvtRulerAccessible()
{
    if( !m_bDisposed )
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
    SolarMutexGuard aSolarGuard;

//  return GetBoundingBox().IsInside( vcl::unohelper::ConvertToVCLPoint( rPoint ) );
    return tools::Rectangle(Point(0, 0), GetBoundingBox().GetSize())
        .Contains(vcl::unohelper::ConvertToVCLPoint(rPoint));
}

uno::Reference< XAccessible > SAL_CALL SvtRulerAccessible::getAccessibleAtPoint( const awt::Point& )
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard( m_aMutex );
    throwIfDisposed(aGuard);

    return uno::Reference< XAccessible >();
}

awt::Rectangle SAL_CALL SvtRulerAccessible::getBounds()
{
    SolarMutexGuard aSolarGuard;
    return vcl::unohelper::ConvertToAWTRect(GetBoundingBox());
}

awt::Point SAL_CALL SvtRulerAccessible::getLocation()
{
    SolarMutexGuard aSolarGuard;
    return vcl::unohelper::ConvertToAWTPoint(GetBoundingBox().TopLeft());
}

awt::Point SAL_CALL SvtRulerAccessible::getLocationOnScreen()
{
    SolarMutexGuard aSolarGuard;
    return vcl::unohelper::ConvertToAWTPoint(GetBoundingBoxOnScreen().TopLeft());
}

awt::Size SAL_CALL SvtRulerAccessible::getSize()
{
    SolarMutexGuard aSolarGuard;
    return vcl::unohelper::ConvertToAWTSize(GetBoundingBox().GetSize());
}

//=====  XAccessibleContext  ==================================================
sal_Int64 SAL_CALL SvtRulerAccessible::getAccessibleChildCount()
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard( m_aMutex );
    throwIfDisposed(aGuard);

    return 0;
}

uno::Reference< XAccessible > SAL_CALL SvtRulerAccessible::getAccessibleChild( sal_Int64 )
{
    SolarMutexGuard aSolarGuard;
    uno::Reference< XAccessible >   xChild ;

    return xChild;
}

uno::Reference< XAccessible > SAL_CALL SvtRulerAccessible::getAccessibleParent()
{
    SolarMutexGuard aSolarGuard;
    return mxParent;
}

sal_Int64 SAL_CALL SvtRulerAccessible::getAccessibleIndexInParent()
{
    SolarMutexGuard aSolarGuard;

    //  Use a simple but slow solution for now.  Optimize later.

    //  Iterate over all the parent's children and search for this object.
    if( mxParent.is() )
    {
        uno::Reference< XAccessibleContext >        xParentContext( mxParent->getAccessibleContext() );
        if( xParentContext.is() )
        {
            sal_Int64 nChildCount = xParentContext->getAccessibleChildCount();
            for( sal_Int64 i = 0 ; i < nChildCount ; ++i )
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
    SolarMutexGuard aSolarGuard;
    return AccessibleRole::RULER;
}

OUString SAL_CALL SvtRulerAccessible::getAccessibleDescription()
{
    SolarMutexGuard aSolarGuard;
    return OUString();
}

OUString SAL_CALL SvtRulerAccessible::getAccessibleName()
{
    SolarMutexGuard aSolarGuard;
    return msName;
}

/** Return empty uno::Reference to indicate that the relation set is not
    supported.
*/
uno::Reference< XAccessibleRelationSet > SAL_CALL SvtRulerAccessible::getAccessibleRelationSet()
{
    SolarMutexGuard aSolarGuard;
    return uno::Reference< XAccessibleRelationSet >();
}


sal_Int64 SAL_CALL SvtRulerAccessible::getAccessibleStateSet()
{
    SolarMutexGuard aSolarGuard;

    sal_Int64 nStateSet = 0;

    if( !m_bDisposed )
    {
        nStateSet |= AccessibleStateType::ENABLED;

        nStateSet |= AccessibleStateType::SHOWING;

        if( mpRepr->IsVisible() )
            nStateSet |= AccessibleStateType::VISIBLE;

        if ( mpRepr->GetStyle() & WB_HORZ )
            nStateSet |= AccessibleStateType::HORIZONTAL;
        else
            nStateSet |= AccessibleStateType::VERTICAL;
    }

    return nStateSet;
}

lang::Locale SAL_CALL SvtRulerAccessible::getLocale()
{
    SolarMutexGuard aSolarGuard;

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
        std::unique_lock aGuard( m_aMutex );
        if (!mnClientId)
            mnClientId = comphelper::AccessibleEventNotifier::registerClient( );
        comphelper::AccessibleEventNotifier::addEventListener( mnClientId, xListener );
    }
}

void SAL_CALL SvtRulerAccessible::removeAccessibleEventListener( const uno::Reference< XAccessibleEventListener >& xListener )
{
    if (!xListener.is())
        return;

    std::unique_lock aGuard( m_aMutex );

    if (!mnClientId)
        return;

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
    SolarMutexGuard aSolarGuard;

    if (!mpRepr)
        throw css::lang::DisposedException(OUString(), static_cast<cppu::OWeakObject*>(this));

    mpRepr->GrabFocus();
}

sal_Int32 SvtRulerAccessible::getForeground(  )
{
    SolarMutexGuard aSolarGuard;

    if (!mpRepr)
        throw css::lang::DisposedException(OUString(), static_cast<cppu::OWeakObject*>(this));

    return sal_Int32(mpRepr->GetControlForeground());
}
sal_Int32 SvtRulerAccessible::getBackground(  )
{
    SolarMutexGuard aSolarGuard;

    if (!mpRepr)
        throw css::lang::DisposedException(OUString(), static_cast<cppu::OWeakObject*>(this));

    return sal_Int32(mpRepr->GetControlBackground());
}

// XServiceInfo
OUString SAL_CALL SvtRulerAccessible::getImplementationName()
{
    return u"com.sun.star.comp.ui.SvtRulerAccessible"_ustr;
}

sal_Bool SAL_CALL SvtRulerAccessible::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService( this, sServiceName );
}

Sequence< OUString > SAL_CALL SvtRulerAccessible::getSupportedServiceNames()
{
    return { u"com.sun.star.accessibility.AccessibleContext"_ustr };
}

//=====  XTypeProvider  =======================================================
Sequence< sal_Int8 > SAL_CALL SvtRulerAccessible::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

void SvtRulerAccessible::disposing(std::unique_lock<std::mutex>&)
{
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
    if (!mpRepr)
        throw css::lang::DisposedException(OUString(), static_cast<cppu::OWeakObject*>(this));

    return tools::Rectangle(mpRepr->GetParent()->OutputToAbsoluteScreenPixel(mpRepr->GetPosPixel()), mpRepr->GetSizePixel());
}

tools::Rectangle SvtRulerAccessible::GetBoundingBox()
{
    if (!mpRepr)
        throw css::lang::DisposedException(OUString(), static_cast<cppu::OWeakObject*>(this));

    return tools::Rectangle(mpRepr->GetPosPixel(), mpRepr->GetSizePixel());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
