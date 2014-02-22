/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
#include <threadhelp/resetableguard.hxx>
#include <threadhelp/writeguard.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/threadhelpbase.hxx>
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


enum EActiveState
{
    E_INACTIVE      ,   
    E_ACTIVE        ,   
    E_FOCUS             
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

    @base       ThreadHelpBase
                    help to guarantee correct initialized lock member at startup
    @base       TransactionBase
                    help to implement unbreakable interface calls
    @base       OBroadcastHelper
                OPropertySetHelper
                    implements the property set
    @base       OWeakObject
                    provides the refcount and XInterface, XWeak

    @devstatus  ready to use
    @threadsafe yes
*
class Frame :   
                public  css::lang::XTypeProvider                    ,
                public  css::lang::XServiceInfo                     ,
                public  css::frame::XFrame2                         ,
                public  css::awt::XWindowListener                   ,   
                public  css::awt::XTopWindowListener                ,
                public  css::awt::XFocusListener                    ,
                public  css::document::XActionLockable              ,
                public  css::util::XCloseable                       ,   
                public  css::frame::XComponentLoader                ,
                public  css::frame::XTitle                          ,
                public  css::frame::XTitleChangeBroadcaster         ,
                
                
                public  ThreadHelpBase                              ,
                public  TransactionBase                             ,
                public  PropertySetHelper                           ,   
                public  ::cppu::OWeakObject                             
{
public:

             Frame( const css::uno::Reference< css::uno::XComponentContext >& xContext );
    virtual ~Frame();

    
    void initListeners();

    FWK_DECLARE_XINTERFACE
    FWK_DECLARE_XTYPEPROVIDER

    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException)
    {
        return OUString("com.sun.star.comp.framework.Frame");
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException)
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException)
    {
        css::uno::Sequence< OUString > aSeq(1);
        aSeq[0] = OUString("com.sun.star.frame.Frame");
        return aSeq;
    }

    
    
    
    virtual css::uno::Reference< css::lang::XComponent > SAL_CALL loadComponentFromURL(
            const OUString& sURL,
            const OUString& sTargetFrameName,
            sal_Int32 nSearchFlags,
            const css::uno::Sequence< css::beans::PropertyValue >& lArguments )
        throw( css::io::IOException,
               css::lang::IllegalArgumentException,
               css::uno::RuntimeException );


    
    
    
    virtual css::uno::Reference< css::frame::XFrames >          SAL_CALL getFrames                          (                                                                                                   ) throw( css::uno::RuntimeException );
    virtual css::uno::Reference< css::frame::XFrame >           SAL_CALL getActiveFrame                     (                                                                                                   ) throw( css::uno::RuntimeException );
    virtual void                                                SAL_CALL setActiveFrame                     (   const   css::uno::Reference< css::frame::XFrame >&                          xFrame              ) throw( css::uno::RuntimeException );

    
    
    
    virtual void                                                SAL_CALL initialize                         (   const   css::uno::Reference< css::awt::XWindow >&                           xWindow             ) throw( css::uno::RuntimeException );
    virtual css::uno::Reference< css::awt::XWindow >            SAL_CALL getContainerWindow                 (                                                                                                   ) throw( css::uno::RuntimeException );
    virtual void                                                SAL_CALL setCreator                         (   const   css::uno::Reference< css::frame::XFramesSupplier >&                 xCreator            ) throw( css::uno::RuntimeException );
    virtual css::uno::Reference< css::frame::XFramesSupplier >  SAL_CALL getCreator                         (                                                                                                   ) throw( css::uno::RuntimeException );
    virtual OUString                                     SAL_CALL getName                            (                                                                                                   ) throw( css::uno::RuntimeException );
    virtual void                                                SAL_CALL setName                            (   const   OUString&                                                    sName               ) throw( css::uno::RuntimeException );
    virtual css::uno::Reference< css::frame::XFrame >           SAL_CALL findFrame                          (   const   OUString&                                                    sTargetFrameName    ,
                                                                                                                        sal_Int32                                                           nSearchFlags        ) throw( css::uno::RuntimeException );
    virtual sal_Bool                                            SAL_CALL isTop                              (                                                                                                   ) throw( css::uno::RuntimeException );
    virtual void                                                SAL_CALL activate                           (                                                                                                   ) throw( css::uno::RuntimeException );
    virtual void                                                SAL_CALL deactivate                         (                                                                                                   ) throw( css::uno::RuntimeException );
    virtual sal_Bool                                            SAL_CALL isActive                           (                                                                                                   ) throw( css::uno::RuntimeException );
    virtual void                                                SAL_CALL contextChanged                     (                                                                                                   ) throw( css::uno::RuntimeException );
    virtual sal_Bool                                            SAL_CALL setComponent                       (   const   css::uno::Reference< css::awt::XWindow >&                           xComponentWindow    ,
                                                                                                                const   css::uno::Reference< css::frame::XController >&                     xController         ) throw( css::uno::RuntimeException );
    virtual css::uno::Reference< css::awt::XWindow >            SAL_CALL getComponentWindow                 (                                                                                                   ) throw( css::uno::RuntimeException );
    virtual css::uno::Reference< css::frame::XController >      SAL_CALL getController                      (                                                                                                   ) throw( css::uno::RuntimeException );
    virtual void                                                SAL_CALL addFrameActionListener             (   const   css::uno::Reference< css::frame::XFrameActionListener >&            xListener           ) throw( css::uno::RuntimeException );
    virtual void                                                SAL_CALL removeFrameActionListener          (   const   css::uno::Reference< css::frame::XFrameActionListener >&            xListener           ) throw( css::uno::RuntimeException );

    
    
    
    virtual void                                                SAL_CALL dispose                            (                                                                                                   ) throw( css::uno::RuntimeException );
    virtual void                                                SAL_CALL addEventListener                   (   const   css::uno::Reference< css::lang::XEventListener >&                   xListener           ) throw( css::uno::RuntimeException );
    virtual void                                                SAL_CALL removeEventListener                (   const   css::uno::Reference< css::lang::XEventListener >&                   xListener           ) throw( css::uno::RuntimeException );

    
    
    
    virtual css::uno::Reference< css::task::XStatusIndicator >  SAL_CALL createStatusIndicator              (                                                                                                   ) throw( css::uno::RuntimeException );

    
    
    
    virtual css::uno::Reference< css::frame::XDispatch >        SAL_CALL queryDispatch                      (   const   css::util::URL&                                                     aURL                ,
                                                                                                                const   OUString&                                                    sTargetFrameName    ,
                                                                                                                        sal_Int32                                                           nSearchFlags        ) throw( css::uno::RuntimeException );
    virtual css::uno::Sequence<
                css::uno::Reference< css::frame::XDispatch > >  SAL_CALL queryDispatches                    (   const   css::uno::Sequence< css::frame::DispatchDescriptor >&               lDescriptor         ) throw( css::uno::RuntimeException );

    
    
    
    virtual void                                                SAL_CALL registerDispatchProviderInterceptor(   const   css::uno::Reference< css::frame::XDispatchProviderInterceptor >&    xInterceptor        ) throw( css::uno::RuntimeException );
    virtual void                                                SAL_CALL releaseDispatchProviderInterceptor (   const   css::uno::Reference< css::frame::XDispatchProviderInterceptor >&    xInterceptor        ) throw( css::uno::RuntimeException );

    
    
    
    virtual css::uno::Sequence< sal_Int16 >                       SAL_CALL getSupportedCommandGroups         (                       ) throw (css::uno::RuntimeException);
    virtual css::uno::Sequence< css::frame::DispatchInformation > SAL_CALL getConfigurableDispatchInformation(sal_Int16 nCommandGroup) throw (css::uno::RuntimeException);

    
    
    
    
    virtual void                                                SAL_CALL windowResized                      (   const   css::awt::WindowEvent&                                              aEvent              ) throw( css::uno::RuntimeException );
    virtual void                                                SAL_CALL windowMoved                        (   const   css::awt::WindowEvent&                                              /*aEvent*/          ) throw( css::uno::RuntimeException ) {};
    virtual void                                                SAL_CALL windowShown                        (   const   css::lang::EventObject&                                             aEvent              ) throw( css::uno::RuntimeException );
    virtual void                                                SAL_CALL windowHidden                       (   const   css::lang::EventObject&                                             aEvent              ) throw( css::uno::RuntimeException );

    
    
    
    
    virtual void                                                SAL_CALL focusGained                        (   const   css::awt::FocusEvent&                                               aEvent              ) throw( css::uno::RuntimeException );
    virtual void                                                SAL_CALL focusLost                          (   const   css::awt::FocusEvent&                                               /*aEvent*/          ) throw( css::uno::RuntimeException ) {};

    
    
    
    
    virtual void                                                SAL_CALL windowActivated                    (   const   css::lang::EventObject&                                             aEvent              ) throw( css::uno::RuntimeException );
    virtual void                                                SAL_CALL windowDeactivated                  (   const   css::lang::EventObject&                                             aEvent              ) throw( css::uno::RuntimeException );
    virtual void                                                SAL_CALL windowOpened                       (   const   css::lang::EventObject&                                             /*aEvent*/          ) throw( css::uno::RuntimeException ) {};
    virtual void                                                SAL_CALL windowClosing                      (   const   css::lang::EventObject&                                             aEvent              ) throw( css::uno::RuntimeException );
    virtual void                                                SAL_CALL windowClosed                       (   const   css::lang::EventObject&                                             /*aEvent*/          ) throw( css::uno::RuntimeException ) {};
    virtual void                                                SAL_CALL windowMinimized                    (   const   css::lang::EventObject&                                             /*aEvent*/          ) throw( css::uno::RuntimeException ) {};
    virtual void                                                SAL_CALL windowNormalized                   (   const   css::lang::EventObject&                                             /*aEvent*/          ) throw( css::uno::RuntimeException ) {};

    
    
    
    virtual void                                                SAL_CALL disposing                          (   const   css::lang::EventObject&                                             aEvent              ) throw( css::uno::RuntimeException );

    
    
    
    virtual sal_Bool    SAL_CALL isActionLocked  (                 ) throw( css::uno::RuntimeException );
    virtual void        SAL_CALL addActionLock   (                 ) throw( css::uno::RuntimeException );
    virtual void        SAL_CALL removeActionLock(                 ) throw( css::uno::RuntimeException );
    virtual void        SAL_CALL setActionLocks  ( sal_Int16 nLock ) throw( css::uno::RuntimeException );
    virtual sal_Int16   SAL_CALL resetActionLocks(                 ) throw( css::uno::RuntimeException );

    
    
    
    virtual void SAL_CALL close( sal_Bool bDeliverOwnership ) throw( css::util::CloseVetoException,
                                                                     css::uno::RuntimeException   );

    
    
    
    virtual void SAL_CALL addCloseListener   ( const css::uno::Reference< css::util::XCloseListener >& xListener ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL removeCloseListener( const css::uno::Reference< css::util::XCloseListener >& xListener ) throw (css::uno::RuntimeException);

    
    
    
    virtual OUString SAL_CALL getTitle(                               ) throw (css::uno::RuntimeException);
    virtual void            SAL_CALL setTitle( const OUString& sTitle ) throw (css::uno::RuntimeException);

    
    
    
    virtual void SAL_CALL addTitleChangeListener   ( const css::uno::Reference< css::frame::XTitleChangeListener >& xListener) throw (css::uno::RuntimeException);
    virtual void SAL_CALL removeTitleChangeListener( const css::uno::Reference< css::frame::XTitleChangeListener >& xListenr ) throw (css::uno::RuntimeException);


    
    
    
    virtual css::uno::Reference<css::container::XNameContainer> SAL_CALL getUserDefinedAttributes() throw (css::uno::RuntimeException);

    virtual css::uno::Reference<css::frame::XDispatchRecorderSupplier> SAL_CALL getDispatchRecorderSupplier() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setDispatchRecorderSupplier(const css::uno::Reference<css::frame::XDispatchRecorderSupplier>&) throw (css::uno::RuntimeException);

    virtual css::uno::Reference<css::uno::XInterface> SAL_CALL getLayoutManager() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setLayoutManager(const css::uno::Reference<css::uno::XInterface>&) throw (css::uno::RuntimeException);


    
    
    
private:

    void impl_initializePropInfo();

    virtual void SAL_CALL impl_setPropertyValue(const OUString& sProperty,
                                                      sal_Int32        nHandle  ,
                                                const css::uno::Any&   aValue   );

    virtual css::uno::Any SAL_CALL impl_getPropertyValue(const OUString& sProperty,
                                                               sal_Int32        nHandle  );





private:

    /*-****************************************************************************************************
        @short      helper methods
        @descr      Follow methods are needed at different points of our code (more then ones!).

        @attention  Threadsafe methods are signed by "implts_..."!
    *

    
    void implts_sendFrameActionEvent     ( const css::frame::FrameAction&                        aAction          );
    void implts_resizeComponentWindow    (                                                                        );
    void implts_setIconOnWindow          (                                                                        );
    void implts_startWindowListening     (                                                                        );
    void implts_stopWindowListening      (                                                                        );
    void implts_saveWindowAttributes     (                                                                        );
    void implts_checkSuicide             (                                                                        );
    void implts_forgetSubFrames          (                                                                        );
    DECL_LINK( implts_windowClosing, void* );
    css::uno::Reference< css::awt::XTopWindow >             impl_searchTopWindow            ( const css::uno::Reference< css::awt::XWindow >&       xWindow          );

    
    void impl_checkMenuCloser            (                                                                        );
    void impl_setCloser                  ( const css::uno::Reference< css::frame::XFrame2 >& xFrame , sal_Bool bState );
    void impl_disposeContainerWindow     (       css::uno::Reference< css::awt::XWindow >&       xWindow          );






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
    *

private:

    static sal_Bool implcp_setActiveFrame                      (   const   css::uno::Reference< css::frame::XFrame >&                          xFrame              );
    static sal_Bool implcp_addFrameActionListener              (   const   css::uno::Reference< css::frame::XFrameActionListener >&            xListener           );
    static sal_Bool implcp_removeFrameActionListener           (   const   css::uno::Reference< css::frame::XFrameActionListener >&            xListener           );
    static sal_Bool implcp_addEventListener                    (   const   css::uno::Reference< css::lang::XEventListener >&                   xListener           );
    static sal_Bool implcp_removeEventListener                 (   const   css::uno::Reference< css::lang::XEventListener >&                   xListener           );
    static sal_Bool implcp_windowResized                       (   const   css::awt::WindowEvent&                                              aEvent              );
    static sal_Bool implcp_focusGained                         (   const   css::awt::FocusEvent&                                               aEvent              );
    static sal_Bool implcp_windowActivated                     (   const   css::lang::EventObject&                                             aEvent              );
    static sal_Bool implcp_windowDeactivated                   (   const   css::lang::EventObject&                                             aEvent              );
    static sal_Bool implcp_disposing                           (   const   css::lang::EventObject&                                             aEvent              );





private:

    css::uno::Reference< css::uno::XComponentContext >                      m_xContext                          ;   
    css::uno::Reference< css::task::XStatusIndicatorFactory >               m_xIndicatorFactoryHelper           ;   
    css::uno::WeakReference< css::task::XStatusIndicator >                  m_xIndicatorInterception            ;   
    css::uno::Reference< css::frame::XDispatchProvider >                    m_xDispatchHelper                   ;   
    css::uno::Reference< css::frame::XFrames >                              m_xFramesHelper                     ;   
    ::cppu::OMultiTypeInterfaceContainerHelper                              m_aListenerContainer                ;   
    css::uno::Reference< css::frame::XFramesSupplier >                      m_xParent                           ;   
    css::uno::Reference< css::awt::XWindow >                                m_xContainerWindow                  ;   
    css::uno::Reference< css::awt::XWindow >                                m_xComponentWindow                  ;   
    css::uno::Reference< css::frame::XController >                          m_xController                       ;   
    css::uno::Reference< css::datatransfer::dnd::XDropTargetListener >      m_xDropTargetListener               ;   
    EActiveState                                                            m_eActiveState                      ;   
    OUString                                                         m_sName                             ;   
    sal_Bool                                                                m_bIsFrameTop                       ;   
    sal_Bool                                                                m_bConnected                        ;   
    sal_Int16                                                               m_nExternalLockCount                ;
    css::uno::Reference< css::frame::XDispatchRecorderSupplier >            m_xDispatchRecorderSupplier         ;   
    SvtCommandOptions                                                       m_aCommandOptions                   ;   
    sal_Bool                                                                m_bSelfClose                        ;   
    sal_Bool                                                                m_bIsHidden                         ;   
    static css::uno::WeakReference< css::frame::XFrame2 >                   m_xCloserFrame                      ;   
    css::uno::Reference< ::css::frame::XLayoutManager2 >                    m_xLayoutManager                    ;   
    css::uno::Reference< css::frame::XDispatchInformationProvider >         m_xDispatchInfoHelper               ;
    css::uno::Reference< css::frame::XTitle >                               m_xTitleHelper                      ;

    WindowCommandDispatch*                                                  m_pWindowCommandDispatch            ;

protected:

    FrameContainer                                                          m_aChildFrameContainer              ;   

    inline css::uno::Reference< css::uno::XComponentContext > impl_getComponentContext()
    {
        ReadGuard aReadLock( m_aLock );
        return m_xContext;
    }

    inline OUString impl_getName()
    {
        ReadGuard aReadLock( m_aLock );
        return m_sName;
    }

    inline css::uno::Reference< css::awt::XWindow > impl_getContainerWindow()
    {
        ReadGuard aReadLock( m_aLock );
        return m_xContainerWindow;
    }

    inline css::uno::Reference< css::frame::XDispatchProvider > impl_getDispatchHelper()
    {
        ReadGuard aReadLock( m_aLock );
        return m_xDispatchHelper;
    }

    inline css::uno::Reference< css::frame::XFramesSupplier > impl_getParent()
    {
        ReadGuard aReadLock( m_aLock );
        return m_xParent;
    }
};

css::uno::WeakReference< css::frame::XFrame2 > Frame::m_xCloserFrame = css::uno::WeakReference< css::frame::XFrame2 >();




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
                    Do such things in DEFINE_INIT_SERVICE() method, which is called automaticly after your ctor!!!
                b)  Baseclass OBroadcastHelper is a typedef in namespace cppu!
                    The microsoft compiler has some problems to handle it right BY using namespace explicitly ::cppu::OBroadcastHelper.
                    If we write it without a namespace or expand the typedef to OBrodcastHelperVar<...> -> it will be OK!?
                    I don't know why! (other compiler not tested .. but it works!)

    @seealso    method DEFINE_INIT_SERVICE()

    @param      xContext is the multi service manager, which creates this instance.
                    The value must be different from NULL!
    @return     -

    @onerror    ASSERT in debug version or nothing in relaese version.
*
Frame::Frame( const css::uno::Reference< css::uno::XComponentContext >& xContext )
        :   ThreadHelpBase              ( &Application::GetSolarMutex()                     )
        ,   TransactionBase             (                                                   )
        ,   PropertySetHelper           ( &m_aLock,
                                          &m_aTransactionManager,
                                          sal_False) 
        
        ,   m_xContext                  ( xContext                                          )
        ,   m_aListenerContainer        ( m_aLock.getShareableOslMutex()                    )
        ,   m_xParent                   (                                                   )
        ,   m_xContainerWindow          (                                                   )
        ,   m_xComponentWindow          (                                                   )
        ,   m_xController               (                                                   )
        ,   m_eActiveState              ( E_INACTIVE                                        )
        ,   m_sName                     (                                                   )
        ,   m_bIsFrameTop               ( sal_True                                          ) 
        ,   m_bConnected                ( sal_False                                         ) 
        ,   m_nExternalLockCount        ( 0                                                 )
        ,   m_bSelfClose                ( sal_False                                         ) 
        ,   m_bIsHidden                 ( sal_True                                          )
        ,   m_xTitleHelper              (                                                   )
        ,   m_pWindowCommandDispatch    ( 0                                                 )
        ,   m_aChildFrameContainer      (                                                   )
{
}

void Frame::initListeners()
{
    css::uno::Reference< css::uno::XInterface > xThis(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY_THROW);

    
    
    
    
    DispatchProvider* pDispatchHelper = new DispatchProvider( m_xContext, this );
    css::uno::Reference< css::frame::XDispatchProvider > xDispatchProvider( static_cast< ::cppu::OWeakObject* >(pDispatchHelper), css::uno::UNO_QUERY );

    
    DispatchInformationProvider* pInfoHelper = new DispatchInformationProvider(m_xContext, this);
    m_xDispatchInfoHelper = css::uno::Reference< css::frame::XDispatchInformationProvider >( static_cast< ::cppu::OWeakObject* >(pInfoHelper), css::uno::UNO_QUERY );

    
    
    
    
    
    InterceptionHelper* pInterceptionHelper = new InterceptionHelper( this, xDispatchProvider );
    m_xDispatchHelper = css::uno::Reference< css::frame::XDispatchProvider >( static_cast< ::cppu::OWeakObject* >(pInterceptionHelper), css::uno::UNO_QUERY );

    
    
    
    
    
    OFrames* pFramesHelper = new OFrames( this, &m_aChildFrameContainer );
    m_xFramesHelper = css::uno::Reference< css::frame::XFrames >( static_cast< ::cppu::OWeakObject* >(pFramesHelper), css::uno::UNO_QUERY );

    
    
    
    OpenFileDropTargetListener* pDropListener = new OpenFileDropTargetListener( m_xContext, this );
    m_xDropTargetListener = css::uno::Reference< css::datatransfer::dnd::XDropTargetListener >( static_cast< ::cppu::OWeakObject* >(pDropListener), css::uno::UNO_QUERY );

    
    
    SAL_WARN_IF( !xDispatchProvider.is(), "fwk", "Frame::Frame(): Slowest slave for dispatch- and interception helper isn't valid. XDispatchProvider, XDispatch, XDispatchProviderInterception are not full supported!" );
    SAL_WARN_IF( !m_xDispatchHelper.is(), "fwk", "Frame::Frame(): Interception helper isn't valid. XDispatchProvider, XDispatch, XDispatchProviderInterception are not full supported!" );
    SAL_WARN_IF( !m_xFramesHelper.is(), "fwk", "Frame::Frame(): Frames helper isn't valid. XFrames, XIndexAccess and XElementAcces are not supported!" );
    SAL_WARN_IF( !m_xDropTargetListener.is(), "fwk", "Frame::Frame(): DropTarget helper isn't valid. Drag and drop without functionality!" );

    
    
    m_aCommandOptions.EstablisFrameCallback(this);

    
    
    
    m_xLayoutManager = css::frame::LayoutManager::create(m_xContext);

    
    
    impl_initializePropInfo();
}

/*-****************************************************************************************************
    @short      standard destructor
    @descr      This one do NOTHING! Use dispose() instaed of this.

    @seealso    method dispose()

    @param      -
    @return     -

    @onerror    -
*
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

    @seealso    -

    @param      "sURL"              , URL, which represant the content
    @param      "sTargetFrameName"  , name of target frame or special value like "_self", "_blank" ...
    @param      "nSearchFlags"      , optional arguments for frame search, if target isn't a special one
    @param      "lArguments"        , optional arguments for loading
    @return     A valid component reference, if loading was successfully.
                A null reference otherwise.

    @onerror    We return a null reference.
    @threadsafe yes
*
css::uno::Reference< css::lang::XComponent > SAL_CALL Frame::loadComponentFromURL( const OUString&                                 sURL            ,
                                                                                   const OUString&                                 sTargetFrameName,
                                                                                         sal_Int32                                        nSearchFlags    ,
                                                                                   const css::uno::Sequence< css::beans::PropertyValue >& lArguments      ) throw( css::io::IOException                ,
                                                                                                                                                                   css::lang::IllegalArgumentException ,
                                                                                                                                                                   css::uno::RuntimeException          )
{
    {
        
        
        
        

        TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );
    }

    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::frame::XComponentLoader > xThis(static_cast< css::frame::XComponentLoader* >(this), css::uno::UNO_QUERY);
    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;
    aReadLock.unlock();

    return LoadEnv::loadComponentFromURL(xThis, xContext, sURL, sTargetFrameName, nSearchFlags, lArguments);
}

/*-****************************************************************************************************
    @short      return access to append or remove children on desktop
    @descr      We don't implement these interface directly. We use a helper class to do this.
                If you wish to add or delete children to/from the container, call these method to get
                a reference to the helper.

    @seealso    class OFrames

    @param      -
    @return     A reference to the helper which answer your queries.

    @onerror    A null reference is returned.
*
css::uno::Reference< css::frame::XFrames > SAL_CALL Frame::getFrames() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    

/*TODO
    This is a temp. HACK!
    Our parent (a Task!) stand in close/dispose and set working mode to E_BEFOERECLOSE
    and call dispose on us! We tra to get this xFramesHelper and are reject by an "already closed" pranet instance ....
    => We use SOFTEXCEPTIONS here ... but we should make it right in further times ....
 */

    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );

    
    
    return m_xFramesHelper;
}

