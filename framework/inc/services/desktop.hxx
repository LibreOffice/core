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

#ifndef INCLUDED_FRAMEWORK_INC_SERVICES_DESKTOP_HXX
#define INCLUDED_FRAMEWORK_INC_SERVICES_DESKTOP_HXX

#include <sal/config.h>

#include <memory>
#include <vector>

#include <classes/framecontainer.hxx>

#include <com/sun/star/frame/XUntitledNumbers.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XDesktop2.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/frame/XTask.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/frame/XFrames.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XTasksSupplier.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/frame/XDispatchRecorderSupplier.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/propshlp.hxx>
#include <unotools/cmdoptions.hxx>

namespace framework{

enum ELoadState
{
    E_NOTSET      ,
    E_SUCCESSFUL  ,
    E_FAILED      ,
    E_INTERACTION
};

/*-************************************************************************************************************
    @short      implement the topframe of frame tree
    @descr      This is the root of the frame tree. The desktop has no window, is not visible but he is the logical
                "masternode" to build the hierarchy.

    @implements XInterface
                XTypeProvider
                XServiceInfo
                XDesktop
                XComponentLoader
                XTasksSupplier
                XDispatchProvider
                XFramesSupplier
                XFrame
                XComponent
                XPropertySet
                XFastPropertySet
                XMultiPropertySet
                XDispatchResultListener
                XEventListener
                XInteractionHandler

    @devstatus  ready to use
    @threadsafe yes
*//*-*************************************************************************************************************/
typedef cppu::WeakComponentImplHelper<
           css::lang::XServiceInfo              ,
           css::frame::XDesktop2                ,
           css::frame::XTasksSupplier           ,
           css::frame::XDispatchResultListener  ,   // => XEventListener
           css::task::XInteractionHandler       ,
           css::frame::XUntitledNumbers > Desktop_BASE;

class Desktop : private cppu::BaseMutex,
                private TransactionBase,
                public Desktop_BASE,
                public cppu::OPropertySetHelper
{
    // internal used types, const etcpp.
    private:

        /** used temporary to know which listener was already called or not. */
        typedef ::std::vector< css::uno::Reference< css::frame::XTerminateListener > > TTerminateListenerList;

    // public methods
    public:

        //  constructor / destructor
                 Desktop( const css::uno::Reference< css::uno::XComponentContext >& xContext );
        virtual ~Desktop(                                                                    ) override;

        void constructorInit();

        //  XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;

        virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override;

        virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

        // XInterface
        virtual void SAL_CALL acquire() throw () override
            { OWeakObject::acquire(); }
        virtual void SAL_CALL release() throw () override
            { OWeakObject::release(); }
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& type) override;

        // XTypeProvider
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;

        /**
            @interface  XDesktop

            @short      try to shutdown these desktop environment.

            @descr      Will try to close all frames. If at least one frame could
                        not be closed successfully termination will be stopped.

                        Registered termination listener will be taken into account
                        also. As special feature some of our registered listener
                        are well known by its UNO implementation name. They are handled
                        different to all other listener.

                        Btw: Desktop.terminate() was designed in the past to be used
                        within an UI based environment. So it's allowed e.g. to
                        call XController.suspend() here. If UI isn't an option ... please
                        use XCloseable.close() at these desktop implementation.
                        ... if it will be supported in the future .-))

            @seealso    XTerminateListener
            @seealso    XTerminateListener2

            @return     true if all open frames could be closed and no listener threw
                        a veto exception; false otherwise.

            @onerror    False will be returned.
            @threadsafe yes
         */
        virtual sal_Bool SAL_CALL terminate() override;

        /**
            @interface  XDesktop

            @short      add a listener for termination events

            @descr      Additional to adding normal listener these method was implemented special.
                        Every listener will be asked for its uno implementation name.
                        Some of them are well known... and the corresponding listener won't be added
                        to the container of "normal listener". Those listener will be set as special
                        member.
                        see e.g. member m_xSfxTerminator

            @seealso    terminate()

            @param      xListener
                        the listener for registration.

            @threadsafe yes
         */
        virtual void SAL_CALL addTerminateListener( const css::uno::Reference< css::frame::XTerminateListener >& xListener ) override;

        /**
            @interface  XDesktop

            @short      remove a listener from this container.

            @descr      Additional to removing normal listener these method was implemented special.
                        Every listener will be asked for its uno implementation name.
                        Some of them are well known... and the corresponding listener was set as special member.
                        Now those special member will be reset also.
                        see e.g. member m_xSfxTerminator

            @seealso    terminate()

            @param      xListener
                        the listener for deregistration.

            @threadsafe yes
         */
        virtual void SAL_CALL removeTerminateListener( const css::uno::Reference< css::frame::XTerminateListener >& xListener ) override;

