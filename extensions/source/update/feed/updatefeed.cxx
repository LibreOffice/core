/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: updatefeed.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2007-01-29 14:35:50 $
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

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/implementationentry.hxx>

#ifndef  _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif
#ifndef  _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef  _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_DEPLOYMENT_UPDATEINFORMATIONPROVIDER_HPP_
#include <com/sun/star/deployment/UpdateInformationProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_XCOMMMANDENVIRONMENT_HPP_
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XWEBDAVCOMMMANDENVIRONMENT_HPP_
#include <com/sun/star/ucb/XWebDAVCommandEnvironment.hpp>
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
#ifndef _COM_SUN_STAR_UCB_OPENCOMMANDARGUMENT2_HPP_
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENMODE_HPP_
#include <com/sun/star/ucb/OpenMode.hpp>
#endif

#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif

#ifndef _COM_SUN_STAR_XML_DOM_XDOCUMENTBUILDER_HPP_
#include <com/sun/star/xml/dom/XDocumentBuilder.hpp>
#endif

#ifndef _COM_SUN_STAR_XML_XPATH_XXPATHAPI_HPP_
#include <com/sun/star/xml/xpath/XXPathAPI.hpp>
#endif

#include <rtl/ref.hxx>
#include <rtl/memory.h>
#include <rtl/bootstrap.hxx>
#include <osl/process.h>
#include <osl/conditn.hxx>

#ifndef _ZLIB_H
#ifdef SYSTEM_ZLIB
#include "zlib.h"
#else
#include "zlib/zlib.h"
#endif
#endif

namespace beans = com::sun::star::beans ;
namespace container = com::sun::star::container ;
namespace deployment = com::sun::star::deployment ;
namespace io = com::sun::star::io ;
namespace lang = com::sun::star::lang ;
namespace task = com::sun::star::task ;
namespace ucb = com::sun::star::ucb ;
namespace uno = com::sun::star::uno ;
namespace xml = com::sun::star::xml ;
namespace sdbc = com::sun::star::sdbc ;

#define UNISTRING(s) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(s))

//------------------------------------------------------------------------------

namespace
{

#ifdef DEBUG

class InputStreamWrapper : public ::cppu::WeakImplHelper1< io::XInputStream >
{
    uno::Reference< io::XInputStream > m_xStream;

public:
    InputStreamWrapper(const uno::Reference< io::XInputStream >& rxStream) :
        m_xStream(rxStream) {};

    virtual sal_Int32 SAL_CALL readBytes(uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead)
        throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
        {
            sal_Int32 n = m_xStream->readBytes(aData, nBytesToRead);
            OSL_TRACE( aData.get()->elements );
            return n;
        };
    virtual sal_Int32 SAL_CALL readSomeBytes(uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead)
        throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
        {
            sal_Int32 n = m_xStream->readSomeBytes(aData, nMaxBytesToRead);
            OSL_TRACE( aData.get()->elements );
            return n;
        };
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip )
        throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
        { m_xStream->skipBytes(nBytesToSkip); };
    virtual sal_Int32 SAL_CALL available()
        throw (io::NotConnectedException, io::IOException, uno::RuntimeException)
        { return m_xStream->available(); };
    virtual void SAL_CALL closeInput( )
        throw (io::NotConnectedException, io::IOException, uno::RuntimeException)
        {};
};

#define INPUT_STREAM(i) new InputStreamWrapper(i)
#else
#define INPUT_STREAM(i) i
#endif

//------------------------------------------------------------------------------

class ActiveDataSink : public ::cppu::WeakImplHelper1< io::XActiveDataSink >
{
    uno::Reference< io::XInputStream > m_xStream;

public:
    ActiveDataSink() {};

    inline operator uno::Reference< io::XActiveDataSink > () { return this; };

    virtual uno::Reference< io::XInputStream > SAL_CALL getInputStream()
        throw (uno::RuntimeException) { return m_xStream; };
    virtual void SAL_CALL setInputStream( uno::Reference< io::XInputStream > const & rStream )
        throw (uno::RuntimeException) { m_xStream = rStream; };
};

//------------------------------------------------------------------------------

