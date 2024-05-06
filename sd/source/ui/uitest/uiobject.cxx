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
#include <svx/uiobject.hxx>
#include <tools/debug.hxx>
#include <utility>

namespace
{
class ImpressSdrObject : public SdrUIObject
{
public:
    ImpressSdrObject(const VclPtr<sd::Window>& xImpressWin, OUString aName);

    SdrObject* get_object() override;

    virtual bool equals(const UIObject& rOther) const override;

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

SdrObject* ImpressSdrObject::get_object() { return getObject(mxWindow, maName); }

bool ImpressSdrObject::equals(const UIObject& rOther) const
{
    const ImpressSdrObject* pOther = dynamic_cast<const ImpressSdrObject*>(&rOther);
    if (!pOther)
        return false;
    return mxWindow.get() == pOther->mxWindow.get();
}

ImpressWindowUIObject::ImpressWindowUIObject(const VclPtr<sd::Window>& xWindow)
    : WindowUIObject(xWindow)
    , mxWindow(xWindow)
{
}

StringMap ImpressWindowUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();

    aMap["SelectedText"] = getViewShell(mxWindow)->GetSelectionText(false);
    aMap["CurrentSlide"] = OUString::number(getViewShell(mxWindow)->GetCurPagePos() + 1);
    aMap["Zoom"] = OUString::number(getViewShell(mxWindow)->GetZoom());

    return aMap;
}

void ImpressWindowUIObject::execute(const OUString& rAction, const StringMap& rParameters)
{
    if (rAction == "SET")
    {
        if (rParameters.find("ZOOM") != rParameters.end())
        {
            auto itr = rParameters.find("ZOOM");
            OUString aVal = itr->second;
            sal_Int32 nVal = aVal.toInt32();
            getViewShell(mxWindow)->SetZoom(nVal);
        }
    }
    else if (rAction == "GOTO")
    {
        if (rParameters.find("PAGE") != rParameters.end())
        {
            auto itr = rParameters.find("PAGE");
            OUString aVal = itr->second;
            sal_Int32 nVal = aVal.toInt32();
            getViewShell(mxWindow)->SwitchPage(nVal - 1);
        }
    }
    else if (rAction == "SELECT")
    {
        if (rParameters.find("OBJECT") != rParameters.end())
        {
            auto itr = rParameters.find("OBJECT");
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

        auto itr = rParameters.find("PANEL");
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

OUString ImpressWindowUIObject::get_name() const { return "ImpressWindowUIObject"; }

std::unique_ptr<UIObject> ImpressWindowUIObject::create(vcl::Window* pWindow)
{
    sd::Window* pWin = dynamic_cast<sd::Window*>(pWindow);
    assert(pWin);
    return std::unique_ptr<UIObject>(new ImpressWindowUIObject(pWin));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
