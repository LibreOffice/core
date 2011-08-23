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

#ifndef _XMLBACKGROUNDIMAGECONTEXT_HXX
#define _XMLBACKGROUNDIMAGECONTEXT_HXX

#ifndef _COM_SUN_STAR_STYLE_GRAPHICLOCATION_HPP_ 
#include <com/sun/star/style/GraphicLocation.hpp>
#endif

#ifndef _XMLOFF_XMLELEMENTPROPERTYCONTEXT_HXX
#include "XMLElementPropertyContext.hxx"
#endif

namespace com { namespace sun { namespace star {
    namespace io { class XOutputStream; }
} } }
namespace binfilter {

class XMLBackgroundImageContext : public XMLElementPropertyContext
{
    XMLPropertyState aPosProp;
    XMLPropertyState aFilterProp;
    XMLPropertyState aTransparencyProp;

    ::com::sun::star::style::GraphicLocation ePos;
    ::rtl::OUString sURL;
    ::rtl::OUString sFilter;
    sal_Int8 nTransparency;

    ::com::sun::star::uno::Reference < ::com::sun::star::io::XOutputStream > xBase64Stream;

private:
    void ProcessAttrs(
            const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

public:
    TYPEINFO();

    XMLBackgroundImageContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const ::rtl::OUString& rLName,
        const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
        const XMLPropertyState& rProp,
        sal_Int32 nPosIdx,
        sal_Int32 nFilterIdx,
        sal_Int32 nTransparencyIdx,
        ::std::vector< XMLPropertyState > &rProps );

    virtual ~XMLBackgroundImageContext();

    SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                const ::rtl::OUString& rLocalName,
                 const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual void EndElement();
};


}//end of namespace binfilter
#endif

