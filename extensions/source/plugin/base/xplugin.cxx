/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifdef AIX
#define _LINUX_SOURCE_COMPAT
#include <sys/timer.h>
#undef _LINUX_SOURCE_COMPAT
#endif

#ifdef WNT
#include <prewin.h>
#include <postwin.h>
#undef OPTIONAL
#endif

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/loader/XImplementationLoader.hpp>
#include <com/sun/star/plugin/PluginManager.hpp>

#include <comphelper/componentcontext.hxx>
#include <comphelper/processfactory.hxx>
#include <plugin/impl.hxx>
#include <tools/fsys.hxx>
#include <ucbhelper/content.hxx>
#include <tools/urlobj.hxx>
#include <tools/string.hxx>
#include <vcl/svapp.hxx>
#include <salhelper/timer.hxx>
#include <osl/file.hxx>

#ifdef UNX
#include <sys/types.h>
#include <sys/socket.h>
#endif

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

using namespace com::sun::star;
using namespace com::sun::star::io;
using namespace com::sun::star::beans;
using namespace com::sun::star::plugin;
using namespace osl;

using ::rtl::OUString;
using ::rtl::OString;

class PluginDisposer : public salhelper::Timer
{
private:
    XPlugin_Impl*       m_pPlugin;

    virtual void SAL_CALL onShot();
public:
    PluginDisposer( XPlugin_Impl* pPlugin ) :
        salhelper::Timer( salhelper::TTimeValue( 2, 0 ),
                          salhelper::TTimeValue( 2, 0 ) ),
        m_pPlugin( pPlugin )
        { start(); }
    ~PluginDisposer() {}
};

void PluginDisposer::onShot()
{
    if( m_pPlugin )
    {
        if( m_pPlugin->isDisposable() )
        {
            sal_uLong nEvent;
            Application::PostUserEvent( nEvent, LINK( m_pPlugin, XPlugin_Impl, secondLevelDispose ), (void*)m_pPlugin );
        }
    }
    else
        release();
}

//==================================================================================================

Any XPlugin_Impl::queryInterface( const Type& type ) throw( RuntimeException )
{
    return OWeakAggObject::queryInterface( type );
}

Any XPlugin_Impl::queryAggregation( const Type& type ) throw( RuntimeException )
{
    Any aRet( cppu::queryInterface( type, static_cast< XPlugin* >(this) ) );
    if( ! aRet.hasValue() )
        aRet = PluginControl_Impl::queryAggregation( type );
    return aRet;
}


XPlugin_Impl::XPlugin_Impl( const uno::Reference< com::sun::star::lang::XMultiServiceFactory >  & rSMgr) :
        PluginControl_Impl(),
        m_xSMgr( rSMgr ),
        m_pPluginComm( NULL ),
        m_pSysPlugData( CreateSysPlugData() ),
        m_aEncoding( osl_getThreadTextEncoding() ),
        m_pArgv( NULL ),
        m_pArgn( NULL ),
        m_nArgs( 0 ),
        m_aPluginMode( NP_FULL ),
        m_nProvidingState( PROVIDING_NONE ),
        m_nCalledFromPlugin( 0 ),
        m_pDisposer( NULL ),
        m_bIsDisposed( sal_False )
{
    memset( &m_aInstance, 0, sizeof( m_aInstance ) );
    memset( &m_aNPWindow, 0, sizeof( m_aNPWindow ) );

    m_xModel = new PluginModel();
    uno::Reference< com::sun::star::beans::XPropertySet >  xPS( m_xModel, UNO_QUERY );
    xPS->addPropertyChangeListener( OUString(), this );

    Guard< Mutex > aGuard( ::PluginManager::get().getPluginMutex() );
    ::PluginManager::get().getPlugins().push_back( this );
}

void XPlugin_Impl::destroyInstance()
{
    Guard< Mutex > aGuard( m_aMutex );

    NPSavedData* pSavedData = NULL;

    destroyStreams();
    if( getPluginComm() )
    {
        getPluginComm()->NPP_Destroy( this, &pSavedData );
        getPluginComm()->decRef();
        m_pPluginComm = NULL;
    }

    freeArgs();

    while( m_aPEventListeners.size() )
    {
        delete *m_aPEventListeners.begin();
        m_aPEventListeners.pop_front();
    }
}

XPlugin_Impl::~XPlugin_Impl()
{
    destroyInstance();
}

void XPlugin_Impl::checkListeners( const char* normalizedURL )
{
    if( ! normalizedURL )
        return;

    Guard< Mutex > aGuard( m_aMutex );

    std::list<PluginEventListener*>::iterator iter;
    for( iter = m_aPEventListeners.begin();
         iter != m_aPEventListeners.end();
         ++iter )
    {
        if( ! strcmp( normalizedURL, (*iter)->getURL() ) ||
            ! strcmp( normalizedURL, (*iter)->getNormalizedURL() ) )
        {
            (*iter)->disposing( com::sun::star::lang::EventObject() );
            delete *iter;
            m_aPEventListeners.remove( *iter );
            return;
        }
    }
}

