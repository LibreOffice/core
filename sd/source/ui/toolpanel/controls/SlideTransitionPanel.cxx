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

#include "SlideTransitionPanel.hxx"

#include "taskpane/TaskPaneControlFactory.hxx"
#include "taskpane/ToolPanelViewShell.hxx"

#include "strings.hrc"
#include "sdresid.hxx"
#include <com/sun/star/frame/XModel.hpp>

namespace sd
{

    class ViewShellBase;
    extern ::Window * createSlideTransitionPanel( ::Window* pParent, ViewShellBase& rBase,
                                                  const cssu::Reference<css::frame::XFrame>& rxFrame );

namespace toolpanel { namespace controls {



SlideTransitionPanel::SlideTransitionPanel(Window& i_rParentWindow, ToolPanelViewShell& i_rToolPanelShell)
    :SubToolPanel( i_rParentWindow )
    ,maPreferredSize( 100, 200 )
    ,m_pPanelViewShell( &i_rToolPanelShell )
{
    mpWrappedControl = createSlideTransitionPanel( &i_rParentWindow, i_rToolPanelShell.GetViewShellBase(), NULL );
    mpWrappedControl->Show();
}

SlideTransitionPanel::~SlideTransitionPanel()
{
    delete mpWrappedControl;
}

std::auto_ptr< ControlFactory > SlideTransitionPanel::CreateControlFactory( ToolPanelViewShell& i_rToolPanelShell )
{
    return std::auto_ptr< ControlFactory >(
        new RootControlFactoryWithArg< SlideTransitionPanel, ToolPanelViewShell >( i_rToolPanelShell ) );
}

TaskPaneShellManager* SlideTransitionPanel::GetShellManager()
{
    if ( m_pPanelViewShell )
        return &m_pPanelViewShell->GetSubShellManager();
    return SubToolPanel::GetShellManager();
}

Size SlideTransitionPanel::GetPreferredSize()
{
    return maPreferredSize;
}
sal_Int32 SlideTransitionPanel::GetPreferredWidth(sal_Int32 )
{
    return maPreferredSize.Width();
}
sal_Int32 SlideTransitionPanel::GetPreferredHeight(sal_Int32 )
{
    return maPreferredSize.Height();
}
::Window* SlideTransitionPanel::GetWindow()
{
    return mpWrappedControl;
}
bool SlideTransitionPanel::IsResizable()
{
    return true;
}
bool SlideTransitionPanel::IsExpandable() const
{
    return true;
}




::com::sun::star::uno::Reference<
    ::com::sun::star::accessibility::XAccessible> SlideTransitionPanel::CreateAccessibleObject (
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
