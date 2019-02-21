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

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b3dpolypolygon.hxx>
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#include <basegfx/tuple/b2dtuple.hxx>
#include <basegfx/vector/b3dvector.hxx>

#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XIdentifierAccess.hpp>
#include <com/sun/star/container/XIdentifierContainer.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/drawing/Alignment.hpp>
#include <com/sun/star/drawing/CameraGeometry.hpp>
#include <com/sun/star/drawing/CircleKind.hpp>
#include <com/sun/star/drawing/ConnectorType.hpp>
#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/drawing/DoubleSequence.hpp>
#include <com/sun/star/drawing/EscapeDirection.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeGluePointType.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterType.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegmentCommand.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeTextFrame.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeTextPathMode.hpp>
#include <com/sun/star/drawing/GluePoint2.hpp>
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/PolyPolygonShape3D.hpp>
#include <com/sun/star/drawing/Position3D.hpp>
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/drawing/XCustomShapeEngine.hpp>
#include <com/sun/star/drawing/XGluePointsSupplier.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/io/XSeekableInputStream.hpp>
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/media/ZoomLevel.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>

#include <comphelper/classids.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
#include <officecfg/Office/Common.hxx>

#include <o3tl/any.hxx>
#include <o3tl/typed_flags_set.hxx>

#include <rtl/math.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>

#include <sax/tools/converter.hxx>

#include <tools/debug.hxx>
#include <tools/globname.hxx>
#include <tools/helpers.hxx>
#include <tools/urlobj.hxx>
#include <tools/diagnose_ex.h>

#include <xmloff/contextid.hxx>
#include <xmloff/families.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/shapeexport.hxx>
#include <xmloff/unointerfacetouniqueidentifiermapper.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/table/XMLTableExport.hxx>
#include <xmloff/ProgressBarHelper.hxx>

#include <anim.hxx>
#include <EnhancedCustomShapeToken.hxx>
#include <PropertySetMerger.hxx>
#include "sdpropls.hxx"
#include "sdxmlexp_impl.hxx"
#include <xexptran.hxx>
#include "ximpshap.hxx"
#include <XMLBase64Export.hxx>
#include <XMLImageMapExport.hxx>
#include <memory>

#include <config_features.h>

using namespace ::com::sun::star;
using namespace ::xmloff::EnhancedCustomShapeToken;
using namespace ::xmloff::token;

#define XML_EMBEDDEDOBJECTGRAPHIC_URL_BASE "vnd.sun.star.GraphicObject:"

namespace {

bool supportsText(XmlShapeType eShapeType)
{
        return eShapeType != XmlShapeTypePresChartShape &&
        eShapeType != XmlShapeTypePresOLE2Shape &&
        eShapeType != XmlShapeTypeDrawSheetShape &&
        eShapeType != XmlShapeTypePresSheetShape &&
        eShapeType != XmlShapeTypeDraw3DSceneObject &&
        eShapeType != XmlShapeTypeDraw3DCubeObject &&
        eShapeType != XmlShapeTypeDraw3DSphereObject &&
        eShapeType != XmlShapeTypeDraw3DLatheObject &&
        eShapeType != XmlShapeTypeDraw3DExtrudeObject &&
        eShapeType != XmlShapeTypeDrawPageShape &&
        eShapeType != XmlShapeTypePresPageShape &&
        eShapeType != XmlShapeTypeDrawGroupShape;

}

}

static const OUStringLiteral gsZIndex( "ZOrder" );
static const OUStringLiteral gsPrintable( "Printable" );
static const OUStringLiteral gsVisible( "Visible" );
static const OUStringLiteral gsModel( "Model" );
static const OUStringLiteral gsStartShape( "StartShape" );
static const OUStringLiteral gsEndShape( "EndShape" );
static const OUStringLiteral gsOnClick( "OnClick" );
static const OUStringLiteral gsEventType( "EventType" );
static const OUStringLiteral gsPresentation( "Presentation" );
static const OUStringLiteral gsMacroName( "MacroName" );
static const OUStringLiteral gsScript( "Script" );
static const OUStringLiteral gsLibrary( "Library" );
static const OUStringLiteral gsClickAction( "ClickAction" );
static const OUStringLiteral gsBookmark( "Bookmark" );
static const OUStringLiteral gsEffect( "Effect" );
static const OUStringLiteral gsPlayFull( "PlayFull" );
static const OUStringLiteral gsVerb( "Verb" );
static const OUStringLiteral gsSoundURL( "SoundURL" );
static const OUStringLiteral gsSpeed( "Speed" );
static const OUStringLiteral gsStarBasic( "StarBasic" );

XMLShapeExport::XMLShapeExport(SvXMLExport& rExp,
                                SvXMLExportPropertyMapper *pExtMapper )
:   mrExport( rExp ),
    maShapesInfos(),
    maCurrentShapesIter(maShapesInfos.end()),
    mbExportLayer( false ),
    // #88546# init to sal_False
    mbHandleProgressBar( false )
{
    // construct PropertySetMapper
    mxPropertySetMapper = CreateShapePropMapper( mrExport );
    if( pExtMapper )
    {
        rtl::Reference < SvXMLExportPropertyMapper > xExtMapper( pExtMapper );
        mxPropertySetMapper->ChainExportMapper( xExtMapper );
    }

/*
    // chain text attributes
    xPropertySetMapper->ChainExportMapper(XMLTextParagraphExport::CreateParaExtPropMapper(rExp));
*/

    mrExport.GetAutoStylePool()->AddFamily(
        XML_STYLE_FAMILY_SD_GRAPHICS_ID,
        XML_STYLE_FAMILY_SD_GRAPHICS_NAME,
        GetPropertySetMapper(),
        XML_STYLE_FAMILY_SD_GRAPHICS_PREFIX);
    mrExport.GetAutoStylePool()->AddFamily(
        XML_STYLE_FAMILY_SD_PRESENTATION_ID,
        XML_STYLE_FAMILY_SD_PRESENTATION_NAME,
        GetPropertySetMapper(),
        XML_STYLE_FAMILY_SD_PRESENTATION_PREFIX);

    // create table export helper and let him add his families in time
    GetShapeTableExport();
}

XMLShapeExport::~XMLShapeExport()
{
}

// sj: replacing CustomShapes with standard objects that are also supported in OpenOffice.org format
uno::Reference< drawing::XShape > XMLShapeExport::checkForCustomShapeReplacement( const uno::Reference< drawing::XShape >& xShape )
{
    uno::Reference< drawing::XShape > xCustomShapeReplacement;

    if( !( GetExport().getExportFlags() & SvXMLExportFlags::OASIS ) )
    {
        OUString aType( xShape->getShapeType() );
        if( aType == "com.sun.star.drawing.CustomShape" )
        {
            uno::Reference< beans::XPropertySet > xSet( xShape, uno::UNO_QUERY );
            if( xSet.is() )
            {
                OUString aEngine;
                xSet->getPropertyValue("CustomShapeEngine") >>= aEngine;
                if ( aEngine.isEmpty() )
                {
                    aEngine = "com.sun.star.drawing.EnhancedCustomShapeEngine";
                }
                uno::Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );

                if ( !aEngine.isEmpty() )
                {
                    uno::Sequence< uno::Any > aArgument( 1 );
                    uno::Sequence< beans::PropertyValue > aPropValues( 2 );
                    aPropValues[ 0 ].Name = "CustomShape";
                    aPropValues[ 0 ].Value <<= xShape;
                    aPropValues[ 1 ].Name = "ForceGroupWithText";
                    aPropValues[ 1 ].Value <<= true;
                    aArgument[ 0 ] <<= aPropValues;
                    uno::Reference< uno::XInterface > xInterface(
                        xContext->getServiceManager()->createInstanceWithArgumentsAndContext(aEngine, aArgument, xContext) );
                    if ( xInterface.is() )
                    {
                        uno::Reference< drawing::XCustomShapeEngine > xCustomShapeEngine(
                            uno::Reference< drawing::XCustomShapeEngine >( xInterface, uno::UNO_QUERY ) );
                        if ( xCustomShapeEngine.is() )
                            xCustomShapeReplacement = xCustomShapeEngine->render();
                    }
                }
            }
        }
    }
    return xCustomShapeReplacement;
}

// This method collects all automatic styles for the given XShape
void XMLShapeExport::collectShapeAutoStyles(const uno::Reference< drawing::XShape >& xShape )
{
    if( maCurrentShapesIter == maShapesInfos.end() )
    {
        OSL_FAIL( "XMLShapeExport::collectShapeAutoStyles(): no call to seekShapes()!" );
        return;
    }
    sal_Int32 nZIndex = 0;
    uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if( xPropSet.is() )
        xPropSet->getPropertyValue(gsZIndex) >>= nZIndex;

    ImplXMLShapeExportInfoVector& aShapeInfoVector = (*maCurrentShapesIter).second;

    if( static_cast<sal_Int32>(aShapeInfoVector.size()) <= nZIndex )
    {
        OSL_FAIL( "XMLShapeExport::collectShapeAutoStyles(): no shape info allocated for a given shape" );
        return;
    }

    ImplXMLShapeExportInfo& aShapeInfo = aShapeInfoVector[nZIndex];

    uno::Reference< drawing::XShape > xCustomShapeReplacement = checkForCustomShapeReplacement( xShape );
    if ( xCustomShapeReplacement.is() )
        aShapeInfo.xCustomShapeReplacement = xCustomShapeReplacement;

    // first compute the shapes type
    ImpCalcShapeType(xShape, aShapeInfo.meShapeType);

    // #i118485# enabled XmlShapeTypeDrawChartShape and XmlShapeTypeDrawOLE2Shape
    // to have text
    const bool bObjSupportsText =
        supportsText(aShapeInfo.meShapeType);

    const bool bObjSupportsStyle =
        aShapeInfo.meShapeType != XmlShapeTypeDrawGroupShape;

    bool bIsEmptyPresObj = false;

    if ( aShapeInfo.xCustomShapeReplacement.is() )
        xPropSet.clear();

    // prep text styles
    if( xPropSet.is() && bObjSupportsText )
    {
        uno::Reference< text::XText > xText(xShape, uno::UNO_QUERY);
        bool bSkip = false;
        if (xText.is())
        {
            try
            {
                bSkip = xText->getString().isEmpty();
            }
            catch (uno::RuntimeException const&)
            {
                // tdf#102479: SwXTextFrame that contains only a table will
                // throw, but the table must be iterated so that
                // SwXMLExport::ExportTableLines() can find its auto styles
                // so do not skip it!
            }
        }
        if (!bSkip)
        {
            uno::Reference< beans::XPropertySetInfo > xPropSetInfo( xPropSet->getPropertySetInfo() );

            if( xPropSetInfo.is() && xPropSetInfo->hasPropertyByName("IsEmptyPresentationObject") )
            {
                uno::Any aAny = xPropSet->getPropertyValue("IsEmptyPresentationObject");
                aAny >>= bIsEmptyPresObj;
            }

            if(!bIsEmptyPresObj)
            {
                GetExport().GetTextParagraphExport()->collectTextAutoStyles( xText );
            }
        }
    }

    // compute the shape parent style
    if( xPropSet.is() )
    {
        uno::Reference< beans::XPropertySetInfo > xPropertySetInfo( xPropSet->getPropertySetInfo() );

        OUString aParentName;
        uno::Reference< style::XStyle > xStyle;

        if( bObjSupportsStyle )
        {
            if( xPropertySetInfo.is() && xPropertySetInfo->hasPropertyByName("Style") )
                xPropSet->getPropertyValue("Style") >>= xStyle;

            if(xStyle.is())
            {
                // get family ID
                uno::Reference< beans::XPropertySet > xStylePropSet(xStyle, uno::UNO_QUERY);
                SAL_WARN_IF( !xStylePropSet.is(), "xmloff", "style without a XPropertySet?" );
                try
                {
                    if(xStylePropSet.is())
                    {
                        OUString aFamilyName;
                        xStylePropSet->getPropertyValue("Family") >>= aFamilyName;
                        if( !aFamilyName.isEmpty() && aFamilyName != "graphics" )
                            aShapeInfo.mnFamily = XML_STYLE_FAMILY_SD_PRESENTATION_ID;
                    }
                }
                catch(const beans::UnknownPropertyException&)
                {
                    // Ignored.
                    SAL_WARN( "xmloff",
                        "XMLShapeExport::collectShapeAutoStyles: style has no 'Family' property");
                }

                // get parent-style name
                if(XML_STYLE_FAMILY_SD_PRESENTATION_ID == aShapeInfo.mnFamily)
                {
                    aParentName = msPresentationStylePrefix;
                }

                aParentName += xStyle->getName();
            }
        }

        if (aParentName.isEmpty() && xPropertySetInfo->hasPropertyByName("TextBox") && xPropSet->getPropertyValue("TextBox").hasValue() && xPropSet->getPropertyValue("TextBox").get<bool>())
        {
            // Shapes with a Writer TextBox always have a parent style.
            // If there would be none, then just assign the first available.
            uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(GetExport().GetModel(), uno::UNO_QUERY);
            if (xStyleFamiliesSupplier.is()) // tdf#108231
            {
                uno::Reference<container::XNameAccess> xStyleFamilies = xStyleFamiliesSupplier->getStyleFamilies();
                uno::Reference<container::XNameAccess> xFrameStyles = xStyleFamilies->getByName("FrameStyles").get< uno::Reference<container::XNameAccess> >();
                uno::Sequence<OUString> aFrameStyles = xFrameStyles->getElementNames();
                if (aFrameStyles.hasElements())
                {
                    aParentName = aFrameStyles[0];
                }
            }
        }

        // filter propset
        std::vector< XMLPropertyState > aPropStates;

        sal_Int32 nCount = 0;
        if( !bIsEmptyPresObj || (aShapeInfo.meShapeType != XmlShapeTypePresPageShape) )
        {
            aPropStates = GetPropertySetMapper()->Filter( xPropSet );

            if (XmlShapeTypeDrawControlShape == aShapeInfo.meShapeType)
            {
                // for control shapes, we additionally need the number format style (if any)
                uno::Reference< drawing::XControlShape > xControl(xShape, uno::UNO_QUERY);
                DBG_ASSERT(xControl.is(), "XMLShapeExport::collectShapeAutoStyles: ShapeType control, but no XControlShape!");
                if (xControl.is())
                {
                    uno::Reference< beans::XPropertySet > xControlModel(xControl->getControl(), uno::UNO_QUERY);
                    DBG_ASSERT(xControlModel.is(), "XMLShapeExport::collectShapeAutoStyles: no control model on the control shape!");

                    OUString sNumberStyle = mrExport.GetFormExport()->getControlNumberStyle(xControlModel);
                    if (!sNumberStyle.isEmpty())
                    {
                        sal_Int32 nIndex = GetPropertySetMapper()->getPropertySetMapper()->FindEntryIndex(CTF_SD_CONTROL_SHAPE_DATA_STYLE);
                            // TODO : this retrieval of the index could be moved into the ctor, holding the index
                            //          as member, thus saving time.
                        DBG_ASSERT(-1 != nIndex, "XMLShapeExport::collectShapeAutoStyles: could not obtain the index for our context id!");

                        XMLPropertyState aNewState(nIndex, uno::makeAny(sNumberStyle));
                        aPropStates.push_back(aNewState);
                    }
                }
            }

            nCount = std::count_if(aPropStates.cbegin(), aPropStates.cend(),
                [](const XMLPropertyState& rProp) { return rProp.mnIndex != -1; });
        }

        if(nCount == 0)
        {
            // no hard attributes, use parent style name for export
            aShapeInfo.msStyleName = aParentName;
        }
        else
        {
            // there are filtered properties -> hard attributes
            // try to find this style in AutoStylePool
            aShapeInfo.msStyleName = mrExport.GetAutoStylePool()->Find(aShapeInfo.mnFamily, aParentName, aPropStates);

            if(aShapeInfo.msStyleName.isEmpty())
            {
                // Style did not exist, add it to AutoStalePool
                aShapeInfo.msStyleName = mrExport.GetAutoStylePool()->Add(aShapeInfo.mnFamily, aParentName, aPropStates);
            }
        }

        // optionally generate auto style for text attributes
        if( (!bIsEmptyPresObj || (aShapeInfo.meShapeType != XmlShapeTypePresPageShape)) && bObjSupportsText )
        {
            aPropStates = GetExport().GetTextParagraphExport()->GetParagraphPropertyMapper()->Filter( xPropSet );

            // yet more additionally, we need to care for the ParaAdjust property
            if ( XmlShapeTypeDrawControlShape == aShapeInfo.meShapeType )
            {
                uno::Reference< beans::XPropertySetInfo > xPropSetInfo( xPropSet->getPropertySetInfo() );
                uno::Reference< beans::XPropertyState > xPropState( xPropSet, uno::UNO_QUERY );
                if ( xPropSetInfo.is() && xPropState.is() )
                {
                    // this is because:
                    // * if controls shapes have a ParaAdjust property, then this is the Align property of the control model
                    // * control models are allowed to have an Align of "void"
                    // * the Default for control model's Align is TextAlign_LEFT
                    // * defaults for style properties are not written, but we need to write the "left",
                    //   because we need to distinguish this "left" from the case where not align attribute
                    //   is present which means "void"
                    if  (   xPropSetInfo->hasPropertyByName( "ParaAdjust" )
                        &&  ( beans::PropertyState_DEFAULT_VALUE == xPropState->getPropertyState( "ParaAdjust" ) )
                        )
                    {
                        sal_Int32 nIndex = GetExport().GetTextParagraphExport()->GetParagraphPropertyMapper()->getPropertySetMapper()->FindEntryIndex( CTF_SD_SHAPE_PARA_ADJUST );
                            // TODO : this retrieval of the index should be moved into the ctor, holding the index
                            //          as member, thus saving time.
                        DBG_ASSERT(-1 != nIndex, "XMLShapeExport::collectShapeAutoStyles: could not obtain the index for the ParaAdjust context id!");

                        uno::Any aParaAdjustValue = xPropSet->getPropertyValue( "ParaAdjust" );
                        XMLPropertyState aAlignDefaultState( nIndex, aParaAdjustValue );

                        aPropStates.push_back( aAlignDefaultState );
                    }
                }
            }

            nCount = std::count_if(aPropStates.cbegin(), aPropStates.cend(),
                [](const XMLPropertyState& rProp) { return rProp.mnIndex != -1; });

            if( nCount )
            {
                aShapeInfo.msTextStyleName = mrExport.GetAutoStylePool()->Find( XML_STYLE_FAMILY_TEXT_PARAGRAPH, "", aPropStates );
                if(aShapeInfo.msTextStyleName.isEmpty())
                {
                    // Style did not exist, add it to AutoStalePool
                    aShapeInfo.msTextStyleName = mrExport.GetAutoStylePool()->Add(XML_STYLE_FAMILY_TEXT_PARAGRAPH, "", aPropStates);
                }
            }
        }
    }

    // prepare animation information if needed
    if( mxAnimationsExporter.is() )
        XMLAnimationsExporter::prepare( xShape );

    // check for special shapes

    switch( aShapeInfo.meShapeType )
    {
        case XmlShapeTypeDrawConnectorShape:
        {
            uno::Reference< uno::XInterface > xConnection;

            // create shape ids for export later
            xPropSet->getPropertyValue( gsStartShape ) >>= xConnection;
            if( xConnection.is() )
                mrExport.getInterfaceToIdentifierMapper().registerReference( xConnection );

            xPropSet->getPropertyValue( gsEndShape ) >>= xConnection;
            if( xConnection.is() )
                mrExport.getInterfaceToIdentifierMapper().registerReference( xConnection );
            break;
        }
        case XmlShapeTypePresTableShape:
        case XmlShapeTypeDrawTableShape:
        {
            try
            {
                uno::Reference< table::XColumnRowRange > xRange( xPropSet->getPropertyValue( gsModel ), uno::UNO_QUERY_THROW );
                GetShapeTableExport()->collectTableAutoStyles( xRange );
            }
            catch(const uno::Exception&)
            {
                DBG_UNHANDLED_EXCEPTION( "xmloff", "collecting auto styles for a table" );
            }
            break;
        }
        default:
            break;
    }

    maShapeInfos.push_back( aShapeInfo );

    // check for shape collections (group shape or 3d scene)
    // and collect contained shapes style infos
    const uno::Reference< drawing::XShape >& xCollection = aShapeInfo.xCustomShapeReplacement.is()
                                                ? aShapeInfo.xCustomShapeReplacement : xShape;
    {
        uno::Reference< drawing::XShapes > xShapes( xCollection, uno::UNO_QUERY );
        if( xShapes.is() )
        {
            collectShapesAutoStyles( xShapes );
        }
    }
}

namespace
{
    class NewTextListsHelper
    {
        public:
            explicit NewTextListsHelper( SvXMLExport& rExp )
                : mrExport( rExp )
            {
                mrExport.GetTextParagraphExport()->PushNewTextListsHelper();
            }

            ~NewTextListsHelper()
            {
                mrExport.GetTextParagraphExport()->PopTextListsHelper();
            }

        private:
            SvXMLExport& mrExport;
    };
}
// This method exports the given XShape
void XMLShapeExport::exportShape(const uno::Reference< drawing::XShape >& xShape,
                                 XMLShapeExportFlags nFeatures /* = SEF_DEFAULT */,
                                 css::awt::Point* pRefPoint /* = NULL */,
                                 SvXMLAttributeList* pAttrList /* = NULL */ )
{
    SAL_INFO("xmloff", xShape->getShapeType());
    if( maCurrentShapesIter == maShapesInfos.end() )
    {
        SAL_WARN( "xmloff", "XMLShapeExport::exportShape(): no auto styles where collected before export" );
        return;
    }
    sal_Int32 nZIndex = 0;
    uno::Reference< beans::XPropertySet > xSet( xShape, uno::UNO_QUERY );

    std::unique_ptr< SvXMLElementExport >  pHyperlinkElement;

    // export hyperlinks with <a><shape/></a>. Currently only in draw since draw
    // does not support document events
    if( xSet.is() && (GetExport().GetModelType() == SvtModuleOptions::EFactory::DRAW) ) try
    {
        presentation::ClickAction eAction = presentation::ClickAction_NONE;
        xSet->getPropertyValue("OnClick") >>= eAction;

        if( (eAction == presentation::ClickAction_DOCUMENT) ||
            (eAction == presentation::ClickAction_BOOKMARK) )
        {
            OUString sURL;
            xSet->getPropertyValue(gsBookmark) >>= sURL;

            if( !sURL.isEmpty() )
            {
                mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, sURL );
                mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
                mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
                pHyperlinkElement.reset( new SvXMLElementExport(mrExport, XML_NAMESPACE_DRAW, XML_A, true, true) );
            }
        }
    }
    catch(const uno::Exception&)
    {
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN("xmloff", "XMLShapeExport::exportShape(): exception during hyperlink export " << exceptionToString(ex));
    }

    if( xSet.is() )
        xSet->getPropertyValue(gsZIndex) >>= nZIndex;

    ImplXMLShapeExportInfoVector& aShapeInfoVector = (*maCurrentShapesIter).second;

    if( static_cast<sal_Int32>(aShapeInfoVector.size()) <= nZIndex )
    {
        SAL_WARN( "xmloff", "XMLShapeExport::exportShape(): no shape info collected for a given shape" );
        return;
    }

    NewTextListsHelper aNewTextListsHelper( mrExport );

    const ImplXMLShapeExportInfo& aShapeInfo = aShapeInfoVector[nZIndex];

#ifdef DBG_UTIL
    // check if this is the correct ShapesInfo
    uno::Reference< container::XChild > xChild( xShape, uno::UNO_QUERY );
    if( xChild.is() )
    {
        uno::Reference< drawing::XShapes > xParent( xChild->getParent(), uno::UNO_QUERY );
        SAL_WARN_IF( !xParent.is() && xParent.get() == (*maCurrentShapesIter).first.get(), "xmloff", "XMLShapeExport::exportShape(): Wrong call to XMLShapeExport::seekShapes()" );
    }

    // first compute the shapes type
    {
        XmlShapeType eShapeType(XmlShapeTypeNotYetSet);
        ImpCalcShapeType(xShape, eShapeType);

        SAL_WARN_IF( eShapeType != aShapeInfo.meShapeType, "xmloff", "exportShape callings do not correspond to collectShapeAutoStyles calls!: " << xShape->getShapeType() );
    }
