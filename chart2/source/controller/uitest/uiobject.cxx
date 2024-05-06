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

#include <ChartWindow.hxx>
#include <ChartView.hxx>
#include <ChartController.hxx>
#include <ChartModel.hxx>
#include <ObjectHierarchy.hxx>
#include <chartview/ExplicitValueProvider.hxx>

#include <utility>
#include <vcl/svapp.hxx>

#include <algorithm>
#include <iterator>

ChartUIObject::ChartUIObject(const VclPtr<chart::ChartWindow>& xChartWindow,
        OUString aCID):
    maCID(std::move(aCID)),
    mxChartWindow(xChartWindow)
{
}

StringMap ChartUIObject::get_state()
{
    StringMap aMap;
    aMap[u"CID"_ustr] = maCID;

    return aMap;
}

void ChartUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    if (rAction == "SELECT")
    {
        std::unique_ptr<UIObject> pWindow = mxChartWindow->GetUITestFactory()(mxChartWindow.get());

        StringMap aParams;
        aParams[u"NAME"_ustr] = maCID;
        pWindow->execute(rAction, aParams);
    }
    else if (rAction == "COMMAND")
    {
        // first select object
        std::unique_ptr<UIObject> pWindow = mxChartWindow->GetUITestFactory()(mxChartWindow.get());

        StringMap aParams;
        aParams[u"NAME"_ustr] = maCID;
        pWindow->execute(u"SELECT"_ustr, aParams);

        auto itr = rParameters.find(u"COMMAND"_ustr);
        if (itr == rParameters.end())
            throw css::uno::RuntimeException(u"missing COMMAND parameter"_ustr);

        maCommands.emplace_back(new OUString(itr->second));
        OUString* pCommand = maCommands.rbegin()->get();

        Application::PostUserEvent(LINK(this, ChartUIObject, PostCommand), pCommand);
    }
}

IMPL_LINK(ChartUIObject, PostCommand, void*, pCommand, void)
{
        css::util::URL aURL;
        aURL.Path = *static_cast<OUString*>(pCommand);
        mxChartWindow->GetController()->dispatch(aURL, css::uno::Sequence<css::beans::PropertyValue>());
}

std::unique_ptr<UIObject> ChartUIObject::get_child(const OUString& rID)
{
    std::unique_ptr<UIObject> pWindow = mxChartWindow->GetUITestFactory()(mxChartWindow.get());

    return pWindow->get_child(rID);
}

std::set<OUString> ChartUIObject::get_children() const
{
    std::unique_ptr<UIObject> pWindow = mxChartWindow->GetUITestFactory()(mxChartWindow.get());

    return pWindow->get_children();
}

OUString ChartUIObject::get_type() const
{
    return u"ChartUIObject for type: "_ustr;
}

bool ChartUIObject::equals(const UIObject& rOther) const
{
    const ChartUIObject* pOther = dynamic_cast<const ChartUIObject*>(&rOther);
    if (!pOther)
        return false;
    return mxChartWindow.get() == pOther->mxChartWindow.get();
}

ChartWindowUIObject::ChartWindowUIObject(const VclPtr<chart::ChartWindow>& xChartWindow):
    WindowUIObject(xChartWindow),
    mxChartWindow(xChartWindow)
{
}

StringMap ChartWindowUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();

    chart::ChartController* pController = mxChartWindow->GetController();
    if (pController)
    {
        css::uno::Any aAny = pController->getSelection();
        OUString aSelectedObject;
        aAny >>= aSelectedObject;
        aMap[u"SelectedObject"_ustr] = aSelectedObject;
    }

    return aMap;
}

void ChartWindowUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    if (rAction == "SELECT")
    {
        auto itr = rParameters.find(u"NAME"_ustr);
        if (itr == rParameters.end())
            throw css::uno::RuntimeException(u"Missing Parameter 'NAME' for action 'SELECT'"_ustr);


        const OUString& rName = itr->second;
        css::uno::Any aAny;
        aAny <<= rName;

        chart::ChartController* pController = mxChartWindow->GetController();
        pController->select(aAny);
    }
    else
        WindowUIObject::execute(rAction, rParameters);
}

std::unique_ptr<UIObject> ChartWindowUIObject::get_child(const OUString& rID)
{
    if (chart::ObjectIdentifier::isCID(rID))
        return std::unique_ptr<UIObject>(new ChartUIObject(mxChartWindow, rID));

    throw css::uno::RuntimeException(u"unknown child"_ustr);
}

namespace {

void recursiveAdd(chart::ObjectIdentifier const & rID, std::set<OUString>& rChildren, const chart::ObjectHierarchy& rHierarchy)
{
    std::vector<chart::ObjectIdentifier> aChildIdentifiers = rHierarchy.getChildren(rID);
    std::transform(aChildIdentifiers.begin(), aChildIdentifiers.end(), std::inserter(rChildren, rChildren.begin()),
                [](const chart::ObjectIdentifier& rObject)
                {
                    return rObject.getObjectCID();
                }
            );

    for (const chart::ObjectIdentifier& ID: aChildIdentifiers)
        recursiveAdd(ID, rChildren, rHierarchy);
}

}

std::set<OUString> ChartWindowUIObject::get_children() const
{
    std::set<OUString> aChildren;

    chart::ChartController* pController = mxChartWindow->GetController();
    if (!pController)
        return aChildren;

    rtl::Reference<::chart::ChartModel> xChartDoc = pController->getChartModel();
    rtl::Reference<::chart::ChartView> xChartView = pController->getChartView();
    chart::ExplicitValueProvider* pValueProvider = xChartView.get();
    chart::ObjectHierarchy aHierarchy(xChartDoc, pValueProvider);
    chart::ObjectIdentifier aIdentifier = chart::ObjectHierarchy::getRootNodeOID();
    aChildren.insert(aIdentifier.getObjectCID());

    recursiveAdd(aIdentifier, aChildren, aHierarchy);

    return aChildren;
}

std::unique_ptr<UIObject> ChartWindowUIObject::create(vcl::Window* pWindow)
{
    chart::ChartWindow* pChartWindow = dynamic_cast<chart::ChartWindow*>(pWindow);
    assert(pChartWindow);

    return std::unique_ptr<UIObject>(new ChartWindowUIObject(pChartWindow));
}

OUString ChartWindowUIObject::get_name() const
{
    return u"ChartWindowUIObject"_ustr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