/*-****************************************************************************************************
    @short      get the current active child frame
    @descr      It must be a frameto. Direct children of a frame are frames only! No task or desktop is accepted.
                We don't save this information directly in this class. We use ouer container-helper
                to do that.

    @seealso    class OFrameContainer
    @seealso    method setActiveFrame()

    @param      -
    @return     A reference to ouer current active childframe, if anyone exist.
    @return     A null reference, if nobody is active.

    @onerror    A null reference is returned.
*
css::uno::Reference< css::frame::XFrame > SAL_CALL Frame::getActiveFrame() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );

    
    
    return m_aChildFrameContainer.getActive();
}

/*-****************************************************************************************************
    @short      set the new active direct child frame
    @descr      It must be a frame to. Direct children of frame are frames only! No task or desktop is accepted.
                We don't save this information directly in this class. We use ouer container-helper
                to do that.

    @seealso    class OFrameContainer
    @seealso    method getActiveFrame()

    @param      "xFrame", reference to new active child. It must be an already existing child!
    @return     -

    @onerror    An assertion is thrown and element is ignored, if given frame is'nt already a child of us.
*
void SAL_CALL Frame::setActiveFrame( const css::uno::Reference< css::frame::XFrame >& xFrame ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    SAL_WARN_IF( implcp_setActiveFrame( xFrame ), "fwk", "Frame::setActiveFrame(): Invalid parameter detected!" );
    
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    
    
    
    css::uno::Reference< css::frame::XFrame > xActiveChild = m_aChildFrameContainer.getActive();
    EActiveState                              eActiveState = m_eActiveState             ;

    aWriteLock.unlock();
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */

    
    
    if( xActiveChild != xFrame )
    {
        
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
        
        
        if( eActiveState == E_FOCUS )
        {
            aWriteLock.lock();
            eActiveState   = E_ACTIVE    ;
            m_eActiveState = eActiveState;
            aWriteLock.unlock();
            implts_sendFrameActionEvent( css::frame::FrameAction_FRAME_UI_DEACTIVATING );
        }

        
        
        
        if  (
                ( eActiveState          ==  E_ACTIVE    )   &&
                ( xFrame->isActive()    ==  sal_False   )
            )
        {
            xFrame->activate();
        }
    }
    else
    
    if( eActiveState == E_ACTIVE )
    {
        aWriteLock.lock();
        eActiveState   = E_FOCUS     ;
        m_eActiveState = eActiveState;
        aWriteLock.unlock();
        implts_sendFrameActionEvent( css::frame::FrameAction_FRAME_UI_ACTIVATED );
    }
}

