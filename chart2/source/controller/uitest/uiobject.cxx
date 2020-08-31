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
#include <ChartController.hxx>
#include <ObjectHierarchy.hxx>
#include <chartview/ExplicitValueProvider.hxx>
#include <com/sun/star/chart2/XChartDocument.hpp>

#include <comphelper/servicehelper.hxx>

#include <vcl/svapp.hxx>

#include <algorithm>
#include <iterator>

ChartUIObject::ChartUIObject(const VclPtr<chart::ChartWindow>& xChartWindow,
        const OUString& rCID):
    maCID(rCID),
    mxChartWindow(xChartWindow)
{
}

StringMap ChartUIObject::get_state()
{
    StringMap aMap;
    aMap["CID"] = maCID;

    return aMap;
}

void ChartUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    if (rAction == "SELECT")
    {
        std::unique_ptr<UIObject> pWindow = mxChartWindow->GetUITestFactory()(mxChartWindow.get());

        StringMap aParams;
        aParams["NAME"] = maCID;
        pWindow->execute(rAction, aParams);
    }
    else if (rAction == "COMMAND")
    {
        // first select object
        std::unique_ptr<UIObject> pWindow = mxChartWindow->GetUITestFactory()(mxChartWindow.get());

        StringMap aParams;
        aParams["NAME"] = maCID;
        pWindow->execute("SELECT", aParams);

        auto itr = rParameters.find("COMMAND");
        if (itr == rParameters.end())
            throw css::uno::RuntimeException("missing COMMAND parameter");

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
    return "ChartUIObject for type: ";
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
        aMap["SelectedObject"] = aSelectedObject;
    }

    return aMap;
}

void ChartWindowUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    if (rAction == "SELECT")
    {
        auto itr = rParameters.find("NAME");
        if (itr == rParameters.end())
            throw css::uno::RuntimeException("Missing Parameter 'NAME' for action 'SELECT'");


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

    throw css::uno::RuntimeException("unknown child");
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

    css::uno::Reference< css::chart2::XChartDocument > xChartDoc( pController->getModel(), css::uno::UNO_QUERY );

    css::uno::Reference<css::uno::XInterface> xChartView = pController->getChartView();
    chart::ExplicitValueProvider* pValueProvider = comphelper::getUnoTunnelImplementation<chart::ExplicitValueProvider>( xChartView );
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
    return "ChartWindowUIObject";
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
