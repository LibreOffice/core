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



#include <sfx2/layout.hxx>

#include <com/sun/star/awt/PosSize.hpp>
#include <toolkit/awt/vclxwindow.hxx>

namespace layout
{

using namespace ::com::sun::star;

#if 0
SfxTabDialog::SfxTabDialog (::Window *parent, char const* xml_file, char const* id, ResId const& res_id, SfxItemSet const* set)
    : ::SfxTabDialog (parent, res_id, set)
    , InPlug (parent, xml_file, id)
{
}

::Window* SfxTabDialog::GetParent () const
{
    return Window::GetParent ();
}

::Window* SfxTabDialog::GetWindow () const
{
    return Window::GetParent ();
}

void SfxTabDialog::FreeResource ()
{
    //::Window::FreeResource ();
}
#endif

SfxTabPage::SfxTabPage (::Window *parent, char const* xml_file, char const* id, SfxItemSet const* set)
    : ::SfxTabPage (parent, 0, *set)
    , InPlug (parent, xml_file, id)
{
    dynamic_cast< ::Window* > (this)->SetComponentInterface (GetVCLXWindow ());
}

::Window* SfxTabPage::GetParent () const
{
    return Window::GetParent ();
}

::Window* SfxTabPage::GetWindow () const
{
    return ::layout::Window::GetWindow();
}

void SfxTabPage::FreeResource ()
{
    //::Window::FreeResource ();
}

Size SfxTabPage::GetOptimalSize (WindowSizeType) const
{
    awt::Size s = GetVCLXWindow()->getMinimumSize ();
    return Size (s.Width, s.Height);
}

} // end namespace layout
