/*************************************************************************
 *
 *  $RCSfile: DrawController.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-01-20 12:31:25 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "DrawController.hxx"

#ifndef SD_DRAW_SUB_CONTROLLER_HXX
#include "DrawSubController.hxx"
#endif
#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif

using namespace ::std;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::vos;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sd {

enum properties
{
    PROPERTY_WORKAREA = 0,
    PROPERTY_PRIMARY_SUB_CONTROLLER,
    PROPERTY_COUNT
};


DrawController::DrawController (ViewShellBase& rBase) throw()
    : SfxBaseController (&rBase),
      OBroadcastHelper(DrawControllerMutexOwner::maMutex),
      OPropertySetHelper( *static_cast<OBroadcastHelperVar<
          OMultiTypeInterfaceContainerHelper,
          OMultiTypeInterfaceContainerHelper::keyType> *>(this)),
      mrBase(rBase),
      mbDisposing(false),
      mpSubController(NULL)
{
}




DrawController::~DrawController (void) throw()
{
}




DrawSubController* DrawController::GetSubController (void)
{
    if (mpSubController == NULL)
    {
        ::osl::MutexGuard aGuard (maMutex);
        if (mpSubController == NULL)
        {
            mpSubController = mrBase.GetSubShellManager().GetMainSubShell()
                ->GetSubController();

            // Register as listener at the sub-controller.
            Reference<XComponent> xEventBroadcaster (
                static_cast<XWeak*>(mpSubController), UNO_QUERY);
            if (xEventBroadcaster.is())
                xEventBroadcaster->addEventListener (this);

            Reference<XSelectionSupplier> xSelectionBroadcaster (
                static_cast<XWeak*>(mpSubController), UNO_QUERY);
            if (xSelectionBroadcaster.is())
                xSelectionBroadcaster->addSelectionChangeListener (this);
        }
    }
    return mpSubController;
}




// XInterface

IMPLEMENT_FORWARD_XINTERFACE3(
    DrawController,
    SfxBaseController,
    OPropertySetHelper,
    DrawControllerInterfaceBase);


// XTypeProvider

Sequence<Type> SAL_CALL DrawController::getTypes (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    // OPropertySetHelper does not provide getTypes, so we have to
    // implement this method manually and list its three interfaces.
    OTypeCollection aTypeCollection (
        ::getCppuType (( const Reference<beans::XMultiPropertySet>*)NULL),
        ::getCppuType (( const Reference<beans::XFastPropertySet>*)NULL),
        ::getCppuType (( const Reference<beans::XPropertySet>*)NULL));

    return ::comphelper::concatSequences(
        SfxBaseController::getTypes(),
        aTypeCollection.getTypes(),
        DrawControllerInterfaceBase::getTypes());
}

IMPLEMENT_GET_IMPLEMENTATION_ID(DrawController);



// XComponent


void SAL_CALL DrawController::dispose()
    throw( RuntimeException )
{
    if( !mbDisposing )
    {
        OGuard aGuard( Application::GetSolarMutex() );

        if( !mbDisposing )
        {
            mbDisposing = true;

            SfxBaseController::dispose();
        }
    }
}




void SAL_CALL DrawController::addEventListener(
    const Reference<lang::XEventListener >& xListener)
    throw (RuntimeException)
{
    SfxBaseController::addEventListener( xListener );
}




void SAL_CALL DrawController::removeEventListener (
    const Reference<lang::XEventListener >& aListener)
    throw (RuntimeException)
{
    SfxBaseController::removeEventListener( aListener );
}




// XServiceInfo

OUString SAL_CALL DrawController::getImplementationName(  ) throw(RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "DrawController" ) );
}



static OUString ssServiceName (OUString::createFromAscii(
    "com.sun.star.drawing.DrawController"));

sal_Bool SAL_CALL DrawController::supportsService (
    const OUString& rsServiceName)
    throw(RuntimeException)
{
    return rsServiceName.equals(ssServiceName);
}




Sequence<OUString> SAL_CALL DrawController::getSupportedServiceNames (void)
    throw(RuntimeException)
{
    Sequence<OUString> aSupportedServices (1);
    OUString* pServices = aSupportedServices.getArray();
    pServices[0] = ssServiceName;
    return aSupportedServices;
}




//------ XSelectionSupplier --------------------------------------------

sal_Bool SAL_CALL DrawController::select (const Any& aSelection)
    throw(lang::IllegalArgumentException, RuntimeException)
{
    sal_Bool bResult = sal_False;

    // Forward call to sub-controller of currently active sub-shell.
    DrawSubController* pSubController = GetSubController();
    if (pSubController != NULL)
        bResult = pSubController->select (aSelection);

    return bResult;
}




Any SAL_CALL DrawController::getSelection()
    throw(RuntimeException)
{
    Any aSelection;

    // Forward call to sub-controller of currently active sub-shell.
    DrawSubController* pSubController = GetSubController();
    if (pSubController != NULL)
        aSelection = pSubController->getSelection();

    return aSelection;
}




void SAL_CALL DrawController::addSelectionChangeListener(
    const Reference< view::XSelectionChangeListener >& xListener)
    throw(RuntimeException)
{
    addListener(
        ::getCppuType((Reference<view::XSelectionChangeListener>*)0),
        xListener);
}




void SAL_CALL DrawController::removeSelectionChangeListener(
    const Reference< view::XSelectionChangeListener >& xListener )
    throw(RuntimeException)
{
    removeListener(
        ::getCppuType((Reference<view::XSelectionChangeListener>*)0),
        xListener);
}




//----------------------------------------------------------------------
//------ The Properties of this implementation -------------------------
//----------------------------------------------------------------------


/**
 * All Properties of this implementation. Must be sorted by name.
 */
