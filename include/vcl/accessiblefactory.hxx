/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#pragma once

#include <vcl/dllapi.h>

#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <salhelper/simplereferenceobject.hxx>
#include <tools/gen.hxx>
#include <rtl/ref.hxx>

namespace com::sun::star {
    namespace accessibility {
        class XAccessible;
        class XAccessibleContext;
    }
    namespace awt {
        class XWindow;
    }
}

namespace vcl { class Window; }

class CheckBox;
class ComboBox;
class Edit;
class FixedHyperlink;
class FixedText;
class FormattedField;
class HeaderBar;
class ListBox;
class PushButton;
class RadioButton;
class ScrollBar;
class SvHeaderTabListBox;
class ToolBox;
class SvTreeListBox;
class TextEngine;
class TextView;

class VCL_DLLPUBLIC AccessibleFactory
{
public:
    AccessibleFactory() = delete;

    static css::uno::Reference<css::accessibility::XAccessibleContext>
    createAccessibleContext(PushButton* pButton);
    static css::uno::Reference<css::accessibility::XAccessibleContext>
    createAccessibleContext(CheckBox* pCheckBox);
    static css::uno::Reference<css::accessibility::XAccessibleContext>
    createAccessibleContext(RadioButton* pRadioButton);
    static css::uno::Reference<css::accessibility::XAccessibleContext>
    createAccessibleContext(ListBox* pListBox);
    static css::uno::Reference<css::accessibility::XAccessibleContext>
    createAccessibleContext(FixedText* pFixedText);
    static css::uno::Reference<css::accessibility::XAccessibleContext>
    createAccessibleContext(FixedHyperlink* pFixedHyperlink);
    static css::uno::Reference<css::accessibility::XAccessibleContext>
    createAccessibleContext(ScrollBar* pScrollBar);
    static css::uno::Reference<css::accessibility::XAccessibleContext>
    createAccessibleContext(Edit* pEdit);
    static css::uno::Reference<css::accessibility::XAccessibleContext>
    createAccessibleContext(ComboBox* pComboBox);
    static css::uno::Reference<css::accessibility::XAccessibleContext>
    createAccessibleContext(ToolBox* pToolBox);
    static css::uno::Reference<css::accessibility::XAccessibleContext>
    createAccessibleContext(HeaderBar* pHeaderBar);
    static css::uno::Reference<css::accessibility::XAccessibleContext>
    createAccessibleContext(FormattedField* pFormattedField);
    static css::uno::Reference<css::accessibility::XAccessibleContext>
    createAccessibleContext(vcl::Window* pWindow);

    static css::uno::Reference<css::accessibility::XAccessibleContext>
    createAccessibleTextWindowContext(vcl::Window* pWindow, TextEngine& rEngine, TextView& rView);

    static css::uno::Reference<css::accessibility::XAccessible>
    createAccessibleIconView(SvTreeListBox& _rListBox,
                             const css::uno::Reference<css::accessibility::XAccessible>& _xParent);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
