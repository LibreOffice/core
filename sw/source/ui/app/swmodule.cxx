/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swmodule.cxx,v $
 *
 *  $Revision: 1.66 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 15:02:41 $
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
#include "precompiled_sw.hxx"


#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _SWERROR_H
#include <swerror.h>
#endif

#ifndef _SV_WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SV_GRAPH_HXX
#include <vcl/graph.hxx>
#endif
#ifndef _SVX_GALBRWS_HXX_
#include <svx/galbrws.hxx>
#endif
#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif
#ifndef _EHDL_HXX //autogen
#include <svtools/ehdl.hxx>
#endif
#ifndef _SVX_FNTSZCTL_HXX //autogen
#include <svx/fntszctl.hxx>
#endif
#ifndef _SVX_FNTCTL_HXX //autogen
#include <svx/fntctl.hxx>
#endif
#ifndef _SVX_SMARTTAGSCONTROL_HXX
#include <svx/SmartTagCtl.hxx>
#endif
#ifndef _SVX_PSZCTRL_HXX //autogen
#include <svx/pszctrl.hxx>
#endif
#ifndef _SVX_INSCTRL_HXX //autogen
#include <svx/insctrl.hxx>
#endif
#ifndef _SVX_SELCTRL_HXX //autogen
#include <svx/selctrl.hxx>
#endif
#ifndef _SVX_LINECTRL_HXX //autogen
#include <svx/linectrl.hxx>
#endif
#include <svx/tbxctl.hxx>           //z-Zt falscher includeschutz!
#ifndef _FILLCTRL_HXX //autogen
#include <svx/fillctrl.hxx>
#endif
#ifndef _SVX_TBCONTRL_HXX //autogen
#include <svx/tbcontrl.hxx>
#endif
#ifndef _SVX_VERT_TEXT_TBXCTRL_HXX
#include <svx/verttexttbxctrl.hxx>
#endif
#ifndef _SVX_FORMATPAINTBRUSHCTRL_HXX
#include <svx/formatpaintbrushctrl.hxx>
#endif
#ifndef _CONTDLG_HXX_ //autogen
#include <svx/contdlg.hxx>
#endif
#ifndef _SVX_LAYCTRL_HXX //autogen
#include <svx/layctrl.hxx>
#endif
#ifndef _SVX_FONTWORK_HXX //autogen
#include <svx/fontwork.hxx>
#endif
#ifndef SW_SPELL_DIALOG_CHILD_WINDOW_HXX
#include <SwSpellDialogChildWindow.hxx>
#endif
#ifndef _TBXALIGN_HXX //autogen
#include <svx/tbxalign.hxx>
#endif
#ifndef _SVX_GRAFCTRL_HXX
#include <svx/grafctrl.hxx>
#endif
#ifndef _SVX_TBXCOLOR_HXX
#include <svx/tbxcolor.hxx>
#endif
#ifndef _SVX_CLIPBOARDCTL_HXX_
#include <svx/clipboardctl.hxx>
#endif
#ifndef _SVX_LBOXCTRL_HXX_
#include <svx/lboxctrl.hxx>
#endif
#ifndef _SVX_EXTRUSION_CONTROLS_HXX
#include <svx/extrusioncontrols.hxx>
#endif
#ifndef _SVX_DLG_HYPERLINK_HXX //autogen
#include <svx/hyprlink.hxx>
#endif
#ifndef _SVX_TBXCUSTOMSHAPES_HXX
#include <svx/tbxcustomshapes.hxx>
#endif
#ifndef _SVX_FONTWORK_GALLERY_DIALOG_HXX
#include <svx/fontworkgallery.hxx>
#endif
#include <svx/imapdlg.hxx>
#include <svx/srchdlg.hxx>
#include <svx/hyperdlg.hxx>
#ifndef _COM_SUN_STAR_SCANNER_XSCANNERMANAGER_HPP_
#include <com/sun/star/scanner/XScannerManager.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XSET_HPP_
#include <com/sun/star/container/XSet.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _SWEVENT_HXX
#include <swevent.hxx>
#endif
#ifndef _SWACORR_HXX
#include <swacorr.hxx>
#endif
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _DOBJFAC_HXX
#include <dobjfac.hxx>
#endif
#ifndef _INIT_HXX
#include <init.hxx>
#endif
#ifndef _SWPVIEW_HXX //autogen
#include <pview.hxx>
#endif
#ifndef _SWWVIEW_HXX //autogen
#include <wview.hxx>
#endif
#ifndef _SWWDOCSH_HXX //autogen
#include <wdocsh.hxx>
#endif
#ifndef _SWGLOBDOCSH_HXX //autogen
#include <globdoc.hxx>
#endif
#ifndef _SRCVIEW_HXX //autogen
#include <srcview.hxx>
#endif
#ifndef _AUTODOC_HXX //autogen
#include <glshell.hxx>
#endif
#ifndef _SWTABSH_HXX //autogen
#include <tabsh.hxx>
#endif
#ifndef _SWLISTSH_HXX //autogen
#include <listsh.hxx>
#endif
#ifndef _SWGRFSH_HXX //autogen
#include <grfsh.hxx>
#endif
#ifndef _SWMEDIASH_HXX //autogen
#include <mediash.hxx>
#endif
#ifndef _SWOLESH_HXX //autogen
#include <olesh.hxx>
#endif
#ifndef _SWDRAWSH_HXX //autogen
#include <drawsh.hxx>
#endif
#ifndef _SWWDRWFORMSH_HXX //autogen
#include <wformsh.hxx>
#endif
#ifndef _SWDRWTXTSH_HXX //autogen
#include <drwtxtsh.hxx>
#endif
#ifndef _SWBEZIERSH_HXX //autogen
#include <beziersh.hxx>
#endif
#ifndef _SWWTEXTSH_HXX //autogen
#include <wtextsh.hxx>
#endif
#ifndef _SWWFRMSH_HXX //autogen
#include <wfrmsh.hxx>
#endif
#ifndef _DRFORMSH_HXX
#include <drformsh.hxx>
#endif
#ifndef _WGRFSH_HXX
#include <wgrfsh.hxx>
#endif
#ifndef _WOLESH_HXX
#include <wolesh.hxx>
#endif
#ifndef _WLISTSH_HXX
#include <wlistsh.hxx>
#endif
#ifndef _WTABSH_HXX
#include <wtabsh.hxx>
#endif
#ifndef _NAVIPI_HXX //autogen
#include <navipi.hxx>
#endif
#ifndef _CHARTINS_HXX //autogen
#include <chartins.hxx>
#endif
#ifndef SW_INPUTWIN_HXX //autogen
#include <inputwin.hxx>
#endif
#ifndef _USRPREF_HXX //autogen
#include <usrpref.hxx>
#endif
#ifndef _UINUMS_HXX //autogen
#include <uinums.hxx>
#endif
#ifndef _PRTOPT_HXX //autogen
#include <prtopt.hxx>
#endif
#ifndef _BOOKCTRL_HXX
#include <bookctrl.hxx>
#endif
#ifndef _TMPLCTRL_HXX
#include <tmplctrl.hxx>
#endif
#ifndef _VIEWLAYOUTCTRL_HXX
#include <viewlayoutctrl.hxx>
#endif
#ifndef _ZOOMSLIDER_STBCONTRL_HXX
#include <svx/zoomsliderctrl.hxx>
#endif
#ifndef _TBLCTRL_HXX
#include <tblctrl.hxx>
#endif
#ifndef _ZOOMCTRL_HXX
#include <zoomctrl.hxx>
#endif
#ifndef _WORKCTRL_HXX
#include <workctrl.hxx>
#endif
#ifndef _TBXANCHR_HXX
#include <tbxanchr.hxx>
#endif
#ifndef _FLDWRAP_HXX
#include <fldwrap.hxx>
#endif
#ifndef _REDLNDLG_HXX
#include <redlndlg.hxx>
#endif
#ifndef _SYNCBTN_HXX
#include <syncbtn.hxx>
#endif
#ifndef _SWMAILMERGECHILDWINDOW_HXX
#include <mailmergechildwindow.hxx>
#endif
#ifndef _MODOPT_HXX //autogen
#include <modcfg.hxx>
#endif
#ifndef _FONTCFG_HXX //autogen
#include <fontcfg.hxx>
#endif
#ifndef _SFX_EVENTCONF_HXX
#include <sfx2/evntconf.hxx>
#endif
#ifndef _SFX_APPUNO_HXX //autogen
#include <sfx2/appuno.hxx>
#endif
#ifndef _SWATRSET_HXX //autogen
#include <swatrset.hxx>
#endif
#ifndef _IDXMRK_HXX
#include <idxmrk.hxx>
#endif
#ifndef _DLELSTNR_HXX_
#include <dlelstnr.hxx>
#endif
#ifndef _BARCFG_HXX
#include <barcfg.hxx>
#endif
#ifndef _SVX_RUBYDLG_HXX_
#include <svx/rubydialog.hxx>
#endif
// OD 14.02.2003 #107424#
#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif

