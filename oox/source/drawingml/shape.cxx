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

#include <oox/drawingml/shape.hxx>
#include <drawingml/customshapeproperties.hxx>
#include <oox/drawingml/theme.hxx>
#include <drawingml/fillproperties.hxx>
#include <drawingml/graphicproperties.hxx>
#include <drawingml/scene3dcontext.hxx>
#include <drawingml/lineproperties.hxx>
#include <drawingml/presetgeometrynames.hxx>
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
#include <oox/ppt/pptimport.hxx>
#include <oox/vml/vmldrawing.hxx>
#include <oox/vml/vmlshape.hxx>
#include <oox/vml/vmlshapecontainer.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <oox/helper/graphichelper.hxx>
#include <oox/helper/propertyset.hxx>
#include <oox/helper/modelobjecthelper.hxx>
#include <oox/mathml/importutils.hxx>
#include <oox/mathml/import.hxx>
#include <oox/token/properties.hxx>

#include <comphelper/classids.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>
#include <tools/gen.hxx>
#include <tools/globname.hxx>
#include <tools/mapunit.hxx>
#include <editeng/unoprnms.hxx>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/xml/AttributeData.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/sax/XFastSAXSerializable.hpp>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeTextPathMode.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/io/XOutputStream.hpp>

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <com/sun/star/document/XActionLockable.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <svl/outstrm.hxx>
#include <svx/svdtrans.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/fltrcfg.hxx>
#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wmfexternal.hxx>
#include <sal/log.hxx>
#include <svx/unoshape.hxx>

#include <vcl/wmf.hxx>

using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::style;

