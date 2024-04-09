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

#ifndef INCLUDED_DBACCESS_GENERICCONTROLLER_HXX
#define INCLUDED_DBACCESS_GENERICCONTROLLER_HXX

#include <sal/config.h>

#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

#include <optional>

#include <com/sun/star/awt/XUserInputInterception.hpp>
#include <com/sun/star/frame/CommandGroup.hpp>
#include <com/sun/star/frame/DispatchInformation.hpp>
#include <com/sun/star/frame/XController2.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchInformationProvider.hpp>
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/frame/XTitle.hpp>
#include <com/sun/star/frame/XTitleChangeBroadcaster.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <comphelper/sharedmutex.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <connectivity/dbexception.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <dbaccess/AsynchronousLink.hxx>
#include <dbaccess/controllerframe.hxx>
#include <dbaccess/dbaccessdllapi.h>
#include <dbaccess/IController.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <tools/link.hxx>
#include <vcl/vclptr.hxx>
#include <sfx2/userinputinterception.hxx>

namespace com::sun::star {
    namespace awt { class XKeyHandler; }
    namespace awt { class XMouseClickHandler; }
    namespace awt { class XWindow; }
    namespace beans { struct PropertyValue; }
    namespace frame { class XController; }
    namespace frame { class XDispatchProvider; }
    namespace frame { class XFrame; }
    namespace frame { class XFrameActionListener; }
    namespace frame { class XModel; }
    namespace frame { class XStatusListener; }
    namespace frame { class XTitleChangeListener; }
    namespace frame { struct DispatchDescriptor; }
    namespace frame { struct FrameActionEvent; }
    namespace lang { class XEventListener; }
    namespace sdb { class XDatabaseContext; }
    namespace sdbc { class XConnection; }
    namespace sdbc { class XDataSource; }
    namespace ui { class XSidebarProvider; }
    namespace uno { class XComponentContext; }
    namespace util { class XURLTransformer; }
}

namespace vcl { class Window; }
namespace weld { class Window; }
class NotifyEvent;

namespace dbaui
{
    class ODataView;

    template< typename T >
    inline bool SAL_CALL operator >>= (const css::uno::Any& _any, std::optional< T >& _value)
    {
        _value.reset();  // de-init the optional value

        T directValue = T();
        if ( _any >>= directValue )
            _value = directValue;

        return !!_value;
    }


    // = FeatureState

    /** describes the state of a feature

        In opposite to the FeatureStateEvent in css.frame, this one allows for multiple states to be specified at once.
        With this, you can for instance specify that a toolbox item is checked, and has a certain title, at the same
        time.
    */
    struct FeatureState
    {
        bool                        bEnabled;

        std::optional<bool> bChecked;
        std::optional<bool> bInvisible;
        css::uno::Any               aValue;
        std::optional<OUString> sTitle;

        FeatureState() : bEnabled(false) { }
    };


    // = helper


    struct ControllerFeature : public css::frame::DispatchInformation
    {
        sal_uInt16 nFeatureId;
    };


    typedef ::std::map  <   OUString
                        ,   ControllerFeature
                        ,   ::std::less< OUString >
                        >   SupportedFeatures;


    class CompareFeatureById
    {
        const sal_Int32 m_nId;
    public:
        CompareFeatureById(sal_Int32 _nId) : m_nId(_nId)
        {}

        bool operator()( const SupportedFeatures::value_type& _aType ) const
        {
            return m_nId == _aType.second.nFeatureId;
        }
    };


    struct FeatureListener
    {
        css::uno::Reference< css::frame::XStatusListener >
                    xListener;
        sal_Int32   nId;
        bool        bForceBroadcast;
    };


    class FindFeatureListener
    {
        const css::uno::Reference< css::frame::XStatusListener >& m_xListener;
    public:
        FindFeatureListener(const css::uno::Reference< css::frame::XStatusListener >& _xListener)
            : m_xListener(_xListener)
        {}

