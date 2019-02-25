/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_XMLOFF_SOURCE_DRAW_XIMP3DOBJECT_HXX
#define INCLUDED_XMLOFF_SOURCE_DRAW_XIMP3DOBJECT_HXX

#include <xmloff/xmlictxt.hxx>
#include "sdxmlimp_impl.hxx"
#include <xmloff/nmspmap.hxx>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/drawing/Position3D.hpp>
#include "ximpshap.hxx"

// common shape context

class SdXML3DObjectContext : public SdXMLShapeContext
{
    // the shape group this object should be created inside

    css::drawing::HomogenMatrix mxHomMat;
    bool                        mbSetTransform;

public:

    SdXML3DObjectContext( SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes > const & rShapes);
    virtual ~SdXML3DObjectContext() override;

    virtual void StartElement(const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList) override;
};

// dr3d:3dcube context

class SdXML3DCubeObjectShapeContext : public SdXML3DObjectContext
{
    ::basegfx::B3DVector    maMinEdge;
    ::basegfx::B3DVector    maMaxEdge;

public:

    SdXML3DCubeObjectShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes > const & rShapes);
    virtual ~SdXML3DCubeObjectShapeContext() override;

    virtual void StartElement(const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList) override;
};

// dr3d:3dsphere context

class SdXML3DSphereObjectShapeContext : public SdXML3DObjectContext
{
    ::basegfx::B3DVector    maCenter;
    ::basegfx::B3DVector    maSphereSize;

public:

    SdXML3DSphereObjectShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes > const & rShapes);
    virtual ~SdXML3DSphereObjectShapeContext() override;

    virtual void StartElement(const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList) override;
};

// polygonbased context

class SdXML3DPolygonBasedShapeContext : public SdXML3DObjectContext
{
    OUString               maPoints;
    OUString               maViewBox;

public:

    SdXML3DPolygonBasedShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes > const & rShapes);
    virtual ~SdXML3DPolygonBasedShapeContext() override;

    virtual void StartElement(const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList) override;
};

// dr3d:3dlathe context

class SdXML3DLatheObjectShapeContext : public SdXML3DPolygonBasedShapeContext
{
public:

    SdXML3DLatheObjectShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes > const & rShapes);
    virtual ~SdXML3DLatheObjectShapeContext() override;

    virtual void StartElement(const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList) override;
};

// dr3d:3dextrude context

class SdXML3DExtrudeObjectShapeContext : public SdXML3DPolygonBasedShapeContext
{
public:

    SdXML3DExtrudeObjectShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes > const & rShapes);
    virtual ~SdXML3DExtrudeObjectShapeContext() override;

    virtual void StartElement(const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList) override;
};

#endif // INCLUDED_XMLOFF_SOURCE_DRAW_XIMP3DOBJECT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
