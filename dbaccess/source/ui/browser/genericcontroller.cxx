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

#include <dbaccess/genericcontroller.hxx>
#include <comphelper/uno3.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include "browserids.hxx"
#include <vcl/svapp.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <dbaccess/dataview.hxx>
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>
#include "dbustrings.hrc"
#include <vcl/stdtext.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <framework/titlehelper.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/extract.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include "UITools.hxx"
#include "commontypes.hxx"

#include <com/sun/star/ui/XSidebarProvider.hpp>
#include <sfx2/sidebar/UnoSidebar.hxx>


#include <vcl/waitobj.hxx>
#include <svl/urihelper.hxx>
#include "datasourceconnector.hxx"
#include <unotools/moduleoptions.hxx>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/status/Visibility.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <rtl/ustring.hxx>
#include <o3tl/functional.hxx>
#include <boost/scoped_ptr.hpp>
#include <cppuhelper/implbase1.hxx>
#include <limits>
#include <unordered_map>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::frame::status;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::ui;
using namespace ::dbtools;
using namespace ::comphelper;

#define ALL_FEATURES                -1
#define FIRST_USER_DEFINED_FEATURE  ( ::std::numeric_limits< sal_uInt16 >::max() - 1000 )
#define LAST_USER_DEFINED_FEATURE   ( ::std::numeric_limits< sal_uInt16 >::max()        )

typedef std::unordered_map< sal_Int16, sal_Int16 > CommandHashMap;

namespace dbaui
{

// UserDefinedFeatures
class UserDefinedFeatures
{
public:
    explicit UserDefinedFeatures( const Reference< XController >& _rxController );

    static FeatureState getState( const URL& _rFeatureURL );
    void            execute( const URL& _rFeatureURL, const Sequence< PropertyValue>& _rArgs );

private:
    css::uno::WeakReference< XController > m_aController;
};

UserDefinedFeatures::UserDefinedFeatures( const Reference< XController >& _rxController )
    :m_aController( _rxController )
{
}

FeatureState UserDefinedFeatures::getState( const URL& /*_rFeatureURL*/ )
{
    // for now, enable all the time
    // TODO: we should ask the dispatcher. However, this is laborious, since you cannot ask a dispatcher
    // directly, but need to add a status listener.
    FeatureState aState;
    aState.bEnabled = true;
    return aState;
}

void UserDefinedFeatures::execute( const URL& _rFeatureURL, const Sequence< PropertyValue>& _rArgs )
{
    try
    {
        Reference< XController > xController( Reference< XController >(m_aController), UNO_SET_THROW );
        Reference< XDispatchProvider > xDispatchProvider( xController->getFrame(), UNO_QUERY_THROW );
        Reference< XDispatch > xDispatch( xDispatchProvider->queryDispatch(
            _rFeatureURL,
            OUString( "_self" ),
            FrameSearchFlag::AUTO
        ) );

        if ( xDispatch == xController )
        {
            SAL_WARN("dbaccess.ui", "UserDefinedFeatures::execute: the controller shouldn't be the dispatcher here!" );
            xDispatch.clear();
        }

        if ( xDispatch.is() )
            xDispatch->dispatch( _rFeatureURL, _rArgs );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

// OGenericUnoController_Data
struct OGenericUnoController_Data
{
    ::sfx2::UserInputInterception   m_aUserInputInterception;
    UserDefinedFeatures             m_aUserDefinedFeatures;

    OGenericUnoController_Data( OGenericUnoController& _rController, ::osl::Mutex& _rMutex )
        :m_aUserInputInterception( _rController, _rMutex )
        ,m_aUserDefinedFeatures( _rController.getXController() )
    {
    }
};

// OGenericUnoController
OGenericUnoController::OGenericUnoController(const Reference< XComponentContext >& _rM)
    :OGenericUnoController_Base( getMutex() )
    ,m_pView(NULL)
#ifdef DBG_UTIL
    ,m_bDescribingSupportedFeatures( false )
#endif
    ,m_aAsyncInvalidateAll(LINK(this, OGenericUnoController, OnAsyncInvalidateAll))
    ,m_aAsyncCloseTask(LINK(this, OGenericUnoController, OnAsyncCloseTask))
    ,m_xContext(_rM)
    ,m_aCurrentFrame( *this )
    ,m_bPreview(false)
    ,m_bReadOnly(false)
    ,m_bCurrentlyModified(false)
    ,m_bExternalTitle(false)
{
    osl_atomic_increment( &m_refCount );
    {
        m_pData.reset( new OGenericUnoController_Data( *this, getMutex() ) );
    }
    osl_atomic_decrement( &m_refCount );


    try
    {
        m_xUrlTransformer = URLTransformer::create(_rM);
    }
    catch(Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

#ifdef _MSC_VER

#pragma warning(push)
#pragma warning(disable:4702)

OGenericUnoController::OGenericUnoController()
    :OGenericUnoController_Base( getMutex() )
    ,m_pView(NULL)
#ifdef DBG_UTIL
    ,m_bDescribingSupportedFeatures( false )
#endif
    ,m_aAsyncInvalidateAll(LINK(this, OGenericUnoController, OnAsyncInvalidateAll))
    ,m_aAsyncCloseTask(LINK(this, OGenericUnoController, OnAsyncCloseTask))
    ,m_aCurrentFrame( *this )
    ,m_bPreview(sal_False)
    ,m_bReadOnly(sal_False)
    ,m_bCurrentlyModified(sal_False)
{
    SAL_WARN("dbaccess.ui", "OGenericUnoController::OGenericUnoController: illegal call!" );
    // This ctor only exists because the MSVC compiler complained about an unresolved external
    // symbol. It should not be used at all. Since using it yields strange runtime problems,
    // we simply abort here.
    abort();
}

#pragma warning(pop)

#endif

OGenericUnoController::~OGenericUnoController()
{

}

bool OGenericUnoController::Construct(vcl::Window* /*pParent*/)
{
    OSL_ENSURE( getView(), "the view is NULL!" );

    if ( getView() )
    {
        getView()->Construct();
        getView()->Show();
    }

    m_aSupportedFeatures.clear();
    fillSupportedFeatures();

    // create the database context
    OSL_ENSURE(getORB().is(), "OGenericUnoController::Construct need a service factory!");
    try
    {
        m_xDatabaseContext = DatabaseContext::create(getORB());
    }
    catch(const Exception&)
    {
        SAL_WARN("dbaccess.ui","OGenericUnoController::Construct: could not create (or start listening at) the database context!");
        // at least notify the user. Though the whole component does not make any sense without the database context ...
        ShowServiceNotAvailableError(getView(), OUString("com.sun.star.sdb.DatabaseContext"), true);
    }

    return true;
}

IMPL_LINK_NOARG_TYPED(OGenericUnoController, OnAsyncInvalidateAll, void*, void)
{
    if ( !OGenericUnoController_Base::rBHelper.bInDispose && !OGenericUnoController_Base::rBHelper.bDisposed )
        InvalidateFeature_Impl();
}

void OGenericUnoController::impl_initialize()
{
}

void SAL_CALL OGenericUnoController::initialize( const Sequence< Any >& aArguments ) throw(Exception, RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getMutex() );

    Reference< XWindow >        xParent;
    Reference< XFrame > xFrame;

    PropertyValue aValue;
    const Any* pIter    = aArguments.getConstArray();
    const Any* pEnd     = pIter + aArguments.getLength();

    for ( ; pIter != pEnd; ++pIter )
    {
        if ( ( *pIter >>= aValue ) && aValue.Name == "Frame" )
        {
            xFrame.set(aValue.Value,UNO_QUERY_THROW);
        }
        else if ( ( *pIter >>= aValue ) && aValue.Name == "Preview" )
        {
            aValue.Value >>= m_bPreview;
            m_bReadOnly = true;
        }
    }
    try
    {
        if ( !xFrame.is() )
            throw IllegalArgumentException("need a frame", *this, 1 );

        xParent = xFrame->getContainerWindow();
        VCLXWindow* pParentComponent = VCLXWindow::GetImplementation(xParent);
        VclPtr< vcl::Window > pParentWin = pParentComponent ? pParentComponent->GetWindow() : VclPtr< vcl::Window >();
        if (!pParentWin)
        {
            throw IllegalArgumentException("Parent window is null", *this, 1 );
        }

        m_aInitParameters.assign( aArguments );
        Construct( pParentWin );

        ODataView* pView = getView();
        if ( !pView )
            throw RuntimeException("unable to create a view", *this );

        if ( m_bReadOnly || m_bPreview )
            pView->EnableInput( false );

        impl_initialize();
    }
    catch(Exception&)
    {
        // no one clears my view if I won't
        m_pView = NULL;
        throw;
    }
}

void SAL_CALL OGenericUnoController::acquire(  ) throw ()
{
    OGenericUnoController_Base::acquire();
}

void SAL_CALL OGenericUnoController::release(  ) throw ()
{
    OGenericUnoController_Base::release();
}

void OGenericUnoController::startFrameListening( const Reference< XFrame >& _rxFrame )
{
    if ( _rxFrame.is() )
        _rxFrame->addFrameActionListener( this );
}

void OGenericUnoController::stopFrameListening( const Reference< XFrame >& _rxFrame )
{
    if ( _rxFrame.is() )
        _rxFrame->removeFrameActionListener( this );
}

void OGenericUnoController::disposing(const EventObject& Source) throw( RuntimeException, std::exception )
{
    // our frame ?
    if ( Source.Source == getFrame() )
        stopFrameListening( getFrame() );
}

void OGenericUnoController::modified(const EventObject& aEvent) throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( getMutex() );
    if ( !isDataSourceReadOnly() )
    {
        Reference<XModifiable> xModi(aEvent.Source,UNO_QUERY);
        if ( xModi.is() )
            m_bCurrentlyModified = xModi->isModified(); // can only be reset by save
        else
            m_bCurrentlyModified = true;
    }
    InvalidateFeature(ID_BROWSER_SAVEDOC);
    InvalidateFeature(ID_BROWSER_UNDO);
}

Reference< XWindow > SAL_CALL OGenericUnoController::getComponentWindow() throw (RuntimeException, std::exception)
{
    SolarMutexGuard g;
    return VCLUnoHelper::GetInterface( getView() );
}

Reference<XSidebarProvider> SAL_CALL OGenericUnoController::getSidebar() throw (RuntimeException, std::exception)
{
        return NULL;
}

OUString SAL_CALL OGenericUnoController::getViewControllerName() throw (css::uno::RuntimeException, std::exception)
{
    return OUString( "Default" );
}

Sequence< PropertyValue > SAL_CALL OGenericUnoController::getCreationArguments() throw (RuntimeException, std::exception)
{
    // currently we do not support any creation args, so anything passed to XModel2::createViewController would be
    // lost, so we can equally return an empty sequence here
    return Sequence< PropertyValue >();
}

void OGenericUnoController::attachFrame( const Reference< XFrame >& _rxFrame ) throw( RuntimeException, std::exception )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getMutex() );

    stopFrameListening( m_aCurrentFrame.getFrame() );
    Reference< XFrame > xFrame = m_aCurrentFrame.attachFrame( _rxFrame );
    startFrameListening( xFrame );

    loadMenu( xFrame );

    if ( getView() )
        getView()->attachFrame( xFrame );
}

namespace
{
    typedef ::std::vector< Any >    States;

