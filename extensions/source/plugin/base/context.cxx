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

#include <cstdarg>
#include <cstdio>

#include <plugin/impl.hxx>

#include <sal/log.hxx>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <tools/urlobj.hxx>
#include <osl/file.hxx>

#include <cppuhelper/implbase1.hxx>

using namespace com::sun::star::io;
using namespace com::sun::star::frame;

namespace ext_plug {

class FileSink : public ::cppu::WeakAggImplHelper1< ::com::sun::star::io::XOutputStream >
{
private:
    Reference< ::com::sun::star::uno::XComponentContext >   m_xContext;
    FILE*                   fp;
    Reference< ::com::sun::star::plugin::XPlugin >          m_xPlugin;
    OUString                 m_aMIMEType;
    OUString                 m_aTarget;
    OUString                 m_aFileName;

public:
    FileSink( const Reference< ::com::sun::star::uno::XComponentContext > &,
              const Reference< ::com::sun::star::plugin::XPlugin > & plugin,
              const OUString& mimetype,
              const OUString& target,
              const Reference< ::com::sun::star::io::XActiveDataSource > & source );
    virtual ~FileSink();

    // ::com::sun::star::io::XOutputStream
    virtual void SAL_CALL writeBytes( const Sequence<sal_Int8>& ) throw();
    virtual void SAL_CALL flush() throw();
    virtual void SAL_CALL closeOutput() throw();
};

}
using namespace ext_plug;

class XPluginContext_Impl : public ::cppu::WeakAggImplHelper1< ::com::sun::star::plugin::XPluginContext >
{
    Reference< ::com::sun::star::uno::XComponentContext >   m_xContext;
    rtl_TextEncoding                                        m_aEncoding;
public:

    XPluginContext_Impl( const Reference< ::com::sun::star::uno::XComponentContext >  & );
    virtual ~XPluginContext_Impl();


    virtual OUString SAL_CALL getValue(const Reference< ::com::sun::star::plugin::XPlugin > & plugin, ::com::sun::star::plugin::PluginVariable variable) throw( ::com::sun::star::plugin::PluginException, RuntimeException );
    virtual void SAL_CALL getURLNotify(const Reference< ::com::sun::star::plugin::XPlugin > & plugin, const OUString& url, const OUString& target, const Reference< ::com::sun::star::lang::XEventListener > & listener) throw( ::com::sun::star::plugin::PluginException, RuntimeException );
    virtual void SAL_CALL getURL(const Reference< ::com::sun::star::plugin::XPlugin > & plugin, const OUString& url, const OUString& target) throw( ::com::sun::star::plugin::PluginException, RuntimeException );
    virtual void SAL_CALL postURLNotify(const Reference< ::com::sun::star::plugin::XPlugin > & plugin, const OUString& url, const OUString& target, const Sequence< sal_Int8 >& buf, sal_Bool file, const Reference< ::com::sun::star::lang::XEventListener > & listener) throw( ::com::sun::star::plugin::PluginException, RuntimeException );
    virtual void SAL_CALL postURL(const Reference< ::com::sun::star::plugin::XPlugin > & plugin, const OUString& url, const OUString& target, const Sequence< sal_Int8 >& buf, sal_Bool file) throw( ::com::sun::star::plugin::PluginException, RuntimeException );
    virtual void SAL_CALL newStream(const Reference< ::com::sun::star::plugin::XPlugin > & plugin, const OUString& mimetype, const OUString& target, const Reference< ::com::sun::star::io::XActiveDataSource > & source) throw( ::com::sun::star::plugin::PluginException, RuntimeException );
    virtual void SAL_CALL displayStatusText(const Reference< ::com::sun::star::plugin::XPlugin > & plugin, const OUString& message) throw( ::com::sun::star::plugin::PluginException, RuntimeException );
    virtual OUString SAL_CALL getUserAgent(const Reference< ::com::sun::star::plugin::XPlugin > & plugin) throw( ::com::sun::star::plugin::PluginException, RuntimeException );
};

Reference< ::com::sun::star::plugin::XPluginContext >  XPluginManager_Impl::createPluginContext() throw()
{
    return new XPluginContext_Impl( m_xContext );
}

XPluginContext_Impl::XPluginContext_Impl( const Reference< ::com::sun::star::uno::XComponentContext >  & rxContext )
    : m_xContext( rxContext ),
      m_aEncoding( osl_getThreadTextEncoding() )
{
}

XPluginContext_Impl::~XPluginContext_Impl()
{
}

OUString XPluginContext_Impl::getValue( const Reference< ::com::sun::star::plugin::XPlugin > & /*plugin*/, ::com::sun::star::plugin::PluginVariable /*variable*/ )
    throw( ::com::sun::star::plugin::PluginException, RuntimeException )
{
    return OUString();
}


