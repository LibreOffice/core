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

#include "datamodel.hxx"
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <drawingml/fillproperties.hxx>
#include <drawingml/textbody.hxx>
#include <drawingml/textparagraph.hxx>
#include <drawingml/textrun.hxx>
#include <oox/drawingml/shape.hxx>
#include <comphelper/xmltools.hxx>

#include <unordered_set>
#include <fstream>

using namespace ::com::sun::star;

namespace oox::drawingml {

Shape* DiagramData::getOrCreateAssociatedShape(const svx::diagram::Point& rPoint, bool bCreateOnDemand) const
{
    if(maPointShapeMap.end() == maPointShapeMap.find(rPoint.msModelId))
    {
        const_cast<DiagramData*>(this)->maPointShapeMap[rPoint.msModelId] = ShapePtr();
    }

    const ShapePtr& rShapePtr = maPointShapeMap.find(rPoint.msModelId)->second;

    if(!rShapePtr && bCreateOnDemand)
    {
        const_cast<ShapePtr&>(rShapePtr) = std::make_shared<Shape>();

        // If we did create a new oox::drawingml::Shape, directly apply
        // available data from the Diagram ModelData to it as preparation
        restoreDataFromModelToShapeAfterReCreation(rPoint, *rShapePtr);
    }

    return rShapePtr.get();
}

void DiagramData::restoreDataFromModelToShapeAfterReCreation(const svx::diagram::Point& rPoint, Shape& rNewShape) const
{
    // If we did create a new oox::drawingml::Shape, directly apply
    // available data from the Diagram ModelData to it as preparation

    // This is e.g. the Text, but may get more (styles?)
    const auto pTextForShape = maPointTextMap.find(rPoint.msModelId);
    if (pTextForShape != maPointTextMap.end())
    {
        rNewShape.setTextBody(pTextForShape->second);
    }
}

void DiagramData::secureDataFromShapeToModelAfterDiagramImport()
{
    // After Diagram import, parts of the Diagram ModelData is at the
    // oox::drawingml::Shape. Since these objects are temporary helpers,
    // secure that data at the Diagram ModelData by copying.

    // This is currently mainly the Text, but may get more (styles?)
    for (auto & point : maPoints)
    {
        Shape* pShapeCandidate(getOrCreateAssociatedShape(point));

        if(nullptr != pShapeCandidate
            && pShapeCandidate->getTextBody()
            && !pShapeCandidate->getTextBody()->isEmpty())
        {
            maPointTextMap[point.msModelId] = pShapeCandidate->getTextBody();
        }
    }

    // At this place a mechanism to find missing data should be added:
    // Create a Shape from so-far secured data & compare it with the
    // imported one. Report differences to allow extending the mechanism
    // more easily.
}

DiagramData::DiagramData()
: svx::diagram::DiagramData()
, mpFillProperties( std::make_shared<FillProperties>() )
{
}

DiagramData::~DiagramData()
{
}

static void Connection_dump(const svx::diagram::Connection& rConnection)
{
    SAL_INFO(
        "oox.drawingml",
        "cnx modelId " << rConnection.msModelId << ", srcId " << rConnection.msSourceId << ", dstId "
            << rConnection.msDestId << ", parTransId " << rConnection.msParTransId << ", presId "
            << rConnection.msPresId << ", sibTransId " << rConnection.msSibTransId << ", srcOrd "
            << rConnection.mnSourceOrder << ", dstOrd " << rConnection.mnDestOrder);
}

static void Point_dump(const svx::diagram::Point& rPoint, const Shape* pShape)
{
    SAL_INFO(
        "oox.drawingml",
        "pt text " << pShape << ", cnxId " << rPoint.msCnxId << ", modelId "
            << rPoint.msModelId << ", type " << rPoint.mnXMLType);
}

void DiagramData::dump() const
{
    SAL_INFO("oox.drawingml", "Dgm: DiagramData # of cnx: " << maConnections.size() );
    for (const auto& rConnection : maConnections)
        Connection_dump(rConnection);

    SAL_INFO("oox.drawingml", "Dgm: DiagramData # of pt: " << maPoints.size() );
    for (const auto& rPoint : maPoints)
        Point_dump(rPoint, getOrCreateAssociatedShape(rPoint));
}

void DiagramData::getChildrenString(
    OUStringBuffer& rBuf,
    const svx::diagram::Point* pPoint,
    sal_Int32 nLevel) const
{
    if (!pPoint)
        return;

    Shape* pShape(getOrCreateAssociatedShape(*pPoint));
    if(!pShape)
        return;

    if (nLevel > 0)
    {
        for (sal_Int32 i = 0; i < nLevel-1; i++)
            rBuf.append('\t');
        rBuf.append('+');
        rBuf.append(' ');
        if(pShape->getTextBody())
            rBuf.append(pShape->getTextBody()->toString());
        rBuf.append('\n');
    }

    std::vector< const svx::diagram::Point* > aChildren;
    for (const auto& rCxn : maConnections)
        if (rCxn.mnXMLType == svx::diagram::TypeConstant::XML_parOf && rCxn.msSourceId == pPoint->msModelId)
        {
            if (rCxn.mnSourceOrder >= static_cast<sal_Int32>(aChildren.size()))
                aChildren.resize(rCxn.mnSourceOrder + 1);
            const auto pChild = maPointNameMap.find(rCxn.msDestId);
            if (pChild != maPointNameMap.end())
                aChildren[rCxn.mnSourceOrder] = pChild->second;
        }

    for (auto pChild : aChildren)
        getChildrenString(rBuf, pChild, nLevel + 1);
}

std::vector<std::pair<OUString, OUString>> DiagramData::getChildren(const OUString& rParentId) const
{
    const OUString sModelId = rParentId.isEmpty() ? getRootPoint()->msModelId : rParentId;
    std::vector<std::pair<OUString, OUString>> aChildren;
    for (const auto& rCxn : maConnections)
        if (rCxn.mnXMLType == svx::diagram::TypeConstant::XML_parOf && rCxn.msSourceId == sModelId)
        {
            if (rCxn.mnSourceOrder >= static_cast<sal_Int32>(aChildren.size()))
                aChildren.resize(rCxn.mnSourceOrder + 1);
            const auto pChild = maPointNameMap.find(rCxn.msDestId);
            if (pChild != maPointNameMap.end())
            {
                Shape* pShape(getOrCreateAssociatedShape(*(pChild->second)));
                aChildren[rCxn.mnSourceOrder] = std::make_pair(
                    pChild->second->msModelId,
                    nullptr != pShape && pShape->getTextBody() ? pShape->getTextBody()->toString() : OUString());
            }
        }

    // HACK: empty items shouldn't appear there
    aChildren.erase(std::remove_if(aChildren.begin(), aChildren.end(),
                                   [](const std::pair<OUString, OUString>& aItem) { return aItem.first.isEmpty(); }),
                    aChildren.end());

    return aChildren;
}

OUString DiagramData::addNode(const OUString& rText)
{
    const svx::diagram::Point& rDataRoot = *getRootPoint();
    OUString sPresRoot;
    for (const auto& aCxn : maConnections)
        if (aCxn.mnXMLType == svx::diagram::TypeConstant::XML_presOf && aCxn.msSourceId == rDataRoot.msModelId)
            sPresRoot = aCxn.msDestId;

    if (sPresRoot.isEmpty())
        return OUString();

    OUString sNewNodeId = OStringToOUString(comphelper::xml::generateGUIDString(), RTL_TEXTENCODING_UTF8);

    svx::diagram::Point aDataPoint;
    aDataPoint.mnXMLType = svx::diagram::TypeConstant::XML_node;
    aDataPoint.msModelId = sNewNodeId;

    Shape* pShape(getOrCreateAssociatedShape(aDataPoint, true));
    pShape->setTextBody(std::make_shared<TextBody>());
    TextRunPtr pTextRun = std::make_shared<TextRun>();
    pTextRun->getText() = rText;
    pShape->getTextBody()->addParagraph().addRun(pTextRun);

    OUString sDataSibling;
    for (const auto& aCxn : maConnections)
        if (aCxn.mnXMLType == svx::diagram::TypeConstant::XML_parOf && aCxn.msSourceId == rDataRoot.msModelId)
            sDataSibling = aCxn.msDestId;

    OUString sPresSibling;
    for (const auto& aCxn : maConnections)
        if (aCxn.mnXMLType == svx::diagram::TypeConstant::XML_presOf && aCxn.msSourceId == sDataSibling)
            sPresSibling = aCxn.msDestId;

    svx::diagram::Point aPresPoint;
    aPresPoint.mnXMLType = svx::diagram::TypeConstant::XML_pres;
    aPresPoint.msModelId = OStringToOUString(comphelper::xml::generateGUIDString(), RTL_TEXTENCODING_UTF8);

    // create pesPoint shape
    getOrCreateAssociatedShape(aPresPoint, true);

    aPresPoint.msPresentationAssociationId = aDataPoint.msModelId;
    if (!sPresSibling.isEmpty())
    {
        // no idea where to get these values from, so copy from previous sibling
        const svx::diagram::Point* pSiblingPoint = maPointNameMap[sPresSibling];
        aPresPoint.msPresentationLayoutName = pSiblingPoint->msPresentationLayoutName;
        aPresPoint.msPresentationLayoutStyleLabel = pSiblingPoint->msPresentationLayoutStyleLabel;
        aPresPoint.mnLayoutStyleIndex = pSiblingPoint->mnLayoutStyleIndex;
        aPresPoint.mnLayoutStyleCount = pSiblingPoint->mnLayoutStyleCount;
    }

    addConnection(svx::diagram::TypeConstant::XML_parOf, rDataRoot.msModelId, aDataPoint.msModelId);
    addConnection(svx::diagram::TypeConstant::XML_presParOf, sPresRoot, aPresPoint.msModelId);
    addConnection(svx::diagram::TypeConstant::XML_presOf, aDataPoint.msModelId, aPresPoint.msModelId);

    // adding at the end, so that references are not invalidated in between
    maPoints.push_back(aDataPoint);
    maPoints.push_back(aPresPoint);

    build(true);
    return sNewNodeId;
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

static sal_Int32 calcDepth( std::u16string_view rNodeName,
                            const svx::diagram::Connections& rCnx )
{
    // find length of longest path in 'isChild' graph, ending with rNodeName
    for (auto const& elem : rCnx)
    {
        if( !elem.msParTransId.isEmpty() &&
            !elem.msSibTransId.isEmpty() &&
            !elem.msSourceId.isEmpty() &&
            !elem.msDestId.isEmpty() &&
            elem.mnXMLType == svx::diagram::TypeConstant::XML_parOf &&
            rNodeName == elem.msDestId )
        {
            return calcDepth(elem.msSourceId, rCnx) + 1;
        }
    }

    return 0;
}

void DiagramData::build(bool bClearOoxShapes)
{
    // Delete/remove all existing oox::drawingml::Shape
    if(bClearOoxShapes)
    {
        maPointShapeMap.clear();
    }

    // build name-object maps
    maPointNameMap.clear();
    maPointsPresNameMap.clear();
    maConnectionNameMap.clear();
    maPresOfNameMap.clear();

#ifdef DEBUG_OOX_DIAGRAM
    std::ofstream output("tree.dot");

    output << "digraph datatree {" << std::endl;
#endif
    svx::diagram::Points& rPoints = getPoints();
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

        // Create/get shape. Re-create here, that may also set needed
        // and available data from the Diagram ModelData at the Shape
        Shape* pShape(getOrCreateAssociatedShape(point, true));

        // does currpoint have any text set?
        if( nullptr != pShape && pShape->getTextBody() && !pShape->getTextBody()->isEmpty() )
        {
#ifdef DEBUG_OOX_DIAGRAM
            static sal_Int32 nCount=0;
            output << "\t"
                   << "textNode" << nCount
                   << " ["
                   << "label=\""
                   << OUStringToOString(
                       pShape->getTextBody()->toString(),
                       RTL_TEXTENCODING_UTF8).getStr()
                   << "\"" << "];" << std::endl;
            output << "\t"
                   << normalizeDotName(point.msModelId).getStr()
                   << " -> "
                   << "textNode" << nCount++
                   << ";" << std::endl;
#endif
        }

        const bool bInserted1 = getPointNameMap().insert(
            std::make_pair(point.msModelId,&point)).second;

        SAL_WARN_IF(!bInserted1, "oox.drawingml", "DiagramData::build(): non-unique point model id");

        if( !point.msPresentationLayoutName.isEmpty() )
        {
            DiagramData::PointsNameMap::value_type::second_type& rVec=
                getPointsPresNameMap()[point.msPresentationLayoutName];
            rVec.push_back(&point);
        }
    }

    const svx::diagram::Connections& rConnections = getConnections();
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

        const bool bInserted1 = maConnectionNameMap.insert(
            std::make_pair(connection.msModelId,&connection)).second;

        SAL_WARN_IF(!bInserted1, "oox.drawingml", "DiagramData::build(): non-unique connection model id");

        if( connection.mnXMLType == svx::diagram::TypeConstant::XML_presOf )
        {
            DiagramData::StringMap::value_type::second_type& rVec = getPresOfNameMap()[connection.msDestId];
            rVec[connection.mnDestOrder] = { connection.msSourceId, sal_Int32(0) };
        }
    }

    // assign outline levels
    DiagramData::StringMap& rStringMap = getPresOfNameMap();
    for (auto & elemPresOf : rStringMap)
    {
        for (auto & elem : elemPresOf.second)
        {
            const sal_Int32 nDepth = calcDepth(elem.second.msSourceId, getConnections());
            elem.second.mnDepth = nDepth != 0 ? nDepth : -1;
        }
    }
#ifdef DEBUG_OOX_DIAGRAM
    output << "}" << std::endl;
#endif
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
