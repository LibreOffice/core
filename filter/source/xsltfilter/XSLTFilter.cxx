/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 **************************************************************/

 // MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

#include <stdio.h>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/implbase.hxx>

#include <osl/time.h>
#include <osl/conditn.h>
#include <tools/urlobj.hxx>
#include <osl/module.h>
#include <osl/file.hxx>
#include <osl/process.h>

#include <com/sun/star/lang/XComponent.hpp>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Type.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>

#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/XImportFilter.hpp>
#include <com/sun/star/xml/XExportFilter.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XActiveDataControl.hpp>
#include <com/sun/star/io/XStreamListener.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/util/XStringSubstitution.hpp>
#include <com/sun/star/beans/NamedValue.hpp>


#include "uof2splitter.hxx"
#include "uof2storage.hxx"
#include "uof2merge.hxx"

using namespace ::rtl;
using namespace ::cppu;
using namespace ::osl;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::xml;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::util;

namespace XSLT {


class XSLTFilter : public WeakImplHelper4< XImportFilter, XExportFilter, XDocumentHandler, XStreamListener>
{
public:

    // ctor...
    XSLTFilter( const Reference< XMultiServiceFactory > &r );

    // XStreamListener
    virtual void SAL_CALL error(const Any& a) throw (RuntimeException);
    virtual void SAL_CALL closed() throw (RuntimeException);
    virtual void SAL_CALL terminated() throw (RuntimeException);
    virtual void SAL_CALL started() throw (RuntimeException);
    virtual void SAL_CALL disposing(const EventObject& e) throw (RuntimeException);


    // XImportFilter
    virtual sal_Bool SAL_CALL importer(
            const Sequence<PropertyValue>& aSourceData,
            const Reference<XDocumentHandler>& xHandler,
            const Sequence<OUString>& msUserData)
        throw(RuntimeException);

    // XExportFilter
    virtual sal_Bool SAL_CALL exporter(
            const Sequence<PropertyValue>& aSourceData,
            const Sequence<OUString>& msUserData)
        throw(RuntimeException);

    // XDocumentHandler
    virtual void SAL_CALL startDocument()
        throw (SAXException,RuntimeException);
    virtual void SAL_CALL endDocument()
        throw (SAXException, RuntimeException);
    virtual void SAL_CALL startElement(const OUString& str, const Reference<XAttributeList>& attriblist)
        throw (SAXException,RuntimeException);
    virtual void SAL_CALL endElement(const OUString& str)
        throw (SAXException, RuntimeException);
    virtual void SAL_CALL characters(const OUString& str)
        throw (SAXException, RuntimeException);
    virtual void SAL_CALL ignorableWhitespace(const OUString& str)
        throw (SAXException, RuntimeException);
    virtual void SAL_CALL processingInstruction(const OUString& str, const OUString& str2)
        throw (com::sun::star::xml::sax::SAXException,RuntimeException);
    virtual void SAL_CALL setDocumentLocator(const Reference<XLocator>& doclocator)
        throw (SAXException,RuntimeException);

private:
    // the UNO ServiceFactory
    Reference< XMultiServiceFactory > m_rServiceFactory;

    // DocumentHandler interface of the css::xml::sax::Writer service
    Reference < XExtendedDocumentHandler > m_rDocumentHandler;
    Reference < XOutputStream > m_rOutputStream;

    // controls pretty-printing
    sal_Bool m_bPrettyPrint;

    Reference< XActiveDataControl > m_tcontrol;
    oslCondition m_cTransformed;

    sal_Bool m_bTerminated;
    sal_Bool m_bError;

    OUString m_aExportBaseUrl;
    OUString m_aOldBaseUrl;

    OUString rel2abs(const OUString&);
    OUString expandUrl(const OUString&);

    // for support of UOF v2.0
    Reference< XActiveDataControl > m_splitControl;
    Reference< XStream > m_rStream;

