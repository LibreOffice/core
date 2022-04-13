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

#include <unordered_set>
#include <algorithm>
#include <fstream>

#include <svx/diagram/datamodel.hxx>
#include <comphelper/xmltools.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/text/XText.hpp>

#include <editeng/unoprnms.hxx>
#include <sal/log.hxx>

namespace svx::diagram {

Connection::Connection()
: mnXMLType( XML_none )
, mnSourceOrder( 0 )
, mnDestOrder( 0 )
{
}

Point::Point()
: mnXMLType(XML_none)
, mnMaxChildren(-1)
, mnPreferredChildren(-1)
, mnDirection(XML_norm)
, mnResizeHandles(XML_rel)
, mnCustomAngle(-1)
, mnPercentageNeighbourWidth(-1)
, mnPercentageNeighbourHeight(-1)
, mnPercentageOwnWidth(-1)
, mnPercentageOwnHeight(-1)
, mnIncludeAngleScale(-1)
, mnRadiusScale(-1)
, mnWidthScale(-1)
, mnHeightScale(-1)
, mnWidthOverride(-1)
, mnHeightOverride(-1)
, mnLayoutStyleCount(-1)
, mnLayoutStyleIndex(-1)
, mbOrgChartEnabled(false)
, mbBulletEnabled(false)
, mbCoherent3DOffset(false)
, mbCustomHorizontalFlip(false)
, mbCustomVerticalFlip(false)
, mbCustomText(false)
, mbIsPlaceholder(false)
{
}

static void addProperty(const OUString& rName,
    const css::uno::Reference< css::beans::XPropertySetInfo >& xInfo,
    std::vector< std::pair< OUString, css::uno::Any >>& rTarget,
    const css::uno::Reference< css::beans::XPropertySet >& xPropSet )
{
    if(xInfo->hasPropertyByName(rName))
            rTarget.push_back(std::pair(OUString(rName), xPropSet->getPropertyValue(rName)));
}

void Point::securePropertiesFromXShape(const css::uno::Reference< css::drawing::XShape >& rXShape)
{
    if(!rXShape)
        return;

#ifdef DBG_UTIL
    // to easier decide which additional properties may/should be preserved,
    // create a full list of set properties to browse/decide (in debugger)
    const css::uno::Reference< css::beans::XPropertyState > xAllPropStates(rXShape, css::uno::UNO_QUERY);
    const css::uno::Reference< css::beans::XPropertySet > xAllPropSet( rXShape, css::uno::UNO_QUERY );
    const css::uno::Sequence< css::beans::Property > allSequence(xAllPropSet->getPropertySetInfo()->getProperties());
    std::vector< std::pair< OUString, css::uno::Any >> allSetProps;
    for (auto& rProp : allSequence)
    {
        try
        {
            if (xAllPropStates->getPropertyState(rProp.Name) == css::beans::PropertyState::PropertyState_DIRECT_VALUE)
            {
                css::uno::Any aValue(xAllPropSet->getPropertyValue(rProp.Name));
                if(aValue.hasValue())
                    allSetProps.push_back(std::pair(rProp.Name, aValue));
            }
        }
        catch (...)
        {
        }
    }
#endif

    const css::uno::Reference< css::beans::XPropertySet > xPropSet( rXShape, css::uno::UNO_QUERY );
    if(!xPropSet)
        return;

    const css::uno::Reference< css::lang::XServiceInfo > xServiceInfo( rXShape, css::uno::UNO_QUERY );
    if(!xServiceInfo)
        return;

    const css::uno::Reference< css::beans::XPropertySetInfo > xInfo(xPropSet->getPropertySetInfo());
    if (!xInfo.is())
        return;

    if(!msPointStylePtr)
        msPointStylePtr = std::make_shared< svx::diagram::PointStyle >();

    // shortcut to target
    std::vector< std::pair< OUString, css::uno::Any >>& rTarget(msPointStylePtr->maProperties);

    // Note: The Text may also be secured here, so it may also be possible to
    // secure/store it at PointStyle instead of at TextBody, same maybe evaluated
    // for the text attributes - where when securing here the attributes would be
    // in our UNO API format already.
    // if(xServiceInfo->supportsService("com.sun.star.drawing.Text"))
    // {
    //     css::uno::Reference< css::text::XText > xText(rXShape, css::uno::UNO_QUERY);
    //     const OUString aText(xText->getString());
    //
    //     if(!aText.isEmpty())
    //     {
    //     }
    // }

    // Add all kinds of properties that are needed to re-create the XShape.
    // For now this is a minimal example-selection, it will need to be extended
    // over time for all kind of cases/properties

    // text properties
    if(xServiceInfo->supportsService("com.sun.star.drawing.TextProperties"))
    {
        addProperty(UNO_NAME_CHAR_COLOR, xInfo, rTarget, xPropSet);
        addProperty(UNO_NAME_CHAR_HEIGHT, xInfo, rTarget, xPropSet);
        addProperty(UNO_NAME_CHAR_SHADOWED, xInfo, rTarget, xPropSet);
        addProperty(UNO_NAME_CHAR_WEIGHT, xInfo, rTarget, xPropSet);
    }

    // fill properties
    if(xServiceInfo->supportsService("com.sun.star.drawing.FillProperties"))
    {
        css::drawing::FillStyle eFillStyle(css::drawing::FillStyle_NONE);
        if (xInfo->hasPropertyByName(UNO_NAME_FILLSTYLE))
            xPropSet->getPropertyValue(UNO_NAME_FILLSTYLE) >>= eFillStyle;

        if(css::drawing::FillStyle_NONE != eFillStyle)
        {
            addProperty(UNO_NAME_FILLSTYLE, xInfo, rTarget, xPropSet);

            switch(eFillStyle)
            {
                case css::drawing::FillStyle_SOLID:
                {
                    addProperty(UNO_NAME_FILLCOLOR, xInfo, rTarget, xPropSet);
                    break;
                }
                default:
                case css::drawing::FillStyle_NONE:
                case css::drawing::FillStyle_GRADIENT:
                case css::drawing::FillStyle_HATCH:
                case css::drawing::FillStyle_BITMAP:
                    break;
            }
        }
    }

    // line properties
    if(xServiceInfo->supportsService("com.sun.star.drawing.LineProperties"))
    {
        css::drawing::LineStyle eLineStyle(css::drawing::LineStyle_NONE);
        if (xInfo->hasPropertyByName(UNO_NAME_LINESTYLE))
            xPropSet->getPropertyValue(UNO_NAME_LINESTYLE) >>= eLineStyle;

        if(css::drawing::LineStyle_NONE != eLineStyle)
        {
            addProperty(UNO_NAME_LINESTYLE, xInfo, rTarget, xPropSet);
            addProperty(UNO_NAME_LINECOLOR, xInfo, rTarget, xPropSet);
            addProperty(UNO_NAME_LINEWIDTH, xInfo, rTarget, xPropSet);

            switch(eLineStyle)
            {
                case css::drawing::LineStyle_SOLID:
                    break;
                default:
                case css::drawing::LineStyle_NONE:
                case css::drawing::LineStyle_DASH:
                    break;
            }
        }
    }
}

void Point::restorePropertiesToXShape(const css::uno::Reference< css::drawing::XShape >& rXShape) const
{
    if(!msPointStylePtr)
        return;

    if(!rXShape)
        return;

    css::uno::Reference<css::beans::XPropertySet> xPropSet(rXShape, css::uno::UNO_QUERY);
    if(!xPropSet)
        return;

    std::vector< std::pair< OUString, css::uno::Any >>& rSource(msPointStylePtr->maProperties);

    for (auto const& prop : rSource)
    {
        xPropSet->setPropertyValue(prop.first, prop.second);
    }
}

DiagramData::DiagramData()
{
}

DiagramData::~DiagramData()
{
}

const Point* DiagramData::getRootPoint() const
{
    for (const auto & aCurrPoint : maPoints)
        if (aCurrPoint.mnXMLType == TypeConstant::XML_doc)
            return &aCurrPoint;

    SAL_WARN("svx.diagram", "No root point");
    return nullptr;
}

OUString DiagramData::getString() const
{
    OUStringBuffer aBuf;
    const Point* pPoint = getRootPoint();
    getChildrenString(aBuf, pPoint, 0);
    return aBuf.makeStringAndClear();
}

bool DiagramData::removeNode(const OUString& rNodeId)
{
    // check if it doesn't have children
    for (const auto& aCxn : maConnections)
        if (aCxn.mnXMLType == TypeConstant::XML_parOf && aCxn.msSourceId == rNodeId)
        {
            SAL_WARN("svx.diagram", "Node has children - can't be removed");
            return false;
        }

    Connection aParCxn;
    for (const auto& aCxn : maConnections)
        if (aCxn.mnXMLType == TypeConstant::XML_parOf && aCxn.msDestId == rNodeId)
            aParCxn = aCxn;

    std::unordered_set<OUString> aIdsToRemove;
    aIdsToRemove.insert(rNodeId);
    if (!aParCxn.msParTransId.isEmpty())
        aIdsToRemove.insert(aParCxn.msParTransId);
    if (!aParCxn.msSibTransId.isEmpty())
        aIdsToRemove.insert(aParCxn.msSibTransId);

    for (const Point& rPoint : maPoints)
        if (aIdsToRemove.count(rPoint.msPresentationAssociationId))
            aIdsToRemove.insert(rPoint.msModelId);

    // insert also transition nodes
    for (const auto& aCxn : maConnections)
        if (aIdsToRemove.count(aCxn.msSourceId) || aIdsToRemove.count(aCxn.msDestId))
            if (!aCxn.msPresId.isEmpty())
                aIdsToRemove.insert(aCxn.msPresId);

    // remove connections
    maConnections.erase(std::remove_if(maConnections.begin(), maConnections.end(),
                                       [aIdsToRemove](const Connection& rCxn) {
                                           return aIdsToRemove.count(rCxn.msSourceId) || aIdsToRemove.count(rCxn.msDestId);
                                       }),
                        maConnections.end());

    // remove data and presentation nodes
    maPoints.erase(std::remove_if(maPoints.begin(), maPoints.end(),
                                  [aIdsToRemove](const Point& rPoint) {
                                      return aIdsToRemove.count(rPoint.msModelId);
                                  }),
                   maPoints.end());

    // TODO: fix source/dest order
    return true;
}

void DiagramData::getChildrenString(
    OUStringBuffer& rBuf,
    const svx::diagram::Point* pPoint,
    sal_Int32 nLevel) const
{
    if (!pPoint)
        return;

    if (nLevel > 0)
    {
        for (sal_Int32 i = 0; i < nLevel-1; i++)
            rBuf.append('\t');
        rBuf.append('+');
        rBuf.append(' ');
        if(pPoint->msTextBody)
            rBuf.append(pPoint->msTextBody->msText);
        rBuf.append('\n');
    }

    std::vector< const svx::diagram::Point* > aChildren;
    for (const auto& rCxn : maConnections)
        if (rCxn.mnXMLType == TypeConstant::XML_parOf && rCxn.msSourceId == pPoint->msModelId)
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
        if (rCxn.mnXMLType == TypeConstant::XML_parOf && rCxn.msSourceId == sModelId)
        {
            if (rCxn.mnSourceOrder >= static_cast<sal_Int32>(aChildren.size()))
                aChildren.resize(rCxn.mnSourceOrder + 1);
            const auto pChild = maPointNameMap.find(rCxn.msDestId);
            if (pChild != maPointNameMap.end())
            {
                aChildren[rCxn.mnSourceOrder] = std::make_pair(
                    pChild->second->msModelId,
                    pChild->second->msTextBody ? pChild->second->msTextBody->msText : OUString());
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
        if (aCxn.mnXMLType == TypeConstant::XML_presOf && aCxn.msSourceId == rDataRoot.msModelId)
            sPresRoot = aCxn.msDestId;

    if (sPresRoot.isEmpty())
        return OUString();

    OUString sNewNodeId = OStringToOUString(comphelper::xml::generateGUIDString(), RTL_TEXTENCODING_UTF8);

    svx::diagram::Point aDataPoint;
    aDataPoint.mnXMLType = TypeConstant::XML_node;
    aDataPoint.msModelId = sNewNodeId;
    aDataPoint.msTextBody = std::make_shared<TextBody>();
    aDataPoint.msTextBody->msText = rText;

    OUString sDataSibling;
    for (const auto& aCxn : maConnections)
        if (aCxn.mnXMLType == TypeConstant::XML_parOf && aCxn.msSourceId == rDataRoot.msModelId)
            sDataSibling = aCxn.msDestId;

    OUString sPresSibling;
    for (const auto& aCxn : maConnections)
        if (aCxn.mnXMLType == TypeConstant::XML_presOf && aCxn.msSourceId == sDataSibling)
            sPresSibling = aCxn.msDestId;

    svx::diagram::Point aPresPoint;
    aPresPoint.mnXMLType = TypeConstant::XML_pres;
    aPresPoint.msModelId = OStringToOUString(comphelper::xml::generateGUIDString(), RTL_TEXTENCODING_UTF8);

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

    return sNewNodeId;
}

void DiagramData::addConnection(svx::diagram::TypeConstant nType, const OUString& sSourceId, const OUString& sDestId)
{
    sal_Int32 nMaxOrd = -1;
    for (const auto& aCxn : maConnections)
        if (aCxn.mnXMLType == nType && aCxn.msSourceId == sSourceId)
            nMaxOrd = std::max(nMaxOrd, aCxn.mnSourceOrder);

    svx::diagram::Connection& rCxn = maConnections.emplace_back();
    rCxn.mnXMLType = nType;
    rCxn.msSourceId = sSourceId;
    rCxn.msDestId = sDestId;
    rCxn.mnSourceOrder = nMaxOrd + 1;
}

// #define DEBUG_OOX_DIAGRAM
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
            elem.mnXMLType == TypeConstant::XML_parOf &&
            rNodeName == elem.msDestId )
        {
            return calcDepth(elem.msSourceId, rCnx) + 1;
        }
    }

