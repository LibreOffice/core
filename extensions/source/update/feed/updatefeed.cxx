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


#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/deployment/UpdateInformationEntry.hpp>
#include <com/sun/star/deployment/UpdateInformationProvider.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XWebDAVCommandEnvironment.hpp>
#include <com/sun/star/ucb/XCommandProcessor2.hpp>
#include <com/sun/star/ucb/XContentIdentifierFactory.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include "com/sun/star/ucb/XInteractionSupplyAuthentication.hpp"
#include <com/sun/star/ucb/OpenCommandArgument3.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/task/PasswordContainerInteractionHandler.hpp>
#include <com/sun/star/xml/dom/DocumentBuilder.hpp>
#include <com/sun/star/xml/xpath/XXPathAPI.hpp>

#include <rtl/ref.hxx>
#include <rtl/memory.h>
#include <rtl/bootstrap.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/process.h>
#include <osl/conditn.hxx>

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
            if ( n )
                OSL_TRACE( "Read [%d] bytes: %s", n, aData.get()->elements );
            return n;
        };
    virtual sal_Int32 SAL_CALL readSomeBytes(uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead)
        throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
        {
            sal_Int32 n = m_xStream->readSomeBytes(aData, nMaxBytesToRead);
            if ( n )
                OSL_TRACE( "Read [%d] bytes: %s", n, aData.get()->elements );
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

class UpdateInformationProvider :
    public ::cppu::WeakImplHelper4< deployment::XUpdateInformationProvider,
                                    ucb::XCommandEnvironment,
                                    ucb::XWebDAVCommandEnvironment,
                                    lang::XServiceInfo >
{
public:
    static uno::Reference< uno::XInterface > createInstance(const uno::Reference<uno::XComponentContext>& xContext);

    static uno::Sequence< rtl::OUString > getServiceNames();
    static rtl::OUString getImplName();

    uno::Reference< xml::dom::XElement > getDocumentRoot(const uno::Reference< xml::dom::XNode >& rxNode);
    uno::Reference< xml::dom::XNode > getChildNode(const uno::Reference< xml::dom::XNode >& rxNode, const rtl::OUString& rName);


    // XUpdateInformationService
    virtual uno::Sequence< uno::Reference< xml::dom::XElement > > SAL_CALL
    getUpdateInformation(
        uno::Sequence< rtl::OUString > const & repositories,
        rtl::OUString const & extensionId
    ) throw (uno::Exception, uno::RuntimeException);

    virtual void SAL_CALL cancel()
        throw (uno::RuntimeException);

    virtual void SAL_CALL setInteractionHandler(
        uno::Reference< task::XInteractionHandler > const & handler )
        throw (uno::RuntimeException);

    virtual uno::Reference< container::XEnumeration > SAL_CALL
    getUpdateInformationEnumeration(
        uno::Sequence< rtl::OUString > const & repositories,
        rtl::OUString const & extensionId
    ) throw (uno::Exception, uno::RuntimeException);

    // XCommandEnvironment
    virtual uno::Reference< task::XInteractionHandler > SAL_CALL getInteractionHandler()
        throw ( uno::RuntimeException );

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

protected:

    virtual ~UpdateInformationProvider();
    static uno::Any getConfigurationItem(uno::Reference<lang::XMultiServiceFactory> const & configurationProvider, rtl::OUString const & node, rtl::OUString const & item);

private:
    uno::Reference< io::XInputStream > load(const rtl::OUString& rURL);

    void storeCommandInfo( sal_Int32 nCommandId,
        uno::Reference< ucb::XCommandProcessor > const & rxCommandProcessor);

    UpdateInformationProvider(const uno::Reference<uno::XComponentContext>& xContext,
                              const uno::Reference< ucb::XUniversalContentBroker >& xUniversalContentBroker,
                              const uno::Reference< xml::dom::XDocumentBuilder >& xDocumentBuilder,
                              const uno::Reference< xml::xpath::XXPathAPI >& xXPathAPI);

    const uno::Reference< uno::XComponentContext> m_xContext;

    const uno::Reference< ucb::XUniversalContentBroker > m_xUniversalContentBroker;
    const uno::Reference< xml::dom::XDocumentBuilder > m_xDocumentBuilder;
    const uno::Reference< xml::xpath::XXPathAPI > m_xXPathAPI;

    uno::Sequence< beans::NamedValue > m_aRequestHeaderList;

    uno::Reference< ucb::XCommandProcessor > m_xCommandProcessor;
    uno::Reference< task::XInteractionHandler > m_xInteractionHandler;
    uno::Reference< task::XInteractionHandler > m_xPwContainerInteractionHandler;

    osl::Mutex m_aMutex;
    osl::Condition m_bCancelled;

    sal_Int32 m_nCommandId;
};

//------------------------------------------------------------------------------

class UpdateInformationEnumeration : public ::cppu::WeakImplHelper1< container::XEnumeration >
{
public:
    UpdateInformationEnumeration(const uno::Reference< xml::dom::XNodeList >& xNodeList,
                                 const uno::Reference< UpdateInformationProvider > xUpdateInformationProvider) :
        m_xUpdateInformationProvider(xUpdateInformationProvider),
        m_xNodeList(xNodeList),
        m_nNodes(xNodeList.is() ? xNodeList->getLength() : 0),
        m_nCount(0)
    {
    };

    virtual ~UpdateInformationEnumeration() {};

    // XEnumeration
    sal_Bool SAL_CALL hasMoreElements() throw (uno::RuntimeException) { return m_nCount < m_nNodes; };
    uno::Any SAL_CALL nextElement() throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        OSL_ASSERT( m_xNodeList.is() );
        OSL_ASSERT( m_xUpdateInformationProvider.is() );

        if( !(m_nCount < m_nNodes ) )
            throw container::NoSuchElementException(rtl::OUString::valueOf(m_nCount), *this);

        try
        {
            deployment::UpdateInformationEntry aEntry;

            uno::Reference< xml::dom::XNode > xAtomEntryNode( m_xNodeList->item(m_nCount++) );

            uno::Reference< xml::dom::XNode > xSummaryNode(
                m_xUpdateInformationProvider->getChildNode( xAtomEntryNode, UNISTRING( "summary/text()" ) )
            );

            if( xSummaryNode.is() )
                aEntry.Description = xSummaryNode->getNodeValue();

            uno::Reference< xml::dom::XNode > xContentNode(
                m_xUpdateInformationProvider->getChildNode( xAtomEntryNode, UNISTRING( "content" ) ) );

            if( xContentNode.is() )
                aEntry.UpdateDocument = m_xUpdateInformationProvider->getDocumentRoot(xContentNode);

            return uno::makeAny(aEntry);
        }

        // action has been aborted
        catch( ucb::CommandAbortedException const & e)
            { throw lang::WrappedTargetException( UNISTRING( "Command aborted" ), *this, uno::makeAny(e) ); }

        // let runtime exception pass
        catch( uno::RuntimeException const & ) { throw; }

        // document not accessible
        catch( uno::Exception const & e)
            { throw lang::WrappedTargetException( UNISTRING( "Document not accessible" ), *this, uno::makeAny(e) ); }
    }

private:
    const uno::Reference< UpdateInformationProvider > m_xUpdateInformationProvider;
    const uno::Reference< xml::dom::XNodeList > m_xNodeList;
    const sal_Int32 m_nNodes;
    sal_Int32 m_nCount;
};