IMPL_LINK( XPlugin_Impl, secondLevelDispose, XPlugin_Impl*, /*pThis*/ )
{
    Guard< Mutex > aGuard( m_aMutex );

    // may have become undisposable between PostUserEvent and here
    // or may have disposed and receive a second UserEvent
    std::list<XPlugin_Impl*>& rList = ::PluginManager::get().getPlugins();
    std::list<XPlugin_Impl*>::iterator iter;

    {
        Guard< Mutex > aPluginGuard( ::PluginManager::get().getPluginMutex() );
        for( iter = rList.begin(); iter != rList.end(); ++iter )
        {
            if( *iter == this )
                break;
        }
        if( iter == rList.end() || ! isDisposable() )
            return 0;
    }

    if (m_pDisposer)
    {
        m_pDisposer->release();
        m_pDisposer = NULL;
    }

    uno::Reference< XPlugin >  xProtection( this );
    uno::Reference< com::sun::star::beans::XPropertySet >  xPS( m_xModel, UNO_QUERY );
    xPS->removePropertyChangeListener( OUString(), this );
    {
        Guard< Mutex > aPluginGuard( ::PluginManager::get().getPluginMutex() );
        rList.remove( this );
    }
    m_aNPWindow.window = NULL;
#ifndef UNX
    // acrobat does an unconditional XtParent on the windows widget
    getPluginComm()->NPP_SetWindow( this );
#endif
    destroyInstance();
    PluginControl_Impl::dispose();
    return 0;
}

void XPlugin_Impl::dispose() throw()
{
    Guard< Mutex > aGuard( m_aMutex );

    if (m_bIsDisposed || !getPluginComm())
        return;
    m_bIsDisposed = sal_True;

    if( isDisposable() )
        secondLevelDispose( this );
    else
    {
        m_pDisposer = new PluginDisposer( this );
        m_pDisposer->acquire();
    }
}

void XPlugin_Impl::initArgs( const Sequence< OUString >& argn,
                             const Sequence< OUString >& argv,
                             sal_Int16 mode )
{
    m_aPluginMode = mode;

    m_nArgs = argn.getLength();
    m_pArgn = new const char*[m_nArgs];
    m_pArgv = new const char*[m_nArgs];
    const OUString* pUArgn = argn.getConstArray();
    const OUString* pUArgv = argv.getConstArray();
    for( int i = 0; i < m_nArgs; i++ )
    {
        m_pArgn[i] = strdup(
            OUStringToOString( pUArgn[i], m_aEncoding ).getStr()
            );
        m_pArgv[i] = strdup(
            OUStringToOString( pUArgv[i], m_aEncoding ).getStr()
            );
    }
}

void XPlugin_Impl::freeArgs()
{
    if( m_nArgs > 0 )
    {
        for( ; m_nArgs--; )
        {
            free( (void*)m_pArgn[m_nArgs] );
            free( (void*)m_pArgv[m_nArgs] );
        }
        delete [] m_pArgn;
        delete [] m_pArgv;
    }
}

void XPlugin_Impl::prependArg( const char* pName, const char* pValue )
{
    const char** pNewNames      = new const char*[m_nArgs+1];
    const char** pNewValues = new const char*[m_nArgs+1];

    pNewNames[0]        = strdup( pName );
    pNewValues[0]       = strdup( pValue );
    for( int nIndex = 0; nIndex < m_nArgs; ++nIndex )
    {
        pNewNames[nIndex+1] = m_pArgn[nIndex];
        pNewValues[nIndex+1]= m_pArgv[nIndex];
    }
    // free old arrays
    delete [] m_pArgn;
    delete [] m_pArgv;
    // set new arrays
    m_pArgn = pNewNames;
    m_pArgv = pNewValues;
    // set new number of arguments
    m_nArgs++;
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "inserted %s=%s\n", pNewNames[0], pNewValues[0] );
#endif
}

