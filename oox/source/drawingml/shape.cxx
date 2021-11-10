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
#include <drawingml/lineproperties.hxx>
#include <drawingml/presetgeometrynames.hxx>
#include <drawingml/shape3dproperties.hxx>
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
#include <oox/mathml/importutils.hxx>
#include <oox/mathml/import.hxx>
#include <oox/token/properties.hxx>
#include "diagram/datamodel.hxx"

#include <comphelper/classids.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>
#include <tools/diagnose_ex.h>
#include <tools/gen.hxx>
#include <tools/globname.hxx>
#include <tools/mapunit.hxx>
#include <tools/UnitConversion.hxx>
#include <editeng/unoprnms.hxx>
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
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <com/sun/star/document/XActionLockable.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <svx/svdtrans.hxx>
#include <tools/stream.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/fltrcfg.hxx>
#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wmfexternal.hxx>
#include <sal/log.hxx>
#include <svx/unoapi.hxx>
#include <svx/unoshape.hxx>
#include <svx/sdtaitm.hxx>

using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::style;

namespace oox::drawingml {

Shape::Shape( const char* pServiceName, bool bDefaultHeight )
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
, mbLockedCanvas( pSourceShape->mbLockedCanvas )
, mbWps( pSourceShape->mbWps )
, mbTextBox( pSourceShape->mbTextBox )
, mbHasLinkedTxbx(false)
, maDiagramDoms( pSourceShape->maDiagramDoms )
, mnZOrder(pSourceShape->mnZOrder)
, mnZOrderOff(pSourceShape->mnZOrderOff)
, mnDataNodeType(pSourceShape->mnDataNodeType)
, mfAspectRatio(pSourceShape->mfAspectRatio)
, mbUseBgFill(pSourceShape->mbUseBgFill)
, maDiagramFontHeights(pSourceShape->maDiagramFontHeights)
{}

Shape::~Shape()
{
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
        msServiceName = "com.sun.star.drawing.temporaryForXMLImportOLE2Shape";
    else
        msServiceName = "com.sun.star.drawing.OLE2Shape";
    mxChartShapeInfo = std::make_shared<ChartShapeInfo>( bEmbedShapes );
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

void Shape::setServiceName( const char* pServiceName )
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

            if( meFrameType == FRAMETYPE_DIAGRAM )
            {
                keepDiagramCompatibilityInfo();

                // Check if this is the PPTX import, so far converting SmartArt to a non-editable
                // metafile is only implemented for DOCX.
                bool bPowerPoint = dynamic_cast<oox::ppt::PowerPointImport*>(&rFilterBase) != nullptr;

                if (!SvtFilterOptions::Get().IsSmartArt2Shape() && !bPowerPoint)
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

    // The DrawingML shapes from the presetTextWarpDefinitions are mapped to the definitions
    // in svx/../EnhancedCustomShapeGeometry.cxx, which are used for WordArt shapes from
    // binary MS Office. Therefore all adjustment values need to be adapted.
    auto aAdjGdList = pCustomShapePropertiesPtr->getAdjustmentGuideList();
    Sequence<drawing::EnhancedCustomShapeAdjustmentValue> aAdjustment(
        !aAdjGdList.empty() ? aAdjGdList.size() : 1 );
    auto pAdjustment = aAdjustment.getArray();
    int nIndex = 0;
    for (const auto& aEntry : aAdjGdList)
    {
        double fValue = aEntry.maFormula.toDouble();
        // then: polar-handle, else: XY-handle
        // There exist only 8 polar-handles at all in presetTextWarp.
        if ((rClass == "fontwork-arch-down-curve")
            || (rClass == "fontwork-arch-down-pour" && aEntry.maName == "adj1")
            || (rClass == "fontwork-arch-up-curve")
            || (rClass == "fontwork-arch-up-pour" && aEntry.maName == "adj1")
            || (rClass == "fontwork-open-circle-curve")
            || (rClass == "fontwork-open-circle-pour" && aEntry.maName == "adj1")
            || (rClass == "fontwork-circle-curve")
            || (rClass == "fontwork-circle-pour" && aEntry.maName == "adj1"))
        {
            // DrawingML has 1/60000 degree unit, but WordArt simple degree. Range [0..360[
            // or range ]-180..180] doesn't matter, because only cos(angle) and
            // sin(angle) are used.
            fValue = NormAngle360(fValue / 60000.0);
        }
        else
        {
            // DrawingML writes adjustment guides as relative value with 100% = 100000,
            // but WordArt definitions use values absolute in viewBox 0 0 21600 21600,
            // so scale with 21600/100000 = 0.216, with two exceptions:
            // X-handles of waves describe increase/decrease relative to horizontal center.
            // The gdRefR of pour-shapes is not relative to viewBox but to radius.
            if ((rClass == "mso-spt158" && aEntry.maName == "adj2") // textDoubleWave1
                || (rClass == "fontwork-wave" && aEntry.maName == "adj2") // textWave1
                || (rClass == "mso-spt157" && aEntry.maName == "adj2") // textWave2
                || (rClass == "mso-spt159" && aEntry.maName == "adj2")) // textWave4
            {
                fValue = (fValue + 50000.0) * 0.216;
            }
            else if ( (rClass == "fontwork-arch-down-pour" && aEntry.maName == "adj2")
                    || (rClass == "fontwork-arch-up-pour" && aEntry.maName == "adj2")
                    || (rClass == "fontwork-open-circle-pour" && aEntry.maName == "adj2")
                    || (rClass == "fontwork-circle-pour" && aEntry.maName == "adj2"))
                {
                    fValue *= 0.108;
                }
            else
            {
                fValue *= 0.216;
            }
        }

        pAdjustment[nIndex].Value <<= fValue;
        pAdjustment[nIndex++].State = css::beans::PropertyState_DIRECT_VALUE;
    }

    // Set properties
    xSet->setPropertyValue( UNO_NAME_TEXT_AUTOGROWHEIGHT, uno::makeAny( false ) );
    xSet->setPropertyValue( UNO_NAME_TEXT_AUTOGROWWIDTH, uno::makeAny( false ) );
    xSet->setPropertyValue( UNO_NAME_FILLSTYLE, uno::makeAny( drawing::FillStyle_SOLID ) );

    // ToDo: Old binary WordArt does not allow different styles for different paragraphs, so it
    // was not necessary to examine all paragraphs. Solution for DrawingML is needed.
    // Currently different alignment of paragraphs are lost, for example.
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
                eAdjust = *pParagraph->getProperties().getParaAdjust();
            xSet->setPropertyValue( "ParaAdjust", uno::makeAny( eAdjust ) );
            SdrObject* pShape = SdrObject::getSdrObjectFromXShape( xShape );
            assert(pShape);
            SdrTextHorzAdjust eHorzAdjust = lcl_convertAdjust( eAdjust );
            pShape->SetMergedItem( SdrTextHorzAdjustItem( eHorzAdjust ) );
        }
    }

