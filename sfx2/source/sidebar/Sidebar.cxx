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

#include "precompiled_sfx2.hxx"

#include "Sidebar.hxx"
#include "ResourceManager.hxx"

using namespace css;
using namespace cssu;

namespace sfx2 { namespace sidebar {

Sidebar::Sidebar(
    Window& rParentWindow,
    const Reference<frame::XFrame>& rxDocumentFrame)
    : Window(&rParentWindow, WB_DIALOGCONTROL)
{
    ContentPanelManager::Instance();
}




Sidebar::~Sidebar (void)
{
}




void Sidebar::Resize (void)
{
    Window::Resize();
    //    m_pImpl->OnResize();
}




void Sidebar::GetFocus (void)
{
    Window::GetFocus();
    //    m_pImpl->OnGetFocus();
}




void Sidebar::DataChanged (const DataChangedEvent& rDataChangedEvent)
{
    if  (rDataChangedEvent.GetType() == DATACHANGED_SETTINGS
        &&  (rDataChangedEvent.GetFlags() & SETTINGS_STYLE)!= 0)
    {
        Invalidate();
    }
    else
        Window::DataChanged(rDataChangedEvent);
}



} } // end of namespace sfx2::sidebar