#endif

    // collect animation information if needed
    if( mxAnimationsExporter.is() )
        mxAnimationsExporter->collect( xShape, mrExport );

    /* Export shapes name if he has one (#i51726#)
       Export of the shape name for text documents only if the OpenDocument
       file format is written - exceptions are group shapes.
       Note: Writer documents in OpenOffice.org file format doesn't contain
             any names for shapes, except for group shapes.
    */
    {
        if ( ( GetExport().GetModelType() != SvtModuleOptions::EFactory::WRITER &&
               GetExport().GetModelType() != SvtModuleOptions::EFactory::WRITERWEB &&
               GetExport().GetModelType() != SvtModuleOptions::EFactory::WRITERGLOBAL ) ||
             ( GetExport().getExportFlags() & SvXMLExportFlags::OASIS ) ||
             aShapeInfo.meShapeType == XmlShapeTypeDrawGroupShape ||
             ( aShapeInfo.meShapeType == XmlShapeTypeDrawCustomShape &&
               aShapeInfo.xCustomShapeReplacement.is() ) )
        {
            uno::Reference< container::XNamed > xNamed( xShape, uno::UNO_QUERY );
            if( xNamed.is() )
            {
                const OUString aName( xNamed->getName() );
                if( !aName.isEmpty() )
                    mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_NAME, aName );
            }
        }
    }

    // export style name
    if( !aShapeInfo.msStyleName.isEmpty() )
    {
        if(XML_STYLE_FAMILY_SD_GRAPHICS_ID == aShapeInfo.mnFamily)
            mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_STYLE_NAME, mrExport.EncodeStyleName( aShapeInfo.msStyleName) );
        else
            mrExport.AddAttribute(XML_NAMESPACE_PRESENTATION, XML_STYLE_NAME, mrExport.EncodeStyleName( aShapeInfo.msStyleName) );
    }

    // export text style name
    if( !aShapeInfo.msTextStyleName.isEmpty() )
    {
        mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_TEXT_STYLE_NAME, aShapeInfo.msTextStyleName );
    }

    // export shapes id if needed
    {
        uno::Reference< uno::XInterface > xRef( xShape, uno::UNO_QUERY );
        const OUString& rShapeId = mrExport.getInterfaceToIdentifierMapper().getIdentifier( xRef );
        if( !rShapeId.isEmpty() )
        {
            mrExport.AddAttributeIdLegacy(XML_NAMESPACE_DRAW, rShapeId);
        }
    }

    // export layer information
    if( mbExportLayer )
    {
        // check for group or scene shape and not export layer if this is one
        uno::Reference< drawing::XShapes > xShapes( xShape, uno::UNO_QUERY );
        if( !xShapes.is() )
        {
            try
            {
                uno::Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY );
                OUString aLayerName;
                xProps->getPropertyValue("LayerName") >>= aLayerName;
                mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_LAYER, aLayerName );

            }
            catch(const uno::Exception&)
            {
                DBG_UNHANDLED_EXCEPTION( "xmloff", "exporting layer name for shape" );
            }
        }
    }

    // export draw:display (do not export in ODF 1.2 or older)
    if( xSet.is() && ( mrExport.getDefaultVersion() > SvtSaveOptions::ODFVER_012 ) )
    {
        if( aShapeInfo.meShapeType != XmlShapeTypeDrawPageShape && aShapeInfo.meShapeType != XmlShapeTypePresPageShape &&
            aShapeInfo.meShapeType != XmlShapeTypeHandoutShape && aShapeInfo.meShapeType != XmlShapeTypeDrawChartShape )

        try
        {
            bool bVisible = true;
            bool bPrintable = true;

            xSet->getPropertyValue(gsVisible) >>= bVisible;
            xSet->getPropertyValue(gsPrintable) >>= bPrintable;

            XMLTokenEnum eDisplayToken = XML_TOKEN_INVALID;
            const unsigned short nDisplay = (bVisible ? 2 : 0) | (bPrintable ? 1 : 0);
            switch( nDisplay )
            {
            case 0: eDisplayToken = XML_NONE; break;
            case 1: eDisplayToken = XML_PRINTER; break;
            case 2: eDisplayToken = XML_SCREEN; break;
            // case 3: eDisplayToken = XML_ALWAYS break; this is the default
            }

            if( eDisplayToken != XML_TOKEN_INVALID )
                mrExport.AddAttribute(XML_NAMESPACE_DRAW_EXT, XML_DISPLAY, eDisplayToken );
        }
        catch(const uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("xmloff.draw");
        }
    }

    // #82003# test export count
    // #91587# ALWAYS increment since now ALL to be exported shapes are counted.
    if(mrExport.GetShapeExport()->IsHandleProgressBarEnabled())
    {
        mrExport.GetProgressBarHelper()->Increment();
    }

    onExport( xShape );

    // export shape element
    switch(aShapeInfo.meShapeType)
    {
        case XmlShapeTypeDrawRectangleShape:
        {
            ImpExportRectangleShape(xShape, nFeatures, pRefPoint );
            break;
        }
        case XmlShapeTypeDrawEllipseShape:
        {
            ImpExportEllipseShape(xShape, nFeatures, pRefPoint );
            break;
        }
        case XmlShapeTypeDrawLineShape:
        {
            ImpExportLineShape(xShape, nFeatures, pRefPoint );
            break;
        }
        case XmlShapeTypeDrawPolyPolygonShape:  // closed PolyPolygon
        case XmlShapeTypeDrawPolyLineShape:     // open PolyPolygon
        case XmlShapeTypeDrawClosedBezierShape: // closed tools::PolyPolygon containing curves
        case XmlShapeTypeDrawOpenBezierShape:   // open tools::PolyPolygon containing curves
        {
            ImpExportPolygonShape(xShape, aShapeInfo.meShapeType, nFeatures, pRefPoint );
            break;
        }

        case XmlShapeTypeDrawTextShape:
        case XmlShapeTypePresTitleTextShape:
        case XmlShapeTypePresOutlinerShape:
        case XmlShapeTypePresSubtitleShape:
        case XmlShapeTypePresNotesShape:
        case XmlShapeTypePresHeaderShape:
        case XmlShapeTypePresFooterShape:
        case XmlShapeTypePresSlideNumberShape:
        case XmlShapeTypePresDateTimeShape:
        {
            ImpExportTextBoxShape(xShape, aShapeInfo.meShapeType, nFeatures, pRefPoint );
            break;
        }

        case XmlShapeTypeDrawGraphicObjectShape:
        case XmlShapeTypePresGraphicObjectShape:
        {
            ImpExportGraphicObjectShape(xShape, aShapeInfo.meShapeType, nFeatures, pRefPoint );
            break;
        }

        case XmlShapeTypeDrawChartShape:
        case XmlShapeTypePresChartShape:
        {
            ImpExportChartShape(xShape, aShapeInfo.meShapeType, nFeatures, pRefPoint, pAttrList );
            break;
        }

        case XmlShapeTypeDrawControlShape:
        {
            ImpExportControlShape(xShape, nFeatures, pRefPoint );
            break;
        }

        case XmlShapeTypeDrawConnectorShape:
        {
            ImpExportConnectorShape(xShape, nFeatures, pRefPoint );
            break;
        }

        case XmlShapeTypeDrawMeasureShape:
        {
            ImpExportMeasureShape(xShape, nFeatures, pRefPoint );
            break;
        }

        case XmlShapeTypeDrawOLE2Shape:
        case XmlShapeTypePresOLE2Shape:
        case XmlShapeTypeDrawSheetShape:
        case XmlShapeTypePresSheetShape:
        {
            ImpExportOLE2Shape(xShape, aShapeInfo.meShapeType, nFeatures, pRefPoint );
            break;
        }

        case XmlShapeTypePresTableShape:
        case XmlShapeTypeDrawTableShape:
        {
            ImpExportTableShape( xShape, aShapeInfo.meShapeType, nFeatures, pRefPoint );
            break;
        }

        case XmlShapeTypeDrawPageShape:
        case XmlShapeTypePresPageShape:
        case XmlShapeTypeHandoutShape:
        {
            ImpExportPageShape(xShape, aShapeInfo.meShapeType, nFeatures, pRefPoint );
            break;
        }

        case XmlShapeTypeDrawCaptionShape:
        {
            ImpExportCaptionShape(xShape, nFeatures, pRefPoint );
            break;
        }

        case XmlShapeTypeDraw3DCubeObject:
        case XmlShapeTypeDraw3DSphereObject:
        case XmlShapeTypeDraw3DLatheObject:
        case XmlShapeTypeDraw3DExtrudeObject:
        {
            ImpExport3DShape(xShape, aShapeInfo.meShapeType);
            break;
        }

        case XmlShapeTypeDraw3DSceneObject:
        {
            ImpExport3DSceneShape( xShape, nFeatures, pRefPoint );
            break;
        }

        case XmlShapeTypeDrawGroupShape:
        {
            // empty group
            ImpExportGroupShape( xShape, nFeatures, pRefPoint );
            break;
        }

        case XmlShapeTypeDrawFrameShape:
        {
            ImpExportFrameShape(xShape, nFeatures, pRefPoint );
            break;
        }

        case XmlShapeTypeDrawAppletShape:
        {
            ImpExportAppletShape(xShape, nFeatures, pRefPoint );
            break;
        }

        case XmlShapeTypeDrawPluginShape:
        {
            ImpExportPluginShape(xShape, nFeatures, pRefPoint );
            break;
        }

        case XmlShapeTypeDrawCustomShape:
        {
            if ( aShapeInfo.xCustomShapeReplacement.is() )
                ImpExportGroupShape( aShapeInfo.xCustomShapeReplacement, nFeatures, pRefPoint );
            else
                ImpExportCustomShape( xShape, nFeatures, pRefPoint );
            break;
        }

        case XmlShapeTypePresMediaShape:
        case XmlShapeTypeDrawMediaShape:
        {
            ImpExportMediaShape( xShape, aShapeInfo.meShapeType, nFeatures, pRefPoint );
            break;
        }

        case XmlShapeTypePresOrgChartShape:
        case XmlShapeTypeUnknown:
        case XmlShapeTypeNotYetSet:
        default:
        {
            // this should never happen and is an error
            OSL_FAIL("XMLEXP: WriteShape: unknown or unexpected type of shape in export!");
            break;
        }
    }

    pHyperlinkElement.reset();

    // #97489# #97111#
    // if there was an error and no element for the shape was exported
    // we need to clear the attribute list or the attributes will be
    // set on the next exported element, which can result in corrupt
    // xml files due to duplicate attributes

    mrExport.CheckAttrList();   // asserts in non pro if we have attributes left
    mrExport.ClearAttrList();   // clears the attributes
}

// This method collects all automatic styles for the shapes inside the given XShapes collection
void XMLShapeExport::collectShapesAutoStyles( const uno::Reference < drawing::XShapes >& xShapes )
{
    ShapesInfos::iterator aOldCurrentShapesIter = maCurrentShapesIter;
    seekShapes( xShapes );

    uno::Reference< drawing::XShape > xShape;
    const sal_Int32 nShapeCount(xShapes->getCount());
    for(sal_Int32 nShapeId = 0; nShapeId < nShapeCount; nShapeId++)
    {
        xShapes->getByIndex(nShapeId) >>= xShape;
        SAL_WARN_IF( !xShape.is(), "xmloff", "Shape without a XShape?" );
        if(!xShape.is())
            continue;

        collectShapeAutoStyles( xShape );
    }

    maCurrentShapesIter = aOldCurrentShapesIter;
}

// This method exports all XShape inside the given XShapes collection
void XMLShapeExport::exportShapes( const uno::Reference < drawing::XShapes >& xShapes, XMLShapeExportFlags nFeatures /* = SEF_DEFAULT */, awt::Point* pRefPoint /* = NULL */ )
{
    ShapesInfos::iterator aOldCurrentShapesIter = maCurrentShapesIter;
    seekShapes( xShapes );

    uno::Reference< drawing::XShape > xShape;
    const sal_Int32 nShapeCount(xShapes->getCount());
    for(sal_Int32 nShapeId = 0; nShapeId < nShapeCount; nShapeId++)
    {
        xShapes->getByIndex(nShapeId) >>= xShape;
        SAL_WARN_IF( !xShape.is(), "xmloff", "Shape without a XShape?" );
        if(!xShape.is())
            continue;

        exportShape( xShape, nFeatures, pRefPoint );
    }

    maCurrentShapesIter = aOldCurrentShapesIter;
}

void XMLShapeExport::seekShapes( const uno::Reference< drawing::XShapes >& xShapes ) throw()
{
    if( xShapes.is() )
    {
        maCurrentShapesIter = maShapesInfos.find( xShapes );
        if( maCurrentShapesIter == maShapesInfos.end() )
        {
            ImplXMLShapeExportInfoVector aNewInfoVector;
            aNewInfoVector.resize( static_cast<ShapesInfos::size_type>(xShapes->getCount()) );
            maShapesInfos[ xShapes ] = aNewInfoVector;

            maCurrentShapesIter = maShapesInfos.find( xShapes );

            SAL_WARN_IF( maCurrentShapesIter == maShapesInfos.end(), "xmloff", "XMLShapeExport::seekShapes(): insert into stl::map failed" );
        }

        SAL_WARN_IF( (*maCurrentShapesIter).second.size() != static_cast<ShapesInfos::size_type>(xShapes->getCount()), "xmloff", "XMLShapeExport::seekShapes(): XShapes size varied between calls" );

    }
    else
    {
        maCurrentShapesIter = maShapesInfos.end();
    }
}

void XMLShapeExport::exportAutoStyles()
{
    // export all autostyle infos

    // ...for graphic
    {
        GetExport().GetAutoStylePool()->exportXML( XML_STYLE_FAMILY_SD_GRAPHICS_ID );
    }

    // ...for presentation
    {
        GetExport().GetAutoStylePool()->exportXML( XML_STYLE_FAMILY_SD_PRESENTATION_ID );
    }

    if( mxShapeTableExport.is() )
        mxShapeTableExport->exportAutoStyles();
}

/// returns the export property mapper for external chaining
SvXMLExportPropertyMapper* XMLShapeExport::CreateShapePropMapper(
    SvXMLExport& rExport )
{
    rtl::Reference< XMLPropertyHandlerFactory > xFactory = new XMLSdPropHdlFactory( rExport.GetModel(), rExport );
    rtl::Reference < XMLPropertySetMapper > xMapper = new XMLShapePropertySetMapper( xFactory, true );
    rExport.GetTextParagraphExport(); // get or create text paragraph export
    SvXMLExportPropertyMapper* pResult =
        new XMLShapeExportPropertyMapper( xMapper, rExport );
    // chain text attributes
    return pResult;
}

void XMLShapeExport::ImpCalcShapeType(const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType& eShapeType)
{
    // set in every case, so init here
    eShapeType = XmlShapeTypeUnknown;

    uno::Reference< drawing::XShapeDescriptor > xShapeDescriptor(xShape, uno::UNO_QUERY);
    if(xShapeDescriptor.is())
    {
        OUString aType(xShapeDescriptor->getShapeType());

        if(aType.match("com.sun.star."))
        {
            if(aType.match("drawing.", 13))
            {
                // drawing shapes
                if     (aType.match("Rectangle", 21)) { eShapeType = XmlShapeTypeDrawRectangleShape; }

                // #i72177# Note: Correcting CustomShape, CustomShape->Custom, len from 9 (was wrong anyways) to 6.
                // As can be seen at the other compares, the appendix "Shape" is left out of the comparison.
                else if(aType.match("Custom", 21)) { eShapeType = XmlShapeTypeDrawCustomShape; }

                else if(aType.match("Ellipse", 21)) { eShapeType = XmlShapeTypeDrawEllipseShape; }
                else if(aType.match("Control", 21)) { eShapeType = XmlShapeTypeDrawControlShape; }
                else if(aType.match("Connector", 21)) { eShapeType = XmlShapeTypeDrawConnectorShape; }
                else if(aType.match("Measure", 21)) { eShapeType = XmlShapeTypeDrawMeasureShape; }
                else if(aType.match("Line", 21)) { eShapeType = XmlShapeTypeDrawLineShape; }

                // #i72177# Note: This covers two types by purpose, PolyPolygonShape and PolyPolygonPathShape
                else if(aType.match("PolyPolygon", 21)) { eShapeType = XmlShapeTypeDrawPolyPolygonShape; }

                // #i72177# Note: This covers two types by purpose, PolyLineShape and PolyLinePathShape
                else if(aType.match("PolyLine", 21)) { eShapeType = XmlShapeTypeDrawPolyLineShape; }

                else if(aType.match("OpenBezier", 21)) { eShapeType = XmlShapeTypeDrawOpenBezierShape; }
                else if(aType.match("ClosedBezier", 21)) { eShapeType = XmlShapeTypeDrawClosedBezierShape; }

                // #i72177# FreeHand (opened and closed) now supports the types OpenFreeHandShape and
                // ClosedFreeHandShape respectively. Represent them as bezier shapes
                else if(aType.match("OpenFreeHand", 21)) { eShapeType = XmlShapeTypeDrawOpenBezierShape; }
                else if(aType.match("ClosedFreeHand", 21)) { eShapeType = XmlShapeTypeDrawClosedBezierShape; }

                else if(aType.match("GraphicObject", 21)) { eShapeType = XmlShapeTypeDrawGraphicObjectShape; }
                else if(aType.match("Group", 21)) { eShapeType = XmlShapeTypeDrawGroupShape; }
                else if(aType.match("Text", 21)) { eShapeType = XmlShapeTypeDrawTextShape; }
                else if(aType.match("OLE2", 21))
                {
                    eShapeType = XmlShapeTypeDrawOLE2Shape;

                    // get info about presentation shape
                    uno::Reference <beans::XPropertySet> xPropSet(xShape, uno::UNO_QUERY);

                    if(xPropSet.is())
                    {
                        OUString sCLSID;
                        if(xPropSet->getPropertyValue("CLSID") >>= sCLSID)
                        {
                            if (sCLSID == mrExport.GetChartExport()->getChartCLSID() ||
                                sCLSID == SvGlobalName( SO3_RPTCH_CLASSID ).GetHexName() )
                            {
                                eShapeType = XmlShapeTypeDrawChartShape;
                            }
                            else if (sCLSID == SvGlobalName( SO3_SC_CLASSID ).GetHexName() )
                            {
                                eShapeType = XmlShapeTypeDrawSheetShape;
                            }
                            else
                            {
                                // general OLE2 Object
                            }
                        }
                    }
                }
                else if(aType.match("Page", 21)) { eShapeType = XmlShapeTypeDrawPageShape; }
                else if(aType.match("Frame", 21)) { eShapeType = XmlShapeTypeDrawFrameShape; }
                else if(aType.match("Caption", 21)) { eShapeType = XmlShapeTypeDrawCaptionShape; }
                else if(aType.match("Plugin", 21)) { eShapeType = XmlShapeTypeDrawPluginShape; }
                else if(aType.match("Applet", 21)) { eShapeType = XmlShapeTypeDrawAppletShape; }
                else if(aType.match("MediaShape", 21)) { eShapeType = XmlShapeTypeDrawMediaShape; }
                else if(aType.match("TableShape", 21)) { eShapeType = XmlShapeTypeDrawTableShape; }

                // 3D shapes
                else if(aType.match("Scene", 21 + 7)) { eShapeType = XmlShapeTypeDraw3DSceneObject; }
                else if(aType.match("Cube", 21 + 7)) { eShapeType = XmlShapeTypeDraw3DCubeObject; }
                else if(aType.match("Sphere", 21 + 7)) { eShapeType = XmlShapeTypeDraw3DSphereObject; }
                else if(aType.match("Lathe", 21 + 7)) { eShapeType = XmlShapeTypeDraw3DLatheObject; }
                else if(aType.match("Extrude", 21 + 7)) { eShapeType = XmlShapeTypeDraw3DExtrudeObject; }
            }
            else if(aType.match("presentation.", 13))
            {
                // presentation shapes
                if     (aType.match("TitleText", 26)) { eShapeType = XmlShapeTypePresTitleTextShape; }
                else if(aType.match("Outliner", 26)) { eShapeType = XmlShapeTypePresOutlinerShape;  }
                else if(aType.match("Subtitle", 26)) { eShapeType = XmlShapeTypePresSubtitleShape;  }
                else if(aType.match("GraphicObject", 26)) { eShapeType = XmlShapeTypePresGraphicObjectShape;  }
                else if(aType.match("Page", 26)) { eShapeType = XmlShapeTypePresPageShape;  }
                else if(aType.match("OLE2", 26))
                {
                    eShapeType = XmlShapeTypePresOLE2Shape;

                    // get info about presentation shape
                    uno::Reference <beans::XPropertySet> xPropSet(xShape, uno::UNO_QUERY);

                    if(xPropSet.is()) try
                    {
                        OUString sCLSID;
                        if(xPropSet->getPropertyValue("CLSID") >>= sCLSID)
                        {
                            if( sCLSID == SvGlobalName( SO3_SC_CLASSID ).GetHexName() )
                            {
                                eShapeType = XmlShapeTypePresSheetShape;
                            }
                        }
                    }
                    catch(const uno::Exception&)
                    {
                        SAL_WARN( "xmloff", "XMLShapeExport::ImpCalcShapeType(), expected ole shape to have the CLSID property?" );
                    }
                }
                else if(aType.match("Chart", 26)) { eShapeType = XmlShapeTypePresChartShape;  }
                else if(aType.match("OrgChart", 26)) { eShapeType = XmlShapeTypePresOrgChartShape;  }
                else if(aType.match("CalcShape", 26)) { eShapeType = XmlShapeTypePresSheetShape; }
                else if(aType.match("TableShape", 26)) { eShapeType = XmlShapeTypePresTableShape; }
                else if(aType.match("Notes", 26)) { eShapeType = XmlShapeTypePresNotesShape;  }
                else if(aType.match("HandoutShape", 26)) { eShapeType = XmlShapeTypeHandoutShape; }
                else if(aType.match("HeaderShape", 26)) { eShapeType = XmlShapeTypePresHeaderShape; }
                else if(aType.match("FooterShape", 26)) { eShapeType = XmlShapeTypePresFooterShape; }
                else if(aType.match("SlideNumberShape", 26)) { eShapeType = XmlShapeTypePresSlideNumberShape; }
                else if(aType.match("DateTimeShape", 26)) { eShapeType = XmlShapeTypePresDateTimeShape; }
                else if(aType.match("MediaShape", 26)) { eShapeType = XmlShapeTypePresMediaShape; }
            }
        }
    }
}

/** exports all user defined glue points */
void XMLShapeExport::ImpExportGluePoints( const uno::Reference< drawing::XShape >& xShape )
{
    uno::Reference< drawing::XGluePointsSupplier > xSupplier( xShape, uno::UNO_QUERY );
    if( !xSupplier.is() )
        return;

    uno::Reference< container::XIdentifierAccess > xGluePoints( xSupplier->getGluePoints(), uno::UNO_QUERY );
    if( !xGluePoints.is() )
        return;

    drawing::GluePoint2 aGluePoint;

    uno::Sequence< sal_Int32 > aIdSequence( xGluePoints->getIdentifiers() );

    const sal_Int32 nCount = aIdSequence.getLength();
    for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
    {
        const sal_Int32 nIdentifier = aIdSequence[nIndex];
        if( (xGluePoints->getByIdentifier( nIdentifier ) >>= aGluePoint) && aGluePoint.IsUserDefined )
        {
            // export only user defined glue points

            const OUString sId( OUString::number( nIdentifier ) );
            mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_ID, sId );

            mrExport.GetMM100UnitConverter().convertMeasureToXML(msBuffer,
                    aGluePoint.Position.X);
            mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_X, msBuffer.makeStringAndClear());

            mrExport.GetMM100UnitConverter().convertMeasureToXML(msBuffer,
                    aGluePoint.Position.Y);
            mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_Y, msBuffer.makeStringAndClear());

            if( !aGluePoint.IsRelative )
            {
                SvXMLUnitConverter::convertEnum( msBuffer, aGluePoint.PositionAlignment, aXML_GlueAlignment_EnumMap );
                mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_ALIGN, msBuffer.makeStringAndClear() );
            }

            if( aGluePoint.Escape != drawing::EscapeDirection_SMART )
            {
                SvXMLUnitConverter::convertEnum( msBuffer, aGluePoint.Escape, aXML_GlueEscapeDirection_EnumMap );
                mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_ESCAPE_DIRECTION, msBuffer.makeStringAndClear() );
            }

            SvXMLElementExport aEventsElemt(mrExport, XML_NAMESPACE_DRAW, XML_GLUE_POINT, true, true);
        }
    }
}

