/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <editeng/editdata.hxx>
#include <editeng/outliner.hxx>
#include <svx/svxids.hrc>
#include <editeng/eeitem.hxx>

#include <editeng/flditem.hxx>
#include <svx/imapdlg.hxx>
#include <svx/bmpmask.hxx>
#include <svx/galbrws.hxx>
#include <svx/srchdlg.hxx>
#include <svx/fontwork.hxx>
#include <svx/SvxColorChildWindow.hxx>
#include <svx/verttexttbxctrl.hxx>
#include <svx/hyperdlg.hxx>
#include <svx/fillctrl.hxx>
#include <svx/linectrl.hxx>
#include <svx/tbcontrl.hxx>
#include <svx/zoomctrl.hxx>
#include <svx/zoomsliderctrl.hxx>
#include <svx/pszctrl.hxx>
#include <svx/modctrl.hxx>
#include <svx/fntctl.hxx>
#include <svx/fntszctl.hxx>
#include <svx/f3dchild.hxx>
#include <svx/grafctrl.hxx>
#include <svx/tbxcustomshapes.hxx>
#include <svx/lboxctrl.hxx>
#include <svx/clipboardctl.hxx>
#include <svx/extrusioncolorcontrol.hxx>
#include <svx/fontworkgallery.hxx>
#include <svx/tbxcolor.hxx>
#include <avmedia/mediaplayer.hxx>
#include <avmedia/mediatoolbox.hxx>

#include <svx/layctrl.hxx>
#include <svx/subtoolboxcontrol.hxx>
#include <sfx2/sidebar/SidebarChildWindow.hxx>

#include "sddll.hxx"
#include "sdmod.hxx"
#include "diactrl.hxx"
#include "gluectrl.hxx"
#include "tbx_ww.hxx"
#include "AnimationChildWindow.hxx"
#include "animobjs.hxx"
#include "NavigatorChildWindow.hxx"
#include "LayerDialogChildWindow.hxx"
#include "app.hrc"
#include "SpellDialogChildWindow.hxx"
#include "DrawViewShell.hxx"
#include "GraphicViewShell.hxx"
#include <svx/xmlsecctrl.hxx>
#include <svx/formatpaintbrushctrl.hxx>
#include "PaneChildWindows.hxx"

#include "tmplctrl.hxx"

/*************************************************************************
|*
|* Register all Controllers
|*
\************************************************************************/


