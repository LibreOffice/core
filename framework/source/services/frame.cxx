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

#include <dispatch/dispatchprovider.hxx>
#include <dispatch/interceptionhelper.hxx>
#include <dispatch/closedispatcher.hxx>
#include <dispatch/windowcommanddispatch.hxx>
#include <loadenv/loadenv.hxx>
#include <helper/oframes.hxx>
#include <framework/titlehelper.hxx>
#include <svtools/openfiledroptargetlistener.hxx>
#include <classes/taskcreator.hxx>
#include <loadenv/targethelper.hxx>
#include <framework/framelistanalyzer.hxx>
#include <helper/dockingareadefaultacceptor.hxx>
#include <dispatch/dispatchinformationprovider.hxx>
#include <classes/framecontainer.hxx>
#include <classes/propertysethelper.hxx>
#include <threadhelp/transactionguard.hxx>
#include <threadhelp/transactionbase.hxx>
#include <general.h>

#include <pattern/window.hxx>
#include <properties.h>

#include <com/sun/star/awt/Toolkit.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#include <com/sun/star/frame/XFrame2.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XTitleChangeBroadcaster.hpp>
#include <com/sun/star/frame/LayoutManager.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/task/StatusIndicatorFactory.hpp>
#include <com/sun/star/task/theJobExecutor.hpp>
#include <com/sun/star/task/XJobExecutor.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <comphelper/sequenceashashmap.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/proptypehlp.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustrbuf.hxx>
#include <vcl/window.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/svapp.hxx>

#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/moduleoptions.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/menu.hxx>
#include <unotools/cmdoptions.hxx>

using namespace framework;