void XMLShapeExport::ImpExportSignatureLine(const uno::Reference<drawing::XShape>& xShape)
{
    uno::Reference<beans::XPropertySet> xPropSet(xShape, uno::UNO_QUERY);

    bool bIsSignatureLine = false;
    xPropSet->getPropertyValue("IsSignatureLine") >>= bIsSignatureLine;
    if (!bIsSignatureLine)
        return;

    OUString aSignatureLineId;
    xPropSet->getPropertyValue("SignatureLineId") >>= aSignatureLineId;
    mrExport.AddAttribute(XML_NAMESPACE_LO_EXT, XML_ID, aSignatureLineId);

    OUString aSuggestedSignerName;
    xPropSet->getPropertyValue("SignatureLineSuggestedSignerName") >>= aSuggestedSignerName;
    if (!aSuggestedSignerName.isEmpty())
        mrExport.AddAttribute(XML_NAMESPACE_LO_EXT, XML_SUGGESTED_SIGNER_NAME, aSuggestedSignerName);

    OUString aSuggestedSignerTitle;
    xPropSet->getPropertyValue("SignatureLineSuggestedSignerTitle") >>= aSuggestedSignerTitle;
    if (!aSuggestedSignerTitle.isEmpty())
        mrExport.AddAttribute(XML_NAMESPACE_LO_EXT, XML_SUGGESTED_SIGNER_TITLE, aSuggestedSignerTitle);

    OUString aSuggestedSignerEmail;
    xPropSet->getPropertyValue("SignatureLineSuggestedSignerEmail") >>= aSuggestedSignerEmail;
    if (!aSuggestedSignerEmail.isEmpty())
        mrExport.AddAttribute(XML_NAMESPACE_LO_EXT, XML_SUGGESTED_SIGNER_EMAIL, aSuggestedSignerEmail);

    OUString aSigningInstructions;
    xPropSet->getPropertyValue("SignatureLineSigningInstructions") >>= aSigningInstructions;
    if (!aSigningInstructions.isEmpty())
        mrExport.AddAttribute(XML_NAMESPACE_LO_EXT, XML_SIGNING_INSTRUCTIONS, aSigningInstructions);

    bool bShowSignDate = false;
    xPropSet->getPropertyValue("SignatureLineShowSignDate") >>= bShowSignDate;
    mrExport.AddAttribute(XML_NAMESPACE_LO_EXT, XML_SHOW_SIGN_DATE,
                          bShowSignDate ? XML_TRUE : XML_FALSE);

    bool bCanAddComment = false;
    xPropSet->getPropertyValue("SignatureLineCanAddComment") >>= bCanAddComment;
    mrExport.AddAttribute(XML_NAMESPACE_LO_EXT, XML_CAN_ADD_COMMENT,
                          bCanAddComment ? XML_TRUE : XML_FALSE);

    SvXMLElementExport aSignatureLineElement(mrExport, XML_NAMESPACE_LO_EXT, XML_SIGNATURELINE, true,
                                             true);
}

void XMLShapeExport::ExportGraphicDefaults()
{
    rtl::Reference<XMLStyleExport> aStEx(new XMLStyleExport(mrExport, mrExport.GetAutoStylePool().get()));

    // construct PropertySetMapper
    rtl::Reference< SvXMLExportPropertyMapper > xPropertySetMapper( CreateShapePropMapper( mrExport ) );
    static_cast<XMLShapeExportPropertyMapper*>(xPropertySetMapper.get())->SetAutoStyles( false );

    // chain text attributes
    xPropertySetMapper->ChainExportMapper(XMLTextParagraphExport::CreateParaExtPropMapper(mrExport));

    // chain special Writer/text frame default attributes
    xPropertySetMapper->ChainExportMapper(XMLTextParagraphExport::CreateParaDefaultExtPropMapper(mrExport));

    // write graphic family default style
    uno::Reference< lang::XMultiServiceFactory > xFact( mrExport.GetModel(), uno::UNO_QUERY );
    if( xFact.is() )
    {
        try
        {
            uno::Reference< beans::XPropertySet > xDefaults( xFact->createInstance("com.sun.star.drawing.Defaults"), uno::UNO_QUERY );
            if( xDefaults.is() )
            {
                aStEx->exportDefaultStyle( xDefaults, XML_STYLE_FAMILY_SD_GRAPHICS_NAME, xPropertySetMapper );

                // write graphic family styles
                aStEx->exportStyleFamily("graphics", OUString(XML_STYLE_FAMILY_SD_GRAPHICS_NAME), xPropertySetMapper, false, XML_STYLE_FAMILY_SD_GRAPHICS_ID);
            }
        }
        catch(const lang::ServiceNotRegisteredException&)
        {
        }
    }
}

void XMLShapeExport::onExport( const css::uno::Reference < css::drawing::XShape >& )
{
}

const rtl::Reference< XMLTableExport >& XMLShapeExport::GetShapeTableExport()
{
    if( !mxShapeTableExport.is() )
    {
        rtl::Reference< XMLPropertyHandlerFactory > xFactory( new XMLSdPropHdlFactory( mrExport.GetModel(), mrExport ) );
        rtl::Reference < XMLPropertySetMapper > xMapper( new XMLShapePropertySetMapper( xFactory.get(), true ) );
        mrExport.GetTextParagraphExport(); // get or create text paragraph export
        rtl::Reference< SvXMLExportPropertyMapper > xPropertySetMapper( new XMLShapeExportPropertyMapper( xMapper, mrExport ) );
        mxShapeTableExport = new XMLTableExport( mrExport, xPropertySetMapper, xFactory );
    }

    return mxShapeTableExport;
}

void XMLShapeExport::ImpExportNewTrans(const uno::Reference< beans::XPropertySet >& xPropSet,
    XMLShapeExportFlags nFeatures, awt::Point* pRefPoint)
{
    // get matrix
    ::basegfx::B2DHomMatrix aMatrix;
    ImpExportNewTrans_GetB2DHomMatrix(aMatrix, xPropSet);

    // decompose and correct abour pRefPoint
    ::basegfx::B2DTuple aTRScale;
    double fTRShear(0.0);
    double fTRRotate(0.0);
    ::basegfx::B2DTuple aTRTranslate;
    ImpExportNewTrans_DecomposeAndRefPoint(aMatrix, aTRScale, fTRShear, fTRRotate, aTRTranslate, pRefPoint);

    // use features and write
    ImpExportNewTrans_FeaturesAndWrite(aTRScale, fTRShear, fTRRotate, aTRTranslate, nFeatures);
}

void XMLShapeExport::ImpExportNewTrans_GetB2DHomMatrix(::basegfx::B2DHomMatrix& rMatrix,
    const uno::Reference< beans::XPropertySet >& xPropSet)
{
    /* Get <TransformationInHoriL2R>, if it exist
       and if the document is exported into the OpenOffice.org file format.
       This property only exists at service css::text::Shape - the
       Writer UNO service for shapes.
       This code is needed, because the positioning attributes in the
       OpenOffice.org file format are given in horizontal left-to-right layout
       regardless the layout direction the shape is in. In the OASIS Open Office
       file format the positioning attributes are correctly given in the layout
       direction the shape is in. Thus, this code provides the conversion from
       the OASIS Open Office file format to the OpenOffice.org file format. (#i28749#)
    */
    uno::Any aAny;
    if ( !( GetExport().getExportFlags() & SvXMLExportFlags::OASIS ) &&
         xPropSet->getPropertySetInfo()->hasPropertyByName("TransformationInHoriL2R") )
    {
        aAny = xPropSet->getPropertyValue("TransformationInHoriL2R");
    }
    else
    {
        aAny = xPropSet->getPropertyValue("Transformation");
    }
    drawing::HomogenMatrix3 aMatrix;
    aAny >>= aMatrix;

    rMatrix.set(0, 0, aMatrix.Line1.Column1);
    rMatrix.set(0, 1, aMatrix.Line1.Column2);
    rMatrix.set(0, 2, aMatrix.Line1.Column3);
    rMatrix.set(1, 0, aMatrix.Line2.Column1);
    rMatrix.set(1, 1, aMatrix.Line2.Column2);
    rMatrix.set(1, 2, aMatrix.Line2.Column3);
    rMatrix.set(2, 0, aMatrix.Line3.Column1);
    rMatrix.set(2, 1, aMatrix.Line3.Column2);
    rMatrix.set(2, 2, aMatrix.Line3.Column3);
}

void XMLShapeExport::ImpExportNewTrans_DecomposeAndRefPoint(const ::basegfx::B2DHomMatrix& rMatrix, ::basegfx::B2DTuple& rTRScale,
    double& fTRShear, double& fTRRotate, ::basegfx::B2DTuple& rTRTranslate, css::awt::Point* pRefPoint)
{
    // decompose matrix
    rMatrix.decompose(rTRScale, rTRTranslate, fTRRotate, fTRShear);

    // correct translation about pRefPoint
    if(pRefPoint)
    {
        rTRTranslate -= ::basegfx::B2DTuple(pRefPoint->X, pRefPoint->Y);
    }
}

void XMLShapeExport::ImpExportNewTrans_FeaturesAndWrite(::basegfx::B2DTuple const & rTRScale, double fTRShear,
    double fTRRotate, ::basegfx::B2DTuple const & rTRTranslate, const XMLShapeExportFlags nFeatures)
{
    // always write Size (rTRScale) since this statement carries the union
    // of the object
    OUString aStr;
    OUStringBuffer sStringBuffer;
    ::basegfx::B2DTuple aTRScale(rTRScale);

    // svg: width
    if(!(nFeatures & XMLShapeExportFlags::WIDTH))
    {
        aTRScale.setX(1.0);
    }
    else
    {
        if( aTRScale.getX() > 0.0 )
            aTRScale.setX(aTRScale.getX() - 1.0);
        else if( aTRScale.getX() < 0.0 )
            aTRScale.setX(aTRScale.getX() + 1.0);
    }

    mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
            FRound(aTRScale.getX()));
    aStr = sStringBuffer.makeStringAndClear();
    mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_WIDTH, aStr);

    // svg: height
    if(!(nFeatures & XMLShapeExportFlags::HEIGHT))
    {
        aTRScale.setY(1.0);
    }
    else
    {
        if( aTRScale.getY() > 0.0 )
            aTRScale.setY(aTRScale.getY() - 1.0);
        else if( aTRScale.getY() < 0.0 )
            aTRScale.setY(aTRScale.getY() + 1.0);
    }

    mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
            FRound(aTRScale.getY()));
    aStr = sStringBuffer.makeStringAndClear();
    mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_HEIGHT, aStr);

    // decide if transformation is necessary
    bool bTransformationIsNecessary(fTRShear != 0.0 || fTRRotate != 0.0);

    if(bTransformationIsNecessary)
    {
        // write transformation, but WITHOUT scale which is exported as size above
        SdXMLImExTransform2D aTransform;

        aTransform.AddSkewX(atan(fTRShear));

        // #i78696#
        // fTRRotate is mathematically correct, but due to the error
        // we export/import it mirrored. Since the API implementation is fixed and
        // uses the correctly oriented angle, it is necessary for compatibility to
        // mirror the angle here to stay at the old behaviour. There is a follow-up
        // task (#i78698#) to fix this in the next ODF FileFormat version
        aTransform.AddRotate(-fTRRotate);

        aTransform.AddTranslate(rTRTranslate);

        // does transformation need to be exported?
        if(aTransform.NeedsAction())
            mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_TRANSFORM, aTransform.GetExportString(mrExport.GetMM100UnitConverter()));
    }
    else
    {
        // no shear, no rotate; just add object position to export and we are done
        if(nFeatures & XMLShapeExportFlags::X)
        {
            // svg: x
            mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                    FRound(rTRTranslate.getX()));
            aStr = sStringBuffer.makeStringAndClear();
            mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_X, aStr);
        }

        if(nFeatures & XMLShapeExportFlags::Y)
        {
            // svg: y
            mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                    FRound(rTRTranslate.getY()));
            aStr = sStringBuffer.makeStringAndClear();
            mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_Y, aStr);
        }
    }
}

bool XMLShapeExport::ImpExportPresentationAttributes( const uno::Reference< beans::XPropertySet >& xPropSet, const OUString& rClass )
{
    bool bIsEmpty = false;

    // write presentation class entry
    mrExport.AddAttribute(XML_NAMESPACE_PRESENTATION, XML_CLASS, rClass);

    if( xPropSet.is() )
    {
        uno::Reference< beans::XPropertySetInfo > xPropSetInfo( xPropSet->getPropertySetInfo() );


        // is empty pres. shape?
        if( xPropSetInfo.is() && xPropSetInfo->hasPropertyByName("IsEmptyPresentationObject"))
        {
            xPropSet->getPropertyValue("IsEmptyPresentationObject") >>= bIsEmpty;
            if( bIsEmpty )
                mrExport.AddAttribute(XML_NAMESPACE_PRESENTATION, XML_PLACEHOLDER, XML_TRUE);
        }

        // is user-transformed?
        if( xPropSetInfo.is() && xPropSetInfo->hasPropertyByName("IsPlaceholderDependent"))
        {
            bool bTemp = false;
            xPropSet->getPropertyValue("IsPlaceholderDependent") >>= bTemp;
            if(!bTemp)
                mrExport.AddAttribute(XML_NAMESPACE_PRESENTATION, XML_USER_TRANSFORMED, XML_TRUE);
        }
    }

    return bIsEmpty;
}

void XMLShapeExport::ImpExportText( const uno::Reference< drawing::XShape >& xShape, TextPNS eExtensionNS )
{
    if (eExtensionNS == TextPNS::EXTENSION)
    {
        if (mrExport.getDefaultVersion() <= SvtSaveOptions::ODFVER_012)
        {
            return; // do not export to ODF 1.1/1.2
        }
    }
    uno::Reference< text::XText > xText( xShape, uno::UNO_QUERY );
    if( xText.is() )
    {
        uno::Reference< container::XEnumerationAccess > xEnumAccess( xShape, uno::UNO_QUERY );
        if( xEnumAccess.is() && xEnumAccess->hasElements() )
            mrExport.GetTextParagraphExport()->exportText( xText, false, true, eExtensionNS );
    }
}


enum class Found {
    NONE              = 0x0000,
    CLICKACTION       = 0x0001,
    BOOKMARK          = 0x0002,
    EFFECT            = 0x0004,
    PLAYFULL          = 0x0008,
    VERB              = 0x0010,
    SOUNDURL          = 0x0020,
    SPEED             = 0x0040,
    CLICKEVENTTYPE    = 0x0080,
    MACRO             = 0x0100,
    LIBRARY           = 0x0200,
};
namespace o3tl {
    template<> struct typed_flags<Found> : is_typed_flags<Found, 0x03ff> {};
}

void XMLShapeExport::ImpExportEvents( const uno::Reference< drawing::XShape >& xShape )
{
    uno::Reference< document::XEventsSupplier > xEventsSupplier( xShape, uno::UNO_QUERY );
    if( !xEventsSupplier.is() )
        return;

    uno::Reference< container::XNameAccess > xEvents( xEventsSupplier->getEvents(), uno::UNO_QUERY );
    SAL_WARN_IF( !xEvents.is(), "xmloff", "XEventsSupplier::getEvents() returned NULL" );
    if( !xEvents.is() )
        return;

    Found nFound = Found::NONE;

    OUString aClickEventType;
    presentation::ClickAction eClickAction = presentation::ClickAction_NONE;
    presentation::AnimationEffect eEffect = presentation::AnimationEffect_NONE;
    presentation::AnimationSpeed eSpeed = presentation::AnimationSpeed_SLOW;
    OUString aStrSoundURL;
    bool bPlayFull = false;
    sal_Int32 nVerb = 0;
    OUString aStrMacro;
    OUString aStrLibrary;
    OUString aStrBookmark;

    uno::Sequence< beans::PropertyValue > aClickProperties;
    if( xEvents->hasByName( gsOnClick ) && (xEvents->getByName( gsOnClick ) >>= aClickProperties) )
    {
        const beans::PropertyValue* pProperty = aClickProperties.getConstArray();
        const beans::PropertyValue* pPropertyEnd = pProperty + aClickProperties.getLength();
        for( ; pProperty != pPropertyEnd; ++pProperty )
        {
            if( !( nFound & Found::CLICKEVENTTYPE ) && pProperty->Name == gsEventType )
            {
                if( pProperty->Value >>= aClickEventType )
                    nFound |= Found::CLICKEVENTTYPE;
            }
            else if( !( nFound & Found::CLICKACTION ) && pProperty->Name == gsClickAction )
            {
                if( pProperty->Value >>= eClickAction )
                    nFound |= Found::CLICKACTION;
            }
            else if( !( nFound & Found::MACRO ) && ( pProperty->Name == gsMacroName || pProperty->Name == gsScript ) )
            {
                if( pProperty->Value >>= aStrMacro )
                    nFound |= Found::MACRO;
            }
            else if( !( nFound & Found::LIBRARY ) && pProperty->Name == gsLibrary )
            {
                if( pProperty->Value >>= aStrLibrary )
                    nFound |= Found::LIBRARY;
            }
            else if( !( nFound & Found::EFFECT ) && pProperty->Name == gsEffect )
            {
                if( pProperty->Value >>= eEffect )
                    nFound |= Found::EFFECT;
            }
            else if( !( nFound & Found::BOOKMARK ) && pProperty->Name == gsBookmark )
            {
                if( pProperty->Value >>= aStrBookmark )
                    nFound |= Found::BOOKMARK;
            }
            else if( !( nFound & Found::SPEED ) && pProperty->Name == gsSpeed )
            {
                if( pProperty->Value >>= eSpeed )
                    nFound |= Found::SPEED;
            }
            else if( !( nFound & Found::SOUNDURL ) && pProperty->Name == gsSoundURL )
            {
                if( pProperty->Value >>= aStrSoundURL )
                    nFound |= Found::SOUNDURL;
            }
            else if( !( nFound & Found::PLAYFULL ) && pProperty->Name == gsPlayFull )
            {
                if( pProperty->Value >>= bPlayFull )
                    nFound |= Found::PLAYFULL;
            }
            else if( !( nFound & Found::VERB ) && pProperty->Name == gsVerb )
            {
                if( pProperty->Value >>= nVerb )
                    nFound |= Found::VERB;
            }
        }
    }

    // create the XML elements

    if( aClickEventType == gsPresentation )
    {
        if( !(nFound & Found::CLICKACTION) || (eClickAction == presentation::ClickAction_NONE) )
            return;

        SvXMLElementExport aEventsElemt(mrExport, XML_NAMESPACE_OFFICE, XML_EVENT_LISTENERS, true, true);

        enum XMLTokenEnum eStrAction;

        switch( eClickAction )
        {
            case presentation::ClickAction_PREVPAGE:        eStrAction = XML_PREVIOUS_PAGE; break;
            case presentation::ClickAction_NEXTPAGE:        eStrAction = XML_NEXT_PAGE; break;
            case presentation::ClickAction_FIRSTPAGE:       eStrAction = XML_FIRST_PAGE; break;
            case presentation::ClickAction_LASTPAGE:        eStrAction = XML_LAST_PAGE; break;
            case presentation::ClickAction_INVISIBLE:       eStrAction = XML_HIDE; break;
            case presentation::ClickAction_STOPPRESENTATION:eStrAction = XML_STOP; break;
            case presentation::ClickAction_PROGRAM:         eStrAction = XML_EXECUTE; break;
            case presentation::ClickAction_BOOKMARK:        eStrAction = XML_SHOW; break;
            case presentation::ClickAction_DOCUMENT:        eStrAction = XML_SHOW; break;
            case presentation::ClickAction_MACRO:           eStrAction = XML_EXECUTE_MACRO; break;
            case presentation::ClickAction_VERB:            eStrAction = XML_VERB; break;
            case presentation::ClickAction_VANISH:          eStrAction = XML_FADE_OUT; break;
            case presentation::ClickAction_SOUND:           eStrAction = XML_SOUND; break;
            default:
                OSL_FAIL( "unknown presentation::ClickAction found!" );
                eStrAction = XML_UNKNOWN;
        }

        OUString aEventQName(
            mrExport.GetNamespaceMap().GetQNameByKey(
                    XML_NAMESPACE_DOM, "click" ) );
        mrExport.AddAttribute( XML_NAMESPACE_SCRIPT, XML_EVENT_NAME, aEventQName );
        mrExport.AddAttribute( XML_NAMESPACE_PRESENTATION, XML_ACTION, eStrAction );

        if( eClickAction == presentation::ClickAction_VANISH )
        {
            if( nFound & Found::EFFECT )
            {
                XMLEffect eKind;
                XMLEffectDirection eDirection;
                sal_Int16 nStartScale;
                bool bIn;

                SdXMLImplSetEffect( eEffect, eKind, eDirection, nStartScale, bIn );

                if( eKind != EK_none )
                {
                    SvXMLUnitConverter::convertEnum( msBuffer, eKind, aXML_AnimationEffect_EnumMap );
                    mrExport.AddAttribute( XML_NAMESPACE_PRESENTATION, XML_EFFECT, msBuffer.makeStringAndClear() );
                }

                if( eDirection != ED_none )
                {
                    SvXMLUnitConverter::convertEnum( msBuffer, eDirection, aXML_AnimationDirection_EnumMap );
                    mrExport.AddAttribute( XML_NAMESPACE_PRESENTATION, XML_DIRECTION, msBuffer.makeStringAndClear() );
                }

                if( nStartScale != -1 )
                {
                    ::sax::Converter::convertPercent( msBuffer, nStartScale );
                    mrExport.AddAttribute( XML_NAMESPACE_PRESENTATION, XML_START_SCALE, msBuffer.makeStringAndClear() );
                }
            }

            if( nFound & Found::SPEED && eEffect != presentation::AnimationEffect_NONE )
            {
                 if( eSpeed != presentation::AnimationSpeed_MEDIUM )
                    {
                    SvXMLUnitConverter::convertEnum( msBuffer, eSpeed, aXML_AnimationSpeed_EnumMap );
                    mrExport.AddAttribute( XML_NAMESPACE_PRESENTATION, XML_SPEED, msBuffer.makeStringAndClear() );
                }
            }
        }

        if( eClickAction == presentation::ClickAction_PROGRAM ||
            eClickAction == presentation::ClickAction_BOOKMARK ||
            eClickAction == presentation::ClickAction_DOCUMENT )
        {
            if( eClickAction == presentation::ClickAction_BOOKMARK )
                msBuffer.append( '#' );

            msBuffer.append( aStrBookmark );
            mrExport.AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, GetExport().GetRelativeReference(msBuffer.makeStringAndClear()) );
            mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
            mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
            mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONREQUEST );
        }

        if( ( nFound & Found::VERB ) && eClickAction == presentation::ClickAction_VERB )
        {
            msBuffer.append( nVerb );
            mrExport.AddAttribute(XML_NAMESPACE_PRESENTATION, XML_VERB, msBuffer.makeStringAndClear());
        }

        SvXMLElementExport aEventElemt(mrExport, XML_NAMESPACE_PRESENTATION, XML_EVENT_LISTENER, true, true);

        if( eClickAction == presentation::ClickAction_VANISH || eClickAction == presentation::ClickAction_SOUND )
        {
            if( ( nFound & Found::SOUNDURL ) && !aStrSoundURL.isEmpty() )
            {
                mrExport.AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, GetExport().GetRelativeReference(aStrSoundURL) );
                mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
                mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_SHOW, XML_NEW );
                mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONREQUEST );
                if( nFound & Found::PLAYFULL && bPlayFull )
                    mrExport.AddAttribute( XML_NAMESPACE_PRESENTATION, XML_PLAY_FULL, XML_TRUE );

                SvXMLElementExport aElem( mrExport, XML_NAMESPACE_PRESENTATION, XML_SOUND, true, true );
            }
       }
    }
    else if( aClickEventType == gsStarBasic )
    {
        if( nFound & Found::MACRO )
        {
            SvXMLElementExport aEventsElemt(mrExport, XML_NAMESPACE_OFFICE, XML_EVENT_LISTENERS, true, true);

            mrExport.AddAttribute( XML_NAMESPACE_SCRIPT, XML_LANGUAGE,
                        mrExport.GetNamespaceMap().GetQNameByKey(
                            XML_NAMESPACE_OOO,
                            "starbasic" ) );
            OUString aEventQName(
                mrExport.GetNamespaceMap().GetQNameByKey(
                        XML_NAMESPACE_DOM, "click" ) );
            mrExport.AddAttribute( XML_NAMESPACE_SCRIPT, XML_EVENT_NAME, aEventQName );

            if( nFound & Found::LIBRARY )
            {
                const OUString& sLocation( GetXMLToken(
                    (aStrLibrary.equalsIgnoreAsciiCase("StarOffice") ||
                     aStrLibrary.equalsIgnoreAsciiCase("application") ) ? XML_APPLICATION
                                                                       : XML_DOCUMENT ) );
                mrExport.AddAttribute(XML_NAMESPACE_SCRIPT, XML_MACRO_NAME,
                    sLocation + ":" + aStrMacro);
            }
            else
            {
                mrExport.AddAttribute( XML_NAMESPACE_SCRIPT, XML_MACRO_NAME, aStrMacro );
            }

            SvXMLElementExport aEventElemt(mrExport, XML_NAMESPACE_SCRIPT, XML_EVENT_LISTENER, true, true);
        }
    }
    else if( aClickEventType == gsScript )
    {
        if( nFound & Found::MACRO )
        {
            SvXMLElementExport aEventsElemt(mrExport, XML_NAMESPACE_OFFICE, XML_EVENT_LISTENERS, true, true);

            mrExport.AddAttribute( XML_NAMESPACE_SCRIPT, XML_LANGUAGE, mrExport.GetNamespaceMap().GetQNameByKey(
                     XML_NAMESPACE_OOO, GetXMLToken(XML_SCRIPT) ) );
            OUString aEventQName(
                mrExport.GetNamespaceMap().GetQNameByKey(
                        XML_NAMESPACE_DOM, "click" ) );
            mrExport.AddAttribute( XML_NAMESPACE_SCRIPT, XML_EVENT_NAME, aEventQName );
            mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, aStrMacro );
            mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, "simple" );

            SvXMLElementExport aEventElemt(mrExport, XML_NAMESPACE_SCRIPT, XML_EVENT_LISTENER, true, true);
        }
    }
}

