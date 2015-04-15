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
#include <cppuhelper/compbase11.hxx>
#include <cppuhelper/interfacecontainer.h>

#include <boost/optional.hpp>
#include <sfx2/userinputinterception.hxx>

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
    inline bool SAL_CALL operator >>= ( const ::com::sun::star::uno::Any & _any, optional< T >& _value )
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
        ::com::sun::star::uno::Any  aValue;
        optional< OUString > sTitle;

        FeatureState() : bEnabled(false) { }
    };


    // = helper



    struct ControllerFeature : public ::com::sun::star::frame::DispatchInformation
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
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >
                    xListener;
        sal_Int32   nId;
        bool        bForceBroadcast;
    };


    typedef ::std::deque< FeatureListener > FeatureListeners;


    struct FindFeatureListener : ::std::binary_function< FeatureListener, ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >, bool >
    {

        inline bool operator()( const FeatureListener& lhs, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& rhs ) const
        {
            return !!( lhs.xListener == rhs );
        }
    };


    typedef ::comphelper::SharedMutexBase   OGenericUnoController_MBASE;

    typedef ::cppu::WeakComponentImplHelper11   <   ::com::sun::star::frame::XDispatch
                                                ,   ::com::sun::star::frame::XDispatchProviderInterceptor
                                                ,   ::com::sun::star::util::XModifyListener
                                                ,   ::com::sun::star::frame::XFrameActionListener
                                                ,   ::com::sun::star::lang::XInitialization
                                                ,   ::com::sun::star::lang::XServiceInfo
                                                ,   ::com::sun::star::frame::XDispatchInformationProvider
                                                ,   ::com::sun::star::frame::XController2
                                                ,   ::com::sun::star::frame::XTitle
                                                ,   ::com::sun::star::frame::XTitleChangeBroadcaster
                                                ,   ::com::sun::star::awt::XUserInputInterception
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
        ODataView*                      m_pView;                // our (VCL) "main window"

#ifdef DBG_UTIL
        bool                            m_bDescribingSupportedFeatures;