namespace oox { namespace drawingml {

Shape::Shape( const sal_Char* pServiceName, bool bDefaultHeight )
: mpLinePropertiesPtr( new LineProperties )
, mpShapeRefLinePropPtr( new LineProperties )
, mpFillPropertiesPtr( new FillProperties )
, mpShapeRefFillPropPtr( new FillProperties )
, mpGraphicPropertiesPtr( new GraphicProperties )
, mpCustomShapePropertiesPtr( new CustomShapeProperties )
, mp3DPropertiesPtr( new Shape3DProperties )
, mpEffectPropertiesPtr( new EffectProperties )
, mpShapeRefEffectPropPtr( new EffectProperties )
, mpMasterTextListStyle( new TextListStyle )
, mnSubType( 0 )
, meFrameType( FRAMETYPE_GENERIC )
, mnRotation( 0 )
, mnDiagramRotation( 0 )
, mbFlipH( false )
, mbFlipV( false )
, mbHidden( false )
, mbHiddenMasterShape( false )
, mbLockedCanvas( false )
, mbWps( false )
, mbTextBox( false )
, mbHasLinkedTxbx( false )
, maDiagramDoms( 0 )
{
    if ( pServiceName )
        msServiceName = OUString::createFromAscii( pServiceName );
    setDefaults(bDefaultHeight);
}

Shape::Shape( const ShapePtr& pSourceShape )
: maChildren()
, mpTextBody(pSourceShape->mpTextBody)
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
, mxShape()
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
, mbLockedCanvas( pSourceShape->mbLockedCanvas )
, mbWps( pSourceShape->mbWps )
, mbTextBox( pSourceShape->mbTextBox )
, maLinkedTxbxAttr()
, mbHasLinkedTxbx(false)
, maDiagramDoms( pSourceShape->maDiagramDoms )
, mnZOrder(pSourceShape->mnZOrder)
, mnZOrderOff(pSourceShape->mnZOrderOff)
, mnDataNodeType(pSourceShape->mnDataNodeType)
, mfAspectRatio(pSourceShape->mfAspectRatio)
{}

Shape::~Shape()
{
}

table::TablePropertiesPtr const & Shape::getTableProperties()
{
    if ( !mpTablePropertiesPtr.get() )
        mpTablePropertiesPtr.reset( new table::TableProperties() );
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
    maDefaultShapeProperties.setProperty(PROP_ParaAdjust, static_cast< sal_Int16 >( ParagraphAdjust_LEFT )); // check for RTL?
}

::oox::vml::OleObjectInfo& Shape::setOleObjectType()
{
    OSL_ENSURE( meFrameType == FRAMETYPE_GENERIC, "Shape::setOleObjectType - multiple frame types" );
    meFrameType = FRAMETYPE_OLEOBJECT;
    mxOleObjectInfo.reset( new ::oox::vml::OleObjectInfo( true ) );
    return *mxOleObjectInfo;
}

ChartShapeInfo& Shape::setChartType( bool bEmbedShapes )
{
    OSL_ENSURE( meFrameType == FRAMETYPE_GENERIC, "Shape::setChartType - multiple frame types" );
    meFrameType = FRAMETYPE_CHART;
    msServiceName = "com.sun.star.drawing.OLE2Shape";
    mxChartShapeInfo.reset( new ChartShapeInfo( bEmbedShapes ) );
    return *mxChartShapeInfo;
}

void Shape::setDiagramType()
{
    OSL_ENSURE( meFrameType == FRAMETYPE_GENERIC, "Shape::setDiagramType - multiple frame types" );
    meFrameType = FRAMETYPE_DIAGRAM;
    msServiceName = "com.sun.star.drawing.GroupShape";
    mnSubType = 0;
}

void Shape::setTableType()
{
    OSL_ENSURE( meFrameType == FRAMETYPE_GENERIC, "Shape::setTableType - multiple frame types" );
    meFrameType = FRAMETYPE_TABLE;
    msServiceName = "com.sun.star.drawing.TableShape";
    mnSubType = 0;
}

void Shape::setServiceName( const sal_Char* pServiceName )
{
    if ( pServiceName )
        msServiceName = OUString::createFromAscii( pServiceName );
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
        FillProperties& rShapeOrParentShapeFillProps,
        ShapeIdMap* pShapeMap,
        bool bInGroup )
{
    SAL_INFO("oox.drawingml", "Shape::addShape: id='" << msId << "'");

    try
    {
        OUString sServiceName( msServiceName );
        if( !sServiceName.isEmpty() )
        {
            basegfx::B2DHomMatrix aMatrix( aTransformation );
            Reference< XShape > xShape( createAndInsert( rFilterBase, sServiceName, pTheme, rxShapes, false, false, aMatrix, rShapeOrParentShapeFillProps, bInGroup ) );

            if( pShapeMap && !msId.isEmpty() )
            {
                (*pShapeMap)[ msId ] = shared_from_this();
            }

            // if this is a group shape, we have to add also each child shape
            Reference< XShapes > xShapes( xShape, UNO_QUERY );
            if ( xShapes.is() )
                addChildren( rFilterBase, *this, pTheme, xShapes, pShapeMap, aMatrix );

            if( meFrameType == FRAMETYPE_DIAGRAM )
            {
                keepDiagramCompatibilityInfo();
                if( !SvtFilterOptions::Get().IsSmartArt2Shape() )
                    convertSmartArtToMetafile( rFilterBase );
            }
        }
    }
    catch( const Exception& e )
    {
        SAL_WARN( "oox.drawingml", "Shape::addShape: " << e );
    }
}

void Shape::setLockedCanvas(bool bLockedCanvas)
{
    mbLockedCanvas = bLockedCanvas;
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

    if ( rReferencedShape.mpTextBody.get() && bUseText )
        mpTextBody = std::make_shared<TextBody>( *rReferencedShape.mpTextBody.get() );
    else
        mpTextBody.reset();
    maShapeProperties = rReferencedShape.maShapeProperties;
    mpShapeRefLinePropPtr = std::make_shared<LineProperties>( rReferencedShape.getActualLineProperties(nullptr) );
    mpShapeRefFillPropPtr = std::make_shared<FillProperties>( rReferencedShape.getActualFillProperties(nullptr, nullptr) );
    mpCustomShapePropertiesPtr = std::make_shared<CustomShapeProperties>( *rReferencedShape.mpCustomShapePropertiesPtr.get() );
    mpTablePropertiesPtr = table::TablePropertiesPtr( rReferencedShape.mpTablePropertiesPtr.get() ? new table::TableProperties( *rReferencedShape.mpTablePropertiesPtr.get() ) : nullptr );
    mpShapeRefEffectPropPtr = std::make_shared<EffectProperties>( rReferencedShape.getActualEffectProperties(nullptr) );
    mpMasterTextListStyle = std::make_shared<TextListStyle>( *rReferencedShape.mpMasterTextListStyle.get() );
    maSize = rReferencedShape.maSize;
    maPosition = rReferencedShape.maPosition;
    mnRotation = rReferencedShape.mnRotation;
    mbFlipH = rReferencedShape.mbFlipH;
    mbFlipV = rReferencedShape.mbFlipV;
    mbHidden = rReferencedShape.mbHidden;
}

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

// for group shapes, the following method is also adding each child
void Shape::addChildren(
        XmlFilterBase& rFilterBase,
        Shape& rMaster,
        const Theme* pTheme,
        const Reference< XShapes >& rxShapes,
        ShapeIdMap* pShapeMap,
        const basegfx::B2DHomMatrix& aTransformation )
{
    basegfx::B2DHomMatrix aChildTransformation;

    aChildTransformation.translate(-maChPosition.X, -maChPosition.Y);
    aChildTransformation.scale(1/(maChSize.Width ? maChSize.Width : 1.0), 1/(maChSize.Height ? maChSize.Height : 1.0));

    // Child position and size is typically non-zero, but it's allowed to have
    // it like that, and in that case Word ignores the parent transformation
    // (excluding translate component).
    if (!mbWps || maChPosition.X || maChPosition.Y || maChSize.Width || maChSize.Height)
    {
        aChildTransformation *= aTransformation;
    }
    else
    {
        basegfx::B2DVector aScale, aTranslate;
        double fRotate, fShearX;
        aTransformation.decompose(aScale, aTranslate, fRotate, fShearX);
        aChildTransformation.translate(aTranslate.getX(), aTranslate.getY());
    }

    SAL_INFO("oox.drawingml", "Shape::addChildren: parent matrix:\n"
             << aChildTransformation.get(0, 0) << " "
             << aChildTransformation.get(0, 1) << " "
             << aChildTransformation.get(0, 2) << "\n"
             << aChildTransformation.get(1, 0) << " "
             << aChildTransformation.get(1, 1) << " "
             << aChildTransformation.get(1, 2) << "\n"
             << aChildTransformation.get(2, 0) << " "
             << aChildTransformation.get(2, 1) << " "
             << aChildTransformation.get(2, 2));

    for (auto const& child : rMaster.maChildren)
    {
        child->setMasterTextListStyle( mpMasterTextListStyle );
        child->addShape( rFilterBase, pTheme, rxShapes, aChildTransformation, getFillProperties(), pShapeMap, true );
    }
}

static void lcl_resetPropertyValue( std::vector<beans::PropertyValue>& rPropVec, const OUString& rName )
{
    auto aIterator = std::find_if( rPropVec.begin(), rPropVec.end(),
        [rName]( const beans::PropertyValue& rValue ) { return rValue.Name == rName; } );

    if (aIterator != rPropVec.end())
        rPropVec.erase( aIterator );
}

static void lcl_setPropertyValue( std::vector<beans::PropertyValue>& rPropVec,
                           const OUString& rName,
                           const beans::PropertyValue& rPropertyValue )
{
    lcl_resetPropertyValue( rPropVec, rName );

    rPropVec.push_back( rPropertyValue );
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

static void lcl_createPresetShape(const uno::Reference<drawing::XShape>& xShape,
                                         const OUString& rClass, const OUString& rPresetType,
                                         const CustomShapePropertiesPtr& pCustomShapePropertiesPtr,
                                         const TextBodyPtr& pTextBody,
                                         const GraphicHelper& rGraphicHelper)
{
    if (!xShape.is() || !pCustomShapePropertiesPtr || !pTextBody)
        return;

    uno::Reference<drawing::XEnhancedCustomShapeDefaulter> xDefaulter( xShape,
                                                                       uno::UNO_QUERY );

    if (!xDefaulter.is() || rClass.isEmpty())
        return;

    Reference<XPropertySet> xSet( xShape, UNO_QUERY );
    if (!xSet.is())
        return;

    auto aGdList = pCustomShapePropertiesPtr->getAdjustmentGuideList();
    Sequence<drawing::EnhancedCustomShapeAdjustmentValue> aAdjustment(
        !aGdList.empty() ? aGdList.size() : 1 );

    int nIndex = 0;
    for (auto& aEntry : aGdList)
    {
        double fAngle = NormAngle36000( aEntry.maFormula.toDouble() / -600.0 );
        fAngle = 360.0 - fAngle / 100.0;

        aAdjustment[nIndex].Value <<= fAngle;
        aAdjustment[nIndex++].State = css::beans::PropertyState_DIRECT_VALUE;
    }

    if (aGdList.empty())
    {
        // Default angle
        double fAngle = 0;
        if (rClass == "fontwork-arch-up-curve")
            fAngle = 180;

        aAdjustment[0].Value <<= fAngle;
        aAdjustment[0].State = css::beans::PropertyState_DIRECT_VALUE;
    }

    // Set properties
    xSet->setPropertyValue( UNO_NAME_TEXT_AUTOGROWHEIGHT, uno::makeAny( false ) );
    xSet->setPropertyValue( UNO_NAME_TEXT_AUTOGROWWIDTH, uno::makeAny( false ) );
    xSet->setPropertyValue( UNO_NAME_FILLSTYLE, uno::makeAny( drawing::FillStyle_SOLID ) );

    const TextParagraphVector& rParagraphs = pTextBody->getParagraphs();
    if (!rParagraphs.empty() && !rParagraphs[0]->getRuns().empty())
    {
        std::shared_ptr<TextParagraph> pParagraph = rParagraphs[0];
        std::shared_ptr<TextRun> pRun = pParagraph->getRuns()[0];
        TextCharacterProperties& pProperties = pRun->getTextCharacterProperties();

        if (pProperties.moBold.has() && pProperties.moBold.get())
        {
            xSet->setPropertyValue( UNO_NAME_CHAR_WEIGHT, uno::makeAny( css::awt::FontWeight::BOLD ) );
        }
        if (pProperties.moItalic.has() && pProperties.moItalic.get())
        {
            xSet->setPropertyValue( UNO_NAME_CHAR_POSTURE, uno::makeAny( css::awt::FontSlant::FontSlant_ITALIC ) );
        }
        if (pProperties.moHeight.has())
        {
            sal_Int32 nHeight = pProperties.moHeight.get() / 100;
            xSet->setPropertyValue( UNO_NAME_CHAR_HEIGHT, uno::makeAny( nHeight ) );
        }
        if (pProperties.maFillProperties.maFillColor.isUsed())
        {
            const sal_Int32 aFillColor = static_cast<sal_Int32>(
                pProperties.maFillProperties.maFillColor.getColor( rGraphicHelper ).GetRGBColor() );
            xSet->setPropertyValue( UNO_NAME_FILLCOLOR, uno::makeAny( aFillColor ) );
        }
        else
        {
            // Set default color
            xSet->setPropertyValue( UNO_NAME_FILLCOLOR, uno::makeAny( COL_BLACK ) );
        }
        {
            ParagraphAdjust eAdjust = ParagraphAdjust_LEFT;
            if (pParagraph->getProperties().getParaAdjust())
                eAdjust = pParagraph->getProperties().getParaAdjust().get();
            xSet->setPropertyValue( "ParaAdjust", uno::makeAny( eAdjust ) );
            SvxShape* pShape = SvxShape::getImplementation( xShape );
            assert(pShape);
            SdrTextHorzAdjust eHorzAdjust = lcl_convertAdjust( eAdjust );
            pShape->GetSdrObject()->SetMergedItem( SdrTextHorzAdjustItem( eHorzAdjust ) );
        }
    }

    // Apply vertical adjustment for text on arc
    SvxShape* pShape = SvxShape::getImplementation(xShape);
    assert(pShape);
    if (rClass == "fontwork-arch-up-curve")
        pShape->GetSdrObject()->SetMergedItem( SdrTextVertAdjustItem( SdrTextVertAdjust::SDRTEXTVERTADJUST_BOTTOM ) );
    else if (rClass == "fontwork-arch-down-curve")
        pShape->GetSdrObject()->SetMergedItem( SdrTextVertAdjustItem( SdrTextVertAdjust::SDRTEXTVERTADJUST_TOP ) );

    // Apply preset shape
    xDefaulter->createCustomShapeDefaults( rClass );

    auto aGeomPropSeq = xSet->getPropertyValue( "CustomShapeGeometry" )
                            .get<uno::Sequence<beans::PropertyValue>>();
    auto aGeomPropVec
        = comphelper::sequenceToContainer<std::vector<beans::PropertyValue>>(
            aGeomPropSeq );

    // Reset old properties
    const OUString sCoordinateSize( "CoordinateSize" );
    const OUString sEquations( "Equations" );
    const OUString sPath( "Path" );
    const OUString sTextPath( "TextPath" );
    const OUString sAdjustmentValues( "AdjustmentValues" );
    const OUString sPresetTextWarp( "PresetTextWarp" );

    lcl_resetPropertyValue( aGeomPropVec, sCoordinateSize );
    lcl_resetPropertyValue( aGeomPropVec, sEquations );
    lcl_resetPropertyValue( aGeomPropVec, sPath );

    // Some shapes don't need scaling
    bool bScale = true;
    if ( rPresetType == "textRingInside"
        || rPresetType == "textRingOutside"
        || rPresetType == "textCirclePour" )
    {
        bScale = false;
    }

    // Apply geometry properties
    uno::Sequence<beans::PropertyValue> aPropertyValues(
        comphelper::InitPropertySequence(
            { { sTextPath, uno::makeAny( true ) },
                { "TextPathMode",
                uno::Any( drawing::EnhancedCustomShapeTextPathMode_PATH ) },
                { "ScaleX", uno::Any( bScale ) } } ) );

    lcl_setPropertyValue( aGeomPropVec, sTextPath,
        comphelper::makePropertyValue( sTextPath, aPropertyValues ) );

    lcl_setPropertyValue( aGeomPropVec, sPresetTextWarp,
        comphelper::makePropertyValue( sPresetTextWarp, rPresetType ) );

    if ( rClass == "fontwork-arch-up-curve" || rClass == "fontwork-circle-curve"
        || rClass == "fontwork-arch-down-curve" || rClass == "fontwork-open-circle-curve" )
        lcl_setPropertyValue( aGeomPropVec, sAdjustmentValues,
            comphelper::makePropertyValue( sAdjustmentValues, aAdjustment ) );

    xSet->setPropertyValue(
        "CustomShapeGeometry",
        uno::makeAny(comphelper::containerToSequence(aGeomPropVec)));
}

Reference< XShape > const & Shape::createAndInsert(
        ::oox::core::XmlFilterBase& rFilterBase,
        const OUString& rServiceName,
        const Theme* pTheme,
        const css::uno::Reference< css::drawing::XShapes >& rxShapes,
        bool bClearText,
        bool bDoNotInsertEmptyTextBody,
        basegfx::B2DHomMatrix& aParentTransformation,
        FillProperties& rShapeOrParentShapeFillProps,
        bool bInGroup )
{
    bool bIsEmbMedia = false;
    SAL_INFO("oox.drawingml", "Shape::createAndInsert: id='" << msId << "' service='" << rServiceName << "'");

    formulaimport::XmlStreamBuilder * pMathXml(nullptr);
    if (mpTextBody.get())
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
    if ( mpTablePropertiesPtr.get() && rServiceName == "com.sun.star.drawing.TableShape" )
    {
        maSize.Width = 0;
        for (auto const& elem : mpTablePropertiesPtr->getTableGrid())
        {
            maSize.Width = o3tl::saturating_add(maSize.Width, static_cast<sal_Int32>(elem));
        }
        maSize.Height = 0;
        for (auto const& elem : mpTablePropertiesPtr->getTableRows())
        {
            maSize.Height = o3tl::saturating_add(maSize.Height, elem.getHeight());
        }
    }

    awt::Rectangle aShapeRectHmm( maPosition.X / EMU_PER_HMM, maPosition.Y / EMU_PER_HMM, maSize.Width / EMU_PER_HMM, maSize.Height / EMU_PER_HMM );

    OUString aServiceName;
    if (pMathXml)
    {
        // convert this shape to OLE
        aServiceName = "com.sun.star.drawing.OLE2Shape";
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
    bool bIsCustomShape = ( aServiceName == "com.sun.star.drawing.CustomShape" ||
                            aServiceName == "com.sun.star.drawing.ConnectorShape" );
    bool bUseRotationTransform = ( !mbWps ||
            aServiceName == "com.sun.star.drawing.LineShape" ||
            aServiceName == "com.sun.star.drawing.GroupShape" ||
            mbFlipH ||
            mbFlipV );

    basegfx::B2DHomMatrix aTransformation;

    if (bUseRotationTransform && mnDiagramRotation != 0)
    {
        // rotate diagram's shape around object's center before sizing
        aTransformation.translate(-0.5, -0.5);
        aTransformation.rotate(basegfx::deg2rad(mnDiagramRotation / 60000.0));
        aTransformation.translate(0.5, 0.5);
    }

    if( maSize.Width != 1 || maSize.Height != 1)
    {
        // take care there are no zeros used by error
        aTransformation.scale(
            maSize.Width ? maSize.Width : 1.0,
            maSize.Height ? maSize.Height : 1.0 );
    }

    bool bNoTranslation = !aParentTransformation.isIdentity();
    if( mbFlipH || mbFlipV || mnRotation != 0 || bNoTranslation )
    {
        // calculate object's center
        basegfx::B2DPoint aCenter(0.5, 0.5);
        aCenter *= aTransformation;

        // center object at origin
        aTransformation.translate( -aCenter.getX(), -aCenter.getY() );

        if( !bIsCustomShape && ( mbFlipH || mbFlipV ) )
        {
            // mirror around object's center
            aTransformation.scale( mbFlipH ? -1.0 : 1.0, mbFlipV ? -1.0 : 1.0 );
        }

        if( bUseRotationTransform )
        {
            // OOXML flips shapes before rotating them.
            if( bIsCustomShape )
            {
                basegfx::B2DVector aScale, aTranslate;
                double fRotate, fShearX;
                aParentTransformation.decompose(aScale, aTranslate, fRotate, fShearX);
                // A negative scale means that the shape needs to be flipped
                if(aScale.getX() < 0)
                {
                    mbFlipH = !mbFlipH;
                    aTransformation.scale(-1, 1);
                }
                if(aScale.getY() < 0)
                {
                    mbFlipV = !mbFlipV;
                    aTransformation.scale(1, -1);
                }
            }
            // rotate around object's center
            aTransformation.rotate(basegfx::deg2rad(static_cast<double>(mnRotation) / 60000.0));
        }

        // move object back from center
        aTransformation.translate( aCenter.getX(), aCenter.getY() );
    }

    if( maPosition.X != 0 || maPosition.Y != 0)
    {
        // if global position is used, add it to transformation
        if (mbWps && !bInGroup)
            aTransformation.translate( maPosition.X * EMU_PER_HMM, maPosition.Y * EMU_PER_HMM);
        else
            aTransformation.translate( maPosition.X, maPosition.Y );
    }

    aTransformation = aParentTransformation*aTransformation;
    aParentTransformation = aTransformation;
    aTransformation.scale(1/double(EMU_PER_HMM), 1/double(EMU_PER_HMM));

    if( bIsCustomShape && mbFlipH != mbFlipV )
    {
        basegfx::B2DVector aScale, aTranslate;
        double fRotate, fShearX;
        aTransformation.decompose(aScale, aTranslate, fRotate, fShearX);

        if(fRotate != 0)
        {
            // calculate object's center
            basegfx::B2DPoint aCenter(0.5, 0.5);
            aCenter *= aTransformation;
            aTransformation.translate( -aCenter.getX(), -aCenter.getY() );
            // OOXML flips shapes before rotating them, so the rotation needs to be inverted
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
        uno::Reference<lang::XServiceInfo> xModelInfo(rFilterBase.getModel(), uno::UNO_QUERY);
        bool bIsWriter = xModelInfo->supportsService("com.sun.star.text.TextDocument");
        for( i = 0; i < nNumPoints; ++i )
        {
            const basegfx::B2DPoint aPoint( aPoly.getB2DPoint( i ) );

            // tdf#106792 Not needed anymore due to the change in SdrPathObj::NbcResize:
            // tdf#96674: Guard against zero width or height.

            if (bIsWriter && bNoTranslation)
                // Writer's draw page is in twips, and these points get passed
                // to core without any unit conversion when Writer
                // postprocesses only the group shape itself.
                pPoints[i] = awt::Point(static_cast<sal_Int32>(convertMm100ToTwip(aPoint.getX())), static_cast<sal_Int32>(convertMm100ToTwip(aPoint.getY())));
            else
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

        aMatrix.Line3.Column1 = aTransformation.get(2,0);
        aMatrix.Line3.Column2 = aTransformation.get(2,1);
        aMatrix.Line3.Column3 = aTransformation.get(2,2);

        maShapeProperties.setProperty(PROP_Transformation, aMatrix);
    }

    Reference< lang::XMultiServiceFactory > xServiceFact( rFilterBase.getModel(), UNO_QUERY_THROW );
    if ( !mxShape.is() )
        mxShape.set( xServiceFact->createInstance( aServiceName ), UNO_QUERY_THROW );

    Reference< XPropertySet > xSet( mxShape, UNO_QUERY );
    if( mxShape.is() && xSet.is() )
    {
        if( !msName.isEmpty() )
        {
            Reference< container::XNamed > xNamed( mxShape, UNO_QUERY );
            if( xNamed.is() )
                xNamed->setName( msName );
        }
        if( !msDescription.isEmpty() )
        {
            const OUString sDescription( "Description" );
            xSet->setPropertyValue( sDescription, Any( msDescription ) );
        }
        if (aServiceName != "com.sun.star.text.TextFrame")
            rxShapes->add( mxShape );

        if ( mbHidden || mbHiddenMasterShape )
        {
            SAL_INFO("oox.drawingml", "Shape::createAndInsert: invisible shape with id='" << msId << "'");
            const OUString sVisible( "Visible" );
            xSet->setPropertyValue( sVisible, Any( false ) );
            // In Excel hidden means not printed, let's use visibility for now until that's handled separately
            const OUString sPrintable( "Printable" );
            xSet->setPropertyValue( sPrintable, Any( false ) );
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
            xSet->setPropertyValue("CLSID", uno::makeAny(name.GetHexName()));
            uno::Reference<embed::XEmbeddedObject> const xObj(
                xSet->getPropertyValue("EmbeddedObject"), uno::UNO_QUERY);
            uno::Reference<uno::XInterface> const xMathModel(xObj->getComponent());
            oox::FormulaImportBase *const pMagic(
                    dynamic_cast<oox::FormulaImportBase*>(xMathModel.get()));
            assert(pMagic);
            pMagic->readFormulaOoxml(*pMathXml);
        }

        const GraphicHelper& rGraphicHelper = rFilterBase.getGraphicHelper();

        ::Color nLinePhClr(0xffffffff);
        ::Color nFillPhClr(0xffffffff);
        // TODO: use ph color when applying effect properties
        //sal_Int32 nEffectPhClr = -1;

        if( pTheme )
        {
            if( const ShapeStyleRef* pLineRef = getShapeStyleRef( XML_lnRef ) )
            {
                LineProperties aLineProperties;
                aLineProperties.maLineFill.moFillType = XML_noFill;
                if( const LineProperties* pLineProps = pTheme->getLineStyle( pLineRef->mnThemedIdx ) )
                    aLineProperties.assignUsed( *pLineProps );
                nLinePhClr = pLineRef->maPhClr.getColor( rGraphicHelper );

                // Store style-related properties to InteropGrabBag to be able to export them back
                uno::Sequence<beans::PropertyValue> aProperties = comphelper::InitPropertySequence(
                {
                    {"SchemeClr", uno::makeAny(pLineRef->maPhClr.getSchemeName())},
                    {"Idx", uno::makeAny(pLineRef->mnThemedIdx)},
                    {"Color", uno::makeAny(nLinePhClr)},
                    {"LineStyle", uno::makeAny(aLineProperties.getLineStyle())},
                    {"LineJoint", uno::makeAny(aLineProperties.getLineJoint())},
                    {"LineWidth", uno::makeAny(aLineProperties.getLineWidth())},
                    {"Transformations", uno::makeAny(pLineRef->maPhClr.getTransformations())}
                });
                putPropertyToGrabBag( "StyleLnRef", Any( aProperties ) );
            }
            if( const ShapeStyleRef* pFillRef = getShapeStyleRef( XML_fillRef ) )
            {
                nFillPhClr = pFillRef->maPhClr.getColor( rGraphicHelper );

                OUString sColorScheme = pFillRef->maPhClr.getSchemeName();
                if( !sColorScheme.isEmpty() )
                {
                    uno::Sequence<beans::PropertyValue> aProperties = comphelper::InitPropertySequence(
                    {
                        {"SchemeClr", uno::makeAny(sColorScheme)},
                        {"Idx", uno::makeAny(pFillRef->mnThemedIdx)},
                        {"Color", uno::makeAny(nFillPhClr)},
                        {"Transformations", uno::makeAny(pFillRef->maPhClr.getTransformations())}
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
                    {"SchemeClr", uno::makeAny(pEffectRef->maPhClr.getSchemeName())},
                    {"Idx", uno::makeAny(pEffectRef->mnThemedIdx)},
                    {"Transformations", uno::makeAny(pEffectRef->maPhClr.getTransformations())}
                });
                putPropertyToGrabBag( "StyleEffectRef", Any( aProperties ) );
            }
        }
        ShapePropertyMap aShapeProps( rFilterBase.getModelObjectHelper() );

        // add properties from textbody to shape properties
        if( mpTextBody.get() )
        {
            mpTextBody->getTextProperties().pushRotationAdjustments();
            aShapeProps.assignUsed( mpTextBody->getTextProperties().maPropertyMap );
            // Push char properties as well - specifically useful when this is a placeholder
            if( mpMasterTextListStyle &&  mpMasterTextListStyle->getListStyle()[0]->getTextCharacterProperties().moHeight.has() )
                aShapeProps.setProperty(PROP_CharHeight, GetFontHeight( mpMasterTextListStyle->getListStyle()[0]->getTextCharacterProperties().moHeight.get() ));
        }

        // applying properties
        aShapeProps.assignUsed( getShapeProperties() );
        aShapeProps.assignUsed( maDefaultShapeProperties );
        if ( bIsEmbMedia || aServiceName == "com.sun.star.drawing.GraphicObjectShape" || aServiceName == "com.sun.star.drawing.OLE2Shape" )
            mpGraphicPropertiesPtr->pushToPropMap( aShapeProps, rGraphicHelper );
        if ( mpTablePropertiesPtr.get() && aServiceName == "com.sun.star.drawing.TableShape" )
            mpTablePropertiesPtr->pushToPropSet( rFilterBase, xSet, mpMasterTextListStyle );

        FillProperties aFillProperties = getActualFillProperties(pTheme, &rShapeOrParentShapeFillProps);
        aFillProperties.pushToPropMap( aShapeProps, rGraphicHelper, mnRotation, nFillPhClr, mbFlipH, mbFlipV );
        LineProperties aLineProperties = getActualLineProperties(pTheme);
        aLineProperties.pushToPropMap( aShapeProps, rGraphicHelper, nLinePhClr );
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
                    aGrabBag[length].Name = "mso-orig-shape-type";
                    uno::Sequence< sal_Int8 > const & aNameSeq =
                        mpCustomShapePropertiesPtr->getShapePresetTypeName();
                    OUString sShapePresetTypeName(reinterpret_cast< const char* >(
                        aNameSeq.getConstArray()), aNameSeq.getLength(), RTL_TEXTENCODING_UTF8);
                    aGrabBag[length].Value <<= sShapePresetTypeName;
                    propertySet->setPropertyValue("FrameInteropGrabBag",uno::makeAny(aGrabBag));
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
                    aGrabBag[length].Name = "TxbxHasLink";
                    aGrabBag[length].Value <<= isLinkedTxbx();
                    aGrabBag[length + 1 ].Name = "Txbx-Id";
                    aGrabBag[length + 1 ].Value <<= getLinkedTxbxAttributes().id;
                    aGrabBag[length + 2 ].Name = "Txbx-Seq";
                    aGrabBag[length + 2 ].Value <<= getLinkedTxbxAttributes().seq;
                    propertySet->setPropertyValue("FrameInteropGrabBag",uno::makeAny(aGrabBag));
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
                // TextFrames have BackGrahic, not FillBitmap
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
                        if (aLineProperties.moLineWidth.has())
                            aBorderLine.LineWidth = convertEmuToHmm(aLineProperties.moLineWidth.get());
                        aShapeProps.setProperty(nBorder, aBorderLine);
                    }
                    aShapeProps.erase(PROP_LineColor);
                }
                if(mnRotation)
                {
                    uno::Reference<beans::XPropertySet> xPropertySet(mxShape, uno::UNO_QUERY);
                    const OUString aGrabBagPropName = "FrameInteropGrabBag";
                    uno::Sequence<beans::PropertyValue> aGrabBag;
                    xPropertySet->getPropertyValue(aGrabBagPropName) >>= aGrabBag;
                    beans::PropertyValue aPair;
                    aPair.Name = "mso-rotation-angle";
                    aPair.Value <<= mnRotation;
                    if (aGrabBag.hasElements())
                    {
                        sal_Int32 nLength = aGrabBag.getLength();
                        aGrabBag.realloc(nLength + 1);
                        aGrabBag[nLength] = aPair;
                    }
                    else
                    {
                        aGrabBag.realloc(1);
                        aGrabBag[0] = aPair;
                    }
                    xPropertySet->setPropertyValue(aGrabBagPropName, uno::makeAny(aGrabBag));
                }
                // TextFrames have ShadowFormat, not individual shadow properties.
                boost::optional<sal_Int32> oShadowDistance;
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
                boost::optional<sal_Int32> oShadowColor;
                if (aShapeProps.hasProperty(PROP_ShadowColor))
                {
                    oShadowColor = aShapeProps.getProperty(PROP_ShadowColor).get<sal_Int32>();
                    aShapeProps.erase(PROP_ShadowColor);
                }
                if (aShapeProps.hasProperty(PROP_Shadow))
                    aShapeProps.erase(PROP_Shadow);

                if (oShadowDistance || oShadowColor || aEffectProperties.maShadow.moShadowDir.has())
                {
                    css::table::ShadowFormat aFormat;
                    if (oShadowColor)
                        aFormat.Color = *oShadowColor;
                    if (aEffectProperties.maShadow.moShadowDir.has())
                    {
                        css::table::ShadowLocation nLocation = css::table::ShadowLocation_NONE;
                        switch (aEffectProperties.maShadow.moShadowDir.get())
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
                aShapeProps.setProperty(PROP_TextBox, true);
            }

            if (aServiceName != "com.sun.star.text.TextFrame" && isLinkedTxbx())
            {
                uno::Reference<beans::XPropertySet> propertySet (mxShape, uno::UNO_QUERY);
                uno::Sequence<beans::PropertyValue> aGrabBag;
                propertySet->getPropertyValue("InteropGrabBag") >>= aGrabBag;
                sal_Int32 length = aGrabBag.getLength();
                aGrabBag.realloc( length + 3 );
                aGrabBag[length].Name = "TxbxHasLink";
                aGrabBag[length].Value <<= isLinkedTxbx();
                aGrabBag[length + 1 ].Name = "Txbx-Id";
                aGrabBag[length + 1 ].Value <<= getLinkedTxbxAttributes().id;
                aGrabBag[length + 2 ].Name = "Txbx-Seq";
                aGrabBag[length + 2 ].Value <<= getLinkedTxbxAttributes().seq;
                propertySet->setPropertyValue("InteropGrabBag",uno::makeAny(aGrabBag));
            }

            PropertySet( xSet ).setProperties( aShapeProps );
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

            // Store original fill and line colors of the shape and the theme color name to InteropGrabBag
            std::vector<beans::PropertyValue> aProperties;
            aProperties.push_back(comphelper::makePropertyValue("OriginalSolidFillClr", aShapeProps.getProperty(PROP_FillColor)));
            aProperties.push_back(comphelper::makePropertyValue("OriginalLnSolidFillClr", aShapeProps.getProperty(PROP_LineColor)));
            OUString sColorFillScheme = aFillProperties.maFillColor.getSchemeName();
            if( !aFillProperties.maFillColor.isPlaceHolder() && !sColorFillScheme.isEmpty() )
            {
                aProperties.push_back(comphelper::makePropertyValue("SpPrSolidFillSchemeClr", sColorFillScheme));
                aProperties.push_back(comphelper::makePropertyValue("SpPrSolidFillSchemeClrTransformations", aFillProperties.maFillColor.getTransformations()));
            }
            OUString sLnColorFillScheme = aLineProperties.maLineFill.maFillColor.getSchemeName();
            if( !aLineProperties.maLineFill.maFillColor.isPlaceHolder() && !sLnColorFillScheme.isEmpty() )
            {
                aProperties.push_back(comphelper::makePropertyValue("SpPrLnSolidFillSchemeClr", sLnColorFillScheme));
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
                    std::vector<beans::PropertyValue> aGradientStop;
                    aGradientStop.push_back(comphelper::makePropertyValue("Pos", rPos));

                    OUString sStopColorScheme = rColor.getSchemeName();
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
                if( getFillProperties().moFillType.has() )
                    putPropertyToGrabBag( "GradFillDefinition", uno::Any(comphelper::containerToSequence(aGradientStops)));
                putPropertyToGrabBag( "OriginalGradFill", aShapeProps.getProperty(PROP_FillGradient) );
            }

            // store unsupported effect attributes in the grab bag
            if (!aEffectProperties.m_Effects.empty())
            {
                std::vector<beans::PropertyValue> aEffects;
                sal_uInt32 i = 0;
                for (auto const& it : aEffectProperties.m_Effects)
                {
                    PropertyValue aEffect = it->getEffect();
                    if( !aEffect.Name.isEmpty() )
                    {
                        std::vector<beans::PropertyValue> aEffectsGrabBag;
                        aEffectsGrabBag.push_back(comphelper::makePropertyValue("Attribs", aEffect.Value));

                        Color& aColor( it->moColor );
                        OUString sColorScheme = aColor.getSchemeName();
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
                        ++i;
                    }
                }
                putPropertyToGrabBag("EffectProperties", uno::Any(comphelper::containerToSequence(aEffects)));
            }

            // add 3D effects if any
            Sequence< PropertyValue > aCamera3DEffects = get3DProperties().getCameraAttributes();
            Sequence< PropertyValue > aLightRig3DEffects = get3DProperties().getLightRigAttributes();
            Sequence< PropertyValue > aShape3DEffects = get3DProperties().getShape3DAttributes( rGraphicHelper, nFillPhClr );
            if( aCamera3DEffects.getLength() > 0 || aLightRig3DEffects.getLength() > 0 || aShape3DEffects.getLength() > 0 )
            {
                uno::Sequence<beans::PropertyValue> a3DEffectsGrabBag = comphelper::InitPropertySequence(
                {
                    {"Camera", uno::makeAny(aCamera3DEffects)},
                    {"LightRig", uno::makeAny(aLightRig3DEffects)},
                    {"Shape3D", uno::makeAny(aShape3DEffects)}
                });
                putPropertyToGrabBag( "3DEffectProperties", Any( a3DEffectsGrabBag ) );
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

        // These can have a custom geometry, so position should be set here,
        // after creation but before custom shape handling, using the position
        // we got from the caller.
        if (mbWps && aServiceName == "com.sun.star.drawing.LineShape")
            mxShape->setPosition(maPosition);

        if( bIsCustomShape )
        {
            if ( mbFlipH )
                mpCustomShapePropertiesPtr->setMirroredX( true );
            if ( mbFlipV )
                mpCustomShapePropertiesPtr->setMirroredY( true );
            if( getTextBody() )
            {
                sal_Int32 nTextRotateAngle = static_cast< sal_Int32 >( getTextBody()->getTextProperties().moRotation.get( 0 ) );
                nTextRotateAngle -= mnDiagramRotation;
                /* OOX measures text rotation clockwise in 1/60000th degrees,
                   relative to the containing shape. setTextRotateAngle wants
                   degrees anticlockwise. */
                mpCustomShapePropertiesPtr->setTextRotateAngle( -1 * nTextRotateAngle / 60000 );
            }

            // Note that the script oox/source/drawingml/customshapes/generatePresetsData.pl looks
            // for these ==cscode== and ==csdata== markers, so don't "clean up" these SAL_INFOs
            SAL_INFO("oox.cscode", "==cscode== shape name: '" << msName << "'");
            SAL_INFO("oox.csdata", "==csdata== shape name: '" << msName << "'");
            mpCustomShapePropertiesPtr->pushToPropSet( xSet, mxShape, maSize );

            if (mpTextBody)
            {
                bool bIsPresetShape = !mpTextBody->getTextProperties().msPrst.isEmpty();
                if (bIsPresetShape)
                {
                    OUString sClass;
                    const OUString sPresetType = mpTextBody->getTextProperties().msPrst;
                    sClass = PresetGeometryTypeNames::GetFontworkType( sPresetType );

                    lcl_createPresetShape( mxShape, sClass, sPresetType, mpCustomShapePropertiesPtr, mpTextBody, rGraphicHelper );
                }
            }
        }
        else if( getTextBody() )
            getTextBody()->getTextProperties().pushVertSimulation();

        PropertySet aPropertySet(mxShape);
        if ( !bUseRotationTransform && mnRotation != 0 )
        {
            // use the same logic for rotation from VML exporter (SimpleShape::implConvertAndInsert at vmlshape.cxx)
            aPropertySet.setAnyProperty( PROP_RotateAngle, makeAny( sal_Int32( NormAngle36000( mnRotation / -600 ) ) ) );
            aPropertySet.setAnyProperty( PROP_HoriOrientPosition, makeAny( maPosition.X ) );
            aPropertySet.setAnyProperty( PROP_VertOrientPosition, makeAny( maPosition.Y ) );
        }

        // in some cases, we don't have any text body.
        if( getTextBody() && ( !bDoNotInsertEmptyTextBody || !mpTextBody->isEmpty() ) )
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
                            aCharStyleProperties.maFillProperties.moFillType.set(XML_solidFill);
                        }
                    }
                }
                xText->setString("");
                Reference < XTextCursor > xAt = xText->createTextCursor();
                getTextBody()->insertAt( rFilterBase, xText, xAt, aCharStyleProperties, mpMasterTextListStyle );
            }
        }
        else if (mbTextBox)
        {
            // No drawingML text, but WPS text is expected: save the theme
            // character color on the shape, then.
            if(const ShapeStyleRef* pFontRef = getShapeStyleRef(XML_fontRef))
            {
                ::Color nCharColor = pFontRef->maPhClr.getColor(rGraphicHelper);
                aPropertySet.setAnyProperty(PROP_CharColor, uno::makeAny(nCharColor));
            }
        }
    }

    if( mxShape.is() )
        finalizeXShape( rFilterBase, rxShapes );

    return mxShape;
}

void Shape::keepDiagramDrawing(XmlFilterBase& rFilterBase, const OUString& rFragmentPath)
{
    uno::Sequence<uno::Any> diagramDrawing(2);
    // drawingValue[0] => dom, drawingValue[1] => Sequence of associated relationships

    sal_Int32 length = maDiagramDoms.getLength();
    maDiagramDoms.realloc(length + 1);

    diagramDrawing[0] <<= rFilterBase.importFragment(rFragmentPath);
    diagramDrawing[1] <<= resolveRelationshipsOfTypeFromOfficeDoc(rFilterBase, rFragmentPath, "image");

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
        {
            sal_Int32 length = aGrabBag.getLength();
            aGrabBag.realloc( length+maDiagramDoms.getLength() );

            for( sal_Int32 i = 0; i < maDiagramDoms.getLength(); ++i )
                aGrabBag[length+i] = maDiagramDoms[i];

            xSet->setPropertyValue( aGrabBagPropName, Any( aGrabBag ) );
        } else
            xSet->setPropertyValue( aGrabBagPropName, Any( maDiagramDoms ) );
    }
    catch( const Exception& e )
    {
        SAL_WARN( "oox.drawingml", "Shape::keepDiagramCompatibilityInfo: " << e );
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
        while (xShapes->hasElements())
            xShapes->remove(Reference<XShape>(xShapes->getByIndex(0), UNO_QUERY_THROW));
        xShapes->add(xShape);
    }
    catch (const Exception& e)
    {
        SAL_WARN("oox.drawingml", "Shape::convertSmartArtToMetafile: " << e);
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
        awt::Size aSize = awt::Size( static_cast < sal_Int32 > ( ( fPixelsPer100thmm * aActualSize.Width ) + 0.5 ),
                                     static_cast < sal_Int32 > ( ( fPixelsPer100thmm * aActualSize.Height ) + 0.5 ) );

        Sequence< PropertyValue > aFilterData( 4 );
        aFilterData[ 0 ].Name = "PixelWidth";
        aFilterData[ 0 ].Value <<= aSize.Width;
        aFilterData[ 1 ].Name = "PixelHeight";
        aFilterData[ 1 ].Value <<= aSize.Height;
        aFilterData[ 2 ].Name = "LogicalWidth";
        aFilterData[ 2 ].Value <<= aActualSize.Width;
        aFilterData[ 3 ].Name = "LogicalHeight";
        aFilterData[ 3 ].Value <<= aActualSize.Height;

        Sequence < PropertyValue > aDescriptor( 3 );
        aDescriptor[ 0 ].Name = "OutputStream";
        aDescriptor[ 0 ].Value <<= xOutputStream;
        aDescriptor[ 1 ].Name = "FilterName";
        aDescriptor[ 1 ].Value <<= OUString("SVM"); // Rendering format
        aDescriptor[ 2 ].Name = "FilterData";
        aDescriptor[ 2 ].Value <<= aFilterData;

        Reference < lang::XComponent > xSourceDoc( mxShape, UNO_QUERY_THROW );
        Reference < XGraphicExportFilter > xGraphicExporter = GraphicExportFilter::create( rFilterBase.getComponentContext() );
        xGraphicExporter->setSourceDocument( xSourceDoc );
        xGraphicExporter->filter( aDescriptor );

        aTempStream.Seek( STREAM_SEEK_TO_BEGIN );

        Graphic aGraphic;
        GraphicFilter aFilter( false );
        if ( aFilter.ImportGraphic( aGraphic, "", aTempStream, GRFILTER_FORMAT_NOTFOUND, nullptr, GraphicFilterImportFlags::NONE, static_cast < Sequence < PropertyValue >* > ( nullptr ) ) != ERRCODE_NONE )
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
    catch( const Exception& e )
    {
        SAL_WARN( "oox.drawingml", "Shape::renderDiagramToGraphic: " << e );
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
                bool bMSO2007Doc = rFilter.isMSO2007Document();
                chart::ChartSpaceModel aModel(bMSO2007Doc);
                chart::ChartSpaceFragment *pChartSpaceFragment = new chart::ChartSpaceFragment(
                        rFilter, mxChartShapeInfo->maFragmentPath, aModel );
                const OUString aThemeOverrideFragmentPath( pChartSpaceFragment->
                        getFragmentPathFromFirstTypeFromOfficeDoc("themeOverride") );
                rFilter.importFragment( pChartSpaceFragment );
                ::oox::ppt::PowerPointImport *pPowerPointImport =
                    dynamic_cast< ::oox::ppt::PowerPointImport* >(&rFilter);
                if (!aThemeOverrideFragmentPath.isEmpty() && pPowerPointImport)
                {
                    uno::Reference< xml::sax::XFastSAXSerializable > xDoc(
                            rFilter.importFragment(aThemeOverrideFragmentPath), uno::UNO_QUERY_THROW);
                    ThemePtr pTheme = pPowerPointImport->getActualSlidePersist()->getTheme();
                    rFilter.importFragment(new ThemeOverrideFragmentHandler(
                                rFilter, aThemeOverrideFragmentPath, *pTheme), xDoc);
                    pPowerPointImport->getActualSlidePersist()->setTheme(pTheme);
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
                        Reference< chart2::data::XDataSource > xData( xDataRec->getUsedData(), UNO_QUERY );
                        if( xData->getDataSequences().getLength() <= 0 || !xData->getDataSequences()[0]->getValues().is() ||
                                xData->getDataSequences()[0]->getValues()->getData().getLength() <= 0 )
                        {
                            rFilter.useInternalChartDataTable( true );
                            rFilter.getChartConverter()->convertFromModel( rFilter, aModel, xChartDoc, xExternalPage, mxShape->getPosition(), mxShape->getSize() );
                            rFilter.useInternalChartDataTable( false );
                        }
                    }

                }
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
        aGrabBag[length] = pProperty;

        xSet->setPropertyValue( aGrabBagPropName, Any( aGrabBag ) );
    }
}

void Shape::putPropertiesToGrabBag( const Sequence< PropertyValue >& aProperties )
{
    Reference< XPropertySet > xSet( mxShape, UNO_QUERY );
    Reference< XPropertySetInfo > xSetInfo( xSet->getPropertySetInfo() );
    const OUString aGrabBagPropName = UNO_NAME_MISC_OBJ_INTEROPGRABBAG;
    if( mxShape.is() && xSet.is() && xSetInfo.is() && xSetInfo->hasPropertyByName( aGrabBagPropName ) )
    {
        // get existing grab bag
        Sequence< PropertyValue > aGrabBag;
        xSet->getPropertyValue( aGrabBagPropName ) >>= aGrabBag;
        sal_Int32 length = aGrabBag.getLength();

        // update grab bag size to contain the new items
        aGrabBag.realloc( length + aProperties.getLength() );

        // put the new items
        for( sal_Int32 i=0; i < aProperties.getLength(); ++i )
        {
            aGrabBag[length + i].Name = aProperties[i].Name;
            aGrabBag[length + i].Value = aProperties[i].Value;
        }

        // put it back to the shape
        xSet->setPropertyValue( aGrabBagPropName, Any( aGrabBag ) );
    }
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
        if( getFillProperties().moFillType.has() && getFillProperties().moFillType.get() == XML_grpFill )
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

uno::Sequence< uno::Sequence< uno::Any > >  Shape::resolveRelationshipsOfTypeFromOfficeDoc(core::XmlFilterBase& rFilter, const OUString& sFragment, const OUString& sType )
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
            for (auto const& imageRel : *xImageRels)
            {
                uno::Sequence< uno::Any > diagramRelTuple (3);
                // [0] => RID, [1] => InputStream [2] => extension
                OUString sRelId = imageRel.second.maId;

                diagramRelTuple[0] <<= sRelId;
                OUString sTarget = xImageRels->getFragmentPathFromRelId( sRelId );

                uno::Reference< io::XInputStream > xImageInputStrm( rFilter.openInputStream( sTarget ), uno::UNO_SET_THROW );
                StreamDataSequence dataSeq;
                if ( rFilter.importBinaryData( dataSeq, sTarget ) )
                {
                    diagramRelTuple[1] <<= dataSeq;
                }

                diagramRelTuple[2] <<= sTarget.copy( sTarget.lastIndexOf(".") );

                xRelListTemp[counter] = diagramRelTuple;
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
} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
