/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLLabelSeparatorContext.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-22 16:34:18 $
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
#ifndef XMLOFF_LABELSEPARATORCONTEXT_HXX_
#define XMLOFF_LABELSEPARATORCONTEXT_HXX_

#include "XMLElementPropertyContext.hxx"

class XMLLabelSeparatorContext : public XMLElementPropertyContext
{
public:
    TYPEINFO();

    XMLLabelSeparatorContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                           const ::rtl::OUString& rLName,
                           const XMLPropertyState& rProp,
                           ::std::vector< XMLPropertyState > &rProps );
    virtual ~XMLLabelSeparatorContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual SvXMLImportContext *CreateChildContext(
        USHORT nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();

private:
    ::rtl::OUString         m_aSeparator;
};

#endif  // _XMLOFF_SYMBOLIMAGECONTEXT_HXX_
