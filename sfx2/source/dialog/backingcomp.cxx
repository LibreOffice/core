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

#include "backingwindow.hxx"

#include <helpids.h>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/Toolkit.hpp>
#include <com/sun/star/awt/XDataTransferProviderAccess.hpp>
#include <com/sun/star/awt/KeyEvent.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XKeyListener.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/keycod.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/svapp.hxx>
#include <vcl/syswin.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustrbuf.hxx>

#include <svl/solar.hrc>
#include <svl/urihelper.hxx>
#include <osl/file.hxx>
#include <unotools/configmgr.hxx>

#include <unotools/bootstrap.hxx>

#include <sfx2/notebookbar/SfxNotebookBar.hxx>

namespace {

/**
    implements the backing component.

    This component is a special one, which doesn't provide a controller
    nor a model. It supports the following features:
        - Drag & Drop
        - Key Accelerators
        - Simple Menu
        - Progress Bar
        - Background
 */
class BackingComp : public  css::lang::XTypeProvider
                  , public  css::lang::XServiceInfo
                  , public  css::lang::XInitialization
                  , public  css::frame::XController  // => XComponent
                  , public  css::awt::XKeyListener // => XEventListener
                  , public css::frame::XDispatchProvider
                  , public css::frame::XDispatch
                  , public  ::cppu::OWeakObject
{
private:
    /** reference to the component window. */
    css::uno::Reference< css::awt::XWindow > m_xWindow;

    /** the owner frame of this component. */
    css::uno::Reference< css::frame::XFrame > m_xFrame;

    osl::Mutex m_aTypeProviderMutex;

    Size m_aInitialWindowMinSize;

public:

    explicit BackingComp();

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;
    virtual void          SAL_CALL acquire       (                             ) throw(                          ) override;
    virtual void          SAL_CALL release       (                             ) throw(                          ) override;

    // XTypeProvide
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes           () override;
    virtual css::uno::Sequence< sal_Int8 >       SAL_CALL getImplementationId() override;

    // XServiceInfo
    virtual OUString                       SAL_CALL getImplementationName   (                                     ) override;
    virtual sal_Bool                              SAL_CALL supportsService         ( const OUString& sServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(                                     ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& lArgs ) override;

    // XController
    virtual void SAL_CALL attachFrame( const css::uno::Reference< css::frame::XFrame >& xFrame ) override;
    virtual sal_Bool SAL_CALL attachModel( const css::uno::Reference< css::frame::XModel >& xModel ) override;
    virtual sal_Bool SAL_CALL suspend( sal_Bool bSuspend ) override;
    virtual css::uno::Any SAL_CALL getViewData() override;
    virtual void SAL_CALL restoreViewData( const css::uno::Any& aData ) override;
    virtual css::uno::Reference< css::frame::XModel > SAL_CALL getModel() override;
    virtual css::uno::Reference< css::frame::XFrame > SAL_CALL getFrame() override;

    // XKeyListener
    virtual void SAL_CALL keyPressed ( const css::awt::KeyEvent& aEvent ) override;
    virtual void SAL_CALL keyReleased( const css::awt::KeyEvent& aEvent ) override;

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent ) override;

    // XComponent
    virtual void SAL_CALL dispose            (                                                                   ) override;
    virtual void SAL_CALL addEventListener   ( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;

    // XDispatchProvider
    virtual css::uno::Reference< css::frame::XDispatch > SAL_CALL queryDispatch( const css::util::URL& aURL, const OUString& sTargetFrameName , sal_Int32 nSearchFlags ) override;
    virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptions    ) override;

    // XDispatch
    virtual void SAL_CALL dispatch( const css::util::URL& aURL, const css::uno::Sequence< css::beans::PropertyValue >& lArguments ) override;
    virtual void SAL_CALL addStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xListener, const css::util::URL& aURL ) override;
    virtual void SAL_CALL removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xListener, const css::util::URL& aURL ) override;
};

BackingComp::BackingComp()
{
}

/** return information about supported interfaces.

    Some interfaces are supported by his class directly, but some other ones are
    used by aggregation. An instance of this class must provide some window interfaces.
    But it must represent a VCL window behind such interfaces too! So we use an internal
    saved window member to ask it for its interfaces and return it. But we must be aware then,
    that it can be destroyed from outside too ...

    @param  aType
                describe the required interface type

    @return An Any holding the instance, which provides the queried interface.
            Note: There exist two possible results ... this instance itself and her window member!
 */