void XPlugin_Impl::handleSpecialArgs()
{
    // special handling for real audio which needs a lot of parameters
    // or won't function at all
    if( ! m_aDescription.Mimetype.compareToAscii( "audio/x-pn-realaudio-plugin" ) && m_nArgs < 1 )
    {
        OUString aURL;
        if( m_xModel.is() )
        {
            try
            {
                uno::Reference< XPropertySet > xProp( m_xModel, UNO_QUERY );
                Any aProp = xProp->getPropertyValue( OUString(  "URL"  ) );
                aProp >>= aURL;
            }
            catch(const UnknownPropertyException &)
            {
            }
        }

        if( !aURL.isEmpty() )
        {
            // set new args, old args need not be freed as there were none set
            m_nArgs = 6;
            m_pArgn = new const char*[m_nArgs];
            m_pArgv = new const char*[m_nArgs];

            // SRC
            m_pArgn[0]      = strdup( "SRC" );
            m_pArgv[0]      = strdup( OUStringToOString( aURL, m_aEncoding ).getStr() );
            // WIDTH
            m_pArgn[1]      = strdup( "WIDTH" );
            m_pArgv[1]      = strdup( "200" );
            // HEIGHT
            m_pArgn[2]      = strdup( "HEIGHT" );
            m_pArgv[2]      = strdup( "200" );
            // CONTROLS
            m_pArgn[3]      = strdup( "CONTROLS" );
            m_pArgv[3]      = strdup( "PlayButton,StopButton,ImageWindow" );
            // AUTOSTART
            m_pArgn[4]      = strdup( "AUTOSTART" );
            m_pArgv[4]      = strdup( "TRUE" );
            // NOJAVA
            m_pArgn[5]      = strdup( "NOJAVA" );
            m_pArgv[5]      = strdup( "TRUE" );
        }
    }
    // #69333# special for pdf
    else if( ! m_aDescription.Mimetype.compareToAscii( "application/pdf" ) )
        m_aPluginMode = PluginMode::FULL;

    // see if we have a TYPE tag
    int nIndex;
    for( nIndex = 0; nIndex < m_nArgs; ++nIndex )
        if( m_pArgn[nIndex][0] == 'T' &&
            m_pArgn[nIndex][1] == 'Y' &&
            m_pArgn[nIndex][2] == 'P' &&
            m_pArgn[nIndex][3] == 'E' &&
            m_pArgn[nIndex][4] == 0 )
            break;
    if( nIndex >= m_nArgs )
    {
        // TYPE
        prependArg( "TYPE", OUStringToOString( m_aDescription.Mimetype, m_aEncoding ).getStr() );
    }

    // see if we have a SRC tag
    for( nIndex = 0; nIndex < m_nArgs; ++nIndex )
        if( m_pArgn[nIndex][0] == 'S' &&
            m_pArgn[nIndex][1] == 'R' &&
            m_pArgn[nIndex][2] == 'C' &&
            m_pArgn[nIndex][3] == 0 )
            break;
    if( nIndex >= m_nArgs )
    {
        // need a SRC parameter (as all browser set one on the plugin
        OUString aURL;
        if( m_xModel.is() )
        {
            try
            {
                uno::Reference< XPropertySet > xProp( m_xModel, UNO_QUERY );
                Any aProp = xProp->getPropertyValue( OUString(  "URL"  ) );
                aProp >>= aURL;
            }
            catch(const UnknownPropertyException &)
            {
            }
        }

        if( !aURL.isEmpty() )
        {
            // SRC
            prependArg( "SRC", OUStringToOString( aURL, m_aEncoding ).getStr() );
        }
    }
}

void XPlugin_Impl::initInstance( const PluginDescription& rDescription,
                                 const Sequence< OUString >& argn,
                                 const Sequence< OUString >& argv,
                                 sal_Int16 mode )
{
    Guard< Mutex > aGuard( m_aMutex );

    m_aDescription = rDescription;
    initArgs( argn, argv, mode );
    handleSpecialArgs();
}

void XPlugin_Impl::initInstance( const OUString& rURL,
                                 const Sequence< OUString >& argn,
                                 const Sequence< OUString >& argv,
                                 sal_Int16 mode )
{
    Guard< Mutex > aGuard( m_aMutex );

    initArgs( argn, argv, mode );
    m_aDescription = fitDescription( rURL );

    m_xModel = new PluginModel( rURL, m_aDescription.Mimetype );
    handleSpecialArgs();
}

void XPlugin_Impl::modelChanged()
{
    Guard< Mutex > aGuard( m_aMutex );

    m_nProvidingState = PROVIDING_MODEL_UPDATE;

    destroyInstance();

    m_aDescription = fitDescription( getCreationURL() );
    if( m_aDescription.Mimetype.isEmpty() )
    {
        m_nProvidingState = PROVIDING_NONE;
        return;
    }

    OUString aURL = getCreationURL();
    provideNewStream( m_aDescription.Mimetype,
                      uno::Reference< XActiveDataSource >(),
                      aURL,
                      0, 0, (sal_Bool)(aURL.compareToAscii( "file:", 5 ) == 0) );
    m_nProvidingState = PROVIDING_NONE;
}

OUString XPlugin_Impl::getCreationURL()
{
    Guard< Mutex > aGuard( m_aMutex );

    OUString aRet;
    uno::Reference< com::sun::star::beans::XPropertySet >  xPS( m_xModel, UNO_QUERY );
    if( xPS.is() )
    {
        Any aValue = xPS->getPropertyValue( OUString("URL") );
        aValue >>= aRet;
    }
    return aRet;
}