    void    lcl_notifyMultipleStates( XStatusListener& _rListener, FeatureStateEvent& _rEvent, const States& _rStates )
    {
        for (   States::const_iterator state = _rStates.begin();
                state != _rStates.end();
                ++state
            )
        {
            _rEvent.State = *state;
            _rListener.statusChanged( _rEvent );
        }
    }

    void    lcl_collectStates( const FeatureState& _rFeatureState, States& _out_rStates )
    {
        // order matters, due to a bug in framework which resets the check state when any non-boolean event
        // arrives
        // #i68215# is the bug to (re-)introduce this "ordered" notification here
        // #i67882# is the bug which was caused by the real fix which we did in framework
        // #i68216# is the bug which requests to fix the code in Draw which relies on
        //          framework's implementation details
        if ( !!_rFeatureState.sTitle )
            _out_rStates.push_back( makeAny( *_rFeatureState.sTitle ) );
        if ( !!_rFeatureState.bChecked )
            _out_rStates.push_back( makeAny( *_rFeatureState.bChecked ) );
        if ( !!_rFeatureState.bInvisible )
            _out_rStates.push_back( makeAny( Visibility( !*_rFeatureState.bInvisible ) ) );
        if ( _rFeatureState.aValue.hasValue() )
            _out_rStates.push_back( _rFeatureState.aValue );
        if ( _out_rStates.empty() )
            _out_rStates.push_back( Any() );
    }
}

void OGenericUnoController::ImplBroadcastFeatureState(const OUString& _rFeature, const Reference< XStatusListener > & xListener, bool _bIgnoreCache)
{
    sal_uInt16 nFeat = m_aSupportedFeatures[ _rFeature ].nFeatureId;
    FeatureState aFeatState( GetState( nFeat ) );

    FeatureState& rCachedState = m_aStateCache[nFeat];  // creates if necessary
    if ( !_bIgnoreCache )
    {
        // check if we really need to notify the listeners : this method may be called much more often than needed, so check
        // the cached state of the feature
        bool bAlreadyCached = ( m_aStateCache.find(nFeat) != m_aStateCache.end() );
        if ( bAlreadyCached )
            if  (   ( rCachedState.bEnabled == aFeatState.bEnabled )
                &&  ( rCachedState.bChecked == aFeatState.bChecked )
                &&  ( rCachedState.bInvisible == aFeatState.bInvisible )
                &&  ( rCachedState.sTitle == aFeatState.sTitle )
                )
            return;
    }
    rCachedState = aFeatState;

    FeatureStateEvent aEvent;
    aEvent.FeatureURL.Complete = _rFeature;
    if (m_xUrlTransformer.is())
        m_xUrlTransformer->parseStrict(aEvent.FeatureURL);
    aEvent.Source       = static_cast<XDispatch*>(this);
    aEvent.IsEnabled    = aFeatState.bEnabled;

    // collect all states to be notified
    States aStates;
    lcl_collectStates( aFeatState, aStates );

    // a special listener ?
    if ( xListener.is() )
        lcl_notifyMultipleStates( *xListener.get(), aEvent, aStates );
    else
    {   // no -> iterate through all listeners responsible for the URL
        StringBag aFeatureCommands;
        for( const auto& rFeature : m_aSupportedFeatures )
        {
            if( rFeature.second.nFeatureId == nFeat )
                aFeatureCommands.insert( rFeature.first );
        }

        // it is possible that listeners are registered or revoked while
        // we are notifying them, so we must use a copy of m_arrStatusListener, not
        // m_arrStatusListener itself
        Dispatch aNotifyLoop( m_arrStatusListener );
        Dispatch::iterator iterSearch = aNotifyLoop.begin();
        Dispatch::iterator iterEnd = aNotifyLoop.end();

        while (iterSearch != iterEnd)
        {
            DispatchTarget& rCurrent = *iterSearch;
            if ( aFeatureCommands.find( rCurrent.aURL.Complete ) != aFeatureCommands.end() )
            {
                aEvent.FeatureURL = rCurrent.aURL;
                lcl_notifyMultipleStates( *rCurrent.xListener.get(), aEvent, aStates );
            }
            ++iterSearch;
        }
    }

}

bool OGenericUnoController::isFeatureSupported( sal_Int32 _nId )
{
    SupportedFeatures::iterator aFeaturePos = ::std::find_if(
        m_aSupportedFeatures.begin(),
        m_aSupportedFeatures.end(),
        ::std::bind2nd( CompareFeatureById(), _nId )
    );

    return ( m_aSupportedFeatures.end() != aFeaturePos && !aFeaturePos->first.isEmpty());
}

void OGenericUnoController::InvalidateFeature_Impl()
{
#ifdef DBG_UTIL
    static sal_Int32 s_nRecursions = 0;
    ++s_nRecursions;
#endif

    bool bEmpty = true;
    FeatureListener aNextFeature;
    {
        ::osl::MutexGuard aGuard( m_aFeatureMutex);
        bEmpty = m_aFeaturesToInvalidate.empty();
        if (!bEmpty)
            aNextFeature = m_aFeaturesToInvalidate.front();
    }
    while(!bEmpty)
    {
        if ( ALL_FEATURES == aNextFeature.nId )
        {
            InvalidateAll_Impl();
            break;
        }
        else
        {
            SupportedFeatures::iterator aFeaturePos = ::std::find_if(
                m_aSupportedFeatures.begin(),
                m_aSupportedFeatures.end(),
                ::std::bind2nd( CompareFeatureById(), aNextFeature.nId )
            );

#if OSL_DEBUG_LEVEL > 0
            if ( m_aSupportedFeatures.end() == aFeaturePos )
            {
                OString sMessage( "OGenericUnoController::InvalidateFeature_Impl: feature id " );
                sMessage += OString::number( aNextFeature.nId );
                sMessage += OString( " has been invalidated, but is not supported!" );
                SAL_WARN("dbaccess.ui", sMessage.getStr() );
            }
#endif
            if ( m_aSupportedFeatures.end() != aFeaturePos )
                // we really know this feature
                ImplBroadcastFeatureState( aFeaturePos->first, aNextFeature.xListener, aNextFeature.bForceBroadcast );
        }

        ::osl::MutexGuard aGuard( m_aFeatureMutex);
        m_aFeaturesToInvalidate.pop_front();
        bEmpty = m_aFeaturesToInvalidate.empty();
        if (!bEmpty)
            aNextFeature = m_aFeaturesToInvalidate.front();
    }

#ifdef DBG_UTIL
    --s_nRecursions;
#endif
}

void OGenericUnoController::ImplInvalidateFeature( sal_Int32 _nId, const Reference< XStatusListener >& _xListener, bool _bForceBroadcast )
{
#if OSL_DEBUG_LEVEL > 0
    if ( _nId != -1 )
    {
        SupportedFeatures::iterator aFeaturePos = ::std::find_if(
            m_aSupportedFeatures.begin(),
            m_aSupportedFeatures.end(),
            ::std::bind2nd( CompareFeatureById(), _nId )
        );
        OSL_ENSURE( aFeaturePos != m_aSupportedFeatures.end(), "OGenericUnoController::ImplInvalidateFeature: invalidating an unsupported feature is suspicious, at least!" );
    }
#endif

    FeatureListener aListener;
    aListener.nId               = _nId;
    aListener.xListener         = _xListener;
    aListener.bForceBroadcast   = _bForceBroadcast;

    bool bWasEmpty;
    {
        ::osl::MutexGuard aGuard( m_aFeatureMutex );
        bWasEmpty = m_aFeaturesToInvalidate.empty();
        m_aFeaturesToInvalidate.push_back( aListener );
    }

    if ( bWasEmpty )
        m_aAsyncInvalidateAll.Call();
}

void OGenericUnoController::InvalidateFeature(sal_uInt16 _nId, const Reference< XStatusListener > & _xListener, bool _bForceBroadcast)
{
    ImplInvalidateFeature( _nId, _xListener, _bForceBroadcast );
}

void OGenericUnoController::InvalidateAll()
{
    ImplInvalidateFeature( ALL_FEATURES, NULL, true );
}

void OGenericUnoController::InvalidateAll_Impl()
{
    // invalidate all supported features

    for (   SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.begin();
            aIter != m_aSupportedFeatures.end();
            ++aIter
        )
        ImplBroadcastFeatureState( aIter->first, NULL, true );

    {
        ::osl::MutexGuard aGuard( m_aFeatureMutex);
        OSL_ENSURE(m_aFeaturesToInvalidate.size(), "OGenericUnoController::InvalidateAll_Impl: to be called from within InvalidateFeature_Impl only!");
        m_aFeaturesToInvalidate.pop_front();
        if(!m_aFeaturesToInvalidate.empty())
            m_aAsyncInvalidateAll.Call();
    }
}

Reference< XDispatch >  OGenericUnoController::queryDispatch(const URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags) throw( RuntimeException, std::exception )
{
    Reference< XDispatch > xReturn;

    OSL_PRECOND( !m_aSupportedFeatures.empty(), "OGenericUnoController::queryDispatch: shouldn't this be filled at construction time?" );
    if ( m_aSupportedFeatures.empty() )
        fillSupportedFeatures();

    // URL's we can handle ourself?
    if  (   aURL.Complete == ".uno:FormSlots/ConfirmDeletion"
        ||  (   ( m_aSupportedFeatures.find( aURL.Complete ) != m_aSupportedFeatures.end() )
            &&  !isUserDefinedFeature( aURL.Complete )
            )
        )
    {
        xReturn = this;
    }
    // no? -> ask the slave dispatcher
    else if ( m_xSlaveDispatcher.is() )
    {
        xReturn = m_xSlaveDispatcher->queryDispatch(aURL, aTargetFrameName, nSearchFlags);
    }

    // outta here
    return xReturn;
}

Sequence< Reference< XDispatch > > OGenericUnoController::queryDispatches(const Sequence< DispatchDescriptor >& aDescripts) throw( RuntimeException, std::exception )
{
    Sequence< Reference< XDispatch > > aReturn;
    sal_Int32 nLen = aDescripts.getLength();
    if ( nLen )
    {
        aReturn.realloc( nLen );
                Reference< XDispatch >* pReturn     = aReturn.getArray();
        const   Reference< XDispatch >* pReturnEnd  = aReturn.getArray() + nLen;
        const   DispatchDescriptor*     pDescripts  = aDescripts.getConstArray();

        for ( ; pReturn != pReturnEnd; ++ pReturn, ++pDescripts )
        {
            *pReturn = queryDispatch( pDescripts->FeatureURL, pDescripts->FrameName, pDescripts->SearchFlags );
        }
    }

    return aReturn;
}

Reference< XDispatchProvider >  OGenericUnoController::getSlaveDispatchProvider() throw( RuntimeException, std::exception )
{
    return m_xSlaveDispatcher;
}

void OGenericUnoController::setSlaveDispatchProvider(const Reference< XDispatchProvider > & _xNewProvider) throw( RuntimeException, std::exception )
{
    m_xSlaveDispatcher = _xNewProvider;
}

Reference< XDispatchProvider >  OGenericUnoController::getMasterDispatchProvider() throw( RuntimeException, std::exception )
{
    return m_xMasterDispatcher;
}

void OGenericUnoController::setMasterDispatchProvider(const Reference< XDispatchProvider > & _xNewProvider) throw( RuntimeException, std::exception )
{
    m_xMasterDispatcher = _xNewProvider;
}

void OGenericUnoController::dispatch(const URL& _aURL, const Sequence< PropertyValue >& aArgs) throw(RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    // The SolarMutex is not locked anymore when the framework calls into
    // here. So, lock it ourself. The real solution would be to lock it only in the places
    // where it's needed, but a) this might turn out difficult, since we then also need to care
    // for locking in the proper order (SolarMutex and m_aMutex), and b) this would be too many places
    // for the time frame of the fix.
    // #i52602#
    executeChecked(_aURL,aArgs);
}

void OGenericUnoController::addStatusListener(const Reference< XStatusListener > & aListener, const URL& _rURL) throw(RuntimeException, std::exception)
{
    // parse the URL now and here, this saves later parsing in each notification round
    URL aParsedURL( _rURL );
    if ( m_xUrlTransformer.is() )
        m_xUrlTransformer->parseStrict( aParsedURL );

    // remember the listener together with the URL
    m_arrStatusListener.insert( m_arrStatusListener.end(), DispatchTarget( aParsedURL, aListener ) );

    // initially broadcast the state
    ImplBroadcastFeatureState( aParsedURL.Complete, aListener, true );
        // force the new state to be broadcast to the new listener
}

void OGenericUnoController::removeStatusListener(const Reference< XStatusListener > & aListener, const URL& _rURL) throw(RuntimeException, std::exception)
{
    Dispatch::iterator iterSearch = m_arrStatusListener.begin();

    bool bRemoveForAll = _rURL.Complete.isEmpty();
    while ( iterSearch != m_arrStatusListener.end() )
    {
        DispatchTarget& rCurrent = *iterSearch;
        if  (   (rCurrent.xListener == aListener)
            &&  (   bRemoveForAll
                ||  (rCurrent.aURL.Complete.equals(_rURL.Complete))
                )
            )
        {
            m_arrStatusListener.erase( iterSearch++ );
            if (!bRemoveForAll)
                // remove the listener only for the given URL, so we can exit the loop after deletion
                break;
        }
        else
            ++iterSearch;
    }

    OSL_PRECOND( !m_aSupportedFeatures.empty(), "OGenericUnoController::removeStatusListener: shouldn't this be filled at construction time?" );
    if ( m_aSupportedFeatures.empty() )
        fillSupportedFeatures();

    SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.find(_rURL.Complete);
    if (aIter != m_aSupportedFeatures.end())
    {   // clear the cache for that feature
        StateCache::iterator aCachePos = m_aStateCache.find( aIter->second.nFeatureId );
        if ( aCachePos != m_aStateCache.end() )
            m_aStateCache.erase( aCachePos );
    }

    // now remove the listener from the deque
    ::osl::MutexGuard aGuard( m_aFeatureMutex );
    m_aFeaturesToInvalidate.erase(
        ::std::remove_if(   m_aFeaturesToInvalidate.begin(),
                            m_aFeaturesToInvalidate.end(),
                            ::std::bind2nd(FindFeatureListener(),aListener))
        ,m_aFeaturesToInvalidate.end());
}

void OGenericUnoController::releaseNumberForComponent()
{
    try
    {
        Reference< XUntitledNumbers > xUntitledProvider(getPrivateModel(), UNO_QUERY      );
        if ( xUntitledProvider.is() )
            xUntitledProvider->releaseNumberForComponent(static_cast<XWeak*>(this));
    }
    catch( const Exception& )
    {
        // NII
    }
}

void OGenericUnoController::disposing()
{
    {
        EventObject aDisposeEvent;
        aDisposeEvent.Source = static_cast<XWeak*>(this);
        Dispatch aStatusListener = m_arrStatusListener;
        Dispatch::iterator aEnd = aStatusListener.end();
        for (Dispatch::iterator aIter = aStatusListener.begin(); aIter != aEnd; ++aIter)
        {
            aIter->xListener->disposing(aDisposeEvent);
        }
        m_arrStatusListener.clear();
    }

    m_xDatabaseContext = NULL;
    {
        ::osl::MutexGuard aGuard( m_aFeatureMutex);
        m_aAsyncInvalidateAll.CancelCall();
        m_aFeaturesToInvalidate.clear();
    }

    releaseNumberForComponent();

    // check out from all the objects we are listening
    // the frame
    stopFrameListening( m_aCurrentFrame.getFrame() );
    m_aCurrentFrame.attachFrame( NULL );

    m_xMasterDispatcher = NULL;
    m_xSlaveDispatcher = NULL;
    m_xTitleHelper.clear();
    m_xUrlTransformer.clear();
    m_aInitParameters.clear();
}

void SAL_CALL OGenericUnoController::addEventListener( const Reference< XEventListener >& xListener ) throw (RuntimeException, std::exception)
{
    // disambiguate
    OGenericUnoController_Base::WeakComponentImplHelperBase::addEventListener( xListener );
}

void SAL_CALL OGenericUnoController::removeEventListener( const Reference< XEventListener >& xListener ) throw (RuntimeException, std::exception)
{
    // disambiguate
    OGenericUnoController_Base::WeakComponentImplHelperBase::removeEventListener( xListener );
}

void OGenericUnoController::frameAction(const FrameActionEvent& aEvent) throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( getMutex() );
    if ( aEvent.Frame == m_aCurrentFrame.getFrame() )
        m_aCurrentFrame.frameAction( aEvent.Action );
}