        virtual css::uno::Reference< css::container::XEnumerationAccess >           SAL_CALL getComponents              (                                                                                 ) override;
        virtual css::uno::Reference< css::lang::XComponent >                        SAL_CALL getCurrentComponent        (                                                                                 ) override;
        virtual css::uno::Reference< css::frame::XFrame >                           SAL_CALL getCurrentFrame            (                                                                                 ) override;

        //  XComponentLoader
        virtual css::uno::Reference< css::lang::XComponent >                        SAL_CALL loadComponentFromURL       ( const OUString&                                         sURL             ,
                                                                                                                          const OUString&                                         sTargetFrameName ,
                                                                                                                                sal_Int32                                                nSearchFlags     ,
                                                                                                                          const css::uno::Sequence< css::beans::PropertyValue >&         lArguments       ) override;

        //  XTasksSupplier
        virtual css::uno::Reference< css::container::XEnumerationAccess >           SAL_CALL getTasks                   (                                                                                 ) override;
        virtual css::uno::Reference< css::frame::XTask >                            SAL_CALL getActiveTask              (                                                                                 ) override;

        //  XDispatchProvider
        virtual css::uno::Reference< css::frame::XDispatch >                        SAL_CALL queryDispatch              ( const css::util::URL&                                          aURL             ,
                                                                                                                          const OUString&                                         sTargetFrameName ,
                                                                                                                                sal_Int32                                                nSearchFlags     ) override;
        virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > >  SAL_CALL queryDispatches            ( const css::uno::Sequence< css::frame::DispatchDescriptor >&    lQueries         ) override;

        // XDispatchProviderInterception
        virtual void                                                                SAL_CALL registerDispatchProviderInterceptor( const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor) override;
        virtual void                                                                SAL_CALL releaseDispatchProviderInterceptor ( const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor) override;

        //  XFramesSupplier
        virtual css::uno::Reference< css::frame::XFrames >                          SAL_CALL getFrames                  (                                                                                 ) override;
        virtual css::uno::Reference< css::frame::XFrame >                           SAL_CALL getActiveFrame             (                                                                                 ) override;
        virtual void                                                                SAL_CALL setActiveFrame             ( const css::uno::Reference< css::frame::XFrame >&               xFrame           ) override;

        //   XFrame
        //  Attention: findFrame() is implemented only! Other methods make no sense for our desktop!
        virtual css::uno::Reference< css::frame::XFrame >                           SAL_CALL findFrame                  ( const OUString&                                         sTargetFrameName ,
                                                                                                                                sal_Int32                                                nSearchFlags     ) override;
        virtual void                                                                SAL_CALL initialize                 ( const css::uno::Reference< css::awt::XWindow >&                xWindow          ) override;
        virtual css::uno::Reference< css::awt::XWindow >                            SAL_CALL getContainerWindow         (                                                                                 ) override;
        virtual void                                                                SAL_CALL setCreator                 ( const css::uno::Reference< css::frame::XFramesSupplier >&      xCreator         ) override;
        virtual css::uno::Reference< css::frame::XFramesSupplier >                  SAL_CALL getCreator                 (                                                                                 ) override;
        virtual OUString                                                     SAL_CALL getName                    (                                                                                 ) override;
        virtual void                                                                SAL_CALL setName                    ( const OUString&                                         sName            ) override;
        virtual sal_Bool                                                            SAL_CALL isTop                      (                                                                                 ) override;
        virtual void                                                                SAL_CALL activate                   (                                                                                 ) override;
        virtual void                                                                SAL_CALL deactivate                 (                                                                                 ) override;
        virtual sal_Bool                                                            SAL_CALL isActive                   (                                                                                 ) override;
        virtual sal_Bool                                                            SAL_CALL setComponent               ( const css::uno::Reference< css::awt::XWindow >&                xComponentWindow ,
                                                                                                                          const css::uno::Reference< css::frame::XController >&          xController      ) override;
        virtual css::uno::Reference< css::awt::XWindow >                            SAL_CALL getComponentWindow         (                                                                                 ) override;
        virtual css::uno::Reference< css::frame::XController >                      SAL_CALL getController              (                                                                                 ) override;
        virtual void                                                                SAL_CALL contextChanged             (                                                                                 ) override;
        virtual void                                                                SAL_CALL addFrameActionListener     ( const css::uno::Reference< css::frame::XFrameActionListener >& xListener        ) override;
        virtual void                                                                SAL_CALL removeFrameActionListener  ( const css::uno::Reference< css::frame::XFrameActionListener >& xListener        ) override;