    bool isUOF2ExportStyleSheet( const OUString& rExportStyleSheet );

};

XSLTFilter::XSLTFilter( const Reference< XMultiServiceFactory > &r )
    : m_rServiceFactory( r )
    , m_rDocumentHandler()
    , m_rOutputStream()
    , m_bPrettyPrint( sal_True )
    , m_tcontrol()
    , m_cTransformed()
    , m_bTerminated( sal_False )
    , m_bError( sal_False )
    , m_aExportBaseUrl()
    , m_aOldBaseUrl()
    , m_splitControl()
    , m_rStream()
{
    m_cTransformed = osl_createCondition();
}

void XSLTFilter::disposing(const EventObject& ) throw (RuntimeException)
{
}

::rtl::OUString XSLTFilter::expandUrl( const ::rtl::OUString& sUrl )
{
    ::rtl::OUString sExpandedUrl;
    try
    {
        Reference< XComponentContext > xContext;
        Reference< XPropertySet > xProps( m_rServiceFactory, UNO_QUERY_THROW );
        xContext.set( xProps->getPropertyValue( ::rtl::OUString::createFromAscii( "DefaultContext" ) ), UNO_QUERY_THROW );
        Reference< XMacroExpander > xMacroExpander( xContext->getValueByName( ::rtl::OUString::createFromAscii( "/singletons/com.sun.star.util.theMacroExpander" ) ), UNO_QUERY_THROW );
        sExpandedUrl = xMacroExpander->expandMacros(sUrl);
        sal_Int32 nPos = sExpandedUrl.indexOf(::rtl::OUString::createFromAscii("vnd.sun.star.expand:"));
        if ( nPos != -1 )
            sExpandedUrl = sExpandedUrl.copy(nPos+20);
    }
    catch (Exception&) {}
    return sExpandedUrl;
}

void XSLTFilter::started() throw (RuntimeException)
{
    osl_resetCondition(m_cTransformed);
}
void XSLTFilter::error(const Any& a) throw (RuntimeException)
{
    Exception e;
    if ( a >>= e)
    {
        OString aMessage("XSLTFilter::error was called: ");
        aMessage += OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US);
        OSL_ENSURE(sal_False, aMessage);
    }
    m_bError = sal_True;
    osl_setCondition(m_cTransformed);
}
void XSLTFilter::closed() throw (RuntimeException)
{
    osl_setCondition(m_cTransformed);
}
void XSLTFilter::terminated() throw (RuntimeException)
{
    m_bTerminated = sal_True;
    osl_setCondition(m_cTransformed);
}

OUString XSLTFilter::rel2abs(const OUString& s)
{

    Reference< XStringSubstitution > subs(m_rServiceFactory->createInstance(
        OUString::createFromAscii("com.sun.star.util.PathSubstitution")), UNO_QUERY);
    OUString aWorkingDir = subs->getSubstituteVariableValue(OUString::createFromAscii("$(progurl)"));
    INetURLObject aObj( aWorkingDir );
    aObj.setFinalSlash();
    bool bWasAbsolute;
    INetURLObject aURL = aObj.smartRel2Abs(
        s, bWasAbsolute, false, INetURLObject::WAS_ENCODED, RTL_TEXTENCODING_UTF8, true );
    return aURL.GetMainURL(INetURLObject::NO_DECODE);
}



