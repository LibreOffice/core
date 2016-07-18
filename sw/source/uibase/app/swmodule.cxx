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

#include <config_features.h>

#include <hintids.hxx>
#include <swerror.h>
#include <vcl/wrkwin.hxx>
#include <vcl/graph.hxx>
#include <svtools/ehdl.hxx>
#include <svx/ParaLineSpacingPopup.hxx>
#include <svx/TextCharacterSpacingPopup.hxx>
#include <svx/TextUnderlinePopup.hxx>
#include <svx/ParaSpacingControl.hxx>
#include <svx/svdobj.hxx>
#include <svx/pszctrl.hxx>
#include <svx/insctrl.hxx>
#include <svx/selctrl.hxx>
#include <svx/linectrl.hxx>
#include <svx/tbxctl.hxx>
#include <svx/fillctrl.hxx>
#include <svx/tbcontrl.hxx>
#include <svx/verttexttbxctrl.hxx>
#include <svx/formatpaintbrushctrl.hxx>
#include <svx/contdlg.hxx>
#include <svx/layctrl.hxx>
#include <svx/fontwork.hxx>
#include <SwSpellDialogChildWindow.hxx>
#include <svx/grafctrl.hxx>
#include <svx/tbxcolor.hxx>
#include <svx/clipboardctl.hxx>
#include <svx/lboxctrl.hxx>
#include <svx/imapdlg.hxx>
#include <svx/srchdlg.hxx>
#include <svx/hyperdlg.hxx>
#include <svx/modctrl.hxx>
#include <com/sun/star/scanner/ScannerManager.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/linguistic2/LanguageGuessing.hpp>
#include <comphelper/processfactory.hxx>
#include <docsh.hxx>
#include <swmodule.hxx>
#include <swevent.hxx>
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
#include <zoomctrl.hxx>
#include <wordcountctrl.hxx>
#include <workctrl.hxx>
#include <fldwrap.hxx>
#include <redlndlg.hxx>
#include <syncbtn.hxx>
#include <modcfg.hxx>
#include <fontcfg.hxx>
#include <sfx2/sidebar/SidebarChildWindow.hxx>
#include <sfx2/evntconf.hxx>
#include <swatrset.hxx>
#include <idxmrk.hxx>
#include <wordcountdialog.hxx>
#include <dlelstnr.hxx>
#include <barcfg.hxx>
#include <svx/rubydialog.hxx>
#include <svtools/colorcfg.hxx>

#include <unotools/configmgr.hxx>
#include <unotools/moduleoptions.hxx>

#include <avmedia/mediaplayer.hxx>
#include <avmedia/mediatoolbox.hxx>

#include <annotsh.hxx>
#include <navsh.hxx>

#include <app.hrc>
#include <svx/xmlsecctrl.hxx>
ResMgr *pSwResMgr = nullptr;
bool     g_bNoInterrupt     = false;

#include <sfx2/app.hxx>

#include <svx/svxerr.hxx>

#include <unomid.h>

#include "swdllimpl.hxx"

using namespace com::sun::star;


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

SwModule::SwModule( SfxObjectFactory* pWebFact,
                    SfxObjectFactory* pFact,
                    SfxObjectFactory* pGlobalFact )
    : SfxModule( ResMgr::CreateResMgr( "sw" ), {pWebFact, pFact, pGlobalFact} ),
    m_pModuleConfig(nullptr),
    m_pUsrPref(nullptr),
    m_pWebUsrPref(nullptr),
    m_pPrintOptions(nullptr),
    m_pWebPrintOptions(nullptr),
    m_pChapterNumRules(nullptr),
    m_pStdFontConfig(nullptr),
    m_pNavigationConfig(nullptr),
    m_pToolbarConfig(nullptr),
    m_pWebToolbarConfig(nullptr),
    m_pDBConfig(nullptr),
    m_pColorConfig(nullptr),
    m_pAccessibilityOptions(nullptr),
    m_pCTLOptions(nullptr),
    m_pUserOptions(nullptr),
    m_pAttrPool(nullptr),
    m_pView(nullptr),
    m_bAuthorInitialised(false),
    m_bEmbeddedLoadSave( false ),
    m_pDragDrop( nullptr ),
    m_pXSelection( nullptr )
{
    SetName( "StarWriter" );
    pSwResMgr = GetResMgr();
    SvxErrorHandler::ensure();
    m_pErrorHandler = new SfxErrorHandler( RID_SW_ERRHDL,
                                     ERRCODE_AREA_SW,
                                     ERRCODE_AREA_SW_END,
                                     pSwResMgr );

    m_pModuleConfig = new SwModuleOptions;

    // We need them anyways
    m_pToolbarConfig = new SwToolbarConfigItem( false );
    m_pWebToolbarConfig = new SwToolbarConfigItem( true );

    m_pStdFontConfig = new SwStdFontConfig;

    m_pAuthorNames = new std::vector<OUString>;  // All Redlining-Authors

    StartListening( *SfxGetpApp() );

    if (!utl::ConfigManager::IsAvoidConfig())
    {
        // init color configuration
        // member <pColorConfig> is created and the color configuration is applied
        // at the view options.
        GetColorConfig();
    }
}
uno::Reference< scanner::XScannerManager2 > const &
SwModule::GetScannerManager()
{
    if (!m_xScannerManager.is())
    {
        m_xScannerManager = scanner::ScannerManager::create( comphelper::getProcessComponentContext() );
    }
    return m_xScannerManager;
}

