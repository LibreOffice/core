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

#ifndef _XMLOFF_XMLBASICEXPORTFILTER_HXX
#define _XMLOFF_XMLBASICEXPORTFILTER_HXX

#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_ 
#include <cppuhelper/implbase1.hxx>
#endif

//.........................................................................
namespace binfilter
{
//.........................................................................

    // =============================================================================
    // class XMLBasicExportFilter
    // =============================================================================

    typedef ::cppu::WeakImplHelper1<
        ::com::sun::star::xml::sax::XDocumentHandler > XMLBasicExportFilter_BASE;

    class XMLBasicExportFilter : public XMLBasicExportFilter_BASE
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >    m_xHandler;

    public:
        XMLBasicExportFilter(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >& rxHandler );
        virtual ~XMLBasicExportFilter();

        // XDocumentHandler
        virtual void SAL_CALL startDocument()
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL endDocument()
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL startElement( const ::rtl::OUString& aName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttribs )
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL endElement( const ::rtl::OUString& aName )
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL characters( const ::rtl::OUString& aChars )
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL ignorableWhitespace( const ::rtl::OUString& aWhitespaces )
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL processingInstruction( const ::rtl::OUString& aTarget, const ::rtl::OUString& aData )
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setDocumentLocator( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator >& xLocator )
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    };

//.........................................................................
}	// namespace binfilter
//.........................................................................

#endif // _XMLOFF_XMLBASICEXPORTFILTER_HXX
