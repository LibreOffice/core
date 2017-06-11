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


#include "svx/DescriptionGenerator.hxx"
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/XShapeDescriptor.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <comphelper/processfactory.hxx>
#include <vcl/svapp.hxx>

#include <com/sun/star/uno/Exception.hpp>

// Includes for string resources.
#include "svx/strings.hrc"
#include <svx/dialmgr.hxx>

#include <svx/xdef.hxx>
#include "svx/unoapi.hxx"
#include "lookupcolorname.hxx"

using namespace ::com::sun::star;


namespace accessibility {


DescriptionGenerator::DescriptionGenerator (
    const uno::Reference<drawing::XShape>& xShape)
    : mxShape (xShape),
      mxSet (mxShape, uno::UNO_QUERY),
      mbIsFirstProperty (true)
{
}


DescriptionGenerator::~DescriptionGenerator()
{
}


void DescriptionGenerator::Initialize(const char* pResourceId)
{
    // Get the string from the resource for the specified id.
    OUString sPrefix;
    {
        SolarMutexGuard aGuard;
        sPrefix = SvxResId(pResourceId);
    }

    // Forward the call with the resulting string.
    Initialize (sPrefix);
}


void DescriptionGenerator::Initialize (const OUString& sPrefix)
{
    msDescription = sPrefix;
    if (mxSet.is())
    {
        {
            SolarMutexGuard aGuard;

            msDescription.append(' ');
            msDescription.append(SvxResId(RID_SVXSTR_A11Y_WITH));
            msDescription.append(' ');

            msDescription.append(SvxResId (RID_SVXSTR_A11Y_STYLE));
            msDescription.append('=');
        }

        try
        {
            if (mxSet.is())
            {
                uno::Any aValue = mxSet->getPropertyValue ("Style");
                uno::Reference<container::XNamed> xStyle (aValue, uno::UNO_QUERY);
                if (xStyle.is())
                    msDescription.append (xStyle->getName());
            }
            else
                msDescription.append ("<no style>");
        }
        catch (const css::beans::UnknownPropertyException &)
        {
            msDescription.append ("<unknown>");
        }
    }
}


OUString DescriptionGenerator::operator() (void)
{
    msDescription.append('.');
    return msDescription.makeStringAndClear();
}


void DescriptionGenerator::AddProperty (
    const OUString& sPropertyName,
    PropertyType aType,
    const char* pLocalizedNameId,
    long nWhichId)
{
    OUString sLocalizedName;
    {
        SolarMutexGuard aGuard;
        sLocalizedName = SvxResId(pLocalizedNameId);
    }
    AddProperty (sPropertyName, aType, sLocalizedName, nWhichId);
}


void DescriptionGenerator::AddProperty (const OUString& sPropertyName,
    PropertyType aType, const OUString& sLocalizedName, long nWhichId)
{
    uno::Reference<beans::XPropertyState> xState (mxShape, uno::UNO_QUERY);
    if (xState.is()
        && xState->getPropertyState(sPropertyName)!=beans::PropertyState_DEFAULT_VALUE)
        if (mxSet.is())
        {
            // Append a separator from previous Properties.
            if ( ! mbIsFirstProperty)
                msDescription.append(',');
            else
            {
                SolarMutexGuard aGuard;

                msDescription.append(' ');
                msDescription.append(SvxResId(RID_SVXSTR_A11Y_AND));
                msDescription.append(' ');
                mbIsFirstProperty = false;
            }

            // Delegate to type specific property handling.
            switch (aType)
            {
                case PropertyType::Color:
                    AddColor (sPropertyName, sLocalizedName);
                    break;
                case PropertyType::Integer:
                    AddInteger (sPropertyName, sLocalizedName);
                    break;
                case PropertyType::String:
                    AddString (sPropertyName, sLocalizedName, nWhichId);
                    break;
                case PropertyType::FillStyle:
                    AddFillStyle (sPropertyName, sLocalizedName);
                    break;
            }
        }
}


void DescriptionGenerator::AppendString (const OUString& sString)
{
    msDescription.append (sString);
}


void DescriptionGenerator::AddLineProperties()
{
    AddProperty ("LineColor", DescriptionGenerator::PropertyType::Color, SIP_XA_LINECOLOR);
    AddProperty ("LineDashName", DescriptionGenerator::PropertyType::String,
                 SIP_XA_LINEDASH, XATTR_LINEDASH);
    AddProperty ("LineWidth", DescriptionGenerator::PropertyType::Integer, SIP_XA_LINEWIDTH);
}


/** The fill style is described by the property "FillStyle".  Depending on
    its value a hatch-, gradient-, or bitmap name is appended.
*/
void DescriptionGenerator::AddFillProperties()
{
    AddProperty ("FillStyle", DescriptionGenerator::PropertyType::FillStyle, SIP_XA_FILLSTYLE);
}


void DescriptionGenerator::Add3DProperties()
{
    AddProperty ("D3DMaterialColor", DescriptionGenerator::PropertyType::Color,
        RID_SVXSTR_A11Y_3D_MATERIAL_COLOR);
    AddLineProperties ();
    AddFillProperties ();
}


void DescriptionGenerator::AddTextProperties()
{
    AddProperty ("CharColor", DescriptionGenerator::PropertyType::Color);
    AddFillProperties ();
}


/** Search for the given color in the global color table.  If found append
    its name to the description.  Otherwise append its RGB tuple.
*/
void DescriptionGenerator::AddColor (const OUString& sPropertyName,
    const OUString& sLocalizedName)
{
    msDescription.append(sLocalizedName);
    msDescription.append('=');

    try
    {

        long nValue(0);
        if (mxSet.is())
        {
            uno::Any aValue = mxSet->getPropertyValue (sPropertyName);
            aValue >>= nValue;
        }

        msDescription.append (lookUpColorName(nValue));
    }
    catch (const css::beans::UnknownPropertyException &)
    {
        msDescription.append ("<unknown>");
    }
}


void DescriptionGenerator::AddInteger (const OUString& sPropertyName,
    const OUString& sLocalizedName)
{
    msDescription.append(sLocalizedName);
    msDescription.append('=');

    try
    {
        if (mxSet.is())
        {
            uno::Any aValue = mxSet->getPropertyValue (sPropertyName);
            long nValue = 0;
            aValue >>= nValue;
            msDescription.append (nValue);
        }
    }
    catch (const css::beans::UnknownPropertyException &)
    {
        msDescription.append ("<unknown>");
    }
}


void DescriptionGenerator::AddString (const OUString& sPropertyName,
    const OUString& sLocalizedName, long nWhichId)
{
    msDescription.append(sLocalizedName);
    msDescription.append('=');

    try
    {
        if (mxSet.is())
        {
            uno::Any aValue = mxSet->getPropertyValue (sPropertyName);
            OUString sValue;
            aValue >>= sValue;

            if (nWhichId >= 0)
            {
                SolarMutexGuard aGuard;
                OUString sLocalizedValue =
                    SvxUnogetInternalNameForItem(sal::static_int_cast<sal_Int16>(nWhichId),
                                              sValue);
                msDescription.append (sLocalizedValue);
            }
            else
                msDescription.append (sValue);
        }
    }
    catch (const css::beans::UnknownPropertyException &)
    {
        msDescription.append ("<unknown>");
    }
}


void DescriptionGenerator::AddFillStyle (const OUString& sPropertyName,
    const OUString& sLocalizedName)
{
    msDescription.append(sLocalizedName);
    msDescription.append('=');

    try
    {
        if (mxSet.is())
        {
            uno::Any aValue = mxSet->getPropertyValue (sPropertyName);
            drawing::FillStyle aFillStyle;
            aValue >>= aFillStyle;

            // Get the fill style name from the resource.
            OUString sFillStyleName;
            {
                SolarMutexGuard aGuard;
                switch (aFillStyle)
                {
                    case drawing::FillStyle_NONE:
                        sFillStyleName = SvxResId(RID_SVXSTR_A11Y_FILLSTYLE_NONE);
                        break;
                    case drawing::FillStyle_SOLID:
                        sFillStyleName = SvxResId(RID_SVXSTR_A11Y_FILLSTYLE_SOLID);
                        break;
                    case drawing::FillStyle_GRADIENT:
                        sFillStyleName = SvxResId(RID_SVXSTR_A11Y_FILLSTYLE_GRADIENT);
                        break;
                    case drawing::FillStyle_HATCH:
                        sFillStyleName = SvxResId(RID_SVXSTR_A11Y_FILLSTYLE_HATCH);
                        break;
                    case drawing::FillStyle_BITMAP:
                        sFillStyleName = SvxResId(RID_SVXSTR_A11Y_FILLSTYLE_BITMAP);
                        break;
                    default:
                        break;
                }
            }
            msDescription.append (sFillStyleName);

            // Append the appropriate properties.
            switch (aFillStyle)
            {
                case drawing::FillStyle_NONE:
                    break;
                case drawing::FillStyle_SOLID:
                    AddProperty ("FillColor", PropertyType::Color, SIP_XA_FILLCOLOR);
                    break;
                case drawing::FillStyle_GRADIENT:
                    AddProperty ("FillGradientName", PropertyType::String, SIP_XA_FILLGRADIENT,
                        XATTR_FILLGRADIENT);
                    break;
                case drawing::FillStyle_HATCH:
                    AddProperty ("FillColor", PropertyType::Color, SIP_XA_FILLCOLOR);
                    AddProperty ("FillHatchName", PropertyType::String, SIP_XA_FILLHATCH,
                        XATTR_FILLHATCH);
                    break;
                case drawing::FillStyle_BITMAP:
                    AddProperty ("FillBitmapName", PropertyType::String, SIP_XA_FILLBITMAP,
                        XATTR_FILLBITMAP);
                    break;
                default:
                    break;
            }
        }
    }
    catch (const css::beans::UnknownPropertyException &)
    {
        msDescription.append ("<unknown>");
    }
}

} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