class InflateInputStream : public ::cppu::WeakImplHelper1< io::XInputStream >
{
    uno::Reference< io::XInputStream > m_xStream;

    uno::Sequence < sal_Int8 > m_aBuffer;
    bool m_bRead;
    sal_Int32 m_nOffset;

    rtl::OUString m_aContentEncoding;

    void readIntoMemory();

public:
    InflateInputStream(const uno::Reference< io::XInputStream >& rxStream,const rtl::OUString& rContentEncoding) :
        m_xStream(rxStream), m_nOffset(0), m_bRead(false), m_aContentEncoding(rContentEncoding) {};

    virtual sal_Int32 SAL_CALL readBytes(uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead)
        throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException);
    virtual sal_Int32 SAL_CALL readSomeBytes(uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead)
        throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
        { readIntoMemory(); return readBytes(aData, nMaxBytesToRead ); };
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip )
        throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
        {
            readIntoMemory();
            if( m_nOffset + nBytesToSkip < m_aBuffer.getLength() )
                m_nOffset += nBytesToSkip;
            else
                m_nOffset = m_aBuffer.getLength();
        };
    virtual sal_Int32 SAL_CALL available()
        throw (io::NotConnectedException, io::IOException, uno::RuntimeException)
        {   readIntoMemory(); return m_aBuffer.getLength() - m_nOffset; };
    virtual void SAL_CALL closeInput( )
        throw (io::NotConnectedException, io::IOException, uno::RuntimeException)
        { m_xStream->closeInput(); };
};


sal_Int32 SAL_CALL
InflateInputStream::readBytes(uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead)
    throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
{
    readIntoMemory();
    sal_Int32 nAvailable = available();
    sal_Int32 nBytesToCopy =  nAvailable < nBytesToRead ? nAvailable : nBytesToRead;
    if( nBytesToCopy > 0 )
    {
        aData.realloc(nBytesToCopy);
        rtl_copyMemory(aData.getArray(), m_aBuffer.getConstArray() + m_nOffset, nBytesToCopy);
        m_nOffset += nBytesToCopy;
    }

    return nBytesToCopy;
};

void InflateInputStream::readIntoMemory()
{
    if( !m_bRead && m_xStream.is() )
    {
        uno::Sequence < sal_Int8 > aCompressedBuffer;
        uno::Sequence < sal_Int8 > aTempBuffer(4096);
        sal_Int32 nBytesRead;

        m_bRead = true;

        do
        {
            nBytesRead = m_xStream->readBytes(aTempBuffer, aTempBuffer.getLength());

            if( nBytesRead > 0 )
            {
                sal_Int32 nOffset = aCompressedBuffer.getLength();
                aCompressedBuffer.realloc( nOffset + nBytesRead );

                rtl_copyMemory(aCompressedBuffer.getArray() + nOffset, aTempBuffer.getConstArray(), nBytesRead);
            }
        }
        while( nBytesRead == aTempBuffer.getLength() );

        z_stream *pStream = new z_stream;
        /* memset to 0 to set zalloc/opaque etc */
        rtl_zeroMemory (pStream, sizeof(*pStream));

        int windowSize = 15;
        int headerOffset = 0;

        if( m_aContentEncoding.equalsAscii("gzip") )
        {
            sal_uInt8 magic[2];
            magic[0] = *((sal_uInt8 *) aCompressedBuffer.getConstArray());
            magic[1] = *((sal_uInt8 *) aCompressedBuffer.getConstArray() + 1);

            if( (magic[0] == 0x1f) && (magic[1] == 0x8b) )
            {
                windowSize = -14;
                headerOffset = 10;
            }
        }

        pStream->next_in = (unsigned char *) aCompressedBuffer.getConstArray();
        pStream->avail_in = aCompressedBuffer.getLength();

        pStream->next_in += headerOffset;
        pStream->avail_in -= headerOffset;

        if( Z_OK == inflateInit2(pStream, windowSize) )
        {
            int result;

            do
            {
                sal_Int32 nOffset = m_aBuffer.getLength();
                m_aBuffer.realloc(nOffset + 4096);

                pStream->next_out  = reinterpret_cast < unsigned char* > ( m_aBuffer.getArray() + nOffset );
                pStream->avail_out = 4096;

                result = ::inflate(pStream, Z_FINISH);

                if( result ==  Z_STREAM_END )
                    break;

            } while( result ==  Z_BUF_ERROR );

            inflateEnd(pStream);
            m_aBuffer.realloc(pStream->total_out);

        }

        if (pStream != NULL)
        {
            delete pStream;
            pStream = NULL;
        }
    }
}


