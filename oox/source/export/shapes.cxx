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

#include <unotools/mediadescriptor.hxx>
#include <filter/msfilter/util.hxx>
#include "oox/core/xmlfilterbase.hxx"
#include "oox/export/shapes.hxx"
#include "oox/export/utils.hxx"
#include <oox/token/tokens.hxx>

#include <cstdio>
#include <com/sun/star/awt/CharSet.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/drawing/ConnectorType.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/text/XSimpleText.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/table/XTable.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/XMergeableCell.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <tools/stream.hxx>
#include <vcl/cvtgrf.hxx>
#include <unotools/fontcvt.hxx>
#include <vcl/graph.hxx>
#include <vcl/outdev.hxx>
#include <svtools/grfmgr.hxx>
#include <rtl/strbuf.hxx>
#include <sfx2/app.hxx>
#include <svl/languageoptions.hxx>
#include <filter/msfilter/escherex.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdoole2.hxx>
#include <editeng/svxenum.hxx>
#include <svx/unoapi.hxx>
#include <oox/export/chartexport.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::table;
using ::com::sun::star::beans::PropertyState;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XPropertyState;
using ::com::sun::star::container::XEnumeration;
using ::com::sun::star::container::XEnumerationAccess;
using ::com::sun::star::container::XIndexAccess;
using ::com::sun::star::document::XExporter;
using ::com::sun::star::document::XFilter;
using ::com::sun::star::drawing::FillStyle;
using ::com::sun::star::graphic::XGraphic;
using ::com::sun::star::io::XOutputStream;
using ::com::sun::star::lang::XComponent;
using ::com::sun::star::text::XSimpleText;
using ::com::sun::star::text::XText;
using ::com::sun::star::text::XTextContent;
using ::com::sun::star::text::XTextField;
using ::com::sun::star::text::XTextRange;
using ::oox::core::XmlFilterBase;
using ::com::sun::star::chart2::XChartDocument;
using ::com::sun::star::frame::XModel;
using ::com::sun::star::sheet::XSpreadsheetDocument;
using ::sax_fastparser::FSHelperPtr;

#define IDS(x) OString(OStringLiteral(#x " ") + OString::number( mnShapeIdMax++ )).getStr()

