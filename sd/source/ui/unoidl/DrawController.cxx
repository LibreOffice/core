/*************************************************************************
 *
 *  $RCSfile: DrawController.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:48:21 $
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
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPSE, OR NON-INFRINGING.
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

#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#include "View.hxx"
#include "Window.hxx"
#include "DrawDocShell.hxx"
#include "unomodel.hxx"

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
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
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
#include <svx/unoshcol.hxx>
#include <svx/unopage.hxx>

using namespace ::std;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::vos;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace {
static const ::com::sun::star::uno::Type saComponentTypeIdentifier (
    ::getCppuType( (Reference<lang::XEventListener > *)0 ));
static const ::com::sun::star::uno::Type saSelectionTypeIdentifier (
    ::getCppuType( (Reference<view::XSelectionChangeListener > *)0 ));

} // end of anonymous namespace

namespace sd {


DrawController::DrawController (
    ViewShellBase& rBase,
    ViewShell& rViewShell,
    View& rView) throw()
    : SfxBaseController (&rBase),
      OBroadcastHelper(SfxBaseController::m_aMutex),
      OPropertySetHelper( *static_cast<OBroadcastHelperVar<
          OMultiTypeInterfaceContainerHelper,
          OMultiTypeInterfaceContainerHelper::keyType> *>(this)),
      mrView(rView),
      mrViewShell(rViewShell),
      maLastVisArea(),
      mrBase(rBase),
      mbDisposing(false)
{
}




DrawController::~DrawController (void) throw()
{
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
    ThrowIfDisposed();
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
    ThrowIfDisposed();
    SfxBaseController::addEventListener( xListener );
}




void SAL_CALL DrawController::removeEventListener (
    const Reference<lang::XEventListener >& aListener)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    SfxBaseController::removeEventListener( aListener );
}




// XServiceInfo

OUString SAL_CALL DrawController::getImplementationName(  ) throw(RuntimeException)
{
    ThrowIfDisposed();
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "DrawController" ) );
}



static OUString ssServiceName (OUString::createFromAscii(
    "com.sun.star.drawing.DrawController"));

sal_Bool SAL_CALL DrawController::supportsService (
    const OUString& rsServiceName)
    throw(RuntimeException)
{
    ThrowIfDisposed();
    return rsServiceName.equals(ssServiceName);
}




Sequence<OUString> SAL_CALL DrawController::getSupportedServiceNames (void)
    throw(RuntimeException)
{
    ThrowIfDisposed();
    Sequence<OUString> aSupportedServices (1);
    OUString* pServices = aSupportedServices.getArray();
    pServices[0] = ssServiceName;
    return aSupportedServices;
}




//------ XSelectionSupplier --------------------------------------------

sal_Bool SAL_CALL DrawController::select (const Any& aSelection)
    throw(lang::IllegalArgumentException, RuntimeException)
{
    return false;
}




Any SAL_CALL DrawController::getSelection()
    throw(RuntimeException)
{
    ThrowIfDisposed();
    OGuard aGuard( Application::GetSolarMutex() );

    SdXImpressDocument* pModel = GetModel();

    Reference< drawing::XShapes > xShapes( SvxShapeCollection_NewInstance(), UNO_QUERY );

    DBG_ASSERT (&mrView != NULL,
        "view is NULL in SdUnoDrawView::getSelection()");

    const SdrMarkList& rMarkList = mrView.GetMarkList();
    sal_uInt32 nCount = rMarkList.GetMarkCount();
    for( sal_uInt32 nNum = 0; nNum < nCount; nNum++)
    {
        SdrMark *pMark = rMarkList.GetMark(nNum);
        if(pMark==NULL)
            continue;

        SdrObject *pObj = pMark->GetObj();
        if(pObj==NULL || pObj->GetPage() == NULL)
            continue;

        Reference< drawing::XDrawPage > xPage( pObj->GetPage()->getUnoPage(), UNO_QUERY);

        if(!xPage.is())
            continue;

        SvxDrawPage* pDrawPage = SvxDrawPage::getImplementation( xPage );

        if(pDrawPage==NULL)
            continue;

        Reference< drawing::XShape > xShape( pObj->getUnoShape(), UNO_QUERY );

        if(xShape.is())
            xShapes->add(xShape);
    }

    Any aAny;
    if( 0 != xShapes->getCount() )
        aAny <<= xShapes;

    return aAny;
}




void SAL_CALL DrawController::addSelectionChangeListener(
    const Reference< view::XSelectionChangeListener >& xListener)
    throw(RuntimeException)
{
    if( mbDisposing )
        throw lang::DisposedException();

    addListener (saSelectionTypeIdentifier, xListener);
}




void SAL_CALL DrawController::removeSelectionChangeListener(
    const Reference< view::XSelectionChangeListener >& xListener )
    throw(RuntimeException)
{
    if( mbDisposing )
        throw lang::DisposedException();

    removeListener (saSelectionTypeIdentifier, xListener);
}



// XWindow

Reference<awt::XWindow> DrawController::GetWindow (void)
{
    Reference< ::com::sun::star::awt::XWindow > xWindow;
    Window* pWindow = mrViewShell.GetActiveWindow();
    if (pWindow != NULL)
        xWindow = VCLUnoHelper::GetInterface (pWindow);
    return xWindow;
}

void SAL_CALL DrawController::setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags ) throw (::com::sun::star::uno::RuntimeException)
{
    if( mbDisposing )
        throw lang::DisposedException();

    Reference< ::com::sun::star::awt::XWindow > xWindow( GetWindow() );
    if( xWindow.is() )
        xWindow->setPosSize( X, Y, Width, Height, Flags );
}

::com::sun::star::awt::Rectangle SAL_CALL DrawController::getPosSize(  ) throw (::com::sun::star::uno::RuntimeException)
{
    if( mbDisposing )
        throw lang::DisposedException();

    ::com::sun::star::awt::Rectangle aRect;

    Reference< ::com::sun::star::awt::XWindow > xWindow( GetWindow() );
    if( xWindow.is() )
        aRect = xWindow->getPosSize();

    return aRect;
}

void SAL_CALL DrawController::setVisible( sal_Bool Visible ) throw (::com::sun::star::uno::RuntimeException)
{
    if( mbDisposing )
        throw lang::DisposedException();

    Reference< ::com::sun::star::awt::XWindow > xWindow( GetWindow() );
    if( xWindow.is() )
        xWindow->setVisible( Visible );
}

void SAL_CALL DrawController::setEnable( sal_Bool Enable ) throw (::com::sun::star::uno::RuntimeException)
{
    if( mbDisposing )
        throw lang::DisposedException();

    Reference< ::com::sun::star::awt::XWindow > xWindow( GetWindow() );
    if( xWindow.is() )
        xWindow->setEnable( Enable );
}

void SAL_CALL DrawController::setFocus(  ) throw (::com::sun::star::uno::RuntimeException)
{
    if( mbDisposing )
        throw lang::DisposedException();

    Reference< ::com::sun::star::awt::XWindow > xWindow( GetWindow() );
    if( xWindow.is() )
        xWindow->setFocus();
}

void SAL_CALL DrawController::addWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    if( mbDisposing )
        throw lang::DisposedException();

    Reference< ::com::sun::star::awt::XWindow > xWindow( GetWindow() );
    if( xWindow.is() )
        xWindow->addWindowListener( xListener );
}

void SAL_CALL DrawController::removeWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    if( mbDisposing )
        throw lang::DisposedException();

    Reference< ::com::sun::star::awt::XWindow > xWindow( GetWindow() );
    if( xWindow.is() )
        xWindow->removeWindowListener( xListener );
}

void SAL_CALL DrawController::addFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    if( mbDisposing )
        throw lang::DisposedException();

    Reference< ::com::sun::star::awt::XWindow > xWindow( GetWindow() );
    if( xWindow.is() )
        xWindow->addFocusListener( xListener );
}

void SAL_CALL DrawController::removeFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    if( mbDisposing )
        throw lang::DisposedException();

    Reference< ::com::sun::star::awt::XWindow > xWindow( GetWindow() );
    if( xWindow.is() )
        xWindow->removeFocusListener( xListener );
}

void SAL_CALL DrawController::addKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    if( mbDisposing )
        throw lang::DisposedException();

    Reference< ::com::sun::star::awt::XWindow > xWindow( GetWindow() );
    if( xWindow.is() )
        xWindow->addKeyListener( xListener );
}

void SAL_CALL DrawController::removeKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    if( mbDisposing )
        throw lang::DisposedException();

    Reference< ::com::sun::star::awt::XWindow > xWindow( GetWindow() );
    if( xWindow.is() )
        xWindow->removeKeyListener( xListener );
}

void SAL_CALL DrawController::addMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    if( mbDisposing )
        throw lang::DisposedException();

    Reference< ::com::sun::star::awt::XWindow > xWindow( GetWindow() );
    if( xWindow.is() )
        xWindow->addMouseListener( xListener );
}

void SAL_CALL DrawController::removeMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    if( mbDisposing )
        throw lang::DisposedException();

    Reference< ::com::sun::star::awt::XWindow > xWindow( GetWindow() );
    if( xWindow.is() )
        xWindow->removeMouseListener( xListener );
}

void SAL_CALL DrawController::addMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    if( mbDisposing )
        throw lang::DisposedException();

    Reference< ::com::sun::star::awt::XWindow > xWindow( GetWindow() );
    if( xWindow.is() )
        xWindow->addMouseMotionListener( xListener );
}

void SAL_CALL DrawController::removeMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    if( mbDisposing )
        throw lang::DisposedException();

    Reference< ::com::sun::star::awt::XWindow > xWindow( GetWindow() );
    if( xWindow.is() )
        xWindow->removeMouseMotionListener( xListener );
}

void SAL_CALL DrawController::addPaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    if( mbDisposing )
        throw lang::DisposedException();

    Reference< ::com::sun::star::awt::XWindow > xWindow( GetWindow() );
    if( xWindow.is() )
        xWindow->addPaintListener( xListener );
}

void SAL_CALL DrawController::removePaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    if( mbDisposing )
        throw lang::DisposedException();

    Reference< ::com::sun::star::awt::XWindow > xWindow( GetWindow() );
    if( xWindow.is() )
        xWindow->removePaintListener( xListener );
}




//=====  lang::XEventListener  ================================================

void SAL_CALL
    DrawController::disposing (const lang::EventObject& rEventObject)
    throw (uno::RuntimeException)
{
}




//=====  view::XSelectionChangeListener  ======================================

void  SAL_CALL
    DrawController::selectionChanged (const lang::EventObject& rEvent)
        throw (uno::RuntimeException)
{
    ThrowIfDisposed();
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




SdXImpressDocument* DrawController::GetModel (void) const throw()
{
    if (mrView.GetDocSh())
    {
        Reference< frame::XModel > xModel (mrView.GetDocSh()->GetModel());
        return SdXImpressDocument::getImplementation(xModel);
    }
    else
        return NULL;
}



::awt::Rectangle DrawController::GetVisArea (void) const
{
    return awt::Rectangle(
        maLastVisArea.Left(),
        maLastVisArea.Top(),
        maLastVisArea.GetWidth(),
        maLastVisArea.GetHeight());
}




// XDrawView

void SAL_CALL DrawController::setCurrentPage( const Reference< drawing::XDrawPage >& xPage )
    throw(RuntimeException)
{
    ThrowIfDisposed();
}




Reference< drawing::XDrawPage > SAL_CALL DrawController::getCurrentPage()
    throw(RuntimeException)
{
    ThrowIfDisposed();
    OGuard aGuard( Application::GetSolarMutex() );

    Reference< drawing::XDrawPage >  xPage;

    return xPage;
}




void DrawController::FireVisAreaChanged (const Rectangle& rVisArea) throw()
{
    if( maLastVisArea != rVisArea )
    {
        Any aNewValue;
        aNewValue <<= awt::Rectangle(
            rVisArea.Left(),
            rVisArea.Top(),
            rVisArea.GetWidth(),
            rVisArea.GetHeight() );

        Any aOldValue;
        aOldValue <<= awt::Rectangle(
            maLastVisArea.Left(),
            maLastVisArea.Top(),
            maLastVisArea.GetWidth(),
            maLastVisArea.GetHeight() );

        FirePropertyChange (PROPERTY_WORKAREA, aNewValue, aOldValue);

        maLastVisArea = rVisArea;
    }
}




void DrawController::FireSelectionChangeListener() throw()
{
    OInterfaceContainerHelper * pLC = getContainer(
        saSelectionTypeIdentifier);
    if( pLC )
    {
        Reference< XInterface > xSource( (XWeak*)this );
        const lang::EventObject aEvent( xSource );

        // Ueber alle Listener iterieren und Events senden
        OInterfaceIteratorHelper aIt( *pLC);
        while( aIt.hasMoreElements() )
        {
            try
            {
                view::XSelectionChangeListener * pL =
                    static_cast<view::XSelectionChangeListener*>(aIt.next());
                if (pL != NULL)
                    pL->selectionChanged( aEvent );
            }
            catch (RuntimeException aException)
            {
            }
        }
    }
}




void DrawController::FirePropertyChange (
    sal_Int32 nHandle,
    const Any& rNewValue,
    const Any& rOldValue)
{
    try
    {
        fire (&nHandle, &rNewValue, &rOldValue, 1, sal_False);
    }
    catch (RuntimeException aException)
    {
        // Ignore this exception.  Exceptions should be handled in the
        // fire() function so that all listeners are called.  This is
        // not the case at the moment, so we simply ignore the
        // exception.
    }

}




//===== Properties ============================================================

Sequence<beans::Property>&  DrawController::GetPropertyTable (void)
{
    if (mpProperties.get() == NULL)
    {
        ::osl::MutexGuard guard( ::osl::Mutex::getGlobalMutex() );
        if (mpProperties.get() == NULL)
        {
            ::std::vector<beans::Property> aProperties;
            FillPropertyTable (aProperties);
            mpProperties = ::std::auto_ptr<Sequence<beans::Property> > (
                new Sequence<beans::Property> (aProperties.size()));
            for (unsigned int i=0; i<aProperties.size(); i++)
                (*mpProperties)[i] = aProperties[i];
        }
    }
    return *mpProperties.get();
}




void DrawController::FillPropertyTable (
    ::std::vector<beans::Property>& rProperties)
{
    rProperties.push_back (
        beans::Property(
            OUString( RTL_CONSTASCII_USTRINGPARAM("VisibleArea") ),
            PROPERTY_WORKAREA,
            ::getCppuType((const ::com::sun::star::awt::Rectangle*)0),
            beans::PropertyAttribute::BOUND
            | beans::PropertyAttribute::READONLY));
}




IPropertyArrayHelper & DrawController::getInfoHelper()
{
    OGuard aGuard( Application::GetSolarMutex() );

    static OPropertyArrayHelper aInfo(GetPropertyTable(), sal_False);
    return aInfo;
}




Reference < beans::XPropertySetInfo >  DrawController::getPropertySetInfo()
        throw ( ::com::sun::star::uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    static Reference < beans::XPropertySetInfo >  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}



sal_Bool DrawController::convertFastPropertyValue (
    Any & rConvertedValue,
    Any & rOldValue,
    sal_Int32 nHandle,
    const Any& rValue)
    throw ( com::sun::star::lang::IllegalArgumentException)
{
    return sal_False;
}




void DrawController::setFastPropertyValue_NoBroadcast (
    sal_Int32 nHandle,
    const Any& rValue)
    throw ( com::sun::star::uno::Exception)
{
}




void DrawController::getFastPropertyValue (
    Any & rRet,
    sal_Int32 nHandle ) const
{
    OGuard aGuard( Application::GetSolarMutex() );

    switch( nHandle )
    {
        case PROPERTY_WORKAREA:
            rRet <<= awt::Rectangle(
                maLastVisArea.Left(),
                maLastVisArea.Top(),
                maLastVisArea.GetWidth(),
                maLastVisArea.GetHeight());
            break;
    }
}




void DrawController::ThrowIfDisposed (void) const
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        OSL_TRACE ("Calling disposed DrawController object. Throwing exception:");
        throw lang::DisposedException (
            OUString(RTL_CONSTASCII_USTRINGPARAM(
                "DrawController object has already been disposed")),
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}





} // end of namespace sd

