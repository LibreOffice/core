/*************************************************************************
 *
 *  $RCSfile: shapeexport.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: cl $ $Date: 2001-03-08 14:33:34 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif

#ifndef _COM_SUN_STAR_CHART_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart/XChartDocument.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_XCONTROLSHAPE_HPP_
#include <com/sun/star/drawing/XControlShape.hpp>
#endif

#ifndef _COM_SUN_STAR_STYLE_XSTYLE_HPP_
#include <com/sun/star/style/XStyle.hpp>
#endif

#ifndef _XMLOFF_PROPERTYSETMERGER_HXX_
#include "PropertySetMerger.hxx"
#endif

#ifndef _XMLOFF_SHAPEEXPORT_HXX
#include "shapeexport.hxx"
#endif

#ifndef _SDPROPLS_HXX
#include "sdpropls.hxx"
#endif

#ifndef _SDXMLEXP_IMPL_HXX
#include "sdxmlexp_impl.hxx"
#endif

#ifndef _XMLOFF_FAMILIES_HXX_
#include "families.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#include "xmlkywd.hxx"
#include "xmlnmspe.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////////

XMLShapeExport::XMLShapeExport(SvXMLExport& rExp,
                                SvXMLExportPropertyMapper *pExtMapper )
:   rExport( rExp ),
    mnNextUniqueShapeId(1),
    mbExportLayer( sal_False ),
    msZIndex( RTL_CONSTASCII_USTRINGPARAM("ZOrder") ),
    msEmptyPres( RTL_CONSTASCII_USTRINGPARAM("IsEmptyPresentationObject") ),
    msModel( RTL_CONSTASCII_USTRINGPARAM("Model") ),
    msStartShape( RTL_CONSTASCII_USTRINGPARAM("StartShape") ),
    msEndShape( RTL_CONSTASCII_USTRINGPARAM("EndShape") ),
    msOnClick( RTL_CONSTASCII_USTRINGPARAM("OnClick") ),
    msEventType( RTL_CONSTASCII_USTRINGPARAM("EventType") ),
    msPresentation( RTL_CONSTASCII_USTRINGPARAM("Presentation") ),
    msMacroName( RTL_CONSTASCII_USTRINGPARAM("MacroName") ),
    msLibrary( RTL_CONSTASCII_USTRINGPARAM("Library") ),
    msClickAction( RTL_CONSTASCII_USTRINGPARAM("ClickAction") ),
    msBookmark( RTL_CONSTASCII_USTRINGPARAM("Bookmark") ),
    msEffect( RTL_CONSTASCII_USTRINGPARAM("Effect") ),
    msPlayFull( RTL_CONSTASCII_USTRINGPARAM("PlayFull") ),
    msVerb( RTL_CONSTASCII_USTRINGPARAM("Verb") ),
    msSoundURL( RTL_CONSTASCII_USTRINGPARAM("SoundURL") ),
    msSpeed( RTL_CONSTASCII_USTRINGPARAM("Speed") ),
    msStarBasic( RTL_CONSTASCII_USTRINGPARAM("StarBasic") )
{
    // construct PropertyHandlerFactory
    xSdPropHdlFactory = new XMLSdPropHdlFactory( rExport.GetModel() );

    // construct PropertySetMapper
    UniReference < XMLPropertySetMapper > xMapper = new XMLShapePropertySetMapper( xSdPropHdlFactory);
    xPropertySetMapper = new XMLShapeExportPropertyMapper( xMapper, (XMLTextListAutoStylePool*)&rExport.GetTextParagraphExport()->GetListAutoStylePool(), rExp );
    if( pExtMapper )
    {
        UniReference < SvXMLExportPropertyMapper > xExtMapper( pExtMapper );
        xPropertySetMapper->ChainExportMapper( xExtMapper );
    }

    // chain text attributes
    xPropertySetMapper->ChainExportMapper(XMLTextParagraphExport::CreateParaExtPropMapper(rExp));

/*
    // chain form attributes
    const UniReference< SvXMLExportPropertyMapper> xFormMapper( rExp.GetFormExport()->getStylePropertyMapper().getBodyPtr() );
    xPropertySetMapper->ChainExportMapper(xFormMapper);
*/

    rExport.GetAutoStylePool()->AddFamily(
        XML_STYLE_FAMILY_SD_GRAPHICS_ID,
        OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_SD_GRAPHICS_NAME)),
        GetPropertySetMapper(),
        OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_SD_GRAPHICS_PREFIX)));
    rExport.GetAutoStylePool()->AddFamily(
        XML_STYLE_FAMILY_SD_PRESENTATION_ID,
        OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_SD_PRESENTATION_NAME)),
        GetPropertySetMapper(),
        OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_SD_PRESENTATION_PREFIX)));

    maCurrentInfo = maShapeInfos.end();
}

