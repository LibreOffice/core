/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string_view>

#include "ChartColorWrapper.hxx"
#include <ChartModel.hxx>

#include <ObjectIdentifier.hxx>
#include <PropertyHelper.hxx>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/frame/XController.hpp>

#include <svx/linectrl.hxx>
#include <svx/tbcontrl.hxx>
#include <svx/xlndsit.hxx>
#include <svx/unomid.hxx>

#include <comphelper/lok.hxx>
#include <sal/log.hxx>
#include <sfx2/viewsh.hxx>
#include <utility>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

namespace chart::sidebar {

namespace {

OUString getCID(const css::uno::Reference<css::frame::XModel>& xModel)
{
    if (!xModel.is())
        return OUString();

    css::uno::Reference<css::frame::XController> xController(xModel->getCurrentController());
    css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(xController, css::uno::UNO_QUERY);
    if (!xSelectionSupplier.is())
        return OUString();

    css::uno::Any aAny = xSelectionSupplier->getSelection();
    if (!aAny.hasValue())
        return OUString();

    OUString aCID;
    aAny >>= aCID;

    return aCID;
}

css::uno::Reference<css::beans::XPropertySet> getPropSet(
        const rtl::Reference<::chart::ChartModel>& xModel)
{
    OUString aCID = getCID(xModel);
    css::uno::Reference<css::beans::XPropertySet> xPropSet =
        ObjectIdentifier::getObjectPropertySet(aCID, xModel);

    ObjectType eType = ObjectIdentifier::getObjectType(aCID);
    if (eType == OBJECTTYPE_DIAGRAM)
    {
        css::uno::Reference<css::chart2::XDiagram> xDiagram(
                xPropSet, css::uno::UNO_QUERY);
        if (!xDiagram.is())
            return xPropSet;

        xPropSet.set(xDiagram->getWall());
    }

    return xPropSet;
}

}

ChartColorWrapper::ChartColorWrapper(
        rtl::Reference<::chart::ChartModel> xModel,
        SvxColorToolBoxControl* pControl,
        OUString aName):
    mxModel(std::move(xModel)),
    mpControl(pControl),
    maPropertyName(std::move(aName))
{
}

void ChartColorWrapper::operator()([[maybe_unused]] const OUString& , const NamedColor& rColor)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);

    if (!xPropSet.is())
    {
        SAL_WARN("chart2", "Invalid reference to xPropSet");
        return;
    }

    xPropSet->setPropertyValue(maPropertyName, css::uno::Any(rColor.m_aColor));
}

void ChartColorWrapper::updateModel(const rtl::Reference<::chart::ChartModel>& xModel)
{
    mxModel = xModel;
}

void ChartColorWrapper::updateData()
{
    static constexpr OUString aLineColor = u"LineColor"_ustr;
    static const std::u16string_view aCommands[2] = {u".uno:XLineColor", u".uno:FillColor"};

    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    css::util::URL aUrl;
    aUrl.Complete = (maPropertyName == aLineColor) ? aCommands[0] : aCommands[1];

    css::frame::FeatureStateEvent aEvent;
    aEvent.FeatureURL = aUrl;
    aEvent.IsEnabled = true;
    aEvent.State = xPropSet->getPropertyValue(maPropertyName);
    mpControl->statusChanged(aEvent);

    SfxViewShell* pViewShell = SfxViewShell::Current();
    if (comphelper::LibreOfficeKit::isActive() && pViewShell && (maPropertyName == aLineColor))
    {
        OString sCommand = OUStringToOString(aUrl.Complete, RTL_TEXTENCODING_ASCII_US);
        sal_Int32 nColor = -1;
        aEvent.State >>= nColor;
        pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_STATE_CHANGED,
                                               sCommand + "=" + OString::number(nColor));
    }
}

ChartLineStyleWrapper::ChartLineStyleWrapper(
        rtl::Reference<::chart::ChartModel> xModel,
        SvxLineStyleToolBoxControl* pControl)
    : mxModel(std::move(xModel))
    , mpControl(pControl)
{
}

void ChartLineStyleWrapper::updateModel(const rtl::Reference<::chart::ChartModel>& xModel)
{
    mxModel = xModel;
}

namespace
{
    css::uno::Any getLineDash(
            const css::uno::Reference<css::frame::XModel>& xModel, const OUString& rDashName)
    {
        css::uno::Reference<css::lang::XMultiServiceFactory> xFact(xModel, css::uno::UNO_QUERY);
        css::uno::Reference<css::container::XNameAccess> xNameAccess(
                xFact->createInstance(u"com.sun.star.drawing.DashTable"_ustr),
                css::uno::UNO_QUERY );
        if(xNameAccess.is())
        {
            if (!xNameAccess->hasByName(rDashName))
                return css::uno::Any();

            return xNameAccess->getByName(rDashName);
        }

        return css::uno::Any();
    }
}

void ChartLineStyleWrapper::updateData()
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    css::util::URL aUrl;
    aUrl.Complete = ".uno:XLineStyle";

    css::frame::FeatureStateEvent aEvent;
    aEvent.IsEnabled = true;

    aEvent.FeatureURL = aUrl;
    aEvent.State = xPropSet->getPropertyValue(u"LineStyle"_ustr);
    mpControl->statusChanged(aEvent);

    aUrl.Complete = ".uno:LineDash";

    auto aLineDashName = xPropSet->getPropertyValue(u"LineDashName"_ustr);
    OUString aDashName;
    aLineDashName >>= aDashName;
    css::uno::Any aLineDash = getLineDash(mxModel, aDashName);
    XLineDashItem aDashItem;
    aDashItem.PutValue(aLineDash, MID_LINEDASH);

    aEvent.FeatureURL = aUrl;
    aDashItem.QueryValue(aEvent.State);
    mpControl->statusChanged(aEvent);
}

bool ChartLineStyleWrapper::operator()(std::u16string_view rCommand, const css::uno::Any& rValue)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);

    if (!xPropSet.is())
    {
        SAL_WARN("chart2", "Invalid reference to xPropSet");
        return false;
    }

    if (rCommand == u".uno:XLineStyle")
    {
        xPropSet->setPropertyValue(u"LineStyle"_ustr, rValue);
        return true;
    }
    else if (rCommand == u".uno:LineDash")
    {
        XLineDashItem aDashItem;
        aDashItem.PutValue(rValue, 0);
        css::uno::Any aAny;
        aDashItem.QueryValue(aAny, MID_LINEDASH);
        OUString aDashName = PropertyHelper::addLineDashUniqueNameToTable(aAny,
                mxModel,
                u""_ustr);
        xPropSet->setPropertyValue(u"LineDash"_ustr, aAny);
        xPropSet->setPropertyValue(u"LineDashName"_ustr, css::uno::Any(aDashName));
        return true;
    }
    return false;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
