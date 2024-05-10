/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svx/uiobject.hxx>
#include <svx/svdobj.hxx>
#include <svx/SvxColorValueSet.hxx>
#include <tools/fract.hxx>
#include <vcl/window.hxx>
#include <memory>

SdrUIObject::~SdrUIObject() {}

StringMap SdrUIObject::get_state()
{
    StringMap aMap;
    SdrObject* pObject = get_object();

    if (!pObject)
        return aMap;

    aMap[u"Name"_ustr] = pObject->GetName();
    aMap[u"Description"_ustr] = pObject->GetDescription();
    aMap[u"Title"_ustr] = pObject->GetTitle();
    aMap[u"Z-Order"_ustr] = OUString::number(pObject->GetOrdNum());
    aMap[u"Layer"_ustr] = OUString::number(pObject->GetLayer().get());
    aMap[u"IsGroupObject"_ustr] = OUString::boolean(pObject->IsGroupObject());
    aMap[u"IsPolyObject"_ustr] = OUString::boolean(pObject->IsPolyObj());
    aMap[u"PointCount"_ustr] = OUString::number(pObject->GetPointCount());
    aMap[u"HasTextEdit"_ustr] = OUString::boolean(pObject->HasTextEdit());
    aMap[u"HasMacro"_ustr] = OUString::boolean(pObject->HasMacro());
    aMap[u"IsClosed"_ustr] = OUString::boolean(pObject->IsClosedObj());
    aMap[u"IsEdgeObject"_ustr] = OUString::boolean(pObject->IsEdgeObj());
    aMap[u"Is3DObject"_ustr] = OUString::boolean(pObject->Is3DObj());
    aMap[u"IsUNOObject"_ustr] = OUString::boolean(pObject->IsUnoObj());
    aMap[u"MoveProtected"_ustr] = OUString::boolean(pObject->IsMoveProtect());
    aMap[u"ResizeProtected"_ustr] = OUString::boolean(pObject->IsResizeProtect());
    aMap[u"Printable"_ustr] = OUString::boolean(pObject->IsPrintable());
    aMap[u"Visible"_ustr] = OUString::boolean(pObject->IsVisible());
    aMap[u"HasText"_ustr] = OUString::boolean(pObject->HasText());

    return aMap;
}

void SdrUIObject::execute(const OUString& rAction, const StringMap& rParameters)
{
    SdrObject* pObj = get_object();
    if (!pObj)
        return;

    if (rAction == "MOVE")
    {
        auto itrNX = rParameters.find(u"X"_ustr);
        if (itrNX == rParameters.end())
            throw css::uno::RuntimeException(u"missing parameter X"_ustr);

        auto itrNY = rParameters.find(u"Y"_ustr);
        if (itrNY == rParameters.end())
            throw css::uno::RuntimeException(u"missing parameter Y"_ustr);

        tools::Long nX = itrNX->second.toInt32();
        tools::Long nY = itrNY->second.toInt32();
        Size aMoveRange(nX, nY);
        pObj->Move(aMoveRange);
    }
    else if (rAction == "RESIZE")
    {
        auto itrNX = rParameters.find(u"X"_ustr);
        if (itrNX == rParameters.end())
            throw css::uno::RuntimeException(u"missing parameter X"_ustr);

        auto itrNY = rParameters.find(u"Y"_ustr);
        if (itrNY == rParameters.end())
            throw css::uno::RuntimeException(u"missing parameter Y"_ustr);

        tools::Long nX = itrNX->second.toInt32();
        tools::Long nY = itrNY->second.toInt32();
        Point aPos(nX, nY);

        auto itrFracX = rParameters.find(u"FRAC_X"_ustr);
        if (itrFracX == rParameters.end())
            throw css::uno::RuntimeException(u"missing parameter FRAC_X"_ustr);
        double nFracX = itrFracX->second.toDouble();
        Fraction aFracX(nFracX);

        auto itrFracY = rParameters.find(u"FRAC_Y"_ustr);
        if (itrFracY == rParameters.end())
            throw css::uno::RuntimeException(u"missing parameter FRAC_Y"_ustr);
        double nFracY = itrFracY->second.toDouble();
        Fraction aFracY(nFracY);
        pObj->Resize(aPos, aFracX, aFracY, true /*bRelative*/);
    }
    else if (rAction == "CROP")
    {
        // RotateFlyFrame3: Note: Crop does nothing at SdrObject
        // anymore, see comment at SdrObject::NbcCrop
        auto itrNX = rParameters.find(u"X"_ustr);
        if (itrNX == rParameters.end())
            throw css::uno::RuntimeException(u"missing parameter X"_ustr);

        auto itrNY = rParameters.find(u"Y"_ustr);
        if (itrNY == rParameters.end())
            throw css::uno::RuntimeException(u"missing parameter Y"_ustr);

        const double fX(itrNX->second.toDouble());
        const double fY(itrNY->second.toDouble());
        const basegfx::B2DPoint aPos(fX, fY);

        auto itrFracX = rParameters.find(u"FRAC_X"_ustr);
        if (itrFracX == rParameters.end())
            throw css::uno::RuntimeException(u"missing parameter FRAC_X"_ustr);
        const double fFracX(itrFracX->second.toDouble());

        auto itrFracY = rParameters.find(u"FRAC_Y"_ustr);
        if (itrFracY == rParameters.end())
            throw css::uno::RuntimeException(u"missing parameter FRAC_Y"_ustr);
        const double fFracY(itrFracY->second.toDouble());

        pObj->Crop(aPos, fFracX, fFracY);
    }
    else if (rAction == "ROTATE")
    {
        auto itrNX = rParameters.find(u"X"_ustr);
        if (itrNX == rParameters.end())
            throw css::uno::RuntimeException(u"missing parameter X"_ustr);

        auto itrNY = rParameters.find(u"Y"_ustr);
        if (itrNY == rParameters.end())
            throw css::uno::RuntimeException(u"missing parameter Y"_ustr);

        tools::Long nX = itrNX->second.toInt32();
        tools::Long nY = itrNY->second.toInt32();
        Point aPos(nX, nY);

        auto itrAngle = rParameters.find(u"ANGLE"_ustr);
        if (itrAngle == rParameters.end())
            throw css::uno::RuntimeException(u"missing parameter ANGLE"_ustr);

        double nAngle = itrAngle->second.toDouble();
        pObj->Rotate(aPos, Degree100(sal_Int32(nAngle)), 0, 0);
    }
    else if (rAction == "Mirror")
    {
        pObj->Mirror(Point(), Point());
    }
    else if (rAction == "SHEAR")
    {
        pObj->Shear(Point(), 0_deg100 /*nAngle*/, 0, false);
    }
}

