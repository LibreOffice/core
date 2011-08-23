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
#ifndef _XMLOFF_SHAPEEXPORT_HXX_
#define _XMLOFF_SHAPEEXPORT_HXX_

#include <rtl/ustring.hxx>

#include <rtl/ustrbuf.hxx>

#include <bf_xmloff/uniref.hxx>

#include <com/sun/star/drawing/XShape.hpp>

#include <com/sun/star/drawing/XShapes.hpp>

#include <com/sun/star/awt/Point.hpp>

#include <map>

#include <bf_xmloff/xmlprmap.hxx>

#include <bf_xmloff/xmlexppr.hxx>

#include <bf_xmloff/animexp.hxx>

#include <bf_xmloff/families.hxx>
class Vector2D;
namespace binfilter {
class Matrix3D;

// shape export features are bits used for the nFeature
// parameter of XMLShapeExport::exportShape

#define SEF_EXPORT_X		0x0001
#define SEF_EXPORT_Y		0x0002
#define SEF_EXPORT_POSITION	0x0003

#define SEF_EXPORT_WIDTH	0x0004
#define SEF_EXPORT_HEIGHT	0x0008
#define SEF_EXPORT_SIZE		0x000c

// when you set this flag a chart does NOT export its own data as table element
#define SEF_EXPORT_NO_CHART_DATA	0x0010

// When setting the flag below no ignorableWhiteSpace will be called around
// the drawing object elements
#define SEF_EXPORT_NO_WS	0x0020

#define SEF_DEFAULT			SEF_EXPORT_POSITION|SEF_EXPORT_SIZE

enum XmlShapeType
{
    XmlShapeTypeUnknown,							// not known

    XmlShapeTypeDrawRectangleShape,					// "com.sun.star.drawing.RectangleShape"
    XmlShapeTypeDrawEllipseShape,					// "com.sun.star.drawing.EllipseShape"
    XmlShapeTypeDrawControlShape,					// "com.sun.star.drawing.ControlShape"
    XmlShapeTypeDrawConnectorShape,					// "com.sun.star.drawing.ConnectorShape"
    XmlShapeTypeDrawMeasureShape,					// "com.sun.star.drawing.MeasureShape"
    XmlShapeTypeDrawLineShape,						// "com.sun.star.drawing.LineShape"
    XmlShapeTypeDrawPolyPolygonShape,				// "com.sun.star.drawing.PolyPolygonShape"
    XmlShapeTypeDrawPolyLineShape,					// "com.sun.star.drawing.PolyLineShape"
    XmlShapeTypeDrawOpenBezierShape,				// "com.sun.star.drawing.OpenBezierShape"
    XmlShapeTypeDrawClosedBezierShape,				// "com.sun.star.drawing.ClosedBezierShape"
    XmlShapeTypeDrawGraphicObjectShape,				// "com.sun.star.drawing.GraphicObjectShape"
    XmlShapeTypeDrawGroupShape,						// "com.sun.star.drawing.GroupShape"
    XmlShapeTypeDrawTextShape,						// "com.sun.star.drawing.TextShape"
    XmlShapeTypeDrawOLE2Shape,						// "com.sun.star.drawing.OLE2Shape"
    XmlShapeTypeDrawChartShape,						// embedded com.sun.star.chart
    XmlShapeTypeDrawTableShape,						// embedded com.sun.star.sheet
    XmlShapeTypeDrawPageShape,						// "com.sun.star.drawing.PageShape"
    XmlShapeTypeDrawFrameShape,						// "com.sun.star.drawing.FrameShape"
    XmlShapeTypeDrawCaptionShape,					// "com.sun.star.drawing.CaptionShape"
    XmlShapeTypeDrawAppletShape,					// "com.sun.star.drawing.AppletShape"
    XmlShapeTypeDrawPluginShape,					// "com.sun.star.drawing.PlugginShape"

    XmlShapeTypeDraw3DSceneObject,					// "com.sun.star.drawing.Shape3DSceneObject"
    XmlShapeTypeDraw3DCubeObject,					// "com.sun.star.drawing.Shape3DCubeObject"
    XmlShapeTypeDraw3DSphereObject,					// "com.sun.star.drawing.Shape3DSphereObject"
    XmlShapeTypeDraw3DLatheObject,					// "com.sun.star.drawing.Shape3DLatheObject"
    XmlShapeTypeDraw3DExtrudeObject,				// "com.sun.star.drawing.Shape3DExtrudeObject"

