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

#ifndef _XIMP3DOBJECT_HXX
#define _XIMP3DOBJECT_HXX

#include "xmlictxt.hxx"

#include "sdxmlimp_impl.hxx"

#include "nmspmap.hxx"

#include <com/sun/star/drawing/XShapes.hpp>

#include <tools/rtti.hxx>

#include <com/sun/star/drawing/HomogenMatrix.hpp>

#include <bf_goodies/vector3d.hxx>

#include <com/sun/star/drawing/Direction3D.hpp>

#include <com/sun/star/drawing/Position3D.hpp>

#include "ximpshap.hxx"
namespace binfilter {

//////////////////////////////////////////////////////////////////////////////
// common shape context

class SdXML3DObjectContext : public SdXMLShapeContext
{
protected:
    // the shape group this object should be created inside

    ::com::sun::star::drawing::HomogenMatrix mxHomMat;
    BOOL						mbSetTransform;

/*
    void SetStyle();
    void AddShape( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape);
*/
public:
    TYPEINFO();

    SdXML3DObjectContext( SvXMLImport& rImport, 
        sal_uInt16 nPrfx,
        const ::rtl::OUString& rLocalName, 
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXML3DObjectContext();

    virtual void StartElement(const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList>& xAttrList);
    virtual void EndElement();
};

//////////////////////////////////////////////////////////////////////////////
// dr3d:3dcube context

class SdXML3DCubeObjectShapeContext : public SdXML3DObjectContext
{
    Vector3D				maMinEdge;
    Vector3D				maMaxEdge;
    BOOL					mbMinEdgeUsed;
    BOOL					mbMaxEdgeUsed;

public:
    TYPEINFO();

    SdXML3DCubeObjectShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const ::rtl::OUString& rLocalName, 
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXML3DCubeObjectShapeContext();

    virtual void StartElement(const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList>& xAttrList);
    virtual void EndElement();
};

//////////////////////////////////////////////////////////////////////////////
// dr3d:3dsphere context

class SdXML3DSphereObjectShapeContext : public SdXML3DObjectContext
{
    Vector3D				maCenter;
    Vector3D				maSize;
    BOOL					mbCenterUsed;
    BOOL					mbSizeUsed;

public:
    TYPEINFO();

    SdXML3DSphereObjectShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const ::rtl::OUString& rLocalName, 
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXML3DSphereObjectShapeContext();

    virtual void StartElement(const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList>& xAttrList);
    virtual void EndElement();
};

//////////////////////////////////////////////////////////////////////////////
// polygonbased context

class SdXML3DPolygonBasedShapeContext : public SdXML3DObjectContext
{
    ::rtl::OUString				maPoints;
    ::rtl::OUString				maViewBox;

public:
    TYPEINFO();

    SdXML3DPolygonBasedShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const ::rtl::OUString& rLocalName, 
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXML3DPolygonBasedShapeContext();

    virtual void StartElement(const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList>& xAttrList);
    virtual void EndElement();
};

//////////////////////////////////////////////////////////////////////////////
// dr3d:3dlathe context

class SdXML3DLatheObjectShapeContext : public SdXML3DPolygonBasedShapeContext
{
public:
    TYPEINFO();

    SdXML3DLatheObjectShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const ::rtl::OUString& rLocalName, 
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXML3DLatheObjectShapeContext();

    virtual void StartElement(const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList>& xAttrList);
    virtual void EndElement();
};

//////////////////////////////////////////////////////////////////////////////
// dr3d:3dextrude context

class SdXML3DExtrudeObjectShapeContext : public SdXML3DPolygonBasedShapeContext
{
public:
    TYPEINFO();

    SdXML3DExtrudeObjectShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const ::rtl::OUString& rLocalName, 
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXML3DExtrudeObjectShapeContext();

    virtual void StartElement(const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList>& xAttrList);
    virtual void EndElement();
};

}//end of namespace binfilter
#endif	//  _XIMP3DOBJECT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
