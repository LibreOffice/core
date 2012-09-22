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

#include "genericcontroller.hxx"
#include <comphelper/uno3.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include "browserids.hxx"
#include <vcl/svapp.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include "dataview.hxx"
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>
#include "dbustrings.hrc"
#include <vcl/stdtext.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <framework/titlehelper.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/extract.hxx>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include "UITools.hxx"
#include "commontypes.hxx"

#include <vcl/waitobj.hxx>
#include <svl/urihelper.hxx>
#include "datasourceconnector.hxx"
#include <unotools/moduleoptions.hxx>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/status/Visibility.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <rtl/ustring.hxx>
#include <rtl/logfile.hxx>
#include <algorithm>
#include <o3tl/compat_functional.hxx>
#include <boost/unordered_map.hpp>
#include <cppuhelper/implbase1.hxx>
#include <limits>

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
using namespace ::com::sun::star;
using namespace ::dbtools;
using namespace ::comphelper;

// -------------------------------------------------------------------------
#define ALL_FEATURES                -1
#define FIRST_USER_DEFINED_FEATURE  ( ::std::numeric_limits< sal_uInt16 >::max() - 1000 )
#define LAST_USER_DEFINED_FEATURE   ( ::std::numeric_limits< sal_uInt16 >::max()        )

// -------------------------------------------------------------------------
typedef ::boost::unordered_map< sal_Int16, sal_Int16 > CommandHashMap;
typedef ::std::list< DispatchInformation > DispatchInfoList;


// -------------------------------------------------------------------------
const ::rtl::OUString& getConfirmDeletionURL()
{
    static const ::rtl::OUString sConfirmDeletionURL( RTL_CONSTASCII_USTRINGPARAM( ".uno:FormSlots/ConfirmDeletion" ) );
    return sConfirmDeletionURL;
}

namespace dbaui
{

//==========================================================================
//= UserDefinedFeatures
//==========================================================================
class UserDefinedFeatures
{
public:
    UserDefinedFeatures( const Reference< XController >& _rxController );

    FeatureState    getState( const URL& _rFeatureURL );
    void            execute( const URL& _rFeatureURL, const Sequence< PropertyValue>& _rArgs );

private:
    ::com::sun::star::uno::WeakReference< XController > m_aController;
};

//--------------------------------------------------------------------------
UserDefinedFeatures::UserDefinedFeatures( const Reference< XController >& _rxController )
    :m_aController( _rxController )
{
}

//--------------------------------------------------------------------------
FeatureState UserDefinedFeatures::getState( const URL& /*_rFeatureURL*/ )
{
    // for now, enable all the time
    // TODO: we should ask the dispatcher. However, this is laborious, since you cannot ask a dispatcher
    // directly, but need to add a status listener.
    FeatureState aState;
    aState.bEnabled = sal_True;
    return aState;
}

//--------------------------------------------------------------------------
void UserDefinedFeatures::execute( const URL& _rFeatureURL, const Sequence< PropertyValue>& _rArgs )
{
    try
    {
        Reference< XController > xController( (Reference< XController >)m_aController, UNO_SET_THROW );
        Reference< XDispatchProvider > xDispatchProvider( xController->getFrame(), UNO_QUERY_THROW );
        Reference< XDispatch > xDispatch( xDispatchProvider->queryDispatch(
            _rFeatureURL,
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "_self" ) ),
            FrameSearchFlag::AUTO
        ) );