//------------------------------------------------------------------------------

class UpdateInformationProvider :
    public ::cppu::WeakImplHelper4< deployment::XUpdateInformationProvider,
                                    ucb::XCommandEnvironment,
                                    ucb::XWebDAVCommandEnvironment,
                                    lang::XServiceInfo >
{
    uno::Reference< uno::XComponentContext> m_xContext;
    uno::Reference< ucb::XCommandProcessor > m_xCommandProcessor;
    uno::Reference< task::XInteractionHandler > m_xInteractionHandler;

    uno::Sequence< beans::NamedValue > m_aRequestHeaderList;

    osl::Mutex m_aMutex;
    osl::Condition m_bCancelled;

    sal_Int32 m_nCommandId;

    uno::Reference< io::XInputStream > load(
        uno::Reference< ucb::XContentIdentifierFactory > const & rxIdFactory,
        uno::Reference< ucb::XContentProvider > const & rxProvider,
        rtl::OUString const & rURL);

    uno::Reference< xml::dom::XElement > getDocumentRoot(
        uno::Reference< ucb::XContentIdentifierFactory > const & rxIdFactory,
        uno::Reference< ucb::XContentProvider > const & rxProvider,
        uno::Reference< xml::dom::XDocumentBuilder > const & rxBuilder,
        uno::Reference< xml::dom::XNode > const & rxNode);

    void storeCommandInfo( sal_Int32 nCommandId,
        uno::Reference< ucb::XCommandProcessor > const & rxCommandProcessor);

    rtl::OUString m_aUpdateURL;

protected:

    static uno::Any getUILanguage(uno::Reference<uno::XComponentContext> const & xContext);

public:
    UpdateInformationProvider(const uno::Reference<uno::XComponentContext>&);
    virtual ~UpdateInformationProvider();

    static uno::Sequence< rtl::OUString > getServiceNames();
    static rtl::OUString getImplName();

    // XUpdateInformationService
    virtual uno::Sequence< uno::Reference< xml::dom::XElement > > SAL_CALL
    getUpdateInformation(
        uno::Sequence< rtl::OUString > const & repositories,
        rtl::OUString const & extensionId
    ) throw (uno::Exception, uno::RuntimeException);

    virtual sal_Bool SAL_CALL hasPredeterminedUpdateURL()
        throw (uno::RuntimeException);

    virtual void SAL_CALL cancel()
        throw (uno::RuntimeException);

    virtual void SAL_CALL setInteractionHandler(
        uno::Reference< task::XInteractionHandler > const & handler )
        throw (uno::RuntimeException);

    // XCommandEnvironment
    virtual uno::Reference< task::XInteractionHandler > SAL_CALL getInteractionHandler()
        throw ( uno::RuntimeException ) { osl::MutexGuard aGuard(m_aMutex); return m_xInteractionHandler; };

    virtual uno::Reference< ucb::XProgressHandler > SAL_CALL getProgressHandler()
        throw ( uno::RuntimeException ) { return  uno::Reference< ucb::XProgressHandler >(); };

    // XWebDAVCommandEnvironment
    virtual uno::Sequence< beans::NamedValue > SAL_CALL getUserRequestHeaders(
        const rtl::OUString&, const rtl::OUString& )
        throw ( uno::RuntimeException ) { return m_aRequestHeaderList; };

    // XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName()
        throw (uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService(rtl::OUString const & serviceName)
        throw (uno::RuntimeException);
    virtual uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw (uno::RuntimeException);
};

//------------------------------------------------------------------------------

UpdateInformationProvider::UpdateInformationProvider(uno::Reference<uno::XComponentContext> const & xContext) :
    m_xContext(xContext), m_aRequestHeaderList(2)
{
    rtl::OUString aPath;
    if( osl_getExecutableFile(&aPath.pData) == osl_Process_E_None )
    {
        sal_uInt32 lastIndex = aPath.lastIndexOf('/');
        if ( lastIndex > 0 )
        {
            aPath = aPath.copy( 0, lastIndex+1 );
            aPath += UNISTRING( SAL_CONFIGFILE( "version" ) );
        }

        rtl::Bootstrap aVersionFile(aPath);
        aVersionFile.getFrom(UNISTRING("UpdateURL"), m_aUpdateURL, rtl::OUString());

        rtl::OUString aUserAgent;
        aVersionFile.getFrom(UNISTRING("UpdateUserAgent"), aUserAgent, rtl::OUString());

        m_aRequestHeaderList[0].Name = UNISTRING("Accept-Language");
        m_aRequestHeaderList[0].Value = getUILanguage( xContext );
        m_aRequestHeaderList[1].Name = UNISTRING("Accept-Encoding");
        m_aRequestHeaderList[1].Value = uno::makeAny( UNISTRING("gzip,deflate") );

        if( aUserAgent.getLength() > 0 )
        {
            m_aRequestHeaderList.realloc(3);
            m_aRequestHeaderList[2].Name = UNISTRING("User-Agent");
            m_aRequestHeaderList[2].Value = uno::makeAny(aUserAgent);
        }
    }
}

//------------------------------------------------------------------------------

UpdateInformationProvider::~UpdateInformationProvider()
{
}

//------------------------------------------------------------------------------

uno::Any
UpdateInformationProvider::getUILanguage(uno::Reference<uno::XComponentContext> const & xContext)
{
    uno::Reference< lang::XMultiComponentFactory > xServiceManager(xContext->getServiceManager());
    if( !xServiceManager.is() )
        throw uno::RuntimeException(
            UNISTRING("unable to obtain service manager from component context"),
            uno::Reference< uno::XInterface >());

    uno::Reference< lang::XMultiServiceFactory > xConfigurationProvider(
        xServiceManager->createInstanceWithContext(
            UNISTRING("com.sun.star.configuration.ConfigurationProvider"),
            xContext ),
        uno::UNO_QUERY_THROW);

    beans::PropertyValue aProperty;
    aProperty.Name  = UNISTRING("nodepath");
    aProperty.Value = uno::makeAny(UNISTRING("org.openoffice.Setup/L10N"));

    uno::Sequence< uno::Any > aArgumentList( 1 );
    aArgumentList[0] = uno::makeAny( aProperty );

    uno::Reference< container::XNameAccess > xNameAccess(
        xConfigurationProvider->createInstanceWithArguments(
            UNISTRING("com.sun.star.configuration.ConfigurationAccess"),
            aArgumentList ),
        uno::UNO_QUERY_THROW);

    return xNameAccess->getByName(UNISTRING("ooLocale"));
}

//------------------------------------------------------------------------------

void
UpdateInformationProvider::storeCommandInfo(
    sal_Int32 nCommandId,
    uno::Reference< ucb::XCommandProcessor > const & rxCommandProcessor)
{
    osl::MutexGuard aGuard(m_aMutex);

    m_nCommandId = nCommandId;
    m_xCommandProcessor = rxCommandProcessor;
}

//------------------------------------------------------------------------------

uno::Reference< io::XInputStream >
UpdateInformationProvider::load(
    uno::Reference< ucb::XContentIdentifierFactory > const & rxIdFactory,
    uno::Reference< ucb::XContentProvider > const & rxProvider,
    rtl::OUString const & rURL)
{
    uno::Reference< ucb::XContentIdentifier > xId = rxIdFactory->createContentIdentifier(rURL);

    if( !xId.is() )
        throw uno::RuntimeException(
            UNISTRING( "unable to obtain universal content id" ), *this);

    uno::Reference< ucb::XCommandProcessor > xCommandProcessor(rxProvider->queryContent(xId), uno::UNO_QUERY_THROW);
    rtl::Reference< ActiveDataSink > aSink(new ActiveDataSink());

    ucb::OpenCommandArgument2 aOpenArgument;
    aOpenArgument.Mode = ucb::OpenMode::DOCUMENT;
    aOpenArgument.Priority = 32768;
    aOpenArgument.Sink = *aSink;

    ucb::Command aCommand;
    aCommand.Name = UNISTRING("open");
    aCommand.Argument = uno::makeAny(aOpenArgument);

    sal_Int32 nCommandId = xCommandProcessor->createCommandIdentifier();

    storeCommandInfo(nCommandId, xCommandProcessor);
    try
    {
        uno::Any aResult = xCommandProcessor->execute(aCommand, nCommandId,
            static_cast < XCommandEnvironment *> (this));
    }
    catch( const uno::Exception & e )
    {
        storeCommandInfo(0, uno::Reference< ucb::XCommandProcessor > ());

        uno::Reference< ucb::XCommandProcessor2 > xCommandProcessor2(xCommandProcessor, uno::UNO_QUERY);
        if( xCommandProcessor2.is() )
            xCommandProcessor2->releaseCommandIdentifier(nCommandId);

        throw;
    }

    uno::Sequence< beans::Property > aProps( 1 );
    aProps[0].Name = UNISTRING( "Content-Encoding" );

    aCommand.Name = UNISTRING("getPropertyValues");
    aCommand.Argument = uno::makeAny( aProps );

    sal_Bool bCompressed = sal_False;
    rtl::OUString aContentEncoding;

    try
    {
        uno::Any aResult = xCommandProcessor->execute(aCommand, 0,
            static_cast < XCommandEnvironment *> (this));
        uno::Reference< sdbc::XRow > xPropList( aResult, uno::UNO_QUERY );
        if ( xPropList.is() ) {
            aContentEncoding = xPropList->getString(1);
            if( aContentEncoding.equalsAscii("gzip") ||  aContentEncoding.equalsAscii("deflate"))
                bCompressed = sal_True;
        }
    }
    catch( const uno::Exception &e )
    {
        OSL_TRACE( "Caught exception: %s\n",
            rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8).getStr() );
    }

    storeCommandInfo(0, uno::Reference< ucb::XCommandProcessor > ());

    uno::Reference< ucb::XCommandProcessor2 > xCommandProcessor2(xCommandProcessor, uno::UNO_QUERY);
    if( xCommandProcessor2.is() )
        xCommandProcessor2->releaseCommandIdentifier(nCommandId);

    if ( bCompressed )
        return INPUT_STREAM( new InflateInputStream( aSink->getInputStream(), aContentEncoding ) );
    else
        return INPUT_STREAM(aSink->getInputStream());
}

