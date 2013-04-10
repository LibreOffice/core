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

#include "precompiled_sd.hxx"

#include "framework/TaskPanelResource.hxx"

#include <vcl/window.hxx>
#include <toolkit/helper/vclunohelper.hxx>


using namespace css;
using namespace cssu;
using namespace cssdf;


namespace sd { namespace framework {

namespace {
    ::Window* GetWindowForResource (
        ViewShellBase& rViewShellBase,
        const cssu::Reference<cssdf::XResourceId>& rxResourceId)
    {
        ::Window* pWindow = NULL;
        if (rxResourceId.is() && rxResourceId->getAnchor().is())
        {
            ::boost::shared_ptr<FrameworkHelper> pFrameworkHelper (FrameworkHelper::Instance(rViewShellBase));
            Reference<awt::XWindow> xWindow (
                pFrameworkHelper->GetPaneWindow(rxResourceId->getAnchor()->getAnchor()));
            pWindow = VCLUnoHelper::GetWindow(xWindow);
        }
        return pWindow;
    }
}




TaskPanelResource::TaskPanelResource (
    sidebar::SidebarViewShell& rSidebarViewShell,
    sidebar::PanelId ePanelId,
    const Reference<XResourceId>& rxResourceId)
    : TaskPanelResourceInterfaceBase(m_aMutex),
      mxResourceId(rxResourceId),
      mpControl(rSidebarViewShell.CreatePanel(
              GetWindowForResource(rSidebarViewShell.GetViewShellBase(), rxResourceId),
              ePanelId))
{
    if (mpControl.get() != NULL)
    {
        mpControl->Show();
        mpControl->GetParent()->Show();
        mpControl->AddEventListener(LINK(this,TaskPanelResource,WindowEventHandler));
    }
}




TaskPanelResource::~TaskPanelResource (void)
{
    mpControl.reset();
}




void SAL_CALL TaskPanelResource::disposing ()
{
    mpControl.reset();
}




Reference<XResourceId> SAL_CALL TaskPanelResource::getResourceId ()
    throw (css::uno::RuntimeException)
{
    return mxResourceId;
}




sal_Bool SAL_CALL TaskPanelResource::isAnchorOnly (void)
    throw (RuntimeException)
{
    return false;
}




::Window* TaskPanelResource::GetControl (void) const
{
    return mpControl.get();
}




IMPL_LINK(TaskPanelResource,WindowEventHandler,VclWindowEvent*,pEvent)
{
    if (pEvent!=NULL && pEvent->GetId()==SFX_HINT_DYING)
    {
        // Somebody else deleted the window.  Release our reference so
        // that we do not delete it again.
        mpControl.release();
        return sal_True;
    }
    else
        return sal_False;
}

} } // end of namespace sd::framework