        if ( xDispatch == xController )
        {
            OSL_FAIL( "UserDefinedFeatures::execute: the controller shouldn't be the dispatcher here!" );
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

//==========================================================================
//= OGenericUnoController_Data
//==========================================================================
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

//==========================================================================
//= OGenericUnoController
//==========================================================================
DBG_NAME(OGenericUnoController)
// -------------------------------------------------------------------------
OGenericUnoController::OGenericUnoController(const Reference< XMultiServiceFactory >& _rM)
    :OGenericUnoController_Base( getMutex() )
    ,m_pView(NULL)
#ifdef DBG_UTIL
    ,m_bDescribingSupportedFeatures( false )
#endif
    ,m_aAsyncInvalidateAll(LINK(this, OGenericUnoController, OnAsyncInvalidateAll))
    ,m_aAsyncCloseTask(LINK(this, OGenericUnoController, OnAsyncCloseTask))
    ,m_xServiceFactory(_rM)
    ,m_aCurrentFrame( *this )
    ,m_bPreview(sal_False)
    ,m_bReadOnly(sal_False)
    ,m_bCurrentlyModified(sal_False)
    ,m_bExternalTitle(sal_False)
{
    osl_atomic_increment( &m_refCount );
    {
        m_pData.reset( new OGenericUnoController_Data( *this, getMutex() ) );
    }
    osl_atomic_decrement( &m_refCount );

    DBG_CTOR(OGenericUnoController,NULL);

    try
    {
        m_xUrlTransformer = URLTransformer::create(comphelper::getComponentContext(_rM));
    }
    catch(Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

#ifdef WNT
// -----------------------------------------------------------------------------
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
    OSL_FAIL( "OGenericUnoController::OGenericUnoController: illegal call!" );
    // This ctor only exists because the MSVC compiler complained about an unresolved external
    // symbol. It should not be used at all. Since using it yields strange runtime problems,
    // we simply abort here.
    abort();
}
#endif

// -----------------------------------------------------------------------------
OGenericUnoController::~OGenericUnoController()
{

    DBG_DTOR(OGenericUnoController,NULL);
}

// -----------------------------------------------------------------------------
sal_Bool OGenericUnoController::Construct(Window* /*pParent*/)
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
        m_xDatabaseContext = Reference< XNameAccess >(getORB()->createInstance(SERVICE_SDB_DATABASECONTEXT), UNO_QUERY);
    }
    catch(Exception&)
    {
        OSL_FAIL("OGenericUnoController::Construct: could not create (or start listening at) the database context!");
    }

    if (!m_xDatabaseContext.is())
    {       // at least notify the user. Though the whole component does not make any sense without the database context ...
        ShowServiceNotAvailableError(getView(), String(SERVICE_SDB_DATABASECONTEXT), sal_True);
    }
    return sal_True;
}
//------------------------------------------------------------------------------
IMPL_LINK_NOARG(OGenericUnoController, OnAsyncInvalidateAll)
{
    if ( !OGenericUnoController_Base::rBHelper.bInDispose && !OGenericUnoController_Base::rBHelper.bDisposed )
        InvalidateFeature_Impl();
    return 0L;
}
// -----------------------------------------------------------------------------
void OGenericUnoController::impl_initialize()
{
}
// -------------------------------------------------------------------------
void SAL_CALL OGenericUnoController::initialize( const Sequence< Any >& aArguments ) throw(Exception, RuntimeException)
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
        if ( ( *pIter >>= aValue ) && ( 0 == aValue.Name.compareToAscii( "Frame" ) ) )
        {
            xFrame.set(aValue.Value,UNO_QUERY_THROW);
        }
        else if ( ( *pIter >>= aValue ) && ( 0 == aValue.Name.compareToAscii( "Preview" ) ) )
        {
            aValue.Value >>= m_bPreview;
            m_bReadOnly = sal_True;
        }
    }
    try
    {
        if ( !xFrame.is() )
            throw IllegalArgumentException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "need a frame" ) ), *this, 1 );

        xParent = xFrame->getContainerWindow();
        VCLXWindow* pParentComponent = VCLXWindow::GetImplementation(xParent);
        Window* pParentWin = pParentComponent ? pParentComponent->GetWindow() : NULL;
        if (!pParentWin)
        {
            throw IllegalArgumentException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Parent window is null")), *this, 1 );
        }

        m_aInitParameters.assign( aArguments );
        Construct( pParentWin );

        ODataView* pView = getView();
        if ( !pView )
            throw RuntimeException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("unable to create a view")), *this );

        if ( m_bReadOnly || m_bPreview )
            pView->EnableInput( sal_False );

        impl_initialize();
    }
    catch(Exception&)
    {
        // no one clears my view if I won't
        ::std::auto_ptr<Window> aTemp(m_pView);
        m_pView = NULL;
        throw;
    }
}

//------------------------------------------------------------------------------
void SAL_CALL OGenericUnoController::acquire(  ) throw ()
{
    OGenericUnoController_Base::acquire();
}

//------------------------------------------------------------------------------
void SAL_CALL OGenericUnoController::release(  ) throw ()
{
    OGenericUnoController_Base::release();
}

// -------------------------------------------------------------------------
void OGenericUnoController::startFrameListening( const Reference< XFrame >& _rxFrame )
{
    if ( _rxFrame.is() )
        _rxFrame->addFrameActionListener( this );
}

// -------------------------------------------------------------------------
void OGenericUnoController::stopFrameListening( const Reference< XFrame >& _rxFrame )
{
    if ( _rxFrame.is() )
        _rxFrame->removeFrameActionListener( this );
}

// -------------------------------------------------------------------------
void OGenericUnoController::disposing(const EventObject& Source) throw( RuntimeException )
{
    // our frame ?
    if ( Source.Source == getFrame() )
        stopFrameListening( getFrame() );
}
//------------------------------------------------------------------------
void OGenericUnoController::modified(const EventObject& aEvent) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( getMutex() );
    if ( !isDataSourceReadOnly() )
    {
        Reference<XModifiable> xModi(aEvent.Source,UNO_QUERY);
        if ( xModi.is() )
            m_bCurrentlyModified = xModi->isModified(); // can only be reset by save
        else
            m_bCurrentlyModified = sal_True;
    }
    InvalidateFeature(ID_BROWSER_SAVEDOC);
    InvalidateFeature(ID_BROWSER_UNDO);
}
// -----------------------------------------------------------------------
Reference< XWindow > SAL_CALL OGenericUnoController::getComponentWindow() throw (RuntimeException)
{
    return VCLUnoHelper::GetInterface( getView() );
}

// -----------------------------------------------------------------------
::rtl::OUString SAL_CALL OGenericUnoController::getViewControllerName() throw (::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Default" ) );
}

