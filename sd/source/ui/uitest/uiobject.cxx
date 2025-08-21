/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <uiobject.hxx>

#include <Window.hxx>
#include <DrawViewShell.hxx>
#include <sdpage.hxx>

#include <sfx2/sidebar/Sidebar.hxx>
#include <sfx2/sfxsids.hrc>
#include <tools/debug.hxx>
#include <utility>

namespace
{
class ImpressSdrObject : public UIObject
{
public:
    ImpressSdrObject(const VclPtr<sd::Window>& xImpressWin, OUString aName);

    virtual bool equals(const UIObject& rOther) const override;

    virtual StringMap get_state() override;

    virtual void execute(const OUString& rAction, const StringMap& rParameters) override;

    virtual OUString get_type() const override;

private:
    VclPtr<sd::Window> mxWindow;

    OUString maName;
};

sd::DrawViewShell* getViewShell(const VclPtr<sd::Window>& xWindow)
{
    sd::DrawViewShell* pViewShell = dynamic_cast<sd::DrawViewShell*>(xWindow->GetViewShell());
    assert(pViewShell);

    return pViewShell;
}

OUString getObjectName(SdrObject const* pObject)
{
    if (pObject->GetName().isEmpty())
        return "Unnamed Drawinglayer object " + OUString::number(pObject->GetOrdNum());
    else
        return pObject->GetName();
}

SdrObject* getObject(const VclPtr<sd::Window>& xWindow, std::u16string_view rName)
{
    SdrPage* pPage = getViewShell(xWindow)->getCurrentPage();

    if (!pPage)
        return nullptr;

    for (const rtl::Reference<SdrObject>& pObj : *pPage)
        if (rName == getObjectName(pObj.get()))
            return pObj.get();

    return nullptr;
}
}

ImpressSdrObject::ImpressSdrObject(const VclPtr<sd::Window>& xImpressWin, OUString aName)
    : mxWindow(xImpressWin)
    , maName(std::move(aName))
{
}

bool ImpressSdrObject::equals(const UIObject& rOther) const
{
    const ImpressSdrObject* pOther = dynamic_cast<const ImpressSdrObject*>(&rOther);
    if (!pOther)
        return false;
    return mxWindow.get() == pOther->mxWindow.get();
}

StringMap ImpressSdrObject::get_state()
{
    StringMap aMap;
    SdrObject* pObject = getObject(mxWindow, maName);

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

void ImpressSdrObject::execute(const OUString& rAction, const StringMap& rParameters)
{
    SdrObject* pObj = getObject(mxWindow, maName);
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

OUString ImpressSdrObject::get_type() const { return u"ImpressSdrObject"_ustr; }

ImpressWindowUIObject::ImpressWindowUIObject(const VclPtr<sd::Window>& xWindow)
    : WindowUIObject(xWindow)
    , mxWindow(xWindow)
{
}

StringMap ImpressWindowUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();

    aMap[u"SelectedText"_ustr] = getViewShell(mxWindow)->GetSelectionText(false);
    aMap[u"CurrentSlide"_ustr] = OUString::number(getViewShell(mxWindow)->GetCurPagePos() + 1);
    aMap[u"Zoom"_ustr] = OUString::number(getViewShell(mxWindow)->GetZoom());

    return aMap;
}

void ImpressWindowUIObject::execute(const OUString& rAction, const StringMap& rParameters)
{
    if (rAction == "SET")
    {
        if (rParameters.find(u"ZOOM"_ustr) != rParameters.end())
        {
            auto itr = rParameters.find(u"ZOOM"_ustr);
            OUString aVal = itr->second;
            sal_Int32 nVal = aVal.toInt32();
            getViewShell(mxWindow)->SetZoom(nVal);
        }
    }
    else if (rAction == "GOTO")
    {
        if (rParameters.find(u"PAGE"_ustr) != rParameters.end())
        {
            auto itr = rParameters.find(u"PAGE"_ustr);
            OUString aVal = itr->second;
            sal_Int32 nVal = aVal.toInt32();
            getViewShell(mxWindow)->SwitchPage(nVal - 1);
        }
    }
    else if (rAction == "SELECT")
    {
        if (rParameters.find(u"OBJECT"_ustr) != rParameters.end())
        {
            auto itr = rParameters.find(u"OBJECT"_ustr);
            OUString aName = itr->second;
            SdrObject* pObj = getObject(mxWindow, aName);
            SdrPageView* pPageView = getViewShell(mxWindow)->GetView()->GetSdrPageView();
            getViewShell(mxWindow)->GetView()->MarkObj(pObj, pPageView);
        }
    }
    else if (rAction == "SIDEBAR")
    {
        SfxViewFrame* pViewFrm = SfxViewFrame::Current();
        assert(pViewFrm && "ImpressWindowUIObject::execute: no viewframe");
        pViewFrm->ShowChildWindow(SID_SIDEBAR);

        auto itr = rParameters.find(u"PANEL"_ustr);
        if (itr != rParameters.end())
        {
            OUString aVal = itr->second;
            ::sfx2::sidebar::Sidebar::ShowPanel(aVal, pViewFrm->GetFrame().GetFrameInterface());
        }
    }
    else if (rAction == "DESELECT")
    {
        getViewShell(mxWindow)->GetView()->UnMarkAll();
    }
    else
        WindowUIObject::execute(rAction, rParameters);
}

std::unique_ptr<UIObject> ImpressWindowUIObject::get_child(const OUString& rID)
{
    return std::unique_ptr<UIObject>(new ImpressSdrObject(mxWindow, rID));
}

std::set<OUString> ImpressWindowUIObject::get_children() const
{
    SdrPage* pPage = getViewShell(mxWindow)->getCurrentPage();

    std::set<OUString> aRet;
    if (!pPage)
        return aRet;

    for (const rtl::Reference<SdrObject>& pObject : *pPage)
        aRet.insert(getObjectName(pObject.get()));

    return aRet;
}

OUString ImpressWindowUIObject::get_name() const { return u"ImpressWindowUIObject"_ustr; }

std::unique_ptr<UIObject> ImpressWindowUIObject::create(vcl::Window* pWindow)
{
    sd::Window* pWin = dynamic_cast<sd::Window*>(pWindow);
    assert(pWin);
    return std::unique_ptr<UIObject>(new ImpressWindowUIObject(pWin));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
