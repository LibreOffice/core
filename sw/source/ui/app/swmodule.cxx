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

#include <hintids.hxx>
#include <swerror.h>
#include <vcl/wrkwin.hxx>
#include <vcl/graph.hxx>
#include <svx/galbrws.hxx>
#include <svx/svdobj.hxx>
#include <svtools/ehdl.hxx>
#include <svx/fntszctl.hxx>
#include <svx/fntctl.hxx>
#include <svx/SmartTagCtl.hxx>
#include <svx/pszctrl.hxx>
#include <svx/insctrl.hxx>
#include <svx/selctrl.hxx>
#include <svx/linectrl.hxx>
#include <svx/tbxctl.hxx>            // at the moment wrong include-protection!
#include <svx/fillctrl.hxx>
#include <svx/tbcontrl.hxx>
#include <svx/verttexttbxctrl.hxx>
#include <svx/formatpaintbrushctrl.hxx>
#include <svx/contdlg.hxx>
#include <svx/layctrl.hxx>
#include <svx/fontwork.hxx>
#include <SwSpellDialogChildWindow.hxx>
#include <svx/tbxalign.hxx>
#include <svx/grafctrl.hxx>
#include <svx/tbxcolor.hxx>
#include <svx/clipboardctl.hxx>
#include <svx/lboxctrl.hxx>
#include <svx/tbxcustomshapes.hxx>
#include <svx/imapdlg.hxx>
#include <svx/srchdlg.hxx>
#include <svx/hyperdlg.hxx>
#include <svx/extrusioncolorcontrol.hxx>
#include <svx/fontworkgallery.hxx>
#include <svx/modctrl.hxx>
#include <com/sun/star/scanner/ScannerManager.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/linguistic2/LanguageGuessing.hpp>
#include <comphelper/processfactory.hxx>
#include <docsh.hxx>
#include <swmodule.hxx>
#include <swevent.hxx>
#include <swacorr.hxx>
#include <cmdid.h>
#include <dobjfac.hxx>
#include <init.hxx>
#include <pview.hxx>
#include <wview.hxx>
#include <wdocsh.hxx>
#include <globdoc.hxx>
#include <srcview.hxx>
#include <glshell.hxx>
#include <tabsh.hxx>
#include <listsh.hxx>
#include <grfsh.hxx>
#include <mediash.hxx>
#include <olesh.hxx>
#include <drawsh.hxx>
#include <wformsh.hxx>
#include <drwtxtsh.hxx>
#include <beziersh.hxx>
#include <wtextsh.hxx>
#include <wfrmsh.hxx>
#include <drformsh.hxx>
#include <wgrfsh.hxx>
#include <wolesh.hxx>
#include <wlistsh.hxx>
#include <wtabsh.hxx>
#include <navipi.hxx>
#include <chartins.hxx>
#include <inputwin.hxx>
#include <usrpref.hxx>
#include <uinums.hxx>
#include <prtopt.hxx>
#include <bookctrl.hxx>
#include <tmplctrl.hxx>
#include <viewlayoutctrl.hxx>
#include <svx/zoomsliderctrl.hxx>
#include <tblctrl.hxx>
#include <zoomctrl.hxx>
#include <wordcountctrl.hxx>
#include <workctrl.hxx>
#include <tbxanchr.hxx>
#include <fldwrap.hxx>
#include <redlndlg.hxx>
#include <syncbtn.hxx>
#include <mailmergechildwindow.hxx>
#include <modcfg.hxx>
#include <fontcfg.hxx>
#include <sfx2/sidebar/SidebarChildWindow.hxx>
#include <sfx2/taskpane.hxx>
#include <sfx2/evntconf.hxx>
#include <sfx2/appuno.hxx>
#include <swatrset.hxx>
#include <idxmrk.hxx>
#include <wordcountdialog.hxx>
#include <dlelstnr.hxx>
#include <barcfg.hxx>
#include <svx/rubydialog.hxx>
#include <svtools/colorcfg.hxx>

#include <editeng/acorrcfg.hxx>
#include <unotools/moduleoptions.hxx>

