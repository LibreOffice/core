/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include <sal/config.h>

#include <string_view>
#include <com/sun/star/xml/csax/XMLAttribute.hpp>

#include <libxml/parser.h>

namespace com::sun::star::xml::csax { struct XMLAttribute; }
namespace com::sun::star::uno { template <typename > class Sequence; }

/** This class represents a SAX handler which simply forwards to
    the corresponding libxml API and translates parameter if necessary.
*/
class SAXHelper final
{
    private:
        xmlParserCtxtPtr m_pParserCtxt ;
        xmlSAXHandlerPtr m_pSaxHandler ;

    public:
        SAXHelper( ) ;
        ~SAXHelper() ;

        xmlNodePtr getCurrentNode() { return m_pParserCtxt->node;}
        void setCurrentNode(const xmlNodePtr pNode);
        xmlDocPtr getDocument() { return m_pParserCtxt->myDoc;}

        /// @throws css::xml::sax::SAXException
        /// @throws css::uno::RuntimeException
        void startDocument() ;

        /// @throws css::xml::sax::SAXException
        /// @throws css::uno::RuntimeException
        void endDocument() ;

        /// @throws css::xml::sax::SAXException
        /// @throws css::uno::RuntimeException
        void startElement(
            std::u16string_view aName ,
            const css::uno::Sequence<
                css::xml::csax::XMLAttribute >& aAttributes ) ;

        /// @throws css::xml::sax::SAXException
        /// @throws css::uno::RuntimeException
        void endElement( std::u16string_view aName ) ;

        /// @throws css::xml::sax::SAXException
        /// @throws css::uno::RuntimeException
        void characters( std::u16string_view aChars ) ;

        /// @throws css::xml::sax::SAXException
        /// @throws css::uno::RuntimeException
        void ignorableWhitespace( std::u16string_view aWhitespaces ) ;

        /// @throws css::xml::sax::SAXException
        /// @throws css::uno::RuntimeException
        void processingInstruction(
            std::u16string_view aTarget ,
            std::u16string_view aData ) ;

} ;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