sal_Bool XPlugin_Impl::setModel( const uno::Reference< com::sun::star::awt::XControlModel > & Model )
    throw( RuntimeException )
{
    Guard< Mutex > aGuard( m_aMutex );

    uno::Reference< com::sun::star::beans::XPropertySet >  xPS( Model, UNO_QUERY );
    if( ! xPS.is() )
        return sal_False;

    if( !getCreationURL().isEmpty() )
    {
        m_xModel = Model;
        modelChanged();
        xPS->addPropertyChangeListener( OUString(), this );
        return sal_True;
    }
    return sal_False;
}

void XPlugin_Impl::createPeer( const uno::Reference< com::sun::star::awt::XToolkit > & xToolkit, const uno::Reference< com::sun::star::awt::XWindowPeer > & Parent )
    throw( RuntimeException )
{
    Guard< Mutex > aGuard( m_aMutex );

    if( ! _xPeer.is() )
    {
        if( ! Parent.is() )
            throw  RuntimeException();
        PluginControl_Impl::createPeer( xToolkit, Parent );
    }
}

void XPlugin_Impl::loadPlugin()
{
    Guard< Mutex > aGuard( m_aMutex );

    std::list<PluginComm*>::iterator iter;
    for( iter = ::PluginManager::get().getPluginComms().begin();
         iter != ::PluginManager::get().getPluginComms().end(); ++iter )
    {
        if( OStringToOUString( (*iter)->getLibName(), m_aEncoding ) == m_aDescription.PluginName )
        {
            setPluginComm( *iter );
            break;
        }
    }
    const SystemEnvData* pEnvData = getSysChildSysData();
#if defined( UNX ) && !(defined(QUARTZ))
    if (pEnvData->pDisplay) // headless?
    {
        XSync( (Display*)pEnvData->pDisplay, False );
    }
#endif
    if( ! getPluginComm() )
    {
        if( !m_aDescription.PluginName.isEmpty() )
        {
#if defined QUARTZ
            PluginComm* pComm = new MacPluginComm( m_aDescription.Mimetype,
                                                   m_aDescription.PluginName,
                                                   pEnvData->pView );
#elif defined UNX
            // need a new PluginComm
            PluginComm* pComm = NULL;
            int sv[2];
            if( !socketpair( AF_UNIX, SOCK_STREAM, 0, sv ) )
                pComm = new UnxPluginComm( m_aDescription.Mimetype,
                                           m_aDescription.PluginName,
                                           (XLIB_Window)pEnvData->aWindow,
                                           sv[0],
                                           sv[1]
                                           );
#elif defined WNT
            PluginComm* pComm = new PluginComm_Impl( m_aDescription.Mimetype,
                                                     m_aDescription.PluginName,
                                                     (HWND)pEnvData->hWnd );
#endif
            setPluginComm( pComm );
        }
        else
            return;
    }

    getPluginComm()->
        NPP_New( (char*)OUStringToOString( m_aDescription.Mimetype,
                                                  m_aEncoding).getStr(),
                 getNPPInstance(),
                 m_aPluginMode == PluginMode::FULL ? NP_FULL : NP_EMBED,
                 ::sal::static_int_cast< int16_t, int >( m_nArgs ),
                 (char**)(m_nArgs ? m_pArgn : NULL),
                 (char**)(m_nArgs ? m_pArgv : NULL),
                 NULL );
#ifdef QUARTZ
    // m_aNPWindow is set up in the MacPluginComm from the view
    SetSysPlugDataParentView(*pEnvData);
#elif defined( UNX )
    if (pEnvData->pDisplay) // headless?
    {
        XSync( (Display*)pEnvData->pDisplay, False );
        m_aNPWindow.window  = (void*)pEnvData->aWindow;
    }
    else
    {
        m_aNPWindow.window  = NULL;
    }
    m_aNPWindow.ws_info     = NULL;
#else
    m_aNPWindow.window = (void*)pEnvData->hWnd;
#endif
    com::sun::star::awt::Rectangle aPosSize = getPosSize();

    for( int i = 0; i < m_nArgs; i++ )
    {
        OString aName( m_pArgn[i] );
        if( aName.equalsIgnoreAsciiCase( "width" ) )
        {
            OString aValue( m_pArgv[i] );
            aPosSize.Width = aValue.toInt32();
        }
        else if( aName.equalsIgnoreAsciiCase( "height" ) )
        {
            OString aValue( m_pArgv[i] );
            aPosSize.Height = aValue.toInt32();
        }
    }

    m_aNPWindow.clipRect.top        = 0;
    m_aNPWindow.clipRect.left       = 0;
    m_aNPWindow.clipRect.bottom     = ::sal::static_int_cast< uint16_t, sal_Int32 >( aPosSize.Height );
    m_aNPWindow.clipRect.right      = ::sal::static_int_cast< uint16_t, sal_Int32 >( aPosSize.Width );
    m_aNPWindow.type = NPWindowTypeWindow;

    m_aNPWindow.x       = 0;
    m_aNPWindow.y       = 0;
    m_aNPWindow.width   = aPosSize.Width ? aPosSize.Width : 600;
    m_aNPWindow.height  = aPosSize.Height ? aPosSize.Height : 600;

    getPluginComm()->NPP_SetWindow( this );
}