    // Apply vertical adjustment for text on arc
    // ToDo: The property is currently not evaluated.
    SdrObject* pShape = SdrObject::getSdrObjectFromXShape(xShape);
    assert(pShape);
    if (rClass == "fontwork-arch-up-curve" || rClass == "fontwork-circle-curve")
        pShape->SetMergedItem( SdrTextVertAdjustItem( SdrTextVertAdjust::SDRTEXTVERTADJUST_BOTTOM ) );
    else if (rClass == "fontwork-arch-down-curve")
        pShape->SetMergedItem( SdrTextVertAdjustItem( SdrTextVertAdjust::SDRTEXTVERTADJUST_TOP ) );

    // Apply preset shape
    xDefaulter->createCustomShapeDefaults( rClass );

    auto aGeomPropSeq = xSet->getPropertyValue( "CustomShapeGeometry" )
                            .get<uno::Sequence<beans::PropertyValue>>();
    auto aGeomPropVec
        = comphelper::sequenceToContainer<std::vector<beans::PropertyValue>>(
            aGeomPropSeq );

    // Reset old properties
    static const OUStringLiteral sTextPath( u"TextPath" );
    static const OUStringLiteral sAdjustmentValues( u"AdjustmentValues" );
    static const OUStringLiteral sPresetTextWarp( u"PresetTextWarp" );

