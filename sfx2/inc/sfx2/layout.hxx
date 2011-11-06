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



#ifndef _SFX2_LAYOUT_HXX
#define _SFX2_LAYOUT_HXX

#include <layout/layout.hxx>
#include <sfx2/tabdlg.hxx>
#if ENABLE_LAYOUT
#undef ENABLE_LAYOUT_SFX_TABDIALOG
#define ENABLE_LAYOUT_SFX_TABDIALOG 1
#include <sfx2/layout-tabdlg.hxx>
#endif

class SfxChildWindow;
class SfxBindings;

namespace layout
{

class SFX2_DLLPUBLIC SfxDialog
    : public Dialog
{
public:
    SfxDialog (::Window* parent, char const* xml_file, char const* id, SfxBindings* bindings=0, SfxChildWindow* child=0);
};

#if 0
class SFX2_DLLPUBLIC SfxTabDialog
    : public ::SfxTabDialog
    , public InPlug
{
public:
    SfxTabDialog (::Window *parent, char const* xml_file, char const* id, ResId const& res_id, SfxItemSet const* set=0);
    ::Window* GetParent () const;
    void FreeResource ();
};
#endif

class SFX2_DLLPUBLIC SfxTabPage
    : public ::SfxTabPage
    , public InPlug
{
public:
    SfxTabPage (::Window *parent, char const* xml_file, char const* id, SfxItemSet const* set=0);
    ::Window* GetParent () const;
    using ::Window::GetWindow;
    ::Window* GetWindow () const;
    Size GetOptimalSize (WindowSizeType eType) const;
    void FreeResource ();
};
} // end namespace layout

#endif /* _SFX2_LAYOUT_HXX */