    XmlShapeTypePresTitleTextShape,					// "com.sun.star.presentation.TitleTextShape"
    XmlShapeTypePresOutlinerShape,					// "com.sun.star.presentation.OutlinerShape"
    XmlShapeTypePresSubtitleShape,					// "com.sun.star.presentation.SubtitleShape"
    XmlShapeTypePresGraphicObjectShape,				// "com.sun.star.presentation.GraphicObjectShape"
    XmlShapeTypePresPageShape,						// "com.sun.star.presentation.PageShape"
    XmlShapeTypePresOLE2Shape,						// "com.sun.star.presentation.OLE2Shape"
    XmlShapeTypePresChartShape,						// "com.sun.star.presentation.ChartShape"
    XmlShapeTypePresTableShape,						// "com.sun.star.presentation.TableShape"
    XmlShapeTypePresOrgChartShape,					// "com.sun.star.presentation.OrgChartShape"
    XmlShapeTypePresNotesShape,						// "com.sun.star.presentation.NotesShape"
    XmlShapeTypeHandoutShape,						// "com.sun.star.presentation.HandoutShape"

    XmlShapeTypeNotYetSet
};

struct XShapeCompareHelper
{
  bool operator()( ::com::sun::star::uno::Reference < ::com::sun::star::drawing::XShape > x1,
                  ::com::sun::star::uno::Reference < ::com::sun::star::drawing::XShape > x2 ) const
  {
    return x1.get() < x2.get();
  }
};

struct XShapesCompareHelper
{
  bool operator()( ::com::sun::star::uno::Reference < ::com::sun::star::drawing::XShapes > x1,
                  ::com::sun::star::uno::Reference < ::com::sun::star::drawing::XShapes > x2 ) const
  {
    return x1.get() < x2.get();
  }
};

/** this map store all shapes that had a shape id assigned with the corresponding shape id */
typedef ::std::map< ::com::sun::star::uno::Reference < ::com::sun::star::drawing::XShape >, sal_Int32, XShapeCompareHelper > ShapeIdsMap;

/** caches style and type info after a collectShapeAutostyle for later use in exportShape */
struct ImplXMLShapeExportInfo
{
    ::rtl::OUString	msStyleName;
    ::rtl::OUString	msTextStyleName;
    sal_Int32 mnFamily;
    XmlShapeType meShapeType;

    ImplXMLShapeExportInfo() : meShapeType( XmlShapeTypeNotYetSet ), mnFamily( XML_STYLE_FAMILY_SD_GRAPHICS_ID ) {}
};

/** a vector for shape style and type cache information */
typedef ::std::vector< ImplXMLShapeExportInfo > ImplXMLShapeExportInfoVector;

/** a map to store all cache data for already collected XShapes */
typedef ::std::map< ::com::sun::star::uno::Reference < ::com::sun::star::drawing::XShapes >, ImplXMLShapeExportInfoVector, XShapesCompareHelper > ShapesInfos;

class SvXMLExport;
class SvXMLExportPropertyMapper;

class XMLShapeExport : public UniRefBase
{
private:

    SvXMLExport&								rExport;
    UniReference< XMLPropertyHandlerFactory >	xSdPropHdlFactory;
    UniReference< SvXMLExportPropertyMapper >	xPropertySetMapper;
    UniReference< XMLAnimationsExporter >		mxAnimationsExporter;
    sal_Int32									mnNextUniqueShapeId;
    ShapeIdsMap									maShapeIds;
    ShapesInfos									maShapesInfos;
    ShapesInfos::iterator						maCurrentShapesIter;
    sal_Bool									mbExportLayer;
    ImplXMLShapeExportInfoVector				maShapeInfos;
    ImplXMLShapeExportInfoVector::iterator		maCurrentInfo;
    ::rtl::OUString								msPresentationStylePrefix;