// -----------------------------------------------------------------------
Sequence< PropertyValue > SAL_CALL OGenericUnoController::getCreationArguments() throw (RuntimeException)
{
    // currently we do not support any creation args, so anything passed to XModel2::createViewController would be
    // lost, so we can equally return an empty sequence here
    return Sequence< PropertyValue >();
}

// -----------------------------------------------------------------------
void OGenericUnoController::attachFrame( const Reference< XFrame >& _rxFrame ) throw( RuntimeException )
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

// -----------------------------------------------------------------------------
struct CommandCollector : public ::std::unary_function< SupportedFeatures::value_type, void>
{
    sal_uInt16  m_nFeature;
    StringBag&  m_rFeatureCommands;
    CommandCollector( sal_uInt16 _nFeature, StringBag& _rFeatureCommands )
        :m_nFeature        ( _nFeature         )
        ,m_rFeatureCommands( _rFeatureCommands )
    {
    }

    void operator() ( const SupportedFeatures::value_type& lhs )
    {
        if ( lhs.second.nFeatureId == m_nFeature )
            m_rFeatureCommands.insert( lhs.first );
    }
};

// -----------------------------------------------------------------------
namespace
{
    typedef ::std::vector< Any >    States;

    // ...................................................................
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

    // ...................................................................
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
            _out_rStates.push_back( makeAny( (sal_Bool)*_rFeatureState.bChecked ) );
        if ( !!_rFeatureState.bInvisible )
            _out_rStates.push_back( makeAny( Visibility( !*_rFeatureState.bInvisible ) ) );
        if ( _rFeatureState.aValue.hasValue() )
            _out_rStates.push_back( _rFeatureState.aValue );
        if ( _out_rStates.empty() )
            _out_rStates.push_back( Any() );
    }
}

