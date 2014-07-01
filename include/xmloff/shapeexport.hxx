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
#ifndef INCLUDED_XMLOFF_SHAPEEXPORT_HXX
#define INCLUDED_XMLOFF_SHAPEEXPORT_HXX

#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <sal/types.h>

#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <salhelper/simplereferenceobject.hxx>

#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <map>
#include <xmloff/xmlprmap.hxx>
#include <xmloff/xmlexppr.hxx>
#include <xmloff/animexp.hxx>
#include <xmloff/families.hxx>

#include <xmloff/table/XMLTableExport.hxx>

// shape export features are bits used for the nFeature
// parameter of XMLShapeExport::exportShape

#define SEF_EXPORT_X        0x0001
#define SEF_EXPORT_Y        0x0002
#define SEF_EXPORT_POSITION 0x0003

#define SEF_EXPORT_WIDTH    0x0004
#define SEF_EXPORT_HEIGHT   0x0008
#define SEF_EXPORT_SIZE     0x000c

// when you set this flag a chart does NOT export its own data as table element
#define SEF_EXPORT_NO_CHART_DATA    0x0010

// When setting the flag below no ignorableWhiteSpace will be called around
// the drawing object elements
#define SEF_EXPORT_NO_WS    0x0020

// When setting the flag below a callout shape is exported as office:annotation
#define SEF_EXPORT_ANNOTATION 0x0040

#define SEF_DEFAULT         SEF_EXPORT_POSITION|SEF_EXPORT_SIZE

enum XmlShapeType
{
    XmlShapeTypeUnknown,                            // not known

    XmlShapeTypeDrawRectangleShape,                 // "com.sun.star.drawing.RectangleShape"
    XmlShapeTypeDrawEllipseShape,                   // "com.sun.star.drawing.EllipseShape"
    XmlShapeTypeDrawControlShape,                   // "com.sun.star.drawing.ControlShape"
    XmlShapeTypeDrawConnectorShape,                 // "com.sun.star.drawing.ConnectorShape"
    XmlShapeTypeDrawMeasureShape,                   // "com.sun.star.drawing.MeasureShape"
    XmlShapeTypeDrawLineShape,                      // "com.sun.star.drawing.LineShape"
    XmlShapeTypeDrawPolyPolygonShape,               // "com.sun.star.drawing.PolyPolygonShape"
    XmlShapeTypeDrawPolyLineShape,                  // "com.sun.star.drawing.PolyLineShape"
    XmlShapeTypeDrawOpenBezierShape,                // "com.sun.star.drawing.OpenBezierShape"
    XmlShapeTypeDrawClosedBezierShape,              // "com.sun.star.drawing.ClosedBezierShape"
    XmlShapeTypeDrawGraphicObjectShape,             // "com.sun.star.drawing.GraphicObjectShape"
    XmlShapeTypeDrawGroupShape,                     // "com.sun.star.drawing.GroupShape"
    XmlShapeTypeDrawTextShape,                      // "com.sun.star.drawing.TextShape"
    XmlShapeTypeDrawOLE2Shape,                      // "com.sun.star.drawing.OLE2Shape"
    XmlShapeTypeDrawChartShape,                     // embedded com.sun.star.chart
    XmlShapeTypeDrawSheetShape,                     // embedded com.sun.star.sheet
    XmlShapeTypeDrawPageShape,                      // "com.sun.star.drawing.PageShape"
    XmlShapeTypeDrawFrameShape,                     // "com.sun.star.drawing.FrameShape"
    XmlShapeTypeDrawCaptionShape,                   // "com.sun.star.drawing.CaptionShape"
    XmlShapeTypeDrawAppletShape,                    // "com.sun.star.drawing.AppletShape"
    XmlShapeTypeDrawPluginShape,                    // "com.sun.star.drawing.PlugginShape"

    XmlShapeTypeDraw3DSceneObject,                  // "com.sun.star.drawing.Shape3DSceneObject"
    XmlShapeTypeDraw3DCubeObject,                   // "com.sun.star.drawing.Shape3DCubeObject"
    XmlShapeTypeDraw3DSphereObject,                 // "com.sun.star.drawing.Shape3DSphereObject"
    XmlShapeTypeDraw3DLatheObject,                  // "com.sun.star.drawing.Shape3DLatheObject"
    XmlShapeTypeDraw3DExtrudeObject,                // "com.sun.star.drawing.Shape3DExtrudeObject"

