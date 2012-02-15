/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
