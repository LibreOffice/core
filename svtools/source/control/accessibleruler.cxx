/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#ifndef _SVTRULERACCESSIBLE_HXX
#include <svtools/accessibleruler.hxx>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX_
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYCHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif

#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif
#ifndef _TOOLKIT_HELPER_CONVERT_HXX_
#include <toolkit/helper/convert.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif

#include "ruler.hxx"

#ifndef COMPHELPER_ACCESSIBLE_EVENT_NOTIFIER
#include <comphelper/accessibleeventnotifier.hxx>
#endif

using namespace ::cppu;
using namespace ::osl;
using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;

DBG_NAME( SvtRulerAccessible )


//=====  internal  ============================================================

SvtRulerAccessible::SvtRulerAccessible(
    const uno::Reference< XAccessible >& rxParent, Ruler& rRepr, const OUString& rName ) :

    SvtRulerAccessible_Base( m_aMutex ),
    msName( rName ),
    mxParent( rxParent ),
    mpRepr( &rRepr ),
    mnClientId( 0 )
{
    DBG_CTOR( SvtRulerAccessible, NULL );
}

SvtRulerAccessible::~SvtRulerAccessible()
{
    DBG_DTOR( SvtRulerAccessible, NULL );

    if( IsAlive() )
    {
        osl_incrementInterlockedCount( &m_refCount );
        dispose();      // set mpRepr = NULL & release all childs
    }
}

//=====  XAccessible  =========================================================

uno::Reference< XAccessibleContext > SAL_CALL SvtRulerAccessible::getAccessibleContext( void ) throw( RuntimeException )
{
    return this;
}

//=====  XAccessibleComponent  ================================================

sal_Bool SAL_CALL SvtRulerAccessible::containsPoint( const awt::Point& rPoint ) throw( RuntimeException )
{
    // no guard -> done in getBounds()
//  return GetBoundingBox().IsInside( VCLPoint( rPoint ) );
    return Rectangle( Point( 0, 0 ), GetBoundingBox().GetSize() ).IsInside( VCLPoint( rPoint ) );
}

uno::Reference< XAccessible > SAL_CALL SvtRulerAccessible::getAccessibleAtPoint( const awt::Point& ) throw( RuntimeException )
{
    ::osl::MutexGuard           aGuard( m_aMutex );

    ThrowExceptionIfNotAlive();

    uno::Reference< XAccessible >   xRet;


    return xRet;
}

awt::Rectangle SAL_CALL SvtRulerAccessible::getBounds() throw( RuntimeException )
{
    // no guard -> done in GetBoundingBox()
    return AWTRectangle( GetBoundingBox() );
}

awt::Point SAL_CALL SvtRulerAccessible::getLocation() throw( RuntimeException )
{
    // no guard -> done in GetBoundingBox()
    return AWTPoint( GetBoundingBox().TopLeft() );
}

awt::Point SAL_CALL SvtRulerAccessible::getLocationOnScreen() throw( RuntimeException )
{
    // no guard -> done in GetBoundingBoxOnScreen()
    return AWTPoint( GetBoundingBoxOnScreen().TopLeft() );
}

awt::Size SAL_CALL SvtRulerAccessible::getSize() throw( RuntimeException )
{
    // no guard -> done in GetBoundingBox()
    return AWTSize( GetBoundingBox().GetSize() );
}

sal_Bool SAL_CALL SvtRulerAccessible::isShowing() throw( RuntimeException )
{
    return sal_True;
}

sal_Bool SAL_CALL SvtRulerAccessible::isVisible() throw( RuntimeException )
{
    ::osl::MutexGuard           aGuard( m_aMutex );

    ThrowExceptionIfNotAlive();

    return mpRepr->IsVisible();
}

sal_Bool SAL_CALL SvtRulerAccessible::isFocusTraversable() throw( RuntimeException )
{
    return sal_True;
}

//=====  XAccessibleContext  ==================================================

sal_Int32 SAL_CALL SvtRulerAccessible::getAccessibleChildCount( void ) throw( RuntimeException )
{
    ::osl::MutexGuard   aGuard( m_aMutex );

    ThrowExceptionIfNotAlive();

    return 0;
}

uno::Reference< XAccessible > SAL_CALL SvtRulerAccessible::getAccessibleChild( sal_Int32 )
    throw( RuntimeException, lang::IndexOutOfBoundsException )
{
    uno::Reference< XAccessible >   xChild ;

    return xChild;
}

