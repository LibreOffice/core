/*************************************************************************
 *
 *  $RCSfile: xplugin.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:51 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <string>   // workaround for SUNPRO workshop include conflicts

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LOADER_XIMPLEMENTATIONLOADER_HPP_
#include <com/sun/star/loader/XImplementationLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_LOADER_CANNOTACTIVATEFACTORYEXCEPTION_HPP_
#include <com/sun/star/loader/CannotActivateFactoryException.hpp>
#endif

#include <plugin/impl.hxx>
#include <tools/fsys.hxx>

#include <tools/urlobj.hxx>
#include <tools/string.hxx>
#include <vcl/svapp.hxx>
#include <vos/timer.hxx>

#if STLPORT_VERSION>=321
#include <cstdarg>
#endif

class PluginDisposer : public ::vos::OTimer
{
private:
    XPlugin_Impl*       m_pPlugin;

    virtual void SAL_CALL onShot();
public:
    PluginDisposer( XPlugin_Impl* pPlugin ) :
        OTimer( ::vos::TTimeValue( 2, 0 ),
                ::vos::TTimeValue( 2, 0 ) ),
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
            sal_uInt32 nEvent;
            Application::PostUserEvent( nEvent, LINK( m_pPlugin, XPlugin_Impl, secondLevelDispose ), (void*)m_pPlugin );
        }
    }
    else
        release();
}

//==================================================================================================

//==================================================================================================
//  const Reference< ::com::sun::star::reflection::XIdlClass > & XPlugin_Impl::staticGetIdlClass()
//  {
//      static Reference< ::com::sun::star::reflection::XIdlClass >  aClass = createStandardClass( L"Plugin",
//                                                        OWeakAggObject::getStaticIdlClass(), 6,
//                                                        ::getCppuType((const ::com::sun::star::awt::XControl*)0),
//                                                        ::getCppuType((const ::com::sun::star::awt::XControlModel*)0),
//                                                        ::getCppuType((const ::com::sun::star::awt::XWindow*)0),
//                                                        ::getCppuType((const ::com::sun::star::lang::XComponent*)0),
//                                                        ::getCppuType((const ::com::sun::star::awt::XView*)0),
//                                                        ::getCppuType((const ::com::sun::star::plugin::XPlugin*)0) );
//      return aClass;
//  }

Any XPlugin_Impl::queryInterface( const Type& type )

{
    return OWeakAggObject::queryInterface( type );
}

Any XPlugin_Impl::queryAggregation( const Type& type )
{
    Any aRet( ::cppu::queryInterface( type, static_cast< ::com::sun::star::plugin::XPlugin* >(this) ) );
    if( ! aRet.hasValue() )
        aRet = PluginControl_Impl::queryAggregation( type );
    return aRet;
}


XPlugin_Impl::XPlugin_Impl( const Reference< ::com::sun::star::lang::XMultiServiceFactory >  & rSMgr) :
        m_xSMgr( rSMgr ),
        PluginControl_Impl(),
        m_pPluginComm( NULL ),
        m_pArgn( NULL ),
        m_pArgv( NULL ),
        m_nArgs( 0 ),
        m_aPluginMode( NP_FULL ),
        m_nProvidingState( PROVIDING_NONE ),
        m_nCalledFromPlugin( 0 ),
        m_pDisposer( NULL ),
        m_bIsDisposed( sal_False ),
        m_aEncoding( gsl_getSystemTextEncoding() )
{
    memset( &m_aInstance, 0, sizeof( m_aInstance ) );
    memset( &m_aNPWindow, 0, sizeof( m_aNPWindow ) );
#ifdef UNX
    m_aAppContext = NULL;
    m_pDisplay = NULL;
    memset( &m_aWSInfo, 0, sizeof( m_aWSInfo ) );
#endif

    m_xModel = new PluginModel();
    Reference< ::com::sun::star::beans::XPropertySet >  xPS( m_xModel, UNO_QUERY );
    xPS->addPropertyChangeListener( ::rtl::OUString(), this );

    ::osl::Guard< ::osl::Mutex > aGuard( PluginManager::get().getPluginMutex() );
    PluginManager::get().getPlugins().push_back( this );
}

void XPlugin_Impl::destroyInstance()
{
    NPSavedData* pSavedData = NULL;

    destroyStreams();
    if( getPluginComm() )
    {
        getPluginComm()->NPP_Destroy( getNPPInstance(),
                                      &pSavedData );
        getPluginComm()->decRef();
        m_pPluginComm = NULL;
    }

    if( m_nArgs > 0 )
    {
        for( ; m_nArgs--; )
        {
            free( (void*)m_pArgn[m_nArgs] );
            free( (void*)m_pArgv[m_nArgs] );
        }
        delete m_pArgn;
        delete m_pArgv;
    }
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

    ::std::list<PluginEventListener*>::iterator iter;
    for( iter = m_aPEventListeners.begin();
         iter != m_aPEventListeners.end();
         ++iter )
    {
        if( ! strcmp( normalizedURL, (*iter)->getURL() ) ||
            ! strcmp( normalizedURL, (*iter)->getNormalizedURL() ) )
        {
            (*iter)->disposing( ::com::sun::star::lang::EventObject() );
            delete *iter;
            m_aPEventListeners.remove( *iter );
            return;
        }
    }
}

IMPL_LINK( XPlugin_Impl, secondLevelDispose, XPlugin_Impl*, pThis )
{
    // may have become undisposable between PostUserEvent and here
    // or may have disposed and receive a second UserEvent
    ::std::list<XPlugin_Impl*>& rList = PluginManager::get().getPlugins();
    ::std::list<XPlugin_Impl*>::iterator iter;

    {
        ::osl::Guard< ::osl::Mutex > aGuard( PluginManager::get().getPluginMutex() );
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

    Reference< ::com::sun::star::plugin::XPlugin >  xProtection( this );
    Reference< ::com::sun::star::beans::XPropertySet >  xPS( m_xModel, UNO_QUERY );
    xPS->removePropertyChangeListener( ::rtl::OUString(), this );
    {
        ::osl::Guard< ::osl::Mutex > aGuard( PluginManager::get().getPluginMutex() );
        rList.remove( this );
    }
    m_aNPWindow.window = NULL;
#ifndef UNX
    // acrobat does an unconditional XtParent on the windows widget
    getPluginComm()->
        NPP_SetWindow( getNPPInstance(), &m_aNPWindow );
#endif
    destroyInstance();
    PluginControl_Impl::dispose();
    return 0;
}

void XPlugin_Impl::dispose()
{
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

void XPlugin_Impl::initInstance( const ::com::sun::star::plugin::PluginDescription& rDescription,
                                 const Sequence< ::rtl::OUString >& argn,
                                 const Sequence< ::rtl::OUString >& argv,
                                 sal_Int16 mode )
{
    m_aDescription = rDescription;

    // #69333# special for pdf
    m_aPluginMode = mode;
    if( m_aDescription.Mimetype.compareToAscii( "application/pdf" ) )
        m_aPluginMode = ::com::sun::star::plugin::PluginMode::FULL;

    m_nArgs = argn.getLength();
    m_pArgn = new const char*[m_nArgs];
    m_pArgv = new const char*[m_nArgs];
    const ::rtl::OUString* pUArgn = argn.getConstArray();
    const ::rtl::OUString* pUArgv = argv.getConstArray();
    for( int i = 0; i < m_nArgs; i++ )
    {
        m_pArgn[i] = strdup(
            ::rtl::OUStringToOString( pUArgn[i], m_aEncoding ).getStr()
            );
        m_pArgv[i] = strdup(
            ::rtl::OUStringToOString( pUArgv[i], m_aEncoding ).getStr()
            );
    }
}

void XPlugin_Impl::modelChanged()
{
    m_nProvidingState = PROVIDING_MODEL_UPDATE;

    // empty description is only set when created by createPluginFromURL
    if( m_aDescription.Mimetype.getLength() )
        destroyInstance();

    Reference< ::com::sun::star::plugin::XPluginManager >  xPMgr( m_xSMgr->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.plugin.PluginManager" ) ), UNO_QUERY );
    if( !xPMgr.is() )
    {
        m_nProvidingState = PROVIDING_NONE;
        return;
    }

    int nDescr = -1;
    Sequence< ::com::sun::star::plugin::PluginDescription > aDescrs = xPMgr->getPluginDescriptions();
    const ::com::sun::star::plugin::PluginDescription* pDescrs = aDescrs.getConstArray();

    ::rtl::OUString aURL = getCreationURL();
    int nPos = aURL.lastIndexOf( (sal_Unicode)'.' );
    ::rtl::OUString aExt = aURL.copy( nPos ).toLowerCase();
    if( nPos != -1 )
    {
        for( int i = 0; i < aDescrs.getLength(); i++ )
        {
            ::rtl::OUString aThisExt = pDescrs[ i ].Extension.toLowerCase();
            if( aThisExt.indexOf( aExt ) != -1 )
            {
                nDescr = i;
                break;
            }
        }
    }

    if( nDescr != -1 )
    {
        INetURLObject aURL;
        aURL.SetSmartProtocol( INET_PROT_FILE );
        aURL.SetSmartURL( ::rtl::OUStringToOString( getCreationURL(), m_aEncoding ) );

        Reference< ::com::sun::star::lang::XMultiServiceFactory >  xFact( m_xSMgr->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.frame.DataSourceFactory" ) ), UNO_QUERY );
        if ( xFact.is() )
        {
            Sequence < Any > aArgs(1);
            aArgs.getArray()[0] <<= ::rtl::OUString( aURL.GetMainURL() );
            ::rtl::OUString aProt( INetURLObject::GetScheme( aURL.GetProtocol() ) );
            Reference< ::com::sun::star::io::XActiveDataSource >  xSource( xFact->createInstanceWithArguments( aProt, aArgs ), UNO_QUERY );
            if ( xSource.is() )
            {
                m_aDescription = pDescrs[ nDescr ];
                provideNewStream( m_aDescription.Mimetype,
                                xSource,
                                  getRefererURL(),
                                  0, 0, sal_False );
            }
        }
    }
    m_nProvidingState = PROVIDING_NONE;
}

::rtl::OUString XPlugin_Impl::getCreationURL()
{
    ::rtl::OUString aRet;
    Reference< ::com::sun::star::beans::XPropertySet >  xPS( m_xModel, UNO_QUERY );
    if( xPS.is() )
    {
        Any aValue = xPS->getPropertyValue( ::rtl::OUString::createFromAscii( "URL" ) );
        aValue >>= aRet;
    }
    return aRet;
}


sal_Bool XPlugin_Impl::setModel( const Reference< ::com::sun::star::awt::XControlModel > & Model )
    throw( RuntimeException )
{
    Reference< ::com::sun::star::beans::XPropertySet >  xPS( Model, UNO_QUERY );
    if( ! xPS.is() )
        return sal_False;

    if( getCreationURL().getLength() )
    {
        m_xModel = Model;
        modelChanged();
        xPS->addPropertyChangeListener( ::rtl::OUString(), this );
        return sal_True;
    }
    return sal_False;
}

void XPlugin_Impl::createPeer( const Reference< ::com::sun::star::awt::XToolkit > & xToolkit, const Reference< ::com::sun::star::awt::XWindowPeer > & Parent )
    throw( RuntimeException )
{
    if( ! _xPeer.is() )
    {
        if( ! Parent.is() )
            throw  RuntimeException();
        PluginControl_Impl::createPeer( xToolkit, Parent );
    }
}

void XPlugin_Impl::loadPlugin()
{
    ::std::list<PluginComm*>::iterator iter;
    for( iter = PluginManager::get().getPluginComms().begin();
         iter != PluginManager::get().getPluginComms().end(); ++iter )
    {
        if( ::rtl::OStringToOUString( (*iter)->getLibName(), m_aEncoding ) == m_aDescription.PluginName )
        {
            setPluginComm( *iter );
            break;
        }
    }
    const SystemEnvData* pEnvData = getSysChildSysData();
#ifdef UNX
    XSync( (Display*)pEnvData->pDisplay, False );
#endif
    if( ! getPluginComm() )
    {
#ifdef UNX
        m_pDisplay      = (Display*)pEnvData->pDisplay;
        m_aAppContext   = (XtAppContext)pEnvData->pAppContext;
        // need a new PluginComm
        PluginComm* pComm = new UnxPluginComm( ::rtl::OUStringToOString( m_aDescription.PluginName, gsl_getSystemTextEncoding() ) );
#elif (defined WNT || defined OS2)
        PluginComm* pComm = new PluginComm_Impl( m_aDescription.Mimetype,
                                                 m_aDescription.PluginName,
                                                 (HWND)pEnvData->hWnd );
#endif
        setPluginComm( pComm );
    }

    NPError aError = getPluginComm()->
        NPP_New( (char*)::rtl::OUStringToOString( m_aDescription.Mimetype,
                                                  m_aEncoding).getStr(),
                 getNPPInstance(),
                 m_aPluginMode,
                 m_nArgs,
                 (char**)(m_nArgs ? m_pArgn : NULL),
                 (char**)(m_nArgs ? m_pArgv : NULL),
                 NULL );

#ifdef UNX
    XSync( (Display*)pEnvData->pDisplay, False );
#endif
#ifdef UNX
    m_aNPWindow.window      = (void*)pEnvData->aWindow;
    m_aNPWindow.ws_info     = &m_aWSInfo;

    m_aWSInfo.type          = NP_SETWINDOW;
    m_aWSInfo.display       = (Display*)pEnvData->pDisplay;
    m_aWSInfo.visual        = (Visual*)pEnvData->pVisual;
    m_aWSInfo.colormap      = (Colormap)pEnvData->aColormap;
    m_aWSInfo.depth         = pEnvData->nDepth;
#else
    m_aNPWindow.window = (void*)pEnvData->hWnd;
#endif
    ::com::sun::star::awt::Rectangle aPosSize = getPosSize();

    m_aNPWindow.clipRect.top        = 0;
    m_aNPWindow.clipRect.left       = 0;
    m_aNPWindow.clipRect.bottom     = 0;
    m_aNPWindow.clipRect.right      = 0;
    m_aNPWindow.type = NPWindowTypeWindow;

    m_aNPWindow.x       = aPosSize.X;
    m_aNPWindow.y       = aPosSize.Y;
    m_aNPWindow.width   = aPosSize.Width ? aPosSize.Width : 600;
    m_aNPWindow.height  = aPosSize.Height ? aPosSize.Height : 600;

    aError = getPluginComm()->
        NPP_SetWindow( getNPPInstance(), &m_aNPWindow );
}

void XPlugin_Impl::destroyStreams()
{
    // streams remove themselves from this list when deleted
    while( m_aOutputStreams.size() )
        delete *m_aOutputStreams.begin();

    // input streams are XOutputStreams, they cannot be simply deleted
    ::std::list<PluginInputStream*> aLocalList( m_aInputStreams );
    for( ::std::list<PluginInputStream*>::iterator it = aLocalList.begin();
         it != aLocalList.end(); ++it )
        (*it)->setMode( -1 );
}

PluginStream* XPlugin_Impl::getStreamFromNPStream( NPStream* stream )
{
    ::std::list<PluginInputStream*>::iterator iter;
    for( iter = m_aInputStreams.begin(); iter != m_aInputStreams.end(); ++iter )
        if( (*iter)->getStream() == stream )
            return *iter;

    ::std::list<PluginOutputStream*>::iterator iter2;
    for( iter2 = m_aOutputStreams.begin(); iter2 != m_aOutputStreams.end(); ++iter2 )
        if( (*iter2)->getStream() == stream )
            return *iter2;

    return NULL;
}

sal_Bool XPlugin_Impl::provideNewStream(const ::rtl::OUString& mimetype,
                                    const Reference< ::com::sun::star::io::XActiveDataSource > & stream,
                                    const ::rtl::OUString& url, sal_Int32 length,
                                    sal_Int32 lastmodified, sal_Bool isfile)

{
    if( m_nProvidingState == PROVIDING_NONE )
    {
        m_nProvidingState = PROVIDING_NOW;
        Any aAny;
        aAny <<= url;
        Reference< ::com::sun::star::beans::XPropertySet >  xPS( m_xModel, UNO_QUERY );
        xPS->setPropertyValue( ::rtl::OUString::createFromAscii( "URL" ), aAny );
    }

    m_nProvidingState = PROVIDING_NOW;
    if( ! m_pPluginComm )
        loadPlugin();

    ::rtl::OString aMIME;
    if( mimetype.len() )
        aMIME = ::rtl::OUStringToOString( mimetype, m_aEncoding );
    else
        // Notnagel
        aMIME = ::rtl::OUStringToOString( m_aDescription.Mimetype, m_aEncoding );

    ::rtl::OString aURL  = ::rtl::OUStringToOString( url, m_aEncoding );

    // check wether there is a notifylistener for this stream
    // this means that the strema is created from the plugin
    // via NPN_GetURLNotify or NPN_PostURLNotify
    ::std::list<PluginEventListener*>::iterator iter;
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

    if( iter == m_aPEventListeners.end() )
    {
        // e.g. plugger.so does not like file:///
        if( ! aURL.compareTo( "file://", 7 ) )
        {
            INetURLObject aPath( url );
            aURL = ::rtl::OUStringToOString( aPath.PathToFileName(), m_aEncoding );
        }
    }

    PluginInputStream* pStream = new PluginInputStream( this, aURL.getStr(),
                                                        length, lastmodified );
    Reference< ::com::sun::star::io::XOutputStream > xNewStream( pStream );

    if( iter != m_aPEventListeners.end() )
        pStream->getStream()->notifyData = (*iter)->getNotifyData();

    uint16 stype = 0;

    // sal_False in the following statement should logically be isfile
    // but e.g. the acrobat reader plugin does not WANT a file
    // NP_ASFILE or NP_ASFILEONLY if the new stream is seekable
    // the reason for this behaviour is unknown
#ifdef DEBUG
    fprintf( stderr, "new stream \"%s\" of MIMEType \"%s\"\nfor plugin \"%s\"\n", aURL.getStr(), aMIME.getStr(), getPluginComm()->getLibName().getStr() );

#endif
    if( ! m_pPluginComm->NPP_NewStream( &m_aInstance, (char*)aMIME.getStr(),
                                        pStream->getStream(), sal_False,
                                        &stype ) )
    {
        getPluginComm()->NPP_SetWindow( getNPPInstance(), &m_aNPWindow );
#ifdef DEBUG
        char* pType;
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
            m_pPluginComm->
                NPP_StreamAsFile( &m_aInstance,
                                  pStream->getStream(),
                                  pStream->getStream()->url );
            m_nProvidingState = PROVIDING_NONE;
            return sal_True;
        }

        if( ! stream.is() )
        {
            m_pPluginComm->
                NPP_DestroyStream( &m_aInstance,
                                   pStream->getStream(),
                                   NPRES_NETWORK_ERR );
            m_nProvidingState = PROVIDING_NONE;
            throw RuntimeException();
        }
        pStream->setMode( stype );
        Reference< ::com::sun::star::io::XConnectable > xConnectable( stream, UNO_QUERY );
        pStream->setPredecessor( xConnectable );
        if( xConnectable.is() )
        {
            xConnectable->setSuccessor( static_cast< ::com::sun::star::io::XConnectable* >(pStream) );
            while( xConnectable->getPredecessor().is() )
                xConnectable = xConnectable->getPredecessor();
        }
        stream->setOutputStream( xNewStream );
        pStream->setSource( stream );
        Reference< ::com::sun::star::io::XActiveDataControl > xController;
        if( xConnectable.is() )
            xController = Reference< ::com::sun::star::io::XActiveDataControl >( xConnectable, UNO_QUERY );
        else
            xController = Reference< ::com::sun::star::io::XActiveDataControl >( stream, UNO_QUERY );

        if( xController.is() )
            xController->start();
    }

    m_nProvidingState = PROVIDING_NONE;

    return sal_False;
}

void XPlugin_Impl::disposing( const ::com::sun::star::lang::EventObject& rSource )
{
}

void XPlugin_Impl::propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& rEvent )
{
    if( ! rEvent.PropertyName.compareToAscii( "URL" ) )
    {
        ::rtl::OUString aStr;
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

void XPlugin_Impl::setPluginContext( const Reference< ::com::sun::star::plugin::XPluginContext > & rContext )
{
    m_rBrowserContext = rContext;
}

void XPlugin_Impl::setPosSize( sal_Int32 nX_, sal_Int32 nY_, sal_Int32 nWidth_, sal_Int32 nHeight_, sal_Int16 nFlags )
        throw( RuntimeException )
{
#ifdef DEBUG
    fprintf( stderr, "XPlugin_Impl::setPosSize( %d, %d, %d, %d, %d )\n",
             nX_, nY_, nWidth_, nHeight_, nFlags );
#endif

    PluginControl_Impl::setPosSize(nX_, nY_, nWidth_, nHeight_, nFlags);
    m_aNPWindow.x       = nX_;
    m_aNPWindow.y       = nY_;
    m_aNPWindow.width   = nWidth_;
    m_aNPWindow.height  = nHeight_;

    if(getPluginComm())
        getPluginComm()->NPP_SetWindow( getNPPInstance(), &m_aNPWindow );
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
    if( m_pPlugin && m_pPlugin->getPluginComm() )
    {
        m_pPlugin->getPluginComm()->NPP_DestroyStream( m_pPlugin->getNPPInstance(),
                                                       &m_aNPStream, NPRES_DONE );
        m_pPlugin->checkListeners( m_aNPStream.url );
        m_pPlugin->getPluginComm()->
            NPP_SetWindow( m_pPlugin->getNPPInstance(),
                           m_pPlugin->getNPWindow());
    }
    ::free( (void*)m_aNPStream.url );
}

PluginInputStream::PluginInputStream( XPlugin_Impl* pPlugin,
                                      const char* url,
                                      sal_uInt32 len,
                                      sal_uInt32 lastmod ) :
        PluginStream( pPlugin, url, len, lastmod ),
        m_nMode( NP_NORMAL ),
        m_nWritePos( 0 )
{
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
    String aFile( m_aFileStream.GetFileName() );
    m_aFileStream.Close();
    if( m_pPlugin )
    {
        ByteString aFileName( aFile, m_pPlugin->getTextEncoding() );
        if( m_pPlugin->getPluginComm() && m_nMode != -1 )
            // mode -1 means either an error occured,
            // or the plugin is already disposing
        {
            m_pPlugin->getPluginComm()->addFileToDelete( aFile );
            if( m_nMode == NP_ASFILE )
            {
                m_pPlugin->getPluginComm()->
                    NPP_StreamAsFile( m_pPlugin->getNPPInstance(),
                                      &m_aNPStream,
                                      aFileName.GetBuffer() );
            }
            m_pPlugin->getPluginComm()->NPP_SetWindow( m_pPlugin->getNPPInstance(), m_pPlugin->getNPWindow());
            m_pPlugin->getInputStreams().remove( this );
        }
        else
            DirEntry( m_aFileStream.GetFileName() ).Kill();
    }
    else
        DirEntry( m_aFileStream.GetFileName() ).Kill();
}

PluginStreamType PluginInputStream::getStreamType()
{
    return InputStream;
}

void PluginInputStream::setMode( sal_uInt32 nMode )
{
    m_nMode = nMode;

    // invalidation by plugin
    if( m_nMode == -1 && m_pPlugin )
    {
        m_pPlugin->getInputStreams().remove( this );
        m_pPlugin = NULL;
    }
}

void PluginInputStream::writeBytes( const Sequence<sal_Int8>& Buffer )
{
    if( m_nMode == -1 )
        return;

    m_aFileStream.Seek( STREAM_SEEK_TO_END );
    m_aFileStream.Write( Buffer.getConstArray(), Buffer.getLength() );

    int nPos = m_aFileStream.Tell();
    int nBytes = 0;
    while( m_nMode != NP_SEEK && m_nMode != NP_ASFILEONLY &&
           ( nBytes = m_pPlugin->getPluginComm()->
             NPP_WriteReady( m_pPlugin->getNPPInstance(),
                             &m_aNPStream ) ) > 0 &&
        m_nWritePos < nPos )
    {
        nBytes = nBytes > nPos - m_nWritePos ? nPos - m_nWritePos : nBytes;

        char* pBuffer = new char[ nBytes ];
        m_aFileStream.Seek( m_nWritePos );
        nBytes = m_aFileStream.Read( pBuffer, nBytes );

        int nBytesRead = 0;
        try
        {
            nBytesRead = m_pPlugin->getPluginComm()->
                NPP_Write( m_pPlugin->getNPPInstance(),
                           &m_aNPStream,
                           m_nWritePos,
                           nBytes,
                           pBuffer );
            delete pBuffer;
        }
        catch( ... )
        {
            delete pBuffer;
            return;
        }

        if( nBytesRead < 0 )
        {
            m_nMode = -1;
            return;
        }

        m_nWritePos += nBytesRead;
    }

    m_pPlugin->getPluginComm()->
        NPP_SetWindow( m_pPlugin->getNPPInstance(),
                       m_pPlugin->getNPWindow());
}

void PluginInputStream::closeOutput()
{
    flush();
    m_xSource = Reference< ::com::sun::star::io::XActiveDataSource >();
}

sal_uInt32 PluginInputStream::read( sal_uInt32 offset, sal_Int8* buffer, sal_uInt32 size )
{
    if( m_nMode != NP_SEEK )
        return 0;

    m_aFileStream.Seek( offset );
    int nBytes = m_aFileStream.Read( buffer, size );

    return nBytes;
}

void PluginInputStream::flush(void)
{
}

PluginOutputStream::PluginOutputStream( XPlugin_Impl* pPlugin,
                                        const char* url,
                                        sal_uInt32 len,
                                        sal_uInt32 lastmod ) :
        PluginStream( pPlugin, url, len, lastmod ),
        m_xStream( pPlugin->getServiceManager()->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.io.DataOutputStream" ) ), UNO_QUERY )
{
    m_pPlugin->getOutputStreams().push_back( this );
}

PluginOutputStream::~PluginOutputStream()
{
    m_pPlugin->getOutputStreams().remove( this );
}

PluginStreamType PluginOutputStream::getStreamType()
{
    return OutputStream;
}