///////////////////////////////////////////////////////////////////////

XMLShapeExport::~XMLShapeExport()
{
}

///////////////////////////////////////////////////////////////////////

// This method collects all automatic styles for the given XShape
void XMLShapeExport::collectShapeAutoStyles(const uno::Reference< drawing::XShape >& xShape)
{
    if( maCurrentShapesIter == maShapesInfos.end() )
    {
        DBG_ERROR( "XMLShapeExport::collectShapeAutoStyles(): no call to seekShapes()!" );
        return;
    }

    sal_Int32 nZIndex = 0;
    uno::Reference< beans::XPropertySet > xSet( xShape, uno::UNO_QUERY );
    if( xSet.is() )
        xSet->getPropertyValue(msZIndex) >>= nZIndex;

    ImplXMLShapeExportInfo& aShapeInfo = (*maCurrentShapesIter).second[nZIndex];

    // -----------------------------
    // first compute the shapes type
    // -----------------------------
    ImpCalcShapeType(xShape, aShapeInfo.meShapeType);

    // ------------------------------
    // compute the shape parent style
    // ------------------------------
    uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        uno::Reference< beans::XPropertySetInfo > xPropSetInfo( xPropSet->getPropertySetInfo() );

        OUString aParentName;
        uno::Reference< style::XStyle > xStyle;

        if( xPropSetInfo.is() && xPropSetInfo->hasPropertyByName( OUString(RTL_CONSTASCII_USTRINGPARAM("Style"))) )
            xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("Style"))) >>= xStyle;

        if(xStyle.is())
        {
            // get family ID
            uno::Reference< beans::XPropertySet > xStylePropSet(xStyle, uno::UNO_QUERY);
            DBG_ASSERT( xStylePropSet.is(), "style without a XPropertySet?" );
            if(xStylePropSet.is())
            {
                OUString aFamilyName;
                xStylePropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("Family"))) >>= aFamilyName;
                if(aFamilyName.getLength() && aFamilyName.equals(OUString(RTL_CONSTASCII_USTRINGPARAM("presentation"))))
                    aShapeInfo.mnFamily = XML_STYLE_FAMILY_SD_PRESENTATION_ID;
            }

            // get parent-style name
            if(XML_STYLE_FAMILY_SD_PRESENTATION_ID == aShapeInfo.mnFamily)
            {
                aParentName = msPresentationStylePrefix;
            }

            aParentName += xStyle->getName();
        }

        // if this shape is a control shape we merge the styles
        // from this shape with the styles from the controls model
        if( aShapeInfo.meShapeType == XmlShapeTypeDrawControlShape )
        {
            do
            {
                uno::Reference< drawing::XControlShape > xControl( xShape, uno::UNO_QUERY );
                if( !xControl.is() )
                    break;

                uno::Reference< beans::XPropertySet > xControlModel( xControl->getControl(), uno::UNO_QUERY );
                if( !xControlModel.is() )
                    break;

                xPropSet = PropertySetMerger_CreateInstance( xPropSet, xControlModel );
            } while(0);
        }

        // filter propset
        std::vector< XMLPropertyState > xPropStates = GetPropertySetMapper()->Filter( xPropSet );

        sal_Int32 nCount = 0;
        std::vector< XMLPropertyState >::iterator aIter = xPropStates.begin();
        const std::vector< XMLPropertyState >::iterator aEnd = xPropStates.end();
        while( aIter != aEnd )
        {
            if( aIter->mnIndex != -1 )
                nCount++;
            aIter++;
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
            aShapeInfo.msStyleName = rExport.GetAutoStylePool()->Find(aShapeInfo.mnFamily, aParentName, xPropStates);

            if(!aShapeInfo.msStyleName.getLength())
            {
                // Style did not exist, add it to AutoStalePool
                aShapeInfo.msStyleName = rExport.GetAutoStylePool()->Add(aShapeInfo.mnFamily, aParentName, xPropStates);
            }
        }
    }

    // ----------------
    // prep text styles
    // ----------------
    if( aShapeInfo.meShapeType != XmlShapeTypeDrawControlShape &&
        aShapeInfo.meShapeType != XmlShapeTypeDrawChartShape &&
        aShapeInfo.meShapeType != XmlShapeTypePresChartShape &&
        aShapeInfo.meShapeType != XmlShapeTypeDrawOLE2Shape &&
        aShapeInfo.meShapeType != XmlShapeTypePresOLE2Shape )
    {
        uno::Reference< text::XText > xText(xShape, uno::UNO_QUERY);
        if(xText.is())
        {
            uno::Reference< beans::XPropertySetInfo > xPropSetInfo( xPropSet->getPropertySetInfo() );

            sal_Bool bIsEmptyPresObj = sal_False;

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

    // -------------------
    // check for chart ole
    // -------------------
    if( aShapeInfo.meShapeType == XmlShapeTypeDrawChartShape || aShapeInfo.meShapeType == XmlShapeTypePresChartShape )
    {
        uno::Reference< chart::XChartDocument > xChartDoc;
        xPropSet->getPropertyValue(msModel) >>= xChartDoc;
        if( xChartDoc.is() )
        {
            GetExport().GetChartExport()->collectAutoStyles( xChartDoc );
        }
    }

    // -------------------
    // check for connector
    // -------------------
    if( aShapeInfo.meShapeType == XmlShapeTypeDrawConnectorShape )
    {
        uno::Reference< drawing::XShape > xConnection;

        // create shape ids for export later
        if( xPropSet->getPropertyValue( msStartShape ) >>= xConnection )
        {
            createShapeId( xConnection );
        }
        if( xPropSet->getPropertyValue( msEndShape ) >>= xConnection )
        {
            createShapeId( xConnection );
        }
    }

    maShapeInfos.push_back( aShapeInfo );
    maCurrentInfo = maShapeInfos.begin();

    // -----------------------------------------------------
    // check for shape collections (group shape or 3d scene)
    // and collect contained shapes style infos
    // -----------------------------------------------------
    {
        uno::Reference< drawing::XShapes > xShapes( xShape, uno::UNO_QUERY );
        if( xShapes.is() )
        {
            collectShapesAutoStyles( xShapes );
        }
    }
}

///////////////////////////////////////////////////////////////////////

// This method exports the given XShape
void XMLShapeExport::exportShape(const uno::Reference< drawing::XShape >& xShape,
                                 sal_Int32 nFeatures /* = SEF_DEFAULT */,
                                 com::sun::star::awt::Point* pRefPoint /* = NULL */ )
{
    if( maCurrentShapesIter == maShapesInfos.end() )
    {
        DBG_ERROR( "XMLShapeExport::exportShape(): no auto styles where collected before export" );
        return;
    }

    sal_Int32 nZIndex = 0;
    uno::Reference< beans::XPropertySet > xSet( xShape, uno::UNO_QUERY );
    if( xSet.is() )
        xSet->getPropertyValue(msZIndex) >>= nZIndex;

    const ImplXMLShapeExportInfo& aShapeInfo = (*maCurrentShapesIter).second[nZIndex];

#ifndef PRODUCT
    // ---------------------------------------
    // check if this is the correct ShapesInfo
    // ---------------------------------------
    uno::Reference< container::XChild > xChild( xShape, uno::UNO_QUERY );
    if( xChild.is() )
    {
        uno::Reference< drawing::XShapes > xParent( xChild->getParent(), uno::UNO_QUERY );
        DBG_ASSERT( xParent.is() && xParent.get() == (*maCurrentShapesIter).first.get(), "XMLShapeExport::exportShape(): Wrong call to XMLShapeExport::seekShapes()" );
    }

    // -----------------------------
    // first compute the shapes type
    // -----------------------------
    {
        XmlShapeType eShapeType(XmlShapeTypeNotYetSet);
        ImpCalcShapeType(xShape, eShapeType);

        DBG_ASSERT( eShapeType == aShapeInfo.meShapeType, "exportShape callings do not correspond to collectShapeAutoStyles calls!" );
    }
#endif

    // ----------------------------------------
    // collect animation informations if needed
    // ----------------------------------------
    if( mxAnimationsExporter.is() )
        mxAnimationsExporter->collect( xShape );

    // -------------------------------
    // export shapes name if he has one
    // -------------------------------
    {
        uno::Reference< container::XNamed > xNamed( xShape, uno::UNO_QUERY );
        if( xNamed.is() )
        {
            const OUString aName( xNamed->getName() );
            if( aName.getLength() )
                rExport.AddAttribute(XML_NAMESPACE_DRAW, sXML_name, aName );
        }
    }

    // ------------------
    // export style name
    // ------------------
    if( aShapeInfo.msStyleName.getLength() != 0 )
    {
        if(XML_STYLE_FAMILY_SD_GRAPHICS_ID == aShapeInfo.mnFamily)
            rExport.AddAttribute(XML_NAMESPACE_DRAW, sXML_style_name, aShapeInfo.msStyleName);
        else
            rExport.AddAttribute(XML_NAMESPACE_PRESENTATION, sXML_style_name, aShapeInfo.msStyleName);
    }

    // --------------------------
    // export shapes id if needed
    // --------------------------
    {
        const sal_Int32 nShapeId = getShapeId( xShape );
        if( nShapeId != -1 )
        {
            const OUString sId( OUString::valueOf( nShapeId ) );
            rExport.AddAttribute(XML_NAMESPACE_DRAW, sXML_id, sId );
        }
    }

    // --------------------------
    // export layer information
    // --------------------------
    if( IsLayerExportEnabled() )
    {
        try
        {
            uno::Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY );
            OUString aLayerName;
            xProps->getPropertyValue( OUString::createFromAscii( "LayerName" ) ) >>= aLayerName;
            rExport.AddAttribute(XML_NAMESPACE_DRAW, sXML_layer, aLayerName );

        }
        catch( uno::Exception e )
        {
            DBG_ERROR( "could not export layer name for shape!" );
        }
    }

    // --------------------
    // export shape element
    // --------------------
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
            ImpExportChartShape(xShape, aShapeInfo.meShapeType, nFeatures, pRefPoint );
            break;
        }

        case XmlShapeTypeDrawTableShape:
        case XmlShapeTypePresTableShape:
        {
            ImpExportSpreadsheetShape(xShape, aShapeInfo.meShapeType, nFeatures, pRefPoint );
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
        {
            ImpExportOLE2Shape(xShape, aShapeInfo.meShapeType, nFeatures, pRefPoint );
            break;
        }

        case XmlShapeTypeDrawPageShape:
        case XmlShapeTypePresPageShape:
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

        case XmlShapeTypePresOrgChartShape:
        case XmlShapeTypeDrawFrameShape:
        case XmlShapeTypeUnknown:
        case XmlShapeTypeNotYetSet:
        default:
        {
            // this should never happen and is an error
            DBG_ERROR("XMLEXP: WriteShape: unknown or unexpected type of shape in export!");
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////

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
        DBG_ASSERT( xShape.is(), "Shape without a XShape?" );
        if(!xShape.is())
            continue;

        collectShapeAutoStyles( xShape );
    }

    maCurrentShapesIter = aOldCurrentShapesIter;
}

