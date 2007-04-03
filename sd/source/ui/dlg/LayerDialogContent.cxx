/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: LayerDialogContent.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-03 15:39:50 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "LayerDialogContent.hxx"

#ifndef _COM_SUN_STAR_PRESENTATION_ANIMATIONEFFECT_HPP_
#include <com/sun/star/presentation/AnimationEffect.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_ANIMATIONSPEED_HPP_
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#endif

#define ITEMID_COLOR SID_COLOR_TABLE

#include <svx/gallery.hxx>
#include <svx/colritem.hxx>
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#include "sdattr.hxx"

#include "LayerDialog.hrc"
#include "app.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "sdresid.hxx"
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
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




LayerTabBar& LayerDialogContent::GetLayerTabBar (void)
{
    return maLayerTabBar;
}




BOOL LayerDialogContent::Close (void)
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
