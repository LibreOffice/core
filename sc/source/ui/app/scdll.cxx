/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/************************************************************************
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

#include <editeng/eeitem.hxx>


#include <svx/fmobjfac.hxx>
#include <svx/objfac3d.hxx>
#include <svx/tbxcolor.hxx>

#include <sot/clsids.hxx>
#include <sfx2/taskpane.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/app.hxx>
#include <avmedia/mediaplayer.hxx>
#include <avmedia/mediatoolbox.hxx>
#include <comphelper/types.hxx>
#include <svx/extrusioncolorcontrol.hxx>
#include <svx/fontworkgallery.hxx>
#include <svx/tbxcustomshapes.hxx>

#include <svtools/parhtml.hxx>
#include <sot/formats.hxx>

#include "scitems.hxx"      // fuer tbxctrls etc.
#include "scmod.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "cfgids.hxx"

//! die Registrierung wird wegen CLOOKs in ein eigenes File wandern muessen...

// Interface-Registrierung
#include "docsh.hxx"
#include "tabvwsh.hxx"
#include "prevwsh.hxx"
#include "drawsh.hxx"
#include "drformsh.hxx"
#include "drtxtob.hxx"
#include "editsh.hxx"
#include "pivotsh.hxx"
#include "auditsh.hxx"
#include "cellsh.hxx"
#include "oleobjsh.hxx"
#include "chartsh.hxx"
#include "graphsh.hxx"
#include "mediash.hxx"
#include "pgbrksh.hxx"

#include "docpool.hxx"
#include "appoptio.hxx"

// Controls

#include <svx/tbxalign.hxx>
#include <svx/tbxctl.hxx>
#include <svx/fillctrl.hxx>
#include <svx/linectrl.hxx>
#include <svx/tbcontrl.hxx>
#include <svx/selctrl.hxx>
#include <svx/insctrl.hxx>
#include <svx/zoomctrl.hxx>
#include <editeng/flditem.hxx>
#include <svx/modctrl.hxx>
#include <svx/pszctrl.hxx>
#include <svx/fntctl.hxx>
#include <svx/fntszctl.hxx>
#include <svx/grafctrl.hxx>
#include <svx/galbrws.hxx>
#include <svx/clipboardctl.hxx>
#include <svx/lboxctrl.hxx>
#include <svx/verttexttbxctrl.hxx>
#include <svx/formatpaintbrushctrl.hxx>
#include "tbinsert.hxx"
#include "tbzoomsliderctrl.hxx"
#include <svx/zoomsliderctrl.hxx>

#include <svx/xmlsecctrl.hxx>
// Child-Windows
#include "reffact.hxx"
#include "navipi.hxx"
#include "inputwin.hxx"
#include "spelldialog.hxx"
#include <svx/fontwork.hxx>
#include <svx/srchdlg.hxx>
#include <svx/hyperdlg.hxx>
#include <svx/imapdlg.hxx>

#include "editutil.hxx"
#include <svx/svdfield.hxx>     //  SdrRegisterFieldClasses
#include <rtl/logfile.hxx>

#include "dwfunctr.hxx"
#include "acredlin.hxx"

//------------------------------------------------------------------


//------------------------------------------------------------------

ScResId::ScResId( sal_uInt16 nId ) :
    ResId( nId, *SC_MOD()->GetResMgr() )
{
}

//------------------------------------------------------------------

void ScDLL::Init()
{
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDLL::Init" );

    ScModule **ppShlPtr = (ScModule**) GetAppData(SHL_CALC);
    if ( *ppShlPtr )
        return;

    ScDocumentPool::InitVersionMaps();  // wird im ScModule ctor gebraucht

    ScModule* pMod = new ScModule( &ScDocShell::Factory() );
    (*ppShlPtr) = pMod;

    ScDocShell::Factory().SetDocumentServiceName( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sheet.SpreadsheetDocument" )) );

    ScGlobal::Init();       // erst wenn der ResManager initialisiert ist
                            //  erst nach ScGlobal::Init duerfen die App-Optionen
                            //  initialisiert werden

    // register your view-factories here

    ScTabViewShell      ::RegisterFactory(1);
    ScPreviewShell      ::RegisterFactory(2);

    // register your shell-interfaces here

    ScModule            ::RegisterInterface(pMod);
    ScDocShell          ::RegisterInterface(pMod);
    ScTabViewShell      ::RegisterInterface(pMod);
    ScPreviewShell      ::RegisterInterface(pMod);
    ScDrawShell         ::RegisterInterface(pMod);
    ScDrawFormShell     ::RegisterInterface(pMod);
    ScDrawTextObjectBar ::RegisterInterface(pMod);
    ScEditShell         ::RegisterInterface(pMod);
    ScPivotShell        ::RegisterInterface(pMod);
    ScAuditingShell     ::RegisterInterface(pMod);
    ScFormatShell       ::RegisterInterface(pMod);
    ScCellShell         ::RegisterInterface(pMod);
    ScOleObjectShell    ::RegisterInterface(pMod);
    ScChartShell        ::RegisterInterface(pMod);
    ScGraphicShell      ::RegisterInterface(pMod);
    ScMediaShell        ::RegisterInterface(pMod);
    ScPageBreakShell    ::RegisterInterface(pMod);

    //  eigene Controller
    ScTbxInsertCtrl     ::RegisterControl(SID_TBXCTL_INSERT, pMod);
    ScTbxInsertCtrl     ::RegisterControl(SID_TBXCTL_INSCELLS, pMod);
    ScTbxInsertCtrl     ::RegisterControl(SID_TBXCTL_INSOBJ, pMod);
    ScZoomSliderControl ::RegisterControl(SID_PREVIEW_SCALINGFACTOR, pMod);

    //  Svx-Toolbox-Controller
    SvxTbxCtlDraw                   ::RegisterControl(SID_INSERT_DRAW,          pMod);
    SvxTbxCtlCustomShapes           ::RegisterControl(SID_DRAWTBX_CS_BASIC,     pMod);
    SvxTbxCtlCustomShapes           ::RegisterControl(SID_DRAWTBX_CS_SYMBOL,    pMod);
    SvxTbxCtlCustomShapes           ::RegisterControl(SID_DRAWTBX_CS_ARROW,     pMod);
    SvxTbxCtlCustomShapes           ::RegisterControl(SID_DRAWTBX_CS_FLOWCHART, pMod);
    SvxTbxCtlCustomShapes           ::RegisterControl(SID_DRAWTBX_CS_CALLOUT,   pMod);
    SvxTbxCtlCustomShapes           ::RegisterControl(SID_DRAWTBX_CS_STAR,      pMod);
    SvxTbxCtlAlign                  ::RegisterControl(SID_OBJECT_ALIGN,         pMod);
    SvxFillToolBoxControl           ::RegisterControl(0, pMod);
    SvxLineStyleToolBoxControl      ::RegisterControl(0, pMod);
    SvxLineWidthToolBoxControl      ::RegisterControl(0, pMod);
    SvxLineColorToolBoxControl      ::RegisterControl(0, pMod);
    SvxLineEndToolBoxControl        ::RegisterControl(SID_ATTR_LINEEND_STYLE,   pMod);
    SvxStyleToolBoxControl          ::RegisterControl(SID_STYLE_APPLY,          pMod);
    SvxFontNameToolBoxControl       ::RegisterControl(SID_ATTR_CHAR_FONT,       pMod);
    SvxColorExtToolBoxControl       ::RegisterControl(SID_ATTR_CHAR_COLOR,      pMod);
    SvxColorExtToolBoxControl       ::RegisterControl(SID_BACKGROUND_COLOR,     pMod);
    SvxFrameToolBoxControl          ::RegisterControl(SID_ATTR_BORDER,          pMod);
    SvxFrameLineStyleToolBoxControl ::RegisterControl(SID_FRAME_LINESTYLE,      pMod);
    SvxColorExtToolBoxControl       ::RegisterControl(SID_FRAME_LINECOLOR,      pMod);
    SvxClipBoardControl             ::RegisterControl(SID_PASTE,                pMod );
    SvxUndoRedoControl              ::RegisterControl(SID_UNDO,                 pMod );
    SvxUndoRedoControl              ::RegisterControl(SID_REDO,                 pMod );
    svx::FormatPaintBrushToolBoxControl::RegisterControl(SID_FORMATPAINTBRUSH,  pMod );

    SvxGrafModeToolBoxControl       ::RegisterControl(SID_ATTR_GRAF_MODE,       pMod);
    SvxGrafRedToolBoxControl        ::RegisterControl(SID_ATTR_GRAF_RED,        pMod);
    SvxGrafGreenToolBoxControl      ::RegisterControl(SID_ATTR_GRAF_GREEN,      pMod);
    SvxGrafBlueToolBoxControl       ::RegisterControl(SID_ATTR_GRAF_BLUE,       pMod);
    SvxGrafLuminanceToolBoxControl  ::RegisterControl(SID_ATTR_GRAF_LUMINANCE,  pMod);
    SvxGrafContrastToolBoxControl   ::RegisterControl(SID_ATTR_GRAF_CONTRAST,   pMod);
    SvxGrafGammaToolBoxControl      ::RegisterControl(SID_ATTR_GRAF_GAMMA,      pMod);
    SvxGrafTransparenceToolBoxControl::RegisterControl(SID_ATTR_GRAF_TRANSPARENCE, pMod);
    SvxGrafFilterToolBoxControl     ::RegisterControl(SID_GRFFILTER,            pMod);

    SvxVertTextTbxCtrl::RegisterControl(SID_DRAW_CAPTION_VERTICAL,          pMod);
    SvxVertTextTbxCtrl::RegisterControl(SID_DRAW_TEXT_VERTICAL,             pMod);
    SvxVertTextTbxCtrl::RegisterControl(SID_TEXTDIRECTION_LEFT_TO_RIGHT,    pMod);
    SvxVertTextTbxCtrl::RegisterControl(SID_TEXTDIRECTION_TOP_TO_BOTTOM,    pMod);
    SvxCTLTextTbxCtrl::RegisterControl(SID_ATTR_PARA_LEFT_TO_RIGHT, pMod);
    SvxCTLTextTbxCtrl::RegisterControl(SID_ATTR_PARA_RIGHT_TO_LEFT, pMod);

    //Media Controller
    ::avmedia::MediaToolBoxControl::RegisterControl( SID_AVMEDIA_TOOLBOX, pMod );

    // common SFX controller
    ::sfx2::TaskPaneWrapper::RegisterChildWindow( false, pMod );

    // Svx-StatusBar-Controller
    SvxInsertStatusBarControl       ::RegisterControl(SID_ATTR_INSERT,      pMod);
    SvxSelectionModeControl         ::RegisterControl(SID_STATUS_SELMODE,   pMod);
    SvxZoomStatusBarControl         ::RegisterControl(SID_ATTR_ZOOM,        pMod);
    SvxZoomSliderControl            ::RegisterControl(SID_ATTR_ZOOMSLIDER,  pMod);
    SvxModifyControl                ::RegisterControl(SID_DOC_MODIFIED,     pMod);
    XmlSecStatusBarControl          ::RegisterControl( SID_SIGNATURE,       pMod );

    SvxPosSizeStatusBarControl      ::RegisterControl(SID_ATTR_SIZE,        pMod);

    // Svx-Menue-Controller
    SvxFontMenuControl              ::RegisterControl(SID_ATTR_CHAR_FONT,       pMod);
    SvxFontSizeMenuControl          ::RegisterControl(SID_ATTR_CHAR_FONTHEIGHT, pMod);

    // CustomShape extrusion controller
    svx::ExtrusionColorControl::RegisterControl( SID_EXTRUSION_3D_COLOR, pMod );
    svx::FontWorkShapeTypeControl::RegisterControl( SID_FONTWORK_SHAPE_TYPE, pMod );

    //  Child-Windows

    // Hack: Eingabezeile mit 42 registrieren, damit sie im PlugIn immer sichtbar ist
    ScInputWindowWrapper        ::RegisterChildWindow(42, pMod, SFX_CHILDWIN_TASK|SFX_CHILDWIN_FORCEDOCK);
    ScNavigatorDialogWrapper    ::RegisterChildWindowContext(
            sal::static_int_cast<sal_uInt16>(ScTabViewShell::GetInterfaceId()), pMod);
    ScSolverDlgWrapper          ::RegisterChildWindow(false, pMod);
    ScOptSolverDlgWrapper       ::RegisterChildWindow(false, pMod);
    ScNameDlgWrapper            ::RegisterChildWindow(false, pMod);
    ScNameDefDlgWrapper         ::RegisterChildWindow(false, pMod);
    ScCondFormatConditionDlgWrapper ::RegisterChildWindow(false, pMod);
    ScCondFormatColorScaleDlgWrapper ::RegisterChildWindow(false, pMod);
    ScCondFormatDataBarDlgWrapper ::RegisterChildWindow(false, pMod);
    ScPivotLayoutWrapper        ::RegisterChildWindow(false, pMod);
    ScTabOpDlgWrapper           ::RegisterChildWindow(false, pMod);
    ScFilterDlgWrapper          ::RegisterChildWindow(false, pMod);
    ScSpecialFilterDlgWrapper   ::RegisterChildWindow(false, pMod);
    ScDbNameDlgWrapper          ::RegisterChildWindow(false, pMod);
    ScConsolidateDlgWrapper     ::RegisterChildWindow(false, pMod);
    ScPrintAreasDlgWrapper      ::RegisterChildWindow(false, pMod);
    ScColRowNameRangesDlgWrapper::RegisterChildWindow(false, pMod);
    ScFormulaDlgWrapper         ::RegisterChildWindow(false, pMod);

    // First docking Window for Calc
    ScFunctionChildWindow       ::RegisterChildWindow(false, pMod);

    // Redlining- Window
    ScAcceptChgDlgWrapper       ::RegisterChildWindow(false, pMod);
    ScSimpleRefDlgWrapper       ::RegisterChildWindow(false, pMod, SFX_CHILDWIN_ALWAYSAVAILABLE|SFX_CHILDWIN_NEVERHIDE );
    ScHighlightChgDlgWrapper    ::RegisterChildWindow(false, pMod);

    SvxSearchDialogWrapper      ::RegisterChildWindow(false, pMod);
    SvxHlinkDlgWrapper          ::RegisterChildWindow(false, pMod);
    SvxFontWorkChildWindow      ::RegisterChildWindow(false, pMod);
    SvxIMapDlgChildWindow       ::RegisterChildWindow(false, pMod);
    GalleryChildWindow          ::RegisterChildWindow(false, pMod);
    ScSpellDialogChildWindow    ::RegisterChildWindow(false, pMod);

    ScValidityRefChildWin::RegisterChildWindow(false, pMod);

    //  Edit-Engine-Felder, soweit nicht schon in OfficeApplication::Init

    SvClassManager& rClassManager = SvxFieldItem::GetClassManager();
    rClassManager.Register(SvxPagesField::StaticClassId(), SvxPagesField::CreateInstance);
    rClassManager.Register(SvxFileField::StaticClassId(),  SvxFileField::CreateInstance);
    rClassManager.Register(SvxTableField::StaticClassId(), SvxTableField::CreateInstance);

    SdrRegisterFieldClasses();      // SvDraw-Felder registrieren

    // 3D-Objekt-Factory eintragen
    E3dObjFactory();

    // ::com::sun::star::form::component::Form-Objekt-Factory eintragen
    FmFormObjFactory();

    pMod->PutItem( SfxUInt16Item( SID_ATTR_METRIC, sal::static_int_cast<sal_uInt16>(pMod->GetAppOptions().GetAppMetric()) ) );

    //  StarOne Services are now handled in the registry
}

//------------------------------------------------------------------
//  Statusbar
//------------------------------------------------------------------

#define TEXT_WIDTH(s)   rStatusBar.GetTextWidth((s))

#undef TEXT_WIDTH

// DetectFilter functionality has moved - please update your bookmarks
// see sc/source/ui/unoobj/scdetect.cxx, have a nice day.

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