void XPlugin_Impl::destroyStreams()
{
    Guard< Mutex > aGuard( m_aMutex );

    // streams remove themselves from this list when deleted
    while( m_aOutputStreams.size() )
        delete *m_aOutputStreams.begin();

    // input streams are XOutputStreams, they cannot be simply deleted
    std::list<PluginInputStream*> aLocalList( m_aInputStreams );
    for( std::list<PluginInputStream*>::iterator it = aLocalList.begin();
         it != aLocalList.end(); ++it )
        (*it)->setMode( -1 );
}

PluginStream* XPlugin_Impl::getStreamFromNPStream( NPStream* stream )
{
    Guard< Mutex > aGuard( m_aMutex );

    std::list<PluginInputStream*>::iterator iter;
    for( iter = m_aInputStreams.begin(); iter != m_aInputStreams.end(); ++iter )
        if( (*iter)->getStream() == stream )
            return *iter;

    std::list<PluginOutputStream*>::iterator iter2;
    for( iter2 = m_aOutputStreams.begin(); iter2 != m_aOutputStreams.end(); ++iter2 )
        if( (*iter2)->getStream() == stream )
            return *iter2;

    return NULL;
}

sal_Bool XPlugin_Impl::provideNewStream(const OUString& mimetype,
                                        const uno::Reference< com::sun::star::io::XActiveDataSource > & stream,
                                        const OUString& url, sal_Int32 length,
                                        sal_Int32 lastmodified, sal_Bool isfile) throw()

{
    Guard< Mutex > aGuard( m_aMutex );
    sal_Bool bRet = sal_False;

    if( m_nProvidingState != PROVIDING_NONE )
    {
        m_nProvidingState = PROVIDING_NOW;
        Any aAny;
        aAny <<= url;
        uno::Reference< com::sun::star::beans::XPropertySet >  xPS( m_xModel, UNO_QUERY );
        if( xPS.is() )
        {
            try
            {
                xPS->setPropertyValue( OUString("URL"), aAny );
                aAny <<= mimetype;
                xPS->setPropertyValue( OUString("TYPE"), aAny );
            }
            catch(...)
            {
            }
        }
    }
    m_nProvidingState = PROVIDING_NOW;

    OString aMIME;
    if( !mimetype.isEmpty() )
        aMIME = OUStringToOString( mimetype, m_aEncoding );
    else
        aMIME = OUStringToOString( m_aDescription.Mimetype, m_aEncoding );

    OString aURL  = OUStringToOString( url, m_aEncoding );

    // check whether there is a notifylistener for this stream
    // this means that the strema is created from the plugin
    // via NPN_GetURLNotify or NPN_PostURLNotify
    std::list<PluginEventListener*>::iterator iter;
    for( iter = m_aPEventListeners.begin();
         iter != m_aPEventListeners.end();
         ++iter )
    {
        if( (*iter)->getNormalizedURL() == aURL )
        {
            aURL = (*iter)->getURL();
            break;
        }
    }

    if( ! m_pPluginComm )
    {
        loadPlugin();
        if( !m_aLastGetUrl.isEmpty() && m_aLastGetUrl == aURL )
        {
            // plugin is pulling data, don't push the same stream;
            // this complicated method could have been avoided if
            // all plugins respected the SRC parameter; but e.g.
            // acrobat reader plugin does not
            m_nProvidingState = PROVIDING_NONE;
            return sal_True;
        }
    }
     if( ! m_pPluginComm )
        return sal_False;

     if(  url.isEmpty() )
         // this is valid if the plugin is supposed to
         // pull data (via e.g. NPN_GetURL)
         return sal_True;

     // set mimetype on model
     {
         uno::Reference< com::sun::star::beans::XPropertySet >  xPS( m_xModel, UNO_QUERY );
         if( xPS.is() )
         {
             try
             {
                 Any aAny;
                 aAny <<= m_aDescription.Mimetype;
                 xPS->setPropertyValue( OUString("TYPE"), aAny );
             }
             catch(...)
             {
             }
         }
     }

     // there may be plugins that can use the file length information,
     // but currently none are known. Since this file opening/seeking/closing
     // is rather costly, it is not implemented. If there are plugins known to
     // make use of the file length, simply put it in

     PluginInputStream* pStream = new PluginInputStream( this, aURL.getStr(),
                                                        length, lastmodified );
     uno::Reference< com::sun::star::io::XOutputStream > xNewStream( pStream );

     if( iter != m_aPEventListeners.end() )
         pStream->getStream()->notifyData = (*iter)->getNotifyData();

    uint16_t stype = 0;

    // special handling acrobat reader
    // presenting a seekable stream to it does not seem to work correctly
    if( aMIME.equals( "application/pdf" ) )
        isfile = sal_False;

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr,
             "new stream \"%s\" of MIMEType \"%s\"\n"
             "for plugin \"%s\"\n"
             "seekable = %s, length = %" SAL_PRIdINT32 "\n",
             aURL.getStr(), aMIME.getStr(), getPluginComm()->getLibName().getStr(),
             isfile ? "true" : "false", length );

