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

#include <memory>

#include <xmloff/unointerfacetouniqueidentifiermapper.hxx>
#include <com/sun/star/presentation/ClickAction.hpp>
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/drawing/XGluePointsSupplier.hpp>
#include <com/sun/star/container/XIdentifierAccess.hpp>
#include <com/sun/star/drawing/GluePoint2.hpp>
#include <com/sun/star/drawing/Alignment.hpp>
#include <com/sun/star/drawing/EscapeDirection.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <xmloff/xmluconv.hxx>
#include "PropertySetMerger.hxx"

#include <xmloff/shapeexport.hxx>
#include "sdpropls.hxx"
#include "sdxmlexp_impl.hxx"
#include <xmloff/families.hxx>
#include <tools/debug.hxx>
#include <xmloff/contextid.hxx>
#include <xmloff/xmltoken.hxx>
#include <comphelper/classids.hxx>
#include <tools/globname.hxx>
#include <com/sun/star/beans/XPropertyState.hpp>

#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/drawing/XCustomShapeEngine.hpp>

#include "xmloff/xmlnmspe.hxx"

using namespace ::com::sun::star;
using namespace ::xmloff::token;

XMLShapeExport::XMLShapeExport(SvXMLExport& rExp,
                                SvXMLExportPropertyMapper *pExtMapper )
