/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <jsdialog/jsdialogregister.hxx>
#include <jsdialog/jsdialogbuilder.hxx>

JSInstanceBuilder::JSDialogRegister JSInstanceBuilder::m_aWidgetRegister;

OUString JSInstanceBuilder::getMapIdFromWindowId() const
{
    if (m_sTypeOfJSON == "sidebar" || m_sTypeOfJSON == "notebookbar"
        || m_sTypeOfJSON == "formulabar" || m_sTypeOfJSON == "addressinputfield"
        || m_sTypeOfJSON == "navigator")
    {
        return OUString::number(m_nWindowId) + m_sTypeOfJSON;
    }
    else
        return OUString::number(m_nWindowId);
}

void JSInstanceBuilder::InsertWindowToMap(const OUString& nWindowId)
{
    auto aWidgetMap = JSInstanceBuilder::Widgets().Find(nWindowId);
    if (!aWidgetMap)
        JSInstanceBuilder::Widgets().Remember(nWindowId, std::make_shared<WidgetMap>());
}

void JSInstanceBuilder::RememberWidget(OUString sId, weld::Widget* pWidget)
{
    // do not use the same id for two widgets inside one builder
    // exception is sidebar where we base our full invalidation on that "Panel" id sharing
    if (m_sTypeOfJSON != "sidebar")
    {
        static std::atomic<unsigned long long int> nNotRepeatIndex = 0;
        auto aWindowIt = JSInstanceBuilder::Widgets().Find(getMapIdFromWindowId());
        if (aWindowIt)
        {
            auto aWidgetIt = aWindowIt->Find(sId);
            if (aWidgetIt)
            {
                unsigned long long int nIndex = nNotRepeatIndex++;
                // found duplicated it -> add some number to the id and apply to the widget
                sId = sId + OUString::number(nIndex);
                SalInstanceWidget* pSalWidget = dynamic_cast<SalInstanceWidget*>(pWidget);
                assert(pSalWidget && "can only be a SalInstanceWidget");
                vcl::Window* pVclWidget = pSalWidget->getWidget();
                pVclWidget->set_id(pVclWidget->get_id() + OUString::number(nIndex));
            }
        }
    }

    JSInstanceBuilder::RememberWidget(getMapIdFromWindowId(), sId, pWidget);
    m_aRememberedWidgets.push_back(sId);
}

void JSInstanceBuilder::RememberWidget(const OUString& nWindowId, const OUString& id,
                                       weld::Widget* pWidget)
{
    auto aWindowMap = JSInstanceBuilder::Widgets().Find(nWindowId);
    if (aWindowMap)
    {
        aWindowMap->Forget(id);
        aWindowMap->Remember(id, pWidget);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