#endif
    if( ! m_pPluginComm->NPP_NewStream( &m_aInstance,
                                        (char*)aMIME.getStr(),
                                        pStream->getStream(), isfile,
                                        &stype ) )
    {
#if OSL_DEBUG_LEVEL > 1
        const char* pType;
        switch( stype )
        {
            case NP_NORMAL:     pType = "NP_NORMAL";break;
            case NP_SEEK:       pType = "NP_SEEK";break;
            case NP_ASFILE:     pType = "NP_ASFILE";break;
            case NP_ASFILEONLY: pType = "NP_ASFILEONLY";break;
            default:            pType = "unknown!!!";
        }
        fprintf( stderr, "Plugin wants it in Mode %s\n", pType );
#endif
        if( isfile && stype == NP_ASFILEONLY )
        {
            rtl::OString aFileName;
            if( url.compareToAscii( "file:", 5 ) == 0 )
            {
                OUString aSysName;
                osl_getSystemPathFromFileURL( url.pData, &aSysName.pData );
                aFileName = rtl::OUStringToOString( aSysName, m_aEncoding );
            }
            else
                aFileName = rtl::OUStringToOString( url, m_aEncoding );
            m_pPluginComm->
                NPP_StreamAsFile( &m_aInstance,
                                  pStream->getStream(),
                                  aFileName.getStr() );
        }
        else
        {
            pStream->setMode( stype );

            if( ! stream.is() )
            {
                // stream has to be loaded by PluginStream itself via UCB
                pStream->load();
            }
            else
            {
                uno::Reference< com::sun::star::io::XConnectable > xConnectable( stream, UNO_QUERY );
                pStream->setPredecessor( xConnectable );
                if( xConnectable.is() )
                {
                    xConnectable->setSuccessor( static_cast< com::sun::star::io::XConnectable* >(pStream) );
                    while( xConnectable->getPredecessor().is() )
                        xConnectable = xConnectable->getPredecessor();
                }
                stream->setOutputStream( xNewStream );
                pStream->setSource( stream );
                uno::Reference< com::sun::star::io::XActiveDataControl > xController;
                if( xConnectable.is() )
                    xController = uno::Reference< com::sun::star::io::XActiveDataControl >( xConnectable, UNO_QUERY );
                else
                    xController = uno::Reference< com::sun::star::io::XActiveDataControl >( stream, UNO_QUERY );

                if( xController.is() )
                    xController->start();
            }
        }
        bRet = sal_True;
    }

    m_nProvidingState = PROVIDING_NONE;

    return bRet;
}

void XPlugin_Impl::disposing( const com::sun::star::lang::EventObject& /*rSource*/ ) throw()
{
}

void XPlugin_Impl::propertyChange( const com::sun::star::beans::PropertyChangeEvent& rEvent ) throw()
{
    Guard< Mutex > aGuard( m_aMutex );

    if( ! rEvent.PropertyName.compareToAscii( "URL" ) )
    {
        OUString aStr;
        rEvent.NewValue >>= aStr;
        if( m_nProvidingState == PROVIDING_NONE )
        {
            if( aStr != m_aURL )
            {
                m_aURL = aStr;
                modelChanged();
            }
        }
    }
}

void XPlugin_Impl::setPluginContext( const uno::Reference< XPluginContext > & rContext )
{
    m_rBrowserContext = rContext;
}

void XPlugin_Impl::setPosSize( sal_Int32 nX_, sal_Int32 nY_, sal_Int32 nWidth_, sal_Int32 nHeight_, sal_Int16 nFlags )
        throw( RuntimeException )
{
    Guard< Mutex > aGuard( m_aMutex );

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "XPlugin_Impl::setPosSize( %" SAL_PRIdINT32 ", %" SAL_PRIdINT32 ", %" SAL_PRIdINT32 ", %" SAL_PRIdINT32 ", %d )\n",
             nX_, nY_, nWidth_, nHeight_, nFlags );
#endif

    PluginControl_Impl::setPosSize(nX_, nY_, nWidth_, nHeight_, nFlags);

    m_aNPWindow.x                   = 0;
    m_aNPWindow.y                   = 0;
    m_aNPWindow.width               = nWidth_;
    m_aNPWindow.height              = nHeight_;
    m_aNPWindow.clipRect.top        = 0;
    m_aNPWindow.clipRect.left       = 0;
    m_aNPWindow.clipRect.right      = ::sal::static_int_cast< uint16_t, sal_Int32 >( nWidth_ );
    m_aNPWindow.clipRect.bottom     = ::sal::static_int_cast< uint16_t, sal_Int32 >( nHeight_ );

    if( getPluginComm() )
        getPluginComm()->NPP_SetWindow( this );
}

