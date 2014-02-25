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
#include <comphelper/accessiblekeybindinghelper.hxx>
#include <swurl.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <ndtxt.hxx>
#include <txtinet.hxx>

#include <accpara.hxx>
#include <accfield.hxx>

#include <comphelper/processfactory.hxx>

#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/document/XLinkTargetSupplier.hpp>
#include <comphelper/accessibleeventnotifier.hxx>

//Add end

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;

SwAccessibleField::SwAccessibleField( SwField *pSwFld,SwAccessibleParagraph *p,sal_Int16 nRole) :
    m_xPara( p ),m_pSwField(pSwFld),m_nRole(nRole)
{
    m_nClientId=0;
}

uno::Reference< XAccessibleContext > SAL_CALL
    SwAccessibleField::getAccessibleContext( void )
        throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    uno::Reference < XAccessibleContext > xRet( this );
    return xRet;
}

sal_Int32 SAL_CALL SwAccessibleField::getAccessibleChildCount( void )
        throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return 0;
}

uno::Reference< XAccessible> SAL_CALL
    SwAccessibleField::getAccessibleChild( sal_Int32 )
        throw (::com::sun::star::uno::RuntimeException,
                ::com::sun::star::lang::IndexOutOfBoundsException, std::exception)
{
    SolarMutexGuard aGuard;
    return uno::Reference< XAccessible >();
}

uno::Reference< XAccessible> SAL_CALL SwAccessibleField::getAccessibleParent (void)
        throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    uno::Reference< XAccessible > xParent(static_cast<XAccessible*>(m_xPara.get()),UNO_QUERY);
    return xParent;
}

sal_Int32 SAL_CALL SwAccessibleField::getAccessibleIndexInParent (void)
        throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return 0;
}

sal_Int16 SAL_CALL SwAccessibleField::getAccessibleRole (void)
        throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;
    return m_nRole;
}

OUString SAL_CALL SwAccessibleField::getAccessibleDescription (void)
        throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    OSL_ENSURE( !this, "description needs to be overloaded" );
    return OUString();
}

OUString SAL_CALL SwAccessibleField::getAccessibleName (void)
        throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    return OUString();
}

uno::Reference< XAccessibleRelationSet> SAL_CALL
    SwAccessibleField::getAccessibleRelationSet (void)
        throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    return NULL;
}

uno::Reference<XAccessibleStateSet> SAL_CALL
    SwAccessibleField::getAccessibleStateSet (void)
        throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return uno::Reference<XAccessibleStateSet>();
}

com::sun::star::lang::Locale SAL_CALL SwAccessibleField::getLocale (void)
        throw (::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    com::sun::star::lang::Locale aLoc( Application::GetSettings().GetLanguageTag().getLocale() );
    return aLoc;
}

static sal_Bool lcl_PointInRectangle(const awt::Point & aPoint,
                                     const awt::Rectangle & aRect)
{
    long nDiffX = aPoint.X - aRect.X;
    long nDiffY = aPoint.Y - aRect.Y;

    return
        nDiffX >= 0 && nDiffX < aRect.Width && nDiffY >= 0 &&
        nDiffY < aRect.Height;

}

sal_Bool SAL_CALL SwAccessibleField::containsPoint(
            const ::com::sun::star::awt::Point& aPoint )
        throw (RuntimeException, std::exception)
{
    awt::Rectangle aPixBounds = getBoundsImpl(sal_True);
    aPixBounds.X = 0;
    aPixBounds.Y = 0;

    return lcl_PointInRectangle(aPoint, aPixBounds);
}

uno::Reference< XAccessible > SAL_CALL SwAccessibleField::getAccessibleAtPoint(
                const awt::Point& aPoint )
        throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    uno::Reference< XAccessible > xAcc;
    awt::Rectangle rc = getBounds();
    if(aPoint.X >= rc.X && aPoint.X <= rc.X + rc.Width &&
        aPoint.Y >= rc.Y && aPoint.Y <= rc.Y + rc.Height )
    {
        xAcc =this;
    }
    return xAcc;
}