///////////////////////////////////////////////////////////////////////

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
        DBG_ASSERT( xShape.is(), "Shape without a XShape?" );
        if(!xShape.is())
            continue;

        exportShape( xShape, nFeatures, pRefPoint );
    }

    maCurrentShapesIter = aOldCurrentShapesIter;
}

///////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////

void XMLShapeExport::exportAutoStyles()
{
    // export all autostyle infos

    // ...for graphic
//  if(IsFamilyGraphicUsed())
    {
        GetExport().GetAutoStylePool()->exportXML(
            XML_STYLE_FAMILY_SD_GRAPHICS_ID,
            GetExport().GetDocHandler(),
            GetExport().GetMM100UnitConverter(),
            GetExport().GetNamespaceMap());
    }

    // ...for presentation
//  if(IsFamilyPresentationUsed())
    {
        GetExport().GetAutoStylePool()->exportXML(
            XML_STYLE_FAMILY_SD_PRESENTATION_ID,
            GetExport().GetDocHandler(),
            GetExport().GetMM100UnitConverter(),
            GetExport().GetNamespaceMap());
    }
}

///////////////////////////////////////////////////////////////////////

/** creates a unique id for this shape, this id is saved and exported with this shape later
    with the exportShape method. Its ok to call this twice with the same shape */
