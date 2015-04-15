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
#ifndef INCLUDED_EXTENSIONS_SOURCE_PLUGIN_INC_PLUGIN_IMPL_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PLUGIN_INC_PLUGIN_IMPL_HXX

#include <config_lgpl.h>

#ifdef SOLARIS
#include <limits>
#endif

#include <boost/shared_ptr.hpp>

#include "cppuhelper/weak.hxx"

#include "com/sun/star/awt/Key.hpp"
#include "com/sun/star/awt/KeyFunction.hpp"
#include "com/sun/star/beans/PropertyAttribute.hpp"
#include "com/sun/star/plugin/PluginMode.hpp"
#include "com/sun/star/plugin/PluginDescription.hpp"
#include "com/sun/star/plugin/PluginException.hpp"
#include "com/sun/star/plugin/PluginVariable.hpp"
#include "com/sun/star/plugin/XPlugin.hpp"
#include "com/sun/star/plugin/XPluginManager.hpp"
#include "com/sun/star/plugin/XPluginContext.hpp"
#include "com/sun/star/io/XConnectable.hpp"
#include "com/sun/star/io/XOutputStream.hpp"
#include "com/sun/star/io/XDataOutputStream.hpp"
#include "com/sun/star/io/XActiveDataControl.hpp"
#include "com/sun/star/io/XDataInputStream.hpp"
#include "com/sun/star/io/XMarkableStream.hpp"
#include "com/sun/star/io/XInputStream.hpp"
#include "com/sun/star/io/XStreamListener.hpp"
#include "com/sun/star/io/XActiveDataSink.hpp"
#include "com/sun/star/io/XActiveDataSource.hpp"
#include "com/sun/star/lang/XServiceName.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/lang/XSingleServiceFactory.hpp"
#include "com/sun/star/awt/GradientStyle.hpp"
#include "com/sun/star/awt/RasterOperation.hpp"
#include "com/sun/star/awt/Gradient.hpp"
#include "com/sun/star/awt/XGraphics.hpp"
#include <com/sun/star/uno/XComponentContext.hpp>

#include "cppuhelper/implbase3.hxx"
#include "cppuhelper/implbase2.hxx"
#include "cppuhelper/implbase1.hxx"

#include <list>

#ifdef WNT
#include <plugin/win/sysplug.hxx>
#elif defined(MACOSX)
#include "plugin/aqua/sysplug.hxx"
#elif defined(UNX)
#include "plugin/unx/sysplug.hxx"
#endif

struct SysPlugData;

::boost::shared_ptr<SysPlugData> CreateSysPlugData();

extern "C" {

void /*SAL_CALL NP_LOADDS*/  NPN_ForceRedraw_Impl(NPP instance);
NPError /*SAL_CALL NP_LOADDS*/  NPN_SetValue_Impl( NPP instance,
                                          NPPVariable variable,
                                          void* value );
} // extern "C"

#include "plugin/plctrl.hxx"
#include "plugin/model.hxx"

#include "vcl/sysdata.hxx"
#include "vcl/syschild.hxx"

#include "tools/link.hxx"
#include "tools/stream.hxx"


using namespace com::sun::star::uno;

#define PROVIDING_NONE              0
#define PROVIDING_NOW               1
#define PROVIDING_MODEL_UPDATE      2

// forwards
namespace ucbhelper { class Content; }
class PluginStream;
class PluginInputStream;
class PluginOutputStream;
class XPlugin_Impl;
class PluginDisposer;
class PluginEventListener;

