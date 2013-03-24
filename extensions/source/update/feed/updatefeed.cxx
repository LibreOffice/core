/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase3.hxx>
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
#include <com/sun/star/xml/xpath/XPathAPI.hpp>

#include <rtl/ref.hxx>
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
    public ::cppu::WeakImplHelper3< deployment::XUpdateInformationProvider,
                                    ucb::XWebDAVCommandEnvironment,
                                    lang::XServiceInfo >
{
public:
    static uno::Reference< uno::XInterface > createInstance(const uno::Reference<uno::XComponentContext>& xContext);

    static uno::Sequence< OUString > getServiceNames();
    static OUString getImplName();

    uno::Reference< xml::dom::XElement > getDocumentRoot(const uno::Reference< xml::dom::XNode >& rxNode);
    uno::Reference< xml::dom::XNode > getChildNode(const uno::Reference< xml::dom::XNode >& rxNode, const OUString& rName);


    // XUpdateInformationService
    virtual uno::Sequence< uno::Reference< xml::dom::XElement > > SAL_CALL
    getUpdateInformation(
        uno::Sequence< OUString > const & repositories,
        OUString const & extensionId
    ) throw (uno::Exception, uno::RuntimeException);

    virtual void SAL_CALL cancel()
        throw (uno::RuntimeException);

    virtual void SAL_CALL setInteractionHandler(
        uno::Reference< task::XInteractionHandler > const & handler )
        throw (uno::RuntimeException);

    virtual uno::Reference< container::XEnumeration > SAL_CALL
    getUpdateInformationEnumeration(
        uno::Sequence< OUString > const & repositories,
        OUString const & extensionId
    ) throw (uno::Exception, uno::RuntimeException);

    // XCommandEnvironment
    virtual uno::Reference< task::XInteractionHandler > SAL_CALL getInteractionHandler()
        throw ( uno::RuntimeException );

    virtual uno::Reference< ucb::XProgressHandler > SAL_CALL getProgressHandler()
        throw ( uno::RuntimeException ) { return  uno::Reference< ucb::XProgressHandler >(); };

    // XWebDAVCommandEnvironment
    virtual uno::Sequence< beans::StringPair > SAL_CALL getUserRequestHeaders(
        const OUString&, const OUString& )
        throw ( uno::RuntimeException ) { return m_aRequestHeaderList; };

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService(OUString const & serviceName)
        throw (uno::RuntimeException);
    virtual uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw (uno::RuntimeException);

protected:

    virtual ~UpdateInformationProvider();
    static OUString getConfigurationItem(uno::Reference<lang::XMultiServiceFactory> const & configurationProvider, OUString const & node, OUString const & item);

private:
    uno::Reference< io::XInputStream > load(const OUString& rURL);

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

    uno::Sequence< beans::StringPair > m_aRequestHeaderList;

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
            throw container::NoSuchElementException(OUString::valueOf(m_nCount), *this);

        try
        {
            deployment::UpdateInformationEntry aEntry;

            uno::Reference< xml::dom::XNode > xAtomEntryNode( m_xNodeList->item(m_nCount++) );

            uno::Reference< xml::dom::XNode > xSummaryNode(
                m_xUpdateInformationProvider->getChildNode( xAtomEntryNode, "summary/text()" )
            );

            if( xSummaryNode.is() )
                aEntry.Description = xSummaryNode->getNodeValue();

            uno::Reference< xml::dom::XNode > xContentNode(
                m_xUpdateInformationProvider->getChildNode( xAtomEntryNode, "content" ) );

            if( xContentNode.is() )
                aEntry.UpdateDocument = m_xUpdateInformationProvider->getDocumentRoot(xContentNode);

            return uno::makeAny(aEntry);
        }

        // action has been aborted
        catch( ucb::CommandAbortedException const & e)
            { throw lang::WrappedTargetException( "Command aborted", *this, uno::makeAny(e) ); }

        // let runtime exception pass
        catch( uno::RuntimeException const & ) { throw; }

        // document not accessible
        catch( uno::Exception const & e)
            { throw lang::WrappedTargetException( "Document not accessible", *this, uno::makeAny(e) ); }
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
            throw container::NoSuchElementException(OUString::valueOf(m_nCount), *this);

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

    OUStringBuffer buf;
    buf.append(
        getConfigurationItem(
            xConfigurationProvider,
            "org.openoffice.Setup/Product",
            "ooName"));
    buf.append(sal_Unicode(' '));
    buf.append(
        getConfigurationItem(
            xConfigurationProvider,
            "org.openoffice.Setup/Product",
            "ooSetupVersion"));
    OUString extension(
        getConfigurationItem(
            xConfigurationProvider,
            "org.openoffice.Setup/Product",
            "ooSetupExtension"));
    if (!extension.isEmpty()) {
        buf.append(extension);
    }
    OUString product(buf.makeStringAndClear());

    OUString aUserAgent( "${$BRAND_BASE_DIR/program/" SAL_CONFIGFILE("version") ":UpdateUserAgent}" );
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

    SAL_INFO("extensions.update", "UpdateUserAgent: " << aUserAgent);

    m_aRequestHeaderList[0].First = "Accept-Language";
    m_aRequestHeaderList[0].Second = getConfigurationItem( xConfigurationProvider, "org.openoffice.Setup/L10N", "ooLocale" );
    if( !aUserAgent.isEmpty() )
    {
        m_aRequestHeaderList.realloc(2);
        m_aRequestHeaderList[1].First = "User-Agent";
        m_aRequestHeaderList[1].Second = aUserAgent;
    }
}