/** #i68101# export shape Title and Description */
void XMLShapeExport::ImpExportDescription( const uno::Reference< drawing::XShape >& xShape )
{
    try
    {
        OUString aTitle;
        OUString aDescription;

        uno::Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY_THROW );
        xProps->getPropertyValue("Title") >>= aTitle;
        xProps->getPropertyValue("Description") >>= aDescription;

        if(!aTitle.isEmpty())
        {
            SvXMLElementExport aEventElemt(mrExport, XML_NAMESPACE_SVG, XML_TITLE, true, false);
            mrExport.Characters( aTitle );
        }

        if(!aDescription.isEmpty())
        {
            SvXMLElementExport aEventElemt(mrExport, XML_NAMESPACE_SVG, XML_DESC, true, false );
            mrExport.Characters( aDescription );
        }
    }
    catch( uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION( "xmloff", "exporting Title and/or Description for shape" );
    }
}

void XMLShapeExport::ImpExportGroupShape( const uno::Reference< drawing::XShape >& xShape, XMLShapeExportFlags nFeatures, awt::Point* pRefPoint)
{
    uno::Reference< drawing::XShapes > xShapes(xShape, uno::UNO_QUERY);
    if(xShapes.is() && xShapes->getCount())
    {
        // write group shape
        bool bCreateNewline( (nFeatures & XMLShapeExportFlags::NO_WS) == XMLShapeExportFlags::NONE ); // #86116#/#92210#
        SvXMLElementExport aPGR(mrExport, XML_NAMESPACE_DRAW, XML_G, bCreateNewline, true);

        ImpExportDescription( xShape ); // #i68101#
        ImpExportEvents( xShape );
        ImpExportGluePoints( xShape );

        // #89764# if export of position is suppressed for group shape,
        // positions of contained objects should be written relative to
        // the upper left edge of the group.
        awt::Point aUpperLeft;

        if(!(nFeatures & XMLShapeExportFlags::POSITION))
        {
            nFeatures |= XMLShapeExportFlags::POSITION;
            aUpperLeft = xShape->getPosition();
            pRefPoint = &aUpperLeft;
        }

        // write members
        exportShapes( xShapes, nFeatures, pRefPoint );
    }
}

void XMLShapeExport::ImpExportTextBoxShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, XMLShapeExportFlags nFeatures, awt::Point* pRefPoint)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(!xPropSet.is())
        return;

    // presentation attribute (if presentation)
    bool bIsPresShape(false);
    bool bIsEmptyPresObj(false);
    OUString aStr;

    switch(eShapeType)
    {
        case XmlShapeTypePresSubtitleShape:
        {
            aStr = GetXMLToken(XML_PRESENTATION_SUBTITLE);
            bIsPresShape = true;
            break;
        }
        case XmlShapeTypePresTitleTextShape:
        {
            aStr = GetXMLToken(XML_PRESENTATION_TITLE);
            bIsPresShape = true;
            break;
        }
        case XmlShapeTypePresOutlinerShape:
        {
            aStr = GetXMLToken(XML_PRESENTATION_OUTLINE);
            bIsPresShape = true;
            break;
        }
        case XmlShapeTypePresNotesShape:
        {
            aStr = GetXMLToken(XML_PRESENTATION_NOTES);
            bIsPresShape = true;
            break;
        }
        case XmlShapeTypePresHeaderShape:
        {
            aStr = GetXMLToken(XML_HEADER);
            bIsPresShape = true;
            break;
        }
        case XmlShapeTypePresFooterShape:
        {
            aStr = GetXMLToken(XML_FOOTER);
            bIsPresShape = true;
            break;
        }
        case XmlShapeTypePresSlideNumberShape:
        {
            aStr = GetXMLToken(XML_PAGE_NUMBER);
            bIsPresShape = true;
            break;
        }
        case XmlShapeTypePresDateTimeShape:
        {
            aStr = GetXMLToken(XML_DATE_TIME);
            bIsPresShape = true;
            break;
        }
        default:
            break;
    }

    // Transformation
    ImpExportNewTrans(xPropSet, nFeatures, pRefPoint);

    if(bIsPresShape)
        bIsEmptyPresObj = ImpExportPresentationAttributes( xPropSet, aStr );

    bool bCreateNewline( (nFeatures & XMLShapeExportFlags::NO_WS) == XMLShapeExportFlags::NONE ); // #86116#/#92210#
    SvXMLElementExport aElem( mrExport, XML_NAMESPACE_DRAW,
                              XML_FRAME, bCreateNewline, true );

    // evtl. corner radius?
    sal_Int32 nCornerRadius(0);
    xPropSet->getPropertyValue("CornerRadius") >>= nCornerRadius;
    if(nCornerRadius)
    {
        OUStringBuffer sStringBuffer;
        mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                nCornerRadius);
        mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_CORNER_RADIUS, sStringBuffer.makeStringAndClear());
    }

    {
        // write text-box
        SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DRAW, XML_TEXT_BOX, true, true);
        if(!bIsEmptyPresObj)
            ImpExportText( xShape );
    }

    ImpExportDescription( xShape ); // #i68101#
    ImpExportEvents( xShape );
    ImpExportGluePoints( xShape );

}

void XMLShapeExport::ImpExportRectangleShape(
    const uno::Reference< drawing::XShape >& xShape,
    XMLShapeExportFlags nFeatures, css::awt::Point* pRefPoint)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        // Transformation
        ImpExportNewTrans(xPropSet, nFeatures, pRefPoint);

        // evtl. corner radius?
        sal_Int32 nCornerRadius(0);
        xPropSet->getPropertyValue("CornerRadius") >>= nCornerRadius;
        if(nCornerRadius)
        {
            OUStringBuffer sStringBuffer;
            mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                    nCornerRadius);
            mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_CORNER_RADIUS, sStringBuffer.makeStringAndClear());
        }

        // write rectangle
        bool bCreateNewline( (nFeatures & XMLShapeExportFlags::NO_WS) == XMLShapeExportFlags::NONE ); // #86116#/#92210#
        SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DRAW, XML_RECT, bCreateNewline, true);

        ImpExportDescription( xShape ); // #i68101#
        ImpExportEvents( xShape );
        ImpExportGluePoints( xShape );
        ImpExportText( xShape );
    }
}

void XMLShapeExport::ImpExportLineShape(
    const uno::Reference< drawing::XShape >& xShape,
    XMLShapeExportFlags nFeatures, awt::Point* pRefPoint)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(!xPropSet.is())
        return;

    OUString aStr;
    OUStringBuffer sStringBuffer;
    awt::Point aStart(0,0);
    awt::Point aEnd(1,1);

    // #85920# use 'Geometry' to get the points of the line
    // since this slot take anchor pos into account.

    // get matrix
    ::basegfx::B2DHomMatrix aMatrix;
    ImpExportNewTrans_GetB2DHomMatrix(aMatrix, xPropSet);

    // decompose and correct about pRefPoint
    ::basegfx::B2DTuple aTRScale;
    double fTRShear(0.0);
    double fTRRotate(0.0);
    ::basegfx::B2DTuple aTRTranslate;
    ImpExportNewTrans_DecomposeAndRefPoint(aMatrix, aTRScale, fTRShear, fTRRotate, aTRTranslate, pRefPoint);

    // create base position
    awt::Point aBasePosition(FRound(aTRTranslate.getX()), FRound(aTRTranslate.getY()));

    // get the two points
    uno::Any aAny(xPropSet->getPropertyValue("Geometry"));
    if (auto pSourcePolyPolygon
            = o3tl::tryAccess<drawing::PointSequenceSequence>(aAny))
    {
        if (pSourcePolyPolygon->getLength() > 0)
        {
            const drawing::PointSequence& rInnerSequence = (*pSourcePolyPolygon)[0];
            if (rInnerSequence.getLength() > 0)
            {
                const awt::Point& rPoint = rInnerSequence[0];
                aStart = awt::Point(rPoint.X + aBasePosition.X, rPoint.Y + aBasePosition.Y);
            }
            if (rInnerSequence.getLength() > 1)
            {
                const awt::Point& rPoint = rInnerSequence[1];
                aEnd = awt::Point(rPoint.X + aBasePosition.X, rPoint.Y + aBasePosition.Y);
            }
        }
    }

    if( nFeatures & XMLShapeExportFlags::X )
    {
        // svg: x1
        mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                aStart.X);
        aStr = sStringBuffer.makeStringAndClear();
        mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_X1, aStr);
    }
    else
    {
        aEnd.X -= aStart.X;
    }

    if( nFeatures & XMLShapeExportFlags::Y )
    {
        // svg: y1
        mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                aStart.Y);
        aStr = sStringBuffer.makeStringAndClear();
        mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_Y1, aStr);
    }
    else
    {
        aEnd.Y -= aStart.Y;
    }

    // svg: x2
    mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
            aEnd.X);
    aStr = sStringBuffer.makeStringAndClear();
    mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_X2, aStr);

    // svg: y2
    mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
            aEnd.Y);
    aStr = sStringBuffer.makeStringAndClear();
    mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_Y2, aStr);

    // write line
    bool bCreateNewline( (nFeatures & XMLShapeExportFlags::NO_WS) == XMLShapeExportFlags::NONE ); // #86116#/#92210#
    SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DRAW, XML_LINE, bCreateNewline, true);

    ImpExportDescription( xShape ); // #i68101#
    ImpExportEvents( xShape );
    ImpExportGluePoints( xShape );
    ImpExportText( xShape );

}

void XMLShapeExport::ImpExportEllipseShape(
    const uno::Reference< drawing::XShape >& xShape,
    XMLShapeExportFlags nFeatures, awt::Point* pRefPoint)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(!xPropSet.is())
        return;

    // get size to decide between Circle and Ellipse
    awt::Size aSize = xShape->getSize();
    sal_Int32 nRx((aSize.Width + 1) / 2);
    sal_Int32 nRy((aSize.Height + 1) / 2);
    bool bCircle(nRx == nRy);

    // Transformation
    ImpExportNewTrans(xPropSet, nFeatures, pRefPoint);

    drawing::CircleKind eKind = drawing::CircleKind_FULL;
    xPropSet->getPropertyValue("CircleKind") >>= eKind;
    if( eKind != drawing::CircleKind_FULL )
    {
        OUStringBuffer sStringBuffer;
        sal_Int32 nStartAngle = 0;
        sal_Int32 nEndAngle = 0;
        xPropSet->getPropertyValue("CircleStartAngle") >>= nStartAngle;
        xPropSet->getPropertyValue("CircleEndAngle") >>= nEndAngle;

        const double dStartAngle = nStartAngle / 100.0;
        const double dEndAngle = nEndAngle / 100.0;

        // export circle kind
        SvXMLUnitConverter::convertEnum( sStringBuffer, eKind, aXML_CircleKind_EnumMap );
        mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_KIND, sStringBuffer.makeStringAndClear() );

        // export start angle
        ::sax::Converter::convertDouble( sStringBuffer, dStartAngle );
        mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_START_ANGLE, sStringBuffer.makeStringAndClear() );

        // export end angle
        ::sax::Converter::convertDouble( sStringBuffer, dEndAngle );
        mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_END_ANGLE, sStringBuffer.makeStringAndClear() );
    }

    bool bCreateNewline( (nFeatures & XMLShapeExportFlags::NO_WS) == XMLShapeExportFlags::NONE ); // #86116#/#92210#

    // write ellipse or circle
    SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DRAW,
                            bCircle ? XML_CIRCLE : XML_ELLIPSE,
                            bCreateNewline, true);

    ImpExportDescription( xShape ); // #i68101#
    ImpExportEvents( xShape );
    ImpExportGluePoints( xShape );
    ImpExportText( xShape );

}

void XMLShapeExport::ImpExportPolygonShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, XMLShapeExportFlags nFeatures, awt::Point* pRefPoint)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(!xPropSet.is())
        return;

    bool bBezier(eShapeType == XmlShapeTypeDrawClosedBezierShape
        || eShapeType == XmlShapeTypeDrawOpenBezierShape);

    // get matrix
    ::basegfx::B2DHomMatrix aMatrix;
    ImpExportNewTrans_GetB2DHomMatrix(aMatrix, xPropSet);

    // decompose and correct abour pRefPoint
    ::basegfx::B2DTuple aTRScale;
    double fTRShear(0.0);
    double fTRRotate(0.0);
    ::basegfx::B2DTuple aTRTranslate;
    ImpExportNewTrans_DecomposeAndRefPoint(aMatrix, aTRScale, fTRShear, fTRRotate, aTRTranslate, pRefPoint);

    // use features and write
    ImpExportNewTrans_FeaturesAndWrite(aTRScale, fTRShear, fTRRotate, aTRTranslate, nFeatures);

    // create and export ViewBox
    awt::Size aSize(FRound(aTRScale.getX()), FRound(aTRScale.getY()));
    SdXMLImExViewBox aViewBox(0, 0, aSize.Width, aSize.Height);
    mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_VIEWBOX, aViewBox.GetExportString());

    bool bCreateNewline( (nFeatures & XMLShapeExportFlags::NO_WS) == XMLShapeExportFlags::NONE ); // #86116#/#92210#

    // prepare name (with most used)
    enum ::xmloff::token::XMLTokenEnum eName(XML_PATH);

    if(bBezier)
    {
        // get PolygonBezier
        uno::Any aAny( xPropSet->getPropertyValue("Geometry") );
        const basegfx::B2DPolyPolygon aPolyPolygon(
            basegfx::utils::UnoPolyPolygonBezierCoordsToB2DPolyPolygon(*o3tl::doAccess<drawing::PolyPolygonBezierCoords>(aAny)));

        if(aPolyPolygon.count())
        {
            // complex polygon shape, write as svg:d
            const OUString aPolygonString(
                basegfx::utils::exportToSvgD(
                    aPolyPolygon,
                    true,       // bUseRelativeCoordinates
                    false,      // bDetectQuadraticBeziers: not used in old, but maybe activated now
                    true));     // bHandleRelativeNextPointCompatible

            // write point array
            mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_D, aPolygonString);
        }
    }
    else
    {
        // get non-bezier polygon
        uno::Any aAny( xPropSet->getPropertyValue("Geometry") );
        const basegfx::B2DPolyPolygon aPolyPolygon(
            basegfx::utils::UnoPointSequenceSequenceToB2DPolyPolygon(*o3tl::doAccess<drawing::PointSequenceSequence>(aAny)));

        if(!aPolyPolygon.areControlPointsUsed() && 1 == aPolyPolygon.count())
        {
            // simple polygon shape, can be written as svg:points sequence
            const basegfx::B2DPolygon& aPolygon(aPolyPolygon.getB2DPolygon(0));
            const OUString aPointString(basegfx::utils::exportToSvgPoints(aPolygon));

            // write point array
            mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_POINTS, aPointString);

            // set name
            eName = aPolygon.isClosed() ? XML_POLYGON : XML_POLYLINE;
        }
        else
        {
            // complex polygon shape, write as svg:d
            const OUString aPolygonString(
                basegfx::utils::exportToSvgD(
                    aPolyPolygon,
                    true,       // bUseRelativeCoordinates
                    false,      // bDetectQuadraticBeziers: not used in old, but maybe activated now
                    true));     // bHandleRelativeNextPointCompatible

            // write point array
            mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_D, aPolygonString);
        }
    }

    // write object, but after attributes are added since this call will
    // consume all of these added attributes and the destructor will close the
    // scope. Also before text is added; this may add sub-scopes as needed
    SvXMLElementExport aOBJ(
        mrExport,
        XML_NAMESPACE_DRAW,
        eName,
        bCreateNewline,
        true);

    ImpExportDescription( xShape ); // #i68101#
    ImpExportEvents( xShape );
    ImpExportGluePoints( xShape );
    ImpExportText( xShape );

}

namespace
{

OUString getNameFromStreamURL(OUString const & rURL)
{
    const OUString sPackageURL("vnd.sun.star.Package:");

    OUString sResult;

    if (rURL.match(sPackageURL))
    {
        OUString sRequestedName = rURL.copy(sPackageURL.getLength());
        sal_Int32 nLastIndex = sRequestedName.lastIndexOf('/') + 1;
        if ((nLastIndex > 0) && (nLastIndex < sRequestedName.getLength()))
            sRequestedName = sRequestedName.copy(nLastIndex);
        nLastIndex = sRequestedName.lastIndexOf('.');
        if (nLastIndex >= 0)
            sRequestedName = sRequestedName.copy(0, nLastIndex);
        if (!sRequestedName.isEmpty())
            sResult = sRequestedName;
    }

    return sResult;
}

} // end anonymous namespace

void XMLShapeExport::ImpExportGraphicObjectShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, XMLShapeExportFlags nFeatures, awt::Point* pRefPoint)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(!xPropSet.is())
        return;

    bool bIsEmptyPresObj = false;

    // Transformation
    ImpExportNewTrans(xPropSet, nFeatures, pRefPoint);

    if(eShapeType == XmlShapeTypePresGraphicObjectShape)
        bIsEmptyPresObj = ImpExportPresentationAttributes( xPropSet, GetXMLToken(XML_PRESENTATION_GRAPHIC) );

    bool bCreateNewline( (nFeatures & XMLShapeExportFlags::NO_WS) == XMLShapeExportFlags::NONE ); // #86116#/#92210#
    SvXMLElementExport aElem( mrExport, XML_NAMESPACE_DRAW,
                              XML_FRAME, bCreateNewline, true );

    const bool bSaveBackwardsCompatible = bool( mrExport.getExportFlags() & SvXMLExportFlags::SAVEBACKWARDCOMPATIBLE );

    if (!bIsEmptyPresObj || bSaveBackwardsCompatible)
    {
        uno::Reference<graphic::XGraphic> xGraphic;
        OUString sOutMimeType;

        if (!bIsEmptyPresObj)
        {
            OUString aStreamURL;
            xPropSet->getPropertyValue("GraphicStreamURL") >>= aStreamURL;
            OUString sRequestedName = getNameFromStreamURL(aStreamURL);

            xPropSet->getPropertyValue("Graphic") >>= xGraphic;

            OUString sInternalURL;

            if (xGraphic.is())
                sInternalURL = mrExport.AddEmbeddedXGraphic(xGraphic, sOutMimeType, sRequestedName);

            if (!sInternalURL.isEmpty())
            {
                // apply possible changed stream URL to embedded image object
                if (!sRequestedName.isEmpty())
                {
                    const OUString sPackageURL("vnd.sun.star.Package:");
                    OUString newStreamURL = sPackageURL;
                    if (sInternalURL[0] == '#')
                    {
                        newStreamURL += sInternalURL.copy(1, sInternalURL.getLength() - 1);
                    }
                    else
                    {
                        newStreamURL += sInternalURL;
                    }

                    if (newStreamURL != aStreamURL)
                    {
                        xPropSet->setPropertyValue("GraphicStreamURL", uno::Any(newStreamURL));
                    }
                }

                mrExport.AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, sInternalURL);
                mrExport.AddAttribute(XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE);
                mrExport.AddAttribute(XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED);
                mrExport.AddAttribute(XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD);
            }
        }
        else
        {
            mrExport.AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, OUString());
            mrExport.AddAttribute(XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE);
            mrExport.AddAttribute(XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED);
            mrExport.AddAttribute(XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD);
        }

        {
            if (GetExport().getDefaultVersion() > SvtSaveOptions::ODFVER_012)
            {
                if (sOutMimeType.isEmpty())
                {
                    GetExport().GetGraphicMimeTypeFromStream(xGraphic, sOutMimeType);
                }
                if (!sOutMimeType.isEmpty())
                {
                    GetExport().AddAttribute(XML_NAMESPACE_LO_EXT, "mime-type", sOutMimeType);
                }
            }

            SvXMLElementExport aElement(mrExport, XML_NAMESPACE_DRAW, XML_IMAGE, true, true);

            // optional office:binary-data
            if (xGraphic.is())
            {
                mrExport.AddEmbeddedXGraphicAsBase64(xGraphic);
            }
            if (!bIsEmptyPresObj)
                ImpExportText(xShape);
        }

        //Resolves: fdo#62461 put preferred image first above, followed by
        //fallback here
        const bool bAddReplacementImages = officecfg::Office::Common::Save::Graphic::AddReplacementImages::get();
        if( !bIsEmptyPresObj && bAddReplacementImages)
        {
            uno::Reference<graphic::XGraphic> xReplacementGraphic;
            xPropSet->getPropertyValue("ReplacementGraphic") >>= xReplacementGraphic;

            // If there is no url, then the graphic is empty
            if (xReplacementGraphic.is())
            {
                OUString aMimeType;
                const OUString aHref = mrExport.AddEmbeddedXGraphic(xReplacementGraphic, aMimeType);

                if (aMimeType.isEmpty())
                    mrExport.GetGraphicMimeTypeFromStream(xReplacementGraphic, aMimeType);

                if (!aHref.isEmpty())
                {
                    mrExport.AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, aHref);
                    mrExport.AddAttribute(XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
                    mrExport.AddAttribute(XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
                    mrExport.AddAttribute(XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );
                }

                if (!aMimeType.isEmpty() && GetExport().getDefaultVersion() > SvtSaveOptions::ODFVER_012)
                    mrExport.AddAttribute(XML_NAMESPACE_LO_EXT, "mime-type", aMimeType);

                SvXMLElementExport aElement(mrExport, XML_NAMESPACE_DRAW, XML_IMAGE, true, true);

                // optional office:binary-data
                mrExport.AddEmbeddedXGraphicAsBase64(xReplacementGraphic);
            }
        }
    }

    ImpExportEvents( xShape );
    ImpExportGluePoints( xShape );

    // image map
    GetExport().GetImageMapExport().Export( xPropSet );
    ImpExportDescription( xShape ); // #i68101#

    // Signature Line - needs to be after the images!
    if (GetExport().getDefaultVersion() > SvtSaveOptions::ODFVER_012)
        ImpExportSignatureLine(xShape);
}