        //   XComponent
        virtual void SAL_CALL disposing() override;
        virtual void                                                                SAL_CALL addEventListener           ( const css::uno::Reference< css::lang::XEventListener >&        xListener        ) override;
        virtual void                                                                SAL_CALL removeEventListener        ( const css::uno::Reference< css::lang::XEventListener >&        xListener        ) override;

        //   XDispatchResultListener
        virtual void SAL_CALL dispatchFinished      ( const css::frame::DispatchResultEvent&                    aEvent     ) override;

        //   XEventListener
        virtual void                                                                SAL_CALL disposing                  ( const css::lang::EventObject&                                  aSource          ) override;

        //   XInteractionHandler
        virtual void                                                                SAL_CALL handle                     ( const css::uno::Reference< css::task::XInteractionRequest >&   xRequest         ) override;

        // css.frame.XUntitledNumbers
        virtual ::sal_Int32 SAL_CALL leaseNumber( const css::uno::Reference< css::uno::XInterface >& xComponent ) override;

        // css.frame.XUntitledNumbers
        virtual void SAL_CALL releaseNumber( ::sal_Int32 nNumber ) override;

        // css.frame.XUntitledNumbers
        virtual void SAL_CALL releaseNumberForComponent( const css::uno::Reference< css::uno::XInterface >& xComponent ) override;

        // css.frame.XUntitledNumbers
        virtual OUString SAL_CALL getUntitledPrefix() override;

        // we need this wrapped terminate()-call to terminate even the QuickStarter
        // non-virtual and non-UNO for now
        /// @throws css::uno::RuntimeException
        bool terminateQuickstarterToo();

    //  protected methods

    protected:

        //  OPropertySetHelper
        virtual sal_Bool                                            SAL_CALL convertFastPropertyValue        (       css::uno::Any&  aConvertedValue ,
                                                                                                                     css::uno::Any&  aOldValue       ,
                                                                                                                     sal_Int32       nHandle         ,
                                                                                                               const css::uno::Any&  aValue          ) override;
        virtual void                                                SAL_CALL setFastPropertyValue_NoBroadcast(       sal_Int32       nHandle         ,
                                                                                                               const css::uno::Any&  aValue          ) override;
        using cppu::OPropertySetHelper::getFastPropertyValue;
        virtual void                                                SAL_CALL getFastPropertyValue            (       css::uno::Any&  aValue          ,
                                                                                                                     sal_Int32       nHandle         ) const override;
        virtual ::cppu::IPropertyArrayHelper&                       SAL_CALL getInfoHelper                   (                                       ) override;
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo              (                                       ) override;


    private:

        css::uno::Reference< css::lang::XComponent >            impl_getFrameComponent          ( const css::uno::Reference< css::frame::XFrame >&  xFrame          ) const;

        /** calls queryTermination() on every registered termination listener.
         *
         *  Note: Only normal termination listener (registered in list m_aListenerContainer
         *        will be recognized here. Special listener like quick starter, pipe or others
         *        has to be handled explicitly !
         *
         *  @param  [out] lCalledListener
         *          every called listener will be returned here.
         *          Those list will be used to inform all called listener
         *          about cancel this termination request.
         *
         *  @param  [out] bVeto
         *          will be true if at least one listener threw a veto exception;
         *          false otherwise.
         *
         *  @see    impl_sendCancelTerminationEvent()
         */
        void impl_sendQueryTerminationEvent(TTerminateListenerList& lCalledListener,
                                            bool&             bVeto          );

        /** calls cancelTermination() on every termination listener
         *  where queryTermination() was called before.
         *
         *  Note: Only normal termination listener (registered in list m_aListenerContainer
         *        will be recognized here. Special listener like quick starter, pipe or others
         *        has to be handled explicitly !
         *
         *  @param  [in] lCalledListener
         *          every listener in this list was called within its method
         *          queryTermination() before.
         *
         *  @see    impl_sendQueryTerminationEvent()
         */
        void impl_sendCancelTerminationEvent(const TTerminateListenerList& lCalledListener);

        /** calls notifyTermination() on the clipboard listener
         *
         * The system clipboard may decide that it wants copies
         * in several formats of the clipboard content requiring
         * nearly all the services
         *
         */
        void impl_sendTerminateToClipboard();

        /** calls notifyTermination() on every registered termination listener.
         *
         *  Note: Only normal termination listener (registered in list m_aListenerContainer
         *        will be recognized here. Special listener like quick starter, pipe or others
         *        has to be handled explicitly !
         */
        void impl_sendNotifyTerminationEvent();

