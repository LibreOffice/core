/*************************************************************************
 *
 *  $RCSfile: schemaparser.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: jb $ $Date: 2002-08-23 11:15:37 $
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

#include "schemaparser.hxx"

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

SchemaParser::SchemaParser(ServiceFactory const & _xSvcFactory, uno::Reference< backenduno::XSchemaHandler > const & _xHandler, Select _selector)
: BasicParser(_xSvcFactory)
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

    OSL_ENSURE(aInfo.op == Operation::none || this->isSkipping(),
                "Schema Parser: The 'op' attribute is not supported in a schema");
}
// -----------------------------------------------------------------------------

void SAL_CALL SchemaParser::endElement( const OUString& aName )
        throw (sax::SAXException, uno::RuntimeException)
{
    if ( this->wasSkipping(aName) )
        return;

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
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void SchemaParser::startSchema( ElementInfo const & aInfo, const uno::Reference< sax::XAttributeList >& xAttribs )
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

void SchemaParser::handleImport( ElementInfo const & aInfo, const uno::Reference< sax::XAttributeList >& xAttribs )
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

void SchemaParser::handleItemType( ElementInfo const & aInfo, const uno::Reference< sax::XAttributeList >& xAttribs )
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

        // HACK: If type is string we use an empty string as default value
        // ('Normal' behavior would yield 'no default' aka NULL)
        if (getActivePropertyType().getTypeClass() == uno::TypeClass_STRING)
        {
            m_xHandler->addPropertyWithDefault( aInfo.name,
                                                aInfo.flags,
                                                uno::makeAny(OUString()) );
        }
        else
        {
            m_xHandler->addProperty(aInfo.name,
                                    aInfo.flags,
                                    getActivePropertyType());
        }
    }

    BasicParser::endProperty();
}
// -----------------------------------------------------------------------------

void SchemaParser::startValueData(const uno::Reference< sax::XAttributeList >& xAttribs)
{
    OSL_ENSURE( this->isInUnhandledProperty(),"Schema XML parser - multiple values in property are not permitted in the schema.\n");

    BasicParser::startValueData(xAttribs);

    OSL_ENSURE(!this->isValueDataLocalized(),"Schema XML parser - language attributes on values are ignored in the schema.\n");
}
// -----------------------------------------------------------------------------

void SchemaParser::endValueData()
{
    uno::Any aValue = this->getCurrentValue();

    ElementInfo const & aInfo = this->getActiveNodeInfo();

    if (aValue.hasValue())
        m_xHandler->addPropertyWithDefault(aInfo.name,aInfo.flags,aValue);

    else
        m_xHandler->addProperty(aInfo.name,aInfo.flags,getActivePropertyType());

    BasicParser::endValueData();

    OSL_ENSURE( !isInUnhandledProperty(), "Property not recognizable as handled");
}
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
    } // namespace

// -----------------------------------------------------------------------------
} // namespace

