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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

#include <stdio.h>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>
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

#include <xmloff/attrlist.hxx>
#include <fla.hxx>

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


class FLABridge : public WeakImplHelper1< XDocumentHandler>
{
private:
    const Reference<XDocumentHandler>& m_rDocumentHandler;
    const sal_Unicode *eval(const sal_Unicode *expr, sal_Int32 exprLen);
    FLA::Evaluator ev;
    bool active;

public:
    FLABridge(const Reference<XDocumentHandler>& m_rDocumentHandler);

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

};

FLABridge::FLABridge(const Reference<XDocumentHandler>& _rDocumentHandler) : m_rDocumentHandler(_rDocumentHandler), active(false)
{
}

void FLABridge::startDocument() throw (SAXException,RuntimeException){
    OSL_ASSERT(m_rDocumentHandler.is());
    m_rDocumentHandler->startDocument();
}

void FLABridge::endDocument() throw (SAXException,RuntimeException){
    OSL_ASSERT(m_rDocumentHandler.is());
    m_rDocumentHandler->endDocument();

}


void FLABridge::startElement(const OUString& str, const Reference<XAttributeList>& attriblist)
    throw (SAXException, RuntimeException)
{
    OSL_ASSERT(m_rDocumentHandler.is());
    if (active)
    {
//		SvXMLAttributeList* _attriblist=SvXMLAttributeList::getImplementation(attriblist);
        const int len=attriblist->getLength();
        SvXMLAttributeList *_newattriblist= new SvXMLAttributeList();
        for(int i=0;i<len;i++)
        {
            const OUString& name=attriblist->getNameByIndex( sal::static_int_cast<sal_Int16>( i ) );
            sal_Int32 pos;
            static const OUString _value_(".value", 6, RTL_TEXTENCODING_ASCII_US);
            if ((pos=name.lastIndexOf(L'.'))!=-1 && name.match(_value_, pos))
            {
                const OUString newName(name, pos);
                const OUString& value=attriblist->getValueByIndex( sal::static_int_cast<sal_Int16>( i ) );
                const OUString newValue(ev.eval(value.getStr(), value.getLength()));
                if (newValue.getLength()>0)
                {
                    _newattriblist->AddAttribute(newName, newValue);
                }
            }
            else
            {
                _newattriblist->AddAttribute(name, attriblist->getValueByIndex( sal::static_int_cast<sal_Int16>( i )));
            }
        };
        const Reference<XAttributeList> newattriblist(_newattriblist);
        m_rDocumentHandler->startElement(str, newattriblist);
    }
    else
    {
        if (str.compareToAscii("fla:fla.activate")==0) 
        {
            active=1;
        }
        m_rDocumentHandler->startElement(str, attriblist);
    }
}

void FLABridge::endElement(const OUString& str)
    throw (SAXException, RuntimeException)
{
    OSL_ASSERT(m_rDocumentHandler.is());
    m_rDocumentHandler->endElement(str);
}

void FLABridge::characters(const OUString& str)
    throw (SAXException, RuntimeException)
{
    OSL_ASSERT(m_rDocumentHandler.is());
    m_rDocumentHandler->characters(str);
}

void FLABridge::ignorableWhitespace(const OUString& str)
    throw (SAXException, RuntimeException)
{
    OSL_ASSERT(m_rDocumentHandler.is());
    m_rDocumentHandler->ignorableWhitespace(str);
}

void  FLABridge::processingInstruction(const OUString& str, const OUString& str2)
    throw (SAXException, RuntimeException)
{
    OSL_ASSERT(m_rDocumentHandler.is());
    m_rDocumentHandler->processingInstruction(str, str2);
}

void FLABridge::setDocumentLocator(const Reference<XLocator>& doclocator)
    throw (SAXException, RuntimeException)
{
    OSL_ASSERT(m_rDocumentHandler.is());
    m_rDocumentHandler->setDocumentLocator(doclocator);
}



class XSLTFilter : public WeakImplHelper4< XImportFilter, XExportFilter, XDocumentHandler, XStreamListener>
{
private:
    // the UNO ServiceFactory
    Reference< XMultiServiceFactory > m_rServiceFactory;

    // DocumentHandler interface of the css::xml::sax::Writer service
    Reference < XExtendedDocumentHandler > m_rDocumentHandler;
    Reference < XOutputStream > m_rOutputStream;

    // controls pretty-printing
    sal_Bool m_bPrettyPrint;

