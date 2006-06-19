/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: schemaparser.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 23:36:50 $
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

#include "schemaparser.hxx"

// -----------------------------------------------------------------------------
#ifndef CONFIGMGR_WRAPEXCEPTION_HXX
#include "wrapexception.hxx"
#endif

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

SchemaParser::SchemaParser(Context const & _xContext, uno::Reference< backenduno::XSchemaHandler > const & _xHandler, Select _selector)
: BasicParser(_xContext)
, m_xHandler(_xHandler)
, m_sComponent()
, m_selector(_selector)
, m_selected(selectNone)
{
    if (!m_xHandler.is())
    {
        OUString sMessage(RTL_CONSTASCII_USTRINGPARAM("Cannot create SchemaParser: Unexpected NULL Handler"));
        throw uno::RuntimeException(sMessage, *this);
    }
    OSL_ENSURE(m_selector != selectNone, "Warning: Schema handler will handle no part of the schema");
}
// -----------------------------------------------------------------------------

SchemaParser::~SchemaParser()
{
}
// -----------------------------------------------------------------------------

void SAL_CALL SchemaParser::startDocument(  )
        throw (sax::SAXException, uno::RuntimeException)
{
    BasicParser::startDocument();

    OSL_ENSURE(isEmptyNode(), "BasicParser does not mark new document as empty");

    m_sComponent = OUString();
    m_selected = selectNone;
}
// -----------------------------------------------------------------------------

void SAL_CALL SchemaParser::endDocument(  ) throw (sax::SAXException, uno::RuntimeException)
{
    if (isSelected())
        raiseParseException("Schema XML Parser: Invalid XML: Document ends while section is open");

    if (isEmptyNode()) OSL_TRACE("Configuration Parser: XML schema document ended without any data");

    BasicParser::endDocument();
}
// -----------------------------------------------------------------------------

void SAL_CALL SchemaParser::startElement( const OUString& aName, const uno::Reference< sax::XAttributeList >& xAttribs )
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
        case ElementType::schema:
            this->startSchema(aInfo,xAttribs);
            break;

        case ElementType::component:
            this->startSection(selectComponent, aInfo, xAttribs);
            break;

        case ElementType::templates:
            this->startSection(selectTemplates, aInfo, xAttribs);
            break;

        case ElementType::import:
            this->handleImport(aInfo,xAttribs);
            this->startSkipping( aName, xAttribs );
            break;

        case ElementType::uses:
            this->startSkipping( aName, xAttribs );
            break;

        case ElementType::instance:
            this->handleInstance(aInfo,xAttribs);
            this->startSkipping( aName, xAttribs );
            break;

        case ElementType::item_type:
            this->handleItemType(aInfo,xAttribs);
            this->startSkipping( aName, xAttribs );
            break;

        case ElementType::layer:
        case ElementType::node:
            raiseParseException( "Schema XML parser - Invalid data: found unspecified 'node' element.\n");
            // fall thru
        case ElementType::group: case ElementType::set:
            this->startNode(aInfo,xAttribs);
            OSL_ASSERT( this->isInNode()  );
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
            OSL_ENSURE( aInfo.type <= ElementType::other, "Schema XML parser - Error: invalid element type value\n");
            OSL_ENSURE( aInfo.type >= ElementType::other, "Schema XML parser - Unexpected: found layer element in schema data\n");
        // accept (and skip) unknown (ElementType::other) tags in schema to allow documentation and constraints to pass without assertion
          //OSL_ENSURE( aInfo.type <  ElementType::other, "Schema XML parser - Warning: ignoring unknown element tag\n");

            this->startSkipping( aName, xAttribs );
            OSL_ASSERT( this->isSkipping() );
            return;
        }
    }
    WRAP_PARSE_EXCEPTIONS_MSG("LayerParser - Starting Element")

    OSL_ENSURE(aInfo.op == Operation::none || this->isSkipping(),
                "Schema Parser: The 'op' attribute is not supported in a schema");
}
// -----------------------------------------------------------------------------

void SAL_CALL SchemaParser::endElement( const OUString& aName )
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

        else if (this->isSelected())
            this->endSection();

        else
            this->endSchema();
    }
    WRAP_PARSE_EXCEPTIONS_MSG("LayerParser - Ending Element")
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void SchemaParser::startSchema( ElementInfo const & aInfo, const uno::Reference< sax::XAttributeList >& /*xAttribs*/ )
{
    m_sComponent = aInfo.name;
    m_xHandler->startSchema();
}
// -----------------------------------------------------------------------------

void SchemaParser::endSchema( )
{
    m_xHandler->endSchema();
    m_sComponent = OUString();
}
// -----------------------------------------------------------------------------

bool SchemaParser::select(Select _select)
{
    if (isSelected())
        raiseParseException("Schema XML parser - Invalid data: found start of section while a section is still open.\n");

    m_selected = static_cast<Select>(m_selector & _select);

    return m_selected != 0;
}
// -----------------------------------------------------------------------------

void SchemaParser::startSection( Select _select, ElementInfo const & aInfo, const uno::Reference< sax::XAttributeList >& xAttribs )
{
    if (this->select(_select))
    {
        if (_select == selectComponent)
        {
            m_xHandler->startComponent(m_sComponent);
        }
    }
    else
        startSkipping(aInfo.name,xAttribs);
}
// -----------------------------------------------------------------------------