void XMLShapeExport::ImpExportChartShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, XMLShapeExportFlags nFeatures, awt::Point* pRefPoint,
    SvXMLAttributeList* pAttrList )
{
    ImpExportOLE2Shape( xShape, eShapeType, nFeatures, pRefPoint, pAttrList );
}

void XMLShapeExport::ImpExportControlShape(
    const uno::Reference< drawing::XShape >& xShape,
    XMLShapeExportFlags nFeatures, awt::Point* pRefPoint)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        // Transformation
        ImpExportNewTrans(xPropSet, nFeatures, pRefPoint);
    }

    uno::Reference< drawing::XControlShape > xControl( xShape, uno::UNO_QUERY );
    SAL_WARN_IF( !xControl.is(), "xmloff", "Control shape is not supporting XControlShape" );
    if( xControl.is() )
    {
        uno::Reference< beans::XPropertySet > xControlModel( xControl->getControl(), uno::UNO_QUERY );
        SAL_WARN_IF( !xControlModel.is(), "xmloff", "Control shape has not XControlModel" );
        if( xControlModel.is() )
        {
            mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CONTROL, mrExport.GetFormExport()->getControlId( xControlModel ) );
        }
    }

    bool bCreateNewline( (nFeatures & XMLShapeExportFlags::NO_WS) == XMLShapeExportFlags::NONE ); // #86116#/#92210#
    SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DRAW, XML_CONTROL, bCreateNewline, true);

    ImpExportDescription( xShape ); // #i68101#
}

void XMLShapeExport::ImpExportConnectorShape(
    const uno::Reference< drawing::XShape >& xShape,
    XMLShapeExportFlags nFeatures /* = SEF_DEFAULT */, awt::Point* pRefPoint /* = NULL */)
{
    uno::Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY );

    OUString aStr;
    OUStringBuffer sStringBuffer;

    // export connection kind
    drawing::ConnectorType eType = drawing::ConnectorType_STANDARD;
    uno::Any aAny = xProps->getPropertyValue("EdgeKind");
    aAny >>= eType;

    if( eType != drawing::ConnectorType_STANDARD )
    {
        SvXMLUnitConverter::convertEnum( sStringBuffer, eType, aXML_ConnectionKind_EnumMap );
        aStr = sStringBuffer.makeStringAndClear();
        mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_TYPE, aStr);
    }

    // export line skew
    sal_Int32 nDelta1 = 0, nDelta2 = 0, nDelta3 = 0;

    aAny = xProps->getPropertyValue("EdgeLine1Delta");
    aAny >>= nDelta1;
    aAny = xProps->getPropertyValue("EdgeLine2Delta");
    aAny >>= nDelta2;
    aAny = xProps->getPropertyValue("EdgeLine3Delta");
    aAny >>= nDelta3;

    if( nDelta1 != 0 || nDelta2 != 0 || nDelta3 != 0 )
    {
        mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                nDelta1);
        if( nDelta2 != 0 || nDelta3 != 0 )
        {
            sStringBuffer.append( ' ' );
            mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                    nDelta2);
            if( nDelta3 != 0 )
            {
                sStringBuffer.append( ' ' );
                mrExport.GetMM100UnitConverter().convertMeasureToXML(
                        sStringBuffer, nDelta3);
            }
        }

        aStr = sStringBuffer.makeStringAndClear();
        mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_LINE_SKEW, aStr);
    }

    // export start and end point
    awt::Point aStart(0,0);
    awt::Point aEnd(1,1);

    /* Get <StartPositionInHoriL2R> and
       <EndPositionInHoriL2R>, if they exist and if the document is exported
       into the OpenOffice.org file format.
       These properties only exist at service css::text::Shape - the
       Writer UNO service for shapes.
       This code is needed, because the positioning attributes in the
       OpenOffice.org file format are given in horizontal left-to-right layout
       regardless the layout direction the shape is in. In the OASIS Open Office
       file format the positioning attributes are correctly given in the layout
       direction the shape is in. Thus, this code provides the conversion from
       the OASIS Open Office file format to the OpenOffice.org file format. (#i36248#)
    */
    if ( !( GetExport().getExportFlags() & SvXMLExportFlags::OASIS ) &&
         xProps->getPropertySetInfo()->hasPropertyByName("StartPositionInHoriL2R") &&
         xProps->getPropertySetInfo()->hasPropertyByName("EndPositionInHoriL2R") )
    {
        xProps->getPropertyValue("StartPositionInHoriL2R") >>= aStart;
        xProps->getPropertyValue("EndPositionInHoriL2R") >>= aEnd;
    }
    else
    {
        xProps->getPropertyValue("StartPosition") >>= aStart;
        xProps->getPropertyValue("EndPosition") >>= aEnd;
    }

    if( pRefPoint )
    {
        aStart.X -= pRefPoint->X;
        aStart.Y -= pRefPoint->Y;
        aEnd.X -= pRefPoint->X;
        aEnd.Y -= pRefPoint->Y;
    }

    if( nFeatures & XMLShapeExportFlags::X )
    {
        // svg: x1
        mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                aStart.X);
        aStr = sStringBuffer.makeStringAndClear();
        mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_X1, aStr);
    }
    else
    {
        aEnd.X -= aStart.X;
    }

    if( nFeatures & XMLShapeExportFlags::Y )
    {
        // svg: y1
        mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                aStart.Y);
        aStr = sStringBuffer.makeStringAndClear();
        mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_Y1, aStr);
    }
    else
    {
        aEnd.Y -= aStart.Y;
    }

    // svg: x2
    mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer, aEnd.X);
    aStr = sStringBuffer.makeStringAndClear();
    mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_X2, aStr);

    // svg: y2
    mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer, aEnd.Y);
    aStr = sStringBuffer.makeStringAndClear();
    mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_Y2, aStr);

    // #i39320#
    uno::Reference< uno::XInterface > xRefS;
    uno::Reference< uno::XInterface > xRefE;

    // export start connection
    xProps->getPropertyValue("StartShape") >>= xRefS;
    if( xRefS.is() )
    {
        const OUString& rShapeId = mrExport.getInterfaceToIdentifierMapper().getIdentifier( xRefS );
        mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_START_SHAPE, rShapeId);

        aAny = xProps->getPropertyValue("StartGluePointIndex");
        sal_Int32 nGluePointId = 0;
        if( aAny >>= nGluePointId )
        {
            if( nGluePointId != -1 )
            {
                mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_START_GLUE_POINT, OUString::number( nGluePointId ));
            }
        }
    }

    // export end connection
    xProps->getPropertyValue("EndShape") >>= xRefE;
    if( xRefE.is() )
    {
        const OUString& rShapeId = mrExport.getInterfaceToIdentifierMapper().getIdentifier( xRefE );
        mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_END_SHAPE, rShapeId);

        aAny = xProps->getPropertyValue("EndGluePointIndex");
        sal_Int32 nGluePointId = 0;
        if( aAny >>= nGluePointId )
        {
            if( nGluePointId != -1 )
            {
                mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_END_GLUE_POINT, OUString::number( nGluePointId ));
            }
        }
    }

    // get PolygonBezier
    aAny = xProps->getPropertyValue("PolyPolygonBezier");
    auto pSourcePolyPolygon = o3tl::tryAccess<drawing::PolyPolygonBezierCoords>(aAny);
    if(pSourcePolyPolygon && pSourcePolyPolygon->Coordinates.getLength())
    {
        const basegfx::B2DPolyPolygon aPolyPolygon(
            basegfx::utils::UnoPolyPolygonBezierCoordsToB2DPolyPolygon(
                *pSourcePolyPolygon));
        const OUString aPolygonString(
            basegfx::utils::exportToSvgD(
                aPolyPolygon,
                true,           // bUseRelativeCoordinates
                false,          // bDetectQuadraticBeziers: not used in old, but maybe activated now
                true));         // bHandleRelativeNextPointCompatible

        // write point array
        mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_D, aPolygonString);
    }

    // get matrix
    ::basegfx::B2DHomMatrix aMatrix;
    ImpExportNewTrans_GetB2DHomMatrix(aMatrix, xProps);

    // decompose and correct about pRefPoint
    ::basegfx::B2DTuple aTRScale;
    double fTRShear(0.0);
    double fTRRotate(0.0);
    ::basegfx::B2DTuple aTRTranslate;
    ImpExportNewTrans_DecomposeAndRefPoint(aMatrix, aTRScale, fTRShear,
            fTRRotate, aTRTranslate, pRefPoint);

    // fdo#49678: create and export ViewBox
    awt::Size aSize(FRound(aTRScale.getX()), FRound(aTRScale.getY()));
    SdXMLImExViewBox aViewBox(0, 0, aSize.Width, aSize.Height);
    mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_VIEWBOX, aViewBox.GetExportString());

    // write connector shape. Add Export later.
    bool bCreateNewline( (nFeatures & XMLShapeExportFlags::NO_WS) == XMLShapeExportFlags::NONE ); // #86116#/#92210#
    SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DRAW, XML_CONNECTOR, bCreateNewline, true);

    ImpExportDescription( xShape ); // #i68101#
    ImpExportEvents( xShape );
    ImpExportGluePoints( xShape );
    ImpExportText( xShape );
}

void XMLShapeExport::ImpExportMeasureShape(
    const uno::Reference< drawing::XShape >& xShape,
    XMLShapeExportFlags nFeatures /* = SEF_DEFAULT */, awt::Point const * pRefPoint /* = NULL */)
{
    uno::Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY );

    OUString aStr;
    OUStringBuffer sStringBuffer;

    // export start and end point
    awt::Point aStart(0,0);
    awt::Point aEnd(1,1);

    /* Get <StartPositionInHoriL2R> and
       <EndPositionInHoriL2R>, if they exist and if the document is exported
       into the OpenOffice.org file format.
       These properties only exist at service css::text::Shape - the
       Writer UNO service for shapes.
       This code is needed, because the positioning attributes in the
       OpenOffice.org file format are given in horizontal left-to-right layout
       regardless the layout direction the shape is in. In the OASIS Open Office
       file format the positioning attributes are correctly given in the layout
       direction the shape is in. Thus, this code provides the conversion from
       the OASIS Open Office file format to the OpenOffice.org file format. (#i36248#)
    */
    if ( !( GetExport().getExportFlags() & SvXMLExportFlags::OASIS ) &&
         xProps->getPropertySetInfo()->hasPropertyByName("StartPositionInHoriL2R") &&
         xProps->getPropertySetInfo()->hasPropertyByName("EndPositionInHoriL2R") )
    {
        xProps->getPropertyValue("StartPositionInHoriL2R") >>= aStart;
        xProps->getPropertyValue("EndPositionInHoriL2R") >>= aEnd;
    }
    else
    {
        xProps->getPropertyValue("StartPosition") >>= aStart;
        xProps->getPropertyValue("EndPosition") >>= aEnd;
    }

    if( pRefPoint )
    {
        aStart.X -= pRefPoint->X;
        aStart.Y -= pRefPoint->Y;
        aEnd.X -= pRefPoint->X;
        aEnd.Y -= pRefPoint->Y;
    }

    if( nFeatures & XMLShapeExportFlags::X )
    {
        // svg: x1
        mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                aStart.X);
        aStr = sStringBuffer.makeStringAndClear();
        mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_X1, aStr);
    }
    else
    {
        aEnd.X -= aStart.X;
    }

    if( nFeatures & XMLShapeExportFlags::Y )
    {
        // svg: y1
        mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                aStart.Y);
        aStr = sStringBuffer.makeStringAndClear();
        mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_Y1, aStr);
    }
    else
    {
        aEnd.Y -= aStart.Y;
    }

    // svg: x2
    mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer, aEnd.X);
    aStr = sStringBuffer.makeStringAndClear();
    mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_X2, aStr);

    // svg: y2
    mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer, aEnd.Y);
    aStr = sStringBuffer.makeStringAndClear();
    mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_Y2, aStr);

    // write measure shape
    bool bCreateNewline( (nFeatures & XMLShapeExportFlags::NO_WS) == XMLShapeExportFlags::NONE ); // #86116#/#92210#
    SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DRAW, XML_MEASURE, bCreateNewline, true);

    ImpExportDescription( xShape ); // #i68101#
    ImpExportEvents( xShape );
    ImpExportGluePoints( xShape );

    uno::Reference< text::XText > xText( xShape, uno::UNO_QUERY );
    if( xText.is() )
        mrExport.GetTextParagraphExport()->exportText( xText );
}

void XMLShapeExport::ImpExportOLE2Shape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, XMLShapeExportFlags nFeatures /* = SEF_DEFAULT */, awt::Point* pRefPoint /* = NULL */,
    SvXMLAttributeList* pAttrList /* = NULL */ )
{
    uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    uno::Reference< container::XNamed > xNamed(xShape, uno::UNO_QUERY);

    SAL_WARN_IF( !xPropSet.is() || !xNamed.is(), "xmloff", "ole shape is not implementing needed interfaces");
    if(!(xPropSet.is() && xNamed.is()))
        return;

    // Transformation
    ImpExportNewTrans(xPropSet, nFeatures, pRefPoint);

    bool bIsEmptyPresObj = false;

    // presentation settings
    if(eShapeType == XmlShapeTypePresOLE2Shape)
        bIsEmptyPresObj = ImpExportPresentationAttributes( xPropSet, GetXMLToken(XML_PRESENTATION_OBJECT) );
    else if(eShapeType == XmlShapeTypePresChartShape)
        bIsEmptyPresObj = ImpExportPresentationAttributes( xPropSet, GetXMLToken(XML_PRESENTATION_CHART) );
    else if(eShapeType == XmlShapeTypePresSheetShape)
        bIsEmptyPresObj = ImpExportPresentationAttributes( xPropSet, GetXMLToken(XML_PRESENTATION_TABLE) );

    bool bCreateNewline( (nFeatures & XMLShapeExportFlags::NO_WS) == XMLShapeExportFlags::NONE ); // #86116#/#92210#
    bool bExportEmbedded(mrExport.getExportFlags() & SvXMLExportFlags::EMBEDDED);
    OUString sPersistName;
    SvXMLElementExport aElement( mrExport, XML_NAMESPACE_DRAW,
                              XML_FRAME, bCreateNewline, true );

    const bool bSaveBackwardsCompatible = bool( mrExport.getExportFlags() & SvXMLExportFlags::SAVEBACKWARDCOMPATIBLE );

    if( !bIsEmptyPresObj || bSaveBackwardsCompatible )
    {
        if (pAttrList)
        {
            mrExport.AddAttributeList(pAttrList);
        }

        OUString sClassId;
        OUString sURL;
        bool bInternal = false;
        xPropSet->getPropertyValue("IsInternal") >>= bInternal;

        if( !bIsEmptyPresObj )
        {

            if ( bInternal )
            {
                // OOo internal links have no storage persistence, URL is stored in the XML file
                // the result LinkURL is empty in case the object is not a link
                xPropSet->getPropertyValue("LinkURL") >>= sURL;
            }

            xPropSet->getPropertyValue("PersistName") >>= sPersistName;
            if ( sURL.isEmpty() )
            {
                if( !sPersistName.isEmpty() )
                {
                    sURL = "vnd.sun.star.EmbeddedObject:" + sPersistName;
                }
            }

            if( !bInternal )
                xPropSet->getPropertyValue("CLSID") >>= sClassId;

            if( !sClassId.isEmpty() )
                mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_CLASS_ID, sClassId );

            if(!bExportEmbedded)
            {
                // xlink:href
                if( !sURL.isEmpty() )
                {
                    // #96717# in theorie, if we don't have a url we shouldn't even
                    // export this ole shape. But practical its to risky right now
                    // to change this so we better dispose this on load
                    sURL = mrExport.AddEmbeddedObject( sURL );

                    mrExport.AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, sURL );
                    mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
                    mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
                    mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );
                }
            }
        }
        else
        {
            // export empty href for empty placeholders to be valid ODF
            mrExport.AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, "" );
            mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
            mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
            mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );
        }

        enum XMLTokenEnum eElem = sClassId.isEmpty() ? XML_OBJECT : XML_OBJECT_OLE ;
        SvXMLElementExport aElem( mrExport, XML_NAMESPACE_DRAW, eElem, true, true );

        // tdf#112547 export text as child of draw:object, where import expects it
        if (!bIsEmptyPresObj && supportsText(eShapeType))
        {
            // #i118485# Add text export, the draw OLE shape allows text now
            ImpExportText( xShape, TextPNS::EXTENSION );
        }

        if(bExportEmbedded && !bIsEmptyPresObj)
        {
            if(bInternal)
            {
                // embedded XML
                uno::Reference< lang::XComponent > xComp;
                xPropSet->getPropertyValue("Model") >>= xComp;
                SAL_WARN_IF( !xComp.is(), "xmloff", "no xModel for own OLE format" );
                mrExport.ExportEmbeddedOwnObject( xComp );
            }
            else
            {
                // embed as Base64
                // this is an alien object ( currently MSOLE is the only supported type of such objects )
                // in case it is not an OASIS format the object should be asked to store replacement image if possible

                OUString sURLRequest( sURL );
                if ( !( mrExport.getExportFlags() & SvXMLExportFlags::OASIS ) )
                    sURLRequest +=  "?oasis=false";
                mrExport.AddEmbeddedObjectAsBase64( sURLRequest );
            }
        }
    }
    if( !bIsEmptyPresObj )
    {
        OUString sURL(XML_EMBEDDEDOBJECTGRAPHIC_URL_BASE);
        sURL += sPersistName;
        if( !bExportEmbedded )
        {
            sURL = GetExport().AddEmbeddedObject( sURL );
            mrExport.AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, sURL );
            mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
            mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
            mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );
        }

        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_DRAW,
                                  XML_IMAGE, false, true );

        if( bExportEmbedded )
            GetExport().AddEmbeddedObjectAsBase64( sURL );
    }

    ImpExportEvents( xShape );
    ImpExportGluePoints( xShape );
    ImpExportDescription( xShape ); // #i68101#

}

void XMLShapeExport::ImpExportPageShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, XMLShapeExportFlags nFeatures /* = SEF_DEFAULT */, awt::Point* pRefPoint /* = NULL */)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        // #86163# Transformation
        ImpExportNewTrans(xPropSet, nFeatures, pRefPoint);

        // export page number used for this page
        uno::Reference< beans::XPropertySetInfo > xPropSetInfo( xPropSet->getPropertySetInfo() );
        const OUString aPageNumberStr("PageNumber");
        if( xPropSetInfo.is() && xPropSetInfo->hasPropertyByName(aPageNumberStr))
        {
            sal_Int32 nPageNumber = 0;
            xPropSet->getPropertyValue(aPageNumberStr) >>= nPageNumber;
            if( nPageNumber )
                mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_PAGE_NUMBER, OUString::number(nPageNumber));
        }

        // a presentation page shape, normally used on notes pages only. If
        // it is used not as presentation shape, it may have been created with
        // copy-paste exchange between draw and impress (this IS possible...)
        if(eShapeType == XmlShapeTypePresPageShape)
        {
            mrExport.AddAttribute(XML_NAMESPACE_PRESENTATION, XML_CLASS,
                                 XML_PRESENTATION_PAGE);
        }

        // write Page shape
        bool bCreateNewline( (nFeatures & XMLShapeExportFlags::NO_WS) == XMLShapeExportFlags::NONE ); // #86116#/#92210#
        SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DRAW, XML_PAGE_THUMBNAIL, bCreateNewline, true);
    }
}

void XMLShapeExport::ImpExportCaptionShape(
    const uno::Reference< drawing::XShape >& xShape,
    XMLShapeExportFlags nFeatures /* = SEF_DEFAULT */, awt::Point* pRefPoint /* = NULL */)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(!xPropSet.is())
        return;

    // Transformation
    ImpExportNewTrans(xPropSet, nFeatures, pRefPoint);

    // evtl. corner radius?
    sal_Int32 nCornerRadius(0);
    xPropSet->getPropertyValue("CornerRadius") >>= nCornerRadius;
    if(nCornerRadius)
    {
        OUStringBuffer sStringBuffer;
        mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                nCornerRadius);
        mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_CORNER_RADIUS, sStringBuffer.makeStringAndClear());
    }

    awt::Point aCaptionPoint;
    xPropSet->getPropertyValue("CaptionPoint") >>= aCaptionPoint;

    mrExport.GetMM100UnitConverter().convertMeasureToXML(msBuffer,
            aCaptionPoint.X);
    mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CAPTION_POINT_X, msBuffer.makeStringAndClear() );
    mrExport.GetMM100UnitConverter().convertMeasureToXML(msBuffer,
            aCaptionPoint.Y);
    mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CAPTION_POINT_Y, msBuffer.makeStringAndClear() );

    // write Caption shape. Add export later.
    bool bCreateNewline( (nFeatures & XMLShapeExportFlags::NO_WS) == XMLShapeExportFlags::NONE ); // #86116#/#92210#
    bool bAnnotation( (nFeatures & XMLShapeExportFlags::ANNOTATION) == XMLShapeExportFlags::ANNOTATION );

    SvXMLElementExport aObj( mrExport,
                             (bAnnotation ? XML_NAMESPACE_OFFICE
                                           : XML_NAMESPACE_DRAW),
                             (bAnnotation ? XML_ANNOTATION : XML_CAPTION),
                             bCreateNewline, true );

    ImpExportDescription( xShape ); // #i68101#
    ImpExportEvents( xShape );
    ImpExportGluePoints( xShape );
    if( bAnnotation )
        mrExport.exportAnnotationMeta( xShape );
    ImpExportText( xShape );

}

void XMLShapeExport::ImpExportFrameShape(
    const uno::Reference< drawing::XShape >& xShape,
    XMLShapeExportFlags nFeatures, css::awt::Point* pRefPoint)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(!xPropSet.is())
        return;

    // Transformation
    ImpExportNewTrans(xPropSet, nFeatures, pRefPoint);

    bool bCreateNewline( (nFeatures & XMLShapeExportFlags::NO_WS) == XMLShapeExportFlags::NONE ); // #86116#/#92210#
    SvXMLElementExport aElem( mrExport, XML_NAMESPACE_DRAW,
                              XML_FRAME, bCreateNewline, true );

    // export frame url
    OUString aStr;
    xPropSet->getPropertyValue("FrameURL") >>= aStr;
    mrExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_HREF, GetExport().GetRelativeReference(aStr) );
    mrExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
    mrExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
    mrExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );

    // export name
    xPropSet->getPropertyValue("FrameName") >>= aStr;
    if( !aStr.isEmpty() )
        mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_FRAME_NAME, aStr );

    // write floating frame
    {
        SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DRAW, XML_FLOATING_FRAME, true, true);
    }

}