namespace oox { namespace drawingml {

#define GETA(propName) \
    GetProperty( rXPropSet, OUString(#propName))

#define GETAD(propName) \
    ( GetPropertyAndState( rXPropSet, rXPropState, OUString(#propName), eState ) && eState == beans::PropertyState_DIRECT_VALUE )

#define GET(variable, propName) \
    if ( GETA(propName) ) \
        mAny >>= variable;

// not thread safe
int ShapeExport::mnSpreadsheetCounter = 1;

ShapeExport::ShapeExport( sal_Int32 nXmlNamespace, FSHelperPtr pFS, ShapeHashMap* pShapeMap, XmlFilterBase* pFB, DocumentType eDocumentType, DMLTextExport* pTextExport )
    : DrawingML( pFS, pFB, eDocumentType, pTextExport )
    , mnShapeIdMax( 1 )
    , mnPictureIdMax( 1 )
    , mnXmlNamespace( nXmlNamespace )
    , maFraction( 1, 576 )
    , maMapModeSrc( MAP_100TH_MM )
    , maMapModeDest( MAP_INCH, Point(), maFraction, maFraction )
    , mpShapeMap( pShapeMap ? pShapeMap : &maShapeMap )
{
}

awt::Size ShapeExport::MapSize( const awt::Size& rSize ) const
{
    Size aRetSize( OutputDevice::LogicToLogic( Size( rSize.Width, rSize.Height ), maMapModeSrc, maMapModeDest ) );

    if ( !aRetSize.Width() )
        aRetSize.Width()++;
    if ( !aRetSize.Height() )
        aRetSize.Height()++;
    return awt::Size( aRetSize.Width(), aRetSize.Height() );
}

bool ShapeExport::NonEmptyText( Reference< XInterface > xIface )
{
    Reference< XPropertySet > xPropSet( xIface, UNO_QUERY );

    if( xPropSet.is() )
    {
        Reference< XPropertySetInfo > xPropSetInfo = xPropSet->getPropertySetInfo();
        if ( xPropSetInfo.is() )
        {
            if ( xPropSetInfo->hasPropertyByName( "IsEmptyPresentationObject" ) )
            {
                sal_Bool bIsEmptyPresObj = sal_False;
                if ( xPropSet->getPropertyValue( "IsEmptyPresentationObject" ) >>= bIsEmptyPresObj )
                {
                    DBG(fprintf(stderr, "empty presentation object %d, props:\n", bIsEmptyPresObj));
                    if( bIsEmptyPresObj )
                       return true;
                }
            }

            if ( xPropSetInfo->hasPropertyByName( "IsPresentationObject" ) )
            {
                sal_Bool bIsPresObj = sal_False;
                if ( xPropSet->getPropertyValue( "IsPresentationObject" ) >>= bIsPresObj )
                {
                    DBG(fprintf(stderr, "presentation object %d, props:\n", bIsPresObj));
                    if( bIsPresObj )
                       return true;
                }
            }
        }
    }

    Reference< XSimpleText > xText( xIface, UNO_QUERY );

    if( xText.is() )
        return xText->getString().getLength();

    return false;
}

ShapeExport& ShapeExport::WriteBezierShape( Reference< XShape > xShape, bool bClosed )
{
    DBG(fprintf(stderr, "write open bezier shape\n"));

    FSHelperPtr pFS = GetFS();
    pFS->startElementNS( mnXmlNamespace, (GetDocumentType() != DOCUMENT_DOCX ? XML_sp : XML_wsp), FSEND );

    PolyPolygon aPolyPolygon = EscherPropertyContainer::GetPolyPolygon( xShape );
    Rectangle aRect( aPolyPolygon.GetBoundRect() );

#if OSL_DEBUG_LEVEL > 0
    awt::Size size = MapSize( awt::Size( aRect.GetWidth(), aRect.GetHeight() ) );
    DBG(fprintf(stderr, "poly count %d\nsize: %d x %d", aPolyPolygon.Count(), int( size.Width ), int( size.Height )));
#endif

    // non visual shape properties
    if (GetDocumentType() != DOCUMENT_DOCX)
    {
        pFS->startElementNS( mnXmlNamespace, XML_nvSpPr, FSEND );
        pFS->singleElementNS( mnXmlNamespace, XML_cNvPr,
                              XML_id, I32S( GetNewShapeID( xShape ) ),
                              XML_name, IDS( Freeform ),
                              FSEND );
    }
    pFS->singleElementNS( mnXmlNamespace, XML_cNvSpPr, FSEND );
    if (GetDocumentType() != DOCUMENT_DOCX)
    {
        WriteNonVisualProperties( xShape );
        pFS->endElementNS( mnXmlNamespace, XML_nvSpPr );
    }

    // visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_spPr, FSEND );
    WriteTransformation( aRect, XML_a );
    WritePolyPolygon( aPolyPolygon );
    Reference< XPropertySet > xProps( xShape, UNO_QUERY );
    if( xProps.is() ) {
        if( bClosed )
            WriteFill( xProps );
        WriteOutline( xProps );
    }

    pFS->endElementNS( mnXmlNamespace, XML_spPr );

    // write text
    WriteTextBox( xShape, mnXmlNamespace );

    pFS->endElementNS( mnXmlNamespace, (GetDocumentType() != DOCUMENT_DOCX ? XML_sp : XML_wsp) );

    return *this;
}

ShapeExport& ShapeExport::WriteClosedBezierShape( Reference< XShape > xShape )
{
    return WriteBezierShape( xShape, true );
}

ShapeExport& ShapeExport::WriteOpenBezierShape( Reference< XShape > xShape )
{
    return WriteBezierShape( xShape, false );
}

ShapeExport& ShapeExport::WriteGroupShape(uno::Reference<drawing::XShape> xShape)
{
    FSHelperPtr pFS = GetFS();
    bool bToplevel = !m_xParent.is();
    if (!bToplevel)
        mnXmlNamespace = XML_wpg;
    pFS->startElementNS(mnXmlNamespace, (bToplevel ? XML_wgp : XML_grpSp), FSEND);

    // non visual properties
    pFS->singleElementNS(mnXmlNamespace, XML_cNvGrpSpPr, FSEND);

    // visual properties
    pFS->startElementNS(mnXmlNamespace, XML_grpSpPr, FSEND);
    WriteShapeTransformation(xShape, XML_a);
    pFS->endElementNS(mnXmlNamespace, XML_grpSpPr);

    uno::Reference<drawing::XShapes> xGroupShape(xShape, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xParent = m_xParent;
    m_xParent = xShape;
    for (sal_Int32 i = 0; i < xGroupShape->getCount(); ++i)
    {
        uno::Reference<drawing::XShape> xChild(xGroupShape->getByIndex(i), uno::UNO_QUERY_THROW);
        sal_Int32 nSavedNamespace = mnXmlNamespace;

        uno::Reference<lang::XServiceInfo> xServiceInfo(xChild, uno::UNO_QUERY_THROW);
        if (xServiceInfo->supportsService("com.sun.star.drawing.GraphicObjectShape"))
            mnXmlNamespace = XML_pic;
        else
            mnXmlNamespace = XML_wps;
        WriteShape(xChild);

        mnXmlNamespace = nSavedNamespace;
    }
    m_xParent = xParent;

    pFS->endElementNS(mnXmlNamespace, (bToplevel ? XML_wgp : XML_grpSp));
    return *this;
}

ShapeExport& ShapeExport::WriteCustomShape( Reference< XShape > xShape )
{
    DBG(fprintf(stderr, "write custom shape\n"));

    Reference< XPropertySet > rXPropSet( xShape, UNO_QUERY );
    bool bPredefinedHandlesUsed = true;
    OUString sShapeType;
    sal_uInt32 nMirrorFlags = 0;
    MSO_SPT eShapeType = EscherPropertyContainer::GetCustomShapeType( xShape, nMirrorFlags, sShapeType );
    SdrObjCustomShape* pShape = (SdrObjCustomShape*) GetSdrObjectFromXShape( xShape );
    bool bIsDefaultObject = EscherPropertyContainer::IsDefaultObject( pShape, eShapeType );
    const char* sPresetShape = msfilter::util::GetOOXMLPresetGeometry( USS( sShapeType ) );
    DBG(fprintf(stderr, "custom shape type: %s ==> %s\n", USS( sShapeType ), sPresetShape));
    Sequence< PropertyValue > aGeometrySeq;
    sal_Int32 nAdjustmentValuesIndex = -1;

    bool bFlipH = false;
    bool bFlipV = false;

    if( GETA( CustomShapeGeometry ) ) {
        DBG(fprintf(stderr, "got custom shape geometry\n"));
        if( mAny >>= aGeometrySeq ) {

            DBG(fprintf(stderr, "got custom shape geometry sequence\n"));
            for( int i = 0; i < aGeometrySeq.getLength(); i++ ) {
                const PropertyValue& rProp = aGeometrySeq[ i ];
                DBG(fprintf(stderr, "geometry property: %s\n", USS( rProp.Name )));

                if ( rProp.Name == "MirroredX" )
                    rProp.Value >>= bFlipH;

                if ( rProp.Name == "MirroredY" )
                    rProp.Value >>= bFlipV;
                if ( rProp.Name == "AdjustmentValues" )
                    nAdjustmentValuesIndex = i;
                else if ( rProp.Name == "Handles" ) {
                    if( !bIsDefaultObject )
                        bPredefinedHandlesUsed = false;
                    // TODO: update nAdjustmentsWhichNeedsToBeConverted here
                }
            }
        }
    }

    FSHelperPtr pFS = GetFS();
    pFS->startElementNS( mnXmlNamespace, (GetDocumentType() != DOCUMENT_DOCX ? XML_sp : XML_wsp), FSEND );

    // non visual shape properties
    if (GetDocumentType() != DOCUMENT_DOCX)
    {
        pFS->startElementNS( mnXmlNamespace, XML_nvSpPr, FSEND );
        pFS->singleElementNS( mnXmlNamespace, XML_cNvPr,
                XML_id, I32S( GetNewShapeID( xShape ) ),
                XML_name, IDS( CustomShape ),
                FSEND );
        pFS->singleElementNS( mnXmlNamespace, XML_cNvSpPr, FSEND );
        WriteNonVisualProperties( xShape );
        pFS->endElementNS( mnXmlNamespace, XML_nvSpPr );
    }
    else
        pFS->singleElementNS(mnXmlNamespace, XML_cNvSpPr, FSEND);

    // visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_spPr, FSEND );
    WriteShapeTransformation( xShape, XML_a, bFlipH, bFlipV, false);

    if( sShapeType == "ooxml-non-primitive" ) // non-primitiv -> custom geometry
    {
        WritePolyPolygon( EscherPropertyContainer::GetPolyPolygon( xShape ) );
    }
    else // preset geometry
    {
        if( nAdjustmentValuesIndex != -1 )
        {
            sal_Int32 nAdjustmentsWhichNeedsToBeConverted = 0;
            WritePresetShape( sPresetShape, eShapeType, bPredefinedHandlesUsed,
                              nAdjustmentsWhichNeedsToBeConverted, aGeometrySeq[ nAdjustmentValuesIndex ] );
        }
        else
            WritePresetShape( sPresetShape );
    }
    if( rXPropSet.is() )
    {
        WriteFill( rXPropSet );
        WriteOutline( rXPropSet );
    }

    pFS->endElementNS( mnXmlNamespace, XML_spPr );

    pFS->startElementNS( mnXmlNamespace, XML_style, FSEND );
    WriteShapeStyle( rXPropSet );
    pFS->endElementNS( mnXmlNamespace, XML_style );

    // write text
    WriteTextBox( xShape, mnXmlNamespace );

    pFS->endElementNS( mnXmlNamespace, (GetDocumentType() != DOCUMENT_DOCX ? XML_sp : XML_wsp) );

    return *this;
}

ShapeExport& ShapeExport::WriteEllipseShape( Reference< XShape > xShape )
{
    DBG(fprintf(stderr, "write ellipse shape\n"));

    FSHelperPtr pFS = GetFS();

    pFS->startElementNS( mnXmlNamespace, (GetDocumentType() != DOCUMENT_DOCX ? XML_sp : XML_wsp), FSEND );

    // TODO: arc, section, cut, connector

    // non visual shape properties
    if (GetDocumentType() != DOCUMENT_DOCX)
    {
        pFS->startElementNS( mnXmlNamespace, XML_nvSpPr, FSEND );
        pFS->singleElementNS( mnXmlNamespace, XML_cNvPr,
                XML_id, I32S( GetNewShapeID( xShape ) ),
                XML_name, IDS( Ellipse ),
                FSEND );
        pFS->singleElementNS( mnXmlNamespace, XML_cNvSpPr, FSEND );
        WriteNonVisualProperties( xShape );
        pFS->endElementNS( mnXmlNamespace, XML_nvSpPr );
    }
    else
        pFS->singleElementNS(mnXmlNamespace, XML_cNvSpPr, FSEND);

    // visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_spPr, FSEND );
    WriteShapeTransformation( xShape, XML_a, false, false, false);
    WritePresetShape( "ellipse" );
    Reference< XPropertySet > xProps( xShape, UNO_QUERY );
    if( xProps.is() )
    {
        WriteFill( xProps );
        WriteOutline( xProps );
    }
    pFS->endElementNS( mnXmlNamespace, XML_spPr );

    // write text
    WriteTextBox( xShape, mnXmlNamespace );

    pFS->endElementNS( mnXmlNamespace, (GetDocumentType() != DOCUMENT_DOCX ? XML_sp : XML_wsp) );

    return *this;
}

ShapeExport& ShapeExport::WriteGraphicObjectShape( Reference< XShape > xShape )
{
    WriteGraphicObjectShapePart( xShape );

    return *this;
}

void ShapeExport::WriteGraphicObjectShapePart( Reference< XShape > xShape, const Graphic* pGraphic )
{
    DBG(fprintf(stderr, "write graphic object shape\n"));

    if( NonEmptyText( xShape ) )
    {
        // avoid treating all 'IsPresentationObject' objects as having text.
        Reference< XSimpleText > xText( xShape, UNO_QUERY );

        if( xText.is() && xText->getString().getLength() )
        {
            DBG(fprintf(stderr, "graphicObject: wrote only text\n"));

            WriteTextShape( xShape );

            //DBG(dump_pset(mXPropSet));
            return;
        }
    }

    DBG(fprintf(stderr, "graphicObject without text\n"));

    OUString sGraphicURL;
    Reference< XPropertySet > xShapeProps( xShape, UNO_QUERY );
    if( !pGraphic && ( !xShapeProps.is() || !( xShapeProps->getPropertyValue( "GraphicURL" ) >>= sGraphicURL ) ) )
    {
        DBG(fprintf(stderr, "no graphic URL found\n"));
        return;
    }

    FSHelperPtr pFS = GetFS();

    if (GetDocumentType() != DOCUMENT_DOCX)
        pFS->startElementNS( mnXmlNamespace, XML_pic, FSEND );
    else
        pFS->startElementNS( mnXmlNamespace, XML_pic,
                             FSNS(XML_xmlns, XML_pic), "http://schemas.openxmlformats.org/drawingml/2006/picture",
                             FSEND );

    pFS->startElementNS( mnXmlNamespace, XML_nvPicPr, FSEND );

    OUString sName, sDescr;
    bool bHaveName, bHaveDesc;

    if ( ( bHaveName= GetProperty( xShapeProps, "Name" ) ) )
        mAny >>= sName;
    if ( ( bHaveDesc = GetProperty( xShapeProps, "Description" ) ) )
        mAny >>= sDescr;

    pFS->singleElementNS( mnXmlNamespace, XML_cNvPr,
                          XML_id,     I32S( GetNewShapeID( xShape ) ),
                          XML_name,   bHaveName ? USS( sName ) : OString( "Picture " + OString::number( mnPictureIdMax++ )).getStr(),
                          XML_descr,  bHaveDesc ? USS( sDescr ) : NULL,
                          FSEND );
    // OOXTODO: //cNvPr children: XML_extLst, XML_hlinkClick, XML_hlinkHover

    pFS->singleElementNS( mnXmlNamespace, XML_cNvPicPr,
                          // OOXTODO: XML_preferRelativeSize
                          FSEND );

    WriteNonVisualProperties( xShape );

    pFS->endElementNS( mnXmlNamespace, XML_nvPicPr );

    pFS->startElementNS( mnXmlNamespace, XML_blipFill, FSEND );

    WriteBlip( xShapeProps, sGraphicURL, false, pGraphic );

    WriteSrcRect( xShapeProps, sGraphicURL );

    // now we stretch always when we get pGraphic (when changing that
    // behavior, test n#780830 for regression, where the OLE sheet might get tiled
    bool bStretch = false;
    if( !pGraphic && GetProperty( xShapeProps, "FillBitmapStretch" ) )
        mAny >>= bStretch;

    if ( pGraphic || bStretch )
        pFS->singleElementNS( XML_a, XML_stretch, FSEND );

    pFS->endElementNS( mnXmlNamespace, XML_blipFill );

    // visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_spPr, FSEND );
    WriteShapeTransformation( xShape, XML_a, false, false, false);
    WritePresetShape( "rect" );
    // graphic object can come with the frame (bnc#654525)
    WriteOutline( xShapeProps );
    pFS->endElementNS( mnXmlNamespace, XML_spPr );

    pFS->endElementNS( mnXmlNamespace, XML_pic );
}

ShapeExport& ShapeExport::WriteConnectorShape( Reference< XShape > xShape )
{
    bool bFlipH = false;
    bool bFlipV = false;

    DBG(fprintf(stderr, "write connector shape\n"));

    FSHelperPtr pFS = GetFS();

    const char* sGeometry = "line";
    Reference< XPropertySet > rXPropSet( xShape, UNO_QUERY );
    Reference< XPropertyState > rXPropState( xShape, UNO_QUERY );
    awt::Point aStartPoint, aEndPoint;
    Reference< XShape > rXShapeA;
    Reference< XShape > rXShapeB;
    PropertyState eState;
    ConnectorType eConnectorType;
    if( GETAD( EdgeKind ) ) {
        mAny >>= eConnectorType;

        switch( eConnectorType ) {
            case ConnectorType_CURVE:
                sGeometry = "curvedConnector3";
                break;
            case ConnectorType_STANDARD:
                sGeometry = "bentConnector3";
                break;
            default:
            case ConnectorType_LINE:
            case ConnectorType_LINES:
                sGeometry = "straightConnector1";
                break;
        }

        if( GETAD( EdgeStartPoint ) ) {
            mAny >>= aStartPoint;
            if( GETAD( EdgeEndPoint ) ) {
                mAny >>= aEndPoint;
            }
        }
        GET( rXShapeA, EdgeStartConnection );
        GET( rXShapeB, EdgeEndConnection );
    }
    EscherConnectorListEntry aConnectorEntry( xShape, aStartPoint, rXShapeA, aEndPoint, rXShapeB );

    Rectangle aRect( Point( aStartPoint.X, aStartPoint.Y ), Point( aEndPoint.X, aEndPoint.Y ) );
    if( aRect.getWidth() < 0 ) {
        bFlipH = true;
        aRect.setX( aEndPoint.X );
        aRect.setWidth( aStartPoint.X - aEndPoint.X );
    }

    if( aRect.getHeight() < 0 ) {
        bFlipV = true;
        aRect.setY( aEndPoint.Y );
        aRect.setHeight( aStartPoint.Y - aEndPoint.Y );
    }

    pFS->startElementNS( mnXmlNamespace, XML_cxnSp, FSEND );

    // non visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_nvCxnSpPr, FSEND );
    pFS->singleElementNS( mnXmlNamespace, XML_cNvPr,
                          XML_id, I32S( GetNewShapeID( xShape ) ),
                          XML_name, IDS( Line ),
                          FSEND );
    // non visual connector shape drawing properties
    pFS->startElementNS( mnXmlNamespace, XML_cNvCxnSpPr, FSEND );
    WriteConnectorConnections( aConnectorEntry, GetShapeID( rXShapeA ), GetShapeID( rXShapeB ) );
    pFS->endElementNS( mnXmlNamespace, XML_cNvCxnSpPr );
    pFS->singleElementNS( mnXmlNamespace, XML_nvPr, FSEND );
    pFS->endElementNS( mnXmlNamespace, XML_nvCxnSpPr );

    // visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_spPr, FSEND );
    WriteTransformation( aRect, XML_a, bFlipH, bFlipV );
    // TODO: write adjustments (ppt export doesn't work well there either)
    WritePresetShape( sGeometry );
    Reference< XPropertySet > xShapeProps( xShape, UNO_QUERY );
    if( xShapeProps.is() )
        WriteOutline( xShapeProps );
    pFS->endElementNS( mnXmlNamespace, XML_spPr );

    // write text
    WriteTextBox( xShape, mnXmlNamespace );

    pFS->endElementNS( mnXmlNamespace, XML_cxnSp );

    return *this;
}

ShapeExport& ShapeExport::WriteLineShape( Reference< XShape > xShape )
{
    bool bFlipH = false;
    bool bFlipV = false;

    DBG(fprintf(stderr, "write line shape\n"));

    FSHelperPtr pFS = GetFS();

    pFS->startElementNS( mnXmlNamespace, (GetDocumentType() != DOCUMENT_DOCX ? XML_sp : XML_wsp), FSEND );

    PolyPolygon aPolyPolygon = EscherPropertyContainer::GetPolyPolygon( xShape );
    if( aPolyPolygon.Count() == 1 && aPolyPolygon[ 0 ].GetSize() == 2)
    {
        const Polygon& rPoly = aPolyPolygon[ 0 ];

        bFlipH = ( rPoly[ 0 ].X() > rPoly[ 1 ].X() );
        bFlipV = ( rPoly[ 0 ].Y() > rPoly[ 1 ].Y() );
    }

    // non visual shape properties
    if (GetDocumentType() != DOCUMENT_DOCX)
    {
        pFS->startElementNS( mnXmlNamespace, XML_nvSpPr, FSEND );
        pFS->singleElementNS( mnXmlNamespace, XML_cNvPr,
                              XML_id, I32S( GetNewShapeID( xShape ) ),
                              XML_name, IDS( Line ),
                              FSEND );
    }
    pFS->singleElementNS( mnXmlNamespace, XML_cNvSpPr, FSEND );
    if (GetDocumentType() != DOCUMENT_DOCX)
    {
        WriteNonVisualProperties( xShape );
        pFS->endElementNS( mnXmlNamespace, XML_nvSpPr );
    }

    // visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_spPr, FSEND );
    WriteShapeTransformation( xShape, XML_a, bFlipH, bFlipV, true);
    WritePresetShape( "line" );
    Reference< XPropertySet > xShapeProps( xShape, UNO_QUERY );
    if( xShapeProps.is() )
        WriteOutline( xShapeProps );
    pFS->endElementNS( mnXmlNamespace, XML_spPr );

    // write text
    WriteTextBox( xShape, mnXmlNamespace );

    pFS->endElementNS( mnXmlNamespace, (GetDocumentType() != DOCUMENT_DOCX ? XML_sp : XML_wsp) );

    return *this;
}

ShapeExport& ShapeExport::WriteNonVisualDrawingProperties( Reference< XShape > xShape, const char* pName )
{
    GetFS()->singleElementNS( mnXmlNamespace, XML_cNvPr,
                              XML_id, I32S( GetNewShapeID( xShape ) ),
                              XML_name, pName,
                              FSEND );

    return *this;
}

ShapeExport& ShapeExport::WriteNonVisualProperties( Reference< XShape > )
{
    // Override to generate //nvPr elements.
    return *this;
}

ShapeExport& ShapeExport::WriteRectangleShape( Reference< XShape > xShape )
{
    DBG(fprintf(stderr, "write rectangle shape\n"));

    FSHelperPtr pFS = GetFS();

    pFS->startElementNS( mnXmlNamespace, (GetDocumentType() != DOCUMENT_DOCX ? XML_sp : XML_wsp), FSEND );

    sal_Int32 nRadius = 0;

    Reference< XPropertySet > xShapeProps( xShape, UNO_QUERY );
    if( xShapeProps.is() )
    {
        xShapeProps->getPropertyValue( "CornerRadius" ) >>= nRadius;
    }

    if( nRadius )
    {
        nRadius = MapSize( awt::Size( nRadius, 0 ) ).Width;
    }

    // non visual shape properties
    if (GetDocumentType() == DOCUMENT_DOCX)
        pFS->singleElementNS( mnXmlNamespace, XML_cNvSpPr, FSEND );
    pFS->startElementNS( mnXmlNamespace, XML_nvSpPr, FSEND );
    pFS->singleElementNS( mnXmlNamespace, XML_cNvPr,
                          XML_id, I32S( GetNewShapeID( xShape ) ),
                          XML_name, IDS( Rectangle ),
                          FSEND );
    pFS->singleElementNS( mnXmlNamespace, XML_cNvSpPr, FSEND );
    WriteNonVisualProperties( xShape );
    pFS->endElementNS( mnXmlNamespace, XML_nvSpPr );

    // visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_spPr, FSEND );
    WriteShapeTransformation( xShape, XML_a, false, false, false);
    WritePresetShape( "rect" );
    Reference< XPropertySet > xProps( xShape, UNO_QUERY );
    if( xProps.is() )
    {
        WriteFill( xProps );
        WriteOutline( xProps );
    }
    pFS->endElementNS( mnXmlNamespace, XML_spPr );

    // write text
    WriteTextBox( xShape, mnXmlNamespace );

    pFS->endElementNS( mnXmlNamespace, (GetDocumentType() != DOCUMENT_DOCX ? XML_sp : XML_wsp) );

    return *this;
}

typedef ShapeExport& (ShapeExport::*ShapeConverter)( Reference< XShape > );
typedef boost::unordered_map< const char*, ShapeConverter, rtl::CStringHash, rtl::CStringEqual> NameToConvertMapType;

static const NameToConvertMapType& lcl_GetConverters(DrawingML::DocumentType eDocumentType)
{
    static bool shape_map_inited = false;
    static NameToConvertMapType shape_converters;
    if( shape_map_inited )
    {
        return shape_converters;
    }

    shape_converters[ "com.sun.star.drawing.ClosedBezierShape" ]        = &ShapeExport::WriteClosedBezierShape;
    shape_converters[ "com.sun.star.drawing.ConnectorShape" ]           = &ShapeExport::WriteConnectorShape;
    shape_converters[ "com.sun.star.drawing.CustomShape" ]              = &ShapeExport::WriteCustomShape;
    shape_converters[ "com.sun.star.drawing.EllipseShape" ]             = &ShapeExport::WriteEllipseShape;
    shape_converters[ "com.sun.star.drawing.GraphicObjectShape" ]       = &ShapeExport::WriteGraphicObjectShape;
    shape_converters[ "com.sun.star.drawing.LineShape" ]                = &ShapeExport::WriteLineShape;
    shape_converters[ "com.sun.star.drawing.OpenBezierShape" ]          = &ShapeExport::WriteOpenBezierShape;
    shape_converters[ "com.sun.star.drawing.RectangleShape" ]           = &ShapeExport::WriteRectangleShape;
    shape_converters[ "com.sun.star.drawing.OLE2Shape" ]                = &ShapeExport::WriteOLE2Shape;
    shape_converters[ "com.sun.star.drawing.TableShape" ]               = &ShapeExport::WriteTableShape;
    shape_converters[ "com.sun.star.drawing.TextShape" ]                = &ShapeExport::WriteTextShape;

    shape_converters[ "com.sun.star.presentation.GraphicObjectShape" ]  = &ShapeExport::WriteGraphicObjectShape;
    shape_converters[ "com.sun.star.presentation.OLE2Shape" ]           = &ShapeExport::WriteOLE2Shape;
    shape_converters[ "com.sun.star.presentation.TableShape" ]          = &ShapeExport::WriteTableShape;
    shape_converters[ "com.sun.star.presentation.TextShape" ]           = &ShapeExport::WriteTextShape;

    shape_converters[ "com.sun.star.presentation.DateTimeShape" ]       = &ShapeExport::WriteTextShape;
    shape_converters[ "com.sun.star.presentation.FooterShape" ]         = &ShapeExport::WriteTextShape;
    shape_converters[ "com.sun.star.presentation.HeaderShape" ]         = &ShapeExport::WriteTextShape;
    shape_converters[ "com.sun.star.presentation.NotesShape" ]          = &ShapeExport::WriteTextShape;
    shape_converters[ "com.sun.star.presentation.OutlinerShape" ]       = &ShapeExport::WriteTextShape;
    shape_converters[ "com.sun.star.presentation.SlideNumberShape" ]    = &ShapeExport::WriteTextShape;
    shape_converters[ "com.sun.star.presentation.TitleTextShape" ]      = &ShapeExport::WriteTextShape;
    if (eDocumentType == DrawingML::DOCUMENT_DOCX)
        shape_converters[ "com.sun.star.drawing.GroupShape" ] = &ShapeExport::WriteGroupShape;
    shape_map_inited = true;

    return shape_converters;
}

ShapeExport& ShapeExport::WriteShape( Reference< XShape > xShape )
{
    OUString sShapeType = xShape->getShapeType();
    DBG( fprintf( stderr, "write shape: %s\n", USS( sShapeType ) ) );
    NameToConvertMapType::const_iterator aConverter = lcl_GetConverters(GetDocumentType()).find( USS( sShapeType ) );
    if( aConverter == lcl_GetConverters(GetDocumentType()).end() )
    {
        DBG( fprintf( stderr, "unknown shape\n" ) );
        return WriteUnknownShape( xShape );
    }
    (this->*(aConverter->second))( xShape );

    return *this;
}

ShapeExport& ShapeExport::WriteTextBox( Reference< XInterface > xIface, sal_Int32 nXmlNamespace )
{
    if( NonEmptyText( xIface ) )
    {
        FSHelperPtr pFS = GetFS();

        pFS->startElementNS( nXmlNamespace, (GetDocumentType() != DOCUMENT_DOCX ? XML_txBody : XML_txbx), FSEND );
        WriteText( xIface, /*bBodyPr=*/(GetDocumentType() != DOCUMENT_DOCX), /*bText=*/true );
        pFS->endElementNS( nXmlNamespace, (GetDocumentType() != DOCUMENT_DOCX ? XML_txBody : XML_txbx) );
        if (GetDocumentType() == DOCUMENT_DOCX)
            WriteText( xIface, /*bBodyPr=*/true, /*bText=*/false, /*nXmlNamespace=*/nXmlNamespace );
    }
    else if (GetDocumentType() == DOCUMENT_DOCX)
        mpFS->singleElementNS(nXmlNamespace, XML_bodyPr, FSEND);

    return *this;
}

void ShapeExport::WriteTable( Reference< XShape > rXShape  )
{
    OSL_TRACE("write table");

    Reference< XTable > xTable;
    Reference< XPropertySet > xPropSet( rXShape, UNO_QUERY );

    mpFS->startElementNS( XML_a, XML_graphic, FSEND );
    mpFS->startElementNS( XML_a, XML_graphicData, XML_uri, "http://schemas.openxmlformats.org/drawingml/2006/table", FSEND );

    if ( xPropSet.is() && ( xPropSet->getPropertyValue( "Model" ) >>= xTable ) )
    {
        mpFS->startElementNS( XML_a, XML_tbl, FSEND );
        mpFS->singleElementNS( XML_a, XML_tblPr, FSEND );

        Reference< container::XIndexAccess > xColumns( xTable->getColumns(), UNO_QUERY_THROW );
        Reference< container::XIndexAccess > xRows( xTable->getRows(), UNO_QUERY_THROW );
        sal_uInt16 nRowCount = static_cast< sal_uInt16 >( xRows->getCount() );
        sal_uInt16 nColumnCount = static_cast< sal_uInt16 >( xColumns->getCount() );

        mpFS->startElementNS( XML_a, XML_tblGrid, FSEND );

        for ( sal_Int32 x = 0; x < nColumnCount; x++ )
        {
            Reference< XPropertySet > xColPropSet( xColumns->getByIndex( x ), UNO_QUERY_THROW );
            sal_Int32 nWidth(0);
            xColPropSet->getPropertyValue( "Width" ) >>= nWidth;

            mpFS->singleElementNS( XML_a, XML_gridCol, XML_w, I64S(MM100toEMU(nWidth)), FSEND );
        }

        mpFS->endElementNS( XML_a, XML_tblGrid );

        for( sal_Int32 nRow = 0; nRow < nRowCount; nRow++ )
        {
            Reference< XPropertySet > xRowPropSet( xRows->getByIndex( nRow ), UNO_QUERY_THROW );
            sal_Int32 nRowHeight(0);

            xRowPropSet->getPropertyValue( "Height" ) >>= nRowHeight;

            mpFS->startElementNS( XML_a, XML_tr, XML_h, I64S( MM100toEMU( nRowHeight ) ), FSEND );

            for( sal_Int32 nColumn = 0; nColumn < nColumnCount; nColumn++ )
            {
                Reference< XMergeableCell > xCell( xTable->getCellByPosition( nColumn, nRow ), UNO_QUERY_THROW );
                if ( !xCell->isMerged() )
                {
                    mpFS->startElementNS( XML_a, XML_tc, FSEND );

                    WriteTextBox( xCell, XML_a );

                    mpFS->singleElementNS( XML_a, XML_tcPr, FSEND );
                    mpFS->endElementNS( XML_a, XML_tc );
                }
            }

            mpFS->endElementNS( XML_a, XML_tr );
        }

        mpFS->endElementNS( XML_a, XML_tbl );
    }

    mpFS->endElementNS( XML_a, XML_graphicData );
    mpFS->endElementNS( XML_a, XML_graphic );
}

ShapeExport& ShapeExport::WriteTableShape( Reference< XShape > xShape )
{
    FSHelperPtr pFS = GetFS();

    OSL_TRACE("write table shape");

    pFS->startElementNS( mnXmlNamespace, XML_graphicFrame, FSEND );

    pFS->startElementNS( mnXmlNamespace, XML_nvGraphicFramePr, FSEND );

    pFS->singleElementNS( mnXmlNamespace, XML_cNvPr,
                          XML_id,     I32S( GetNewShapeID( xShape ) ),
                          XML_name,   IDS(Table),
                          FSEND );

    pFS->singleElementNS( mnXmlNamespace, XML_cNvGraphicFramePr,
                          FSEND );

    if( GetDocumentType() == DOCUMENT_PPTX )
        pFS->singleElementNS( mnXmlNamespace, XML_nvPr,
                          FSEND );
    pFS->endElementNS( mnXmlNamespace, XML_nvGraphicFramePr );

    WriteShapeTransformation( xShape, mnXmlNamespace, false);
    WriteTable( xShape );

    pFS->endElementNS( mnXmlNamespace, XML_graphicFrame );

    return *this;
}

ShapeExport& ShapeExport::WriteTextShape( Reference< XShape > xShape )
{
    FSHelperPtr pFS = GetFS();

    pFS->startElementNS( mnXmlNamespace, (GetDocumentType() != DOCUMENT_DOCX ? XML_sp : XML_wsp), FSEND );

    // non visual shape properties
    if (GetDocumentType() != DOCUMENT_DOCX)
    {
        pFS->startElementNS( mnXmlNamespace, XML_nvSpPr, FSEND );
        WriteNonVisualDrawingProperties( xShape, IDS( TextShape ) );
    }
    pFS->singleElementNS( mnXmlNamespace, XML_cNvSpPr, XML_txBox, "1", FSEND );
    if (GetDocumentType() != DOCUMENT_DOCX)
    {
        WriteNonVisualProperties( xShape );
        pFS->endElementNS( mnXmlNamespace, XML_nvSpPr );
    }

    // visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_spPr, FSEND );
    WriteShapeTransformation( xShape, XML_a, false, false, false);
    WritePresetShape( "rect" );
    uno::Reference<beans::XPropertySet> xPropertySet(xShape, UNO_QUERY);
    WriteBlipOrNormalFill(xPropertySet, "GraphicURL");
    WriteOutline(xPropertySet);
    pFS->endElementNS( mnXmlNamespace, XML_spPr );

    WriteTextBox( xShape, mnXmlNamespace );

    pFS->endElementNS( mnXmlNamespace, (GetDocumentType() != DOCUMENT_DOCX ? XML_sp : XML_wsp) );

    return *this;
}

ShapeExport& ShapeExport::WriteOLE2Shape( Reference< XShape > xShape )
{
    Reference< XPropertySet > xPropSet( xShape, UNO_QUERY );
    if( xPropSet.is() ) {
        if( GetProperty( xPropSet, "Model" ) )
        {
            Reference< XChartDocument > xChartDoc;
            mAny >>= xChartDoc;
            if( xChartDoc.is() )
            {
                //export the chart
                Reference< XModel > xModel( xChartDoc, UNO_QUERY );
                ChartExport aChartExport( mnXmlNamespace, GetFS(), xModel, GetFB(), GetDocumentType() );
                static sal_Int32 nChartCount = 0;
                aChartExport.WriteChartObj( xShape, ++nChartCount );
            }
            else
            {
                // this part now supports only embedded spreadsheets, it can be extended to support remaining ooxml documents
                // only exporter, counter and object filename are specific to spreadsheet
                Reference< XSpreadsheetDocument > xSheetDoc( mAny, UNO_QUERY );
                if( xSheetDoc.is() && mpFB)
                {
                    Reference< XComponent > xDocument( mAny, UNO_QUERY );
                    if( xDocument.is() )
                    {
                        Reference< XOutputStream > xOutStream = mpFB->openFragmentStream( OUStringBuffer()
                                                                                          .appendAscii( GetComponentDir() )
                                                                                          .appendAscii( "/embeddings/spreadsheet" )
                                                                                          .append( (sal_Int32) mnSpreadsheetCounter )
                                                                                          .appendAscii( ".xlsx" )
                                                                                          .makeStringAndClear(),
                                                                                          "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet" );
                        // export the embedded document
                        Sequence< PropertyValue > rMedia(1);

                        rMedia[0].Name = utl::MediaDescriptor::PROP_STREAMFOROUTPUT();
                        rMedia[0].Value <<= xOutStream;

                        Reference< XExporter > xExporter(
                            mpFB->getComponentContext()->getServiceManager()->
                                createInstanceWithContext(
                                    "com.sun.star.comp.oox.xls.ExcelFilter",
                                    mpFB->getComponentContext() ),
                            UNO_QUERY_THROW );
                        xExporter->setSourceDocument( xDocument );
                        Reference< XFilter >( xExporter, UNO_QUERY_THROW )->
                            filter( rMedia );

                        xOutStream->closeOutput();

                        OUString sRelId = mpFB->addRelation( mpFS->getOutputStream(),
                                                             "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package",
                                                             OUStringBuffer()
                                                             .appendAscii( GetRelationCompPrefix() )
                                                             .appendAscii( "embeddings/spreadsheet" )
                                                             .append( (sal_Int32) mnSpreadsheetCounter ++ )
                                                             .appendAscii( ".xlsx" )
                                                             .makeStringAndClear() );

                        mpFS->startElementNS( mnXmlNamespace, XML_graphicFrame, FSEND );

                        mpFS->startElementNS( mnXmlNamespace, XML_nvGraphicFramePr, FSEND );

                        mpFS->singleElementNS( mnXmlNamespace, XML_cNvPr,
                                               XML_id,     I32S( GetNewShapeID( xShape ) ),
                                               XML_name,   IDS(Object),
                                               FSEND );

                        mpFS->singleElementNS( mnXmlNamespace, XML_cNvGraphicFramePr,
                                               FSEND );

                        if( GetDocumentType() == DOCUMENT_PPTX )
                            mpFS->singleElementNS( mnXmlNamespace, XML_nvPr,
                                                   FSEND );
                        mpFS->endElementNS( mnXmlNamespace, XML_nvGraphicFramePr );

                        WriteShapeTransformation( xShape, mnXmlNamespace );

                        mpFS->startElementNS( XML_a, XML_graphic, FSEND );
                        mpFS->startElementNS( XML_a, XML_graphicData,
                                              XML_uri, "http://schemas.openxmlformats.org/presentationml/2006/ole",
                                              FSEND );
                        mpFS->startElementNS( mnXmlNamespace, XML_oleObj,
                                              XML_name, "Spreadsheet",
                                              FSNS(XML_r, XML_id), USS( sRelId ),
                                              FSEND );

                        mpFS->singleElementNS( mnXmlNamespace, XML_embed, FSEND );

                        // pic element
                        SdrObject* pSdrOLE2( GetSdrObjectFromXShape( xShape ) );
                        if ( pSdrOLE2 && pSdrOLE2->ISA( SdrOle2Obj ) )
                        {
                            const Graphic* pGraphic = ((SdrOle2Obj*)pSdrOLE2)->GetGraphic();
                            if ( pGraphic )
                                WriteGraphicObjectShapePart( xShape, pGraphic );
                        }

                        mpFS->endElementNS( mnXmlNamespace, XML_oleObj );

                        mpFS->endElementNS( XML_a, XML_graphicData );
                        mpFS->endElementNS( XML_a, XML_graphic );

                        mpFS->endElementNS( mnXmlNamespace, XML_graphicFrame );
                    }
                }
            }
        }
    }
    return *this;
}

ShapeExport& ShapeExport::WriteUnknownShape( Reference< XShape > )
{
    // Override this method to do something useful.
    return *this;
}

size_t ShapeExport::ShapeHash::operator()( const Reference < XShape > rXShape ) const
{
    return rXShape->getShapeType().hashCode();
}

sal_Int32 ShapeExport::GetNewShapeID( const Reference< XShape > rXShape )
{
    return GetNewShapeID( rXShape, GetFB() );
}

sal_Int32 ShapeExport::GetNewShapeID( const Reference< XShape > rXShape, XmlFilterBase* pFB )
{
    if( !rXShape.is() )
        return -1;

    sal_Int32 nID = pFB->GetUniqueId();

    (*mpShapeMap)[ rXShape ] = nID;

    return nID;
}

sal_Int32 ShapeExport::GetShapeID( const Reference< XShape > rXShape )
{
    return GetShapeID( rXShape, mpShapeMap );
}

sal_Int32 ShapeExport::GetShapeID( const Reference< XShape > rXShape, ShapeHashMap* pShapeMap )
{
    if( !rXShape.is() )
        return -1;

    ShapeHashMap::const_iterator aIter = pShapeMap->find( rXShape );

    if( aIter == pShapeMap->end() )
        return -1;

    return aIter->second;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
