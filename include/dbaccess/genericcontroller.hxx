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
#include <vector>

#include <dbaccess/AsynchronousLink.hxx>
#include <dbaccess/controllerframe.hxx>
#include <dbaccess/dbaccessdllapi.h>
#include <dbaccess/IController.hxx>

#include <com/sun/star/frame/CommandGroup.hpp>
#include <com/sun/star/frame/XController2.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchInformationProvider.hpp>
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/frame/XTitle.hpp>
#include <com/sun/star/frame/XTitleChangeBroadcaster.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sdb/XDatabaseContext.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/awt/XUserInputInterception.hpp>

#include <comphelper/broadcasthelper.hxx>
#include <comphelper/sharedmutex.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <connectivity/dbexception.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/interfacecontainer.h>

#include <boost/optional.hpp>
#include <sfx2/userinputinterception.hxx>
#include <vcl/vclptr.hxx>

#include <com/sun/star/ui/XSidebarProvider.hpp>

namespace dbtools
{
    class SQLExceptionInfo;
}

namespace vcl { class Window; }
namespace dbaui
{
    class ODataView;


    // = optional

    /** convenience wrapper around boost::optional, allowing typed assignments
    */
    template < typename T >
    class optional : public ::boost::optional< T >
    {
        typedef ::boost::optional< T >  base_type;

    public:
                 optional ( ) : base_type( ) { }
        explicit optional ( T const& val ) : base_type( val ) { }
                 optional ( optional const& rhs ) : base_type( static_cast<base_type const&>(rhs) ) { }

    public:
        optional& operator= ( T const& rhs )
        {
            base_type::reset( rhs );
            return *this;
        }
        optional& operator= ( optional< T > const& rhs )
        {
            if ( rhs.is_initialized() )
                base_type::reset( rhs.get() );
            else
                base_type::reset();
            return *this;
        }
    };

