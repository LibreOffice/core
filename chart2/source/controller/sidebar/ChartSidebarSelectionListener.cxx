/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ChartSidebarSelectionListener.hxx"

#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/frame/XController.hpp>

#include <ObjectIdentifier.hxx>

namespace chart {
namespace sidebar {

ChartSidebarSelectionListenerParent::~ChartSidebarSelectionListenerParent()
{
}

ChartSidebarSelectionListener::ChartSidebarSelectionListener(
        ChartSidebarSelectionListenerParent* pParent):
    mpParent(pParent)
{
}

ChartSidebarSelectionListener::ChartSidebarSelectionListener(
        ChartSidebarSelectionListenerParent* pParent,
        ObjectType eType):
    mpParent(pParent)
{
    maTypes.push_back(eType);
}

ChartSidebarSelectionListener::~ChartSidebarSelectionListener()
{
}

void ChartSidebarSelectionListener::selectionChanged(const css::lang::EventObject& rEvent)
{
    if (!mpParent)
        return;

    bool bCorrectObjectSelected = false;

    css::uno::Reference<css::frame::XController> xController(rEvent.Source, css::uno::UNO_QUERY);
    if (xController.is())
    {
        css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(xController, css::uno::UNO_QUERY);
        if (xSelectionSupplier.is())
        {
            css::uno::Any aAny = xSelectionSupplier->getSelection();
            if (aAny.hasValue())
            {
                OUString aCID;
                aAny >>= aCID;
                ObjectType eType = ObjectIdentifier::getObjectType(aCID);
                bCorrectObjectSelected = std::any_of(maTypes.begin(), maTypes.end(),
                        [eType](const ObjectType& eTypeInVector) { return eType == eTypeInVector; });
            }
        }
    }

    mpParent->selectionChanged(bCorrectObjectSelected);
}

void ChartSidebarSelectionListener::disposing(const css::lang::EventObject& /*rEvent*/)
{
    if (!mpParent)
        return;

    mpParent = nullptr;
}

void ChartSidebarSelectionListener::setAcceptedTypes(const std::vector<ObjectType>& aTypes)
{
    maTypes = aTypes;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