//------------------------------------------------------------------------------

class SingleUpdateInformationEnumeration : public ::cppu::WeakImplHelper1< container::XEnumeration >
{
public:
    SingleUpdateInformationEnumeration(const uno::Reference< xml::dom::XElement >& xElement)
        : m_nCount(0) { m_aEntry.UpdateDocument = xElement; };
    virtual ~SingleUpdateInformationEnumeration() {};

    // XEnumeration
    sal_Bool SAL_CALL hasMoreElements() throw (uno::RuntimeException) { return 0 == m_nCount; };
    uno::Any SAL_CALL nextElement() throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if( m_nCount > 0 )
            throw container::NoSuchElementException(rtl::OUString::valueOf(m_nCount), *this);

        ++m_nCount;
        return uno::makeAny(m_aEntry);
    };

private:
    sal_uInt8 m_nCount;
    deployment::UpdateInformationEntry m_aEntry;
};


//------------------------------------------------------------------------------

UpdateInformationProvider::UpdateInformationProvider(
    const uno::Reference<uno::XComponentContext>& xContext,
    const uno::Reference< ucb::XUniversalContentBroker >& xUniversalContentBroker,
    const uno::Reference< xml::dom::XDocumentBuilder >& xDocumentBuilder,
    const uno::Reference< xml::xpath::XXPathAPI >& xXPathAPI
) : m_xContext(xContext), m_xUniversalContentBroker(xUniversalContentBroker),
    m_xDocumentBuilder(xDocumentBuilder),
    m_xXPathAPI(xXPathAPI), m_aRequestHeaderList(1)
{
    uno::Reference< lang::XMultiServiceFactory > xConfigurationProvider(
        com::sun::star::configuration::theDefaultProvider::get(xContext));

    rtl::OUStringBuffer buf;
    rtl::OUString name;
    getConfigurationItem(
        xConfigurationProvider,
        UNISTRING("org.openoffice.Setup/Product"),
        UNISTRING("ooName")) >>= name;
    buf.append(name);
    buf.append(sal_Unicode(' '));
    rtl::OUString version;
    getConfigurationItem(
        xConfigurationProvider,
        UNISTRING("org.openoffice.Setup/Product"),
        UNISTRING("ooSetupVersion")) >>= version;
    buf.append(version);
    rtl::OUString edition(
        UNISTRING(
            "${${BRAND_BASE_DIR}/program/edition/edition.ini:"
            "EDITIONNAME}"));
    rtl::Bootstrap::expandMacros(edition);
    if (!edition.isEmpty()) {
        buf.append(sal_Unicode(' '));
        buf.append(edition);
    }
    rtl::OUString extension;
    getConfigurationItem(
        xConfigurationProvider,
        UNISTRING("org.openoffice.Setup/Product"),
        UNISTRING("ooSetupExtension")) >>= extension;
    if (!extension.isEmpty()) {
        buf.append(sal_Unicode(' '));
        buf.append(extension);
    }
    rtl::OUString product(buf.makeStringAndClear());

    rtl::OUString aUserAgent( UNISTRING( "${$BRAND_BASE_DIR/program/" SAL_CONFIGFILE("version") ":UpdateUserAgent}" ) );
    rtl::Bootstrap::expandMacros( aUserAgent );

    for (sal_Int32 i = 0;;) {
        i = aUserAgent.indexOfAsciiL(
            RTL_CONSTASCII_STRINGPARAM("<PRODUCT>"), i);
        if (i == -1) {
            break;
        }
        aUserAgent = aUserAgent.replaceAt(
            i, RTL_CONSTASCII_LENGTH("<PRODUCT>"), product);
        i += product.getLength();
    }

    m_aRequestHeaderList[0].Name = UNISTRING("Accept-Language");
    m_aRequestHeaderList[0].Value = getConfigurationItem( xConfigurationProvider, UNISTRING("org.openoffice.Setup/L10N"), UNISTRING("ooLocale") );
    if( !aUserAgent.isEmpty() )
    {
        m_aRequestHeaderList.realloc(2);
        m_aRequestHeaderList[1].Name = UNISTRING("User-Agent");
        m_aRequestHeaderList[1].Value = uno::makeAny(aUserAgent);
    }
}

