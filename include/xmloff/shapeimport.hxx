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

#ifndef INCLUDED_XMLOFF_SHAPEIMPORT_HXX
#define INCLUDED_XMLOFF_SHAPEIMPORT_HXX

#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>
#include <salhelper/simplereferenceobject.hxx>
#include <xmloff/xmlictxt.hxx>

#include <xmloff/table/XMLTableImport.hxx>
#include <basegfx/vector/b3dvector.hxx>
#include <vector>
#include <memory>

class SvXMLImport;
class SvXMLImportContext;
class SvXMLTokenMap;
class SvXMLStylesContext;
class XMLSdPropHdlFactory;
class XMLPropertySetMapper;
class SvXMLImportPropertyMapper;


enum SdXMLGroupShapeElemTokenMap
{
    XML_TOK_GROUP_GROUP,
    XML_TOK_GROUP_RECT,
    XML_TOK_GROUP_LINE,
    XML_TOK_GROUP_CIRCLE,
    XML_TOK_GROUP_ELLIPSE,
    XML_TOK_GROUP_POLYGON,
    XML_TOK_GROUP_POLYLINE,
    XML_TOK_GROUP_PATH,

    XML_TOK_GROUP_CONTROL,
    XML_TOK_GROUP_CONNECTOR,
    XML_TOK_GROUP_MEASURE,
    XML_TOK_GROUP_PAGE,
    XML_TOK_GROUP_CAPTION,

    XML_TOK_GROUP_CHART,
    XML_TOK_GROUP_3DSCENE,

    XML_TOK_GROUP_FRAME,

    XML_TOK_GROUP_CUSTOM_SHAPE,

    XML_TOK_GROUP_ANNOTATION,

    XML_TOK_GROUP_A
};

enum SdXMLFrameShapeElemTokenMap
{
    XML_TOK_FRAME_TEXT_BOX,
    XML_TOK_FRAME_IMAGE,
    XML_TOK_FRAME_OBJECT,
    XML_TOK_FRAME_OBJECT_OLE,
    XML_TOK_FRAME_PLUGIN,
    XML_TOK_FRAME_FRAME,
    XML_TOK_FRAME_FLOATING_FRAME,
    XML_TOK_FRAME_APPLET,
    XML_TOK_FRAME_TABLE
};

enum SdXML3DSceneShapeElemTokenMap
{
    XML_TOK_3DSCENE_3DSCENE,
    XML_TOK_3DSCENE_3DCUBE,
    XML_TOK_3DSCENE_3DSPHERE,
    XML_TOK_3DSCENE_3DLATHE,
    XML_TOK_3DSCENE_3DEXTRUDE
};

enum SdXML3DObjectAttrTokenMap
{
    XML_TOK_3DOBJECT_DRAWSTYLE_NAME,
    XML_TOK_3DOBJECT_TRANSFORM
};

enum SdXML3DPolygonBasedAttrTokenMap
{
    XML_TOK_3DPOLYGONBASED_VIEWBOX,
    XML_TOK_3DPOLYGONBASED_D
};

enum SdXML3DCubeObjectAttrTokenMap
{
    XML_TOK_3DCUBEOBJ_MINEDGE,
    XML_TOK_3DCUBEOBJ_MAXEDGE
};

enum SdXML3DSphereObjectAttrTokenMap
{
    XML_TOK_3DSPHEREOBJ_CENTER,
    XML_TOK_3DSPHEREOBJ_SIZE
};

enum SdXML3DLightAttrTokenMap
{
    XML_TOK_3DLIGHT_DIFFUSE_COLOR,
    XML_TOK_3DLIGHT_DIRECTION,
    XML_TOK_3DLIGHT_ENABLED,
    XML_TOK_3DLIGHT_SPECULAR
};


// dr3d:3dlight context

class SdXML3DLightContext: public SvXMLImportContext
{
    // local parameters which need to be read
    sal_Int32                   maDiffuseColor;
    ::basegfx::B3DVector        maDirection;
    bool                        mbEnabled;
    bool                        mbSpecular;

public:
    SdXML3DLightContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList);
    virtual ~SdXML3DLightContext() override;

    sal_Int32 GetDiffuseColor() { return maDiffuseColor; }
    const ::basegfx::B3DVector& GetDirection() { return maDirection; }
    bool GetEnabled() { return mbEnabled; }
};


class SdXML3DSceneAttributesHelper
{
protected:
    SvXMLImport& mrImport;

    // list for local light contexts
    ::std::vector< rtl::Reference< SdXML3DLightContext > >
                                maList;

    // local parameters which need to be read
    css::drawing::HomogenMatrix mxHomMat;
    bool                        mbSetTransform;

