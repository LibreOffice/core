/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLChartStyleContext.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-22 16:31:45 $
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
#ifndef _XMLOFF_XMLCHARTSTYLECONTEXT_HXX_
#define _XMLOFF_XMLCHARTSTYLECONTEXT_HXX_

#ifndef _XMLOFF_XMLSHAPESTYLECONTEXT_HXX
#include <xmloff/XMLShapeStyleContext.hxx>
#endif

class XMLChartStyleContext : public XMLShapeStyleContext
{
private:
    ::rtl::OUString msDataStyleName;
    ::rtl::OUString msPercentageDataStyleName;
    SvXMLStylesContext& mrStyles;

protected:
    /// is called when an attribute at the (auto)style element is found
    virtual void SetAttribute( sal_uInt16 nPrefixKey,
                               const ::rtl::OUString& rLocalName,
                               const ::rtl::OUString& rValue );

public:
    TYPEINFO();

    XMLChartStyleContext(
        SvXMLImport& rImport, sal_uInt16 nPrfx,
        const ::rtl::OUString& rLName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
        SvXMLStylesContext& rStyles, sal_uInt16 nFamily );
    virtual ~XMLChartStyleContext();

    /// is called after all styles have been read to apply styles
    void FillPropertySet(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > & rPropSet );

    /// necessary for property context (element-property symbol-image)
    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
};

#endif  // _XMLOFF_XMLCHARTSTYLECONTEXT_HXX_