    template< typename T >
    inline bool SAL_CALL operator >>= ( const css::uno::Any & _any, optional< T >& _value )
    {
        _value.reset();  // de-init the optional value

        T directValue = T();
        if ( _any >>= directValue )
            _value.reset( directValue );

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

        optional< bool >            bChecked;
        optional< bool >            bInvisible;
        css::uno::Any               aValue;
        optional< OUString >        sTitle;

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


    struct CompareFeatureById : ::std::binary_function< SupportedFeatures::value_type, sal_Int32, bool >
    {

        inline bool operator()( const SupportedFeatures::value_type& _aType, const sal_Int32& _nId ) const
        {
            return !!( _nId == _aType.second.nFeatureId );
        }
    };


    struct FeatureListener
    {
        css::uno::Reference< css::frame::XStatusListener >
                    xListener;
        sal_Int32   nId;
        bool        bForceBroadcast;
    };


    typedef ::std::deque< FeatureListener > FeatureListeners;


    struct FindFeatureListener : ::std::binary_function< FeatureListener, css::uno::Reference< css::frame::XStatusListener >, bool >
    {

        inline bool operator()( const FeatureListener& lhs, const css::uno::Reference< css::frame::XStatusListener >& rhs ) const
        {
            return !!( lhs.xListener == rhs );
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

    class DBACCESS_DLLPUBLIC OGenericUnoController
                                :public OGenericUnoController_MBASE
                                ,public OGenericUnoController_Base
                                ,public IController
    {
    private:
        SupportedFeatures               m_aSupportedFeatures;
        ::comphelper::NamedValueCollection
                                        m_aInitParameters;

        ::std::unique_ptr< OGenericUnoController_Data >
                                        m_pData;
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

            DispatchTarget(const css::util::URL& rURL, const css::uno::Reference< css::frame::XStatusListener >& rRef) : aURL(rURL), xListener(rRef) { }
        };

        typedef std::map<sal_uInt16, FeatureState> StateCache;
        typedef std::vector<DispatchTarget> Dispatch;

        FeatureListeners        m_aFeaturesToInvalidate;

        ::osl::Mutex            m_aFeatureMutex;        // locked when features are append to or remove from deque
        StateCache              m_aStateCache;          // save the current status of feature state
        Dispatch                m_arrStatusListener;    // all our listeners where we dispatch status changes
        OAsynchronousLink       m_aAsyncInvalidateAll;
        OAsynchronousLink       m_aAsyncCloseTask;      // called when a task should be closed

        css::uno::Reference< css::util::XURLTransformer >         m_xUrlTransformer;      // needed sometimes
        css::uno::Reference< css::uno::XComponentContext >        m_xContext;
        ControllerFrame                                                                     m_aCurrentFrame;
        css::uno::Reference< css::frame::XDispatchProvider >      m_xSlaveDispatcher;     // for intercepting dispatches
        css::uno::Reference< css::frame::XDispatchProvider >      m_xMasterDispatcher;    // dito
        css::uno::Reference< css::sdb::XDatabaseContext >         m_xDatabaseContext;
        css::uno::Reference< css::frame::XTitle >                 m_xTitleHelper;

        bool                    m_bPreview;
        bool                    m_bReadOnly;

        bool                    m_bCurrentlyModified : 1;
        bool                    m_bExternalTitle : 1;


        // attribute access
        ::osl::Mutex&               getMutex() const            { return OGenericUnoController_MBASE::getMutex(); }
        ::cppu::OBroadcastHelper&   getBroadcastHelper()        { return OGenericUnoController_Base::rBHelper; }


        // methods
        OGenericUnoController( const css::uno::Reference< css::uno::XComponentContext >& _rM );
        const ::comphelper::NamedValueCollection&
                                    getInitParams() const   { return m_aInitParameters; }


        /** open the help agent for the given help id.
            @param  _nHelpId
                The help id to dispatch.
        */
        void openHelpAgent( const OString& _sHelpId );

        /** open the help agent for the given help url.
            @param  _pHelpStringURL
                The help url to dispatch.
        */
        void openHelpAgent( const OUString& _suHelpStringURL );

        /** opens the given Help URL in the help agent

            The URL does not need to be parsed already, it is passed through
            XURLTransformer::parseStrict before it is used.
        */
        void openHelpAgent( const css::util::URL& _rURL );

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

            @param _pAsciiCommandURL
                the URL of the feature command
            @param _nFeatureId
                the id of the feature. Later references to this feature usually happen by id, not by
                URL
            @param _nCommandGroup
                the command group of the feature. This is important for configuring the controller UI
                by the user, see also <type scope="css::frame">CommandGroup</type>.
        */
        void    implDescribeSupportedFeature(
                    const sal_Char* _pAsciiCommandURL,
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
            const css::uno::Reference< css::sdbc::XDataSource>& _xDataSource,
            ::dbtools::SQLExceptionInfo* _pErrorInfo
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
        virtual void impl_initialize();

        virtual OUString getPrivateTitle() const { return OUString(); }

        css::uno::Reference< css::frame::XTitle > impl_getTitleHelper_throw();
        virtual css::uno::Reference< css::frame::XModel > getPrivateModel() const
        {
            return css::uno::Reference< css::frame::XModel >();
        }

        virtual void    startFrameListening( const css::uno::Reference< css::frame::XFrame >& _rxFrame );
        virtual void    stopFrameListening( const css::uno::Reference< css::frame::XFrame >& _rxFrame );

        void releaseNumberForComponent();

        virtual ~OGenericUnoController();

    private:
        void fillSupportedFeatures();

        void InvalidateAll_Impl();
        void InvalidateFeature_Impl();

        void ImplInvalidateFeature( sal_Int32 _nId, const css::uno::Reference< css::frame::XStatusListener >& _xListener, bool _bForceBroadcast );

        void ImplBroadcastFeatureState(const OUString& _rFeature, const css::uno::Reference< css::frame::XStatusListener > & xListener, bool _bIgnoreCache);

        // link methods
        DECL_LINK_TYPED(OnAsyncInvalidateAll, void*, void);
        DECL_LINK_TYPED(OnAsyncCloseTask, void*, void);

    public:
        css::uno::Reference< css::uno::XComponentContext >  getORB() const { return m_xContext; }
        ODataView*  getView() const { return m_pView; }
        void        setView( const VclPtr<ODataView>& i_rView );
        void        clearView();
        // shows a error box if the SQLExceptionInfo is valid
        void showError(const ::dbtools::SQLExceptionInfo& _rInfo);

        // if there is an css::util::URL translation for the id
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
        virtual sal_uInt16 registerCommandURL( const OUString& _rCompleteCommandURL ) override;
        virtual void notifyHiContrastChanged() override;
        virtual bool isDataSourceReadOnly() const override;
        virtual css::uno::Reference< css::frame::XController > getXController() throw( css::uno::RuntimeException ) override;
        virtual bool interceptUserInput( const NotifyEvent& _rEvent ) override;

        // misc
        bool isCommandChecked(sal_uInt16 _nCommandId) const;

        // css::lang::XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& Source) throw( css::uno::RuntimeException, std::exception ) override;

        // css::util::XModifyListener
        virtual void SAL_CALL modified(const css::lang::EventObject& aEvent) throw( css::uno::RuntimeException, std::exception ) override;

        // XInterface
        virtual void SAL_CALL acquire(  ) throw () override;
        virtual void SAL_CALL release(  ) throw () override;

        // css::frame::XController2
        virtual css::uno::Reference< css::awt::XWindow > SAL_CALL getComponentWindow() throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getViewControllerName() throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getCreationArguments() throw (css::uno::RuntimeException, std::exception) override;

        virtual css::uno::Reference< css::ui::XSidebarProvider > SAL_CALL getSidebar() throw (css::uno::RuntimeException, std::exception) override;


        // css::frame::XController
        virtual void SAL_CALL attachFrame(const css::uno::Reference< css::frame::XFrame > & xFrame) throw( css::uno::RuntimeException, std::exception ) override;
        virtual sal_Bool SAL_CALL attachModel(const css::uno::Reference< css::frame::XModel > & xModel) throw( css::uno::RuntimeException, std::exception ) override;
        virtual sal_Bool SAL_CALL suspend(sal_Bool bSuspend) throw( css::uno::RuntimeException, std::exception ) override = 0;
        virtual css::uno::Any SAL_CALL getViewData() throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL restoreViewData(const css::uno::Any& Data) throw( css::uno::RuntimeException, std::exception ) override;
        virtual css::uno::Reference< css::frame::XModel >  SAL_CALL getModel() throw( css::uno::RuntimeException, std::exception ) override;
        virtual css::uno::Reference< css::frame::XFrame >  SAL_CALL getFrame() throw( css::uno::RuntimeException, std::exception ) override;

        // css::frame::XDispatch
        virtual void        SAL_CALL dispatch(const css::util::URL& aURL, const css::uno::Sequence< css::beans::PropertyValue>& aArgs) throw(css::uno::RuntimeException, std::exception) override;
        virtual void        SAL_CALL addStatusListener(const css::uno::Reference< css::frame::XStatusListener > & aListener, const css::util::URL& aURL) throw(css::uno::RuntimeException, std::exception) override;
        virtual void        SAL_CALL removeStatusListener(const css::uno::Reference< css::frame::XStatusListener > & aListener, const css::util::URL& aURL) throw(css::uno::RuntimeException, std::exception) override;

        // css::frame::XDispatchProviderInterceptor
        virtual css::uno::Reference< css::frame::XDispatchProvider >  SAL_CALL getSlaveDispatchProvider() throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setSlaveDispatchProvider(const css::uno::Reference< css::frame::XDispatchProvider > & _xNewProvider) throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::frame::XDispatchProvider >  SAL_CALL getMasterDispatchProvider() throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setMasterDispatchProvider(const css::uno::Reference< css::frame::XDispatchProvider > & _xNewProvider) throw(css::uno::RuntimeException, std::exception) override;

        // css::frame::XDispatchProvider
        virtual css::uno::Reference< css::frame::XDispatch >  SAL_CALL queryDispatch(const css::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags) throw( css::uno::RuntimeException, std::exception ) override;
        virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch >  > SAL_CALL queryDispatches(const css::uno::Sequence< css::frame::DispatchDescriptor >& aDescripts) throw( css::uno::RuntimeException, std::exception ) override;

        // css::lang::XComponent
        virtual void SAL_CALL dispose() throw(css::uno::RuntimeException, std::exception) override; //LLA: need solar mutex {OGenericUnoController_COMPBASE::dispose(); }
        virtual void SAL_CALL disposing() override;
        virtual void SAL_CALL addEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener) throw(css::uno::RuntimeException, std::exception) override;

        // css::frame::XFrameActionListener
        virtual void SAL_CALL frameAction(const css::frame::FrameActionEvent& aEvent) throw( css::uno::RuntimeException, std::exception ) override;
        // lang::XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw(css::uno::Exception, css::uno::RuntimeException, std::exception) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(css::uno::RuntimeException, std::exception) override = 0;
        virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString> SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override = 0;

        // XDispatchInformationProvider
        virtual css::uno::Sequence< ::sal_Int16 > SAL_CALL getSupportedCommandGroups() throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< css::frame::DispatchInformation > SAL_CALL getConfigurableDispatchInformation( ::sal_Int16 ) throw (css::uno::RuntimeException, std::exception) override;

        // XTitle
        virtual OUString SAL_CALL getTitle(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setTitle( const OUString& sTitle ) throw (css::uno::RuntimeException, std::exception) override;

        // XTitleChangeBroadcaster
        virtual void SAL_CALL addTitleChangeListener( const css::uno::Reference< css::frame::XTitleChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeTitleChangeListener( const css::uno::Reference< css::frame::XTitleChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;

        // XUserInputInterception
        virtual void SAL_CALL addKeyHandler( const css::uno::Reference< css::awt::XKeyHandler >& xHandler ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeKeyHandler( const css::uno::Reference< css::awt::XKeyHandler >& xHandler ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addMouseClickHandler( const css::uno::Reference< css::awt::XMouseClickHandler >& xHandler ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeMouseClickHandler( const css::uno::Reference< css::awt::XMouseClickHandler >& xHandler ) throw (css::uno::RuntimeException, std::exception) override;

    protected:
#ifdef _MSC_VER
        OGenericUnoController();    // never implemented
#endif
    };
}

#endif // INCLUDED_DBACCESS_GENERICCONTROLLER_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