#include <avmedia/mediaplayer.hxx>
#include <avmedia/mediatoolbox.hxx>

#include <annotsh.hxx>
#include <navsh.hxx>

#include <app.hrc>
#include <svx/xmlsecctrl.hxx>
ResMgr *pSwResMgr = 0;
bool     bNoInterrupt     = false;

#include <sfx2/app.hxx>

#include <svx/svxerr.hxx>

#include <unomid.h>

#include "swdllimpl.hxx"

using namespace com::sun::star;


TYPEINIT1( SwModule, SfxModule );

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

SwModule::SwModule( SfxObjectFactory* pWebFact,
                    SfxObjectFactory* pFact,
                    SfxObjectFactory* pGlobalFact )
    : SfxModule( SfxApplication::CreateResManager( "sw" ), sal_False, pWebFact,
                     pFact, pGlobalFact, NULL ),
    pModuleConfig(0),
    pUsrPref(0),
    pWebUsrPref(0),
    pPrtOpt(0),
    pWebPrtOpt(0),
    pChapterNumRules(0),
    pStdFontConfig(0),
    pNavigationConfig(0),
    pToolbarConfig(0),
    pWebToolbarConfig(0),
    pDBConfig(0),
    pColorConfig(0),
    pAccessibilityOptions(0),
    pCTLOptions(0),
    pUserOptions(0),
    pAttrPool(0),
    pView(0),
    bAuthorInitialised(sal_False),
    bEmbeddedLoadSave( sal_False ),
    pDragDrop( 0 ),
    pXSelection( 0 )
{
    SetName( OUString("StarWriter") );
    pSwResMgr = GetResMgr();
    SvxErrorHandler::ensure();
    pErrorHdl = new SfxErrorHandler( RID_SW_ERRHDL,
                                     ERRCODE_AREA_SW,
                                     ERRCODE_AREA_SW_END,
                                     pSwResMgr );

    pModuleConfig = new SwModuleOptions;

    // We need them anyways
    pToolbarConfig = new SwToolbarConfigItem( false );
    pWebToolbarConfig = new SwToolbarConfigItem( true );

    pStdFontConfig = new SwStdFontConfig;

    pAuthorNames = new std::vector<OUString>;  // All Redlining-Authors

    // replace SvxAutocorrect with SwAutocorrect
    SvxAutoCorrCfg& rACfg = SvxAutoCorrCfg::Get();
    const SvxAutoCorrect* pOld = rACfg.GetAutoCorrect();
    rACfg.SetAutoCorrect(new SwAutoCorrect( *pOld ));

    StartListening( *SFX_APP() );

    // OD 14.02.2003 #107424# - init color configuration
    // member <pColorConfig> is created and the color configuration is applied
    // at the view options.
    GetColorConfig();
}
uno::Reference< scanner::XScannerManager2 >
SwModule::GetScannerManager()
{
    if (!m_xScannerManager.is())
    {
        m_xScannerManager = scanner::ScannerManager::create( comphelper::getProcessComponentContext() );
    }
    return m_xScannerManager;
}

uno::Reference< linguistic2::XLanguageGuessing > SwModule::GetLanguageGuesser()
{
    if (!m_xLanguageGuesser.is())
    {
        m_xLanguageGuesser = linguistic2::LanguageGuessing::create( comphelper::getProcessComponentContext() );
    }
    return m_xLanguageGuesser;
}

SwModule::~SwModule()
{
    delete pErrorHdl;
    EndListening( *SFX_APP() );
}

void SwModule::CreateLngSvcEvtListener()
{
    if (!xLngSvcEvtListener.is())
        xLngSvcEvtListener = new SwLinguServiceEventListener;
}

void SwDLL::RegisterFactories()
{
    // These Id's must not be changed. Through these Id's the View (resume Documentview)
    // is created by Sfx.
    if ( SvtModuleOptions().IsWriter() )
        SwView::RegisterFactory         ( 2 );

    SwWebView::RegisterFactory        ( 5 );

    if ( SvtModuleOptions().IsWriter() )
    {
        SwSrcView::RegisterFactory      ( 6 );
        SwPagePreView::RegisterFactory  ( 7 );
    }
}

