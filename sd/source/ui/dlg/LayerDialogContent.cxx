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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "LayerDialogContent.hxx"
#include <com/sun/star/presentation/AnimationEffect.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>

#include <svx/gallery.hxx>
#include <editeng/colritem.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/aeitem.hxx>
#include <vcl/msgbox.hxx>

#include "sdattr.hxx"

#include "LayerDialog.hrc"
#include "app.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "sdresid.hxx"
#include "View.hxx"
#include "drawdoc.hxx"
#include "ViewShellBase.hxx"
#include "DrawViewShell.hxx"
#include "framework/FrameworkHelper.hxx"

using namespace ::com::sun::star;

namespace sd {



LayerDialogContent::LayerDialogContent (
    SfxBindings* pInBindings,
    SfxChildWindow *pCW,
    Window* pParent,
    const SdResId& rSdResId,
    ViewShellBase& rBase)
    : SfxDockingWindow (pInBindings, pCW, pParent, rSdResId),
      maLayerTabBar(
          dynamic_cast<DrawViewShell*>(
              framework::FrameworkHelper::Instance(rBase)->GetViewShell(
                  framework::FrameworkHelper::msCenterPaneURL).get()),
          this,
          SdResId(TB_LAYERS))
{
    FreeResource();

    maLayerTabBar.Show();
}




LayerDialogContent::~LayerDialogContent (void)
{
}




sal_Bool LayerDialogContent::Close (void)
{
    return SfxDockingWindow::Close();
}




void LayerDialogContent::Resize (void)
{
    maLayerTabBar.SetPosSizePixel (
        Point(0,0),
        Size(GetSizePixel().Width(), 17));
    SfxDockingWindow::Resize();
}


} // end of namespace sd
