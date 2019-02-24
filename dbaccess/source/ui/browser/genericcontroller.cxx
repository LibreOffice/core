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
#include <toolkit/awt/vclxwindow.hxx>
#include <browserids.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <dbaccess/dataview.hxx>
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>
#include <stringconstants.hxx>
#include <vcl/stdtext.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <framework/titlehelper.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <UITools.hxx>
#include <commontypes.hxx>

#include <com/sun/star/ui/XSidebarProvider.hpp>
#include <sfx2/sidebar/UnoSidebar.hxx>
#include <sfx2/userinputinterception.hxx>

#include <vcl/waitobj.hxx>
#include <svl/urihelper.hxx>
#include <datasourceconnector.hxx>
#include <unotools/moduleoptions.hxx>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/status/Visibility.hpp>
#include <com/sun/star/frame/XUntitledNumbers.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <o3tl/functional.hxx>
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
#define FIRST_USER_DEFINED_FEATURE  ( std::numeric_limits< sal_uInt16 >::max() - 1000 )
#define LAST_USER_DEFINED_FEATURE   ( std::numeric_limits< sal_uInt16 >::max()        )

typedef std::unordered_map< sal_Int16, sal_Int16 > CommandHashMap;

namespace dbaui
{

// UserDefinedFeatures
class UserDefinedFeatures
{
public:
    explicit UserDefinedFeatures( const Reference< XController >& _rxController );