// -----------------------------------------------------------------------
void OGenericUnoController::ImplBroadcastFeatureState(const ::rtl::OUString& _rFeature, const Reference< XStatusListener > & xListener, sal_Bool _bIgnoreCache)
{
    sal_uInt16 nFeat = m_aSupportedFeatures[ _rFeature ].nFeatureId;
    FeatureState aFeatState( GetState( nFeat ) );

    FeatureState& rCachedState = m_aStateCache[nFeat];  // creates if neccessary
    if ( !_bIgnoreCache )
    {
        // check if we really need to notify the listeners : this method may be called much more often than needed, so check
        // the cached state of the feature
        sal_Bool bAlreadyCached = ( m_aStateCache.find(nFeat) != m_aStateCache.end() );
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
    aEvent.Source       = (XDispatch*)this;
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
        ::std::for_each(
            m_aSupportedFeatures.begin(),
            m_aSupportedFeatures.end(),
            CommandCollector( nFeat, aFeatureCommands )
        );

        // it is possible that listeners are registered or revoked while
        // we are notifying them, so we must use a copy of m_arrStatusListener, not
        // m_arrStatusListener itself
        Dispatch aNotifyLoop( m_arrStatusListener );
        DispatchIterator iterSearch = aNotifyLoop.begin();
        DispatchIterator iterEnd = aNotifyLoop.end();

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

//------------------------------------------------------------------------------
sal_Bool OGenericUnoController::isFeatureSupported( sal_Int32 _nId )
{
    SupportedFeatures::iterator aFeaturePos = ::std::find_if(
        m_aSupportedFeatures.begin(),
        m_aSupportedFeatures.end(),
        ::std::bind2nd( CompareFeatureById(), _nId )
    );

    return ( m_aSupportedFeatures.end() != aFeaturePos && !aFeaturePos->first.isEmpty());
}

// -----------------------------------------------------------------------
void OGenericUnoController::InvalidateFeature(const ::rtl::OUString& _rURLPath, const Reference< XStatusListener > & _xListener, sal_Bool _bForceBroadcast)
{
    ImplInvalidateFeature( m_aSupportedFeatures[ _rURLPath ].nFeatureId, _xListener, _bForceBroadcast );
}

// -----------------------------------------------------------------------------
void OGenericUnoController::InvalidateFeature_Impl()
{
#ifdef DBG_UTIL
    static sal_Int32 s_nRecursions = 0;
    ++s_nRecursions;
#endif

    sal_Bool bEmpty = sal_True;
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
                ::rtl::OString sMessage( "OGenericUnoController::InvalidateFeature_Impl: feature id " );
                sMessage += ::rtl::OString::valueOf( aNextFeature.nId );
                sMessage += ::rtl::OString( " has been invalidated, but is not supported!" );
                OSL_FAIL( sMessage.getStr() );
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

// -----------------------------------------------------------------------
void OGenericUnoController::ImplInvalidateFeature( sal_Int32 _nId, const Reference< XStatusListener >& _xListener, sal_Bool _bForceBroadcast )
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

    sal_Bool bWasEmpty;
    {
        ::osl::MutexGuard aGuard( m_aFeatureMutex );
        bWasEmpty = m_aFeaturesToInvalidate.empty();
        m_aFeaturesToInvalidate.push_back( aListener );
    }

    if ( bWasEmpty )
        m_aAsyncInvalidateAll.Call();
}

// -----------------------------------------------------------------------
void OGenericUnoController::InvalidateFeature(sal_uInt16 _nId, const Reference< XStatusListener > & _xListener, sal_Bool _bForceBroadcast)
{
    ImplInvalidateFeature( _nId, _xListener, _bForceBroadcast );
}

// -----------------------------------------------------------------------
void OGenericUnoController::InvalidateAll()
{
    ImplInvalidateFeature( ALL_FEATURES, NULL, sal_True );
}

// -----------------------------------------------------------------------------
void OGenericUnoController::InvalidateAll_Impl()
{
    // ---------------------------------
    // invalidate all supported features

    for (   SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.begin();
            aIter != m_aSupportedFeatures.end();
            ++aIter
        )
        ImplBroadcastFeatureState( aIter->first, NULL, sal_True );

    {
        ::osl::MutexGuard aGuard( m_aFeatureMutex);
        OSL_ENSURE(m_aFeaturesToInvalidate.size(), "OGenericUnoController::InvalidateAll_Impl: to be called from within InvalidateFeature_Impl only!");
        m_aFeaturesToInvalidate.pop_front();
        if(!m_aFeaturesToInvalidate.empty())
            m_aAsyncInvalidateAll.Call();
    }
}

// -----------------------------------------------------------------------
Reference< XDispatch >  OGenericUnoController::queryDispatch(const URL& aURL, const ::rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags) throw( RuntimeException )
{
    Reference< XDispatch > xReturn;

    OSL_PRECOND( !m_aSupportedFeatures.empty(), "OGenericUnoController::queryDispatch: shouldn't this be filled at construction time?" );
    if ( m_aSupportedFeatures.empty() )
        fillSupportedFeatures();

    // URL's we can handle ourself?
    if  (   aURL.Complete.equals( getConfirmDeletionURL() )
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

// -----------------------------------------------------------------------
Sequence< Reference< XDispatch > > OGenericUnoController::queryDispatches(const Sequence< DispatchDescriptor >& aDescripts) throw( RuntimeException )
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

// -----------------------------------------------------------------------
Reference< XDispatchProvider >  OGenericUnoController::getSlaveDispatchProvider(void) throw( RuntimeException )
{
    return m_xSlaveDispatcher;
}

// -----------------------------------------------------------------------
void OGenericUnoController::setSlaveDispatchProvider(const Reference< XDispatchProvider > & _xNewProvider) throw( RuntimeException )
{
    m_xSlaveDispatcher = _xNewProvider;
}

// -----------------------------------------------------------------------
Reference< XDispatchProvider >  OGenericUnoController::getMasterDispatchProvider(void) throw( RuntimeException )
{
    return m_xMasterDispatcher;
}

// -----------------------------------------------------------------------
void OGenericUnoController::setMasterDispatchProvider(const Reference< XDispatchProvider > & _xNewProvider) throw( RuntimeException )
{
    m_xMasterDispatcher = _xNewProvider;
}

// -----------------------------------------------------------------------
void OGenericUnoController::dispatch(const URL& _aURL, const Sequence< PropertyValue >& aArgs) throw(RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    // The SolarMutex is not locked anymore when the framework calls into
    // here. So, lock it ourself. The real solution would be to lock it only in the places
    // where it's needed, but a) this might turn out difficult, since we then also need to care
    // for locking in the proper order (SolarMutex and m_aMutex), and b) this would be too many places
    // for the time frame of the fix.
    // #i52602#

#ifdef TIMELOG
    ::rtl::OString sLog( "OGenericUnoController::dispatch( '" );
    sLog += ::rtl::OString( _aURL.Main.getStr(), _aURL.Main.getLength(), osl_getThreadTextEncoding() );
    sLog += ::rtl::OString( "' )" );
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "frank.schoenheit@sun.com", sLog.getStr() );
#endif

    executeChecked(_aURL,aArgs);
}

// -----------------------------------------------------------------------
void OGenericUnoController::addStatusListener(const Reference< XStatusListener > & aListener, const URL& _rURL) throw(RuntimeException)
{
    // parse the ULR now and here, this saves later parsing in each notification round
    URL aParsedURL( _rURL );
    if ( m_xUrlTransformer.is() )
        m_xUrlTransformer->parseStrict( aParsedURL );

    // remember the listener together with the URL
    m_arrStatusListener.insert( m_arrStatusListener.end(), DispatchTarget( aParsedURL, aListener ) );

    // initially broadcast the state
    ImplBroadcastFeatureState( aParsedURL.Complete, aListener, sal_True );
        // force the new state to be broadcast to the new listener
}

// -----------------------------------------------------------------------
void OGenericUnoController::removeStatusListener(const Reference< XStatusListener > & aListener, const URL& _rURL) throw(RuntimeException)
{
    DispatchIterator iterSearch = m_arrStatusListener.begin();

    sal_Bool bRemoveForAll = _rURL.Complete.isEmpty();
    while ( iterSearch != m_arrStatusListener.end() )
    {
        DispatchTarget& rCurrent = *iterSearch;
        if  (   (rCurrent.xListener == aListener)
            &&  (   bRemoveForAll
                ||  (rCurrent.aURL.Complete.equals(_rURL.Complete))
                )
            )
        {
            m_arrStatusListener.erase( iterSearch );
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
        StateCacheIterator aCachePos = m_aStateCache.find( aIter->second.nFeatureId );
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
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------
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
    m_xServiceFactory = NULL;
    m_xTitleHelper.clear();
    m_xUrlTransformer.clear();
    m_aInitParameters.clear();
}

// -----------------------------------------------------------------------------
void SAL_CALL OGenericUnoController::addEventListener( const Reference< XEventListener >& xListener ) throw (RuntimeException)
{
    // disambiguate
    OGenericUnoController_Base::WeakComponentImplHelperBase::addEventListener( xListener );
}

// -----------------------------------------------------------------------------
void SAL_CALL OGenericUnoController::removeEventListener( const Reference< XEventListener >& xListener ) throw (RuntimeException)
{
    // disambiguate
    OGenericUnoController_Base::WeakComponentImplHelperBase::removeEventListener( xListener );
}

//------------------------------------------------------------------------------
void OGenericUnoController::frameAction(const FrameActionEvent& aEvent) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( getMutex() );
    if ( aEvent.Frame == m_aCurrentFrame.getFrame() )
        m_aCurrentFrame.frameAction( aEvent.Action );
}

//------------------------------------------------------------------------------
void OGenericUnoController::implDescribeSupportedFeature( const sal_Char* _pAsciiCommandURL,
        sal_uInt16 _nFeatureId, sal_Int16 _nCommandGroup )
{
#ifdef DBG_UTIL
    OSL_ENSURE( m_bDescribingSupportedFeatures, "OGenericUnoController::implDescribeSupportedFeature: bad timing for this call!" );
#endif
    OSL_PRECOND( _nFeatureId < FIRST_USER_DEFINED_FEATURE, "OGenericUnoController::implDescribeSupportedFeature: invalid feature id!" );

    ControllerFeature aFeature;
    aFeature.Command = ::rtl::OUString::createFromAscii( _pAsciiCommandURL );
    aFeature.nFeatureId = _nFeatureId;
    aFeature.GroupId = _nCommandGroup;

#if OSL_DEBUG_LEVEL > 0
    OSL_ENSURE( m_aSupportedFeatures.find( aFeature.Command ) == m_aSupportedFeatures.end(),
        "OGenericUnoController::implDescribeSupportedFeature: this feature is already there!" );
#endif
    m_aSupportedFeatures[ aFeature.Command ] = aFeature;
}

//------------------------------------------------------------------------------
void OGenericUnoController::describeSupportedFeatures()
{
    // add all supported features
    implDescribeSupportedFeature( ".uno:Copy", ID_BROWSER_COPY, CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:Cut", ID_BROWSER_CUT, CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:Paste", ID_BROWSER_PASTE, CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:ClipboardFormatItems", ID_BROWSER_CLIPBOARD_FORMAT_ITEMS );
    implDescribeSupportedFeature( ".uno:DSBEditDoc", ID_BROWSER_EDITDOC, CommandGroup::DOCUMENT );
}

//------------------------------------------------------------------------------
FeatureState OGenericUnoController::GetState( sal_uInt16 _nId ) const
{
    FeatureState aReturn;
        // (disabled automatically)

    switch ( _nId )
    {
        case ID_BROWSER_UNDO:
        case ID_BROWSER_SAVEDOC:
            aReturn.bEnabled = sal_True;
            break;
        default:
            aReturn = m_pData->m_aUserDefinedFeatures.getState( getURLForId( _nId ) );
            break;
    }

    return aReturn;
}

//------------------------------------------------------------------------------
void OGenericUnoController::Execute( sal_uInt16 _nId, const Sequence< PropertyValue>& _rArgs )
{
    OSL_ENSURE( isUserDefinedFeature( _nId ),
        "OGenericUnoController::Execute: responsible for user defined features only!" );

    // user defined features can be handled by dispatch interceptors resp. protocol handlers only.
    // So, we need to do a queryDispatch, and dispatch the URL
    m_pData->m_aUserDefinedFeatures.execute( getURLForId( _nId ), _rArgs );
}

//------------------------------------------------------------------------------
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

//-------------------------------------------------------------------------
bool OGenericUnoController::isUserDefinedFeature( const sal_uInt16 _nFeatureId ) const
{
    return ( _nFeatureId >= FIRST_USER_DEFINED_FEATURE ) && ( _nFeatureId < LAST_USER_DEFINED_FEATURE );
}

//-------------------------------------------------------------------------
bool OGenericUnoController::isUserDefinedFeature( const ::rtl::OUString& _rFeatureURL ) const
{
    SupportedFeatures::const_iterator pos = m_aSupportedFeatures.find( _rFeatureURL );
    OSL_PRECOND( pos != m_aSupportedFeatures.end(),
        "OGenericUnoController::isUserDefinedFeature: this is no supported feature at all!" );

    return ( pos != m_aSupportedFeatures.end() ) ? isUserDefinedFeature( pos->second.nFeatureId ) : false;
}

//-------------------------------------------------------------------------
sal_Bool SAL_CALL OGenericUnoController::supportsService(const ::rtl::OUString& ServiceName) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());

    const ::rtl::OUString* pArray = aSupported.getConstArray();
    const ::rtl::OUString* pArrayEnd = aSupported.getConstArray() + aSupported.getLength();

    for ( ;( pArray != pArrayEnd ) && !pArray->equals( ServiceName ); ++pArray )
        ;
    return pArray != pArrayEnd;
}

// -----------------------------------------------------------------------------
void OGenericUnoController::startConnectionListening(const Reference< XConnection >& _rxConnection)
{
    // we have to remove ourself before dispoing the connection
    Reference< XComponent >  xComponent(_rxConnection, UNO_QUERY);
    if (xComponent.is())
        xComponent->addEventListener(static_cast<XFrameActionListener*>(this));
}

// -----------------------------------------------------------------------------
void OGenericUnoController::stopConnectionListening(const Reference< XConnection >& _rxConnection)
{
    // we have to remove ourself before dispoing the connection
    Reference< XComponent >  xComponent(_rxConnection, UNO_QUERY);
    if (xComponent.is())
        xComponent->removeEventListener(static_cast<XFrameActionListener*>(this));
}
// -----------------------------------------------------------------------------
Reference< XConnection > OGenericUnoController::connect( const Reference< XDataSource>& _xDataSource,
    ::dbtools::SQLExceptionInfo* _pErrorInfo )
{
    WaitObject aWaitCursor( getView() );

    ODatasourceConnector aConnector( getORB(), getView(), ::rtl::OUString() );
    Reference< XConnection > xConnection = aConnector.connect( _xDataSource, _pErrorInfo );
    startConnectionListening( xConnection );

    return xConnection;
}
// -----------------------------------------------------------------------------
Reference< XConnection > OGenericUnoController::connect( const ::rtl::OUString& _rDataSourceName,
    const ::rtl::OUString& _rContextInformation, ::dbtools::SQLExceptionInfo* _pErrorInfo )
{
    WaitObject aWaitCursor( getView() );

    ODatasourceConnector aConnector( getORB(), getView(), _rContextInformation );
    Reference<XConnection> xConnection = aConnector.connect( _rDataSourceName, _pErrorInfo );
    startConnectionListening( xConnection );

    return xConnection;
}

// -----------------------------------------------------------------------------
void OGenericUnoController::showError(const SQLExceptionInfo& _rInfo)
{
    ::dbaui::showError(_rInfo,getView(),getORB());
}
// -----------------------------------------------------------------------------
Reference< XLayoutManager > OGenericUnoController::getLayoutManager(const Reference< XFrame >& _xFrame) const
{
    Reference< XPropertySet > xPropSet( _xFrame, UNO_QUERY );
    Reference< XLayoutManager > xLayoutManager;
    if ( xPropSet.is() )
    {
        try
        {
            xLayoutManager.set(xPropSet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LayoutManager" ))),UNO_QUERY);
        }
        catch ( Exception& )
        {
        }
    }
    return xLayoutManager;
}
// -----------------------------------------------------------------------------
void OGenericUnoController::loadMenu(const Reference< XFrame >& _xFrame)
{
    Reference< XLayoutManager > xLayoutManager = getLayoutManager(_xFrame);
    if ( xLayoutManager.is() )
    {
        xLayoutManager->lock();
        xLayoutManager->createElement( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:resource/menubar/menubar" )));
        xLayoutManager->createElement( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/toolbar" )));
        xLayoutManager->unlock();
        xLayoutManager->doLayout();
    }

    onLoadedMenu( xLayoutManager );
}

// -----------------------------------------------------------------------------
void OGenericUnoController::onLoadedMenu(const Reference< XLayoutManager >& /*_xLayoutManager*/)
{
    // not interested in
}

// -----------------------------------------------------------------------------
void OGenericUnoController::closeTask()
{
    m_aAsyncCloseTask.Call();
}
// -----------------------------------------------------------------------------
IMPL_LINK_NOARG(OGenericUnoController, OnAsyncCloseTask)
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
    return 0L;
}
// -----------------------------------------------------------------------------
Any SAL_CALL OGenericUnoController::getViewData(void) throw( RuntimeException )
{
    return Any();
}
// -----------------------------------------------------------------------------
void SAL_CALL OGenericUnoController::restoreViewData(const Any& /*Data*/) throw( RuntimeException )
{
}

// -----------------------------------------------------------------------------
Reference< XModel > SAL_CALL OGenericUnoController::getModel(void) throw( RuntimeException )
{
    return Reference< XModel >();
}

// -----------------------------------------------------------------------------
Reference< XFrame > SAL_CALL OGenericUnoController::getFrame(void) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( getMutex() );
    return m_aCurrentFrame.getFrame();
}

// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OGenericUnoController::attachModel(const Reference< XModel > & /*xModel*/) throw( RuntimeException )
{
    OSL_FAIL( "OGenericUnoController::attachModel: not supported!" );
    return sal_False;
}

// -----------------------------------------------------------------------------
void OGenericUnoController::executeUnChecked(sal_uInt16 _nCommandId, const Sequence< PropertyValue >& aArgs)
{
    Execute(_nCommandId, aArgs);
}
// -----------------------------------------------------------------------------
void OGenericUnoController::executeUnChecked(const util::URL& _rCommand, const Sequence< PropertyValue >& aArgs)
{
    OSL_PRECOND( !m_aSupportedFeatures.empty(), "OGenericUnoController::executeUnChecked: shouldn't this be filled at construction time?" );
    if ( m_aSupportedFeatures.empty() )
        fillSupportedFeatures();

    SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.find( _rCommand.Complete );
    if (aIter != m_aSupportedFeatures.end())
        Execute( aIter->second.nFeatureId, aArgs );
}
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
//------------------------------------------------------------------------------
namespace
{
    ::rtl::OUString lcl_getModuleHelpModuleName( const Reference< XFrame >& _rxFrame )
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
                    xParentFrame = xParentFrame.query( _rxFrame->getCreator() );
                // did we find a parent frame? Which is no top-level frame?
                if ( xParentFrame.is() && !_rxFrame->isTop() )
                    // TODO: to prevent framework assertions, re-insert this "isTop" once 98303 is fixed
                    return lcl_getModuleHelpModuleName( xParentFrame );
            }
            else
            {
#if OSL_DEBUG_LEVEL > 0
                Sequence< ::rtl::OUString > sServiceNames = xSI->getSupportedServiceNames();
                const ::rtl::OUString* pLoop = sServiceNames.getConstArray();
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
                    if ( xSI->supportsService( ::rtl::OUString::createFromAscii( *pDocumentService ) ) )
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
                if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SWRITER ) )
                    pReturn = "swriter";
                else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SDATABASE ) )
                    pReturn = "sdatabase";
                else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SCALC ) )
                    pReturn = "scalc";
                else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SIMPRESS ) )
                    pReturn = "simpress";
                else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SDRAW ) )
                    pReturn = "sdraw";
                else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SMATH ) )
                    pReturn = "smath";
                else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SCHART ) )
                    pReturn = "schart";
                else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SBASIC ) )
                    pReturn = "sbasic";
                else
                {
                    OSL_FAIL( "lcl_getModuleHelpModuleName: no installed module found" );
                }
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        if ( !pReturn )
            pReturn = "swriter";

        return ::rtl::OUString::createFromAscii( pReturn );
    }
}

