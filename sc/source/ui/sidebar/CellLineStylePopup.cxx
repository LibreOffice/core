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

#include "precompiled_sc.hxx"

#include "CellLineStylePopup.hxx"
#include "CellLineStyleControl.hxx"

namespace sc { namespace sidebar {

CellLineStylePopup::CellLineStylePopup (
    Window* pParent,
    const ::boost::function<svx::sidebar::PopupControl*(svx::sidebar::PopupContainer*)>& rControlCreator)
:   Popup(pParent, rControlCreator,  ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CellLineStyle")))
{
}

CellLineStylePopup::~CellLineStylePopup (void)
{
}

void CellLineStylePopup::SetLineStyleSelect(sal_uInt16 out, sal_uInt16 in, sal_uInt16 dis)
{
    ProvideContainerAndControl();

    CellLineStyleControl* pControl = dynamic_cast< CellLineStyleControl* >(mpControl.get());

    if(pControl)
    {
        pControl->SetLineStyleSelect(out, in, dis);
    }
}

} } // end of namespace sc::sidebar

// eof
