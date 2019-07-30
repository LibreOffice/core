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

#include <tools/fract.hxx>

SdrUIObject::~SdrUIObject()
{
}

StringMap SdrUIObject::get_state()
{
    StringMap aMap;
    SdrObject* pObject = get_object();

    if (!pObject)
        return aMap;

    aMap["Name"] = pObject->GetName();
    aMap["Description"] = pObject->GetDescription();
    aMap["Title"] = pObject->GetTitle();
    aMap["Z-Order"] = OUString::number(pObject->GetOrdNum());
    aMap["Layer"] = OUString::number(sal_uInt8(pObject->GetLayer()));
    aMap["IsGroupObject"] = OUString::boolean(pObject->IsGroupObject());
    aMap["IsPolyObject"] = OUString::boolean(pObject->IsPolyObj());
    aMap["PointCount"] = OUString::number(pObject->GetPointCount());
    aMap["HasTextEdit"] = OUString::boolean(pObject->HasTextEdit());
    aMap["HasMacro"] = OUString::boolean(pObject->HasMacro());
    aMap["IsClosed"] = OUString::boolean(pObject->IsClosedObj());
    aMap["IsEdgeObject"] = OUString::boolean(pObject->IsEdgeObj());
    aMap["Is3DObject"] = OUString::boolean(pObject->Is3DObj());
    aMap["IsUNOObject"] = OUString::boolean(pObject->IsUnoObj());
    aMap["MoveProtected"] = OUString::boolean(pObject->IsMoveProtect());
    aMap["ResizeProtected"] = OUString::boolean(pObject->IsResizeProtect());
    aMap["Printable"] = OUString::boolean(pObject->IsPrintable());
    aMap["Visible"] = OUString::boolean(pObject->IsVisible());
    aMap["HasText"] = OUString::boolean(pObject->HasText());

    return aMap;
}

void SdrUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    SdrObject* pObj = get_object();
    if (!pObj)
        return;

    if (rAction == "MOVE")
    {
        auto itrNX = rParameters.find("X");
        if (itrNX == rParameters.end())
            throw css::uno::RuntimeException("missing parameter X");

        auto itrNY = rParameters.find("Y");
        if (itrNY == rParameters.end())
            throw css::uno::RuntimeException("missing parameter Y");

        long nX = itrNX->second.toInt32();
        long nY = itrNY->second.toInt32();
        Size aMoveRange(nX, nY);
        pObj->Move(aMoveRange);
    }
    else if (rAction == "RESIZE")
    {
        auto itrNX = rParameters.find("X");
        if (itrNX == rParameters.end())
            throw css::uno::RuntimeException("missing parameter X");

        auto itrNY = rParameters.find("Y");
        if (itrNY == rParameters.end())
            throw css::uno::RuntimeException("missing parameter Y");

        long nX = itrNX->second.toInt32();
        long nY = itrNY->second.toInt32();
        Point aPos(nX, nY);

        auto itrFracX = rParameters.find("FRAC_X");
        if (itrFracX == rParameters.end())
            throw css::uno::RuntimeException("missing parameter FRAC_X");
        double nFracX = itrFracX->second.toDouble();
        Fraction aFracX(nFracX);

        auto itrFracY = rParameters.find("FRAC_Y");
        if (itrFracY == rParameters.end())
            throw css::uno::RuntimeException("missing parameter FRAC_Y");
        double nFracY = itrFracY->second.toDouble();
        Fraction aFracY(nFracY);
        pObj->Resize(aPos, aFracX, aFracY, true/*bRelative*/);
    }
    else if (rAction == "CROP")
    {
        // RotateFlyFrame3: Note: Crop does nothing at SdrObject
        // anymore, see comment at SdrObject::NbcCrop
        auto itrNX = rParameters.find("X");
        if (itrNX == rParameters.end())
            throw css::uno::RuntimeException("missing parameter X");

        auto itrNY = rParameters.find("Y");
        if (itrNY == rParameters.end())
            throw css::uno::RuntimeException("missing parameter Y");

        const double fX(itrNX->second.toDouble());
        const double fY(itrNY->second.toDouble());
        const basegfx::B2DPoint aPos(fX, fY);

        auto itrFracX = rParameters.find("FRAC_X");
        if (itrFracX == rParameters.end())
            throw css::uno::RuntimeException("missing parameter FRAC_X");
        const double fFracX(itrFracX->second.toDouble());

        auto itrFracY = rParameters.find("FRAC_Y");
        if (itrFracY == rParameters.end())
            throw css::uno::RuntimeException("missing parameter FRAC_Y");
        const double fFracY(itrFracY->second.toDouble());

        pObj->Crop(aPos, fFracX, fFracY);
    }
    else if (rAction == "ROTATE")
    {
        auto itrNX = rParameters.find("X");
        if (itrNX == rParameters.end())
            throw css::uno::RuntimeException("missing parameter X");

        auto itrNY = rParameters.find("Y");
        if (itrNY == rParameters.end())
            throw css::uno::RuntimeException("missing parameter Y");

        long nX = itrNX->second.toInt32();
        long nY = itrNY->second.toInt32();
        Point aPos(nX, nY);

        auto itrAngle = rParameters.find("ANGLE");
        if (itrAngle  == rParameters.end())
            throw css::uno::RuntimeException("missing parameter ANGLE");

        double nAngle = itrAngle->second.toDouble();
        pObj->Rotate(aPos, nAngle, 0, 0);
    }
    else if (rAction == "Mirror")
    {
        pObj->Mirror(Point(), Point());
    }
    else if (rAction == "SHEAR")
    {
        pObj->Shear(Point(), 0.0/*nAngle*/, 0, false);
    }
}

OUString SdrUIObject::get_type() const
{
    return "SdrUIObject";
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