void XMLShapeExport::ImpExportAppletShape(
    const uno::Reference< drawing::XShape >& xShape,
    XMLShapeExportFlags nFeatures, css::awt::Point* pRefPoint)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(!xPropSet.is())
        return;

    // Transformation
    ImpExportNewTrans(xPropSet, nFeatures, pRefPoint);

    bool bCreateNewline( (nFeatures & XMLShapeExportFlags::NO_WS) == XMLShapeExportFlags::NONE ); // #86116#/#92210#
    SvXMLElementExport aElement( mrExport, XML_NAMESPACE_DRAW,
                              XML_FRAME, bCreateNewline, true );

    // export frame url
    OUString aStr;
    xPropSet->getPropertyValue("AppletCodeBase") >>= aStr;
    mrExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_HREF, GetExport().GetRelativeReference(aStr) );
    mrExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
    mrExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
    mrExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );

    // export draw:applet-name
    xPropSet->getPropertyValue("AppletName") >>= aStr;
    if( !aStr.isEmpty() )
        mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_APPLET_NAME, aStr );

    // export draw:code
    xPropSet->getPropertyValue("AppletCode") >>= aStr;
    mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CODE, aStr );

    // export draw:may-script
    bool bIsScript = false;
    xPropSet->getPropertyValue("AppletIsScript") >>= bIsScript;
    mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_MAY_SCRIPT, bIsScript ? XML_TRUE : XML_FALSE );

    {
        // write applet
        SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DRAW, XML_APPLET, true, true);

        // export parameters
        uno::Sequence< beans::PropertyValue > aCommands;
        xPropSet->getPropertyValue("AppletCommands") >>= aCommands;
        const sal_Int32 nCount = aCommands.getLength();
        for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
        {
            aCommands[nIndex].Value >>= aStr;
            mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME, aCommands[nIndex].Name );
            mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_VALUE, aStr );
            SvXMLElementExport aElem( mrExport, XML_NAMESPACE_DRAW, XML_PARAM, false, true );
        }
    }

}

void XMLShapeExport::ImpExportPluginShape(
    const uno::Reference< drawing::XShape >& xShape,
    XMLShapeExportFlags nFeatures, css::awt::Point* pRefPoint)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(!xPropSet.is())
        return;

    // Transformation
    ImpExportNewTrans(xPropSet, nFeatures, pRefPoint);

    bool bCreateNewline( (nFeatures & XMLShapeExportFlags::NO_WS) == XMLShapeExportFlags::NONE ); // #86116#/#92210#
    SvXMLElementExport aElement( mrExport, XML_NAMESPACE_DRAW,
                              XML_FRAME, bCreateNewline, true );

    // export plugin url
    OUString aStr;
    xPropSet->getPropertyValue("PluginURL") >>= aStr;
    mrExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_HREF, GetExport().GetRelativeReference(aStr) );
    mrExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
    mrExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
    mrExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );

    // export mime-type
    xPropSet->getPropertyValue("PluginMimeType") >>= aStr;
    if(!aStr.isEmpty())
        mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_MIME_TYPE, aStr );

    {
        // write plugin
        SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DRAW, XML_PLUGIN, true, true);

        // export parameters
        uno::Sequence< beans::PropertyValue > aCommands;
        xPropSet->getPropertyValue("PluginCommands") >>= aCommands;
        const sal_Int32 nCount = aCommands.getLength();
        for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
        {
            aCommands[nIndex].Value >>= aStr;
            mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME, aCommands[nIndex].Name );
            mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_VALUE, aStr );
            SvXMLElementExport aElem( mrExport, XML_NAMESPACE_DRAW, XML_PARAM, false, true );
        }
    }

}

static void lcl_CopyStream(
        uno::Reference<io::XInputStream> const& xInStream,
        uno::Reference<embed::XStorage> const& xTarget,
        OUString const& rPath, const OUString& rMimeType)
{
    ::comphelper::LifecycleProxy proxy;
    uno::Reference<io::XStream> const xStream(
        ::comphelper::OStorageHelper::GetStreamAtPackageURL(xTarget, rPath,
            embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE, proxy));
    uno::Reference<io::XOutputStream> const xOutStream(
            (xStream.is()) ? xStream->getOutputStream() : nullptr);
    if (!xOutStream.is())
    {
        SAL_WARN("xmloff", "no output stream");
        throw uno::Exception("no output stream",nullptr);
    }
    uno::Reference< beans::XPropertySet > const xStreamProps(xStream,
        uno::UNO_QUERY);
    if (xStreamProps.is()) { // this is NOT supported in FileSystemStorage
        xStreamProps->setPropertyValue("MediaType",
            uno::makeAny(rMimeType));
        xStreamProps->setPropertyValue( // turn off compression
            "Compressed",
            uno::makeAny(false));
    }
    ::comphelper::OStorageHelper::CopyInputToOutput(xInStream, xOutStream);
    xOutStream->closeOutput();
    proxy.commitStorages();
}

static OUString
lcl_StoreMediaAndGetURL(SvXMLExport & rExport,
    uno::Reference<beans::XPropertySet> const& xPropSet,
    OUString const& rURL, const OUString& rMimeType)
{
    OUString urlPath;
    if (rURL.startsWithIgnoreAsciiCase("vnd.sun.star.Package:", &urlPath))
    {
        try // video is embedded
        {
            uno::Reference<embed::XStorage> const xTarget(
                    rExport.GetTargetStorage(), uno::UNO_QUERY_THROW);
            uno::Reference<io::XInputStream> xInStream;
            xPropSet->getPropertyValue("PrivateStream")
                    >>= xInStream;

            if (!xInStream.is())
            {
                SAL_WARN("xmloff", "no input stream");
                return OUString();
            }

            lcl_CopyStream(xInStream, xTarget, rURL, rMimeType);

            return urlPath;
        }
        catch (uno::Exception const& e)
        {
            SAL_INFO("xmloff", "exception while storing embedded media: " << e);
        }
        return OUString();
    }
    else
    {
        return rExport.GetRelativeReference(rURL); // linked
    }
}

void XMLShapeExport::ImpExportMediaShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, XMLShapeExportFlags nFeatures, css::awt::Point* pRefPoint)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(!xPropSet.is())
        return;

    // Transformation
    ImpExportNewTrans(xPropSet, nFeatures, pRefPoint);

    if(eShapeType == XmlShapeTypePresMediaShape)
    {
        (void)ImpExportPresentationAttributes( xPropSet, GetXMLToken(XML_PRESENTATION_OBJECT) );
    }
    bool bCreateNewline( (nFeatures & XMLShapeExportFlags::NO_WS) == XMLShapeExportFlags::NONE ); // #86116#/#92210#
    SvXMLElementExport aElem( mrExport, XML_NAMESPACE_DRAW,
                              XML_FRAME, bCreateNewline, true );

    // export media url
    OUString aMediaURL;
    xPropSet->getPropertyValue("MediaURL") >>= aMediaURL;
    OUString sMimeType;
    xPropSet->getPropertyValue("MediaMimeType") >>= sMimeType;

    OUString const persistentURL =
        lcl_StoreMediaAndGetURL(GetExport(), xPropSet, aMediaURL, sMimeType);

    mrExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_HREF, persistentURL );
    mrExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
    mrExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
    mrExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );

    // export mime-type
    mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_MIME_TYPE, sMimeType );

    // write plugin
    auto pPluginOBJ = std::make_unique<SvXMLElementExport>(mrExport, XML_NAMESPACE_DRAW, XML_PLUGIN, !( nFeatures & XMLShapeExportFlags::NO_WS ), true);

    // export parameters
    const OUString aFalseStr(  "false"  ), aTrueStr(  "true"  );

    bool bLoop = false;
    const OUString aLoopStr(  "Loop"  );
    xPropSet->getPropertyValue( aLoopStr ) >>= bLoop;
    mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME, aLoopStr );
    mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_VALUE, bLoop ? aTrueStr : aFalseStr );
    delete new SvXMLElementExport( mrExport, XML_NAMESPACE_DRAW, XML_PARAM, false, true );

    bool bMute = false;
    const OUString aMuteStr(  "Mute"  );
    xPropSet->getPropertyValue( aMuteStr ) >>= bMute;
    mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME, aMuteStr );
    mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_VALUE, bMute ? aTrueStr : aFalseStr );
    delete new SvXMLElementExport( mrExport, XML_NAMESPACE_DRAW, XML_PARAM, false, true );

    sal_Int16 nVolumeDB = 0;
    const OUString aVolumeDBStr(  "VolumeDB"  );
    xPropSet->getPropertyValue("VolumeDB") >>= nVolumeDB;
    mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME, aVolumeDBStr );
    mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_VALUE, OUString::number( nVolumeDB ) );
    delete new SvXMLElementExport( mrExport, XML_NAMESPACE_DRAW, XML_PARAM, false, true );

    media::ZoomLevel eZoom;
    const OUString aZoomStr(  "Zoom"  );
    OUString aZoomValue;
    xPropSet->getPropertyValue("Zoom") >>= eZoom;
    switch( eZoom )
    {
        case media::ZoomLevel_ZOOM_1_TO_4  : aZoomValue = "25%"; break;
        case media::ZoomLevel_ZOOM_1_TO_2  : aZoomValue = "50%"; break;
        case media::ZoomLevel_ORIGINAL     : aZoomValue = "100%"; break;
        case media::ZoomLevel_ZOOM_2_TO_1  : aZoomValue = "200%"; break;
        case media::ZoomLevel_ZOOM_4_TO_1  : aZoomValue = "400%"; break;
        case media::ZoomLevel_FIT_TO_WINDOW: aZoomValue = "fit"; break;
        case media::ZoomLevel_FIT_TO_WINDOW_FIXED_ASPECT: aZoomValue = "fixedfit"; break;
        case media::ZoomLevel_FULLSCREEN   : aZoomValue = "fullscreen"; break;

        default:
        break;
    }

    if( !aZoomValue.isEmpty() )
    {
        mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME, aZoomStr );
        mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_VALUE, aZoomValue );
        delete new SvXMLElementExport( mrExport, XML_NAMESPACE_DRAW, XML_PARAM, false, true );
    }

}

void XMLShapeExport::ImpExport3DSceneShape( const uno::Reference< drawing::XShape >& xShape, XMLShapeExportFlags nFeatures, awt::Point* pRefPoint)
{
    uno::Reference< drawing::XShapes > xShapes(xShape, uno::UNO_QUERY);
    if(xShapes.is() && xShapes->getCount())
    {
        uno::Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_QUERY );
        SAL_WARN_IF( !xPropSet.is(), "xmloff", "XMLShapeExport::ImpExport3DSceneShape can't export a scene without a propertyset" );
        if( xPropSet.is() )
        {
            // Transformation
            ImpExportNewTrans(xPropSet, nFeatures, pRefPoint);

            // 3d attributes
            export3DSceneAttributes( xPropSet );

            // write 3DScene shape
            bool bCreateNewline( (nFeatures & XMLShapeExportFlags::NO_WS) == XMLShapeExportFlags::NONE ); // #86116#/#92210#
            SvXMLElementExport aOBJ( mrExport, XML_NAMESPACE_DR3D, XML_SCENE, bCreateNewline, true);

            ImpExportDescription( xShape ); // #i68101#
            ImpExportEvents( xShape );

            // write 3DSceneLights
            export3DLamps( xPropSet );

            // #89764# if export of position is suppressed for group shape,
            // positions of contained objects should be written relative to
            // the upper left edge of the group.
            awt::Point aUpperLeft;

            if(!(nFeatures & XMLShapeExportFlags::POSITION))
            {
                nFeatures |= XMLShapeExportFlags::POSITION;
                aUpperLeft = xShape->getPosition();
                pRefPoint = &aUpperLeft;
            }

            // write members
            exportShapes( xShapes, nFeatures, pRefPoint );
        }
    }
}

void XMLShapeExport::ImpExport3DShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        OUString aStr;
        OUStringBuffer sStringBuffer;

        // transformation (UNO_NAME_3D_TRANSFORM_MATRIX == "D3DTransformMatrix")
        uno::Any aAny = xPropSet->getPropertyValue("D3DTransformMatrix");
        drawing::HomogenMatrix aHomMat;
        aAny >>= aHomMat;
        SdXMLImExTransform3D aTransform;
        aTransform.AddHomogenMatrix(aHomMat);
        if(aTransform.NeedsAction())
            mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_TRANSFORM, aTransform.GetExportString(mrExport.GetMM100UnitConverter()));

        switch(eShapeType)
        {
            case XmlShapeTypeDraw3DCubeObject:
            {
                // minEdge
                aAny = xPropSet->getPropertyValue("D3DPosition");
                drawing::Position3D aPosition3D;
                aAny >>= aPosition3D;
                ::basegfx::B3DVector aPos3D(aPosition3D.PositionX, aPosition3D.PositionY, aPosition3D.PositionZ);

                // maxEdge
                aAny = xPropSet->getPropertyValue("D3DSize");
                drawing::Direction3D aDirection3D;
                aAny >>= aDirection3D;
                ::basegfx::B3DVector aDir3D(aDirection3D.DirectionX, aDirection3D.DirectionY, aDirection3D.DirectionZ);

                // transform maxEdge from distance to pos
                aDir3D = aPos3D + aDir3D;

                // write minEdge
                if(aPos3D != ::basegfx::B3DVector(-2500.0, -2500.0, -2500.0)) // write only when not default
                {
                    SvXMLUnitConverter::convertB3DVector(sStringBuffer, aPos3D);
                    aStr = sStringBuffer.makeStringAndClear();
                    mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_MIN_EDGE, aStr);
                }

                // write maxEdge
                if(aDir3D != ::basegfx::B3DVector(2500.0, 2500.0, 2500.0)) // write only when not default
                {
                    SvXMLUnitConverter::convertB3DVector(sStringBuffer, aDir3D);
                    aStr = sStringBuffer.makeStringAndClear();
                    mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_MAX_EDGE, aStr);
                }

                // write 3DCube shape
                // #i123542# Do this *after* the attributes are added, else these will be lost since opening
                // the scope will clear the global attribute list at the exporter
                SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DR3D, XML_CUBE, true, true);

                break;
            }
            case XmlShapeTypeDraw3DSphereObject:
            {
                // Center
                aAny = xPropSet->getPropertyValue("D3DPosition");
                drawing::Position3D aPosition3D;
                aAny >>= aPosition3D;
                ::basegfx::B3DVector aPos3D(aPosition3D.PositionX, aPosition3D.PositionY, aPosition3D.PositionZ);

                // Size
                aAny = xPropSet->getPropertyValue("D3DSize");
                drawing::Direction3D aDirection3D;
                aAny >>= aDirection3D;
                ::basegfx::B3DVector aDir3D(aDirection3D.DirectionX, aDirection3D.DirectionY, aDirection3D.DirectionZ);

                // write Center
                if(aPos3D != ::basegfx::B3DVector(0.0, 0.0, 0.0)) // write only when not default
                {
                    SvXMLUnitConverter::convertB3DVector(sStringBuffer, aPos3D);
                    aStr = sStringBuffer.makeStringAndClear();
                    mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_CENTER, aStr);
                }

                // write Size
                if(aDir3D != ::basegfx::B3DVector(5000.0, 5000.0, 5000.0)) // write only when not default
                {
                    SvXMLUnitConverter::convertB3DVector(sStringBuffer, aDir3D);
                    aStr = sStringBuffer.makeStringAndClear();
                    mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_SIZE, aStr);
                }

                // write 3DSphere shape
                // #i123542# Do this *after* the attributes are added, else these will be lost since opening
                // the scope will clear the global attribute list at the exporter
                SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DR3D, XML_SPHERE, true, true);

                break;
            }
            case XmlShapeTypeDraw3DLatheObject:
            case XmlShapeTypeDraw3DExtrudeObject:
            {
                // write special 3DLathe/3DExtrude attributes, get 3D tools::PolyPolygon as drawing::PolyPolygonShape3D
                aAny = xPropSet->getPropertyValue("D3DPolyPolygon3D");
                drawing::PolyPolygonShape3D aUnoPolyPolygon3D;
                aAny >>= aUnoPolyPolygon3D;

                // convert to 3D PolyPolygon
                const basegfx::B3DPolyPolygon aPolyPolygon3D(
                    basegfx::utils::UnoPolyPolygonShape3DToB3DPolyPolygon(
                        aUnoPolyPolygon3D));

                // convert to 2D tools::PolyPolygon using identity 3D transformation (just grep X and Y)
                const basegfx::B3DHomMatrix aB3DHomMatrixFor2DConversion;
                const basegfx::B2DPolyPolygon aPolyPolygon(
                    basegfx::utils::createB2DPolyPolygonFromB3DPolyPolygon(
                        aPolyPolygon3D,
                        aB3DHomMatrixFor2DConversion));

                // get 2D range of it
                const basegfx::B2DRange aPolyPolygonRange(aPolyPolygon.getB2DRange());

                // export ViewBox
                SdXMLImExViewBox aViewBox(
                    aPolyPolygonRange.getMinX(),
                    aPolyPolygonRange.getMinY(),
                    aPolyPolygonRange.getWidth(),
                    aPolyPolygonRange.getHeight());

                mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_VIEWBOX, aViewBox.GetExportString());

                // prepare svg:d string
                const OUString aPolygonString(
                    basegfx::utils::exportToSvgD(
                        aPolyPolygon,
                        true,           // bUseRelativeCoordinates
                        false,          // bDetectQuadraticBeziers TTTT: not used in old, but maybe activated now
                        true));         // bHandleRelativeNextPointCompatible

                // write point array
                mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_D, aPolygonString);

                if(eShapeType == XmlShapeTypeDraw3DLatheObject)
                {
                    // write 3DLathe shape
                    SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DR3D, XML_ROTATE, true, true);
                }
                else
                {
                    // write 3DExtrude shape
                    SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DR3D, XML_EXTRUDE, true, true);
                }
                break;
            }
            default:
                break;
        }
    }
}

/** helper for chart that adds all attributes of a 3d scene element to the export */
void XMLShapeExport::export3DSceneAttributes( const css::uno::Reference< css::beans::XPropertySet >& xPropSet )
{
    OUString aStr;
    OUStringBuffer sStringBuffer;

    // world transformation (UNO_NAME_3D_TRANSFORM_MATRIX == "D3DTransformMatrix")
    uno::Any aAny = xPropSet->getPropertyValue("D3DTransformMatrix");
    drawing::HomogenMatrix aHomMat;
    aAny >>= aHomMat;
    SdXMLImExTransform3D aTransform;
    aTransform.AddHomogenMatrix(aHomMat);
    if(aTransform.NeedsAction())
        mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_TRANSFORM, aTransform.GetExportString(mrExport.GetMM100UnitConverter()));

    // VRP, VPN, VUP
    aAny = xPropSet->getPropertyValue("D3DCameraGeometry");
    drawing::CameraGeometry aCamGeo;
    aAny >>= aCamGeo;

    ::basegfx::B3DVector aVRP(aCamGeo.vrp.PositionX, aCamGeo.vrp.PositionY, aCamGeo.vrp.PositionZ);
    if(aVRP != ::basegfx::B3DVector(0.0, 0.0, 1.0)) // write only when not default
    {
        SvXMLUnitConverter::convertB3DVector(sStringBuffer, aVRP);
        aStr = sStringBuffer.makeStringAndClear();
        mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_VRP, aStr);
    }

    ::basegfx::B3DVector aVPN(aCamGeo.vpn.DirectionX, aCamGeo.vpn.DirectionY, aCamGeo.vpn.DirectionZ);
    if(aVPN != ::basegfx::B3DVector(0.0, 0.0, 1.0)) // write only when not default
    {
        SvXMLUnitConverter::convertB3DVector(sStringBuffer, aVPN);
        aStr = sStringBuffer.makeStringAndClear();
        mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_VPN, aStr);
    }

    ::basegfx::B3DVector aVUP(aCamGeo.vup.DirectionX, aCamGeo.vup.DirectionY, aCamGeo.vup.DirectionZ);
    if(aVUP != ::basegfx::B3DVector(0.0, 1.0, 0.0)) // write only when not default
    {
        SvXMLUnitConverter::convertB3DVector(sStringBuffer, aVUP);
        aStr = sStringBuffer.makeStringAndClear();
        mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_VUP, aStr);
    }

    // projection "D3DScenePerspective" drawing::ProjectionMode
    aAny = xPropSet->getPropertyValue("D3DScenePerspective");
    drawing::ProjectionMode aPrjMode;
    aAny >>= aPrjMode;
    if(aPrjMode == drawing::ProjectionMode_PARALLEL)
        aStr = GetXMLToken(XML_PARALLEL);
    else
        aStr = GetXMLToken(XML_PERSPECTIVE);
    mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_PROJECTION, aStr);

    // distance
    aAny = xPropSet->getPropertyValue("D3DSceneDistance");
    sal_Int32 nDistance = 0;
    aAny >>= nDistance;
    mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
            nDistance);
    aStr = sStringBuffer.makeStringAndClear();
    mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_DISTANCE, aStr);

    // focalLength
    aAny = xPropSet->getPropertyValue("D3DSceneFocalLength");
    sal_Int32 nFocalLength = 0;
    aAny >>= nFocalLength;
    mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
            nFocalLength);
    aStr = sStringBuffer.makeStringAndClear();
    mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_FOCAL_LENGTH, aStr);

    // shadowSlant
    aAny = xPropSet->getPropertyValue("D3DSceneShadowSlant");
    sal_Int16 nShadowSlant = 0;
    aAny >>= nShadowSlant;
    mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_SHADOW_SLANT, OUString::number(static_cast<sal_Int32>(nShadowSlant)));

    // shadeMode
    aAny = xPropSet->getPropertyValue("D3DSceneShadeMode");
    drawing::ShadeMode aShadeMode;
    if(aAny >>= aShadeMode)
    {
        if(aShadeMode == drawing::ShadeMode_FLAT)
            aStr = GetXMLToken(XML_FLAT);
        else if(aShadeMode == drawing::ShadeMode_PHONG)
            aStr = GetXMLToken(XML_PHONG);
        else if(aShadeMode == drawing::ShadeMode_SMOOTH)
            aStr = GetXMLToken(XML_GOURAUD);
        else
            aStr = GetXMLToken(XML_DRAFT);
    }
    else
    {
        // ShadeMode enum not there, write default
        aStr = GetXMLToken(XML_GOURAUD);
    }
    mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_SHADE_MODE, aStr);

    // ambientColor
    aAny = xPropSet->getPropertyValue("D3DSceneAmbientColor");
    sal_Int32 nAmbientColor = 0;
    aAny >>= nAmbientColor;
    ::sax::Converter::convertColor(sStringBuffer, nAmbientColor);
    aStr = sStringBuffer.makeStringAndClear();
    mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_AMBIENT_COLOR, aStr);

    // lightingMode
    aAny = xPropSet->getPropertyValue("D3DSceneTwoSidedLighting");
    bool bTwoSidedLighting = false;
    aAny >>= bTwoSidedLighting;
    ::sax::Converter::convertBool(sStringBuffer, bTwoSidedLighting);
    aStr = sStringBuffer.makeStringAndClear();
    mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_LIGHTING_MODE, aStr);
}

/** helper for chart that exports all lamps from the propertyset */
void XMLShapeExport::export3DLamps( const css::uno::Reference< css::beans::XPropertySet >& xPropSet )
{
    // write lamps 1..8 as content
    OUString aStr;
    OUStringBuffer sStringBuffer;

    const OUString aColorPropName("D3DSceneLightColor");
    const OUString aDirectionPropName("D3DSceneLightDirection");
    const OUString aLightOnPropName("D3DSceneLightOn");

    ::basegfx::B3DVector aLightDirection;
    drawing::Direction3D aLightDir;
    bool bLightOnOff = false;
    for(sal_Int32 nLamp = 1; nLamp <= 8; nLamp++)
    {
        OUString aIndexStr = OUString::number( nLamp );

        // lightcolor
        OUString aPropName = aColorPropName + aIndexStr;
        sal_Int32 nLightColor = 0;
        xPropSet->getPropertyValue( aPropName ) >>= nLightColor;
        ::sax::Converter::convertColor(sStringBuffer, nLightColor);
        aStr = sStringBuffer.makeStringAndClear();
        mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_DIFFUSE_COLOR, aStr);

        // lightdirection
        aPropName = aDirectionPropName + aIndexStr;
        xPropSet->getPropertyValue(aPropName) >>= aLightDir;
        aLightDirection = ::basegfx::B3DVector(aLightDir.DirectionX, aLightDir.DirectionY, aLightDir.DirectionZ);
        SvXMLUnitConverter::convertB3DVector(sStringBuffer, aLightDirection);
        aStr = sStringBuffer.makeStringAndClear();
        mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_DIRECTION, aStr);

        // lighton
        aPropName = aLightOnPropName + aIndexStr;
        xPropSet->getPropertyValue(aPropName) >>= bLightOnOff;
        ::sax::Converter::convertBool(sStringBuffer, bLightOnOff);
        aStr = sStringBuffer.makeStringAndClear();
        mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_ENABLED, aStr);

        // specular
        mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_SPECULAR,
            nLamp == 1 ? XML_TRUE : XML_FALSE);

        // write light entry
        SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DR3D, XML_LIGHT, true, true);
    }
}