OUString SdrUIObject::get_type() const { return u"SdrUIObject"_ustr; }

SvxColorValueSetUIObject::SvxColorValueSetUIObject(vcl::Window* pColorSetWin)
    : DrawingAreaUIObject(pColorSetWin)
    , mpColorSet(static_cast<SvxColorValueSet*>(mpController))
{
}

void SvxColorValueSetUIObject::execute(const OUString& rAction, const StringMap& rParameters)
{
    if (rAction == "CHOOSE")
    {
        if (rParameters.find(u"POS"_ustr) != rParameters.end())
        {
            OUString aIndexStr = rParameters.find(u"POS"_ustr)->second;
            sal_Int32 nIndex = aIndexStr.toInt32();
            mpColorSet->SelectItem(nIndex);
            mpColorSet->Select();
        }
    }
    else
        DrawingAreaUIObject::execute(rAction, rParameters);
}

std::unique_ptr<UIObject> SvxColorValueSetUIObject::create(vcl::Window* pWindow)
{
    return std::unique_ptr<UIObject>(new SvxColorValueSetUIObject(pWindow));
}

OUString SvxColorValueSetUIObject::get_name() const { return u"SvxColorValueSetUIObject"_ustr; }

StringMap SvxColorValueSetUIObject::get_state()
{
    StringMap aMap = DrawingAreaUIObject::get_state();
    aMap[u"CurrColorId"_ustr] = OUString::number(mpColorSet->GetSelectedItemId());
    aMap[u"CurrColorPos"_ustr] = OUString::number(mpColorSet->GetSelectItemPos());
    aMap[u"ColorsCount"_ustr] = OUString::number(mpColorSet->GetItemCount());
    aMap[u"ColCount"_ustr] = OUString::number(mpColorSet->GetColCount());
    aMap[u"ColorText"_ustr] = mpColorSet->GetItemText(mpColorSet->GetSelectedItemId());
    Color currColor = mpColorSet->GetItemColor(mpColorSet->GetSelectedItemId());
    aMap[u"R"_ustr] = OUString::number(currColor.GetRed());
    aMap[u"G"_ustr] = OUString::number(currColor.GetGreen());
    aMap[u"B"_ustr] = OUString::number(currColor.GetBlue());
    aMap[u"RGB"_ustr] = "(" + OUString::number(currColor.GetRed()) + ","
                        + OUString::number(currColor.GetGreen()) + ","
                        + OUString::number(currColor.GetBlue()) + ")";
    return aMap;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
