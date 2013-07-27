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

#include "ContextPanel.hxx"

namespace svx { namespace sidebar {

ContextPanel::ContextPanel (::Window* pParent)
    : Window(pParent, 0),
      maApplicationName(this, 0),
      maContextName(this, 0)
{
    maApplicationName.SetPosSizePixel(5,5, 250,15);
    maContextName.SetPosSizePixel(5,25, 250,15);

    maApplicationName.Show();
    maContextName.Show();
    Show();
}




ContextPanel::~ContextPanel (void)
{
}




sal_Int32 ContextPanel::GetPreferredHeight (sal_Int32 /* nWidth */)
{
    const sal_Int32 nHeight (maContextName.GetPosPixel().Y() + maContextName.GetSizePixel().Height() + 5);
    return nHeight;
}




void ContextPanel::HandleContextChange (const sfx2::sidebar::EnumContext aContext)
{
    maApplicationName.SetText(aContext.GetApplicationName());
    maContextName.SetText(aContext.GetContextName());
}




} } // end of namespace ::svx::sidebar
