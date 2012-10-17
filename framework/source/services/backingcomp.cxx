/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include "services/backingcomp.hxx"

#include "backingwindow.hxx"

#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>
#include <classes/droptargetlistener.hxx>
#include <targets.h>
#include <properties.h>
#include <services.h>

#include <helpid.hrc>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/Toolkit.hpp>
#include <com/sun/star/awt/XDataTransferProviderAccess.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#include <com/sun/star/awt/KeyEvent.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/util/URLTransformer.hpp>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/factory.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/keycod.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/svapp.hxx>
#include <rtl/ustrbuf.hxx>

#include <svl/solar.hrc>
#include <svl/urihelper.hxx>
#include <osl/file.hxx>
#include <unotools/configmgr.hxx>

#include <unotools/bootstrap.hxx>

namespace framework
{

//_______________________________________________

//_______________________________________________

BackingComp::BackingComp( const css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR )
    : ThreadHelpBase    (&Application::GetSolarMutex()                  )
    , m_xSMGR           (xSMGR                                          )
{
}

//_______________________________________________

BackingComp::~BackingComp()
{
}

//_______________________________________________

/** return information about supported interfaces.

    Some interfaces are supported by his class directly, but some other ones are
    used by aggregation. An instance of this class must provide some window interfaces.
    But it must represent a VCL window behind such interfaces too! So we use an internal
    saved window member to ask it for it's interfaces and return it. But we must be aware then,
    that it can be destroyed from outside too ...

    @param  aType
                describe the required interface type

    @return An Any holding the instance, which provides the queried interface.
            Note: There exist two possible results ... this instance itself and her window member!
 */

css::uno::Any SAL_CALL BackingComp::queryInterface( /*IN*/ const css::uno::Type& aType )
    throw(css::uno::RuntimeException)
{
    css::uno::Any aResult;

    // first look for own supported interfaces
    aResult = ::cppu::queryInterface(
                aType,
                static_cast< css::lang::XTypeProvider* >(this),
                static_cast< css::lang::XServiceInfo* >(this),
                static_cast< css::lang::XInitialization* >(this),
                static_cast< css::frame::XController* >(this),
                static_cast< css::lang::XComponent* >(this),
                static_cast< css::lang::XEventListener* >(this),
                static_cast< css::awt::XKeyListener* >(static_cast< css::lang::XEventListener* >(this)));

    // then look for supported window interfaces
    // Note: They exist only, if this instance was initialized
    // with a valid window reference. It's aggregation on demand ...
    if (!aResult.hasValue())
    {
        /* SAFE { */
        ReadGuard aReadLock(m_aLock);
        if (m_xWindow.is())
            aResult = m_xWindow->queryInterface(aType);
        aReadLock.unlock();
        /* } SAFE */
    }

    // look for XWeak and XInterface
    if (!aResult.hasValue())
        aResult = OWeakObject::queryInterface(aType);

    return aResult;
}

//_______________________________________________

/** increase ref count of this instance.
 */

void SAL_CALL BackingComp::acquire()
    throw()
{
    OWeakObject::acquire();
}

//_______________________________________________

/** decrease ref count of this instance.
 */

void SAL_CALL BackingComp::release()
    throw()
{
    OWeakObject::release();
}

//_______________________________________________

/** return collection about all supported interfaces.

    Optimize this method !
    We initialize a static variable only one time.
    And we don't must use a mutex at every call!
    For the first call; pTypeCollection is NULL -
    for the second call pTypeCollection is different from NULL!

    @return A list of all supported interface types.
*/

css::uno::Sequence< css::uno::Type > SAL_CALL BackingComp::getTypes()
    throw(css::uno::RuntimeException)
{
    static ::cppu::OTypeCollection* pTypeCollection = NULL;
    if (!pTypeCollection)
    {
        /* GLOBAL SAFE { */
        ::osl::MutexGuard aGlobalLock(::osl::Mutex::getGlobalMutex());
        // Control these pointer again ... it can be, that another instance will be faster then this one!
        if (!pTypeCollection)
        {
            /* LOCAL SAFE { */
            ReadGuard aReadLock(m_aLock);
            css::uno::Reference< css::lang::XTypeProvider > xProvider(m_xWindow, css::uno::UNO_QUERY);
            aReadLock.unlock();
            /* } LOCAL SAFE */

            css::uno::Sequence< css::uno::Type > lWindowTypes;
            if (xProvider.is())
                lWindowTypes = xProvider->getTypes();

            static ::cppu::OTypeCollection aTypeCollection(
                    ::getCppuType((const ::com::sun::star::uno::Reference< css::lang::XInitialization >*)NULL ),
                    ::getCppuType((const ::com::sun::star::uno::Reference< css::lang::XTypeProvider >*)NULL ),
                    ::getCppuType((const ::com::sun::star::uno::Reference< css::lang::XServiceInfo >*)NULL ),
                    ::getCppuType((const ::com::sun::star::uno::Reference< css::frame::XController >*)NULL ),
                    ::getCppuType((const ::com::sun::star::uno::Reference< css::lang::XComponent >*)NULL ),
                    lWindowTypes);

            pTypeCollection = &aTypeCollection;
        }
        /* } GLOBAL SAFE */
    }
    return pTypeCollection->getTypes();
}

//_______________________________________________

/** create one unique Id for all instances of this class.

    Optimize this method
    We initialize a static variable only one time. And we don't must use a mutex at every call!
    For the first call; pID is NULL - for the second call pID is different from NULL!

    @return A byte array, which represent the unique id.
*/

css::uno::Sequence< sal_Int8 > SAL_CALL BackingComp::getImplementationId()
    throw(css::uno::RuntimeException)
{
    static ::cppu::OImplementationId* pID = NULL;
    if (!pID)
    {
        /* GLOBAL SAFE { */
        ::osl::MutexGuard aLock(::osl::Mutex::getGlobalMutex());
        // Control these pointer again ... it can be, that another instance will be faster then this one!
        if (!pID)
        {
            static ::cppu::OImplementationId aID(sal_False);
            pID = &aID;
        }
        /* } GLOBAL SAFE */
    }
    return pID->getImplementationId();
}

//_______________________________________________

/** returns a static implementation name for this UNO service.

    Because this value is needed at different places and our class is used
    by some generic macros too, we have to use a static impl method for that!

    @see impl_getStaticImplementationName()
    @see IMPLEMENTATIONNAME

    @return The implementation name of this class.
*/

::rtl::OUString SAL_CALL BackingComp::getImplementationName()
    throw(css::uno::RuntimeException)
{
    return impl_getStaticImplementationName();
}

//_______________________________________________

/** returns information about supported services.

    Because this value is needed at different places and our class is used
    by some generic macros too, we have to use a static impl method for that!

    @see impl_getStaticSupportedServiceNames()
    @see SERVICENAME

    @return <TRUE/> if the queried service is supported;
            <br><FALSE/> otherwise.
*/

sal_Bool SAL_CALL BackingComp::supportsService( /*IN*/ const ::rtl::OUString& sServiceName )
    throw(css::uno::RuntimeException)
{
    return (
            sServiceName.equals(SERVICENAME_STARTMODULE    ) ||
            sServiceName.equals(SERVICENAME_FRAMECONTROLLER)
           );
}

//_______________________________________________

/** returns collection of supported services.

    Because this value is needed at different places and our class is used
    by some generic macros too, we have to use a static impl method for that!

    @see impl_getStaticSupportedServiceNames()
    @see SERVICENAME

    @return A list of all supported uno service names.
*/

css::uno::Sequence< ::rtl::OUString > SAL_CALL BackingComp::getSupportedServiceNames()
    throw(css::uno::RuntimeException)
{
    return impl_getStaticSupportedServiceNames();
}

//_______________________________________________

/** returns static implementation name.

    Because this value is needed at different places and our class is used
    by some generic macros too, we have to use a static impl method for that!

    @see impl_getStaticSupportedServiceNames()
    @see SERVICENAME

    @return The implementation name of this class.
*/

::rtl::OUString BackingComp::impl_getStaticImplementationName()
{
    return IMPLEMENTATIONNAME_STARTMODULE;
}

//_______________________________________________

/** returns static list of supported service names.

    Because this value is needed at different places and our class is used
    by some generic macros too, we have to use a static impl method for that!

    @see impl_getStaticSupportedServiceNames()
    @see SERVICENAME

    @return A list of all supported uno service names.
*/

css::uno::Sequence< ::rtl::OUString > BackingComp::impl_getStaticSupportedServiceNames()
{
    css::uno::Sequence< ::rtl::OUString > lNames(1);
    lNames[0] = SERVICENAME_STARTMODULE;
    return lNames;
}

//_______________________________________________

/** returns a new instance of this class.

    This factory method is registered inside the UNO runtime
    and will be called for every createInstance() request from outside,
    which wish to use this service.

    @param  xSMGR
                reference to the uno service manager, which call us
                We use it too, to set it at the new created instance.

    @return A new instance as uno reference.
*/

css::uno::Reference< css::uno::XInterface > SAL_CALL BackingComp::impl_createInstance( /*IN*/ const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR )
    throw(css::uno::Exception)
{
    BackingComp* pObject = new BackingComp(xSMGR);
    return css::uno::Reference< css::uno::XInterface >(static_cast< ::cppu::OWeakObject* >(pObject), css::uno::UNO_QUERY);
}

//_______________________________________________

/** returns a new factory instance for instances of this class.

    It uses a helper class of the cppuhelper project as factory.
    It will be initialized with all neccessary informations and
    will be able afterwards to create instance of this class.
    This factory call us back inside our method impl_createInstance().
    So we can create and initialize ourself. Only filtering of creation
    requests will be done by this factory.

    @param  xSMGR
                reference to the uno service manager, which call us

    @return A new instance of our factory.
*/

css::uno::Reference< css::lang::XSingleServiceFactory > BackingComp::impl_createFactory( /*IN*/ const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR )
{
    css::uno::Reference< css::lang::XSingleServiceFactory > xReturn(
        cppu::createSingleFactory(
            xSMGR,
            BackingComp::impl_getStaticImplementationName(),
            BackingComp::impl_createInstance,
            BackingComp::impl_getStaticSupportedServiceNames()));
    return xReturn;
}

//_______________________________________________

/**
    attach this component to a target frame.

    We has to use the container window of this frame as parent window of our own component window.
    But it's not allowed to work with it realy. May another component used it too.
    Currently we need it only to create our child component window and support it's
    interfaces inside our queryInterface() method. The user of us must have e.g. the
    XWindow interface of it to be able to call setComponent(xWindow,xController) at the
    frame!

    May he will do the following things:

    <listing>
        XController xBackingComp = (XController)UnoRuntime.queryInterface(
            XController.class,
            xSMGR.createInstance(SERVICENAME_STARTMODULE));

        // at this time XWindow isn't present at this instance!
        XWindow xBackingComp = (XWindow)UnoRuntime.queryInterface(
            XWindow.class,
            xBackingComp);

        // attach controller to the frame
        // We will use it's container window, to create
        // the component window. From now we offer the window interfaces!
        xBackingComp.attachFrame(xFrame);

        XWindow xBackingComp = (XWindow)UnoRuntime.queryInterface(
            XWindow.class,
            xBackingComp);

        // Our user can set us at the frame as new component
        xFrame.setComponent(xBackingWin, xBackingComp);

        // But that had no effect to our view state.
        // We must be started to create our UI elements like e.g. menu, title, background ...
        XInitialization xBackingInit = (XInitialization)UnoRuntime.queryInterface(
            XInitialization.class,
            xBackingComp);

        xBackingInit.initialize(lArgs);
    </listing>

    @param  xFrame
                reference to our new target frame

    @throw  com::sun::star::uno::RuntimeException
                if the given frame reference is wrong or component window couldn't be created
                successfully.
                We throw it too, if we already attached to a frame. Because we don't support
                reparenting of our component window on demand!
*/

void SAL_CALL BackingComp::attachFrame( /*IN*/ const css::uno::Reference< css::frame::XFrame >& xFrame )
    throw (css::uno::RuntimeException)
{
    /* SAFE */
    WriteGuard aWriteLock(m_aLock);

    // check some required states
    if (m_xFrame.is())
        throw css::uno::RuntimeException(
                ::rtl::OUString("already attached"),
                static_cast< ::cppu::OWeakObject* >(this));

    if (!xFrame.is())
        throw css::uno::RuntimeException(
                ::rtl::OUString("invalid frame reference"),
                static_cast< ::cppu::OWeakObject* >(this));

    if (!m_xWindow.is())
        return; // disposed

    // safe the frame reference
    m_xFrame = xFrame;

    // establish drag&drop mode
    ::framework::DropTargetListener* pDropListener = new ::framework::DropTargetListener(m_xSMGR, m_xFrame);
    m_xDropTargetListener = css::uno::Reference< css::datatransfer::dnd::XDropTargetListener >(static_cast< ::cppu::OWeakObject* >(pDropListener), css::uno::UNO_QUERY);

    css::uno::Reference< css::awt::XToolkit2 > xToolkit = css::awt::Toolkit::create( comphelper::getComponentContext(m_xSMGR) );
    css::uno::Reference< css::datatransfer::dnd::XDropTarget > xDropTarget = xToolkit->getDropTarget(m_xWindow);
    if (xDropTarget.is())
    {
        xDropTarget->addDropTargetListener(m_xDropTargetListener);
        xDropTarget->setActive(sal_True);
    }

    // initialize the component and it's parent window
    css::uno::Reference< css::awt::XWindow > xParentWindow = xFrame->getContainerWindow();
    WorkWindow* pParent = (WorkWindow*)VCLUnoHelper::GetWindow(xParentWindow);
    Window*     pWindow = VCLUnoHelper::GetWindow(m_xWindow);

    // disable full screen mode of the frame!
    if (pParent && pParent->IsFullScreenMode())
    {
        pParent->ShowFullScreenMode(sal_False);
        pParent->SetMenuBarMode(MENUBAR_MODE_NORMAL);
    }

    // create the menu bar for the backing component
    css::uno::Reference< css::beans::XPropertySet > xPropSet(m_xFrame, css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::frame::XLayoutManager > xLayoutManager;
    xPropSet->getPropertyValue(FRAME_PROPNAME_LAYOUTMANAGER) >>= xLayoutManager;
    if (xLayoutManager.is())
    {
        xLayoutManager->lock();
        xLayoutManager->createElement( DECLARE_ASCII( "private:resource/menubar/menubar"     ));
        xLayoutManager->unlock();
    }

    if (pWindow)
    {
        // set help ID for our canvas
        pWindow->SetHelpId(HID_BACKINGWINDOW);
    }

    // inform BackingWindow about frame
    BackingWindow* pBack = dynamic_cast<BackingWindow*>(pWindow );
    if( pBack )
        pBack->setOwningFrame( m_xFrame );

    aWriteLock.unlock();
    /* } SAFE */
}

//_______________________________________________

/** not supported.

    This component does not know any model. It will be represented by a window and
    it's controller only.

    return  <FALSE/> everytime.
 */

sal_Bool SAL_CALL BackingComp::attachModel( /*IN*/ const css::uno::Reference< css::frame::XModel >& )
    throw (css::uno::RuntimeException)
{
    return sal_False;
}

//_______________________________________________

/** not supported.

    This component does not know any model. It will be represented by a window and
    it's controller only.

    return  An empty reference every time.
 */

css::uno::Reference< css::frame::XModel > SAL_CALL BackingComp::getModel()
    throw (css::uno::RuntimeException)
{
    return css::uno::Reference< css::frame::XModel >();
}

//_______________________________________________

/** not supported.

    return  An empty value.
 */

css::uno::Any SAL_CALL BackingComp::getViewData()
    throw (css::uno::RuntimeException)
{
    return css::uno::Any();
}

//_______________________________________________

/** not supported.

    @param  aData
                not used.
 */

void SAL_CALL BackingComp::restoreViewData( /*IN*/ const css::uno::Any& )
    throw (css::uno::RuntimeException)
{
}

//_______________________________________________

/** returns the attached frame for this component.

    @see    attachFrame()

    @return The internaly saved frame reference.
            Can be null, if attachFrame() was not called before.
 */

css::uno::Reference< css::frame::XFrame > SAL_CALL BackingComp::getFrame()
    throw (css::uno::RuntimeException)
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    return m_xFrame;
    /* } SAFE */
}

//_______________________________________________

/** ask controller for it's current working state.

    If somehwere whish to close this component, it must suspend the controller before.
    That will be a chance for it to disagree with that AND show any UI for a possible
    UI user.

    @param  bSuspend
                If its set to sal_True this controller should be suspended.
                sal_False will resuspend it.

    @return sal_True if the request could be finished successfully; sal_False otherwise.
 */

sal_Bool SAL_CALL BackingComp::suspend( /*IN*/ sal_Bool )
    throw (css::uno::RuntimeException)
{
    /* FIXME ... implemented by using default :-( */
    return sal_True;
}

//_______________________________________________

/** callback from our window member.

    Our internal saved window wish to die. It will be disposed from outside (may be the frame)
    and inform us. We must release its reference only here. Of course we check the given reference
    here and reject callback from unknown sources.

    Note: deregistration as listener isnt neccessary here. The broadcaster do it automaticly.

    @param  aEvent
                describe the broadcaster of this callback

    @throw ::com::sun::star::uno::RuntimeException
                if the broadcaster doesn't represent the expected window reference.
*/

void SAL_CALL BackingComp::disposing( /*IN*/ const css::lang::EventObject& aEvent )
    throw(css::uno::RuntimeException)
{
    // Attention: dont free m_pAccExec here! see comments inside dtor and
    // keyPressed() for further details.

    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);

