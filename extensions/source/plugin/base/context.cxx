/*************************************************************************
 *
 *  $RCSfile: context.cxx,v $
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

#if STLPORT_VERSION>=321
#include <cstdarg>
#endif

#include <plugin/impl.hxx>

#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAGS_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#include <tools/fsys.hxx>
#include <tools/urlobj.hxx>

#include <cppuhelper/implbase1.hxx>

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
    virtual void SAL_CALL writeBytes( const Sequence<sal_Int8>& );
    virtual void SAL_CALL flush();
    virtual void SAL_CALL closeOutput();
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

Reference< ::com::sun::star::plugin::XPluginContext >  XPluginManager_Impl::createPluginContext()
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

::rtl::OUString XPluginContext_Impl::getValue( const Reference< ::com::sun::star::plugin::XPlugin > & plugin, ::com::sun::star::plugin::PluginVariable variable )
    throw( ::com::sun::star::plugin::PluginException, RuntimeException )
{
    return ::rtl::OUString();
}


void XPluginContext_Impl::getURL(const Reference< ::com::sun::star::plugin::XPlugin > & plugin, const ::rtl::OUString& url, const ::rtl::OUString& target) throw( ::com::sun::star::plugin::PluginException, RuntimeException )
{
    Reference< XInterface >  xInst = m_xSMgr->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.frame.Desktop" ) );
    if( ! xInst.is() )
        return;

    if( ! target.len() )
    {
        INetURLObject aURL;
        aURL.SetSmartProtocol( INET_PROT_FILE );
        aURL.SetSmartURL( ::rtl::OUStringToOString( url, m_aEncoding ) );

        Reference< ::com::sun::star::lang::XMultiServiceFactory >  xFact( m_xSMgr->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.frame.DataSourceFactory" ) ), UNO_QUERY );
        if ( xFact.is() )
        {
            Sequence < Any > aArgs(1);
            ::rtl::OUString aUURL( aURL.GetMainURL() );
            aArgs.getArray()[0] <<= aUURL;
            ::rtl::OUString aProt( INetURLObject::GetScheme( aURL.GetProtocol() ) );
            Reference< ::com::sun::star::io::XActiveDataSource >  aSource( xFact->createInstanceWithArguments( aProt, aArgs ), UNO_QUERY );
            if ( aSource.is() )
            {
                // the mimetype cannot be specified
                plugin->provideNewStream( ::rtl::OUString(),
                                          aSource,
                                          aUURL,
                                          0, 0, sal_False );
            }
        }
        return;
    }

    Reference< ::com::sun::star::frame::XComponentLoader >  xLoader( xInst, UNO_QUERY );
    XPlugin_Impl* pPlugin = XPluginManager_Impl::getPluginImplementation( plugin );

    if( xLoader.is() && pPlugin )
    {
        ::com::sun::star::beans::PropertyValue aValue;
        aValue.Name     = ::rtl::OUString::createFromAscii( "Referer" );
        aValue.Value <<= pPlugin->getRefererURL();

        Sequence< ::com::sun::star::beans::PropertyValue > aArgs( &aValue, 1 );
        Reference< ::com::sun::star::lang::XComponent >  xComp =
            xLoader->loadComponentFromURL(
                url,
                target,
                ::com::sun::star::frame::FrameSearchFlag::PARENT        |
                ::com::sun::star::frame::FrameSearchFlag::SELF          |
                ::com::sun::star::frame::FrameSearchFlag::CHILDREN      |
                ::com::sun::star::frame::FrameSearchFlag::SIBLINGS      |
                ::com::sun::star::frame::FrameSearchFlag::TASKS         |
                ::com::sun::star::frame::FrameSearchFlag::CREATE,
                aArgs
                );
    }
}

void XPluginContext_Impl::getURLNotify(const Reference< ::com::sun::star::plugin::XPlugin > & plugin, const ::rtl::OUString& url, const ::rtl::OUString& target, const Reference< ::com::sun::star::lang::XEventListener > & listener )
    throw( ::com::sun::star::plugin::PluginException, RuntimeException )
{
    getURL( plugin, url, target );
    if( listener.is() )
        listener->disposing( ::com::sun::star::lang::EventObject() );
}

::rtl::OUString XPluginContext_Impl::getUserAgent(const Reference< ::com::sun::star::plugin::XPlugin > & plugin)
    throw( ::com::sun::star::plugin::PluginException, RuntimeException )
{
    return ::rtl::OUString::createFromAscii( "Mozilla 3.0" );
}

void XPluginContext_Impl::displayStatusText(const Reference< ::com::sun::star::plugin::XPlugin > & plugin, const ::rtl::OUString& message)
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
                ::com::sun::star::frame::FrameSearchFlag::PARENT        |
                ::com::sun::star::frame::FrameSearchFlag::SELF          |
                ::com::sun::star::frame::FrameSearchFlag::CHILDREN      |
                ::com::sun::star::frame::FrameSearchFlag::SIBLINGS      |
                ::com::sun::star::frame::FrameSearchFlag::TASKS         |
                ::com::sun::star::frame::FrameSearchFlag::CREATE,
                aArgs
                );
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
        m_aMIMEType( mimetype ),
        m_aTarget( target ),
        m_xPlugin( plugin )
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

void FileSink::closeOutput()
{
    if( fp )
        fclose( fp );

    Reference< XInterface >  xInst = m_xSMgr->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.frame.Desktop" ) );
    Reference< ::com::sun::star::frame::XComponentLoader >  xLoader( xInst, UNO_QUERY );
    XPlugin_Impl* pPlugin = XPluginManager_Impl::getPluginImplementation( m_xPlugin );

    if( xLoader.is() && pPlugin )
    {
        ::com::sun::star::beans::PropertyValue aValue;
        aValue.Name = ::rtl::OUString::createFromAscii( "Referer" );
        aValue.Value <<= pPlugin->getRefererURL();

        Sequence< ::com::sun::star::beans::PropertyValue > aArgs( &aValue, 1 );
        Reference< ::com::sun::star::lang::XComponent >  xComp =
            xLoader->loadComponentFromURL(
                m_aFileName,
                m_aTarget,
                ::com::sun::star::frame::FrameSearchFlag::PARENT        |
                ::com::sun::star::frame::FrameSearchFlag::SELF          |
                ::com::sun::star::frame::FrameSearchFlag::CHILDREN      |
                ::com::sun::star::frame::FrameSearchFlag::SIBLINGS      |
                ::com::sun::star::frame::FrameSearchFlag::TASKS         |
                ::com::sun::star::frame::FrameSearchFlag::CREATE,
                aArgs
                );
    }
    release();
}

void FileSink::writeBytes( const Sequence<sal_Int8>& Buffer )
{
    if( fp )
        fwrite( Buffer.getConstArray(), 1, Buffer.getLength(), fp );
}

void FileSink::flush()
{
    if( fp )
        fflush( fp );
}