// using namespace css::io;
// using namespace ::xmloff::EnhancedCustomShapeToken;


static void ExportParameter( OUStringBuffer& rStrBuffer, const css::drawing::EnhancedCustomShapeParameter& rParameter )
{
    if ( !rStrBuffer.isEmpty() )
        rStrBuffer.append( ' ' );
    if ( rParameter.Value.getValueTypeClass() == uno::TypeClass_DOUBLE )
    {
        double fNumber = 0.0;
        rParameter.Value >>= fNumber;
        ::rtl::math::doubleToUStringBuffer( rStrBuffer, fNumber, rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max, '.', true );
    }
    else
    {
        sal_Int32 nValue = 0;
        rParameter.Value >>= nValue;

        switch( rParameter.Type )
        {
            case css::drawing::EnhancedCustomShapeParameterType::EQUATION :
            {
                rStrBuffer.append( "?f" ).append(OUString::number( nValue ) );
            }
            break;

            case css::drawing::EnhancedCustomShapeParameterType::ADJUSTMENT :
            {
                rStrBuffer.append( '$' );
                rStrBuffer.append( OUString::number( nValue ) );
            }
            break;

            case css::drawing::EnhancedCustomShapeParameterType::BOTTOM :
                rStrBuffer.append( GetXMLToken( XML_BOTTOM ) ); break;
            case css::drawing::EnhancedCustomShapeParameterType::RIGHT :
                rStrBuffer.append( GetXMLToken( XML_RIGHT ) ); break;
            case css::drawing::EnhancedCustomShapeParameterType::TOP :
                rStrBuffer.append( GetXMLToken( XML_TOP ) ); break;
            case css::drawing::EnhancedCustomShapeParameterType::LEFT :
                rStrBuffer.append( GetXMLToken( XML_LEFT ) ); break;
            case css::drawing::EnhancedCustomShapeParameterType::XSTRETCH :
                rStrBuffer.append( GetXMLToken( XML_XSTRETCH ) ); break;
            case css::drawing::EnhancedCustomShapeParameterType::YSTRETCH :
                rStrBuffer.append( GetXMLToken( XML_YSTRETCH ) ); break;
            case css::drawing::EnhancedCustomShapeParameterType::HASSTROKE :
                rStrBuffer.append( GetXMLToken( XML_HASSTROKE ) ); break;
            case css::drawing::EnhancedCustomShapeParameterType::HASFILL :
                rStrBuffer.append( GetXMLToken( XML_HASFILL ) ); break;
            case css::drawing::EnhancedCustomShapeParameterType::WIDTH :
                rStrBuffer.append( GetXMLToken( XML_WIDTH ) ); break;
            case css::drawing::EnhancedCustomShapeParameterType::HEIGHT :
                rStrBuffer.append( GetXMLToken( XML_HEIGHT ) ); break;
            case css::drawing::EnhancedCustomShapeParameterType::LOGWIDTH :
                rStrBuffer.append( GetXMLToken( XML_LOGWIDTH ) ); break;
            case css::drawing::EnhancedCustomShapeParameterType::LOGHEIGHT :
                rStrBuffer.append( GetXMLToken( XML_LOGHEIGHT ) ); break;
            default :
                rStrBuffer.append( OUString::number( nValue ) );
        }
    }
}

static void ImpExportEquations( SvXMLExport& rExport, const uno::Sequence< OUString >& rEquations )
{
    sal_Int32 i;
    for ( i = 0; i < rEquations.getLength(); i++ )
    {
        OUString aStr('f');
        aStr += OUString::number( i );
        rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME, aStr );

        aStr = rEquations[ i ];
        sal_Int32 nIndex = 0;
        do
        {
            nIndex = aStr.indexOf( '?', nIndex );
            if ( nIndex != -1 )
            {
                aStr = aStr.copy(0, nIndex + 1) + "f"
                    + aStr.copy(nIndex + 1, aStr.getLength() - nIndex - 1);
                nIndex++;
            }
        } while( nIndex != -1 );
        rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_FORMULA, aStr );
        SvXMLElementExport aOBJ( rExport, XML_NAMESPACE_DRAW, XML_EQUATION, true, true );
    }
}

static void ImpExportHandles( SvXMLExport& rExport, const uno::Sequence< beans::PropertyValues >& rHandles )
{
    sal_uInt32 i, j, nElements = rHandles.getLength();
    if ( nElements )
    {
        OUString       aStr;
        OUStringBuffer aStrBuffer;

        for ( i = 0; i < nElements; i++ )
        {
            bool bPosition = false;
            const uno::Sequence< beans::PropertyValue >& rPropSeq = rHandles[ i ];
            for ( j = 0; j < static_cast<sal_uInt32>(rPropSeq.getLength()); j++ )
            {
                const beans::PropertyValue& rPropVal = rPropSeq[ j ];
                switch( EASGet( rPropVal.Name ) )
                {
                    case EAS_Position :
                    {
                        css::drawing::EnhancedCustomShapeParameterPair aPosition;
                        if ( rPropVal.Value >>= aPosition )
                        {
                            ExportParameter( aStrBuffer, aPosition.First );
                            ExportParameter( aStrBuffer, aPosition.Second );
                            aStr = aStrBuffer.makeStringAndClear();
                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_HANDLE_POSITION, aStr );
                            bPosition = true;
                        }
                    }
                    break;
                    case EAS_MirroredX :
                    {
                        bool bMirroredX;
                        if ( rPropVal.Value >>= bMirroredX )
                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_HANDLE_MIRROR_HORIZONTAL,
                                bMirroredX ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                    }
                    break;
                    case EAS_MirroredY :
                    {
                        bool bMirroredY;
                        if ( rPropVal.Value >>= bMirroredY )
                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_HANDLE_MIRROR_VERTICAL,
                                bMirroredY ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                    }
                    break;
                    case EAS_Switched :
                    {
                        bool bSwitched;
                        if ( rPropVal.Value >>= bSwitched )
                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_HANDLE_SWITCHED,
                                bSwitched ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                    }
                    break;
                    case EAS_Polar :
                    {
                        css::drawing::EnhancedCustomShapeParameterPair aPolar;
                        if ( rPropVal.Value >>= aPolar )
                        {
                            ExportParameter( aStrBuffer, aPolar.First );
                            ExportParameter( aStrBuffer, aPolar.Second );
                            aStr = aStrBuffer.makeStringAndClear();
                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_HANDLE_POLAR, aStr );
                        }
                    }
                    break;
                    case EAS_RadiusRangeMinimum :
                    {
                        css::drawing::EnhancedCustomShapeParameter aRadiusRangeMinimum;
                        if ( rPropVal.Value >>= aRadiusRangeMinimum )
                        {
                            ExportParameter( aStrBuffer, aRadiusRangeMinimum );
                            aStr = aStrBuffer.makeStringAndClear();
                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_HANDLE_RADIUS_RANGE_MINIMUM, aStr );
                        }
                    }
                    break;
                    case EAS_RadiusRangeMaximum :
                    {
                        css::drawing::EnhancedCustomShapeParameter aRadiusRangeMaximum;
                        if ( rPropVal.Value >>= aRadiusRangeMaximum )
                        {
                            ExportParameter( aStrBuffer, aRadiusRangeMaximum );
                            aStr = aStrBuffer.makeStringAndClear();
                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_HANDLE_RADIUS_RANGE_MAXIMUM, aStr );
                        }
                    }
                    break;
                    case EAS_RangeXMinimum :
                    {
                        css::drawing::EnhancedCustomShapeParameter aXRangeMinimum;
                        if ( rPropVal.Value >>= aXRangeMinimum )
                        {
                            ExportParameter( aStrBuffer, aXRangeMinimum );
                            aStr = aStrBuffer.makeStringAndClear();
                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_HANDLE_RANGE_X_MINIMUM, aStr );
                        }
                    }
                    break;
                    case EAS_RangeXMaximum :
                    {
                        css::drawing::EnhancedCustomShapeParameter aXRangeMaximum;
                        if ( rPropVal.Value >>= aXRangeMaximum )
                        {
                            ExportParameter( aStrBuffer, aXRangeMaximum );
                            aStr = aStrBuffer.makeStringAndClear();
                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_HANDLE_RANGE_X_MAXIMUM, aStr );
                        }
                    }
                    break;
                    case EAS_RangeYMinimum :
                    {
                        css::drawing::EnhancedCustomShapeParameter aYRangeMinimum;
                        if ( rPropVal.Value >>= aYRangeMinimum )
                        {
                            ExportParameter( aStrBuffer, aYRangeMinimum );
                            aStr = aStrBuffer.makeStringAndClear();
                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_HANDLE_RANGE_Y_MINIMUM, aStr );
                        }
                    }
                    break;
                    case EAS_RangeYMaximum :
                    {
                        css::drawing::EnhancedCustomShapeParameter aYRangeMaximum;
                        if ( rPropVal.Value >>= aYRangeMaximum )
                        {
                            ExportParameter( aStrBuffer, aYRangeMaximum );
                            aStr = aStrBuffer.makeStringAndClear();
                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_HANDLE_RANGE_Y_MAXIMUM, aStr );
                        }
                    }
                    break;
                    default:
                        break;
                }
            }
            if ( bPosition )
                SvXMLElementExport aOBJ( rExport, XML_NAMESPACE_DRAW, XML_HANDLE, true, true );
            else
                rExport.ClearAttrList();
        }
    }
}

static void ImpExportEnhancedPath( SvXMLExport& rExport,
                            const uno::Sequence< css::drawing::EnhancedCustomShapeParameterPair >& rCoordinates,
                            const uno::Sequence< css::drawing::EnhancedCustomShapeSegment >& rSegments,
                            bool bExtended = false )
{

    OUString       aStr;
    OUStringBuffer aStrBuffer;
    bool bNeedExtended = false;

    sal_Int32 i, j, k, l;

    sal_Int32 nCoords = rCoordinates.getLength();
    sal_Int32 nSegments = rSegments.getLength();
    bool bSimpleSegments = nSegments == 0;
    if ( bSimpleSegments )
        nSegments = 4;
    for ( j = i = 0; j < nSegments; j++ )
    {
        css::drawing::EnhancedCustomShapeSegment aSegment;
        if ( bSimpleSegments )
        {
            // if there are not enough segments we will default them
            switch( j )
            {
                case 0 :
                {
                    aSegment.Count = 1;
                    aSegment.Command = css::drawing::EnhancedCustomShapeSegmentCommand::MOVETO;
                }
                break;
                case 1 :
                {
                    aSegment.Count = static_cast<sal_Int16>(std::min( nCoords - 1, sal_Int32(32767) ));
                    aSegment.Command = css::drawing::EnhancedCustomShapeSegmentCommand::LINETO;
                }
                break;
                case 2 :
                {
                    aSegment.Count = 1;
                    aSegment.Command = css::drawing::EnhancedCustomShapeSegmentCommand::CLOSESUBPATH;
                }
                break;
                case 3 :
                {
                    aSegment.Count = 1;
                    aSegment.Command = css::drawing::EnhancedCustomShapeSegmentCommand::ENDSUBPATH;
                }
                break;
            }
        }
        else
            aSegment = rSegments[ j ];

        if ( !aStrBuffer.isEmpty() )
            aStrBuffer.append( ' ' );

        sal_Int32 nParameter = 0;
        switch( aSegment.Command )
        {
            case css::drawing::EnhancedCustomShapeSegmentCommand::CLOSESUBPATH :
                aStrBuffer.append( 'Z' ); break;
            case css::drawing::EnhancedCustomShapeSegmentCommand::ENDSUBPATH :
                aStrBuffer.append( 'N' ); break;
            case css::drawing::EnhancedCustomShapeSegmentCommand::NOFILL :
                aStrBuffer.append( 'F' ); break;
            case css::drawing::EnhancedCustomShapeSegmentCommand::NOSTROKE :
                aStrBuffer.append( 'S' ); break;

            case css::drawing::EnhancedCustomShapeSegmentCommand::MOVETO :
                aStrBuffer.append( 'M' ); nParameter = 1; break;
            case css::drawing::EnhancedCustomShapeSegmentCommand::LINETO :
                aStrBuffer.append( 'L' ); nParameter = 1; break;
            case css::drawing::EnhancedCustomShapeSegmentCommand::CURVETO :
                aStrBuffer.append( 'C' ); nParameter = 3; break;
            case css::drawing::EnhancedCustomShapeSegmentCommand::ANGLEELLIPSETO :
                aStrBuffer.append( 'T' ); nParameter = 3; break;
            case css::drawing::EnhancedCustomShapeSegmentCommand::ANGLEELLIPSE :
                aStrBuffer.append( 'U' ); nParameter = 3; break;
            case css::drawing::EnhancedCustomShapeSegmentCommand::ARCTO :
                aStrBuffer.append( 'A' ); nParameter = 4; break;
            case css::drawing::EnhancedCustomShapeSegmentCommand::ARC :
                aStrBuffer.append( 'B' ); nParameter = 4; break;
            case css::drawing::EnhancedCustomShapeSegmentCommand::CLOCKWISEARCTO :
                aStrBuffer.append( 'W' ); nParameter = 4; break;
            case css::drawing::EnhancedCustomShapeSegmentCommand::CLOCKWISEARC :
                aStrBuffer.append( 'V' ); nParameter = 4; break;
            case css::drawing::EnhancedCustomShapeSegmentCommand::ELLIPTICALQUADRANTX :
                aStrBuffer.append( 'X' ); nParameter = 1; break;
            case css::drawing::EnhancedCustomShapeSegmentCommand::ELLIPTICALQUADRANTY :
                aStrBuffer.append( 'Y' ); nParameter = 1; break;
            case css::drawing::EnhancedCustomShapeSegmentCommand::QUADRATICCURVETO :
                aStrBuffer.append( 'Q' ); nParameter = 2; break;
            case css::drawing::EnhancedCustomShapeSegmentCommand::ARCANGLETO :
                if ( bExtended ) {
                    aStrBuffer.append( 'G' );
                    nParameter = 2;
                } else {
                    aStrBuffer.setLength( aStrBuffer.getLength() - 1);
                    bNeedExtended = true;
                    i += 2;
                }
                break;
            case css::drawing::EnhancedCustomShapeSegmentCommand::DARKEN :
                if ( bExtended )
                    aStrBuffer.append( 'H' );
                else
                    bNeedExtended = true;
                break;
            case css::drawing::EnhancedCustomShapeSegmentCommand::DARKENLESS :
                if ( bExtended )
                    aStrBuffer.append( 'I' );
                else
                    bNeedExtended = true;
                break;
            case css::drawing::EnhancedCustomShapeSegmentCommand::LIGHTEN :
                if ( bExtended )
                    aStrBuffer.append( 'J' );
                else
                    bNeedExtended = true;
                break;
            case css::drawing::EnhancedCustomShapeSegmentCommand::LIGHTENLESS :
                if ( bExtended )
                    aStrBuffer.append( 'K' );
                else
                    bNeedExtended = true;
                break;
            default : // ups, seems to be something wrong
            {
                aSegment.Count = 1;
                aSegment.Command = css::drawing::EnhancedCustomShapeSegmentCommand::LINETO;
            }
            break;
        }
        if ( nParameter )
        {
            for ( k = 0; k < aSegment.Count; k++ )
            {
                if ( ( i + nParameter ) <= nCoords )
                {
                    for ( l = 0; l < nParameter; l++ )
                    {
                        ExportParameter( aStrBuffer, rCoordinates[ i ].First );
                        ExportParameter( aStrBuffer, rCoordinates[ i++ ].Second );
                    }
                }
                else
                {
                    j = nSegments;  // error -> exiting
                    break;
                }
            }
        }
    }
    aStr = aStrBuffer.makeStringAndClear();
    rExport.AddAttribute( bExtended ? XML_NAMESPACE_DRAW_EXT : XML_NAMESPACE_DRAW, XML_ENHANCED_PATH, aStr );
    if ( !bExtended && bNeedExtended && (rExport.getDefaultVersion() > SvtSaveOptions::ODFVER_012) )
        ImpExportEnhancedPath( rExport, rCoordinates, rSegments, true );
}

