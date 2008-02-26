/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlmetae.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 13:30:44 $
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

#ifndef _XMLOFF_XMLMETAE_HXX
#define _XMLOFF_XMLMETAE_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_XMLOFF_DLLAPI_H
#include "xmloff/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

#include <cppuhelper/implbase1.hxx>
#include <xmloff/xmltoken.hxx>

#include <vector>

#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>


class SvXMLExport;

/** export meta data from an <type>XDocumentProperties</type> instance.

    <p>
    This class will start the export at the office:meta element,
    not at the root element. This means that when <method>Export</method>
    is called here, the document root element must already be written, but
    office:meta must <em>not</em> be written.
    </p>
 */
class XMLOFF_DLLPUBLIC SvXMLMetaExport : public ::cppu::WeakImplHelper1<
                ::com::sun::star::xml::sax::XDocumentHandler >
{
private:
    SvXMLExport& mrExport;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::document::XDocumentProperties> mxDocProps;
    /// counts levels of the xml document. necessary for special handling.
    int m_level;
    /// preserved namespaces. necessary because we do not write the root node.
    std::vector< ::com::sun::star::beans::StringPair > m_preservedNSs;

    SAL_DLLPRIVATE void SimpleStringElement(
        const ::rtl::OUString& rText, sal_uInt16 nNamespace,
        enum ::xmloff::token::XMLTokenEnum eElementName );
    SAL_DLLPRIVATE void SimpleDateTimeElement(
        const ::com::sun::star::util::DateTime & rDate, sal_uInt16 nNamespace,
        enum ::xmloff::token::XMLTokenEnum eElementName );

    /// currently unused; for exporting via the XDocumentProperties interface
    SAL_DLLPRIVATE void _Export();

public:
    SvXMLMetaExport( SvXMLExport& i_rExport,
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::document::XDocumentProperties>& i_rDocProps);

    virtual ~SvXMLMetaExport();

    /// export via XSAXWriter interface, with fallback to _Export
    void Export();

    static ::rtl::OUString GetISODateTimeString(
                        const ::com::sun::star::util::DateTime& rDateTime );

    // ::com::sun::star::xml::sax::XDocumentHandler:
    virtual void SAL_CALL startDocument()
        throw (::com::sun::star::uno::RuntimeException,
               ::com::sun::star::xml::sax::SAXException);
    virtual void SAL_CALL endDocument()
        throw (::com::sun::star::uno::RuntimeException,
               ::com::sun::star::xml::sax::SAXException);
    virtual void SAL_CALL startElement(const ::rtl::OUString & i_rName,
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & i_xAttribs)
        throw (::com::sun::star::uno::RuntimeException,
               ::com::sun::star::xml::sax::SAXException);
    virtual void SAL_CALL endElement(const ::rtl::OUString & i_rName)
        throw (::com::sun::star::uno::RuntimeException,
               ::com::sun::star::xml::sax::SAXException);
    virtual void SAL_CALL characters(const ::rtl::OUString & i_rChars)
        throw (::com::sun::star::uno::RuntimeException,
               ::com::sun::star::xml::sax::SAXException);
    virtual void SAL_CALL ignorableWhitespace(
        const ::rtl::OUString & i_rWhitespaces)
        throw (::com::sun::star::uno::RuntimeException,
               ::com::sun::star::xml::sax::SAXException);
    virtual void SAL_CALL processingInstruction(
        const ::rtl::OUString & i_rTarget, const ::rtl::OUString & i_rData)
        throw (::com::sun::star::uno::RuntimeException,
               ::com::sun::star::xml::sax::SAXException);
    virtual void SAL_CALL setDocumentLocator(
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XLocator > & i_xLocator)
        throw (::com::sun::star::uno::RuntimeException,
               ::com::sun::star::xml::sax::SAXException);

};

#endif // _XMLOFF_XMLMETAE_HXX