    return 0;
}

void DiagramData::buildDiagramDataModel(bool /*bClearOoxShapes*/)
{
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

        switch( point.mnXMLType )
        {
            case TypeConstant::XML_doc: output << "style=filled, color=red"; break;
            case TypeConstant::XML_asst: output << "style=filled, color=green"; break;
            default:
            case TypeConstant::XML_node: output << "style=filled, color=blue"; break;
            case TypeConstant::XML_pres: output << "style=filled, color=yellow"; break;
            case TypeConstant::XML_parTrans: output << "color=grey"; break;
            case TypeConstant::XML_sibTrans: output << " "; break;
        }

        output << "];" << std::endl;
#endif

        // does currpoint have any text set?
        if(point.msTextBody && !point.msTextBody->msText.isEmpty())
        {
#ifdef DEBUG_OOX_DIAGRAM
            static sal_Int32 nCount=0;
            output << "\t"
                   << "textNode" << nCount
                   << " ["
                   << "label=\""
                   << OUStringToOString(
                       point.msTextBody->msText,
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
                       << ((connection.mnXMLType == TypeConstant::XML_presOf) ? " color=red, " : ((connection.mnXMLType == TypeConstant::XML_presParOf) ? " color=green, " : " "))
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
                       << ((connection.mnXMLType == TypeConstant::XML_presOf) ? " color=red, " : ((connection.mnXMLType == TypeConstant::XML_presParOf) ? " color=green, " : " "))
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
                   << ((connection.mnXMLType == TypeConstant::XML_presOf) ? "\", color=red]" : ((connection.mnXMLType == TypeConstant::XML_presParOf) ? "\", color=green]" : "\"]"))
                   << ";" << std::endl;
#endif

        const bool bInserted1 = maConnectionNameMap.insert(
            std::make_pair(connection.msModelId,&connection)).second;

        SAL_WARN_IF(!bInserted1, "oox.drawingml", "DiagramData::build(): non-unique connection model id");

        if( connection.mnXMLType == TypeConstant::XML_presOf )
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
