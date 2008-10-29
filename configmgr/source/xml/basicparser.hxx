/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: basicparser.hxx,v $
 * $Revision: 1.10 $
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

#ifndef CONFIGMGR_XML_BASICPARSER_HXX
#define CONFIGMGR_XML_BASICPARSER_HXX

#include "elementparser.hxx"
#include "utility.hxx"
#include "stack.hxx"
#ifndef CONFIGMGR_LOGGER_HXX_
#include "logger.hxx"
#endif
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <cppuhelper/implbase1.hxx>

namespace com { namespace sun { namespace star { namespace script {
    class XTypeConverter;
} } } }

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace xml
    {
// -----------------------------------------------------------------------------
        namespace uno           = ::com::sun::star::uno;
        namespace lang          = ::com::sun::star::lang;

        namespace sax           = ::com::sun::star::xml::sax;
// -----------------------------------------------------------------------------

        class BasicParser
        :   public cppu::WeakImplHelper1<sax::XDocumentHandler>
        {
            struct ValueData;

            uno::Reference< com::sun::star::script::XTypeConverter >
                m_xTypeConverter;
            uno::Reference< sax::XLocator > m_xLocator;
            ElementParser                   m_aDataParser;
            Stack< ElementInfo >            m_aNodes;
            uno::Type                       m_aValueType;
            ValueData *                     m_pValueData;
            sal_uInt16                      m_nSkipLevels;
            bool                            m_bEmpty;
            bool                            m_bInProperty;

#if OSL_DEBUG_LEVEL > 0
#ifdef DBG_UTIL
            rtl::OUString  dbgPublicId,    dbgSystemId;
            sal_Int32 dbgLineNo,      dbgColumnNo;
#endif // DBG_UTIL
            void dbgUpdateLocation();
#endif // OSL_DEBUG_LEVEL

        public:
            explicit BasicParser(uno::Reference< uno::XComponentContext > const & _xContext);
            virtual ~BasicParser();

        // XDocumentHandler
        public:
            virtual void SAL_CALL
                startDocument(  )
                    throw (sax::SAXException, uno::RuntimeException);

            virtual void SAL_CALL
                endDocument(  ) throw (sax::SAXException, uno::RuntimeException);

            virtual void SAL_CALL
                characters( const rtl::OUString& aChars )
                    throw (sax::SAXException, uno::RuntimeException);

            virtual void SAL_CALL
                ignorableWhitespace( const rtl::OUString& aWhitespaces )
                    throw (sax::SAXException, uno::RuntimeException);

            virtual void SAL_CALL
                processingInstruction( const rtl::OUString& aTarget, const rtl::OUString& aData )
                    throw (sax::SAXException, uno::RuntimeException);

            virtual void SAL_CALL
                setDocumentLocator( const uno::Reference< sax::XLocator >& xLocator )
                    throw (sax::SAXException, uno::RuntimeException);

        protected:
            ElementParser const & getDataParser() const { return m_aDataParser; }

            Logger const & getLogger() { return m_aDataParser.logger(); }

            /// start an node
            void startNode( ElementInfo const & aInfo, const uno::Reference< sax::XAttributeList >& xAttribs );
            /// are we in the content of a  node ?
            bool isInNode();
            /// are we in the content of node for which no content was started yet ?
            bool isEmptyNode();
            /// make sure we are in the content of a  node ?
            void ensureInNode();
            /// get the info about of the node currently being processed
            ElementInfo const & getActiveNodeInfo();
            /// end a node
            void endNode();

            /// start a property
            void startProperty( ElementInfo const & aInfo, const uno::Reference< sax::XAttributeList >& xAttribs );
            /// are we in the content of a property node ?
            bool isInProperty();
            /// are we in the content of a property node (and there has been no value for that property) ?
            bool isInUnhandledProperty();
            /// get the data type of the active property ?
            uno::Type getActivePropertyType();
            /// end a property
            void endProperty();

            /// start collecting data for a value - returns the locale of the value (property must have been started)
            void startValueData(const uno::Reference< sax::XAttributeList >& xAttribs);
            /// are we in the content of a property node ?
            bool isInValueData();
            /// check if the current value data has a locale set
            bool isValueDataLocalized();
            /// get the locale of the current value data, if localized
            rtl::OUString getValueDataLocale();
            /// return the collected value
            uno::Any getCurrentValue();
            /// end collecting data for a value
            void endValueData();

            /// start a node to be skipped
            void startSkipping( const rtl::OUString& aTag, const uno::Reference< sax::XAttributeList >& xAttribs );
            /// are we inside a skipped node ?
            bool isSkipping( );
            /// ending a node: was this skipped ?
            bool wasSkipping( const rtl::OUString& aTag );

        protected:
            void raiseParseException( uno::Any const & _aTargetException, sal_Char const * _pMsg = NULL)
                SAL_THROW((sax::SAXException, uno::RuntimeException));
            void raiseParseException( sal_Char const * _pMsg )
                SAL_THROW((sax::SAXException, uno::RuntimeException));
            void raiseParseException( rtl::OUString const & aMsg )
                SAL_THROW((sax::SAXException, uno::RuntimeException));
        };
// -----------------------------------------------------------------------------
    } // namespace xml
// -----------------------------------------------------------------------------

} // namespace configmgr
#endif