    // #88546# possibility to swich progress bar handling on/off
    sal_Bool									mbHandleProgressBar;

protected:
    SvXMLExport& GetExport() { return rExport; }
    const SvXMLExport& GetExport() const  { return rExport; }
private:

    UniReference< SvXMLExportPropertyMapper > GetPropertySetMapper() const { return xPropertySetMapper;	}

    const ::rtl::OUString							msZIndex;
    const ::rtl::OUString							msEmptyPres;
    const ::rtl::OUString							msModel;
    const ::rtl::OUString							msStartShape;
    const ::rtl::OUString							msEndShape;
    const ::rtl::OUString							msOnClick;
    const ::rtl::OUString							msEventType;
    const ::rtl::OUString							msPresentation;
    const ::rtl::OUString							msMacroName;
    const ::rtl::OUString							msLibrary;
    const ::rtl::OUString							msClickAction;
    const ::rtl::OUString							msBookmark;
    const ::rtl::OUString							msEffect;
    const ::rtl::OUString							msPlayFull;
    const ::rtl::OUString							msVerb;
    const ::rtl::OUString							msSoundURL;
    const ::rtl::OUString							msSpeed;
    const ::rtl::OUString							msStarBasic;

    ::rtl::OUStringBuffer	msBuffer;

    void ImpCalcShapeType(const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape, XmlShapeType& eShapeType);

    void ImpExportNewTrans(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xPropSet, sal_Int32 nFeatures, ::com::sun::star::awt::Point* pRefPoint);
    void ImpExportNewTrans_GetMatrix3D(Matrix3D& rMat, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xPropSet);
    void ImpExportNewTrans_DecomposeAndRefPoint(const Matrix3D& rMat, Vector2D& rTRScale, double& fTRShear, double& fTRRotate, Vector2D& rTRTranslate, ::com::sun::star::awt::Point* pRefPoint);
    void ImpExportNewTrans_FeaturesAndWrite(Vector2D& rTRScale, double fTRShear, double fTRRotate, Vector2D& rTRTranslate, const sal_Int32 nFeatures);
    sal_Bool ImpExportPresentationAttributes( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xPropSet, const ::rtl::OUString& rClass );
    void ImpExportText( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape );
    void ImpExportEvents( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape );
    void ImpExportGluePoints( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape );

    // single shape exporters
    void ImpExportGroupShape( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT, ::com::sun::star::awt::Point* pRefPoint = NULL );
    void ImpExport3DSceneShape( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT, ::com::sun::star::awt::Point* pRefPoint = NULL );
    void ImpExportRectangleShape( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT, ::com::sun::star::awt::Point* pRefPoint = NULL );
    void ImpExportLineShape(const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT, ::com::sun::star::awt::Point* pRefPoint = NULL );
    void ImpExportEllipseShape(const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT, ::com::sun::star::awt::Point* pRefPoint = NULL );
    void ImpExportPolygonShape(const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT, ::com::sun::star::awt::Point* pRefPoint = NULL );
    void ImpExportTextBoxShape(const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT, ::com::sun::star::awt::Point* pRefPoint = NULL );
    void ImpExportGraphicObjectShape(const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT, ::com::sun::star::awt::Point* pRefPoint = NULL );
    void ImpExportChartShape(const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT, ::com::sun::star::awt::Point* pRefPoint = NULL );
    void ImpExportSpreadsheetShape(const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT,	::com::sun::star::awt::Point* pRefPoint = NULL );
    void ImpExportControlShape(const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT,	::com::sun::star::awt::Point* pRefPoint = NULL );
    void ImpExportConnectorShape(const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT, ::com::sun::star::awt::Point* pRefPoint = NULL );
    void ImpExportMeasureShape(const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT,	::com::sun::star::awt::Point* pRefPoint = NULL );
    void ImpExportOLE2Shape(const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT, ::com::sun::star::awt::Point* pRefPoint = NULL );
    void ImpExportPageShape(const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT, ::com::sun::star::awt::Point* pRefPoint = NULL );
    void ImpExportCaptionShape(const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT, ::com::sun::star::awt::Point* pRefPoint = NULL );
    void ImpExport3DShape(const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT, ::com::sun::star::awt::Point* pRefPoint = NULL );
    void ImpExportFrameShape( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT, ::com::sun::star::awt::Point* pRefPoint = NULL );
    void ImpExportPluginShape( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT, ::com::sun::star::awt::Point* pRefPoint = NULL );
    void ImpExportAppletShape( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures = SEF_DEFAULT, ::com::sun::star::awt::Point* pRefPoint = NULL );

public:
    XMLShapeExport(SvXMLExport& rExp, SvXMLExportPropertyMapper *pExtMapper=0 );
    virtual ~XMLShapeExport();

