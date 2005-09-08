/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: schemaparser.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 04:41:59 $
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

#ifndef CONFIGMGR_XML_SCHEMAPARSER_HXX
#define CONFIGMGR_XML_SCHEMAPARSER_HXX

#ifndef CONFIGMGR_XML_BASICPARSER_HXX
#include "basicparser.hxx"
#endif

#include <com/sun/star/configuration/backend/XSchemaHandler.hpp>

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace xml
    {
// -----------------------------------------------------------------------------
        namespace uno       = ::com::sun::star::uno;
        namespace lang      = ::com::sun::star::lang;

        namespace sax           = ::com::sun::star::xml::sax;
        namespace backenduno    = ::com::sun::star::configuration::backend;

// -----------------------------------------------------------------------------


        class SchemaParser :    public BasicParser
        {
        public:
            typedef uno::Reference< backenduno::XSchemaHandler > HandlerRef;
            enum Select {
                selectNone = 0,
                selectComponent = 0x01,
                selectTemplates = 0x02,
                selectAll = 0x03
            };

        public:
            SchemaParser(Context const & _xContext, HandlerRef const & _xHandler, Select _selector);
            virtual ~SchemaParser();

        // XDocumentHandler
        public:
            virtual void SAL_CALL
                startDocument(  )
                    throw (sax::SAXException, uno::RuntimeException);

            virtual void SAL_CALL
                endDocument(  ) throw (sax::SAXException, uno::RuntimeException);

            virtual void SAL_CALL
                startElement( const OUString& aName, const uno::Reference< sax::XAttributeList >& xAttribs )
                    throw (sax::SAXException, uno::RuntimeException);

            virtual void SAL_CALL
                endElement( const OUString& aName )
                    throw (sax::SAXException, uno::RuntimeException);

        private:
            /// start the schema
            void startSchema( ElementInfo const & aInfo, const uno::Reference< sax::XAttributeList >& xAttribs );
           /// end the schema
            void endSchema();

            /// start a section (components or templates)
            void startSection( Select _select, ElementInfo const & aInfo, const uno::Reference< sax::XAttributeList >& xAttribs );
           /// end the current section
            void endSection();

            /// handle a import directive element
            void handleImport( ElementInfo const & aInfo, const uno::Reference< sax::XAttributeList >& xAttribs );

            /// start an node
            void startNode( ElementInfo const & aInfo, const uno::Reference< sax::XAttributeList >& xAttribs );
           /// end a node
            void endNode();

            /// start a property
            void startProperty( ElementInfo const & aInfo, const uno::Reference< sax::XAttributeList >& xAttribs );
            /// end a property
            void endProperty();

            /// start collecting data for a value - returns the locale of the value (property must have been started)
            void startValueData(const uno::Reference< sax::XAttributeList >& xAttribs);
            /// end collecting data for a value - returns the collected value
            void endValueData();

            /// handle a instance ref element
            void handleInstance( ElementInfo const & aInfo, const uno::Reference< sax::XAttributeList >& xAttribs );
            /// handle a item-type declaration element
            void handleItemType( ElementInfo const & aInfo, const uno::Reference< sax::XAttributeList >& xAttribs );

            bool isSelected()   const { return m_selected != selectNone; }
            bool select(Select _select);
        private:
            HandlerRef  m_xHandler;
            OUString    m_sComponent;
            Select      m_selector;
            Select      m_selected;
        };
// -----------------------------------------------------------------------------
    } // namespace xml
// -----------------------------------------------------------------------------

} // namespace configmgr
#endif