void SchemaParser::endSection( )
{
    if (m_selected == selectComponent)
    {
        m_xHandler->endComponent();
    }
    m_selected = selectNone;
}
// -----------------------------------------------------------------------------

void SchemaParser::handleImport( ElementInfo const & /*aInfo*/, const uno::Reference< sax::XAttributeList >& xAttribs )
{
    OUString aComponent;
    if (getDataParser().getImportComponent(xAttribs,aComponent))
        m_xHandler->importComponent(aComponent);

    else
        raiseParseException("Schema XML parser - Invalid data: Missing component attribute for import directive.\n");
}
// -----------------------------------------------------------------------------

void SchemaParser::handleInstance( ElementInfo const & aInfo, const uno::Reference< sax::XAttributeList >& xAttribs )
{
    backenduno::TemplateIdentifier aTemplate;
    if (getDataParser().getInstanceType(xAttribs, aTemplate.Name, aTemplate.Component))
        m_xHandler->addInstance(aInfo.name, aTemplate);

    else
        raiseParseException("Schema XML parser - Invalid data: Missing type information for instantiation directive.\n");
}
// -----------------------------------------------------------------------------

void SchemaParser::handleItemType( ElementInfo const & /*aInfo*/, const uno::Reference< sax::XAttributeList >& xAttribs )
{
    backenduno::TemplateIdentifier aTemplate;
    if (getDataParser().getInstanceType(xAttribs, aTemplate.Name, aTemplate.Component))
        m_xHandler->addItemType(aTemplate);

    else
        raiseParseException("Schema XML parser - Invalid data: Missing type information for instantiation directive.\n");
}
// -----------------------------------------------------------------------------

void SchemaParser::startNode( ElementInfo const & aInfo, const uno::Reference< sax::XAttributeList >& xAttribs )
{
    bool bStartTemplate = ( !isInNode() && m_selected == selectTemplates );

    BasicParser::startNode(aInfo,xAttribs);

    OSL_ASSERT(aInfo.type == ElementType::set || aInfo.type == ElementType::group);

    using backenduno::TemplateIdentifier;

    if (aInfo.type == ElementType::group)
    {
        if (bStartTemplate)
            m_xHandler->startGroupTemplate( TemplateIdentifier(aInfo.name,m_sComponent), aInfo.flags );

        else
            m_xHandler->startGroup( aInfo.name, aInfo.flags );
    }
    else
    {
        TemplateIdentifier aItemType;

        if (!getDataParser().getSetElementType(xAttribs, aItemType.Name, aItemType.Component))
            raiseParseException("Schema XML parser - Invalid data: Missing item-type information for set node.\n");

        if (bStartTemplate)
            m_xHandler->startSetTemplate( TemplateIdentifier(aInfo.name,m_sComponent), aInfo.flags, aItemType );

        else
            m_xHandler->startSet( aInfo.name, aInfo.flags, aItemType );
    }
}
// -----------------------------------------------------------------------------

void SchemaParser::endNode()
{
    BasicParser::endNode();

    bool bEndedTemplate = ( !isInNode() && m_selected == selectTemplates );

    if (bEndedTemplate)
        m_xHandler->endTemplate();

    else
        m_xHandler->endNode();
}
// -----------------------------------------------------------------------------

void SchemaParser::startProperty( ElementInfo const & aInfo, const uno::Reference< sax::XAttributeList >& xAttribs )
{
    BasicParser::startProperty(aInfo,xAttribs);

    OSL_ENSURE( isInUnhandledProperty(), "Property not recognizable as unhandled");
}
// -----------------------------------------------------------------------------

void SchemaParser::endProperty()
{
    if (isInUnhandledProperty())
    {
        ElementInfo const & aInfo = this->getActiveNodeInfo();

        m_xHandler->addProperty(aInfo.name,
                                aInfo.flags,
                                getActivePropertyType());
    }

    BasicParser::endProperty();
}
// -----------------------------------------------------------------------------

void SchemaParser::startValueData(const uno::Reference< sax::XAttributeList >& xAttribs)
{
    OSL_ENSURE( this->isInUnhandledProperty(),"Schema XML parser - multiple values in property are not permitted in the schema.\n");

    BasicParser::startValueData(xAttribs);

    if (this->isValueDataLocalized())
        getLogger().warning("Language attributes on values are ignored in the schema.",
                            "endValueData()","configuration::xml::SchemaParser");
}
// -----------------------------------------------------------------------------

void SchemaParser::endValueData()
{
    uno::Any aValue = this->getCurrentValue();

    ElementInfo const & aInfo = this->getActiveNodeInfo();

    if (aValue.hasValue())
    {
        m_xHandler->addPropertyWithDefault(aInfo.name,aInfo.flags,aValue);
    }
    else
    {
        getLogger().warning("Found deprecated explicit NIL value in schema data.",
                            "endValueData()","configuration::xml::SchemaParser");
        m_xHandler->addProperty(aInfo.name,aInfo.flags,getActivePropertyType());
    }

    BasicParser::endValueData();

    OSL_ENSURE( !isInUnhandledProperty(), "Property not recognizable as handled");
}
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
    } // namespace

// -----------------------------------------------------------------------------
} // namespace

