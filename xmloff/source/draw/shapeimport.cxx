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

#include <tools/debug.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <sal/log.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/PositionLayoutDir.hpp>
#include <com/sun/star/drawing/XShapes3.hpp>

#include <utility>
#include <xmloff/unointerfacetouniqueidentifiermapper.hxx>

#include <xmloff/shapeimport.hxx>
#include <xmloff/xmlstyle.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/table/XMLTableImport.hxx>
#include "eventimp.hxx"
#include "ximpshap.hxx"
#include "sdpropls.hxx"
#include <xmloff/xmlprmap.hxx>
#include "ximp3dscene.hxx"
#include "ximp3dobject.hxx"
#include "ximpgrp.hxx"
#include "ximplink.hxx"

#include <unordered_map>
#include <vector>

namespace {

class ShapeGroupContext;

}

using namespace ::com::sun::star;
using namespace ::xmloff::token;

namespace {

struct ConnectionHint
{
    css::uno::Reference< css::drawing::XShape > mxConnector;
    OUString  aDestShapeId;
    sal_Int32 nDestGlueId;
    bool      bStart;
};

}

/** this map store all gluepoint id mappings for shapes that had user defined gluepoints. This
    is needed because on insertion the gluepoints will get a new and unique id */
typedef std::map<sal_Int32,sal_Int32> GluePointIdMap;
typedef std::unordered_map< css::uno::Reference < css::drawing::XShape >, GluePointIdMap > ShapeGluePointsMap;

/** this struct is created for each startPage() call and stores information that is needed during
    import of shapes for one page. Since pages could be nested ( notes pages inside impress ) there
    is a pointer so one can build up a stack of this structs */
struct XMLShapeImportPageContextImpl
{
    ShapeGluePointsMap      maShapeGluePointsMap;

    uno::Reference < drawing::XShapes > mxShapes;

    std::shared_ptr<XMLShapeImportPageContextImpl> mpNext;
};

/** this class is to enable adding members to the XMLShapeImportHelper without getting incompatible */
struct XMLShapeImportHelperImpl
{
    // context for sorting shapes
    std::shared_ptr<ShapeGroupContext> mpGroupContext;

    std::vector<ConnectionHint> maConnections;

    // #88546# possibility to switch progress bar handling on/off
    bool                        mbHandleProgressBar;

    // stores the capability of the current model to create presentation shapes
    bool                        mbIsPresentationShapesSupported;
};

XMLShapeImportHelper::XMLShapeImportHelper(
        SvXMLImport& rImporter,
        const uno::Reference< frame::XModel>& rModel,
        SvXMLImportPropertyMapper *pExtMapper )
:   mpImpl( new XMLShapeImportHelperImpl ),
    mrImporter( rImporter )
{
    mpImpl->mpGroupContext = nullptr;

    // #88546# init to sal_False
    mpImpl->mbHandleProgressBar = false;

    mpSdPropHdlFactory = new XMLSdPropHdlFactory( rModel, rImporter );

    // construct PropertySetMapper
    rtl::Reference < XMLPropertySetMapper > xMapper = new XMLShapePropertySetMapper(mpSdPropHdlFactory, false);
    mpPropertySetMapper = new SvXMLImportPropertyMapper( xMapper, rImporter );

    if( pExtMapper )
    {
        rtl::Reference < SvXMLImportPropertyMapper > xExtMapper( pExtMapper );
        mpPropertySetMapper->ChainImportMapper( xExtMapper );
    }

    // chain text attributes
    mpPropertySetMapper->ChainImportMapper(XMLTextImportHelper::CreateParaExtPropMapper(rImporter));
    mpPropertySetMapper->ChainImportMapper(XMLTextImportHelper::CreateParaDefaultExtPropMapper(rImporter));

    // construct PresPagePropsMapper
    xMapper = new XMLPropertySetMapper(aXMLSDPresPageProps, mpSdPropHdlFactory, false);
    mpPresPagePropsMapper = new SvXMLImportPropertyMapper( xMapper, rImporter );

    uno::Reference< lang::XServiceInfo > xInfo( rImporter.GetModel(), uno::UNO_QUERY );
    mpImpl->mbIsPresentationShapesSupported = xInfo.is() && xInfo->supportsService( u"com.sun.star.presentation.PresentationDocument"_ustr );
}

