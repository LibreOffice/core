/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: parsersvc.cxx,v $
 * $Revision: 1.11 $
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
#include "precompiled_configmgr.hxx"

#include "parsersvc.hxx"

#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "confapifactory.hxx"
#endif
#include "schemaparser.hxx"
#include "layerparser.hxx"
#include <rtl/ustrbuf.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <com/sun/star/configuration/backend/XSchema.hpp>
#include <com/sun/star/configuration/backend/XLayer.hpp>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/configuration/backend/MalformedDataException.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
// -----------------------------------------------------------------------------

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace xml
    {
// -----------------------------------------------------------------------------
        namespace uno   = ::com::sun::star::uno;
        namespace lang  = ::com::sun::star::lang;
        namespace io    = ::com::sun::star::io;
        namespace sax   = ::com::sun::star::xml::sax;
        namespace backenduno = ::com::sun::star::configuration::backend;
// -----------------------------------------------------------------------------

template <class BackendInterface>
struct ParserServiceTraits;
// -----------------------------------------------------------------------------
static inline void clear(rtl::OUString & _rs) { _rs = rtl::OUString(); }

// -----------------------------------------------------------------------------
template <class BackendInterface>
ParserService<BackendInterface>::ParserService(uno::Reference< uno::XComponentContext > const & _xContext)
: m_xContext(_xContext)
, m_aInputSource()
{
    if (!m_xContext.is())
    {
        rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration Parser: NULL Context"));
        throw uno::RuntimeException(sMessage,NULL);
    }
}
// -----------------------------------------------------------------------------

// XInitialization
template <class BackendInterface>
void SAL_CALL
    ParserService<BackendInterface>::initialize( const uno::Sequence< uno::Any >& aArguments )
        throw (uno::Exception, uno::RuntimeException)
{
    switch(aArguments.getLength())
    {
    case 0:
        break;

    case 1:
        if (aArguments[0] >>= m_aInputSource)
            break;

        if (aArguments[0] >>= m_aInputSource.aInputStream)
            break;

        {
            rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Cannot use argument to initialize a Configuration Parser"
                                                            "- InputSource or XInputStream expected"));
            throw lang::IllegalArgumentException(sMessage,*this,1);
        }
    default:
        {
            rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Too many arguments to initialize a Configuration Parser"));
            throw lang::IllegalArgumentException(sMessage,*this,0);
        }
    }
}
// -----------------------------------------------------------------------------

template <class BackendInterface>
inline
ServiceInfoHelper ParserService<BackendInterface>::getServiceInfo()
{
    return ParserServiceTraits<BackendInterface>::getServiceInfo();
}
// -----------------------------------------------------------------------------

// XServiceInfo
template <class BackendInterface>
::rtl::OUString SAL_CALL
    ParserService<BackendInterface>::getImplementationName(  )
        throw (uno::RuntimeException)
{
    return getServiceInfo().getImplementationName();
}
// -----------------------------------------------------------------------------

template <class BackendInterface>
sal_Bool SAL_CALL
    ParserService<BackendInterface>::supportsService( const ::rtl::OUString& ServiceName )
        throw (uno::RuntimeException)
{
    return getServiceInfo().supportsService( ServiceName );
}
// -----------------------------------------------------------------------------

template <class BackendInterface>
uno::Sequence< ::rtl::OUString > SAL_CALL
    ParserService<BackendInterface>::getSupportedServiceNames(  )
        throw (uno::RuntimeException)
{
    return getServiceInfo().getSupportedServiceNames( );
}
// -----------------------------------------------------------------------------

template <class BackendInterface>
void SAL_CALL
    ParserService<BackendInterface>::setInputStream( const uno::Reference< io::XInputStream >& aStream )
        throw (uno::RuntimeException)
{
    clear( m_aInputSource.sEncoding );
    clear( m_aInputSource.sSystemId );
    // clear( m_aInputSource.sPublicId );
    m_aInputSource.aInputStream = aStream;
}
// -----------------------------------------------------------------------------