/**
   Get bounding box.

   There are two modes.

   - realative

     Return bounding box relative to parent if parent is no root
     frame. Otherwise return the absolute bounding box.

   - absolute

     Return the absolute bounding box.

   @param bRelative
   true: Use relative mode.
   false: Use absolute mode.
*/
awt::Rectangle SAL_CALL SwAccessibleField::getBoundsImpl( sal_Bool )
        throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    return awt::Rectangle();
}

awt::Rectangle SAL_CALL SwAccessibleField::getBounds()
        throw (RuntimeException, std::exception)
{
    return getBoundsImpl(sal_True);
}

awt::Point SAL_CALL SwAccessibleField::getLocation()
    throw (RuntimeException, std::exception)
{
    awt::Rectangle aRect = getBoundsImpl(sal_True);
    awt::Point aPoint(aRect.X, aRect.Y);

    return aPoint;
}

awt::Point SAL_CALL SwAccessibleField::getLocationOnScreen()
        throw (RuntimeException, std::exception)
{
    awt::Rectangle aRect = getBoundsImpl(sal_False);
    //Point aPixPos = m_xPara->getLocationOnScreen();
    return awt::Point( aRect.X,aRect.Y);//aPixPos.X() + aRect.nLeft , aPixPos.Y() + + aRect.nRight );
}

awt::Size SAL_CALL SwAccessibleField::getSize()
        throw (RuntimeException, std::exception)
{
    awt::Rectangle aRect = getBoundsImpl(sal_False);
    awt::Size aSize( aRect.Width, aRect.Height );

    return aSize;
}

void SAL_CALL SwAccessibleField::grabFocus()
        throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return;
}

sal_Int32 SAL_CALL SwAccessibleField::getForeground()
        throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    return 0;
}

sal_Int32 SAL_CALL SwAccessibleField::getBackground()
        throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    return 0xffffff;
}
::com::sun::star::uno::Any SAL_CALL SwAccessibleField::queryInterface(
        const ::com::sun::star::uno::Type& rType )
        throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    Any aRet;
    if ( rType == ::getCppuType((uno::Reference<XAccessibleContext> *)0) )
    {
        Reference<XAccessibleContext> aAccContext = (XAccessibleContext *) this; // resolve ambiguity
        aRet <<= aAccContext;
    }
    else if ( rType == ::getCppuType((Reference<XAccessibleComponent> *)0) )
    {
        Reference<XAccessibleComponent> aAccEditComponent = this;
        aRet <<= aAccEditComponent;
    }
    if (rType == ::getCppuType((Reference<XAccessibleEventBroadcaster> *)0))
    {
        Reference<XAccessibleEventBroadcaster> aAccBroadcaster= this;
        aRet <<= aAccBroadcaster;
    }
    return aRet;
}

void SAL_CALL SwAccessibleField::acquire(  ) throw ()
{
}
void SAL_CALL SwAccessibleField::release(  ) throw ()
{
}

void SAL_CALL SwAccessibleField::addEventListener(
            const Reference< XAccessibleEventListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException)
{
    //DBG_MSG( "accessible event listener added" )

    if (xListener.is())
    {
        SolarMutexGuard aGuard;
        if (!m_nClientId)
            m_nClientId = comphelper::AccessibleEventNotifier::registerClient( );
        comphelper::AccessibleEventNotifier::addEventListener( m_nClientId, xListener );
    }
}

void SAL_CALL SwAccessibleField::removeEventListener(
            const Reference< XAccessibleEventListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException)
{
    //DBG_MSG( "accessible event listener removed" )

    if (xListener.is())
    {
        SolarMutexGuard aGuard;
        sal_Int32 nListenerCount = comphelper::AccessibleEventNotifier::removeEventListener( m_nClientId, xListener );
        if ( !nListenerCount )
        {
            // no listeners anymore
            // -> revoke ourself. This may lead to the notifier thread dying (if we were the last client),
            // and at least to us not firing any events anymore, in case somebody calls
            // NotifyAccessibleEvent, again
            comphelper::AccessibleEventNotifier::revokeClient( m_nClientId );
            m_nClientId = 0;
        }
    }
}