XMLShapeImportHelper::~XMLShapeImportHelper()
{
    SAL_WARN_IF( !mpImpl->maConnections.empty(), "xmloff", "XMLShapeImportHelper::restoreConnections() was not called!" );

    // cleanup factory, decrease refcount. Should lead to destruction.
    mpSdPropHdlFactory.clear();

    // cleanup mapper, decrease refcount. Should lead to destruction.
    mpPropertySetMapper.clear();

    // cleanup presPage mapper, decrease refcount. Should lead to destruction.
    mpPresPagePropsMapper.clear();

    // Styles or AutoStyles context?
    if(mxStylesContext.is())
        mxStylesContext->dispose();

    if(mxAutoStylesContext.is())
        mxAutoStylesContext->dispose();
}


SvXMLShapeContext* XMLShapeImportHelper::Create3DSceneChildContext(
    SvXMLImport& rImport,
    sal_Int32 nElement,
    const uno::Reference< xml::sax::XFastAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes > const & rShapes)
{
    SdXMLShapeContext *pContext = nullptr;

    if(rShapes.is())
    {
        switch(nElement)
        {
            case XML_ELEMENT(DR3D, XML_SCENE):
            {
                // dr3d:3dscene inside dr3d:3dscene context
                pContext = new SdXML3DSceneShapeContext( rImport, xAttrList, rShapes, false);
                break;
            }
            case XML_ELEMENT(DR3D, XML_CUBE):
            {
                // dr3d:3dcube inside dr3d:3dscene context
                pContext = new SdXML3DCubeObjectShapeContext( rImport, xAttrList, rShapes);
                break;
            }
            case XML_ELEMENT(DR3D, XML_SPHERE):
            {
                // dr3d:3dsphere inside dr3d:3dscene context
                pContext = new SdXML3DSphereObjectShapeContext( rImport, xAttrList, rShapes);
                break;
            }
            case XML_ELEMENT(DR3D, XML_ROTATE):
            {
                // dr3d:3dlathe inside dr3d:3dscene context
                pContext = new SdXML3DLatheObjectShapeContext( rImport, xAttrList, rShapes);
                break;
            }
            case XML_ELEMENT(DR3D, XML_EXTRUDE):
            {
                // dr3d:3dextrude inside dr3d:3dscene context
                pContext = new SdXML3DExtrudeObjectShapeContext( rImport, xAttrList, rShapes);
                break;
            }
            default:
                XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
        }
    }

    if (!pContext)
        return nullptr;

    // now parse the attribute list and call the child context for each unknown attribute
    for(auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        if (!pContext->processAttribute( aIter ))
            XMLOFF_WARN_UNKNOWN("xmloff", aIter);

    }

    return pContext;
}

void XMLShapeImportHelper::SetStylesContext(SvXMLStylesContext* pNew)
{
    mxStylesContext.set(pNew);
}

void XMLShapeImportHelper::SetAutoStylesContext(SvXMLStylesContext* pNew)
{
    mxAutoStylesContext.set(pNew);
}

