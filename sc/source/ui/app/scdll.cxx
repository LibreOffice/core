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

#include <editeng/eeitem.hxx>

#include <svx/fmobjfac.hxx>
#include <svx/objfac3d.hxx>
#include <svx/tbxcolor.hxx>

#include <comphelper/classids.hxx>
#include <sfx2/sidebar/SidebarChildWindow.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/app.hxx>
#include <avmedia/mediaplayer.hxx>
#include <avmedia/mediatoolbox.hxx>
#include <comphelper/types.hxx>
#include <svx/ParaLineSpacingPopup.hxx>
#include <svx/TextCharacterSpacingPopup.hxx>
#include <svx/TextUnderlinePopup.hxx>
#include <NumberFormatControl.hxx>

#include <svtools/parhtml.hxx>
#include <sot/formats.hxx>

#include "scitems.hxx"
#include "scmod.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "cfgids.hxx"

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
#include <searchresults.hxx>

// Controls

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
#include <svx/grafctrl.hxx>
#include <svx/clipboardctl.hxx>
#include <svx/lboxctrl.hxx>
#include <svx/verttexttbxctrl.hxx>
#include <svx/formatpaintbrushctrl.hxx>
#include "tbzoomsliderctrl.hxx"
#include <svx/zoomsliderctrl.hxx>
#include <sfx2/emojipopup.hxx>

#include <svx/xmlsecctrl.hxx>
// Child windows
#include "reffact.hxx"
#include "navipi.hxx"
#include "inputwin.hxx"
#include "spelldialog.hxx"
#include <svx/fontwork.hxx>
#include <svx/srchdlg.hxx>
#include <svx/hyperdlg.hxx>
#include <svx/imapdlg.hxx>

#include "editutil.hxx"
#include <svx/svdfield.hxx>

#include "dwfunctr.hxx"
#include "acredlin.hxx"
#include <o3tl/make_unique.hxx>

ScResId::ScResId( sal_uInt16 nId ) :
    ResId( nId, *SC_MOD()->GetResMgr() )
{
}

