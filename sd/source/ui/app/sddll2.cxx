/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sddll2.cxx,v $
 *
 *  $Revision: 1.35 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 13:26:12 $
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



#include <svx/editdata.hxx>
#include "eetext.hxx"
#include <svx/svxids.hrc>

#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif

#include <svx/flditem.hxx>
#ifndef _IMAPDLG_HXX_ //autogen
#include <svx/imapdlg.hxx>
#endif
#ifndef _BMPMASK_HXX_ //autogen
#include <svx/bmpmask.hxx>
#endif
#ifndef _SVX_GALBRWS_HXX_ //autogen
#include <svx/galbrws.hxx>
#endif
#ifndef _SVX_SRCHDLG_HXX //autogen
#include <svx/srchdlg.hxx>
#endif
#ifndef _SVX_FONTWORK_HXX //autogen
#include <svx/fontwork.hxx>
#endif
#ifndef _SVX_COLRCTRL_HXX //autogen
#include <svx/colrctrl.hxx>
#endif
#ifndef _SVX_VERT_TEXT_TBXCTRL_HXX
#include <svx/verttexttbxctrl.hxx>
#endif
#ifndef _SVX_DLG_HYPERLINK_HXX //autogen
#include <svx/hyprlink.hxx>
#endif
#ifndef _SVX_TAB_HYPERLINK_HXX
#include <svx/hyperdlg.hxx>
#endif
#ifndef _FILLCTRL_HXX //autogen
#include <svx/fillctrl.hxx>
#endif
#ifndef _SVX_LINECTRL_HXX //autogen
#include <svx/linectrl.hxx>
#endif
#ifndef _SVX_TBCONTRL_HXX //autogen
#include <svx/tbcontrl.hxx>
#endif
#ifndef _SVX_ZOOMCTRL_HXX //autogen
#include <svx/zoomctrl.hxx>
#endif
#ifndef _SVX_PSZCTRL_HXX //autogen
#include <svx/pszctrl.hxx>
#endif
#ifndef _SVX_MODCTRL_HXX //autogen
#include <svx/modctrl.hxx>
#endif
#ifndef _SVX_FNTCTL_HXX //autogen
#include <svx/fntctl.hxx>
#endif
#ifndef _SVX_FNTSZCTL_HXX //autogen
#include <svx/fntszctl.hxx>
#endif
#ifndef _SVX_F3DCHILD_HXX //autogen
#include <svx/f3dchild.hxx>
#endif
#ifndef _SVX_GRAFCTRL_HXX
#include <svx/grafctrl.hxx>
#endif
#ifndef _SVX_TBXCUSTOMSHAPES_HXX
#include <svx/tbxcustomshapes.hxx>
#endif
#ifndef _SVX_FONTWORK_GALLERY_DIALOG_HXX
#include <svx/fontworkgallery.hxx>
#endif
#ifndef _SVX_LBOXCTRL_HXX_
#include <svx/lboxctrl.hxx>
#endif
#ifndef _SVX_CLIPBOARDCTL_HXX_
#include <svx/clipboardctl.hxx>
#endif
#ifndef _SVX_EXTRUSION_CONTROLS_HXX
#include <svx/extrusioncontrols.hxx>
#endif
#ifndef _SVX_TBXCOLOR_HXX
#include <svx/tbxcolor.hxx>
#endif
#ifndef _AVMEDIA_MEDIACHILD_HXX
#include <avmedia/mediaplayer.hxx>
#endif
#ifndef _AVMEDIA_MEDIATOOLBOX_HXX
#include <avmedia/mediatoolbox.hxx>
#endif

#include <svx/layctrl.hxx>
#include <svx/subtoolboxcontrol.hxx>

#include "sddll.hxx"
#define _SD_DIACTRL_CXX
#include "diactrl.hxx"
#include "gluectrl.hxx"
#include "tbx_ww.hxx"
#ifndef SD_ANIMATION_CHILD_WINDOW_HXX
#include "AnimationChildWindow.hxx"
#endif
#include "animobjs.hxx"
#ifndef SD_NAVIGATOR_CHILD_WINDOW_HXX
#include "NavigatorChildWindow.hxx"
#endif
#ifndef SD_LAZER_DIALOG_CHILD_WINDOW_HXX
#include "LayerDialogChildWindow.hxx"
#endif
//#include "3dchld.hxx"
#include "app.hrc"
#include "SpellDialogChildWindow.hxx"
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#ifndef SD_GRAPHIC_VIEW_SHELL_HXX
#include "GraphicViewShell.hxx"
#endif
#include <svx/xmlsecctrl.hxx>
#ifndef _SVX_FORMATPAINTBRUSHCTRL_HXX
#include <svx/formatpaintbrushctrl.hxx>
#endif
#ifndef SD_PANE_CHILD_WINDOWS_HXX
#include "PaneChildWindows.hxx"
#endif

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
//  SdTbxCtlDiaEffect::RegisterControl(0, pMod);
//  SdTbxCtlDiaSpeed::RegisterControl(0, pMod);
//  SdTbxCtlDiaAuto::RegisterControl(0, pMod);
//  SdTbxCtlDiaTime::RegisterControl(0, pMod);

    SdTbxCtlDiaPages::RegisterControl( SID_PAGES_PER_ROW, pMod );
    SdTbxCtlGlueEscDir::RegisterControl( SID_GLUE_ESCDIR, pMod );

    ::sd::AnimationChildWindow::RegisterChildWindow(0, pMod);
    ::sd::NavigatorChildWindow::RegisterChildWindowContext( (sal_uInt16) ::sd::DrawViewShell::GetInterfaceId(), pMod );
    ::sd::NavigatorChildWindow::RegisterChildWindowContext( (sal_uInt16) ::sd::GraphicViewShell::GetInterfaceId(), pMod );
    ::sd::LayerDialogChildWindow::RegisterChildWindow(0, pMod);
    //Sd3DChildWindow::RegisterChildWindow(0, pMod);