void OGenericUnoController::implDescribeSupportedFeature( const sal_Char* _pAsciiCommandURL,
        sal_uInt16 _nFeatureId, sal_Int16 _nCommandGroup )
{
#ifdef DBG_UTIL
    OSL_ENSURE( m_bDescribingSupportedFeatures, "OGenericUnoController::implDescribeSupportedFeature: bad timing for this call!" );
#endif
    OSL_PRECOND( _nFeatureId < FIRST_USER_DEFINED_FEATURE, "OGenericUnoController::implDescribeSupportedFeature: invalid feature id!" );

    ControllerFeature aFeature;
    aFeature.Command = OUString::createFromAscii( _pAsciiCommandURL );
    aFeature.nFeatureId = _nFeatureId;
    aFeature.GroupId = _nCommandGroup;

#if OSL_DEBUG_LEVEL > 0
    OSL_ENSURE( m_aSupportedFeatures.find( aFeature.Command ) == m_aSupportedFeatures.end(),
        "OGenericUnoController::implDescribeSupportedFeature: this feature is already there!" );
#endif
    m_aSupportedFeatures[ aFeature.Command ] = aFeature;
}

void OGenericUnoController::describeSupportedFeatures()
{
    // add all supported features
    implDescribeSupportedFeature( ".uno:Copy", ID_BROWSER_COPY, CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:Cut", ID_BROWSER_CUT, CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:Paste", ID_BROWSER_PASTE, CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:ClipboardFormatItems", ID_BROWSER_CLIPBOARD_FORMAT_ITEMS );
    implDescribeSupportedFeature( ".uno:DSBEditDoc", ID_BROWSER_EDITDOC, CommandGroup::DOCUMENT );
}

FeatureState OGenericUnoController::GetState( sal_uInt16 _nId ) const
{
    FeatureState aReturn;
        // (disabled automatically)

    switch ( _nId )
    {
        case ID_BROWSER_UNDO:
        case ID_BROWSER_SAVEDOC:
            aReturn.bEnabled = true;
            break;
        default:
            aReturn = UserDefinedFeatures::getState( getURLForId( _nId ) );
            break;
    }

    return aReturn;
}

void OGenericUnoController::Execute( sal_uInt16 _nId, const Sequence< PropertyValue>& _rArgs )
{
    OSL_ENSURE( isUserDefinedFeature( _nId ),
        "OGenericUnoController::Execute: responsible for user defined features only!" );

    // user defined features can be handled by dispatch interceptors resp. protocol handlers only.
    // So, we need to do a queryDispatch, and dispatch the URL
    m_pData->m_aUserDefinedFeatures.execute( getURLForId( _nId ), _rArgs );
}

URL OGenericUnoController::getURLForId(sal_Int32 _nId) const
{
    URL aReturn;
    if ( m_xUrlTransformer.is() )
    {
        SupportedFeatures::const_iterator aIter = ::std::find_if(
            m_aSupportedFeatures.begin(),
            m_aSupportedFeatures.end(),
            ::std::bind2nd( CompareFeatureById(), _nId )
        );

        if ( m_aSupportedFeatures.end() != aIter && !aIter->first.isEmpty() )
        {
            aReturn.Complete = aIter->first;
            m_xUrlTransformer->parseStrict( aReturn );
        }
    }
    return aReturn;
}

bool OGenericUnoController::isUserDefinedFeature( const sal_uInt16 _nFeatureId )
{
    return ( _nFeatureId >= FIRST_USER_DEFINED_FEATURE ) && ( _nFeatureId < LAST_USER_DEFINED_FEATURE );
}

bool OGenericUnoController::isUserDefinedFeature( const OUString& _rFeatureURL ) const
{
    SupportedFeatures::const_iterator pos = m_aSupportedFeatures.find( _rFeatureURL );
    OSL_PRECOND( pos != m_aSupportedFeatures.end(),
        "OGenericUnoController::isUserDefinedFeature: this is no supported feature at all!" );

    return ( pos != m_aSupportedFeatures.end() ) && isUserDefinedFeature( pos->second.nFeatureId );
}

sal_Bool SAL_CALL OGenericUnoController::supportsService(const OUString& ServiceName) throw(RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

void OGenericUnoController::startConnectionListening(const Reference< XConnection >& _rxConnection)
{
    // we have to remove ourself before dispoing the connection
    Reference< XComponent >  xComponent(_rxConnection, UNO_QUERY);
    if (xComponent.is())
        xComponent->addEventListener(static_cast<XFrameActionListener*>(this));
}

void OGenericUnoController::stopConnectionListening(const Reference< XConnection >& _rxConnection)
{
    // we have to remove ourself before dispoing the connection
    Reference< XComponent >  xComponent(_rxConnection, UNO_QUERY);
    if (xComponent.is())
        xComponent->removeEventListener(static_cast<XFrameActionListener*>(this));
}

Reference< XConnection > OGenericUnoController::connect( const Reference< XDataSource>& _xDataSource,
    ::dbtools::SQLExceptionInfo* _pErrorInfo )
{
    WaitObject aWaitCursor( getView() );

    ODatasourceConnector aConnector( getORB(), getView(), OUString() );
    Reference< XConnection > xConnection = aConnector.connect( _xDataSource, _pErrorInfo );
    startConnectionListening( xConnection );

    return xConnection;
}

Reference< XConnection > OGenericUnoController::connect( const OUString& _rDataSourceName,
    const OUString& _rContextInformation, ::dbtools::SQLExceptionInfo* _pErrorInfo )
{
    WaitObject aWaitCursor( getView() );

    ODatasourceConnector aConnector( getORB(), getView(), _rContextInformation );
    Reference<XConnection> xConnection = aConnector.connect( _rDataSourceName, _pErrorInfo );
    startConnectionListening( xConnection );

    return xConnection;
}

void OGenericUnoController::setView( const VclPtr<ODataView> &i_rView )
{
    m_pView = i_rView;
}

void OGenericUnoController::clearView()
{
    m_pView = NULL;
}

void OGenericUnoController::showError(const SQLExceptionInfo& _rInfo)
{
    ::dbaui::showError(_rInfo,getView(),getORB());
}

Reference< XLayoutManager > OGenericUnoController::getLayoutManager(const Reference< XFrame >& _xFrame)
{
    Reference< XPropertySet > xPropSet( _xFrame, UNO_QUERY );
    Reference< XLayoutManager > xLayoutManager;
    if ( xPropSet.is() )
    {
        try
        {
            xLayoutManager.set(xPropSet->getPropertyValue("LayoutManager"),UNO_QUERY);
        }
        catch ( Exception& )
        {
        }
    }
    return xLayoutManager;
}

void OGenericUnoController::loadMenu(const Reference< XFrame >& _xFrame)
{
    Reference< XLayoutManager > xLayoutManager = getLayoutManager(_xFrame);
    if ( xLayoutManager.is() )
    {
        xLayoutManager->lock();
        xLayoutManager->createElement( OUString( "private:resource/menubar/menubar" ));
        xLayoutManager->createElement( OUString( "private:resource/toolbar/toolbar" ));
        xLayoutManager->unlock();
        xLayoutManager->doLayout();
    }

    onLoadedMenu( xLayoutManager );
}

void OGenericUnoController::onLoadedMenu(const Reference< XLayoutManager >& /*_xLayoutManager*/)
{
    // not interested in
}

void OGenericUnoController::closeTask()
{
    m_aAsyncCloseTask.Call();
}

IMPL_LINK_NOARG_TYPED(OGenericUnoController, OnAsyncCloseTask, void*, void)
{
    if ( !OGenericUnoController_Base::rBHelper.bInDispose )
    {
        try
        {
            Reference< util::XCloseable > xCloseable( m_aCurrentFrame.getFrame(), UNO_QUERY_THROW );
            xCloseable->close( sal_False ); // false - holds the owner ship for this frame inside this object!
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}

Any SAL_CALL OGenericUnoController::getViewData() throw( RuntimeException, std::exception )
{
    return Any();
}

void SAL_CALL OGenericUnoController::restoreViewData(const Any& /*Data*/) throw( RuntimeException, std::exception )
{
}

Reference< XModel > SAL_CALL OGenericUnoController::getModel() throw( RuntimeException, std::exception )
{
    return Reference< XModel >();
}

Reference< XFrame > SAL_CALL OGenericUnoController::getFrame() throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( getMutex() );
    return m_aCurrentFrame.getFrame();
}

sal_Bool SAL_CALL OGenericUnoController::attachModel(const Reference< XModel > & /*xModel*/) throw( RuntimeException, std::exception )
{
    SAL_WARN("dbaccess.ui", "OGenericUnoController::attachModel: not supported!" );
    return sal_False;
}

void OGenericUnoController::executeUnChecked(sal_uInt16 _nCommandId, const Sequence< PropertyValue >& aArgs)
{
    Execute(_nCommandId, aArgs);
}

void OGenericUnoController::executeUnChecked(const util::URL& _rCommand, const Sequence< PropertyValue >& aArgs)
{
    OSL_PRECOND( !m_aSupportedFeatures.empty(), "OGenericUnoController::executeUnChecked: shouldn't this be filled at construction time?" );
    if ( m_aSupportedFeatures.empty() )
        fillSupportedFeatures();

    SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.find( _rCommand.Complete );
    if (aIter != m_aSupportedFeatures.end())
        Execute( aIter->second.nFeatureId, aArgs );
}

void OGenericUnoController::executeChecked(const util::URL& _rCommand, const Sequence< PropertyValue >& aArgs)
{
    OSL_PRECOND( !m_aSupportedFeatures.empty(), "OGenericUnoController::executeChecked: shouldn't this be filled at construction time?" );
    if ( m_aSupportedFeatures.empty() )
        fillSupportedFeatures();

    SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.find( _rCommand.Complete );
    if ( aIter != m_aSupportedFeatures.end() )
    {
        sal_uInt16 nFeatureId = aIter->second.nFeatureId;
        if ( GetState( nFeatureId ).bEnabled )
            Execute( nFeatureId, aArgs );
    }
}

namespace
{
    OUString lcl_getModuleHelpModuleName( const Reference< XFrame >& _rxFrame )
    {
        const sal_Char* pReturn = NULL;

        try
        {
            // get the model of the document in the given frame
            Reference< XController > xController;
            if ( _rxFrame.is() )
                xController = _rxFrame->getController();
            Reference< XModel > xModel;
            if ( xController.is() )
                xModel = xController->getModel();
            Reference< XServiceInfo > xSI( xModel, UNO_QUERY );

            if ( !xSI.is() )
            {   // try to go up the frame hierarchy

                Reference< XFrame > xParentFrame;
                if ( _rxFrame.is() )
                    xParentFrame.set(_rxFrame->getCreator(), css::uno::UNO_QUERY);
                // did we find a parent frame? Which is no top-level frame?
                if ( xParentFrame.is() && !_rxFrame->isTop() )
                    // TODO: to prevent framework assertions, re-insert this "isTop" once 98303 is fixed
                    return lcl_getModuleHelpModuleName( xParentFrame );
            }
            else
            {
#if OSL_DEBUG_LEVEL > 0
                Sequence< OUString > sServiceNames = xSI->getSupportedServiceNames();
                const OUString* pLoop = sServiceNames.getConstArray();
                for ( sal_Int32 i=0; i<sServiceNames.getLength(); ++i, ++pLoop )
                {
                    sal_Int32 nDummy = 0;
                    (void)nDummy;
                }
#endif

                // check which service we know ....
                static const sal_Char* pTransTable[] = {
                    "com.sun.star.sdb.OfficeDatabaseDocument","sdatabase",
                    "com.sun.star.report.ReportDefinition","sdatabase",
                    "com.sun.star.text.TextDocument",   "swriter",
                    "com.sun.star.sheet.SpreadsheetDocument", "scalc",
                    "com.sun.star.presentation.PresentationDocument", "simpress",
                    "com.sun.star.drawing.DrawingDocument", "sdraw",
                    "com.sun.star.formula.FormularProperties", "smath",
                    "com.sun.star.chart.ChartDocument", "schart"
                };
                OSL_ENSURE( ( sizeof( pTransTable ) / sizeof( pTransTable[0] ) ) % 2 == 0,
                    "lcl_getModuleHelpModuleName: odd size of translation table!" );

                // loop through the table
                sal_Int32 nTableEntries = ( sizeof( pTransTable ) / sizeof( pTransTable[0] ) ) / 2;
                const sal_Char** pDocumentService = pTransTable;
                const sal_Char** pHelpModuleName = pTransTable + 1;
                for ( sal_Int32 j=0; j<nTableEntries; ++j )
                {
                    if ( xSI->supportsService( OUString::createFromAscii( *pDocumentService ) ) )
                    {   // found a table entry which matches the model's services
                        pReturn = *pHelpModuleName;
                        break;
                    }

                    ++pDocumentService; ++pDocumentService;
                    ++pHelpModuleName; ++pHelpModuleName;
                }
            }

            if ( !pReturn )
            {
                // could not determine the document type we're living in
                // ->fallback
                SvtModuleOptions aModOpt;
                if ( aModOpt.IsModuleInstalled( SvtModuleOptions::EModule::WRITER ) )
                    pReturn = "swriter";
                else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::EModule::DATABASE ) )
                    pReturn = "sdatabase";
                else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::EModule::CALC ) )
                    pReturn = "scalc";
                else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::EModule::IMPRESS ) )
                    pReturn = "simpress";
                else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::EModule::DRAW ) )
                    pReturn = "sdraw";
                else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::EModule::MATH ) )
                    pReturn = "smath";
                else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::EModule::CHART ) )
                    pReturn = "schart";
                else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::EModule::BASIC ) )
                    pReturn = "sbasic";
                else
                {
                    SAL_WARN("dbaccess.ui", "lcl_getModuleHelpModuleName: no installed module found" );
                }
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        if ( !pReturn )
            pReturn = "swriter";

        return OUString::createFromAscii( pReturn );
    }
}

