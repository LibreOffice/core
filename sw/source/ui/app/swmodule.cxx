/*************************************************************************
 *
 *  $RCSfile: swmodule.cxx,v $
 *
 *  $Revision: 1.40 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 09:57:47 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _UIPARAM_HXX
#include <uiparam.hxx>
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
#ifndef _TBXALIGN_HXX //autogen
#include <svx/tbxalign.hxx>
#endif
#ifndef _SVX_GRAFCTRL_HXX
#include <svx/grafctrl.hxx>
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
#include <svx/imapdlg.hxx>
#include <svx/srchdlg.hxx>
#include <svx/hyperdlg.hxx>
#ifndef _SVSTDARR_STRINGSDTOR
#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SCANNER_XSCANNERMANAGER_HPP_
#include <com/sun/star/scanner/XScannerManager.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XSET_HPP_
#include <com/sun/star/container/XSet.hpp>
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _SVX_VERT_TEXT_TBXCTRL_HXX
#include <svx/verttexttbxctrl.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
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
#ifndef _WDRWBASE_HXX
#include <wdrwbase.hxx>
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
#ifndef _TBLCTRL_HXX
#include <tblctrl.hxx>
#endif
#ifndef _ZOOMCTRL_HXX
#include <zoomctrl.hxx>
#endif
#ifndef _SWSTBCTL_HXX
#include <swstbctl.hxx>
#endif
#ifndef _WORKCTRL_HXX
#include <workctrl.hxx>
#endif
#ifndef _NUMCTRL_HXX
#include <numctrl.hxx>
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

#include <app.hrc>

ResMgr *pSwResMgr = 0;
sal_Bool    bNoInterrupt    = sal_False;

#ifndef PROFILE
// Code zum Initialisieren von Statics im eigenen Code-Segment
#pragma code_seg( "SWSTATICS" )
#endif

#ifndef PROFILE
#pragma code_seg()
#endif

#include <svx/svxerr.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::scanner;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;
using namespace ::rtl;

#define C2S(cChar) String::CreateFromAscii(cChar)

TYPEINIT1( SwModule, SfxModule );


//************************************************************************

SwModule::SwModule( SfxObjectFactory* pWebFact,
                    SfxObjectFactory* pFact,
                    SfxObjectFactory* pGlobalFact )
    : SfxModule( SfxApplication::CreateResManager( "sw" ), sal_False, pWebFact,
                     pFact, pGlobalFact, NULL ),
    pModuleConfig(0),
    pView(0),
    pChapterNumRules(0),
    pStdFontConfig(0),
    pNavigationConfig(0),
    pPrtOpt(0),
    pWebPrtOpt(0),
    pWebUsrPref(0),
    pUsrPref(0),
    pToolbarConfig(0),
    pWebToolbarConfig(0),
    pDBConfig(0),
    pColorConfig(0),
    pAccessibilityOptions(0),
    pCTLOptions(0),
    pUserOptions(0),
    pUndoOptions(0),
    pClipboard(0),
    pDragDrop(0),
    pXSelection(0),
    pAttrPool(0),
    bAuthorInitialised(sal_False),
    bEmbeddedLoadSave( sal_False )
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

    Reference< XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
    if( xMgr.is() )
    {
        m_xScannerManager = Reference< XScannerManager >(
                        xMgr->createInstance( OUString::createFromAscii( "com.sun.star.scanner.ScannerManager" ) ),
                        UNO_QUERY );

//      if( m_xScannerManager.is() )
//      {
//          m_xScannerListener = Reference< lang::XEventListener >(
//                                      OWeakObject* ( new ScannerEventListener( this ) ), UNO_QUERY );
//      }
    }

    // OD 14.02.2003 #107424# - init color configuration
    // member <pColorConfig> is created and the color configuration is applied
    // at the view options.
    GetColorConfig();
}

//************************************************************************

SwModule::~SwModule()
{
    SetPool(0);
    delete pAttrPool;
    delete pErrorHdl;
    EndListening( *OFF_APP() );
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
    SwWebDrawBaseShell::RegisterInterface(pMod);
    SwWebDrawFormShell::RegisterInterface(pMod);
    SwWebOleShell::RegisterInterface(pMod);
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
    SvxFontHeightToolBoxControl::RegisterControl(SID_ATTR_CHAR_FONTHEIGHT, pMod );
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
    SwHyperlinkControl::RegisterControl(FN_STAT_HYPERLINKS, pMod );
    SvxPosSizeStatusBarControl::RegisterControl(0, pMod );
    SvxInsertStatusBarControl::RegisterControl(0, pMod );
    SvxSelectionModeControl::RegisterControl(FN_STAT_SELMODE, pMod );

    SwBookmarkControl::RegisterControl(FN_STAT_PAGE, pMod );
    SwTemplateControl::RegisterControl(FN_STAT_TEMPLATE, pMod );

    SwTableOptimizeCtrl::RegisterControl(FN_OPTIMIZE_TABLE, pMod);

    SvxIMapDlgChildWindow::RegisterChildWindow( sal_False, pMod );
    SvxSearchDialogWrapper::RegisterChildWindow( sal_False, pMod );
    SvxHlinkDlgWrapper::RegisterChildWindow( sal_False, pMod );
    SvxHyperlinkDlgWrapper::RegisterChildWindow( sal_False, pMod );
    SvxFontWorkChildWindow::RegisterChildWindow( sal_False, pMod );
    SwFldDlgWrapper::RegisterChildWindow( sal_False, pMod );
    SwFldDataOnlyDlgWrapper::RegisterChildWindow( sal_False, pMod );
    SvxContourDlgChildWindow::RegisterChildWindow( sal_False, pMod );
    SwInsertChartChild::RegisterChildWindow( sal_False, pMod );
    SwNavigationChild::RegisterChildWindowContext( pMod );
    SwInputChild::RegisterChildWindow( sal_False, pMod );
    SwRedlineAcceptChild::RegisterChildWindow( sal_False, pMod );
    SwSyncChildWin::RegisterChildWindow( sal_True, pMod );
    SwInsertIdxMarkWrapper::RegisterChildWindow( sal_False, pMod );
    SwInsertAuthMarkWrapper::RegisterChildWindow( sal_False, pMod );
    SvxRubyChildWindow::RegisterChildWindow( sal_False, pMod);

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