/*
    SvxChildWinCustomShapesBasic::RegisterChildWindow( 0, pMod );
    SvxChildWinCustomShapesSymbol::RegisterChildWindow( 0, pMod );
    SvxChildWinCustomShapesArrow::RegisterChildWindow( 0, pMod );
    SvxChildWinCustomShapesFlowChart::RegisterChildWindow( 0, pMod );
    SvxChildWinCustomShapesCallout::RegisterChildWindow( 0, pMod );
    SvxChildWinCustomShapesStar::RegisterChildWindow( 0, pMod );
*/
    Svx3DChildWindow::RegisterChildWindow(0, pMod);
    SvxFontWorkChildWindow::RegisterChildWindow(0, pMod);
    SvxColorChildWindow::RegisterChildWindow(0, pMod, SFX_CHILDWIN_TASK);
    SvxSearchDialogWrapper::RegisterChildWindow(0, pMod);
    SvxBmpMaskChildWindow::RegisterChildWindow(0, pMod);
    GalleryChildWindow::RegisterChildWindow(0, pMod);
    SvxIMapDlgChildWindow::RegisterChildWindow(0, pMod);
    SvxHyperlinkDlgWrapper::RegisterChildWindow(0, pMod, SFX_CHILDWIN_FORCEDOCK);
    SvxHlinkDlgWrapper::RegisterChildWindow(0, pMod);
    ::sd::SpellDialogChildWindow::RegisterChildWindow(0, pMod);
    ::avmedia::MediaPlayer::RegisterChildWindow(0, pMod);
    ::sd::LeftPaneImpressChildWindow::RegisterChildWindow(0, pMod);
    ::sd::LeftPaneDrawChildWindow::RegisterChildWindow(0, pMod);
    ::sd::RightPaneChildWindow::RegisterChildWindow(0, pMod);

    SvxFillToolBoxControl::RegisterControl(0, pMod);
    SvxLineStyleToolBoxControl::RegisterControl(0, pMod);
    SvxLineWidthToolBoxControl::RegisterControl(0, pMod);
    SvxLineColorToolBoxControl::RegisterControl(0, pMod);

    SvxLineEndToolBoxControl::RegisterControl( SID_ATTR_LINEEND_STYLE, pMod );

    SvxStyleToolBoxControl::RegisterControl(0, pMod);
    SvxFontNameToolBoxControl::RegisterControl(0, pMod);
//  SvxFontHeightToolBoxControl::RegisterControl(0, pMod);
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

    // StatusBarControls registrieren
    SvxZoomStatusBarControl::RegisterControl( SID_ATTR_ZOOM, pMod );
    SvxPosSizeStatusBarControl::RegisterControl( SID_ATTR_SIZE, pMod );
    SvxModifyControl::RegisterControl( SID_DOC_MODIFIED, pMod );
    //SvxInsertStatusBarControl::RegisterControl(0, pModd);

    // MenuControls fuer PopupMenu
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

    svx::ExtrusionDepthControl::RegisterControl( SID_EXTRUSION_DEPTH_FLOATER, pMod );
    svx::ExtrusionDirectionControl::RegisterControl( SID_EXTRUSION_DIRECTION_FLOATER, pMod );
    svx::ExtrusionLightingControl::RegisterControl( SID_EXTRUSION_LIGHTING_FLOATER, pMod );
    svx::ExtrusionSurfaceControl::RegisterControl( SID_EXTRUSION_SURFACE_FLOATER, pMod );
    svx::ExtrusionColorControl::RegisterControl( SID_EXTRUSION_3D_COLOR, pMod );

    svx::FontWorkShapeTypeControl::RegisterControl( SID_FONTWORK_SHAPE_TYPE, pMod );
    svx::FontWorkAlignmentControl::RegisterControl( SID_FONTWORK_ALIGNMENT_FLOATER, pMod );
    svx::FontWorkCharacterSpacingControl::RegisterControl( SID_FONTWORK_CHARACTER_SPACING_FLOATER, pMod );
    ::avmedia::MediaToolBoxControl::RegisterControl( SID_AVMEDIA_TOOLBOX, pMod );
    XmlSecStatusBarControl::RegisterControl( SID_SIGNATURE, pMod );
    SdTemplateControl::RegisterControl( SID_STATUS_LAYOUT, pMod );
    SvxTableToolBoxControl::RegisterControl(SID_INSERT_TABLE, pMod );
    SvxFrameLineStyleToolBoxControl::RegisterControl(SID_FRAME_LINESTYLE, pMod );
    SvxFrameLineColorToolBoxControl::RegisterControl(SID_FRAME_LINECOLOR, pMod );
    SvxFrameToolBoxControl::RegisterControl(SID_ATTR_BORDER, pMod );
    SvxSubToolBoxControl::RegisterControl(SID_OPTIMIZE_TABLE, pMod);
}