void OGenericUnoController::openHelpAgent(OUString const& _suHelpStringURL )
{
    OUString suURL(_suHelpStringURL);
    OUString sLanguage( "Language=" );
    if (suURL.indexOf(sLanguage) == -1)
    {
        AppendConfigToken(suURL, false /* sal_False := add '&' */ );
    }
    URL aURL;
    aURL.Complete = suURL;

    openHelpAgent( aURL );
}

void OGenericUnoController::openHelpAgent(const OString& _sHelpId)
{
    openHelpAgent( createHelpAgentURL( lcl_getModuleHelpModuleName( getFrame() ), _sHelpId ) );
}

void OGenericUnoController::openHelpAgent( const URL& _rURL )
{
    try
    {
        URL aURL( _rURL );

        if ( m_xUrlTransformer.is() )
            m_xUrlTransformer->parseStrict(aURL);

        Reference< XDispatchProvider > xDispProv( m_aCurrentFrame.getFrame(), UNO_QUERY );
        Reference< XDispatch > xHelpDispatch;
        if ( xDispProv.is() )
            xHelpDispatch = xDispProv->queryDispatch(aURL, OUString( "_helpagent" ), FrameSearchFlag::PARENT | FrameSearchFlag::SELF);
        OSL_ENSURE(xHelpDispatch.is(), "SbaTableQueryBrowser::openHelpAgent: could not get a dispatcher!");
        if (xHelpDispatch.is())
        {
            xHelpDispatch->dispatch(aURL, Sequence< PropertyValue >());
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

Reference< awt::XWindow> OGenericUnoController::getTopMostContainerWindow() const
{
    Reference< css::awt::XWindow> xWindow;

    // get the top most window
    Reference< XFrame > xFrame( m_aCurrentFrame.getFrame() );
    if ( xFrame.is() )
    {
        xWindow = xFrame->getContainerWindow();

        while ( xFrame.is() && !xFrame->isTop() )
        {
            xFrame.set( xFrame->getCreator(), UNO_QUERY );
        }
        if ( xFrame.is() )
            xWindow = xFrame->getContainerWindow();
    }
    return xWindow;
}

Reference< XTitle > OGenericUnoController::impl_getTitleHelper_throw()
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getMutex() );

    if ( ! m_xTitleHelper.is ())
    {
        Reference< XUntitledNumbers > xUntitledProvider(getPrivateModel(), UNO_QUERY      );
        Reference< XController >      xThis(static_cast< XController* >(this), UNO_QUERY_THROW);

        ::framework::TitleHelper* pHelper = new ::framework::TitleHelper( m_xContext );
        m_xTitleHelper.set( static_cast< ::cppu::OWeakObject* >(pHelper), UNO_QUERY_THROW);

        pHelper->setOwner                   (xThis            );
        pHelper->connectWithUntitledNumbers (xUntitledProvider);
    }

    return m_xTitleHelper;
}

// XTitle
OUString SAL_CALL OGenericUnoController::getTitle()
    throw (RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( getMutex() );
    if ( m_bExternalTitle )
        return impl_getTitleHelper_throw()->getTitle ();
    return getPrivateTitle() + impl_getTitleHelper_throw()->getTitle ();
}

// XTitle
void SAL_CALL OGenericUnoController::setTitle(const OUString& sTitle)
    throw (RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getMutex() );
    m_bExternalTitle = true;
    impl_getTitleHelper_throw()->setTitle (sTitle);
}

