/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _XIMP3DSCENE_HXX
#define _XIMP3DSCENE_HXX

#include "xmlictxt.hxx"

#include "sdxmlimp_impl.hxx"

#include "nmspmap.hxx"

#include <com/sun/star/drawing/XShapes.hpp>

#include <tools/rtti.hxx>

#include <tools/color.hxx>

#include <ximpshap.hxx>
namespace binfilter {

//////////////////////////////////////////////////////////////////////////////
// dr3d:3dscene context

class SdXML3DSceneShapeContext : public SdXMLShapeContext, public SdXML3DSceneAttributesHelper
{
    // the shape group this group is working on
    // this is the scene at the same time
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes > mxChilds;

    const SdXMLImport& GetSdImport() const { return (const SdXMLImport&)GetImport(); }
    SdXMLImport& GetSdImport() { return (SdXMLImport&)GetImport(); }

protected:
    void SetLocalShapesContext( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rNew)
        { mxChilds = rNew; }
    
public:
    TYPEINFO();

    SdXML3DSceneShapeContext( 
        SvXMLImport& rImport, 
        USHORT nPrfx, 
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXML3DSceneShapeContext();

    virtual void StartElement(const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList>& xAttrList);
    virtual void EndElement();

    virtual SvXMLImportContext *CreateChildContext( 
        USHORT nPrefix, const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& GetLocalShapesContext() const
        { return mxShapes; }
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& GetLocalShapesContext()
        { return mxShapes; }
};


}//end of namespace binfilter
#endif	//  _XIMPGROUP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