#endif

    protected:

        // attributes
        struct DispatchTarget
        {
            ::com::sun::star::util::URL                 aURL;
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >    xListener;

            DispatchTarget() { }
            DispatchTarget(const ::com::sun::star::util::URL& rURL, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& rRef) : aURL(rURL), xListener(rRef) { }
        };

        typedef std::map<sal_uInt16, FeatureState> StateCache;
        typedef std::vector<DispatchTarget> Dispatch;

        FeatureListeners        m_aFeaturesToInvalidate;

        ::osl::Mutex            m_aFeatureMutex;        // locked when features are append to or remove from deque
        StateCache              m_aStateCache;          // save the current status of feature state
        Dispatch                m_arrStatusListener;    // all our listeners where we dispatch status changes
        OAsynchronousLink        m_aAsyncInvalidateAll;
        OAsynchronousLink        m_aAsyncCloseTask;      // called when a task should be closed

        ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer >         m_xUrlTransformer;      // needed sometimes
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >        m_xContext;
        ControllerFrame                                                                     m_aCurrentFrame;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >      m_xSlaveDispatcher;     // for intercepting dispatches
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >      m_xMasterDispatcher;    // dito
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XDatabaseContext >         m_xDatabaseContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XTitle >                 m_xTitleHelper;

        bool                    m_bPreview;
        bool                    m_bReadOnly;

        bool                    m_bCurrentlyModified : 1;
        bool                    m_bExternalTitle : 1;




        // attribute access
        ::osl::Mutex&               getMutex() const            { return OGenericUnoController_MBASE::getMutex(); }
        ::cppu::OBroadcastHelper&   getBroadcastHelper()        { return OGenericUnoController_Base::rBHelper; }


        // methods
        OGenericUnoController( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rM );
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
        void openHelpAgent( const ::com::sun::star::util::URL& _rURL );

        // closes the task when possible
        void closeTask();

        // if getMenu returns a non empty string than this will be dispatched at the frame
        virtual void            loadMenu(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _xFrame);

        /** called when our menu has been loaded into our frame, can be used to load sub toolbars

            @param _xLayoutManager
                The layout manager.
        */
        virtual void            onLoadedMenu(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager >& _xLayoutManager);

        // all the features which should be handled by this class
        virtual void            describeSupportedFeatures();

        // state of a feature. 'feature' may be the handle of a ::com::sun::star::util::URL somebody requested a dispatch interface for OR a toolbar slot.
        virtual FeatureState    GetState(sal_uInt16 nId) const;
        // execute a feature
        virtual void            Execute(sal_uInt16 nId , const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs);

        /** describes a feature supported by the controller

            Must not be called outside <member>describeSupportedFeatures</member>.

            @param _pAsciiCommandURL
                the URL of the feature command
            @param _nFeatureId
                the id of the feature. Later references to this feature usually happen by id, not by
                URL
            @param _nCommandGroup
                the command group of the feature. This is important for configuring the controller UI
                by the user, see also <type scope="com::sun::star::frame">CommandGroup</type>.
        */
        void    implDescribeSupportedFeature(
                    const sal_Char* _pAsciiCommandURL,
                    sal_uInt16 _nFeatureId,
                    sal_Int16 _nCommandGroup = ::com::sun::star::frame::CommandGroup::INTERNAL
                );

        /** returns <TRUE/> if the feature is supported, otherwise <FALSE/>
            @param  _nId
                The ID of the feature.
        */
        bool isFeatureSupported( sal_Int32 _nId );

        // gets the URL which the given id is assigned to
        ::com::sun::star::util::URL getURLForId(sal_Int32 _nId) const;

        /** determines whether the given feature ID denotes a user-defined feature

            @see IController::registerCommandURL
        */
        bool    isUserDefinedFeature( const sal_uInt16 nFeatureId ) const;

        /** determines whether the given feature URL denotes a user-defined feature

            @see IController::registerCommandURL
        */
        bool    isUserDefinedFeature( const OUString& _rFeatureURL ) const;

        // connect to a datasource
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > connect(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource>& _xDataSource,
            ::dbtools::SQLExceptionInfo* _pErrorInfo
        );

        // connect to a datasource
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > connect(
            const OUString& _rsDataSourceName,
            const OUString& _rContextInformation,
            ::dbtools::SQLExceptionInfo* _pErrorInfo
        );

        void startConnectionListening(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection);
        void stopConnectionListening(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection);

        /** return the container window of the top most frame
            @return
                The top most container window, nmay be <NULL/>.
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow> getTopMostContainerWindow() const;

        // XInitialize will be called inside initialize
        virtual void impl_initialize();

        virtual OUString getPrivateTitle() const { return OUString(); }

        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XTitle > impl_getTitleHelper_throw();
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > getPrivateModel() const
        {
            return ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >();
        }

        virtual void    startFrameListening( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxFrame );
        virtual void    stopFrameListening( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxFrame );

        void releaseNumberForComponent();

        virtual ~OGenericUnoController();

    private:
        void fillSupportedFeatures();

        void InvalidateAll_Impl();
        void InvalidateFeature_Impl();

        void ImplInvalidateFeature( sal_Int32 _nId, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& _xListener, bool _bForceBroadcast );

        bool ImplInvalidateTBItem(sal_uInt16 nId, const FeatureState& rState);
        void ImplBroadcastFeatureState(const OUString& _rFeature, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xListener, bool _bIgnoreCache);

        // link methods
        DECL_LINK(OnAsyncInvalidateAll, void*);
        DECL_LINK(OnAsyncCloseTask, void*);

    public:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >  getORB() const { return m_xContext; }
        ODataView*  getView() const { return m_pView; }
        void        setView( ODataView& i_rView ) { m_pView = &i_rView; }
        void        clearView() { m_pView = NULL; }
        // shows a error box if the SQLExceptionInfo is valid
        void showError(const ::dbtools::SQLExceptionInfo& _rInfo);

        // if there is an ::com::sun::star::util::URL translation for the id
        // ('handle') then if if xListener is NULL the change will be forwarded
        // to all listeners to the given ::com::sun::star::util::URL
        // if there is a toolbar slot with the given id it is updated (the new state is determined via GetState)
        // if _bForceBroadcast is sal_True, the current feature state is broadcasted no matter if it is the same as the cached state
        void InvalidateFeature(sal_uInt16 nId, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xListener = NULL, bool _bForceBroadcast = false);

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
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager > getLayoutManager(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _xFrame) const;

        // IController
        virtual void executeUnChecked(const ::com::sun::star::util::URL& _rCommand, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs) SAL_OVERRIDE;
        virtual void executeChecked(const ::com::sun::star::util::URL& _rCommand, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs) SAL_OVERRIDE;
        virtual void executeUnChecked(sal_uInt16 _nCommandId, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs) SAL_OVERRIDE;
        virtual void executeChecked(sal_uInt16 _nCommandId, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs) SAL_OVERRIDE;
        virtual bool isCommandEnabled(sal_uInt16 _nCommandId) const SAL_OVERRIDE;
        virtual bool isCommandEnabled(const OUString& _rCompleteCommandURL) const SAL_OVERRIDE;
        virtual sal_uInt16 registerCommandURL( const OUString& _rCompleteCommandURL ) SAL_OVERRIDE;
        virtual void notifyHiContrastChanged() SAL_OVERRIDE;
        virtual bool isDataSourceReadOnly() const SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController > getXController() throw( ::com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;
        virtual bool interceptUserInput( const NotifyEvent& _rEvent ) SAL_OVERRIDE;

        // misc
        bool isCommandChecked(sal_uInt16 _nCommandId) const;

        // ::com::sun::star::lang::XEventListener
        virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // ::com::sun::star::util::XModifyListener
        virtual void SAL_CALL modified(const ::com::sun::star::lang::EventObject& aEvent) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // XInterface
        virtual void SAL_CALL acquire(  ) throw () SAL_OVERRIDE;
        virtual void SAL_CALL release(  ) throw () SAL_OVERRIDE;

        // ::com::sun::star::frame::XController2
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL getComponentWindow() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual OUString SAL_CALL getViewControllerName() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getCreationArguments() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // ::com::sun::star::frame::XController
        virtual void SAL_CALL attachFrame(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > & xFrame) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL attachModel(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & xModel) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL suspend(sal_Bool bSuspend) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE = 0;
        virtual ::com::sun::star::uno::Any SAL_CALL getViewData() throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
        virtual void SAL_CALL restoreViewData(const ::com::sun::star::uno::Any& Data) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >  SAL_CALL getModel() throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >  SAL_CALL getFrame() throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // ::com::sun::star::frame::XDispatch
        virtual void        SAL_CALL dispatch(const ::com::sun::star::util::URL& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void        SAL_CALL addStatusListener(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & aListener, const ::com::sun::star::util::URL& aURL) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void        SAL_CALL removeStatusListener(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & aListener, const ::com::sun::star::util::URL& aURL) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // ::com::sun::star::frame::XDispatchProviderInterceptor
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >  SAL_CALL getSlaveDispatchProvider() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setSlaveDispatchProvider(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > & _xNewProvider) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >  SAL_CALL getMasterDispatchProvider() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setMasterDispatchProvider(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > & _xNewProvider) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // ::com::sun::star::frame::XDispatchProvider
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  SAL_CALL queryDispatch(const ::com::sun::star::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  > SAL_CALL queryDispatches(const ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchDescriptor >& aDescripts) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // ::com::sun::star::lang::XComponent
        virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE; //LLA: need solar mutex {OGenericUnoController_COMPBASE::dispose(); }
        virtual void SAL_CALL disposing() SAL_OVERRIDE;
        virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // ::com::sun::star::frame::XFrameActionListener
        virtual void        SAL_CALL frameAction(const ::com::sun::star::frame::FrameActionEvent& aEvent) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
        // lang::XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE = 0;
        virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< OUString> SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE = 0;

        // XDispatchInformationProvider
        virtual ::com::sun::star::uno::Sequence< ::sal_Int16 > SAL_CALL getSupportedCommandGroups() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchInformation > SAL_CALL getConfigurableDispatchInformation( ::sal_Int16 ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XTitle
        virtual OUString SAL_CALL getTitle(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setTitle( const OUString& sTitle ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XTitleChangeBroadcaster
        virtual void SAL_CALL addTitleChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XTitleChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL removeTitleChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XTitleChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XUserInputInterception
        virtual void SAL_CALL addKeyHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyHandler >& xHandler ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL removeKeyHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyHandler >& xHandler ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL addMouseClickHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseClickHandler >& xHandler ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL removeMouseClickHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseClickHandler >& xHandler ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    protected:
#ifdef _MSC_VER
        OGenericUnoController();    // never implemented
#endif
    };
}

#endif // INCLUDED_DBACCESS_GENERICCONTROLLER_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
