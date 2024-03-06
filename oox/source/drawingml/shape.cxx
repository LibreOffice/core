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

#include <config_wasm_strip.h>

#include <oox/drawingml/shape.hxx>
#include <drawingml/customshapeproperties.hxx>
#include <oox/drawingml/theme.hxx>
#include <drawingml/fillproperties.hxx>
#include <drawingml/fontworkhelpers.hxx>
#include <drawingml/graphicproperties.hxx>
#include <drawingml/lineproperties.hxx>
#include <drawingml/presetgeometrynames.hxx>
#include <drawingml/shape3dproperties.hxx>
#include <drawingml/scene3dhelper.hxx>
#include "effectproperties.hxx"
#include <oox/drawingml/shapepropertymap.hxx>
#include <drawingml/textbody.hxx>
#include <drawingml/textparagraph.hxx>
#include <drawingml/ThemeOverrideFragmentHandler.hxx>
#include <drawingml/table/tableproperties.hxx>
#include <oox/drawingml/chart/chartconverter.hxx>
#include <drawingml/chart/chartspacefragment.hxx>
#include <drawingml/chart/chartspacemodel.hxx>
#include <o3tl/safeint.hxx>
#include <o3tl/unit_conversion.hxx>
#include <oox/ppt/pptimport.hxx>
#include <oox/vml/vmldrawing.hxx>
#include <oox/vml/vmlshape.hxx>
#include <oox/vml/vmlshapecontainer.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <oox/helper/graphichelper.hxx>
#include <oox/helper/propertyset.hxx>
#include <oox/helper/modelobjecthelper.hxx>
#include <oox/mathml/imexport.hxx>
#include <oox/mathml/importutils.hxx>
#include <oox/token/properties.hxx>
#include "diagram/datamodel.hxx"
#include "diagram/diagramhelper.hxx"

#include <comphelper/classids.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <tools/gen.hxx>
#include <tools/globname.hxx>
#include <tools/mapunit.hxx>
#include <editeng/unoprnms.hxx>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/sax/XFastSAXSerializable.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <com/sun/star/drawing/XEnhancedCustomShapeDefaulter.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeTextPathMode.hpp>
#include <com/sun/star/drawing/ConnectorType.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/text/WritingMode2.hpp>

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <com/sun/star/document/XActionLockable.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <officecfg/Office/Common.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdotable.hxx>
#include <svx/svdtrans.hxx>
#include <tools/stream.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/mediadescriptor.hxx>
#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wmfexternal.hxx>
#include <sal/log.hxx>
#include <svx/sdtaitm.hxx>
#include <oox/drawingml/diagram/diagram.hxx>
#include <docmodel/theme/Theme.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/mslangid.hxx>

using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::style;