:   mrExport( rExp ),
    mnNextUniqueShapeId(1),
    maShapesInfos(),
    maCurrentShapesIter(maShapesInfos.end()),
    mbExportLayer( sal_False ),
    // #88546# init to sal_False
    mbHandleProgressBar( sal_False ),
    msZIndex( "ZOrder" ),
    msPrintable( "Printable" ),
    msVisible( "Visible" ),
    msEmptyPres( "IsEmptyPresentationObject" ),
    msModel( "Model" ),
    msStartShape( "StartShape" ),
    msEndShape( "EndShape" ),
    msOnClick( "OnClick" ),
    msEventType( "EventType" ),
    msPresentation( "Presentation" ),
    msMacroName( "MacroName" ),
    msScript( "Script" ),
    msLibrary( "Library" ),
    msClickAction( "ClickAction" ),
    msBookmark( "Bookmark" ),
    msEffect( "Effect" ),
    msPlayFull( "PlayFull" ),
    msVerb( "Verb" ),
    msSoundURL( "SoundURL" ),
    msSpeed( "Speed" ),
    msStarBasic( "StarBasic" )
{
    // construct PropertyHandlerFactory
    mxSdPropHdlFactory = new XMLSdPropHdlFactory( mrExport.GetModel(), rExp );
    // construct PropertySetMapper
    mxPropertySetMapper = CreateShapePropMapper( mrExport );
    if( pExtMapper )
    {
        UniReference < SvXMLExportPropertyMapper > xExtMapper( pExtMapper );
        mxPropertySetMapper->ChainExportMapper( xExtMapper );
    }

/*
    // chain text attributes
    xPropertySetMapper->ChainExportMapper(XMLTextParagraphExport::CreateParaExtPropMapper(rExp));
*/

    mrExport.GetAutoStylePool()->AddFamily(
        XML_STYLE_FAMILY_SD_GRAPHICS_ID,
        OUString(XML_STYLE_FAMILY_SD_GRAPHICS_NAME),
        GetPropertySetMapper(),
        OUString(XML_STYLE_FAMILY_SD_GRAPHICS_PREFIX));
    mrExport.GetAutoStylePool()->AddFamily(
        XML_STYLE_FAMILY_SD_PRESENTATION_ID,
        OUString(XML_STYLE_FAMILY_SD_PRESENTATION_NAME),
        GetPropertySetMapper(),
        OUString(XML_STYLE_FAMILY_SD_PRESENTATION_PREFIX));

    maCurrentInfo = maShapeInfos.end();

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

    if( ( GetExport().getExportFlags() & EXPORT_OASIS ) == 0 )
    {
        OUString aType( xShape->getShapeType() );
        if( aType.equalsAscii( "com.sun.star.drawing.CustomShape" ) )
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
                    aPropValues[ 0 ].Name = OUString( "CustomShape" );
                    aPropValues[ 0 ].Value <<= xShape;
                    sal_Bool bForceGroupWithText = sal_True;
                    aPropValues[ 1 ].Name = OUString( "ForceGroupWithText" );
                    aPropValues[ 1 ].Value <<= bForceGroupWithText;
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
    uno::Reference< beans::XPropertySet > xSet( xShape, uno::UNO_QUERY );
    if( xSet.is() )
        xSet->getPropertyValue(msZIndex) >>= nZIndex;

    ImplXMLShapeExportInfoVector& aShapeInfoVector = (*maCurrentShapesIter).second;

    if( (sal_Int32)aShapeInfoVector.size() <= nZIndex )
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
        aShapeInfo.meShapeType != XmlShapeTypePresChartShape &&
        aShapeInfo.meShapeType != XmlShapeTypePresOLE2Shape &&
        aShapeInfo.meShapeType != XmlShapeTypeDrawSheetShape &&
        aShapeInfo.meShapeType != XmlShapeTypePresSheetShape &&
        aShapeInfo.meShapeType != XmlShapeTypeDraw3DSceneObject &&
        aShapeInfo.meShapeType != XmlShapeTypeDraw3DCubeObject &&
        aShapeInfo.meShapeType != XmlShapeTypeDraw3DSphereObject &&
        aShapeInfo.meShapeType != XmlShapeTypeDraw3DLatheObject &&
        aShapeInfo.meShapeType != XmlShapeTypeDraw3DExtrudeObject &&
        aShapeInfo.meShapeType != XmlShapeTypeDrawPageShape &&
        aShapeInfo.meShapeType != XmlShapeTypePresPageShape &&
        aShapeInfo.meShapeType != XmlShapeTypeDrawGroupShape;

    const bool bObjSupportsStyle =
        aShapeInfo.meShapeType != XmlShapeTypeDrawGroupShape;

    sal_Bool bIsEmptyPresObj = sal_False;

    uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if ( aShapeInfo.xCustomShapeReplacement.is() )
        xPropSet.clear();

    // prep text styles
    if( xPropSet.is() && bObjSupportsText )
    {
        uno::Reference< text::XText > xText(xShape, uno::UNO_QUERY);
        if(xText.is() && !xText->getString().isEmpty())
        {
            uno::Reference< beans::XPropertySetInfo > xPropSetInfo( xPropSet->getPropertySetInfo() );

            if( xPropSetInfo.is() && xPropSetInfo->hasPropertyByName(msEmptyPres) )
            {
                uno::Any aAny = xPropSet->getPropertyValue(msEmptyPres);
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
                DBG_ASSERT( xStylePropSet.is(), "style without a XPropertySet?" );
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
                    DBG_ASSERT(false,
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

        // filter propset
        std::vector< XMLPropertyState > xPropStates;

        sal_Int32 nCount = 0;
        if( (!bIsEmptyPresObj || (aShapeInfo.meShapeType != XmlShapeTypePresPageShape)) )
        {
            xPropStates = GetPropertySetMapper()->Filter( xPropSet );

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
                        xPropStates.push_back(aNewState);
                    }
                }
            }

            std::vector< XMLPropertyState >::iterator aIter = xPropStates.begin();
            std::vector< XMLPropertyState >::iterator aEnd = xPropStates.end();
            while( aIter != aEnd )
            {
                if( aIter->mnIndex != -1 )
                    nCount++;
                ++aIter;
            }
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
            aShapeInfo.msStyleName = mrExport.GetAutoStylePool()->Find(aShapeInfo.mnFamily, aParentName, xPropStates);

            if(aShapeInfo.msStyleName.isEmpty())
            {
                // Style did not exist, add it to AutoStalePool
                aShapeInfo.msStyleName = mrExport.GetAutoStylePool()->Add(aShapeInfo.mnFamily, aParentName, xPropStates);
            }
        }

        // optionaly generate auto style for text attributes
        if( (!bIsEmptyPresObj || (aShapeInfo.meShapeType != XmlShapeTypePresPageShape)) && bObjSupportsText )
        {
            xPropStates = GetExport().GetTextParagraphExport()->GetParagraphPropertyMapper()->Filter( xPropSet );

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
                    //   because we need to distiguish this "left" from the case where not align attribute
                    //   is present which means "void"
                    static const OUString s_sParaAdjustPropertyName(  "ParaAdjust"  );
                    if  (   xPropSetInfo->hasPropertyByName( s_sParaAdjustPropertyName )
                        &&  ( beans::PropertyState_DEFAULT_VALUE == xPropState->getPropertyState( s_sParaAdjustPropertyName ) )
                        )
                    {
                        sal_Int32 nIndex = GetExport().GetTextParagraphExport()->GetParagraphPropertyMapper()->getPropertySetMapper()->FindEntryIndex( CTF_SD_SHAPE_PARA_ADJUST );
                            // TODO : this retrieval of the index should be moved into the ctor, holding the index
                            //          as member, thus saving time.
                        DBG_ASSERT(-1 != nIndex, "XMLShapeExport::collectShapeAutoStyles: could not obtain the index for the ParaAdjust context id!");

                        uno::Any aParaAdjustValue = xPropSet->getPropertyValue( s_sParaAdjustPropertyName );
                        XMLPropertyState aAlignDefaultState( nIndex, aParaAdjustValue );

                        xPropStates.push_back( aAlignDefaultState );
                    }
                }
            }

            nCount = 0;
            std::vector< XMLPropertyState >::iterator aIter = xPropStates.begin();
            std::vector< XMLPropertyState >::iterator aEnd = xPropStates.end();
            while( aIter != aEnd )
            {
                if( aIter->mnIndex != -1 )
                    nCount++;
                ++aIter;
            }

            if( nCount )
            {
                const OUString aEmpty;
                aShapeInfo.msTextStyleName = mrExport.GetAutoStylePool()->Find( XML_STYLE_FAMILY_TEXT_PARAGRAPH, aEmpty, xPropStates );
                if(aShapeInfo.msTextStyleName.isEmpty())
                {
                    // Style did not exist, add it to AutoStalePool
                    aShapeInfo.msTextStyleName = mrExport.GetAutoStylePool()->Add(XML_STYLE_FAMILY_TEXT_PARAGRAPH, aEmpty, xPropStates);
                }
            }
        }
    }

    // prepare animation information if needed
    if( mxAnimationsExporter.is() )
        mxAnimationsExporter->prepare( xShape, mrExport );

    // check for special shapes

    switch( aShapeInfo.meShapeType )
    {
        case XmlShapeTypeDrawConnectorShape:
        {
            uno::Reference< uno::XInterface > xConnection;

            // create shape ids for export later
            xPropSet->getPropertyValue( msStartShape ) >>= xConnection;
            if( xConnection.is() )
                mrExport.getInterfaceToIdentifierMapper().registerReference( xConnection );

            xPropSet->getPropertyValue( msEndShape ) >>= xConnection;
            if( xConnection.is() )
                mrExport.getInterfaceToIdentifierMapper().registerReference( xConnection );
            break;
        }
        case XmlShapeTypePresTableShape:
        case XmlShapeTypeDrawTableShape:
        {
            try
            {
                uno::Reference< table::XColumnRowRange > xRange( xSet->getPropertyValue( msModel ), uno::UNO_QUERY_THROW );
                GetShapeTableExport()->collectTableAutoStyles( xRange );
            }
            catch(const uno::Exception&)
            {
                OSL_FAIL( "XMLShapeExport::collectShapeAutoStyles(): exception caught while collection auto styles for a table!" );
            }
            break;
        }
        default:
            break;
    }

    maShapeInfos.push_back( aShapeInfo );
    maCurrentInfo = maShapeInfos.begin();

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
            NewTextListsHelper( SvXMLExport& rExp )
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
                                 sal_Int32 nFeatures /* = SEF_DEFAULT */,
                                 com::sun::star::awt::Point* pRefPoint /* = NULL */,
                                 SvXMLAttributeList* pAttrList /* = NULL */ )
{
    SAL_WARN("xmloff", xShape->getShapeType());
    if( maCurrentShapesIter == maShapesInfos.end() )
    {
        SAL_WARN( "xmloff", "XMLShapeExport::exportShape(): no auto styles where collected before export" );
        return;
    }
    sal_Int32 nZIndex = 0;
    uno::Reference< beans::XPropertySet > xSet( xShape, uno::UNO_QUERY );

    ::std::auto_ptr< SvXMLElementExport >  mpHyperlinkElement;

    // export hyperlinks with <a><shape/></a>. Currently only in draw since draw
    // does not support document events
    if( xSet.is() && (GetExport().GetModelType() == SvtModuleOptions::E_DRAW) ) try
    {
        presentation::ClickAction eAction = presentation::ClickAction_NONE;
        xSet->getPropertyValue("OnClick") >>= eAction;

        if( (eAction == presentation::ClickAction_DOCUMENT) ||
            (eAction == presentation::ClickAction_BOOKMARK) )
        {
            OUString sURL;
            xSet->getPropertyValue(msBookmark) >>= sURL;

            if( !sURL.isEmpty() )
            {
                mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, sURL );
                mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
                mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
                mpHyperlinkElement.reset( new SvXMLElementExport(mrExport, XML_NAMESPACE_DRAW, XML_A, sal_True, sal_True) );
            }
        }
    }
    catch(const uno::Exception&)
    {
        SAL_WARN("xmloff", "XMLShapeExport::exportShape(): exception during hyperlink export");
    }

    if( xSet.is() )
        xSet->getPropertyValue(msZIndex) >>= nZIndex;

    ImplXMLShapeExportInfoVector& aShapeInfoVector = (*maCurrentShapesIter).second;

    if( (sal_Int32)aShapeInfoVector.size() <= nZIndex )
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
        DBG_ASSERT( xParent.is() && xParent.get() == (*maCurrentShapesIter).first.get(), "XMLShapeExport::exportShape(): Wrong call to XMLShapeExport::seekShapes()" );
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
        if ( ( GetExport().GetModelType() != SvtModuleOptions::E_WRITER &&
               GetExport().GetModelType() != SvtModuleOptions::E_WRITERWEB &&
               GetExport().GetModelType() != SvtModuleOptions::E_WRITERGLOBAL ) ||
             ( GetExport().getExportFlags() & EXPORT_OASIS ) != 0 ||
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
    if( IsLayerExportEnabled() )
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
                OSL_FAIL( "could not export layer name for shape!" );
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
            sal_Bool bVisible = sal_True;
            sal_Bool bPrintable = sal_True;

            xSet->getPropertyValue(msVisible) >>= bVisible;
            xSet->getPropertyValue(msPrintable) >>= bPrintable;

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
            OSL_FAIL( "XMLShapeExport::exportShape(), exception caught!" );
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
            ImpExportRectangleShape(xShape, aShapeInfo.meShapeType, nFeatures, pRefPoint );
            break;
        }
        case XmlShapeTypeDrawEllipseShape:
        {
            ImpExportEllipseShape(xShape, aShapeInfo.meShapeType, nFeatures, pRefPoint );
            break;
        }
        case XmlShapeTypeDrawLineShape:
        {
            ImpExportLineShape(xShape, aShapeInfo.meShapeType, nFeatures, pRefPoint );
            break;
        }
        case XmlShapeTypeDrawPolyPolygonShape:  // closed PolyPolygon
        case XmlShapeTypeDrawPolyLineShape:     // open PolyPolygon
        case XmlShapeTypeDrawClosedBezierShape: // closed PolyPolygon containing curves
        case XmlShapeTypeDrawOpenBezierShape:   // open PolyPolygon containing curves
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
            ImpExportControlShape(xShape, aShapeInfo.meShapeType, nFeatures, pRefPoint );
            break;
        }

        case XmlShapeTypeDrawConnectorShape:
        {
            ImpExportConnectorShape(xShape, aShapeInfo.meShapeType, nFeatures, pRefPoint );
            break;
        }

        case XmlShapeTypeDrawMeasureShape:
        {
            ImpExportMeasureShape(xShape, aShapeInfo.meShapeType, nFeatures, pRefPoint );
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
            ImpExportCaptionShape(xShape, aShapeInfo.meShapeType, nFeatures, pRefPoint );
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
            ImpExport3DSceneShape( xShape, aShapeInfo.meShapeType, nFeatures, pRefPoint );
            break;
        }

        case XmlShapeTypeDrawGroupShape:
        {
            // empty group
            ImpExportGroupShape( xShape, aShapeInfo.meShapeType, nFeatures, pRefPoint );
            break;
        }

        case XmlShapeTypeDrawFrameShape:
        {
            ImpExportFrameShape(xShape, aShapeInfo.meShapeType, nFeatures, pRefPoint );
            break;
        }

        case XmlShapeTypeDrawAppletShape:
        {
            ImpExportAppletShape(xShape, aShapeInfo.meShapeType, nFeatures, pRefPoint );
            break;
        }

        case XmlShapeTypeDrawPluginShape:
        {
            ImpExportPluginShape(xShape, aShapeInfo.meShapeType, nFeatures, pRefPoint );
            break;
        }

        case XmlShapeTypeDrawCustomShape:
        {
            if ( aShapeInfo.xCustomShapeReplacement.is() )
                ImpExportGroupShape( aShapeInfo.xCustomShapeReplacement, XmlShapeTypeDrawGroupShape, nFeatures, pRefPoint );
            else
                ImpExportCustomShape( xShape, aShapeInfo.meShapeType, nFeatures, pRefPoint );
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

    mpHyperlinkElement.reset();

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
void XMLShapeExport::exportShapes( const uno::Reference < drawing::XShapes >& xShapes, sal_Int32 nFeatures /* = SEF_DEFAULT */, awt::Point* pRefPoint /* = NULL */ )
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
            aNewInfoVector.resize( (ShapesInfos::size_type) xShapes->getCount() );
            maShapesInfos[ xShapes ] = aNewInfoVector;

            maCurrentShapesIter = maShapesInfos.find( xShapes );

            DBG_ASSERT( maCurrentShapesIter != maShapesInfos.end(), "XMLShapeExport::seekShapes(): insert into stl::map failed" );
        }

        DBG_ASSERT( (*maCurrentShapesIter).second.size() == (ShapesInfos::size_type)xShapes->getCount(), "XMLShapeExport::seekShapes(): XShapes size varied between calls" );

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
        GetExport().GetAutoStylePool()->exportXML(
            XML_STYLE_FAMILY_SD_GRAPHICS_ID
            , GetExport().GetDocHandler(),
            GetExport().GetMM100UnitConverter(),
            GetExport().GetNamespaceMap()
            );
    }

    // ...for presentation
    {
        GetExport().GetAutoStylePool()->exportXML(
            XML_STYLE_FAMILY_SD_PRESENTATION_ID
            , GetExport().GetDocHandler(),
            GetExport().GetMM100UnitConverter(),
            GetExport().GetNamespaceMap()
            );
    }

    if( mxShapeTableExport.is() )
        mxShapeTableExport->exportAutoStyles();
}