        /** try to close all open frames.
         *
         *  Iterates over all child frames and try to close them.
         *  Given parameter bAllowUI enable/disable showing any UI
         *  (which mostly occur on calling XController->suspend()).
         *
         *  This method doesn't stop if one frame could not be closed.
         *  It will ignore such frames and try all other ones.
         *  But it returns false in such case - true otherwise.
         *
         *  @param  bAllowUI
         *          enable/disable showing of UI.
         *
         *  @return true if all frames could be closed; false otherwise.
         */
        bool impl_closeFrames(bool bAllowUI);

    private:

        /** check flag to protect us against dispose before terminate!
          *   see dispose() for further information!
          */
        bool m_bIsTerminated;

        /** when true, the call came form session manager
          *   the method is Desktop::terminateQuickstarterToo()
          *   this the only one place where set this to true
          *   In this case,  when one frame break, not make
          *   question for other, the break of shutdown or logout
          *   can be only once.
          *   In Desktop::impl_closeFrames would be test and break
          *   the loop and reset to false
          */
        bool m_bSession;

        css::uno::Reference< css::uno::XComponentContext >              m_xContext;               /// reference to factory, which has create this instance
        FrameContainer                                                  m_aChildTaskContainer;    /// array of child tasks (children of desktop are tasks; and tasks are also frames - But pure frames are not accepted!)
        ::cppu::OMultiTypeInterfaceContainerHelper                      m_aListenerContainer;     /// container for ALL Listener
        css::uno::Reference< css::frame::XFrames >                      m_xFramesHelper;          /// helper for XFrames, XIndexAccess, XElementAccess and implementation of a childcontainer!
        css::uno::Reference< css::frame::XDispatchProvider >            m_xDispatchHelper;        /// helper to dispatch something for new tasks, created by "_blank"!
        ELoadState                                                      m_eLoadState;             /// hold information about state of asynchron loading of component for loadComponentFromURL()!
        bool                                                            m_bSuspendQuickstartVeto; /// don't ask quickstart for a veto
        std::unique_ptr<SvtCommandOptions>                              m_xCommandOptions;        /// ref counted class to support disabling commands defined by configuration file
        OUString                                                        m_sName;
        OUString                                                        m_sTitle;
        css::uno::Reference< css::frame::XDispatchRecorderSupplier >    m_xDispatchRecorderSupplier;

        /** special terminate listener to close pipe and block external requests
          * during/after termination process is/was running
          */
        css::uno::Reference< css::frame::XTerminateListener > m_xPipeTerminator;

        /** special terminate listener shown inside system tray (quick starter)
          * Will hinder the office on shutdown ... but wish to allow closing
          * of open documents. And because that's different to a normal terminate listener
          * it has to be handled special .-)
          */
        css::uno::Reference< css::frame::XTerminateListener > m_xQuickLauncher;

        /** special terminate listener active when a macro is executing.
          * Because basic runs Application::Yield internally the application may quit
          * while running inside the internal basic event loop. So all the basic
          * infrastructure may be deleted while the call is executing, leading to
          * a variant of crashes. So this special terminate listener will
          * veto the current quit attempt, stop basic execution, which will
          * cause the inner event loop to quit, and on return to the outer normal
          * application event loop then resend the quit attempt.
          * So these implementation must be a special terminate listener too .-(
          */
        css::uno::Reference< css::frame::XTerminateListener > m_xStarBasicQuitGuard;

        /** special terminate listener which loads images asynchronous for current open documents.
          * Because internally it uses blocking system APIs... it can't be guaranteed that
          * running jobs can be cancelled successfully if the corresponding document will be closed...
          * it will not hinder those documents on closing. Instead it let all jobs running...
          * but at least on terminate we have to wait for all those blocked requests.
          * So these implementation must be a special terminate listener too .-(
          */
        css::uno::Reference< css::frame::XTerminateListener > m_xSWThreadManager;

        /** special terminate listener shutting down the SfxApplication.
          * Because these desktop instance closes documents and informs listener
          * only... it does not really shutdown the whole application.
          *
          * Btw: that wouldn't be possible by design... because Desktop.terminate()
          * has to return a boolean value about success... it can't really shutdown the
          * process .-)
          *
          * So we uses a trick: a special listener (exactly these one here) listen for notifyTermination()
          * and shutdown the process asynchronous. But desktop has to make this special
          * notification as really last one ... Otherwise it can happen that asynchronous
          * shutdown will be faster then all other code around Desktop.terminate() .-))
          */
        css::uno::Reference< css::frame::XTerminateListener > m_xSfxTerminator;

        css::uno::Reference< css::frame::XUntitledNumbers > m_xTitleNumberGenerator;

        std::vector<css::uno::Reference<css::frame::XTerminateListener>> m_xComponentDllListeners;

};      //  class Desktop

}       //  namespace framework

#endif // INCLUDED_FRAMEWORK_INC_SERVICES_DESKTOP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