uno::Reference< XAccessible > SAL_CALL SvtRulerAccessible::getAccessibleParent( void ) throw( RuntimeException )
{
    return mxParent;
}

sal_Int32 SAL_CALL SvtRulerAccessible::getAccessibleIndexInParent( void ) throw( RuntimeException )
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
                if( xChild.get() == ( XAccessible* ) this )
                    return i;
            }
        }
   }

   //   Return -1 to indicate that this object's parent does not know about the
   //   object.
   return -1;
}

sal_Int16 SAL_CALL SvtRulerAccessible::getAccessibleRole( void ) throw( RuntimeException )
{
    return AccessibleRole::RULER;
}

OUString SAL_CALL SvtRulerAccessible::getAccessibleDescription( void ) throw( RuntimeException )
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    return msDescription;
}

OUString SAL_CALL SvtRulerAccessible::getAccessibleName( void ) throw( RuntimeException )
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    return msName;
}

/** Return empty uno::Reference to indicate that the relation set is not
    supported.
*/
uno::Reference< XAccessibleRelationSet > SAL_CALL SvtRulerAccessible::getAccessibleRelationSet( void ) throw( RuntimeException )
{
    return uno::Reference< XAccessibleRelationSet >();
}


uno::Reference< XAccessibleStateSet > SAL_CALL SvtRulerAccessible::getAccessibleStateSet( void ) throw( RuntimeException )
{
    ::osl::MutexGuard                       aGuard( m_aMutex );
    utl::AccessibleStateSetHelper*          pStateSetHelper = new utl::AccessibleStateSetHelper;

    if( IsAlive() )
    {
        pStateSetHelper->AddState( AccessibleStateType::ENABLED );

        if( isShowing() )
            pStateSetHelper->AddState( AccessibleStateType::SHOWING );

        if( isVisible() )
            pStateSetHelper->AddState( AccessibleStateType::VISIBLE );


        if ( mpRepr )
        {
            if ( mpRepr->GetStyle() & WB_HORZ )
                pStateSetHelper->AddState( AccessibleStateType::HORIZONTAL );
            else
                pStateSetHelper->AddState( AccessibleStateType::VERTICAL );
        }
        if(pStateSetHelper->contains(AccessibleStateType::FOCUSABLE))
        {
            pStateSetHelper->RemoveState( AccessibleStateType::FOCUSABLE );
        }

    }


    return pStateSetHelper;
}

lang::Locale SAL_CALL SvtRulerAccessible::getLocale( void ) throw( IllegalAccessibleComponentStateException, RuntimeException )
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

void SAL_CALL SvtRulerAccessible::addEventListener( const uno::Reference< XAccessibleEventListener >& xListener )
    throw( RuntimeException )
{
    if (xListener.is())
    {
        ::osl::MutexGuard   aGuard( m_aMutex );
        if (!mnClientId)
            mnClientId = comphelper::AccessibleEventNotifier::registerClient( );
        comphelper::AccessibleEventNotifier::addEventListener( mnClientId, xListener );
    }
}

void SAL_CALL SvtRulerAccessible::removeEventListener( const uno::Reference< XAccessibleEventListener >& xListener )
    throw( RuntimeException )
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

void SAL_CALL SvtRulerAccessible::addFocusListener( const uno::Reference< awt::XFocusListener >& xListener )
    throw( RuntimeException )
{
    if( xListener.is() )
    {
        ::osl::MutexGuard   aGuard( m_aMutex );

        ThrowExceptionIfNotAlive();

        uno::Reference< awt::XWindow >  xWindow = VCLUnoHelper::GetInterface( mpRepr );
        if( xWindow.is() )
            xWindow->addFocusListener( xListener );
    }
}

void SAL_CALL SvtRulerAccessible::removeFocusListener( const uno::Reference< awt::XFocusListener >& xListener )
    throw (RuntimeException)
{
    if( xListener.is() )
    {
        ::osl::MutexGuard   aGuard( m_aMutex );

        ThrowExceptionIfNotAlive();

        uno::Reference< awt::XWindow >  xWindow = VCLUnoHelper::GetInterface( mpRepr );
        if( xWindow.is() )
            xWindow->removeFocusListener( xListener );
    }
}

void SAL_CALL SvtRulerAccessible::grabFocus() throw( RuntimeException )
{
    ::vos::OGuard       aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard   aGuard( m_aMutex );

    ThrowExceptionIfNotAlive();

    mpRepr->GrabFocus();
}