css::uno::Any SAL_CALL BackingComp::queryInterface( /*IN*/ const css::uno::Type& aType )
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
                static_cast< css::awt::XKeyListener* >(static_cast< css::lang::XEventListener* >(this)),
                static_cast< css::frame::XDispatchProvider* >(this),
                static_cast< css::frame::XDispatch* >(this) );

    // then look for supported window interfaces
    // Note: They exist only, if this instance was initialized
    // with a valid window reference. It's aggregation on demand ...
    if (!aResult.hasValue())
    {
        /* SAFE { */
        SolarMutexGuard aGuard;
        if (m_xWindow.is())
            aResult = m_xWindow->queryInterface(aType);
        /* } SAFE */
    }

    // look for XWeak and XInterface
    if (!aResult.hasValue())
        aResult = OWeakObject::queryInterface(aType);

    return aResult;
}


/** increase ref count of this instance.
 */

void SAL_CALL BackingComp::acquire()
    throw()
{
    OWeakObject::acquire();
}


/** decrease ref count of this instance.
 */

void SAL_CALL BackingComp::release()
    throw()
{
    OWeakObject::release();
}


/** return collection about all supported interfaces.

    Optimize this method !
    We initialize a static variable only one time.
    And we don't must use a mutex at every call!
    For the first call; pTypeCollection is NULL -
    for the second call pTypeCollection is different from NULL!

    @return A list of all supported interface types.
*/

css::uno::Sequence< css::uno::Type > SAL_CALL BackingComp::getTypes()
{
    static ::cppu::OTypeCollection* pTypeCollection = nullptr;
    if (!pTypeCollection)
    {
        /* GLOBAL SAFE { */
        ::osl::MutexGuard aGlobalLock(m_aTypeProviderMutex);
        // Control these pointer again ... it can be, that another instance will be faster then this one!
        if (!pTypeCollection)
        {
            /* LOCAL SAFE { */
            SolarMutexGuard aGuard;
            css::uno::Reference< css::lang::XTypeProvider > xProvider(m_xWindow, css::uno::UNO_QUERY);

            css::uno::Sequence< css::uno::Type > lWindowTypes;
            if (xProvider.is())
                lWindowTypes = xProvider->getTypes();

            static ::cppu::OTypeCollection aTypeCollection(
                    cppu::UnoType<css::lang::XInitialization>::get(),
                    cppu::UnoType<css::lang::XTypeProvider>::get(),
                    cppu::UnoType<css::lang::XServiceInfo>::get(),
                    cppu::UnoType<css::frame::XController>::get(),
                    cppu::UnoType<css::lang::XComponent>::get(),
                    cppu::UnoType<css::frame::XDispatchProvider>::get(),
                    cppu::UnoType<css::frame::XDispatch>::get(),
                    lWindowTypes);

            pTypeCollection = &aTypeCollection;
            /* } LOCAL SAFE */
        }
        /* } GLOBAL SAFE */
    }
    return pTypeCollection->getTypes();
}


/** create one unique Id for all instances of this class.

    Optimize this method
    We initialize a static variable only one time. And we don't must use a mutex at every call!
    For the first call; pID is NULL - for the second call pID is different from NULL!

    @return A byte array, which represent the unique id.
*/

css::uno::Sequence< sal_Int8 > SAL_CALL BackingComp::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

OUString SAL_CALL BackingComp::getImplementationName()
{
    return OUString("com.sun.star.comp.sfx2.BackingComp");
}