//------------------------------------------------------------------------------
uno::Reference< uno::XInterface >
UpdateInformationProvider::createInstance(const uno::Reference<uno::XComponentContext>& xContext)
{
    uno::Reference< lang::XMultiComponentFactory > xServiceManager(xContext->getServiceManager());
    if( !xServiceManager.is() )
        throw uno::RuntimeException(
            UNISTRING( "unable to obtain service manager from component context" ),
            uno::Reference< uno::XInterface > ());

    uno::Reference< ucb::XUniversalContentBroker > xUniversalContentBroker =
        ucb::UniversalContentBroker::create(xContext);

    uno::Reference< xml::dom::XDocumentBuilder > xDocumentBuilder(
        xml::dom::DocumentBuilder::create(xContext));

    uno::Reference< xml::xpath::XXPathAPI > xXPath(
        xServiceManager->createInstanceWithContext( UNISTRING( "com.sun.star.xml.xpath.XPathAPI" ), xContext ),
        uno::UNO_QUERY_THROW);

    xXPath->registerNS( UNISTRING("atom"), UNISTRING("http://www.w3.org/2005/Atom") );

    return *new UpdateInformationProvider(xContext, xUniversalContentBroker, xDocumentBuilder, xXPath);
}

//------------------------------------------------------------------------------

