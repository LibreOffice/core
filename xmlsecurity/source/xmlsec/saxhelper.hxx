/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
            const ::rtl::OUString& aName ,
            const com::sun::star::uno::Sequence<
                com::sun::star::xml::csax::XMLAttribute >& aAttributes )
            throw( ::com::sun::star::xml::sax::SAXException , ::com::sun::star::uno::RuntimeException ) ;

        void endElement( const ::rtl::OUString& aName )
            throw( ::com::sun::star::xml::sax::SAXException , ::com::sun::star::uno::RuntimeException ) ;

        void characters( const ::rtl::OUString& aChars )
            throw( ::com::sun::star::xml::sax::SAXException , ::com::sun::star::uno::RuntimeException ) ;

        void ignorableWhitespace( const ::rtl::OUString& aWhitespaces )
            throw( ::com::sun::star::xml::sax::SAXException , ::com::sun::star::uno::RuntimeException ) ;

        void processingInstruction(
            const ::rtl::OUString& aTarget ,
            const ::rtl::OUString& aData )
            throw( ::com::sun::star::xml::sax::SAXException , ::com::sun::star::uno::RuntimeException ) ;

        void setDocumentLocator( const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XLocator > & xLocator )
            throw( ::com::sun::star::xml::sax::SAXException , ::com::sun::star::uno::RuntimeException ) ;
} ;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
