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


#include "CustomAnimationPanel.hxx"

#include "taskpane/TaskPaneControlFactory.hxx"
#include "taskpane/ToolPanelViewShell.hxx"

#include "strings.hrc"
#include "sdresid.hxx"

namespace sd
{

    class ViewShellBase;
    extern ::Window * createCustomAnimationPanel( ::Window* pParent, ViewShellBase& rBase,
                                                  const cssu::Reference<css::frame::XFrame>& rxFrame );

namespace toolpanel { namespace controls {


CustomAnimationPanel::CustomAnimationPanel(Window& i_rParentWindow, ToolPanelViewShell& i_rPanelViewShell)
    :SubToolPanel( i_rParentWindow )
    ,m_pPanelViewShell( &i_rPanelViewShell )
{
    mpWrappedControl = createCustomAnimationPanel( &i_rParentWindow, i_rPanelViewShell.GetViewShellBase(), NULL );
    mpWrappedControl->Show();
}

CustomAnimationPanel::~CustomAnimationPanel()
{
    delete mpWrappedControl;
}

std::auto_ptr< ControlFactory > CustomAnimationPanel::CreateControlFactory( ToolPanelViewShell& i_rToolPanelShell )
{
    return std::auto_ptr< ControlFactory >(
        new RootControlFactoryWithArg< CustomAnimationPanel, ToolPanelViewShell >( i_rToolPanelShell ) );
}

TaskPaneShellManager* CustomAnimationPanel::GetShellManager()
{
    if ( m_pPanelViewShell )
        return &m_pPanelViewShell->GetSubShellManager();
    return SubToolPanel::GetShellManager();
}

Size CustomAnimationPanel::GetPreferredSize()
{
    return maPreferredSize;
}
sal_Int32 CustomAnimationPanel::GetPreferredWidth(sal_Int32 )
{
    return maPreferredSize.Width();
}
sal_Int32 CustomAnimationPanel::GetPreferredHeight(sal_Int32 )
{
    return maPreferredSize.Height();
}
::Window* CustomAnimationPanel::GetWindow()
{
    return mpWrappedControl;
}
bool CustomAnimationPanel::IsResizable()
{
    return true;
}
bool CustomAnimationPanel::IsExpandable() const
{
    return true;
}




::com::sun::star::uno::Reference<
    ::com::sun::star::accessibility::XAccessible> CustomAnimationPanel::CreateAccessibleObject (
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible>& )
{
    if (GetWindow() != NULL)
        return GetWindow()->GetAccessible();
    else
        return NULL;
}

} } } // end of namespace ::sd::toolpanel::controls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
