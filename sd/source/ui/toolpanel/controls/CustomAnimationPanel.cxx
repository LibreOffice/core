/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "CustomAnimationPanel.hxx"

#include "taskpane/TaskPaneControlFactory.hxx"
#include "taskpane/ToolPanelViewShell.hxx"

#include "strings.hrc"
#include "sdresid.hxx"

namespace sd
{

    class ViewShellBase;
    extern ::Window * createCustomAnimationPanel( ::Window* pParent, ViewShellBase& rBase );

namespace toolpanel { namespace controls {


CustomAnimationPanel::CustomAnimationPanel(Window& i_rParentWindow, ToolPanelViewShell& i_rPanelViewShell)
    :SubToolPanel( i_rParentWindow )
    ,m_pPanelViewShell( &i_rPanelViewShell )
{
    mpWrappedControl = createCustomAnimationPanel( &i_rParentWindow, i_rPanelViewShell.GetViewShellBase() );
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
