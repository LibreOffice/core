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

#include <AccessiblePresentationGraphicShape.hxx>
#include <com/sun/star/accessibility/AccessibleRole.hpp>

#include <SdShapeTypes.hxx>

#include <svx/DescriptionGenerator.hxx>
#include <svx/ShapeTypeHandler.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

namespace accessibility {

//=====  internal  ============================================================

AccessiblePresentationGraphicShape::AccessiblePresentationGraphicShape (
    const AccessibleShapeInfo& rShapeInfo,
    const AccessibleShapeTreeInfo& rShapeTreeInfo)
    : AccessibleGraphicShape (rShapeInfo, rShapeTreeInfo)
{
}

AccessiblePresentationGraphicShape::~AccessiblePresentationGraphicShape()
{
}

// XServiceInfo

OUString SAL_CALL
    AccessiblePresentationGraphicShape::getImplementationName()
{
    return OUString("AccessiblePresentationGraphicShape");
}

/// Set this object's name if is different to the current name.
OUString
    AccessiblePresentationGraphicShape::CreateAccessibleBaseName()
{
    OUString sName;

    ShapeTypeId nShapeType = ShapeTypeHandler::Instance().GetTypeId (mxShape);
    switch (nShapeType)
    {
        case PRESENTATION_GRAPHIC_OBJECT:
            sName = "ImpressGraphicObject";
            break;
        default:
            sName = "UnknownAccessibleImpressShape";
            uno::Reference<drawing::XShapeDescriptor> xDescriptor (mxShape, uno::UNO_QUERY);
            if (xDescriptor.is())
                sName += ": " + xDescriptor->getShapeType();
    }

    return sName;
}

OUString
    AccessiblePresentationGraphicShape::CreateAccessibleDescription()
{
    //    return createAccessibleName ();
    DescriptionGenerator aDG (mxShape);
    ShapeTypeId nShapeType = ShapeTypeHandler::Instance().GetTypeId (mxShape);
    switch (nShapeType)
    {
        case PRESENTATION_GRAPHIC_OBJECT:
            aDG.Initialize ("PresentationGraphicShape");
            break;
        default:
            aDG.Initialize ("Unknown accessible presentation graphic shape");
            uno::Reference<drawing::XShapeDescriptor> xDescriptor (mxShape, uno::UNO_QUERY);
            if (xDescriptor.is())
            {
                aDG.AppendString ("service name=");
                aDG.AppendString (xDescriptor->getShapeType());
            }
    }

    return aDG();
}

sal_Int16 SAL_CALL AccessiblePresentationGraphicShape::getAccessibleRole ()
{
    return  AccessibleRole::GRAPHIC ;
}
} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
