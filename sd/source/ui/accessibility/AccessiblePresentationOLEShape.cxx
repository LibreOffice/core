/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"
#include "AccessiblePresentationOLEShape.hxx"

#include "SdShapeTypes.hxx"

#include <svx/DescriptionGenerator.hxx>
#include <rtl/ustring.h>

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

namespace accessibility {

//=====  internal  ============================================================

AccessiblePresentationOLEShape::AccessiblePresentationOLEShape (
    const AccessibleShapeInfo& rShapeInfo,
    const AccessibleShapeTreeInfo& rShapeTreeInfo)
    :   AccessibleOLEShape (rShapeInfo, rShapeTreeInfo)
{
}

AccessiblePresentationOLEShape::~AccessiblePresentationOLEShape (void)
{
}

//=====  XServiceInfo  ========================================================

::rtl::OUString SAL_CALL
    AccessiblePresentationOLEShape::getImplementationName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AccessiblePresentationOLEShape"));
}

/// Set this object's name if it is different to the current name.
::rtl::OUString
    AccessiblePresentationOLEShape::CreateAccessibleBaseName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ::rtl::OUString sName;

    ShapeTypeId nShapeType = ShapeTypeHandler::Instance().GetTypeId (mxShape);
    switch (nShapeType)
    {
        case PRESENTATION_OLE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressOLE"));
            break;
        case PRESENTATION_CHART:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressChart"));
            break;
        case PRESENTATION_TABLE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressTable"));
            break;
        default:
            sName = ::rtl::OUString (
                RTL_CONSTASCII_USTRINGPARAM("UnknownAccessibleImpressOLEShape"));
            uno::Reference<drawing::XShapeDescriptor> xDescriptor (mxShape, uno::UNO_QUERY);
            if (xDescriptor.is())
                sName += ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM(": "))
                    + xDescriptor->getShapeType();
    }

    return sName;
}

::rtl::OUString
    AccessiblePresentationOLEShape::CreateAccessibleDescription (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    //    return createAccessibleName();
    DescriptionGenerator aDG (mxShape);
    ShapeTypeId nShapeType = ShapeTypeHandler::Instance().GetTypeId (mxShape);
    switch (nShapeType)
    {
        case PRESENTATION_OLE:
            aDG.Initialize (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("PresentationOLEShape")));
            //SVX_RESSTR(RID_SVXSTR_A11Y_ST_RECTANGLE));
            aDG.AddProperty (OUString(RTL_CONSTASCII_USTRINGPARAM ("CLSID")),
                DescriptionGenerator::STRING);
            break;
        case PRESENTATION_CHART:
            aDG.Initialize (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PresentationChartShape")));
            //SVX_RESSTR(RID_SVXSTR_A11Y_ST_RECTANGLE));
            aDG.AddProperty (OUString(RTL_CONSTASCII_USTRINGPARAM("CLSID")),
                DescriptionGenerator::STRING);
            break;
        case PRESENTATION_TABLE:
            aDG.Initialize (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PresentationTableShape")));
            //SVX_RESSTR(RID_SVXSTR_A11Y_ST_RECTANGLE));
            aDG.AddProperty (OUString(RTL_CONSTASCII_USTRINGPARAM("CLSID")),
                DescriptionGenerator::STRING);
            break;
        default:
            aDG.Initialize (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Unknown accessible presentation OLE shape")));
            uno::Reference<drawing::XShapeDescriptor> xDescriptor (mxShape, uno::UNO_QUERY);
            if (xDescriptor.is())
            {
                aDG.AppendString (::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("service name=")));
                aDG.AppendString (xDescriptor->getShapeType());
            }
    }

    return aDG();
}

} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