/*-****************************************************************************************************
   initialize new created layout manager
**/
void lcl_enableLayoutManager(const css::uno::Reference< css::frame::XLayoutManager2 >& xLayoutManager,
                             const css::uno::Reference< css::frame::XFrame >&         xFrame        )
{
    
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
    @return     -

    @onerror    We do nothing.
*
void SAL_CALL Frame::initialize( const css::uno::Reference< css::awt::XWindow >& xWindow ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    if (!xWindow.is())
        throw css::uno::RuntimeException(
                    OUString("Frame::initialize() called without a valid container window reference."),
                    static_cast< css::frame::XFrame* >(this));

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    if ( m_xContainerWindow.is() )
        throw css::uno::RuntimeException(
                OUString("Frame::initialized() is called more then once, which isnt useful nor allowed."),
                static_cast< css::frame::XFrame* >(this));

    
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    
    m_aTransactionManager.setWorkingMode( E_WORK );

    
    
    
    SAL_WARN_IF( m_xContainerWindow.is(), "fwk", "Frame::initialize(): Leak detected! This state should never occur ..." );
    m_xContainerWindow = xWindow;

    
    Window* pWindow = VCLUnoHelper::GetWindow(xWindow);
    if (pWindow && pWindow->IsVisible())
        m_bIsHidden = sal_False;

    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;
    css::uno::Reference< css::frame::XLayoutManager2 >  xLayoutManager = m_xLayoutManager;

    
    
    aWriteLock.unlock();
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */

    if (xLayoutManager.is())
        lcl_enableLayoutManager(xLayoutManager, this);

    
    css::uno::Reference< css::frame::XFrame >                 xThis            (static_cast< css::frame::XFrame* >(this)                        , css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::task::XStatusIndicatorFactory > xIndicatorFactory =
        css::task::StatusIndicatorFactory::createWithFrame(xContext, xThis, sal_False/*DisableReschedule*/, sal_True/*AllowParentShow*/ );

    
    aWriteLock.lock();
    m_xIndicatorFactoryHelper = xIndicatorFactory;
    aWriteLock.unlock();
    

    
    
    implts_startWindowListening();

    impl_enablePropertySet();

    m_pWindowCommandDispatch = new WindowCommandDispatch(xContext, this);

    
    TitleHelper* pTitleHelper = new TitleHelper( xContext );
    m_xTitleHelper = css::uno::Reference< css::frame::XTitle >(static_cast< ::cppu::OWeakObject* >(pTitleHelper), css::uno::UNO_QUERY_THROW);
    pTitleHelper->setOwner(xThis);
}

/*-****************************************************************************************************
    @short      returns current set container window
    @descr      The ContainerWindow property is used as a container for the component
                in this frame. So this object implements a container interface too.
                The instantiation of the container window is done by the user of this class.
                The frame is the owner of its container window.

    @seealso    method initialize()

    @param      -
    @return     A reference to current set containerwindow.

    @onerror    A null reference is returned.
*
css::uno::Reference< css::awt::XWindow > SAL_CALL Frame::getContainerWindow() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );

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
*
void SAL_CALL Frame::setCreator( const css::uno::Reference< css::frame::XFramesSupplier >& xCreator ) throw( css::uno::RuntimeException )
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE { */
        WriteGuard aWriteLock( m_aLock );
            m_xParent = xCreator;
        aWriteLock.unlock();
    /* } SAFE */

    css::uno::Reference< css::frame::XDesktop > xIsDesktop( xCreator, css::uno::UNO_QUERY );
    m_bIsFrameTop = ( xIsDesktop.is() || ! xCreator.is() );
}

/*-****************************************************************************************************
    @short      returns current parent frame
    @descr      The Creator is the parent frame container. If it is NULL, the frame is the uppermost one.

    @seealso    method setCreator()

    @param      -
    @return     A reference to current set parent frame container.

    @onerror    A null reference is returned.
*
css::uno::Reference< css::frame::XFramesSupplier > SAL_CALL Frame::getCreator() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );

    return m_xParent;
}

/*-****************************************************************************************************
    @short      returns current set name of frame
    @descr      This name is used to find target of findFrame() or queryDispatch() calls.

    @seealso    method setName()

    @param      -
    @return     Current set name of frame.

    @onerror    An empty string is returned.
*
OUString SAL_CALL Frame::getName() throw( css::uno::RuntimeException )
{
    /* SAFE { */
    ReadGuard aReadLock( m_aLock );
    return m_sName;
    /* } SAFE */
}