PluginDescription XPlugin_Impl::fitDescription( const OUString& rURL )
{
    uno::Reference< XPluginManager >  xPMgr( plugin::PluginManager::create(comphelper::ComponentContext(m_xSMgr).getUNOContext()) );

    Sequence< PluginDescription > aDescrs = xPMgr->getPluginDescriptions();
    const PluginDescription* pDescrs = aDescrs.getConstArray();

    for( int nArg = 0; nArg < m_nArgs; nArg++ )
    {
        if( strncmp( m_pArgn[nArg], "TYPE", 4 ) == 0 &&
            m_pArgn[nArg][4] == 0 )
        {
            for( int i = 0; i < aDescrs.getLength(); i++ )
            {
                if( pDescrs[i].Mimetype.compareToAscii( m_pArgv[nArg] ) == 0 )
                    return pDescrs[i];
            }
        }
    }

    int nPos = rURL.lastIndexOf( (sal_Unicode)'.' );
    if( nPos != -1 )
    {
        OUString const aExt = rURL.copy( nPos ).toAsciiLowerCase();
        for( int i = 0; i < aDescrs.getLength(); i++ )
        {
            OUString aThisExt = pDescrs[ i ].Extension.toAsciiLowerCase();
            if( aThisExt.indexOf( aExt ) != -1 )
            {
                return pDescrs[i];
            }
        }
    }
    return PluginDescription();
}


PluginStream::PluginStream( XPlugin_Impl* pPlugin,
                            const char* url, sal_uInt32 len, sal_uInt32 lastmod ) :
        m_pPlugin( pPlugin )
{
    memset( &m_aNPStream, 0, sizeof( m_aNPStream ) );
    m_aNPStream.url             = strdup( url );
    m_aNPStream.end             = len;
    m_aNPStream.lastmodified    = lastmod;
}

PluginStream::~PluginStream()
{
    Guard< Mutex > aGuard( m_pPlugin->getMutex() );

    if( m_pPlugin && m_pPlugin->getPluginComm() )
    {
        m_pPlugin->getPluginComm()->NPP_DestroyStream( m_pPlugin->getNPPInstance(),
                                                       &m_aNPStream, NPRES_DONE );
        m_pPlugin->checkListeners( m_aNPStream.url );
        m_pPlugin->getPluginComm()->NPP_SetWindow( m_pPlugin );
    }
    ::free( (void*)m_aNPStream.url );
}

PluginInputStream::PluginInputStream( XPlugin_Impl* pPlugin,
                                      const char* url,
                                      sal_uInt32 len,
                                      sal_uInt32 lastmod ) :
        PluginStream( pPlugin, url, len, lastmod ),
        m_pContent( NULL ),
        m_nMode( NP_NORMAL ),
        m_nWritePos( 0 )
{
    Guard< Mutex > aGuard( m_pPlugin->getMutex() );

    m_pPlugin->getInputStreams().push_back( this );
    DirEntry aEntry;
    aEntry = aEntry.TempName();

    // set correct extension, some plugins need that
    DirEntry aName( String( m_aNPStream.url, m_pPlugin->getTextEncoding() ) );
    String aExtension = aName.GetExtension();
    if( aExtension.Len() )
        aEntry.SetExtension( aExtension );
    m_aFileStream.Open( aEntry.GetFull(), STREAM_READ | STREAM_WRITE );
    if( ! m_aFileStream.IsOpen() )
    {
        // #74808# might be that the extension scrambled the whole filename
        aEntry = aEntry.TempName();
        m_aFileStream.Open( aEntry.GetFull(), STREAM_READ | STREAM_WRITE );
    }
}

PluginInputStream::~PluginInputStream()
{
    Guard< Mutex > aGuard( m_pPlugin->getMutex() );

    m_pPlugin->getInputStreams().remove( this );

    String aFile( m_aFileStream.GetFileName() );

    m_aFileStream.Close();
    if( m_pPlugin )
    {
        rtl::OString aFileName(rtl::OUStringToOString(aFile, m_pPlugin->getTextEncoding()));
        if( m_pPlugin->getPluginComm() && m_nMode != -1 )
            // mode -1 means either an error occurred,
            // or the plugin is already disposing
        {
            m_pPlugin->getPluginComm()->addFileToDelete( aFile );
            if( m_nMode == NP_ASFILE )
            {
                m_pPlugin->getPluginComm()->
                    NPP_StreamAsFile( m_pPlugin->getNPPInstance(),
                                      &m_aNPStream,
                                      aFileName.getStr() );
            }
            m_pPlugin->getPluginComm()->NPP_SetWindow( m_pPlugin );
            m_pPlugin->getInputStreams().remove( this );
        }
        else
            DirEntry( m_aFileStream.GetFileName() ).Kill();
    }
    else
        DirEntry( m_aFileStream.GetFileName() ).Kill();
    if( m_pContent )
        delete m_pContent;
}