sal_Bool XSLTFilter::importer(
        const Sequence<PropertyValue>& aSourceData,
        const Reference<XDocumentHandler>& xHandler,
        const Sequence<OUString>& msUserData)
    throw (RuntimeException)
{
    if ( msUserData.getLength() < 5 )
        return sal_False;

    const OUString udStyleSheet = rel2abs( msUserData[4] );

    // get information from media descriptor
    // the imput stream that represents the imported file
    // is most important here since we need to supply it to
    // the sax parser that drives the supplied document handler
    const sal_Int32 nLength = aSourceData.getLength();
    OUString aFileName;
    OUString aURL;
    Reference< XInputStream > xInputStream;
    for ( sal_Int32 i = 0 ; i < nLength; i++)
    {
        const OUString aName = aSourceData[i].Name;
        if ( aName.equalsAscii( "InputStream" ) )
            aSourceData[i].Value >>= xInputStream;
        else if ( aName.equalsAscii( "FileName" ) )
            aSourceData[i].Value >>= aFileName;
        else if ( aName.equalsAscii( "URL" ) )
            aSourceData[i].Value >>= aURL;
    }
    OSL_ASSERT(xInputStream.is());
    if ( !xInputStream.is() )
        return sal_False;

    // create SAX parser that will read the document file
    // and provide events to xHandler passed to this call
    Reference < XParser > xSaxParser( m_rServiceFactory->createInstance(
        OUString::createFromAscii("com.sun.star.xml.sax.Parser")), UNO_QUERY );
    OSL_ASSERT(xSaxParser.is());
    if( !xSaxParser.is() )
        return sal_False;

    // create transformer
    Sequence< Any > args(3);
    NamedValue nv;

    nv.Name = OUString::createFromAscii("StylesheetURL");
    nv.Value <<= expandUrl(udStyleSheet); args[0] <<= nv;
    nv.Name = OUString::createFromAscii("SourceURL");
    nv.Value <<= aURL; args[1] <<= nv;
    nv.Name = OUString::createFromAscii("SourceBaseURL");
    nv.Value <<= OUString(INetURLObject(aURL).getBase());
    args[2] <<= nv;

    m_tcontrol = Reference< XActiveDataControl >(m_rServiceFactory->createInstanceWithArguments(
        OUString::createFromAscii("com.sun.star.comp.JAXTHelper"), args), UNO_QUERY);

    OSL_ASSERT(xHandler.is());
    OSL_ASSERT(xInputStream.is());
    OSL_ASSERT(m_tcontrol.is());
    if (xHandler.is() && xInputStream.is() && m_tcontrol.is())
    {
        try
        {
            // we want to be notified when the processing is done...
            m_tcontrol->addListener(Reference< XStreamListener >(this));

            // connect input to transformer
            Reference< XActiveDataSink > tsink(m_tcontrol, UNO_QUERY);
            //UOF v2 import
            UOF2Storage aUOF2Storage( m_rServiceFactory, xInputStream );
            if ( aUOF2Storage.isValidUOF2Doc() )
            {
                UOF2Merge aUOF2Merge( aUOF2Storage, m_rServiceFactory );
                aUOF2Merge.merge();
                tsink->setInputStream( aUOF2Merge.getMergedInStream() );
            }
            else
            {
                tsink->setInputStream( xInputStream );
            }

            // create pipe
            Reference< XOutputStream > pipeout(m_rServiceFactory->createInstance(
                OUString::createFromAscii("com.sun.star.io.Pipe")), UNO_QUERY);
            Reference< XInputStream > pipein(pipeout, UNO_QUERY);

            //connect transformer to pipe
            Reference< XActiveDataSource > tsource(m_tcontrol, UNO_QUERY);
            tsource->setOutputStream(pipeout);

            // connect pipe to sax parser
            InputSource aInput;
            aInput.sSystemId = aURL;
            aInput.sPublicId = aURL;
            aInput.aInputStream = pipein;

            // set doc handler
            xSaxParser->setDocumentHandler(xHandler);

            // transform
            m_tcontrol->start();
            // osl_waitCondition(m_cTransformed, 0);
            if (!m_bError && !m_bTerminated)
            {
                // parse the transformed XML buffered in the pipe
                xSaxParser->parseStream(aInput);
                osl_waitCondition(m_cTransformed, 0);
                return sal_True;
            } else {
                return sal_False;
            }
        }
#if OSL_DEBUG_LEVEL > 0
        catch( Exception& exc)
#else
        catch( Exception& )
#endif
        {
            // something went wrong
            OSL_ENSURE(0, OUStringToOString(exc.Message, RTL_TEXTENCODING_ASCII_US).getStr());
            return sal_False;
        }
    } else
    {
        return sal_False;
    }
}

bool XSLTFilter::isUOF2ExportStyleSheet( const OUString& rExportStyleSheet )
{
    bool bIsUOFDocumentType = false;

    if ( rExportStyleSheet.endsWithAsciiL( "uof.xsl", 7 ) )
    {
        bIsUOFDocumentType = true;
    }

    return bIsUOFDocumentType;
}

