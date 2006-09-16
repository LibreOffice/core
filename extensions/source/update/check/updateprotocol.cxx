/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: updateprotocol.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 13:30:37 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#ifndef _COM_SUN_STAR_UCB_XCOMMMANDENVIRONMENT_HPP_
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMMANDPROCESSOR2_HPP_
#include <com/sun/star/ucb/XCommandProcessor2.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTIDNETIFIERFACTORY_HPP_
#include <com/sun/star/ucb/XContentIdentifierFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDER_HPP_
#include <com/sun/star/ucb/XContentProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_POSTCOMMANDARGUMENT2_HPP_
#include <com/sun/star/ucb/PostCommandArgument2.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif

#include "updateprotocol.hxx"

#include <rtl/ref.hxx>
#include <rtl/uri.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/bootstrap.hxx>
#include <osl/process.h>

#include <cppuhelper/implbase1.hxx>

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace ucb = css::ucb ;
namespace io = css::io ;
namespace task = css::task ;
namespace lang = css::lang ;

#define UNISTRING(s) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(s))


/* We encode key value pairs so they correspond to the content type
 * application/x-www-form-urlencoded according to
 * http://www.w3.org/TR/html4/interact/forms.html#h-17.13.4.
 * The only difference is that wie encode space to %20 and not to '+'
 * but this does not matter here.
 */
