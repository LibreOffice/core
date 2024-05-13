/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/theUICommandDescription.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <comphelper/dispatchcommand.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequenceashashmap.hxx>

#include "SmPropertiesPanel.hxx"

namespace sm::sidebar
{
// static
std::unique_ptr<PanelLayout>
SmPropertiesPanel::Create(weld::Widget& rParent,
                          const css::uno::Reference<css::frame::XFrame>& xFrame)
{
    return std::make_unique<SmPropertiesPanel>(rParent, xFrame);
}

SmPropertiesPanel::SmPropertiesPanel(weld::Widget& rParent,
                                     const css::uno::Reference<css::frame::XFrame>& xFrame)
    : PanelLayout(&rParent, u"MathPropertiesPanel"_ustr,
                  u"modules/smath/ui/sidebarproperties_math.ui"_ustr)
    , mxFrame(xFrame)
    , mpFormatFontsButton(m_xBuilder->weld_button(u"btnFormatFonts"_ustr))
    , mpFormatFontSizeButton(m_xBuilder->weld_button(u"btnFormatFontSize"_ustr))
    , mpFormatSpacingButton(m_xBuilder->weld_button(u"btnFormatSpacing"_ustr))
    , mpFormatAlignmentButton(m_xBuilder->weld_button(u"btnFormatAlignment"_ustr))
    , maButtonCommands{ { mpFormatFontsButton.get(), ".uno:ChangeFont" },
                        { mpFormatFontSizeButton.get(), ".uno:ChangeFontSize" },
                        { mpFormatSpacingButton.get(), ".uno:ChangeDistance" },
                        { mpFormatAlignmentButton.get(), ".uno:ChangeAlignment" } }
{
    // Set localized labels to the buttons
    auto xConfs
        = css::frame::theUICommandDescription::get(comphelper::getProcessComponentContext());
    if (css::uno::Reference<css::container::XNameAccess> xConf{
            xConfs->getByName(u"com.sun.star.formula.FormulaProperties"_ustr),
            css::uno::UNO_QUERY })
    {
        for (const auto & [ button, command ] : maButtonCommands)
        {
            comphelper::SequenceAsHashMap props(xConf->getByName(command));
            button->set_label(props.getUnpackedValueOrDefault(u"Name"_ustr, button->get_label()));
        }
    }

    mpFormatFontsButton->connect_clicked(LINK(this, SmPropertiesPanel, ButtonClickHandler));
    mpFormatFontSizeButton->connect_clicked(LINK(this, SmPropertiesPanel, ButtonClickHandler));
    mpFormatSpacingButton->connect_clicked(LINK(this, SmPropertiesPanel, ButtonClickHandler));
    mpFormatAlignmentButton->connect_clicked(LINK(this, SmPropertiesPanel, ButtonClickHandler));
}

SmPropertiesPanel::~SmPropertiesPanel()
{
    maButtonCommands.clear();

    mpFormatFontsButton.reset();
    mpFormatFontSizeButton.reset();
    mpFormatSpacingButton.reset();
    mpFormatAlignmentButton.reset();
}

IMPL_LINK(SmPropertiesPanel, ButtonClickHandler, weld::Button&, rButton, void)
{
    if (OUString command = maButtonCommands[&rButton]; !command.isEmpty())
        comphelper::dispatchCommand(command, mxFrame, {});
}

} // end of namespace sm::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