void XMLShapeExport::createShapeId( const uno::Reference < drawing::XShape >& xShape )
{
    ShapeIdsMap::iterator aId( maShapeIds.find( xShape ) );

    if( aId == maShapeIds.end() )
        maShapeIds[xShape] = mnNextUniqueShapeId++;
}

///////////////////////////////////////////////////////////////////////

/** returns the unique id for this shape. It returns -1 if the was no createShapeId call
    for this shape yet. */
sal_Int32 XMLShapeExport::getShapeId( const uno::Reference < drawing::XShape >& xShape )
{
    ShapeIdsMap::iterator aId( maShapeIds.find( xShape ) );

    if( aId != maShapeIds.end() )
        return (*aId).second;

    return -1;
}

///////////////////////////////////////////////////////////////////////

/// returns the export property mapper for external chaining
SvXMLExportPropertyMapper* XMLShapeExport::CreateShapePropMapper(
    SvXMLExport& rExport )
{
    UniReference< XMLPropertyHandlerFactory > xFactory = new XMLSdPropHdlFactory( rExport.GetModel() );
    UniReference < XMLPropertySetMapper > xMapper = new XMLShapePropertySetMapper( xFactory );
    SvXMLExportPropertyMapper* pResult =
        new XMLShapeExportPropertyMapper( xMapper,
                                          (XMLTextListAutoStylePool*)&rExport.GetTextParagraphExport()->GetListAutoStylePool(),
                                          rExport );
    // chain text attributes
    pResult->ChainExportMapper( XMLTextParagraphExport::CreateParaExtPropMapper( rExport ));
    return pResult;
}