SvXMLShapeContext* XMLShapeImportHelper::CreateGroupChildContext(
    SvXMLImport& rImport,
    sal_Int32 nElement,
    const uno::Reference< xml::sax::XFastAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes > const & rShapes,
    bool bTemporaryShape)
{
    SdXMLShapeContext *pContext = nullptr;
    switch (nElement)
    {
        case XML_ELEMENT(DRAW, XML_G):
            // draw:g inside group context (RECURSIVE)
            pContext = new SdXMLGroupShapeContext( rImport, xAttrList, rShapes, bTemporaryShape);
            break;
        case XML_ELEMENT(DR3D, XML_SCENE):
        {
            // dr3d:3dscene inside group context
            pContext = new SdXML3DSceneShapeContext( rImport, xAttrList, rShapes, bTemporaryShape);
            break;
        }
        case XML_ELEMENT(DRAW, XML_RECT):
        {
            // draw:rect inside group context
            pContext = new SdXMLRectShapeContext( rImport, xAttrList, rShapes, bTemporaryShape );
            break;
        }
        case XML_ELEMENT(DRAW, XML_LINE):
        {
            // draw:line inside group context
            pContext = new SdXMLLineShapeContext( rImport, xAttrList, rShapes, bTemporaryShape );
            break;
        }
        case XML_ELEMENT(DRAW, XML_CIRCLE):
        case XML_ELEMENT(DRAW, XML_ELLIPSE):
        {
            // draw:circle or draw:ellipse inside group context
            pContext = new SdXMLEllipseShapeContext( rImport, xAttrList, rShapes, bTemporaryShape );
            break;
        }
        case XML_ELEMENT(DRAW, XML_POLYGON):
        case XML_ELEMENT(DRAW, XML_POLYLINE):
        {
            // draw:polygon or draw:polyline inside group context
            pContext = new SdXMLPolygonShapeContext( rImport, xAttrList, rShapes,
                           nElement == XML_ELEMENT(DRAW, XML_POLYGON), bTemporaryShape );
            break;
        }
        case XML_ELEMENT(DRAW, XML_PATH):
        {
            // draw:path inside group context
            pContext = new SdXMLPathShapeContext( rImport, xAttrList, rShapes, bTemporaryShape);
            break;
        }
        case XML_ELEMENT(DRAW, XML_FRAME):
        {
            // text:text-box inside group context
            pContext = new SdXMLFrameShapeContext( rImport, xAttrList, rShapes, bTemporaryShape );
            break;
        }
        case XML_ELEMENT(DRAW, XML_CONTROL):
        {
            // draw:control inside group context
            pContext = new SdXMLControlShapeContext( rImport, xAttrList, rShapes, bTemporaryShape );
            break;
        }
        case XML_ELEMENT(DRAW, XML_CONNECTOR):
        {
            // draw:connector inside group context
            pContext = new SdXMLConnectorShapeContext( rImport, xAttrList, rShapes, bTemporaryShape );
            break;
        }
        case XML_ELEMENT(DRAW, XML_MEASURE):
        {
            // draw:measure inside group context
            pContext = new SdXMLMeasureShapeContext( rImport, xAttrList, rShapes, bTemporaryShape );
            break;
        }
        case XML_ELEMENT(DRAW, XML_PAGE_THUMBNAIL):
        {
            // draw:page inside group context
            pContext = new SdXMLPageShapeContext( rImport, xAttrList, rShapes, bTemporaryShape );
            break;
        }
        case XML_ELEMENT(DRAW, XML_CAPTION):
        case XML_ELEMENT(OFFICE, XML_ANNOTATION):
        {
            // draw:caption inside group context
            pContext = new SdXMLCaptionShapeContext( rImport, xAttrList, rShapes, bTemporaryShape );
            break;
        }
        case XML_ELEMENT(CHART, XML_CHART):
        {
            // chart:chart inside group context
            pContext = new SdXMLChartShapeContext( rImport, xAttrList, rShapes, bTemporaryShape );
            break;
        }
        case XML_ELEMENT(DRAW, XML_CUSTOM_SHAPE):
        {
            // draw:customshape
            pContext = new SdXMLCustomShapeContext( rImport, xAttrList, rShapes );
            break;
        }
        case XML_ELEMENT(DRAW, XML_A):
            return new SdXMLShapeLinkContext( rImport, xAttrList, rShapes );
        // add other shapes here...
        default:
            XMLOFF_INFO_UNKNOWN_ELEMENT("xmloff", nElement);
            return new SvXMLShapeContext( rImport, bTemporaryShape );
    }

    // now parse the attribute list and call the child context for each unknown attribute
    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        if (!pContext->processAttribute( aIter ))
            XMLOFF_INFO_UNKNOWN("xmloff", aIter);
    }
    return pContext;
}

