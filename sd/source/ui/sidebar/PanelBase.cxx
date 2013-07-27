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

#include "TableDesignPanel.hxx"



namespace sd { namespace sidebar {


PanelBase::PanelBase (
    ::Window* pParentWindow,
    ViewShellBase& rViewShellBase)
    : Control(pParentWindow),
      mpWrappedControl(NULL),
      mxSidebar(),
      mrViewShellBase(rViewShellBase)
{
    OSL_TRACE("created PanelBase at %x for parent %x", this, pParentWindow);

#ifdef DEBUG
    SetText(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sd:PanelBase")));
#endif
}




PanelBase::~PanelBase (void)
{
    OSL_TRACE("deleting wrapped control at %x", mpWrappedControl.get());
    mpWrappedControl.reset();
    OSL_TRACE("deleting PanelBase at %x from parent %x", this, GetParent());
}





void PanelBase::Dispose (void)
{
    OSL_TRACE("PanelBase::DisposeL: deleting wrapped control at %x", mpWrappedControl.get());
    mpWrappedControl.reset();
}




css::ui::LayoutSize PanelBase::GetHeightForWidth (const sal_Int32 nWidth)
{
    sal_Int32 nHeight (0);
    if (ProvideWrappedControl())
        nHeight = mpWrappedControl->GetSizePixel().Height();
    return css::ui::LayoutSize(nHeight,nHeight,nHeight);
}




void PanelBase::Resize (void)
{
    if (ProvideWrappedControl())
    {
        Size aNewSize (GetSizePixel());
        mpWrappedControl->SetOutputSizePixel(aNewSize);
    }
}




::com::sun::star::uno::Reference<
    ::com::sun::star::accessibility::XAccessible> PanelBase::CreateAccessibleObject (
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible>& )
{
    if (ProvideWrappedControl())
        return mpWrappedControl->GetAccessible();
    else
        return NULL;
}




void PanelBase::SetSidebar (const cssu::Reference<css::ui::XSidebar>& rxSidebar)
{
    mxSidebar = rxSidebar;
    if (mxSidebar.is() && mpWrappedControl!=NULL)
        mxSidebar->requestLayout();
}




bool PanelBase::ProvideWrappedControl (void)
{
    if ( ! mpWrappedControl)
    {
        mpWrappedControl.reset(CreateWrappedControl(this, mrViewShellBase));
        OSL_TRACE("created wrapped control at %x for parent PanelBase at %x", mpWrappedControl.get(), this);
        if (mpWrappedControl)
            mpWrappedControl->Show();
        if (mxSidebar.is())
            mxSidebar->requestLayout();
    }
    return mpWrappedControl.get() != NULL;
}

} } // end of namespace sd::sidebar