template <class BackendInterface>
uno::Reference< io::XInputStream > SAL_CALL
    ParserService<BackendInterface>::getInputStream(  )
        throw (uno::RuntimeException)
{
    return m_aInputSource.aInputStream;
}
// -----------------------------------------------------------------------------

template <class BackendInterface>
void ParserService<BackendInterface>::parse(uno::Reference< sax::XDocumentHandler > const & _xHandler)
//    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_PRECOND( _xHandler.is(), "ParserService: No SAX handler to parse to");

    rtl::OUString const k_sSaxParserService( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Parser"));

    uno::Reference< lang::XMultiComponentFactory > xServiceFactory = m_xContext->getServiceManager();

    uno::Reference< sax::XParser > xParser = uno::Reference< sax::XParser >::query( xServiceFactory->createInstanceWithContext(k_sSaxParserService,m_xContext) );

    if (!xParser.is())
    {
        rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration Parser: Cannot create SAX Parser"));
        throw uno::RuntimeException(sMessage,*this);
    }

    try
    {
        xParser->setDocumentHandler(_xHandler);

        sax::InputSource aInputSourceCopy = m_aInputSource;
        //Set the sax input stream to null, an input stream can only be parsed once
        m_aInputSource.aInputStream = NULL;
        xParser->parseStream( aInputSourceCopy );
    }
    catch (sax::SAXException & e)
    {
        uno::Any aWrapped = e.WrappedException.hasValue() ? e.WrappedException : uno::makeAny( e );
        rtl::OUString sSAXMessage = e.Message;

        // Expatwrap SAX service doubly wraps its errors ??
        sax::SAXException eInner;
        if (aWrapped >>= eInner)
        {
            if (eInner.WrappedException.hasValue()) aWrapped = eInner.WrappedException;

            rtl::OUStringBuffer sMsgBuf(eInner.Message);
            sMsgBuf.appendAscii("- {Parser Error: ").append(sSAXMessage).appendAscii(" }.");
            sSAXMessage = sMsgBuf.makeStringAndClear();
        }

        static backenduno::MalformedDataException const * const forDataError = 0;
        static lang::WrappedTargetException const * const forWrappedError = 0;
        if (aWrapped.isExtractableTo(getCppuType(forDataError)) ||
            aWrapped.isExtractableTo(getCppuType(forWrappedError)))
        {
            cppu::throwException(aWrapped);

            OSL_ASSERT(!"not reached");
        }

        rtl::OUStringBuffer sMessageBuf;
        sMessageBuf.appendAscii("Configuration Parser: a ").append( aWrapped.getValueTypeName() );
        sMessageBuf.appendAscii(" occurred while parsing: ");
        sMessageBuf.append(sSAXMessage);

        throw lang::WrappedTargetException(sMessageBuf.makeStringAndClear(),*this,aWrapped);
    }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
sal_Char const * const aSchemaParserServices[] =
{
    "com.sun.star.configuration.backend.xml.SchemaParser",
    0
};
const ServiceImplementationInfo aSchemaParserSI =
{
    "com.sun.star.comp.configuration.backend.xml.SchemaParser",
    aSchemaParserServices,
    0
};
// -----------------------------------------------------------------------------
sal_Char const * const aLayerParserServices[] =
{
    "com.sun.star.configuration.backend.xml.LayerParser",
    0
};
const ServiceImplementationInfo aLayerParserSI =
{
    "com.sun.star.comp.configuration.backend.xml.LayerParser",
    aLayerParserServices,
    0
};
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
template <>
struct ParserServiceTraits< backenduno::XSchema >
{
    static ServiceImplementationInfo const * getServiceInfo()
    { return & aSchemaParserSI; }
};
// -----------------------------------------------------------------------------
template <>
struct ParserServiceTraits< backenduno::XLayer >
{
    static ServiceImplementationInfo const * getServiceInfo()
    { return & aLayerParserSI; }
};
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

class SchemaParserService : public ParserService< backenduno::XSchema >
{
public:
    SchemaParserService(uno::Reference< uno::XComponentContext > const & _xContext)
    : ParserService< backenduno::XSchema >(_xContext)
    {
    }

    virtual void SAL_CALL readSchema( uno::Reference< backenduno::XSchemaHandler > const & aHandler )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException,
               lang::NullPointerException, uno::RuntimeException);

    virtual void SAL_CALL readComponent( uno::Reference< backenduno::XSchemaHandler > const & aHandler )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException,
               lang::NullPointerException, uno::RuntimeException);

    virtual void SAL_CALL readTemplates( uno::Reference< backenduno::XSchemaHandler > const & aHandler )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException,
               lang::NullPointerException, uno::RuntimeException);
};
// -----------------------------------------------------------------------------

