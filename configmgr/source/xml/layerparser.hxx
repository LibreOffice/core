/*************************************************************************
 *
 *  $RCSfile: layerparser.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-17 13:34:54 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef CONFIGMGR_XML_LAYERPARSER_HXX
#define CONFIGMGR_XML_LAYERPARSER_HXX

#ifndef CONFIGMGR_XML_BASICPARSER_HXX
#include "basicparser.hxx"
#endif

#include <com/sun/star/configuration/backend/XLayerHandler.hpp>

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


        class LayerParser : public BasicParser
        {
        public:
            typedef uno::Reference< backenduno::XLayerHandler > HandlerRef;

        public:
            LayerParser(ServiceFactory const & _xSvcFactory, HandlerRef const & _xHandler);
            virtual ~LayerParser();

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

            /**
              Forces the addition or replacement of a property.
              As it is possible to "replace" an existing property,
              even though this amounts to a modify, this method
              first tries to add a new property and failing that,
              to replace the value of an existing one.

              @param aValue value to be set for the property
              */
            void addOrReplaceCurrentProperty(const uno::Any& aValue) ;

            bool isInRemoved() const { return m_bRemoved; }
            void checkNotRemoved();
        private:
            HandlerRef  m_xHandler;
            bool        m_bRemoved;
            bool        m_bNewProp;
        };
// -----------------------------------------------------------------------------
    } // namespace xml
// -----------------------------------------------------------------------------

} // namespace configmgr
#endif