static ::rtl::OString encodeForPost(const rtl::OUString& rValue)
{
    // Uric derived char class. Additionally & = + are encoded
    static sal_Bool const aCharClass[] =
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, /* !"#$%&'()*+,-./*/
          1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, /*0123456789:;<=>?*/
          1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*@ABCDEFGHIJKLMNO*/
          1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, /*PQRSTUVWXYZ[\]^_*/
          0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*`abcdefghijklmno*/
          1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0  /*pqrstuvwxyz{|}~ */
        };

    ::rtl::OUString aEncodedValue =
          rtl::Uri::encode(rValue,
                           aCharClass,
                           rtl_UriEncodeIgnoreEscapes,
                           RTL_TEXTENCODING_UTF8);

    return ::rtl::OUStringToOString(aEncodedValue, RTL_TEXTENCODING_UTF8);;
}

//------------------------------------------------------------------------------

static bool getBootstrapData(rtl::OUString& rUpdateURL, rtl::OString& rPostData)
{
    rtl::OUString aPath, aPath2;
    if( osl_getExecutableFile(&aPath.pData) != osl_Process_E_None )
        return false;

    sal_uInt32 lastIndex = aPath.lastIndexOf('/');
    if ( lastIndex > 0 )
    {
        aPath2 = aPath = aPath.copy( 0, lastIndex+1 );
        aPath  += UNISTRING( SAL_CONFIGFILE( "bootstrap" ) );
        aPath2 += UNISTRING( SAL_CONFIGFILE( "version" ) );
    }

    rtl::OStringBuffer aPostData(512);

    rtl::OUString aValue;
    rtl::Bootstrap aBootstrapFile(aPath);

    aBootstrapFile.getFrom(UNISTRING("ProductKey"), aValue);
    aPostData.append("ProductKey=");
    aPostData.append(encodeForPost(aValue));

    rtl::Bootstrap aVersionFile(aPath2);

    aVersionFile.getFrom(UNISTRING("buildid"), aValue, rtl::OUString());
    aPostData.append("&buildid=");
    aPostData.append(encodeForPost(aValue));

    aVersionFile.getFrom(UNISTRING("ProductPatch"), aValue, rtl::OUString());
    aPostData.append("&ProductPatch=");
    aPostData.append(encodeForPost(aValue));

    aVersionFile.getFrom(UNISTRING("ProductSource"), aValue, rtl::OUString());
    aPostData.append("&ProductSource=");
    aPostData.append(encodeForPost(aValue));

    aVersionFile.getFrom(UNISTRING("AllLanguages"), aValue, rtl::OUString());
    aPostData.append("&AllLanguages=");
    aPostData.append(encodeForPost(aValue));

    rtl::Bootstrap::get(UNISTRING("_OS"), aValue);
    aPostData.append("&_OS=");
    aPostData.append(encodeForPost(aValue));

    rtl::Bootstrap::get(UNISTRING("_ARCH"), aValue);
    aPostData.append("&_ARCH=");
    aPostData.append(encodeForPost(aValue));

    aVersionFile.getFrom(UNISTRING("UpdateURL"), rUpdateURL, rtl::OUString());
    rPostData = aPostData.makeStringAndClear();

    return true;
}


//------------------------------------------------------------------------------

namespace {

class CommandEnvironment : public ::cppu::WeakImplHelper1< ucb::XCommandEnvironment >
{
    uno::Reference< task::XInteractionHandler > m_xInteractionHandler;

public:
    CommandEnvironment( const uno::Reference< task::XInteractionHandler >& xInteractionHandler ) :
        m_xInteractionHandler(xInteractionHandler) {};

    // XCommandEnvironment
    virtual uno::Reference< task::XInteractionHandler > SAL_CALL getInteractionHandler()
        throw ( uno::RuntimeException ) { return m_xInteractionHandler; };

    virtual uno::Reference< ucb::XProgressHandler > SAL_CALL getProgressHandler()
        throw ( uno::RuntimeException ) { return  uno::Reference< ucb::XProgressHandler >(); };
};

//------------------------------------------------------------------------------

class InputStream : public ::cppu::WeakImplHelper1< io::XInputStream >
{
    sal_Int32    m_nPos;
    rtl::OString m_aData;

public:
    InputStream( const rtl::OString aData ) : m_nPos(0), m_aData(aData) {};

    // XInputStream
    virtual sal_Int32 SAL_CALL readBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException);
    virtual sal_Int32 SAL_CALL readSomeBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
        throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException);
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip )
        throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException);
    virtual sal_Int32 SAL_CALL available(  ) throw (io::NotConnectedException, io::IOException, uno::RuntimeException);
    virtual void SAL_CALL closeInput(  ) throw (io::NotConnectedException, io::IOException, uno::RuntimeException);
};

//------------------------------------------------------------------------------

sal_Int32 SAL_CALL
InputStream::readBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
    throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
{
    if( nBytesToRead < 0 )
        throw io::BufferSizeExceededException(rtl::OUString(),*this);

    sal_Int32 nAvail = available();

    if (nAvail < nBytesToRead)
        nBytesToRead = nAvail;

    aData.realloc(nBytesToRead);
    rtl_copyMemory(aData.getArray(), m_aData.getStr() + m_nPos, nBytesToRead);
    m_nPos += nBytesToRead;

    return nBytesToRead;
}

//------------------------------------------------------------------------------

sal_Int32 SAL_CALL
InputStream::readSomeBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
    throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
{
    return readBytes(aData, nMaxBytesToRead);
}

//------------------------------------------------------------------------------

void SAL_CALL
InputStream::skipBytes( sal_Int32 nBytesToSkip )
    throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
{
    if (nBytesToSkip < 0)
        throw io::BufferSizeExceededException(::rtl::OUString(),*this);

    sal_Int32 nAvail = available();

    if (nAvail < nBytesToSkip)
        nBytesToSkip = nAvail;

    m_nPos += nBytesToSkip;
}

//------------------------------------------------------------------------------

sal_Int32 SAL_CALL
InputStream::available(  )
    throw (io::NotConnectedException, io::IOException, uno::RuntimeException)
{
    return m_aData.getLength() - m_nPos;
};

//------------------------------------------------------------------------------

void SAL_CALL
InputStream::closeInput(  )
    throw (io::NotConnectedException, io::IOException, uno::RuntimeException)
{
};

//------------------------------------------------------------------------------

class OutputStream : public ::cppu::WeakImplHelper1< io::XOutputStream >
{
    rtl::OStringBuffer m_aData;

public:
    OutputStream() {};

    rtl::OString getData() { return m_aData.makeStringAndClear(); };

    inline operator uno::Reference< io::XOutputStream > () { return this; };

    virtual void SAL_CALL writeBytes( const uno::Sequence< sal_Int8 >& aData )
        throw( io::NotConnectedException, io::BufferSizeExceededException, io::IOException)
    { m_aData.append( reinterpret_cast < const sal_Char * > (aData.getConstArray()), aData.getLength()); };

    virtual void SAL_CALL flush()
        throw( io::NotConnectedException, io::BufferSizeExceededException, io::IOException) {};

    virtual void SAL_CALL closeOutput()
        throw( io::NotConnectedException, io::BufferSizeExceededException, io::IOException) {};
};

} // anonymous namespace

//------------------------------------------------------------------------------

// Returns 'true' if successfully connected to the update server
bool
checkForUpdates(
    uno::Reference< uno::XComponentContext > const & rxContext,
    uno::Reference< task::XInteractionHandler > const & rxInteractionHandler,
    rtl::OUString& rUpdateURL, rtl::OUString& rVersionFound )
{
    OSL_TRACE("checking for updates ..\n");

    ::rtl::OUString aUpdateURL;
    ::rtl::OString  aPostData;

    if( ! ( getBootstrapData(aUpdateURL, aPostData) && (aUpdateURL.getLength() > 0) ) )
        return false;

//    OSL_TRACE("post data: %s\n", aPostData.getStr());

    if( !rxContext.is() )
        throw uno::RuntimeException(
            UNISTRING( "checkForUpdates: empty component context" ), uno::Reference< uno::XInterface >() );

    uno::Reference< lang::XMultiComponentFactory > xServiceManager(rxContext->getServiceManager());

    if( !xServiceManager.is() )
        throw uno::RuntimeException(
            UNISTRING( "checkForUpdates: unable to obtain service manager from component context" ), uno::Reference< uno::XInterface >() );

    // Retrieve UCB / Content identifier factory
    uno::Reference< ucb::XContentIdentifierFactory > xIdFactory(
        xServiceManager->createInstanceWithContext( UNISTRING( "com.sun.star.ucb.UniversalContentBroker" ), rxContext ),
        uno::UNO_QUERY_THROW);

    uno::Reference< ucb::XContentIdentifier > xId = xIdFactory->createContentIdentifier(aUpdateURL);

    if( !xId.is() )
        throw uno::RuntimeException(
            UNISTRING( "checkForUpdates: unable to obtain universal content id" ), uno::Reference< uno::XInterface >() );

    // Query content provider and command processor
    uno::Reference< ucb::XContentProvider > xProvider(xIdFactory, uno::UNO_QUERY_THROW);
    uno::Reference< ucb::XCommandProcessor > xCommandProcessor(xProvider->queryContent(xId), uno::UNO_QUERY_THROW);

    rtl::Reference< OutputStream > aOutputStream(new OutputStream());

    ucb::PostCommandArgument2 aPostArgument( new InputStream(aPostData), *aOutputStream,
        UNISTRING("application/x-www-form-urlencoded"), rtl::OUString() );

    ucb::Command aCommand(UNISTRING("post"), -1, uno::makeAny(aPostArgument));

    // Execute the http post
    sal_Int32 nCommandId = xCommandProcessor->createCommandIdentifier();

    try
    {
        xCommandProcessor->execute(aCommand, nCommandId, new CommandEnvironment(rxInteractionHandler));
    }
    catch(const uno::Exception& e)
    {
        OSL_TRACE( "Caught exception: %s\n", rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8).getStr());
        return false;
    }

    uno::Reference< ucb::XCommandProcessor2 > xCommandProcessor2(xCommandProcessor, uno::UNO_QUERY);
    if( xCommandProcessor2.is() )
        xCommandProcessor2->releaseCommandIdentifier(nCommandId);

    rtl::OString aReply = aOutputStream->getData();
    aOutputStream.clear();

//    OSL_TRACE( "server replied: %s\n", aReply.getStr());

    if( aReply.getLength() < 2 )
        return false;

    if( 0 != aReply.compareTo("no", 2) )
    {
        if( rtl_str_shortenedCompare_WithLength(aReply.pData->buffer, aReply.pData->length, "yes$$$", 6, 6) != 0 )
            return false;

        // extract URL ..
        sal_Int32 nIndex = aReply.indexOf("$$$", 6);

        // .. followed by a list of key value pairs 'key=value\xa'
        if( aReply.getLength() < nIndex + 3 )
            return false;

        rUpdateURL = rtl::OStringToOUString(aReply.copy(6, nIndex - 6), RTL_TEXTENCODING_UTF8);

        nIndex = aReply.indexOf("buildid=", nIndex + 3) + 8;
        rVersionFound = rtl::OStringToOUString( aReply.getToken(0, '\xa', nIndex), RTL_TEXTENCODING_UTF8);
    }

    return true;
}
