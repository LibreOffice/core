/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "uiobject.hxx"

#include "Window.hxx"
#include "DrawViewShell.hxx"
#include "sdpage.hxx"

ImpressWindowUIObject::ImpressWindowUIObject(VclPtr<sd::Window> xWindow):
    WindowUIObject(xWindow),
    mxWindow(xWindow)
{
}

StringMap ImpressWindowUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();

    aMap["SelectedText"] = getViewShell()->GetSelectionText();
    aMap["CurrentSlide"] = OUString::number(getViewShell()->GetCurPageId());

    return aMap;
}

void ImpressWindowUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    if (rAction == "SET")
    {
        if (rParameters.find("ZOOM") != rParameters.end())
        {
            auto itr = rParameters.find("ZOOM");
            OUString aVal = itr->second;
            sal_Int32 nVal = aVal.toInt32();
            getViewShell()->SetZoom(nVal);
        }
    }
    else if (rAction == "GOTO")
    {
        if (rParameters.find("PAGE") != rParameters.end())
        {
            auto itr = rParameters.find("PAGE");
            OUString aVal = itr->second;
            sal_Int32 nVal = aVal.toInt32();
            getViewShell()->SwitchPage(nVal);
        }
    }
    else if (rAction == "SELECT")
    {
        if (rParameters.find("OBJECT") != rParameters.end())
        {
            auto itr = rParameters.find("OBJECT");
            OUString aName = itr->second;
            SdrObject* pObj = getObject(aName);
            SdrPageView* pPageView = getPageView();
            getViewShell()->GetView()->MarkObj(pObj, pPageView);
        }
    }
    else if (rAction == "DESELECT")
    {
        getViewShell()->GetView()->UnMarkAll();
    }
    else
        WindowUIObject::execute(rAction, rParameters);
}

namespace {

OUString getObjectName(SdrObject* pObject)
{
    if (pObject->GetName().isEmpty())
        return "Unnamed Drawinglayer object " + OUString::number(pObject->GetOrdNum());
    else
        return pObject->GetName();
}

}

std::set<OUString> ImpressWindowUIObject::get_children() const
{
    SdrPage* pPage = getViewShell()->getCurrentPage();

    std::set<OUString> aRet;
    if (!pPage)
        return aRet;

    size_t nObjs = pPage->GetObjCount();
    for (size_t i = 0; i < nObjs; ++i)
    {
        SdrObject* pObject = pPage->GetObj(i);
        aRet.insert(getObjectName(pObject));
    }

    return aRet;
}

OUString ImpressWindowUIObject::get_name() const
{
    return OUString("ImpressWindowUIObject");
}

std::unique_ptr<UIObject> ImpressWindowUIObject::create(vcl::Window* pWindow)
{
    sd::Window* pWin = dynamic_cast<sd::Window*>(pWindow);
    assert(pWin);
    return std::unique_ptr<UIObject>(new ImpressWindowUIObject(pWin));
}

sd::DrawViewShell* ImpressWindowUIObject::getViewShell() const
{
    sd::DrawViewShell* pViewShell = dynamic_cast<sd::DrawViewShell*>(mxWindow->GetViewShell());
    assert(pViewShell);

    return pViewShell;
}

SdrObject* ImpressWindowUIObject::getObject(const OUString& rName)
{
    SdrPage* pPage = getViewShell()->getCurrentPage();

    if (!pPage)
        return nullptr;

    size_t nObjs = pPage->GetObjCount();
    for (size_t i = 0; i < nObjs; ++i)
    {
        SdrObject* pObj = pPage->GetObj(i);
        if (rName == getObjectName(pObj))
            return pObj;
    }

    return nullptr;
}

SdrPageView* ImpressWindowUIObject::getPageView()
{
    return getViewShell()->GetView()->GetSdrPageView();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
