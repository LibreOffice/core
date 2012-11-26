/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _XIMP3DOBJECT_HXX
#define _XIMP3DOBJECT_HXX

#include <xmloff/xmlictxt.hxx>
#include "sdxmlimp_impl.hxx"
#include <xmloff/nmspmap.hxx>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/drawing/Position3D.hpp>
#include "ximpshap.hxx"

//////////////////////////////////////////////////////////////////////////////
// common shape context

class SdXML3DObjectContext : public SdXMLShapeContext
{
protected:
    // the shape group this object should be created inside

    com::sun::star::drawing::HomogenMatrix mxHomMat;
    sal_Bool                        mbSetTransform;

/*
    void SetStyle();
    void AddShape(com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape);
*/
public:
    SdXML3DObjectContext( SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        sal_Bool bTemporaryShape);
    virtual ~SdXML3DObjectContext();

    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList);
    virtual void EndElement();
};

//////////////////////////////////////////////////////////////////////////////
// dr3d:3dcube context

class SdXML3DCubeObjectShapeContext : public SdXML3DObjectContext
{
    ::basegfx::B3DVector    maMinEdge;
    ::basegfx::B3DVector    maMaxEdge;
    sal_Bool                    mbMinEdgeUsed;
    sal_Bool                    mbMaxEdgeUsed;

public:
    SdXML3DCubeObjectShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        sal_Bool bTemporaryShape);
    virtual ~SdXML3DCubeObjectShapeContext();

    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList);
    virtual void EndElement();
};

//////////////////////////////////////////////////////////////////////////////
// dr3d:3dsphere context

class SdXML3DSphereObjectShapeContext : public SdXML3DObjectContext
{
    ::basegfx::B3DVector    maCenter;
    ::basegfx::B3DVector    maSize;
    sal_Bool                    mbCenterUsed;
    sal_Bool                    mbSizeUsed;

public:
    SdXML3DSphereObjectShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        sal_Bool bTemporaryShape);
    virtual ~SdXML3DSphereObjectShapeContext();

    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList);
    virtual void EndElement();
};

//////////////////////////////////////////////////////////////////////////////
// polygonbased context

class SdXML3DPolygonBasedShapeContext : public SdXML3DObjectContext
{
    rtl::OUString               maPoints;
    rtl::OUString               maViewBox;

public:
    SdXML3DPolygonBasedShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        sal_Bool bTemporaryShape);
    virtual ~SdXML3DPolygonBasedShapeContext();

    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList);
    virtual void EndElement();
};

//////////////////////////////////////////////////////////////////////////////
// dr3d:3dlathe context

class SdXML3DLatheObjectShapeContext : public SdXML3DPolygonBasedShapeContext
{
public:
    SdXML3DLatheObjectShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        sal_Bool bTemporaryShape);
    virtual ~SdXML3DLatheObjectShapeContext();

    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList);
    virtual void EndElement();
};

//////////////////////////////////////////////////////////////////////////////
// dr3d:3dextrude context

class SdXML3DExtrudeObjectShapeContext : public SdXML3DPolygonBasedShapeContext
{
public:
    SdXML3DExtrudeObjectShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        sal_Bool bTemporaryShape);
    virtual ~SdXML3DExtrudeObjectShapeContext();

    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList);
    virtual void EndElement();
};

#endif  //  _XIMP3DOBJECT_HXX