void SdDLL::RegisterControllers()
{
    SfxModule* pMod = SD_MOD();

    // ToolBoxControls registrieren
    SdTbxControl::RegisterControl( SID_OBJECT_ALIGN, pMod );
    SdTbxControl::RegisterControl( SID_ZOOM_TOOLBOX, pMod );
    SdTbxControl::RegisterControl( SID_OBJECT_CHOOSE_MODE, pMod );
    SdTbxControl::RegisterControl( SID_POSITION, pMod );
    SdTbxControl::RegisterControl( SID_DRAWTBX_TEXT, pMod );
    SdTbxControl::RegisterControl( SID_DRAWTBX_RECTANGLES, pMod );
    SdTbxControl::RegisterControl( SID_DRAWTBX_ELLIPSES, pMod );
    SdTbxControl::RegisterControl( SID_DRAWTBX_LINES, pMod );
    SdTbxControl::RegisterControl( SID_DRAWTBX_ARROWS, pMod );
    SdTbxControl::RegisterControl( SID_DRAWTBX_3D_OBJECTS, pMod );
    SdTbxControl::RegisterControl( SID_DRAWTBX_CONNECTORS, pMod );
    SdTbxControl::RegisterControl( SID_DRAWTBX_INSERT, pMod );

    SvxTbxCtlCustomShapes::RegisterControl( SID_DRAWTBX_CS_BASIC, pMod );
    SvxTbxCtlCustomShapes::RegisterControl( SID_DRAWTBX_CS_SYMBOL, pMod );
    SvxTbxCtlCustomShapes::RegisterControl( SID_DRAWTBX_CS_ARROW, pMod );
    SvxTbxCtlCustomShapes::RegisterControl( SID_DRAWTBX_CS_FLOWCHART, pMod );
    SvxTbxCtlCustomShapes::RegisterControl( SID_DRAWTBX_CS_CALLOUT, pMod );
    SvxTbxCtlCustomShapes::RegisterControl( SID_DRAWTBX_CS_STAR, pMod );

    SdTbxCtlDiaPages::RegisterControl( SID_PAGES_PER_ROW, pMod );
    SdTbxCtlGlueEscDir::RegisterControl( SID_GLUE_ESCDIR, pMod );

    ::sd::AnimationChildWindow::RegisterChildWindow(0, pMod);
    ::sd::NavigatorChildWindow::RegisterChildWindowContext( (sal_uInt16) ::sd::DrawViewShell::GetInterfaceId(), pMod );
    ::sd::NavigatorChildWindow::RegisterChildWindowContext( (sal_uInt16) ::sd::GraphicViewShell::GetInterfaceId(), pMod );
    ::sd::LayerDialogChildWindow::RegisterChildWindow(0, pMod);

    Svx3DChildWindow::RegisterChildWindow(0, pMod);
    SvxFontWorkChildWindow::RegisterChildWindow(0, pMod);
    SvxColorChildWindow::RegisterChildWindow(0, pMod, SFX_CHILDWIN_TASK);
    SvxSearchDialogWrapper::RegisterChildWindow(0, pMod);
    SvxBmpMaskChildWindow::RegisterChildWindow(0, pMod);
    GalleryChildWindow::RegisterChildWindow(0, pMod);
    SvxIMapDlgChildWindow::RegisterChildWindow(0, pMod);
    SvxHlinkDlgWrapper::RegisterChildWindow(0, pMod);
    ::sd::SpellDialogChildWindow::RegisterChildWindow(0, pMod);
    ::avmedia::MediaPlayer::RegisterChildWindow(0, pMod);
    ::sd::LeftPaneImpressChildWindow::RegisterChildWindow(0, pMod);
    ::sd::LeftPaneDrawChildWindow::RegisterChildWindow(0, pMod);
    ::sfx2::sidebar::SidebarChildWindow::RegisterChildWindow(0, pMod);

    SvxFillToolBoxControl::RegisterControl(0, pMod);
    SvxLineStyleToolBoxControl::RegisterControl(0, pMod);
    SvxLineWidthToolBoxControl::RegisterControl(0, pMod);
    SvxLineColorToolBoxControl::RegisterControl(0, pMod);

    SvxLineEndToolBoxControl::RegisterControl( SID_ATTR_LINEEND_STYLE, pMod );

    SvxStyleToolBoxControl::RegisterControl(0, pMod);
    SvxFontNameToolBoxControl::RegisterControl(0, pMod);
    SvxFontColorToolBoxControl::RegisterControl(0, pMod);

    SvxGrafFilterToolBoxControl::RegisterControl( SID_GRFFILTER, pMod );
    SvxGrafModeToolBoxControl::RegisterControl( SID_ATTR_GRAF_MODE, pMod );
    SvxGrafRedToolBoxControl::RegisterControl( SID_ATTR_GRAF_RED, pMod );
    SvxGrafGreenToolBoxControl::RegisterControl( SID_ATTR_GRAF_GREEN, pMod );
    SvxGrafBlueToolBoxControl::RegisterControl( SID_ATTR_GRAF_BLUE, pMod );
    SvxGrafLuminanceToolBoxControl::RegisterControl( SID_ATTR_GRAF_LUMINANCE, pMod );
    SvxGrafContrastToolBoxControl::RegisterControl( SID_ATTR_GRAF_CONTRAST, pMod );
    SvxGrafGammaToolBoxControl::RegisterControl( SID_ATTR_GRAF_GAMMA, pMod );
    SvxGrafTransparenceToolBoxControl::RegisterControl( SID_ATTR_GRAF_TRANSPARENCE, pMod );
    SvxVertTextTbxCtrl::RegisterControl(SID_TEXTDIRECTION_TOP_TO_BOTTOM, pMod);
    SvxVertTextTbxCtrl::RegisterControl(SID_TEXTDIRECTION_LEFT_TO_RIGHT, pMod);
    SvxVertTextTbxCtrl::RegisterControl(SID_DRAW_CAPTION_VERTICAL, pMod);
    SvxVertTextTbxCtrl::RegisterControl(SID_DRAW_FONTWORK_VERTICAL, pMod);
    SvxVertTextTbxCtrl::RegisterControl(SID_DRAW_TEXT_VERTICAL, pMod);
    SvxVertTextTbxCtrl::RegisterControl(SID_TEXT_FITTOSIZE_VERTICAL, pMod);
    SvxCTLTextTbxCtrl::RegisterControl(SID_ATTR_PARA_LEFT_TO_RIGHT, pMod);
    SvxCTLTextTbxCtrl::RegisterControl(SID_ATTR_PARA_RIGHT_TO_LEFT, pMod);

    // register StatusBarControls
    SvxZoomStatusBarControl::RegisterControl( SID_ATTR_ZOOM, pMod );
    SvxPosSizeStatusBarControl::RegisterControl( SID_ATTR_SIZE, pMod );
    SvxModifyControl::RegisterControl( SID_DOC_MODIFIED, pMod );
    SvxZoomSliderControl::RegisterControl( SID_ATTR_ZOOMSLIDER, pMod );

    // MenuControls for PopupMenu
    SvxFontMenuControl::RegisterControl( SID_ATTR_CHAR_FONT, pMod );
    SvxFontSizeMenuControl::RegisterControl( SID_ATTR_CHAR_FONTHEIGHT, pMod );

    SfxMenuControl::RegisterControl( SID_SET_SNAPITEM, pMod );
    SfxMenuControl::RegisterControl( SID_DELETE_SNAPITEM, pMod );
    SfxMenuControl::RegisterControl( SID_BEZIER_CLOSE, pMod );

    // #UndoRedo#
    SvxUndoRedoControl::RegisterControl( SID_UNDO , pMod );
    SvxUndoRedoControl::RegisterControl( SID_REDO , pMod );

    svx::FormatPaintBrushToolBoxControl::RegisterControl(SID_FORMATPAINTBRUSH, pMod );

    SvxClipBoardControl::RegisterControl( SID_PASTE, pMod );

    svx::ExtrusionColorControl::RegisterControl( SID_EXTRUSION_3D_COLOR, pMod );
    svx::FontWorkShapeTypeControl::RegisterControl( SID_FONTWORK_SHAPE_TYPE, pMod );

    ::avmedia::MediaToolBoxControl::RegisterControl( SID_AVMEDIA_TOOLBOX, pMod );
    XmlSecStatusBarControl::RegisterControl( SID_SIGNATURE, pMod );
    SdTemplateControl::RegisterControl( SID_STATUS_LAYOUT, pMod );
    SvxTableToolBoxControl::RegisterControl(SID_INSERT_TABLE, pMod );
    SvxFrameLineStyleToolBoxControl::RegisterControl(SID_FRAME_LINESTYLE, pMod );
    SvxFrameLineColorToolBoxControl::RegisterControl(SID_FRAME_LINECOLOR, pMod );
    SvxFrameToolBoxControl::RegisterControl(SID_ATTR_BORDER, pMod );
    SvxSubToolBoxControl::RegisterControl(SID_OPTIMIZE_TABLE, pMod);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