// -----------------------------------------------------------------------------

void OGenericUnoController::openHelpAgent(rtl::OUString const& _suHelpStringURL )
{
    rtl::OUString suURL(_suHelpStringURL);
    rtl::OUString sLanguage(RTL_CONSTASCII_USTRINGPARAM("Language="));
    if (suURL.indexOf(sLanguage) == -1)
    {
        AppendConfigToken(suURL, sal_False /* sal_False := add '&' */ );
    }
    URL aURL;
    aURL.Complete = suURL;

    openHelpAgent( aURL );
}

void OGenericUnoController::openHelpAgent(const rtl::OString& _sHelpId)
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
            xHelpDispatch = xDispProv->queryDispatch(aURL, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_helpagent")), FrameSearchFlag::PARENT | FrameSearchFlag::SELF);
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
// -----------------------------------------------------------------------------
Reference< awt::XWindow> OGenericUnoController::getTopMostContainerWindow() const
{
    Reference< ::com::sun::star::awt::XWindow> xWindow;

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
// -----------------------------------------------------------------------------
Reference< XTitle > OGenericUnoController::impl_getTitleHelper_throw()
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getMutex() );

    if ( ! m_xTitleHelper.is ())
    {
        Reference< XUntitledNumbers > xUntitledProvider(getPrivateModel(), UNO_QUERY      );
        Reference< XController >      xThis(static_cast< XController* >(this), UNO_QUERY_THROW);

        ::framework::TitleHelper* pHelper = new ::framework::TitleHelper(m_xServiceFactory);
        m_xTitleHelper.set( static_cast< ::cppu::OWeakObject* >(pHelper), UNO_QUERY_THROW);

        pHelper->setOwner                   (xThis            );
        pHelper->connectWithUntitledNumbers (xUntitledProvider);
    }

    return m_xTitleHelper;
}