PluginStreamType PluginInputStream::getStreamType()
{
    return InputStream;
}

void PluginInputStream::load()
{
    Guard< Mutex > aGuard( m_pPlugin->getMutex() );

    INetURLObject aUrl;
    aUrl.SetSmartProtocol( INET_PROT_FILE );
    aUrl.SetSmartURL(
        String( getStream()->url,
                ::sal::static_int_cast< sal_uInt16, size_t >( strlen( getStream()->url ) ),
                RTL_TEXTENCODING_MS_1252
            ) );
    try
    {
        m_pContent =
            new ::ucbhelper::Content(
                               aUrl.GetMainURL(INetURLObject::DECODE_TO_IURI),
                               uno::Reference< com::sun::star::ucb::XCommandEnvironment >(),
                               comphelper::getProcessComponentContext()
                               );
        m_pContent->openStream( static_cast< XOutputStream* >( this ) );
    }
    catch(const com::sun::star::uno::Exception &)
    {
    }
}

void PluginInputStream::setMode( sal_Int32 nMode )
{
    Guard< Mutex > aGuard( m_pPlugin->getMutex() );

    m_nMode = nMode;

    // invalidation by plugin
    if( m_nMode == -1 && m_pPlugin )
    {
        m_pPlugin->getInputStreams().remove( this );
        m_pPlugin = NULL;
    }
}

void PluginInputStream::writeBytes( const Sequence<sal_Int8>& Buffer ) throw()
{
    Guard< Mutex > aGuard( m_pPlugin->getMutex() );

    m_aFileStream.Seek( STREAM_SEEK_TO_END );
    m_aFileStream.Write( Buffer.getConstArray(), Buffer.getLength() );

    if( m_nMode == NP_SEEK )
        // hold reference, streem gets destroyed in NPN_DestroyStream
        m_xSelf = this;

    if( m_nMode == -1 || !m_pPlugin->getPluginComm() )
        return;

    sal_uInt32 nPos = m_aFileStream.Tell();
    sal_uInt32 nBytes = 0;
    while( m_nMode != NP_ASFILEONLY &&
           m_nWritePos < nPos &&
           (nBytes = m_pPlugin->getPluginComm()-> NPP_WriteReady(
               m_pPlugin->getNPPInstance(), &m_aNPStream )) > 0 )
    {
        nBytes = (nBytes > nPos - m_nWritePos) ? nPos - m_nWritePos : nBytes;

        char* pBuffer = new char[ nBytes ];
        m_aFileStream.Seek( m_nWritePos );
        nBytes = m_aFileStream.Read( pBuffer, nBytes );

        int32_t nBytesRead = 0;
        try
        {
            nBytesRead = m_pPlugin->getPluginComm()->NPP_Write(
                m_pPlugin->getNPPInstance(), &m_aNPStream, m_nWritePos, nBytes, pBuffer );
        }
        catch( ... )
        {
            nBytesRead = 0;
        }
        delete [] pBuffer;

        if( nBytesRead < 0 )
        {
            m_nMode = -1;
            return;
        }

        m_nWritePos += nBytesRead;
    }
}

void PluginInputStream::closeOutput() throw()
{
    Guard< Mutex > aGuard( m_pPlugin->getMutex() );

    flush();
    m_xSource = uno::Reference< com::sun::star::io::XActiveDataSource >();
}

sal_uInt32 PluginInputStream::read( sal_uInt32 offset, sal_Int8* buffer, sal_uInt32 size )
{
    Guard< Mutex > aGuard( m_pPlugin->getMutex() );

    if( m_nMode != NP_SEEK )
        return 0;

    m_aFileStream.Seek( offset );
    return m_aFileStream.Read( buffer, size );
}

void PluginInputStream::flush(void) throw()
{
}

PluginOutputStream::PluginOutputStream( XPlugin_Impl* pPlugin,
                                        const char* url,
                                        sal_uInt32 len,
                                        sal_uInt32 lastmod ) :
        PluginStream( pPlugin, url, len, lastmod ),
        m_xStream( pPlugin->getServiceManager()->createInstance( OUString("com.sun.star.io.DataOutputStream") ), UNO_QUERY )
{
    Guard< Mutex > aGuard( m_pPlugin->getMutex() );

    m_pPlugin->getOutputStreams().push_back( this );
}

PluginOutputStream::~PluginOutputStream()
{
    Guard< Mutex > aGuard( m_pPlugin->getMutex() );

    m_pPlugin->getOutputStreams().remove( this );
}

PluginStreamType PluginOutputStream::getStreamType()
{
    return OutputStream;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
