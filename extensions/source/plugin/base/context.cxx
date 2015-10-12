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
#endif

#include <cstdarg>
#include <cstdio>

#include <plugin/impl.hxx>

#include <sal/log.hxx>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <comphelper/fileurl.hxx>
#include <tools/urlobj.hxx>
#include <osl/file.hxx>

#include <cppuhelper/implbase1.hxx>

using namespace com::sun::star::io;
using namespace com::sun::star::frame;

namespace ext_plug {

class FileSink : public ::cppu::WeakAggImplHelper1< css::io::XOutputStream >
{
private:
    Reference< css::uno::XComponentContext >   m_xContext;
    FILE*                   fp;
    Reference< css::plugin::XPlugin >          m_xPlugin;
    OUString                 m_aTarget;
    OUString                 m_aFileName;

public:
    FileSink( const Reference< css::uno::XComponentContext > &,
              const Reference< css::plugin::XPlugin > & plugin,
              const OUString& target,
              const Reference< css::io::XActiveDataSource > & source );
    virtual ~FileSink();

    // css::io::XOutputStream
    virtual void SAL_CALL writeBytes( const Sequence<sal_Int8>& ) throw(std::exception) override;
    virtual void SAL_CALL flush() throw(std::exception) override;
    virtual void SAL_CALL closeOutput() throw (RuntimeException, std::exception) override;
};

}
using namespace ext_plug;

class XPluginContext_Impl : public ::cppu::WeakAggImplHelper1< css::plugin::XPluginContext >
{
    Reference< css::uno::XComponentContext >   m_xContext;
    rtl_TextEncoding                           m_aEncoding;
public:

    XPluginContext_Impl( const Reference< css::uno::XComponentContext >  & );
    virtual ~XPluginContext_Impl();