        bool operator()( const FeatureListener& lhs ) const
        {
            return lhs.xListener == m_xListener;
        }
    };


    typedef ::comphelper::SharedMutexBase   OGenericUnoController_MBASE;

    typedef ::cppu::WeakComponentImplHelper<   css::frame::XDispatch
                                           ,   css::frame::XDispatchProviderInterceptor
                                           ,   css::util::XModifyListener
                                           ,   css::frame::XFrameActionListener
                                           ,   css::lang::XInitialization
                                           ,   css::lang::XServiceInfo
                                           ,   css::frame::XDispatchInformationProvider
                                           ,   css::frame::XController2
                                           ,   css::frame::XTitle
                                           ,   css::frame::XTitleChangeBroadcaster
                                           ,   css::awt::XUserInputInterception
                                           >   OGenericUnoController_Base;

    struct OGenericUnoController_Data;

    class UNLESS_MERGELIBS_MORE(DBACCESS_DLLPUBLIC) OGenericUnoController
                                :public OGenericUnoController_MBASE
                                ,public OGenericUnoController_Base
                                ,public IController
    {
    private:
        SupportedFeatures               m_aSupportedFeatures;
        ::sfx2::UserInputInterception   m_aUserInputInterception;
        VclPtr<ODataView>               m_pView;                // our (VCL) "main window"

#ifdef DBG_UTIL
        bool                            m_bDescribingSupportedFeatures;
#endif

    protected:

        // attributes
        struct DispatchTarget
        {
            css::util::URL                                        aURL;
            css::uno::Reference< css::frame::XStatusListener >    xListener;

            DispatchTarget(css::util::URL _aURL, css::uno::Reference< css::frame::XStatusListener > xRef) : aURL(std::move(_aURL)), xListener(std::move(xRef)) { }
        };

        ::std::deque< FeatureListener >
                                m_aFeaturesToInvalidate;

        std::mutex              m_aFeatureMutex;        // locked when features are append to or remove from deque
        std::map<sal_uInt16, FeatureState> m_aStateCache; // save the current status of feature state
        std::vector<DispatchTarget> m_arrStatusListener;  // all our listeners where we dispatch status changes
        OAsynchronousLink       m_aAsyncInvalidateAll;
        OAsynchronousLink       m_aAsyncCloseTask;      // called when a task should be closed

        css::uno::Reference< css::util::XURLTransformer >         m_xUrlTransformer;      // needed sometimes
        css::uno::Reference< css::uno::XComponentContext >        m_xContext;
        ControllerFrame                                                                     m_aCurrentFrame;
        css::uno::Reference< css::frame::XDispatchProvider >      m_xSlaveDispatcher;     // for intercepting dispatches
        css::uno::Reference< css::frame::XDispatchProvider >      m_xMasterDispatcher;    // ditto
        css::uno::Reference< css::sdb::XDatabaseContext >         m_xDatabaseContext;
        css::uno::Reference< css::frame::XTitle >                 m_xTitleHelper;

        bool                    m_bPreview;
        bool                    m_bReadOnly;

        bool                    m_bCurrentlyModified : 1;
        bool                    m_bExternalTitle : 1;


        // attribute access
        using OGenericUnoController_MBASE::getMutex;
        ::cppu::OBroadcastHelper&   getBroadcastHelper()        { return OGenericUnoController_Base::rBHelper; }


        // methods
        OGenericUnoController( const css::uno::Reference< css::uno::XComponentContext >& _rM );
        OGenericUnoController() = delete;

        // closes the task when possible
        void closeTask();

        // if getMenu returns a non empty string than this will be dispatched at the frame
        virtual void            loadMenu(const css::uno::Reference< css::frame::XFrame >& _xFrame);

        /** called when our menu has been loaded into our frame, can be used to load sub toolbars

            @param _xLayoutManager
                The layout manager.
        */
        virtual void            onLoadedMenu(const css::uno::Reference< css::frame::XLayoutManager >& _xLayoutManager);

        // all the features which should be handled by this class
        virtual void            describeSupportedFeatures();

        // state of a feature. 'feature' may be the handle of a css::util::URL somebody requested a dispatch interface for OR a toolbar slot.
        virtual FeatureState    GetState(sal_uInt16 nId) const;
        // execute a feature
        virtual void            Execute(sal_uInt16 nId , const css::uno::Sequence< css::beans::PropertyValue>& aArgs);

        /** describes a feature supported by the controller

            Must not be called outside <member>describeSupportedFeatures</member>.

            @param _rCommandURL
                the URL of the feature command
            @param _nFeatureId
                the id of the feature. Later references to this feature usually happen by id, not by
                URL
            @param _nCommandGroup
                the command group of the feature. This is important for configuring the controller UI
                by the user, see also <type scope="css::frame">CommandGroup</type>.
        */
        void    implDescribeSupportedFeature(
                    const OUString& _rCommandURL,
                    sal_uInt16 _nFeatureId,
                    sal_Int16 _nCommandGroup = css::frame::CommandGroup::INTERNAL
                );

        /** returns <TRUE/> if the feature is supported, otherwise <FALSE/>
            @param  _nId
                The ID of the feature.
        */
        bool isFeatureSupported( sal_Int32 _nId );

        // gets the URL which the given id is assigned to
        css::util::URL getURLForId(sal_Int32 _nId) const;

        /** determines whether the given feature ID denotes a user-defined feature

            @see IController::registerCommandURL
        */
        static bool isUserDefinedFeature( const sal_uInt16 nFeatureId );

        /** determines whether the given feature URL denotes a user-defined feature

            @see IController::registerCommandURL
        */
        bool    isUserDefinedFeature( const OUString& _rFeatureURL ) const;

        // connect to a datasource
        css::uno::Reference< css::sdbc::XConnection > connect(
            const css::uno::Reference< css::sdbc::XDataSource>& _xDataSource
        );

        // connect to a datasource
        css::uno::Reference< css::sdbc::XConnection > connect(
            const OUString& _rsDataSourceName,
            const OUString& _rContextInformation,
            ::dbtools::SQLExceptionInfo* _pErrorInfo
        );

        void startConnectionListening(const css::uno::Reference< css::sdbc::XConnection >& _rxConnection);
        void stopConnectionListening(const css::uno::Reference< css::sdbc::XConnection >& _rxConnection);

        /** return the container window of the top most frame
            @return
                The top most container window, nmay be <NULL/>.
        */
        css::uno::Reference< css::awt::XWindow> getTopMostContainerWindow() const;

        // XInitialize will be called inside initialize
        virtual void impl_initialize(const ::comphelper::NamedValueCollection& rArguments);

        virtual OUString getPrivateTitle() const { return OUString(); }

        css::uno::Reference< css::frame::XTitle > impl_getTitleHelper_throw(bool bCreateIfNecessary = true);
        virtual css::uno::Reference< css::frame::XModel > getPrivateModel() const
        {
            return css::uno::Reference< css::frame::XModel >();
        }

        virtual void    startFrameListening( const css::uno::Reference< css::frame::XFrame >& _rxFrame );
        virtual void    stopFrameListening( const css::uno::Reference< css::frame::XFrame >& _rxFrame );

        void releaseNumberForComponent();

        virtual ~OGenericUnoController() override;

    private:
        void fillSupportedFeatures();

        void InvalidateAll_Impl();
        void InvalidateFeature_Impl();

        void ImplInvalidateFeature( sal_Int32 _nId, const css::uno::Reference< css::frame::XStatusListener >& _xListener, bool _bForceBroadcast );

        void ImplBroadcastFeatureState(const OUString& _rFeature, const css::uno::Reference< css::frame::XStatusListener > & xListener, bool _bIgnoreCache);

        void executeUserDefinedFeatures( const css::util::URL& _rFeatureURL, const css::uno::Sequence< css::beans::PropertyValue>& _rArgs );

        // link methods
        DECL_DLLPRIVATE_LINK(OnAsyncInvalidateAll, void*, void);
        DECL_DLLPRIVATE_LINK(OnAsyncCloseTask, void*, void);

    public:
        const css::uno::Reference< css::uno::XComponentContext >& getORB() const { return m_xContext; }
        ODataView*  getView() const { return m_pView; }
        weld::Window* getFrameWeld() const;
        void        setView( const VclPtr<ODataView>& i_rView );
        void        clearView();
        // shows an error box if the SQLExceptionInfo is valid
        void showError(const ::dbtools::SQLExceptionInfo& _rInfo);

        // if there is a css::util::URL translation for the id
        // ('handle') then if xListener is NULL the change will be forwarded
        // to all listeners to the given css::util::URL
        // if there is a toolbar slot with the given id it is updated (the new state is determined via GetState)
        // if _bForceBroadcast is sal_True, the current feature state is broadcasted no matter if it is the same as the cached state
        void InvalidateFeature(sal_uInt16 nId, const css::uno::Reference< css::frame::XStatusListener > & xListener = nullptr, bool _bForceBroadcast = false);

        /** InvalidateAll invalidates all features currently known
        */
        void InvalidateAll();
        // late construction
        virtual bool Construct(vcl::Window* pParent);

        /** get the layout manager
            @param  _xFrame
                The frame to ask for the layout manager.
            @return
                The layout manager of the frame, can be <NULL/> if the frame isn't initialized.
        */
        static css::uno::Reference< css::frame::XLayoutManager > getLayoutManager(const css::uno::Reference< css::frame::XFrame >& _xFrame);

        // IController
        virtual void executeUnChecked(const css::util::URL& _rCommand, const css::uno::Sequence< css::beans::PropertyValue>& aArgs) override;
        virtual void executeChecked(const css::util::URL& _rCommand, const css::uno::Sequence< css::beans::PropertyValue>& aArgs) override;
        virtual void executeUnChecked(sal_uInt16 _nCommandId, const css::uno::Sequence< css::beans::PropertyValue>& aArgs) override;
        virtual void executeChecked(sal_uInt16 _nCommandId, const css::uno::Sequence< css::beans::PropertyValue>& aArgs) override;
        virtual bool isCommandEnabled(sal_uInt16 _nCommandId) const override;
        virtual bool isCommandEnabled(const OUString& _rCompleteCommandURL) const override;
        virtual bool isDataSourceReadOnly() const override;
        virtual css::uno::Reference< css::frame::XController > getXController() override;
        virtual bool interceptUserInput( const NotifyEvent& _rEvent ) override;

        // misc
        bool isCommandChecked(sal_uInt16 _nCommandId) const;

        // css::lang::XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& Source) override;

        // css::util::XModifyListener
        virtual void SAL_CALL modified(const css::lang::EventObject& aEvent) override;

        // XInterface
        virtual void SAL_CALL acquire(  ) noexcept override;
        virtual void SAL_CALL release(  ) noexcept override;

        // css::frame::XController2
        virtual css::uno::Reference< css::awt::XWindow > SAL_CALL getComponentWindow() override;
        virtual OUString SAL_CALL getViewControllerName() override;
        virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getCreationArguments() override;

        virtual css::uno::Reference< css::ui::XSidebarProvider > SAL_CALL getSidebar() override;


        // css::frame::XController
        virtual void SAL_CALL attachFrame(const css::uno::Reference< css::frame::XFrame > & xFrame) override;
        virtual sal_Bool SAL_CALL attachModel(const css::uno::Reference< css::frame::XModel > & xModel) override;
        virtual sal_Bool SAL_CALL suspend(sal_Bool bSuspend) override = 0;
        virtual css::uno::Any SAL_CALL getViewData() override;
        virtual void SAL_CALL restoreViewData(const css::uno::Any& Data) override;
        virtual css::uno::Reference< css::frame::XModel >  SAL_CALL getModel() override;
        virtual css::uno::Reference< css::frame::XFrame >  SAL_CALL getFrame() override;

        // css::frame::XDispatch
        virtual void        SAL_CALL dispatch(const css::util::URL& aURL, const css::uno::Sequence< css::beans::PropertyValue>& aArgs) override;
        virtual void        SAL_CALL addStatusListener(const css::uno::Reference< css::frame::XStatusListener > & aListener, const css::util::URL& aURL) override;
        virtual void        SAL_CALL removeStatusListener(const css::uno::Reference< css::frame::XStatusListener > & aListener, const css::util::URL& aURL) override;

        // css::frame::XDispatchProviderInterceptor
        virtual css::uno::Reference< css::frame::XDispatchProvider >  SAL_CALL getSlaveDispatchProvider() override;
        virtual void SAL_CALL setSlaveDispatchProvider(const css::uno::Reference< css::frame::XDispatchProvider > & _xNewProvider) override;
        virtual css::uno::Reference< css::frame::XDispatchProvider >  SAL_CALL getMasterDispatchProvider() override;
        virtual void SAL_CALL setMasterDispatchProvider(const css::uno::Reference< css::frame::XDispatchProvider > & _xNewProvider) override;

        // css::frame::XDispatchProvider
        virtual css::uno::Reference< css::frame::XDispatch >  SAL_CALL queryDispatch(const css::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags) override;
        virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch >  > SAL_CALL queryDispatches(const css::uno::Sequence< css::frame::DispatchDescriptor >& aDescripts) override;

        // css::lang::XComponent
        virtual void SAL_CALL dispose() override; //LLA: need solar mutex {OGenericUnoController_COMPBASE::dispose(); }
        virtual void SAL_CALL disposing() override;
        virtual void SAL_CALL addEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener) override;
        virtual void SAL_CALL removeEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener) override;

        // css::frame::XFrameActionListener
        virtual void SAL_CALL frameAction(const css::frame::FrameActionEvent& aEvent) override;
        // lang::XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override = 0;
        virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
        virtual css::uno::Sequence< OUString> SAL_CALL getSupportedServiceNames() override = 0;

        // XDispatchInformationProvider
        virtual css::uno::Sequence< ::sal_Int16 > SAL_CALL getSupportedCommandGroups() override;
        virtual css::uno::Sequence< css::frame::DispatchInformation > SAL_CALL getConfigurableDispatchInformation( ::sal_Int16 ) override;

        // XTitle
        virtual OUString SAL_CALL getTitle(  ) override;
        virtual void SAL_CALL setTitle( const OUString& sTitle ) override;

        // XTitleChangeBroadcaster
        virtual void SAL_CALL addTitleChangeListener( const css::uno::Reference< css::frame::XTitleChangeListener >& xListener ) override;
        virtual void SAL_CALL removeTitleChangeListener( const css::uno::Reference< css::frame::XTitleChangeListener >& xListener ) override;

        // XUserInputInterception
        virtual void SAL_CALL addKeyHandler( const css::uno::Reference< css::awt::XKeyHandler >& xHandler ) override;
        virtual void SAL_CALL removeKeyHandler( const css::uno::Reference< css::awt::XKeyHandler >& xHandler ) override;
        virtual void SAL_CALL addMouseClickHandler( const css::uno::Reference< css::awt::XMouseClickHandler >& xHandler ) override;
        virtual void SAL_CALL removeMouseClickHandler( const css::uno::Reference< css::awt::XMouseClickHandler >& xHandler ) override;
    };
}

#endif // INCLUDED_DBACCESS_GENERICCONTROLLER_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