class LayerParserService : public ParserService< backenduno::XLayer >
{
public:
    LayerParserService(uno::Reference< uno::XComponentContext > const & _xContext)
    : ParserService< backenduno::XLayer >(_xContext)
    {
    }

    virtual void SAL_CALL readData( uno::Reference< backenduno::XLayerHandler > const & aHandler )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException,
               lang::NullPointerException, uno::RuntimeException);
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL instantiateSchemaParser( uno::Reference< uno::XComponentContext > const& xContext )
{
    return * new SchemaParserService(xContext);
}
uno::Reference< uno::XInterface > SAL_CALL instantiateLayerParser( uno::Reference< uno::XComponentContext > const& xContext )
{
    return * new LayerParserService(xContext);
}
// -----------------------------------------------------------------------------
const ServiceRegistrationInfo* getSchemaParserServiceInfo()
{ return getRegistrationInfo(& aSchemaParserSI); }
const ServiceRegistrationInfo* getLayerParserServiceInfo()
{ return getRegistrationInfo(& aLayerParserSI); }
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
static rtl::OUString nullHandlerMessage(char const * where)
{
    OSL_ASSERT(where);
    rtl::OUString msg = rtl::OUString::createFromAscii(where);
    return msg.concat(rtl::OUString::createFromAscii(": Error - NULL handler passed."));
}
// -----------------------------------------------------------------------------
void SAL_CALL SchemaParserService::readSchema( uno::Reference< backenduno::XSchemaHandler > const & aHandler )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException,
           lang::NullPointerException, uno::RuntimeException)
{
    if (!aHandler.is())
        throw lang::NullPointerException(nullHandlerMessage("SchemaParserService::readSchema"),*this);

    uno::Reference< sax::XDocumentHandler > xHandler = new SchemaParser(this->getContext(),aHandler, SchemaParser::selectAll);
    this->parse( xHandler );
}
// -----------------------------------------------------------------------------
void SAL_CALL SchemaParserService::readComponent( uno::Reference< backenduno::XSchemaHandler > const & aHandler )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException,
           lang::NullPointerException, uno::RuntimeException)
{
    if (!aHandler.is())
        throw lang::NullPointerException(nullHandlerMessage("SchemaParserService::readComponent"),*this);

    uno::Reference< sax::XDocumentHandler > xHandler = new SchemaParser(this->getContext(),aHandler, SchemaParser::selectComponent);
    this->parse( xHandler );
}
// -----------------------------------------------------------------------------
void SAL_CALL SchemaParserService::readTemplates( uno::Reference< backenduno::XSchemaHandler > const & aHandler )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException,
           lang::NullPointerException, uno::RuntimeException)
{
    if (!aHandler.is())
        throw lang::NullPointerException(nullHandlerMessage("SchemaParserService::readTemplates"),*this);

    uno::Reference< sax::XDocumentHandler > xHandler = new SchemaParser(this->getContext(),aHandler, SchemaParser::selectTemplates);
    this->parse( xHandler );
}
// -----------------------------------------------------------------------------
void SAL_CALL LayerParserService::readData( uno::Reference< backenduno::XLayerHandler > const & aHandler )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException,
           lang::NullPointerException, uno::RuntimeException)
{
    if (!aHandler.is())
        throw lang::NullPointerException(nullHandlerMessage("LayerParserService::readData"),*this);

    uno::Reference< sax::XDocumentHandler > xHandler = new LayerParser(this->getContext(),aHandler);
    this->parse( xHandler );
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    } // namespace

// -----------------------------------------------------------------------------
} // namespace