// XTitleChangeBroadcaster
void SAL_CALL OGenericUnoController::addTitleChangeListener(const Reference< XTitleChangeListener >& xListener)
    throw (RuntimeException, std::exception)
{
    Reference< XTitleChangeBroadcaster > xBroadcaster(impl_getTitleHelper_throw(), UNO_QUERY);
    if (xBroadcaster.is ())
        xBroadcaster->addTitleChangeListener (xListener);
}

void SAL_CALL OGenericUnoController::removeTitleChangeListener(const Reference< XTitleChangeListener >& xListener)
    throw (RuntimeException, std::exception)
{
    Reference< XTitleChangeBroadcaster > xBroadcaster(impl_getTitleHelper_throw(), UNO_QUERY);
    if (xBroadcaster.is ())
        xBroadcaster->removeTitleChangeListener (xListener);
}

// XUserInputInterception
void SAL_CALL OGenericUnoController::addKeyHandler( const Reference< XKeyHandler >& _rxHandler ) throw (RuntimeException, std::exception)
{
    if ( _rxHandler.is() )
        m_pData->m_aUserInputInterception.addKeyHandler( _rxHandler );
}

void SAL_CALL OGenericUnoController::removeKeyHandler( const Reference< XKeyHandler >& _rxHandler ) throw (RuntimeException, std::exception)
{
    m_pData->m_aUserInputInterception.removeKeyHandler( _rxHandler );
}