///////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpCalcShapeType(const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType& eShapeType)
{
    // set in every case, so init here
    eShapeType = XmlShapeTypeUnknown;

    uno::Reference< drawing::XShapeDescriptor > xShapeDescriptor(xShape, uno::UNO_QUERY);
    if(xShapeDescriptor.is())
    {
        String aType((OUString)xShapeDescriptor->getShapeType());

        if(aType.EqualsAscii((const sal_Char*)"com.sun.star.", 0, 13))
        {
            if(aType.EqualsAscii("drawing.", 13, 8))
            {
                // drawing shapes
                if     (aType.EqualsAscii("Rectangle", 21, 9)) { eShapeType = XmlShapeTypeDrawRectangleShape; }
                else if(aType.EqualsAscii("Ellipse", 21, 7)) { eShapeType = XmlShapeTypeDrawEllipseShape; }
                else if(aType.EqualsAscii("Control", 21, 7)) { eShapeType = XmlShapeTypeDrawControlShape; }
                else if(aType.EqualsAscii("Connector", 21, 9)) { eShapeType = XmlShapeTypeDrawConnectorShape; }
                else if(aType.EqualsAscii("Measure", 21, 7)) { eShapeType = XmlShapeTypeDrawMeasureShape; }
                else if(aType.EqualsAscii("Line", 21, 4)) { eShapeType = XmlShapeTypeDrawLineShape; }
                else if(aType.EqualsAscii("PolyPolygon", 21, 11)) { eShapeType = XmlShapeTypeDrawPolyPolygonShape; }
                else if(aType.EqualsAscii("PolyLine", 21, 8)) { eShapeType = XmlShapeTypeDrawPolyLineShape; }
                else if(aType.EqualsAscii("OpenBezier", 21, 10)) { eShapeType = XmlShapeTypeDrawOpenBezierShape; }
                else if(aType.EqualsAscii("ClosedBezier", 21, 12)) { eShapeType = XmlShapeTypeDrawClosedBezierShape; }
                else if(aType.EqualsAscii("GraphicObject", 21, 13)) { eShapeType = XmlShapeTypeDrawGraphicObjectShape; }
                else if(aType.EqualsAscii("Group", 21, 5)) { eShapeType = XmlShapeTypeDrawGroupShape; }
                else if(aType.EqualsAscii("Text", 21, 4)) { eShapeType = XmlShapeTypeDrawTextShape; }
                else if(aType.EqualsAscii("OLE2", 21, 4))
                {
                    eShapeType = XmlShapeTypeDrawOLE2Shape;

                    // get info about presentation shape
                    uno::Reference <beans::XPropertySet> xPropSet(xShape, uno::UNO_QUERY);

                    if(xPropSet.is())
                    {
                        uno::Any aAny;
                        aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("Model")));
                        uno::Reference <lang::XServiceInfo> xObjectInfo;

                        if(aAny >>= xObjectInfo)
                        {
                            if(xObjectInfo->supportsService(OUString(RTL_CONSTASCII_USTRINGPARAM
                                ("com.sun.star.chart.ChartDocument"))))
                            {
                                eShapeType = XmlShapeTypeDrawChartShape;
                            }
                            else if(xObjectInfo->supportsService(OUString(RTL_CONSTASCII_USTRINGPARAM
                                ("com.sun.star.sheet.SpreadsheetDocument"))))
                            {
                                eShapeType = XmlShapeTypeDrawTableShape;
                            }
                            else
                            {
                                // general OLE2 Object
                            }
                        }
                    }
                }
                else if(aType.EqualsAscii("Page", 21, 4)) { eShapeType = XmlShapeTypeDrawPageShape; }
                else if(aType.EqualsAscii("Frame", 21, 5)) { eShapeType = XmlShapeTypeDrawFrameShape; }
                else if(aType.EqualsAscii("Caption", 21, 6)) { eShapeType = XmlShapeTypeDrawCaptionShape; }

                // 3D shapes
                else if(aType.EqualsAscii("Scene", 21 + 7, 5)) { eShapeType = XmlShapeTypeDraw3DSceneObject; }
                else if(aType.EqualsAscii("Cube", 21 + 7, 4)) { eShapeType = XmlShapeTypeDraw3DCubeObject; }
                else if(aType.EqualsAscii("Sphere", 21 + 7, 6)) { eShapeType = XmlShapeTypeDraw3DSphereObject; }
                else if(aType.EqualsAscii("Lathe", 21 + 7, 5)) { eShapeType = XmlShapeTypeDraw3DLatheObject; }
                else if(aType.EqualsAscii("Extrude", 21 + 7, 7)) { eShapeType = XmlShapeTypeDraw3DExtrudeObject; }
            }
            else if(aType.EqualsAscii("presentation.", 13, 13))
            {
                // presentation shapes
                if     (aType.EqualsAscii("TitleText", 26, 9)) { eShapeType = XmlShapeTypePresTitleTextShape; }
                else if(aType.EqualsAscii("Outliner", 26, 8)) { eShapeType = XmlShapeTypePresOutlinerShape;  }
                else if(aType.EqualsAscii("Subtitle", 26, 8)) { eShapeType = XmlShapeTypePresSubtitleShape;  }
                else if(aType.EqualsAscii("GraphicObject", 26, 13)) { eShapeType = XmlShapeTypePresGraphicObjectShape;  }
                else if(aType.EqualsAscii("Page", 26, 4)) { eShapeType = XmlShapeTypePresPageShape;  }
                else if(aType.EqualsAscii("OLE2", 26, 4)) { eShapeType = XmlShapeTypePresOLE2Shape; }
                else if(aType.EqualsAscii("Chart", 26, 5)) { eShapeType = XmlShapeTypePresChartShape;  }
                else if(aType.EqualsAscii("Table", 26, 5)) { eShapeType = XmlShapeTypePresTableShape;  }
                else if(aType.EqualsAscii("OrgChart", 26, 8)) { eShapeType = XmlShapeTypePresOrgChartShape;  }
                else if(aType.EqualsAscii("Notes", 26, 5)) { eShapeType = XmlShapeTypePresNotesShape;  }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////
