/*************************************************************************
 *
 *  $RCSfile: scdll.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: nn $ $Date: 2000-10-20 18:23:17 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

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
#include <svx/flditem.hxx>
#include <svx/modctrl.hxx>
#include <svx/pszctrl.hxx>
#include <svx/fntctl.hxx>
#include <svx/fntszctl.hxx>
#include <svx/grafctrl.hxx>

#include "tbinsert.hxx"

// Child-Windows
#include "reffact.hxx"
#include "navipi.hxx"
#include "inputwin.hxx"
#include <svx/fontwork.hxx>
#include <svx/srchdlg.hxx>
#include <offmgr/hyprlink.hxx>
#include <svx/imapdlg.hxx>

#include "editutil.hxx"
#include <svx/svdfield.hxx>     //  SdrRegisterFieldClasses

#include "dwfunctr.hxx"
#include "acredlin.hxx"

//------------------------------------------------------------------

ScResId::ScResId( USHORT nId ) :
    ResId( nId, SC_MOD()->GetResMgr() )
{
}

//------------------------------------------------------------------

void ScDLL::Init()
{
    // called directly after loading the DLL
    // do whatever you want, you may use Sxx-DLL too

    ScDocumentPool::InitVersionMaps();  // wird im ScModule ctor gebraucht

    // the ScModule must be created
    ScModuleDummy **ppShlPtr = (ScModuleDummy**) GetAppData(SHL_CALC);
#ifndef SO3
    SvFactory *pFact = (*ppShlPtr)->pScDocShellFactory;
#else
    SvFactory *pFact = (SvFactory*)(*ppShlPtr)->pScDocShellFactory;
#endif
    ScLibSignalFunc pFunc = (*ppShlPtr)->pSignalFunc;
    delete (*ppShlPtr);
    ScModule* pMod = new ScModule((SfxObjectFactory*)pFact);
    (*ppShlPtr) = pMod;
    (*ppShlPtr)->pScDocShellFactory = pFact;
    (*ppShlPtr)->pSignalFunc = pFunc;

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
    ScPageBreakShell    ::RegisterInterface(pMod);



    // register your controllers here

    ScDocShell::Factory().RegisterMenuBar( ScResId(SCCFG_MENUBAR) );
    ScDocShell::Factory().RegisterPluginMenuBar( ScResId(SCCFG_PLUGINMENU) );
    ScDocShell::Factory().RegisterAccel( ScResId(SCCFG_ACCELERATOR) );

    //  eigene Controller
    ScTbxInsertCtrl     ::RegisterControl(SID_TBXCTL_INSERT, pMod);
    ScTbxInsertCtrl     ::RegisterControl(SID_TBXCTL_INSCELLS, pMod);
    ScTbxInsertCtrl     ::RegisterControl(SID_TBXCTL_INSOBJ, pMod);

    //  Svx-Toolbox-Controller
    SvxTbxCtlDraw                   ::RegisterControl(SID_INSERT_DRAW, pMod);
    SvxTbxCtlAlign                  ::RegisterControl(SID_OBJECT_ALIGN, pMod);
    SvxFillToolBoxControl           ::RegisterControl(0, pMod);
    SvxLineStyleToolBoxControl      ::RegisterControl(0, pMod);
    SvxLineWidthToolBoxControl      ::RegisterControl(0, pMod);
    SvxLineColorToolBoxControl      ::RegisterControl(0, pMod);
    SvxLineEndToolBoxControl        ::RegisterControl(SID_ATTR_LINEEND_STYLE,   pMod);
    SvxStyleToolBoxControl          ::RegisterControl(SID_STYLE_APPLY,          pMod);
    SvxFontNameToolBoxControl       ::RegisterControl(SID_ATTR_CHAR_FONT,       pMod);
    SvxFontHeightToolBoxControl     ::RegisterControl(SID_ATTR_CHAR_FONTHEIGHT, pMod);
    SvxFontColorToolBoxControl      ::RegisterControl(SID_ATTR_CHAR_COLOR,      pMod);
    SvxColorToolBoxControl          ::RegisterControl(SID_BACKGROUND_COLOR,     pMod);
    SvxFrameToolBoxControl          ::RegisterControl(SID_ATTR_BORDER,          pMod);
    SvxFrameLineStyleToolBoxControl ::RegisterControl(SID_FRAME_LINESTYLE,      pMod);
    SvxFrameLineColorToolBoxControl ::RegisterControl(SID_FRAME_LINECOLOR,      pMod);

    SvxGrafModeToolBoxControl       ::RegisterControl(SID_ATTR_GRAF_MODE,       pMod);
    SvxGrafRedToolBoxControl        ::RegisterControl(SID_ATTR_GRAF_RED,        pMod);
    SvxGrafGreenToolBoxControl      ::RegisterControl(SID_ATTR_GRAF_GREEN,      pMod);
    SvxGrafBlueToolBoxControl       ::RegisterControl(SID_ATTR_GRAF_BLUE,       pMod);
    SvxGrafLuminanceToolBoxControl  ::RegisterControl(SID_ATTR_GRAF_LUMINANCE,  pMod);
    SvxGrafContrastToolBoxControl   ::RegisterControl(SID_ATTR_GRAF_CONTRAST,   pMod);
    SvxGrafGammaToolBoxControl      ::RegisterControl(SID_ATTR_GRAF_GAMMA,      pMod);
    SvxGrafTransparenceToolBoxControl::RegisterControl(SID_ATTR_GRAF_TRANSPARENCE, pMod);

    // Svx-StatusBar-Controller
    SvxInsertStatusBarControl       ::RegisterControl(SID_ATTR_INSERT,      pMod);
    SvxSelectionModeControl         ::RegisterControl(SID_STATUS_SELMODE,   pMod);
    SvxZoomStatusBarControl         ::RegisterControl(SID_ATTR_ZOOM,        pMod);
    SvxModifyControl                ::RegisterControl(SID_DOC_MODIFIED,     pMod);
    SvxPosSizeStatusBarControl      ::RegisterControl(SID_ATTR_SIZE,        pMod);

    // Svx-Menue-Controller
    SvxFontMenuControl              ::RegisterControl(SID_ATTR_CHAR_FONT,       pMod);
    SvxFontSizeMenuControl          ::RegisterControl(SID_ATTR_CHAR_FONTHEIGHT, pMod);

    //  Child-Windows

    // Hack: Eingabezeile mit 42 registrieren, damit sie im PlugIn immer sichtbar ist
    ScInputWindowWrapper        ::RegisterChildWindow(42, pMod, SFX_CHILDWIN_TASK);
    ScNavigatorDialogWrapper    ::RegisterChildWindowContext(pMod);
    ScSolverDlgWrapper          ::RegisterChildWindow(FALSE, pMod);
    ScNameDlgWrapper            ::RegisterChildWindow(FALSE, pMod);
    ScPivotLayoutWrapper        ::RegisterChildWindow(FALSE, pMod);
    ScTabOpDlgWrapper           ::RegisterChildWindow(FALSE, pMod);
    ScFilterDlgWrapper          ::RegisterChildWindow(FALSE, pMod);
    ScSpecialFilterDlgWrapper   ::RegisterChildWindow(FALSE, pMod);
    ScDbNameDlgWrapper          ::RegisterChildWindow(FALSE, pMod);
    ScConsolidateDlgWrapper     ::RegisterChildWindow(FALSE, pMod);
    ScChartDlgWrapper           ::RegisterChildWindow(FALSE, pMod);
    ScPrintAreasDlgWrapper      ::RegisterChildWindow(FALSE, pMod);
    ScCondFormatDlgWrapper      ::RegisterChildWindow(FALSE, pMod);
    ScColRowNameRangesDlgWrapper::RegisterChildWindow(FALSE, pMod);
    ScFormulaDlgWrapper         ::RegisterChildWindow(FALSE, pMod);

    // First docking Window for Calc
    ScFunctionChildWindow       ::RegisterChildWindow(FALSE, pMod);

    // Redlining- Window
    ScAcceptChgDlgWrapper       ::RegisterChildWindow(FALSE, pMod);
    ScSimpleRefDlgWrapper       ::RegisterChildWindow(FALSE, pMod);
    ScHighlightChgDlgWrapper    ::RegisterChildWindow(FALSE, pMod);


    SvxFontWorkChildWindow      ::RegisterChildWindow(FALSE, pMod);
    SvxHyperlinkDlgWrapper      ::RegisterChildWindow(FALSE, pMod);
    SvxIMapDlgChildWindow       ::RegisterChildWindow(FALSE, pMod);

    //  Edit-Engine-Felder, soweit nicht schon in OfficeApplication::Init

    SvClassManager& rClassManager = SvxFieldItem::GetClassManager();
//  rClassManager.SV_CLASS_REGISTER( SvxURLField );
//  rClassManager.SV_CLASS_REGISTER( SvxDateField );
//  rClassManager.SV_CLASS_REGISTER( SvxPageField );
    rClassManager.SV_CLASS_REGISTER( SvxPagesField );
//  rClassManager.SV_CLASS_REGISTER( SvxTimeField );
    rClassManager.SV_CLASS_REGISTER( SvxFileField );
//  rClassManager.SV_CLASS_REGISTER( SvxExtFileField );
    rClassManager.SV_CLASS_REGISTER( SvxTableField );

    SdrRegisterFieldClasses();      // SvDraw-Felder registrieren

    pMod->PutItem( SfxUInt16Item( SID_ATTR_METRIC, pMod->GetAppOptions().GetAppMetric() ) );

    //  StarOne Services are now handled in the registry
}

void ScDLL::Exit()
{
    // called directly befor unloading the DLL
    // do whatever you want, Sxx-DLL is accessible

    // the SxxModule must be destroyed
    ScModuleDummy **ppShlPtr = (ScModuleDummy**) GetAppData(SHL_CALC);
    delete (*ppShlPtr);
    (*ppShlPtr) = NULL;

    //  auf keinen Fall ein neues ScModuleDummy anlegen, weil dessen vtable sonst
    //  in der DLL waere und das Loeschen im LibExit schiefgehen wuerde

    //  ScGlobal::Clear ist schon im Module-dtor
}

//------------------------------------------------------------------
//  Statusbar
//------------------------------------------------------------------

#define TEXT_WIDTH(s)   rStatusBar.GetTextWidth((s))

void ScDLL::FillStatusBar(StatusBar &rStatusBar)
{
    // Dokumentposition (Tabelle x / y)
    rStatusBar.InsertItem( SID_STATUS_DOCPOS,
                            TEXT_WIDTH( String().Fill( 10, 'X' ) ),
                            SIB_LEFT|SIB_AUTOSIZE );

    // Seitenvorlage
    rStatusBar.InsertItem( SID_STATUS_PAGESTYLE,
                            TEXT_WIDTH( String().Fill( 15, 'X' ) ),
                            SIB_LEFT|SIB_AUTOSIZE );

    // Ma"sstab
    rStatusBar.InsertItem(  SID_ATTR_ZOOM,
                            SvxZoomStatusBarControl::GetDefItemWidth(rStatusBar),
                            SIB_CENTER );

    // Einfuege-/Ueberschreibmodus
    rStatusBar.InsertItem( SID_ATTR_INSERT,
                            SvxInsertStatusBarControl::GetDefItemWidth(rStatusBar),
                            SIB_CENTER );

    // Selektionsmodus
    rStatusBar.InsertItem( SID_STATUS_SELMODE,
                            SvxSelectionModeControl::GetDefItemWidth(rStatusBar),
                            SIB_CENTER );

    // Dokument geaendert
    rStatusBar.InsertItem( SID_DOC_MODIFIED,
                            SvxModifyControl::GetDefItemWidth(rStatusBar));

    // Mail
    rStatusBar.InsertItem( SID_MAIL_NOTIFY,
                            TEXT_WIDTH( String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Mail")) ),
                            SIB_CENTER );

    // den aktuellen Kontext anzeigen Uhrzeit / FramePos / TabellenInfo / Errors
    rStatusBar.InsertItem( SID_ATTR_SIZE,
                            SvxPosSizeStatusBarControl::GetDefItemWidth(rStatusBar),
                            SIB_AUTOSIZE|SIB_LEFT|SIB_USERDRAW);
}

#undef TEXT_WIDTH