    if (!aEvent.Source.is() || aEvent.Source!=m_xWindow || !m_xWindow.is())
        throw css::uno::RuntimeException(
                ::rtl::OUString("unexpected source or called twice"),
                static_cast< ::cppu::OWeakObject* >(this));

    m_xWindow = css::uno::Reference< css::awt::XWindow >();

    aWriteLock.unlock();
    /* } SAFE */
}

//_______________________________________________

/** kill this instance.

    It can be called from our owner frame only. But there is no possibility to check the calli.
    We have to release all our internal used resources and die. From this point we can throw
    DisposedExceptions for every further interface request ... but current implementation doesn`t do so ...

*/

void SAL_CALL BackingComp::dispose()
    throw(css::uno::RuntimeException)
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);

    // kill the menu
    css::util::URL aURL;
    aURL.Complete = DECLARE_ASCII(".uno:close");
    css::uno::Reference< css::util::XURLTransformer > xParser(css::util::URLTransformer::create(::comphelper::getComponentContext(m_xSMGR)));
    if (xParser.is())
        xParser->parseStrict(aURL);

    css::uno::Reference< css::frame::XDispatchProvider > xProvider(m_xFrame, css::uno::UNO_QUERY);
    if (xProvider.is())
    {
        css::uno::Reference< css::frame::XDispatch > xDispatch = xProvider->queryDispatch(aURL, SPECIALTARGET_MENUBAR, 0);
        if (xDispatch.is())
            xDispatch->dispatch(aURL, css::uno::Sequence< css::beans::PropertyValue>());
    }

    // deregister drag&drop helper
    if (m_xDropTargetListener.is())
    {
        css::uno::Reference< css::awt::XToolkit2 > xToolkit = css::awt::Toolkit::create( comphelper::getComponentContext(m_xSMGR) );
        css::uno::Reference< css::datatransfer::dnd::XDropTarget > xDropTarget = xToolkit->getDropTarget(m_xWindow);
        if (xDropTarget.is())
        {
            xDropTarget->removeDropTargetListener(m_xDropTargetListener);
            xDropTarget->setActive(sal_False);
        }
        m_xDropTargetListener = css::uno::Reference< css::datatransfer::dnd::XDropTargetListener >();
    }

    // stop listening at the window
    if (m_xWindow.is())
    {
        css::uno::Reference< css::lang::XComponent > xBroadcaster(m_xWindow, css::uno::UNO_QUERY);
        if (xBroadcaster.is())
        {
            css::uno::Reference< css::lang::XEventListener > xEventThis(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);
            xBroadcaster->removeEventListener(xEventThis);
        }
        css::uno::Reference< css::awt::XKeyListener > xKeyThis(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);
        m_xWindow->removeKeyListener(xKeyThis);
        m_xWindow = css::uno::Reference< css::awt::XWindow >();
    }

    // forget all other used references
    m_xFrame  = css::uno::Reference< css::frame::XFrame >();
    m_xSMGR   = css::uno::Reference< css::lang::XMultiServiceFactory >();

    aWriteLock.unlock();
    /* } SAFE */
}