    css::drawing::ProjectionMode mxPrjMode;
    sal_Int32                   mnDistance;
    sal_Int32                   mnFocalLength;
    sal_Int32                   mnShadowSlant;
    css::drawing::ShadeMode     mxShadeMode;
    sal_Int32                   maAmbientColor;
    bool                        mbLightingMode;

    ::basegfx::B3DVector        maVRP;
    ::basegfx::B3DVector        maVPN;
    ::basegfx::B3DVector        maVUP;
    bool                        mbVRPUsed;
    bool                        mbVPNUsed;
    bool                        mbVUPUsed;

public:
    SdXML3DSceneAttributesHelper( SvXMLImport& rImporter );

    /** creates a 3d light context and adds it to the internal list for later processing */
    SvXMLImportContext * create3DLightContext( sal_uInt16 nPrfx, const OUString& rLName, const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList);

    /** this should be called for each scene attribute */
    void processSceneAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue );

    /** this sets the scene attributes at this propertyset */
    void setSceneAttributes( const css::uno::Reference< css::beans::XPropertySet >& xPropSet );
};


class SvXMLShapeContext : public SvXMLImportContext
{
protected:
    css::uno::Reference< css::drawing::XShape >   mxShape;
    bool                                          mbTemporaryShape;
    OUString                                      msHyperlink;

public:
    SvXMLShapeContext( SvXMLImport& rImp, sal_uInt16 nPrfx,
        const OUString& rLName, bool bTemporaryShape ) : SvXMLImportContext( rImp, nPrfx, rLName ), mbTemporaryShape(bTemporaryShape) {}


    const css::uno::Reference< css::drawing::XShape >& getShape() const { return mxShape; }

    void setHyperlink( const OUString& rHyperlink );
};


class ShapeSortContext;
struct XMLShapeImportHelperImpl;
struct XMLShapeImportPageContextImpl;

class XMLOFF_DLLPUBLIC XMLShapeImportHelper : public salhelper::SimpleReferenceObject
{
    std::unique_ptr<XMLShapeImportHelperImpl> mpImpl;

    std::shared_ptr<XMLShapeImportPageContextImpl> mpPageContext;

    // PropertySetMappers and factory
    rtl::Reference<XMLSdPropHdlFactory>       mpSdPropHdlFactory;
    rtl::Reference<SvXMLImportPropertyMapper> mpPropertySetMapper;
    rtl::Reference<SvXMLImportPropertyMapper> mpPresPagePropsMapper;

    // contexts for Style and AutoStyle import
    rtl::Reference<SvXMLStylesContext> mxStylesContext;
    rtl::Reference<SvXMLStylesContext> mxAutoStylesContext;

    // contexts for xShape contents TokenMaps
    std::unique_ptr<SvXMLTokenMap>              mpGroupShapeElemTokenMap;
    std::unique_ptr<SvXMLTokenMap>              mpFrameShapeElemTokenMap;
    std::unique_ptr<SvXMLTokenMap>              mp3DSceneShapeElemTokenMap;
    std::unique_ptr<SvXMLTokenMap>              mp3DObjectAttrTokenMap;
    std::unique_ptr<SvXMLTokenMap>              mp3DPolygonBasedAttrTokenMap;
    std::unique_ptr<SvXMLTokenMap>              mp3DCubeObjectAttrTokenMap;
    std::unique_ptr<SvXMLTokenMap>              mp3DSphereObjectAttrTokenMap;
    std::unique_ptr<SvXMLTokenMap>              mp3DLightAttrTokenMap;

    const OUString       msStartShape;
    const OUString       msEndShape;
    const OUString       msStartGluePointIndex;
    const OUString       msEndGluePointIndex;

    rtl::Reference< XMLTableImport > mxShapeTableImport;

protected:
    SvXMLImport& mrImporter;

public:
    XMLShapeImportHelper( SvXMLImport& rImporter,
        const css::uno::Reference< css::frame::XModel>& rModel,
    SvXMLImportPropertyMapper *pExtMapper=nullptr );

    virtual ~XMLShapeImportHelper() override;

    SvXMLShapeContext* CreateGroupChildContext(
        SvXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList,
        css::uno::Reference< css::drawing::XShapes >& rShapes,
        bool bTemporaryShape = false);

    SvXMLShapeContext* CreateFrameChildContext(
        SvXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList,
        css::uno::Reference< css::drawing::XShapes >& rShapes,
        const css::uno::Reference< css::xml::sax::XAttributeList >& xFrameAttrList);
    static SvXMLImportContext* CreateFrameChildContext(
        SvXMLImportContext *pThisContext, sal_uInt16 nPrefix, const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList );

