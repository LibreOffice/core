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

#include <functional>

#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/sax/XFastSAXSerializable.hpp>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>
#include <editeng/unoprnms.hxx>
#include "drawingml/textbody.hxx"
#include "drawingml/textparagraph.hxx"
#include "drawingml/textrun.hxx"
#include "drawingml/diagram/diagram.hxx"
#include "drawingml/fillproperties.hxx"
#include "oox/ppt/pptshapegroupcontext.hxx"
#include "oox/ppt/pptshape.hxx"

#include "diagramlayoutatoms.hxx"
#include "layoutatomvisitors.hxx"
#include "diagramfragmenthandler.hxx"

#include <iostream>
#include <fstream>

using namespace ::com::sun::star;

namespace oox { namespace drawingml {

namespace dgm {

void Connection::dump()
{
    SAL_INFO(
        "oox.drawingml",
        "cnx modelId " << msModelId << ", srcId " << msSourceId << ", dstId "
            << msDestId << ", parTransId " << msParTransId << ", presId "
            << msPresId << ", sibTransId " << msSibTransId << ", srcOrd "
            << mnSourceOrder << ", dstOrd " << mnDestOrder);
}

void Point::dump()
{
    SAL_INFO(
        "oox.drawingml",
        "pt text " << mpShape.get() << ", cnxId " << msCnxId << ", modelId "
            << msModelId << ", type " << mnType);
}

} // dgm namespace

DiagramData::DiagramData()
    : mpFillProperties( new FillProperties )
{
}

void DiagramData::dump()
{
    SAL_INFO("oox.drawingml", "Dgm: DiagramData # of cnx: " << maConnections.size() );
    for (auto& rConnection : maConnections)
        rConnection.dump();

    SAL_INFO("oox.drawingml", "Dgm: DiagramData # of pt: " << maPoints.size() );
    for (auto& rPoint : maPoints)
        rPoint.dump();
}

void Diagram::setData( const DiagramDataPtr & pData)
{
    mpData = pData;
}

void Diagram::setLayout( const DiagramLayoutPtr & pLayout)
{
    mpLayout = pLayout;
}

#ifdef DEBUG_OOX_DIAGRAM
OString normalizeDotName( const OUString& rStr )
{
    OUStringBuffer aBuf;
    aBuf.append('N');

    const sal_Int32 nLen(rStr.getLength());
    sal_Int32 nCurrIndex(0);
    while( nCurrIndex < nLen )
    {
        const sal_Int32 aChar=rStr.iterateCodePoints(&nCurrIndex);
        if( aChar != '-' && aChar != '{' && aChar != '}' )
            aBuf.append((sal_Unicode)aChar);
    }

    return OUStringToOString(aBuf.makeStringAndClear(),
                                  RTL_TEXTENCODING_UTF8);
}
#endif

static sal_Int32 calcDepth( const OUString& rNodeName,
                            const dgm::Connections& rCnx )
{
    // find length of longest path in 'isChild' graph, ending with rNodeName
    dgm::Connections::const_iterator aCurrCxn( rCnx.begin() );
    const dgm::Connections::const_iterator aEndCxn( rCnx.end() );
    while( aCurrCxn != aEndCxn )
    {
        if( !aCurrCxn->msParTransId.isEmpty() &&
            !aCurrCxn->msSibTransId.isEmpty() &&
            !aCurrCxn->msSourceId.isEmpty() &&
            !aCurrCxn->msDestId.isEmpty() &&
            aCurrCxn->mnType != XML_presOf &&
            aCurrCxn->mnType != XML_presParOf &&
            rNodeName == aCurrCxn->msDestId )
        {
            return calcDepth(aCurrCxn->msSourceId,
                             rCnx) + 1;
        }
        ++aCurrCxn;
    }

    return 0;
}

void Diagram::build(  )
{
    // build name-object maps
#ifdef DEBUG_OOX_DIAGRAM
    std::ofstream output("/tmp/tree.dot");

    output << "digraph datatree {" << std::endl;
#endif
    dgm::Points& rPoints = getData()->getPoints();
    dgm::Points::iterator aCurrPoint(rPoints.begin());
    dgm::Points::iterator aEndPoint(rPoints.end());
    while( aCurrPoint != aEndPoint )
    {
#ifdef DEBUG_OOX_DIAGRAM
        output << "\t"
               << normalizeDotName(aCurrPoint->msModelId).getStr()
               << "[";

        if( !aCurrPoint->msPresentationLayoutName.isEmpty() )
            output << "label=\""
                   << OUStringToOString(
                       aCurrPoint->msPresentationLayoutName,
                       RTL_TEXTENCODING_UTF8).getStr() << "\", ";
        else
            output << "label=\""
                   << OUStringToOString(
                       aCurrPoint->msModelId,
                       RTL_TEXTENCODING_UTF8).getStr() << "\", ";

        switch( aCurrPoint->mnType )
        {
            case XML_doc: output << "style=filled, color=red"; break;
            case XML_asst: output << "style=filled, color=green"; break;
            default:
            case XML_node: output << "style=filled, color=blue"; break;
            case XML_pres: output << "style=filled, color=yellow"; break;
            case XML_parTrans: output << "color=grey"; break;
            case XML_sibTrans: output << " "; break;
        }

        output << "];" << std::endl;
#endif

        // does currpoint have any text set?
        if( aCurrPoint->mpShape &&
            aCurrPoint->mpShape->getTextBody() &&
            !aCurrPoint->mpShape->getTextBody()->getParagraphs().empty() &&
            !aCurrPoint->mpShape->getTextBody()->getParagraphs().front()->getRuns().empty() )
        {
#ifdef DEBUG_OOX_DIAGRAM
            static sal_Int32 nCount=0;
            output << "\t"
                   << "textNode" << nCount
                   << " ["
                   << "label=\""
                   << OUStringToOString(
                       aCurrPoint->mpShape->getTextBody()->getParagraphs().front()->getRuns().front()->getText(),
                       RTL_TEXTENCODING_UTF8).getStr()
                   << "\"" << "];" << std::endl;
            output << "\t"
                   << normalizeDotName(aCurrPoint->msModelId).getStr()
                   << " -> "
                   << "textNode" << nCount++
                   << ";" << std::endl;
#endif
        }

        const bool bInserted1=getData()->getPointNameMap().insert(
            std::make_pair(aCurrPoint->msModelId,&(*aCurrPoint))).second;

        OSL_ENSURE(bInserted1,"Diagram::build(): non-unique point model id");

        if( !aCurrPoint->msPresentationLayoutName.isEmpty() )
        {
            DiagramData::PointsNameMap::value_type::second_type& rVec=
                getData()->getPointsPresNameMap()[aCurrPoint->msPresentationLayoutName];
            rVec.push_back(&(*aCurrPoint));
        }
        ++aCurrPoint;
    }

    const dgm::Connections& rConnections = getData()->getConnections();
    dgm::Connections::const_iterator aCurrCxn(rConnections.begin());
    const dgm::Connections::const_iterator aEndCxn(rConnections.end());
    while( aCurrCxn != aEndCxn )
    {
#ifdef DEBUG_OOX_DIAGRAM
        if( !aCurrCxn->msParTransId.isEmpty() ||
            !aCurrCxn->msSibTransId.isEmpty() )
        {
            if( !aCurrCxn->msSourceId.isEmpty() ||
                !aCurrCxn->msDestId.isEmpty() )
            {
                output << "\t"
                       << normalizeDotName(aCurrCxn->msSourceId).getStr()
                       << " -> "
                       << normalizeDotName(aCurrCxn->msParTransId).getStr()
                       << " -> "
                       << normalizeDotName(aCurrCxn->msSibTransId).getStr()
                       << " -> "
                       << normalizeDotName(aCurrCxn->msDestId).getStr()
                       << " [style=dotted,"
                       << ((aCurrCxn->mnType == XML_presOf) ? " color=red, " : ((aCurrCxn->mnType == XML_presParOf) ? " color=green, " : " "))
                       << "label=\""
                       << OUStringToOString(aCurrCxn->msModelId,
                                                 RTL_TEXTENCODING_UTF8 ).getStr()
                       << "\"];" << std::endl;
            }
            else
            {
                output << "\t"
                       << normalizeDotName(aCurrCxn->msParTransId).getStr()
                       << " -> "
                       << normalizeDotName(aCurrCxn->msSibTransId).getStr()
                       << " ["
                       << ((aCurrCxn->mnType == XML_presOf) ? " color=red, " : ((aCurrCxn->mnType == XML_presParOf) ? " color=green, " : " "))
                       << "label=\""
                       << OUStringToOString(aCurrCxn->msModelId,
                                                 RTL_TEXTENCODING_UTF8 ).getStr()
                       << "\"];" << std::endl;
            }
        }
        else if( !aCurrCxn->msSourceId.isEmpty() ||
                 !aCurrCxn->msDestId.isEmpty() )
            output << "\t"
                   << normalizeDotName(aCurrCxn->msSourceId).getStr()
                   << " -> "
                   << normalizeDotName(aCurrCxn->msDestId).getStr()
                   << " [label=\""
                   << OUStringToOString(aCurrCxn->msModelId,
                                             RTL_TEXTENCODING_UTF8 ).getStr()
                   << ((aCurrCxn->mnType == XML_presOf) ? "\", color=red]" : ((aCurrCxn->mnType == XML_presParOf) ? "\", color=green]" : "\"]"))
                   << ";" << std::endl;
#endif

        const bool bInserted1=getData()->getConnectionNameMap().insert(
            std::make_pair(aCurrCxn->msModelId,&(*aCurrCxn))).second;

        OSL_ENSURE(bInserted1,"Diagram::build(): non-unique connection model id");

        if( aCurrCxn->mnType == XML_presOf )
        {
            DiagramData::StringMap::value_type::second_type& rVec=getData()->getPresOfNameMap()[aCurrCxn->msDestId];
            rVec.push_back(
                std::make_pair(
                    aCurrCxn->msSourceId,sal_Int32(0)));
        }

        ++aCurrCxn;
    }

    // assign outline levels
    DiagramData::StringMap& rStringMap = getData()->getPresOfNameMap();
    DiagramData::StringMap::iterator aPresOfIter=rStringMap.begin();
    const DiagramData::StringMap::iterator aPresOfEnd=rStringMap.end();
    while( aPresOfIter != aPresOfEnd )
    {
        DiagramData::StringMap::value_type::second_type::iterator aPresOfNodeIterCalcLevel=aPresOfIter->second.begin();
        const DiagramData::StringMap::value_type::second_type::iterator aPresOfNodeEnd=aPresOfIter->second.end();
        while(aPresOfNodeIterCalcLevel != aPresOfNodeEnd)
        {
            const sal_Int32 nDepth=calcDepth(aPresOfNodeIterCalcLevel->first,
                                             getData()->getConnections());
            aPresOfNodeIterCalcLevel->second = nDepth != 0 ? nDepth : -1;
            ++aPresOfNodeIterCalcLevel;
        }

        ++aPresOfIter;
    }
#ifdef DEBUG_OOX_DIAGRAM
    output << "}" << std::endl;
#endif
}

void Diagram::addTo( const ShapePtr & pParentShape )
{
    // collect data, init maps
    build( );

    if (pParentShape->getSize().Width == 0 || pParentShape->getSize().Height == 0)
        SAL_WARN("oox.drawingml", "Diagram cannot be correctly laid out. Size: "
            << pParentShape->getSize().Width << "x" << pParentShape->getSize().Height);

    pParentShape->setChildSize(pParentShape->getSize());

    if( mpLayout->getNode() )
    {
        // create Shape hierarchy
        ShapeCreationVisitor aCreationVisitor(pParentShape, *this);
        mpLayout->getNode()->setExistingShape(pParentShape);
        mpLayout->getNode()->accept( aCreationVisitor );
    }
    pParentShape->setDiagramDoms( getDomsAsPropertyValues() );
}

uno::Sequence<beans::PropertyValue> Diagram::getDomsAsPropertyValues() const
{
    sal_Int32 length = maMainDomMap.size();

    if ( 0 < maDataRelsMap.getLength() )
        ++length;

    uno::Sequence<beans::PropertyValue> aValue(length);
    beans::PropertyValue* pValue = aValue.getArray();
    for (DiagramDomMap::const_iterator i = maMainDomMap.begin();
         i != maMainDomMap.end();
         ++i)
    {
        pValue[0].Name = i->first;
        pValue[0].Value <<= i->second;
        ++pValue;
    }

    if ( 0 < maDataRelsMap.getLength() )
    {
        pValue[0].Name = "OOXDiagramDataRels";
        pValue[0].Value <<= maDataRelsMap;
        ++pValue;
    }

    return aValue;
}

uno::Reference<xml::dom::XDocument> loadFragment(
    core::XmlFilterBase& rFilter,
    const OUString& rFragmentPath )
{
    // load diagramming fragments into DOM representation, that later
    // gets serialized back to SAX events and parsed
    return rFilter.importFragment( rFragmentPath );
}

uno::Reference<xml::dom::XDocument> loadFragment(
    core::XmlFilterBase& rFilter,
    const rtl::Reference< core::FragmentHandler >& rxHandler )
{
    return loadFragment( rFilter, rxHandler->getFragmentPath() );
}

void importFragment( core::XmlFilterBase& rFilter,
                     const uno::Reference<xml::dom::XDocument>& rXDom,
                     const char* pDocName,
                     const DiagramPtr& pDiagram,
                     const rtl::Reference< core::FragmentHandler >& rxHandler )
{
    DiagramDomMap& rMainDomMap = pDiagram->getDomMap();
    rMainDomMap[OUString::createFromAscii(pDocName)] = rXDom;

    uno::Reference<xml::sax::XFastSAXSerializable> xSerializer(
        rXDom, uno::UNO_QUERY_THROW);

    // now serialize DOM tree into internal data structures
    rFilter.importFragment( rxHandler, xSerializer );
}

void loadDiagram( ShapePtr& pShape,
                  core::XmlFilterBase& rFilter,
                  const OUString& rDataModelPath,
                  const OUString& rLayoutPath,
                  const OUString& rQStylePath,
                  const OUString& rColorStylePath )
{
    DiagramPtr pDiagram( new Diagram );

    DiagramDataPtr pData( new DiagramData() );
    pDiagram->setData( pData );

    DiagramLayoutPtr pLayout( new DiagramLayout );
    pDiagram->setLayout( pLayout );

    // data
    if( !rDataModelPath.isEmpty() )
    {
        rtl::Reference< core::FragmentHandler > xRefDataModel(
                new DiagramDataFragmentHandler( rFilter, rDataModelPath, pData ));

        importFragment(rFilter,
                       loadFragment(rFilter,xRefDataModel),
                       "OOXData",
                       pDiagram,
                       xRefDataModel);

        pDiagram->getDataRelsMap() = pShape->resolveRelationshipsOfTypeFromOfficeDoc( rFilter,
                xRefDataModel->getFragmentPath(), "image" );

        // Pass the info to pShape
        for( ::std::vector<OUString>::const_iterator aIt = pData->getExtDrawings().begin(), aEnd = pData->getExtDrawings().end();
                aIt != aEnd; ++aIt )
                pShape->addExtDrawingRelId( *aIt );
    }

    // extLst is present, lets bet on that and ignore the rest of the data from here
    if( pData->getExtDrawings().empty() )
    {
        // layout
        if( !rLayoutPath.isEmpty() )
        {
            rtl::Reference< core::FragmentHandler > xRefLayout(
                    new DiagramLayoutFragmentHandler( rFilter, rLayoutPath, pLayout ));

            importFragment(rFilter,
                    loadFragment(rFilter,xRefLayout),
                    "OOXLayout",
                    pDiagram,
                    xRefLayout);
        }

        // style
        if( !rQStylePath.isEmpty() )
        {
            rtl::Reference< core::FragmentHandler > xRefQStyle(
                    new DiagramQStylesFragmentHandler( rFilter, rQStylePath, pDiagram->getStyles() ));

            importFragment(rFilter,
                    loadFragment(rFilter,xRefQStyle),
                    "OOXStyle",
                    pDiagram,
                    xRefQStyle);
        }
    } else {
        // We still want to add the XDocuments to the DiagramDomMap
        DiagramDomMap& rMainDomMap = pDiagram->getDomMap();
        rMainDomMap[OUString("OOXLayout")] = loadFragment(rFilter,rLayoutPath);
        rMainDomMap[OUString("OOXStyle")] = loadFragment(rFilter,rQStylePath);
    }

    // colors
    if( !rColorStylePath.isEmpty() )
    {
        rtl::Reference< core::FragmentHandler > xRefColorStyle(
            new ColorFragmentHandler( rFilter, rColorStylePath, pDiagram->getColors() ));

        importFragment(rFilter,
            loadFragment(rFilter,xRefColorStyle),
            "OOXColor",
            pDiagram,
            xRefColorStyle);
    }

    if( !pData->getExtDrawings().empty() )
    {
        const DiagramColorMap::const_iterator aColor = pDiagram->getColors().find("node0");
        if( aColor != pDiagram->getColors().end() )
        {
            pShape->setFontRefColorForNodes(aColor->second.maTextFillColor);
        }
    }

    // diagram loaded. now lump together & attach to shape
    pDiagram->addTo(pShape);
}

void loadDiagram( const ShapePtr& pShape,
                  core::XmlFilterBase& rFilter,
                  const uno::Reference<xml::dom::XDocument>& rXDataModelDom,
                  const uno::Reference<xml::dom::XDocument>& rXLayoutDom,
                  const uno::Reference<xml::dom::XDocument>& rXQStyleDom,
                  const uno::Reference<xml::dom::XDocument>& rXColorStyleDom )
{
    DiagramPtr pDiagram( new Diagram );

    DiagramDataPtr pData( new DiagramData() );
    pDiagram->setData( pData );

    DiagramLayoutPtr pLayout( new DiagramLayout );
    pDiagram->setLayout( pLayout );

    // data
    if( rXDataModelDom.is() )
        importFragment(rFilter,
                       rXDataModelDom,
                       "OOXData",
                       pDiagram,
                       new DiagramDataFragmentHandler( rFilter, "", pData ));

    // layout
    if( rXLayoutDom.is() )
        importFragment(rFilter,
                       rXLayoutDom,
                       "OOXLayout",
                       pDiagram,
                       new DiagramLayoutFragmentHandler( rFilter, "", pLayout ));

    // style
    if( rXQStyleDom.is() )
        importFragment(rFilter,
                       rXQStyleDom,
                       "OOXStyle",
                       pDiagram,
                       new DiagramQStylesFragmentHandler( rFilter, "", pDiagram->getStyles() ));

    // colors
    if( rXColorStyleDom.is() )
        importFragment(rFilter,
                       rXColorStyleDom,
                       "OOXColor",
                       pDiagram,
                       new ColorFragmentHandler( rFilter, "", pDiagram->getColors() ));

    // diagram loaded. now lump together & attach to shape
    pDiagram->addTo(pShape);
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