Any SAL_CALL SvtRulerAccessible::getAccessibleKeyBinding() throw( RuntimeException )
{
    // here is no implementation, because here are no KeyBindings for every object
    return Any();
}

sal_Int32 SvtRulerAccessible::getForeground(  )
        throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard       aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard   aGuard( m_aMutex );
    ThrowExceptionIfNotAlive();

    return mpRepr->GetControlForeground().GetColor();
}
sal_Int32 SvtRulerAccessible::getBackground(  )
        throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard       aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard   aGuard( m_aMutex );
    ThrowExceptionIfNotAlive();

    return mpRepr->GetControlBackground().GetColor();
}

//=====  XServiceInfo  ========================================================

OUString SAL_CALL SvtRulerAccessible::getImplementationName( void ) throw( RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.ui.SvtRulerAccessible" ) );
}

sal_Bool SAL_CALL SvtRulerAccessible::supportsService( const OUString& sServiceName ) throw( RuntimeException )
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    //  Iterate over all supported service names and return true if on of them
    //  matches the given name.
    Sequence< OUString >    aSupportedServices( getSupportedServiceNames() );
    int                     nLength = aSupportedServices.getLength();
    const OUString*         pStr = aSupportedServices.getConstArray();

    for( int i = nLength ; i ; --i, ++pStr )
    {
        if( sServiceName == *pStr )
            return sal_True;
    }

    return sal_False;
}

Sequence< OUString > SAL_CALL SvtRulerAccessible::getSupportedServiceNames( void ) throw( RuntimeException )
{
    const OUString sServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.accessibility.AccessibleContext" ) );
    return Sequence< OUString >( &sServiceName, 1 );
}

//=====  XTypeProvider  =======================================================

Sequence< sal_Int8 > SAL_CALL SvtRulerAccessible::getImplementationId( void ) throw( RuntimeException )
{
    return getUniqueId();
}


//=====  internals ========================================================

void SvtRulerAccessible::setName( const OUString& rName )
{
        msName = rName;

}

void SvtRulerAccessible::setDescription( const OUString& rDescr )
{

        msDescription = rDescr;

}



void SAL_CALL SvtRulerAccessible::disposing()
{
    if( !rBHelper.bDisposed )
    {
        {
            ::osl::MutexGuard   aGuard( m_aMutex );
            mpRepr = NULL;      // object dies with representation

        }

        {
            ::osl::MutexGuard   aGuard( m_aMutex );

            // Send a disposing to all listeners.
            if ( mnClientId )
            {
                comphelper::AccessibleEventNotifier::revokeClientNotifyDisposing( mnClientId, *this );
                mnClientId =  0;
            }
            mxParent = uno::Reference< XAccessible >();
        }
    }
}

Rectangle SvtRulerAccessible::GetBoundingBoxOnScreen( void ) throw( RuntimeException )
{
    ::vos::OGuard       aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard   aGuard( m_aMutex );

    ThrowExceptionIfNotAlive();
    //the absolute on screen pixel is wrong
    //return Rectangle( mpRepr->GetParent()->OutputToScreenPixel( mpRepr->GetPosPixel() ), mpRepr->GetSizePixel() );
    return Rectangle( mpRepr->GetParent()->OutputToAbsoluteScreenPixel( mpRepr->GetPosPixel() ), mpRepr->GetSizePixel() );
}

Rectangle SvtRulerAccessible::GetBoundingBox( void ) throw( RuntimeException )
{
    ::vos::OGuard       aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard   aGuard( m_aMutex );

    ThrowExceptionIfNotAlive();

    return Rectangle( mpRepr->GetPosPixel(), mpRepr->GetSizePixel() );
}

Sequence< sal_Int8 > SvtRulerAccessible::getUniqueId( void )
{
    static OImplementationId*   pId = 0;
    if( !pId )
    {
        MutexGuard                      aGuard( Mutex::getGlobalMutex() );
        if( !pId)
        {
            static OImplementationId    aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

void SvtRulerAccessible::ThrowExceptionIfNotAlive( void ) throw( lang::DisposedException )
{
    if( IsNotAlive() )
        throw lang::DisposedException();
}

void SvtRulerAccessible::addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener )throw( com::sun::star::uno::RuntimeException )
{
    cppu::WeakAggComponentImplHelperBase::addEventListener( xListener );
}

void SvtRulerAccessible::removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw( com::sun::star::uno::RuntimeException )
{
    cppu::WeakAggComponentImplHelperBase::removeEventListener( xListener );
}


// -------------------------------------------------------------------------------------------------