    virtual OUString SAL_CALL getValue(const Reference< css::plugin::XPlugin > & plugin, css::plugin::PluginVariable variable) throw( css::plugin::PluginException, RuntimeException, std::exception ) override;
    virtual void SAL_CALL getURLNotify(const Reference< css::plugin::XPlugin > & plugin, const OUString& url, const OUString& target, const Reference< css::lang::XEventListener > & listener) throw( css::plugin::PluginException, RuntimeException, std::exception ) override;
    virtual void SAL_CALL getURL(const Reference< css::plugin::XPlugin > & plugin, const OUString& url, const OUString& target) throw( css::plugin::PluginException, RuntimeException, std::exception ) override;
    virtual void SAL_CALL postURLNotify(const Reference< css::plugin::XPlugin > & plugin, const OUString& url, const OUString& target, const Sequence< sal_Int8 >& buf, sal_Bool file, const Reference< css::lang::XEventListener > & listener) throw( css::plugin::PluginException, RuntimeException, std::exception ) override;
    virtual void SAL_CALL postURL(const Reference< css::plugin::XPlugin > & plugin, const OUString& url, const OUString& target, const Sequence< sal_Int8 >& buf, sal_Bool file) throw( css::plugin::PluginException, RuntimeException, std::exception ) override;
    virtual void SAL_CALL newStream(const Reference< css::plugin::XPlugin > & plugin, const OUString& mimetype, const OUString& target, const Reference< css::io::XActiveDataSource > & source) throw( css::plugin::PluginException, RuntimeException, std::exception ) override;
    virtual void SAL_CALL displayStatusText(const Reference< css::plugin::XPlugin > & plugin, const OUString& message) throw( css::plugin::PluginException, RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL getUserAgent(const Reference< css::plugin::XPlugin > & plugin) throw( css::plugin::PluginException, RuntimeException, std::exception ) override;
};

Reference< css::plugin::XPluginContext >  XPluginManager_Impl::createPluginContext() throw(std::exception)
{
    return new XPluginContext_Impl( m_xContext );
}

XPluginContext_Impl::XPluginContext_Impl( const Reference< css::uno::XComponentContext >  & rxContext )
    : m_xContext( rxContext ),
      m_aEncoding( osl_getThreadTextEncoding() )
{
}

XPluginContext_Impl::~XPluginContext_Impl()
{
}

OUString XPluginContext_Impl::getValue( const Reference< css::plugin::XPlugin > & /*plugin*/, css::plugin::PluginVariable /*variable*/ )
    throw( css::plugin::PluginException, RuntimeException, std::exception )
{
    return OUString();
}


void XPluginContext_Impl::getURL(const Reference< css::plugin::XPlugin > & plugin, const OUString& url, const OUString& target) throw( css::plugin::PluginException, RuntimeException, std::exception )
{
    Reference< XDesktop2 > xDesktop = Desktop::create(m_xContext);

    if(  target.isEmpty() )
    {
        INetURLObject aURL;
        aURL.SetSmartProtocol( INetProtocol::File );
        aURL.SetSmartURL( url );

        OUString aUrl = aURL.GetMainURL(INetURLObject::DECODE_TO_IURI);
        // the mimetype cannot be specified
        plugin->provideNewStream( OUString(),
                                  Reference< XActiveDataSource >(),
                                  aUrl,
                                  0, 0, comphelper::isFileUrl(aUrl) );
        return;
    }

    XPlugin_Impl* pPlugin = XPluginManager_Impl::getPluginImplementation( plugin );

    if( pPlugin )
    {
        try
        {
            css::beans::PropertyValue aValue;
            aValue.Name     = "Referer";
            aValue.Value <<= pPlugin->getRefererURL();

            Sequence< css::beans::PropertyValue > aArgs( &aValue, 1 );
            Reference< css::lang::XComponent >  xComp =
                xDesktop->loadComponentFromURL(
                                              url,
                                              target,
                                              css::frame::FrameSearchFlag::PARENT          |
                                              css::frame::FrameSearchFlag::SELF            |
                                              css::frame::FrameSearchFlag::CHILDREN        |
                                              css::frame::FrameSearchFlag::SIBLINGS        |
                                              css::frame::FrameSearchFlag::TASKS           |
                                              css::frame::FrameSearchFlag::CREATE,
                                              aArgs
                                              );
        }
        catch(...)
        {
            throw css::plugin::PluginException();
        }
    }
}

void XPluginContext_Impl::getURLNotify(const Reference< css::plugin::XPlugin > & plugin, const OUString& url, const OUString& target, const Reference< css::lang::XEventListener > & listener )
    throw( css::plugin::PluginException, RuntimeException, std::exception )
{
    getURL( plugin, url, target );
    if( listener.is() )
        listener->disposing( css::lang::EventObject() );
}

OUString XPluginContext_Impl::getUserAgent(const Reference< css::plugin::XPlugin > & /*plugin*/)
    throw( css::plugin::PluginException, RuntimeException, std::exception )
{
    return OUString("Mozilla 3.0");
}

void XPluginContext_Impl::displayStatusText(const Reference< css::plugin::XPlugin > & /*plugin*/, const OUString& /*message*/)
    throw( css::plugin::PluginException, RuntimeException, std::exception )
{
}

void XPluginContext_Impl::postURL(const Reference< css::plugin::XPlugin > & plugin, const OUString& url, const OUString& target, const Sequence< sal_Int8 >& buf, sal_Bool file)
    throw( css::plugin::PluginException, RuntimeException, std::exception )
{
    Sequence< sal_Int8 > aBuf;

    if( file )
    {
        OUString aFileName( reinterpret_cast<char const *>(buf.getConstArray()), strlen(reinterpret_cast<char const *>(buf.getConstArray())), m_aEncoding );
        INetURLObject aFilePath( aFileName );
        aFileName = aFilePath.PathToFileName();
        SvFileStream aStream( aFileName, StreamMode::READ );
        if( aStream.IsOpen() )
        {
            sal_Int64 const nBytes = aStream.remainingSize();
            aBuf = Sequence<sal_Int8>( nBytes );
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
            css::beans::PropertyValue aValues[2];
            aValues[0].Name = "Referer";
            aValues[0].Value <<= pPlugin->getRefererURL();

            aValues[1].Name = "PostString";
            aValues[1].Value <<= OStringToOUString( reinterpret_cast<char const *>(( file ? aBuf : buf ).getConstArray()), m_aEncoding );
            Sequence< css::beans::PropertyValue > aArgs( aValues, 2 );
            Reference< css::lang::XComponent >  xComp =
                xDesktop->loadComponentFromURL(
                                              url,
                                              target,
                                              css::frame::FrameSearchFlag::PARENT          |
                                              css::frame::FrameSearchFlag::SELF            |
                                              css::frame::FrameSearchFlag::CHILDREN        |
                                              css::frame::FrameSearchFlag::SIBLINGS        |
                                              css::frame::FrameSearchFlag::TASKS           |
                                              css::frame::FrameSearchFlag::CREATE,
                                              aArgs
                                              );
        }
        catch( ... )
        {
            throw css::plugin::PluginException();
        }
    }
}

void XPluginContext_Impl::postURLNotify(const Reference< css::plugin::XPlugin > & plugin, const OUString& url, const OUString& target, const Sequence< sal_Int8 >& buf, sal_Bool file, const Reference< css::lang::XEventListener > & listener )
    throw( css::plugin::PluginException, RuntimeException, std::exception )
{
    postURL( plugin, url, target, buf, file );
    if( listener.is() )
        listener->disposing( css::lang::EventObject() );
}

void XPluginContext_Impl::newStream( const Reference< css::plugin::XPlugin > & plugin, const OUString&, const OUString& target, const Reference< css::io::XActiveDataSource > & source )
    throw( css::plugin::PluginException, RuntimeException, std::exception )
{
    FileSink*  pNewSink = new FileSink( m_xContext, plugin, target, source );
    pNewSink->acquire();
}



FileSink::FileSink( const Reference< css::uno::XComponentContext >  & rxContext, const Reference< css::plugin::XPlugin > & plugin,
                    const OUString& target, const Reference< css::io::XActiveDataSource > & source ) :
        m_xContext( rxContext ),
        m_xPlugin( plugin ),
        m_aTarget( target )
{
    osl::FileBase::createTempFile( 0, 0, &m_aFileName );
    OString aFile = OUStringToOString( m_aFileName, osl_getThreadTextEncoding() );
    fp = fopen( aFile.getStr() , "wb" );

    Reference< css::io::XActiveDataControl >  xControl( source, UNO_QUERY );

    source->setOutputStream( Reference< css::io::XOutputStream > ( this ) );
    if( xControl.is() )
        xControl->start();
}

FileSink::~FileSink()
{
    osl::File::remove( m_aFileName );
}

void FileSink::closeOutput() throw (RuntimeException, std::exception)
{
    if( fp )
        fclose( fp );

    Reference< XDesktop2 > xDesktop = Desktop::create(m_xContext);
    XPlugin_Impl* pPlugin = XPluginManager_Impl::getPluginImplementation( m_xPlugin );

    if( pPlugin )
    {
        try
        {
            css::beans::PropertyValue aValue;
            aValue.Name = "Referer";
            aValue.Value <<= pPlugin->getRefererURL();

            Sequence< css::beans::PropertyValue > aArgs( &aValue, 1 );
            Reference< css::lang::XComponent >  xComp =
                xDesktop->loadComponentFromURL(
                                              m_aFileName,
                                              m_aTarget,
                                              css::frame::FrameSearchFlag::PARENT          |
                                              css::frame::FrameSearchFlag::SELF            |
                                              css::frame::FrameSearchFlag::CHILDREN        |
                                              css::frame::FrameSearchFlag::SIBLINGS        |
                                              css::frame::FrameSearchFlag::TASKS           |
                                              css::frame::FrameSearchFlag::CREATE,
                                              aArgs
                                              );
        }
        catch( ... )
        {
        }
    }
    release();
}

void FileSink::writeBytes( const Sequence<sal_Int8>& Buffer ) throw(std::exception)
{
    if( fp )
    {
        size_t nItems = Buffer.getLength();
        bool bSuccess = (fwrite(Buffer.getConstArray(), 1, nItems, fp) == nItems);
        SAL_WARN_IF( !bSuccess, "extensions.plugin", "short write");
    }
}

void FileSink::flush() throw(std::exception)
{
    if( fp )
        fflush( fp );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