void SAL_CALL OGenericUnoController::addMouseClickHandler( const Reference< XMouseClickHandler >& _rxHandler ) throw (RuntimeException, std::exception)
{
    if ( _rxHandler.is() )
        m_pData->m_aUserInputInterception.addMouseClickHandler( _rxHandler );
}

void SAL_CALL OGenericUnoController::removeMouseClickHandler( const Reference< XMouseClickHandler >& _rxHandler ) throw (RuntimeException, std::exception)
{
    m_pData->m_aUserInputInterception.removeMouseClickHandler( _rxHandler );
}

void OGenericUnoController::executeChecked(sal_uInt16 _nCommandId, const Sequence< PropertyValue >& aArgs)
{
    if ( isCommandEnabled(_nCommandId) )
        Execute(_nCommandId, aArgs);
}

bool OGenericUnoController::isCommandEnabled(sal_uInt16 _nCommandId) const
{
    return GetState( _nCommandId ).bEnabled;
}

sal_uInt16 OGenericUnoController::registerCommandURL( const OUString& _rCompleteCommandURL )
{
    if ( _rCompleteCommandURL.isEmpty() )
        return 0;

    SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.find( _rCompleteCommandURL );
    if ( aIter != m_aSupportedFeatures.end() )
        return aIter->second.nFeatureId;

    // this is a previously unkwnon command
    sal_uInt16 nFeatureId = FIRST_USER_DEFINED_FEATURE;
    while ( isFeatureSupported( nFeatureId ) && ( nFeatureId < LAST_USER_DEFINED_FEATURE ) )
        ++nFeatureId;
    if ( nFeatureId == LAST_USER_DEFINED_FEATURE )
    {
        SAL_WARN("dbaccess.ui", "OGenericUnoController::registerCommandURL: no more space for user defined features!" );
        return 0L;
    }

    ControllerFeature aFeature;
    aFeature.Command = _rCompleteCommandURL;
    aFeature.nFeatureId = nFeatureId;
    aFeature.GroupId = CommandGroup::INTERNAL;
    m_aSupportedFeatures[ aFeature.Command ] = aFeature;

    return nFeatureId;
}