class XPlugin_Impl : public com::sun::star::plugin::XPlugin,
                     public PluginControl_Impl,
                     public com::sun::star::beans::XPropertyChangeListener
{
private:
    ::osl::Mutex                m_aMutex;
    Reference< com::sun::star::lang::XMultiServiceFactory >         m_xSMgr;
    Reference< com::sun::star::plugin::XPluginContext >             m_rBrowserContext;

    PluginComm*                 m_pPluginComm;
    NPP_t                       m_aInstance;
    NPWindow                    m_aNPWindow;
    ::boost::shared_ptr<SysPlugData> m_pSysPlugData;
    rtl_TextEncoding            m_aEncoding;

    const char**                m_pArgv;
    const char**                m_pArgn;
    int                         m_nArgs;
    OString                m_aLastGetUrl;

    Reference< com::sun::star::awt::XControlModel >             m_xModel;

    ::com::sun::star::plugin::PluginDescription         m_aDescription;
    sal_Int16                       m_aPluginMode;

    int                         m_nProvidingState;
    int                         m_nCalledFromPlugin;
    PluginDisposer*             m_pDisposer;

    ::std::list<PluginInputStream*>     m_aInputStreams;
    ::std::list<PluginOutputStream*>    m_aOutputStreams;
    ::std::list<PluginEventListener*>   m_aPEventListeners;
    OUString                     m_aURL;

    bool                        m_bIsDisposed;

#ifdef MACOSX
    void SetSysPlugDataParentView(SystemEnvData const& rEnvData);
#endif

    void prependArg( const char* pName, const char* pValue ); // arguments will be strdup'ed
    void initArgs( const Sequence< OUString >& argn,
                   const Sequence< OUString >& argv,
                   sal_Int16 mode );
    void freeArgs();
    void handleSpecialArgs();

    void loadPlugin();
    void destroyInstance();
    void modelChanged();

public:
    XPlugin_Impl( const Reference< com::sun::star::lang::XMultiServiceFactory >  & rSMgr );
    virtual ~XPlugin_Impl();

    ::osl::Mutex& getMutex() { return m_aMutex; }

    void destroyStreams();

    void setLastGetUrl( const OString& rUrl ) { m_aLastGetUrl = rUrl; }

    com::sun::star::plugin::PluginDescription fitDescription( const OUString& rURL );

    ::std::list<PluginInputStream*>& getInputStreams() { return m_aInputStreams; }
    ::std::list<PluginOutputStream*>& getOutputStreams() { return m_aOutputStreams; }
    PluginComm*     getPluginComm() { return m_pPluginComm; }
    void            setPluginComm( PluginComm* comm )
        {
            if( ! m_pPluginComm )
            {
                m_pPluginComm = comm;
                m_pPluginComm->addRef();
            }
        }
    Reference< com::sun::star::lang::XMultiServiceFactory > getServiceManager() { return m_xSMgr; }
    const com::sun::star::plugin::PluginDescription& getDescription() const { return m_aDescription; }
    rtl_TextEncoding getTextEncoding() { return m_aEncoding; }
    NPP_t&          getNPPInstance() { return m_aInstance; }
    NPWindow&       getNPWindow() { return m_aNPWindow; }
    SysPlugData&    getSysPlugData() { return *m_pSysPlugData; }

    void            enterPluginCallback() { m_nCalledFromPlugin++; }
    void            leavePluginCallback() { m_nCalledFromPlugin--; }
    bool            isDisposable() { return m_nCalledFromPlugin < 1 ? sal_True : sal_False; }
    DECL_LINK( secondLevelDispose, XPlugin_Impl* );

    void addPluginEventListener( PluginEventListener* pListener  )
        { m_aPEventListeners.push_back( pListener ); }
    void checkListeners( const char* normalizedURL );

    void            initInstance(
        const com::sun::star::plugin::PluginDescription& rDescription,
        const Sequence< OUString >& argn,
        const Sequence< OUString >& argv,
        sal_Int16 mode );
    void            initInstance(
        const OUString& rURL,
        const Sequence< OUString >& argn,
        const Sequence< OUString >& argv,
        sal_Int16 mode );

    const OUString&    getRefererURL() { return m_aURL; }
    OUString getCreationURL();

    PluginStream* getStreamFromNPStream( NPStream* );

    const SystemEnvData* getSysChildSysData()
        { return _pSysChild->GetSystemData(); }

    const Reference< com::sun::star::plugin::XPluginContext > & getPluginContext() const
        { return m_rBrowserContext; }
    void setPluginContext( const Reference< com::sun::star::plugin::XPluginContext > & );

    void secondLevelDispose();

//  static const Reference< com::sun::star::reflection::XIdlClass > &   staticGetIdlClass();

    // XInterface
    virtual Any SAL_CALL queryInterface( const Type& ) throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL acquire()  throw() SAL_OVERRIDE
    { OWeakAggObject::acquire(); }
    virtual void SAL_CALL release()  throw() SAL_OVERRIDE
    { OWeakAggObject::release(); }

    // OWeakAggObject
    virtual Any SAL_CALL queryAggregation( const Type& )
        throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // PluginContol_Impl
    virtual void SAL_CALL dispose() throw(std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL createPeer( const Reference< com::sun::star::awt::XToolkit > & xToolkit, const Reference< com::sun::star::awt::XWindowPeer > & Parent) throw( RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL setModel( const Reference< com::sun::star::awt::XControlModel > & Model ) throw( RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual Reference< com::sun::star::awt::XControlModel > SAL_CALL getModel()throw( RuntimeException, std::exception ) SAL_OVERRIDE
    { return m_xModel; }

    virtual void SAL_CALL setPosSize( sal_Int32 nX_, sal_Int32 nY_, sal_Int32 nWidth_, sal_Int32 nHeight_, sal_Int16 nFlags ) throw( RuntimeException, std::exception ) SAL_OVERRIDE;

    // com::sun::star::plugin::XPlugin
    virtual sal_Bool SAL_CALL provideNewStream(const OUString& mimetype, const Reference< com::sun::star::io::XActiveDataSource > & stream, const OUString& url, sal_Int32 length, sal_Int32 lastmodified, sal_Bool isfile) throw(std::exception) SAL_OVERRIDE;

    // com::sun::star::beans::XPropertyChangeListener
    virtual void SAL_CALL disposing( const com::sun::star::lang::EventObject& rSource ) throw(std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL propertyChange(const com::sun::star::beans::PropertyChangeEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

class PluginManager
{
private:
    Reference< com::sun::star::lang::XMultiServiceFactory >         m_xSMgr;
    ::std::list<PluginComm*>        m_aPluginComms;
    ::std::list<XPlugin_Impl*>      m_aAllPlugins;
    ::osl::Mutex                    m_aPluginMutex;

    static PluginManager*       pManager;

    PluginManager();
public:

    static PluginManager& get();
    static void setServiceFactory( const Reference< com::sun::star::lang::XMultiServiceFactory >& xFactory );
    static const Sequence< OUString >& getAdditionalSearchPaths();

    ::std::list<PluginComm*>& getPluginComms() { return m_aPluginComms; }
    ::std::list<XPlugin_Impl*>& getPlugins() { return m_aAllPlugins; }
    ::osl::Mutex& getPluginMutex() { return m_aPluginMutex; }
};

class XPluginManager_Impl :
    public cppu::WeakAggImplHelper2< com::sun::star::plugin::XPluginManager,
                                    com::sun::star::lang::XServiceInfo >
{
    Reference< com::sun::star::uno::XComponentContext >     m_xContext;
public:
    XPluginManager_Impl( const Reference< com::sun::star::uno::XComponentContext >  & );
    virtual ~XPluginManager_Impl();

    static XPlugin_Impl* getXPluginFromNPP( NPP );
    static XPlugin_Impl* getPluginImplementation( const Reference< com::sun::star::plugin::XPlugin >& plugin );

    virtual Reference< com::sun::star::plugin::XPluginContext > SAL_CALL createPluginContext() throw(std::exception) SAL_OVERRIDE;

    // has to be implemented per system
    Sequence< com::sun::star::plugin::PluginDescription > SAL_CALL impl_getPluginDescriptions() throw(css::uno::RuntimeException, std::exception);
    // calls system specific impl_getPluginDescriptions
    // checks whether plugins are disabled
    virtual Sequence< com::sun::star::plugin::PluginDescription > SAL_CALL getPluginDescriptions()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual Reference< com::sun::star::plugin::XPlugin > SAL_CALL createPlugin( const Reference< com::sun::star::plugin::XPluginContext > & acontext, sal_Int16 mode, const Sequence< OUString >& argn, const Sequence< OUString >& argv, const com::sun::star::plugin::PluginDescription& plugintype) throw( RuntimeException,::com::sun::star::plugin::PluginException, std::exception ) SAL_OVERRIDE;

    virtual Reference< com::sun::star::plugin::XPlugin > SAL_CALL createPluginFromURL( const Reference< com::sun::star::plugin::XPluginContext > & acontext, sal_Int16 mode, const Sequence< OUString >& argn, const Sequence< OUString >& argv, const Reference< com::sun::star::awt::XToolkit > & toolkit, const Reference< com::sun::star::awt::XWindowPeer > & parent, const OUString& url ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw() SAL_OVERRIDE;
    virtual OUString SAL_CALL getImplementationName() throw() SAL_OVERRIDE;

    Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(  ) SAL_OVERRIDE;
    static Sequence< OUString > getSupportedServiceNames_Static() throw(  );
    static OUString                getImplementationName_Static() throw(  )
    {
        /** the soplayer uses this name in its source! maybe not after 5.2 */
        return OUString( "com.sun.star.extensions.PluginManager" );
    }
};
Reference< XInterface >  SAL_CALL PluginManager_CreateInstance( const Reference< com::sun::star::lang::XMultiServiceFactory >  & ) throw( Exception );

enum PluginStreamType { InputStream, OutputStream };

class PluginStream
{
protected:
    css::uno::WeakReference<css::uno::XInterface> m_wPlugin;
    XPlugin_Impl*       m_pPlugin;
    NPStream            m_aNPStream;
public:
    PluginStream( XPlugin_Impl* pPlugin,
                   const char* url, sal_uInt32 len, sal_uInt32 lastmod );
    virtual ~PluginStream();

    NPStream& getStream() { return m_aNPStream; }
    XPlugin_Impl* getPlugin() { return m_pPlugin; }

    virtual PluginStreamType getStreamType() = 0;
};

class PluginInputStream :
    public PluginStream,
    public cppu::WeakAggImplHelper2<
                ::com::sun::star::io::XOutputStream,
                ::com::sun::star::io::XConnectable
                >
{
private:
    ::ucbhelper::Content*       m_pContent;
    sal_Int32                   m_nMode;
    sal_uInt32                      m_nWritePos;

    Reference< com::sun::star::io::XActiveDataSource >  m_xSource;
    // hold a reference on input until closeOutput is called

    Reference< com::sun::star::io::XConnectable >           m_xPredecessor;
    Reference< com::sun::star::io::XConnectable >           m_xSuccessor;

    // needed to hold a reference to self in NP_SEEK mode
    Reference< com::sun::star::io::XOutputStream >          m_xSelf;

    SvFileStream                m_aFileStream;
public:
    PluginInputStream( XPlugin_Impl* pPlugin,
                   const char* url, sal_uInt32 len, sal_uInt32 lastmod );

    PluginInputStream() : PluginStream( NULL, NULL, 0, 0 ) {}

    virtual ~PluginInputStream();

    virtual PluginStreamType getStreamType() SAL_OVERRIDE;

    void setMode( sal_Int32 nMode );
    sal_uInt32 read( sal_uInt32 offset, sal_Int8* buffer, sal_uInt32 size );
    void setSource( const Reference< com::sun::star::io::XActiveDataSource >& xSource ) { m_xSource = xSource; }
    // get contents ot url via ucbhelper::Content
    void load();

    // clear reference
    bool releaseSelf()
    { bool bRet = m_xSelf.is(); m_xSelf.clear();  return bRet; }

    // XOutputStream
    virtual void SAL_CALL writeBytes( const Sequence<sal_Int8>& ) throw(std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL flush() throw(std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL closeOutput() throw(std::exception) SAL_OVERRIDE;

    // XConnectable
    virtual void SAL_CALL setPredecessor( const Reference< com::sun::star::io::XConnectable >& xPredecessor ) throw(std::exception) SAL_OVERRIDE
        { m_xPredecessor = xPredecessor; }
    virtual Reference< com::sun::star::io::XConnectable > SAL_CALL getPredecessor() throw(std::exception) SAL_OVERRIDE
        { return m_xPredecessor; }

    virtual void SAL_CALL setSuccessor( const Reference< com::sun::star::io::XConnectable >& xSuccessor ) throw(std::exception) SAL_OVERRIDE
        { m_xSuccessor = xSuccessor; }
    virtual Reference< com::sun::star::io::XConnectable > SAL_CALL getSuccessor() throw(std::exception) SAL_OVERRIDE
        { return m_xSuccessor; }
};

class PluginOutputStream : public PluginStream
{
private:
    Reference< com::sun::star::io::XOutputStream >  m_xStream;
public:
    PluginOutputStream( XPlugin_Impl* pPlugin, const char* url,
                        sal_uInt32 len, sal_uInt32 lastmod );
    virtual ~PluginOutputStream();

    virtual PluginStreamType getStreamType() SAL_OVERRIDE;

    Reference< com::sun::star::io::XOutputStream > & getOutputStream() { return m_xStream; }
};

class PluginEventListener :
    public cppu::WeakAggImplHelper1< com::sun::star::lang::XEventListener >
{
private:
    XPlugin_Impl*   m_pPlugin;
    Reference< com::sun::star::plugin::XPlugin >        m_xPlugin; // just to hold the plugin
    char*           m_pUrl;
    char*           m_pNormalizedUrl;
    void*           m_pNotifyData;
public:
    PluginEventListener( XPlugin_Impl*,
                         const char* url,
                         const char* normurl,
                         void*  notifyData );
    virtual ~PluginEventListener();

    const char* getURL() { return m_pUrl; }
    const char* getNormalizedURL() { return m_pNormalizedUrl; }
    void*       getNotifyData() { return m_pNotifyData; }

    // com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing( const com::sun::star::lang::EventObject&  Source ) throw(std::exception) SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