static beans::Property * getBasicProps()
{
    static beans::Property *pTable = 0;

    if( ! pTable )
    {
        ::osl::MutexGuard guard( ::osl::Mutex::getGlobalMutex() );
        if( ! pTable )
        {

            static beans::Property aBasicProps[PROPERTY_COUNT] =
            {
                beans::Property(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("VisibleArea") ),
                    PROPERTY_WORKAREA,
                    ::getCppuType((const ::com::sun::star::awt::Rectangle*)0),
                    beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY),
                beans::Property(
                    OUString( RTL_CONSTASCII_USTRINGPARAM(
                        "PrimarySubController") ),
                    PROPERTY_PRIMARY_SUB_CONTROLLER,
                    ::getCppuType((const Reference<XInterface>*)0),
                    beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY)
            };
            pTable = aBasicProps;
        }
    }
    return pTable;
}




// XPropertySet & OPropertySetHelper


/**
 * Create a table that map names to index values.
 */
IPropertyArrayHelper & DrawController::getInfoHelper()
{
    OGuard aGuard( Application::GetSolarMutex() );

    static OPropertyArrayHelper aInfo(
        getBasicProps(), PROPERTY_COUNT, sal_False);
    return aInfo;
}




Reference < beans::XPropertySetInfo >  DrawController::getPropertySetInfo()
        throw ( ::com::sun::star::uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    static Reference < beans::XPropertySetInfo >  xInfo(
        createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}



sal_Bool DrawController::convertFastPropertyValue (
    Any & rConvertedValue,
    Any & rOldValue,
    sal_Int32 nHandle,
    const Any& rValue)
    throw (lang::IllegalArgumentException)
{
    return sal_False;
}



/**
 * only set the value.
 */
void DrawController::setFastPropertyValue_NoBroadcast (
    sal_Int32 nHandle,
    const Any& rValue)
    throw (com::sun::star::uno::Exception)
{
}




void DrawController::getFastPropertyValue (Any & rRet, sal_Int32 nHandle) const
{
    OGuard aGuard( Application::GetSolarMutex() );

    switch (nHandle)
    {
        case PROPERTY_WORKAREA:
        {
            // We have to use a const_cast here because a) this method
            // has to be const in order to be recognized by the
            // property helper and b) because GetSubController() is
            // not const because the sub-controller may be repaced by
            // another one during run time.
            DrawSubController* pSubController =
                const_cast<DrawController*>(this)->GetSubController ();
            if (pSubController != NULL)
                rRet <<= pSubController->GetVisArea();
        }
        break;

        case PROPERTY_PRIMARY_SUB_CONTROLLER:
        {
            ViewShell* pShell = mrBase.GetSubShellManager().GetMainSubShell();
            if (pShell != NULL)
                rRet <<= Reference<XInterface>(
                    static_cast<XWeak*>(pShell->GetSubController()));
        }
        break;
    }
}




void DrawController::FireVisAreaChanged (
    const Any& rNewVisArea,
    const Any& rOldVisArea) throw()
{
    sal_Int32 nHandles = PROPERTY_WORKAREA;
    try
    {
        fire (&nHandles, &rNewVisArea, &rOldVisArea, 1, sal_False);
    }
    catch (RuntimeException aException)
    {
    }
}




// XWindow

Reference<awt::XWindow> DrawController::getWindow (void)
{
    if (GetSubController() != NULL)
        return GetSubController()->getWindow();
    else
        return Reference<awt::XWindow>();
}

void SAL_CALL DrawController::setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< ::com::sun::star::awt::XWindow > xWindow( getWindow() );
    if( xWindow.is() )
        xWindow->setPosSize( X, Y, Width, Height, Flags );
}

