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

#ifndef _XIMPBODY_HXX
#define _XIMPBODY_HXX

#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif

#ifndef _SDXMLIMP_IMPL_HXX
#include "sdxmlimp_impl.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XIMPPAGE_HXX
#include "ximppage.hxx"
#endif
namespace binfilter {

//////////////////////////////////////////////////////////////////////////////
// draw:page context

class SdXMLDrawPageContext : public SdXMLGenericPageContext
{
    ::rtl::OUString				maName;
    ::rtl::OUString				maStyleName;
    ::rtl::OUString				maMasterPageName;
    ::rtl::OUString				maHREF;

public:
    SdXMLDrawPageContext( SdXMLImport& rImport, USHORT nPrfx,
        const ::rtl::OUString& rLocalName, 
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXMLDrawPageContext();

    virtual SvXMLImportContext *CreateChildContext( 
        USHORT nPrefix, const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void EndElement();

};

//////////////////////////////////////////////////////////////////////////////
// office:body context

class SdXMLBodyContext : public SvXMLImportContext
{
    const SdXMLImport& GetSdImport() const { return (const SdXMLImport&)GetImport(); }
    SdXMLImport& GetSdImport() { return (SdXMLImport&)GetImport(); }

public:
    SdXMLBodyContext( SdXMLImport& rImport, USHORT nPrfx, const ::rtl::OUString& rLocalName );
    virtual ~SdXMLBodyContext();

    virtual SvXMLImportContext *CreateChildContext( 
        USHORT nPrefix, const ::rtl::OUString& rLocalName, 
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
};


}//end of namespace binfilter
#endif	//  _XIMPBODY_HXX
