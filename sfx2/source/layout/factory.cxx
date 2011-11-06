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



#include <com/sun/star/awt/WindowAttribute.hpp>
#include <sal/types.h>
#include <sfx2/basedlgs.hxx>
#include <toolkit/awt/vclxtoolkit.hxx>
#include <toolkit/awt/vclxdialog.hxx>

using rtl::OUString;

class SfxModelessDialog_unprotect : public SfxModelessDialog
{
public:
    SfxModelessDialog_unprotect (Window *parent, WinBits attributes);
};

SFX2_DLLPUBLIC SfxBindings* sfx2_global_bindings;
SFX2_DLLPUBLIC SfxChildWindow* sfx2_global_child;

SfxModelessDialog_unprotect::SfxModelessDialog_unprotect (Window *parent, WinBits bits)
    : SfxModelessDialog (sfx2_global_bindings, sfx2_global_child, parent, bits)
{
}

extern "C"
{

SAL_DLLPUBLIC_EXPORT Window* CreateWindow (VCLXWindow** vcl, OUString const& name, Window* parent, long& attributes)
{
    Window* window = 0;
    if (0)
    {
        ;
    }
    else if (name.equalsAscii ("sfxmodelessdialog"))
    {
        window = new SfxModelessDialog_unprotect (parent, ImplGetWinBits (attributes, 0));
        *vcl = new layoutimpl::VCLXDialog ();
    }
    return window;
}

}