UpdateInformationProvider::~UpdateInformationProvider()
{
}

//------------------------------------------------------------------------------

uno::Any
UpdateInformationProvider::getConfigurationItem(uno::Reference<lang::XMultiServiceFactory> const & configurationProvider, rtl::OUString const & node, rtl::OUString const & item)
{
    beans::NamedValue aProperty;
    aProperty.Name  = UNISTRING("nodepath");
    aProperty.Value = uno::makeAny(node);

    uno::Sequence< uno::Any > aArgumentList( 1 );
    aArgumentList[0] = uno::makeAny( aProperty );

    uno::Reference< container::XNameAccess > xNameAccess(
        configurationProvider->createInstanceWithArguments(
            UNISTRING("com.sun.star.configuration.ConfigurationAccess"),
            aArgumentList ),
        uno::UNO_QUERY_THROW);

    return xNameAccess->getByName(item);
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
UpdateInformationProvider::load(const rtl::OUString& rURL)
{
    uno::Reference< ucb::XContentIdentifier > xId = m_xUniversalContentBroker->createContentIdentifier(rURL);

    if( !xId.is() )
        throw uno::RuntimeException(
            UNISTRING( "unable to obtain universal content id" ), *this);

    uno::Reference< ucb::XCommandProcessor > xCommandProcessor(m_xUniversalContentBroker->queryContent(xId), uno::UNO_QUERY_THROW);
    rtl::Reference< ActiveDataSink > aSink(new ActiveDataSink());

    // Disable KeepAlive in webdav - don't want millions of office
    // instances phone home & clog up servers
    uno::Sequence< beans::NamedValue > aProps( 1 );
    aProps[ 0 ] = beans::NamedValue(
        UNISTRING("KeepAlive"), uno::makeAny(sal_False));

    ucb::OpenCommandArgument3 aOpenArgument;
    aOpenArgument.Mode = ucb::OpenMode::DOCUMENT;
    aOpenArgument.Priority = 32768;
    aOpenArgument.Sink = *aSink;
    aOpenArgument.OpeningFlags = aProps;

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
    catch( const uno::Exception & /* e */ )
    {
        storeCommandInfo(0, uno::Reference< ucb::XCommandProcessor > ());

        uno::Reference< ucb::XCommandProcessor2 > xCommandProcessor2(xCommandProcessor, uno::UNO_QUERY);
        if( xCommandProcessor2.is() )
            xCommandProcessor2->releaseCommandIdentifier(nCommandId);

        throw;
    }
    storeCommandInfo(0, uno::Reference< ucb::XCommandProcessor > ());

    uno::Reference< ucb::XCommandProcessor2 > xCommandProcessor2(xCommandProcessor, uno::UNO_QUERY);
    if( xCommandProcessor2.is() )
        xCommandProcessor2->releaseCommandIdentifier(nCommandId);

    return INPUT_STREAM(aSink->getInputStream());
}

//------------------------------------------------------------------------------

// TODO: docu content node

uno::Reference< xml::dom::XElement >
UpdateInformationProvider::getDocumentRoot(const uno::Reference< xml::dom::XNode >& rxNode)
{
    OSL_ASSERT(m_xDocumentBuilder.is());

    uno::Reference< xml::dom::XElement > xElement(rxNode, uno::UNO_QUERY_THROW);

    // load the document referenced in 'src' attribute ..
    if( xElement->hasAttribute( UNISTRING("src") ) )
    {
        uno::Reference< xml::dom::XDocument > xUpdateXML =
            m_xDocumentBuilder->parse(load(xElement->getAttribute( UNISTRING("src") )));

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
                uno::Reference< xml::dom::XDocument > xUpdateXML = m_xDocumentBuilder->newDocument();
                xUpdateXML->appendChild( xUpdateXML->importNode(xChildElement.get(), sal_True ) );
                return xUpdateXML->getDocumentElement();
            }
        }
    }

    return uno::Reference< xml::dom::XElement > ();
}

//------------------------------------------------------------------------------

uno::Reference< xml::dom::XNode >
UpdateInformationProvider::getChildNode(const uno::Reference< xml::dom::XNode >& rxNode,
                                        const rtl::OUString& rName)
{
    OSL_ASSERT(m_xXPathAPI.is());
    try {
        return m_xXPathAPI->selectSingleNode(rxNode, UNISTRING( "./atom:" ) + rName);
    } catch (const xml::xpath::XPathException &) {
        // ignore
        return 0;
    }
}

