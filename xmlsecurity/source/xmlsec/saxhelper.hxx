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

#ifndef _SAXHELPER_HXX
#define _SAXHELPER_HXX

#include "libxml/tree.h"

#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/sax/XLocator.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/xml/csax/XMLAttribute.hpp>

/** This class represents a SAX handler which simply forwards to
    the corresponding libxml API and translates parameter if necessary.
*/
class SAXHelper
{
    private :
        xmlParserCtxtPtr m_pParserCtxt ;
        xmlSAXHandlerPtr m_pSaxHandler ;

    public:
        SAXHelper( ) ;
        virtual ~SAXHelper() ;

        xmlNodePtr getCurrentNode();
        void setCurrentNode(const xmlNodePtr pNode);
        xmlDocPtr getDocument();

        void startDocument( void )
            throw( ::com::sun::star::xml::sax::SAXException , ::com::sun::star::uno::RuntimeException ) ;

        void endDocument( void )
            throw( ::com::sun::star::xml::sax::SAXException , ::com::sun::star::uno::RuntimeException ) ;

        void startElement(
            const OUString& aName ,
            const com::sun::star::uno::Sequence<
                com::sun::star::xml::csax::XMLAttribute >& aAttributes )
            throw( ::com::sun::star::xml::sax::SAXException , ::com::sun::star::uno::RuntimeException ) ;

        void endElement( const OUString& aName )
            throw( ::com::sun::star::xml::sax::SAXException , ::com::sun::star::uno::RuntimeException ) ;

        void characters( const OUString& aChars )
            throw( ::com::sun::star::xml::sax::SAXException , ::com::sun::star::uno::RuntimeException ) ;

        void ignorableWhitespace( const OUString& aWhitespaces )
            throw( ::com::sun::star::xml::sax::SAXException , ::com::sun::star::uno::RuntimeException ) ;

        void processingInstruction(
            const OUString& aTarget ,
            const OUString& aData )
            throw( ::com::sun::star::xml::sax::SAXException , ::com::sun::star::uno::RuntimeException ) ;

        void setDocumentLocator( const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XLocator > & xLocator )
            throw( ::com::sun::star::xml::sax::SAXException , ::com::sun::star::uno::RuntimeException ) ;
} ;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
