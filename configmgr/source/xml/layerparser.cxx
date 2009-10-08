/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: layerparser.cxx,v $
 * $Revision: 1.15 $
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

#include "com/sun/star/configuration/backend/NodeAttribute.hpp"
#include "layerparser.hxx"

// -----------------------------------------------------------------------------
#include "wrapexception.hxx"

#define WRAP_PARSE_EXCEPTIONS()  \
    PASS_EXCEPTION(sax::SAXException)       \
    PASS_EXCEPTION(uno::RuntimeException)   \
    WRAP_CONFIGDATA_EXCEPTIONS( raiseParseException )   \
    WRAP_OTHER_EXCEPTIONS( raiseParseException )

#define WRAP_PARSE_EXCEPTIONS_MSG( msg )  \
    PASS_EXCEPTION(sax::SAXException)       \
    PASS_EXCEPTION(uno::RuntimeException)   \
    WRAP_CONFIGDATA_EXCEPTIONS1( raiseParseException, msg ) \
    WRAP_OTHER_EXCEPTIONS1( raiseParseException, msg )

// -----------------------------------------------------------------------------

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace xml
    {
// -----------------------------------------------------------------------------
        namespace uno       = ::com::sun::star::uno;
        namespace sax       = ::com::sun::star::xml::sax;
// -----------------------------------------------------------------------------

LayerParser::LayerParser(uno::Reference< uno::XComponentContext > const & _xContext, uno::Reference< backenduno::XLayerHandler > const & _xHandler)
: BasicParser(_xContext)
, m_xHandler(_xHandler)
, m_bRemoved(false)
, m_bNewProp(false)
{
    if (!m_xHandler.is())
    {
        rtl::OUString sMessage(RTL_CONSTASCII_USTRINGPARAM("Cannot create LayerParser: Unexpected NULL Handler"));
        throw uno::RuntimeException(sMessage, NULL);
    }
}
// -----------------------------------------------------------------------------

LayerParser::~LayerParser()
{
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerParser::startDocument(  )
        throw (sax::SAXException, uno::RuntimeException)
{
    BasicParser::startDocument();

    try
    {
        OSL_ENSURE(isEmptyNode(), "BasicParser does not mark new document as empty");

        m_xHandler->startLayer();
        m_bRemoved = false;
        m_bNewProp = false;
    }
    WRAP_PARSE_EXCEPTIONS_MSG("LayerParser - Starting layer")
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerParser::endDocument(  ) throw (sax::SAXException, uno::RuntimeException)
{
    if (isEmptyNode()) OSL_TRACE("Configuration Parser: XML layer document ended without any data");

    BasicParser::endDocument();

    try
    {
        m_xHandler->endLayer();
    }
    WRAP_PARSE_EXCEPTIONS_MSG("LayerParser - Ending layer")
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerParser::startElement( const rtl::OUString& aName, const uno::Reference< sax::XAttributeList >& xAttribs )
        throw (sax::SAXException, uno::RuntimeException)
{
    if ( this->isSkipping() )
    {
        this->startSkipping( aName, xAttribs );
        return;
    }

    ElementInfo aInfo = getDataParser().parseElementInfo(aName,xAttribs);

    try
    {
        switch (aInfo.type)
        {
        case ElementType::group: case ElementType::set:
            OSL_ENSURE( false, "Layer XML parser - Unexpected: found group/set element (should be 'node')\n");
            // fall thru
        case ElementType::layer:
        case ElementType::node:
            this->startNode(aInfo,xAttribs);
            OSL_ASSERT( this->isInNode() && !this->isInProperty() );
            break;

        case ElementType::property:
            this->startProperty(aInfo,xAttribs);
            OSL_ASSERT( this->isInUnhandledProperty() );
            break;

        case ElementType::value:
            this->startValueData(xAttribs);
            OSL_ASSERT( this->isInValueData() );
            break;

        default: // skip unknown elements
            OSL_ENSURE( aInfo.type >= ElementType::other, "Layer XML parser - Unexpected: found schema element in layer data\n");
            OSL_ENSURE( aInfo.type <  ElementType::other, "Layer XML parser - Warning: ignoring unknown element tag\n");
            this->startSkipping( aName, xAttribs );
            return;
        }
    }
    WRAP_PARSE_EXCEPTIONS_MSG("LayerParser - Starting Element")
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerParser::endElement( const rtl::OUString& aName )
        throw (sax::SAXException, uno::RuntimeException)
{
    if ( this->wasSkipping(aName) )
        return;

    try
    {
        if ( this->isInValueData())
            this->endValueData();

        else if (this->isInProperty())
            this->endProperty();

        else if (this->isInNode())
            this->endNode();

        else {
            this->raiseParseException("Layer parser: Invalid XML: endElement without matching startElement");
        }
    }
    WRAP_PARSE_EXCEPTIONS_MSG("LayerParser - Ending Element")
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void LayerParser::startNode( ElementInfo const & aInfo, const uno::Reference< sax::XAttributeList >& xAttribs )
{
    this->checkNotRemoved();

    BasicParser::startNode(aInfo,xAttribs);

    switch (aInfo.op)
    {
    case Operation::none:
    case Operation::modify:
        m_xHandler->overrideNode(aInfo.name,aInfo.flags,false);
        break;

    case Operation::clear:
        m_xHandler->overrideNode(aInfo.name,aInfo.flags,true);
        break;

    case Operation::replace:
    case Operation::fuse:
        {
            backenduno::TemplateIdentifier aTemplate;
            sal_Int16 flags = aInfo.flags;
            if (aInfo.op == Operation::fuse) {
                flags |=
                    com::sun::star::configuration::backend::NodeAttribute::FUSE;
            }
            if (getDataParser().getInstanceType(xAttribs,aTemplate.Name,aTemplate.Component))
                m_xHandler->addOrReplaceNodeFromTemplate(aInfo.name,aTemplate,flags);
            else
                m_xHandler->addOrReplaceNode(aInfo.name,flags);
        }
        break;

    case Operation::remove:
        m_xHandler->dropNode(aInfo.name);
        m_bRemoved = true;
        break;

    default: OSL_ASSERT(false);
    case Operation::unknown:
        this->raiseParseException("Layer parser: Invalid Data: unknown operation");
    }
}
// -----------------------------------------------------------------------------

void LayerParser::endNode()
{
    if (!this->isInRemoved())
        m_xHandler->endNode();
    else
        m_bRemoved = false;

    BasicParser::endNode();
}
// -----------------------------------------------------------------------------

void LayerParser::startProperty( ElementInfo const & aInfo, const uno::Reference< sax::XAttributeList >& xAttribs )
{
    this->checkNotRemoved();

    BasicParser::startProperty(aInfo,xAttribs);

    switch (aInfo.op)
    {
    case Operation::none:
    case Operation::modify:
        m_xHandler->overrideProperty(aInfo.name,aInfo.flags,getActivePropertyType(),false);
        OSL_ASSERT(!m_bNewProp);
        break;

    case Operation::clear:
        m_xHandler->overrideProperty(aInfo.name,aInfo.flags,getActivePropertyType(),true);
        OSL_ASSERT(!m_bNewProp);
        break;

    case Operation::replace:
        // defer to later
        m_bNewProp = true;
        break;

    case Operation::fuse:
        this->raiseParseException("Layer parser: Invalid Data: operation 'fuse' is not permitted for properties");

    case Operation::remove:
        this->raiseParseException("Layer parser: Invalid Data: operation 'remove' is not permitted for properties");

    default: OSL_ASSERT(false);
    case Operation::unknown:
        this->raiseParseException("Layer parser: Invalid Data: unknown operation");
    }
    OSL_POSTCOND(this->isInUnhandledProperty(),"Error: Property not reckognized as unhandled");
}
// -----------------------------------------------------------------------------

void LayerParser::addOrReplaceCurrentProperty(const uno::Any& aValue)
{
    const ElementInfo& currentInfo = getActiveNodeInfo() ;

    OSL_ASSERT(currentInfo.op == Operation::replace) ;

    if (aValue.hasValue())
    {
        m_xHandler->addPropertyWithValue(currentInfo.name,
                                         currentInfo.flags, aValue) ;
    }
    else
    {
        m_xHandler->addProperty(currentInfo.name, currentInfo.flags,
                                getActivePropertyType()) ;
    }
}
// -----------------------------------------------------------------------------

void LayerParser::endProperty()
{
    OSL_ASSERT(!this->isInRemoved());

    if (m_bNewProp)
    {
        OSL_ASSERT(getActiveNodeInfo().op == Operation::replace);
        if (this->isInUnhandledProperty())
        {
            // No value tag is treated as NULL
            addOrReplaceCurrentProperty( uno::Any() ) ;
        }
        m_bNewProp = false;
    }
    else
    {
        OSL_ASSERT(getActiveNodeInfo().op != Operation::replace);
        m_xHandler->endProperty();
    }

    BasicParser::endProperty();
}
// -----------------------------------------------------------------------------

void LayerParser::startValueData(const uno::Reference< sax::XAttributeList >& xAttribs)
{
    this->checkNotRemoved();

    BasicParser::startValueData(xAttribs);
}
// -----------------------------------------------------------------------------

void LayerParser::endValueData()
{
    uno::Any aValue = this->getCurrentValue();

    if (m_bNewProp)
    {
        if (this->isValueDataLocalized())
            getLogger().warning("Language attribute ignored for value of added property.",
                                "endValueData()","configuration::xml::SchemaParser");

        addOrReplaceCurrentProperty(aValue) ;
    }
    else if (this->isValueDataLocalized())
    {
        rtl::OUString aLocale = this->getValueDataLocale();

        m_xHandler->setPropertyValueForLocale(aValue,aLocale);
    }
    else
    {
        m_xHandler->setPropertyValue(aValue);
    }

    BasicParser::endValueData();

    OSL_POSTCOND(!this->isInUnhandledProperty(),"Error: Property not reckognized as unhandled");
}
// -----------------------------------------------------------------------------

void LayerParser::checkNotRemoved()
{
    if (m_bRemoved)
        raiseParseException("Layer parser: Invalid Data: Data inside removed node.");
}
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
    } // namespace

// -----------------------------------------------------------------------------
} // namespace