//=============================================================================
// XTitle
::rtl::OUString SAL_CALL OGenericUnoController::getTitle()
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( getMutex() );
    if ( m_bExternalTitle )
        return impl_getTitleHelper_throw()->getTitle ();
    return getPrivateTitle() + impl_getTitleHelper_throw()->getTitle ();
}

//=============================================================================
// XTitle
void SAL_CALL OGenericUnoController::setTitle(const ::rtl::OUString& sTitle)
    throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getMutex() );
    m_bExternalTitle = sal_True;
    impl_getTitleHelper_throw()->setTitle (sTitle);
}

//=============================================================================
// XTitleChangeBroadcaster
void SAL_CALL OGenericUnoController::addTitleChangeListener(const Reference< XTitleChangeListener >& xListener)
    throw (RuntimeException)
{
    Reference< XTitleChangeBroadcaster > xBroadcaster(impl_getTitleHelper_throw(), UNO_QUERY);
    if (xBroadcaster.is ())
        xBroadcaster->addTitleChangeListener (xListener);
}

// -----------------------------------------------------------------------------
void SAL_CALL OGenericUnoController::removeTitleChangeListener(const Reference< XTitleChangeListener >& xListener)
    throw (RuntimeException)
{
    Reference< XTitleChangeBroadcaster > xBroadcaster(impl_getTitleHelper_throw(), UNO_QUERY);
    if (xBroadcaster.is ())
        xBroadcaster->removeTitleChangeListener (xListener);
}