//_______________________________________________

/** not supported.

    @param  xListener
                not used.

    @throw  ::com::sun::star::uno::RuntimeException
                because the listener expect to be holded alive by this container.
                We must inform it about this unsupported feature.
 */

void SAL_CALL BackingComp::addEventListener( /*IN*/ const css::uno::Reference< css::lang::XEventListener >& )
    throw(css::uno::RuntimeException)
{
    throw css::uno::RuntimeException(
            ::rtl::OUString("not supported"),
            static_cast< ::cppu::OWeakObject* >(this));
}

//_______________________________________________

/** not supported.

    Because registration is not supported too, we must do nothing here. Nobody can call this method realy.

    @param  xListener
                not used.
 */

void SAL_CALL BackingComp::removeEventListener( /*IN*/ const css::uno::Reference< css::lang::XEventListener >& )
    throw(css::uno::RuntimeException)
{
}

//_______________________________________________

/**
    force initialiation for this component.

    Inside attachFrame() we created our component window. But it was not allowed there, to
    initialitze it. E.g. the menu must be set at the container window of the frame, which
    is our parent window. But may at that time another component used it.
    That's why our creator has to inform us, when it's time to initialize us realy.
    Currently only calling of this method must be done. But further implementatoins
    can use special in parameter to configure this initialization ...

    @param  lArgs
                currently not used

    @throw  com::sun::star::uno::RuntimeException
                if some resources are missing
                Means if may be attachedFrame() wasn't called before.
 */

