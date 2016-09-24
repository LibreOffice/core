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
#include <svx/svdmodel.hxx>

#include <cppuhelper/queryinterface.hxx>

using namespace ::accessibility;
using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

// internal
AccessibleGraphicShape::AccessibleGraphicShape (
    const AccessibleShapeInfo& rShapeInfo,
    const AccessibleShapeTreeInfo& rShapeTreeInfo)
    : AccessibleShape (rShapeInfo, rShapeTreeInfo)
{
}


AccessibleGraphicShape::~AccessibleGraphicShape()
{
}

// XAccessibleImage
OUString SAL_CALL AccessibleGraphicShape::getAccessibleImageDescription()
    throw (css::uno::RuntimeException, std::exception)
{
    if (m_pShape)
        return m_pShape->GetTitle();
    return AccessibleShape::getAccessibleDescription ();
}


sal_Int32 SAL_CALL AccessibleGraphicShape::getAccessibleImageHeight()
    throw (css::uno::RuntimeException, std::exception)
{
    return AccessibleShape::getSize().Height;
}


sal_Int32 SAL_CALL AccessibleGraphicShape::getAccessibleImageWidth()
    throw (css::uno::RuntimeException, std::exception)
{
    return AccessibleShape::getSize().Width;
}

// XInterface
css::uno::Any SAL_CALL
    AccessibleGraphicShape::queryInterface (const css::uno::Type & rType)
    throw (css::uno::RuntimeException, std::exception)
{
    css::uno::Any aReturn = AccessibleShape::queryInterface (rType);
    if ( ! aReturn.hasValue())
        aReturn = ::cppu::queryInterface (rType,
            static_cast<XAccessibleImage*>(this));
    return aReturn;
}


void SAL_CALL
    AccessibleGraphicShape::acquire()
    throw ()
{
    AccessibleShape::acquire ();
}


void SAL_CALL
    AccessibleGraphicShape::release()
    throw ()
{
    AccessibleShape::release ();
}

// XServiceInfo
OUString SAL_CALL
    AccessibleGraphicShape::getImplementationName()
    throw (css::uno::RuntimeException, std::exception)
{
    return OUString("AccessibleGraphicShape");
}


css::uno::Sequence< OUString> SAL_CALL
    AccessibleGraphicShape::getSupportedServiceNames()
    throw (css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed ();
    // Get list of supported service names from base class...
    uno::Sequence<OUString> aServiceNames =
        AccessibleShape::getSupportedServiceNames();
    sal_Int32 nCount (aServiceNames.getLength());

    // ...and add additional names.
    aServiceNames.realloc (nCount + 1);
    aServiceNames[nCount] = "com.sun.star.drawing.AccessibleGraphicShape";

    return aServiceNames;
}

// XTypeProvider
uno::Sequence<uno::Type> SAL_CALL
    AccessibleGraphicShape::getTypes()
    throw (uno::RuntimeException, std::exception)
{
    // Get list of types from the context base implementation...
    uno::Sequence<uno::Type> aTypeList (AccessibleShape::getTypes());
    // ...and add the additional type for the component.
    long nTypeCount = aTypeList.getLength();
    aTypeList.realloc (nTypeCount + 1);
    const uno::Type aImageType =
        cppu::UnoType<XAccessibleImage>::get();
    aTypeList[nTypeCount] = aImageType;

    return aTypeList;
}


/// Create the base name of this object, i.e. the name without appended number.
OUString
    AccessibleGraphicShape::CreateAccessibleBaseName()
    throw (css::uno::RuntimeException)
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
            uno::Reference<drawing::XShapeDescriptor> xDescriptor (mxShape, uno::UNO_QUERY);
            if (xDescriptor.is())
                sName += ": " + xDescriptor->getShapeType();
    }

    return sName;
}

OUString AccessibleGraphicShape::CreateAccessibleDescription()
    throw (css::uno::RuntimeException, std::exception)
{
    //Don't use the same information for accessible name and accessible description.
    OUString sDesc;
    if (m_pShape)
        sDesc =  m_pShape->GetTitle();
    if (!sDesc.isEmpty())
        return sDesc;
    return CreateAccessibleBaseName();
}

//  Return this object's role.
sal_Int16 SAL_CALL AccessibleGraphicShape::getAccessibleRole()
        throw (css::uno::RuntimeException, std::exception)
{
    if( m_pShape->GetModel()->GetImageMapForObject(m_pShape) != nullptr )
        return AccessibleRole::IMAGE_MAP;
    else
        return AccessibleShape::getAccessibleRole();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