static void ImpExportEnhancedGeometry( SvXMLExport& rExport, const uno::Reference< beans::XPropertySet >& xPropSet )
{
    bool bEquations = false;
    uno::Sequence< OUString > aEquations;

    bool bHandles = false;
    uno::Sequence< beans::PropertyValues > aHandles;

    uno::Sequence< css::drawing::EnhancedCustomShapeSegment > aSegments;
    uno::Sequence< css::drawing::EnhancedCustomShapeParameterPair > aCoordinates;

    uno::Sequence< css::drawing::EnhancedCustomShapeAdjustmentValue > aAdjustmentValues;

    OUString       aStr;
    OUStringBuffer aStrBuffer;
    SvXMLUnitConverter& rUnitConverter = rExport.GetMM100UnitConverter();

    uno::Reference< beans::XPropertySetInfo > xPropSetInfo( xPropSet->getPropertySetInfo() );

    // geometry
    const OUString sCustomShapeGeometry( "CustomShapeGeometry" );
    if ( xPropSetInfo.is() && xPropSetInfo->hasPropertyByName( sCustomShapeGeometry ) )
    {
        uno::Any aGeoPropSet( xPropSet->getPropertyValue( sCustomShapeGeometry ) );
        uno::Sequence< beans::PropertyValue > aGeoPropSeq;

        if ( aGeoPropSet >>= aGeoPropSeq )
        {
            bool bCoordinates = false;
            OUString aCustomShapeType( "non-primitive" );

            sal_Int32 j, nGeoPropCount = aGeoPropSeq.getLength();
            for ( j = 0; j < nGeoPropCount; j++ )
            {
                const beans::PropertyValue& rGeoProp = aGeoPropSeq[ j ];
                switch( EASGet( rGeoProp.Name ) )
                {
                    case EAS_Type :
                    {
                        rGeoProp.Value >>= aCustomShapeType;
                    }
                    break;
                    case EAS_MirroredX :
                    {
                        bool bMirroredX;
                        if ( rGeoProp.Value >>= bMirroredX )
                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_MIRROR_HORIZONTAL,
                                bMirroredX ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                    }
                    break;
                    case EAS_MirroredY :
                    {
                        bool bMirroredY;
                        if ( rGeoProp.Value >>= bMirroredY )
                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_MIRROR_VERTICAL,
                                bMirroredY ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                    }
                    break;
                    case EAS_ViewBox :
                    {
                        awt::Rectangle aRect;
                        if ( rGeoProp.Value >>= aRect )
                        {
                            SdXMLImExViewBox aViewBox( aRect.X, aRect.Y, aRect.Width, aRect.Height );
                            rExport.AddAttribute( XML_NAMESPACE_SVG, XML_VIEWBOX, aViewBox.GetExportString() );
                        }
                    }
                    break;
                    case EAS_TextPreRotateAngle :
                    case EAS_TextRotateAngle :
                    {
                        double fTextRotateAngle = 0;
                        if ( ( rGeoProp.Value >>= fTextRotateAngle ) && fTextRotateAngle != 0 )
                        {
                            ::sax::Converter::convertDouble(
                                    aStrBuffer, fTextRotateAngle );
                            aStr = aStrBuffer.makeStringAndClear();
                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_TEXT_ROTATE_ANGLE, aStr );
                        }
                    }
                    break;
                    case EAS_Extrusion :
                    {
                        uno::Sequence< beans::PropertyValue > aExtrusionPropSeq;
                        if ( rGeoProp.Value >>= aExtrusionPropSeq )
                        {
                            sal_Int32 i, nCount = aExtrusionPropSeq.getLength();
                            for ( i = 0; i < nCount; i++ )
                            {
                                const beans::PropertyValue& rProp = aExtrusionPropSeq[ i ];
                                switch( EASGet( rProp.Name ) )
                                {
                                    case EAS_Extrusion :
                                    {
                                        bool bExtrusionOn;
                                        if ( rProp.Value >>= bExtrusionOn )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION,
                                                bExtrusionOn ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                                    }
                                    break;
                                    case EAS_Brightness :
                                    {
                                        double fExtrusionBrightness = 0;
                                        if ( rProp.Value >>= fExtrusionBrightness )
                                        {
                                            ::sax::Converter::convertDouble(
                                                aStrBuffer,
                                                fExtrusionBrightness,
                                                false,
                                                util::MeasureUnit::PERCENT,
                                                util::MeasureUnit::PERCENT);
                                            aStrBuffer.append( '%' );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_BRIGHTNESS, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_Depth :
                                    {
                                        css::drawing::EnhancedCustomShapeParameterPair aDepthParaPair;
                                        if ( rProp.Value >>= aDepthParaPair )
                                        {
                                            double fDepth = 0;
                                            if ( aDepthParaPair.First.Value >>= fDepth )
                                            {
                                                rExport.GetMM100UnitConverter().convertDouble( aStrBuffer, fDepth );
                                                ExportParameter( aStrBuffer, aDepthParaPair.Second );
                                                aStr = aStrBuffer.makeStringAndClear();
                                                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_DEPTH, aStr );
                                            }
                                        }
                                    }
                                    break;
                                    case EAS_Diffusion :
                                    {
                                        double fExtrusionDiffusion = 0;
                                        if ( rProp.Value >>= fExtrusionDiffusion )
                                        {
                                            ::sax::Converter::convertDouble(
                                                aStrBuffer,
                                                fExtrusionDiffusion,
                                                false,
                                                util::MeasureUnit::PERCENT,
                                                util::MeasureUnit::PERCENT);
                                            aStrBuffer.append( '%' );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_DIFFUSION, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_NumberOfLineSegments :
                                    {
                                        sal_Int32 nExtrusionNumberOfLineSegments = 0;
                                        if ( rProp.Value >>= nExtrusionNumberOfLineSegments )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_NUMBER_OF_LINE_SEGMENTS, OUString::number( nExtrusionNumberOfLineSegments ) );
                                    }
                                    break;
                                    case EAS_LightFace :
                                    {
                                        bool bExtrusionLightFace;
                                        if ( rProp.Value >>= bExtrusionLightFace )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_LIGHT_FACE,
                                                bExtrusionLightFace ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                                    }
                                    break;
                                    case EAS_FirstLightHarsh :
                                    {
                                        bool bExtrusionFirstLightHarsh;
                                        if ( rProp.Value >>= bExtrusionFirstLightHarsh )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_FIRST_LIGHT_HARSH,
                                                bExtrusionFirstLightHarsh ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                                    }
                                    break;
                                    case EAS_SecondLightHarsh :
                                    {
                                        bool bExtrusionSecondLightHarsh;
                                        if ( rProp.Value >>= bExtrusionSecondLightHarsh )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_SECOND_LIGHT_HARSH,
                                                bExtrusionSecondLightHarsh ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                                    }
                                    break;
                                    case EAS_FirstLightLevel :
                                    {
                                        double fExtrusionFirstLightLevel = 0;
                                        if ( rProp.Value >>= fExtrusionFirstLightLevel )
                                        {
                                            ::sax::Converter::convertDouble(
                                                aStrBuffer,
                                                fExtrusionFirstLightLevel,
                                                false,
                                                util::MeasureUnit::PERCENT,
                                                util::MeasureUnit::PERCENT);
                                            aStrBuffer.append( '%' );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_FIRST_LIGHT_LEVEL, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_SecondLightLevel :
                                    {
                                        double fExtrusionSecondLightLevel = 0;
                                        if ( rProp.Value >>= fExtrusionSecondLightLevel )
                                        {
                                            ::sax::Converter::convertDouble(
                                                aStrBuffer,
                                                fExtrusionSecondLightLevel,
                                                false,
                                                util::MeasureUnit::PERCENT,
                                                util::MeasureUnit::PERCENT);
                                            aStrBuffer.append( '%' );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_SECOND_LIGHT_LEVEL, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_FirstLightDirection :
                                    {
                                        drawing::Direction3D aExtrusionFirstLightDirection;
                                        if ( rProp.Value >>= aExtrusionFirstLightDirection )
                                        {
                                            ::basegfx::B3DVector aVec3D( aExtrusionFirstLightDirection.DirectionX, aExtrusionFirstLightDirection.DirectionY,
                                                aExtrusionFirstLightDirection.DirectionZ );
                                            SvXMLUnitConverter::convertB3DVector( aStrBuffer, aVec3D );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_FIRST_LIGHT_DIRECTION, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_SecondLightDirection :
                                    {
                                        drawing::Direction3D aExtrusionSecondLightDirection;
                                        if ( rProp.Value >>= aExtrusionSecondLightDirection )
                                        {
                                            ::basegfx::B3DVector aVec3D( aExtrusionSecondLightDirection.DirectionX, aExtrusionSecondLightDirection.DirectionY,
                                                aExtrusionSecondLightDirection.DirectionZ );
                                            SvXMLUnitConverter::convertB3DVector( aStrBuffer, aVec3D );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_SECOND_LIGHT_DIRECTION, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_Metal :
                                    {
                                        bool bExtrusionMetal;
                                        if ( rProp.Value >>= bExtrusionMetal )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_METAL,
                                                bExtrusionMetal ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                                    }
                                    break;
                                    case EAS_ShadeMode :
                                    {
                                        // shadeMode
                                        drawing::ShadeMode eShadeMode;
                                        if( rProp.Value >>= eShadeMode )
                                        {
                                            if( eShadeMode == drawing::ShadeMode_FLAT )
                                                aStr = GetXMLToken( XML_FLAT );
                                            else if( eShadeMode == drawing::ShadeMode_PHONG )
                                                aStr = GetXMLToken( XML_PHONG );
                                            else if( eShadeMode == drawing::ShadeMode_SMOOTH )
                                                aStr = GetXMLToken( XML_GOURAUD );
                                            else
                                                aStr = GetXMLToken( XML_DRAFT );
                                        }
                                        else
                                        {
                                            // ShadeMode enum not there, write default
                                            aStr = GetXMLToken( XML_FLAT);
                                        }
                                        rExport.AddAttribute( XML_NAMESPACE_DR3D, XML_SHADE_MODE, aStr );
                                    }
                                    break;
                                    case EAS_RotateAngle :
                                    {
                                        css::drawing::EnhancedCustomShapeParameterPair aRotateAngleParaPair;
                                        if ( rProp.Value >>= aRotateAngleParaPair )
                                        {
                                            ExportParameter( aStrBuffer, aRotateAngleParaPair.First );
                                            ExportParameter( aStrBuffer, aRotateAngleParaPair.Second );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_ROTATION_ANGLE, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_RotationCenter :
                                    {
                                        drawing::Direction3D aExtrusionRotationCenter;
                                        if ( rProp.Value >>= aExtrusionRotationCenter )
                                        {
                                            ::basegfx::B3DVector aVec3D( aExtrusionRotationCenter.DirectionX, aExtrusionRotationCenter.DirectionY,
                                                aExtrusionRotationCenter.DirectionZ );
                                            SvXMLUnitConverter::convertB3DVector( aStrBuffer, aVec3D );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_ROTATION_CENTER, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_Shininess :
                                    {
                                        double fExtrusionShininess = 0;
                                        if ( rProp.Value >>= fExtrusionShininess )
                                        {
                                            ::sax::Converter::convertDouble(
                                                aStrBuffer,
                                                fExtrusionShininess,
                                                false,
                                                util::MeasureUnit::PERCENT,
                                                util::MeasureUnit::PERCENT);
                                            aStrBuffer.append( '%' );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_SHININESS, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_Skew :
                                    {
                                        css::drawing::EnhancedCustomShapeParameterPair aSkewParaPair;
                                        if ( rProp.Value >>= aSkewParaPair )
                                        {
                                            ExportParameter( aStrBuffer, aSkewParaPair.First );
                                            ExportParameter( aStrBuffer, aSkewParaPair.Second );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_SKEW, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_Specularity :
                                    {
                                        double fExtrusionSpecularity = 0;
                                        if ( rProp.Value >>= fExtrusionSpecularity )
                                        {
                                            ::sax::Converter::convertDouble(
                                                aStrBuffer,
                                                fExtrusionSpecularity,
                                                false,
                                                util::MeasureUnit::PERCENT,
                                                util::MeasureUnit::PERCENT);
                                            aStrBuffer.append( '%' );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_SPECULARITY, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_ProjectionMode :
                                    {
                                        drawing::ProjectionMode eProjectionMode;
                                        if ( rProp.Value >>= eProjectionMode )
                                            rExport.AddAttribute( XML_NAMESPACE_DR3D, XML_PROJECTION,
                                                eProjectionMode == drawing::ProjectionMode_PARALLEL ? GetXMLToken( XML_PARALLEL ) : GetXMLToken( XML_PERSPECTIVE ) );
                                    }
                                    break;
                                    case EAS_ViewPoint :
                                    {
                                        drawing::Position3D aExtrusionViewPoint;
                                        if ( rProp.Value >>= aExtrusionViewPoint )
                                        {
                                            rUnitConverter.convertPosition3D( aStrBuffer, aExtrusionViewPoint );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_VIEWPOINT, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_Origin :
                                    {
                                        css::drawing::EnhancedCustomShapeParameterPair aOriginParaPair;
                                        if ( rProp.Value >>= aOriginParaPair )
                                        {
                                            ExportParameter( aStrBuffer, aOriginParaPair.First );
                                            ExportParameter( aStrBuffer, aOriginParaPair.Second );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_ORIGIN, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_Color :
                                    {
                                        bool bExtrusionColor;
                                        if ( rProp.Value >>= bExtrusionColor )
                                        {
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_COLOR,
                                                bExtrusionColor ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                                        }
                                    }
                                    break;
                                    default:
                                        break;
                                }
                            }
                        }
                    }
                    break;
                    case EAS_TextPath :
                    {
                        uno::Sequence< beans::PropertyValue > aTextPathPropSeq;
                        if ( rGeoProp.Value >>= aTextPathPropSeq )
                        {
                            sal_Int32 i, nCount = aTextPathPropSeq.getLength();
                            for ( i = 0; i < nCount; i++ )
                            {
                                const beans::PropertyValue& rProp = aTextPathPropSeq[ i ];
                                switch( EASGet( rProp.Name ) )
                                {
                                    case EAS_TextPath :
                                    {
                                        bool bTextPathOn;
                                        if ( rProp.Value >>= bTextPathOn )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_TEXT_PATH,
                                                bTextPathOn ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                                    }
                                    break;
                                    case EAS_TextPathMode :
                                    {
                                        css::drawing::EnhancedCustomShapeTextPathMode eTextPathMode;
                                        if ( rProp.Value >>= eTextPathMode )
                                        {
                                            switch ( eTextPathMode )
                                            {
                                                case css::drawing::EnhancedCustomShapeTextPathMode_NORMAL: aStr = GetXMLToken( XML_NORMAL ); break;
                                                case css::drawing::EnhancedCustomShapeTextPathMode_PATH  : aStr = GetXMLToken( XML_PATH );   break;
                                                case css::drawing::EnhancedCustomShapeTextPathMode_SHAPE : aStr = GetXMLToken( XML_SHAPE );  break;
                                                default:
                                                    break;
                                            }
                                            if ( !aStr.isEmpty() )
                                                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_TEXT_PATH_MODE, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_ScaleX :
                                    {
                                        bool bScaleX;
                                        if ( rProp.Value >>= bScaleX )
                                        {
                                            aStr = bScaleX ? GetXMLToken( XML_SHAPE ) : GetXMLToken( XML_PATH );
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_TEXT_PATH_SCALE, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_SameLetterHeights :
                                    {
                                        bool bSameLetterHeights;
                                        if ( rProp.Value >>= bSameLetterHeights )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_TEXT_PATH_SAME_LETTER_HEIGHTS,
                                                bSameLetterHeights ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                                    }
                                    break;
                                    default:
                                        break;
                                }
                            }
                        }
                    }
                    break;
                    case EAS_Path :
                    {
                        uno::Sequence< beans::PropertyValue > aPathPropSeq;
                        if ( rGeoProp.Value >>= aPathPropSeq )
                        {
                            sal_Int32 i, nCount = aPathPropSeq.getLength();
                            for ( i = 0; i < nCount; i++ )
                            {
                                const beans::PropertyValue& rProp = aPathPropSeq[ i ];

                                switch( EASGet( rProp.Name ) )
                                {
                                    case EAS_SubViewSize:
                                    {
                                        // export draw:sub-view-size (do not export in ODF 1.2 or older)
                                        if (rExport.getDefaultVersion() <= SvtSaveOptions::ODFVER_012)
                                        {
                                            continue;
                                        }
                                        uno::Sequence< awt::Size > aSubViewSizes;
                                        rProp.Value >>= aSubViewSizes;

                                        for ( int nIdx = 0; nIdx < aSubViewSizes.getLength(); nIdx++ )
                                        {
                                            if ( nIdx )
                                                aStrBuffer.append(' ');
                                            aStrBuffer.append( aSubViewSizes[nIdx].Width );
                                            aStrBuffer.append(' ');
                                            aStrBuffer.append( aSubViewSizes[nIdx].Height );
                                        }
                                        aStr = aStrBuffer.makeStringAndClear();
                                        rExport.AddAttribute( XML_NAMESPACE_DRAW_EXT, XML_SUB_VIEW_SIZE, aStr );
                                    }
                                    break;
                                    case EAS_ExtrusionAllowed :
                                    {
                                        bool bExtrusionAllowed;
                                        if ( rProp.Value >>= bExtrusionAllowed )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_ALLOWED,
                                                bExtrusionAllowed ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                                    }
                                    break;
                                    case EAS_ConcentricGradientFillAllowed :
                                    {
                                        bool bConcentricGradientFillAllowed;
                                        if ( rProp.Value >>= bConcentricGradientFillAllowed )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CONCENTRIC_GRADIENT_FILL_ALLOWED,
                                                bConcentricGradientFillAllowed ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                                    }
                                    break;
                                    case EAS_TextPathAllowed  :
                                    {
                                        bool bTextPathAllowed;
                                        if ( rProp.Value >>= bTextPathAllowed )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_TEXT_PATH_ALLOWED,
                                                bTextPathAllowed ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                                    }
                                    break;
                                    case EAS_GluePoints :
                                    {
                                        css::uno::Sequence< css::drawing::EnhancedCustomShapeParameterPair> aGluePoints;
                                        if ( rProp.Value >>= aGluePoints )
                                        {
                                            sal_Int32 k, nElements = aGluePoints.getLength();
                                            if ( nElements )
                                            {
                                                for( k = 0; k < nElements; k++ )
                                                {
                                                    ExportParameter( aStrBuffer, aGluePoints[ k ].First );
                                                    ExportParameter( aStrBuffer, aGluePoints[ k ].Second );
                                                }
                                                aStr = aStrBuffer.makeStringAndClear();
                                            }
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_GLUE_POINTS, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_GluePointType :
                                    {
                                        sal_Int16 nGluePointType = sal_Int16();
                                        if ( rProp.Value >>= nGluePointType )
                                        {
                                            switch ( nGluePointType )
                                            {
                                                case css::drawing::EnhancedCustomShapeGluePointType::NONE     : aStr = GetXMLToken( XML_NONE );    break;
                                                case css::drawing::EnhancedCustomShapeGluePointType::SEGMENTS : aStr = GetXMLToken( XML_SEGMENTS ); break;
                                                case css::drawing::EnhancedCustomShapeGluePointType::RECT     : aStr = GetXMLToken( XML_RECTANGLE ); break;
                                            }
                                            if ( !aStr.isEmpty() )
                                                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_GLUE_POINT_TYPE, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_Coordinates :
                                    {
                                        bCoordinates = ( rProp.Value >>= aCoordinates );
                                    }
                                    break;
                                    case EAS_Segments :
                                    {
                                        rProp.Value >>= aSegments;
                                    }
                                    break;
                                    case EAS_StretchX :
                                    {
                                        sal_Int32 nStretchPoint = 0;
                                        if ( rProp.Value >>= nStretchPoint )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_PATH_STRETCHPOINT_X, OUString::number( nStretchPoint ) );
                                    }
                                    break;
                                    case EAS_StretchY :
                                    {
                                        sal_Int32 nStretchPoint = 0;
                                        if ( rProp.Value >>= nStretchPoint )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_PATH_STRETCHPOINT_Y, OUString::number( nStretchPoint ) );
                                    }
                                    break;
                                    case EAS_TextFrames :
                                    {
                                        css::uno::Sequence< css::drawing::EnhancedCustomShapeTextFrame > aPathTextFrames;
                                        if ( rProp.Value >>= aPathTextFrames )
                                        {
                                            if ( static_cast<sal_uInt16>(aPathTextFrames.getLength()) )
                                            {
                                                sal_uInt16 k, nElements = static_cast<sal_uInt16>(aPathTextFrames.getLength());
                                                for ( k = 0; k < nElements; k++ )
                                                {
                                                    ExportParameter( aStrBuffer, aPathTextFrames[ k ].TopLeft.First );
                                                    ExportParameter( aStrBuffer, aPathTextFrames[ k ].TopLeft.Second );
                                                    ExportParameter( aStrBuffer, aPathTextFrames[ k ].BottomRight.First );
                                                    ExportParameter( aStrBuffer, aPathTextFrames[ k ].BottomRight.Second );
                                                }
                                                aStr = aStrBuffer.makeStringAndClear();
                                            }
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_TEXT_AREAS, aStr );
                                        }
                                    }
                                    break;
                                    default:
                                        break;
                                }
                            }
                        }
                    }
                    break;
                    case EAS_Equations :
                    {
                        bEquations = ( rGeoProp.Value >>= aEquations );
                    }
                    break;
                    case EAS_Handles :
                    {
                        bHandles = ( rGeoProp.Value >>= aHandles );
                    }
                    break;
                    case EAS_AdjustmentValues :
                    {
                        rGeoProp.Value >>= aAdjustmentValues;
                    }
                    break;
                    default:
                        break;
                }
            }   // for
            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_TYPE, aCustomShapeType );

            // adjustments
            sal_Int32 nAdjustmentValues = aAdjustmentValues.getLength();
            if ( nAdjustmentValues )
            {
                sal_Int32 i, nValue = 0;
                for ( i = 0; i < nAdjustmentValues; i++ )
                {
                    if ( i )
                        aStrBuffer.append( ' ' );

                    const css::drawing::EnhancedCustomShapeAdjustmentValue& rAdj = aAdjustmentValues[ i ];
                    if ( rAdj.State == beans::PropertyState_DIRECT_VALUE )
                    {
                        if ( rAdj.Value.getValueTypeClass() == uno::TypeClass_DOUBLE )
                        {
                            double fValue = 0.0;
                            rAdj.Value >>= fValue;
                            ::sax::Converter::convertDouble(aStrBuffer, fValue);
                        }
                        else
                        {
                            rAdj.Value >>= nValue;
                            aStrBuffer.append(nValue);
                        }
                    }
                    else
                    {
                        // this should not be, but better than setting nothing
                        aStrBuffer.append("0");
                    }
                }
                aStr = aStrBuffer.makeStringAndClear();
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_MODIFIERS, aStr );
            }
            if ( bCoordinates )
                ImpExportEnhancedPath( rExport, aCoordinates, aSegments );
        }
    }
    SvXMLElementExport aOBJ( rExport, XML_NAMESPACE_DRAW, XML_ENHANCED_GEOMETRY, true, true );
    if ( bEquations )
        ImpExportEquations( rExport, aEquations );
    if ( bHandles )
        ImpExportHandles( rExport, aHandles );
}

void XMLShapeExport::ImpExportCustomShape(
    const uno::Reference< drawing::XShape >& xShape,
    XMLShapeExportFlags nFeatures, css::awt::Point* pRefPoint )
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if ( !xPropSet.is() )
        return;

    OUString aStr;
    uno::Reference< beans::XPropertySetInfo > xPropSetInfo( xPropSet->getPropertySetInfo() );

    // Transformation
    ImpExportNewTrans( xPropSet, nFeatures, pRefPoint );

    if ( xPropSetInfo.is() )
    {
        if ( xPropSetInfo->hasPropertyByName( "CustomShapeEngine" ) )
        {
            uno::Any aEngine( xPropSet->getPropertyValue( "CustomShapeEngine" ) );
            if ( ( aEngine >>= aStr ) && !aStr.isEmpty() )
                mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_ENGINE, aStr );
        }
        if ( xPropSetInfo->hasPropertyByName( "CustomShapeData" ) )
        {
            uno::Any aData( xPropSet->getPropertyValue( "CustomShapeData" ) );
            if ( ( aData >>= aStr ) && !aStr.isEmpty() )
                mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_DATA, aStr );
        }
    }
    bool bCreateNewline( (nFeatures & XMLShapeExportFlags::NO_WS) == XMLShapeExportFlags::NONE ); // #86116#/#92210#
    SvXMLElementExport aOBJ( mrExport, XML_NAMESPACE_DRAW, XML_CUSTOM_SHAPE, bCreateNewline, true );
    ImpExportDescription( xShape ); // #i68101#
    ImpExportEvents( xShape );
    ImpExportGluePoints( xShape );
    ImpExportText( xShape );
    ImpExportEnhancedGeometry( mrExport, xPropSet );

}

void XMLShapeExport::ImpExportTableShape( const uno::Reference< drawing::XShape >& xShape, XmlShapeType eShapeType, XMLShapeExportFlags nFeatures, css::awt::Point* pRefPoint )
{
    uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    uno::Reference< container::XNamed > xNamed(xShape, uno::UNO_QUERY);

    SAL_WARN_IF( !xPropSet.is() || !xNamed.is(), "xmloff", "xmloff::XMLShapeExport::ImpExportTableShape(), table shape is not implementing needed interfaces");
    if(xPropSet.is() && xNamed.is()) try
    {
        // Transformation
        ImpExportNewTrans(xPropSet, nFeatures, pRefPoint);

        bool bIsEmptyPresObj = false;

        // presentation settings
        if(eShapeType == XmlShapeTypePresTableShape)
            bIsEmptyPresObj = ImpExportPresentationAttributes( xPropSet, GetXMLToken(XML_PRESENTATION_TABLE) );

        const bool bCreateNewline( (nFeatures & XMLShapeExportFlags::NO_WS) == XMLShapeExportFlags::NONE );
        const bool bExportEmbedded(mrExport.getExportFlags() & SvXMLExportFlags::EMBEDDED);

        SvXMLElementExport aElement( mrExport, XML_NAMESPACE_DRAW, XML_FRAME, bCreateNewline, true );

        // do not export in ODF 1.1 or older
        if( mrExport.getDefaultVersion() >= SvtSaveOptions::ODFVER_012 )
        {
            if( !bIsEmptyPresObj )
            {
                uno::Reference< container::XNamed > xTemplate( xPropSet->getPropertyValue("TableTemplate"), uno::UNO_QUERY );
                if( xTemplate.is() )
                {
                    const OUString sTemplate( xTemplate->getName() );
                    if( !sTemplate.isEmpty() )
                    {
                        mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_TEMPLATE_NAME, sTemplate );

                        for( const XMLPropertyMapEntry* pEntry = &aXMLTableShapeAttributes[0]; pEntry->msApiName; pEntry++ )
                        {
                            try
                            {
                                bool bBool = false;
                                const OUString sAPIPropertyName( pEntry->msApiName, pEntry->nApiNameLength, RTL_TEXTENCODING_ASCII_US );

                                xPropSet->getPropertyValue( sAPIPropertyName ) >>= bBool;
                                if( bBool )
                                    mrExport.AddAttribute(pEntry->mnNameSpace, pEntry->meXMLName, XML_TRUE );
                            }
                            catch( uno::Exception& )
                            {
                                DBG_UNHANDLED_EXCEPTION("xmloff.draw");
                            }
                        }
                    }
                }

                uno::Reference< table::XColumnRowRange > xRange( xPropSet->getPropertyValue( gsModel ), uno::UNO_QUERY_THROW );
                GetShapeTableExport()->exportTable( xRange );
            }
        }

        if( !bIsEmptyPresObj )
        {
            uno::Reference< graphic::XGraphic > xGraphic( xPropSet->getPropertyValue("ReplacementGraphic"), uno::UNO_QUERY );
            if( xGraphic.is() ) try
            {
                uno::Reference< uno::XComponentContext > xContext = GetExport().getComponentContext();

                uno::Reference< embed::XStorage > xPictureStorage;
                uno::Reference< embed::XStorage > xStorage;
                uno::Reference< io::XStream > xPictureStream;

                OUString sPictureName;
                if( bExportEmbedded )
                {
                    xPictureStream.set( xContext->getServiceManager()->createInstanceWithContext( "com.sun.star.comp.MemoryStream", xContext), uno::UNO_QUERY_THROW );
                }
                else
                {
                    xStorage.set( GetExport().GetTargetStorage(), uno::UNO_QUERY_THROW );

                    xPictureStorage.set( xStorage->openStorageElement( "Pictures" , ::embed::ElementModes::READWRITE ), uno::UNO_QUERY_THROW );

                    sal_Int32 nIndex = 0;
                    do
                    {
                        sPictureName = "TablePreview" + OUString::number( ++nIndex ) + ".svm";
                    }
                    while( xPictureStorage->hasByName( sPictureName ) );

                    xPictureStream.set( xPictureStorage->openStreamElement( sPictureName, ::embed::ElementModes::READWRITE ), uno::UNO_QUERY_THROW );
                }

                uno::Reference< graphic::XGraphicProvider > xProvider( graphic::GraphicProvider::create(xContext) );
                uno::Sequence< beans::PropertyValue > aArgs( 2 );
                aArgs[ 0 ].Name = "MimeType";
                aArgs[ 0 ].Value <<= OUString( "image/x-vclgraphic" );
                aArgs[ 1 ].Name = "OutputStream";
                aArgs[ 1 ].Value <<= xPictureStream->getOutputStream();
                xProvider->storeGraphic( xGraphic, aArgs );

                if( xPictureStorage.is() )
                {
                    uno::Reference< embed::XTransactedObject > xTrans( xPictureStorage, uno::UNO_QUERY );
                    if( xTrans.is() )
                        xTrans->commit();
                }

                if( !bExportEmbedded )
                {
                    OUString sURL( "Pictures/" );
                    sURL += sPictureName;
                    mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, sURL );
                    mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
                    mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
                    mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );
                }

                SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_DRAW, XML_IMAGE, false, true );

                if( bExportEmbedded )
                {
                    uno::Reference< io::XSeekableInputStream > xSeekable( xPictureStream, uno::UNO_QUERY_THROW );
                    xSeekable->seek(0);

                    XMLBase64Export aBase64Exp( GetExport() );
                    aBase64Exp.exportOfficeBinaryDataElement( uno::Reference < io::XInputStream >( xPictureStream, uno::UNO_QUERY_THROW ) );
                }
            }
            catch( uno::Exception const & )
            {
                DBG_UNHANDLED_EXCEPTION("xmloff.draw");
            }
        }

        ImpExportEvents( xShape );
        ImpExportGluePoints( xShape );
        ImpExportDescription( xShape ); // #i68101#
    }
    catch( uno::Exception const & )
    {
        DBG_UNHANDLED_EXCEPTION("xmloff.draw");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