void ScDLL::Init()
{
    if ( SfxApplication::GetModule(SfxToolsModule::Calc) )    // Module already active
        return;

    ScDocumentPool::InitVersionMaps(); // Is needed in the ScModule ctor

    auto pUniqueModule = o3tl::make_unique<ScModule>(&ScDocShell::Factory());
    ScModule* pMod = pUniqueModule.get();
    SfxApplication::SetModule(SfxToolsModule::Calc, std::move(pUniqueModule));

    ScDocShell::Factory().SetDocumentServiceName( "com.sun.star.sheet.SpreadsheetDocument" );

    // Not until the ResManager is initialized
    // The AppOptions must be initialized not until after ScGlobal::Init
    ScGlobal::Init();

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

    // Own Controller
    ScZoomSliderControl             ::RegisterControl(SID_PREVIEW_SCALINGFACTOR, pMod);
    SvxCurrencyToolBoxControl       ::RegisterControl(SID_NUMBER_CURRENCY, pMod);

    // SvxToolboxController
    SvxTbxCtlDraw                   ::RegisterControl(SID_INSERT_DRAW,          pMod);
    SvxFillToolBoxControl           ::RegisterControl(0, pMod);
    SvxLineStyleToolBoxControl      ::RegisterControl(0, pMod);
    SvxLineWidthToolBoxControl      ::RegisterControl(0, pMod);
    SvxColorToolBoxControl          ::RegisterControl(SID_ATTR_LINE_COLOR,      pMod);
    SvxColorToolBoxControl          ::RegisterControl(SID_ATTR_FILL_COLOR,      pMod);
    SvxLineEndToolBoxControl        ::RegisterControl(SID_ATTR_LINEEND_STYLE,   pMod);
    SvxStyleToolBoxControl          ::RegisterControl(SID_STYLE_APPLY,          pMod);
    SvxFontNameToolBoxControl       ::RegisterControl(SID_ATTR_CHAR_FONT,       pMod);
    SvxColorToolBoxControl          ::RegisterControl(SID_ATTR_CHAR_COLOR,      pMod);
    SvxColorToolBoxControl          ::RegisterControl(SID_BACKGROUND_COLOR,     pMod);
    SvxColorToolBoxControl          ::RegisterControl(SID_ATTR_CHAR_BACK_COLOR, pMod);
    SvxFrameToolBoxControl          ::RegisterControl(SID_ATTR_BORDER,          pMod);
    SvxFrameLineStyleToolBoxControl ::RegisterControl(SID_FRAME_LINESTYLE,      pMod);
    SvxColorToolBoxControl          ::RegisterControl(SID_FRAME_LINECOLOR,      pMod);
    SvxClipBoardControl             ::RegisterControl(SID_PASTE,                pMod );
    SvxUndoRedoControl              ::RegisterControl(SID_UNDO,                 pMod );
    SvxUndoRedoControl              ::RegisterControl(SID_REDO,                 pMod );
    svx::ParaLineSpacingPopup       ::RegisterControl(SID_ATTR_PARA_LINESPACE,  pMod );
    svx::TextCharacterSpacingPopup  ::RegisterControl(SID_ATTR_CHAR_KERNING,    pMod );
    svx::TextUnderlinePopup         ::RegisterControl(SID_ATTR_CHAR_UNDERLINE,  pMod );
    svx::FormatPaintBrushToolBoxControl::RegisterControl(SID_FORMATPAINTBRUSH,  pMod );
    sc::ScNumberFormatControl       ::RegisterControl(SID_NUMBER_TYPE_FORMAT,   pMod );

    SvxGrafModeToolBoxControl       ::RegisterControl(SID_ATTR_GRAF_MODE,       pMod);
    SvxGrafRedToolBoxControl        ::RegisterControl(SID_ATTR_GRAF_RED,        pMod);
    SvxGrafGreenToolBoxControl      ::RegisterControl(SID_ATTR_GRAF_GREEN,      pMod);
    SvxGrafBlueToolBoxControl       ::RegisterControl(SID_ATTR_GRAF_BLUE,       pMod);
    SvxGrafLuminanceToolBoxControl  ::RegisterControl(SID_ATTR_GRAF_LUMINANCE,  pMod);
    SvxGrafContrastToolBoxControl   ::RegisterControl(SID_ATTR_GRAF_CONTRAST,   pMod);
    SvxGrafGammaToolBoxControl      ::RegisterControl(SID_ATTR_GRAF_GAMMA,      pMod);
    SvxGrafTransparenceToolBoxControl::RegisterControl(SID_ATTR_GRAF_TRANSPARENCE, pMod);

    SvxVertTextTbxCtrl::RegisterControl(SID_DRAW_CAPTION_VERTICAL,          pMod);
    SvxVertTextTbxCtrl::RegisterControl(SID_DRAW_TEXT_VERTICAL,             pMod);
    SvxVertTextTbxCtrl::RegisterControl(SID_TEXTDIRECTION_LEFT_TO_RIGHT,    pMod);
    SvxVertTextTbxCtrl::RegisterControl(SID_TEXTDIRECTION_TOP_TO_BOTTOM,    pMod);
    SvxCTLTextTbxCtrl::RegisterControl(SID_ATTR_PARA_LEFT_TO_RIGHT, pMod);
    SvxCTLTextTbxCtrl::RegisterControl(SID_ATTR_PARA_RIGHT_TO_LEFT, pMod);

    EmojiPopup::RegisterControl(SID_EMOJI_CONTROL, pMod );

    // Media Controller
    ::avmedia::MediaToolBoxControl::RegisterControl( SID_AVMEDIA_TOOLBOX, pMod );

    // Common SFX Controller
    ::sfx2::sidebar::SidebarChildWindow::RegisterChildWindow(false, pMod);

    // SvxStatusBar Controller
    SvxInsertStatusBarControl       ::RegisterControl(SID_ATTR_INSERT,      pMod);
    SvxSelectionModeControl         ::RegisterControl(SID_STATUS_SELMODE,   pMod);
    SvxZoomStatusBarControl         ::RegisterControl(SID_ATTR_ZOOM,        pMod);
    SvxZoomSliderControl            ::RegisterControl(SID_ATTR_ZOOMSLIDER,  pMod);
    SvxModifyControl                ::RegisterControl(SID_DOC_MODIFIED,     pMod);
    XmlSecStatusBarControl          ::RegisterControl( SID_SIGNATURE,       pMod );

    SvxPosSizeStatusBarControl      ::RegisterControl(SID_ATTR_SIZE,        pMod);

    // CustomShape extrusion controller
    SvxColorToolBoxControl::RegisterControl( SID_EXTRUSION_3D_COLOR, pMod );

    // Child Windows

    ScInputWindowWrapper        ::RegisterChildWindow(true, pMod, SfxChildWindowFlags::TASK|SfxChildWindowFlags::FORCEDOCK);
    ScNavigatorDialogWrapper    ::RegisterChildWindowContext(static_cast<sal_uInt16>(ScTabViewShell::GetInterfaceId()), pMod);
    ScSolverDlgWrapper          ::RegisterChildWindow(false, pMod);
    ScOptSolverDlgWrapper       ::RegisterChildWindow(false, pMod);
    ScXMLSourceDlgWrapper       ::RegisterChildWindow(false, pMod);
    ScNameDlgWrapper            ::RegisterChildWindow(false, pMod);
    ScNameDefDlgWrapper         ::RegisterChildWindow(false, pMod);
    ScPivotLayoutWrapper        ::RegisterChildWindow(false, pMod);
    ScTabOpDlgWrapper           ::RegisterChildWindow(false, pMod);
    ScFilterDlgWrapper          ::RegisterChildWindow(false, pMod);
    ScSpecialFilterDlgWrapper   ::RegisterChildWindow(false, pMod);
    ScDbNameDlgWrapper          ::RegisterChildWindow(false, pMod);
    ScConsolidateDlgWrapper     ::RegisterChildWindow(false, pMod);
    ScPrintAreasDlgWrapper      ::RegisterChildWindow(false, pMod);
    ScColRowNameRangesDlgWrapper::RegisterChildWindow(false, pMod);
    ScFormulaDlgWrapper         ::RegisterChildWindow(false, pMod);

    ScRandomNumberGeneratorDialogWrapper::RegisterChildWindow(false, pMod);
    ScSamplingDialogWrapper             ::RegisterChildWindow(false, pMod);
    ScDescriptiveStatisticsDialogWrapper::RegisterChildWindow(false, pMod);
    ScAnalysisOfVarianceDialogWrapper   ::RegisterChildWindow(false, pMod);
    ScCorrelationDialogWrapper          ::RegisterChildWindow(false, pMod);
    ScCovarianceDialogWrapper           ::RegisterChildWindow(false, pMod);
    ScExponentialSmoothingDialogWrapper ::RegisterChildWindow(false, pMod);
    ScMovingAverageDialogWrapper        ::RegisterChildWindow(false, pMod);
    ScRegressionDialogWrapper           ::RegisterChildWindow(false, pMod);
    ScTTestDialogWrapper                ::RegisterChildWindow(false, pMod);
    ScFTestDialogWrapper                ::RegisterChildWindow(false, pMod);
    ScZTestDialogWrapper                ::RegisterChildWindow(false, pMod);
    ScChiSquareTestDialogWrapper        ::RegisterChildWindow(false, pMod);

    // Redlining Window
    ScAcceptChgDlgWrapper       ::RegisterChildWindow(false, pMod);
    ScSimpleRefDlgWrapper       ::RegisterChildWindow(false, pMod, SfxChildWindowFlags::ALWAYSAVAILABLE|SfxChildWindowFlags::NEVERHIDE );
    ScHighlightChgDlgWrapper    ::RegisterChildWindow(false, pMod);

    SvxSearchDialogWrapper      ::RegisterChildWindow(false, pMod);
    SvxHlinkDlgWrapper          ::RegisterChildWindow(false, pMod);
    SvxFontWorkChildWindow      ::RegisterChildWindow(false, pMod);
    SvxIMapDlgChildWindow       ::RegisterChildWindow(false, pMod);
    ScSpellDialogChildWindow    ::RegisterChildWindow(false, pMod);

    ScValidityRefChildWin::RegisterChildWindow(false, pMod);
    sc::SearchResultsDlgWrapper::RegisterChildWindow(false, pMod);
    ScCondFormatDlgWrapper::RegisterChildWindow(false, pMod);

    // EditEngine Field; insofar not already defined in OfficeApplication::Init
    SvClassManager& rClassManager = SvxFieldItem::GetClassManager();
    rClassManager.Register(SvxPagesField::StaticClassId(), SvxPagesField::CreateInstance);
    rClassManager.Register(SvxFileField::StaticClassId(),  SvxFileField::CreateInstance);
    rClassManager.Register(SvxTableField::StaticClassId(), SvxTableField::CreateInstance);

    SdrRegisterFieldClasses(); // Register SvDraw fields

    // Add 3DObject Factory
    E3dObjFactory();

    // Add css::form::component::FormObject Factory
    FmFormObjFactory();

    pMod->PutItem( SfxUInt16Item( SID_ATTR_METRIC, sal::static_int_cast<sal_uInt16>(pMod->GetAppOptions().GetAppMetric()) ) );

    //  StarOne Services are now handled in the registry
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