/*-****************************************************************************************************
    @short      set new name for frame
    @descr      This name is used to find target of findFrame() or queryDispatch() calls.

    @attention  Special names like "_blank", "_self" aren't allowed ...
                "_beamer" or "_menubar" excepts this rule!

    @seealso    method getName()

    @param      "sName", new frame name.
    @return     -

    @onerror    We do nothing.
*
void SAL_CALL Frame::setName( const OUString& sName ) throw( css::uno::RuntimeException )
{
    /* SAFE { */
    WriteGuard aWriteLock( m_aLock );
    
    
    if (TargetHelper::isValidNameForFrame(sName))
        m_sName = sName;
    aWriteLock.unlock();
    /* } SAFE */
}

/*-****************************************************************************************************
    @short      search for frames
    @descr      This method searches for a frame with the specified name.
                Frames may contain other frames (e.g. a frameset) and may
                be contained in other frames. This hierarchie is searched by
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
*
css::uno::Reference< css::frame::XFrame > SAL_CALL Frame::findFrame( const OUString&  sTargetFrameName,
                                                                           sal_Int32         nSearchFlags    ) throw( css::uno::RuntimeException )
{
    css::uno::Reference< css::frame::XFrame > xTarget;

    
    
    
    
    
    
    
    if (
        (sTargetFrameName==SPECIALTARGET_DEFAULT  )   ||    
        (sTargetFrameName==SPECIALTARGET_MENUBAR  )         
       )
    {
        return NULL;
    }

    
    
    
    

    
    /* SAFE { */
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::frame::XFrame >              xParent      ( m_xParent, css::uno::UNO_QUERY );
    css::uno::Reference< css::uno::XComponentContext >     xContext = m_xContext;
    sal_Bool                                               bIsTopFrame  = m_bIsFrameTop;
    sal_Bool                                               bIsTopWindow = WindowHelper::isTopWindow(m_xContainerWindow);
    aReadLock.unlock();
    /* } SAFE */

    
    
    
    
    
    if ( sTargetFrameName==SPECIALTARGET_BLANK )
    {
        TaskCreator aCreator(xContext);
        xTarget = aCreator.createTask(sTargetFrameName,sal_False);
    }

    
    
    
    
    
    else if ( sTargetFrameName==SPECIALTARGET_PARENT )
    {
        xTarget = xParent;
    }

    
    
    
    
    
    else if ( sTargetFrameName==SPECIALTARGET_TOP )
    {
        if (bIsTopFrame)
            xTarget = this;
        else if (xParent.is()) 
            xTarget = xParent->findFrame(SPECIALTARGET_TOP,0);
    }

    
    
    
    
    else if (
        ( sTargetFrameName==SPECIALTARGET_SELF ) ||
        ( sTargetFrameName.isEmpty()           )
       )
    {
        xTarget = this;
    }

    
    
    
    
    
    
    else if ( sTargetFrameName==SPECIALTARGET_BEAMER )
    {
        
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
        
        else if (xParent.is())
            xTarget = xParent->findFrame(SPECIALTARGET_BEAMER,0);
    }

    else
    {
        
        
        
        
        
        
        

        
        /* SAFE { */
        aReadLock.lock();
        OUString sOwnName = m_sName;
        aReadLock.unlock();
        /* } SAFE */

        
        
        
        
        
        if (
            (nSearchFlags &  css::frame::FrameSearchFlag::SELF)  &&
            (sOwnName     == sTargetFrameName                 )
           )
        {
            xTarget = this;
        }

        
        
        
        
        
        
        
        if (
            ( ! xTarget.is()                                     ) &&
            (nSearchFlags & css::frame::FrameSearchFlag::CHILDREN)
           )
        {
            xTarget = m_aChildFrameContainer.searchOnAllChildrens(sTargetFrameName);
        }

        
        
        
        
        
        
        
        if (
            (   bIsTopFrame && (nSearchFlags & css::frame::FrameSearchFlag::TASKS) )   ||
            ( ! bIsTopFrame                                                        )
           )
        {
            
            
            
            
            
            
            
            
            
            
            
            
            
            if (
                ( ! xTarget.is()                                      ) &&
                (nSearchFlags &  css::frame::FrameSearchFlag::SIBLINGS) &&
                (   xParent.is()                                      ) 
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
                                ( !(xContainer->getByIndex(i)>>=xSibling)                                 ) ||  
                                ( ! xSibling.is()                                     ) ||  
                                ( xSibling==static_cast< ::cppu::OWeakObject* >(this) )     
                            )
                            {
                                continue;
                            }

                            
                            
                            sal_Int32 nRightFlags = css::frame::FrameSearchFlag::SELF;
                            if (nSearchFlags & css::frame::FrameSearchFlag::CHILDREN)
                                nRightFlags |= css::frame::FrameSearchFlag::CHILDREN;
                            xTarget = xSibling->findFrame(sTargetFrameName, nRightFlags );
                            
                            if (xTarget.is())
                                break;
                        }
                    }
                }
            }

            
            
            
            
            
            
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

        
        
        
        
        
        if (
            ( ! xTarget.is()                                   )    &&
            (nSearchFlags & css::frame::FrameSearchFlag::CREATE)
           )
        {
            TaskCreator aCreator(xContext);
            xTarget = aCreator.createTask(sTargetFrameName,sal_False);
        }
    }

    return xTarget;
}