    XmlShapeTypePresTitleTextShape,                 // "com.sun.star.presentation.TitleTextShape"
    XmlShapeTypePresOutlinerShape,                  // "com.sun.star.presentation.OutlinerShape"
    XmlShapeTypePresSubtitleShape,                  // "com.sun.star.presentation.SubtitleShape"
    XmlShapeTypePresGraphicObjectShape,             // "com.sun.star.presentation.GraphicObjectShape"
    XmlShapeTypePresPageShape,                      // "com.sun.star.presentation.PageShape"
    XmlShapeTypePresOLE2Shape,                      // "com.sun.star.presentation.OLE2Shape"
    XmlShapeTypePresChartShape,                     // "com.sun.star.presentation.ChartShape"
    XmlShapeTypePresSheetShape,                     // "com.sun.star.presentation.CalcShape"
    XmlShapeTypePresTableShape,                     // "com.sun.star.presentation.TableShape"
    XmlShapeTypePresOrgChartShape,                  // "com.sun.star.presentation.OrgChartShape"
    XmlShapeTypePresNotesShape,                     // "com.sun.star.presentation.NotesShape"
    XmlShapeTypeHandoutShape,                       // "com.sun.star.presentation.HandoutShape"

    XmlShapeTypePresHeaderShape,                    // "com.sun.star.presentation.HeaderShape"
    XmlShapeTypePresFooterShape,                    // "com.sun.star.presentation.FooterShape"
    XmlShapeTypePresSlideNumberShape,               // "com.sun.star.presentation.SlideNumberShape"
    XmlShapeTypePresDateTimeShape,                  // "com.sun.star.presentation.DateTimeShape"

    XmlShapeTypeDrawCustomShape,                    // "com.sun.star.drawing.CustomShape"
    XmlShapeTypeDrawMediaShape,                     // "com.sun.star.drawing.MediaShape"
    XmlShapeTypePresMediaShape,                     // "com.sun.star.presentation.MediaShape"

    XmlShapeTypeDrawTableShape,                     // "com.sun.star.drawing.TableShape"

    XmlShapeTypeNotYetSet
};

/** caches style and type info after a collectShapeAutostyle for later use in exportShape */
struct ImplXMLShapeExportInfo
{
    OUString   msStyleName;
    OUString   msTextStyleName;
    sal_Int32       mnFamily;
    XmlShapeType    meShapeType;

    com::sun::star::uno::Reference< com::sun::star::drawing::XShape > xCustomShapeReplacement;

    ImplXMLShapeExportInfo() : mnFamily( XML_STYLE_FAMILY_SD_GRAPHICS_ID ), meShapeType( XmlShapeTypeNotYetSet ) {}
};

/** a vector for shape style and type cache information */
typedef std::vector< ImplXMLShapeExportInfo > ImplXMLShapeExportInfoVector;

/** a map to store all cache data for already collected XShapes */
typedef std::map< com::sun::star::uno::Reference < com::sun::star::drawing::XShapes >, ImplXMLShapeExportInfoVector > ShapesInfos;


// predeclarations

class SvXMLExport;
class SvXMLExportPropertyMapper;

namespace basegfx
{
    class B2DTuple;
    class B2DHomMatrix;
} // end of namespace basegfx

class XMLOFF_DLLPUBLIC XMLShapeExport : public salhelper::SimpleReferenceObject
{
private:

    SvXMLExport&                                mrExport;
    rtl::Reference< XMLPropertyHandlerFactory >   mxSdPropHdlFactory;
    rtl::Reference< SvXMLExportPropertyMapper >   mxPropertySetMapper;
    rtl::Reference< XMLAnimationsExporter >       mxAnimationsExporter;
    ShapesInfos                                 maShapesInfos;
    ShapesInfos::iterator                       maCurrentShapesIter;
    bool                                        mbExportLayer;
    ImplXMLShapeExportInfoVector                maShapeInfos;
    ImplXMLShapeExportInfoVector::iterator      maCurrentInfo;
    OUString                                    msPresentationStylePrefix;

    // #88546# possibility to swich progress bar handling on/off
    bool                                        mbHandleProgressBar;

    rtl::Reference< XMLTableExport >            mxShapeTableExport;

protected:
    SvXMLExport& GetExport() { return mrExport; }
    const SvXMLExport& GetExport() const  { return mrExport; }
private:

    SAL_DLLPRIVATE rtl::Reference< SvXMLExportPropertyMapper > GetPropertySetMapper() const { return mxPropertySetMapper; }

    const OUString                         msZIndex;
    const OUString                         msPrintable;
    const OUString                         msVisible;