//------------------------------------------------------------------------------

uno::Reference< xml::dom::XElement >
UpdateInformationProvider::getDocumentRoot(
    uno::Reference< ucb::XContentIdentifierFactory > const & rxIdFactory,
    uno::Reference< ucb::XContentProvider > const & rxProvider,
    uno::Reference< xml::dom::XDocumentBuilder > const & rxBuilder,
    uno::Reference< xml::dom::XNode > const & rxNode
)
{
    uno::Reference< xml::dom::XElement > xElement(rxNode, uno::UNO_QUERY_THROW);

    // load the document referenced in 'src' attribute ..
    if( xElement->hasAttribute( UNISTRING("src") ) )
    {
        uno::Reference< xml::dom::XDocument > xUpdateXML = rxBuilder->parse(
            load(rxIdFactory, rxProvider, xElement->getAttribute( UNISTRING("src") )));

        OSL_ASSERT( xUpdateXML.is() );

        if( xUpdateXML.is() )
            return xUpdateXML->getDocumentElement();
    }
    // .. or return the (single) child element
    else
    {
        uno::Reference< xml::dom::XNodeList> xChildNodes = rxNode->getChildNodes();

        // ignore possible #text nodes
        sal_Int32 nmax = xChildNodes->getLength();
        for(sal_Int32 n=0; n < nmax; n++)
        {
            uno::Reference< xml::dom::XElement > xChildElement(xChildNodes->item(n), uno::UNO_QUERY);
            if( xChildElement.is() )
            {
                /* Copy the content to a dedicated document since XXPathAPI->selectNodeList
                 * seems to evaluate expression always relative to the root node.
                 */
                uno::Reference< xml::dom::XDocument > xUpdateXML = rxBuilder->newDocument();
                xUpdateXML->appendChild( xUpdateXML->importNode(xChildElement.get(), sal_True ) );
                return xUpdateXML->getDocumentElement();
            }
        }
    }

    return uno::Reference< xml::dom::XElement > ();
}