/*-****************************************************************************************************
    @short      -
    @descr      Returns sal_True, if this frame is a "top frame", otherwise sal_False.
                The "m_bIsFrameTop" member must be set in the ctor or setCreator() method.
                A top frame is a member of the top frame container or a member of the
                task frame container. Both containers can create new frames if the findFrame()
                method of their css::frame::XFrame interface is called with a frame name not yet known.

    @seealso    ctor
    @seealso    method setCreator()
    @seealso    method findFrame()

    @param      -
    @return     true, if is it a top frame ... false otherwise.

    @onerror    No error should occur!
*
sal_Bool SAL_CALL Frame::isTop() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );

    
    
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

    @param      -
    @return     -

    @onerror    -
*
void SAL_CALL Frame::activate() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    
    
    
    
    
    css::uno::Reference< css::frame::XFrame >           xActiveChild    = m_aChildFrameContainer.getActive()                                ;
    css::uno::Reference< css::frame::XFramesSupplier >  xParent         ( m_xParent, css::uno::UNO_QUERY )                                ;
    css::uno::Reference< css::frame::XFrame >           xThis           ( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );
    css::uno::Reference< css::awt::XWindow >            xComponentWindow( m_xComponentWindow, css::uno::UNO_QUERY )                       ;
    EActiveState                                        eState          = m_eActiveState                                                  ;

    aWriteLock.unlock();
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */

    
    
    if( eState == E_INACTIVE )
    {
        
        aWriteLock.lock();
        eState         = E_ACTIVE;
        m_eActiveState = eState;
        aWriteLock.unlock();
        
        if( xParent.is() )
        {
            
            
            

            
            
            

            xParent->setActiveFrame( xThis );

            
            
            
            xParent->activate();
        }
        
        
        
        implts_sendFrameActionEvent( css::frame::FrameAction_FRAME_ACTIVATED );
    }

    
    
    
    
    
    if ( eState == E_ACTIVE && xActiveChild.is() && !xActiveChild->isActive() )
    {
        xActiveChild->activate();
    }

    
    
    if ( eState == E_ACTIVE && !xActiveChild.is() )
    {
        aWriteLock.lock();
        eState         = E_FOCUS;
        m_eActiveState = eState;
        aWriteLock.unlock();
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

    @param      -
    @return     -

    @onerror    -
*
void SAL_CALL Frame::deactivate() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    
    css::uno::Reference< css::frame::XFrame >           xActiveChild    = m_aChildFrameContainer.getActive()                                     ;
    css::uno::Reference< css::frame::XFramesSupplier >  xParent         ( m_xParent, css::uno::UNO_QUERY )                                ;
    css::uno::Reference< css::frame::XFrame >           xThis           ( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );
    EActiveState                                        eState          = m_eActiveState                                                  ;

    aWriteLock.unlock();
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */

    
    if( eState != E_INACTIVE )
    {
        
        
        if ( xActiveChild.is() && xActiveChild->isActive() )
        {
            xActiveChild->deactivate();
        }

        
        
        if( eState == E_FOCUS )
        {
            
            
            aWriteLock.lock();
            eState          = E_ACTIVE;
            m_eActiveState  = eState  ;
            aWriteLock.unlock();
            implts_sendFrameActionEvent( css::frame::FrameAction_FRAME_UI_DEACTIVATING );
        }

        
        
        if( eState == E_ACTIVE )
        {
            
            aWriteLock.lock();
            eState          = E_INACTIVE;
            m_eActiveState  = eState    ;
            aWriteLock.unlock();
            implts_sendFrameActionEvent( css::frame::FrameAction_FRAME_DEACTIVATING );
        }

        
        
        
        
        
        if ( xParent.is() && xParent->getActiveFrame() == xThis )
        {
            
            
            
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

    @param      -
    @return     true if active, false otherwise.

    @onerror    No error should occur.
*
sal_Bool SAL_CALL Frame::isActive() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );

    return  (
                ( m_eActiveState    ==  E_ACTIVE    )   ||
                ( m_eActiveState    ==  E_FOCUS     )
            );
}

/*-****************************************************************************************************
    @short      ???
    @descr      -

    @seealso    -

    @param      -
    @return     -

    @onerror    -
*
void SAL_CALL Frame::contextChanged() throw( css::uno::RuntimeException )
{
    
    
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );
    
    
    implts_sendFrameActionEvent( css::frame::FrameAction_CONTEXT_CHANGED );
}

/*-****************************************************************************************************
    @short      set new component inside the frame
    @descr      A frame is a container for a component. Use this method to set, change or realease it!
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
*
sal_Bool SAL_CALL Frame::setComponent(  const   css::uno::Reference< css::awt::XWindow >&       xComponentWindow ,
                                        const   css::uno::Reference< css::frame::XController >& xController      ) throw( css::uno::RuntimeException )
{
    
    
    
    if  ( xController.is() && ! xComponentWindow.is() )
        return sal_True;

    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    
    
    /* SAFE { */
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::awt::XWindow >       xContainerWindow    = m_xContainerWindow;
    css::uno::Reference< css::awt::XWindow >       xOldComponentWindow = m_xComponentWindow;
    css::uno::Reference< css::frame::XController > xOldController      = m_xController;
    Window*                                        pOwnWindow = VCLUnoHelper::GetWindow( xContainerWindow );
    sal_Bool                                       bHadFocus           = pOwnWindow->HasChildPathFocus();
    sal_Bool                                       bWasConnected       = m_bConnected;
    aReadLock.unlock();
    /* } SAFE */

    
    
    
    
    
    implts_stopWindowListening();

    
    if (bWasConnected)
        implts_sendFrameActionEvent( css::frame::FrameAction_COMPONENT_DETACHING );

    
    
    
    
    
    if (
        (xOldController.is()          )   &&
        (xOldController != xController)
       )
    {
        /* ATTENTION
            Don't suspend the old controller here. Because the outside caller must do that
            by definition. We have to dispose it here only.
         */

        
        
        /* SAFE {*/
        WriteGuard aWriteLock( m_aLock );
        m_xController = NULL;
        aWriteLock.unlock();
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
        xOldController = NULL;
    }

    
    
    
    
    
    
    if (
        (xOldComponentWindow.is()               )   &&
        (xOldComponentWindow != xComponentWindow)
       )
    {
        /* SAFE { */
        WriteGuard aWriteLock( m_aLock );
        m_xComponentWindow = NULL;
        aWriteLock.unlock();
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
        xOldComponentWindow = NULL;
    }

    
    
    
    /* SAFE { */
    WriteGuard aWriteLock( m_aLock );
    m_xComponentWindow = xComponentWindow;
    m_xController      = xController     ;
    m_bConnected       = (m_xComponentWindow.is() || m_xController.is());
    sal_Bool bIsConnected       = m_bConnected;
    aWriteLock.unlock();
    /* } SAFE */

    
    
    if (bIsConnected && bWasConnected)
        implts_sendFrameActionEvent( css::frame::FrameAction_COMPONENT_REATTACHED );
    else if (bIsConnected && !bWasConnected)
        implts_sendFrameActionEvent( css::frame::FrameAction_COMPONENT_ATTACHED   );

    
    
    
    if (
        (bHadFocus            ) &&
        (xComponentWindow.is())
       )
    {
        xComponentWindow->setFocus();
    }

    
    
    implts_resizeComponentWindow();
    
    implts_setIconOnWindow();
    
    implts_startWindowListening();

    /* SAFE { */
    aWriteLock.lock();
    impl_checkMenuCloser();
    aWriteLock.unlock();
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

    @param      -
    @return     css::uno::Reference to current set component window.

    @onerror    A null reference is returned.
*
css::uno::Reference< css::awt::XWindow > SAL_CALL Frame::getComponentWindow() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );

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

    @param      -
    @return     css::uno::Reference to current set controller.

    @onerror    A null reference is returned.
*
css::uno::Reference< css::frame::XController > SAL_CALL Frame::getController() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );

    return m_xController;
}

/*-****************************************************************************************************
    @short      add/remove listener for activate/deactivate/contextChanged events
    @descr      -

    @seealso    method activate()
    @seealso    method deactivate()
    @seealso    method contextChanged()

    @param      "xListener" reference to your listener object
    @return     -

    @onerror    Listener is ignored.
*
void SAL_CALL Frame::addFrameActionListener( const css::uno::Reference< css::frame::XFrameActionListener >& xListener ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    SAL_WARN_IF( implcp_addFrameActionListener( xListener ), "fwk", "Frame::addFrameActionListener(): Invalid parameter detected." );
    
    
    
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    m_aListenerContainer.addInterface( ::getCppuType( (const css::uno::Reference< css::frame::XFrameActionListener >*)NULL ), xListener );
}


void SAL_CALL Frame::removeFrameActionListener( const css::uno::Reference< css::frame::XFrameActionListener >& xListener ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    SAL_WARN_IF( implcp_removeFrameActionListener( xListener ), "fwk", "Frame::removeFrameActionListener(): Invalid parameter detected." );
    
    
    
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    m_aListenerContainer.removeInterface( ::getCppuType( (const css::uno::Reference< css::frame::XFrameActionListener >*)NULL ), xListener );
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
*
void SAL_CALL Frame::close( sal_Bool bDeliverOwnership ) throw( css::util::CloseVetoException,
                                                                css::uno::RuntimeException   )
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    
    
    
    css::uno::Reference< css::uno::XInterface > xSelfHold( static_cast< ::cppu::OWeakObject* >(this) );

    
    
    
    
    css::lang::EventObject             aSource    (static_cast< ::cppu::OWeakObject*>(this));
    ::cppu::OInterfaceContainerHelper* pContainer = m_aListenerContainer.getContainer( ::getCppuType( ( const css::uno::Reference< css::util::XCloseListener >*) NULL ) );
    if (pContainer!=NULL)
    {
        ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
        while (pIterator.hasMoreElements())
        {
            try
            {
                ((css::util::XCloseListener*)pIterator.next())->queryClosing( aSource, bDeliverOwnership );
            }
            catch( const css::uno::RuntimeException& )
            {
                pIterator.remove();
            }
        }
    }

    
    
    if (isActionLocked())
    {
        if (bDeliverOwnership)
        {
            /* SAFE */
            WriteGuard aWriteLock( m_aLock );
            m_bSelfClose = sal_True;
            aWriteLock.unlock();
            /* SAFE */
        }

        throw css::util::CloseVetoException("Frame in use for loading document ...",static_cast< ::cppu::OWeakObject*>(this));
    }

    if ( ! setComponent(NULL,NULL) )
        throw css::util::CloseVetoException("Component couldn't be deattached ...",static_cast< ::cppu::OWeakObject*>(this));

    
    pContainer = m_aListenerContainer.getContainer( ::getCppuType( ( const css::uno::Reference< css::util::XCloseListener >*) NULL ) );
    if (pContainer!=NULL)
    {
        ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
        while (pIterator.hasMoreElements())
        {
            try
            {
                ((css::util::XCloseListener*)pIterator.next())->notifyClosing( aSource );
            }
            catch( const css::uno::RuntimeException& )
            {
                pIterator.remove();
            }
        }
    }

    /* SAFE { */
    WriteGuard aWriteLock( m_aLock );
    m_bIsHidden = sal_True;
    aWriteLock.unlock();
    /* } SAFE */
    impl_checkMenuCloser();

    
    
    aTransaction.stop();
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
*
void SAL_CALL Frame::addCloseListener( const css::uno::Reference< css::util::XCloseListener >& xListener ) throw (css::uno::RuntimeException)
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );
    
    
    m_aListenerContainer.addInterface( ::getCppuType( ( const css::uno::Reference< css::util::XCloseListener >* ) NULL ), xListener );
}


void SAL_CALL Frame::removeCloseListener( const css::uno::Reference< css::util::XCloseListener >& xListener ) throw (css::uno::RuntimeException)
{
    
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );
    
    
    m_aListenerContainer.removeInterface( ::getCppuType( ( const css::uno::Reference< css::util::XCloseListener >* ) NULL ), xListener );
}


OUString SAL_CALL Frame::getTitle()
    throw (css::uno::RuntimeException)
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::frame::XTitle > xTitle(m_xTitleHelper, css::uno::UNO_QUERY_THROW);
    aReadLock.unlock();
    

    return xTitle->getTitle();
}


void SAL_CALL Frame::setTitle( const OUString& sTitle )
    throw (css::uno::RuntimeException)
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::frame::XTitle > xTitle(m_xTitleHelper, css::uno::UNO_QUERY_THROW);
    aReadLock.unlock();
    

    xTitle->setTitle(sTitle);
}