sal_Bool XSLTFilter::exporter(
        const Sequence<PropertyValue>& aSourceData,
        const Sequence<OUString>& msUserData)
    throw (RuntimeException)
{
    if ( msUserData.getLength() < 6 )
        return sal_False;

    // get interesting values from user data
    const OUString udStyleSheet = rel2abs( msUserData[5] );

    // read source data
    // we are especially interested in the output stream
    // since that is where our xml-writer will push the data
    // from it's data-source interface
    OUString sURL;
    sal_Bool bIndent = sal_False;
    OUString aDoctypePublic;
    OUString aDoctypeSystem;
    {
        const sal_Int32 nLength = aSourceData.getLength();
        for ( sal_Int32 i = 0; i < nLength; i++ )
        {
            const OUString aName = aSourceData[i].Name;
            if ( aName.equalsAscii( "Indent" ) )
                aSourceData[i].Value >>= bIndent;
            if ( aName.equalsAscii( "DocType_Public" ) )
                aSourceData[i].Value >>= aDoctypePublic;
            if ( aName.equalsAscii( "DocType_System" ) )
                aSourceData[i].Value >>= aDoctypeSystem;
            if ( aName.equalsAscii( "OutputStream" ) )
                aSourceData[i].Value >>= m_rOutputStream;
            else if ( aName.equalsAscii( "URL" ) )
                aSourceData[i].Value >>= sURL;
            //UOF v2.0 export, get Stream for constructing UOF2Storage
            if ( aName.equalsAscii( "StreamForOutput" ) )
                aSourceData[i].Value >>= m_rStream;
        }
    }

    if (!m_rDocumentHandler.is())
    {
        // get the document writer
        m_rDocumentHandler = Reference<XExtendedDocumentHandler>(
            m_rServiceFactory->createInstance(
                OUString::createFromAscii("com.sun.star.xml.sax.Writer") ),
            UNO_QUERY);
    }

    // create transformer
    Sequence< Any > args(4);
    NamedValue nv;
    nv.Name = OUString::createFromAscii("StylesheetURL");
    nv.Value <<= expandUrl(udStyleSheet); args[0] <<= nv;
    nv.Name = OUString::createFromAscii("TargetURL");
    nv.Value <<= sURL; args[1] <<= nv;
    nv.Name = OUString::createFromAscii("DoctypeSystem");
    nv.Value <<= aDoctypeSystem; args[2] <<= nv;
    nv.Name = OUString::createFromAscii("DoctypePublic");
    nv.Value <<= aDoctypePublic; args[3] <<= nv;
    nv.Name = OUString::createFromAscii("TargetBaseURL");
    INetURLObject ineturl(sURL);
    ineturl.removeSegment();
    m_aExportBaseUrl = ineturl.GetMainURL(INetURLObject::NO_DECODE);
    nv.Value <<= m_aExportBaseUrl;
    args[3] <<= nv;

    m_tcontrol = Reference< XActiveDataControl >(m_rServiceFactory->createInstanceWithArguments(
        OUString::createFromAscii("com.sun.star.comp.JAXTHelper"), args), UNO_QUERY);

    OSL_ASSERT(m_rDocumentHandler.is());
    OSL_ASSERT(m_rOutputStream.is());
    OSL_ASSERT(m_tcontrol.is());
    if (m_tcontrol.is() && m_rOutputStream.is() && m_rDocumentHandler.is())
    {
        // we want to be notified when the processing is done...
        m_tcontrol->addListener(Reference< XStreamListener >(this));

        // create pipe
        Reference< XOutputStream > pipeout(m_rServiceFactory->createInstance(
            OUString::createFromAscii("com.sun.star.io.Pipe")), UNO_QUERY);
        Reference< XInputStream > pipein(pipeout, UNO_QUERY);

        // connect sax writer to pipe
        Reference< XActiveDataSource > xmlsource(m_rDocumentHandler, UNO_QUERY);
        xmlsource->setOutputStream(pipeout);

        // connect pipe to transformer
        Reference< XActiveDataSink > tsink(m_tcontrol, UNO_QUERY);
        tsink->setInputStream(pipein);

        // connect transformer to output
        Reference< XActiveDataSource > tsource( m_tcontrol, UNO_QUERY );
        if ( isUOF2ExportStyleSheet( udStyleSheet ) )
        {
            // special handling for UOF 2

            if ( !m_rStream.is() )
            {
                return sal_False;
            }

            //creating pipe2
            Reference< XOutputStream > x_Pipeout(
                m_rServiceFactory->createInstance(
                    OUString::createFromAscii( "com.sun.star.io.Pipe" ) ), UNO_QUERY );
            Reference< XInputStream > x_Pipein( x_Pipeout, UNO_QUERY );

            // connect transformer to pipe2
            tsource->setOutputStream( x_Pipeout );

            UOF2Splitter* pSplitter = new UOF2Splitter( m_rServiceFactory, sURL );
            m_splitControl =
                Reference< XActiveDataControl >(
                    static_cast< cppu::OWeakObject* >( pSplitter ), UNO_QUERY );
            // connect pipe2 to splitter
            Reference< XActiveDataSink > splitsink( m_splitControl, UNO_QUERY );
            splitsink->setInputStream( x_Pipein );
            // connect splitter to output
            Reference< XActiveDataStreamer > splitout( m_splitControl, UNO_QUERY );
            splitout->setStream( m_rStream );
            m_rOutputStream = m_rStream->getOutputStream();
        }
        else
        {
            tsource->setOutputStream( m_rOutputStream );
        }

        // we will start receiving events after returning 'true'.
        // we will start the transformation as soon as we receive the startDocument event.
        return sal_True;
    }
    else
    {
        return sal_False;
    }
}