    SvXMLShapeContext* Create3DSceneChildContext(
        SvXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList,
        css::uno::Reference< css::drawing::XShapes >& rShapes);

    const SvXMLTokenMap& GetGroupShapeElemTokenMap();
    const SvXMLTokenMap& GetFrameShapeElemTokenMap();
    const SvXMLTokenMap& Get3DSceneShapeElemTokenMap();
    const SvXMLTokenMap& Get3DObjectAttrTokenMap();
    const SvXMLTokenMap& Get3DPolygonBasedAttrTokenMap();
    const SvXMLTokenMap& Get3DCubeObjectAttrTokenMap();
    const SvXMLTokenMap& Get3DSphereObjectAttrTokenMap();
    const SvXMLTokenMap& Get3DLightAttrTokenMap();

    // Styles and AutoStyles contexts
    SvXMLStylesContext* GetStylesContext() const { return mxStylesContext.get(); }
    void SetStylesContext(SvXMLStylesContext* pNew);
    SvXMLStylesContext* GetAutoStylesContext() const { return mxAutoStylesContext.get(); }
    void SetAutoStylesContext(SvXMLStylesContext* pNew);

    // get factories and mappers
    SvXMLImportPropertyMapper* GetPropertySetMapper() const { return mpPropertySetMapper.get(); }
    SvXMLImportPropertyMapper* GetPresPagePropsMapper() const { return mpPresPagePropsMapper.get(); }

    // this function is called whenever the implementation classes like to add this new
    // shape to the given XShapes.
    virtual void addShape(
        css::uno::Reference< css::drawing::XShape >& rShape,
        const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList,
        css::uno::Reference< css::drawing::XShapes >& rShapes);

    // this function is called whenever the implementation classes have finished importing
    // a shape to the given XShapes. The shape is already inserted into its XShapes and
    // all properties and styles are set.
    virtual void finishShape(
        css::uno::Reference< css::drawing::XShape >& rShape,
        const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList,
        css::uno::Reference< css::drawing::XShapes >& rShapes);

    // helper functions for z-order sorting
    void pushGroupForSorting( css::uno::Reference< css::drawing::XShapes >& rShapes );
    void popGroupAndSort();

    void shapeWithZIndexAdded( css::uno::Reference< css::drawing::XShape >& rShape,
                               sal_Int32 nZIndex );

    void addShapeConnection( css::uno::Reference< css::drawing::XShape >& rConnectorShape,
                             bool bStart,
                             const OUString& rDestShapeId,
                             sal_Int32 nDestGlueId );

    /** adds a mapping for a glue point identifier from an xml file to the identifier created after inserting
        the new glue point into the core. The saved mappings can be retrieved by getGluePointId() */
    void addGluePointMapping( css::uno::Reference< css::drawing::XShape >& xShape,
                              sal_Int32 nSourceId, sal_Int32 nDestinnationId );

    /** find mapping for given DestinationID. This allows to extract the original draw:id imported with a draw:glue-point */
    sal_Int32 findGluePointMapping(
        const css::uno::Reference< css::drawing::XShape >& xShape,
        sal_Int32 nDestinnationId ) const;

    /** moves all current DestinationId's for rXShape by n */
    void moveGluePointMapping( const css::uno::Reference< css::drawing::XShape >& xShape, const sal_Int32 n );

    /** retrieves a mapping for a glue point identifier from the current xml file to the identifier created after
        inserting the new glue point into the core. The mapping must be initialized first with addGluePointMapping() */
    sal_Int32 getGluePointId( const css::uno::Reference< css::drawing::XShape >& xShape, sal_Int32 nSourceId );

    /** this method must be calling before the first shape is imported for the given page.
        Calls to this method can be nested */
    void startPage( css::uno::Reference< css::drawing::XShapes >& rShapes );

    /** this method must be calling after the last shape is imported for the given page
        Calls to this method can be nested */
    void endPage( css::uno::Reference< css::drawing::XShapes >& rShapes );

    void restoreConnections();

    /** creates a property mapper for external chaining */
    static SvXMLImportPropertyMapper* CreateShapePropMapper(
        const css::uno::Reference< css::frame::XModel>& rModel, SvXMLImport& rImport );

    /** defines if the import should increment the progress bar or not */
    void enableHandleProgressBar();
    bool IsHandleProgressBarEnabled() const;

    /** queries the capability of the current model to create presentation shapes */
    bool IsPresentationShapesSupported();

    XMLSdPropHdlFactory* GetSdPropHdlFactory() const { return mpSdPropHdlFactory.get(); }

    const rtl::Reference< XMLTableImport >&     GetShapeTableImport();
};

#endif // INCLUDED_XMLOFF_SHAPEIMPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