/// returns the export property mapper for external chaining
SvXMLExportPropertyMapper* XMLShapeExport::CreateShapePropMapper(
    SvXMLExport& rExport )
{
    UniReference< XMLPropertyHandlerFactory > xFactory = new XMLSdPropHdlFactory( rExport.GetModel(), rExport );
    UniReference < XMLPropertySetMapper > xMapper = new XMLShapePropertySetMapper( xFactory );
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
                            if (sCLSID.equals(mrExport.GetChartExport()->getChartCLSID()) ||
                                sCLSID.equals(OUString( SvGlobalName( SO3_RPTCH_CLASSID ).GetHexName())))
                            {
                                eShapeType = XmlShapeTypeDrawChartShape;
                            }
                            else if (sCLSID.equals(OUString( SvGlobalName( SO3_SC_CLASSID ).GetHexName())))
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
                            if( sCLSID.equals(OUString( SvGlobalName( SO3_SC_CLASSID ).GetHexName())) )
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

extern SvXMLEnumMapEntry aXML_GlueAlignment_EnumMap[];
extern SvXMLEnumMapEntry aXML_GlueEscapeDirection_EnumMap[];

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

            const OUString sId( OUString::valueOf( nIdentifier ) );
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

            SvXMLElementExport aEventsElemt(mrExport, XML_NAMESPACE_DRAW, XML_GLUE_POINT, sal_True, sal_True);
        }
    }
}