// for the DocumentHandler implementation, we just proxy the the
// events to the XML writer that we created upon the output stream
// that was provided by the XMLFilterAdapter
void XSLTFilter::startDocument() throw (SAXException,RuntimeException){
    OSL_ASSERT(m_rDocumentHandler.is());
    m_rDocumentHandler->startDocument();
    m_tcontrol->start();
}

void XSLTFilter::endDocument() throw (SAXException, RuntimeException){
    OSL_ASSERT(m_rDocumentHandler.is());
    m_rDocumentHandler->endDocument();

    // m_splitControl only set for UOF 2
    if ( m_splitControl.is() )
    {
        //when the inputStream(outputStream of filter) was closed, start to parse it.
        m_splitControl->start();
    }

    // wait for the transformer to finish
    osl_waitCondition(m_cTransformed, 0);
    if (!m_bError && !m_bTerminated)
    {
        return;
    } else {
        throw RuntimeException();
    }

}

void XSLTFilter::startElement(const OUString& str, const Reference<XAttributeList>& attriblist)
    throw (SAXException, RuntimeException)
{
    OSL_ASSERT(m_rDocumentHandler.is());
//  SvXMLAttributeList* _attriblist=SvXMLAttributeList::getImplementation(attriblist);
    m_rDocumentHandler->startElement(str, attriblist);
}

void XSLTFilter::endElement(const OUString& str)
    throw (SAXException, RuntimeException)
{
    OSL_ASSERT(m_rDocumentHandler.is());
    m_rDocumentHandler->endElement(str);
}

void XSLTFilter::characters(const OUString& str)
    throw (SAXException, RuntimeException)
{
    OSL_ASSERT(m_rDocumentHandler.is());
    m_rDocumentHandler->characters(str);
}

void XSLTFilter::ignorableWhitespace(const OUString& str)
    throw (SAXException, RuntimeException)
{
    OSL_ASSERT(m_rDocumentHandler.is());
    if (!m_bPrettyPrint) return;
    m_rDocumentHandler->ignorableWhitespace(str);
}

void  XSLTFilter::processingInstruction(const OUString& str, const OUString& str2)
    throw (SAXException, RuntimeException)
{
    OSL_ASSERT(m_rDocumentHandler.is());
    m_rDocumentHandler->processingInstruction(str, str2);
}

void XSLTFilter::setDocumentLocator(const Reference<XLocator>& doclocator)
    throw (SAXException, RuntimeException)
{
    OSL_ASSERT(m_rDocumentHandler.is());
    m_rDocumentHandler->setDocumentLocator(doclocator);
}

// --------------------------------------
// Component management
// --------------------------------------
#define SERVICE_NAME "com.sun.star.documentconversion.XSLTFilter"
#define IMPLEMENTATION_NAME "com.sun.star.comp.documentconversion.XSLTFilter"

static Reference< XInterface > SAL_CALL CreateInstance( const Reference< XMultiServiceFactory > &r)
{
    return Reference< XInterface >(( OWeakObject *)new XSLTFilter(r));
}

static Sequence< OUString > getSupportedServiceNames()
{
    static Sequence < OUString > *pNames = 0;
    if( ! pNames )
    {
        MutexGuard guard( Mutex::getGlobalMutex() );
        if( !pNames )
        {
            static Sequence< OUString > seqNames(1);
                seqNames.getArray()[0] = OUString::createFromAscii(SERVICE_NAME);
            pNames = &seqNames;
        }
    }
    return *pNames;
}

}

using namespace XSLT;

extern "C"
{
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** /* ppEnv */ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

sal_Bool SAL_CALL component_writeInfo(void * /* pServiceManager */, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            Reference< XRegistryKey > xNewKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                    OUString::createFromAscii( "/" IMPLEMENTATION_NAME "/UNO/SERVICES" ) ) );

            const Sequence< OUString > & rSNL = getSupportedServiceNames();
            const OUString * pArray = rSNL.getConstArray();
            for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}

void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * /* pRegistryKey */ )
{
    void * pRet = 0;

    if (pServiceManager && rtl_str_compare( pImplName, IMPLEMENTATION_NAME ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString::createFromAscii( pImplName ),
            CreateInstance, getSupportedServiceNames() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }
    return pRet;
}

} // extern "C"