#include <svx/acorrcfg.hxx>
#include <svtools/moduleoptions.hxx>

#ifndef _AVMEDIA_MEDIAPPLAYER_HXX
#include <avmedia/mediaplayer.hxx>
#endif
#ifndef _AVMEDIA_MEDIATOOLBOX_HXX
#include <avmedia/mediatoolbox.hxx>
#endif

#include <annotsh.hxx>

#include <app.hrc>
#include <svx/xmlsecctrl.hxx>
ResMgr *pSwResMgr = 0;
sal_Bool    bNoInterrupt    = sal_False;

#include <sfx2/app.hxx>

#include <svx/svxerr.hxx>

#include <unomid.h>

using namespace com::sun::star;


TYPEINIT1( SwModule, SfxModule );

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

//************************************************************************

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
    pUndoOptions(0),
    pAttrPool(0),
    pView(0),
    bAuthorInitialised(sal_False),
    bEmbeddedLoadSave( sal_False ),
    pClipboard( 0 ),
    pDragDrop( 0 ),
    pXSelection( 0 )
{
    SetName( String::CreateFromAscii("StarWriter") );
    pSwResMgr = GetResMgr();
    SvxErrorHandler::Get();
    pErrorHdl = new SfxErrorHandler( RID_SW_ERRHDL,
                                     ERRCODE_AREA_SW,
                                     ERRCODE_AREA_SW_END,
                                     pSwResMgr );

    SfxEventConfiguration::RegisterEvent(SW_EVENT_MAIL_MERGE, SW_RES(STR_PRINT_MERGE_MACRO), String::CreateFromAscii("OnMailMerge"));
    SfxEventConfiguration::RegisterEvent(SW_EVENT_MAIL_MERGE_END, SW_RES(STR_PRINT_MERGE_MACRO), String::CreateFromAscii("OnMailMergeFinished"));
    SfxEventConfiguration::RegisterEvent(SW_EVENT_FIELD_MERGE, String(), String::CreateFromAscii("OnFieldMerge"));
    SfxEventConfiguration::RegisterEvent(SW_EVENT_FIELD_MERGE_FINISHED, String(), String::CreateFromAscii("OnFieldMergeFinished"));
    SfxEventConfiguration::RegisterEvent(SW_EVENT_PAGE_COUNT, SW_RES(STR_PAGE_COUNT_MACRO), String::CreateFromAscii("OnPageCountChange"));
    pModuleConfig = new SwModuleOptions;

    //Die brauchen wie sowieso
    pToolbarConfig = new SwToolbarConfigItem( sal_False );
    pWebToolbarConfig = new SwToolbarConfigItem( sal_True );

    pStdFontConfig = new SwStdFontConfig;

    pAuthorNames = new SvStringsDtor(5, 1); // Alle Redlining-Autoren

    //JP 18.10.96: SvxAutocorrect gegen die SwAutocorrect austauschen
    SvxAutoCorrCfg* pACfg = SvxAutoCorrCfg::Get();
    if( pACfg )
    {
        const SvxAutoCorrect* pOld = pACfg->GetAutoCorrect();
        pACfg->SetAutoCorrect(new SwAutoCorrect( *pOld ));
    }

    StartListening( *SFX_APP() );

    // OD 14.02.2003 #107424# - init color configuration
    // member <pColorConfig> is created and the color configuration is applied
    // at the view options.
    GetColorConfig();
}