void SAL_CALL Frame::addTitleChangeListener( const css::uno::Reference< css::frame::XTitleChangeListener >& xListener)
    throw (css::uno::RuntimeException)
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::frame::XTitleChangeBroadcaster > xTitle(m_xTitleHelper, css::uno::UNO_QUERY_THROW);
    aReadLock.unlock();
    

    xTitle->addTitleChangeListener(xListener);
}


void SAL_CALL Frame::removeTitleChangeListener( const css::uno::Reference< css::frame::XTitleChangeListener >& xListener )
    throw (css::uno::RuntimeException)
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::frame::XTitleChangeBroadcaster > xTitle(m_xTitleHelper, css::uno::UNO_QUERY_THROW);
    aReadLock.unlock();
    

    xTitle->removeTitleChangeListener(xListener);
}

css::uno::Reference<css::container::XNameContainer> SAL_CALL Frame::getUserDefinedAttributes() throw (css::uno::RuntimeException)
{
    
    return 0;
}

css::uno::Reference<css::frame::XDispatchRecorderSupplier> SAL_CALL Frame::getDispatchRecorderSupplier() throw (css::uno::RuntimeException)
{
    ReadGuard aReadLock( m_aLock );
    return m_xDispatchRecorderSupplier;
}

void SAL_CALL Frame::setDispatchRecorderSupplier(const css::uno::Reference<css::frame::XDispatchRecorderSupplier>& p) throw (css::uno::RuntimeException)
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE { */
        WriteGuard aWriteLock( m_aLock );
            m_xDispatchRecorderSupplier.set(p);
        aWriteLock.unlock();
    /* } SAFE */
}

css::uno::Reference<css::uno::XInterface> SAL_CALL Frame::getLayoutManager() throw (css::uno::RuntimeException)
{
    ReadGuard aReadLock( m_aLock );
    return m_xLayoutManager;
}

void SAL_CALL Frame::setLayoutManager(const css::uno::Reference<css::uno::XInterface>& p1) throw (css::uno::RuntimeException)
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE { */
        WriteGuard aWriteLock( m_aLock );
            m_xLayoutManager.set(p1, css::uno::UNO_QUERY);
        aWriteLock.unlock();
    /* } SAFE */
}


/*-****************************************************************************************************/
void Frame::implts_forgetSubFrames()
{
    
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::container::XIndexAccess > xContainer(m_xFramesHelper, css::uno::UNO_QUERY_THROW);
    aReadLock.unlock();
    

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
            
            
        }
    }

    
    WriteGuard aWriteLock(m_aLock);
    m_xFramesHelper.clear(); 
    m_aChildFrameContainer.clear(); 
    aWriteLock.unlock();
    
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

    @param      -
    @return     -

    @onerror    -
*
void SAL_CALL Frame::dispose() throw( css::uno::RuntimeException )
{
    
    
    
    css::uno::Reference< css::frame::XFrame > xThis( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );

    SAL_INFO("fwk.frame", "[Frame] " << m_sName << " send dispose event to listener");

    
    
    
    implts_stopWindowListening();

    if (m_xLayoutManager.is())
        lcl_disableLayoutManager(m_xLayoutManager, this);

    delete m_pWindowCommandDispatch;

    
    css::lang::EventObject aEvent( xThis );
    m_aListenerContainer.disposeAndClear( aEvent );

    
    impl_disablePropertySet();

    
    
    css::uno::Reference< css::lang::XEventListener > xDispatchHelper(m_xDispatchHelper, css::uno::UNO_QUERY_THROW);
    xDispatchHelper->disposing(aEvent);
    xDispatchHelper.clear();

    
    
    
    m_aTransactionManager.setWorkingMode( E_BEFORECLOSE );

    
    
    
    
    
    
    
    
    Application::DialogCancelMode old = Application::GetDialogCancelMode();
    Application::SetDialogCancelMode( Application::DIALOG_CANCEL_SILENT );

    
    

    
    
    
    
    
    
    if( m_xParent.is() )
    {
        m_xParent->getFrames()->remove( xThis );
        m_xParent = css::uno::Reference< css::frame::XFramesSupplier >();
    }

    /* } SAFE */
    
    
    
    
    
    
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

    
    
    m_xDispatchHelper.clear();
    m_xContext.clear();
    m_xDropTargetListener.clear();
    m_xDispatchRecorderSupplier.clear();
    m_xLayoutManager.clear();
    m_xIndicatorFactoryHelper.clear();

    
    
    
    m_eActiveState       = E_INACTIVE;
    m_sName              = OUString();
    m_bIsFrameTop        = sal_False;
    m_bConnected         = sal_False;
    m_nExternalLockCount = 0;
    m_bSelfClose         = sal_False;
    m_bIsHidden          = sal_True;

    
    m_aTransactionManager.setWorkingMode( E_CLOSE );

    
    
    Application::SetDialogCancelMode( old );
}

/*-****************************************************************************************************
    @short      Be a listener for dispose events!
    @descr      Adds/remove an EventListener to this object. If the dispose method is called on
                this object, the disposing method of the listener is called.

    @seealso    -

    @param      "xListener" reference to your listener object.
    @return     -

    @onerror    Listener is ignored.
*
void SAL_CALL Frame::addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    SAL_WARN_IF( implcp_addEventListener( xListener ), "fwk", "Frame::addEventListener(): Invalid parameter detected." );
    
    
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    m_aListenerContainer.addInterface( ::getCppuType( ( const css::uno::Reference< css::lang::XEventListener >* ) NULL ), xListener );
}


void SAL_CALL Frame::removeEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    SAL_WARN_IF( implcp_removeEventListener( xListener ), "fwk", "Frame::removeEventListener(): Invalid parameter detected." );
    
    
    
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    m_aListenerContainer.removeInterface( ::getCppuType( ( const css::uno::Reference< css::lang::XEventListener >* ) NULL ), xListener );
}

/*-****************************************************************************************************
    @short      create new status indicator
    @descr      Use returned status indicator to show progresses and some text information.
                All created objects share the same dialog! Only the last one can show his information.

    @seealso    class StatusIndicatorFactory
    @seealso    class StatusIndicator

    @param      -
    @return     A reference to created object.

    @onerror    We return a null reference.
*
css::uno::Reference< css::task::XStatusIndicator > SAL_CALL Frame::createStatusIndicator() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA ----------------------------------------------------------------------------------------- */
    
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );

    
    css::uno::Reference< css::task::XStatusIndicator >        xExternal(m_xIndicatorInterception.get(), css::uno::UNO_QUERY);
    css::uno::Reference< css::task::XStatusIndicatorFactory > xFactory = m_xIndicatorFactoryHelper;

    aReadLock.unlock();
    /* UNSAFE AREA ----------------------------------------------------------------------------------------- */

    
    if (xExternal.is())
        return xExternal;

    
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
    @param      "nSearchFlags"      , additional flags to regulate search if sTargetFrameName isn't clear
    @return     css::uno::Reference to dispatch handler.

    @onerror    A null reference is returned.
*
css::uno::Reference< css::frame::XDispatch > SAL_CALL Frame::queryDispatch( const css::util::URL&   aURL            ,
                                                                            const OUString&  sTargetFrameName,
                                                                                  sal_Int32         nSearchFlags    ) throw( css::uno::RuntimeException )
{
    const char UNO_PROTOCOL[] = ".uno:";

    
    
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    
    
    OUString aCommand( aURL.Main );
    if ( aURL.Protocol.equalsIgnoreAsciiCaseAsciiL( UNO_PROTOCOL, sizeof( UNO_PROTOCOL )-1 ))
        aCommand = aURL.Path;

    
    if ( m_aCommandOptions.Lookup( SvtCommandOptions::CMDOPTION_DISABLED, aCommand ) )
        return css::uno::Reference< css::frame::XDispatch >();
    else
    {
        
        
        return m_xDispatchHelper->queryDispatch( aURL, sTargetFrameName, nSearchFlags );
    }
}

