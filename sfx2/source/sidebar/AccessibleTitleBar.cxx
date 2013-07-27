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

#include "AccessibleTitleBar.hxx"
#include "Accessible.hxx"
#include "TitleBar.hxx"

#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <unotools/accessiblestatesethelper.hxx>

using namespace css;
using namespace cssu;

namespace sfx2 { namespace sidebar {


Reference<accessibility::XAccessible> AccessibleTitleBar::Create (TitleBar& rTitleBar)
{
    rTitleBar.GetComponentInterface(sal_True);
    VCLXWindow* pWindow = rTitleBar.GetWindowPeer();
    if (pWindow != NULL)
        return new Accessible(new AccessibleTitleBar(pWindow));
    else
        return NULL;
}




AccessibleTitleBar::AccessibleTitleBar (VCLXWindow* pWindow)
    : VCLXAccessibleComponent(pWindow)
{
}




AccessibleTitleBar::~AccessibleTitleBar (void)
{
}




void AccessibleTitleBar::FillAccessibleStateSet (utl::AccessibleStateSetHelper& rStateSet)
{
    VCLXAccessibleComponent::FillAccessibleStateSet(rStateSet);
    rStateSet.AddState(accessibility::AccessibleStateType::FOCUSABLE);
}


} } // end of namespace sfx2::sidebar