    const OUString                         msEmptyPres;
    const OUString                         msModel;
    const OUString                         msStartShape;
    const OUString                         msEndShape;
    const OUString                         msOnClick;
    const OUString                         msEventType;
    const OUString                         msPresentation;
    const OUString                         msMacroName;
    const OUString                         msScript;
    const OUString                         msLibrary;
    const OUString                         msClickAction;
    const OUString                         msBookmark;
    const OUString                         msEffect;
    const OUString                         msPlayFull;
    const OUString                         msVerb;
    const OUString                         msSoundURL;
    const OUString                         msSpeed;
    const OUString                         msStarBasic;

    OUStringBuffer msBuffer;

    SAL_DLLPRIVATE void ImpCalcShapeType(const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape, XmlShapeType& eShapeType);

    SAL_DLLPRIVATE void ImpExportNewTrans(const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet >& xPropSet, sal_Int32 nFeatures, com::sun::star::awt::Point* pRefPoint);
    SAL_DLLPRIVATE void ImpExportNewTrans_GetB2DHomMatrix(::basegfx::B2DHomMatrix& rMatrix, const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet >& xPropSet);
    SAL_DLLPRIVATE void ImpExportNewTrans_DecomposeAndRefPoint(const ::basegfx::B2DHomMatrix& rMat, ::basegfx::B2DTuple& rTRScale, double& fTRShear, double& fTRRotate, ::basegfx::B2DTuple& rTRTranslate, com::sun::star::awt::Point* pRefPoint);
    SAL_DLLPRIVATE void ImpExportNewTrans_FeaturesAndWrite(::basegfx::B2DTuple& rTRScale, double fTRShear, double fTRRotate, ::basegfx::B2DTuple& rTRTranslate, const sal_Int32 nFeatures);
    SAL_DLLPRIVATE bool ImpExportPresentationAttributes( const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet >& xPropSet, const OUString& rClass );
    SAL_DLLPRIVATE void ImpExportText( const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape );
    SAL_DLLPRIVATE void ImpExportEvents( const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape );
    SAL_DLLPRIVATE void ImpExportDescription( const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape ); // #i68101#
    SAL_DLLPRIVATE void ImpExportGluePoints( const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape );

    // single shape exporters
    SAL_DLLPRIVATE void ImpExportGroupShape( const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT, com::sun::star::awt::Point* pRefPoint = NULL );
    SAL_DLLPRIVATE void ImpExport3DSceneShape( const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT, com::sun::star::awt::Point* pRefPoint = NULL );
    SAL_DLLPRIVATE void ImpExportRectangleShape( const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT, com::sun::star::awt::Point* pRefPoint = NULL );
    SAL_DLLPRIVATE void ImpExportLineShape(const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT, com::sun::star::awt::Point* pRefPoint = NULL );
    SAL_DLLPRIVATE void ImpExportEllipseShape(const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT, com::sun::star::awt::Point* pRefPoint = NULL );
    SAL_DLLPRIVATE void ImpExportPolygonShape(const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT, com::sun::star::awt::Point* pRefPoint = NULL );
    SAL_DLLPRIVATE void ImpExportTextBoxShape(const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT, com::sun::star::awt::Point* pRefPoint = NULL );
    SAL_DLLPRIVATE void ImpExportGraphicObjectShape(const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT, com::sun::star::awt::Point* pRefPoint = NULL );
    SAL_DLLPRIVATE void ImpExportChartShape(const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT, com::sun::star::awt::Point* pRefPoint = NULL, SvXMLAttributeList* pAttrList = NULL );
    SAL_DLLPRIVATE void ImpExportControlShape(const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT,  com::sun::star::awt::Point* pRefPoint = NULL );
    SAL_DLLPRIVATE void ImpExportConnectorShape(const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT, com::sun::star::awt::Point* pRefPoint = NULL );
    SAL_DLLPRIVATE void ImpExportMeasureShape(const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT,  com::sun::star::awt::Point* pRefPoint = NULL );
    SAL_DLLPRIVATE void ImpExportOLE2Shape(const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT, com::sun::star::awt::Point* pRefPoint = NULL, SvXMLAttributeList* pAttrList = NULL );
    SAL_DLLPRIVATE void ImpExportPageShape(const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT, com::sun::star::awt::Point* pRefPoint = NULL );
    SAL_DLLPRIVATE void ImpExportCaptionShape(const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT, com::sun::star::awt::Point* pRefPoint = NULL );
    SAL_DLLPRIVATE void ImpExport3DShape(const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT, com::sun::star::awt::Point* pRefPoint = NULL );
    SAL_DLLPRIVATE void ImpExportFrameShape( const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT, com::sun::star::awt::Point* pRefPoint = NULL );
    SAL_DLLPRIVATE void ImpExportPluginShape( const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT, com::sun::star::awt::Point* pRefPoint = NULL );
    SAL_DLLPRIVATE void ImpExportAppletShape( const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT, com::sun::star::awt::Point* pRefPoint = NULL );
    SAL_DLLPRIVATE void ImpExportCustomShape( const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT, com::sun::star::awt::Point* pRefPoint = NULL );
    SAL_DLLPRIVATE void ImpExportMediaShape( const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT, com::sun::star::awt::Point* pRefPoint = NULL );
    SAL_DLLPRIVATE void ImpExportTableShape(const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT,    com::sun::star::awt::Point* pRefPoint = NULL );
public:
    XMLShapeExport(SvXMLExport& rExp, SvXMLExportPropertyMapper *pExtMapper=0 );
    virtual ~XMLShapeExport();