    Reference< XActiveDataControl > m_tcontrol;
    oslCondition  m_cTransformed;
    sal_Bool m_bTerminated;
    sal_Bool m_bError;

    OUString m_aExportBaseUrl;
    OUString m_aOldBaseUrl;

    OUString rel2abs(const OUString&);
    OUString expandUrl(const OUString&);

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
};

XSLTFilter::XSLTFilter( const Reference< XMultiServiceFactory > &r )
    : m_rServiceFactory(r)
    , m_bPrettyPrint(sal_True)
    , m_bTerminated(sal_False)
    , m_bError(sal_False)
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

    OUString udImport = msUserData[2];
    OUString udStyleSheet = rel2abs(msUserData[4]);

    // get information from media descriptor
    // the imput stream that represents the imported file
    // is most important here since we need to supply it to
    // the sax parser that drives the supplied document handler
    sal_Int32 nLength = aSourceData.getLength();
    OUString aName, aFileName, aURL;
    Reference< XInputStream > xInputStream;
    for ( sal_Int32 i = 0 ; i < nLength; i++)
    {
        aName = aSourceData[i].Name;
        if (aName.equalsAscii("InputStream"))
            aSourceData[i].Value >>= xInputStream;
        else if ( aName.equalsAscii("FileName"))
            aSourceData[i].Value >>= aFileName;
        else if ( aName.equalsAscii("URL"))
            aSourceData[i].Value >>= aURL;
    }
    OSL_ASSERT(xInputStream.is());
    if (!xInputStream.is()) return sal_False;

    // create SAX parser that will read the document file
    // and provide events to xHandler passed to this call
    Reference < XParser > xSaxParser( m_rServiceFactory->createInstance(
        OUString::createFromAscii("com.sun.star.xml.sax.Parser")), UNO_QUERY );
    OSL_ASSERT(xSaxParser.is());
    if(!xSaxParser.is())return sal_False;

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
            // we want to be notfied when the processing is done...
            m_tcontrol->addListener(Reference< XStreamListener >(this));

            // connect input to transformer
            Reference< XActiveDataSink > tsink(m_tcontrol, UNO_QUERY);
            tsink->setInputStream(xInputStream);

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
            xSaxParser->setDocumentHandler(new FLABridge(xHandler));

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

sal_Bool XSLTFilter::exporter(
        const Sequence<PropertyValue>& aSourceData,
        const Sequence<OUString>& msUserData)
    throw (RuntimeException)
{
    if ( msUserData.getLength() < 6 )
        return sal_False;

    // get interesting values from user data
    OUString udImport = msUserData[2];
    OUString udStyleSheet = rel2abs(msUserData[5]);

    // read source data
    // we are especialy interested in the output stream
    // since that is where our xml-writer will push the data
    // from it's data-source interface
    OUString aName, sURL;
    sal_Bool bIndent = sal_False;
    OUString aDoctypePublic;
    OUString aDoctypeSystem;
    // Reference<XOutputStream> rOutputStream;
    sal_Int32 nLength = aSourceData.getLength();
    for ( sal_Int32 i = 0 ; i < nLength; i++)
    {
        aName = aSourceData[i].Name;
        if ( aName.equalsAscii("Indent"))
            aSourceData[i].Value >>= bIndent;
        if ( aName.equalsAscii("DocType_Public"))
            aSourceData[i].Value >>= aDoctypePublic;
        if ( aName.equalsAscii("DocType_System"))
            aSourceData[i].Value >>= aDoctypeSystem;
        if ( aName.equalsAscii("OutputStream"))
            aSourceData[i].Value >>= m_rOutputStream;
        else if ( aName.equalsAscii("URL" ))
            aSourceData[i].Value >>= sURL;
    }

    if (!m_rDocumentHandler.is()) {
        // get the document writer
        m_rDocumentHandler = Reference<XExtendedDocumentHandler>(
            m_rServiceFactory->createInstance(
            OUString::createFromAscii("com.sun.star.xml.sax.Writer")),
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
        // we want to be notfied when the processing is done...
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
        Reference< XActiveDataSource > tsource(m_tcontrol, UNO_QUERY);
        tsource->setOutputStream(m_rOutputStream);

        // we will start receiving events after returning 'true'.
        // we will start the transformation as soon as we receive the startDocument 
        // event.
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
//	SvXMLAttributeList* _attriblist=SvXMLAttributeList::getImplementation(attriblist);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
