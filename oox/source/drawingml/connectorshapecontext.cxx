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

#include <com/sun/star/xml/sax/FastToken.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>

#include <oox/drawingml/connectorshapecontext.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <oox/drawingml/shape.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

using namespace oox::core;
using namespace ::com::sun::star;

namespace oox::drawingml {

ConnectorShapePropertiesContext::ConnectorShapePropertiesContext(
    ContextHandler2Helper const& rParent, ShapePtr& pShapePtr,
    std::vector<ConnectorShapeProperties>& rConnectorShapePropertiesList)
    : ContextHandler2(rParent)
    , mrConnectorShapePropertiesList(rConnectorShapePropertiesList)
    , mpConnectorShapePtr(pShapePtr)
{
    mpConnectorShapePtr->setConnectorShape(true);
}

::oox::core::ContextHandlerRef
ConnectorShapePropertiesContext::onCreateContext(sal_Int32 aElementToken,
                                                const AttributeList& rAttribs)
{
    switch (getBaseToken(aElementToken))
    {
        case XML_extLst:
        case XML_ext:
            break;
        case XML_decorative:
        {
            mpConnectorShapePtr->setDecorative(rAttribs.getBool(XML_val, false));
            break;
        }
        case XML_cNvPr:
            mpConnectorShapePtr->setId(rAttribs.getStringDefaulted(XML_id));
            mpConnectorShapePtr->setName(rAttribs.getStringDefaulted(XML_name));
            break;
        case XML_stCxn:
        {
            ConnectorShapeProperties aConnectorShapeProps;
            aConnectorShapeProps.mbStartShape = true;
            aConnectorShapeProps.maDestShapeId = rAttribs.getStringDefaulted(XML_id);
            aConnectorShapeProps.mnDestGlueId = rAttribs.getInteger(XML_idx, 0);
            mrConnectorShapePropertiesList.push_back(aConnectorShapeProps);
            break;
        }
        case XML_endCxn:
        {
            ConnectorShapeProperties aConnectorShapeProps;
            aConnectorShapeProps.mbStartShape = false;
            aConnectorShapeProps.maDestShapeId = rAttribs.getStringDefaulted(XML_id);
            aConnectorShapeProps.mnDestGlueId = rAttribs.getInteger(XML_idx, 0);
            mrConnectorShapePropertiesList.push_back(aConnectorShapeProps);
            break;
        }
        default:
            break;
    }

    return this;
}

ConnectorShapeContext::ConnectorShapeContext(
    ContextHandler2Helper const& rParent, const ShapePtr& pMasterShapePtr,
    const ShapePtr& pGroupShapePtr,
    std::vector<ConnectorShapeProperties>& rConnectorShapePropertiesList)
    : ShapeContext(rParent, pMasterShapePtr, pGroupShapePtr)
    , mrConnectorShapePropertiesList(rConnectorShapePropertiesList)
    , mpConnectorShapePtr(pGroupShapePtr)
{
}

ConnectorShapeContext::~ConnectorShapeContext() {}

ContextHandlerRef ConnectorShapeContext::onCreateContext(sal_Int32 aElementToken,
                                                         const AttributeList& rAttribs)
{
    switch (getBaseToken(aElementToken))
    {
        case XML_nvCxnSpPr:
            return new ConnectorShapePropertiesContext(*this, mpConnectorShapePtr,
                                                      mrConnectorShapePropertiesList);
    }

    return ShapeContext::onCreateContext(aElementToken, rAttribs);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
