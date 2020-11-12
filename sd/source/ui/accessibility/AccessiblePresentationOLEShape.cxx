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

#include <AccessiblePresentationOLEShape.hxx>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/drawing/XShapeDescriptor.hpp>
#include <com/sun/star/drawing/XShape.hpp>

#include <SdShapeTypes.hxx>

#include <svx/ShapeTypeHandler.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

namespace accessibility
{
//=====  internal  ============================================================

AccessiblePresentationOLEShape::AccessiblePresentationOLEShape(
    const AccessibleShapeInfo& rShapeInfo, const AccessibleShapeTreeInfo& rShapeTreeInfo)
    : AccessibleOLEShape(rShapeInfo, rShapeTreeInfo)
{
}

AccessiblePresentationOLEShape::~AccessiblePresentationOLEShape() {}

// XServiceInfo

OUString SAL_CALL AccessiblePresentationOLEShape::getImplementationName()
{
    return "AccessiblePresentationOLEShape";
}

/// Set this object's name if it is different to the current name.
OUString AccessiblePresentationOLEShape::CreateAccessibleBaseName()
{
    OUString sName;

    ShapeTypeId nShapeType = ShapeTypeHandler::Instance().GetTypeId(mxShape);
    switch (nShapeType)
    {
        case PRESENTATION_OLE:
            sName = "ImpressOLE";
            break;
        case PRESENTATION_CHART:
            sName = "ImpressChart";
            break;
        case PRESENTATION_TABLE:
            sName = "ImpressTable";
            break;
        default:
            sName = "UnknownAccessibleImpressOLEShape";
            if (mxShape.is())
                sName += ": " + mxShape->getShapeType();
    }

    return sName;
}

//  Return this object's role.
sal_Int16 SAL_CALL AccessiblePresentationOLEShape::getAccessibleRole()
{
    return AccessibleRole::EMBEDDED_OBJECT;
}

} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