void OGenericUnoController::notifyHiContrastChanged()
{
}

bool OGenericUnoController::isDataSourceReadOnly() const
{
    return false;
}

Reference< XController > OGenericUnoController::getXController() throw( RuntimeException )
{
    return this;
}

bool OGenericUnoController::interceptUserInput( const NotifyEvent& _rEvent )
{
    return m_pData->m_aUserInputInterception.handleNotifyEvent( _rEvent );
}

bool OGenericUnoController::isCommandChecked(sal_uInt16 _nCommandId) const
{
    FeatureState aState = GetState( _nCommandId );

    return aState.bChecked && *aState.bChecked;
}

bool OGenericUnoController::isCommandEnabled( const OUString& _rCompleteCommandURL ) const
{
    OSL_ENSURE( !_rCompleteCommandURL.isEmpty(), "OGenericUnoController::isCommandEnabled: Empty command url!" );

    bool bIsEnabled = false;
    SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.find( _rCompleteCommandURL );
    if ( aIter != m_aSupportedFeatures.end() )
        bIsEnabled = isCommandEnabled( aIter->second.nFeatureId );

    return bIsEnabled;
}

Sequence< ::sal_Int16 > SAL_CALL OGenericUnoController::getSupportedCommandGroups() throw (RuntimeException, std::exception)
{
    CommandHashMap aCmdHashMap;
    for (   SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.begin();
            aIter != m_aSupportedFeatures.end();
            ++aIter
        )
        if ( aIter->second.GroupId != CommandGroup::INTERNAL )
            aCmdHashMap.insert( CommandHashMap::value_type( aIter->second.GroupId, 0 ));

    Sequence< sal_Int16 > aCommandGroups( aCmdHashMap.size() );
    ::std::transform( aCmdHashMap.begin(),
        aCmdHashMap.end(),
        aCommandGroups.getArray(),
        ::o3tl::select1st< CommandHashMap::value_type >()
    );

    return aCommandGroups;
}