//------------------------------------------------------------------------------
uno::Reference< uno::XInterface >
UpdateInformationProvider::createInstance(const uno::Reference<uno::XComponentContext>& xContext)
{
    uno::Reference< ucb::XUniversalContentBroker > xUniversalContentBroker =
        ucb::UniversalContentBroker::create(xContext);

    uno::Reference< xml::dom::XDocumentBuilder > xDocumentBuilder(
        xml::dom::DocumentBuilder::create(xContext));

    uno::Reference< xml::xpath::XXPathAPI > xXPath = xml::xpath::XPathAPI::create( xContext );

    xXPath->registerNS( "atom", "http://www.w3.org/2005/Atom" );

    return *new UpdateInformationProvider(xContext, xUniversalContentBroker, xDocumentBuilder, xXPath);
}

//------------------------------------------------------------------------------

UpdateInformationProvider::~UpdateInformationProvider()
{
}

//------------------------------------------------------------------------------

OUString
UpdateInformationProvider::getConfigurationItem(uno::Reference<lang::XMultiServiceFactory> const & configurationProvider, OUString const & node, OUString const & item)
{
    rtl::OUString sRet;
    beans::PropertyValue aProperty;
    aProperty.Name  = "nodepath";
    aProperty.Value = uno::makeAny(node);

    uno::Sequence< uno::Any > aArgumentList( 1 );
    aArgumentList[0] = uno::makeAny( aProperty );

    uno::Reference< container::XNameAccess > xNameAccess(
        configurationProvider->createInstanceWithArguments(
            "com.sun.star.configuration.ConfigurationAccess",
            aArgumentList ),
        uno::UNO_QUERY_THROW);

    xNameAccess->getByName(item) >>= sRet;
    return sRet;
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
UpdateInformationProvider::load(const OUString& rURL)
{
    uno::Reference< ucb::XContentIdentifier > xId = m_xUniversalContentBroker->createContentIdentifier(rURL);

    if( !xId.is() )
        throw uno::RuntimeException(
            "unable to obtain universal content id", *this);

    uno::Reference< ucb::XCommandProcessor > xCommandProcessor(m_xUniversalContentBroker->queryContent(xId), uno::UNO_QUERY_THROW);
    rtl::Reference< ActiveDataSink > aSink(new ActiveDataSink());

    // Disable KeepAlive in webdav - don't want millions of office
    // instances phone home & clog up servers
    uno::Sequence< beans::NamedValue > aProps( 1 );
    aProps[ 0 ] = beans::NamedValue(
        "KeepAlive", uno::makeAny(sal_False));

    ucb::OpenCommandArgument3 aOpenArgument;
    aOpenArgument.Mode = ucb::OpenMode::DOCUMENT;
    aOpenArgument.Priority = 32768;
    aOpenArgument.Sink = *aSink;
    aOpenArgument.OpeningFlags = aProps;

    ucb::Command aCommand;
    aCommand.Name = "open";
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
    if( xElement->hasAttribute( "src" ) )
    {
        uno::Reference< xml::dom::XDocument > xUpdateXML =
            m_xDocumentBuilder->parse(load(xElement->getAttribute( "src" )));

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
                                        const OUString& rName)
{
    OSL_ASSERT(m_xXPathAPI.is());
    try {
        return m_xXPathAPI->selectSingleNode(rxNode, "./atom:" + rName);
    } catch (const xml::xpath::XPathException &) {
        // ignore
        return 0;
    }
}

//------------------------------------------------------------------------------

uno::Reference< container::XEnumeration > SAL_CALL
UpdateInformationProvider::getUpdateInformationEnumeration(
    uno::Sequence< OUString > const & repositories,
    OUString const & extensionId
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
                    OUString aXPathExpression;

                    if( !extensionId.isEmpty() )
                        aXPathExpression = "//atom:entry/atom:category[@term=\'" + extensionId + "\']/..";
                    else
                        aXPathExpression = "//atom:entry";

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
        catch( uno::RuntimeException const& /*e*/)
        {
            // #i118675# ignore runtime exceptions for now
            // especially the "unsatisfied query for interface of
            // type com.sun.star.ucb.XCommandProcessor!" exception
        }

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
    uno::Sequence< OUString > const & repositories,
    OUString const & extensionId
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

uno::Sequence< OUString >
UpdateInformationProvider::getServiceNames()
{
    uno::Sequence< OUString > aServiceList(1);
    aServiceList[0] = "com.sun.star.deployment.UpdateInformationProvider";
    return aServiceList;
};

//------------------------------------------------------------------------------

OUString
UpdateInformationProvider::getImplName()
{
    return OUString("vnd.sun.UpdateInformationProvider");
}

//------------------------------------------------------------------------------

OUString SAL_CALL
UpdateInformationProvider::getImplementationName() throw (uno::RuntimeException)
{
    return getImplName();
}

//------------------------------------------------------------------------------

uno::Sequence< OUString > SAL_CALL
UpdateInformationProvider::getSupportedServiceNames() throw (uno::RuntimeException)
{
    return getServiceNames();
}

//------------------------------------------------------------------------------

sal_Bool SAL_CALL
UpdateInformationProvider::supportsService( OUString const & serviceName ) throw (uno::RuntimeException)
{
    uno::Sequence< OUString > aServiceNameList = getServiceNames();

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

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL updatefeed_component_getFactory(const sal_Char *pszImplementationName, void *pServiceManager, void *pRegistryKey)
{
    return cppu::component_getFactoryHelper(
        pszImplementationName,
        pServiceManager,
        pRegistryKey,
        kImplementations_entries) ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