// This method is called from SdXMLFrameShapeContext to create children of draw:frame
SvXMLShapeContext* XMLShapeImportHelper::CreateFrameChildContext(
    SvXMLImport& rImport,
    sal_Int32 nElement,
    const uno::Reference< xml::sax::XFastAttributeList>& rAttrList,
    uno::Reference< drawing::XShapes > const & rShapes,
    const uno::Reference< xml::sax::XFastAttributeList>& rFrameAttrList)
{
    SdXMLShapeContext *pContext = nullptr;

    rtl::Reference<sax_fastparser::FastAttributeList> xCombinedAttrList = new sax_fastparser::FastAttributeList(rAttrList);
    if( rFrameAttrList.is() )
        xCombinedAttrList->add(rFrameAttrList);

    switch(nElement)
    {
        case XML_ELEMENT(DRAW, XML_TEXT_BOX):
        {
            // text:text-box inside group context
            pContext = new SdXMLTextBoxShapeContext( rImport, xCombinedAttrList, rShapes );
            break;
        }
        case XML_ELEMENT(DRAW, XML_IMAGE):
        {
            // office:image inside group context
            pContext = new SdXMLGraphicObjectShapeContext( rImport, xCombinedAttrList, rShapes );
            break;
        }
        case XML_ELEMENT(DRAW, XML_OBJECT):
        case XML_ELEMENT(DRAW, XML_OBJECT_OLE):
        {
            // draw:object or draw:object_ole
            pContext = new SdXMLObjectShapeContext( rImport, xCombinedAttrList, rShapes );
            break;
        }
        case XML_ELEMENT(TABLE, XML_TABLE):
        {
            // draw:object or draw:object_ole
            if( rImport.IsTableShapeSupported() )
                pContext = new SdXMLTableShapeContext( rImport, xCombinedAttrList, rShapes );
            break;

        }
        case XML_ELEMENT(DRAW, XML_PLUGIN):
        {
            // draw:plugin
            pContext = new SdXMLPluginShapeContext( rImport, xCombinedAttrList, rShapes );
            break;
        }
        case XML_ELEMENT(DRAW, XML_FLOATING_FRAME):
        {
            // draw:floating-frame
            pContext = new SdXMLFloatingFrameShapeContext( rImport, xCombinedAttrList, rShapes );
            break;
        }
        case XML_ELEMENT(DRAW, XML_APPLET):
        {
            // draw:applet
            pContext = new SdXMLAppletShapeContext( rImport, xCombinedAttrList, rShapes );
            break;
        }
        // add other shapes here...
        default:
            SAL_INFO("xmloff", "unknown element " << SvXMLImport::getPrefixAndNameFromToken(nElement));
            break;
    }

    if( pContext )
    {
        // now parse the attribute list and call the child context for each unknown attribute
        for(auto& aIter : *xCombinedAttrList)
        {
            if (!pContext->processAttribute( aIter ))
                SAL_INFO("xmloff", "unknown attribute " << SvXMLImport::getPrefixAndNameFromToken(aIter.getToken()) << " value=" << aIter.toString());
        }
    }

    return pContext;
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLShapeImportHelper::CreateFrameChildContext(
    SvXMLImportContext *pThisContext,
    sal_Int32 nElement,
    const uno::Reference< xml::sax::XFastAttributeList>& xAttrList )
{
    css::uno::Reference< css::xml::sax::XFastContextHandler > xContext;
    SdXMLFrameShapeContext *pFrameContext = dynamic_cast<SdXMLFrameShapeContext*>( pThisContext  );
    if (pFrameContext)
        xContext = pFrameContext->createFastChildContext( nElement, xAttrList );

    if (!xContext)
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    return xContext;
}

/** this function is called whenever the implementation classes like to add this new
    shape to the given XShapes.
*/
void XMLShapeImportHelper::addShape( uno::Reference< drawing::XShape >& rShape,
                                     const uno::Reference< xml::sax::XFastAttributeList >&,
                                     uno::Reference< drawing::XShapes >& rShapes)
{
    if( rShape.is() && rShapes.is() )
    {
        // add new shape to parent
        rShapes->add( rShape );

        uno::Reference<beans::XPropertySet> xPropertySet(rShape, uno::UNO_QUERY);
        if (xPropertySet.is())
        {
            xPropertySet->setPropertyValue(u"HandlePathObjScale"_ustr, uno::Any(true));
        }
    }
}

/** this function is called whenever the implementation classes have finished importing
    a shape to the given XShapes. The shape is already inserted into its XShapes and
    all properties and styles are set.
*/
void XMLShapeImportHelper::finishShape(
        css::uno::Reference< css::drawing::XShape >& rShape,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >&,
        css::uno::Reference< css::drawing::XShapes >&)
{
    /* Set property <PositionLayoutDir>
       to <PositionInHoriL2R>, if it exists and the import states that
       the shape positioning attributes are in horizontal left-to-right
       layout. This is the case for the OpenOffice.org file format.
       This setting is done for Writer documents, because the property
       only exists at service css::text::Shape - the Writer
       UNO service for shapes.
       The value indicates that the positioning attributes are given
       in horizontal left-to-right layout. The property is evaluated
       during the first positioning of the shape in order to convert
       the shape position given in the OpenOffice.org file format to
       the one for the OASIS Open Office file format. (#i28749#, #i36248#)
    */
    uno::Reference< beans::XPropertySet > xPropSet(rShape, uno::UNO_QUERY);
    if ( xPropSet.is() )
    {
        if ( mrImporter.IsShapePositionInHoriL2R() &&
             xPropSet->getPropertySetInfo()->hasPropertyByName(
                u"PositionLayoutDir"_ustr) )
        {
            uno::Any aPosLayoutDir;
            aPosLayoutDir <<= text::PositionLayoutDir::PositionInHoriL2R;
            xPropSet->setPropertyValue( u"PositionLayoutDir"_ustr, aPosLayoutDir );
        }
    }
}

namespace {

// helper functions for z-order sorting
struct ZOrderHint
{
    sal_Int32 nIs;
    sal_Int32 nShould;
    /// The hint is for this shape. We don't use uno::Reference here to speed up
    /// some operations, and because this shape is always held by mxShapes
    drawing::XShape* pShape;

    bool operator<(const ZOrderHint& rComp) const { return nShould < rComp.nShould; }
};

// a) handle z-order of group contents after it has been imported
// b) apply group events over group contents after it has been imported
class ShapeGroupContext
{
public:
    uno::Reference< drawing::XShapes > mxShapes;
    std::vector<SdXMLEventContextData> maEventData;
    std::vector<ZOrderHint>              maZOrderList;
    std::vector<ZOrderHint>              maUnsortedList;

    sal_Int32                       mnCurrentZ;
    std::shared_ptr<ShapeGroupContext> mpParentContext;

    ShapeGroupContext( uno::Reference< drawing::XShapes > xShapes, std::shared_ptr<ShapeGroupContext> pParentContext );

    void popGroupAndPostProcess();
private:
    void moveShape( sal_Int32 nSourcePos, sal_Int32 nDestPos );
};

}

ShapeGroupContext::ShapeGroupContext( uno::Reference< drawing::XShapes > xShapes, std::shared_ptr<ShapeGroupContext> pParentContext )
:   mxShapes(std::move( xShapes )), mnCurrentZ( 0 ), mpParentContext( std::move(pParentContext) )
{
}

void ShapeGroupContext::moveShape( sal_Int32 nSourcePos, sal_Int32 nDestPos )
{
    uno::Any aAny( mxShapes->getByIndex( nSourcePos ) );
    uno::Reference< beans::XPropertySet > xPropSet;
    aAny >>= xPropSet;

    if( !(xPropSet.is() && xPropSet->getPropertySetInfo()->hasPropertyByName( u"ZOrder"_ustr )) )
        return;

    xPropSet->setPropertyValue( u"ZOrder"_ustr, uno::Any(nDestPos) );

    for( ZOrderHint& rHint : maZOrderList )
    {
        if( rHint.nIs < nSourcePos )
        {
            DBG_ASSERT(rHint.nIs >= nDestPos, "Shape sorting failed" );
            rHint.nIs++;
        }
    }

    for( ZOrderHint& rHint : maUnsortedList )
    {
        if( rHint.nIs < nSourcePos )
        {
            SAL_WARN_IF( rHint.nIs < nDestPos, "xmloff", "shape sorting failed" );
            rHint.nIs++;
        }
    }
}

// sort shapes
void ShapeGroupContext::popGroupAndPostProcess()
{
    if (!maEventData.empty())
    {
        // tdf#127791 wait until a group is popped to set its event data
        for (auto& event : maEventData)
            event.ApplyProperties();
        maEventData.clear();
    }

    // only do something if we have shapes to sort
    if( maZOrderList.empty() )
        return;

    // check if there are more shapes than inserted with ::shapeWithZIndexAdded()
    // This can happen if there where already shapes on the page before import
    // Since the writer may delete some of this shapes during import, we need
    // to do this here and not in our c'tor anymore

    // check if we have more shapes than we know of
    sal_Int32 nCount = mxShapes->getCount();

    nCount -= maZOrderList.size();
    nCount -= maUnsortedList.size();

    if( nCount > 0 )
    {
        // first update offsets of added shapes
        for (ZOrderHint& rHint : maZOrderList)
            rHint.nIs += nCount;
        for (ZOrderHint& rHint : maUnsortedList)
            rHint.nIs += nCount;

        // second add the already existing shapes in the unsorted list
        ZOrderHint aNewHint;
        aNewHint.pShape = nullptr;
        do
        {
            nCount--;

            aNewHint.nIs = nCount;
            aNewHint.nShould = -1;

            maUnsortedList.insert(maUnsortedList.begin(), aNewHint);
        }
        while( nCount );
    }

    bool bSorted = std::is_sorted(maZOrderList.begin(), maZOrderList.end(),
                       [](const ZOrderHint& rLeft, const ZOrderHint& rRight)
                       { return rLeft.nShould < rRight.nShould; } );

    if (bSorted)
        return; // nothin' to do

    // sort z-ordered shapes by nShould field
    std::sort(maZOrderList.begin(), maZOrderList.end());

    uno::Reference<drawing::XShapes3> xShapes3(mxShapes, uno::UNO_QUERY);
    if( xShapes3.is())
    {
        uno::Sequence<sal_Int32> aNewOrder(maZOrderList.size() + maUnsortedList.size());
        auto pNewOrder = aNewOrder.getArray();
        sal_Int32 nIndex = 0;

        for (const ZOrderHint& rHint : maZOrderList)
        {
            // fill in the gaps from unordered list
            for (std::vector<ZOrderHint>::iterator aIt = maUnsortedList.begin(); aIt != maUnsortedList.end() && nIndex < rHint.nShould; )
            {
                pNewOrder[nIndex++] = (*aIt).nIs;
                aIt = maUnsortedList.erase(aIt);
            }

            pNewOrder[nIndex] = rHint.nIs;
            nIndex++;
        }

        try
        {
            xShapes3->sort(aNewOrder);
            maZOrderList.clear();
            return;
        }
        catch (const css::lang::IllegalArgumentException& /*e*/)
        {}
    }

    // this is the current index, all shapes before that
    // index are finished
    sal_Int32 nIndex = 0;
    for (const ZOrderHint& rHint : maZOrderList)
    {
        for (std::vector<ZOrderHint>::iterator aIt = maUnsortedList.begin(); aIt != maUnsortedList.end() && nIndex < rHint.nShould; )
        {
            moveShape( (*aIt).nIs, nIndex++ );
            aIt = maUnsortedList.erase(aIt);

        }

        if(rHint.nIs != nIndex )
            moveShape( rHint.nIs, nIndex );

        nIndex++;
    }
    maZOrderList.clear();
}

void XMLShapeImportHelper::pushGroupForPostProcessing( uno::Reference< drawing::XShapes >& rShapes )
{
    mpImpl->mpGroupContext = std::make_shared<ShapeGroupContext>( rShapes, mpImpl->mpGroupContext );
}

void XMLShapeImportHelper::addShapeEvents(SdXMLEventContextData& rData)
{
    if (mpImpl->mpGroupContext && mpImpl->mpGroupContext->mxShapes == rData.mxShape)
    {
        // tdf#127791 wait until a group is popped to set its event data so
        // that the events are applied to all its children, which are not available
        // at the start of the group tag
        mpImpl->mpGroupContext->maEventData.push_back(rData);
    }
    else
        rData.ApplyProperties();
}

void XMLShapeImportHelper::popGroupAndPostProcess()
{
    SAL_WARN_IF( !mpImpl->mpGroupContext, "xmloff", "No context to sort!" );
    if( !mpImpl->mpGroupContext )
        return;

    try
    {
        mpImpl->mpGroupContext->popGroupAndPostProcess();
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("xmloff", "exception while sorting shapes, sorting failed");
    }

    // put parent on top and drop current context, we are done
    mpImpl->mpGroupContext = mpImpl->mpGroupContext->mpParentContext;
}

void XMLShapeImportHelper::shapeWithZIndexAdded( css::uno::Reference< css::drawing::XShape > const & xShape, sal_Int32 nZIndex )
{
    if( !mpImpl->mpGroupContext)
        return;

    ZOrderHint aNewHint;
    aNewHint.nIs = mpImpl->mpGroupContext->mnCurrentZ++;
    aNewHint.nShould = nZIndex;
    aNewHint.pShape = xShape.get();

    if( nZIndex == -1 )
    {
        // don't care, so add to unsorted list
        mpImpl->mpGroupContext->maUnsortedList.push_back(aNewHint);
    }
    else
    {
        // insert into sort list
        mpImpl->mpGroupContext->maZOrderList.push_back(aNewHint);
    }
}

void XMLShapeImportHelper::shapeRemoved(const uno::Reference<drawing::XShape>& xShape)
{
    auto it = std::find_if(mpImpl->mpGroupContext->maZOrderList.begin(), mpImpl->mpGroupContext->maZOrderList.end(), [&xShape](const ZOrderHint& rHint)
    {
        return rHint.pShape == xShape.get();
    });
    if (it == mpImpl->mpGroupContext->maZOrderList.end())
        // Part of the unsorted list, nothing to do.
        return;

    sal_Int32 nZIndex = it->nIs;

    for (it = mpImpl->mpGroupContext->maZOrderList.begin(); it != mpImpl->mpGroupContext->maZOrderList.end();)
    {
        if (it->nIs == nZIndex)
        {
            // This is xShape: remove it and adjust the max of indexes
            // accordingly.
            it = mpImpl->mpGroupContext->maZOrderList.erase(it);
            mpImpl->mpGroupContext->mnCurrentZ--;
            continue;
        }
        else if (it->nIs > nZIndex)
            // On top of xShape: adjust actual index to reflect removal.
            it->nIs--;

        // On top of or below xShape.
        ++it;
    }
}

void XMLShapeImportHelper::addShapeConnection( css::uno::Reference< css::drawing::XShape > const & rConnectorShape,
                         bool bStart,
                         const OUString& rDestShapeId,
                         sal_Int32 nDestGlueId )
{
    ConnectionHint aHint;
    aHint.mxConnector = rConnectorShape;
    aHint.bStart = bStart;
    aHint.aDestShapeId = rDestShapeId;
    aHint.nDestGlueId = nDestGlueId;

    mpImpl->maConnections.push_back( aHint );
}

void XMLShapeImportHelper::restoreConnections()
{
    const std::vector<ConnectionHint>::size_type nCount = mpImpl->maConnections.size();
    for( std::vector<ConnectionHint>::size_type i = 0; i < nCount; i++ )
    {
        ConnectionHint& rHint = mpImpl->maConnections[i];
        uno::Reference< beans::XPropertySet > xConnector( rHint.mxConnector, uno::UNO_QUERY );
        if( xConnector.is() )
        {
            // #86637# remember line deltas
            uno::Any aLine1Delta;
            uno::Any aLine2Delta;
            uno::Any aLine3Delta;
            OUString aStr1(u"EdgeLine1Delta"_ustr);
            OUString aStr2(u"EdgeLine2Delta"_ustr);
            OUString aStr3(u"EdgeLine3Delta"_ustr);
            aLine1Delta = xConnector->getPropertyValue(aStr1);
            aLine2Delta = xConnector->getPropertyValue(aStr2);
            aLine3Delta = xConnector->getPropertyValue(aStr3);

            // #86637# simply setting these values WILL force the connector to do
            // a new layout promptly. So the line delta values have to be rescued
            // and restored around connector changes.
            uno::Reference< drawing::XShape > xShape(
                mrImporter.getInterfaceToIdentifierMapper().getReference( rHint.aDestShapeId ), uno::UNO_QUERY );
            if( xShape.is() )
            {
                if (rHint.bStart)
                    xConnector->setPropertyValue( u"StartShape"_ustr, uno::Any(xShape) );
                else
                    xConnector->setPropertyValue( u"EndShape"_ustr, uno::Any(xShape) );

                sal_Int32 nGlueId = rHint.nDestGlueId < 4 ? rHint.nDestGlueId : getGluePointId( xShape, rHint.nDestGlueId );
                if(rHint.bStart)
                    xConnector->setPropertyValue( u"StartGluePointIndex"_ustr, uno::Any(nGlueId) );
                else
                    xConnector->setPropertyValue( u"EndGluePointIndex"_ustr, uno::Any(nGlueId) );
            }

            // #86637# restore line deltas
            xConnector->setPropertyValue(aStr1, aLine1Delta );
            xConnector->setPropertyValue(aStr2, aLine2Delta );
            xConnector->setPropertyValue(aStr3, aLine3Delta );
        }
    }
    mpImpl->maConnections.clear();
}

SvXMLImportPropertyMapper* XMLShapeImportHelper::CreateShapePropMapper( const uno::Reference< frame::XModel>& rModel, SvXMLImport& rImport )
{
    rtl::Reference< XMLPropertyHandlerFactory > xFactory = new XMLSdPropHdlFactory( rModel, rImport );
    rtl::Reference < XMLPropertySetMapper > xMapper = new XMLShapePropertySetMapper( xFactory, false );
    SvXMLImportPropertyMapper* pResult = new SvXMLImportPropertyMapper( xMapper, rImport );

    // chain text attributes
    pResult->ChainImportMapper( XMLTextImportHelper::CreateParaExtPropMapper( rImport ) );
    return pResult;
}

/** adds a mapping for a gluepoint identifier from an xml file to the identifier created after inserting
    the new gluepoint into the core. The saved mappings can be retrieved by getGluePointId() */
void XMLShapeImportHelper::addGluePointMapping( css::uno::Reference< css::drawing::XShape > const & xShape,
                          sal_Int32 nSourceId, sal_Int32 nDestinnationId )
{
    if( mpPageContext )
        mpPageContext->maShapeGluePointsMap[xShape][nSourceId] = nDestinnationId;
}

/** moves all current DestinationId's by n */
void XMLShapeImportHelper::moveGluePointMapping( const css::uno::Reference< css::drawing::XShape >& xShape, const sal_Int32 n )
{
    if( mpPageContext )
    {
        ShapeGluePointsMap::iterator aShapeIter( mpPageContext->maShapeGluePointsMap.find( xShape ) );
        if( aShapeIter != mpPageContext->maShapeGluePointsMap.end() )
        {
            for ( auto& rShapeId : (*aShapeIter).second )
            {
                if ( rShapeId.second != -1 )
                    rShapeId.second += n;
            }
        }
    }
}

/** retrieves a mapping for a gluepoint identifier from the current xml file to the identifier created after
    inserting the new gluepoint into the core. The mapping must be initialized first with addGluePointMapping() */
sal_Int32 XMLShapeImportHelper::getGluePointId( const css::uno::Reference< css::drawing::XShape >& xShape, sal_Int32 nSourceId )
{
    if( mpPageContext )
    {
        ShapeGluePointsMap::iterator aShapeIter( mpPageContext->maShapeGluePointsMap.find( xShape ) );
        if( aShapeIter != mpPageContext->maShapeGluePointsMap.end() )
        {
            GluePointIdMap::iterator aIdIter = (*aShapeIter).second.find(nSourceId);
            if( aIdIter != (*aShapeIter).second.end() )
                return (*aIdIter).second;
        }
    }

    return -1;
}

/** this method must be calling before the first shape is imported for the given page */
void XMLShapeImportHelper::startPage( css::uno::Reference< css::drawing::XShapes > const & rShapes )
{
    const std::shared_ptr<XMLShapeImportPageContextImpl> pOldContext = mpPageContext;
    mpPageContext = std::make_shared<XMLShapeImportPageContextImpl>();
    mpPageContext->mpNext = pOldContext;
    mpPageContext->mxShapes = rShapes;
}

/** this method must be calling after the last shape is imported for the given page */
void XMLShapeImportHelper::endPage( css::uno::Reference< css::drawing::XShapes > const & rShapes )
{
    SAL_WARN_IF( !mpPageContext || (mpPageContext->mxShapes != rShapes), "xmloff", "wrong call to endPage(), no startPage called or wrong page" );
    if( nullptr == mpPageContext )
        return;

    restoreConnections();

    mpPageContext = mpPageContext->mpNext;
}

/** defines if the import should increment the progress bar or not */
void XMLShapeImportHelper::enableHandleProgressBar()
{
    mpImpl->mbHandleProgressBar = true;
}

bool XMLShapeImportHelper::IsHandleProgressBarEnabled() const
{
    return mpImpl->mbHandleProgressBar;
}

/** queries the capability of the current model to create presentation shapes */
bool XMLShapeImportHelper::IsPresentationShapesSupported() const
{
    return mpImpl->mbIsPresentationShapesSupported;
}

const rtl::Reference< XMLTableImport >& XMLShapeImportHelper::GetShapeTableImport()
{
    if( !mxShapeTableImport.is() )
    {
        rtl::Reference< XMLPropertyHandlerFactory > xFactory( new XMLSdPropHdlFactory( mrImporter.GetModel(), mrImporter ) );
        rtl::Reference< XMLPropertySetMapper > xPropertySetMapper( new XMLShapePropertySetMapper( xFactory, false ) );
        mxShapeTableImport = new XMLTableImport( mrImporter, xPropertySetMapper, xFactory );
    }

    return mxShapeTableImport;
}

void SvXMLShapeContext::setHyperlink( const OUString& rHyperlink )
{
    msHyperlink = rHyperlink;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
