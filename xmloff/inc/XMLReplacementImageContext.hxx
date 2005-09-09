/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLReplacementImageContext.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:54:13 $
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

#ifndef _XMLOFF_XMLREPLACEMENTIMAGECONTEXT_HXX
#define _XMLOFF_XMLREPLACEMENTIMAGECONTEXT_HXX

#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif

#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif


namespace com { namespace sun { namespace star {
    namespace beans { class XPropertySet; }
    namespace io { class XOutputStream; } } } }

class XMLReplacementImageContext : public SvXMLImportContext
{
    ::com::sun::star::uno::Reference <
        ::com::sun::star::io::XOutputStream > m_xBase64Stream;
::com::sun::star::uno::Reference <
        ::com::sun::star::beans::XPropertySet > m_xPropSet;

    ::rtl::OUString m_sHRef;
    const ::rtl::OUString m_sGraphicURL;

public:

    TYPEINFO();

    XMLReplacementImageContext( SvXMLImport& rImport,
            sal_uInt16 nPrfx,
            const ::rtl::OUString& rLName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & rAttrList,
            const ::com::sun::star::uno::Reference <
                ::com::sun::star::beans::XPropertySet >& rPropSet );
    virtual ~XMLReplacementImageContext();

    virtual void EndElement();

    SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                const ::rtl::OUString& rLocalName,
                 const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

};

#endif  //  _XMLOFF_XMLREPLACEMENTIMAGECONTEXT_HXX