//------------------------------------------------------------------------------

uno::Sequence< uno::Reference< xml::dom::XElement > > SAL_CALL
UpdateInformationProvider::getUpdateInformation(
    uno::Sequence< rtl::OUString > const & repositories,
    rtl::OUString const & extensionId
) throw (uno::Exception, uno::RuntimeException)
{
    // if repository list is empty, try at default update URL
    if( repositories.getLength() == 0 )
    {
        uno::Sequence< rtl::OUString > aDefaultRepository(1);
        aDefaultRepository[0] = m_aUpdateURL;
        return getUpdateInformation(aDefaultRepository, extensionId);
    }

    uno::Reference< lang::XMultiComponentFactory > xServiceManager(m_xContext->getServiceManager());
    if( !xServiceManager.is() )
        throw uno::RuntimeException(
            UNISTRING( "unable to obtain service manager from component context" ), *this);

    uno::Reference< ucb::XContentIdentifierFactory > xIdFactory(
        xServiceManager->createInstanceWithContext( UNISTRING( "com.sun.star.ucb.UniversalContentBroker" ), m_xContext ),
        uno::UNO_QUERY_THROW);

    uno::Reference< ucb::XContentProvider > xProvider(xIdFactory, uno::UNO_QUERY_THROW);

    // reset cancelled flag
    m_bCancelled.reset();

    for(sal_Int32 n=0; n<repositories.getLength(); n++)
    {
        try
        {
            uno::Reference< xml::dom::XDocumentBuilder > xBuilder(
                xServiceManager->createInstanceWithContext( UNISTRING( "com.sun.star.xml.dom.DocumentBuilder" ), m_xContext ),
                uno::UNO_QUERY_THROW);

            uno::Reference< xml::dom::XDocument > xDocument = xBuilder->parse(load(xIdFactory, xProvider, repositories[n]));
            uno::Reference< xml::dom::XElement > xElement;

            if( xDocument.is() )
                xElement = xDocument->getDocumentElement();

            if( xElement.is() )
            {
                if( xElement->getNodeName().equalsAsciiL("feed", 4) )
                {
                    uno::Reference< xml::xpath::XXPathAPI > xXPath(
                        xServiceManager->createInstanceWithContext( UNISTRING( "com.sun.star.xml.xpath.XPathAPI" ), m_xContext ),
                        uno::UNO_QUERY_THROW);

                    xXPath->registerNS( UNISTRING("atom"), UNISTRING("http://www.w3.org/2005/Atom") );

                    rtl::OUString aXPathExpression;

                    if( extensionId.getLength() > 0 )
                        aXPathExpression = UNISTRING("//atom:entry/atom:category[@term=\'") + extensionId + UNISTRING("\']/../atom:content");
                    else
                        aXPathExpression = UNISTRING("//atom:entry/atom:content");

                    uno::Reference< xml::dom::XNodeList > xNodeList =
                        xXPath->selectNodeList(xDocument.get(), aXPathExpression);

                    sal_Int32 nElements = 0;
                    sal_Int32 nNodes = xNodeList->getLength();
                    uno::Sequence< uno::Reference< xml::dom::XElement > > aRet(nNodes);

                    for(sal_Int32 i=0; i < nNodes; i++)
                    {
                        try
                        {
                            uno::Reference< xml::dom::XElement > xRootElement =
                                getDocumentRoot(xIdFactory, xProvider, xBuilder, xNodeList->item(i));

                            if( xRootElement.is() )
                                aRet[nElements++] = xRootElement;
                        }

                        // return what we have got so far
                        catch( ucb::CommandAbortedException const & ) { break; }

                        // let runtime exception pass
                        catch( uno::RuntimeException const & ) { throw; }

                        // ignore files that can't be loaded
                        catch( uno::Exception const & ) { }
                    }

                    aRet.realloc(nElements);
                    return aRet;
                }
                else
                {
                    uno::Sequence< uno::Reference< xml::dom::XElement > > aRet(1);
                    aRet[0] = xElement;
                    return aRet;
                }
            }

            if( m_bCancelled.check() )
                break;
        }
        // rethrow runtime exceptions
        catch( uno::RuntimeException const & ) { throw; }

        // rethrow only if last url in the list
        catch( uno::Exception const & )
        {
            if( n+1 >= repositories.getLength() )
                throw;
        }
    }

    return uno::Sequence< uno::Reference< xml::dom::XElement > > ();
}