    void            execute( const URL& _rFeatureURL, const Sequence< PropertyValue>& _rArgs );

private:
    css::uno::WeakReference< XController > m_aController;
};

UserDefinedFeatures::UserDefinedFeatures( const Reference< XController >& _rxController )
    :m_aController( _rxController )
{
}

void UserDefinedFeatures::execute( const URL& _rFeatureURL, const Sequence< PropertyValue>& _rArgs )
{
    try
    {
        Reference< XController > xController( Reference< XController >(m_aController), UNO_SET_THROW );
        Reference< XDispatchProvider > xDispatchProvider( xController->getFrame(), UNO_QUERY_THROW );
        Reference< XDispatch > xDispatch( xDispatchProvider->queryDispatch(
            _rFeatureURL,
            "_self",
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
        DBG_UNHANDLED_EXCEPTION("dbaccess");
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
    ,m_pView(nullptr)
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
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
}

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
        ShowServiceNotAvailableError(getFrameWeld(), "com.sun.star.sdb.DatabaseContext", true);
    }

    return true;
}

IMPL_LINK_NOARG(OGenericUnoController, OnAsyncInvalidateAll, void*, void)
{
    if ( !OGenericUnoController_Base::rBHelper.bInDispose && !OGenericUnoController_Base::rBHelper.bDisposed )
        InvalidateFeature_Impl();
}

void OGenericUnoController::impl_initialize()
{
}

void SAL_CALL OGenericUnoController::initialize( const Sequence< Any >& aArguments )
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
        m_pView = nullptr;
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

void OGenericUnoController::disposing(const EventObject& Source)
{
    // our frame ?
    if ( Source.Source == getFrame() )
        stopFrameListening( getFrame() );
}

void OGenericUnoController::modified(const EventObject& aEvent)
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

Reference< XWindow > SAL_CALL OGenericUnoController::getComponentWindow()
{
    SolarMutexGuard g;
    return VCLUnoHelper::GetInterface( getView() );
}

Reference<XSidebarProvider> SAL_CALL OGenericUnoController::getSidebar()
{
        return nullptr;
}

OUString SAL_CALL OGenericUnoController::getViewControllerName()
{
    return OUString( "Default" );
}

Sequence< PropertyValue > SAL_CALL OGenericUnoController::getCreationArguments()
{
    // currently we do not support any creation args, so anything passed to XModel2::createViewController would be
    // lost, so we can equally return an empty sequence here
    return Sequence< PropertyValue >();
}

void OGenericUnoController::attachFrame( const Reference< XFrame >& _rxFrame )
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
    typedef std::vector< Any >    States;

    void    lcl_notifyMultipleStates( XStatusListener& _rListener, FeatureStateEvent& _rEvent, const States& _rStates )
    {
        for (auto const& elem : _rStates)
        {
            _rEvent.State = elem;
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
            _out_rStates.emplace_back( );
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
        lcl_notifyMultipleStates( *xListener, aEvent, aStates );
    else
    {   // no -> iterate through all listeners responsible for the URL
        std::set<OUString> aFeatureCommands;
        for( const auto& rFeature : m_aSupportedFeatures )
        {
            if( rFeature.second.nFeatureId == nFeat )
                aFeatureCommands.insert( rFeature.first );
        }

        // it is possible that listeners are registered or revoked while
        // we are notifying them, so we must use a copy of m_arrStatusListener, not
        // m_arrStatusListener itself
        Dispatch aNotifyLoop( m_arrStatusListener );

        for (auto const& elem : aNotifyLoop)
        {
            if ( aFeatureCommands.find( elem.aURL.Complete ) != aFeatureCommands.end() )
            {
                aEvent.FeatureURL = elem.aURL;
                lcl_notifyMultipleStates( *elem.xListener.get(), aEvent, aStates );
            }
        }
    }

}

bool OGenericUnoController::isFeatureSupported( sal_Int32 _nId )
{
    SupportedFeatures::const_iterator aFeaturePos = std::find_if(
        m_aSupportedFeatures.begin(),
        m_aSupportedFeatures.end(),
        CompareFeatureById(_nId)
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
            SupportedFeatures::const_iterator aFeaturePos = std::find_if(
                m_aSupportedFeatures.begin(),
                m_aSupportedFeatures.end(),
                CompareFeatureById( aNextFeature.nId )
            );

#if OSL_DEBUG_LEVEL > 0
            if ( m_aSupportedFeatures.end() == aFeaturePos )
            {
                SAL_WARN( "dbaccess.ui", "OGenericUnoController::InvalidateFeature_Impl: feature id "
                            << aNextFeature.nId
                            << " has been invalidated, but is not supported!" );
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
        auto isSupportedFeature = std::any_of(
            m_aSupportedFeatures.begin(),
            m_aSupportedFeatures.end(),
            CompareFeatureById( _nId )
        );
        OSL_ENSURE( isSupportedFeature, "OGenericUnoController::ImplInvalidateFeature: invalidating an unsupported feature is suspicious, at least!" );
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
    ImplInvalidateFeature( ALL_FEATURES, nullptr, true );
}

void OGenericUnoController::InvalidateAll_Impl()
{
    // invalidate all supported features
    for (auto const& supportedFeature : m_aSupportedFeatures)
        ImplBroadcastFeatureState( supportedFeature.first, nullptr, true );

    {
        ::osl::MutexGuard aGuard( m_aFeatureMutex);
        OSL_ENSURE(m_aFeaturesToInvalidate.size(), "OGenericUnoController::InvalidateAll_Impl: to be called from within InvalidateFeature_Impl only!");
        m_aFeaturesToInvalidate.pop_front();
        if(!m_aFeaturesToInvalidate.empty())
            m_aAsyncInvalidateAll.Call();
    }
}

Reference< XDispatch >  OGenericUnoController::queryDispatch(const URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags)
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

Sequence< Reference< XDispatch > > OGenericUnoController::queryDispatches(const Sequence< DispatchDescriptor >& aDescripts)
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

Reference< XDispatchProvider >  OGenericUnoController::getSlaveDispatchProvider()
{
    return m_xSlaveDispatcher;
}

void OGenericUnoController::setSlaveDispatchProvider(const Reference< XDispatchProvider > & _xNewProvider)
{
    m_xSlaveDispatcher = _xNewProvider;
}

Reference< XDispatchProvider >  OGenericUnoController::getMasterDispatchProvider()
{
    return m_xMasterDispatcher;
}

void OGenericUnoController::setMasterDispatchProvider(const Reference< XDispatchProvider > & _xNewProvider)
{
    m_xMasterDispatcher = _xNewProvider;
}

void OGenericUnoController::dispatch(const URL& _aURL, const Sequence< PropertyValue >& aArgs)
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

void OGenericUnoController::addStatusListener(const Reference< XStatusListener > & aListener, const URL& _rURL)
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

void OGenericUnoController::removeStatusListener(const Reference< XStatusListener > & aListener, const URL& _rURL)
{
    if (_rURL.Complete.isEmpty())
    {
        m_arrStatusListener.erase(std::remove_if(m_arrStatusListener.begin(), m_arrStatusListener.end(),
            [&aListener](const DispatchTarget& rCurrent) { return rCurrent.xListener == aListener; }),
            m_arrStatusListener.end());
    }
    else
    {
        // remove the listener only for the given URL
        Dispatch::iterator iterSearch = std::find_if(m_arrStatusListener.begin(), m_arrStatusListener.end(),
            [&aListener, &_rURL](const DispatchTarget& rCurrent) {
                return (rCurrent.xListener == aListener) && (rCurrent.aURL.Complete == _rURL.Complete); });
        if (iterSearch != m_arrStatusListener.end())
            m_arrStatusListener.erase(iterSearch);
    }

    OSL_PRECOND( !m_aSupportedFeatures.empty(), "OGenericUnoController::removeStatusListener: shouldn't this be filled at construction time?" );
    if ( m_aSupportedFeatures.empty() )
        fillSupportedFeatures();

    SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.find(_rURL.Complete);
    if (aIter != m_aSupportedFeatures.end())
    {   // clear the cache for that feature
        StateCache::const_iterator aCachePos = m_aStateCache.find( aIter->second.nFeatureId );
        if ( aCachePos != m_aStateCache.end() )
            m_aStateCache.erase( aCachePos );
    }

    // now remove the listener from the deque
    ::osl::MutexGuard aGuard( m_aFeatureMutex );
    m_aFeaturesToInvalidate.erase(
        std::remove_if(   m_aFeaturesToInvalidate.begin(),
                            m_aFeaturesToInvalidate.end(),
                            FindFeatureListener(aListener))
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
        for (auto const& statusListener : aStatusListener)
        {
            statusListener.xListener->disposing(aDisposeEvent);
        }
        m_arrStatusListener.clear();
    }

    m_xDatabaseContext = nullptr;
    {
        ::osl::MutexGuard aGuard( m_aFeatureMutex);
        m_aAsyncInvalidateAll.CancelCall();
        m_aFeaturesToInvalidate.clear();
    }

    releaseNumberForComponent();

    // check out from all the objects we are listening
    // the frame
    stopFrameListening( m_aCurrentFrame.getFrame() );
    m_aCurrentFrame.attachFrame( nullptr );

    m_xMasterDispatcher = nullptr;
    m_xSlaveDispatcher = nullptr;
    m_xTitleHelper.clear();
    m_xUrlTransformer.clear();
    m_aInitParameters.clear();
}

void SAL_CALL OGenericUnoController::addEventListener( const Reference< XEventListener >& xListener )
{
    // disambiguate
    OGenericUnoController_Base::WeakComponentImplHelperBase::addEventListener( xListener );
}

void SAL_CALL OGenericUnoController::removeEventListener( const Reference< XEventListener >& xListener )
{
    // disambiguate
    OGenericUnoController_Base::WeakComponentImplHelperBase::removeEventListener( xListener );
}

void OGenericUnoController::frameAction(const FrameActionEvent& aEvent)
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
            // for now, enable all the time
            // TODO: we should ask the dispatcher. However, this is laborious, since you cannot ask a dispatcher
            // directly, but need to add a status listener.
            aReturn.bEnabled = true;
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
        SupportedFeatures::const_iterator aIter = std::find_if(
            m_aSupportedFeatures.begin(),
            m_aSupportedFeatures.end(),
            CompareFeatureById( _nId )
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

sal_Bool SAL_CALL OGenericUnoController::supportsService(const OUString& ServiceName)
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

Reference< XConnection > OGenericUnoController::connect( const Reference< XDataSource>& _xDataSource )
{
    WaitObject aWaitCursor( getView() );

    ODatasourceConnector aConnector( getORB(), getView(), OUString() );
    Reference< XConnection > xConnection = aConnector.connect( _xDataSource, nullptr );
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
    m_pView = nullptr;
}

void OGenericUnoController::showError(const SQLExceptionInfo& _rInfo)
{
    ::dbtools::showError(_rInfo,VCLUnoHelper::GetInterface(getView()),getORB());
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
        xLayoutManager->createElement( "private:resource/menubar/menubar" );
        xLayoutManager->createElement( "private:resource/toolbar/toolbar" );
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

IMPL_LINK_NOARG(OGenericUnoController, OnAsyncCloseTask, void*, void)
{
    if ( !OGenericUnoController_Base::rBHelper.bInDispose )
    {
        try
        {
            Reference< util::XCloseable > xCloseable( m_aCurrentFrame.getFrame(), UNO_QUERY_THROW );
            xCloseable->close( false ); // false - holds the owner ship for this frame inside this object!
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }
    }
}

Any SAL_CALL OGenericUnoController::getViewData()
{
    return Any();
}

void SAL_CALL OGenericUnoController::restoreViewData(const Any& /*Data*/)
{
}

Reference< XModel > SAL_CALL OGenericUnoController::getModel()
{
    return Reference< XModel >();
}

Reference< XFrame > SAL_CALL OGenericUnoController::getFrame()
{
    ::osl::MutexGuard aGuard( getMutex() );
    return m_aCurrentFrame.getFrame();
}

sal_Bool SAL_CALL OGenericUnoController::attachModel(const Reference< XModel > & /*xModel*/)
{
    SAL_WARN("dbaccess.ui", "OGenericUnoController::attachModel: not supported!" );
    return false;
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
{
    ::osl::MutexGuard aGuard( getMutex() );
    if ( m_bExternalTitle )
        return impl_getTitleHelper_throw()->getTitle ();
    return getPrivateTitle() + impl_getTitleHelper_throw()->getTitle ();
}

// XTitle
void SAL_CALL OGenericUnoController::setTitle(const OUString& sTitle)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getMutex() );
    m_bExternalTitle = true;
    impl_getTitleHelper_throw()->setTitle (sTitle);
}

// XTitleChangeBroadcaster
void SAL_CALL OGenericUnoController::addTitleChangeListener(const Reference< XTitleChangeListener >& xListener)
{
    Reference< XTitleChangeBroadcaster > xBroadcaster(impl_getTitleHelper_throw(), UNO_QUERY);
    if (xBroadcaster.is ())
        xBroadcaster->addTitleChangeListener (xListener);
}

void SAL_CALL OGenericUnoController::removeTitleChangeListener(const Reference< XTitleChangeListener >& xListener)
{
    Reference< XTitleChangeBroadcaster > xBroadcaster(impl_getTitleHelper_throw(), UNO_QUERY);
    if (xBroadcaster.is ())
        xBroadcaster->removeTitleChangeListener (xListener);
}

// XUserInputInterception
void SAL_CALL OGenericUnoController::addKeyHandler( const Reference< XKeyHandler >& _rxHandler )
{
    if ( _rxHandler.is() )
        m_pData->m_aUserInputInterception.addKeyHandler( _rxHandler );
}

void SAL_CALL OGenericUnoController::removeKeyHandler( const Reference< XKeyHandler >& _rxHandler )
{
    m_pData->m_aUserInputInterception.removeKeyHandler( _rxHandler );
}

void SAL_CALL OGenericUnoController::addMouseClickHandler( const Reference< XMouseClickHandler >& _rxHandler )
{
    if ( _rxHandler.is() )
        m_pData->m_aUserInputInterception.addMouseClickHandler( _rxHandler );
}

void SAL_CALL OGenericUnoController::removeMouseClickHandler( const Reference< XMouseClickHandler >& _rxHandler )
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

void OGenericUnoController::notifyHiContrastChanged()
{
}

bool OGenericUnoController::isDataSourceReadOnly() const
{
    return false;
}

Reference< XController > OGenericUnoController::getXController()
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

Sequence< ::sal_Int16 > SAL_CALL OGenericUnoController::getSupportedCommandGroups()
{
    CommandHashMap aCmdHashMap;
    for (auto const& supportedFeature : m_aSupportedFeatures)
        if ( supportedFeature.second.GroupId != CommandGroup::INTERNAL )
            aCmdHashMap.emplace( supportedFeature.second.GroupId, 0 );

    return comphelper::mapKeysToSequence( aCmdHashMap );
}

Sequence< DispatchInformation > SAL_CALL OGenericUnoController::getConfigurableDispatchInformation( ::sal_Int16 CommandGroup )
{
    std::vector< DispatchInformation > aInformationVector;
    for (auto const& supportedFeature : m_aSupportedFeatures)
    {
        if ( sal_Int16( supportedFeature.second.GroupId ) == CommandGroup )
        {
            aInformationVector.push_back( supportedFeature.second );
        }
    }

    return comphelper::containerToSequence( aInformationVector );
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

void SAL_CALL OGenericUnoController::dispose()
{
    SolarMutexGuard aSolarGuard;
    OGenericUnoController_Base::dispose();
}

weld::Window* OGenericUnoController::getFrameWeld() const
{
    return m_pView ? m_pView->GetFrameWeld() : nullptr;
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