void SAL_CALL BackingComp::initialize( /*IN*/ const css::uno::Sequence< css::uno::Any >& lArgs )
    throw(css::uno::Exception, css::uno::RuntimeException)
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);

    if (m_xWindow.is())
        throw css::uno::Exception(
                ::rtl::OUString("already initialized"),
                static_cast< ::cppu::OWeakObject* >(this));

    css::uno::Reference< css::awt::XWindow > xParentWindow;
    if (
        (lArgs.getLength()!=1         ) ||
        (!(lArgs[0] >>= xParentWindow)) ||
        (!xParentWindow.is()          )
       )
    {
        throw css::uno::Exception(
                ::rtl::OUString("wrong or corrupt argument list"),
                static_cast< ::cppu::OWeakObject* >(this));
    }

    // create the component window
    Window* pParent   = VCLUnoHelper::GetWindow(xParentWindow);
    Window* pWindow   = new BackingWindow(pParent);
            m_xWindow = VCLUnoHelper::GetInterface(pWindow);

    if (!m_xWindow.is())
        throw css::uno::RuntimeException(
                ::rtl::OUString("couldn't create component window"),
                static_cast< ::cppu::OWeakObject* >(this));

    // start listening for window disposing
    // It's set at our owner frame as component window later too. So it will may be disposed there ...
    css::uno::Reference< css::lang::XComponent > xBroadcaster(m_xWindow, css::uno::UNO_QUERY);
    if (xBroadcaster.is())
        xBroadcaster->addEventListener(static_cast< css::lang::XEventListener* >(this));

    m_xWindow->setVisible(sal_True);

    aWriteLock.unlock();
    /* } SAFE */
}

//_______________________________________________

/**
 */

void SAL_CALL BackingComp::keyPressed( /*IN*/ const css::awt::KeyEvent&  )
    throw(css::uno::RuntimeException)
{
}

//_______________________________________________

/**
 */

void SAL_CALL BackingComp::keyReleased( /*IN*/ const css::awt::KeyEvent& )
    throw(css::uno::RuntimeException)
{
    /* Attention
        Please use keyPressed() instead of this method. Otherwhise it would be possible, that
        - a key input may be first switch to the backing mode
        - and this component register itself as key listener too
        - and it's first event will be a keyRealeased() for the already well known event, which switched to the backing mode!
        So it will be handled twice! document => backing mode => exit app ...
     */
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