    // This method collects all automatic styles for the given XShape
    void collectShapeAutoStyles(
        const com::sun::star::uno::Reference < com::sun::star::drawing::XShape >& xShape);

    // This method exports the given XShape
    void exportShape(
        const com::sun::star::uno::Reference < com::sun::star::drawing::XShape >& xShape,
        sal_Int32 nFeatures = SEF_DEFAULT,
        com::sun::star::awt::Point* pRefPoint = NULL,
        SvXMLAttributeList* pAttrList = NULL
        );

    // This method collects all automatic styles for the shapes inside the given XShapes collection
    void collectShapesAutoStyles(
        const com::sun::star::uno::Reference < com::sun::star::drawing::XShapes >& xShapes);

    // This method exports all XShape inside the given XShapes collection
    void exportShapes(
        const com::sun::star::uno::Reference < com::sun::star::drawing::XShapes >& xShapes,
        sal_Int32 nFeatures = SEF_DEFAULT,
        com::sun::star::awt::Point* pRefPoint = NULL
        );

    /** initializes some internal structures for fast access to the given XShapes collection

        <p>This method has to be called before you use exportShape or collectShapeAutoStyles.
        It is automatically called if you use collectShapesAutoStyles and exportShapes.

        @see collectShapeAutoStyles
        @see exportShape
        @see collectShapesAutoStyles
        @see exportShapes
    */
    void seekShapes(
        const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xShapes ) throw();

    void exportAutoStyles();

    /** sets a new reference to an XMLAnimationExporter.
        If this is a non NULL reference, the animation information from all shapes given to exportShape()
        from now on are collected.
    */
    void setAnimationsExporter( rtl::Reference< XMLAnimationsExporter > xAnimExport ) { mxAnimationsExporter = xAnimExport; }

    /** returns the last set XMLAnimationExport */
    rtl::Reference< XMLAnimationsExporter > getAnimationsExporter() const { return mxAnimationsExporter; }

    /// returns the export property mapper for external chaining
    static SvXMLExportPropertyMapper* CreateShapePropMapper( SvXMLExport& rExport );

    void enableLayerExport( bool bEnable = true ) { mbExportLayer = bEnable; }
    bool IsLayerExportEnabled() const { return mbExportLayer; }

    // #88546#
    /** defines if the export should increment the progress bar or not */
    void enableHandleProgressBar( bool bEnable = true ) { mbHandleProgressBar = bEnable; }
    bool IsHandleProgressBarEnabled() const { return mbHandleProgressBar; }

    void setPresentationStylePrefix( const OUString& rPrefix ) { msPresentationStylePrefix = rPrefix; }

    /** helper for chart that adds all attributes of a 3d scene element to the export */
    void export3DSceneAttributes( const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet >& xPropSet );

    /** helper for chart that exports all lamps from the propertyset */
    void export3DLamps( const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet >& xPropSet );

    /** sj: replacing CustomShapes with standard objects that are also supported in OpenOffice.org format */
    com::sun::star::uno::Reference < com::sun::star::drawing::XShape > checkForCustomShapeReplacement(
        const com::sun::star::uno::Reference < com::sun::star::drawing::XShape >& );

    /** helper to export the style for graphic defaults */
    void ExportGraphicDefaults();

    /** is called before a shape element for the given XShape is exported */
    virtual void onExport( const com::sun::star::uno::Reference < com::sun::star::drawing::XShape >& xShape );

    const rtl::Reference< XMLTableExport >&     GetShapeTableExport();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