::com::sun::star::awt::Rectangle SAL_CALL DrawController::getPosSize(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::awt::Rectangle aRect;

    Reference< ::com::sun::star::awt::XWindow > xWindow( getWindow() );
    if( xWindow.is() )
        aRect = xWindow->getPosSize();

    return aRect;
}

void SAL_CALL DrawController::setVisible( sal_Bool Visible ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< ::com::sun::star::awt::XWindow > xWindow( getWindow() );
    if( xWindow.is() )
        xWindow->setVisible( Visible );
}

void SAL_CALL DrawController::setEnable( sal_Bool Enable ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< ::com::sun::star::awt::XWindow > xWindow( getWindow() );
    if( xWindow.is() )
        xWindow->setEnable( Enable );
}

void SAL_CALL DrawController::setFocus(  ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< ::com::sun::star::awt::XWindow > xWindow( getWindow() );
    if( xWindow.is() )
        xWindow->setFocus();
}

void SAL_CALL DrawController::addWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< ::com::sun::star::awt::XWindow > xWindow( getWindow() );
    if( xWindow.is() )
        xWindow->addWindowListener( xListener );
}

void SAL_CALL DrawController::removeWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< ::com::sun::star::awt::XWindow > xWindow( getWindow() );
    if( xWindow.is() )
        xWindow->removeWindowListener( xListener );
}

void SAL_CALL DrawController::addFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< ::com::sun::star::awt::XWindow > xWindow( getWindow() );
    if( xWindow.is() )
        xWindow->addFocusListener( xListener );
}

void SAL_CALL DrawController::removeFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< ::com::sun::star::awt::XWindow > xWindow( getWindow() );
    if( xWindow.is() )
        xWindow->removeFocusListener( xListener );
}

void SAL_CALL DrawController::addKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< ::com::sun::star::awt::XWindow > xWindow( getWindow() );
    if( xWindow.is() )
        xWindow->addKeyListener( xListener );
}

void SAL_CALL DrawController::removeKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< ::com::sun::star::awt::XWindow > xWindow( getWindow() );
    if( xWindow.is() )
        xWindow->removeKeyListener( xListener );
}

void SAL_CALL DrawController::addMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< ::com::sun::star::awt::XWindow > xWindow( getWindow() );
    if( xWindow.is() )
        xWindow->addMouseListener( xListener );
}

void SAL_CALL DrawController::removeMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< ::com::sun::star::awt::XWindow > xWindow( getWindow() );
    if( xWindow.is() )
        xWindow->removeMouseListener( xListener );
}

void SAL_CALL DrawController::addMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< ::com::sun::star::awt::XWindow > xWindow( getWindow() );
    if( xWindow.is() )
        xWindow->addMouseMotionListener( xListener );
}

void SAL_CALL DrawController::removeMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< ::com::sun::star::awt::XWindow > xWindow( getWindow() );
    if( xWindow.is() )
        xWindow->removeMouseMotionListener( xListener );
}

void SAL_CALL DrawController::addPaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< ::com::sun::star::awt::XWindow > xWindow( getWindow() );
    if( xWindow.is() )
        xWindow->addPaintListener( xListener );
}

void SAL_CALL DrawController::removePaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< ::com::sun::star::awt::XWindow > xWindow( getWindow() );
    if( xWindow.is() )
        xWindow->removePaintListener( xListener );
}




//=====  lang::XEventListener  ================================================

void SAL_CALL
    DrawController::disposing (const lang::EventObject& rEventObject)
    throw (uno::RuntimeException)
{
    // Use mpSubController directly instead of GetSubController() to
    // avoid instancing a new sub controller.
    if (rEventObject.Source.get() == static_cast<XWeak*>(mpSubController))
        // Release the pointer to the sub-controller so that
        // GetSubController will get a new one when called again.
        mpSubController = NULL;
}




//=====  view::XSelectionChangeListener  ======================================

void  SAL_CALL
    DrawController::selectionChanged (const lang::EventObject& rEvent)
        throw (uno::RuntimeException)
{
    // Have to forward the event to our selection change listeners.
    OInterfaceContainerHelper* pListeners = getContainer(
        ::getCppuType((Reference<view::XSelectionChangeListener>*)0));
    if (pListeners)
    {
        // Re-send the event to all of our listeners.
        OInterfaceIteratorHelper aIterator (*pListeners);
        while (aIterator.hasMoreElements())
        {
            try
            {
                view::XSelectionChangeListener* pListener =
                    static_cast<view::XSelectionChangeListener*>(
                        aIterator.next());
                if (pListener != NULL)
                    pListener->selectionChanged (rEvent);
            }
            catch (RuntimeException aException)
            {
            }
        }
    }
}



} // end of namespace sd