    // This method collects all automatic styles for the given XShape
    void collectShapeAutoStyles(
        const ::com::sun::star::uno::Reference < ::com::sun::star::drawing::XShape >& xShape);

    // This method exports the given XShape
    void exportShape(
        const ::com::sun::star::uno::Reference < ::com::sun::star::drawing::XShape >& xShape,
        sal_Int32 nFeatures = SEF_DEFAULT,
        ::com::sun::star::awt::Point* pRefPoint = NULL
        );

    // This method collects all automatic styles for the shapes inside the given XShapes collection
    void collectShapesAutoStyles(
        const ::com::sun::star::uno::Reference < ::com::sun::star::drawing::XShapes >& xShapes);

    // This method exports all XShape inside the given XShapes collection
    void exportShapes(
        const ::com::sun::star::uno::Reference < ::com::sun::star::drawing::XShapes >& xShapes,
        sal_Int32 nFeatures = SEF_DEFAULT,
        ::com::sun::star::awt::Point* pRefPoint = NULL
        );

    /** initializes some internal structures for fast access to the given XShapes collection

        <p>This method has to be called before you use exportShape or collectShapeAutoStyles.
        It is automaticly called if you use collectShapesAutoStyles and exportShapes.

        @see collectShapeAutoStyles
        @see exportShape
        @see collectShapesAutoStyles
        @see exportShapes
    */
    void seekShapes(
        const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xShapes ) throw();

    void exportAutoStyles();

    /** creates a unique id for this shape, this id is saved and exported with this shape later
        with the exportShape method. Its ok to call this twice with the same shape */
    void createShapeId( const ::com::sun::star::uno::Reference < ::com::sun::star::drawing::XShape >& xShape );

    /** returns the unique id for this shape. It returns -1 if the was no createShapeId call
        for this shape yet. */
    sal_Int32 getShapeId( const ::com::sun::star::uno::Reference < ::com::sun::star::drawing::XShape >& xShape );

    /** sets a new reference to an XMLAnimationExporter.
        If this is a non NULL reference, the animation information from all shapes given to exportShape()
        from now on are collected.
    */
    void setAnimationsExporter( UniReference< XMLAnimationsExporter > xAnimExport ) { mxAnimationsExporter = xAnimExport; }

    /** returns the last set XMLAnimationExport */
    UniReference< XMLAnimationsExporter > getAnimationsExporter() const { return mxAnimationsExporter; }

    /// returns the export property mapper for external chaining
    static SvXMLExportPropertyMapper* CreateShapePropMapper( SvXMLExport& rExport );

    void enableLayerExport( sal_Bool bEnable = sal_True ) { mbExportLayer = bEnable; }
    sal_Bool IsLayerExportEnabled() const { return mbExportLayer; }

    // #88546#
    /** defines if the export should increment the progress bar or not */
    void enableHandleProgressBar( sal_Bool bEnable = sal_True ) { mbHandleProgressBar = bEnable; }
    sal_Bool IsHandleProgressBarEnabled() const { return mbHandleProgressBar; }

    void setPresentationStylePrefix( const ::rtl::OUString& rPrefix ) { msPresentationStylePrefix = rPrefix; }

    /** helper for chart that adds all attributes of a 3d scene element to the export */
    void export3DSceneAttributes( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xPropSet );

    /** helper for chart that exports all lamps from the propertyset */
    void export3DLamps( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xPropSet );

    /** helper to export the style for graphic defaults */
    void ExportGraphicDefaults();

    /** is called before a shape element for the given XShape is exported */
    virtual void onExport( const ::com::sun::star::uno::Reference < ::com::sun::star::drawing::XShape >& xShape );
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
