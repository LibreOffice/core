/*************************************************************************
 *
 *  $RCSfile: impl.hxx,v $
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
#ifndef __PLUGIN_SOURCE_MGR_IMPL_HXX
#define __PLUGIN_SOURCE_MGR_IMPL_HXX

#ifdef SOLARIS
#include <limits>
#endif

#include <cppuhelper/weak.hxx>

#ifndef _COM_SUN_STAR_AWT_KEY_HPP_
#include <com/sun/star/awt/Key.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_KEYFUNCTION_HPP_
#include <com/sun/star/awt/KeyFunction.hpp>
#endif
#ifndef _COM_SUN_STAR_PLUGIN_PLUGINMODE_HPP_
#include <com/sun/star/plugin/PluginMode.hpp>
#endif
#ifndef _COM_SUN_STAR_PLUGIN_PLUGINDESCRIPTION_HPP_
#include <com/sun/star/plugin/PluginDescription.hpp>
#endif
#ifndef _COM_SUN_STAR_PLUGIN_PLUGINEXCEPTION_HPP_
#include <com/sun/star/plugin/PluginException.hpp>
#endif
#ifndef _COM_SUN_STAR_PLUGIN_PLUGINVARIABLE_HPP_
#include <com/sun/star/plugin/PluginVariable.hpp>
#endif
#ifndef _COM_SUN_STAR_PLUGIN_XPLUGIN_HPP_
#include <com/sun/star/plugin/XPlugin.hpp>
#endif
#ifndef _COM_SUN_STAR_PLUGIN_XPLUGINMANAGER_HPP_
#include <com/sun/star/plugin/XPluginManager.hpp>
#endif
#ifndef _COM_SUN_STAR_PLUGIN_XPLUGINCONTEXT_HPP_
#include <com/sun/star/plugin/XPluginContext.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XCONNECTABLE_HPP_
#include <com/sun/star/io/XConnectable.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XDATAOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XDataOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATACONTROL_HPP_
#include <com/sun/star/io/XActiveDataControl.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XDATAINPUTSTREAM_HPP_
#include <com/sun/star/io/XDataInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XMARKABLESTREAM_HPP_
#include <com/sun/star/io/XMarkableStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSTREAMLISTENER_HPP_
#include <com/sun/star/io/XStreamListener.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICENAME_HPP_
#include <com/sun/star/lang/XServiceName.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_GRADIENTSTYLE_HPP_
#include <com/sun/star/awt/GradientStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_RASTEROPERATION_HPP_
#include <com/sun/star/awt/RasterOperation.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_GRADIENT_HPP_
#include <com/sun/star/awt/Gradient.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XGRAPHICS_HPP_
#include <com/sun/star/awt/XGraphics.hpp>
#endif

#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase1.hxx>

#ifdef WNT
#include <plugin/win/sysplug.hxx>
#else
#include <list>
#endif

#ifdef OS2
#include <plugin/os2/sysplug.hxx>
#endif

#ifdef UNX
#include <plugin/unx/sysplug.hxx>
#endif

#include <vcl/sysdata.hxx>
#include <vcl/syschild.hxx>
#include <plugin/plctrl.hxx>
#include <plugin/model.hxx>
#include <tools/link.hxx>
#include <tools/stream.hxx>

using namespace com::sun::star::uno;

#define PROVIDING_NONE              0
#define PROVIDING_NOW               1
#define PROVIDING_MODEL_UPDATE      2

class PluginStream;
class PluginInputStream;
class PluginOutputStream;
class XPlugin_Impl;
class PluginDisposer;
class PluginEventListener;

class XPlugin_Impl : public ::com::sun::star::plugin::XPlugin,
                     public PluginControl_Impl,
                     public ::com::sun::star::beans::XPropertyChangeListener
{
private:
    Reference< ::com::sun::star::lang::XMultiServiceFactory >       m_xSMgr;
    Reference< ::com::sun::star::plugin::XPluginContext >           m_rBrowserContext;

    PluginComm*                 m_pPluginComm;
    NPP_t                       m_aInstance;
    NPWindow                    m_aNPWindow;
    rtl_TextEncoding            m_aEncoding;
#ifdef UNX
    NPSetWindowCallbackStruct   m_aWSInfo;
    Display*                    m_pDisplay;
    XtAppContext                m_aAppContext;
#endif

    const char**                m_pArgv;
    const char**                m_pArgn;
    int                         m_nArgs;

    Reference< ::com::sun::star::awt::XControlModel >           m_xModel;

    ::com::sun::star::plugin::PluginDescription         m_aDescription;
    sal_Int16                       m_aPluginMode;

    int                         m_nProvidingState;
    int                         m_nCalledFromPlugin;
    PluginDisposer*             m_pDisposer;

    ::std::list<PluginInputStream*>     m_aInputStreams;
    ::std::list<PluginOutputStream*>    m_aOutputStreams;
    ::std::list<PluginEventListener*>   m_aPEventListeners;
    ::rtl::OUString                     m_aURL;

    sal_Bool                        m_bIsDisposed;

    void init( const ::com::sun::star::plugin::PluginDescription& rDescription );
    void loadPlugin();
    void destroyInstance();
    void modelChanged();

public:
    XPlugin_Impl( const Reference< ::com::sun::star::lang::XMultiServiceFactory >  & rSMgr );
    virtual ~XPlugin_Impl();

    void destroyStreams();

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
#ifdef UNX
    Display*        getAppDisplay() { return m_pDisplay; }
    XtAppContext    getAppContext() { return m_aAppContext; }
#endif
    Reference< ::com::sun::star::lang::XMultiServiceFactory > getServiceManager() { return m_xSMgr; }
    rtl_TextEncoding getTextEncoding() { return m_aEncoding; }
    NPP             getNPPInstance() { return &m_aInstance; }
    NPWindow*       getNPWindow() { return &m_aNPWindow; }

    void            enterPluginCallback() { m_nCalledFromPlugin++; }
    void            leavePluginCallback() { m_nCalledFromPlugin--; }
    sal_Bool            isDisposable() { return m_nCalledFromPlugin < 1 ? sal_True : sal_False; }
    DECL_LINK( secondLevelDispose, XPlugin_Impl* );

    void addPluginEventListener( PluginEventListener* pListener  )
        { m_aPEventListeners.push_back( pListener ); }
    void checkListeners( const char* normalizedURL );

    void            initInstance(
        const ::com::sun::star::plugin::PluginDescription& rDescription,
        const Sequence< ::rtl::OUString >& argn,
        const Sequence< ::rtl::OUString >& argv,
        sal_Int16 mode );

    const ::rtl::OUString&  getRefererURL() { return m_aURL; }
    ::rtl::OUString getCreationURL();

    PluginStream* getStreamFromNPStream( NPStream* );

    const SystemEnvData* getSysChildSysData()
        { return _pSysChild->GetSystemData(); }

    const Reference< ::com::sun::star::plugin::XPluginContext > & getPluginContext() const
        { return m_rBrowserContext; }
    void setPluginContext( const Reference< ::com::sun::star::plugin::XPluginContext > & );

    void secondLevelDispose();

//  static const Reference< ::com::sun::star::reflection::XIdlClass > & staticGetIdlClass();

    // XInterface
    virtual Any SAL_CALL queryInterface( const Type& );
    virtual void SAL_CALL acquire() { OWeakAggObject::acquire(); }
    virtual void SAL_CALL release() { OWeakAggObject::release(); }

    // OWeakAggObject
    virtual Any SAL_CALL queryAggregation( const Type& );

    // PluginContol_Impl
    virtual void SAL_CALL dispose();
    virtual void SAL_CALL createPeer( const Reference< ::com::sun::star::awt::XToolkit > & xToolkit, const Reference< ::com::sun::star::awt::XWindowPeer > & Parent) throw( RuntimeException );

    virtual sal_Bool SAL_CALL setModel( const Reference< ::com::sun::star::awt::XControlModel > & Model ) throw( RuntimeException );
    virtual Reference< ::com::sun::star::awt::XControlModel > SAL_CALL getModel(void) const throw( RuntimeException )
    { return m_xModel; }

    virtual void SAL_CALL setPosSize( sal_Int32 nX_, sal_Int32 nY_, sal_Int32 nWidth_, sal_Int32 nHeight_, sal_Int16 nFlags ) throw( RuntimeException );

    // ::com::sun::star::plugin::XPlugin
    virtual sal_Bool SAL_CALL provideNewStream(const ::rtl::OUString& mimetype, const Reference< ::com::sun::star::io::XActiveDataSource > & stream, const ::rtl::OUString& url, sal_Int32 length, sal_Int32 lastmodified, sal_Bool isfile);

    // ::com::sun::star::beans::XPropertyChangeListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& rSource );
    virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& rEvent );
};

class PluginManager
{
private:
    Reference< ::com::sun::star::lang::XMultiServiceFactory >       m_xSMgr;
    ::std::list<PluginComm*>        m_aPluginComms;
    ::std::list<XPlugin_Impl*>      m_aAllPlugins;
    ::osl::Mutex                    m_aPluginMutex;

    static PluginManager*       pManager;

    PluginManager();
    ~PluginManager();
public:

    static PluginManager& get();
    static void setServiceFactory( const Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory );
    static const Sequence< ::rtl::OUString >& getAdditionalSearchPaths();

    ::std::list<PluginComm*>& getPluginComms() { return m_aPluginComms; }
    ::std::list<XPlugin_Impl*>& getPlugins() { return m_aAllPlugins; }
    ::osl::Mutex& getPluginMutex() { return m_aPluginMutex; }
};

class XPluginManager_Impl :
    public ::cppu::WeakAggImplHelper1< ::com::sun::star::plugin::XPluginManager >
{
    Reference< ::com::sun::star::lang::XMultiServiceFactory >   m_xSMgr;
public:
    XPluginManager_Impl( const Reference< ::com::sun::star::lang::XMultiServiceFactory >  & );
    virtual ~XPluginManager_Impl();

    static XPlugin_Impl* getXPluginFromNPP( NPP );
    static XPlugin_Impl* getFirstXPlugin();
    static XPlugin_Impl* getPluginImplementation( const Reference< ::com::sun::star::plugin::XPlugin >& plugin );

    virtual Reference< ::com::sun::star::plugin::XPluginContext > SAL_CALL createPluginContext();

    // has to be implemented per system
    virtual Sequence< ::com::sun::star::plugin::PluginDescription > SAL_CALL getPluginDescriptions(void) throw(  );

    virtual Reference< ::com::sun::star::plugin::XPlugin > SAL_CALL createPlugin( const Reference< ::com::sun::star::plugin::XPluginContext > & acontext, sal_Int16 mode, const Sequence< ::rtl::OUString >& argn, const Sequence< ::rtl::OUString >& argv, const ::com::sun::star::plugin::PluginDescription& plugintype) throw( RuntimeException,::com::sun::star::plugin::PluginException );

    virtual Reference< ::com::sun::star::plugin::XPlugin > SAL_CALL createPluginFromURL( const Reference< ::com::sun::star::plugin::XPluginContext > & acontext, sal_Int16 mode, const Sequence< ::rtl::OUString >& argn, const Sequence< ::rtl::OUString >& argv, const Reference< ::com::sun::star::awt::XToolkit > & toolkit, const Reference< ::com::sun::star::awt::XWindowPeer > & parent, const ::rtl::OUString& url );

    virtual sal_Bool SAL_CALL supportsService(const ::rtl::OUString& ServiceName) throw(  );
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw();

    Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw(  );
    static Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw(  );
    static ::rtl::OUString              getImplementationName_Static() throw(  )
    {
        /** the soplayer uses this name in its source! maybe not after 5.2 */
        return ::rtl::OUString::createFromAscii( "com.sun.star.extensions.PluginManager" );
    }
};
Reference< XInterface >  SAL_CALL PluginManager_CreateInstance( const Reference< ::com::sun::star::lang::XMultiServiceFactory >  & ) throw( Exception );

