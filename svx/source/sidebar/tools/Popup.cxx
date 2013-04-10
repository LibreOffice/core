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

#include "svx/sidebar/Popup.hxx"
#include "svx/sidebar/PopupContainer.hxx"
#include "svx/sidebar/PopupControl.hxx"

#include <vcl/toolbox.hxx>


namespace svx { namespace sidebar {

Popup::Popup (
    Window* pParent,
    const ::boost::function<PopupControl*(PopupContainer*)>& rControlCreator,
    const ::rtl::OUString& rsAccessibleName)
    : mpControl(),
      mpParent(pParent),
      maControlCreator(rControlCreator),
      maPopupModeEndCallback(),
      msAccessibleName(rsAccessibleName),
      mpContainer()
{
    OSL_ASSERT(mpParent!=NULL);
    OSL_ASSERT(maControlCreator);
}




Popup::~Popup (void)
{
    mpControl.reset();
    mpContainer.reset();
}




void Popup::Show (ToolBox& rToolBox)
{
    rToolBox.SetItemDown(rToolBox.GetCurItemId(), true);

    ProvideContainerAndControl();
    if ( ! (mpContainer && mpControl))
    {
        OSL_ASSERT(mpContainer);
        OSL_ASSERT(mpControl);
        return;
    }

    if ( !mpContainer->IsInPopupMode() )
    {
        mpContainer->SetSizePixel(mpControl->GetOutputSizePixel());

        const Point aPos (mpParent->OutputToScreenPixel(rToolBox.GetPosPixel()));
        const Size aSize (rToolBox.GetSizePixel());
        const Rectangle aRect (aPos, aSize);

        mpContainer->StartPopupMode(
            aRect,
            FLOATWIN_POPUPMODE_NOFOCUSCLOSE|FLOATWIN_POPUPMODE_DOWN);
        mpContainer->SetPopupModeFlags(
            mpContainer->GetPopupModeFlags()
                | FLOATWIN_POPUPMODE_NOAPPFOCUSCLOSE);

        mpControl->GetFocus();
    }
}




void Popup::Hide (void)
{
    if (mpContainer)
        if (mpContainer->IsInPopupMode())
            mpContainer->EndPopupMode();
}




void Popup::SetPopupModeEndHandler (const ::boost::function<void(void)>& rCallback)
{
    maPopupModeEndCallback = rCallback;
    if (mpContainer)
        mpContainer->SetPopupModeEndHdl(LINK(this, Popup, PopupModeEndHandler));
}




void Popup::ProvideContainerAndControl (void)
{
    if ( ! (mpContainer && mpControl)
        && mpParent!=NULL
        && maControlCreator)
    {
        CreateContainerAndControl();
    }
}




void Popup::CreateContainerAndControl (void)
{
    mpContainer.reset(new PopupContainer(mpParent));
    mpContainer->SetAccessibleName(msAccessibleName);
    if (maPopupModeEndCallback)
        mpContainer->SetPopupModeEndHdl(LINK(this, Popup, PopupModeEndHandler));
    mpContainer->SetBorderStyle(mpContainer->GetBorderStyle() | WINDOW_BORDER_MENU);

    mpControl.reset(maControlCreator(mpContainer.get()));
}




IMPL_LINK(Popup, PopupModeEndHandler, void*, EMPTYARG)
{
    if (maPopupModeEndCallback)
        maPopupModeEndCallback();
    return 0;
}



} } // end of namespace svx::sidebar