// =============================================================================
// XUserInputInterception
// -----------------------------------------------------------------------------
void SAL_CALL OGenericUnoController::addKeyHandler( const Reference< XKeyHandler >& _rxHandler ) throw (RuntimeException)
{
    if ( _rxHandler.is() )
        m_pData->m_aUserInputInterception.addKeyHandler( _rxHandler );
}

// -----------------------------------------------------------------------------
void SAL_CALL OGenericUnoController::removeKeyHandler( const Reference< XKeyHandler >& _rxHandler ) throw (RuntimeException)
{
    m_pData->m_aUserInputInterception.removeKeyHandler( _rxHandler );
}

// -----------------------------------------------------------------------------
void SAL_CALL OGenericUnoController::addMouseClickHandler( const Reference< XMouseClickHandler >& _rxHandler ) throw (RuntimeException)
{
    if ( _rxHandler.is() )
        m_pData->m_aUserInputInterception.addMouseClickHandler( _rxHandler );
}

// -----------------------------------------------------------------------------
void SAL_CALL OGenericUnoController::removeMouseClickHandler( const Reference< XMouseClickHandler >& _rxHandler ) throw (RuntimeException)
{
    m_pData->m_aUserInputInterception.removeMouseClickHandler( _rxHandler );
}

// =============================================================================
// -----------------------------------------------------------------------------
void OGenericUnoController::executeChecked(sal_uInt16 _nCommandId, const Sequence< PropertyValue >& aArgs)
{
    if ( isCommandEnabled(_nCommandId) )
        Execute(_nCommandId, aArgs);
}