void SwDLL::RegisterInterfaces()
{
    SwModule* pMod = SW_MOD();
    SwModule::RegisterInterface( pMod );
    SwDocShell::RegisterInterface( pMod );
    SwWebDocShell::RegisterInterface( pMod );
    SwGlosDocShell::RegisterInterface( pMod );
    SwWebGlosDocShell::RegisterInterface( pMod );
    SwView::RegisterInterface( pMod );
    SwWebView::RegisterInterface( pMod );
    SwPagePreView::RegisterInterface( pMod );
    SwSrcView::RegisterInterface( pMod );


    SwBaseShell::RegisterInterface(pMod);
    SwTextShell::RegisterInterface(pMod);
    SwTableShell::RegisterInterface(pMod);
    SwListShell::RegisterInterface(pMod);
    SwFrameShell::RegisterInterface(pMod);
    SwDrawBaseShell::RegisterInterface(pMod);
    SwDrawShell::RegisterInterface(pMod);
    SwDrawFormShell::RegisterInterface(pMod);
    SwDrawTextShell::RegisterInterface(pMod);
    SwBezierShell::RegisterInterface(pMod);
    SwGrfShell::RegisterInterface(pMod);
    SwOleShell::RegisterInterface(pMod);
    SwNavigationShell::RegisterInterface(pMod);
    SwWebTextShell::RegisterInterface(pMod);
    SwWebFrameShell::RegisterInterface(pMod);
    SwWebGrfShell::RegisterInterface(pMod);
    SwWebListShell::RegisterInterface(pMod);
    SwWebTableShell::RegisterInterface(pMod);
    SwWebDrawFormShell::RegisterInterface(pMod);
    SwWebOleShell::RegisterInterface(pMod);
    SwMediaShell::RegisterInterface(pMod);
    SwAnnotationShell::RegisterInterface(pMod);
}