uno::Reference< linguistic2::XLanguageGuessing > const & SwModule::GetLanguageGuesser()
{
    if (!m_xLanguageGuesser.is())
    {
        m_xLanguageGuesser = linguistic2::LanguageGuessing::create( comphelper::getProcessComponentContext() );
    }
    return m_xLanguageGuesser;
}

SwModule::~SwModule()
{
    delete m_pErrorHandler;
    EndListening( *SfxGetpApp() );
}

void SwModule::CreateLngSvcEvtListener()
{
    if (!m_xLinguServiceEventListener.is())
        m_xLinguServiceEventListener = new SwLinguServiceEventListener;
}

void SwDLL::RegisterFactories()
{
    // These Id's must not be changed. Through these Id's the View (resume Documentview)
    // is created by Sfx.
    if (!utl::ConfigManager::IsAvoidConfig() && SvtModuleOptions().IsWriter())
        SwView::RegisterFactory         ( 2 );

#if HAVE_FEATURE_DESKTOP
    SwWebView::RegisterFactory        ( 5 );

    if (!utl::ConfigManager::IsAvoidConfig() && SvtModuleOptions().IsWriter())
    {
        SwSrcView::RegisterFactory      ( 6 );
        SwPagePreview::RegisterFactory  ( 7 );
    }
#endif
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
    SwPagePreview::RegisterInterface( pMod );
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

    SvxTbxCtlDraw::RegisterControl(SID_INSERT_DRAW, pMod );
    SvxTbxCtlDraw::RegisterControl(SID_TRACK_CHANGES_BAR, pMod );
    SwTbxAutoTextCtrl::RegisterControl(FN_GLOSSARY_DLG, pMod );
    svx::ParaLineSpacingPopup::RegisterControl(SID_ATTR_PARA_LINESPACE, pMod);
    svx::TextCharacterSpacingPopup::RegisterControl(SID_ATTR_CHAR_KERNING, pMod);
    svx::TextUnderlinePopup::RegisterControl(SID_ATTR_CHAR_UNDERLINE, pMod);
    svx::ParaULSpacingControl::RegisterControl(SID_ATTR_PARA_ULSPACE, pMod);
    svx::ParaLRSpacingControl::RegisterControl(SID_ATTR_PARA_LRSPACE, pMod);

    SvxColorToolBoxControl::RegisterControl( SID_EXTRUSION_3D_COLOR, pMod );

    SvxClipBoardControl::RegisterControl(SID_PASTE, pMod );
    SvxUndoRedoControl::RegisterControl(SID_UNDO, pMod );
    SvxUndoRedoControl::RegisterControl(SID_REDO, pMod );
    svx::FormatPaintBrushToolBoxControl::RegisterControl(SID_FORMATPAINTBRUSH, pMod );

    SvxFillToolBoxControl::RegisterControl(SID_ATTR_FILL_STYLE, pMod );
    SvxLineStyleToolBoxControl::RegisterControl(SID_ATTR_LINE_STYLE, pMod );
    SvxLineWidthToolBoxControl::RegisterControl(SID_ATTR_LINE_WIDTH, pMod );
    SvxColorToolBoxControl::RegisterControl(SID_ATTR_LINE_COLOR, pMod );
    SvxLineEndToolBoxControl::RegisterControl(SID_ATTR_LINEEND_STYLE, pMod );
    SvxColorToolBoxControl::RegisterControl(SID_ATTR_FILL_COLOR, pMod);
    SvxColorToolBoxControl::RegisterControl(SID_ATTR_CHAR_BACK_COLOR, pMod);

    SvxFontNameToolBoxControl::RegisterControl(SID_ATTR_CHAR_FONT, pMod );
    SvxColorToolBoxControl::RegisterControl(SID_ATTR_CHAR_COLOR, pMod );
    SvxColorToolBoxControl::RegisterControl(SID_ATTR_CHAR_COLOR2, pMod );
    SvxColorToolBoxControl::RegisterControl(SID_ATTR_CHAR_COLOR_BACKGROUND, pMod );
    SvxStyleToolBoxControl::RegisterControl(SID_STYLE_APPLY, pMod );
    SvxColorToolBoxControl::RegisterControl( SID_BACKGROUND_COLOR, pMod );
    SvxFrameToolBoxControl::RegisterControl(SID_ATTR_BORDER, pMod );
    SvxFrameLineStyleToolBoxControl::RegisterControl(SID_FRAME_LINESTYLE, pMod );
    SvxColorToolBoxControl::RegisterControl(SID_FRAME_LINECOLOR, pMod );

    SvxColumnsToolBoxControl::RegisterControl(FN_INSERT_FRAME_INTERACT, pMod );
    SvxColumnsToolBoxControl::RegisterControl(FN_INSERT_FRAME, pMod );
    SvxColumnsToolBoxControl::RegisterControl(FN_INSERT_REGION, pMod );
    SvxTableToolBoxControl::RegisterControl(FN_INSERT_TABLE, pMod );
    SvxTableToolBoxControl::RegisterControl(FN_SHOW_MULTIPLE_PAGES, pMod );

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

    SvxIMapDlgChildWindow::RegisterChildWindow( false, pMod );
    SvxSearchDialogWrapper::RegisterChildWindow( false, pMod );
    SvxHlinkDlgWrapper::RegisterChildWindow( false, pMod );
    SvxFontWorkChildWindow::RegisterChildWindow( false, pMod );
    SwFieldDlgWrapper::RegisterChildWindow( false, pMod );
    SwFieldDataOnlyDlgWrapper::RegisterChildWindow( false, pMod );
    SvxContourDlgChildWindow::RegisterChildWindow( false, pMod );
    SwNavigationChild::RegisterChildWindowContext( pMod );
    SwInputChild::RegisterChildWindow( false, pMod, SfxChildWindowFlags::FORCEDOCK );
    SwRedlineAcceptChild::RegisterChildWindow( false, pMod );
    SwSyncChildWin::RegisterChildWindow( true, pMod );
    SwInsertIdxMarkWrapper::RegisterChildWindow( false, pMod );
    SwInsertAuthMarkWrapper::RegisterChildWindow( false, pMod );
    SwWordCountWrapper::RegisterChildWindow( false, pMod );
    SvxRubyChildWindow::RegisterChildWindow( false, pMod);
    SwSpellDialogChildWindow::RegisterChildWindow(false, pMod);

    SvxGrafRedToolBoxControl::RegisterControl( SID_ATTR_GRAF_RED, pMod );
    SvxGrafGreenToolBoxControl::RegisterControl( SID_ATTR_GRAF_GREEN, pMod );
    SvxGrafBlueToolBoxControl::RegisterControl( SID_ATTR_GRAF_BLUE, pMod );
    SvxGrafLuminanceToolBoxControl::RegisterControl( SID_ATTR_GRAF_LUMINANCE, pMod );
    SvxGrafContrastToolBoxControl::RegisterControl( SID_ATTR_GRAF_CONTRAST, pMod );
    SvxGrafGammaToolBoxControl::RegisterControl( SID_ATTR_GRAF_GAMMA, pMod );
    SvxGrafTransparenceToolBoxControl::RegisterControl( SID_ATTR_GRAF_TRANSPARENCE, pMod );
    SvxGrafModeToolBoxControl::RegisterControl( SID_ATTR_GRAF_MODE, pMod );
    SvxVertTextTbxCtrl::RegisterControl(SID_TEXTDIRECTION_LEFT_TO_RIGHT, pMod);
    SvxVertTextTbxCtrl::RegisterControl(SID_TEXTDIRECTION_TOP_TO_BOTTOM, pMod);
    SvxVertTextTbxCtrl::RegisterControl(SID_DRAW_CAPTION_VERTICAL, pMod);
    SvxVertTextTbxCtrl::RegisterControl(SID_DRAW_TEXT_VERTICAL, pMod);

    SvxCTLTextTbxCtrl::RegisterControl(SID_ATTR_PARA_LEFT_TO_RIGHT, pMod);
    SvxCTLTextTbxCtrl::RegisterControl(SID_ATTR_PARA_RIGHT_TO_LEFT, pMod);

    ::avmedia::MediaToolBoxControl::RegisterControl(SID_AVMEDIA_TOOLBOX, pMod);
    ::avmedia::MediaPlayer::RegisterChildWindow(false, pMod);

    ::sfx2::sidebar::SidebarChildWindow::RegisterChildWindow(false, pMod);

    SwJumpToSpecificPageControl::RegisterControl(SID_JUMP_TO_SPECIFIC_PAGE, pMod);
}

// Load Module (only dummy for linking of the DLL)
void    SwModule::InitAttrPool()
{
    OSL_ENSURE(!m_pAttrPool, "Pool already exists!");
    m_pAttrPool = new SwAttrPool(nullptr);
    SetPool(m_pAttrPool);
}

void    SwModule::RemoveAttrPool()
{
    SetPool(nullptr);
    SfxItemPool::Free(m_pAttrPool);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