    lcl_resetPropertyValue( aGeomPropVec, "CoordinateSize" );
    lcl_resetPropertyValue( aGeomPropVec, "Equations" );
    lcl_resetPropertyValue( aGeomPropVec, "Path" );
    lcl_resetPropertyValue( aGeomPropVec, sAdjustmentValues);

    bool bFromWordArt(false);
    pTextBody->getTextProperties().maPropertyMap.getProperty(PROP_FromWordArt) >>= bFromWordArt;

    bool bScaleX(false);
    if (!bFromWordArt
        && (rPresetType == "textArchDown" || rPresetType == "textArchUp"
            || rPresetType == "textCircle" || rPresetType == "textButton"))
    {
        bScaleX = true;
    }

    // Apply geometry properties
    uno::Sequence<beans::PropertyValue> aPropertyValues(
        comphelper::InitPropertySequence(
            { { sTextPath, uno::makeAny( true ) },
                { "TextPathMode",
                uno::Any( drawing::EnhancedCustomShapeTextPathMode_PATH ) },
                { "ScaleX", uno::Any(bScaleX) } } ) );

    lcl_setPropertyValue( aGeomPropVec, sTextPath,
        comphelper::makePropertyValue( sTextPath, aPropertyValues ) );

    lcl_setPropertyValue( aGeomPropVec, sPresetTextWarp,
        comphelper::makePropertyValue( sPresetTextWarp, rPresetType ) );

    if (!aAdjGdList.empty())
    {
        lcl_setPropertyValue( aGeomPropVec, sAdjustmentValues,
            comphelper::makePropertyValue( sAdjustmentValues, aAdjustment ) );
    }

    xSet->setPropertyValue(
        "CustomShapeGeometry",
        uno::makeAny(comphelper::containerToSequence(aGeomPropVec)));
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

void lcl_RotateAtCenter(basegfx::B2DHomMatrix& aTransformation,sal_Int32 nMSORotationAngle)
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
    // Use custom shape instead of GraphicObjectShape if the image is cropped to
    // shape. Except rectangle, which does not require further cropping
    bool bIsCroppedGraphic = (aServiceName == "com.sun.star.drawing.GraphicObjectShape" &&
                              (mpCustomShapePropertiesPtr->getShapePresetType() >= 0 || mpCustomShapePropertiesPtr->getPath2DList().size() > 0) &&
                              mpCustomShapePropertiesPtr->getShapePresetType() != XML_Rect &&
                              mpCustomShapePropertiesPtr->getShapePresetType() != XML_rect);
    // ToDo: Why is ConnectorShape here treated as custom shape, but below with start and end point?
    bool bIsCustomShape = ( aServiceName == "com.sun.star.drawing.CustomShape" ||
                            aServiceName == "com.sun.star.drawing.ConnectorShape" ||
                            bIsCroppedGraphic);
    if(bIsCroppedGraphic)
    {
        aServiceName = "com.sun.star.drawing.CustomShape";
        mpGraphicPropertiesPtr->mbIsCustomShape = true;
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
            const bool bNeedsMSOWidhtHeightToggle
                = (nDeg >= 45 && nDeg < 135) || (nDeg >= 225 && nDeg < 315);
            if (bNeedsMSOWidhtHeightToggle)
                lcl_doSpecialMSOWidthHeightToggle(aTransformation);

            aTransformation.scale(fFactorX, fFactorY);

            if (bNeedsMSOWidhtHeightToggle)
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
    if (bUseRotationTransform && mnRotation != 0)
        lcl_RotateAtCenter(aTransformation, nOrientation * mnRotation);

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
        uno::Reference<lang::XServiceInfo> xModelInfo(rFilterBase.getModel(), uno::UNO_QUERY);
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

        aMatrix.Line3.Column1 = aTransformation.get(2,0);
        aMatrix.Line3.Column2 = aTransformation.get(2,1);
        aMatrix.Line3.Column3 = aTransformation.get(2,2);

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
            xSet->setPropertyValue("CLSID", uno::makeAny(name.GetHexName()));
            uno::Reference<embed::XEmbeddedObject> const xObj(
                xSet->getPropertyValue("EmbeddedObject"), uno::UNO_QUERY);
            if (xObj.is())
            {
                uno::Reference<uno::XInterface> const xMathModel(xObj->getComponent());
                oox::FormulaImportBase *const pMagic(
                        dynamic_cast<oox::FormulaImportBase*>(xMathModel.get()));
                assert(pMagic);
                pMagic->readFormulaOoxml(*pMathXml);
            }
        }

