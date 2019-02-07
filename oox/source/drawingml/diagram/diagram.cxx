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

#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/sax/XFastSAXSerializable.hpp>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <editeng/unoprnms.hxx>
#include <drawingml/textbody.hxx>
#include <drawingml/textparagraph.hxx>
#include <drawingml/textrun.hxx>
#include <drawingml/diagram/diagram.hxx>
#include <drawingml/fillproperties.hxx>
#include <oox/ppt/pptshapegroupcontext.hxx>
#include <oox/ppt/pptshape.hxx>
#include <oox/token/namespaces.hxx>

#include "diagramlayoutatoms.hxx"
#include "layoutatomvisitors.hxx"
#include "diagramfragmenthandler.hxx"

#include <iostream>
#include <fstream>

using namespace ::com::sun::star;

namespace oox { namespace drawingml {

namespace dgm {

void Connection::dump() const
{
    SAL_INFO(
        "oox.drawingml",
        "cnx modelId " << msModelId << ", srcId " << msSourceId << ", dstId "
            << msDestId << ", parTransId " << msParTransId << ", presId "
            << msPresId << ", sibTransId " << msSibTransId << ", srcOrd "
            << mnSourceOrder << ", dstOrd " << mnDestOrder);
}

void Point::dump() const
{
    SAL_INFO(
        "oox.drawingml",
        "pt text " << mpShape.get() << ", cnxId " << msCnxId << ", modelId "
            << msModelId << ", type " << mnType);
}

} // dgm namespace

DiagramData::DiagramData() :
    mpFillProperties( new FillProperties ),
    mnMaxDepth(0)
{
}

const dgm::Point* DiagramData::getRootPoint() const
{
    for (const auto & aCurrPoint : maPoints)
        if (aCurrPoint.mnType == XML_doc)
            return &aCurrPoint;

    SAL_WARN("oox.drawingml", "No root point");
    return nullptr;
}

void DiagramData::dump() const
{
    SAL_INFO("oox.drawingml", "Dgm: DiagramData # of cnx: " << maConnections.size() );
    for (const auto& rConnection : maConnections)
        rConnection.dump();

    SAL_INFO("oox.drawingml", "Dgm: DiagramData # of pt: " << maPoints.size() );
    for (const auto& rPoint : maPoints)
        rPoint.dump();
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
    for (auto const& elem : rCnx)
    {
        if( !elem.msParTransId.isEmpty() &&
            !elem.msSibTransId.isEmpty() &&
            !elem.msSourceId.isEmpty() &&
            !elem.msDestId.isEmpty() &&
            elem.mnType == XML_parOf &&
            rNodeName == elem.msDestId )
        {
            return calcDepth(elem.msSourceId, rCnx) + 1;
        }
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
    for (auto & point : rPoints)
    {
#ifdef DEBUG_OOX_DIAGRAM
        output << "\t"
               << normalizeDotName(point.msModelId).getStr()
               << "[";

        if( !point.msPresentationLayoutName.isEmpty() )
            output << "label=\""
                   << OUStringToOString(
                       point.msPresentationLayoutName,
                       RTL_TEXTENCODING_UTF8).getStr() << "\", ";
        else
            output << "label=\""
                   << OUStringToOString(
                       point.msModelId,
                       RTL_TEXTENCODING_UTF8).getStr() << "\", ";

        switch( point.mnType )
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
        if( point.mpShape &&
            point.mpShape->getTextBody() &&
            !point.mpShape->getTextBody()->getParagraphs().empty() &&
            !point.mpShape->getTextBody()->getParagraphs().front()->getRuns().empty() )
        {
#ifdef DEBUG_OOX_DIAGRAM
            static sal_Int32 nCount=0;
            output << "\t"
                   << "textNode" << nCount
                   << " ["
                   << "label=\""
                   << OUStringToOString(
                       point.mpShape->getTextBody()->getParagraphs().front()->getRuns().front()->getText(),
                       RTL_TEXTENCODING_UTF8).getStr()
                   << "\"" << "];" << std::endl;
            output << "\t"
                   << normalizeDotName(point.msModelId).getStr()
                   << " -> "
                   << "textNode" << nCount++
                   << ";" << std::endl;
#endif
        }

        const bool bInserted1=getData()->getPointNameMap().insert(
            std::make_pair(point.msModelId,&point)).second;

        SAL_WARN_IF(!bInserted1, "oox.drawingml", "Diagram::build(): non-unique point model id");

        if( !point.msPresentationLayoutName.isEmpty() )
        {
            DiagramData::PointsNameMap::value_type::second_type& rVec=
                getData()->getPointsPresNameMap()[point.msPresentationLayoutName];
            rVec.push_back(&point);
        }
    }

    const dgm::Connections& rConnections = getData()->getConnections();
    for (auto const& connection : rConnections)
    {
#ifdef DEBUG_OOX_DIAGRAM
        if( !connection.msParTransId.isEmpty() ||
            !connection.msSibTransId.isEmpty() )
        {
            if( !connection.msSourceId.isEmpty() ||
                !connection.msDestId.isEmpty() )
            {
                output << "\t"
                       << normalizeDotName(connection.msSourceId).getStr()
                       << " -> "
                       << normalizeDotName(connection.msParTransId).getStr()
                       << " -> "
                       << normalizeDotName(connection.msSibTransId).getStr()
                       << " -> "
                       << normalizeDotName(connection.msDestId).getStr()
                       << " [style=dotted,"
                       << ((connection.mnType == XML_presOf) ? " color=red, " : ((connection.mnType == XML_presParOf) ? " color=green, " : " "))
                       << "label=\""
                       << OUStringToOString(connection.msModelId,
                                                 RTL_TEXTENCODING_UTF8 ).getStr()
                       << "\"];" << std::endl;
            }
            else
            {
                output << "\t"
                       << normalizeDotName(connection.msParTransId).getStr()
                       << " -> "
                       << normalizeDotName(connection.msSibTransId).getStr()
                       << " ["
                       << ((connection.mnType == XML_presOf) ? " color=red, " : ((connection.mnType == XML_presParOf) ? " color=green, " : " "))
                       << "label=\""
                       << OUStringToOString(connection.msModelId,
                                                 RTL_TEXTENCODING_UTF8 ).getStr()
                       << "\"];" << std::endl;
            }
        }
        else if( !connection.msSourceId.isEmpty() ||
                 !connection.msDestId.isEmpty() )
            output << "\t"
                   << normalizeDotName(connection.msSourceId).getStr()
                   << " -> "
                   << normalizeDotName(connection.msDestId).getStr()
                   << " [label=\""
                   << OUStringToOString(connection.msModelId,
                                             RTL_TEXTENCODING_UTF8 ).getStr()
                   << ((connection.mnType == XML_presOf) ? "\", color=red]" : ((connection.mnType == XML_presParOf) ? "\", color=green]" : "\"]"))
                   << ";" << std::endl;
#endif

        const bool bInserted1=getData()->getConnectionNameMap().insert(
            std::make_pair(connection.msModelId,&connection)).second;

        SAL_WARN_IF(!bInserted1, "oox.drawingml", "Diagram::build(): non-unique connection model id");

        if( connection.mnType == XML_presOf )
        {
            DiagramData::StringMap::value_type::second_type& rVec=getData()->getPresOfNameMap()[connection.msDestId];
            rVec[connection.mnDestOrder] = { connection.msSourceId, sal_Int32(0) };
        }
    }

    // assign outline levels
    DiagramData::StringMap& rStringMap = getData()->getPresOfNameMap();
    for (auto & elemPresOf : rStringMap)
    {
        for (auto & elem : elemPresOf.second)
        {
            const sal_Int32 nDepth = calcDepth(elem.second.msSourceId, getData()->getConnections());
            elem.second.mnDepth = nDepth != 0 ? nDepth : -1;
            if (nDepth > getData()->getMaxDepth())
                getData()->setMaxDepth(nDepth);
        }
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
        mpLayout->getNode()->accept(aCreationVisitor);

        // layout shapes - now all shapes are created
        ShapeLayoutingVisitor aLayoutingVisitor;
        mpLayout->getNode()->accept(aLayoutingVisitor);
    }
    pParentShape->setDiagramDoms( getDomsAsPropertyValues() );
}

uno::Sequence<beans::PropertyValue> Diagram::getDomsAsPropertyValues() const
{
    sal_Int32 length = maMainDomMap.size();

    if (maDataRelsMap.hasElements())
        ++length;

    uno::Sequence<beans::PropertyValue> aValue(length);
    beans::PropertyValue* pValue = aValue.getArray();
    for (auto const& mainDom : maMainDomMap)
    {
        pValue->Name = mainDom.first;
        pValue->Value <<= mainDom.second;
        ++pValue;
    }

    if (maDataRelsMap.hasElements())
    {
        pValue->Name = "OOXDiagramDataRels";
        pValue->Value <<= maDataRelsMap;
        ++pValue;
    }

    return aValue;
}

static uno::Reference<xml::dom::XDocument> loadFragment(
    core::XmlFilterBase& rFilter,
    const OUString& rFragmentPath )
{
    // load diagramming fragments into DOM representation, that later
    // gets serialized back to SAX events and parsed
    return rFilter.importFragment( rFragmentPath );
}

static uno::Reference<xml::dom::XDocument> loadFragment(
    core::XmlFilterBase& rFilter,
    const rtl::Reference< core::FragmentHandler >& rxHandler )
{
    return loadFragment( rFilter, rxHandler->getFragmentPath() );
}

static void importFragment( core::XmlFilterBase& rFilter,
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

namespace
{
/**
 * A fragment handler that just counts the number of <dsp:sp> elements in a
 * fragment.
 */
class DiagramShapeCounter : public oox::core::FragmentHandler2
{
public:
    DiagramShapeCounter(oox::core::XmlFilterBase& rFilter, const OUString& rFragmentPath,
                        sal_Int32& nCounter);
    oox::core::ContextHandlerRef onCreateContext(sal_Int32 nElement,
                                                 const AttributeList& rAttribs) override;

private:
    sal_Int32& m_nCounter;
};

DiagramShapeCounter::DiagramShapeCounter(oox::core::XmlFilterBase& rFilter,
                                         const OUString& rFragmentPath, sal_Int32& nCounter)
    : FragmentHandler2(rFilter, rFragmentPath)
    , m_nCounter(nCounter)
{
}

oox::core::ContextHandlerRef DiagramShapeCounter::onCreateContext(sal_Int32 nElement,
                                                                  const AttributeList& /*rAttribs*/)
{
    switch (nElement)
    {
        case DSP_TOKEN(drawing):
        case DSP_TOKEN(spTree):
            return this;
        case DSP_TOKEN(sp):
            ++m_nCounter;
            break;
        default:
            break;
    }

    return nullptr;
}
}

void loadDiagram( ShapePtr const & pShape,
                  core::XmlFilterBase& rFilter,
                  const OUString& rDataModelPath,
                  const OUString& rLayoutPath,
                  const OUString& rQStylePath,
                  const OUString& rColorStylePath,
                  const oox::core::Relations& rRelations )
{
    DiagramPtr pDiagram( new Diagram );

    DiagramDataPtr pData( new DiagramData() );
    pDiagram->setData( pData );

    DiagramLayoutPtr pLayout( new DiagramLayout(*pDiagram) );
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
        for (auto const& extDrawing : pData->getExtDrawings())
        {
            OUString aFragmentPath = rRelations.getFragmentPathFromRelId(extDrawing);
            // Ignore RelIds which don't resolve to a fragment path.
            if (aFragmentPath.isEmpty())
                continue;

            sal_Int32 nCounter = 0;
            rtl::Reference<core::FragmentHandler> xCounter(
                new DiagramShapeCounter(rFilter, aFragmentPath, nCounter));
            rFilter.importFragment(xCounter);
            // Ignore ext drawings which don't actually have any shapes.
            if (nCounter == 0)
                continue;

            pShape->addExtDrawingRelId(extDrawing);
        }
    }

    // extLst is present, lets bet on that and ignore the rest of the data from here
    if( pShape->getExtDrawings().empty() )
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
    }
    else
    {
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

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
