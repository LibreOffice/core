/*************************************************************************
 *
 *  $RCSfile: swmodule.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: os $ $Date: 2000-10-20 14:18:01 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include "hintids.hxx"
#include "uiparam.hxx"
#include "swerror.h"

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
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _EHDL_HXX //autogen
#include <svtools/ehdl.hxx>
#endif
#ifndef _OFAACCFG_HXX //autogen
#include <offmgr/ofaaccfg.hxx>
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
#include <basctl/btbxctl.hxx>           //z-Zt falscher includeschutz!
#include <svx/tbxctl.hxx>           //z-Zt falscher includeschutz!
#ifndef _FILLCTRL_HXX //autogen
#include <svx/fillctrl.hxx>
#endif
#ifndef _SVX_TBCONTRL_HXX //autogen
#include <svx/tbcontrl.hxx>
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
#ifndef _SVX_DLG_HYPERLINK_HXX //autogen
#include <offmgr/hyprlink.hxx>
#endif
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
#include "docsh.hxx"
#include "swmodule.hxx"
#include <swtypes.hxx>
#include <swevent.hxx>
#include <swacorr.hxx>
#include "cmdid.h"
#include <dobjfac.hxx>
#include <init.hxx>
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
#include "drformsh.hxx"
#include "wgrfsh.hxx"
#include "wolesh.hxx"
#include "wlistsh.hxx"
#include "wtabsh.hxx"
#include "wdrwbase.hxx"
#ifndef _NAVIPI_HXX //autogen
#include <navipi.hxx>
#endif
#ifndef _SRCVCFG_HXX //autogen
#include <srcvcfg.hxx>
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
#include "bookctrl.hxx"
#include "tmplctrl.hxx"
#include "tblctrl.hxx"
#include "zoomctrl.hxx"
#include "swstbctl.hxx"
#include "workctrl.hxx"
#include "numctrl.hxx"
#include "tbxanchr.hxx"
#include "fldwrap.hxx"
#include "redlndlg.hxx"
#include "syncbtn.hxx"
#ifndef _HDFTCTRL_HXX
#include <hdftctrl.hxx>
#endif
#ifndef _MODOPT_HXX //autogen
#include <modcfg.hxx>
#endif
#ifndef _FONTCFG_HXX //autogen
#include <fontcfg.hxx>
#endif
#ifndef _UNOMOD_HXX
#include <unomod.hxx>
#endif
#ifndef _UNOATXT_HXX
#include <unoatxt.hxx>
#endif
#ifndef _SFX_APPUNO_HXX //autogen
#include <sfx2/appuno.hxx>
#endif
#ifndef _SWATRSET_HXX //autogen
#include <swatrset.hxx>
#endif
#ifndef _IDXMRK_HXX
#include "idxmrk.hxx"
#endif
#ifndef _DLELSTNR_HXX_
#include <dlelstnr.hxx>
#endif

#include "barcfg.hxx"

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

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::scanner;
using namespace ::com::sun::star::lang;
using namespace ::rtl;
#define C2S(cChar) String::CreateFromAscii(cChar)

TYPEINIT1( SwModuleDummy, SfxModule );
TYPEINIT1( SwModule, SwModuleDummy );


//************************************************************************

SwModule::SwModule( SvFactory* pFact,
                    SvFactory* pWebFact,
                    SvFactory* pGlobalFact )
    : SwModuleDummy( SFX_APP()->CreateResManager( "sw" ), sal_False, pFact,
                     pWebFact, pGlobalFact ),
    pSrcViewConfig(0),
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
    pClipboard(0),
    pDragDrop(0),
    pAttrPool(0),
    bAuthorInitialised(sal_False)
{
    SetName( String::CreateFromAscii("StarWriter") );
    pSwResMgr = GetResMgr();
    pErrorHdl = new SfxErrorHandler( RID_SW_ERRHDL,
                                     ERRCODE_AREA_SW,
                                     ERRCODE_AREA_SW_END,
                                     pSwResMgr );
    OFF_APP()->RegisterEvent(SW_EVENT_MAIL_MERGE, SW_RES(STR_PRINT_MERGE_MACRO));
    OFF_APP()->RegisterEvent(SW_EVENT_PAGE_COUNT, SW_RES(STR_PAGE_COUNT_MACRO));
    pModuleConfig = new SwModuleOptions;

    //Die brauchen wie sowieso
    pToolbarConfig = new SwToolbarConfigItem( sal_False );
    pWebToolbarConfig = new SwToolbarConfigItem( sal_True );

    pStdFontConfig = new SwStdFontConfig;

    pAuthorNames = new SvStringsDtor(5, 1); // Alle Redlining-Autoren

    //JP 18.10.96: SvxAutocorrect gegen die SwAutocorrect austauschen
    OfficeApplication* pOffApp = OFF_APP();
    OfaAutoCorrCfg* pACfg = pOffApp->GetAutoCorrConfig();
    if( pACfg )
    {
        const SvxAutoCorrect* pOld = pACfg->GetAutoCorrect();
        pACfg->SetAutoCorrect(new SwAutoCorrect( *pOld ));
    }
    StartListening( *pOffApp );

    xDicListEvtListener = uno::Reference< linguistic::XDictionaryListEventListener > (
            new SwDicListEvtListener( ::GetDictionaryList() ) );

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

void SwDLL::RegisterFactories()
{
    //Diese Id's duerfen nicht geaendert werden. Mittels der Id's wird vom
    //Sfx die sdbcx::View (Dokumentansicht wiederherstellen) erzeugt.
    SwView::RegisterFactory         ( 2 );
    SwWebView::RegisterFactory      ( 5 );
    SwSrcView::RegisterFactory      ( 6 );
    SwPagePreView::RegisterFactory  ( 7 );
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

    uno::Reference< lang::XMultiServiceFactory >  xMgr = comphelper::getProcessServiceFactory();
    uno::Reference< container::XSet >  xSet(xMgr, uno::UNO_QUERY);
    if (xSet.is())
    {
        OUString sString = C2S("com.sun.star.text.AutoTextContainer");

        uno::Reference< lang::XSingleServiceFactory >  xSingleFactory =
            cppu::createOneInstanceFactory(
                xMgr,
                OUString(),
                SwXAutoTextContainer_CreateInstance,
                uno::Sequence<OUString>(&sString, 1));

        if (xSingleFactory.is())
            xSet->insert(uno::Any(&xSingleFactory, ::getCppuType((uno::Reference< lang::XSingleServiceFactory> *)0)));

        sString = C2S("com.sun.star.text.StarWriter");
        xSingleFactory = cppu::createOneInstanceFactory(
                xMgr,
                OUString(),
                SwXModule_CreateInstance,
                uno::Sequence<OUString>(&sString, 1));

        if (xSingleFactory.is())
            xSet->insert(uno::Any(&xSingleFactory, ::getCppuType((uno::Reference< lang::XSingleServiceFactory>*)0)));
    }
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
    SvxColumnsToolBoxControl::RegisterControl(FN_INSERT_COLUMN_SECTION, pMod );
    SvxTableToolBoxControl::RegisterControl(FN_INSERT_TABLE, pMod );
    SvxTableToolBoxControl::RegisterControl(FN_PREVIEW_ZOOM, pMod );

    SvxFontMenuControl::RegisterControl(SID_ATTR_CHAR_FONT, pMod );
    SvxFontSizeMenuControl::RegisterControl(SID_ATTR_CHAR_FONTHEIGHT, pMod );

    SwZoomControl::RegisterControl(SID_ATTR_ZOOM, pMod );
    SwHyperlinkControl::RegisterControl(FN_STAT_HYPERLINKS, pMod );
    SvxPosSizeStatusBarControl::RegisterControl(0, pMod );
    SvxInsertStatusBarControl::RegisterControl(0, pMod );
    SvxSelectionModeControl::RegisterControl(FN_STAT_SELMODE, pMod );

    SwBookmarkControl::RegisterControl(FN_STAT_PAGE, pMod );
    SwTemplateControl::RegisterControl(FN_STAT_TEMPLATE, pMod );

    SwTableOptimizeCtrl::RegisterControl(FN_OPTIMIZE_TABLE, pMod);

    SfxMenuControl::RegisterControl(FN_FRAME_ALIGN_VERT_TOP, pMod );
    SfxMenuControl::RegisterControl(FN_FRAME_ALIGN_VERT_BOTTOM, pMod );
    SfxMenuControl::RegisterControl(FN_FRAME_ALIGN_VERT_CENTER, pMod );

    SwHeadFootMenuControl::RegisterControl( FN_INSERT_PAGEHEADER, pMod );
    SwHeadFootMenuControl::RegisterControl( FN_INSERT_PAGEFOOTER, pMod );

    SvxHyperlinkDlgWrapper::RegisterChildWindow( sal_False, pMod );
    SvxFontWorkChildWindow::RegisterChildWindow( sal_False, pMod );
    SwFldDlgWrapper::RegisterChildWindow( sal_False, pMod );
    SvxContourDlgChildWindow::RegisterChildWindow( sal_False, pMod );
    SwInsertChartChild::RegisterChildWindow( sal_False, pMod );
    SwNavigationChild::RegisterChildWindowContext( pMod );
    SwInputChild::RegisterChildWindow( sal_False, pMod );
    SwRedlineAcceptChild::RegisterChildWindow( sal_False, pMod );
    SwSyncChildWin::RegisterChildWindow( sal_True, pMod );
    SwInsertIdxMarkWrapper::RegisterChildWindow( sal_False, pMod );
    SwInsertAuthMarkWrapper::RegisterChildWindow( sal_False, pMod );

    SvxGrafRedToolBoxControl::RegisterControl( SID_ATTR_GRAF_RED, pMod );
    SvxGrafGreenToolBoxControl::RegisterControl( SID_ATTR_GRAF_GREEN, pMod );
    SvxGrafBlueToolBoxControl::RegisterControl( SID_ATTR_GRAF_BLUE, pMod );
    SvxGrafLuminanceToolBoxControl::RegisterControl( SID_ATTR_GRAF_LUMINANCE, pMod );
    SvxGrafContrastToolBoxControl::RegisterControl( SID_ATTR_GRAF_CONTRAST, pMod );
    SvxGrafGammaToolBoxControl::RegisterControl( SID_ATTR_GRAF_GAMMA, pMod );
    SvxGrafTransparenceToolBoxControl::RegisterControl( SID_ATTR_GRAF_TRANSPARENCE, pMod );
    SvxGrafModeToolBoxControl::RegisterControl( SID_ATTR_GRAF_MODE, pMod );
    GalleryChildWindow::RegisterChildWindow(0, pMod);
}


/*************************************************************************
|*
|* Modul laden (nur Attrappe fuer das Linken der DLL)
|*
\************************************************************************/


SfxModule* SwModuleDummy::Load()
{
    return (NULL);
}

SwModuleDummy::~SwModuleDummy()
{
}


/*************************************************************************
|*
|* Modul laden
|*
\************************************************************************/

SfxModule* SwModule::Load()
{
    return (this);
}

/*-----------------15.03.98 11:50-------------------

--------------------------------------------------*/
/*Reflection*   SwModule::GetReflection( UsrUik aUIK )
{
    DBG_ERROR("GetReflection - new method not yet available")
    return 0;

    if(aUIK == ::getCppuType((const uno::Reference< text::XModule >*)0))
        return ::getCppuType((const SwXModule*)0)();
    else if(aUIK == ::getCppuType((const uno::Reference< text::XAutoTextContainer >*)0)())
        return ::getCppuType((const SwXAutoTextContainer*)0)();
    else
        return SfxModule::GetReflection(aUIK);
}*/
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


