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

#include "precompiled_svx.hxx"

#include "NotYetImplementedPanel.hxx"

namespace svx { namespace sidebar {

NotYetImplementedPanel::NotYetImplementedPanel (::Window* pParent)
    : Window(pParent, 0),
      maMessageControl(this, 0)
{
    maMessageControl.SetPosSizePixel(5,5, 250,15);
    maMessageControl.SetText(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("not yet implemented")));

    maMessageControl.Show();
    Show();
}




NotYetImplementedPanel::~NotYetImplementedPanel (void)
{
}




} } // end of namespace ::svx::sidebar