namespace
{
    //Current c++0x draft (apparently) has std::identity, but not operator()
    template<typename T> struct SGI_identity : public std::unary_function<T,T>
    {
        T& operator()(T& x) const { return x; }
        const T& operator()(const T& x) const { return x; }
    };
}

Sequence< DispatchInformation > SAL_CALL OGenericUnoController::getConfigurableDispatchInformation( ::sal_Int16 CommandGroup ) throw (RuntimeException, std::exception)
{
    ::std::list< DispatchInformation > aInformationList;
    DispatchInformation aDispatchInfo;
    for (   SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.begin();
            aIter != m_aSupportedFeatures.end();
            ++aIter
        )
    {
        if ( sal_Int16( aIter->second.GroupId ) == CommandGroup )
        {
            aDispatchInfo = aIter->second;
            aInformationList.push_back( aDispatchInfo );
        }
    }

    Sequence< DispatchInformation > aInformation( aInformationList.size() );
    ::std::transform( aInformationList.begin(),
        aInformationList.end(),
        aInformation.getArray(),
        SGI_identity< DispatchInformation >()
    );

    return aInformation;
}

void OGenericUnoController::fillSupportedFeatures()
{
#ifdef DBG_UTIL
    m_bDescribingSupportedFeatures = true;
#endif
    describeSupportedFeatures();
#ifdef DBG_UTIL
    m_bDescribingSupportedFeatures = false;
#endif
}

void SAL_CALL OGenericUnoController::dispose() throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    OGenericUnoController_Base::dispose();
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