namespace oox::drawingml {

Shape::Shape()
: mpLinePropertiesPtr( std::make_shared<LineProperties>() )
, mpShapeRefLinePropPtr( std::make_shared<LineProperties>() )
, mpFillPropertiesPtr( std::make_shared<FillProperties>() )
, mpShapeRefFillPropPtr( std::make_shared<FillProperties>() )
, mpGraphicPropertiesPtr( std::make_shared<GraphicProperties>() )
, mpCustomShapePropertiesPtr( std::make_shared<CustomShapeProperties>() )
, mp3DPropertiesPtr( std::make_shared<Shape3DProperties>() )
, mpEffectPropertiesPtr( std::make_shared<EffectProperties>() )
, mpShapeRefEffectPropPtr( std::make_shared<EffectProperties>() )
, mpMasterTextListStyle( std::make_shared<TextListStyle>() )
, mnSubType( 0 )
, meFrameType( FRAMETYPE_GENERIC )
, mnRotation( 0 )
, mnDiagramRotation( 0 )
, mbFlipH( false )
, mbFlipV( false )
, mbHidden( false )
, mbHiddenMasterShape( false )
, mbLocked( false )
, mbWPGChild(false)
, mbLockedCanvas( false )
, mbWordprocessingCanvas(false)
, mbWps( false )
, mbTextBox( false )
, mbHasLinkedTxbx( false )
, maDiagramDoms( 0 )
, mpDiagramHelper( nullptr )
{
    setDefaults(/*bDefaultHeight*/true);
}


Shape::Shape( const OUString& rServiceName, bool bDefaultHeight )
: mpLinePropertiesPtr( std::make_shared<LineProperties>() )
, mpShapeRefLinePropPtr( std::make_shared<LineProperties>() )
, mpFillPropertiesPtr( std::make_shared<FillProperties>() )
, mpShapeRefFillPropPtr( std::make_shared<FillProperties>() )
, mpGraphicPropertiesPtr( std::make_shared<GraphicProperties>() )
, mpCustomShapePropertiesPtr( std::make_shared<CustomShapeProperties>() )
, mp3DPropertiesPtr( std::make_shared<Shape3DProperties>() )
, mpEffectPropertiesPtr( std::make_shared<EffectProperties>() )
, mpShapeRefEffectPropPtr( std::make_shared<EffectProperties>() )
, mpMasterTextListStyle( std::make_shared<TextListStyle>() )
, mnSubType( 0 )
, meFrameType( FRAMETYPE_GENERIC )
, mnRotation( 0 )
, mnDiagramRotation( 0 )
, mbFlipH( false )
, mbFlipV( false )
, mbHidden( false )
, mbHiddenMasterShape( false )
, mbLocked( false )
, mbWPGChild(false)
, mbLockedCanvas( false )
, mbWordprocessingCanvas(false)
, mbWps( false )
, mbTextBox( false )
, mbHasLinkedTxbx( false )
, maDiagramDoms( 0 )
, mpDiagramHelper( nullptr )
{
    msServiceName = rServiceName;
    setDefaults(bDefaultHeight);
}

Shape::Shape( const ShapePtr& pSourceShape )
: mpTextBody(pSourceShape->mpTextBody)
, mpLinePropertiesPtr( pSourceShape->mpLinePropertiesPtr )
, mpShapeRefLinePropPtr( pSourceShape->mpShapeRefLinePropPtr )
, mpFillPropertiesPtr( pSourceShape->mpFillPropertiesPtr )
, mpShapeRefFillPropPtr( pSourceShape->mpShapeRefFillPropPtr )
, mpGraphicPropertiesPtr( pSourceShape->mpGraphicPropertiesPtr )
, mpCustomShapePropertiesPtr( pSourceShape->mpCustomShapePropertiesPtr )
, mpTablePropertiesPtr( pSourceShape->mpTablePropertiesPtr )
, mp3DPropertiesPtr( pSourceShape->mp3DPropertiesPtr )
, mpEffectPropertiesPtr (pSourceShape->mpEffectPropertiesPtr)
, mpShapeRefEffectPropPtr(pSourceShape->mpShapeRefEffectPropPtr)
, maShapeProperties( pSourceShape->maShapeProperties )
, mpMasterTextListStyle( pSourceShape->mpMasterTextListStyle )
, msServiceName( pSourceShape->msServiceName )
, msName( pSourceShape->msName )
, msInternalName( pSourceShape->msInternalName )
, msId( pSourceShape->msId )
, mnSubType( pSourceShape->mnSubType )
, moSubTypeIndex( pSourceShape->moSubTypeIndex )
, maShapeStyleRefs( pSourceShape->maShapeStyleRefs )
, maSize( pSourceShape->maSize )
, maPosition( pSourceShape->maPosition )
, meFrameType( pSourceShape->meFrameType )
, mnRotation( pSourceShape->mnRotation )
, mnDiagramRotation( pSourceShape->mnDiagramRotation )
, mbFlipH( pSourceShape->mbFlipH )
, mbFlipV( pSourceShape->mbFlipV )
, mbHidden( pSourceShape->mbHidden )
, mbHiddenMasterShape( pSourceShape->mbHiddenMasterShape )
, mbLocked( pSourceShape->mbLocked )
, mbWPGChild( pSourceShape->mbWPGChild )
, mbLockedCanvas( pSourceShape->mbLockedCanvas )
, mbWordprocessingCanvas(pSourceShape->mbWordprocessingCanvas)
, mbWps( pSourceShape->mbWps )
, mbTextBox( pSourceShape->mbTextBox )
, mbHasLinkedTxbx(false)
, maDiagramDoms( pSourceShape->maDiagramDoms )
, mnZOrder(pSourceShape->mnZOrder)
, mnZOrderOff(pSourceShape->mnZOrderOff)
, mnDataNodeType(pSourceShape->mnDataNodeType)
, mfAspectRatio(pSourceShape->mfAspectRatio)
, mpDiagramHelper( nullptr )
, msDiagramDataModelID(pSourceShape->msDiagramDataModelID)
{}

Shape::~Shape()
{
    // DiagramHelper should not be set here anymore, see
    // propagateDiagramHelper below (maybe assert..?)
    delete mpDiagramHelper;
}

void Shape::prepareDiagramHelper(
    const std::shared_ptr< Diagram >& rDiagramPtr,
    const std::shared_ptr<::oox::drawingml::Theme>& rTheme)
{
    // Prepare Diagram data collecting for this Shape
    if( nullptr == mpDiagramHelper && FRAMETYPE_DIAGRAM == meFrameType )
    {
        mpDiagramHelper = new AdvancedDiagramHelper(
            rDiagramPtr,
            rTheme,
            getSize());
    }
}

void Shape::propagateDiagramHelper()
{
    // Propagate collected Diagram data to data holder
    if (FRAMETYPE_DIAGRAM == meFrameType && nullptr != mpDiagramHelper)
    {
        SdrObjGroup* pAnchorObj = dynamic_cast<SdrObjGroup*>(SdrObject::getSdrObjectFromXShape(mxShape));

        if(pAnchorObj)
        {
            mpDiagramHelper->doAnchor(*pAnchorObj, *this);
            mpDiagramHelper = nullptr;
        }
    }

    // If propagation failed, delete/cleanup here. Since the DiagramHelper
    // holds a Diagram and that this Shape it is necessary - the destructor
    // will not be called and will be too late
    if (nullptr != mpDiagramHelper)
    {
        delete mpDiagramHelper;
        mpDiagramHelper = nullptr;
    }
}

void Shape::migrateDiagramHelperToNewShape(const ShapePtr& pTarget)
{
    if(!mpDiagramHelper)
    {
        return;
    }

    if(!pTarget)
    {
        // no migrate target, but cleanup helper
        delete mpDiagramHelper;
        mpDiagramHelper = nullptr;
        return;
    }

    if(pTarget->mpDiagramHelper)
    {
        // this should no happen, but if there is already a helper, clean it up
        delete pTarget->mpDiagramHelper;
        pTarget->mpDiagramHelper = nullptr;
    }

    // exchange and reset to nullptr
    pTarget->mpDiagramHelper = mpDiagramHelper;
    mpDiagramHelper = nullptr;
}

table::TablePropertiesPtr const & Shape::getTableProperties()
{
    if ( !mpTablePropertiesPtr )
        mpTablePropertiesPtr = std::make_shared<table::TableProperties>();
    return mpTablePropertiesPtr;
}

void Shape::setDefaults(bool bHeight)
{
    maDefaultShapeProperties.setProperty(PROP_TextAutoGrowHeight, false);
    maDefaultShapeProperties.setProperty(PROP_TextWordWrap, true);
    maDefaultShapeProperties.setProperty(PROP_TextLeftDistance, static_cast< sal_Int32 >( 250 ));
    maDefaultShapeProperties.setProperty(PROP_TextUpperDistance, static_cast< sal_Int32 >( 125 ));
    maDefaultShapeProperties.setProperty(PROP_TextRightDistance, static_cast< sal_Int32 >( 250 ));
    maDefaultShapeProperties.setProperty(PROP_TextLowerDistance, static_cast< sal_Int32 >( 125 ));
    if (bHeight)
        maDefaultShapeProperties.setProperty(PROP_CharHeight, static_cast< float >( 18.0 ));
    maDefaultShapeProperties.setProperty(PROP_TextVerticalAdjust, TextVerticalAdjust_TOP);
    maDefaultShapeProperties.setProperty(PROP_ParaAdjust,
                                         static_cast<sal_Int16>(ParagraphAdjust_LEFT));
}

::oox::vml::OleObjectInfo& Shape::setOleObjectType()
{
    OSL_ENSURE( meFrameType == FRAMETYPE_GENERIC, "Shape::setOleObjectType - multiple frame types" );
    meFrameType = FRAMETYPE_OLEOBJECT;
    mxOleObjectInfo = std::make_shared<::oox::vml::OleObjectInfo>( true );
    return *mxOleObjectInfo;
}

ChartShapeInfo& Shape::setChartType( bool bEmbedShapes )
{
    OSL_ENSURE( meFrameType == FRAMETYPE_GENERIC, "Shape::setChartType - multiple frame types" );
    meFrameType = FRAMETYPE_CHART;
    if (mbWps)
        msServiceName = u"com.sun.star.drawing.temporaryForXMLImportOLE2Shape"_ustr;
    else
        msServiceName = u"com.sun.star.drawing.OLE2Shape"_ustr;
    mxChartShapeInfo = std::make_shared<ChartShapeInfo>( bEmbedShapes );
    return *mxChartShapeInfo;
}

void Shape::setDiagramType()
{
    OSL_ENSURE( meFrameType == FRAMETYPE_GENERIC, "Shape::setDiagramType - multiple frame types" );
    meFrameType = FRAMETYPE_DIAGRAM;
    msServiceName = u"com.sun.star.drawing.GroupShape"_ustr;
    mnSubType = 0;
}

void Shape::setTableType()
{
    OSL_ENSURE( meFrameType == FRAMETYPE_GENERIC, "Shape::setTableType - multiple frame types" );
    meFrameType = FRAMETYPE_TABLE;
    msServiceName = u"com.sun.star.drawing.TableShape"_ustr;
    mnSubType = 0;
}

const ShapeStyleRef* Shape::getShapeStyleRef( sal_Int32 nRefType ) const
{
    ShapeStyleRefMap::const_iterator aIt = maShapeStyleRefs.find( nRefType );
    return (aIt == maShapeStyleRefs.end()) ? nullptr : &aIt->second;
}

void Shape::addShape(
        ::oox::core::XmlFilterBase& rFilterBase,
        const Theme* pTheme,
        const Reference< XShapes >& rxShapes,
        const basegfx::B2DHomMatrix& aTransformation,
        const FillProperties& rShapeOrParentShapeFillProps,
        ShapeIdMap* pShapeMap,
        oox::drawingml::ShapePtr pParentGroupShape)
{
    SAL_INFO("oox.drawingml", "Shape::addShape: id='" << msId << "'");

    try
    {
        OUString sServiceName( msServiceName );
        if( !sServiceName.isEmpty() )
        {
            basegfx::B2DHomMatrix aMatrix( aTransformation );
            Reference< XShape > xShape( createAndInsert( rFilterBase, sServiceName, pTheme, rxShapes, false, false, aMatrix, rShapeOrParentShapeFillProps, pParentGroupShape) );

            if( pShapeMap && !msId.isEmpty() )
            {
                (*pShapeMap)[ msId ] = shared_from_this();
            }

            // if this is a group shape, we have to add also each child shape
            Reference< XShapes > xShapes( xShape, UNO_QUERY );
            if ( xShapes.is() )
                addChildren( rFilterBase, *this, pTheme, xShapes, pShapeMap, aMatrix );

            if (mbWordprocessingCanvas && !mbWPGChild)
            {
                // This is a drawing canvas. In case the canvas has no fill and no stroke, Word does
                // not render shadow or glow, even if it is set for the canvas. Thus we disable shadow
                // and glow in this case for the ersatz background shape of the drawing canvas.
                try
                {
                    oox::drawingml::ShapePtr pBgShape = getChildren().front();
                    const Reference<css::drawing::XShape>& xBgShape = pBgShape->getXShape();
                    Reference<XPropertySet> xBgProps(xBgShape, uno::UNO_QUERY);
                    drawing::FillStyle eFillStyle = drawing::FillStyle_NONE;
                    xBgProps->getPropertyValue("FillStyle") >>= eFillStyle;
                    drawing::LineStyle eLineStyle = drawing::LineStyle_NONE;
                    xBgProps->getPropertyValue("LineStyle") >>= eLineStyle;
                    if (eFillStyle == drawing::FillStyle_NONE
                        && eLineStyle == drawing::LineStyle_NONE)
                    {
                        xBgProps->setPropertyValue(UNO_NAME_SHADOW, uno::Any(false));
                        xBgProps->setPropertyValue(u"GlowEffectRadius"_ustr, uno::Any(sal_Int32(0)));
                    }
                }
                catch (const Exception&)
                {
                    TOOLS_WARN_EXCEPTION("oox.drawingml", "Shape::addShape mbWordprocessingCanvas");
                }
            }

            if (isWPGChild() && xShape)
            {
                // This is a wps shape and it is the child of the WPG, now copy the
                // the text body properties to the xshape.
                Reference<XPropertySet> xChildWPSProperties(xShape, uno::UNO_QUERY);

                if (getTextBody() && xChildWPSProperties)
                {
                    xChildWPSProperties->setPropertyValue(
                        UNO_NAME_TEXT_VERTADJUST,
                        uno::Any(getTextBody()->getTextProperties().meVA));

                    xChildWPSProperties->setPropertyValue(
                        UNO_NAME_TEXT_LEFTDIST,
                        uno::Any(getTextBody()->getTextProperties().moInsets[0].has_value()
                                     ? *getTextBody()->getTextProperties().moInsets[0]
                                     : 0));
                    xChildWPSProperties->setPropertyValue(
                        UNO_NAME_TEXT_UPPERDIST,
                        uno::Any(getTextBody()->getTextProperties().moInsets[1].has_value()
                                     ? *getTextBody()->getTextProperties().moInsets[1]
                                     : 0));
                    xChildWPSProperties->setPropertyValue(
                        UNO_NAME_TEXT_RIGHTDIST,
                        uno::Any(getTextBody()->getTextProperties().moInsets[2].has_value()
                                     ? *getTextBody()->getTextProperties().moInsets[2]
                                     : 0));
                    xChildWPSProperties->setPropertyValue(
                        UNO_NAME_TEXT_LOWERDIST,
                        uno::Any(getTextBody()->getTextProperties().moInsets[3].has_value()
                                     ? *getTextBody()->getTextProperties().moInsets[3]
                                     : 0));
                }

                // tdf#145147 Set the Hyperlink property to the child wps shape.
                if (getShapeProperties().hasProperty(PROP_URL)) try
                {
                    uno::Any aAny = getShapeProperties().getProperty(PROP_URL);
                    OUString sUrl = aAny.get<OUString>();
                    if (!sUrl.isEmpty())
                        xChildWPSProperties->setPropertyValue(UNO_NAME_HYPERLINK, aAny);
                }
                catch (const Exception&)
                {
                }
            }

            if( meFrameType == FRAMETYPE_DIAGRAM )
            {
                keepDiagramCompatibilityInfo();

                // set DiagramHelper at SdrObjGroup
                propagateDiagramHelper();

                // Check if this is the PPTX import, so far converting SmartArt to a non-editable
                // metafile is only implemented for DOCX.
                bool bPowerPoint = dynamic_cast<oox::ppt::PowerPointImport*>(&rFilterBase) != nullptr;

                if (!officecfg::Office::Common::Filter::Microsoft::Import::SmartArtToShapes::get() && !bPowerPoint)
                    convertSmartArtToMetafile( rFilterBase );
            }

            NamedShapePairs* pNamedShapePairs = rFilterBase.getDiagramFontHeights();
            if (xShape.is() && pNamedShapePairs)
            {
                auto itPairs = pNamedShapePairs->find(getInternalName());
                if (itPairs != pNamedShapePairs->end())
                {
                    auto it = itPairs->second.find(shared_from_this());
                    if (it != itPairs->second.end())
                    {
                        // Our drawingml::Shape is in the list of an internal name, remember the now
                        // inserted XShape.
                        it->second = xShape;
                    }
                }
            }
        }
    }
    catch( const Exception& )
    {
        TOOLS_WARN_EXCEPTION( "oox.drawingml", "Shape::addShape" );
    }
}

void Shape::setLockedCanvas(bool bLockedCanvas)
{
    mbLockedCanvas = bLockedCanvas;
}

void Shape::setWordprocessingCanvas(bool bWordprocessingCanvas)
{
    mbWordprocessingCanvas = bWordprocessingCanvas;
}

void Shape::setWPGChild(bool bWPG)
{
    mbWPGChild = bWPG;
}

void Shape::setWps(bool bWps)
{
    mbWps = bWps;
}

void Shape::setTextBox(bool bTextBox)
{
    mbTextBox = bTextBox;
}

void Shape::applyShapeReference( const Shape& rReferencedShape, bool bUseText )
{
    SAL_INFO("oox.drawingml", "Shape::applyShapeReference: apply '" << rReferencedShape.msId << "' to '" << msId << "'");

    if ( rReferencedShape.mpTextBody && bUseText )
        mpTextBody = std::make_shared<TextBody>( *rReferencedShape.mpTextBody );
    else
        mpTextBody.reset();
    maShapeProperties = rReferencedShape.maShapeProperties;
    mpShapeRefLinePropPtr = std::make_shared<LineProperties>( rReferencedShape.getActualLineProperties(nullptr) );
    mpShapeRefFillPropPtr = std::make_shared<FillProperties>( rReferencedShape.getActualFillProperties(nullptr, nullptr) );
    mpCustomShapePropertiesPtr = std::make_shared<CustomShapeProperties>( *rReferencedShape.mpCustomShapePropertiesPtr );
    mpTablePropertiesPtr = rReferencedShape.mpTablePropertiesPtr ? std::make_shared<table::TableProperties>( *rReferencedShape.mpTablePropertiesPtr ) : nullptr;
    mpShapeRefEffectPropPtr = std::make_shared<EffectProperties>( rReferencedShape.getActualEffectProperties(nullptr) );
    mpMasterTextListStyle = std::make_shared<TextListStyle>( *rReferencedShape.mpMasterTextListStyle );
    maSize = rReferencedShape.maSize;
    maPosition = rReferencedShape.maPosition;
    mnRotation = rReferencedShape.mnRotation;
    mbFlipH = rReferencedShape.mbFlipH;
    mbFlipV = rReferencedShape.mbFlipV;
    mbHidden = rReferencedShape.mbHidden;
    mbLocked = rReferencedShape.mbLocked;
}

namespace {

struct ActionLockGuard
{
    explicit ActionLockGuard(Reference<drawing::XShape> const& xShape)
        : m_xLockable(xShape, UNO_QUERY)
    {
        if (m_xLockable.is()) {
            m_xLockable->addActionLock();
        }
    }
    ~ActionLockGuard()
    {
        if (m_xLockable.is()) {
            m_xLockable->removeActionLock();
        }
    }
private:
    Reference<document::XActionLockable> m_xLockable;
};

}

// for group shapes, the following method is also adding each child
void Shape::addChildren(
        XmlFilterBase& rFilterBase,
        Shape& rMaster,
        const Theme* pTheme,
        const Reference< XShapes >& rxShapes,
        ShapeIdMap* pShapeMap,
        const basegfx::B2DHomMatrix& aTransformation )
{
    for (auto const& child : rMaster.maChildren)
    {
        child->setMasterTextListStyle( mpMasterTextListStyle );
        child->addShape( rFilterBase, pTheme, rxShapes, aTransformation, getFillProperties(), pShapeMap, rMaster.shared_from_this());
    }
}

static SdrTextHorzAdjust lcl_convertAdjust( ParagraphAdjust eAdjust )
{
    if (eAdjust == ParagraphAdjust_LEFT)
        return SDRTEXTHORZADJUST_LEFT;
    else if (eAdjust == ParagraphAdjust_RIGHT)
        return SDRTEXTHORZADJUST_RIGHT;
    else if (eAdjust == ParagraphAdjust_CENTER)
        return SDRTEXTHORZADJUST_CENTER;
    return SDRTEXTHORZADJUST_LEFT;
}

// LO does not interpret properties in styles belonging to the text content of a FontWork shape,
// but only those in the shape style. This method copies properties from the text content styles to
// the shape style.
static void lcl_copyCharPropsToShape(const uno::Reference<drawing::XShape>& xShape,
                                     const TextBodyPtr& pTextBody,
                                     const ::oox::core::XmlFilterBase& rFilter)
{
    if (!xShape.is() || !pTextBody)
        return;

    Reference<XPropertySet> xSet(xShape, UNO_QUERY);
    if (!xSet.is())
        return;

    // Content stretches or scales to given width and height, thus disable autogrow.
    xSet->setPropertyValue(UNO_NAME_TEXT_AUTOGROWHEIGHT, uno::Any(false));
    xSet->setPropertyValue(UNO_NAME_TEXT_AUTOGROWWIDTH, uno::Any(false));

    // LibreOffice is not able (as of Nov 2022) to use different styles for the paragraphs or
    // characters in FontWork, since that was not allowed in old binary WordArt. We use the
    // properties of the first non empty paragraph for now.
    const TextParagraphVector& rParagraphs = pTextBody->getParagraphs();
    auto aParaIt = std::find_if_not(rParagraphs.cbegin(), rParagraphs.cend(),
                                    [](const std::shared_ptr<TextParagraph> pParagraph) {
                                        return pParagraph->getRuns().empty();
                                    });
    if (aParaIt != rParagraphs.cend())
    {
        std::shared_ptr<TextParagraph> pParagraph = *aParaIt;
        const TextRunVector& rRuns = pParagraph->getRuns();
        auto aRunIt = std::find_if_not(
            rRuns.cbegin(), rRuns.cend(),
            [](const std::shared_ptr<TextRun> pRun) { return pRun->getText().isEmpty(); });
        if (aRunIt != rRuns.cend())
        {
            std::shared_ptr<TextRun> pRun = *aRunIt;
            TextCharacterProperties& rCharProps = pRun->getTextCharacterProperties();

            // set language
            if (rCharProps.moLang.has_value() && !rCharProps.moLang.value().isEmpty())
            {
                LanguageTag aTag(rCharProps.moLang.value());
                css::lang::Locale aLocale(aTag.getLocale(false));
                switch (MsLangId::getScriptType(aTag.getLanguageType()))
                {
                    case css::i18n::ScriptType::LATIN:
                        xSet->setPropertyValue(u"CharLocale"_ustr, uno::Any(aLocale));
                        break;
                    case css::i18n::ScriptType::ASIAN:
                        xSet->setPropertyValue(u"CharLocaleAsian"_ustr, uno::Any(aLocale));
                        break;
                    case css::i18n::ScriptType::COMPLEX:
                        xSet->setPropertyValue(u"CharLocaleComplex"_ustr, uno::Any(aLocale));
                        break;
                    default:;
                }
            }

            // Font Weight, Posture, Height
            if (rCharProps.moBold.has_value() && rCharProps.moBold.value())
            {
                xSet->setPropertyValue(UNO_NAME_CHAR_WEIGHT, uno::Any(css::awt::FontWeight::BOLD));
            }
            if (rCharProps.moItalic.has_value() && rCharProps.moItalic.value())
            {
                xSet->setPropertyValue(UNO_NAME_CHAR_POSTURE,
                                       uno::Any(css::awt::FontSlant::FontSlant_ITALIC));
            }
            if (rCharProps.moHeight.has_value())
            {
                sal_Int32 nHeight = rCharProps.moHeight.value() / 100;
                xSet->setPropertyValue(UNO_NAME_CHAR_HEIGHT, uno::Any(nHeight));
            }

            // Put theme fonts into shape properties
            OUString sFontName;
            sal_Int16 nFontPitch = 0;
            sal_Int16 nFontFamily = 0;
            bool bRet(false);
            if (const Theme* pTheme = rFilter.getCurrentTheme())
            {
                // minor Latin
                if (const TextFont* pFont = pTheme->resolveFont(u"+mn-lt"))
                {
                    bRet = pFont->getFontData(sFontName, nFontPitch, nFontFamily, nullptr, rFilter);
                    if (bRet)
                    {
                        xSet->setPropertyValue(u"CharFontName"_ustr, uno::Any(sFontName));
                        xSet->setPropertyValue(u"CharFontPitch"_ustr, uno::Any(nFontPitch));
                        xSet->setPropertyValue(u"CharFontFamily"_ustr, uno::Any(nFontFamily));
                    }
                }
                // minor Asian
                if (const TextFont* pFont = pTheme->resolveFont(u"+mn-ea"))
                {
                    bRet = pFont->getFontData(sFontName, nFontPitch, nFontFamily, nullptr, rFilter);
                    if (bRet)
                    {
                        xSet->setPropertyValue(u"CharFontNameAsian"_ustr, uno::Any(sFontName));
                        xSet->setPropertyValue(u"CharFontPitchAsian"_ustr, uno::Any(nFontPitch));
                        xSet->setPropertyValue(u"CharFontFamilyAsian"_ustr, uno::Any(nFontFamily));
                    }
                }
                // minor Complex
                if (const TextFont* pFont = pTheme->resolveFont(u"+mn-cs"))
                {
                    bRet = pFont->getFontData(sFontName, nFontPitch, nFontFamily, nullptr, rFilter);
                    if (bRet)
                    {
                        xSet->setPropertyValue(u"CharFontNameComplex"_ustr, uno::Any(sFontName));
                        xSet->setPropertyValue(u"CharFontPitchComplex"_ustr, uno::Any(nFontPitch));
                        xSet->setPropertyValue(u"CharFontFamilyComplex"_ustr, uno::Any(nFontFamily));
                    }
                }
            }

            // Replace theme fonts with formatting at run if any. ToDo: Inspect paragraph too?
            // Latin
            bRet = rCharProps.maLatinFont.getFontData(sFontName, nFontPitch, nFontFamily, nullptr, rFilter);
            if (!bRet)
                // In case there is no direct font, try to look it up as a theme reference.
                bRet = rCharProps.maLatinThemeFont.getFontData(sFontName, nFontPitch, nFontFamily, nullptr,
                                                               rFilter);

            if (bRet)
            {
                xSet->setPropertyValue(u"CharFontName"_ustr, uno::Any(sFontName));
                xSet->setPropertyValue(u"CharFontPitch"_ustr, uno::Any(nFontPitch));
                xSet->setPropertyValue(u"CharFontFamily"_ustr, uno::Any(nFontFamily));
            }
            // Asian
            bRet = rCharProps.maAsianFont.getFontData(sFontName, nFontPitch, nFontFamily, nullptr, rFilter);
            if (!bRet)
                // In case there is no direct font, try to look it up as a theme reference.
                bRet = rCharProps.maAsianThemeFont.getFontData(sFontName, nFontPitch, nFontFamily, nullptr,
                                                               rFilter);
            if (bRet)
            {
                xSet->setPropertyValue(u"CharFontNameAsian"_ustr, uno::Any(sFontName));
                xSet->setPropertyValue(u"CharFontPitchAsian"_ustr, uno::Any(nFontPitch));
                xSet->setPropertyValue(u"CharFontFamilyAsian"_ustr, uno::Any(nFontFamily));
            }
            // Complex
            bRet
                = rCharProps.maComplexFont.getFontData(sFontName, nFontPitch, nFontFamily, nullptr, rFilter);
            if (!bRet)
                // In case there is no direct font, try to look it up as a theme reference.
                bRet = rCharProps.maComplexThemeFont.getFontData(sFontName, nFontPitch, nFontFamily, nullptr,
                                                                 rFilter);
            if (bRet)
            {
                xSet->setPropertyValue(u"CharFontNameComplex"_ustr, uno::Any(sFontName));
                xSet->setPropertyValue(u"CharFontPitchComplex"_ustr, uno::Any(nFontPitch));
                xSet->setPropertyValue(u"CharFontFamilyComplex"_ustr, uno::Any(nFontFamily));
            }

            // LO uses shape properties, MS Office character properties. Copy them from char to shape.
            // Outline
            if (rCharProps.moTextOutlineProperties.has_value())
            {
                oox::drawingml::ShapePropertyMap aStrokeShapeProps(rFilter.getModelObjectHelper());
                rCharProps.moTextOutlineProperties.value().pushToPropMap(
                    aStrokeShapeProps, rFilter.getGraphicHelper());
                for (const auto& rProp : aStrokeShapeProps.makePropertyValueSequence())
                {
                    xSet->setPropertyValue(rProp.Name, rProp.Value);
                }
            }
            else
            {
                xSet->setPropertyValue(UNO_NAME_LINESTYLE, uno::Any(drawing::LineStyle_NONE));
            }

            // Fill
            // ToDo: Replace flip and rotate constants in parameters with actual values.
            // tdf#155327 If color is not explicitly set, MS Office uses scheme color 'tx1'.
            oox::drawingml::ShapePropertyMap aFillShapeProps(rFilter.getModelObjectHelper());
            if (!rCharProps.maFillProperties.moFillType.has_value())
                rCharProps.maFillProperties.moFillType = XML_solidFill;
            if (!rCharProps.maFillProperties.maFillColor.isUsed())
                rCharProps.maFillProperties.maFillColor.setSchemeClr(XML_tx1);
            rCharProps.maFillProperties.pushToPropMap(aFillShapeProps, rFilter.getGraphicHelper(),
                                                      /*nShapeRotation*/ 0,
                                                      /*nPhClr*/ API_RGB_TRANSPARENT,
                                                      /*aShapeSize*/ css::awt::Size(0, 0),
                                                      /*nPhClrTheme*/ -1,
                                                      /*bFlipH*/ false, /*bFlipV*/ false,
                                                      /*bIsCustomShape*/ true);
            for (const auto& rProp : aFillShapeProps.makePropertyValueSequence())
            {
                xSet->setPropertyValue(rProp.Name, rProp.Value);
            }

            // ToDo: Import WordArt glow and simple shadow effects. They are available in LO.
        }

        // LO does not evaluate paragraph alignment in text path mode. Use text area anchor instead.
        {
            ParagraphAdjust eAdjust = ParagraphAdjust_LEFT;
            if (pParagraph->getProperties().getParaAdjust())
                eAdjust = *pParagraph->getProperties().getParaAdjust();
            xSet->setPropertyValue("ParaAdjust", uno::Any(eAdjust));
            SdrObject* pShape = SdrObject::getSdrObjectFromXShape(xShape);
            assert(pShape);
            SdrTextHorzAdjust eHorzAdjust = lcl_convertAdjust(eAdjust);
            pShape->SetMergedItem(SdrTextHorzAdjustItem(eHorzAdjust));
        }
    }

    // Vertical adjustment is only meaningful for OOXML WordArt shapes of 'Follow Path' kinds. We set
    // it so, that text position is approximately same as in MS Office.
    const OUString sMSPresetType = pTextBody->getTextProperties().msPrst;
    const OUString sFontworkType = PresetGeometryTypeNames::GetFontworkType(sMSPresetType);
    SdrObject* pShape = SdrObject::getSdrObjectFromXShape(xShape);
    assert(pShape);
    if (sFontworkType == "fontwork-arch-up-curve" || sFontworkType == "fontwork-circle-curve")
        pShape->SetMergedItem(SdrTextVertAdjustItem(SdrTextVertAdjust::SDRTEXTVERTADJUST_BOTTOM));
    else if (sFontworkType == "fontwork-arch-down-curve")
        pShape->SetMergedItem(SdrTextVertAdjustItem(SdrTextVertAdjust::SDRTEXTVERTADJUST_TOP));
    else
        pShape->SetMergedItem(SdrTextVertAdjustItem(SdrTextVertAdjust::SDRTEXTVERTADJUST_CENTER));
}

// Some helper methods for createAndInsert
namespace
{
// mirrors aTransformation at its center axis
// only valid if neither rotation or shear included
void lcl_mirrorAtCenter(basegfx::B2DHomMatrix& aTransformation, bool bFlipH, bool bFlipV)
{
    if (!bFlipH && !bFlipV)
        return;
    basegfx::B2DPoint aCenter(0.5, 0.5);
    aCenter *= aTransformation;
    aTransformation.translate(-aCenter);
    aTransformation.scale(bFlipH ? -1.0 : 1.0, bFlipV ? -1.0 : 1.0);
    aTransformation.translate(aCenter);
    return;
}

// only valid if neither rotation or shear included
void lcl_doSpecialMSOWidthHeightToggle(basegfx::B2DHomMatrix& aTransformation)
{
    // The values are directly set at the matrix without any matrix multiplication.
    // That way it is valid for lines too. Those have zero width or height.
    const double fSx(aTransformation.get(0, 0));
    const double fSy(aTransformation.get(1, 1));
    const double fTx(aTransformation.get(0, 2));
    const double fTy(aTransformation.get(1, 2));
    aTransformation.set(0, 0, fSy);
    aTransformation.set(1, 1, fSx);
    aTransformation.set(0, 2, fTx + 0.5 * (fSx - fSy));
    aTransformation.set(1, 2, fTy + 0.5 * (fSy - fSx));
    return;
}

void lcl_RotateAtCenter(basegfx::B2DHomMatrix& aTransformation, sal_Int32 nMSORotationAngle)
{
    if (nMSORotationAngle == 0)
        return;
    double fRad = basegfx::deg2rad<60000>(nMSORotationAngle);
    basegfx::B2DPoint aCenter(0.5, 0.5);
    aCenter *= aTransformation;
    aTransformation.translate(-aCenter);
    aTransformation.rotate(fRad);
    aTransformation.translate(aCenter);
    return;
}

Degree100 lcl_MSORotateAngleToAPIAngle(const sal_Int32 nMSORotationAngle)
{
    // Converts a shape rotation angle from MSO to angle for API property RotateAngle
    // from unit 1/60000 deg to unit 1/100 deg
    Degree100 nAngle(nMSORotationAngle / 600);
    // API RotateAngle has opposite direction than nMSORotationAngle, thus 'minus'.
    return NormAngle36000(-nAngle);
}
}

Reference< XShape > const & Shape::createAndInsert(
        ::oox::core::XmlFilterBase& rFilterBase,
        const OUString& rServiceName,
        const Theme* pTheme,
        const css::uno::Reference< css::drawing::XShapes >& rxShapes,
        bool bClearText,
        bool bDoNotInsertEmptyTextBody,
        basegfx::B2DHomMatrix& aParentTransformation,
        const FillProperties& rShapeOrParentShapeFillProps,
        oox::drawingml::ShapePtr pParentGroupShape)
{
    bool bIsEmbMedia = false;
    SAL_INFO("oox.drawingml", "Shape::createAndInsert: id='" << msId << "' service='" << rServiceName << "'");

    formulaimport::XmlStreamBuilder * pMathXml(nullptr);
    if (mpTextBody)
    {
        for (auto const& it : mpTextBody->getParagraphs())
        {
            if (it->HasMathXml())
            {
                if (!mpTextBody->isEmpty() || pMathXml != nullptr)
                {
                    SAL_WARN("oox.drawingml", "losing a Math object...");
                }
                else
                {
                    pMathXml = &it->GetMathXml();
                }
            }
        }
    }

    // tdf#90403 PowerPoint ignores a:ext cx and cy values of p:xfrm, and uses real table width and height
    if ( mpTablePropertiesPtr && rServiceName == "com.sun.star.drawing.TableShape" )
    {
        maSize.Width = 0;
        for (auto const& elem : mpTablePropertiesPtr->getTableGrid())
        {
            maSize.Width = o3tl::saturating_add(maSize.Width, static_cast<sal_Int32>(elem));
        }
        maSize.Height = 0;
        for (auto const& elem : mpTablePropertiesPtr->getTableRows())
        {
            // WARN: If some rows can't fit the content, this is not the final height
            maSize.Height = o3tl::saturating_add(maSize.Height, elem.getHeight());
        }
    }

    awt::Rectangle aShapeRectHmm(
        o3tl::convert(maPosition.X, o3tl::Length::emu, o3tl::Length::mm100),
        o3tl::convert(maPosition.Y, o3tl::Length::emu, o3tl::Length::mm100),
        o3tl::convert(maSize.Width, o3tl::Length::emu, o3tl::Length::mm100),
        o3tl::convert(maSize.Height, o3tl::Length::emu, o3tl::Length::mm100));

    OUString aServiceName;
    if (pMathXml)
    {
        // convert this shape to OLE
        aServiceName = u"com.sun.star.drawing.OLE2Shape"_ustr;
        msServiceName = aServiceName;
        meFrameType = FRAMETYPE_GENERIC; // not OLEOBJECT, no stream in package
        mnSubType = 0;
    }
    else if (rServiceName == "com.sun.star.drawing.GraphicObjectShape" &&
        mpGraphicPropertiesPtr && !mpGraphicPropertiesPtr->m_sMediaPackageURL.isEmpty())
    {
        aServiceName = finalizeServiceName( rFilterBase, "com.sun.star.presentation.MediaShape", aShapeRectHmm );
        bIsEmbMedia = true;
    }
    else
    {
        aServiceName = finalizeServiceName( rFilterBase, rServiceName, aShapeRectHmm );
    }
    // Use custom shape instead of GraphicObjectShape if the image is cropped to
    // shape. Except rectangle, which does not require further cropping
    bool bIsCroppedGraphic = (aServiceName == "com.sun.star.drawing.GraphicObjectShape" &&
                              !mpCustomShapePropertiesPtr->representsDefaultShape());

    bool bIsCustomShape = (aServiceName == "com.sun.star.drawing.CustomShape" || bIsCroppedGraphic);
    bool bIsConnectorShape = (aServiceName == "com.sun.star.drawing.ConnectorShape");

    // Look for 3D. Its z-rotation and extrusion color become shape properties. Do it early as
    // graphics might use 3D too and in that case should be imported as custom shape as well.
    // FixMe. tdf#159515. We currently do not use extrusion, if an image has a 3D-scene.
    bool bBlockExtrusion = aServiceName == "com.sun.star.drawing.GraphicObjectShape" &&
                           mp3DPropertiesPtr->mnPreset.has_value();
    double fShapeRotateInclCamera = 0.0; // unit rad; same orientation as shape property RotateAngle
    Color aExtrusionColor;
    Scene3DHelper aScene3DHelper;
    bool bHas3DEffect = aScene3DHelper.setExtrusionProperties(
        mp3DPropertiesPtr, mnRotation, getCustomShapeProperties()->getExtrusionPropertyMap(),
        fShapeRotateInclCamera, aExtrusionColor, bBlockExtrusion);
    // Currently the other places use unit 1/60000deg and MSO shape rotate orientation.
    sal_Int32 nShapeRotateInclCamera = -basegfx::rad2deg<60000>(fShapeRotateInclCamera);
    bool bIs3DGraphic = aServiceName == "com.sun.star.drawing.GraphicObjectShape" && bHas3DEffect;
    bIsCustomShape |= bIs3DGraphic;

    // The extrusion color does not belong to the extrusion properties but is secondary color in
    // the style of the shape, FillColor2 in API. The case that no extrusion color was set is handled
    // further down when FillProperties and LineProperties are handled.
    if (aExtrusionColor.isUsed())
    {
        // FillColor2 is not yet transformed to ComplexColor.
        ::Color aColor = aExtrusionColor.getColor(rFilterBase.getGraphicHelper());
        maShapeProperties.setProperty(PROP_FillColor2, aColor);
    }

    if (bHas3DEffect)
        aScene3DHelper.setLightingProperties(mp3DPropertiesPtr, fShapeRotateInclCamera,
                                             getCustomShapeProperties()->getExtrusionPropertyMap());

    if (bIsCroppedGraphic || bIs3DGraphic)
    {
        aServiceName = "com.sun.star.drawing.CustomShape";
        mpGraphicPropertiesPtr->mbIsCustomShape = true;
        mpGraphicPropertiesPtr->mbIsExtruded = bIs3DGraphic;
    }
    bool bUseRotationTransform = ( !mbWps ||
            aServiceName == "com.sun.star.drawing.LineShape" ||
            aServiceName == "com.sun.star.drawing.GroupShape" ||
            mbFlipH ||
            mbFlipV );

    basegfx::B2DHomMatrix aTransformation; // will be cumulative transformation of this object

    // Special for SmartArt import. Rotate diagram's shape around object's center before sizing.
    if (bUseRotationTransform && mnDiagramRotation != 0)
    {
        aTransformation.translate(-0.5, -0.5);
        aTransformation.rotate(basegfx::deg2rad<60000>(mnDiagramRotation));
        aTransformation.translate(0.5, 0.5);
    }

    // Build object matrix from shape size and position; corresponds to MSO ext and off
    // Only LineShape and ConnectorShape may have zero width or height.
    if (aServiceName == "com.sun.star.drawing.LineShape"
        || aServiceName == "com.sun.star.drawing.ConnectorShape")
        aTransformation.scale(maSize.Width, maSize.Height);
    else
    {
        aTransformation.scale(maSize.Width ? maSize.Width : 1.0,
                              maSize.Height ? maSize.Height : 1.0);
    }

    // Evaluate object flip. Other shapes than custom shapes have no attribute for flip but use
    // negative scale. Flip in MSO is at object center.
    if (!bIsCustomShape && (mbFlipH || mbFlipV))
        lcl_mirrorAtCenter(aTransformation, mbFlipH, mbFlipV);

    // Evaluate parent flip.
    // A CustomShape has mirror not as negative scale, but as attributes.
    basegfx::B2DVector aParentScale(1.0, 1.0);
    basegfx::B2DVector aParentTranslate(0.0, 0.0);
    double fParentRotate(0.0);
    double fParentShearX(0.0);
    if (pParentGroupShape)
    {
        aParentTransformation.decompose(aParentScale, aParentTranslate, fParentRotate, fParentShearX);
        if (bIsCustomShape)
        {
            lcl_mirrorAtCenter(aTransformation, aParentScale.getX() < 0, aParentScale.getY() < 0);
            if(aParentScale.getX() < 0)
                mbFlipH = !mbFlipH;
            if(aParentScale.getY() < 0)
                mbFlipV = !mbFlipV;
        }
    }

    if (maPosition.X != 0 || maPosition.Y != 0)
    {
        // if global position is used, add it to transformation
        if (mbWps && pParentGroupShape == nullptr)
            aTransformation.translate(
                o3tl::convert(maPosition.X, o3tl::Length::mm100, o3tl::Length::emu),
                o3tl::convert(maPosition.Y, o3tl::Length::mm100, o3tl::Length::emu));
        else
            aTransformation.translate(maPosition.X, maPosition.Y);
    }

    // Apply further parent transformations. First scale object then rotate. Other way round would
    // introduce shearing.

    // The attributes chExt and chOff of the group in oox file contain the values on which the size
    // and position of the child is based on. If they differ from the actual size of the group as
    // given in its ext and off attributes, the child has to be transformed according the new values.
    if (pParentGroupShape)
    {
        // ToDo: A diagram in a group might need special handling because it cannot flip and only
        // resize uniformly. But currently it is imported with zero size, see tdf#139575. That needs
        // to be fixed beforehand.

        // Scaling is done from left/top edges of the group. So these need to become coordinate axes.
        aTransformation.translate(-pParentGroupShape->maChPosition.X,
                                  -pParentGroupShape->maChPosition.Y);

        // oox allows zero or missing attribute chExt. In that case the scaling factor is 1.
        // Transform2DContext::onCreateContext has set maChSize to maSize for groups in oox file in
        // such cases. For own made groups (e.g. diagrams) that is missing.
        // The factors cumulate on the way through the parent groups, so we do not use maSize of the
        // direct parent group but the cumulated value from aParentScale.
        double fFactorX = 1.0;
        double fFactorY = 1.0;
        if (pParentGroupShape->maChSize.Width != 0)
            fFactorX = aParentScale.getX() / pParentGroupShape->maChSize.Width;
        if (pParentGroupShape->maChSize.Height != 0)
            fFactorY = aParentScale.getY() / pParentGroupShape->maChSize.Height;
        if (fFactorX != 1 || fFactorY != 1)
        {
            // It depends on the object rotation angle whether scaling is applied to switched
            // width and height. MSO acts strange in that case (as of May 2021).
            const sal_Int32 nDeg(mnRotation / 60000);
            const bool bNeedsMSOWidthHeightToggle
                = (nDeg >= 45 && nDeg < 135) || (nDeg >= 225 && nDeg < 315);
            if (bNeedsMSOWidthHeightToggle)
                lcl_doSpecialMSOWidthHeightToggle(aTransformation);

            aTransformation.scale(fFactorX, fFactorY);

            if (bNeedsMSOWidthHeightToggle)
            {
                lcl_doSpecialMSOWidthHeightToggle(aTransformation);
                // In case of flip the special case needs an additional 180deg rotation.
                if ((aParentScale.getX() < 0) != (aParentScale.getY() < 0))
                    lcl_RotateAtCenter(aTransformation, 10800000);
            }
        }
    }

    // Apply object rotation at current object center
    // The flip contained in aParentScale will affect orientation of object rotation angle.
    sal_Int16 nOrientation = ((aParentScale.getX() < 0) != (aParentScale.getY() < 0)) ? -1 : 1;
    // ToDo: Not sure about the restrictions given by bUseRotationTransform.
    if (bUseRotationTransform && nShapeRotateInclCamera != 0)
    {
        lcl_RotateAtCenter(aTransformation, nOrientation * nShapeRotateInclCamera);
    }

    if (fParentRotate != 0.0)
        aTransformation.rotate(fParentRotate);
    if (!aParentTranslate.equalZero())
        aTransformation.translate(aParentTranslate);

    aParentTransformation = aTransformation;

    constexpr double fEmuToMm100 = o3tl::convert(1.0, o3tl::Length::emu, o3tl::Length::mm100);
    aTransformation.scale(fEmuToMm100, fEmuToMm100);

    // OOXML flips shapes before rotating them, so the rotation needs to be inverted
    if( bIsCustomShape && mbFlipH != mbFlipV )
    {
        basegfx::B2DVector aScale, aTranslate;
        double fRotate, fShearX;
        aTransformation.decompose(aScale, aTranslate, fRotate, fShearX);

        if(fRotate != 0)
        {
            basegfx::B2DPoint aCenter(0.5, 0.5);
            aCenter *= aTransformation;
            aTransformation.translate( -aCenter.getX(), -aCenter.getY() );
            aTransformation.rotate( fRotate * -2.0 );
            aTransformation.translate( aCenter.getX(), aCenter.getY() );
        }
    }

    // special for lineshape
    if ( aServiceName == "com.sun.star.drawing.LineShape" )
    {
        ::basegfx::B2DPolygon aPoly;
        aPoly.insert( 0, ::basegfx::B2DPoint( 0, 0 ) );
        aPoly.insert( 1, ::basegfx::B2DPoint( maSize.Width ? 1 : 0, maSize.Height ? 1 : 0 ) );
        aPoly.transform( aTransformation );

        // now creating the corresponding PolyPolygon
        sal_Int32 i, nNumPoints = aPoly.count();
        uno::Sequence< awt::Point > aPointSequence( nNumPoints );
        awt::Point* pPoints = aPointSequence.getArray();
        for( i = 0; i < nNumPoints; ++i )
        {
            basegfx::B2DPoint aPoint( aPoly.getB2DPoint( i ) );

            // Guard against zero width or height.
            if (i)
            {
                const basegfx::B2DPoint& rPreviousPoint = aPoly.getB2DPoint(i - 1);
                if (aPoint.getX() - rPreviousPoint.getX() == 0)
                    aPoint.setX(aPoint.getX() + 1);
                if (aPoint.getY() - rPreviousPoint.getY() == 0)
                    aPoint.setY(aPoint.getY() + 1);
            }

            pPoints[i] = awt::Point(static_cast<sal_Int32>(aPoint.getX()), static_cast<sal_Int32>(aPoint.getY()));
        }
        uno::Sequence< uno::Sequence< awt::Point > > aPolyPolySequence( 1 );
        aPolyPolySequence.getArray()[ 0 ] = aPointSequence;

        maShapeProperties.setProperty(PROP_PolyPolygon, aPolyPolySequence);
    }
    else if ( aServiceName == "com.sun.star.drawing.ConnectorShape" )
    {
        ::basegfx::B2DPolygon aPoly;
        aPoly.insert( 0, ::basegfx::B2DPoint( 0, 0 ) );
        aPoly.insert( 1, ::basegfx::B2DPoint( maSize.Width ? 1 : 0, maSize.Height ? 1 : 0 ) );
        aPoly.transform( aTransformation );

        basegfx::B2DPoint aStartPosition( aPoly.getB2DPoint( 0 ) );
        basegfx::B2DPoint aEndPosition( aPoly.getB2DPoint( 1 ) );
        awt::Point aAWTStartPosition( static_cast< sal_Int32 >( aStartPosition.getX() ), static_cast< sal_Int32 >( aStartPosition.getY() ) );
        awt::Point aAWTEndPosition( static_cast< sal_Int32 >( aEndPosition.getX() ), static_cast< sal_Int32 >( aEndPosition.getY() ) );

        maShapeProperties.setProperty(PROP_StartPosition, aAWTStartPosition);
        maShapeProperties.setProperty(PROP_EndPosition, aAWTEndPosition);
    }
    else
    {
        // now set transformation for this object
        HomogenMatrix3 aMatrix;

        aMatrix.Line1.Column1 = aTransformation.get(0,0);
        aMatrix.Line1.Column2 = aTransformation.get(0,1);
        aMatrix.Line1.Column3 = aTransformation.get(0,2);

        aMatrix.Line2.Column1 = aTransformation.get(1,0);
        aMatrix.Line2.Column2 = aTransformation.get(1,1);
        aMatrix.Line2.Column3 = aTransformation.get(1,2);

        aMatrix.Line3.Column1 = 0;
        aMatrix.Line3.Column2 = 0;
        aMatrix.Line3.Column3 = 1;

        maShapeProperties.setProperty(PROP_Transformation, aMatrix);
    }

    Reference< lang::XMultiServiceFactory > xServiceFact( rFilterBase.getModel(), UNO_QUERY_THROW );
    if ( !mxShape.is() )
    {
        mxShape.set( xServiceFact->createInstance( aServiceName ), UNO_QUERY_THROW );
    }

    Reference< XPropertySet > xSet( mxShape, UNO_QUERY );
    if (xSet.is())
    {
        if( !msName.isEmpty() )
        {
            Reference< container::XNamed > xNamed( mxShape, UNO_QUERY );
            if( xNamed.is() )
                xNamed->setName( msName );
        }
        if( !msDescription.isEmpty() )
        {
            xSet->setPropertyValue( "Description", Any( msDescription ) );
        }
        if (m_isDecorative)
        {
            xSet->setPropertyValue("Decorative", Any(m_isDecorative));
        }
        if (aServiceName != "com.sun.star.text.TextFrame")
            rxShapes->add( mxShape );

        if ( mbHidden || mbHiddenMasterShape )
        {
            SAL_INFO("oox.drawingml", "Shape::createAndInsert: invisible shape with id='" << msId << "'");
            xSet->setPropertyValue( "Visible", Any( false ) );
            // In Excel hidden means not printed, let's use visibility for now until that's handled separately
            xSet->setPropertyValue( "Printable", Any( false ) );
        }

        if (mbLocked)
        {
            xSet->setPropertyValue("MoveProtect", Any(true));
            xSet->setPropertyValue("SizeProtect", Any(true));
        }

        ActionLockGuard const alg(mxShape);

        // sj: removing default text of placeholder objects such as SlideNumberShape or HeaderShape
        if ( bClearText )
        {
            uno::Reference< text::XText > xText( mxShape, uno::UNO_QUERY );
            if ( xText.is() )
            {
                xText->setString( "" );
            }
        }

        if (pMathXml)
        {
            // the "EmbeddedObject" property is read-only, so we have to create
            // the shape first, and it can be read only after the shape is
            // inserted into the document, so delay the actual import until here
            SvGlobalName name(SO3_SM_CLASSID);
            xSet->setPropertyValue("CLSID", uno::Any(name.GetHexName()));
            uno::Reference<embed::XEmbeddedObject> const xObj(
                xSet->getPropertyValue("EmbeddedObject"), uno::UNO_QUERY);
            if (xObj.is())
            {
                uno::Reference<uno::XInterface> const xMathModel(xObj->getComponent());
                oox::FormulaImExportBase *const pMagic(
                        dynamic_cast<oox::FormulaImExportBase*>(xMathModel.get()));
                assert(pMagic);
                pMagic->readFormulaOoxml(*pMathXml);
            }
        }

        const GraphicHelper& rGraphicHelper = rFilterBase.getGraphicHelper();

        ::Color nLinePhClr(ColorTransparency, 0xffffffff);
        ::Color nFillPhClr(ColorTransparency, 0xffffffff);
        sal_Int16 nFillPhClrTheme = -1;
        sal_Int16 nLinePhClrTheme = -1;
        // TODO: use ph color when applying effect properties
        //sal_Int32 nEffectPhClr = -1;

        // dmapper needs the original rotation angle for calculating square wrap. This angle is not
        // available as property there, so store it in InteropGrabBag.
        putPropertyToGrabBag("mso-rotation-angle", Any(mnRotation));

        if( pTheme )
        {
            if( const ShapeStyleRef* pLineRef = getShapeStyleRef( XML_lnRef ) )
            {
                LineProperties aLineProperties;
                aLineProperties.maLineFill.moFillType = XML_noFill;
                if( const LineProperties* pLineProps = pTheme->getLineStyle( pLineRef->mnThemedIdx ) )
                    aLineProperties.assignUsed( *pLineProps );
                nLinePhClr = pLineRef->maPhClr.getColor( rGraphicHelper );
                nLinePhClrTheme = pLineRef->maPhClr.getSchemeColorIndex();

                // Store style-related properties to InteropGrabBag to be able to export them back
                uno::Sequence<beans::PropertyValue> aProperties = comphelper::InitPropertySequence(
                {
                    {"SchemeClr", uno::Any(pLineRef->maPhClr.getSchemeColorName())},
                    {"Idx", uno::Any(pLineRef->mnThemedIdx)},
                    {"Color", uno::Any(nLinePhClr)},
                    {"LineStyle", uno::Any(aLineProperties.getLineStyle())},
                    {"LineCap", uno::Any(aLineProperties.getLineCap())},
                    {"LineJoint", uno::Any(aLineProperties.getLineJoint())},
                    {"LineWidth", uno::Any(aLineProperties.getLineWidth())},
                    {"Transformations", uno::Any(pLineRef->maPhClr.getTransformations())}
                });
                putPropertyToGrabBag( "StyleLnRef", Any( aProperties ) );
            }
            if( const ShapeStyleRef* pFillRef = getShapeStyleRef( XML_fillRef ) )
            {
                if (!getFillProperties().moUseBgFill.value_or(false))
                {
                    nFillPhClr = pFillRef->maPhClr.getColor(rGraphicHelper);
                    nFillPhClrTheme = pFillRef->maPhClr.getSchemeColorIndex();
                }

                OUString sColorScheme = pFillRef->maPhClr.getSchemeColorName();
                if( !sColorScheme.isEmpty() )
                {
                    uno::Sequence<beans::PropertyValue> aProperties = comphelper::InitPropertySequence(
                    {
                        {"SchemeClr", uno::Any(sColorScheme)},
                        {"Idx", uno::Any(pFillRef->mnThemedIdx)},
                        {"Color", uno::Any(nFillPhClr)},
                        {"Transformations", uno::Any(pFillRef->maPhClr.getTransformations())}
                    });

                    putPropertyToGrabBag( "StyleFillRef", Any( aProperties ) );
                }
            }
            if( const ShapeStyleRef* pEffectRef = getShapeStyleRef( XML_effectRef ) )
            {
                // TODO: use ph color when applying effect properties
                // nEffectPhClr = pEffectRef->maPhClr.getColor( rGraphicHelper );

                // Store style-related properties to InteropGrabBag to be able to export them back
                uno::Sequence<beans::PropertyValue> aProperties = comphelper::InitPropertySequence(
                {
                    {"SchemeClr", uno::Any(pEffectRef->maPhClr.getSchemeColorName())},
                    {"Idx", uno::Any(pEffectRef->mnThemedIdx)},
                    {"Transformations", uno::Any(pEffectRef->maPhClr.getTransformations())}
                });
                putPropertyToGrabBag( "StyleEffectRef", Any( aProperties ) );
            }
        }
        ShapePropertyMap aShapeProps( rFilterBase.getModelObjectHelper() );

        // add properties from textbody to shape properties
        if( mpTextBody )
        {
            // tdf#67347: In case of Stacked, PP calculates in the vertical direction with the
            // horizontal alignment.
            // In LO, we simulate it by setting TextVerticalAdjust based on the ParagraphAdjust
            // of the 1. paragraph
            // It is not perfect, because we have 1 TextVerticalAdjust / 1 shape, and it
            // does not support justified, while we can have many ParagraphAdjust / 1 shape
            // (if the shape have more paragraphs)
            if (mpTextBody->getTextProperties().maPropertyMap.hasProperty(PROP_WritingMode)
                && mpTextBody->getTextProperties().maPropertyMap.getProperty(PROP_WritingMode)
                       == uno::Any(text::WritingMode2::STACKED)
                && mpTextBody->getParagraphs().size() > 0
                && aServiceName != "com.sun.star.drawing.GroupShape")
            {
                std::optional<css::style::ParagraphAdjust>& oParaAdjust
                    = mpTextBody->getParagraphs()[0]->getProperties().getParaAdjust();

                if (oParaAdjust)
                {
                    switch (*oParaAdjust)
                    {
                        case ParagraphAdjust::ParagraphAdjust_LEFT:
                            mpTextBody->getTextProperties().meVA
                                = TextVerticalAdjust::TextVerticalAdjust_TOP;
                            break;
                        case ParagraphAdjust::ParagraphAdjust_CENTER:
                            mpTextBody->getTextProperties().meVA
                                = TextVerticalAdjust::TextVerticalAdjust_CENTER;
                            break;
                        case ParagraphAdjust::ParagraphAdjust_RIGHT:
                            mpTextBody->getTextProperties().meVA
                                = TextVerticalAdjust::TextVerticalAdjust_BOTTOM;
                            break;
                        default:
                            break;
                    }
                    mpTextBody->getTextProperties().maPropertyMap.setProperty(
                        PROP_TextVerticalAdjust, mpTextBody->getTextProperties().meVA);
                }
            }

            mpTextBody->getTextProperties().pushTextDistances(Size(aShapeRectHmm.Width, aShapeRectHmm.Height));
            aShapeProps.assignUsed( mpTextBody->getTextProperties().maPropertyMap );
            // Push char properties as well - specifically useful when this is a placeholder
            if( mpMasterTextListStyle &&  mpMasterTextListStyle->getListStyle()[0].getTextCharacterProperties().moHeight.has_value() )
                aShapeProps.setProperty(PROP_CharHeight, GetFontHeight( mpMasterTextListStyle->getListStyle()[0].getTextCharacterProperties().moHeight.value() ));
        }

        // applying properties
        aShapeProps.assignUsed( getShapeProperties() );
        aShapeProps.assignUsed( maDefaultShapeProperties );
        if(nShapeRotateInclCamera != 0 && bIsCustomShape)
            aShapeProps.setProperty(PROP_RotateAngle,
                                    sal_Int32(lcl_MSORotateAngleToAPIAngle(nShapeRotateInclCamera)));
        if( bIsEmbMedia ||
            bIsCustomShape ||
            aServiceName == "com.sun.star.drawing.GraphicObjectShape" ||
            aServiceName == "com.sun.star.drawing.OLE2Shape")
        {
            mpGraphicPropertiesPtr->pushToPropMap( aShapeProps, rGraphicHelper, mbFlipH, mbFlipV );
        }
        if ( mpTablePropertiesPtr && aServiceName == "com.sun.star.drawing.TableShape" )
        {
            mpTablePropertiesPtr->pushToPropSet( rFilterBase, xSet, mpMasterTextListStyle );
            if ( auto* pTableShape = dynamic_cast<sdr::table::SdrTableObj*>(SdrObject::getSdrObjectFromXShape(mxShape)) )
            {
                // Disable layouting until table height is expanded to fit the content
                pTableShape->SetSkipChangeLayout(true);
            }
        }

        FillProperties aFillProperties = getActualFillProperties(pTheme, &rShapeOrParentShapeFillProps);
        if (getFillProperties().moFillType.has_value() && getFillProperties().moFillType.value() == XML_grpFill)
            getFillProperties().assignUsed(aFillProperties);
        if(!bIsCroppedGraphic && !bIs3DGraphic)
            aFillProperties.pushToPropMap(aShapeProps, rGraphicHelper, mnRotation, nFillPhClr,
                                          css::awt::Size(aShapeRectHmm.Width, aShapeRectHmm.Height),
                                          nFillPhClrTheme, mbFlipH, mbFlipV, bIsCustomShape);

        LineProperties aLineProperties = getActualLineProperties(pTheme);
        aLineProperties.pushToPropMap( aShapeProps, rGraphicHelper, nLinePhClr, nLinePhClrTheme);
        EffectProperties aEffectProperties = getActualEffectProperties(pTheme);
        // TODO: use ph color when applying effect properties
        aEffectProperties.pushToPropMap( aShapeProps, rGraphicHelper );

        // applying autogrowheight property before setting shape size, because
        // the shape size might be changed if currently autogrowheight is true
        // we must also check that the PropertySet supports the property.
        Reference< XPropertySetInfo > xSetInfo( xSet->getPropertySetInfo() );
        const OUString& rPropName = PropertyMap::getPropertyName( PROP_TextAutoGrowHeight );
        if( xSetInfo.is() && xSetInfo->hasPropertyByName( rPropName ) )
            if( aShapeProps.hasProperty( PROP_TextAutoGrowHeight ) )
                xSet->setPropertyValue( rPropName, Any( false ) );

        // For extruded shapes, MSO uses the line color if no extrusion color is specified. LO uses
        // fill color in 'automatic' case. Thus we set extrusion color explicitely.
        if (bHas3DEffect && !aExtrusionColor.isUsed())
        {
            const OUString& rFillColor2PropName = PropertyMap::getPropertyName(PROP_FillColor2);
            if (xSetInfo.is() && xSetInfo->hasPropertyByName(rFillColor2PropName))
            {
                Color aComplexColor;
                if (aLineProperties.maLineFill.moFillType.has_value()
                    && aLineProperties.maLineFill.moFillType.value() != XML_noFill)
                    aComplexColor = aLineProperties.maLineFill.getBestSolidColor();
                else if (aFillProperties.moFillType.has_value()
                    && aFillProperties.moFillType.value() != XML_noFill)
                    aComplexColor = aFillProperties.getBestSolidColor();
                if (aComplexColor.isUsed())
                {
                    const ::Color aSimpleColor = aComplexColor.getColor(rFilterBase.getGraphicHelper());
                    xSet->setPropertyValue(rFillColor2PropName, Any(aSimpleColor));
                }
            }
        }

        // do not set properties at a group shape (this causes
        // assertions from svx) ...
        if( aServiceName != "com.sun.star.drawing.GroupShape" )
        {
            if (aServiceName == "com.sun.star.text.TextFrame")
            {
                if (mpCustomShapePropertiesPtr && mpCustomShapePropertiesPtr->getShapeTypeOverride())
                {
                    uno::Reference<beans::XPropertySet> propertySet (mxShape, uno::UNO_QUERY);
                    uno::Sequence<beans::PropertyValue> aGrabBag;
                    propertySet->getPropertyValue("FrameInteropGrabBag") >>= aGrabBag;
                    sal_Int32 length = aGrabBag.getLength();
                    aGrabBag.realloc( length+1);
                    auto pGrabBag = aGrabBag.getArray();
                    pGrabBag[length].Name = "mso-orig-shape-type";
                    pGrabBag[length].Value <<= mpCustomShapePropertiesPtr->getShapePresetTypeName();
                    propertySet->setPropertyValue("FrameInteropGrabBag",uno::Any(aGrabBag));
                }
                //If the text box has links then save the link information so that
                //it can be accessed in DomainMapper_Impl.cxx while chaining the text frames.
                if (isLinkedTxbx())
                {
                    uno::Reference<beans::XPropertySet> propertySet (mxShape, uno::UNO_QUERY);
                    uno::Sequence<beans::PropertyValue> aGrabBag;
                    propertySet->getPropertyValue("FrameInteropGrabBag") >>= aGrabBag;
                    sal_Int32 length = aGrabBag.getLength();
                    aGrabBag.realloc( length + 3 );
                    auto pGrabBag = aGrabBag.getArray();
                    pGrabBag[length].Name = "TxbxHasLink";
                    pGrabBag[length].Value <<= isLinkedTxbx();
                    pGrabBag[length + 1 ].Name = "Txbx-Id";
                    pGrabBag[length + 1 ].Value <<= getLinkedTxbxAttributes().id;
                    pGrabBag[length + 2 ].Name = "Txbx-Seq";
                    pGrabBag[length + 2 ].Value <<= getLinkedTxbxAttributes().seq;
                    propertySet->setPropertyValue("FrameInteropGrabBag",uno::Any(aGrabBag));
                }

                // TextFrames have BackColor, not FillColor
                if (aShapeProps.hasProperty(PROP_FillColor))
                {
                    aShapeProps.setAnyProperty(PROP_BackColor, aShapeProps.getProperty(PROP_FillColor));
                    aShapeProps.erase(PROP_FillColor);
                }
                // TextFrames have BackColorTransparency, not FillTransparence
                if (aShapeProps.hasProperty(PROP_FillTransparence))
                {
                    aShapeProps.setAnyProperty(PROP_BackColorTransparency, aShapeProps.getProperty(PROP_FillTransparence));
                    aShapeProps.erase(PROP_FillTransparence);
                }
                // TextFrames have BackGraphic, not FillBitmap
                if (aShapeProps.hasProperty(PROP_FillBitmap))
                {
                    aShapeProps.setAnyProperty(PROP_BackGraphic, aShapeProps.getProperty(PROP_FillBitmap));
                    aShapeProps.erase(PROP_FillBitmap);
                }
                if (aShapeProps.hasProperty(PROP_FillBitmapName))
                {
                    uno::Any aAny = aShapeProps.getProperty(PROP_FillBitmapName);
                    OUString aFillBitmapName = aAny.get<OUString>();
                    uno::Reference<awt::XBitmap> xBitmap = rFilterBase.getModelObjectHelper().getFillBitmap(aFillBitmapName);
                    uno::Reference<graphic::XGraphic> xGraphic(xBitmap, uno::UNO_QUERY);
                    aShapeProps.setProperty(PROP_BackGraphic, xGraphic);
                    // aShapeProps.erase(PROP_FillBitmapName);  // Maybe, leave the name as well
                }
                // And no LineColor property; individual borders can have colors
                if (aShapeProps.hasProperty(PROP_LineColor))
                {
                    uno::Reference<beans::XPropertySet> xPropertySet(mxShape, uno::UNO_QUERY);
                    static const sal_Int32 aBorders[] =
                    {
                        PROP_TopBorder, PROP_LeftBorder, PROP_BottomBorder, PROP_RightBorder
                    };
                    for (sal_Int32 nBorder : aBorders)
                    {
                        css::table::BorderLine2 aBorderLine = xPropertySet->getPropertyValue(PropertyMap::getPropertyName(nBorder)).get<css::table::BorderLine2>();
                        aBorderLine.Color = aShapeProps.getProperty(PROP_LineColor).get<sal_Int32>();
                        if (aLineProperties.moLineWidth.has_value())
                            aBorderLine.LineWidth = convertEmuToHmm(aLineProperties.moLineWidth.value());
                        aShapeProps.setProperty(nBorder, aBorderLine);
                    }
                    aShapeProps.erase(PROP_LineColor);
                }
                if(mnRotation)
                {
                    uno::Reference<beans::XPropertySet> xPropertySet(mxShape, uno::UNO_QUERY);
                    static constexpr OUString aGrabBagPropName = u"FrameInteropGrabBag"_ustr;
                    uno::Sequence<beans::PropertyValue> aGrabBag;
                    xPropertySet->getPropertyValue(aGrabBagPropName) >>= aGrabBag;
                    beans::PropertyValue aPair(comphelper::makePropertyValue("mso-rotation-angle",
                                                                             mnRotation));
                    if (aGrabBag.hasElements())
                    {
                        sal_Int32 nLength = aGrabBag.getLength();
                        aGrabBag.realloc(nLength + 1);
                        aGrabBag.getArray()[nLength] = aPair;
                    }
                    else
                    {
                        aGrabBag = { aPair };
                    }
                    xPropertySet->setPropertyValue(aGrabBagPropName, uno::Any(aGrabBag));
                }
                // TextFrames have ShadowFormat, not individual shadow properties.
                std::optional<sal_Int32> oShadowDistance;
                if (aShapeProps.hasProperty(PROP_ShadowXDistance))
                {
                    oShadowDistance = aShapeProps.getProperty(PROP_ShadowXDistance).get<sal_Int32>();
                    aShapeProps.erase(PROP_ShadowXDistance);
                }
                if (aShapeProps.hasProperty(PROP_ShadowYDistance))
                {
                    // There is a single 'dist' attribute, so no need to count the avg of x and y.
                    aShapeProps.erase(PROP_ShadowYDistance);
                }
                std::optional<sal_Int32> oShadowColor;
                if (aShapeProps.hasProperty(PROP_ShadowColor))
                {
                    oShadowColor = aShapeProps.getProperty(PROP_ShadowColor).get<sal_Int32>();
                    aShapeProps.erase(PROP_ShadowColor);
                }
                if (aShapeProps.hasProperty(PROP_Shadow))
                    aShapeProps.erase(PROP_Shadow);

                if (oShadowDistance || oShadowColor || aEffectProperties.maShadow.moShadowDir.has_value())
                {
                    css::table::ShadowFormat aFormat;
                    if (oShadowColor)
                        aFormat.Color = *oShadowColor;
                    if (aEffectProperties.maShadow.moShadowDir.has_value())
                    {
                        css::table::ShadowLocation nLocation = css::table::ShadowLocation_NONE;
                        switch (aEffectProperties.maShadow.moShadowDir.value())
                        {
                        case 13500000:
                            nLocation = css::table::ShadowLocation_TOP_LEFT;
                            break;
                        case 18900000:
                            nLocation = css::table::ShadowLocation_TOP_RIGHT;
                            break;
                        case 8100000:
                            nLocation = css::table::ShadowLocation_BOTTOM_LEFT;
                            break;
                        case 2700000:
                            nLocation = css::table::ShadowLocation_BOTTOM_RIGHT;
                            break;
                        }
                        aFormat.Location = nLocation;
                    }
                    aFormat.ShadowWidth = *oShadowDistance;
                    aShapeProps.setProperty(PROP_ShadowFormat, aFormat);
                }

            }
            else if (mbTextBox)
            {
                // This introduces a TextBox in a shape in Writer. ToDo: Can we restrict it to cases
                // where the TextBox edit engine is really needed? tdf#82627
                aShapeProps.setProperty(PROP_TextBox, true);
            }

            if (aServiceName != "com.sun.star.text.TextFrame" && isLinkedTxbx())
            {
                uno::Reference<beans::XPropertySet> propertySet (mxShape, uno::UNO_QUERY);
                uno::Sequence<beans::PropertyValue> aGrabBag;
                propertySet->getPropertyValue("InteropGrabBag") >>= aGrabBag;
                sal_Int32 length = aGrabBag.getLength();
                aGrabBag.realloc( length + 3 );
                auto pGrabBag = aGrabBag.getArray();
                pGrabBag[length].Name = "TxbxHasLink";
                pGrabBag[length].Value <<= isLinkedTxbx();
                pGrabBag[length + 1 ].Name = "Txbx-Id";
                pGrabBag[length + 1 ].Value <<= getLinkedTxbxAttributes().id;
                pGrabBag[length + 2 ].Name = "Txbx-Seq";
                pGrabBag[length + 2 ].Value <<= getLinkedTxbxAttributes().seq;
                propertySet->setPropertyValue("InteropGrabBag",uno::Any(aGrabBag));
            }

            PropertySet( xSet ).setProperties( aShapeProps );

            if (mpTablePropertiesPtr && aServiceName == "com.sun.star.drawing.TableShape")
            {
                // Powerpoint exports desired row heights (i.e. what user attempted to set it as, not how it appears visually)
                // Expand table height if there are rows that can't fit the content
                if (auto* pTableShape = dynamic_cast<sdr::table::SdrTableObj*>(SdrObject::getSdrObjectFromXShape(mxShape)))
                {
                    tools::Rectangle aArea{};
                    pTableShape->LayoutTableHeight(aArea);
                    sal_Int32 nCorrectedHeight = aArea.GetHeight();
                    const auto& aShapeSize = mxShape->getSize();
                    if( nCorrectedHeight > aShapeSize.Height )
                        mxShape->setSize( {aShapeSize.Width, nCorrectedHeight} );
                    pTableShape->SetSkipChangeLayout(false);
                }
            }

            if (mbLockedCanvas)
            {
                putPropertyToGrabBag( "LockedCanvas", Any( true ) );
                if (aServiceName == "com.sun.star.drawing.LineShape")
                {
                    // It seems the position and size for lines inside a locked canvas is absolute.
                    mxShape->setPosition(awt::Point(aShapeRectHmm.X, aShapeRectHmm.Y));
                    mxShape->setSize(awt::Size(aShapeRectHmm.Width, aShapeRectHmm.Height));
                }
            }

            if (mbWordprocessingCanvas)
            {
                putPropertyToGrabBag("WordprocessingCanvas", Any(true));
            }

            // Store original fill and line colors of the shape and the theme color name to InteropGrabBag
            std::vector<beans::PropertyValue> aProperties
            {
                comphelper::makePropertyValue("EmuLineWidth", aLineProperties.moLineWidth.value_or(0)),
                comphelper::makePropertyValue("OriginalSolidFillClr", aShapeProps.getProperty(PROP_FillColor)),
                comphelper::makePropertyValue("OriginalLnSolidFillClr", aShapeProps.getProperty(PROP_LineColor))
            };
            OUString sColorFillScheme = aFillProperties.maFillColor.getSchemeColorName();
            if( !aFillProperties.maFillColor.isPlaceHolder() && !sColorFillScheme.isEmpty() )
            {
                aProperties.push_back(comphelper::makePropertyValue("SpPrSolidFillSchemeClr", sColorFillScheme));
                aProperties.push_back(comphelper::makePropertyValue("SpPrSolidFillSchemeClrTransformations", aFillProperties.maFillColor.getTransformations()));
            }
            OUString sLnColorFillScheme = aLineProperties.maLineFill.maFillColor.getSchemeColorName();
            if( !aLineProperties.maLineFill.maFillColor.isPlaceHolder() && !sLnColorFillScheme.isEmpty() )
            {
                aProperties.push_back(comphelper::makePropertyValue("SpPrLnSolidFillSchemeClr", sLnColorFillScheme));
                auto aResolvedSchemeClr = aLineProperties.maLineFill.maFillColor;
                aResolvedSchemeClr.clearTransformations();
                aProperties.push_back(comphelper::makePropertyValue("SpPrLnSolidFillResolvedSchemeClr", aResolvedSchemeClr.getColor(rGraphicHelper, nFillPhClr)));
                aProperties.push_back(comphelper::makePropertyValue("SpPrLnSolidFillSchemeClrTransformations", aLineProperties.maLineFill.maFillColor.getTransformations()));
            }
            putPropertiesToGrabBag(comphelper::containerToSequence(aProperties));

            // Store original gradient fill of the shape to InteropGrabBag
            // LibreOffice doesn't support all the kinds of gradient so we save its complete definition
            if( aShapeProps.hasProperty( PROP_FillGradient ) )
            {
                std::vector<beans::PropertyValue> aGradientStops;
                size_t i = 0;
                for( const auto& [rPos, rColor] : aFillProperties.maGradientProps.maGradientStops )
                { // for each stop in the gradient definition:

                    // save position
                    std::vector<beans::PropertyValue> aGradientStop
                    {
                        comphelper::makePropertyValue("Pos", rPos)
                    };

                    OUString sStopColorScheme = rColor.getSchemeColorName();
                    if( sStopColorScheme.isEmpty() )
                    {
                        // save RGB color
                        aGradientStop.push_back(comphelper::makePropertyValue("RgbClr", rColor.getColor(rGraphicHelper, nFillPhClr)));
                        // in the case of a RGB color, transformations are already applied to
                        // the color with the exception of alpha transformations. We only need
                        // to keep the transparency value to calculate the alpha value later.
                        if( rColor.hasTransparency() )
                            aGradientStop.push_back(comphelper::makePropertyValue("Transparency", rColor.getTransparency()));
                    }
                    else
                    {
                        // save color with scheme name
                        aGradientStop.push_back(comphelper::makePropertyValue("SchemeClr", sStopColorScheme));
                        // save all color transformations
                        aGradientStop.push_back(comphelper::makePropertyValue("Transformations", rColor.getTransformations()));
                    }

                    aGradientStops.push_back(comphelper::makePropertyValue(OUString::number(i), comphelper::containerToSequence(aGradientStop)));
                    ++i;
                }
                // If getFillProperties.moFillType is unused that means gradient is defined by a theme
                // which is already saved into StyleFillRef property, so no need to save the explicit values too
                if( getFillProperties().moFillType.has_value() )
                    putPropertyToGrabBag( "GradFillDefinition", uno::Any(comphelper::containerToSequence(aGradientStops)));
                putPropertyToGrabBag( "OriginalGradFill", aShapeProps.getProperty(PROP_FillGradient) );
            }

            // store unsupported effect attributes in the grab bag
            if (!aEffectProperties.m_Effects.empty())
            {
                std::vector<beans::PropertyValue> aEffects;
                for (auto const& it : aEffectProperties.m_Effects)
                {
                    PropertyValue aEffect = it->getEffect();
                    if( !aEffect.Name.isEmpty() )
                    {
                        std::vector<beans::PropertyValue> aEffectsGrabBag
                        {
                            comphelper::makePropertyValue("Attribs", aEffect.Value)
                        };

                        Color& aColor( it->moColor );
                        OUString sColorScheme = aColor.getSchemeColorName();
                        if( sColorScheme.isEmpty() )
                        {
                            // RGB color and transparency value
                            aEffectsGrabBag.push_back(comphelper::makePropertyValue("RgbClr", aColor.getColor(rGraphicHelper, nFillPhClr)));
                            aEffectsGrabBag.push_back(comphelper::makePropertyValue("RgbClrTransparency", aColor.getTransparency()));
                        }
                        else
                        {
                            // scheme color with name and transformations
                            aEffectsGrabBag.push_back(comphelper::makePropertyValue("SchemeClr", sColorScheme));
                            aEffectsGrabBag.push_back(comphelper::makePropertyValue("SchemeClrTransformations", aColor.getTransformations()));
                        }
                        aEffects.push_back(comphelper::makePropertyValue(aEffect.Name, comphelper::containerToSequence(aEffectsGrabBag)));
                    }
                }
                putPropertyToGrabBag("EffectProperties", uno::Any(comphelper::containerToSequence(aEffects)));
            }

            // add 3D effects if any to GrabBag. They are still used in export.
            Sequence< PropertyValue > aCamera3DEffects = get3DProperties().getCameraAttributes();
            Sequence< PropertyValue > aLightRig3DEffects = get3DProperties().getLightRigAttributes();
            Sequence< PropertyValue > aShape3DEffects = get3DProperties().getShape3DAttributes( rGraphicHelper, nFillPhClr );
            if( aCamera3DEffects.hasElements() || aLightRig3DEffects.hasElements() || aShape3DEffects.hasElements() )
            {
                uno::Sequence<beans::PropertyValue> a3DEffectsGrabBag = comphelper::InitPropertySequence(
                {
                    {"Camera", uno::Any(aCamera3DEffects)},
                    {"LightRig", uno::Any(aLightRig3DEffects)},
                    {"Shape3D", uno::Any(aShape3DEffects)}
                });
                putPropertyToGrabBag( "3DEffectProperties", Any( a3DEffectsGrabBag ) );
            }

            if( bIsCustomShape && getTextBody())
            {

                Sequence< PropertyValue > aTextCamera3DEffects = getTextBody()->get3DProperties().getCameraAttributes();
                Sequence< PropertyValue > aTextLightRig3DEffects = getTextBody()->get3DProperties().getLightRigAttributes();
                Sequence< PropertyValue > aTextShape3DEffects = getTextBody()->get3DProperties().getShape3DAttributes( rGraphicHelper, nFillPhClr );
                if( aTextCamera3DEffects.hasElements() || aTextLightRig3DEffects.hasElements() || aTextShape3DEffects.hasElements() )
                {
                    uno::Sequence<beans::PropertyValue> aText3DEffectsGrabBag = comphelper::InitPropertySequence(
                    {
                        {"Camera", uno::Any(aTextCamera3DEffects)},
                        {"LightRig", uno::Any(aTextLightRig3DEffects)},
                        {"Shape3D", uno::Any(aTextShape3DEffects)}
                    });
                    putPropertyToGrabBag( "Text3DEffectProperties", Any( aText3DEffectsGrabBag ) );
                }
            }

            // store bitmap artistic effects in the grab bag
            if( !mpGraphicPropertiesPtr->maBlipProps.maEffect.isEmpty() )
                putPropertyToGrabBag( "ArtisticEffectProperties",
                                      Any( mpGraphicPropertiesPtr->maBlipProps.maEffect.getEffect() ) );
        }

        else if( mbLockedCanvas )
        {
            //If we have aServiceName as "com.sun.star.drawing.GroupShape" and lockedCanvas
            putPropertyToGrabBag( "LockedCanvas", Any( true ) );
        }
        else if (mbWordprocessingCanvas)
        {
            putPropertyToGrabBag("WordprocessingCanvas", Any(true));
            putPropertyToGrabBag("mso-edit-as", Any(OUString("canvas"))); // for export VML Fallback
        }

        // These can have a custom geometry, so position should be set here,
        // after creation but before custom shape handling, using the position
        // we got from the caller.
        if (mbWps && aServiceName == "com.sun.star.drawing.LineShape" && !pParentGroupShape)
            mxShape->setPosition(maPosition);

        if (bIsConnectorShape)
        {
            msConnectorName = mpCustomShapePropertiesPtr->getShapePresetTypeName();

            auto aAdjustmentList = mpCustomShapePropertiesPtr->getAdjustmentGuideList();
            for (size_t i = 0; i < aAdjustmentList.size(); i++)
                maConnectorAdjustmentList.push_back(aAdjustmentList[i].maFormula);

            sal_Int32 nType = mpCustomShapePropertiesPtr->getShapePresetType();
            switch (nType)
            {
            case XML_line:
            case XML_straightConnector1:
                xSet->setPropertyValue("EdgeKind", Any(ConnectorType_LINE));
                break;
            case XML_bentConnector2:
            case XML_bentConnector3:
            case XML_bentConnector4:
            case XML_bentConnector5:
                xSet->setPropertyValue("EdgeKind", Any(ConnectorType_STANDARD));
                break;
            case XML_curvedConnector2:
            case XML_curvedConnector3:
            case XML_curvedConnector4:
            case XML_curvedConnector5:
                xSet->setPropertyValue("EdgeKind", Any(ConnectorType_CURVE));
                break;
            default:
                break;
            }
        }

        if( bIsCustomShape )
        {
            if ( mbFlipH )
                mpCustomShapePropertiesPtr->setMirroredX( true );
            if ( mbFlipV )
                mpCustomShapePropertiesPtr->setMirroredY( true );
            if( getTextBody() )
            {
                sal_Int32 nTextCameraZRotation = getTextBody()->get3DProperties().maCameraRotation.mnRevolution.value_or(0);
                mpCustomShapePropertiesPtr->setTextCameraZRotateAngle( nTextCameraZRotation / 60000 );

                // TextPreRotateAngle. Text rotates inside the text area. Might be used for diagram layout 'upr' and 'grav'.
                sal_Int32 nTextPreRotateAngle = static_cast< sal_Int32 >( getTextBody()->getTextProperties().moTextPreRotation.value_or( 0 ) );

                nTextPreRotateAngle -= mnDiagramRotation; // Use of mnDiagramRotation is unclear. It seems to be always 0 here.

                // TextRotateAngle. The text area rotates.
                sal_Int32 nTextAreaRotateAngle = getTextBody()->getTextProperties().moTextAreaRotation.value_or(0);
                if (getTextBody()->getTextProperties().moUpright)
                {
                    // When upright is set, any text area transformation and shape rotation is ignored
                    // in MS Office. To simulate this behaviour, we rotate the text area into the
                    // opposite direction of the shape rotation by as much as the shape was rotated
                    // and so compensate the shape rotation, which is added in rendering.
                    nTextAreaRotateAngle = -mnRotation;
                    // If 45° <= shape rotation < 135° or 225° <= shape rotation < 315°,
                    // then MS Office adds an additional 90° rotation to the text area.
                    const sal_Int32 nDeg(mnRotation / 60000);
                    if ((nDeg >= 45 && nDeg < 135) || (nDeg >= 225 && nDeg < 315))
                    {
                        nTextAreaRotateAngle += 5400000;
                        nTextPreRotateAngle -= 5400000; // compensate the additional text area rotation
                    }
                    putPropertyToGrabBag("Upright", Any(true));
                }
                /* OOX measures text rotation clockwise in 1/60000th degrees,
                   relative to the containing shape. set*Angle wants degrees counter-clockwise. */
                mpCustomShapePropertiesPtr->setTextPreRotateAngle(-nTextPreRotateAngle / 60000);
                if (nTextAreaRotateAngle != 0)
                    mpCustomShapePropertiesPtr->setTextAreaRotateAngle(-nTextAreaRotateAngle / 60000);

                auto sHorzOverflow = getTextBody()->getTextProperties().msHorzOverflow;
                if (!sHorzOverflow.isEmpty())
                    putPropertyToGrabBag("horzOverflow", uno::Any(getTextBody()->getTextProperties().msHorzOverflow));
                if (XML_ellipsis == getTextBody()->getTextProperties().moVertOverflow)
                    putPropertyToGrabBag("vertOverflow", uno::Any(OUString{"ellipsis"}));
            }

            // Note that the script oox/source/drawingml/customshapes/generatePresetsData.pl looks
            // for these ==cscode== and ==csdata== markers, so don't "clean up" these SAL_INFOs
            SAL_INFO("oox.cscode", "==cscode== shape name: '" << msName << "'");
            SAL_INFO("oox.csdata", "==csdata== shape name: '" << msName << "'");

            mpCustomShapePropertiesPtr->pushToPropSet(xSet, maSize);

            // Consider WordArt
            if (mpTextBody && !mpTextBody->getTextProperties().msPrst.isEmpty()
                && mpTextBody->getTextProperties().msPrst != u"textNoShape")
            {
                bool bFromWordArt(aShapeProps.hasProperty(PROP_FromWordArt)
                                      ? aShapeProps.getProperty(PROP_FromWordArt).get<bool>()
                                      : false);
                FontworkHelpers::putCustomShapeIntoTextPathMode(
                    mxShape, mpCustomShapePropertiesPtr, mpTextBody->getTextProperties().msPrst,
                    bFromWordArt);
            }
        }
        else if( getTextBody() )
            getTextBody()->getTextProperties().pushVertSimulation();

        // tdf#133037: a bit hackish: force Shape to rotate in the opposite direction the camera would rotate
        PropertySet aPropertySet(mxShape);
        if ( !bUseRotationTransform && (nShapeRotateInclCamera !=0) )
        {
            Degree100 nAngle(lcl_MSORotateAngleToAPIAngle(nShapeRotateInclCamera));
            aPropertySet.setAnyProperty(PROP_RotateAngle, Any( sal_Int32(nAngle)));
            aPropertySet.setAnyProperty( PROP_HoriOrientPosition, Any( maPosition.X ) );
            aPropertySet.setAnyProperty( PROP_VertOrientPosition, Any( maPosition.Y ) );
        }

        // Make sure to not set text to placeholders. Doing it here would eventually call
        // SvxTextEditSourceImpl::UpdateData, SdrObject::SetEmptyPresObj(false), and that
        // would make the object behave like a standard outline object.
        // TODO/FIXME: support custom prompt text in placeholders.
        if (rServiceName == "com.sun.star.presentation.GraphicObjectShape")
            mpTextBody.reset();

        // in some cases, we don't have any text body.
        if( mpTextBody && ( !bDoNotInsertEmptyTextBody || !mpTextBody->isEmpty() ) )
        {
            Reference < XText > xText( mxShape, UNO_QUERY );
            if ( xText.is() )   // not every shape is supporting an XText interface (e.g. GroupShape)
            {
                TextCharacterProperties aCharStyleProperties;
                if( const ShapeStyleRef* pFontRef = getShapeStyleRef( XML_fontRef ) )
                {
                    if( pFontRef->mnThemedIdx != 0 )
                    {
                        if( pTheme )
                            if( const TextCharacterProperties* pCharProps = pTheme->getFontStyle( pFontRef->mnThemedIdx ) )
                                aCharStyleProperties.assignUsed( *pCharProps );
                        SAL_INFO("oox.drawingml", "Shape::createAndInsert: use font color");
                        if ( pFontRef->maPhClr.isUsed() )
                        {
                            aCharStyleProperties.maFillProperties.maFillColor = pFontRef->maPhClr;
                            aCharStyleProperties.maFillProperties.moFillType = XML_solidFill;
                        }
                    }
                }
                xText->setString("");
                Reference < XTextCursor > xAt = xText->createTextCursor();
                getTextBody()->insertAt( rFilterBase, xText, xAt, aCharStyleProperties, mpMasterTextListStyle );

                const TextParagraphVector& rParagraphs = getTextBody()->getParagraphs();
                if (!rParagraphs.empty())
                {
                    const std::shared_ptr<TextParagraph>& pParagraph = rParagraphs[0];
                    if (pParagraph->getProperties().getParaAdjust())
                    {
                        style::ParagraphAdjust eAdjust = *pParagraph->getProperties().getParaAdjust();
                        if (eAdjust == style::ParagraphAdjust_CENTER)
                        {
                            // If the first paragraph is centered, then set the para adjustment of
                            // the shape itself to centered as well.
                            aPropertySet.setAnyProperty(PROP_ParaAdjust, uno::Any(eAdjust));
                        }
                    }

                    // tdf#144092 For empty textboxes push character styles &
                    // endParaRPr into the Shape's properties
                    if (rParagraphs.size() == 1 && pParagraph->getRuns().empty())
                    {
                        TextCharacterProperties aTextCharacterProps{ pParagraph->getCharacterStyle(
                            aCharStyleProperties, *mpMasterTextListStyle,
                            getTextBody()->getTextListStyle()) };
                        aTextCharacterProps.assignUsed(pParagraph->getEndProperties());
                        aTextCharacterProps.pushToPropSet(aPropertySet, rFilterBase);
                    }
                }

                // MS Office has e.g. fill and stroke of WordArt in the character properties,
                // LibreOffice uses shape properties.
                if (!mpTextBody->getTextProperties().msPrst.isEmpty()
                    && mpTextBody->getTextProperties().msPrst != u"textNoShape")
                {
                    lcl_copyCharPropsToShape(mxShape, mpTextBody, rFilterBase);
                }
            }
        }
        else if (mbTextBox)
        {
            // No drawingML text, but WPS text is expected: save the theme
            // character color on the shape, then.
            if(const ShapeStyleRef* pFontRef = getShapeStyleRef(XML_fontRef))
            {
                ::Color nCharColor = pFontRef->maPhClr.getColor(rGraphicHelper);
                aPropertySet.setAnyProperty(PROP_CharColor, uno::Any(nCharColor));
            }
        }

        // Set glow effect properties
        if (aEffectProperties.maGlow.moGlowRad.has_value()
            && aServiceName != "com.sun.star.drawing.GroupShape")
        {
            uno::Reference<beans::XPropertySet> propertySet (mxShape, uno::UNO_QUERY);
            propertySet->setPropertyValue("GlowEffectRadius", Any(convertEmuToHmm(aEffectProperties.maGlow.moGlowRad.value())));
            propertySet->setPropertyValue("GlowEffectColor", Any(aEffectProperties.maGlow.moGlowColor.getColor(rGraphicHelper)));
            propertySet->setPropertyValue("GlowEffectTransparency", Any(aEffectProperties.maGlow.moGlowColor.getTransparency()));
        }

        // Set soft edge effect properties
        if (aEffectProperties.maSoftEdge.moRad.has_value())
        {
            uno::Reference<beans::XPropertySet> propertySet(mxShape, uno::UNO_QUERY);
            propertySet->setPropertyValue(
                "SoftEdgeRadius", Any(convertEmuToHmm(aEffectProperties.maSoftEdge.moRad.value())));
        }

        // Set the stroke and fill-color properties of the OLE shape
        if (aServiceName == "com.sun.star.drawing.OLE2Shape" && mxOleObjectInfo
            && !mxOleObjectInfo->maShapeId.isEmpty())
            if (::oox::vml::Drawing* pVmlDrawing = rFilterBase.getVmlDrawing())
                if (const ::oox::vml::ShapeBase* pVmlShape
                    = pVmlDrawing->getShapes().getShapeById(mxOleObjectInfo->maShapeId))
                {
                    // Apply stroke props from the type model of the related VML shape.
                    ShapePropertyMap aPropMap(rFilterBase.getModelObjectHelper());
                    pVmlShape->getTypeModel().maStrokeModel.pushToPropMap(
                        aPropMap, rFilterBase.getGraphicHelper());
                    // And, fill-color properties as well...
                    pVmlShape->getTypeModel().maFillModel.pushToPropMap(
                        aPropMap, rFilterBase.getGraphicHelper());
                    PropertySet(xSet).setProperties(aPropMap);
                }
    }

    if (mxShape.is())
    {
        finalizeXShape( rFilterBase, rxShapes );

        if (mpTextBody)
        {
            // tdf#151518. The method readjustTextDistances is fix for tdf#148321, but conflicts with
            // text position in some of the SmartArt types in Writer. So exclude Writer here.
            OUString sDocumentService;
            rFilterBase.getMediaDescriptor()[utl::MediaDescriptor::PROP_DOCUMENTSERVICE] >>= sDocumentService;
            if (sDocumentService != u"com.sun.star.text.TextDocument")
                mpTextBody->getTextProperties().readjustTextDistances(mxShape);
        }
    }
    return mxShape;
}

void Shape::keepDiagramDrawing(XmlFilterBase& rFilterBase, const OUString& rFragmentPath)
{

    sal_Int32 length = maDiagramDoms.getLength();
    maDiagramDoms.realloc(length + 1);

    // drawingValue[0] => dom, drawingValue[1] => Sequence of associated relationships
    uno::Sequence<uno::Any> diagramDrawing{
        uno::Any(rFilterBase.importFragment(rFragmentPath)),
        uno::Any(resolveRelationshipsOfTypeFromOfficeDoc(rFilterBase, rFragmentPath, u"image"))
    };

    beans::PropertyValue* pValue = maDiagramDoms.getArray();
    pValue[length].Name = "OOXDrawing";
    pValue[length].Value <<= diagramDrawing;
}

void Shape::keepDiagramCompatibilityInfo()
{
    try
    {
        if( !maDiagramDoms.hasElements() )
            return;

        Reference < XPropertySet > xSet( mxShape, UNO_QUERY_THROW );
        Reference < XPropertySetInfo > xSetInfo( xSet->getPropertySetInfo() );
        if ( !xSetInfo.is() )
            return;

        const OUString aGrabBagPropName = UNO_NAME_MISC_OBJ_INTEROPGRABBAG;
        if( !xSetInfo->hasPropertyByName( aGrabBagPropName ) )
            return;

        Sequence < PropertyValue > aGrabBag;
        xSet->getPropertyValue( aGrabBagPropName ) >>= aGrabBag;

        // We keep the previous items, if present
        if ( aGrabBag.hasElements() )
            xSet->setPropertyValue( aGrabBagPropName, Any( comphelper::concatSequences(aGrabBag, maDiagramDoms) ) );
        else
            xSet->setPropertyValue( aGrabBagPropName, Any( maDiagramDoms ) );
    }
    catch( const Exception& )
    {
        TOOLS_WARN_EXCEPTION( "oox.drawingml", "Shape::keepDiagramCompatibilityInfo" );
    }
}

void Shape::convertSmartArtToMetafile(XmlFilterBase const & rFilterBase)
{
    try
    {
        Reference<XPropertySet> xSet(mxShape, UNO_QUERY_THROW);

        xSet->setPropertyValue("MoveProtect", Any(true));
        xSet->setPropertyValue("SizeProtect", Any(true));

        // Replace existing shapes with a new Graphic Object rendered
        // from them
        Reference<XShape> xShape(renderDiagramToGraphic(rFilterBase));
        Reference<XShapes> xShapes(mxShape, UNO_QUERY_THROW);
        tools::Rectangle aBackgroundRect
            = SdrObject::getSdrObjectFromXShape(
                  Reference<XShape>(xShapes->getByIndex(0), UNO_QUERY_THROW))
                  ->GetLogicRect();
        while (xShapes->hasElements())
            xShapes->remove(Reference<XShape>(xShapes->getByIndex(0), UNO_QUERY_THROW));
        xShapes->add(xShape);
        SdrObject::getSdrObjectFromXShape(
            Reference<XShape>(xShapes->getByIndex(0), UNO_QUERY_THROW))
            ->NbcSetLogicRect(aBackgroundRect);
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION("oox.drawingml", "Shape::convertSmartArtToMetafile");
    }
}

Reference < XShape > Shape::renderDiagramToGraphic( XmlFilterBase const & rFilterBase )
{
    Reference< XShape > xShape;

    try
    {
        if( !maDiagramDoms.hasElements() )
            return xShape;

        // Stream in which to place the rendered shape
        SvMemoryStream aTempStream;
        Reference < io::XStream > xStream( new utl::OStreamWrapper( aTempStream ) );
        Reference < io::XOutputStream > xOutputStream( xStream->getOutputStream() );

        // Size of the rendering
        awt::Size aActualSize = mxShape->getSize();
        Size aResolution(Application::GetDefaultDevice()->LogicToPixel(Size(100, 100), MapMode(MapUnit::MapCM)));
        double fPixelsPer100thmm = static_cast < double > ( aResolution.Width() ) / 100000.0;
        awt::Size aSize( static_cast < sal_Int32 > ( ( fPixelsPer100thmm * aActualSize.Width ) + 0.5 ),
                         static_cast < sal_Int32 > ( ( fPixelsPer100thmm * aActualSize.Height ) + 0.5 ) );

        Sequence< PropertyValue > aFilterData{
            comphelper::makePropertyValue("PixelWidth", aSize.Width),
            comphelper::makePropertyValue("PixelHeight", aSize.Height),
            comphelper::makePropertyValue("LogicalWidth", aActualSize.Width),
            comphelper::makePropertyValue("LogicalHeight", aActualSize.Height)
        };

        Sequence < PropertyValue > aDescriptor{
            comphelper::makePropertyValue("OutputStream", xOutputStream),
            comphelper::makePropertyValue("FilterName", OUString("SVM")), // Rendering format
            comphelper::makePropertyValue("FilterData", aFilterData)
        };

        Reference < lang::XComponent > xSourceDoc( mxShape, UNO_QUERY_THROW );
        Reference < XGraphicExportFilter > xGraphicExporter = GraphicExportFilter::create( rFilterBase.getComponentContext() );
        xGraphicExporter->setSourceDocument( xSourceDoc );
        xGraphicExporter->filter( aDescriptor );

        aTempStream.Seek( STREAM_SEEK_TO_BEGIN );

        Graphic aGraphic;
        GraphicFilter aFilter( false );
        if ( aFilter.ImportGraphic( aGraphic, u"", aTempStream, GRFILTER_FORMAT_NOTFOUND, nullptr, GraphicFilterImportFlags::NONE ) != ERRCODE_NONE )
        {
            SAL_WARN( "oox.drawingml", "Shape::renderDiagramToGraphic: Unable to import rendered stream into graphic object" );
            return xShape;
        }

        Reference < graphic::XGraphic > xGraphic( aGraphic.GetXGraphic() );
        Reference < lang::XMultiServiceFactory > xServiceFact( rFilterBase.getModel(), UNO_QUERY_THROW );
        xShape.set( xServiceFact->createInstance( "com.sun.star.drawing.GraphicObjectShape" ), UNO_QUERY_THROW );
        Reference < XPropertySet > xPropSet( xShape, UNO_QUERY_THROW );
        xPropSet->setPropertyValue(  "Graphic", Any( xGraphic ) );
        xPropSet->setPropertyValue(  "MoveProtect", Any( true ) );
        xPropSet->setPropertyValue(  "SizeProtect", Any( true ) );
        xPropSet->setPropertyValue(  "Name", Any( OUString( "RenderedShapes" ) ) );
    }
    catch( const Exception& )
    {
        TOOLS_WARN_EXCEPTION( "oox.drawingml", "Shape::renderDiagramToGraphic" );
    }

    return xShape;
}

void Shape::setTextBody(const TextBodyPtr & pTextBody)
{
    mpTextBody = pTextBody;
}

void Shape::setMasterTextListStyle( const TextListStylePtr& pMasterTextListStyle )
{
    SAL_INFO("oox.drawingml", "Shape::setMasterTextListStyle: Set master text list style to shape id='" << msId << "'");

    mpMasterTextListStyle = pMasterTextListStyle;
}

OUString Shape::finalizeServiceName( XmlFilterBase& rFilter, const OUString& rServiceName, const awt::Rectangle& rShapeRect )
{
    OUString aServiceName = rServiceName;
    switch( meFrameType )
    {
        case FRAMETYPE_OLEOBJECT:
        {
            awt::Size aOleSize( rShapeRect.Width, rShapeRect.Height );
            if( rFilter.getOleObjectHelper().importOleObject( maShapeProperties, *mxOleObjectInfo, aOleSize ) )
                aServiceName = "com.sun.star.drawing.OLE2Shape";

            // get the path to the representation graphic
            OUString aGraphicPath;
            if( !mxOleObjectInfo->maShapeId.isEmpty() )
                if( ::oox::vml::Drawing* pVmlDrawing = rFilter.getVmlDrawing() )
                    if( const ::oox::vml::ShapeBase* pVmlShape = pVmlDrawing->getShapes().getShapeById( mxOleObjectInfo->maShapeId ) )
                        aGraphicPath = pVmlShape->getGraphicPath();

            // import and store the graphic
            if( !aGraphicPath.isEmpty() )
            {
                // Transfer shape's width and height to graphicsfilter (can be used by WMF/EMF)
                WmfExternal aExtHeader;
                aExtHeader.mapMode = 8; // MM_ANISOTROPIC
                aExtHeader.xExt = rShapeRect.Width;
                aExtHeader.yExt = rShapeRect.Height;

                Reference< graphic::XGraphic > xGraphic = rFilter.getGraphicHelper().importEmbeddedGraphic( aGraphicPath, &aExtHeader );
                if( xGraphic.is() )
                    maShapeProperties.setProperty(PROP_Graphic, xGraphic);
            }
        }
        break;

        default:;
    }
    return aServiceName;
}

void Shape::finalizeXShape( XmlFilterBase& rFilter, const Reference< XShapes >& rxShapes )
{
    switch( meFrameType )
    {
        case FRAMETYPE_CHART:
        {
            OSL_ENSURE( !mxChartShapeInfo->maFragmentPath.isEmpty(), "Shape::finalizeXShape - missing chart fragment" );
            if( mxShape.is() && !mxChartShapeInfo->maFragmentPath.isEmpty() ) try
            {
                // set the chart2 OLE class ID at the OLE shape
                PropertySet aShapeProp( mxShape );
                aShapeProp.setProperty( PROP_CLSID, OUString( "12dcae26-281f-416f-a234-c3086127382e" ) );

                // get the XModel interface of the embedded object from the OLE shape
                Reference< frame::XModel > xDocModel;
                aShapeProp.getProperty( xDocModel, PROP_Model );
                Reference< chart2::XChartDocument > xChartDoc( xDocModel, UNO_QUERY_THROW );

                // load the chart data from the XML fragment
#if ENABLE_WASM_STRIP_CHART
                (void) rFilter;
                (void) rxShapes;
#else
                // WASM_CHART change
                // TODO: Instead of using convertFromModel an alternative may be
                // added to convert not to Chart/OLE SdrObejct, but to GraphicObject
                // with the Chart visualization. There should be a preview available
                // in the imported chart data
                bool bMSO2007Doc = rFilter.isMSO2007Document();
                chart::ChartSpaceModel aModel(bMSO2007Doc);
                oox::ppt::PowerPointImport* pPowerPointImport
                    = dynamic_cast<oox::ppt::PowerPointImport*>(&rFilter);

                ClrMapPtr pClrMap; // The original color map
                if (pPowerPointImport)
                {
                    // Use a copy of current color map, which the fragment may override locally
                    pClrMap = pPowerPointImport->getActualSlidePersist()->getClrMap();
                    aModel.mpClrMap = pClrMap ? std::make_shared<ClrMap>(*pClrMap)
                                              : std::make_shared<ClrMap>();
                    pPowerPointImport->getActualSlidePersist()->setClrMap(aModel.mpClrMap);
                }

                rtl::Reference<chart::ChartSpaceFragment> pChartSpaceFragment = new chart::ChartSpaceFragment(
                        rFilter, mxChartShapeInfo->maFragmentPath, aModel );
                const OUString aThemeOverrideFragmentPath( pChartSpaceFragment->
                        getFragmentPathFromFirstTypeFromOfficeDoc(u"themeOverride") );
                rFilter.importFragment( pChartSpaceFragment );

                // The original theme.
                ThemePtr pTheme;

                if (!aThemeOverrideFragmentPath.isEmpty() && pPowerPointImport)
                {
                    // Handle theme override.
                    uno::Reference< xml::sax::XFastSAXSerializable > xDoc(
                            rFilter.importFragment(aThemeOverrideFragmentPath), uno::UNO_QUERY_THROW);
                    pTheme = pPowerPointImport->getActualSlidePersist()->getTheme();
                    auto pThemeOverride = std::make_shared<Theme>(*pTheme);
                    rFilter.importFragment(
                        new ThemeOverrideFragmentHandler(rFilter, aThemeOverrideFragmentPath, *pThemeOverride, *pThemeOverride->getTheme()),
                        xDoc);
                    pPowerPointImport->getActualSlidePersist()->setTheme(pThemeOverride);
                }

                // convert imported chart model to chart document
                Reference< drawing::XShapes > xExternalPage;
                if( !mxChartShapeInfo->mbEmbedShapes )
                    xExternalPage = rxShapes;
                if( rFilter.getChartConverter() )
                {
                    rFilter.getChartConverter()->convertFromModel( rFilter, aModel, xChartDoc, xExternalPage, mxShape->getPosition(), mxShape->getSize() );
                    if( !xChartDoc->hasInternalDataProvider() )
                    {
                        Reference< chart2::data::XDataReceiver > xDataRec( xChartDoc, UNO_QUERY );
                        Reference< chart2::data::XDataSource > xData = xDataRec->getUsedData();
                        if( !xData->getDataSequences().hasElements() || !xData->getDataSequences()[0]->getValues().is() ||
                                !xData->getDataSequences()[0]->getValues()->getData().hasElements() )
                        {
                            rFilter.useInternalChartDataTable( true );
                            rFilter.getChartConverter()->convertFromModel( rFilter, aModel, xChartDoc, xExternalPage, mxShape->getPosition(), mxShape->getSize() );
                            rFilter.useInternalChartDataTable( false );
                        }
                    }

                }

                if (pPowerPointImport)
                {
                    if (!aThemeOverrideFragmentPath.isEmpty())
                    {
                        // Restore the original theme.
                        pPowerPointImport->getActualSlidePersist()->setTheme(pTheme);
                    }
                    // Restore the original color map
                    pPowerPointImport->getActualSlidePersist()->setClrMap(pClrMap);
                }
#endif
            }
            catch( Exception& )
            {
            }
        }
        break;

        default:;
    }
}

void Shape::putPropertyToGrabBag( const OUString& sPropertyName, const Any& aPropertyValue )
{
    PropertyValue aNewProperty;
    aNewProperty.Name = sPropertyName;
    aNewProperty.Value = aPropertyValue;
    putPropertyToGrabBag( aNewProperty );
}

void Shape::putPropertyToGrabBag( const PropertyValue& pProperty )
{
    Reference< XPropertySet > xSet( mxShape, UNO_QUERY );
    Reference< XPropertySetInfo > xSetInfo( xSet->getPropertySetInfo() );
    const OUString aGrabBagPropName = UNO_NAME_MISC_OBJ_INTEROPGRABBAG;
    if( mxShape.is() && xSet.is() && xSetInfo.is() && xSetInfo->hasPropertyByName( aGrabBagPropName ) )
    {
        Sequence< PropertyValue > aGrabBag;
        xSet->getPropertyValue( aGrabBagPropName ) >>= aGrabBag;

        sal_Int32 length = aGrabBag.getLength();
        aGrabBag.realloc( length + 1 );
        aGrabBag.getArray()[length] = pProperty;

        xSet->setPropertyValue( aGrabBagPropName, Any( aGrabBag ) );
    }
}

void Shape::putPropertiesToGrabBag( const Sequence< PropertyValue >& aProperties )
{
    Reference< XPropertySet > xSet( mxShape, UNO_QUERY );
    Reference< XPropertySetInfo > xSetInfo( xSet->getPropertySetInfo() );
    const OUString aGrabBagPropName = UNO_NAME_MISC_OBJ_INTEROPGRABBAG;
    if( !(mxShape.is() && xSet.is() && xSetInfo.is() && xSetInfo->hasPropertyByName( aGrabBagPropName )) )
        return;

    // get existing grab bag
    Sequence< PropertyValue > aGrabBag;
    xSet->getPropertyValue( aGrabBagPropName ) >>= aGrabBag;

    std::vector<PropertyValue> aVec;
    aVec.reserve(aProperties.getLength());

    // put the new items
    std::transform(aProperties.begin(), aProperties.end(), std::back_inserter(aVec),
        [](const PropertyValue& rProp) {
            PropertyValue aProp;
            aProp.Name = rProp.Name;
            aProp.Value = rProp.Value;
            return aProp;
        });

    // put it back to the shape
    xSet->setPropertyValue( aGrabBagPropName, Any( comphelper::concatSequences(aGrabBag, aVec) ) );
}

FillProperties Shape::getActualFillProperties(const Theme* pTheme, const FillProperties* pParentShapeFillProps) const
{
    FillProperties aFillProperties;
    aFillProperties.moFillType = XML_noFill;

    // Reference shape properties
    aFillProperties.assignUsed( *mpShapeRefFillPropPtr );

    // Theme
    if( pTheme != nullptr )
    {
        if( const ShapeStyleRef* pFillRef = getShapeStyleRef( XML_fillRef ) )
        {
            if( const FillProperties* pFillProps = pTheme->getFillStyle( pFillRef->mnThemedIdx ) )
                aFillProperties.assignUsed( *pFillProps );
        }
    }

    // Properties specified directly for this shape
    aFillProperties.assignUsed(getFillProperties());

    // Parent shape's properties
    if ( pParentShapeFillProps != nullptr)
        if( getFillProperties().moFillType.has_value() && getFillProperties().moFillType.value() == XML_grpFill )
            aFillProperties.assignUsed( *pParentShapeFillProps );

    return aFillProperties;
}

LineProperties Shape::getActualLineProperties(const Theme* pTheme) const
{
    LineProperties aLineProperties;
    aLineProperties.maLineFill.moFillType = XML_noFill;

    // Reference shape properties
    aLineProperties.assignUsed( *mpShapeRefLinePropPtr );

    // Theme
    if( pTheme != nullptr )
    {
        if( const ShapeStyleRef* pLineRef = getShapeStyleRef( XML_lnRef ) )
        {
            if( const LineProperties* pLineProps = pTheme->getLineStyle( pLineRef->mnThemedIdx ) )
                aLineProperties.assignUsed( *pLineProps );
        }
    }

    // Properties specified directly for this shape
    aLineProperties.assignUsed( getLineProperties() );

    return aLineProperties;
}

EffectProperties Shape::getActualEffectProperties(const Theme* pTheme) const
{
    EffectProperties aEffectProperties;

    // Reference shape properties
    aEffectProperties.assignUsed( *mpShapeRefEffectPropPtr );

    // Theme
    if( pTheme != nullptr )
    {
        if( const ShapeStyleRef* pEffectRef = getShapeStyleRef( XML_effectRef ) )
        {
            if( const EffectProperties* pEffectProps = pTheme->getEffectStyle( pEffectRef->mnThemedIdx ) )
                aEffectProperties.assignUsed( *pEffectProps );
        }
    }

    // Properties specified directly for this shape
    aEffectProperties.assignUsed ( getEffectProperties() );

    return aEffectProperties;
}

uno::Sequence< uno::Sequence< uno::Any > >  Shape::resolveRelationshipsOfTypeFromOfficeDoc(core::XmlFilterBase& rFilter, const OUString& sFragment, std::u16string_view sType )
{
    uno::Sequence< uno::Sequence< uno::Any > > xRelListTemp;
    sal_Int32 counter = 0;

    core::RelationsRef xRels = rFilter.importRelations( sFragment );
    if ( xRels )
    {
        core::RelationsRef xImageRels = xRels->getRelationsFromTypeFromOfficeDoc( sType );
        if ( xImageRels )
        {
            xRelListTemp.realloc( xImageRels->size() );
            auto pxRelListTemp = xRelListTemp.getArray();
            for (auto const& imageRel : *xImageRels)
            {
                uno::Sequence< uno::Any > diagramRelTuple (3);
                auto pdiagramRelTuple = diagramRelTuple.getArray();
                // [0] => RID, [1] => InputStream [2] => extension
                OUString sRelId = imageRel.second.maId;

                pdiagramRelTuple[0] <<= sRelId;
                OUString sTarget = xImageRels->getFragmentPathFromRelId( sRelId );

                uno::Reference< io::XInputStream > xImageInputStrm( rFilter.openInputStream( sTarget ), uno::UNO_SET_THROW );
                StreamDataSequence dataSeq;
                if ( rFilter.importBinaryData( dataSeq, sTarget ) )
                {
                    pdiagramRelTuple[1] <<= dataSeq;
                }

                pdiagramRelTuple[2] <<= sTarget.copy( sTarget.lastIndexOf(".") );

                pxRelListTemp[counter] = diagramRelTuple;
                ++counter;
            }
            xRelListTemp.realloc(counter);

        }
    }
    return xRelListTemp;
}

void Shape::cloneFillProperties()
{
    auto pFillProperties = std::make_shared<FillProperties>();
    pFillProperties->assignUsed(*mpFillPropertiesPtr);
    mpFillPropertiesPtr = pFillProperties;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