sal_Bool SAL_CALL BackingComp::supportsService( /*IN*/ const OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

css::uno::Sequence< OUString > SAL_CALL BackingComp::getSupportedServiceNames()
{
    css::uno::Sequence< OUString > lNames(2);
    lNames[0] = "com.sun.star.frame.StartModule";
    lNames[1] = "com.sun.star.frame.ProtocolHandler";
    return lNames;
}


/**
    attach this component to a target frame.

    We have to use the container window of this frame as parent window of our own component window.
    But it's not allowed to work with it really. May another component used it too.
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
        // We will use its container window, to create
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

    @throw  css::uno::RuntimeException
                if the given frame reference is wrong or component window couldn't be created
                successfully.
                We throw it too, if we already attached to a frame. Because we don't support
                reparenting of our component window on demand!
*/

void SAL_CALL BackingComp::attachFrame( /*IN*/ const css::uno::Reference< css::frame::XFrame >& xFrame )
{
    /* SAFE */
    SolarMutexGuard aGuard;

    // check some required states
    if (m_xFrame.is())
        throw css::uno::RuntimeException(
                "already attached",
                static_cast< ::cppu::OWeakObject* >(this));

    if (!xFrame.is())
        throw css::uno::RuntimeException(
                "invalid frame reference",
                static_cast< ::cppu::OWeakObject* >(this));

    if (!m_xWindow.is())
        return; // disposed

    // safe the frame reference
    m_xFrame = xFrame;

    // initialize the component and its parent window
    css::uno::Reference< css::awt::XWindow > xParentWindow = xFrame->getContainerWindow();
    VclPtr< WorkWindow > pParent = static_cast<WorkWindow*>(VCLUnoHelper::GetWindow(xParentWindow).get());
    VclPtr< vcl::Window > pWindow = VCLUnoHelper::GetWindow(m_xWindow);

    // disable full screen mode of the frame!
    if (pParent && pParent->IsFullScreenMode())
    {
        pParent->ShowFullScreenMode(false);
        pParent->SetMenuBarMode(MenuBarMode::Normal);
    }

    // create the menu bar for the backing component
    css::uno::Reference< css::beans::XPropertySet > xPropSet(m_xFrame, css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::frame::XLayoutManager > xLayoutManager;
    xPropSet->getPropertyValue("LayoutManager") >>= xLayoutManager;
    if (xLayoutManager.is())
    {
        xLayoutManager->lock();
        xLayoutManager->createElement("private:resource/menubar/menubar");
        xLayoutManager->unlock();
    }

    if (pWindow)
    {
        // set help ID for our canvas
        pWindow->SetHelpId("FWK_HID_BACKINGWINDOW");
    }

    // inform BackingWindow about frame
    BackingWindow* pBack = dynamic_cast<BackingWindow*>(pWindow.get());
    if( pBack )
        pBack->setOwningFrame( m_xFrame );

    // set NotebookBar
    SystemWindow* pSysWindow = static_cast<SystemWindow*>(pParent);
    if (pSysWindow)
    {
        //sfx2::SfxNotebookBar::StateMethod(pSysWindow, m_xFrame, "sfx/ui/notebookbar.ui");
    }

    // Set a minimum size for Start Center
    if( pParent && pBack )
    {
        long nMenuHeight = 0;
        vcl::Window* pMenu = pParent->GetWindow(GetWindowType::Next);
        if( pMenu )
            nMenuHeight = pMenu->GetSizePixel().Height();

        m_aInitialWindowMinSize = pParent->GetMinOutputSizePixel();
        if (!m_aInitialWindowMinSize.Width())
            m_aInitialWindowMinSize.AdjustWidth(1);
        if (!m_aInitialWindowMinSize.Height())
            m_aInitialWindowMinSize.AdjustHeight(1);

        pParent->SetMinOutputSizePixel(
            Size(
                pBack->get_width_request(),
                pBack->get_height_request() + nMenuHeight));
    }

    /* } SAFE */
}


/** not supported.

    This component does not know any model. It will be represented by a window and
    its controller only.

    return  <FALSE/> every time.
 */

sal_Bool SAL_CALL BackingComp::attachModel( /*IN*/ const css::uno::Reference< css::frame::XModel >& )
{
    return false;
}


/** not supported.

    This component does not know any model. It will be represented by a window and
    its controller only.

    return  An empty reference every time.
 */

css::uno::Reference< css::frame::XModel > SAL_CALL BackingComp::getModel()
{
    return css::uno::Reference< css::frame::XModel >();
}


/** not supported.

    return  An empty value.
 */

css::uno::Any SAL_CALL BackingComp::getViewData()
{
    return css::uno::Any();
}


/** not supported.

    @param  aData
                not used.
 */

void SAL_CALL BackingComp::restoreViewData( /*IN*/ const css::uno::Any& )
{
}


/** returns the attached frame for this component.

    @see    attachFrame()

    @return The internally saved frame reference.
            Can be null, if attachFrame() was not called before.
 */

css::uno::Reference< css::frame::XFrame > SAL_CALL BackingComp::getFrame()
{
    /* SAFE { */
    SolarMutexGuard aGuard;
    return m_xFrame;
    /* } SAFE */
}


/** ask controller for its current working state.

    If someone wishes to close this component, it must suspend the controller before.
    That will be a chance for it to disagree with that AND show any UI for a possible
    UI user.

    @param  bSuspend
                If it's set to sal_True this controller should be suspended.
                sal_False will resuspend it.

    @return sal_True if the request could be finished successfully; sal_False otherwise.
 */

sal_Bool SAL_CALL BackingComp::suspend( /*IN*/ sal_Bool )
{
    /* FIXME ... implemented by using default :-( */
    return true;
}


/** callback from our window member.

    Our internal saved window wish to die. It will be disposed from outside (may be the frame)
    and inform us. We must release its reference only here. Of course we check the given reference
    here and reject callback from unknown sources.

    Note: deregistration as listener isn't necessary here. The broadcaster do it automatically.

    @param  aEvent
                describe the broadcaster of this callback

    @throw css::uno::RuntimeException
                if the broadcaster doesn't represent the expected window reference.
*/

void SAL_CALL BackingComp::disposing( /*IN*/ const css::lang::EventObject& aEvent )
{
    // Attention: don't free m_pAccExec here! see comments inside dtor and
    // keyPressed() for further details.

    /* SAFE { */
    SolarMutexGuard aGuard;

    if (!aEvent.Source.is() || aEvent.Source!=m_xWindow || !m_xWindow.is())
        throw css::uno::RuntimeException(
                "unexpected source or called twice",
                static_cast< ::cppu::OWeakObject* >(this));

    m_xWindow.clear();

    /* } SAFE */
}


/** kill this instance.

    It can be called from our owner frame only. But there is no possibility to check the caller.
    We have to release all our internal used resources and die. From this point we can throw
    DisposedExceptions for every further interface request... but current implementation doesn't do so...

*/

void SAL_CALL BackingComp::dispose()
{
    /* SAFE { */
    SolarMutexGuard aGuard;

    if (m_xFrame.is())
    {
        css::uno::Reference< css::awt::XWindow > xParentWindow = m_xFrame->getContainerWindow();
        VclPtr< WorkWindow > pParent = static_cast<WorkWindow*>(VCLUnoHelper::GetWindow(xParentWindow).get());
        if (pParent)
        {
            pParent->SetMinOutputSizePixel(m_aInitialWindowMinSize);
            // hide NotebookBar
            sfx2::SfxNotebookBar::CloseMethod(static_cast<SystemWindow*>(pParent));
        }
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
        m_xWindow.clear();
    }

    // forget all other used references
    m_xFrame.clear();

    /* } SAFE */
}


/** not supported.

    @param  xListener
                not used.

    @throw  css::uno::RuntimeException
                because the listener expect to be holded alive by this container.
                We must inform it about this unsupported feature.
 */

void SAL_CALL BackingComp::addEventListener( /*IN*/ const css::uno::Reference< css::lang::XEventListener >& )
{
    throw css::uno::RuntimeException(
            "not supported",
            static_cast< ::cppu::OWeakObject* >(this));
}


/** not supported.

    Because registration is not supported too, we must do nothing here. Nobody can call this method really.

    @param  xListener
                not used.
 */

void SAL_CALL BackingComp::removeEventListener( /*IN*/ const css::uno::Reference< css::lang::XEventListener >& )
{
}


/**
    force initialization for this component.

    Inside attachFrame() we created our component window. But it was not allowed there, to
    initialize it. E.g. the menu must be set at the container window of the frame, which
    is our parent window. But may at that time another component used it.
    That's why our creator has to inform us, when it's time to initialize us really.
    Currently only calling of this method must be done. But further implementations
    can use special in parameter to configure this initialization...

    @param  lArgs
                currently not used

    @throw  css::uno::RuntimeException
                if some resources are missing
                Means if may be attachedFrame() wasn't called before.
 */

void SAL_CALL BackingComp::initialize( /*IN*/ const css::uno::Sequence< css::uno::Any >& lArgs )
{
    /* SAFE { */
    SolarMutexGuard aGuard;

    if (m_xWindow.is())
        throw css::uno::Exception(
                "already initialized",
                static_cast< ::cppu::OWeakObject* >(this));

    css::uno::Reference< css::awt::XWindow > xParentWindow;
    if (
        (lArgs.getLength()!=1         ) ||
        (!(lArgs[0] >>= xParentWindow)) ||
        (!xParentWindow.is()          )
       )
    {
        throw css::uno::Exception(
                "wrong or corrupt argument list",
                static_cast< ::cppu::OWeakObject* >(this));
    }

    // create the component window
    VclPtr<vcl::Window> pParent = VCLUnoHelper::GetWindow(xParentWindow);
    VclPtr<vcl::Window> pWindow = VclPtr<BackingWindow>::Create(pParent);
    m_xWindow = VCLUnoHelper::GetInterface(pWindow);

    if (!m_xWindow.is())
        throw css::uno::RuntimeException(
                "couldn't create component window",
                static_cast< ::cppu::OWeakObject* >(this));

    // start listening for window disposing
    // It's set at our owner frame as component window later too. So it will may be disposed there ...
    css::uno::Reference< css::lang::XComponent > xBroadcaster(m_xWindow, css::uno::UNO_QUERY);
    if (xBroadcaster.is())
        xBroadcaster->addEventListener(static_cast< css::lang::XEventListener* >(this));

    m_xWindow->setVisible(true);

    /* } SAFE */
}


void SAL_CALL BackingComp::keyPressed( /*IN*/ const css::awt::KeyEvent&  )
{
}


void SAL_CALL BackingComp::keyReleased( /*IN*/ const css::awt::KeyEvent& )
{
    /* Attention
        Please use keyPressed() instead of this method. Otherwise it would be possible, that
        - a key input may be first switch to the backing mode
        - and this component register itself as key listener too
        - and it's first event will be a keyRealeased() for the already well known event, which switched to the backing mode!
        So it will be handled twice! document => backing mode => exit app ...
     */
}

// XDispatchProvider
css::uno::Reference< css::frame::XDispatch > SAL_CALL BackingComp::queryDispatch( const css::util::URL& aURL, const OUString& /*sTargetFrameName*/, sal_Int32 /*nSearchFlags*/ )
{
    css::uno::Reference< css::frame::XDispatch > xDispatch;
    if ( aURL.Protocol == "vnd.org.libreoffice.recentdocs:" )
        xDispatch = this;

    return xDispatch;
}

css::uno::Sequence < css::uno::Reference< css::frame::XDispatch > > SAL_CALL BackingComp::queryDispatches( const css::uno::Sequence < css::frame::DispatchDescriptor >& seqDescripts )
{
    sal_Int32 nCount = seqDescripts.getLength();
    css::uno::Sequence < css::uno::Reference < XDispatch > > lDispatcher( nCount );

    for( sal_Int32 i=0; i<nCount; ++i )
        lDispatcher[i] = queryDispatch( seqDescripts[i].FeatureURL, seqDescripts[i].FrameName, seqDescripts[i].SearchFlags );

    return lDispatcher;
}

// XDispatch
void SAL_CALL BackingComp::dispatch( const css::util::URL& aURL, const css::uno::Sequence < css::beans::PropertyValue >& /*lArgs*/ )
{
    // vnd.org.libreoffice.recentdocs:ClearRecentFileList  - clear recent files
    if ( aURL.Path == "ClearRecentFileList" )
    {
        VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow(m_xWindow);
        BackingWindow* pBack = dynamic_cast<BackingWindow*>(pWindow.get());
        if( pBack )
        {
            pBack->clearRecentFileList();

            // Recalculate minimum width
            css::uno::Reference< css::awt::XWindow > xParentWindow = m_xFrame->getContainerWindow();
            VclPtr< WorkWindow > pParent = static_cast<WorkWindow*>(VCLUnoHelper::GetWindow(xParentWindow).get());
            if( pParent )
            {
                pParent->SetMinOutputSizePixel( Size(
                        pBack->get_width_request(),
                        pParent->GetMinOutputSizePixel().Height()) );
            }
        }
    }
}

void SAL_CALL BackingComp::addStatusListener( const css::uno::Reference< css::frame::XStatusListener >& /*xControl*/, const css::util::URL& /*aURL*/ )
{
}

void SAL_CALL BackingComp::removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& /*xControl*/, const css::util::URL& /*aURL*/ )
{
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_sfx2_BackingComp_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new BackingComp);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
