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

#ifndef _XMLOFF_XMLBASICI_HXX
#define _XMLOFF_XMLBASICI_HXX

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif

#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif

//.........................................................................
namespace binfilter
{
//.........................................................................

    // =============================================================================
    // class XMLBasicImportContext
    // =============================================================================

    class XMLBasicImportContext : public SvXMLImportContext
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >                 m_xModel;
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >    m_xHandler;

    public:
        XMLBasicImportContext( SvXMLImport& rImport, USHORT nPrfx, const ::rtl::OUString& rLName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& rxModel );
                            
        virtual ~XMLBasicImportContext();

        virtual SvXMLImportContext* CreateChildContext( USHORT nPrefix, const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& rxAttrList );

        virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& rxAttrList );

        virtual void EndElement();

        virtual void Characters( const ::rtl::OUString& rChars );
    };


    // =============================================================================
    // class XMLBasicImportChildContext
    // =============================================================================

    class XMLBasicImportChildContext : public SvXMLImportContext
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >    m_xHandler;

    public:
        XMLBasicImportChildContext( SvXMLImport& rImport, USHORT nPrfx, const ::rtl::OUString& rLName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >& rxHandler );
                            
        virtual ~XMLBasicImportChildContext();

        virtual SvXMLImportContext* CreateChildContext( USHORT nPrefix, const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

        virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

        virtual void EndElement();

        virtual void Characters( const ::rtl::OUString& rChars );
    };

//.........................................................................
}	// namespace binfilter
//.........................................................................

#endif // _XMLOFF_XMLBASICI_HXX