/*-****************************************************************************************************
    @short      handle more then ones dispatches at same call
    @descr      Returns a sequence of dispatches. For details see the queryDispatch method.
                For failed dispatches we return empty items in list!

    @seealso    method queryDispatch()

    @param      "lDescriptor" list of dispatch arguments for queryDispatch()!
    @return     List of dispatch references. Some elements can be NULL!

    @onerror    An empty list is returned.
*
css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL Frame::queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptor ) throw( css::uno::RuntimeException )
{
    
    
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    
    
    return m_xDispatchHelper->queryDispatches( lDescriptor );
}

/*-****************************************************************************************************
    @short      register/unregister interceptor for dispatch calls
    @descr      If you wish to handle some dispatches by himself ... you should be
                an interceptor for it. Please see class OInterceptionHelper for further information.

    @seealso    class OInterceptionHelper

    @param      "xInterceptor", reference to your interceptor implementation.
    @return     -

    @onerror    Interceptor is ignored.
*
void SAL_CALL Frame::registerDispatchProviderInterceptor( const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor ) throw( css::uno::RuntimeException )
{
    
    
    
    
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    css::uno::Reference< css::frame::XDispatchProviderInterception > xInterceptionHelper( m_xDispatchHelper, css::uno::UNO_QUERY );
    xInterceptionHelper->registerDispatchProviderInterceptor( xInterceptor );
}


void SAL_CALL Frame::releaseDispatchProviderInterceptor( const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor ) throw( css::uno::RuntimeException )
{
    
    
    
    
    
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    css::uno::Reference< css::frame::XDispatchProviderInterception > xInterceptionHelper( m_xDispatchHelper, css::uno::UNO_QUERY );
    xInterceptionHelper->releaseDispatchProviderInterceptor( xInterceptor );
}

/*-****************************************************************************************************
    @short      provides information about all possible dispatch functions
                inside the currnt frame environment
*
css::uno::Sequence< sal_Int16 > SAL_CALL Frame::getSupportedCommandGroups()
    throw(css::uno::RuntimeException)
{
    return m_xDispatchInfoHelper->getSupportedCommandGroups();
}


css::uno::Sequence< css::frame::DispatchInformation > SAL_CALL Frame::getConfigurableDispatchInformation(sal_Int16 nCommandGroup)
    throw(css::uno::RuntimeException)
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
    @return     -

    @onerror    -
*
void SAL_CALL Frame::windowResized( const css::awt::WindowEvent& aEvent ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    (void) aEvent;
    SAL_WARN_IF( implcp_windowResized( aEvent ), "fwk", "Frame::windowResized(): Invalid parameter detected." );
    
    
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    
    
    implts_resizeComponentWindow();
}


void SAL_CALL Frame::focusGained( const css::awt::FocusEvent& aEvent ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    (void) aEvent;
    SAL_WARN_IF( implcp_focusGained( aEvent ), "fwk", "Frame::focusGained(): Invalid parameter detected." );
    
    
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );
    
    css::uno::Reference< css::awt::XWindow > xComponentWindow = m_xComponentWindow;
    aReadLock.unlock();
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
    @return     -

    @onerror    -
*
void SAL_CALL Frame::windowActivated( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    (void) aEvent;
    SAL_WARN_IF( implcp_windowActivated( aEvent ), "fwk", "Frame::windowActivated(): Invalid parameter detected." );
    
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );
    
    EActiveState eState = m_eActiveState;
    aReadLock.unlock();
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    if( eState == E_INACTIVE )
    {
        setActiveFrame( css::uno::Reference< css::frame::XFrame >() );
        activate();
    }
}


void SAL_CALL Frame::windowDeactivated( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    (void) aEvent;
    SAL_WARN_IF( implcp_windowDeactivated( aEvent ), "fwk", "Frame::windowDeactivated(): Invalid parameter detected." );
    
    
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );

    css::uno::Reference< css::frame::XFrame > xParent          ( m_xParent, css::uno::UNO_QUERY );
    css::uno::Reference< css::awt::XWindow >  xContainerWindow = m_xContainerWindow;
    EActiveState                              eActiveState     = m_eActiveState    ;

    aReadLock.unlock();

    if( eActiveState != E_INACTIVE )
    {
        
        
        
        SolarMutexClearableGuard aSolarGuard;
        Window* pFocusWindow = Application::GetFocusWindow();
        if  ( xContainerWindow.is() && xParent.is() &&
              !css::uno::Reference< css::frame::XDesktop >( xParent, css::uno::UNO_QUERY ).is()
            )
        {
            css::uno::Reference< css::awt::XWindow >  xParentWindow   = xParent->getContainerWindow()             ;
            Window*                                   pParentWindow   = VCLUnoHelper::GetWindow( xParentWindow    );
            
            
            
            
            
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


void SAL_CALL Frame::windowClosing( const css::lang::EventObject& ) throw( css::uno::RuntimeException )
{
    /* #i62088#
        Some interceptor objects intercept our "internally asynchronous implemented" dispatch call.
        And they close this frame directly (means synchronous then).
        Means: Frame::windowClosing()->Frame::close()
        In such situation its not a good idea to hold this transaction count alive .-)
    */
    {
        
        TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );
        
        deactivate();
    }

    
    
    
    

    /*ATTENTION!
        Don't try to suspend the controller here! Because it's done inside used dispatch().
        Otherwhise the dialog "would you save your changes?" will be shown more then once ...
     */

    /* SAFE */
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;
    aReadLock.unlock();
    /* SAFE */

    css::util::URL aURL;
    aURL.Complete = ".uno:CloseFrame";
    css::uno::Reference< css::util::XURLTransformer > xParser(css::util::URLTransformer::create(xContext));
    xParser->parseStrict(aURL);

    css::uno::Reference< css::frame::XDispatch > xCloser = queryDispatch(aURL, SPECIALTARGET_SELF, 0);
    if (xCloser.is())
        xCloser->dispatch(aURL, css::uno::Sequence< css::beans::PropertyValue >());

    
    
    
}

/*-****************************************************************************************************
    @short      react for a show event for the internal container window
    @descr      Normaly we don't need this information really. But we can use it to
                implement the special feature "trigger first visible task".

                Algorithm: - first we have to check if we are a top (task) frame
                             It's not enough to be a top frame! Because we MUST have the desktop as parent.
                             But frames without a parent are top too. So it's not possible to check isTop() here!
                             We have to look for the type of our parent.
                           - if we are a task frame, then we have to check if we are the first one.
                             We use a static variable to do so. They will be reset to afterwards be shure
                             that further calls of this method doesn't do anything then.
                           - Then we have to trigger the right event string on the global job executor.

    @seealso    css::task::JobExecutor

    @param      aEvent
                    describes the source of this event
                    We are not interested on this information. We are interested on the visible state only.

    @threadsafe yes
*
void SAL_CALL Frame::windowShown( const css::lang::EventObject& ) throw(css::uno::RuntimeException)
{
    static sal_Bool bFirstVisibleTask = sal_True;

    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::frame::XDesktop >             xDesktopCheck( m_xParent, css::uno::UNO_QUERY );
    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;
    m_bIsHidden = sal_False;
    aReadLock.unlock();
    /* } SAFE */

    impl_checkMenuCloser();

    if (xDesktopCheck.is())
    {
        /* STATIC SAFE { */
        WriteGuard aStaticWriteLock( LockHelper::getGlobalLock() );
        sal_Bool bMustBeTriggered  = bFirstVisibleTask;
                 bFirstVisibleTask = sal_False;
        aStaticWriteLock.unlock();
        /* } STATIC SAFE */

        if (bMustBeTriggered)
        {
            css::uno::Reference< css::task::XJobExecutor > xExecutor
                = css::task::theJobExecutor::get( xContext );
            xExecutor->trigger( "onFirstVisibleTask" );
        }
    }
}

void SAL_CALL Frame::windowHidden( const css::lang::EventObject& ) throw(css::uno::RuntimeException)
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    m_bIsHidden = sal_True;
    aReadLock.unlock();
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

    @param      -
    @return     -

    @onerror    -
*
void SAL_CALL Frame::disposing( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    SAL_WARN_IF( implcp_disposing( aEvent ), "fwk", "Frame::disposing(): Invalid parameter detected." );
    
    
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    if( aEvent.Source == m_xContainerWindow )
    {
        
        aWriteLock.unlock();
        implts_stopWindowListening();
        aWriteLock.lock();
        m_xContainerWindow = css::uno::Reference< css::awt::XWindow >();
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

    @param      -
    @return     true if frame/task is locked
                false otherwise

    @onerror    -
    @threadsafe yes
*
sal_Bool SAL_CALL Frame::isActionLocked() throw( css::uno::RuntimeException )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );
    return( m_nExternalLockCount!=0);
}


void SAL_CALL Frame::addActionLock() throw( css::uno::RuntimeException )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    ++m_nExternalLockCount;
}


void SAL_CALL Frame::removeActionLock() throw( css::uno::RuntimeException )
{
    
    

    /* SAFE AREA */{
        WriteGuard aWriteLock( m_aLock );
        SAL_WARN_IF( m_nExternalLockCount<=0, "fwk", "Frame::removeActionLock(): Frame isn't locked! Possible multithreading problem detected." );
        --m_nExternalLockCount;
    }/* SAFE */

    implts_checkSuicide();
}


void SAL_CALL Frame::setActionLocks( sal_Int16 nLock ) throw( css::uno::RuntimeException )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    
    
    
    
    m_nExternalLockCount = m_nExternalLockCount + nLock;
}


sal_Int16 SAL_CALL Frame::resetActionLocks() throw( css::uno::RuntimeException )
{
    
    

    sal_Int16 nCurrentLocks = 0;
    /* SAFE */{
        WriteGuard aWriteLock( m_aLock );
        nCurrentLocks = m_nExternalLockCount;
        m_nExternalLockCount = 0;
    }/* SAFE */

    
    
    
    
    implts_checkSuicide();

    return nCurrentLocks;
}


void Frame::impl_initializePropInfo()
{
    impl_setPropertyChangeBroadcaster(static_cast< css::frame::XFrame* >(this));

    impl_addPropertyInfo(
        css::beans::Property(
            FRAME_PROPNAME_DISPATCHRECORDERSUPPLIER,
            FRAME_PROPHANDLE_DISPATCHRECORDERSUPPLIER,
            ::getCppuType((const css::uno::Reference< css::frame::XDispatchRecorderSupplier >*)NULL),
            css::beans::PropertyAttribute::TRANSIENT));

    impl_addPropertyInfo(
        css::beans::Property(
            FRAME_PROPNAME_INDICATORINTERCEPTION,
            FRAME_PROPHANDLE_INDICATORINTERCEPTION,
            ::getCppuType((const css::uno::Reference< css::task::XStatusIndicator >*)NULL),
            css::beans::PropertyAttribute::TRANSIENT));

    impl_addPropertyInfo(
        css::beans::Property(
            FRAME_PROPNAME_ISHIDDEN,
            FRAME_PROPHANDLE_ISHIDDEN,
            ::getBooleanCppuType(),
            css::beans::PropertyAttribute::TRANSIENT | css::beans::PropertyAttribute::READONLY));

    impl_addPropertyInfo(
        css::beans::Property(
            FRAME_PROPNAME_LAYOUTMANAGER,
            FRAME_PROPHANDLE_LAYOUTMANAGER,
            ::getCppuType((const css::uno::Reference< ::com::sun::star::frame::XLayoutManager >*)NULL),
            css::beans::PropertyAttribute::TRANSIENT));

    impl_addPropertyInfo(
        css::beans::Property(
            FRAME_PROPNAME_TITLE,
            FRAME_PROPHANDLE_TITLE,
            ::getCppuType((const OUString*)NULL),
            css::beans::PropertyAttribute::TRANSIENT));
}


void SAL_CALL Frame::impl_setPropertyValue(const OUString& /*sProperty*/,
                                                 sal_Int32        nHandle  ,
                                           const css::uno::Any&   aValue   )