        const GraphicHelper& rGraphicHelper = rFilterBase.getGraphicHelper();

        ::Color nLinePhClr(ColorTransparency, 0xffffffff);
        ::Color nFillPhClr(ColorTransparency, 0xffffffff);
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

                // Store style-related properties to InteropGrabBag to be able to export them back
                uno::Sequence<beans::PropertyValue> aProperties = comphelper::InitPropertySequence(
                {
                    {"SchemeClr", uno::makeAny(pLineRef->maPhClr.getSchemeColorName())},
                    {"Idx", uno::makeAny(pLineRef->mnThemedIdx)},
                    {"Color", uno::makeAny(nLinePhClr)},
                    {"LineStyle", uno::makeAny(aLineProperties.getLineStyle())},
                    {"LineCap", uno::makeAny(aLineProperties.getLineCap())},
                    {"LineJoint", uno::makeAny(aLineProperties.getLineJoint())},
                    {"LineWidth", uno::makeAny(aLineProperties.getLineWidth())},
                    {"Transformations", uno::makeAny(pLineRef->maPhClr.getTransformations())}
                });
                putPropertyToGrabBag( "StyleLnRef", Any( aProperties ) );
            }
            if( const ShapeStyleRef* pFillRef = getShapeStyleRef( XML_fillRef ) )
            {
                if (!mbUseBgFill)
                {
                    nFillPhClr = pFillRef->maPhClr.getColor(rGraphicHelper);
                }

                OUString sColorScheme = pFillRef->maPhClr.getSchemeColorName();
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
                    {"SchemeClr", uno::makeAny(pEffectRef->maPhClr.getSchemeColorName())},
                    {"Idx", uno::makeAny(pEffectRef->mnThemedIdx)},
                    {"Transformations", uno::makeAny(pEffectRef->maPhClr.getTransformations())}
                });
                putPropertyToGrabBag( "StyleEffectRef", Any( aProperties ) );
            }
        }
        ShapePropertyMap aShapeProps( rFilterBase.getModelObjectHelper() );

        // add properties from textbody to shape properties
        if( mpTextBody )
        {
            mpTextBody->getTextProperties().pushRotationAdjustments();
            aShapeProps.assignUsed( mpTextBody->getTextProperties().maPropertyMap );
            // Push char properties as well - specifically useful when this is a placeholder
            if( mpMasterTextListStyle &&  mpMasterTextListStyle->getListStyle()[0].getTextCharacterProperties().moHeight.has() )
                aShapeProps.setProperty(PROP_CharHeight, GetFontHeight( mpMasterTextListStyle->getListStyle()[0].getTextCharacterProperties().moHeight.get() ));
        }

        // applying properties
        aShapeProps.assignUsed( getShapeProperties() );
        aShapeProps.assignUsed( maDefaultShapeProperties );
        if(mnRotation != 0 && bIsCustomShape)
            aShapeProps.setProperty( PROP_RotateAngle, sal_Int32( NormAngle36000( Degree100(mnRotation / -600) ) ));
        if( bIsEmbMedia ||
            bIsCustomShape ||
            aServiceName == "com.sun.star.drawing.GraphicObjectShape" ||
            aServiceName == "com.sun.star.drawing.OLE2Shape")
        {
            mpGraphicPropertiesPtr->pushToPropMap( aShapeProps, rGraphicHelper, mbFlipH, mbFlipV );
        }
        if ( mpTablePropertiesPtr && aServiceName == "com.sun.star.drawing.TableShape" )
            mpTablePropertiesPtr->pushToPropSet( rFilterBase, xSet, mpMasterTextListStyle );

        FillProperties aFillProperties = getActualFillProperties(pTheme, &rShapeOrParentShapeFillProps);
        if (getFillProperties().moFillType.has() && getFillProperties().moFillType.get() == XML_grpFill)
            getFillProperties().assignUsed(aFillProperties);
        if(!bIsCroppedGraphic)
            aFillProperties.pushToPropMap( aShapeProps, rGraphicHelper, mnRotation, nFillPhClr, mbFlipH, mbFlipV, bIsCustomShape );
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
                    auto pGrabBag = aGrabBag.getArray();
                    pGrabBag[length].Name = "mso-orig-shape-type";
                    uno::Sequence< sal_Int8 > const & aNameSeq =
                        mpCustomShapePropertiesPtr->getShapePresetTypeName();
                    OUString sShapePresetTypeName(reinterpret_cast< const char* >(
                        aNameSeq.getConstArray()), aNameSeq.getLength(), RTL_TEXTENCODING_UTF8);
                    pGrabBag[length].Value <<= sShapePresetTypeName;
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
                    auto pGrabBag = aGrabBag.getArray();
                    pGrabBag[length].Name = "TxbxHasLink";
                    pGrabBag[length].Value <<= isLinkedTxbx();
                    pGrabBag[length + 1 ].Name = "Txbx-Id";
                    pGrabBag[length + 1 ].Value <<= getLinkedTxbxAttributes().id;
                    pGrabBag[length + 2 ].Name = "Txbx-Seq";
                    pGrabBag[length + 2 ].Value <<= getLinkedTxbxAttributes().seq;
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
                        if (aLineProperties.moLineWidth.has())
                            aBorderLine.LineWidth = convertEmuToHmm(aLineProperties.moLineWidth.get());
                        aShapeProps.setProperty(nBorder, aBorderLine);
                    }
                    aShapeProps.erase(PROP_LineColor);
                }
                if(mnRotation)
                {
                    uno::Reference<beans::XPropertySet> xPropertySet(mxShape, uno::UNO_QUERY);
                    static const OUStringLiteral aGrabBagPropName = u"FrameInteropGrabBag";
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
                    xPropertySet->setPropertyValue(aGrabBagPropName, uno::makeAny(aGrabBag));
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
                auto pGrabBag = aGrabBag.getArray();
                pGrabBag[length].Name = "TxbxHasLink";
                pGrabBag[length].Value <<= isLinkedTxbx();
                pGrabBag[length + 1 ].Name = "Txbx-Id";
                pGrabBag[length + 1 ].Value <<= getLinkedTxbxAttributes().id;
                pGrabBag[length + 2 ].Name = "Txbx-Seq";
                pGrabBag[length + 2 ].Value <<= getLinkedTxbxAttributes().seq;
                propertySet->setPropertyValue("InteropGrabBag",uno::makeAny(aGrabBag));
            }

            // If the shape is a picture placeholder.
            if (aServiceName == "com.sun.star.presentation.GraphicObjectShape" && !bClearText)
            {
                // Placeholder text should be in center of the shape.
                aShapeProps.setProperty(PROP_TextContourFrame, false);

                /* Placeholder icon should be at the center of the parent shape.
                 * We use negative graphic crop property because of that we don't
                 * have padding support.
                 */
                uno::Reference<beans::XPropertySet> xGraphic(xSet->getPropertyValue("Graphic"), uno::UNO_QUERY);
                if (xGraphic.is())
                {
                    awt::Size aBitmapSize;
                    xGraphic->getPropertyValue("Size100thMM") >>= aBitmapSize;
                    sal_Int32 nXMargin = (aShapeRectHmm.Width - aBitmapSize.Width) / 2;
                    sal_Int32 nYMargin = (aShapeRectHmm.Height - aBitmapSize.Height) / 2;
                    if (nXMargin > 0 && nYMargin > 0)
                    {
                        text::GraphicCrop aGraphicCrop;
                        aGraphicCrop.Top = nYMargin * -1;
                        aGraphicCrop.Bottom = nYMargin * -1;
                        aGraphicCrop.Left = nXMargin * -1;
                        aGraphicCrop.Right = nXMargin * -1;
                        aShapeProps.setProperty(PROP_GraphicCrop, aGraphicCrop);
                    }
                }
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
            std::vector<beans::PropertyValue> aProperties
            {
                comphelper::makePropertyValue("EmuLineWidth", aLineProperties.moLineWidth.get(0)),
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
                        ++i;
                    }
                }
                putPropertyToGrabBag("EffectProperties", uno::Any(comphelper::containerToSequence(aEffects)));
            }

            // add 3D effects if any
            Sequence< PropertyValue > aCamera3DEffects = get3DProperties().getCameraAttributes();
            Sequence< PropertyValue > aLightRig3DEffects = get3DProperties().getLightRigAttributes();
            Sequence< PropertyValue > aShape3DEffects = get3DProperties().getShape3DAttributes( rGraphicHelper, nFillPhClr );
            if( aCamera3DEffects.hasElements() || aLightRig3DEffects.hasElements() || aShape3DEffects.hasElements() )
            {
                uno::Sequence<beans::PropertyValue> a3DEffectsGrabBag = comphelper::InitPropertySequence(
                {
                    {"Camera", uno::makeAny(aCamera3DEffects)},
                    {"LightRig", uno::makeAny(aLightRig3DEffects)},
                    {"Shape3D", uno::makeAny(aShape3DEffects)}
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
                        {"Camera", uno::makeAny(aTextCamera3DEffects)},
                        {"LightRig", uno::makeAny(aTextLightRig3DEffects)},
                        {"Shape3D", uno::makeAny(aTextShape3DEffects)}
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

        // These can have a custom geometry, so position should be set here,
        // after creation but before custom shape handling, using the position
        // we got from the caller.
        if (mbWps && aServiceName == "com.sun.star.drawing.LineShape" && !pParentGroupShape)
            mxShape->setPosition(maPosition);

        if( bIsCustomShape )
        {
            if ( mbFlipH )
                mpCustomShapePropertiesPtr->setMirroredX( true );
            if ( mbFlipV )
                mpCustomShapePropertiesPtr->setMirroredY( true );
            if( getTextBody() )
            {
                sal_Int32 nTextCameraZRotation = static_cast< sal_Int32 >( getTextBody()->get3DProperties().maCameraRotation.mnRevolution.get() );
                mpCustomShapePropertiesPtr->setTextCameraZRotateAngle( nTextCameraZRotation / 60000 );

                sal_Int32 nTextRotateAngle = static_cast< sal_Int32 >( getTextBody()->getTextProperties().moRotation.get( 0 ) );

                nTextRotateAngle -= mnDiagramRotation;
                /* OOX measures text rotation clockwise in 1/60000th degrees,
                   relative to the containing shape. setTextRotateAngle wants degrees anticlockwise. */
                nTextRotateAngle = -1 * nTextRotateAngle / 60000;

                if (getTextBody()->getTextProperties().moUpright)
                {
                    // When upright is set, we want the text without any rotation.
                    // But if we set 0 here, the text is still rotated if the
                    // shape containing it is rotated.
                    // Hence, we rotate the text into the opposite direction of
                    // the rotation of the shape, by as much as the shape was rotated.
                    mpCustomShapePropertiesPtr->setTextRotateAngle((mnRotation / 60000) + nTextRotateAngle);
                    // Also put the initial angles away in a GrabBag.
                    putPropertyToGrabBag("Upright", Any(true));
                    putPropertyToGrabBag("nShapeRotationAtImport", Any(mnRotation / 60000));
                    putPropertyToGrabBag("nTextRotationAtImport", Any(nTextRotateAngle));
                }
                else
                {
                    mpCustomShapePropertiesPtr->setTextRotateAngle(nTextRotateAngle);
                }

                auto sHorzOverflow = getTextBody()->getTextProperties().msHorzOverflow;
                if (!sHorzOverflow.isEmpty())
                    putPropertyToGrabBag("horzOverflow", uno::makeAny(getTextBody()->getTextProperties().msHorzOverflow));
                auto nVertOverflow = getTextBody()->getTextProperties().msVertOverflow;
                if (!nVertOverflow.isEmpty())
                    putPropertyToGrabBag("vertOverflow", uno::makeAny(getTextBody()->getTextProperties().msVertOverflow));
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

        // tdf#133037: a bit hackish: force Shape to rotate in the opposite direction the camera would rotate
        const sal_Int32 nCameraRotation = get3DProperties().maCameraRotation.mnRevolution.get(0);

        PropertySet aPropertySet(mxShape);
        if ( !bUseRotationTransform && (mnRotation != 0 || nCameraRotation != 0) )
        {
            // use the same logic for rotation from VML exporter (SimpleShape::implConvertAndInsert at vmlshape.cxx)
            Degree100 nAngle = NormAngle36000( Degree100((mnRotation - nCameraRotation) / -600) );
            aPropertySet.setAnyProperty( PROP_RotateAngle, makeAny( sal_Int32( nAngle.get() ) ) );
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
                            aPropertySet.setAnyProperty(PROP_ParaAdjust, uno::makeAny(eAdjust));
                        }
                    }
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
                aPropertySet.setAnyProperty(PROP_CharColor, uno::makeAny(nCharColor));
            }
        }

        // Set glow effect properties
        if ( aEffectProperties.maGlow.moGlowRad.has() )
        {
            uno::Reference<beans::XPropertySet> propertySet (mxShape, uno::UNO_QUERY);
            propertySet->setPropertyValue("GlowEffectRadius", makeAny(convertEmuToHmm(aEffectProperties.maGlow.moGlowRad.get())));
            propertySet->setPropertyValue("GlowEffectColor", makeAny(aEffectProperties.maGlow.moGlowColor.getColor(rGraphicHelper)));
            propertySet->setPropertyValue("GlowEffectTransparency", makeAny(aEffectProperties.maGlow.moGlowColor.getTransparency()));
        }

        // Set soft edge effect properties
        if (aEffectProperties.maSoftEdge.moRad.has())
        {
            uno::Reference<beans::XPropertySet> propertySet(mxShape, uno::UNO_QUERY);
            propertySet->setPropertyValue(
                "SoftEdgeRadius", makeAny(convertEmuToHmm(aEffectProperties.maSoftEdge.moRad.get())));
        }
    }

    if( mxShape.is() )
        finalizeXShape( rFilterBase, rxShapes );

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

        if (mpDiagramData)
        {
            if (SdrObject* pObj = SdrObject::getSdrObjectFromXShape(mxShape))
                pObj->SetDiagramData(mpDiagramData);
        }

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

void Shape::syncDiagramFontHeights()
{
    // Each name represents a group of shapes, for which the font height should have the same
    // scaling.
    for (const auto& rNameAndPairs : maDiagramFontHeights)
    {
        // Find out the minimum scale within this group.
        const ShapePairs& rShapePairs = rNameAndPairs.second;
        sal_Int16 nMinScale = 100;
        for (const auto& rShapePair : rShapePairs)
        {
            uno::Reference<beans::XPropertySet> xPropertySet(rShapePair.second, uno::UNO_QUERY);
            if (xPropertySet.is())
            {
                sal_Int16 nTextFitToSizeScale = 0;
                xPropertySet->getPropertyValue("TextFitToSizeScale") >>= nTextFitToSizeScale;
                if (nTextFitToSizeScale > 0 && nTextFitToSizeScale < nMinScale)
                {
                    nMinScale = nTextFitToSizeScale;
                }
            }
        }

        // Set that minimum scale for all members of the group.
        if (nMinScale < 100)
        {
            for (const auto& rShapePair : rShapePairs)
            {
                uno::Reference<beans::XPropertySet> xPropertySet(rShapePair.second, uno::UNO_QUERY);
                if (xPropertySet.is())
                {
                    xPropertySet->setPropertyValue("TextFitToSizeScale", uno::makeAny(nMinScale));
                }
            }
        }
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
                bool bMSO2007Doc = rFilter.isMSO2007Document();
                chart::ChartSpaceModel aModel(bMSO2007Doc);
                rtl::Reference<chart::ChartSpaceFragment> pChartSpaceFragment = new chart::ChartSpaceFragment(
                        rFilter, mxChartShapeInfo->maFragmentPath, aModel );
                const OUString aThemeOverrideFragmentPath( pChartSpaceFragment->
                        getFragmentPathFromFirstTypeFromOfficeDoc(u"themeOverride") );
                rFilter.importFragment( pChartSpaceFragment );
                ::oox::ppt::PowerPointImport *pPowerPointImport =
                    dynamic_cast< ::oox::ppt::PowerPointImport* >(&rFilter);

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
                        new ThemeOverrideFragmentHandler(rFilter, aThemeOverrideFragmentPath,
                                                         *pThemeOverride),
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

                if (!aThemeOverrideFragmentPath.isEmpty() && pPowerPointImport)
                {
                    // Restore the original theme.
                    pPowerPointImport->getActualSlidePersist()->setTheme(pTheme);
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
