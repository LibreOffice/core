/*************************************************************************
 *
 *  $RCSfile: parsersvc.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:20:05 $
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
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "parsersvc.hxx"

#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "confapifactory.hxx"
#endif
#ifndef CONFIGMGR_XML_SCHEMAPARSER_HXX
#include "schemaparser.hxx"
#endif
#ifndef CONFIGMGR_XML_LAYERPARSER_HXX
#include "layerparser.hxx"
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#include <drafts/com/sun/star/configuration/backend/XSchema.hpp>
#include <drafts/com/sun/star/configuration/backend/XLayer.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>

#ifndef _COM_SUN_STAR_XML_SAX_XPARSER_HPP_
#include <com/sun/star/xml/sax/XParser.hpp>
#endif
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
        namespace backenduno = drafts::com::sun::star::configuration::backend;
// -----------------------------------------------------------------------------

template <class BackendInterface>
struct ParserServiceTraits;
// -----------------------------------------------------------------------------
static inline void clear(OUString & _rs) { _rs = OUString(); }

// -----------------------------------------------------------------------------
template <class BackendInterface>
ParserService<BackendInterface>::ParserService(CreationArg _xContext)
: m_xServiceFactory(_xContext->getServiceManager(), uno::UNO_QUERY)
, m_aInputSource()
{
    if (!m_xServiceFactory.is())
    {
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration Parser: Context has no service factory"));
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
            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Cannot use argument to initialize a Configuration Parser"
                                                            "- InputSource or XInputStream expected"));
            throw lang::IllegalArgumentException(sMessage,*this,1);
        }
    default:
        {
            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Too many arguments to initialize a Configuration Parser"));
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
    throw (uno::Exception)
{
    typedef uno::Reference< sax::XParser > SaxParser;

    rtl::OUString const k_sSaxParserService( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Parser"));

    SaxParser xParser = SaxParser::query( m_xServiceFactory->createInstance(k_sSaxParserService) );

    if (!xParser.is())
    {
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration Parser: Cannot create SAX Parser"));
        throw uno::RuntimeException(sMessage,*this);
    }

    try
    {
        xParser->setDocumentHandler(_xHandler);
        xParser->parseStream( m_aInputSource );
    }
    catch (sax::SAXException & e)
    {
        uno::Any aWrapped = e.WrappedException.hasValue() ? e.WrappedException : uno::makeAny( e );
        OUString sSAXMessage = e.Message;

        // Expatwrap SAX service doubly wraps its errors ??
        sax::SAXException eInner;
        if (aWrapped >>= eInner)
        {
            if (eInner.WrappedException.hasValue()) aWrapped = eInner.WrappedException;

            rtl::OUStringBuffer sMsgBuf(eInner.Message);
            sMsgBuf.appendAscii("- {Parser Error: ").append(sSAXMessage).appendAscii(" }.");
            sSAXMessage = sMsgBuf.makeStringAndClear();
        }

        rtl::OUStringBuffer sMessage;
        sMessage.appendAscii("Configuration Parser: a ").append( aWrapped.getValueTypeName() );
        sMessage.appendAscii(" occurred while parsing: ");
        sMessage.append(sSAXMessage);
        throw lang::WrappedTargetRuntimeException(sMessage.makeStringAndClear(),*this,aWrapped);
    }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
AsciiServiceName const aSchemaParserServices[] =
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
AsciiServiceName const aLayerParserServices[] =
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
    typedef backenduno::XSchemaHandler Handler;

    static ServiceImplementationInfo const * getServiceInfo()
    { return & aSchemaParserSI; }
};
// -----------------------------------------------------------------------------
template <>
struct ParserServiceTraits< backenduno::XLayer >
{
    typedef backenduno::XLayerHandler Handler;

    static ServiceImplementationInfo const * getServiceInfo()
    { return & aLayerParserSI; }
};
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

typedef ParserService< backenduno::XSchema > SchemaParserService_Base;

class SchemaParserService : public SchemaParserService_Base
{
public:
    typedef SchemaParser::HandlerRef HandlerArg;

    SchemaParserService(CreationArg _xContext)
    : SchemaParserService_Base(_xContext)
    {
    }

    virtual void SAL_CALL readSchema( HandlerArg const & aHandler )
        throw (uno::RuntimeException);

    virtual void SAL_CALL readComponent( HandlerArg const & aHandler )
        throw (uno::RuntimeException);

    virtual void SAL_CALL readTemplates( HandlerArg const & aHandler )
        throw (uno::RuntimeException);
};
// -----------------------------------------------------------------------------

typedef ParserService< backenduno::XLayer > LayerParserService_Base;

class LayerParserService : public LayerParserService_Base
{
public:
    typedef LayerParser::HandlerRef HandlerArg;

    LayerParserService(CreationArg _xContext)
    : LayerParserService_Base(_xContext)
    {
    }

    virtual void SAL_CALL readData( HandlerArg const & aHandler )
        throw (uno::RuntimeException);
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL instantiateSchemaParser( CreationContext const& xContext )
{
    return * new SchemaParserService(xContext);
}
uno::Reference< uno::XInterface > SAL_CALL instantiateLayerParser( CreationContext const& xContext )
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
void SAL_CALL SchemaParserService::readSchema( uno::Reference< backenduno::XSchemaHandler > const & aHandler )
    throw (uno::RuntimeException)
{
    // todo: check for NULL handler
    SaxHandler xHandler = new SchemaParser(this->getServiceFactory(),aHandler, SchemaParser::selectAll);
    this->parse( xHandler );
}
// -----------------------------------------------------------------------------
void SAL_CALL SchemaParserService::readComponent( uno::Reference< backenduno::XSchemaHandler > const & aHandler )
    throw (uno::RuntimeException)
{
    // todo: check for NULL handler
    SaxHandler xHandler = new SchemaParser(this->getServiceFactory(),aHandler, SchemaParser::selectComponent);
    this->parse( xHandler );
}
// -----------------------------------------------------------------------------
void SAL_CALL SchemaParserService::readTemplates( uno::Reference< backenduno::XSchemaHandler > const & aHandler )
    throw (uno::RuntimeException)
{
    // todo: check for NULL handler
    SaxHandler xHandler = new SchemaParser(this->getServiceFactory(),aHandler, SchemaParser::selectTemplates);
    this->parse( xHandler );
}
// -----------------------------------------------------------------------------
void SAL_CALL LayerParserService::readData( uno::Reference< backenduno::XLayerHandler > const & aHandler )
    throw (uno::RuntimeException)
{
    // todo: check for NULL handler
    SaxHandler xHandler = new LayerParser(this->getServiceFactory(),aHandler);
    this->parse( xHandler );
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    } // namespace

// -----------------------------------------------------------------------------
} // namespace

