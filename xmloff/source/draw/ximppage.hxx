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

#ifndef _XIMPPAGE_HXX
#define _XIMPPAGE_HXX

#include <xmloff/xmlictxt.hxx>
#include "sdxmlimp_impl.hxx"
#include <xmloff/nmspmap.hxx>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/office/XAnnotationAccess.hpp>
#include <tools/rtti.hxx>
#include "ximpshap.hxx"

//////////////////////////////////////////////////////////////////////////////
// draw:g context (RECURSIVE)

class SdXMLGenericPageContext : public SvXMLImportContext
{
    // the shape group this group is working on
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes > mxShapes;
    ::com::sun::star::uno::Reference< ::com::sun::star::office::XAnnotationAccess > mxAnnotationAccess;

protected:
    rtl::OUString               maPageLayoutName;
    rtl::OUString               maUseHeaderDeclName;
    rtl::OUString               maUseFooterDeclName;
    rtl::OUString               maUseDateTimeDeclName;
    rtl::OUString               msNavOrder;

    void SetLocalShapesContext(com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rNew)
        { mxShapes = rNew; }

    /** sets the page style on this page */
    void SetStyle( rtl::OUString& rStyleName );

    /** sets the presentation layout at this page. It is used for drawing pages and for the handout master */
    void SetLayout();

    /** deletes all shapes on this drawing page */
    void DeleteAllShapes();

    const SdXMLImport& GetSdImport() const { return (const SdXMLImport&)GetImport(); }
    SdXMLImport& GetSdImport() { return (SdXMLImport&)GetImport(); }

    /** sets the properties from a page master style with the given name on this contexts page */
    void SetPageMaster( rtl::OUString& rsPageMasterName );

    void SetNavigationOrder();

public:
    TYPEINFO();

    SdXMLGenericPageContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXMLGenericPageContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix, const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void EndElement();

    const com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& GetLocalShapesContext() const
        { return mxShapes; }
    com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& GetLocalShapesContext()
        { return mxShapes; }
};


#endif  //  _XIMPGROUP_HXX