//------------------------------------------------------------------------------

sal_Bool SAL_CALL
UpdateInformationProvider::hasPredeterminedUpdateURL() throw (uno::RuntimeException)
{
    return (m_aUpdateURL.getLength() > 0) ? sal_True : sal_False;
}

//------------------------------------------------------------------------------

void SAL_CALL
UpdateInformationProvider::cancel() throw (uno::RuntimeException)
{
    m_bCancelled.set();

    osl::MutexGuard aGuard(m_aMutex);
    if( m_xCommandProcessor.is() )
        m_xCommandProcessor->abort(m_nCommandId);
}

//------------------------------------------------------------------------------

void SAL_CALL
UpdateInformationProvider::setInteractionHandler(
        uno::Reference< task::XInteractionHandler > const & handler )
    throw (uno::RuntimeException)
{
    osl::MutexGuard aGuard(m_aMutex);
    m_xInteractionHandler = handler;
}

//------------------------------------------------------------------------------

uno::Sequence< rtl::OUString >
UpdateInformationProvider::getServiceNames()
{
    uno::Sequence< rtl::OUString > aServiceList(1);
    aServiceList[0] = UNISTRING( "com.sun.star.deployment.UpdateInformationProvider");
    return aServiceList;
};

//------------------------------------------------------------------------------