namespace {

// This enum can be used to set different active states of frames
enum EActiveState
{
    E_INACTIVE      ,   // I'am not a member of active path in tree and i don't have the focus.
    E_ACTIVE        ,   // I'am in the middle of an active path in tree and i don't have the focus.
    E_FOCUS             // I have the focus now. I must a member of an active path!
};

/*-************************************************************************************************************
    @short      implements a normal frame of hierarchy
    @descr      An instance of these class can be a normal node in frame tree. A frame support influencing of his
                subtree, find of subframes, activate- and deactivate-mechanism as well as
                set/get of a frame window, component or controller.

    @attention  This implementation supports three states: a)uninitialized, b)working, c)disposed
                If you call wrong methods in modes a) or c) ... you will get some exceptions.
                How you should work with this service:
                    i)      create it by using "xServiceManager->createInstance(...)"
                    ii)     call XInitialization::initialize() with a valid window reference or use createInstanceWithArguments() at i)
                    iii)    works with object
                    iv)     dispose object by calling XComponent::dispose()
                After iv) all further requests are rejected by exceptions! (DisposedException)

    @base       TransactionBase
                    help to implement unbreakable interface calls
    @base       OBroadcastHelper
                OPropertySetHelper
                    implements the property set
    @base       OWeakObject
                    provides the refcount and XInterface, XWeak

    @devstatus  ready to use
    @threadsafe yes
*//*-*************************************************************************************************************/
class Frame :   // interfaces
                public  css::lang::XTypeProvider                    ,
                public  css::lang::XServiceInfo                     ,
                public  css::frame::XFrame2                         ,
                public  css::awt::XWindowListener                   ,   // => XEventListener
                public  css::awt::XTopWindowListener                ,
                public  css::awt::XFocusListener                    ,
                public  css::document::XActionLockable              ,
                public  css::util::XCloseable                       ,   // => XCloseBroadcaster
                public  css::frame::XComponentLoader                ,
                public  css::frame::XTitle                          ,
                public  css::frame::XTitleChangeBroadcaster         ,
                // base classes
                public  TransactionBase                             ,
                private cppu::BaseMutex,
                public  PropertySetHelper                           ,   // helper implements TransactionBase, XPropertySet, XPropertySetInfo
                public  ::cppu::OWeakObject                             // helper implements XInterface, XWeak
{
public:

    explicit Frame(const css::uno::Reference< css::uno::XComponentContext >& xContext);
    virtual ~Frame();

    /// Initialization function after having acquire()'d.
    void initListeners();

    FWK_DECLARE_XINTERFACE
    FWK_DECLARE_XTYPEPROVIDER

    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override
    {
        return OUString("com.sun.star.comp.framework.Frame");
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override
    {
        css::uno::Sequence< OUString > aSeq { "com.sun.star.frame.Frame" };
        return aSeq;
    }

    //  XComponentLoader

    virtual css::uno::Reference< css::lang::XComponent > SAL_CALL loadComponentFromURL(
            const OUString& sURL,
            const OUString& sTargetFrameName,
            sal_Int32 nSearchFlags,
            const css::uno::Sequence< css::beans::PropertyValue >& lArguments )
        throw( css::io::IOException,
               css::lang::IllegalArgumentException,
               css::uno::RuntimeException, std::exception ) override;

    //  XFramesSupplier

    virtual css::uno::Reference< css::frame::XFrames >          SAL_CALL getFrames                          (                                                                                                   ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference< css::frame::XFrame >           SAL_CALL getActiveFrame                     (                                                                                                   ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void                                                SAL_CALL setActiveFrame                     (   const   css::uno::Reference< css::frame::XFrame >&                          xFrame              ) throw( css::uno::RuntimeException, std::exception ) override;

    //  XFrame

    virtual void                                                SAL_CALL initialize                         (   const   css::uno::Reference< css::awt::XWindow >&                           xWindow             ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference< css::awt::XWindow >            SAL_CALL getContainerWindow                 (                                                                                                   ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void                                                SAL_CALL setCreator                         (   const   css::uno::Reference< css::frame::XFramesSupplier >&                 xCreator            ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference< css::frame::XFramesSupplier >  SAL_CALL getCreator                         (                                                                                                   ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual OUString                                     SAL_CALL getName                            (                                                                                                   ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void                                                SAL_CALL setName                            (   const   OUString&                                                    sName               ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference< css::frame::XFrame >           SAL_CALL findFrame                          (   const   OUString&                                                    sTargetFrameName    ,
                                                                                                                        sal_Int32                                                           nSearchFlags        ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool                                            SAL_CALL isTop                              (                                                                                                   ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void                                                SAL_CALL activate                           (                                                                                                   ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void                                                SAL_CALL deactivate                         (                                                                                                   ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool                                            SAL_CALL isActive                           (                                                                                                   ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void                                                SAL_CALL contextChanged                     (                                                                                                   ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool                                            SAL_CALL setComponent                       (   const   css::uno::Reference< css::awt::XWindow >&                           xComponentWindow    ,
                                                                                                                const   css::uno::Reference< css::frame::XController >&                     xController         ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference< css::awt::XWindow >            SAL_CALL getComponentWindow                 (                                                                                                   ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference< css::frame::XController >      SAL_CALL getController                      (                                                                                                   ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void                                                SAL_CALL addFrameActionListener             (   const   css::uno::Reference< css::frame::XFrameActionListener >&            xListener           ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void                                                SAL_CALL removeFrameActionListener          (   const   css::uno::Reference< css::frame::XFrameActionListener >&            xListener           ) throw( css::uno::RuntimeException, std::exception ) override;

    //  XComponent

    virtual void                                                SAL_CALL dispose                            (                                                                                                   ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void                                                SAL_CALL addEventListener                   (   const   css::uno::Reference< css::lang::XEventListener >&                   xListener           ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void                                                SAL_CALL removeEventListener                (   const   css::uno::Reference< css::lang::XEventListener >&                   xListener           ) throw( css::uno::RuntimeException, std::exception ) override;

    //  XStatusIndicatorFactory

    virtual css::uno::Reference< css::task::XStatusIndicator >  SAL_CALL createStatusIndicator              (                                                                                                   ) throw( css::uno::RuntimeException, std::exception ) override;

    //  XDispatchProvider

    virtual css::uno::Reference< css::frame::XDispatch >        SAL_CALL queryDispatch                      (   const   css::util::URL&                                                     aURL                ,
                                                                                                                const   OUString&                                                    sTargetFrameName    ,
                                                                                                                        sal_Int32                                                           nSearchFlags        ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence<
                css::uno::Reference< css::frame::XDispatch > >  SAL_CALL queryDispatches                    (   const   css::uno::Sequence< css::frame::DispatchDescriptor >&               lDescriptor         ) throw( css::uno::RuntimeException, std::exception ) override;

    //  XDispatchProviderInterception

    virtual void                                                SAL_CALL registerDispatchProviderInterceptor(   const   css::uno::Reference< css::frame::XDispatchProviderInterceptor >&    xInterceptor        ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void                                                SAL_CALL releaseDispatchProviderInterceptor (   const   css::uno::Reference< css::frame::XDispatchProviderInterceptor >&    xInterceptor        ) throw( css::uno::RuntimeException, std::exception ) override;

    //  XDispatchInformationProvider

    virtual css::uno::Sequence< sal_Int16 >                       SAL_CALL getSupportedCommandGroups         (                       ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::frame::DispatchInformation > SAL_CALL getConfigurableDispatchInformation(sal_Int16 nCommandGroup) throw (css::uno::RuntimeException, std::exception) override;

    //  XWindowListener
    //  Attention: windowResized() and windowShown() are implement only! All other are empty!

    virtual void                                                SAL_CALL windowResized                      (   const   css::awt::WindowEvent&                                              aEvent              ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void                                                SAL_CALL windowMoved                        (   const   css::awt::WindowEvent&                                              /*aEvent*/          ) throw( css::uno::RuntimeException, std::exception ) override {};
    virtual void                                                SAL_CALL windowShown                        (   const   css::lang::EventObject&                                             aEvent              ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void                                                SAL_CALL windowHidden                       (   const   css::lang::EventObject&                                             aEvent              ) throw( css::uno::RuntimeException, std::exception ) override;

    //  XFocusListener
    //  Attention: focusLost() not implemented yet!

    virtual void                                                SAL_CALL focusGained                        (   const   css::awt::FocusEvent&                                               aEvent              ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void                                                SAL_CALL focusLost                          (   const   css::awt::FocusEvent&                                               /*aEvent*/          ) throw( css::uno::RuntimeException, std::exception ) override {};

    //  XTopWindowListener
    //  Attention: windowActivated(), windowDeactivated() and windowClosing() are implement only! All other are empty!

    virtual void                                                SAL_CALL windowActivated                    (   const   css::lang::EventObject&                                             aEvent              ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void                                                SAL_CALL windowDeactivated                  (   const   css::lang::EventObject&                                             aEvent              ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void                                                SAL_CALL windowOpened                       (   const   css::lang::EventObject&                                             /*aEvent*/          ) throw( css::uno::RuntimeException, std::exception ) override {};
    virtual void                                                SAL_CALL windowClosing                      (   const   css::lang::EventObject&                                             aEvent              ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void                                                SAL_CALL windowClosed                       (   const   css::lang::EventObject&                                             /*aEvent*/          ) throw( css::uno::RuntimeException, std::exception ) override {};
    virtual void                                                SAL_CALL windowMinimized                    (   const   css::lang::EventObject&                                             /*aEvent*/          ) throw( css::uno::RuntimeException, std::exception ) override {};
    virtual void                                                SAL_CALL windowNormalized                   (   const   css::lang::EventObject&                                             /*aEvent*/          ) throw( css::uno::RuntimeException, std::exception ) override {};

    //  XEventListener

    virtual void                                                SAL_CALL disposing                          (   const   css::lang::EventObject&                                             aEvent              ) throw( css::uno::RuntimeException, std::exception ) override;

    //  XActionLockable

    virtual sal_Bool    SAL_CALL isActionLocked  (                 ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void        SAL_CALL addActionLock   (                 ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void        SAL_CALL removeActionLock(                 ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void        SAL_CALL setActionLocks  ( sal_Int16 nLock ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Int16   SAL_CALL resetActionLocks(                 ) throw( css::uno::RuntimeException, std::exception ) override;

    //  XCloseable

    virtual void SAL_CALL close( sal_Bool bDeliverOwnership ) throw( css::util::CloseVetoException,
                                                                     css::uno::RuntimeException, std::exception   ) override;

    //  XCloseBroadcaster

    virtual void SAL_CALL addCloseListener   ( const css::uno::Reference< css::util::XCloseListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeCloseListener( const css::uno::Reference< css::util::XCloseListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;

    //  XTitle

    virtual OUString SAL_CALL getTitle(                               ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void            SAL_CALL setTitle( const OUString& sTitle ) throw (css::uno::RuntimeException, std::exception) override;

    //  XTitleChangeBroadcaster

    virtual void SAL_CALL addTitleChangeListener   ( const css::uno::Reference< css::frame::XTitleChangeListener >& xListener) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeTitleChangeListener( const css::uno::Reference< css::frame::XTitleChangeListener >& xListenr ) throw (css::uno::RuntimeException, std::exception) override;

    //  XFrame2 attributes

    virtual css::uno::Reference<css::container::XNameContainer> SAL_CALL getUserDefinedAttributes() throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference<css::frame::XDispatchRecorderSupplier> SAL_CALL getDispatchRecorderSupplier() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDispatchRecorderSupplier(const css::uno::Reference<css::frame::XDispatchRecorderSupplier>&) throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference<css::uno::XInterface> SAL_CALL getLayoutManager() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setLayoutManager(const css::uno::Reference<css::uno::XInterface>&) throw (css::uno::RuntimeException, std::exception) override;

    //  PropertySetHelper => XPropertySet, XPropertySetInfo

private:

    void impl_initializePropInfo();

    virtual void SAL_CALL impl_setPropertyValue(const OUString& sProperty,
                                                      sal_Int32        nHandle  ,
                                                const css::uno::Any&   aValue   ) override;

    virtual css::uno::Any SAL_CALL impl_getPropertyValue(const OUString& sProperty,
                                                               sal_Int32        nHandle  ) override;


private:

    /*-****************************************************************************************************
        @short      helper methods
        @descr      Follow methods are needed at different points of our code (more than ones!).

        @attention  Threadsafe methods are signed by "implts_..."!
    *//*-*****************************************************************************************************/

    // threadsafe
    void implts_sendFrameActionEvent     ( const css::frame::FrameAction&                        aAction          );
    void implts_resizeComponentWindow    (                                                                        );
    void implts_setIconOnWindow          (                                                                        );
    void implts_startWindowListening     (                                                                        );
    void implts_stopWindowListening      (                                                                        );
    void implts_checkSuicide             (                                                                        );
    void implts_forgetSubFrames          (                                                                        );

    // non threadsafe
    void impl_checkMenuCloser            (                                                                        );
    void impl_setCloser                  ( const css::uno::Reference< css::frame::XFrame2 >& xFrame , bool bState );
    void impl_disposeContainerWindow     (       css::uno::Reference< css::awt::XWindow >&       xWindow          );

//  debug methods
//  (should be private everyway!)

    /*-****************************************************************************************************
        @short      debug-method to check incoming parameter of some other mehods of this class
        @descr      The following methods are used to check parameters for other methods
                    of this class. The return value is used directly for an ASSERT(...).

        @attention  This methods are static and can't use our member directly! It's better for threadsafe code...
                    because we call it with references or pointer to check variables ... and must make it safe
                    by himself!

        @seealso    ASSERTs in implementation!

        @param      references to checking variables
        @return     sal_True  ,on invalid parameter
        @return     sal_False ,otherwise

        @onerror    We return sal_True
    *//*-*****************************************************************************************************/

private:

    static bool implcp_setActiveFrame                      (   const   css::uno::Reference< css::frame::XFrame >&                          xFrame              );
    static bool implcp_addFrameActionListener              (   const   css::uno::Reference< css::frame::XFrameActionListener >&            xListener           );
    static bool implcp_removeFrameActionListener           (   const   css::uno::Reference< css::frame::XFrameActionListener >&            xListener           );
    static bool implcp_addEventListener                    (   const   css::uno::Reference< css::lang::XEventListener >&                   xListener           );
    static bool implcp_removeEventListener                 (   const   css::uno::Reference< css::lang::XEventListener >&                   xListener           );
    static bool implcp_windowResized                       (   const   css::awt::WindowEvent&                                              aEvent              );
    static bool implcp_focusGained                         (   const   css::awt::FocusEvent&                                               aEvent              );
    static bool implcp_windowActivated                     (   const   css::lang::EventObject&                                             aEvent              );
    static bool implcp_windowDeactivated                   (   const   css::lang::EventObject&                                             aEvent              );
    static bool implcp_disposing                           (   const   css::lang::EventObject&                                             aEvent              );

//  variables
//  -threadsafe by SolarMutex

private:

    css::uno::Reference< css::uno::XComponentContext >                      m_xContext;                  /// reference to factory, which has create this instance
    css::uno::Reference< css::task::XStatusIndicatorFactory >               m_xIndicatorFactoryHelper;   /// reference to factory helper to create status indicator objects
    css::uno::WeakReference< css::task::XStatusIndicator >                  m_xIndicatorInterception;    /// points to an external set progress, which should be used instead of the internal one.
    css::uno::Reference< css::frame::XDispatchProvider >                    m_xDispatchHelper;           /// helper for XDispatch/Provider and interception interfaces
    css::uno::Reference< css::frame::XFrames >                              m_xFramesHelper;             /// helper for XFrames, XIndexAccess and XElementAccess interfaces
    ::cppu::OMultiTypeInterfaceContainerHelper                              m_aListenerContainer;        /// container for ALL Listener
    css::uno::Reference< css::frame::XFramesSupplier >                      m_xParent;                   /// parent of this frame
    css::uno::Reference< css::awt::XWindow >                                m_xContainerWindow;          /// containerwindow of this frame for embedded components
    css::uno::Reference< css::awt::XWindow >                                m_xComponentWindow;          /// window of the actual component
    css::uno::Reference< css::frame::XController >                          m_xController;               /// controller of the actual frame
    css::uno::Reference< css::datatransfer::dnd::XDropTargetListener >      m_xDropTargetListener;       /// listen to drag & drop
    EActiveState                                                            m_eActiveState;              /// state, if I am a member of active path in tree or I have the focus or...
    OUString                                                                m_sName;                     /// name of this frame
    bool                                                                    m_bIsFrameTop;               /// frame has no parent or the parent is a task or the desktop
    bool                                                                    m_bConnected;                /// due to FrameActionEvent
    sal_Int16                                                               m_nExternalLockCount;
    css::uno::Reference< css::frame::XDispatchRecorderSupplier >            m_xDispatchRecorderSupplier; /// is used for dispatch recording and will be set/get from outside. Frame provide it only!
    SvtCommandOptions                                                       m_aCommandOptions;           /// ref counted class to support disabling commands defined by configuration file
    bool                                                                    m_bSelfClose;                /// in case of CloseVetoException on method close() was thrown by ourself - we must close ourself later if no internal processes are running
    bool                                                                    m_bIsHidden;                 /// indicates, if this frame is used in hidden mode or not
    css::uno::Reference< css::frame::XLayoutManager2 >                      m_xLayoutManager;            /// is used to layout the child windows of the frame.
    css::uno::Reference< css::frame::XDispatchInformationProvider >         m_xDispatchInfoHelper;
    css::uno::Reference< css::frame::XTitle >                               m_xTitleHelper;

    WindowCommandDispatch*                                                  m_pWindowCommandDispatch;

protected:

    FrameContainer                                                          m_aChildFrameContainer;   /// array of child frames
};

//  XInterface, XTypeProvider, XServiceInfo

DEFINE_XINTERFACE_22                (   Frame                                                                   ,
                                        OWeakObject                                                             ,
                                        DIRECT_INTERFACE(css::lang::XTypeProvider                               ),
                                        DIRECT_INTERFACE(css::lang::XServiceInfo                                ),
                                        DIRECT_INTERFACE(css::frame::XFrame2                                    ),
                                        DIRECT_INTERFACE(css::frame::XFramesSupplier                            ),
                                        DIRECT_INTERFACE(css::frame::XFrame                                     ),
                                        DIRECT_INTERFACE(css::task::XStatusIndicatorFactory                     ),
                                        DIRECT_INTERFACE(css::frame::XDispatchProvider                          ),
                                        DIRECT_INTERFACE(css::frame::XDispatchInformationProvider               ),
                                        DIRECT_INTERFACE(css::frame::XDispatchProviderInterception              ),
                                        DIRECT_INTERFACE(css::lang::XComponent                                  ),
                                        DIRECT_INTERFACE(css::beans::XPropertySet                               ),
                                        DIRECT_INTERFACE(css::beans::XPropertySetInfo                           ),
                                        DIRECT_INTERFACE(css::awt::XWindowListener                              ),
                                        DIRECT_INTERFACE(css::awt::XTopWindowListener                           ),
                                        DIRECT_INTERFACE(css::awt::XFocusListener                               ),
                                        DERIVED_INTERFACE(css::lang::XEventListener, css::awt::XWindowListener  ),
                                        DIRECT_INTERFACE(css::document::XActionLockable                         ),
                                        DIRECT_INTERFACE(css::util::XCloseable                                  ),
                                        DIRECT_INTERFACE(css::util::XCloseBroadcaster                           ),
                                        DIRECT_INTERFACE(css::frame::XComponentLoader                           ),
                                        DIRECT_INTERFACE(css::frame::XTitle                                     ),
                                        DIRECT_INTERFACE(css::frame::XTitleChangeBroadcaster                    )
                                    )

DEFINE_XTYPEPROVIDER_21             (   Frame                                                                   ,
                                        css::lang::XTypeProvider                                                ,
                                        css::lang::XServiceInfo                                                 ,
                                        css::frame::XFrame2                                                     ,
                                        css::frame::XFramesSupplier                                             ,
                                        css::frame::XFrame                                                      ,
                                        css::task::XStatusIndicatorFactory                                      ,
                                        css::frame::XDispatchProvider                                           ,
                                        css::frame::XDispatchInformationProvider                                ,
                                        css::frame::XDispatchProviderInterception                               ,
                                        css::lang::XComponent                                                   ,
                                        css::beans::XPropertySet                                                ,
                                        css::beans::XPropertySetInfo                                            ,
                                        css::awt::XWindowListener                                               ,
                                        css::awt::XTopWindowListener                                            ,
                                        css::awt::XFocusListener                                                ,
                                        css::lang::XEventListener                                               ,
                                        css::util::XCloseable                                                   ,
                                        css::util::XCloseBroadcaster                                            ,
                                        css::frame::XComponentLoader                                            ,
                                        css::frame::XTitle                                                      ,
                                        css::frame::XTitleChangeBroadcaster
                                    )

/*-****************************************************************************************************
    @short      standard constructor to create instance by factory
    @descr      This constructor initialize a new instance of this class by valid factory,
                and will be set valid values on his member and baseclasses.

    @attention  a)  Don't use your own reference during an UNO-Service-ctor! There is no guarantee, that you
                    will get over this. (e.g. using of your reference as parameter to initialize some member)
                    Do such things in DEFINE_INIT_SERVICE() method, which is called automatically after your ctor!!!
                b)  Baseclass OBroadcastHelper is a typedef in namespace cppu!
                    The microsoft compiler has some problems to handle it right BY using namespace explicitly ::cppu::OBroadcastHelper.
                    If we write it without a namespace or expand the typedef to OBrodcastHelperVar<...> -> it will be OK!?
                    I don't know why! (other compiler not tested .. but it works!)

    @seealso    method DEFINE_INIT_SERVICE()

    @param      xContext is the multi service manager, which creates this instance.
                    The value must be different from NULL!
    @onerror    ASSERT in debug version or nothing in release version.
*//*-*****************************************************************************************************/
Frame::Frame( const css::uno::Reference< css::uno::XComponentContext >& xContext )
        :   TransactionBase             (                                                   )
        ,   PropertySetHelper           ( m_aMutex,
                                          &m_aTransactionManager,
                                          false) // sal_False => don't release shared mutex on calling us!
        //  init member
        ,   m_xContext                  ( xContext                                          )
        ,   m_aListenerContainer        ( m_aMutex )
        ,   m_xParent                   (                                                   )
        ,   m_xContainerWindow          (                                                   )
        ,   m_xComponentWindow          (                                                   )
        ,   m_xController               (                                                   )
        ,   m_eActiveState              ( E_INACTIVE                                        )
        ,   m_sName                     (                                                   )
        ,   m_bIsFrameTop               ( true                                          ) // I think we are top without a parent ... and there is no parent yet!
        ,   m_bConnected                ( false                                         ) // There exist no component inside of use => sal_False, we are not connected!
        ,   m_nExternalLockCount        ( 0                                                 )
        ,   m_bSelfClose                ( false                                         ) // Important!
        ,   m_bIsHidden                 ( true                                          )
        ,   m_xTitleHelper              (                                                   )
        ,   m_pWindowCommandDispatch    ( nullptr                                                 )
        ,   m_aChildFrameContainer      (                                                   )
{
}

void Frame::initListeners()
{
    css::uno::Reference< css::uno::XInterface > xThis(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY_THROW);

    // Initialize a new dispatchhelper-object to handle dispatches.
    // We use these helper as slave for our interceptor helper ... not directly!
    // But he is event listener on THIS instance!
    DispatchProvider* pDispatchHelper = new DispatchProvider( m_xContext, this );
    css::uno::Reference< css::frame::XDispatchProvider > xDispatchProvider( static_cast< ::cppu::OWeakObject* >(pDispatchHelper), css::uno::UNO_QUERY );

    DispatchInformationProvider* pInfoHelper = new DispatchInformationProvider(m_xContext, this);
    m_xDispatchInfoHelper.set( static_cast< ::cppu::OWeakObject* >(pInfoHelper), css::uno::UNO_QUERY );

    // Initialize a new interception helper object to handle dispatches and implement an interceptor mechanism.
    // Set created dispatch provider as slowest slave of it.
    // Hold interception helper by reference only - not by pointer!
    // So it's easier to destroy it.
    InterceptionHelper* pInterceptionHelper = new InterceptionHelper( this, xDispatchProvider );
    m_xDispatchHelper.set( static_cast< ::cppu::OWeakObject* >(pInterceptionHelper), css::uno::UNO_QUERY );

    // Initialize a new XFrames-helper-object to handle XIndexAccess and XElementAccess.
    // We hold member as reference ... not as pointer too!
    // Attention: We share our frame container with this helper. Container is threadsafe himself ... So I think we can do that.
    // But look on dispose() for right order of deinitialization.
    OFrames* pFramesHelper = new OFrames( this, &m_aChildFrameContainer );
    m_xFramesHelper.set( static_cast< ::cppu::OWeakObject* >(pFramesHelper), css::uno::UNO_QUERY );

    // Initialize a the drop target listener.
    // We hold member as reference ... not as pointer too!
    OpenFileDropTargetListener* pDropListener = new OpenFileDropTargetListener( m_xContext, this );
    m_xDropTargetListener.set( static_cast< ::cppu::OWeakObject* >(pDropListener), css::uno::UNO_QUERY );

    // Safe impossible cases
    // We can't work without these helpers!
    SAL_WARN_IF( !xDispatchProvider.is(), "fwk", "Frame::Frame(): Slowest slave for dispatch- and interception helper is not valid. XDispatchProvider, XDispatch, XDispatchProviderInterception are not full supported!" );
    SAL_WARN_IF( !m_xDispatchHelper.is(), "fwk", "Frame::Frame(): Interception helper is not valid. XDispatchProvider, XDispatch, XDispatchProviderInterception are not full supported!" );
    SAL_WARN_IF( !m_xFramesHelper.is(), "fwk", "Frame::Frame(): Frames helper is not valid. XFrames, XIndexAccess and XElementAcces are not supported!" );
    SAL_WARN_IF( !m_xDropTargetListener.is(), "fwk", "Frame::Frame(): DropTarget helper is not valid. Drag and drop without functionality!" );

    // establish notifies for changing of "disabled commands" configuration during runtime
    m_aCommandOptions.EstablisFrameCallback(this);

    // Create an initial layout manager
    // Create layout manager and connect it to the newly created frame
    m_xLayoutManager = css::frame::LayoutManager::create(m_xContext);

    // set information about all supported properties at the base class helper PropertySetHelper
    impl_initializePropInfo();
}

/*-****************************************************************************************************
    @short      standard destructor
    @descr      This one do NOTHING! Use dispose() instaed of this.

    @seealso    method dispose()
*//*-*****************************************************************************************************/
Frame::~Frame()
{
    SAL_WARN_IF( m_aTransactionManager.getWorkingMode()!=E_CLOSE, "fwk", "Frame::~Frame(): Who forgot to dispose this service?" );
}

/*-************************************************************************************************************
    @interface  XComponentLoader
    @short      try to load given URL into a task
    @descr      You can give us some information about the content, which you will load into a frame.
                We search or create this target for you, make a type detection of given URL and try to load it.
                As result of this operation we return the new created component or nothing, if loading failed.
    @param      "sURL"              , URL, which represent the content
    @param      "sTargetFrameName"  , name of target frame or special value like "_self", "_blank" ...
    @param      "nSearchFlags"      , optional arguments for frame search, if target is not a special one
    @param      "lArguments"        , optional arguments for loading
    @return     A valid component reference, if loading was successfully.
                A null reference otherwise.

    @onerror    We return a null reference.
    @threadsafe yes
*//*-*************************************************************************************************************/
css::uno::Reference< css::lang::XComponent > SAL_CALL Frame::loadComponentFromURL( const OUString&                                 sURL            ,
                                                                                   const OUString&                                 sTargetFrameName,
                                                                                         sal_Int32                                        nSearchFlags    ,
                                                                                   const css::uno::Sequence< css::beans::PropertyValue >& lArguments      ) throw( css::io::IOException                ,
                                                                                                                                                                   css::lang::IllegalArgumentException ,
                                                                                                                                                                   css::uno::RuntimeException, std::exception          )
{
    {
        // If the frame is closed the call might lead to crash even with target "_blank",
        // so the DisposedException should be thrown in this case
        // It still looks to be too dangerous to set the transaction for the whole loading process
        // so the guard is used in scopes to let the standard check be used

        TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );
    }

    SolarMutexClearableGuard aReadLock;
    css::uno::Reference< css::frame::XComponentLoader > xThis(static_cast< css::frame::XComponentLoader* >(this), css::uno::UNO_QUERY);
    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;
    aReadLock.clear();

    return LoadEnv::loadComponentFromURL(xThis, xContext, sURL, sTargetFrameName, nSearchFlags, lArguments);
}

/*-****************************************************************************************************
    @short      return access to append or remove children on desktop
    @descr      We don't implement these interface directly. We use a helper class to do this.
                If you wish to add or delete children to/from the container, call these method to get
                a reference to the helper.

    @seealso    class OFrames
    @return     A reference to the helper which answer your queries.

    @onerror    A null reference is returned.
*//*-*****************************************************************************************************/
css::uno::Reference< css::frame::XFrames > SAL_CALL Frame::getFrames() throw( css::uno::RuntimeException, std::exception )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.

/*TODO
    This is a temp. HACK!
    Our parent (a Task!) stand in close/dispose and set working mode to E_BEFOERECLOSE
    and call dispose on us! We tra to get this xFramesHelper and are reject by an "already closed" parent instance....
    => We use SOFTEXCEPTIONS here... but we should make it right in further times....
 */

    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    SolarMutexGuard g;
    // Return access to all child frames to caller.
    // Our childframe container is implemented in helper class OFrames and used as a reference m_xFramesHelper!
    return m_xFramesHelper;
}

/*-****************************************************************************************************
    @short      get the current active child frame
    @descr      It must be a frameto. Direct children of a frame are frames only! No task or desktop is accepted.
                We don't save this information directly in this class. We use our container-helper
                to do that.

    @seealso    class OFrameContainer
    @seealso    method setActiveFrame()
    @return     A reference to our current active childframe, if anyone exist.
    @return     A null reference, if nobody is active.

    @onerror    A null reference is returned.
*//*-*****************************************************************************************************/
css::uno::Reference< css::frame::XFrame > SAL_CALL Frame::getActiveFrame() throw( css::uno::RuntimeException, std::exception )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    SolarMutexGuard g;
    // Return current active frame.
    // This information is available on the container.
    return m_aChildFrameContainer.getActive();
}

/*-****************************************************************************************************
    @short      set the new active direct child frame
    @descr      It must be a frame to. Direct children of frame are frames only! No task or desktop is accepted.
                We don't save this information directly in this class. We use our container-helper
                to do that.

    @seealso    class OFrameContainer
    @seealso    method getActiveFrame()

    @param      "xFrame", reference to new active child. It must be an already existing child!
    @onerror    An assertion is thrown and element is ignored, if given frame isn't already a child of us.
*//*-*****************************************************************************************************/
void SAL_CALL Frame::setActiveFrame( const css::uno::Reference< css::frame::XFrame >& xFrame ) throw( css::uno::RuntimeException, std::exception )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Check incoming parameters.
    SAL_WARN_IF( implcp_setActiveFrame( xFrame ), "fwk", "Frame::setActiveFrame(): Invalid parameter detected!" );
    // Look for rejected calls!
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    SolarMutexResettableGuard aWriteLock;

    // Copy necessary member for threadsafe access!
    // m_aChildFrameContainer is threadsafe himself and he live if we live!!!
    // ...and our transaction is non breakable too ...
    css::uno::Reference< css::frame::XFrame > xActiveChild = m_aChildFrameContainer.getActive();
    EActiveState                              eActiveState = m_eActiveState;

    aWriteLock.clear();
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */

    // Don't work, if "new" active frame isn't different from current one!
    // (xFrame==NULL is allowed to UNSET it!)
    if( xActiveChild != xFrame )
    {
        // ... otherwise set new and deactivate old one.
        m_aChildFrameContainer.setActive( xFrame );
        if  (
                ( eActiveState      !=  E_INACTIVE  )   &&
                xActiveChild.is()
            )
        {
            xActiveChild->deactivate();
        }
    }

    if( xFrame.is() )
    {
        // If last active frame had focus ...
        // ... reset state to ACTIVE and send right FrameActionEvent for focus lost.
        if( eActiveState == E_FOCUS )
        {
            aWriteLock.reset();
            eActiveState   = E_ACTIVE;
            m_eActiveState = eActiveState;
            aWriteLock.clear();
            implts_sendFrameActionEvent( css::frame::FrameAction_FRAME_UI_DEACTIVATING );
        }

        // If last active frame was active ...
        // but new one is not it ...
        // ... set it as active one.
        if ( eActiveState == E_ACTIVE && !xFrame->isActive() )
        {
            xFrame->activate();
        }
    }
    else
    // If this frame is active and has no active subframe anymore it is UI active too
    if( eActiveState == E_ACTIVE )
    {
        aWriteLock.reset();
        eActiveState   = E_FOCUS;
        m_eActiveState = eActiveState;
        aWriteLock.clear();
        implts_sendFrameActionEvent( css::frame::FrameAction_FRAME_UI_ACTIVATED );
    }
}

/*-****************************************************************************************************
   initialize new created layout manager
**/
void lcl_enableLayoutManager(const css::uno::Reference< css::frame::XLayoutManager2 >& xLayoutManager,
                             const css::uno::Reference< css::frame::XFrame >&         xFrame        )
{
    // Provide container window to our layout manager implementation
    xLayoutManager->attachFrame(xFrame);

    xFrame->addFrameActionListener(xLayoutManager);

    DockingAreaDefaultAcceptor* pAcceptor = new DockingAreaDefaultAcceptor(xFrame);
    css::uno::Reference< css::ui::XDockingAreaAcceptor > xDockingAreaAcceptor( static_cast< ::cppu::OWeakObject* >(pAcceptor), css::uno::UNO_QUERY_THROW);
    xLayoutManager->setDockingAreaAcceptor(xDockingAreaAcceptor);
}

/*-****************************************************************************************************
   deinitialize layout manager
**/
void lcl_disableLayoutManager(const css::uno::Reference< css::frame::XLayoutManager2 >& xLayoutManager,
                              const css::uno::Reference< css::frame::XFrame >&         xFrame        )
{
    xFrame->removeFrameActionListener(xLayoutManager);
    xLayoutManager->setDockingAreaAcceptor(css::uno::Reference< css::ui::XDockingAreaAcceptor >());
    xLayoutManager->attachFrame(css::uno::Reference< css::frame::XFrame >());
}

/*-****************************************************************************************************
    @short      initialize frame instance
    @descr      A frame needs a window. This method set a new one ... but should called one times only!
                We use this window to listen for window events and forward it to our set component.
                Its used as parent of component window too.

    @seealso    method getContainerWindow()
    @seealso    method setComponent()
    @seealso    member m_xContainerWindow

    @param      "xWindow", reference to new container window - must be valid!
    @onerror    We do nothing.
*//*-*****************************************************************************************************/
void SAL_CALL Frame::initialize( const css::uno::Reference< css::awt::XWindow >& xWindow ) throw( css::uno::RuntimeException, std::exception )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    if (!xWindow.is())
        throw css::uno::RuntimeException(
                    "Frame::initialize() called without a valid container window reference.",
                    static_cast< css::frame::XFrame* >(this));

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    SolarMutexResettableGuard aWriteLock;

    if ( m_xContainerWindow.is() )
        throw css::uno::RuntimeException(
                "Frame::initialized() is called more than once, which is not useful nor allowed.",
                static_cast< css::frame::XFrame* >(this));

    // Look for rejected calls first!
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    // Enable object for real working ... so follow impl methods don't must handle it special! (e.g. E_SOFTEXCEPTIONS for rejected calls)
    m_aTransactionManager.setWorkingMode( E_WORK );

    // This must be the first call of this method!
    // We should initialize our object and open it for working.
    // Set the new window.
    SAL_WARN_IF( m_xContainerWindow.is(), "fwk", "Frame::initialize(): Leak detected! This state should never occur ..." );
    m_xContainerWindow = xWindow;

    // if window is initially visible, we will never get a windowShowing event
    vcl::Window* pWindow = VCLUnoHelper::GetWindow(xWindow);
    if (pWindow && pWindow->IsVisible())
        m_bIsHidden = false;

    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;
    css::uno::Reference< css::frame::XLayoutManager2 >  xLayoutManager = m_xLayoutManager;

    // Release lock ... because we call some impl methods, which are threadsafe by himself.
    // If we hold this lock - we will produce our own deadlock!
    aWriteLock.clear();
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */

    if (xLayoutManager.is())
        lcl_enableLayoutManager(xLayoutManager, this);

    // create progress helper
    css::uno::Reference< css::frame::XFrame >                 xThis            (static_cast< css::frame::XFrame* >(this)                        , css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::task::XStatusIndicatorFactory > xIndicatorFactory =
        css::task::StatusIndicatorFactory::createWithFrame(xContext, xThis, sal_False/*DisableReschedule*/, sal_True/*AllowParentShow*/ );

    // SAFE -> ----------------------------------
    aWriteLock.reset();
    m_xIndicatorFactoryHelper = xIndicatorFactory;
    aWriteLock.clear();
    // <- SAFE ----------------------------------

    // Start listening for events after setting it on helper class ...
    // So superflous messages are filtered to NULL :-)
    implts_startWindowListening();

    m_pWindowCommandDispatch = new WindowCommandDispatch(xContext, this);

    // Initialize title functionality
    TitleHelper* pTitleHelper = new TitleHelper( xContext );
    m_xTitleHelper.set(static_cast< ::cppu::OWeakObject* >(pTitleHelper), css::uno::UNO_QUERY_THROW);
    pTitleHelper->setOwner(xThis);
}

/*-****************************************************************************************************
    @short      returns current set container window
    @descr      The ContainerWindow property is used as a container for the component
                in this frame. So this object implements a container interface too.
                The instantiation of the container window is done by the user of this class.
                The frame is the owner of its container window.

    @seealso    method initialize()
    @return     A reference to current set containerwindow.

    @onerror    A null reference is returned.
*//*-*****************************************************************************************************/
css::uno::Reference< css::awt::XWindow > SAL_CALL Frame::getContainerWindow() throw( css::uno::RuntimeException, std::exception )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    SolarMutexGuard g;
    return m_xContainerWindow;
}

/*-****************************************************************************************************
    @short      set parent frame
    @descr      We need a parent to support some functionality! e.g. findFrame()
                By the way we use the chance to set an internal information about our top state.
                So we must not check this information during every isTop() call.
                We are top, if our parent is the desktop instance or we havent any parent.

    @seealso    getCreator()
    @seealso    findFrame()
    @seealso    isTop()
    @seealos    m_bIsFrameTop

    @param      xCreator
                    valid reference to our new owner frame, which should implement a supplier interface

    @threadsafe yes
*//*-*****************************************************************************************************/
void SAL_CALL Frame::setCreator( const css::uno::Reference< css::frame::XFramesSupplier >& xCreator ) throw( css::uno::RuntimeException, std::exception )
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE { */
        SolarMutexClearableGuard aWriteLock;
            m_xParent = xCreator;
        aWriteLock.clear();
    /* } SAFE */

    css::uno::Reference< css::frame::XDesktop > xIsDesktop( xCreator, css::uno::UNO_QUERY );
    m_bIsFrameTop = ( xIsDesktop.is() || ! xCreator.is() );
}

/*-****************************************************************************************************
    @short      returns current parent frame
    @descr      The Creator is the parent frame container. If it is NULL, the frame is the uppermost one.

    @seealso    method setCreator()
    @return     A reference to current set parent frame container.

    @onerror    A null reference is returned.
*//*-*****************************************************************************************************/
css::uno::Reference< css::frame::XFramesSupplier > SAL_CALL Frame::getCreator() throw( css::uno::RuntimeException, std::exception )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    SolarMutexGuard g;
    return m_xParent;
}

/*-****************************************************************************************************
    @short      returns current set name of frame
    @descr      This name is used to find target of findFrame() or queryDispatch() calls.

    @seealso    method setName()
    @return     Current set name of frame.

    @onerror    An empty string is returned.
*//*-*****************************************************************************************************/
OUString SAL_CALL Frame::getName() throw( css::uno::RuntimeException, std::exception )
{
    SolarMutexGuard g;
    return m_sName;
}

/*-****************************************************************************************************
    @short      set new name for frame
    @descr      This name is used to find target of findFrame() or queryDispatch() calls.

    @attention  Special names like "_blank", "_self" aren't allowed...
                "_beamer" or "_menubar" excepts this rule!

    @seealso    method getName()

    @param      "sName", new frame name.
    @onerror    We do nothing.
*//*-*****************************************************************************************************/
void SAL_CALL Frame::setName( const OUString& sName ) throw( css::uno::RuntimeException, std::exception )
{
    SolarMutexGuard g;
    // Set new name... but look for invalid special target names!
    // They are not allowed to set.
    if (TargetHelper::isValidNameForFrame(sName))
        m_sName = sName;
}

/*-****************************************************************************************************
    @short      search for frames
    @descr      This method searches for a frame with the specified name.
                Frames may contain other frames (e.g. a frameset) and may
                be contained in other frames. This hierarchy is searched by
                this method.
                First some special names are taken into account, i.e. "",
                "_self", "_top", "_blank" etc. The nSearchFlags are ignored
                when comparing these names with sTargetFrameName, further steps are
                controlled by the search flags. If allowed, the name of the frame
                itself is compared with the desired one, then ( again if allowed )
                the method findFrame() is called for all children, for siblings
                and as last for the parent frame.
                If no frame with the given name is found until the top frames container,
                a new top one is created, if this is allowed by a special
                flag. The new frame also gets the desired name.

    @param      sTargetFrameName
                    special names (_blank, _self) or real name of target frame
    @param      nSearchFlags
                    optional flags which regulate search for non special target frames

    @return     A reference to found or may be new created frame.
    @threadsafe yes
*//*-*****************************************************************************************************/
css::uno::Reference< css::frame::XFrame > SAL_CALL Frame::findFrame( const OUString&  sTargetFrameName,
                                                                           sal_Int32         nSearchFlags    ) throw( css::uno::RuntimeException, std::exception )
{
    css::uno::Reference< css::frame::XFrame > xTarget;

    // 0) Ignore wrong parameter!
    //    We don't support search for following special targets.
    //    If we reject this requests - we must not check for such names
    //    in following code again and again. If we do not so -wrong
    //    search results can occur!

    if (
        (sTargetFrameName==SPECIALTARGET_DEFAULT  )   ||    // valid for dispatches - not for findFrame()!
        (sTargetFrameName==SPECIALTARGET_MENUBAR  )         // valid for dispatches - not for findFrame()!
       )
    {
        return nullptr;
    }

    // I) check for special defined targets first which must be handled exclusive.
    //    force using of "if() else if() ..."

    // get threadsafe some necessary member which are necessary for following functionality
    /* SAFE { */
    SolarMutexResettableGuard aReadLock;
    css::uno::Reference< css::frame::XFrame >              xParent      ( m_xParent, css::uno::UNO_QUERY );
    css::uno::Reference< css::uno::XComponentContext >     xContext = m_xContext;
    bool                                               bIsTopFrame  = m_bIsFrameTop;
    bool                                               bIsTopWindow = WindowHelper::isTopWindow(m_xContainerWindow);
    aReadLock.clear();
    /* } SAFE */

    // I.I) "_blank"
    //  Not allowed for a normal frame - but for the desktop.
    //  Use helper class to do so. It use the desktop automatically.

    if ( sTargetFrameName==SPECIALTARGET_BLANK )
    {
        TaskCreator aCreator(xContext);
        xTarget = aCreator.createTask(sTargetFrameName,false);
    }

    // I.II) "_parent"
    //  It doesn't matter if we have a valid parent or not. User ask for him and get it.
    //  An empty result is a valid result too.

    else if ( sTargetFrameName==SPECIALTARGET_PARENT )
    {
        xTarget = xParent;
    }

    // I.III) "_top"
    //  If we are not the top frame in this hierarchy, we must forward request to our parent.
    //  Otherwhise we must return ourself.

    else if ( sTargetFrameName==SPECIALTARGET_TOP )
    {
        if (bIsTopFrame)
            xTarget = this;
        else if (xParent.is()) // If we are not top - the parent MUST exist. But may it's better to check it again .-)
            xTarget = xParent->findFrame(SPECIALTARGET_TOP,0);
    }

    // I.IV) "_self", ""
    //  This mean this frame in every case.

    else if (
        ( sTargetFrameName==SPECIALTARGET_SELF ) ||
        ( sTargetFrameName.isEmpty()           )
       )
    {
        xTarget = this;
    }

    // I.V) "_beamer"
    //  This is a special sub frame of any task. We must return it if we found it on our direct children
    //  or create it there if it not already exists.
    //  Note: Such beamer exists for task(top) frames only!

    else if ( sTargetFrameName==SPECIALTARGET_BEAMER )
    {
        // We are a task => search or create the beamer
        if (bIsTopWindow)
        {
            xTarget = m_aChildFrameContainer.searchOnDirectChildrens(SPECIALTARGET_BEAMER);
            if ( ! xTarget.is() )
            {
                /* TODO
                    Creation not supported yet!
                    Wait for new layout manager service because we can't plug it
                    inside already opened document of this frame ...
                */
            }
        }
        // We arent a task => forward request to our parent or ignore it.
        else if (xParent.is())
            xTarget = xParent->findFrame(SPECIALTARGET_BEAMER,0);
    }

    else
    {

        // II) otherwise use optional given search flags
        //  force using of combinations of such flags. means no "else" part of use if() statements.
        //  But we ust break further searches if target was already found.
        //  Order of using flags is fix: SELF - CHILDREN - SIBLINGS - PARENT
        //  TASK and CREATE are handled special.

        // get threadsafe some necessary member which are necessary for following functionality
        /* SAFE { */
        aReadLock.reset();
        OUString sOwnName = m_sName;
        aReadLock.clear();
        /* } SAFE */

        // II.I) SELF
        //  Check for right name. If it's the searched one return ourself - otherwise
        //  ignore this flag.

        if (
            (nSearchFlags &  css::frame::FrameSearchFlag::SELF)  &&
            (sOwnName     == sTargetFrameName                 )
           )
        {
            xTarget = this;
        }

        // II.II) CHILDREN
        //  Search on all children for the given target name.
        //  An empty name value can't occur here - because it must be already handled as "_self"
        //  before. Used helper function of container doesn't create any frame.
        //  It makes a deep search only.

        if (
            ( ! xTarget.is()                                     ) &&
            (nSearchFlags & css::frame::FrameSearchFlag::CHILDREN)
           )
        {
            xTarget = m_aChildFrameContainer.searchOnAllChildrens(sTargetFrameName);
        }

        // II.III) TASKS
        //  This is a special flag. It regulate search on this task tree only or allow search on
        //  all other ones (which are sibling trees of us) too.
        //  Upper search must stop at this frame if we are the topest one and the TASK flag is not set
        //  or we can ignore it if we have no valid parent.

        if (
            (   bIsTopFrame && (nSearchFlags & css::frame::FrameSearchFlag::TASKS) )   ||
            ( ! bIsTopFrame                                                        )
           )
        {

            // II.III.I) SIBLINGS
            //  Search on all our direct siblings - means all children of our parent.
            //  Use this flag in combination with TASK. We must suppress such upper search if
            //  user has not set it and if we are a top frame.
            //  Attention: Don't forward this request to our parent as a findFrame() call.
            //  In such case we must protect us against recursive calls.
            //  Use snapshot of our parent. But don't use queryFrames() of XFrames interface.
            //  Because it's return all siblings and all her children including our children too
            //  if we call it with the CHILDREN flag. We don't need that - we need the direct container
            //  items of our parent only to start searches there. So we must use the container interface
            //  XIndexAccess instead of XFrames.

            if (
                ( ! xTarget.is()                                      ) &&
                (nSearchFlags &  css::frame::FrameSearchFlag::SIBLINGS) &&
                (   xParent.is()                                      ) // search on siblings is impossible without a parent
               )
            {
                css::uno::Reference< css::frame::XFramesSupplier > xSupplier( xParent, css::uno::UNO_QUERY );
                if (xSupplier.is())
                {
                    css::uno::Reference< css::container::XIndexAccess > xContainer( xSupplier->getFrames(), css::uno::UNO_QUERY );
                    if (xContainer.is())
                    {
                        sal_Int32 nCount = xContainer->getCount();
                        for( sal_Int32 i=0; i<nCount; ++i )
                        {
                            css::uno::Reference< css::frame::XFrame > xSibling;
                            if (
                                ( !(xContainer->getByIndex(i)>>=xSibling)                                 ) ||  // control unpacking
                                ( ! xSibling.is()                                     ) ||  // check for valid items
                                ( xSibling==static_cast< ::cppu::OWeakObject* >(this) )     // ignore ourself! (We are a part of this container too - but search on our children was already done.)
                            )
                            {
                                continue;
                            }

                            // Don't allow upper search here! Use right flags to regulate it.
                            // And allow deep search on children only - if it was allowed for us too.
                            sal_Int32 nRightFlags = css::frame::FrameSearchFlag::SELF;
                            if (nSearchFlags & css::frame::FrameSearchFlag::CHILDREN)
                                nRightFlags |= css::frame::FrameSearchFlag::CHILDREN;
                            xTarget = xSibling->findFrame(sTargetFrameName, nRightFlags );
                            // perform search be breaking further search if a result exist.
                            if (xTarget.is())
                                break;
                        }
                    }
                }
            }

            // II.III.II) PARENT
            //  Forward search to our parent (if he exists.)
            //  To prevent us against recursive and superflous calls (which can occur if we allow him
            //  to search on his children too) we must change used search flags.

            if (
                ( ! xTarget.is()                                    ) &&
                (nSearchFlags &  css::frame::FrameSearchFlag::PARENT) &&
                (   xParent.is()                                    )
               )
            {
                if (xParent->getName() == sTargetFrameName)
                    xTarget = xParent;
                else
                {
                    sal_Int32 nRightFlags  = nSearchFlags;
                              nRightFlags &= ~css::frame::FrameSearchFlag::CHILDREN;
                    xTarget = xParent->findFrame(sTargetFrameName, nRightFlags);
                }
            }
        }

        // II.IV) CREATE
        //  If we haven't found any valid target frame by using normal flags - but user allowed us to create
        //  a new one ... we should do that. Used TaskCreator use Desktop instance automatically as parent!

        if (
            ( ! xTarget.is()                                   )    &&
            (nSearchFlags & css::frame::FrameSearchFlag::CREATE)
           )
        {
            TaskCreator aCreator(xContext);
            xTarget = aCreator.createTask(sTargetFrameName,false);
        }
    }

    return xTarget;
}

/*-****************************************************************************************************
    @descr      Returns sal_True, if this frame is a "top frame", otherwise sal_False.
                The "m_bIsFrameTop" member must be set in the ctor or setCreator() method.
                A top frame is a member of the top frame container or a member of the
                task frame container. Both containers can create new frames if the findFrame()
                method of their css::frame::XFrame interface is called with a frame name not yet known.

    @seealso    ctor
    @seealso    method setCreator()
    @seealso    method findFrame()
    @return     true, if is it a top frame ... false otherwise.

    @onerror    No error should occur!
*//*-*****************************************************************************************************/
sal_Bool SAL_CALL Frame::isTop() throw( css::uno::RuntimeException, std::exception )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    SolarMutexGuard g;
    // This information is set in setCreator().
    // We are top, if our parent is a task or the desktop or if no parent exist!
    return m_bIsFrameTop;
}

/*-****************************************************************************************************
    @short      activate frame in hierarchy
    @descr      This feature is used to mark active paths in our frame hierarchy.
                You can be a listener for this event to react for it ... change some internal states or something else.

    @seealso    method deactivate()
    @seealso    method isActivate()
    @seealso    enum EActiveState
    @seealso    listener mechanism
*//*-*****************************************************************************************************/
void SAL_CALL Frame::activate() throw( css::uno::RuntimeException, std::exception )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    SolarMutexResettableGuard aWriteLock;

    // Copy necessary member and free the lock.
    // It's not necessary for m_aChildFrameContainer ... because
    // he is threadsafe himself and live if we live.
    // We use a registered transaction to prevent us against
    // breaks during this operation!
    css::uno::Reference< css::frame::XFrame >           xActiveChild    = m_aChildFrameContainer.getActive();
    css::uno::Reference< css::frame::XFramesSupplier >  xParent         ( m_xParent, css::uno::UNO_QUERY );
    css::uno::Reference< css::frame::XFrame >           xThis           ( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );
    css::uno::Reference< css::awt::XWindow >            xComponentWindow( m_xComponentWindow, css::uno::UNO_QUERY );
    EActiveState                                        eState          = m_eActiveState;

    aWriteLock.clear();
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */

    //  1)  If I'am not active before ...
    if( eState == E_INACTIVE )
    {
        // ... do it then.
        aWriteLock.reset();
        eState         = E_ACTIVE;
        m_eActiveState = eState;
        aWriteLock.clear();
        // Deactivate sibling path and forward activation to parent ... if any parent exist!
        if( xParent.is() )
        {
            // Every time set THIS frame as active child of parent and activate it.
            // We MUST have a valid path from bottom to top as active path!
            // But we must deactivate the old active sibling path first.

            // Attention: Deactivation of an active path, deactivate the whole path ... from bottom to top!
            // But we wish to deactivate founded sibling-tree only.
            // [ see deactivate() / step 4) for further information! ]

            xParent->setActiveFrame( xThis );

            // Then we can activate from here to top.
            // Attention: We are ACTIVE now. And the parent will call activate() at us!
            // But we do nothing then! We are already activated.
            xParent->activate();
        }
        // Its necessary to send event NOW - not before.
        // Activation goes from bottom to top!
        // Thats the reason to activate parent first and send event now.
        implts_sendFrameActionEvent( css::frame::FrameAction_FRAME_ACTIVATED );
    }

    //  2)  I was active before or current activated and there is a path from here to bottom, who CAN be active.
    //      But our direct child of path is not active yet.
    //      (It can be, if activation occur in the middle of a current path!)
    //      In these case we activate path to bottom to set focus on right frame!
    if ( eState == E_ACTIVE && xActiveChild.is() && !xActiveChild->isActive() )
    {
        xActiveChild->activate();
    }

    //  3)  I was active before or current activated. But if I have no active child => I will get the focus!
    if ( eState == E_ACTIVE && !xActiveChild.is() )
    {
        aWriteLock.reset();
        eState         = E_FOCUS;
        m_eActiveState = eState;
        aWriteLock.clear();
        implts_sendFrameActionEvent( css::frame::FrameAction_FRAME_UI_ACTIVATED );
    }
}

/*-****************************************************************************************************
    @short      deactivate frame in hierarchy
    @descr      This feature is used to deactive paths in our frame hierarchy.
                You can be a listener for this event to react for it ... change some internal states or something else.

    @seealso    method activate()
    @seealso    method isActivate()
    @seealso    enum EActiveState
    @seealso    listener mechanism
*//*-*****************************************************************************************************/
void SAL_CALL Frame::deactivate() throw( css::uno::RuntimeException, std::exception )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    SolarMutexResettableGuard aWriteLock;

    // Copy necessary member and free the lock.
    css::uno::Reference< css::frame::XFrame >           xActiveChild    = m_aChildFrameContainer.getActive();
    css::uno::Reference< css::frame::XFramesSupplier >  xParent         ( m_xParent, css::uno::UNO_QUERY );
    css::uno::Reference< css::frame::XFrame >           xThis           ( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );
    EActiveState                                        eState          = m_eActiveState;

    aWriteLock.clear();
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */

    // Work only, if there something to do!
    if( eState != E_INACTIVE )
    {

        //  1)  Deactivate all active children.
        if ( xActiveChild.is() && xActiveChild->isActive() )
        {
            xActiveChild->deactivate();
        }

        //  2)  If I have the focus - I will lost it now.
        if( eState == E_FOCUS )
        {
            // Set new state INACTIVE(!) and send message to all listener.
            // Don't set ACTIVE as new state. This frame is deactivated for next time - due to activate().
            aWriteLock.reset();
            eState          = E_ACTIVE;
            m_eActiveState  = eState;
            aWriteLock.clear();
            implts_sendFrameActionEvent( css::frame::FrameAction_FRAME_UI_DEACTIVATING );
        }

        //  3)  If I'am active - I will be deactivated now.
        if( eState == E_ACTIVE )
        {
            // Set new state and send message to all listener.
            aWriteLock.reset();
            eState          = E_INACTIVE;
            m_eActiveState  = eState;
            aWriteLock.clear();
            implts_sendFrameActionEvent( css::frame::FrameAction_FRAME_DEACTIVATING );
        }

        //  4)  If there is a path from here to my parent ...
        //      ... I'am on the top or in the middle of deactivated subtree and action was started here.
        //      I must deactivate all frames from here to top, which are members of current path.
        //      Stop, if THESE frame not the active frame of our parent!
        if ( xParent.is() && xParent->getActiveFrame() == xThis )
        {
            // We MUST break the path - otherwise we will get the focus - not our parent! ...
            // Attention: Our parent don't call us again - WE ARE NOT ACTIVE YET!
            // [ see step 3 and condition "if ( m_eActiveState!=INACTIVE ) ..." in this method! ]
            xParent->deactivate();
        }
    }
}

/*-****************************************************************************************************
    @short      returns active state
    @descr      Call it to get information about current active state of this frame.

    @seealso    method activate()
    @seealso    method deactivate()
    @seealso    enum EActiveState
    @return     true if active, false otherwise.

    @onerror    No error should occur.
*//*-*****************************************************************************************************/
sal_Bool SAL_CALL Frame::isActive() throw( css::uno::RuntimeException, std::exception )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    SolarMutexGuard g;
    return  (
                ( m_eActiveState    ==  E_ACTIVE    )   ||
                ( m_eActiveState    ==  E_FOCUS     )
            );
}

/*-****************************************************************************************************
    @short      ???
*//*-*****************************************************************************************************/
void SAL_CALL Frame::contextChanged() throw( css::uno::RuntimeException, std::exception )
{
    // Look for rejected calls!
    // Sometimes called during closing object... => soft exceptions
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );
    // Impl-method is threadsafe himself!
    // Send event to all listener for frame actions.
    implts_sendFrameActionEvent( css::frame::FrameAction_CONTEXT_CHANGED );
}

/*-****************************************************************************************************
    @short      set new component inside the frame
    @descr      A frame is a container for a component. Use this method to set, change or release it!
                We accept null references! The xComponentWindow will be a child of our container window
                and get all window events from us.

    @attention  (a) A current set component can disagree with the suspend() request!
                    We don't set the new one and return with false then.
                (b) It's possible to set:
                        (b1) a simple component here which supports the window only - no controller;
                        (b2) a full featured component which supports window and controller;
                        (b3) or both to NULL if outside code which to forget this component.

    @seealso    method getComponentWindow()
    @seealso    method getController()

    @param      xComponentWindow
                    valid reference to new component window which will be a child of internal container window
                    May <NULL/> for releasing.
    @param      xController
                    reference to new component controller
                    (may <NULL/> for relasing or setting of a simple component)

    @return     <TRUE/> if operation was successful, <FALSE/> otherwise.

    @onerror    We return <FALSE/>.
    @threadsafe yes
*//*-*****************************************************************************************************/
sal_Bool SAL_CALL Frame::setComponent(  const   css::uno::Reference< css::awt::XWindow >&       xComponentWindow ,
                                        const   css::uno::Reference< css::frame::XController >& xController      ) throw( css::uno::RuntimeException, std::exception )
{

    // Ignore this HACK of sfx2!
    // He call us with an valid controller without a valid window ... Thats not allowed!
    if  ( xController.is() && ! xComponentWindow.is() )
        return sal_True;

    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // Get threadsafe some copies of used members.
    /* SAFE { */
    SolarMutexClearableGuard aReadLock;
    css::uno::Reference< css::awt::XWindow >       xContainerWindow    = m_xContainerWindow;
    css::uno::Reference< css::awt::XWindow >       xOldComponentWindow = m_xComponentWindow;
    css::uno::Reference< css::frame::XController > xOldController      = m_xController;
    vcl::Window*                                        pOwnWindow = VCLUnoHelper::GetWindow( xContainerWindow );
    bool                                       bHadFocus           = pOwnWindow->HasChildPathFocus();
    bool                                       bWasConnected       = m_bConnected;
    aReadLock.clear();
    /* } SAFE */

    // stop listening on old window
    // May it produce some trouble.
    // But don't forget to listen on new window again ... or reactivate listening
    // if we reject this setComponent() request and leave this method without changing the old window.
    implts_stopWindowListening();

    // Notify all listener, that this component (if current one exist) will be unloaded.
    if (bWasConnected)
        implts_sendFrameActionEvent( css::frame::FrameAction_COMPONENT_DETACHING );

    // otherwise release old component first
    // Always release controller before releasing window,
    // because controller may want to access its window!
    // But check for real changes - may the new controller is the old one.
    if (
        (xOldController.is()          )   &&
        (xOldController != xController)
       )
    {
        /* ATTENTION
            Don't suspend the old controller here. Because the outside caller must do that
            by definition. We have to dispose it here only.
         */

        // Before we dispose this controller we should hide it inside this frame instance.
        // We hold it alive for next calls by using xOldController!
        /* SAFE {*/
        SolarMutexClearableGuard aWriteLock;
        m_xController = nullptr;
        aWriteLock.clear();
        /* } SAFE */

        css::uno::Reference< css::lang::XComponent > xDisposable( xOldController, css::uno::UNO_QUERY );
        if (xDisposable.is())
        {
            try
            {
                xDisposable->dispose();
            }
            catch(const css::lang::DisposedException&)
                {}
        }
        xOldController = nullptr;
    }

    // Now it's time to release the component window.
    // If controller wasn't released successfully - this code line shouldn't be reached.
    // Because in case of "suspend()==false" we return immediately with false ...
    // see before
    // Check for real changes too.
    if (
        (xOldComponentWindow.is()               )   &&
        (xOldComponentWindow != xComponentWindow)
       )
    {
        /* SAFE { */
        SolarMutexClearableGuard aWriteLock;
        m_xComponentWindow = nullptr;
        aWriteLock.clear();
        /* } SAFE */

        css::uno::Reference< css::lang::XComponent > xDisposable( xOldComponentWindow, css::uno::UNO_QUERY );
        if (xDisposable.is())
        {
            try
            {
                xDisposable->dispose();
            }
            catch(const css::lang::DisposedException&)
            {
            }
        }
        xOldComponentWindow = nullptr;
    }

    // Now it's time to set the new component ...
    // By the way - find out our new "load state" - means if we have a valid component inside.
    /* SAFE { */
    SolarMutexResettableGuard aWriteLock;
    m_xComponentWindow = xComponentWindow;
    m_xController      = xController;
    m_bConnected       = (m_xComponentWindow.is() || m_xController.is());
    bool bIsConnected       = m_bConnected;
    aWriteLock.clear();
    /* } SAFE */

    // notifies all interest listener, that current component was changed or a new one was loaded
    if (bIsConnected && bWasConnected)
        implts_sendFrameActionEvent( css::frame::FrameAction_COMPONENT_REATTACHED );
    else if (bIsConnected && !bWasConnected)
        implts_sendFrameActionEvent( css::frame::FrameAction_COMPONENT_ATTACHED   );

    // A new component window doesn't know anything about current active/focus states.
    // Set this information on it!
    if (
        (bHadFocus            ) &&
        (xComponentWindow.is())
       )
    {
        xComponentWindow->setFocus();
    }

    // If it was a new component window - we must resize it to fill out
    // our container window.
    implts_resizeComponentWindow();
    // New component should change our current icon ...
    implts_setIconOnWindow();
    // OK - start listening on new window again - or do nothing if it is an empty one.
    implts_startWindowListening();

    /* SAFE { */
    aWriteLock.reset();
    impl_checkMenuCloser();
    aWriteLock.clear();
    /* } SAFE */

    return sal_True;
}

/*-****************************************************************************************************
    @short      returns current set component window
    @descr      Frames are used to display components. The actual displayed component is
                held by the m_xComponentWindow property. If the component implements only a
                XComponent interface, the communication between the frame and the
                component is very restricted. Better integration is achievable through a
                XController interface.
                If the component wants other objects to be able to get information about its
                ResourceDescriptor it has to implement a XModel interface.
                This frame is the owner of the component window.

    @seealso    method setComponent()
    @return     css::uno::Reference to current set component window.

    @onerror    A null reference is returned.
*//*-*****************************************************************************************************/
css::uno::Reference< css::awt::XWindow > SAL_CALL Frame::getComponentWindow() throw( css::uno::RuntimeException, std::exception )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    SolarMutexGuard g;
    return m_xComponentWindow;
}

/*-****************************************************************************************************
    @short      returns current set controller
    @descr      Frames are used to display components. The actual displayed component is
                held by the m_xComponentWindow property. If the component implements only a
                XComponent interface, the communication between the frame and the
                component is very restricted. Better integration is achievable through a
                XController interface.
                If the component wants other objects to be able to get information about its
                ResourceDescriptor it has to implement a XModel interface.
                This frame is the owner of the component window.

    @seealso    method setComponent()
    @return     css::uno::Reference to current set controller.

    @onerror    A null reference is returned.
*//*-*****************************************************************************************************/
css::uno::Reference< css::frame::XController > SAL_CALL Frame::getController() throw( css::uno::RuntimeException, std::exception )
{
    SolarMutexGuard g;
    return m_xController;
}

/*-****************************************************************************************************
    @short      add/remove listener for activate/deactivate/contextChanged events
    @seealso    method activate()
    @seealso    method deactivate()
    @seealso    method contextChanged()

    @param      "xListener" reference to your listener object
    @onerror    Listener is ignored.
*//*-*****************************************************************************************************/
void SAL_CALL Frame::addFrameActionListener( const css::uno::Reference< css::frame::XFrameActionListener >& xListener ) throw( css::uno::RuntimeException, std::exception )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Check incoming parameter.
    SAL_WARN_IF( implcp_addFrameActionListener( xListener ), "fwk", "Frame::addFrameActionListener(): Invalid parameter detected." );
    // Listener container is threadsafe by himself ... but we must look for rejected calls!
    // Our OMenuDispatch-helper (is a member of ODispatchProvider!) is create at startup of this frame BEFORE initialize!
    // => soft exceptions!
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    m_aListenerContainer.addInterface( cppu::UnoType<css::frame::XFrameActionListener>::get(), xListener );
}

void SAL_CALL Frame::removeFrameActionListener( const css::uno::Reference< css::frame::XFrameActionListener >& xListener ) throw( css::uno::RuntimeException, std::exception )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Check incoming parameter.
    SAL_WARN_IF( implcp_removeFrameActionListener( xListener ), "fwk", "Frame::removeFrameActionListener(): Invalid parameter detected." );
    // Listener container is threadsafe by himself ... but we must look for rejected calls after disposing!
    // But we must work with E_SOFTEXCEPTIONS ... because sometimes we are called from our listeners
    // during dispose! Our work mode is E_BEFORECLOSE then ... and E_HARDEXCEPTIONS whould throw a DisposedException.
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    m_aListenerContainer.removeInterface( cppu::UnoType<css::frame::XFrameActionListener>::get(), xListener );
}

/*-****************************************************************************************************
    @short      support two way mechanism to release a frame
    @descr      This method ask internal component (controller) if he accept this close request.
                In case of <TRUE/> nothing will be happen (from point of caller of this close method).
                In case of <FALSE/> a CloseVetoException is thrown. After such exception given parameter
                <var>bDeliverOwnership</var> regulate which will be the new owner of this instance.

    @attention  It's the replacement for XTask::close() which is marked as obsolete method.

    @param      bDeliverOwnership
                    If parameter is set to <FALSE/> the original caller will be the owner after thrown
                    veto exception and must try to close this frame at later time again. Otherwhise the
                    source of throwed exception is the right one. May it will be the frame himself.

    @throws     CloseVetoException
                    if any internal things willn't be closed

    @threadsafe yes
*//*-*****************************************************************************************************/
void SAL_CALL Frame::close( sal_Bool bDeliverOwnership ) throw( css::util::CloseVetoException,
                                                                css::uno::RuntimeException, std::exception   )
{
    {
        TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

        // At the end of this method may we must dispose ourself...
        // and may nobody from outside hold a reference to us...
        // then it's a good idea to do that by ourself.
        css::uno::Reference< css::uno::XInterface > xSelfHold( static_cast< ::cppu::OWeakObject* >(this) );

        // Check any close listener before we look for currently running internal processes.
        // Because if a listener disagree with this close() request - we have time to finish this
        // internal operations too...
        // Note: container is threadsafe himself.
        css::lang::EventObject             aSource    (static_cast< ::cppu::OWeakObject*>(this));
        ::cppu::OInterfaceContainerHelper* pContainer = m_aListenerContainer.getContainer( cppu::UnoType<css::util::XCloseListener>::get());
        if (pContainer!=nullptr)
        {
            ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
            while (pIterator.hasMoreElements())
            {
                try
                {
                    static_cast<css::util::XCloseListener*>(pIterator.next())->queryClosing( aSource, bDeliverOwnership );
                }
                catch( const css::uno::RuntimeException& )
                {
                    pIterator.remove();
                }
            }
        }

        // Ok - no listener disagreed with this close() request
        // check if this frame is used for any load process currently
        if (isActionLocked())
        {
            if (bDeliverOwnership)
            {
                SolarMutexGuard g;
                m_bSelfClose = true;
            }

            throw css::util::CloseVetoException("Frame in use for loading document ...",static_cast< ::cppu::OWeakObject*>(this));
        }

        if ( ! setComponent(nullptr,nullptr) )
            throw css::util::CloseVetoException("Component couldn't be deattached ...",static_cast< ::cppu::OWeakObject*>(this));

        // If closing is allowed... inform all listeners and dispose this frame!
        pContainer = m_aListenerContainer.getContainer( cppu::UnoType<css::util::XCloseListener>::get());
        if (pContainer!=nullptr)
        {
            ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
            while (pIterator.hasMoreElements())
            {
                try
                {
                    static_cast<css::util::XCloseListener*>(pIterator.next())->notifyClosing( aSource );
                }
                catch( const css::uno::RuntimeException& )
                {
                    pIterator.remove();
                }
            }
        }

        /* SAFE { */
        SolarMutexClearableGuard aWriteLock;
        m_bIsHidden = true;
        aWriteLock.clear();
        /* } SAFE */
        impl_checkMenuCloser();

        // Attention: We must release our own registered transaction here. Otherwhise following dispose() call
        // wait for us too ....
    }
    dispose();
}

/*-****************************************************************************************************
    @short      be a listener for close events!
    @descr      Adds/remove a CloseListener at this frame instance. If the close() method is called on
                this object, the such listener are informed and can disagree with that by throwing
                a CloseVetoException.

    @seealso    Frame::close()

    @param      xListener
                    reference to your listener object

    @onerror    Listener is ignored.

    @threadsafe yes
*//*-*****************************************************************************************************/
void SAL_CALL Frame::addCloseListener( const css::uno::Reference< css::util::XCloseListener >& xListener ) throw (css::uno::RuntimeException, std::exception)
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );
    // We don't need any lock here ...
    // Container lives if we live and is threadsafe by himself.
    m_aListenerContainer.addInterface( cppu::UnoType<css::util::XCloseListener>::get(), xListener );
}

void SAL_CALL Frame::removeCloseListener( const css::uno::Reference< css::util::XCloseListener >& xListener ) throw (css::uno::RuntimeException, std::exception)
{
    // Use soft exception mode - mostly this method is called during disposing of this frame...
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );
    // We don't need any lock here...
    // Container lives if we live and is threadsafe by himself.
    m_aListenerContainer.removeInterface( cppu::UnoType<css::util::XCloseListener>::get(), xListener );
}

OUString SAL_CALL Frame::getTitle()
    throw (css::uno::RuntimeException, std::exception)
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // SAFE ->
    SolarMutexClearableGuard aReadLock;
    css::uno::Reference< css::frame::XTitle > xTitle(m_xTitleHelper, css::uno::UNO_QUERY_THROW);
    aReadLock.clear();
    // <- SAFE

    return xTitle->getTitle();
}

void SAL_CALL Frame::setTitle( const OUString& sTitle )
    throw (css::uno::RuntimeException, std::exception)
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // SAFE ->
    SolarMutexClearableGuard aReadLock;
    css::uno::Reference< css::frame::XTitle > xTitle(m_xTitleHelper, css::uno::UNO_QUERY_THROW);
    aReadLock.clear();
    // <- SAFE

    xTitle->setTitle(sTitle);
}

void SAL_CALL Frame::addTitleChangeListener( const css::uno::Reference< css::frame::XTitleChangeListener >& xListener)
    throw (css::uno::RuntimeException, std::exception)
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // SAFE ->
    SolarMutexClearableGuard aReadLock;
    css::uno::Reference< css::frame::XTitleChangeBroadcaster > xTitle(m_xTitleHelper, css::uno::UNO_QUERY_THROW);
    aReadLock.clear();
    // <- SAFE

    xTitle->addTitleChangeListener(xListener);
}

void SAL_CALL Frame::removeTitleChangeListener( const css::uno::Reference< css::frame::XTitleChangeListener >& xListener )
    throw (css::uno::RuntimeException, std::exception)
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // SAFE ->
    SolarMutexClearableGuard aReadLock;
    css::uno::Reference< css::frame::XTitleChangeBroadcaster > xTitle(m_xTitleHelper, css::uno::UNO_QUERY_THROW);
    aReadLock.clear();
    // <- SAFE

    xTitle->removeTitleChangeListener(xListener);
}

css::uno::Reference<css::container::XNameContainer> SAL_CALL Frame::getUserDefinedAttributes() throw (css::uno::RuntimeException, std::exception)
{
    // optional attribute
    return nullptr;
}

css::uno::Reference<css::frame::XDispatchRecorderSupplier> SAL_CALL Frame::getDispatchRecorderSupplier() throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;
    return m_xDispatchRecorderSupplier;
}

void SAL_CALL Frame::setDispatchRecorderSupplier(const css::uno::Reference<css::frame::XDispatchRecorderSupplier>& p) throw (css::uno::RuntimeException, std::exception)
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    SolarMutexGuard g;
    m_xDispatchRecorderSupplier.set(p);
}

css::uno::Reference<css::uno::XInterface> SAL_CALL Frame::getLayoutManager() throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;
    return m_xLayoutManager;
}

void SAL_CALL Frame::setLayoutManager(const css::uno::Reference<css::uno::XInterface>& p1) throw (css::uno::RuntimeException, std::exception)
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    SolarMutexGuard g;
    m_xLayoutManager.set(p1, css::uno::UNO_QUERY);
}

/*-****************************************************************************************************/
void Frame::implts_forgetSubFrames()
{
    // SAFE ->
    SolarMutexClearableGuard aReadLock;
    css::uno::Reference< css::container::XIndexAccess > xContainer(m_xFramesHelper, css::uno::UNO_QUERY_THROW);
    aReadLock.clear();
    // <- SAFE

    sal_Int32 c = xContainer->getCount();
    sal_Int32 i = 0;

    for (i=0; i<c; ++i)
    {
        try
        {
            css::uno::Reference< css::frame::XFrame > xFrame;
            xContainer->getByIndex(i) >>= xFrame;
            if (xFrame.is())
                xFrame->setCreator(css::uno::Reference< css::frame::XFramesSupplier >());
        }
        catch(const css::uno::Exception&)
        {
            // Ignore errors here.
            // Nobody can guarantee a stable index in multi threaded environments .-)
        }
    }

    SolarMutexGuard g;
    m_xFramesHelper.clear(); // clear uno reference
    m_aChildFrameContainer.clear(); // clear container content
}

/*-****************************************************************************************************
    @short      destroy instance
    @descr      The owner of this object calles the dispose method if the object
                should be destroyed. All other objects and components, that are registered
                as an EventListener are forced to release their references to this object.
                Furthermore this frame is removed from its parent frame container to release
                this reference. The reference attributes are disposed and released also.

    @attention  Look for globale description at beginning of file too!
                (DisposedException, FairRWLock ..., initialize, dispose)

    @seealso    method initialize()
    @seealso    baseclass FairRWLockBase!
*//*-*****************************************************************************************************/
void SAL_CALL Frame::dispose() throw( css::uno::RuntimeException, std::exception )
{
    // We should hold a reference to ourself ...
    // because our owner dispose us and release our reference ...
    // May be we will die before we could finish this method ...
    css::uno::Reference< css::frame::XFrame > xThis( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );

    SAL_INFO("fwk.frame", "[Frame] " << m_sName << " send dispose event to listener");

    // First operation should be ... "stopp all listening for window events on our container window".
    // These events are superflous but can make trouble!
    // We will die, die and die ...
    implts_stopWindowListening();

    if (m_xLayoutManager.is())
        lcl_disableLayoutManager(m_xLayoutManager, this);

    delete m_pWindowCommandDispatch;

    // Send message to all listener and forget her references.
    css::lang::EventObject aEvent( xThis );
    m_aListenerContainer.disposeAndClear( aEvent );

    // set "end of live" for our property set helper
    impl_disablePropertySet();

    // interception/dispatch chain must be destructed explicitly
    // Otherwhise some dispatches and/or interception objects wont die.
    css::uno::Reference< css::lang::XEventListener > xDispatchHelper(m_xDispatchHelper, css::uno::UNO_QUERY_THROW);
    xDispatchHelper->disposing(aEvent);
    xDispatchHelper.clear();

    // Disable this instance for further work.
    // This will wait for all current running ones ...
    // and reject all further requests!
    m_aTransactionManager.setWorkingMode( E_BEFORECLOSE );

    // Don't show any dialogs, errors or something else any more!
    // If somewhere called dispose() whitout close() before - normally no dialogs
    // should exist. Otherwhise it's the problem of the outside caller.
    // Note:
    //      (a) Do it after stopWindowListening(). May that force some active/deactive
    //          notifications which we don't need here really.
    //      (b) Don't forget to save the old value of IsDialogCancelEnabled() to
    //          restore it afterwards (to not kill headless mode).
    Application::DialogCancelMode old = Application::GetDialogCancelMode();
    Application::SetDialogCancelMode( Application::DIALOG_CANCEL_SILENT );

    // We should be alone for ever and further dispose calls are rejected by lines before ...
    // I hope it :-)

    // Free references of our frame tree.
    // Force parent container to forget this frame too ...
    // ( It's contained in m_xParent and so no css::lang::XEventListener for m_xParent! )
    // It's important to do that before we free some other internal structures.
    // Because if our parent gets an activate and found us as last possible active frame
    // he try to deactivate us ... and we run into some trouble (DisposedExceptions!).
    if( m_xParent.is() )
    {
        m_xParent->getFrames()->remove( xThis );
        m_xParent.clear();
    }

    /* } SAFE */
    // Forget our internal component and her window first.
    // So we can release our container window later without problems.
    // Because this container window is the parent of the component window ...
    // Note: Dispose it hard - because suspending must be done inside close() call!
    // But try to dispose the controller first before you destroy the window.
    // Because the window is used by the controller too ...
    if (m_xController.is())
    {
        css::uno::Reference< css::lang::XComponent > xDisposable( m_xController, css::uno::UNO_QUERY );
        if (xDisposable.is())
            xDisposable->dispose();
    }

    if (m_xComponentWindow.is())
    {
        css::uno::Reference< css::lang::XComponent > xDisposable( m_xComponentWindow, css::uno::UNO_QUERY );
        if (xDisposable.is())
            xDisposable->dispose();
    }

    impl_checkMenuCloser();

    impl_disposeContainerWindow( m_xContainerWindow );

    /*ATTENTION
        Clear container after successful removing from parent container ...
        because our parent could be the desktop which stand in dispose too!
        If we have already cleared our own container we lost our child before this could be
        remove himself at this instance ...
        Release m_xFramesHelper after that ... it's the same problem between parent and child!
        "m_xParent->getFrames()->remove( xThis );" needs this helper ...
        Otherwise we get a null reference and could finish removing successfully.
        => You see: Order of calling operations is important!!!
     */
    implts_forgetSubFrames();

    // Release some other references.
    // This calls should be easy ... I hope it :-)
    m_xDispatchHelper.clear();
    m_xContext.clear();
    m_xDropTargetListener.clear();
    m_xDispatchRecorderSupplier.clear();
    m_xLayoutManager.clear();
    m_xIndicatorFactoryHelper.clear();

    // It's important to set default values here!
    // If may be later somewhere change the disposed-behaviour of this implementation
    // and doesn't throw any DisposedExceptions we must guarantee best matching default values ...
    m_eActiveState       = E_INACTIVE;
    m_sName.clear();
    m_bIsFrameTop        = false;
    m_bConnected         = false;
    m_nExternalLockCount = 0;
    m_bSelfClose         = false;
    m_bIsHidden          = true;

    // Disable this instance for further working really!
    m_aTransactionManager.setWorkingMode( E_CLOSE );

    // Don't forget it restore old value -
    // otherwise no dialogs can be shown anymore in other frames.
    Application::SetDialogCancelMode( old );
}

/*-****************************************************************************************************
    @short      Be a listener for dispose events!
    @descr      Adds/remove an EventListener to this object. If the dispose method is called on
                this object, the disposing method of the listener is called.
    @param      "xListener" reference to your listener object.
    @onerror    Listener is ignored.
*//*-*****************************************************************************************************/
void SAL_CALL Frame::addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw( css::uno::RuntimeException, std::exception )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Check incoming parameter.
    SAL_WARN_IF( implcp_addEventListener( xListener ), "fwk", "Frame::addEventListener(): Invalid parameter detected." );
    // Look for rejected calls only!
    // Container is threadsafe.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    m_aListenerContainer.addInterface( cppu::UnoType<css::lang::XEventListener>::get(), xListener );
}

void SAL_CALL Frame::removeEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw( css::uno::RuntimeException, std::exception )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Check incoming parameter.
    SAL_WARN_IF( implcp_removeEventListener( xListener ), "fwk", "Frame::removeEventListener(): Invalid parameter detected." );
    // Look for rejected calls only!
    // Container is threadsafe.
    // Use E_SOFTEXCEPTIONS to allow removing listeners during dispose call!
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    m_aListenerContainer.removeInterface( cppu::UnoType<css::lang::XEventListener>::get(), xListener );
}

/*-****************************************************************************************************
    @short      create new status indicator
    @descr      Use returned status indicator to show progresses and some text information.
                All created objects share the same dialog! Only the last one can show his information.

    @seealso    class StatusIndicatorFactory
    @seealso    class StatusIndicator
    @return     A reference to created object.

    @onerror    We return a null reference.
*//*-*****************************************************************************************************/
css::uno::Reference< css::task::XStatusIndicator > SAL_CALL Frame::createStatusIndicator() throw( css::uno::RuntimeException, std::exception )
{
    /* UNSAFE AREA ----------------------------------------------------------------------------------------- */
    // Look for rejected calls!
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    SolarMutexClearableGuard aReadLock;

    // Make snapshot of necessary member and define default return value.
    css::uno::Reference< css::task::XStatusIndicator >        xExternal(m_xIndicatorInterception.get(), css::uno::UNO_QUERY);
    css::uno::Reference< css::task::XStatusIndicatorFactory > xFactory = m_xIndicatorFactoryHelper;

    aReadLock.clear();
    /* UNSAFE AREA ----------------------------------------------------------------------------------------- */

    // Was set from outside to intercept any progress activities!
    if (xExternal.is())
        return xExternal;

    // Or use our own factory as fallback, to create such progress.
    if (xFactory.is())
        return xFactory->createStatusIndicator();

    return css::uno::Reference< css::task::XStatusIndicator >();
}

/*-****************************************************************************************************
    @short      search for target to load URL
    @descr      This method searches for a dispatch for the specified DispatchDescriptor.
                The FrameSearchFlags and the FrameName of the DispatchDescriptor are
                treated as described for findFrame.

    @seealso    method findFrame()
    @seealso    method queryDispatches()
    @seealso    method set/getName()
    @seealso    class TargetFinder

    @param      "aURL"              , URL for loading
    @param      "sTargetFrameName"  , name of target frame
    @param      "nSearchFlags"      , additional flags to regulate search if sTargetFrameName is not clear
    @return     css::uno::Reference to dispatch handler.

    @onerror    A null reference is returned.
*//*-*****************************************************************************************************/
css::uno::Reference< css::frame::XDispatch > SAL_CALL Frame::queryDispatch( const css::util::URL&   aURL            ,
                                                                            const OUString&  sTargetFrameName,
                                                                                  sal_Int32         nSearchFlags    ) throw( css::uno::RuntimeException, std::exception )
{
    // Don't check incoming parameter here! Our helper do it for us and it is not a good idea to do it more than ones!
    // But look for rejected calls!
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // Remove uno and cmd protocol part as we want to support both of them. We store only the command part
    // in our hash map. All other protocols are stored with the protocol part.
    OUString aCommand( aURL.Main );
    if ( aURL.Protocol.equalsIgnoreAsciiCase(".uno:") )
        aCommand = aURL.Path;

    // Make std::unordered_map lookup if the current URL is in the disabled list
    if ( m_aCommandOptions.Lookup( SvtCommandOptions::CMDOPTION_DISABLED, aCommand ) )
        return css::uno::Reference< css::frame::XDispatch >();
    else
    {
        // We use a helper to support these interface and an interceptor mechanism.
        // Our helper is threadsafe by himself!
        return m_xDispatchHelper->queryDispatch( aURL, sTargetFrameName, nSearchFlags );
    }
}

/*-****************************************************************************************************
    @short      handle more than ones dispatches at same call
    @descr      Returns a sequence of dispatches. For details see the queryDispatch method.
                For failed dispatches we return empty items in list!

    @seealso    method queryDispatch()

    @param      "lDescriptor" list of dispatch arguments for queryDispatch()!
    @return     List of dispatch references. Some elements can be NULL!

    @onerror    An empty list is returned.
*//*-*****************************************************************************************************/
css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL Frame::queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptor ) throw( css::uno::RuntimeException, std::exception )
{
    // Don't check incoming parameter here! Our helper do it for us and it is not a good idea to do it more than ones!
    // But look for rejected calls!
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // We use a helper to support these interface and an interceptor mechanism.
    // Our helper is threadsafe by himself!
    return m_xDispatchHelper->queryDispatches( lDescriptor );
}

/*-****************************************************************************************************
    @short      register/unregister interceptor for dispatch calls
    @descr      If you wish to handle some dispatches by himself ... you should be
                an interceptor for it. Please see class OInterceptionHelper for further information.

    @seealso    class OInterceptionHelper

    @param      "xInterceptor", reference to your interceptor implementation.
    @onerror    Interceptor is ignored.
*//*-*****************************************************************************************************/
void SAL_CALL Frame::registerDispatchProviderInterceptor( const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor ) throw( css::uno::RuntimeException, std::exception )
{
    // We use a helper to support these interface and an interceptor mechanism.
    // This helper is threadsafe himself and check incoming parameter too.
    // I think we don't need any lock here!
    // But we must look for rejected calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    css::uno::Reference< css::frame::XDispatchProviderInterception > xInterceptionHelper( m_xDispatchHelper, css::uno::UNO_QUERY );
    xInterceptionHelper->registerDispatchProviderInterceptor( xInterceptor );
}

void SAL_CALL Frame::releaseDispatchProviderInterceptor( const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor ) throw( css::uno::RuntimeException, std::exception )
{
    // We use a helper to support these interface and an interceptor mechanism.
    // This helper is threadsafe himself and check incoming parameter too.
    // I think we don't need any lock here!
    // But we must look for rejected calls ...
    // Sometimes we are called during our dispose() method ... => soft exceptions!
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    css::uno::Reference< css::frame::XDispatchProviderInterception > xInterceptionHelper( m_xDispatchHelper, css::uno::UNO_QUERY );
    xInterceptionHelper->releaseDispatchProviderInterceptor( xInterceptor );
}

/*-****************************************************************************************************
    @short      provides information about all possible dispatch functions
                inside the current frame environment
*//*-*****************************************************************************************************/
css::uno::Sequence< sal_Int16 > SAL_CALL Frame::getSupportedCommandGroups()
    throw(css::uno::RuntimeException, std::exception)
{
    return m_xDispatchInfoHelper->getSupportedCommandGroups();
}

css::uno::Sequence< css::frame::DispatchInformation > SAL_CALL Frame::getConfigurableDispatchInformation(sal_Int16 nCommandGroup)
    throw(css::uno::RuntimeException, std::exception)
{
    return m_xDispatchInfoHelper->getConfigurableDispatchInformation(nCommandGroup);
}

/*-****************************************************************************************************
    @short      notifications for window events
    @descr      We are a listener on our container window to forward it to our component window.

    @seealso    method setComponent()
    @seealso    member m_xContainerWindow
    @seealso    member m_xComponentWindow

    @param      "aEvent" describe source of detected event
*//*-*****************************************************************************************************/
void SAL_CALL Frame::windowResized( const css::awt::WindowEvent& aEvent ) throw( css::uno::RuntimeException, std::exception )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Check incoming parameter.
    (void) aEvent;
    SAL_WARN_IF( implcp_windowResized( aEvent ), "fwk", "Frame::windowResized(): Invalid parameter detected." );
    // Look for rejected calls.
    // Part of dispose-mechanism => soft exceptions
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    // Impl-method is threadsafe!
    // If we have a current component window - we must resize it!
    implts_resizeComponentWindow();
}

void SAL_CALL Frame::focusGained( const css::awt::FocusEvent& aEvent ) throw( css::uno::RuntimeException, std::exception )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Check incoming parameter.
    (void) aEvent;
    SAL_WARN_IF( implcp_focusGained( aEvent ), "fwk", "Frame::focusGained(): Invalid parameter detected." );
    // Look for rejected calls.
    // Part of dispose() mechanism ... => soft exceptions!
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    SolarMutexClearableGuard aReadLock;
    // Make snapshot of member!
    css::uno::Reference< css::awt::XWindow > xComponentWindow = m_xComponentWindow;
    aReadLock.clear();
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */

    if( xComponentWindow.is() )
    {
        xComponentWindow->setFocus();
    }
}

/*-****************************************************************************************************
    @short      notifications for window events
    @descr      We are a listener on our container window to forward it to our component window ...
                but a XTopWindowListener we are only if we are a top frame!

    @seealso    method setComponent()
    @seealso    member m_xContainerWindow
    @seealso    member m_xComponentWindow

    @param      "aEvent" describe source of detected event
*//*-*****************************************************************************************************/
void SAL_CALL Frame::windowActivated( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException, std::exception )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Check incoming parameter.
    (void) aEvent;
    SAL_WARN_IF( implcp_windowActivated( aEvent ), "fwk", "Frame::windowActivated(): Invalid parameter detected." );
    // Look for rejected calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    SolarMutexClearableGuard aReadLock;
    // Make snapshot of member!
    EActiveState eState = m_eActiveState;
    aReadLock.clear();
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Activate the new active path from here to top.
    if( eState == E_INACTIVE )
    {
        setActiveFrame( css::uno::Reference< css::frame::XFrame >() );
        activate();
    }
}

void SAL_CALL Frame::windowDeactivated( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException, std::exception )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Check incoming parameter.
    (void) aEvent;
    SAL_WARN_IF( implcp_windowDeactivated( aEvent ), "fwk", "Frame::windowDeactivated(): Invalid parameter detected." );
    // Look for rejected calls.
    // Sometimes called during dispose() => soft exceptions
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    SolarMutexClearableGuard aReadLock;

    css::uno::Reference< css::frame::XFrame > xParent          ( m_xParent, css::uno::UNO_QUERY );
    css::uno::Reference< css::awt::XWindow >  xContainerWindow = m_xContainerWindow;
    EActiveState                              eActiveState     = m_eActiveState;

    aReadLock.clear();

    if( eActiveState != E_INACTIVE )
    {
        // Deactivation is always done implicitly by activation of another frame.
        // Only if no activation is done, deactivations have to be processed if the activated window
        // is a parent window of the last active Window!
        SolarMutexClearableGuard aSolarGuard;
        vcl::Window* pFocusWindow = Application::GetFocusWindow();
        if  ( xContainerWindow.is() && xParent.is() &&
              !css::uno::Reference< css::frame::XDesktop >( xParent, css::uno::UNO_QUERY ).is()
            )
        {
            css::uno::Reference< css::awt::XWindow >  xParentWindow   = xParent->getContainerWindow();
            vcl::Window*                                   pParentWindow   = VCLUnoHelper::GetWindow( xParentWindow    );
            //#i70261#: dialogs opened from an OLE object will cause a deactivate on the frame of the OLE object
            // on Solaris/Linux at that time pFocusWindow is still NULL because the focus handling is different; right after
            // the deactivation the focus will be set into the dialog!
            // currently I see no case where a sub frame could get a deactivate with pFocusWindow being NULL permanently
            // so for now this case is omitted from handled deactivations
            if( pFocusWindow && pParentWindow->IsChild( pFocusWindow ) )
            {
                css::uno::Reference< css::frame::XFramesSupplier > xSupplier( xParent, css::uno::UNO_QUERY );
                if( xSupplier.is() )
                {
                    aSolarGuard.clear();
                    xSupplier->setActiveFrame( css::uno::Reference< css::frame::XFrame >() );
                }
            }
        }
    }
}

void SAL_CALL Frame::windowClosing( const css::lang::EventObject& ) throw( css::uno::RuntimeException, std::exception )
{
    /* #i62088#
        Some interceptor objects intercept our "internally asynchronous implemented" dispatch call.
        And they close this frame directly (means synchronous then).
        Means: Frame::windowClosing()->Frame::close()
        In such situation its not a good idea to hold this transaction count alive .-)
    */
    {
        // Look for rejected calls.
        TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );
        // deactivate this frame ...
        deactivate();
    }

    // ... and try to close it
    // But do it asynchron inside the main thread.
    // VCL has no fun to do such things outside his main thread :-(
    // Note: The used dispatch make it asynchronous for us .-)

    /*ATTENTION!
        Don't try to suspend the controller here! Because it's done inside used dispatch().
        Otherwhise the dialog "would you save your changes?" will be shown more than once ...
     */

    /* SAFE */
    SolarMutexClearableGuard aReadLock;
    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;
    aReadLock.clear();
    /* SAFE */

    css::util::URL aURL;
    aURL.Complete = ".uno:CloseFrame";
    css::uno::Reference< css::util::XURLTransformer > xParser(css::util::URLTransformer::create(xContext));
    xParser->parseStrict(aURL);

    css::uno::Reference< css::frame::XDispatch > xCloser = queryDispatch(aURL, SPECIALTARGET_SELF, 0);
    if (xCloser.is())
        xCloser->dispatch(aURL, css::uno::Sequence< css::beans::PropertyValue >());

    // Attention: If this dispatch works synchronous ... and fulfill its job ...
    // this line of code will never be reached ...
    // Or if it will be reached it will be for sure that all your member are gone .-)
}

/*-****************************************************************************************************
    @short      react for a show event for the internal container window
    @descr      Normally we don't need this information really. But we can use it to
                implement the special feature "trigger first visible task".

                Algorithm: - first we have to check if we are a top (task) frame
                             It's not enough to be a top frame! Because we MUST have the desktop as parent.
                             But frames without a parent are top too. So it's not possible to check isTop() here!
                             We have to look for the type of our parent.
                           - if we are a task frame, then we have to check if we are the first one.
                             We use a static variable to do so. They will be reset to afterwards be sure
                             that further calls of this method doesn't do anything then.
                           - Then we have to trigger the right event string on the global job executor.

    @seealso    css::task::JobExecutor

    @param      aEvent
                    describes the source of this event
                    We are not interested on this information. We are interested on the visible state only.

    @threadsafe yes
*//*-*****************************************************************************************************/
void SAL_CALL Frame::windowShown( const css::lang::EventObject& ) throw(css::uno::RuntimeException, std::exception)
{
    static bool bFirstVisibleTask = true;
    static osl::Mutex aFirstVisibleLock;

    /* SAFE { */
    SolarMutexClearableGuard aReadLock;
    css::uno::Reference< css::frame::XDesktop >             xDesktopCheck( m_xParent, css::uno::UNO_QUERY );
    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;
    m_bIsHidden = false;
    aReadLock.clear();
    /* } SAFE */

    impl_checkMenuCloser();

    if (xDesktopCheck.is())
    {
        osl::ClearableMutexGuard aGuard(aFirstVisibleLock);
        bool bMustBeTriggered = bFirstVisibleTask;
        bFirstVisibleTask = false;
        aGuard.clear();

        if (bMustBeTriggered)
        {
            css::uno::Reference< css::task::XJobExecutor > xExecutor
                = css::task::theJobExecutor::get( xContext );
            xExecutor->trigger( "onFirstVisibleTask" );
        }
    }
}

void SAL_CALL Frame::windowHidden( const css::lang::EventObject& ) throw(css::uno::RuntimeException, std::exception)
{
    /* SAFE { */
    SolarMutexClearableGuard aReadLock;
    m_bIsHidden = true;
    aReadLock.clear();
    /* } SAFE */

    impl_checkMenuCloser();
}

/*-****************************************************************************************************
    @short      called by dispose of our windows!
    @descr      This object is forced to release all references to the interfaces given
                by the parameter source. We are a listener at our container window and
                should listen for his diposing.

    @seealso    XWindowListener
    @seealso    XTopWindowListener
    @seealso    XFocusListener
*//*-*****************************************************************************************************/
void SAL_CALL Frame::disposing( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException, std::exception )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Check incoming parameter.
    SAL_WARN_IF( implcp_disposing( aEvent ), "fwk", "Frame::disposing(): Invalid parameter detected." );
    // Look for rejected calls.
    // May be we are called during releasing our windows in our in dispose call!? => soft exceptions
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    SolarMutexResettableGuard aWriteLock;

    if( aEvent.Source == m_xContainerWindow )
    {
        // NECESSARY: Impl-method is threadsafe by himself!
        aWriteLock.clear();
        implts_stopWindowListening();
        aWriteLock.reset();
        m_xContainerWindow.clear();
    }
}

/*-************************************************************************************************************
    @interface  com.sun.star.document.XActionLockable
    @short      implement locking of frame/task from outside
    @descr      Sometimes we have problems to decide if closing of task is allowed. Because; frame/task
                could be used for pending loading jobs. So you can lock this object from outside and
                prevent instance against closing during using! But - don't do it in a wrong or expensive manner.
                Otherwise task couldn't die anymore!!!

    @seealso    interface XActionLockable
    @seeelso    method BaseDispatcher::implts_loadIt()
    @seeelso    method Desktop::loadComponentFromURL()
    @return     true if frame/task is locked
                false otherwise
    @threadsafe yes
*//*-*************************************************************************************************************/
sal_Bool SAL_CALL Frame::isActionLocked() throw( css::uno::RuntimeException, std::exception )
{
    SolarMutexGuard g;
    return( m_nExternalLockCount!=0);
}

void SAL_CALL Frame::addActionLock() throw( css::uno::RuntimeException, std::exception )
{
    SolarMutexGuard g;
    ++m_nExternalLockCount;
}

void SAL_CALL Frame::removeActionLock() throw( css::uno::RuntimeException, std::exception )
{
    // Register no transaction here! Otherwhise we wait for ever inside possible
    // implts_checkSuicide()/dispose() request ...

    {
        SolarMutexGuard g;
        SAL_WARN_IF( m_nExternalLockCount<=0, "fwk", "Frame::removeActionLock(): Frame is not locked! Possible multithreading problem detected." );
        --m_nExternalLockCount;
    }

    implts_checkSuicide();
}

void SAL_CALL Frame::setActionLocks( sal_Int16 nLock ) throw( css::uno::RuntimeException, std::exception )
{
    SolarMutexGuard g;
    // Attention: If somewhere called resetActionLocks() before and get e.g. 5 locks ...
    //            and tried to set these 5 ones here after his operations ...
    //            we can't ignore setted requests during these two calls!
    //            So we must add(!) these 5 locks here.
    m_nExternalLockCount = m_nExternalLockCount + nLock;
}

sal_Int16 SAL_CALL Frame::resetActionLocks() throw( css::uno::RuntimeException, std::exception )
{
    // Register no transaction here! Otherwhise we wait for ever inside possible
    // implts_checkSuicide()/dispose() request ...

    sal_Int16 nCurrentLocks = 0;
    {
        SolarMutexGuard g;
        nCurrentLocks = m_nExternalLockCount;
        m_nExternalLockCount = 0;
    }

    // Attention:
    // external lock count is 0 here every time ... but if
    // member m_bSelfClose is set to true too .... we call our own close()/dispose().
    // See close() for further information
    implts_checkSuicide();

    return nCurrentLocks;
}

void Frame::impl_initializePropInfo()
{
    impl_setPropertyChangeBroadcaster(static_cast< css::frame::XFrame* >(this));

    impl_addPropertyInfo(
        css::beans::Property(
            FRAME_PROPNAME_ASCII_DISPATCHRECORDERSUPPLIER,
            FRAME_PROPHANDLE_DISPATCHRECORDERSUPPLIER,
            cppu::UnoType<css::frame::XDispatchRecorderSupplier>::get(),
            css::beans::PropertyAttribute::TRANSIENT));

    impl_addPropertyInfo(
        css::beans::Property(
            FRAME_PROPNAME_ASCII_INDICATORINTERCEPTION,
            FRAME_PROPHANDLE_INDICATORINTERCEPTION,
            cppu::UnoType<css::task::XStatusIndicator>::get(),
            css::beans::PropertyAttribute::TRANSIENT));

    impl_addPropertyInfo(
        css::beans::Property(
            FRAME_PROPNAME_ASCII_ISHIDDEN,
            FRAME_PROPHANDLE_ISHIDDEN,
            cppu::UnoType<bool>::get(),
            css::beans::PropertyAttribute::TRANSIENT | css::beans::PropertyAttribute::READONLY));

    impl_addPropertyInfo(
        css::beans::Property(
            FRAME_PROPNAME_ASCII_LAYOUTMANAGER,
            FRAME_PROPHANDLE_LAYOUTMANAGER,
            cppu::UnoType<css::frame::XLayoutManager>::get(),
            css::beans::PropertyAttribute::TRANSIENT));

    impl_addPropertyInfo(
        css::beans::Property(
            FRAME_PROPNAME_ASCII_TITLE,
            FRAME_PROPHANDLE_TITLE,
            cppu::UnoType<OUString>::get(),
            css::beans::PropertyAttribute::TRANSIENT));
}

void SAL_CALL Frame::impl_setPropertyValue(const OUString& /*sProperty*/,
                                                 sal_Int32        nHandle  ,
                                           const css::uno::Any&   aValue   )

{
    /* There is no need to lock any mutex here. Because we share the
       solar mutex with our base class. And we said to our base class: "don't release it on calling us" .-)
       see ctor of PropertySetHelper for further information.
    */

    /* Attention: You can use nHandle only, if you are sure that all supported
                  properties has an unique handle. That must be guaranteed
                  inside method impl_initializePropInfo()!
    */
    switch (nHandle)
    {
        case FRAME_PROPHANDLE_TITLE :
                {
                    OUString sExternalTitle;
                    aValue >>= sExternalTitle;
                    setTitle (sExternalTitle);
                }
                break;

        case FRAME_PROPHANDLE_DISPATCHRECORDERSUPPLIER :
                aValue >>= m_xDispatchRecorderSupplier;
                break;

        case FRAME_PROPHANDLE_LAYOUTMANAGER :
                {
                    css::uno::Reference< css::frame::XLayoutManager2 > xOldLayoutManager = m_xLayoutManager;
                    css::uno::Reference< css::frame::XLayoutManager2 > xNewLayoutManager;
                    aValue >>= xNewLayoutManager;

                    if (xOldLayoutManager != xNewLayoutManager)
                    {
                        m_xLayoutManager = xNewLayoutManager;
                        if (xOldLayoutManager.is())
                            lcl_disableLayoutManager(xOldLayoutManager, this);
                        if (xNewLayoutManager.is())
                            lcl_enableLayoutManager(xNewLayoutManager, this);
                    }
                }
                break;

        case FRAME_PROPHANDLE_INDICATORINTERCEPTION :
                {
                    css::uno::Reference< css::task::XStatusIndicator > xProgress;
                    aValue >>= xProgress;
                    m_xIndicatorInterception = xProgress;
                }
                break;

        default :
                SAL_INFO("fwk",  "Frame::setFastPropertyValue_NoBroadcast(): Invalid handle detected!" );
                break;
    }
}

css::uno::Any SAL_CALL Frame::impl_getPropertyValue(const OUString& /*sProperty*/,
                                                          sal_Int32        nHandle  )
{
    /* There is no need to lock any mutex here. Because we share the
       solar mutex with our base class. And we said to our base class: "don't release it on calling us" .-)
       see ctor of PropertySetHelper for further information.
    */

    /* Attention: You can use nHandle only, if you are sure that all supported
                  properties has an unique handle. That must be guaranteed
                  inside method impl_initializePropInfo()!
    */
    css::uno::Any aValue;
    switch (nHandle)
    {
        case FRAME_PROPHANDLE_TITLE :
                aValue <<= getTitle ();
                break;

        case FRAME_PROPHANDLE_DISPATCHRECORDERSUPPLIER :
                aValue <<= m_xDispatchRecorderSupplier;
                break;

        case FRAME_PROPHANDLE_ISHIDDEN :
                aValue <<= m_bIsHidden;
                break;

        case FRAME_PROPHANDLE_LAYOUTMANAGER :
                aValue <<= m_xLayoutManager;
                break;

        case FRAME_PROPHANDLE_INDICATORINTERCEPTION :
                {
                    css::uno::Reference< css::task::XStatusIndicator > xProgress(m_xIndicatorInterception.get(), css::uno::UNO_QUERY);
                    aValue = css::uno::makeAny(xProgress);
                }
                break;

        default :
                SAL_INFO("fwk",  "Frame::getFastPropertyValue(): Invalid handle detected!" );
                break;
    }

    return aValue;
}

/*-****************************************************************************************************
    @short      dispose old container window and forget his reference
    @descr      Sometimes we must repair our "modal dialog parent mechanism" too!
    @param      "xWindow", reference to old container window to dispose it
    @return     An empty reference.
    @threadsafe NO!
*//*-*****************************************************************************************************/
void Frame::impl_disposeContainerWindow( css::uno::Reference< css::awt::XWindow >& xWindow )
{
    if( xWindow.is() )
    {
        xWindow->setVisible( sal_False );
        // All VclComponents are XComponents; so call dispose before discarding
        // a css::uno::Reference< XVclComponent >, because this frame is the owner of the window
        xWindow->dispose();
        xWindow.clear();
    }
}

/*-****************************************************************************************************
    @short      send frame action event to our listener
    @descr      This method is threadsafe AND can be called by our dispose method too!
    @param      "aAction", describe the event for sending
*//*-*****************************************************************************************************/
void Frame::implts_sendFrameActionEvent( const css::frame::FrameAction& aAction )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Sometimes used by dispose() => soft exceptions!
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    // Log information about order of events to file!
    // (only activated in debug version!)
    SAL_INFO( "fwk.frame",
              "[Frame] " << m_sName << " send event " <<
              (aAction == css::frame::FrameAction_COMPONENT_ATTACHED ? OUString("COMPONENT ATTACHED") :
               (aAction == css::frame::FrameAction_COMPONENT_DETACHING ? OUString("COMPONENT DETACHING") :
                (aAction == css::frame::FrameAction_COMPONENT_REATTACHED ? OUString("COMPONENT REATTACHED") :
                 (aAction == css::frame::FrameAction_FRAME_ACTIVATED ? OUString("FRAME ACTIVATED") :
                  (aAction == css::frame::FrameAction_FRAME_DEACTIVATING ? OUString("FRAME DEACTIVATING") :
                   (aAction == css::frame::FrameAction_CONTEXT_CHANGED ? OUString("CONTEXT CHANGED") :
                    (aAction == css::frame::FrameAction_FRAME_UI_ACTIVATED ? OUString("FRAME UI ACTIVATED") :
                     (aAction == css::frame::FrameAction_FRAME_UI_DEACTIVATING ? OUString("FRAME UI DEACTIVATING") :
                      (aAction == css::frame::FrameAction_MAKE_FIXED_SIZE ? OUString("MAKE_FIXED_SIZE") :
                       OUString("*invalid*")))))))))));

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    // Send css::frame::FrameAction event to all listener.
    // Get container for right listener.
    // FOLLOW LINES ARE THREADSAFE!!!
    // ( OInterfaceContainerHelper is synchronized with m_aListenerContainer! )
    ::cppu::OInterfaceContainerHelper* pContainer = m_aListenerContainer.getContainer( cppu::UnoType<css::frame::XFrameActionListener>::get());

    if( pContainer != nullptr )
    {
        // Build action event.
        css::frame::FrameActionEvent aFrameActionEvent( static_cast< ::cppu::OWeakObject* >(this), this, aAction );

        // Get iterator for access to listener.
        ::cppu::OInterfaceIteratorHelper aIterator( *pContainer );
        // Send message to all listener.
        while( aIterator.hasMoreElements() )
        {
            try
            {
                static_cast<css::frame::XFrameActionListener*>(aIterator.next())->frameAction( aFrameActionEvent );
            }
            catch( const css::uno::RuntimeException& )
            {
                aIterator.remove();
            }
        }
    }
}

/*-****************************************************************************************************
    @short      helper to resize our component window
    @descr      A frame contains 2 windows - a container ~ and a component window.
                This method resize inner component window to full size of outer container window.
                This method is threadsafe AND can be called by our dispose method too!
*//*-*****************************************************************************************************/
void Frame::implts_resizeComponentWindow()
{
    // usually the LayoutManager does the resizing
    // in case there is no LayoutManager resizing has to be done here
    if ( !m_xLayoutManager.is() )
    {
        css::uno::Reference< css::awt::XWindow > xComponentWindow( getComponentWindow() );
        if( xComponentWindow.is() )
        {
            css::uno::Reference< css::awt::XDevice > xDevice( getContainerWindow(), css::uno::UNO_QUERY );

            // Convert relativ size to output size.
            css::awt::Rectangle  aRectangle  = getContainerWindow()->getPosSize();
            css::awt::DeviceInfo aInfo       = xDevice->getInfo();
            css::awt::Size       aSize       (  aRectangle.Width  - aInfo.LeftInset - aInfo.RightInset  ,
                                                aRectangle.Height - aInfo.TopInset  - aInfo.BottomInset );

            // Resize our component window.
            xComponentWindow->setPosSize( 0, 0, aSize.Width, aSize.Height, css::awt::PosSize::POSSIZE );
        }
    }
}

/*-****************************************************************************************************
    @short      helper to set icon on our container window (if it is a system window!)
    @descr      We use our internal set controller (if it exist) to specify which factory he represented.
                These information can be used to find right icon. But our controller can say it us directly
                too ... we should ask his optional property set first ...

    @seealso    method Window::SetIcon()
    @onerror    We do nothing.
*//*-*****************************************************************************************************/
void Frame::implts_setIconOnWindow()
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Look for rejected calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    // Make snapshot of necessary members and release lock.
    SolarMutexClearableGuard aReadLock;
    css::uno::Reference< css::awt::XWindow >       xContainerWindow( m_xContainerWindow, css::uno::UNO_QUERY );
    css::uno::Reference< css::frame::XController > xController     ( m_xController     , css::uno::UNO_QUERY );
    aReadLock.clear();
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */

    if( xContainerWindow.is() && xController.is() )
    {

        // a) set default value to an invalid one. So we can start further searches for right icon id, if
        //    first steps failed!
        //    We must reset it to any fallback value - if no search step returns a valid result.
        sal_Int32 nIcon = -1;

        // b) try to find information on controller propertyset directly
        //    Don't forget to catch possible exceptions - because these property is an optional one!
        css::uno::Reference< css::beans::XPropertySet > xSet( xController, css::uno::UNO_QUERY );
        if( xSet.is() )
        {
            try
            {
                css::uno::Reference< css::beans::XPropertySetInfo > const xPSI( xSet->getPropertySetInfo(), css::uno::UNO_SET_THROW );
                if ( xPSI->hasPropertyByName( "IconId" ) )
                    xSet->getPropertyValue( "IconId" ) >>= nIcon;
            }
            catch( css::uno::Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }

        // c) if b) failed... analyze argument list of currently loaded document inside the frame to find the filter.
        //    He can be used to detect right factory - and these can be used to match factory to icon...
        if( nIcon == -1 )
        {
            css::uno::Reference< css::frame::XModel > xModel = xController->getModel();
            if( xModel.is() )
            {
                SvtModuleOptions::EFactory eFactory = SvtModuleOptions::ClassifyFactoryByModel(xModel);
                if (eFactory != SvtModuleOptions::EFactory::UNKNOWN_FACTORY)
                    nIcon = SvtModuleOptions().GetFactoryIcon( eFactory );
            }
        }

        // d) if all steps failed - use fallback!
        if( nIcon == -1 )
        {
            nIcon = 0;
        }

        // e) set icon on container window now
        //    Don't forget SolarMutex! We use vcl directly :-(
        //    Check window pointer for right WorkWindow class too!!!
        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        {
            SolarMutexGuard aSolarGuard;
            vcl::Window* pWindow = (VCLUnoHelper::GetWindow( xContainerWindow ));
            if(
                ( pWindow            != nullptr              ) &&
                ( pWindow->GetType() == WINDOW_WORKWINDOW )
                )
            {
                WorkWindow* pWorkWindow = static_cast<WorkWindow*>(pWindow);
                pWorkWindow->SetIcon( (sal_uInt16)nIcon );
            }
        }
        /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    }
}

/*-************************************************************************************************************
    @short      helper to start/stop listeneing for window events on container window
    @descr      If we get a new container window, we must set it on internal member ...
                and stop listening at old one ... and start listening on new one!
                But sometimes (in dispose() call!) it's necessary to stop listeneing without starting
                on new connections. So we split this functionality to make it easier at use.

    @seealso    method initialize()
    @seealso    method dispose()
    @onerror    We do nothing!
    @threadsafe yes
*//*-*************************************************************************************************************/
void Frame::implts_startWindowListening()
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    // Make snapshot of necessary member!
    SolarMutexClearableGuard aReadLock;
    css::uno::Reference< css::awt::XWindow >                            xContainerWindow    = m_xContainerWindow;
    css::uno::Reference< css::uno::XComponentContext >                  xContext = m_xContext;
    css::uno::Reference< css::datatransfer::dnd::XDropTargetListener >  xDragDropListener   = m_xDropTargetListener;
    css::uno::Reference< css::awt::XWindowListener >                    xWindowListener     ( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );
    css::uno::Reference< css::awt::XFocusListener >                     xFocusListener      ( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );
    css::uno::Reference< css::awt::XTopWindowListener >                 xTopWindowListener  ( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );
    aReadLock.clear();
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */

    if( xContainerWindow.is() )
    {
        xContainerWindow->addWindowListener( xWindowListener);
        xContainerWindow->addFocusListener ( xFocusListener );

        css::uno::Reference< css::awt::XTopWindow > xTopWindow( xContainerWindow, css::uno::UNO_QUERY );
        if( xTopWindow.is() )
        {
            xTopWindow->addTopWindowListener( xTopWindowListener );

            css::uno::Reference< css::awt::XToolkit2 > xToolkit = css::awt::Toolkit::create( xContext );
            css::uno::Reference< css::datatransfer::dnd::XDropTarget > xDropTarget = xToolkit->getDropTarget( xContainerWindow );
            if( xDropTarget.is() )
            {
                xDropTarget->addDropTargetListener( xDragDropListener );
                xDropTarget->setActive( sal_True );
            }
        }
    }
}

void Frame::implts_stopWindowListening()
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Sometimes used by dispose() => soft exceptions!
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    // Make snapshot of necessary member!
    SolarMutexClearableGuard aReadLock;
    css::uno::Reference< css::awt::XWindow >                            xContainerWindow    = m_xContainerWindow;
    css::uno::Reference< css::uno::XComponentContext >                  xContext            = m_xContext;
    css::uno::Reference< css::datatransfer::dnd::XDropTargetListener >  xDragDropListener   = m_xDropTargetListener;
    css::uno::Reference< css::awt::XWindowListener >                    xWindowListener     ( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );
    css::uno::Reference< css::awt::XFocusListener >                     xFocusListener      ( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );
    css::uno::Reference< css::awt::XTopWindowListener >                 xTopWindowListener  ( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );
    aReadLock.clear();
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */

    if( xContainerWindow.is() )
    {
        xContainerWindow->removeWindowListener( xWindowListener);
        xContainerWindow->removeFocusListener ( xFocusListener );

        css::uno::Reference< css::awt::XTopWindow > xTopWindow( xContainerWindow, css::uno::UNO_QUERY );
        if( xTopWindow.is() )
        {
            xTopWindow->removeTopWindowListener( xTopWindowListener );

            css::uno::Reference< css::awt::XToolkit2 > xToolkit = css::awt::Toolkit::create( xContext );
            css::uno::Reference< css::datatransfer::dnd::XDropTarget > xDropTarget = xToolkit->getDropTarget( xContainerWindow );
            if( xDropTarget.is() )
            {
                xDropTarget->removeDropTargetListener( xDragDropListener );
                xDropTarget->setActive( sal_False );
            }
        }
    }
}

/*-****************************************************************************************************
    @short      helper to force breaked close() request again
    @descr      If we self disagree with a close() request, and detect that all external locks are gone ...
                then we must try to close this frame again.

    @seealso    XCloseable::close()
    @seealso    Frame::close()
    @seealso    Frame::removeActionLock()
    @seealso    Frame::resetActionLock()
    @seealso    m_bSelfClose
    @seealso    m_nExternalLockCount

    @threadsafe yes
*//*-*****************************************************************************************************/
void Frame::implts_checkSuicide()
{
    /* SAFE */
    SolarMutexClearableGuard aReadLock;
    // in case of lock==0 and safed state of previous close() request m_bSelfClose
    // we must force close() again. Because we had disagreed with that before.
    bool bSuicide = (m_nExternalLockCount==0 && m_bSelfClose);
    m_bSelfClose = false;
    aReadLock.clear();
    /* } SAFE */
    // force close and deliver ownership to source of possible throwed veto exception
    // Attention: Because this method is not designed to throw such exception we must suppress
    // it for outside code!
    try
    {
        if (bSuicide)
            close(sal_True);
    }
    catch(const css::util::CloseVetoException&)
        {}
    catch(const css::lang::DisposedException&)
        {}
}

/** little helper to enable/disable the menu closer at the menubar of the given frame.

    @param  xFrame
            we use its layout manager to set/reset a special callback.
            Its existence regulate visibility of this closer item.

    @param  bState
                <TRUE/> enable; <FALSE/> disable this state
 */

void Frame::impl_setCloser( /*IN*/ const css::uno::Reference< css::frame::XFrame2 >& xFrame ,
                            /*IN*/       bool                                   bState  )
{
    // Note: If start module is not installed - no closer has to be shown!
    if (!SvtModuleOptions().IsModuleInstalled(SvtModuleOptions::EModule::STARTMODULE))
        return;

    try
    {
        css::uno::Reference< css::beans::XPropertySet > xFrameProps(xFrame, css::uno::UNO_QUERY_THROW);
        css::uno::Reference< css::frame::XLayoutManager > xLayoutManager;
        xFrameProps->getPropertyValue(FRAME_PROPNAME_ASCII_LAYOUTMANAGER) >>= xLayoutManager;
        css::uno::Reference< css::beans::XPropertySet > xLayoutProps(xLayoutManager, css::uno::UNO_QUERY_THROW);
        xLayoutProps->setPropertyValue(LAYOUTMANAGER_PROPNAME_MENUBARCLOSER, css::uno::makeAny(bState));
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        {}
}

/** it checks, which of the top level task frames must have the special menu closer for
    switching to the backing window mode.

    It analyze the current list of visible top level frames. Only the last real document
    frame can have this symbol. Not the help frame nor the backing task itself.
    Here we do anything related to this closer. We remove it from the old frame and set it
    for the new one.
 */

void Frame::impl_checkMenuCloser()
{
    /* SAFE { */
    SolarMutexClearableGuard aReadLock;

    // only top frames, which are part of our desktop hierarchy, can
    // do so! By the way - we need the desktop instance to have access
    // to all other top level frames too.
    css::uno::Reference< css::frame::XDesktop >        xDesktop     (m_xParent, css::uno::UNO_QUERY);
    css::uno::Reference< css::frame::XFramesSupplier > xTaskSupplier(xDesktop , css::uno::UNO_QUERY);
    if ( !xDesktop.is() || !xTaskSupplier.is() )
        return;

    aReadLock.clear();
    /* } SAFE */

    // analyze the list of current open tasks
    // Suppress search for other views to the same model ...
    // It's not needed here and can be very expensive.
    FrameListAnalyzer aAnalyzer(
        xTaskSupplier,
        this,
        FrameListAnalyzer::E_HIDDEN | FrameListAnalyzer::E_HELP | FrameListAnalyzer::E_BACKINGCOMPONENT);

    // specify the new frame, which must have this special state ...
    css::uno::Reference< css::frame::XFrame2 > xNewCloserFrame;

    // a)
    // If there exist ate least one other frame - there are two frames currently open.
    // But we can enable this closer only, if one of these two tasks includes the help module.
    // The "other frame" couldn't be the help. Because then it wouldn't be part of this "other list".
    // In such case it will be separated to the reference aAnalyzer.m_xHelp!
    // But we must check, if weself includes the help ...
    // Check aAnalyzer.m_bReferenceIsHelp!
    if (
        (aAnalyzer.m_lOtherVisibleFrames.getLength()==1)   &&
        (
            (aAnalyzer.m_bReferenceIsHelp  ) ||
            (aAnalyzer.m_bReferenceIsHidden)
        )
       )
    {
        // others[0] can't be the backing component!
        // Because it's set at the special member aAnalyzer.m_xBackingComponent ... :-)
        xNewCloserFrame.set( aAnalyzer.m_lOtherVisibleFrames[0], css::uno::UNO_QUERY_THROW );
    }

    // b)
    // There is no other frame ... means no other document frame. The help module
    // will be handled separately and must(!) be ignored here ... excepting weself includes the help.
    else if (
        (aAnalyzer.m_lOtherVisibleFrames.getLength()==0) &&
        (!aAnalyzer.m_bReferenceIsHelp                 ) &&
        (!aAnalyzer.m_bReferenceIsHidden               ) &&
        (!aAnalyzer.m_bReferenceIsBacking              )
       )
    {
        xNewCloserFrame = this;
    }

    // Look for necessary actions ...
    // Only if the closer state must be moved from one frame to another one
    // or must be enabled/disabled at all.
    SolarMutexGuard aGuard;
    // Holds the only frame, which must show the special closer menu item (can be NULL!)
    static css::uno::WeakReference< css::frame::XFrame2 > s_xCloserFrame;
    css::uno::Reference< css::frame::XFrame2 > xCloserFrame (s_xCloserFrame.get(), css::uno::UNO_QUERY);
    if (xCloserFrame!=xNewCloserFrame)
    {
        if (xCloserFrame.is())
            impl_setCloser(xCloserFrame, false);
        if (xNewCloserFrame.is())
            impl_setCloser(xNewCloserFrame, true);
        s_xCloserFrame = xNewCloserFrame;
    }
}

//  debug methods

// Its allowed to reset the active frame member variable with a NULL-css::uno::Reference but not with a NULL-pointer!
// And we accept frames only! No tasks and desktops!
bool Frame::implcp_setActiveFrame( const css::uno::Reference< css::frame::XFrame >& xFrame )
{
    return css::uno::Reference< css::frame::XDesktop >( xFrame, css::uno::UNO_QUERY ).is();
}

bool Frame::implcp_addFrameActionListener( const css::uno::Reference< css::frame::XFrameActionListener >& xListener )
{
    return !xListener.is();
}

bool Frame::implcp_removeFrameActionListener( const css::uno::Reference< css::frame::XFrameActionListener >& xListener )
{
    return !xListener.is();
}

bool Frame::implcp_addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener )
{
    return !xListener.is();
}

bool Frame::implcp_removeEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener )
{
    return !xListener.is();
}

bool Frame::implcp_windowResized( const css::awt::WindowEvent& aEvent )
{
    return !aEvent.Source.is();
}

bool Frame::implcp_focusGained( const css::awt::FocusEvent& aEvent )
{
    return !aEvent.Source.is();
}

bool Frame::implcp_windowActivated( const css::lang::EventObject& aEvent )
{
    return !aEvent.Source.is();
}

bool Frame::implcp_windowDeactivated( const css::lang::EventObject& aEvent )
{
    return !aEvent.Source.is();
}

bool Frame::implcp_disposing( const css::lang::EventObject& aEvent )
{
    return !aEvent.Source.is();
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_framework_Frame_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    Frame *inst = new Frame(context);
    css::uno::XInterface *acquired_inst = cppu::acquire(inst);

    inst->initListeners();

    return acquired_inst;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