void SwDLL::RegisterControls()
{
    SwModule* pMod = SW_MOD();

    SfxRecentFilesToolBoxControl::RegisterControl( FN_OPEN_FILE, pMod );
    SvxTbxCtlDraw::RegisterControl(SID_INSERT_DRAW, pMod );
    SvxTbxCtlAlign::RegisterControl(SID_OBJECT_ALIGN, pMod );
    SwTbxAnchor::RegisterControl(FN_TOOL_ANCHOR, pMod );
    SwTbxInsertCtrl::RegisterControl(FN_INSERT_CTRL, pMod );
    SwTbxInsertCtrl::RegisterControl(FN_INSERT_OBJ_CTRL, pMod );
    SwTbxFieldCtrl::RegisterControl(FN_INSERT_FIELD_CTRL, pMod );
    SwTbxAutoTextCtrl::RegisterControl(FN_GLOSSARY_DLG, pMod );

    SvxTbxCtlCustomShapes::RegisterControl( SID_DRAWTBX_CS_BASIC, pMod );
    SvxTbxCtlCustomShapes::RegisterControl( SID_DRAWTBX_CS_SYMBOL, pMod );
    SvxTbxCtlCustomShapes::RegisterControl( SID_DRAWTBX_CS_ARROW, pMod );
    SvxTbxCtlCustomShapes::RegisterControl( SID_DRAWTBX_CS_FLOWCHART, pMod );
    SvxTbxCtlCustomShapes::RegisterControl( SID_DRAWTBX_CS_CALLOUT, pMod );
    SvxTbxCtlCustomShapes::RegisterControl( SID_DRAWTBX_CS_STAR, pMod );

    svx::ExtrusionColorControl::RegisterControl( SID_EXTRUSION_3D_COLOR, pMod );
    svx::FontWorkShapeTypeControl::RegisterControl( SID_FONTWORK_SHAPE_TYPE, pMod );

    SvxClipBoardControl::RegisterControl(SID_PASTE, pMod );
    SvxUndoRedoControl::RegisterControl(SID_UNDO, pMod );
    SvxUndoRedoControl::RegisterControl(SID_REDO, pMod );
    svx::FormatPaintBrushToolBoxControl::RegisterControl(SID_FORMATPAINTBRUSH, pMod );

    SvxFillToolBoxControl::RegisterControl(SID_ATTR_FILL_STYLE, pMod );
    SvxLineStyleToolBoxControl::RegisterControl(SID_ATTR_LINE_STYLE, pMod );
    SvxLineWidthToolBoxControl::RegisterControl(SID_ATTR_LINE_WIDTH, pMod );
    SvxLineColorToolBoxControl::RegisterControl(SID_ATTR_LINE_COLOR, pMod );
    SvxLineEndToolBoxControl::RegisterControl(SID_ATTR_LINEEND_STYLE, pMod );

    SvxFontNameToolBoxControl::RegisterControl(SID_ATTR_CHAR_FONT, pMod );
    SvxFontColorToolBoxControl::RegisterControl(SID_ATTR_CHAR_COLOR, pMod );
    SvxColorExtToolBoxControl::RegisterControl(SID_ATTR_CHAR_COLOR2, pMod );
    SvxColorExtToolBoxControl::RegisterControl(SID_ATTR_CHAR_COLOR_BACKGROUND, pMod );
    SvxStyleToolBoxControl::RegisterControl(SID_STYLE_APPLY, pMod );
    SvxColorExtToolBoxControl::RegisterControl( SID_BACKGROUND_COLOR, pMod );
    SvxFrameToolBoxControl::RegisterControl(SID_ATTR_BORDER, pMod );
    SvxFrameLineStyleToolBoxControl::RegisterControl(SID_FRAME_LINESTYLE, pMod );
    SvxColorExtToolBoxControl::RegisterControl(SID_FRAME_LINECOLOR, pMod );

    SvxColumnsToolBoxControl::RegisterControl(FN_INSERT_FRAME_INTERACT, pMod );
    SvxColumnsToolBoxControl::RegisterControl(FN_INSERT_FRAME, pMod );
    SvxColumnsToolBoxControl::RegisterControl(FN_INSERT_REGION, pMod );
    SvxTableToolBoxControl::RegisterControl(FN_INSERT_TABLE, pMod );
    SvxTableToolBoxControl::RegisterControl(FN_SHOW_MULTIPLE_PAGES, pMod );

    SvxFontMenuControl::RegisterControl(SID_ATTR_CHAR_FONT, pMod );
    SvxFontSizeMenuControl::RegisterControl(SID_ATTR_CHAR_FONTHEIGHT, pMod );

    SwZoomControl::RegisterControl(SID_ATTR_ZOOM, pMod );
    SwPreviewZoomControl::RegisterControl(FN_PREVIEW_ZOOM, pMod);
    SvxPosSizeStatusBarControl::RegisterControl(0, pMod );
    SvxInsertStatusBarControl::RegisterControl(SID_ATTR_INSERT, pMod );
    SvxSelectionModeControl::RegisterControl(FN_STAT_SELMODE, pMod );
    XmlSecStatusBarControl::RegisterControl( SID_SIGNATURE, pMod );
    SwWordCountStatusBarControl::RegisterControl(FN_STAT_WORDCOUNT, pMod);

    SwBookmarkControl::RegisterControl(FN_STAT_PAGE, pMod );
    SwTemplateControl::RegisterControl(FN_STAT_TEMPLATE, pMod );
    SwViewLayoutControl::RegisterControl( SID_ATTR_VIEWLAYOUT, pMod );
    SvxModifyControl::RegisterControl( SID_DOC_MODIFIED, pMod );
    SvxZoomSliderControl::RegisterControl( SID_ATTR_ZOOMSLIDER, pMod );

    SwTableOptimizeCtrl::RegisterControl(FN_OPTIMIZE_TABLE, pMod);

    SvxIMapDlgChildWindow::RegisterChildWindow( sal_False, pMod );
    SvxSearchDialogWrapper::RegisterChildWindow( sal_False, pMod );
    SvxHlinkDlgWrapper::RegisterChildWindow( sal_False, pMod );
    SvxFontWorkChildWindow::RegisterChildWindow( sal_False, pMod );
    SwFldDlgWrapper::RegisterChildWindow( sal_False, pMod );
    SwFldDataOnlyDlgWrapper::RegisterChildWindow( sal_False, pMod );
    SvxContourDlgChildWindow::RegisterChildWindow( sal_False, pMod );
    SwNavigationChild::RegisterChildWindowContext( pMod );
    SwInputChild::RegisterChildWindow( sal_False, pMod, SFX_CHILDWIN_FORCEDOCK );
    SwRedlineAcceptChild::RegisterChildWindow( sal_False, pMod );
    SwSyncChildWin::RegisterChildWindow( sal_True, pMod );
    SwMailMergeChildWindow::RegisterChildWindow( sal_False, pMod );
    SwInsertIdxMarkWrapper::RegisterChildWindow( sal_False, pMod );
    SwInsertAuthMarkWrapper::RegisterChildWindow( sal_False, pMod );
    SwWordCountWrapper::RegisterChildWindow( sal_False, pMod );
    SvxRubyChildWindow::RegisterChildWindow( sal_False, pMod);
    SwSpellDialogChildWindow::RegisterChildWindow(sal_False, pMod);

    SvxGrafRedToolBoxControl::RegisterControl( SID_ATTR_GRAF_RED, pMod );
    SvxGrafGreenToolBoxControl::RegisterControl( SID_ATTR_GRAF_GREEN, pMod );
    SvxGrafBlueToolBoxControl::RegisterControl( SID_ATTR_GRAF_BLUE, pMod );
    SvxGrafLuminanceToolBoxControl::RegisterControl( SID_ATTR_GRAF_LUMINANCE, pMod );
    SvxGrafContrastToolBoxControl::RegisterControl( SID_ATTR_GRAF_CONTRAST, pMod );
    SvxGrafGammaToolBoxControl::RegisterControl( SID_ATTR_GRAF_GAMMA, pMod );
    SvxGrafTransparenceToolBoxControl::RegisterControl( SID_ATTR_GRAF_TRANSPARENCE, pMod );
    SvxGrafModeToolBoxControl::RegisterControl( SID_ATTR_GRAF_MODE, pMod );
    SvxGrafFilterToolBoxControl::RegisterControl( SID_GRFFILTER, pMod );
    SvxVertTextTbxCtrl::RegisterControl(SID_TEXTDIRECTION_LEFT_TO_RIGHT, pMod);
    SvxVertTextTbxCtrl::RegisterControl(SID_TEXTDIRECTION_TOP_TO_BOTTOM, pMod);
    SvxVertTextTbxCtrl::RegisterControl(SID_DRAW_CAPTION_VERTICAL, pMod);
    SvxVertTextTbxCtrl::RegisterControl(SID_DRAW_TEXT_VERTICAL, pMod);

    SvxCTLTextTbxCtrl::RegisterControl(SID_ATTR_PARA_LEFT_TO_RIGHT, pMod);
    SvxCTLTextTbxCtrl::RegisterControl(SID_ATTR_PARA_RIGHT_TO_LEFT, pMod);

    GalleryChildWindow::RegisterChildWindow(0, pMod);

    ::avmedia::MediaToolBoxControl::RegisterControl(SID_AVMEDIA_TOOLBOX, pMod);
    ::avmedia::MediaPlayer::RegisterChildWindow(0, pMod);

    SvxSmartTagsControl::RegisterControl(SID_OPEN_SMARTTAGMENU, pMod);
    ::sfx2::sidebar::SidebarChildWindow::RegisterChildWindowIfEnabled(0, pMod);
    ::sfx2::TaskPaneWrapper::RegisterChildWindow(0, pMod);
}



/*************************************************************************
|*
|* Load Module (only dummy for linking of the DLL)
|*
\************************************************************************/

void    SwModule::InitAttrPool()
{
    OSL_ENSURE(!pAttrPool, "Pool already exists!");
    pAttrPool = new SwAttrPool(0);
    SetPool(pAttrPool);
}

void    SwModule::RemoveAttrPool()
{
    SetPool(0);
    SfxItemPool::Free(pAttrPool);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
