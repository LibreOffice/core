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

#include "LineWidthPopup.hxx"
#include "LineWidthControl.hxx"
#include "LinePropertyPanel.hxx"

#include <unotools/viewoptions.hxx>

#include <boost/bind.hpp>


namespace svx { namespace sidebar {

LineWidthPopup::LineWidthPopup (
    Window* pParent,
    const ::boost::function<PopupControl*(PopupContainer*)>& rControlCreator)
    : Popup(
        pParent,
        rControlCreator,
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Width")))
{
    SetPopupModeEndHandler(::boost::bind(&LineWidthPopup::PopupModeEndCallback, this));
}




LineWidthPopup::~LineWidthPopup (void)
{
}




void LineWidthPopup::SetWidthSelect (long lValue, bool bValuable, SfxMapUnit eMapUnit)
{
    ProvideContainerAndControl();

    LineWidthControl* pControl = dynamic_cast<LineWidthControl*>(mpControl.get());
    if (pControl != NULL)
        pControl->SetWidthSelect(lValue, bValuable, eMapUnit);
}




void LineWidthPopup::PopupModeEndCallback (void)
{
    LineWidthControl* pControl = dynamic_cast<LineWidthControl*>(mpControl.get());
    if (pControl != NULL)
    {
        if (pControl->IsCloseByEdit())
        {
            SvtViewOptions aWinOpt( E_WINDOW, SIDEBAR_LINE_WIDTH_GLOBAL_VALUE );
            ::com::sun::star::uno::Sequence < ::com::sun::star::beans::NamedValue > aSeq(1);
            aSeq[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("LineWidth") );
            aSeq[0].Value <<= ::rtl::OUString(
                String::CreateFromInt32(pControl->GetTmpCustomWidth()));
            aWinOpt.SetUserData( aSeq );
        }
    }
}



} } // end of namespace svx::sidebar


// eof