rtl::OUString
UpdateInformationProvider::getImplName()
{
    return UNISTRING( "vnd.sun.UpdateInformationProvider");
}

//------------------------------------------------------------------------------

rtl::OUString SAL_CALL
UpdateInformationProvider::getImplementationName() throw (uno::RuntimeException)
{
    return getImplName();
}

//------------------------------------------------------------------------------

uno::Sequence< rtl::OUString > SAL_CALL
UpdateInformationProvider::getSupportedServiceNames() throw (uno::RuntimeException)
{
    return getServiceNames();
}

//------------------------------------------------------------------------------

sal_Bool SAL_CALL
UpdateInformationProvider::supportsService( rtl::OUString const & serviceName ) throw (uno::RuntimeException)
{
    uno::Sequence< rtl::OUString > aServiceNameList = getServiceNames();

    for( sal_Int32 n=0; n < aServiceNameList.getLength(); n++ )
        if( aServiceNameList[n].equals(serviceName) )
            return sal_True;

    return sal_False;
}

} // anonymous namespace

//------------------------------------------------------------------------------

static uno::Reference<uno::XInterface> SAL_CALL
createInstance(uno::Reference<uno::XComponentContext> const & xContext)
{
    return * new UpdateInformationProvider(xContext);
}

//------------------------------------------------------------------------------

static const cppu::ImplementationEntry kImplementations_entries[] =
{
    {
        createInstance,
        UpdateInformationProvider::getImplName,
        UpdateInformationProvider::getServiceNames,
        cppu::createSingleComponentFactory,
        NULL,
        0
    },
    { NULL, NULL, NULL, NULL, NULL, 0 }
} ;

//------------------------------------------------------------------------------

extern "C" void SAL_CALL
component_getImplementationEnvironment( const sal_Char **aEnvTypeName, uno_Environment **)
{
    *aEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME ;
}

//------------------------------------------------------------------------------

extern "C" sal_Bool SAL_CALL
component_writeInfo(void *pServiceManager, void *pRegistryKey)
{
    return cppu::component_writeInfoHelper(
        pServiceManager,
        pRegistryKey,
        kImplementations_entries
    );
}

//------------------------------------------------------------------------------

extern "C" void *
component_getFactory(const sal_Char *pszImplementationName, void *pServiceManager, void *pRegistryKey)
{
    return cppu::component_getFactoryHelper(
        pszImplementationName,
        pServiceManager,
        pRegistryKey,
        kImplementations_entries) ;
}

