/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