// -----------------------------------------------------------------------------
sal_Bool OGenericUnoController::isCommandEnabled(sal_uInt16 _nCommandId) const
{
    return GetState( _nCommandId ).bEnabled;
}

// -----------------------------------------------------------------------------
sal_uInt16 OGenericUnoController::registerCommandURL( const ::rtl::OUString& _rCompleteCommandURL )
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
        OSL_FAIL( "OGenericUnoController::registerCommandURL: no more space for user defined features!" );
        return 0L;
    }

    ControllerFeature aFeature;
    aFeature.Command = _rCompleteCommandURL;
    aFeature.nFeatureId = nFeatureId;
    aFeature.GroupId = CommandGroup::INTERNAL;
    m_aSupportedFeatures[ aFeature.Command ] = aFeature;

    return nFeatureId;
}

// -----------------------------------------------------------------------------
void OGenericUnoController::notifyHiContrastChanged()
{
}

// -----------------------------------------------------------------------------
sal_Bool OGenericUnoController::isDataSourceReadOnly() const
{
    return sal_False;
}

// -----------------------------------------------------------------------------
Reference< XController > OGenericUnoController::getXController() throw( RuntimeException )
{
    return this;
}

// -----------------------------------------------------------------------------
bool OGenericUnoController::interceptUserInput( const NotifyEvent& _rEvent )
{
    return m_pData->m_aUserInputInterception.handleNotifyEvent( _rEvent );
}

// -----------------------------------------------------------------------------
sal_Bool OGenericUnoController::isCommandChecked(sal_uInt16 _nCommandId) const
{
    FeatureState aState = GetState( _nCommandId );

    return aState.bChecked && (sal_Bool)*aState.bChecked;
}
// -----------------------------------------------------------------------------
sal_Bool OGenericUnoController::isCommandEnabled( const ::rtl::OUString& _rCompleteCommandURL ) const
{
    OSL_ENSURE( !_rCompleteCommandURL.isEmpty(), "OGenericUnoController::isCommandEnabled: Empty command url!" );

    sal_Bool bIsEnabled = sal_False;
    SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.find( _rCompleteCommandURL );
    if ( aIter != m_aSupportedFeatures.end() )
        bIsEnabled = isCommandEnabled( aIter->second.nFeatureId );

    return bIsEnabled;
}

// -----------------------------------------------------------------------------
Sequence< ::sal_Int16 > SAL_CALL OGenericUnoController::getSupportedCommandGroups() throw (RuntimeException)
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

// -----------------------------------------------------------------------------
Sequence< DispatchInformation > SAL_CALL OGenericUnoController::getConfigurableDispatchInformation( ::sal_Int16 CommandGroup ) throw (RuntimeException)
{
    DispatchInfoList    aInformationList;
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
// -----------------------------------------------------------------------------
void OGenericUnoController::fillSupportedFeatures()
{
#ifdef DBG_UTIL
    m_bDescribingSupportedFeatures = true;
#endif
    describeSupportedFeatures();
// -----------------------------------------------------------------------------
#ifdef DBG_UTIL
    m_bDescribingSupportedFeatures = false;
#endif
}


void SAL_CALL OGenericUnoController::dispose() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    OGenericUnoController_Base::dispose();
}
}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