void XMLShapeExport::ExportGraphicDefaults()
{
    XMLStyleExport aStEx(mrExport, OUString(), mrExport.GetAutoStylePool().get());

    // construct PropertySetMapper
    UniReference< SvXMLExportPropertyMapper > xPropertySetMapper( CreateShapePropMapper( mrExport ) );
    ((XMLShapeExportPropertyMapper*)xPropertySetMapper.get())->SetAutoStyles( sal_False );

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
                aStEx.exportDefaultStyle( xDefaults, OUString(XML_STYLE_FAMILY_SD_GRAPHICS_NAME), xPropertySetMapper );

                // write graphic family styles
                aStEx.exportStyleFamily("graphics", OUString(XML_STYLE_FAMILY_SD_GRAPHICS_NAME), xPropertySetMapper, sal_False, XML_STYLE_FAMILY_SD_GRAPHICS_ID);
            }
        }
        catch(const lang::ServiceNotRegisteredException&)
        {
        }
    }
}

void XMLShapeExport::onExport( const com::sun::star::uno::Reference < com::sun::star::drawing::XShape >& )
{
}

const rtl::Reference< XMLTableExport >& XMLShapeExport::GetShapeTableExport()
{
    if( !mxShapeTableExport.is() )
    {
        rtl::Reference< XMLPropertyHandlerFactory > xFactory( new XMLSdPropHdlFactory( mrExport.GetModel(), mrExport ) );
        UniReference < XMLPropertySetMapper > xMapper( new XMLShapePropertySetMapper( xFactory.get() ) );
        mrExport.GetTextParagraphExport(); // get or create text paragraph export
        rtl::Reference< SvXMLExportPropertyMapper > xPropertySetMapper( new XMLShapeExportPropertyMapper( xMapper, mrExport ) );
        mxShapeTableExport = new XMLTableExport( mrExport, xPropertySetMapper, xFactory );
    }

    return mxShapeTableExport;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
