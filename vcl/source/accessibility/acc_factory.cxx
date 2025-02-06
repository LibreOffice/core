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

#include <config_feature_desktop.h>

#include <cppuhelper/supportsservice.hxx>
#include <vcl/accessiblefactory.hxx>
#include <accessibility/floatingwindowaccessible.hxx>
#include <accessibility/svtaccessiblenumericfield.hxx>
#include <accessibility/vclxaccessiblebutton.hxx>
#include <accessibility/vclxaccessiblecheckbox.hxx>
#include <accessibility/vclxaccessibledropdowncombobox.hxx>
#include <accessibility/vclxaccessiblecombobox.hxx>
#include <accessibility/vclxaccessibledropdownlistbox.hxx>
#include <accessibility/vclxaccessibleedit.hxx>
#include <accessibility/vclxaccessiblefixedhyperlink.hxx>
#include <accessibility/vclxaccessiblefixedtext.hxx>
#include <accessibility/vclxaccessibleheaderbar.hxx>
#include <accessibility/vclxaccessiblelistbox.hxx>
#include <accessibility/vclxaccessibleradiobutton.hxx>
#include <accessibility/vclxaccessiblescrollbar.hxx>
#include <accessibility/vclxaccessibletoolbox.hxx>
#include <accessibility/vclxaccessiblestatusbar.hxx>
#include <accessibility/vclxaccessibletabcontrol.hxx>
#include <accessibility/vclxaccessibletabpagewindow.hxx>
#include <accessibility/accessibletablistbox.hxx>
#include <accessibility/accessibleiconchoicectrl.hxx>
#include <accessibility/AccessibleIconView.hxx>
#include <accessibility/accessiblelistbox.hxx>
#include <accessibility/textwindowaccessibility.hxx>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <vcl/toolkit/lstbox.hxx>
#include <vcl/toolkit/combobox.hxx>
#include <vcl/toolkit/fixed.hxx>
#include <vcl/toolkit/fixedhyper.hxx>
#include <vcl/accessibility/vclxaccessiblecomponent.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::accessibility;
using namespace ::vcl;

namespace {

bool hasFloatingChild(vcl::Window *pWindow)
{
    vcl::Window * pChild = pWindow->GetAccessibleChildWindow(0);
    return pChild && pChild->GetType() == WindowType::FLOATINGWINDOW;
}
};

Reference< XAccessibleContext > AccessibleFactory::createAccessibleContext(PushButton* pButton)
{
    return new VCLXAccessibleButton(pButton);
}

Reference< XAccessibleContext > AccessibleFactory::createAccessibleContext(CheckBox* pCheckBox)
{
    return new VCLXAccessibleCheckBox(pCheckBox);
}

Reference<XAccessibleContext> AccessibleFactory::createAccessibleContext(RadioButton* pRadioButton)
{
    return new VCLXAccessibleRadioButton(pRadioButton);
}

Reference< XAccessibleContext > AccessibleFactory::createAccessibleContext(ListBox* pListBox)
{
    bool bIsDropDownBox = false;
    if (pListBox)
        bIsDropDownBox = ((pListBox->GetStyle() & WB_DROPDOWN) == WB_DROPDOWN);

    if ( bIsDropDownBox )
        return new VCLXAccessibleDropDownListBox(pListBox);
    else
        return new VCLXAccessibleListBox(pListBox);
}

Reference< XAccessibleContext > AccessibleFactory::createAccessibleContext(FixedText* pFixedText)
{
    return new VCLXAccessibleFixedText(pFixedText);
}

Reference< XAccessibleContext > AccessibleFactory::createAccessibleContext(FixedHyperlink* pFixedHyperlink)
{
    return new VCLXAccessibleFixedHyperlink(pFixedHyperlink);
}

Reference< XAccessibleContext > AccessibleFactory::createAccessibleContext(ScrollBar* pScrollBar)
{
    return new VCLXAccessibleScrollBar(pScrollBar);
}

Reference< XAccessibleContext > AccessibleFactory::createAccessibleContext(Edit* pEdit)
{
    return new VCLXAccessibleEdit(pEdit);
}

Reference< XAccessibleContext > AccessibleFactory::createAccessibleContext(ComboBox* pComboBox)
{
    bool bIsDropDownBox = false;
    if (pComboBox)
        bIsDropDownBox = ((pComboBox->GetStyle() & WB_DROPDOWN) == WB_DROPDOWN);

    if ( bIsDropDownBox )
        return new VCLXAccessibleDropDownComboBox(pComboBox);
    else
        return new VCLXAccessibleComboBox(pComboBox);
}

Reference< XAccessibleContext > AccessibleFactory::createAccessibleContext(vcl::Window* pWindow)
{
    if (!pWindow)
        return nullptr;

    WindowType nType = pWindow->GetType();

    if ( nType == WindowType::STATUSBAR )
    {
        return new VCLXAccessibleStatusBar(pWindow);
    }

    else if ( nType == WindowType::TABCONTROL )
    {
        return new VCLXAccessibleTabControl(pWindow);
    }

    else if ( nType == WindowType::TABPAGE && pWindow->GetAccessibleParentWindow() && pWindow->GetAccessibleParentWindow()->GetType() == WindowType::TABCONTROL )
    {
        return new VCLXAccessibleTabPageWindow(pWindow);
    }

    else if ( nType == WindowType::FLOATINGWINDOW )
    {
        return new FloatingWindowAccessible(pWindow);
    }

    else if ( nType == WindowType::BORDERWINDOW && hasFloatingChild( pWindow ) )
    {
        return new FloatingWindowAccessible(pWindow);
    }

    else if ( ( nType == WindowType::HELPTEXTWINDOW ) || ( nType == WindowType::FIXEDLINE ) )
    {
        return new VCLXAccessibleFixedText(pWindow);
    }
    else
    {
        return new VCLXAccessibleComponent(pWindow);
    }
}

Reference< XAccessibleContext > AccessibleFactory::createAccessibleContext(ToolBox* pToolBox)
{
    return new VCLXAccessibleToolBox(pToolBox);
}

Reference< XAccessibleContext > AccessibleFactory::createAccessibleContext(HeaderBar* pHeaderBar)
{
    return new VCLXAccessibleHeaderBar(pHeaderBar);
}

Reference< XAccessibleContext > AccessibleFactory::createAccessibleContext(FormattedField* pFormattedField)
{
    return new SVTXAccessibleNumericField(pFormattedField);
}

Reference<XAccessible>
AccessibleFactory::createAccessibleIconChoiceCtrl(SvtIconChoiceCtrl& _rIconCtrl,
                                                  const Reference<XAccessible>& _xParent)
{
    return new AccessibleIconChoiceCtrl( _rIconCtrl, _xParent );
}

Reference<XAccessibleContext>
AccessibleFactory::createAccessibleTextWindowContext(vcl::Window* pWindow, TextEngine& rEngine,
                                                     TextView& rView)
{
    return new Document(pWindow, rEngine, rView );
}

Reference<XAccessible>
AccessibleFactory::createAccessibleTreeListBox(SvTreeListBox& _rListBox,
                                               const Reference<XAccessible>& _xParent)
{
    return new AccessibleListBox( _rListBox, _xParent );
}

Reference<XAccessible>
AccessibleFactory::createAccessibleIconView(SvTreeListBox& _rListBox,
                                            const Reference<XAccessible>& _xParent)
{
    return new AccessibleIconView( _rListBox, _xParent );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
