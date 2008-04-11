/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: context.cxx,v $
 * $Revision: 1.12 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#if STLPORT_VERSION>=321
#include <cstdarg>
#include <cstdio>
#endif

#include <plugin/impl.hxx>

#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAGS_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <tools/fsys.hxx>
#include <tools/urlobj.hxx>

#include <cppuhelper/implbase1.hxx>

using namespace com::sun::star::io;

namespace ext_plug {

class FileSink : public ::cppu::WeakAggImplHelper1< ::com::sun::star::io::XOutputStream >
{
private:
    Reference< ::com::sun::star::lang::XMultiServiceFactory >   m_xSMgr;
    FILE*                   fp;
    Reference< ::com::sun::star::plugin::XPlugin >              m_xPlugin;
    ::rtl::OUString                 m_aMIMEType;
    ::rtl::OUString                 m_aTarget;
    ::rtl::OUString                 m_aFileName;

public:
    FileSink( const Reference< ::com::sun::star::lang::XMultiServiceFactory >   &,
              const Reference< ::com::sun::star::plugin::XPlugin > & plugin,
              const ::rtl::OUString& mimetype,
              const ::rtl::OUString& target,
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
    Reference< ::com::sun::star::lang::XMultiServiceFactory >   m_xSMgr;
    rtl_TextEncoding                                        m_aEncoding;
public:

    XPluginContext_Impl( const Reference< ::com::sun::star::lang::XMultiServiceFactory >  & );
    virtual ~XPluginContext_Impl();


    virtual ::rtl::OUString SAL_CALL getValue(const Reference< ::com::sun::star::plugin::XPlugin > & plugin, ::com::sun::star::plugin::PluginVariable variable) throw( ::com::sun::star::plugin::PluginException, RuntimeException );
    virtual void SAL_CALL getURLNotify(const Reference< ::com::sun::star::plugin::XPlugin > & plugin, const ::rtl::OUString& url, const ::rtl::OUString& target, const Reference< ::com::sun::star::lang::XEventListener > & listener) throw( ::com::sun::star::plugin::PluginException, RuntimeException );
    virtual void SAL_CALL getURL(const Reference< ::com::sun::star::plugin::XPlugin > & plugin, const ::rtl::OUString& url, const ::rtl::OUString& target) throw( ::com::sun::star::plugin::PluginException, RuntimeException );
    virtual void SAL_CALL postURLNotify(const Reference< ::com::sun::star::plugin::XPlugin > & plugin, const ::rtl::OUString& url, const ::rtl::OUString& target, const Sequence< sal_Int8 >& buf, sal_Bool file, const Reference< ::com::sun::star::lang::XEventListener > & listener) throw( ::com::sun::star::plugin::PluginException, RuntimeException );
    virtual void SAL_CALL postURL(const Reference< ::com::sun::star::plugin::XPlugin > & plugin, const ::rtl::OUString& url, const ::rtl::OUString& target, const Sequence< sal_Int8 >& buf, sal_Bool file) throw( ::com::sun::star::plugin::PluginException, RuntimeException );
    virtual void SAL_CALL newStream(const Reference< ::com::sun::star::plugin::XPlugin > & plugin, const ::rtl::OUString& mimetype, const ::rtl::OUString& target, const Reference< ::com::sun::star::io::XActiveDataSource > & source) throw( ::com::sun::star::plugin::PluginException, RuntimeException );
    virtual void SAL_CALL displayStatusText(const Reference< ::com::sun::star::plugin::XPlugin > & plugin, const ::rtl::OUString& message) throw( ::com::sun::star::plugin::PluginException, RuntimeException );
    virtual ::rtl::OUString SAL_CALL getUserAgent(const Reference< ::com::sun::star::plugin::XPlugin > & plugin) throw( ::com::sun::star::plugin::PluginException, RuntimeException );
};

Reference< ::com::sun::star::plugin::XPluginContext >  XPluginManager_Impl::createPluginContext() throw()
{
    return new XPluginContext_Impl( m_xSMgr );
}

XPluginContext_Impl::XPluginContext_Impl( const Reference< ::com::sun::star::lang::XMultiServiceFactory >  & rSMgr )
    : m_xSMgr( rSMgr ),
      m_aEncoding( gsl_getSystemTextEncoding() )
{
}

XPluginContext_Impl::~XPluginContext_Impl()
{
}

::rtl::OUString XPluginContext_Impl::getValue( const Reference< ::com::sun::star::plugin::XPlugin > & /*plugin*/, ::com::sun::star::plugin::PluginVariable /*variable*/ )
    throw( ::com::sun::star::plugin::PluginException, RuntimeException )
{
    return ::rtl::OUString();
}


void XPluginContext_Impl::getURL(const Reference< ::com::sun::star::plugin::XPlugin > & plugin, const ::rtl::OUString& url, const ::rtl::OUString& target) throw( ::com::sun::star::plugin::PluginException, RuntimeException )
{
    Reference< XInterface >  xInst = m_xSMgr->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.frame.Desktop" ) );
    if( ! xInst.is() )
        return;

    if( ! target.getLength() )
    {
        INetURLObject aURL;
        aURL.SetSmartProtocol( INET_PROT_FILE );
        aURL.SetSmartURL( ::rtl::OUStringToOString( url, m_aEncoding ) );

        rtl::OUString aUrl = aURL.GetMainURL(INetURLObject::DECODE_TO_IURI);
        // the mimetype cannot be specified
        plugin->provideNewStream( ::rtl::OUString(),
                                  Reference< XActiveDataSource >(),
                                  aUrl,
                                  0, 0, (sal_Bool)(aUrl.compareToAscii( "file:", 5 ) == 0) );
        return;
    }

    Reference< ::com::sun::star::frame::XComponentLoader >  xLoader( xInst, UNO_QUERY );
    XPlugin_Impl* pPlugin = XPluginManager_Impl::getPluginImplementation( plugin );

    if( xLoader.is() && pPlugin )
    {
        try
        {
            ::com::sun::star::beans::PropertyValue aValue;
            aValue.Name     = ::rtl::OUString::createFromAscii( "Referer" );
            aValue.Value <<= pPlugin->getRefererURL();

            Sequence< ::com::sun::star::beans::PropertyValue > aArgs( &aValue, 1 );
            Reference< ::com::sun::star::lang::XComponent >  xComp =
                xLoader->loadComponentFromURL(
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

void XPluginContext_Impl::getURLNotify(const Reference< ::com::sun::star::plugin::XPlugin > & plugin, const ::rtl::OUString& url, const ::rtl::OUString& target, const Reference< ::com::sun::star::lang::XEventListener > & listener )
    throw( ::com::sun::star::plugin::PluginException, RuntimeException )
{
    getURL( plugin, url, target );
    if( listener.is() )
        listener->disposing( ::com::sun::star::lang::EventObject() );
}

::rtl::OUString XPluginContext_Impl::getUserAgent(const Reference< ::com::sun::star::plugin::XPlugin > & /*plugin*/)
    throw( ::com::sun::star::plugin::PluginException, RuntimeException )
{
    return ::rtl::OUString::createFromAscii( "Mozilla 3.0" );
}

void XPluginContext_Impl::displayStatusText(const Reference< ::com::sun::star::plugin::XPlugin > & /*plugin*/, const ::rtl::OUString& /*message*/)
    throw( ::com::sun::star::plugin::PluginException, RuntimeException )
{
}

void XPluginContext_Impl::postURL(const Reference< ::com::sun::star::plugin::XPlugin > & plugin, const ::rtl::OUString& url, const ::rtl::OUString& target, const Sequence< sal_Int8 >& buf, sal_Bool file)
    throw( ::com::sun::star::plugin::PluginException, RuntimeException )
{
    Sequence< sal_Int8 > aBuf;

    if( file )
    {
        String aFileName( (char*)buf.getConstArray(), m_aEncoding );
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
            DirEntry( aFileName ).Kill();
        }
    }

    Reference< XInterface > xInst = m_xSMgr->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.frame.Desktop" ) );
    if( ! xInst.is() )
        return ;

    Reference< ::com::sun::star::frame::XComponentLoader >  xLoader( xInst, UNO_QUERY );
    XPlugin_Impl* pPlugin = XPluginManager_Impl::getPluginImplementation( plugin );
    if( xLoader.is() && pPlugin )
    {
        try
        {
            ::com::sun::star::beans::PropertyValue aValues[2];
            aValues[0].Name = ::rtl::OUString::createFromAscii( "Referer" );
            aValues[0].Value <<= pPlugin->getRefererURL();

            aValues[1].Name = ::rtl::OUString::createFromAscii( "PostString" );
            aValues[1].Value <<= ::rtl::OStringToOUString( (char*)( file ? aBuf : buf ).getConstArray(), m_aEncoding );
            Sequence< ::com::sun::star::beans::PropertyValue > aArgs( aValues, 2 );
            Reference< ::com::sun::star::lang::XComponent >  xComp =
                xLoader->loadComponentFromURL(
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

void XPluginContext_Impl::postURLNotify(const Reference< ::com::sun::star::plugin::XPlugin > & plugin, const ::rtl::OUString& url, const ::rtl::OUString& target, const Sequence< sal_Int8 >& buf, sal_Bool file, const Reference< ::com::sun::star::lang::XEventListener > & listener )
    throw( ::com::sun::star::plugin::PluginException, RuntimeException )
{
    postURL( plugin, url, target, buf, file );
    if( listener.is() )
        listener->disposing( ::com::sun::star::lang::EventObject() );
}

void XPluginContext_Impl::newStream( const Reference< ::com::sun::star::plugin::XPlugin > & plugin, const ::rtl::OUString& mimetype, const ::rtl::OUString& target, const Reference< ::com::sun::star::io::XActiveDataSource > & source )
    throw( ::com::sun::star::plugin::PluginException, RuntimeException )
{
    FileSink*  pNewSink = new FileSink( m_xSMgr, plugin, mimetype, target, source );
    pNewSink->acquire();
}



FileSink::FileSink( const Reference< ::com::sun::star::lang::XMultiServiceFactory >  & rSMgr, const Reference< ::com::sun::star::plugin::XPlugin > & plugin,
                    const ::rtl::OUString& mimetype,
                    const ::rtl::OUString& target, const Reference< ::com::sun::star::io::XActiveDataSource > & source ) :
        m_xSMgr( rSMgr ),
        m_xPlugin( plugin ),
        m_aMIMEType( mimetype ),
        m_aTarget( target )
{
    DirEntry aEntry;
    m_aFileName = aEntry.TempName().GetFull();
    ::rtl::OString aFile = ::rtl::OUStringToOString( m_aFileName, gsl_getSystemTextEncoding() );
    fp = fopen( aFile.getStr() , "wb" );

    Reference< ::com::sun::star::io::XActiveDataControl >  xControl( source, UNO_QUERY );

    source->setOutputStream( Reference< ::com::sun::star::io::XOutputStream > ( this ) );
    if( xControl.is() )
        xControl->start();
}

FileSink::~FileSink()
{
    DirEntry aEntry( m_aFileName );
    aEntry.Kill();
}

void FileSink::closeOutput() throw()
{
    if( fp )
        fclose( fp );

    Reference< XInterface >  xInst = m_xSMgr->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.frame.Desktop" ) );
    Reference< ::com::sun::star::frame::XComponentLoader >  xLoader( xInst, UNO_QUERY );
    XPlugin_Impl* pPlugin = XPluginManager_Impl::getPluginImplementation( m_xPlugin );

    if( xLoader.is() && pPlugin )
    {
        try
        {
            ::com::sun::star::beans::PropertyValue aValue;
            aValue.Name = ::rtl::OUString::createFromAscii( "Referer" );
            aValue.Value <<= pPlugin->getRefererURL();

            Sequence< ::com::sun::star::beans::PropertyValue > aArgs( &aValue, 1 );
            Reference< ::com::sun::star::lang::XComponent >  xComp =
                xLoader->loadComponentFromURL(
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
        fwrite( Buffer.getConstArray(), 1, Buffer.getLength(), fp );
}

void FileSink::flush() throw()
{
    if( fp )
        fflush( fp );
}