enum PluginStreamType { InputStream, OutputStream };

class PluginStream
{
protected:
    XPlugin_Impl*       m_pPlugin;
    NPStream            m_aNPStream;
public:
    PluginStream( XPlugin_Impl* pPlugin,
                   const char* url, sal_uInt32 len, sal_uInt32 lastmod );
    virtual ~PluginStream();

    NPStream* getStream() { return &m_aNPStream; }
    XPlugin_Impl* getPlugin() { return m_pPlugin; }

    virtual PluginStreamType getStreamType() = 0;
};

class PluginInputStream :
    public PluginStream,
    public ::cppu::WeakAggImplHelper2<
                ::com::sun::star::io::XOutputStream,
                ::com::sun::star::io::XConnectable
                >
{
private:
    UINT32                      m_nMode;
    UINT32                      m_nWritePos;

    Reference< ::com::sun::star::io::XActiveDataSource >    m_xSource;
    // hold a reference on input until closeOutput is called

    Reference< ::com::sun::star::io::XConnectable >         m_xPredecessor;
    Reference< ::com::sun::star::io::XConnectable >         m_xSuccessor;

    SvFileStream                m_aFileStream;
public:
    PluginInputStream( XPlugin_Impl* pPlugin,
                   const char* url, UINT32 len, UINT32 lastmod );

    PluginInputStream() : PluginStream( NULL, NULL, 0, 0 ) {}

    virtual ~PluginInputStream();

    virtual PluginStreamType getStreamType();

    void setMode( UINT32 nMode );
    UINT32 read( UINT32 offset, sal_Int8* buffer, UINT32 size );
    void setSource( const Reference< ::com::sun::star::io::XActiveDataSource >& xSource ) { m_xSource = xSource; }

    // XOutputStream
    virtual void SAL_CALL writeBytes( const Sequence<sal_Int8>& );
    virtual void SAL_CALL flush();
    virtual void SAL_CALL closeOutput();

    // XConnectable
    virtual void SAL_CALL setPredecessor( const Reference< ::com::sun::star::io::XConnectable >& xPredecessor )
        { m_xPredecessor = xPredecessor; }
    virtual Reference< ::com::sun::star::io::XConnectable > SAL_CALL getPredecessor()
        { return m_xPredecessor; }

    virtual void SAL_CALL setSuccessor( const Reference< ::com::sun::star::io::XConnectable >& xSuccessor )
        { m_xSuccessor = xSuccessor; }
    virtual Reference< ::com::sun::star::io::XConnectable > SAL_CALL getSuccessor()
        { return m_xSuccessor; }
};

class PluginOutputStream : public PluginStream
{
private:
    Reference< ::com::sun::star::io::XOutputStream >    m_xStream;
public:
    PluginOutputStream( XPlugin_Impl* pPlugin, const char* url,
                        sal_uInt32 len, sal_uInt32 lastmod );
    virtual ~PluginOutputStream();

    virtual PluginStreamType getStreamType();

    Reference< ::com::sun::star::io::XOutputStream > & getOutputStream() { return m_xStream; }
};

class PluginEventListener :
    public ::cppu::WeakAggImplHelper1< ::com::sun::star::lang::XEventListener >
{
private:
    XPlugin_Impl*   m_pPlugin;
    Reference< ::com::sun::star::plugin::XPlugin >      m_xPlugin; // just to hold the plugin
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

    // ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject&  Source );
};

#endif
