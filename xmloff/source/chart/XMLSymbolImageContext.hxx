/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLSymbolImageContext.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 13:28:53 $
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
#ifndef _XMLOFF_SYMBOLIMAGECONTEXT_HXX_
#define _XMLOFF_SYMBOLIMAGECONTEXT_HXX_

#include "XMLElementPropertyContext.hxx"

namespace com { namespace sun { namespace star {
    namespace io { class XOutputStream; }
} } }

class XMLSymbolImageContext : public XMLElementPropertyContext
{
public:
    TYPEINFO();

    XMLSymbolImageContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                           const ::rtl::OUString& rLName,
                           const XMLPropertyState& rProp,
                           ::std::vector< XMLPropertyState > &rProps );
    virtual ~XMLSymbolImageContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual SvXMLImportContext *CreateChildContext(
        USHORT nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();

private:
    ::rtl::OUString msURL;
    ::com::sun::star::uno::Reference < ::com::sun::star::io::XOutputStream > mxBase64Stream;
};

#endif  // _XMLOFF_SYMBOLIMAGECONTEXT_HXX_
