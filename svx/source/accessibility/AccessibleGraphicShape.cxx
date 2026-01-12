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

#include <svx/AccessibleGraphicShape.hxx>

#include <svx/ShapeTypeHandler.hxx>
#include <svx/SvxShapeTypes.hxx>
#include <svx/svdobj.hxx>

#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XShapeDescriptor.hpp>
#include <comphelper/sequence.hxx>
#include <cppuhelper/queryinterface.hxx>

using namespace ::accessibility;
using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

// internal
AccessibleGraphicShape::AccessibleGraphicShape(const AccessibleShapeInfo& rShapeInfo,
                                               const AccessibleShapeTreeInfo& rShapeTreeInfo)
    : ImplInheritanceHelper(rShapeInfo, rShapeTreeInfo)
{
}


AccessibleGraphicShape::~AccessibleGraphicShape()
{
}

// XAccessibleImage
OUString SAL_CALL AccessibleGraphicShape::getAccessibleImageDescription()
{
    if (m_pShape)
        return m_pShape->GetTitle();
    return AccessibleShape::getAccessibleDescription ();
}


sal_Int32 SAL_CALL AccessibleGraphicShape::getAccessibleImageHeight()
{
    return AccessibleShape::getSize().Height;
}


sal_Int32 SAL_CALL AccessibleGraphicShape::getAccessibleImageWidth()
{
    return AccessibleShape::getSize().Width;
}

/// Create the base name of this object, i.e. the name without appended number.
OUString
    AccessibleGraphicShape::CreateAccessibleBaseName()
{
    OUString sName;

    ShapeTypeId nShapeType = ShapeTypeHandler::Instance().GetTypeId (mxShape);
    switch (nShapeType)
    {
        case DRAWING_GRAPHIC_OBJECT:
            sName = "GraphicObjectShape";
            break;

        default:
            sName = "UnknownAccessibleGraphicShape";
            uno::Reference<drawing::XShapeDescriptor> xDescriptor (mxShape);
            if (xDescriptor.is())
                sName += ": " + xDescriptor->getShapeType();
    }

    return sName;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
