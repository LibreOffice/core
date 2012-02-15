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
#include "SlideTransitionPanel.hxx"

#include "taskpane/TaskPaneControlFactory.hxx"
#include "taskpane/ToolPanelViewShell.hxx"

#include "strings.hrc"
#include "sdresid.hxx"
#include <com/sun/star/frame/XModel.hpp>

namespace sd
{

    class ViewShellBase;
    extern ::Window * createSlideTransitionPanel( ::Window* pParent, ViewShellBase& rBase );

namespace toolpanel { namespace controls {



SlideTransitionPanel::SlideTransitionPanel(Window& i_rParentWindow, ToolPanelViewShell& i_rToolPanelShell)
    :SubToolPanel( i_rParentWindow )
    ,maPreferredSize( 100, 200 )
    ,m_pPanelViewShell( &i_rToolPanelShell )
{
    mpWrappedControl = createSlideTransitionPanel( &i_rParentWindow, i_rToolPanelShell.GetViewShellBase() );
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