void XPluginContext_Impl::getURL(const Reference< ::com::sun::star::plugin::XPlugin > & plugin, const OUString& url, const OUString& target) throw( ::com::sun::star::plugin::PluginException, RuntimeException )
{
    Reference< XDesktop2 > xDesktop = Desktop::create(m_xContext);

    if(  target.isEmpty() )
    {
        INetURLObject aURL;
        aURL.SetSmartProtocol( INET_PROT_FILE );
        aURL.SetSmartURL( url );

        OUString aUrl = aURL.GetMainURL(INetURLObject::DECODE_TO_IURI);
        // the mimetype cannot be specified
        plugin->provideNewStream( OUString(),
                                  Reference< XActiveDataSource >(),
                                  aUrl,
                                  0, 0, aUrl.startsWith("file:") );
        return;
    }

    XPlugin_Impl* pPlugin = XPluginManager_Impl::getPluginImplementation( plugin );

    if( pPlugin )
    {
        try
        {
            ::com::sun::star::beans::PropertyValue aValue;
            aValue.Name     = OUString("Referer");
            aValue.Value <<= pPlugin->getRefererURL();

            Sequence< ::com::sun::star::beans::PropertyValue > aArgs( &aValue, 1 );
            Reference< ::com::sun::star::lang::XComponent >  xComp =
                xDesktop->loadComponentFromURL(
                                              url,
                                              target,
                                              ::com::sun::star::frame::FrameSearchFlag::PARENT          |
                                              ::com::sun::star::frame::FrameSearchFlag::SELF            |
                                              ::com::sun::star::frame::FrameSearchFlag::CHILDREN        |
                                              ::com::sun::star::frame::FrameSearchFlag::SIBLINGS        |
                                              ::com::sun::star::frame::FrameSearchFlag::TASKS           |
                                              ::com::sun::star::frame::FrameSearchFlag::CREATE,
                                              aArgs
                                              );
        }
        catch(...)
        {
            throw ::com::sun::star::plugin::PluginException();
        }
    }
}

void XPluginContext_Impl::getURLNotify(const Reference< ::com::sun::star::plugin::XPlugin > & plugin, const OUString& url, const OUString& target, const Reference< ::com::sun::star::lang::XEventListener > & listener )
    throw( ::com::sun::star::plugin::PluginException, RuntimeException )
{
    getURL( plugin, url, target );
    if( listener.is() )
        listener->disposing( ::com::sun::star::lang::EventObject() );
}

OUString XPluginContext_Impl::getUserAgent(const Reference< ::com::sun::star::plugin::XPlugin > & /*plugin*/)
    throw( ::com::sun::star::plugin::PluginException, RuntimeException )
{
    return OUString("Mozilla 3.0");
}

void XPluginContext_Impl::displayStatusText(const Reference< ::com::sun::star::plugin::XPlugin > & /*plugin*/, const OUString& /*message*/)
    throw( ::com::sun::star::plugin::PluginException, RuntimeException )
{
}

void XPluginContext_Impl::postURL(const Reference< ::com::sun::star::plugin::XPlugin > & plugin, const OUString& url, const OUString& target, const Sequence< sal_Int8 >& buf, sal_Bool file)
    throw( ::com::sun::star::plugin::PluginException, RuntimeException )
{
    Sequence< sal_Int8 > aBuf;

    if( file )
    {
        OUString aFileName( (char*)buf.getConstArray(), strlen((char*)buf.getConstArray()), m_aEncoding );
        INetURLObject aFilePath( aFileName );
        aFileName = aFilePath.PathToFileName();
        SvFileStream aStream( aFileName, STREAM_READ );
        if( aStream.IsOpen() )
        {
            int nBytes = 0;
            aStream.Seek( STREAM_SEEK_TO_END );
            aBuf = Sequence<sal_Int8>( nBytes = aStream.Tell() );
            aStream.Seek( STREAM_SEEK_TO_BEGIN );
            aStream.Read( aBuf.getArray(), nBytes );
            aStream.Close();
            osl::FileBase::getFileURLFromSystemPath( aFileName, aFileName );
            osl::File::remove( aFileName );
        }
    }

    Reference< XDesktop2 > xDesktop = Desktop::create(m_xContext);

    XPlugin_Impl* pPlugin = XPluginManager_Impl::getPluginImplementation( plugin );
    if( pPlugin )
    {
        try
        {
            ::com::sun::star::beans::PropertyValue aValues[2];
            aValues[0].Name = OUString("Referer");
            aValues[0].Value <<= pPlugin->getRefererURL();

            aValues[1].Name = OUString("PostString");
            aValues[1].Value <<= OStringToOUString( (char*)( file ? aBuf : buf ).getConstArray(), m_aEncoding );
            Sequence< ::com::sun::star::beans::PropertyValue > aArgs( aValues, 2 );
            Reference< ::com::sun::star::lang::XComponent >  xComp =
                xDesktop->loadComponentFromURL(
                                              url,
                                              target,
                                              ::com::sun::star::frame::FrameSearchFlag::PARENT          |
                                              ::com::sun::star::frame::FrameSearchFlag::SELF            |
                                              ::com::sun::star::frame::FrameSearchFlag::CHILDREN        |
                                              ::com::sun::star::frame::FrameSearchFlag::SIBLINGS        |
                                              ::com::sun::star::frame::FrameSearchFlag::TASKS           |
                                              ::com::sun::star::frame::FrameSearchFlag::CREATE,
                                              aArgs
                                              );
        }
        catch( ... )
        {
            throw ::com::sun::star::plugin::PluginException();
        }
    }
}