{
    /* There is no need to lock any mutex here. Because we share the
       solar mutex with our base class. And we said to our base class: "dont release it on calling us" .-)
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
       solar mutex with our base class. And we said to our base class: "dont release it on calling us" .-)
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

    @seealso    -

    @param      "xWindow", reference to old container window to dispose it
    @return     An empty reference.

    @onerror    -
    @threadsafe NO!
*
void Frame::impl_disposeContainerWindow( css::uno::Reference< css::awt::XWindow >& xWindow )
{
    if( xWindow.is() )
    {
        xWindow->setVisible( sal_False );
        
        
        xWindow->dispose();
        xWindow = css::uno::Reference< css::awt::XWindow >();
    }
}

/*-****************************************************************************************************
    @short      send frame action event to our listener
    @descr      This method is threadsafe AND can be called by our dispose method too!

    @seealso    -

    @param      "aAction", describe the event for sending
    @return     -

    @onerror    -
*
void Frame::implts_sendFrameActionEvent( const css::frame::FrameAction& aAction )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    
    
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
    
    
    
    
    ::cppu::OInterfaceContainerHelper* pContainer = m_aListenerContainer.getContainer( ::getCppuType( ( const css::uno::Reference< css::frame::XFrameActionListener >*) NULL ) );

    if( pContainer != NULL )
    {
        
        css::frame::FrameActionEvent aFrameActionEvent( static_cast< ::cppu::OWeakObject* >(this), this, aAction );

        
        ::cppu::OInterfaceIteratorHelper aIterator( *pContainer );
        
        while( aIterator.hasMoreElements() )
        {
            try
            {
                ((css::frame::XFrameActionListener*)aIterator.next())->frameAction( aFrameActionEvent );
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

    @seealso    -

    @param      -
    @return     -

    @onerror    -
*
void Frame::implts_resizeComponentWindow()
{
    
    
    if ( !m_xLayoutManager.is() )
    {
        css::uno::Reference< css::awt::XWindow > xComponentWindow( getComponentWindow() );
        if( xComponentWindow.is() )
        {
            css::uno::Reference< css::awt::XDevice > xDevice( getContainerWindow(), css::uno::UNO_QUERY );

            
            css::awt::Rectangle  aRectangle  = getContainerWindow()->getPosSize();
            css::awt::DeviceInfo aInfo       = xDevice->getInfo();
            css::awt::Size       aSize       (  aRectangle.Width  - aInfo.LeftInset - aInfo.RightInset  ,
                                                aRectangle.Height - aInfo.TopInset  - aInfo.BottomInset );

            
            xComponentWindow->setPosSize( 0, 0, aSize.Width, aSize.Height, css::awt::PosSize::POSSIZE );
        }
    }
}

/*-****************************************************************************************************
    @short      helper to set icon on our container window (if it is a system window!)
    @descr      We use our internal set controller (if it exist) to specify which factory he represanted.
                These information can be used to find right icon. But our controller can say it us directly
                too ... we should ask his optional property set first ...

    @seealso    method Window::SetIcon()

    @param      -
    @return     -

    @onerror    We do nothing.
*
void Frame::implts_setIconOnWindow()
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::awt::XWindow >       xContainerWindow( m_xContainerWindow, css::uno::UNO_QUERY );
    css::uno::Reference< css::frame::XController > xController     ( m_xController     , css::uno::UNO_QUERY );
    aReadLock.unlock();
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */

    if( xContainerWindow.is() && xController.is() )
    {
        
        
        
        
        sal_Int32 nIcon = -1;

        
        
        
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

        
        
        
        if( nIcon == -1 )
        {
            css::uno::Reference< css::frame::XModel > xModel = xController->getModel();
            if( xModel.is() )
            {
                SvtModuleOptions::EFactory eFactory = SvtModuleOptions::ClassifyFactoryByModel(xModel);
                if (eFactory != SvtModuleOptions::E_UNKNOWN_FACTORY)
                    nIcon = SvtModuleOptions().GetFactoryIcon( eFactory );
            }
        }

        
        
        if( nIcon == -1 )
        {
            nIcon = 0;
        }

        
        
        
        
        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        {
            SolarMutexGuard aSolarGuard;
            Window* pWindow = (VCLUnoHelper::GetWindow( xContainerWindow ));
            if(
                ( pWindow            != NULL              ) &&
                ( pWindow->GetType() == WINDOW_WORKWINDOW )
                )
            {
                WorkWindow* pWorkWindow = (WorkWindow*)pWindow;
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

    @param      -
    @return     -

    @onerror    We do nothing!
    @threadsafe yes
*
void Frame::implts_startWindowListening()
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::awt::XWindow >                            xContainerWindow    = m_xContainerWindow   ;
    css::uno::Reference< css::uno::XComponentContext >                  xContext = m_xContext;
    css::uno::Reference< css::datatransfer::dnd::XDropTargetListener >  xDragDropListener   = m_xDropTargetListener;
    css::uno::Reference< css::awt::XWindowListener >                    xWindowListener     ( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );
    css::uno::Reference< css::awt::XFocusListener >                     xFocusListener      ( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );
    css::uno::Reference< css::awt::XTopWindowListener >                 xTopWindowListener  ( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );
    aReadLock.unlock();
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
    
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::awt::XWindow >                            xContainerWindow    = m_xContainerWindow   ;
    css::uno::Reference< css::uno::XComponentContext >                  xContext            = m_xContext           ;
    css::uno::Reference< css::datatransfer::dnd::XDropTargetListener >  xDragDropListener   = m_xDropTargetListener;
    css::uno::Reference< css::awt::XWindowListener >                    xWindowListener     ( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );
    css::uno::Reference< css::awt::XFocusListener >                     xFocusListener      ( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );
    css::uno::Reference< css::awt::XTopWindowListener >                 xTopWindowListener  ( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );
    aReadLock.unlock();
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
*
void Frame::implts_checkSuicide()
{
    /* SAFE */
    ReadGuard aReadLock(m_aLock);
    
    
    sal_Bool bSuicide = (m_nExternalLockCount==0 && m_bSelfClose);
    m_bSelfClose = sal_False;
    aReadLock.unlock();
    /* } SAFE */
    
    
    
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
                            /*IN*/       sal_Bool                                   bState  )
{
    
    if (!SvtModuleOptions().IsModuleInstalled(SvtModuleOptions::E_SSTARTMODULE))
        return;

    try
    {
        css::uno::Reference< css::beans::XPropertySet > xFrameProps(xFrame, css::uno::UNO_QUERY_THROW);
        css::uno::Reference< css::frame::XLayoutManager > xLayoutManager;
        xFrameProps->getPropertyValue(FRAME_PROPNAME_LAYOUTMANAGER) >>= xLayoutManager;
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
    ReadGuard aReadLock(m_aLock);

    
    
    
    css::uno::Reference< css::frame::XDesktop >        xDesktop     (m_xParent, css::uno::UNO_QUERY);
    css::uno::Reference< css::frame::XFramesSupplier > xTaskSupplier(xDesktop , css::uno::UNO_QUERY);
    if ( !xDesktop.is() || !xTaskSupplier.is() )
        return;

    aReadLock.unlock();
    /* } SAFE */

    
    
    
    FrameListAnalyzer aAnalyzer(
        xTaskSupplier,
        this,
        FrameListAnalyzer::E_HIDDEN | FrameListAnalyzer::E_HELP | FrameListAnalyzer::E_BACKINGCOMPONENT);

    
    css::uno::Reference< css::frame::XFrame2 > xNewCloserFrame;

    
    
    
    
    
    
    
    
    if (
        (aAnalyzer.m_lOtherVisibleFrames.getLength()==1)   &&
        (
            (aAnalyzer.m_bReferenceIsHelp  ) ||
            (aAnalyzer.m_bReferenceIsHidden)
        )
       )
    {
        
        
        xNewCloserFrame.set( aAnalyzer.m_lOtherVisibleFrames[0], css::uno::UNO_QUERY_THROW );
    }
    
    
    
    
    else if (
        (aAnalyzer.m_lOtherVisibleFrames.getLength()==0) &&
        (!aAnalyzer.m_bReferenceIsHelp                 ) &&
        (!aAnalyzer.m_bReferenceIsHidden               ) &&
        (!aAnalyzer.m_bReferenceIsBacking              )
       )
    {
        xNewCloserFrame = this;
    }

    
    
    
    /* STATIC SAFE { */
    WriteGuard aStaticWriteLock(LockHelper::getGlobalLock());
    css::uno::Reference< css::frame::XFrame2 > xCloserFrame (m_xCloserFrame.get(), css::uno::UNO_QUERY);
    if (xCloserFrame!=xNewCloserFrame)
    {
        if (xCloserFrame.is())
            impl_setCloser(xCloserFrame, sal_False);
        if (xNewCloserFrame.is())
            impl_setCloser(xNewCloserFrame, sal_True);
        m_xCloserFrame = xNewCloserFrame;
    }
    aStaticWriteLock.unlock();
    /* } STATIC SAFE */
}








sal_Bool Frame::implcp_setActiveFrame( const css::uno::Reference< css::frame::XFrame >& xFrame )
{
    return css::uno::Reference< css::frame::XDesktop >( xFrame, css::uno::UNO_QUERY ).is();
}


sal_Bool Frame::implcp_addFrameActionListener( const css::uno::Reference< css::frame::XFrameActionListener >& xListener )
{
    return !xListener.is();
}


sal_Bool Frame::implcp_removeFrameActionListener( const css::uno::Reference< css::frame::XFrameActionListener >& xListener )
{
    return !xListener.is();
}


sal_Bool Frame::implcp_addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener )
{
    return !xListener.is();
}


sal_Bool Frame::implcp_removeEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener )
{
    return !xListener.is();
}


sal_Bool Frame::implcp_windowResized( const css::awt::WindowEvent& aEvent )
{
    return !aEvent.Source.is();
}


sal_Bool Frame::implcp_focusGained( const css::awt::FocusEvent& aEvent )
{
    return !aEvent.Source.is();
}


sal_Bool Frame::implcp_windowActivated( const css::lang::EventObject& aEvent )
{
    return !aEvent.Source.is();
}


sal_Bool Frame::implcp_windowDeactivated( const css::lang::EventObject& aEvent )
{
    return !aEvent.Source.is();
}

sal_Bool Frame::implcp_disposing( const css::lang::EventObject& aEvent )
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