//************************************************************************

uno::Reference< scanner::XScannerManager >
SwModule::GetScannerManager()
{
    if (!m_xScannerManager.is())
    {
        uno::Reference< lang::XMultiServiceFactory > xMgr (
            comphelper::getProcessServiceFactory() );
        if( xMgr.is() )
        {
            m_xScannerManager =
                uno::Reference< scanner::XScannerManager >(
                    xMgr->createInstance(
                        rtl::OUString::createFromAscii(
                            "com.sun.star.scanner.ScannerManager" ) ),
                    uno::UNO_QUERY );
        }
    }
    return m_xScannerManager;
}

uno::Reference< linguistic2::XLanguageGuessing > SwModule::GetLanguageGuesser()
{
    if (!m_xLanguageGuesser.is())
    {
        uno::Reference< lang::XMultiServiceFactory > xMgr ( comphelper::getProcessServiceFactory() );
        if (xMgr.is())
        {
            m_xLanguageGuesser = uno::Reference< linguistic2::XLanguageGuessing >(
                    xMgr->createInstance(
                        rtl::OUString::createFromAscii( "com.sun.star.linguistic2.LanguageGuessing" ) ),
                        uno::UNO_QUERY );
        }
    }
    return m_xLanguageGuesser;
}

//************************************************************************