void XPluginContext_Impl::postURLNotify(const Reference< ::com::sun::star::plugin::XPlugin > & plugin, const OUString& url, const OUString& target, const Sequence< sal_Int8 >& buf, sal_Bool file, const Reference< ::com::sun::star::lang::XEventListener > & listener )
    throw( ::com::sun::star::plugin::PluginException, RuntimeException )
{
    postURL( plugin, url, target, buf, file );
    if( listener.is() )
        listener->disposing( ::com::sun::star::lang::EventObject() );
}

void XPluginContext_Impl::newStream( const Reference< ::com::sun::star::plugin::XPlugin > & plugin, const OUString& mimetype, const OUString& target, const Reference< ::com::sun::star::io::XActiveDataSource > & source )
    throw( ::com::sun::star::plugin::PluginException, RuntimeException )
{
    FileSink*  pNewSink = new FileSink( m_xContext, plugin, mimetype, target, source );
    pNewSink->acquire();
}



FileSink::FileSink( const Reference< ::com::sun::star::uno::XComponentContext >  & rxContext, const Reference< ::com::sun::star::plugin::XPlugin > & plugin,
                    const OUString& mimetype,
                    const OUString& target, const Reference< ::com::sun::star::io::XActiveDataSource > & source ) :
        m_xContext( rxContext ),
        m_xPlugin( plugin ),
        m_aMIMEType( mimetype ),
        m_aTarget( target )
{
    osl::FileBase::createTempFile( 0, 0, &m_aFileName );
    OString aFile = OUStringToOString( m_aFileName, osl_getThreadTextEncoding() );
    fp = fopen( aFile.getStr() , "wb" );

    Reference< ::com::sun::star::io::XActiveDataControl >  xControl( source, UNO_QUERY );

    source->setOutputStream( Reference< ::com::sun::star::io::XOutputStream > ( this ) );
    if( xControl.is() )
        xControl->start();
}

FileSink::~FileSink()
{
    osl::File::remove( m_aFileName );
}

void FileSink::closeOutput() throw()
{
    if( fp )
        fclose( fp );

    Reference< XDesktop2 > xDesktop = Desktop::create(m_xContext);
    XPlugin_Impl* pPlugin = XPluginManager_Impl::getPluginImplementation( m_xPlugin );

    if( pPlugin )
    {
        try
        {
            ::com::sun::star::beans::PropertyValue aValue;
            aValue.Name = OUString("Referer");
            aValue.Value <<= pPlugin->getRefererURL();

            Sequence< ::com::sun::star::beans::PropertyValue > aArgs( &aValue, 1 );
            Reference< ::com::sun::star::lang::XComponent >  xComp =
                xDesktop->loadComponentFromURL(
                                              m_aFileName,
                                              m_aTarget,
                                              ::com::sun::star::frame::FrameSearchFlag::PARENT          |
                                              ::com::sun::star::frame::FrameSearchFlag::SELF            |
                                              ::com::sun::star::frame::FrameSearchFlag::CHILDREN        |
                                              ::com::sun::star::frame::FrameSearchFlag::SIBLINGS        |
                                              ::com::sun::star::frame::FrameSearchFlag::TASKS           |
                                              ::com::sun::star::frame::FrameSearchFlag::CREATE,
                                              aArgs
                                              );
        }
        catch( ... )
        {
        }
    }
    release();
}

void FileSink::writeBytes( const Sequence<sal_Int8>& Buffer ) throw()
{
    if( fp )
    {
        size_t nItems = Buffer.getLength();
        bool bSuccess = (fwrite(Buffer.getConstArray(), 1, nItems, fp) == nItems);
        SAL_WARN_IF( !bSuccess, "extensions.plugin", "short write");
    }
}

void FileSink::flush() throw()
{
    if( fp )
        fflush( fp );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