//------------------------------------------------------------------------------

uno::Reference< container::XEnumeration > SAL_CALL
UpdateInformationProvider::getUpdateInformationEnumeration(
    uno::Sequence< rtl::OUString > const & repositories,
    rtl::OUString const & extensionId
) throw (uno::Exception, uno::RuntimeException)
{
    OSL_ASSERT(m_xDocumentBuilder.is());

    // reset cancelled flag
    m_bCancelled.reset();

    for(sal_Int32 n=0; n<repositories.getLength(); n++)
    {
        try
        {
            uno::Reference< xml::dom::XDocument > xDocument = m_xDocumentBuilder->parse(load(repositories[n]));
            uno::Reference< xml::dom::XElement > xElement;

            if( xDocument.is() )
                xElement = xDocument->getDocumentElement();

            if( xElement.is() )
            {
                if( xElement->getNodeName().equalsAsciiL("feed", 4) )
                {
                    rtl::OUString aXPathExpression;

                    if( !extensionId.isEmpty() )
                        aXPathExpression = UNISTRING("//atom:entry/atom:category[@term=\'") + extensionId + UNISTRING("\']/..");
                    else
                        aXPathExpression = UNISTRING("//atom:entry");

                    uno::Reference< xml::dom::XNodeList > xNodeList;
                    try {
                        xNodeList = m_xXPathAPI->selectNodeList(xDocument.get(),
                            aXPathExpression);
                    } catch (const xml::xpath::XPathException &) {
                        // ignore
                    }

                    return new UpdateInformationEnumeration(xNodeList, this);
                }
                else
                {
                    return new SingleUpdateInformationEnumeration(xElement);
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

    return uno::Reference< container::XEnumeration >();
}

//------------------------------------------------------------------------------

uno::Sequence< uno::Reference< xml::dom::XElement > > SAL_CALL
UpdateInformationProvider::getUpdateInformation(
    uno::Sequence< rtl::OUString > const & repositories,
    rtl::OUString const & extensionId
) throw (uno::Exception, uno::RuntimeException)
{
    uno::Reference< container::XEnumeration > xEnumeration(
        getUpdateInformationEnumeration(repositories, extensionId)
    );

    uno::Sequence< uno::Reference< xml::dom::XElement > > aRet;

    if( xEnumeration.is() )
    {
        while( xEnumeration->hasMoreElements() )
        {
            try
            {
                deployment::UpdateInformationEntry aEntry;
                if( (xEnumeration->nextElement() >>= aEntry ) && aEntry.UpdateDocument.is() )
                {
                    sal_Int32 n = aRet.getLength();
                    aRet.realloc(n + 1);
                    aRet[n] = aEntry.UpdateDocument;
                }
            }

            catch( const lang::WrappedTargetException& e )
            {
                // command aborted, return what we have got so far
                if( e.TargetException.isExtractableTo( ::cppu::UnoType< ::com::sun::star::ucb::CommandAbortedException >::get() ) )
                {
                    break;
                }

                // ignore files that can't be loaded
            }
        }
    }

    return aRet;
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

uno::Reference< task::XInteractionHandler > SAL_CALL
UpdateInformationProvider::getInteractionHandler()
    throw ( uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( m_xInteractionHandler.is() )
        return m_xInteractionHandler;
    else
    {
        try
        {
            // Supply an interaction handler that uses the password container
            // service to obtain credentials without displaying a password gui.

            if ( !m_xPwContainerInteractionHandler.is() )
                m_xPwContainerInteractionHandler
                    = task::PasswordContainerInteractionHandler::create(
                        m_xContext );
        }
        catch ( uno::RuntimeException const & )
        {
            throw;
        }
        catch ( uno::Exception const & )
        {
        }
        return m_xPwContainerInteractionHandler;
    }
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
    return UpdateInformationProvider::createInstance(xContext);
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

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(const sal_Char *pszImplementationName, void *pServiceManager, void *pRegistryKey)
{
    return cppu::component_getFactoryHelper(
        pszImplementationName,
        pServiceManager,
        pRegistryKey,
        kImplementations_entries) ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