SwModule::~SwModule()
{
    SetPool(0);
    delete pAttrPool;
    delete pErrorHdl;
    EndListening( *SFX_APP() );
}

//************************************************************************

void SwModule::CreateLngSvcEvtListener()
{
    if (!xLngSvcEvtListener.is())
        xLngSvcEvtListener = new SwLinguServiceEventListener;
}

//************************************************************************

void SwDLL::RegisterFactories()
{
    //Diese Id's duerfen nicht geaendert werden. Mittels der Id's wird vom
    //Sfx die View (Dokumentansicht wiederherstellen) erzeugt.
    if ( SvtModuleOptions().IsWriter() )
        SwView::RegisterFactory         ( 2 );

    SwWebView::RegisterFactory      ( 5 );

    if ( SvtModuleOptions().IsWriter() )
    {
        SwSrcView::RegisterFactory      ( 6 );
        SwPagePreView::RegisterFactory  ( 7 );
    }
}

//************************************************************************


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

//************************************************************************

void SwDLL::RegisterControls()
{
    SwModule* pMod = SW_MOD();
    SvxTbxCtlDraw::RegisterControl(SID_INSERT_DRAW, pMod );
    SvxTbxCtlAlign::RegisterControl(SID_OBJECT_ALIGN, pMod );
    SwTbxAnchor::RegisterControl(FN_TOOL_ANKER, pMod );
    SwTbxInsertCtrl::RegisterControl(FN_INSERT_CTRL, pMod );
    SwTbxInsertCtrl::RegisterControl(FN_INSERT_OBJ_CTRL, pMod );
    SwTbxAutoTextCtrl::RegisterControl(FN_INSERT_FIELD_CTRL, pMod );
    SwTbxAutoTextCtrl::RegisterControl(FN_GLOSSARY_DLG, pMod );

    SvxTbxCtlCustomShapes::RegisterControl( SID_DRAWTBX_CS_BASIC, pMod );
    SvxTbxCtlCustomShapes::RegisterControl( SID_DRAWTBX_CS_SYMBOL, pMod );
    SvxTbxCtlCustomShapes::RegisterControl( SID_DRAWTBX_CS_ARROW, pMod );
    SvxTbxCtlCustomShapes::RegisterControl( SID_DRAWTBX_CS_FLOWCHART, pMod );
    SvxTbxCtlCustomShapes::RegisterControl( SID_DRAWTBX_CS_CALLOUT, pMod );
    SvxTbxCtlCustomShapes::RegisterControl( SID_DRAWTBX_CS_STAR, pMod );

    svx::ExtrusionDepthControl::RegisterControl( SID_EXTRUSION_DEPTH_FLOATER, pMod );
    svx::ExtrusionDirectionControl::RegisterControl( SID_EXTRUSION_DIRECTION_FLOATER, pMod );
    svx::ExtrusionLightingControl::RegisterControl( SID_EXTRUSION_LIGHTING_FLOATER, pMod );
    svx::ExtrusionSurfaceControl::RegisterControl( SID_EXTRUSION_SURFACE_FLOATER, pMod );
    svx::ExtrusionColorControl::RegisterControl( SID_EXTRUSION_3D_COLOR, pMod );

    svx::FontWorkShapeTypeControl::RegisterControl( SID_FONTWORK_SHAPE_TYPE, pMod );
    svx::FontWorkAlignmentControl::RegisterControl( SID_FONTWORK_ALIGNMENT_FLOATER, pMod );
    svx::FontWorkCharacterSpacingControl::RegisterControl( SID_FONTWORK_CHARACTER_SPACING_FLOATER, pMod );

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
//  SvxFontHeightToolBoxControl::RegisterControl(SID_ATTR_CHAR_FONTHEIGHT, pMod );
    SvxFontColorToolBoxControl::RegisterControl(SID_ATTR_CHAR_COLOR, pMod );
    SvxFontColorExtToolBoxControl::RegisterControl(SID_ATTR_CHAR_COLOR2, pMod );
    SvxFontColorExtToolBoxControl::RegisterControl(SID_ATTR_CHAR_COLOR_BACKGROUND, pMod );
    SvxStyleToolBoxControl::RegisterControl(SID_STYLE_APPLY, pMod );
    SvxColorToolBoxControl::RegisterControl(SID_BACKGROUND_COLOR, pMod );
    SvxFrameToolBoxControl::RegisterControl(SID_ATTR_BORDER, pMod );
    SvxFrameLineStyleToolBoxControl::RegisterControl(SID_FRAME_LINESTYLE, pMod );
    SvxFrameLineColorToolBoxControl::RegisterControl(SID_FRAME_LINECOLOR, pMod );

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

    SwBookmarkControl::RegisterControl(FN_STAT_PAGE, pMod );
    SwTemplateControl::RegisterControl(FN_STAT_TEMPLATE, pMod );
    SwViewLayoutControl::RegisterControl( SID_ATTR_VIEWLAYOUT, pMod );
    SvxZoomSliderControl::RegisterControl( SID_ATTR_ZOOMSLIDER, pMod );

    SwTableOptimizeCtrl::RegisterControl(FN_OPTIMIZE_TABLE, pMod);

    SvxIMapDlgChildWindow::RegisterChildWindow( sal_False, pMod );
    SvxSearchDialogWrapper::RegisterChildWindow( sal_False, pMod );
    SvxHlinkDlgWrapper::RegisterChildWindow( sal_False, pMod );
    SvxHyperlinkDlgWrapper::RegisterChildWindow( sal_False, pMod, SFX_CHILDWIN_FORCEDOCK );
    SvxFontWorkChildWindow::RegisterChildWindow( sal_False, pMod );
    SwFldDlgWrapper::RegisterChildWindow( sal_False, pMod );
    SwFldDataOnlyDlgWrapper::RegisterChildWindow( sal_False, pMod );
    SvxContourDlgChildWindow::RegisterChildWindow( sal_False, pMod );
    SwNavigationChild::RegisterChildWindowContext( pMod );
    SwInputChild::RegisterChildWindow( sal_False, pMod, SFX_CHILDWIN_FORCEDOCK );
    SwRedlineAcceptChild::RegisterChildWindow( sal_False, pMod );
    SwSyncChildWin::RegisterChildWindow( sal_True, pMod );
    SwMailMergeChildWindow::RegisterChildWindow( sal_False, pMod );
//    SwSendMailChildWindow::RegisterChildWindow( sal_False, pMod );
    SwInsertIdxMarkWrapper::RegisterChildWindow( sal_False, pMod );
    SwInsertAuthMarkWrapper::RegisterChildWindow( sal_False, pMod );
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

    svx::ExtrusionDepthControl::RegisterControl( SID_EXTRUSION_DEPTH_FLOATER, pMod );
    svx::ExtrusionDirectionControl::RegisterControl( SID_EXTRUSION_DIRECTION_FLOATER, pMod );
    svx::ExtrusionLightingControl::RegisterControl( SID_EXTRUSION_LIGHTING_FLOATER, pMod );
    svx::ExtrusionSurfaceControl::RegisterControl( SID_EXTRUSION_SURFACE_FLOATER, pMod );
    svx::ExtrusionColorControl::RegisterControl( SID_EXTRUSION_3D_COLOR, pMod );

    GalleryChildWindow::RegisterChildWindow(0, pMod);

    ::avmedia::MediaToolBoxControl::RegisterControl(SID_AVMEDIA_TOOLBOX, pMod);
    ::avmedia::MediaPlayer::RegisterChildWindow(0, pMod);

    SvxSmartTagsControl::RegisterControl(SID_OPEN_SMARTTAGMENU, pMod);
}



/*************************************************************************
|*
|* Modul laden (nur Attrappe fuer das Linken der DLL)
|*
\************************************************************************/

/* -----------------20.04.99 10:46-------------------
 *
 * --------------------------------------------------*/
void    SwModule::InitAttrPool()
{
    DBG_ASSERT(!pAttrPool, "Pool ist schon da!")
    pAttrPool = new SwAttrPool(0);
    SetPool(pAttrPool);
}
/* -----------------20.04.99 10:46-------------------
 *
 * --------------------------------------------------*/
void    SwModule::RemoveAttrPool()
{
    SetPool(0);
    DELETEZ(pAttrPool);
}


