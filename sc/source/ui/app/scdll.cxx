/*************************************************************************
 *
 *  $RCSfile: scdll.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:53 $
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
#ifndef _BTBXCTL_HXX //autogen
#include <basctl/btbxctl.hxx>
#endif
#ifndef _TBXCTL_HXX //autogen
#include <svx/tbxctl.hxx>
#endif

#include <segmentc.hxx>
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

#include "tbinsert.hxx"
/*
#include <basicsh.hxx>      // TbxControls
#include <tbxctrls.hxx>
#include <tbxctl.hxx>
#include <stbctrls.hxx>
#include <mnuctrls.hxx>
*/
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


SEG_EOFGLOBALS()

//------------------------------------------------------------------

#pragma SEG_FUNCDEF(scdll_01)

ScResId::ScResId( USHORT nId ) :
    ResId( nId, SC_MOD()->GetResMgr() )
{
}

//------------------------------------------------------------------

#pragma SEG_FUNCDEF(scdll_02)

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
    TbxControls                     ::RegisterControl(SID_CHOOSE_CONTROLS,      pMod);

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

#pragma SEG_FUNCDEF(scdll_03)

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

#pragma SEG_FUNCDEF(scdll_04)

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

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.47  2000/09/17 14:08:54  willem.vandorp
    OpenOffice header added.

    Revision 1.46  2000/09/04 13:50:01  tbe
    basicide, isetbrw, si, vcdlged from svx to basctl

    Revision 1.45  2000/08/31 16:38:18  willem.vandorp
    Header and footer replaced

    Revision 1.44  2000/08/03 19:02:30  nn
    pluginmenu

    Revision 1.43  2000/04/17 13:26:28  nn
    unicode changes

    Revision 1.42  1999/06/28 13:30:46  NN
    #67287# StarOne services no longer registered here


      Rev 1.41   28 Jun 1999 15:30:46   NN
   #67287# StarOne services no longer registered here

      Rev 1.40   02 Jun 1999 21:38:20   ANK
   #66547# SubShells

      Rev 1.39   16 Mar 1999 14:17:08   NN
   #62845# GlobalSettings unter richtigem Service-Namen registrieren

      Rev 1.38   04 Mar 1999 21:14:50   NN
   #62191# kein unsigned mehr

      Rev 1.37   22 Feb 1999 20:52:34   ANK
   #47158# Erweiterungen fuer neue DrawForm-Shell

      Rev 1.36   02 Feb 1999 20:23:22   NN
   #53308# TF_ONE51: AutoFormate wiederbelebt

      Rev 1.35   26 Jan 1999 14:42:32   NN
   #53308# TF_ONE51-includes

      Rev 1.34   18 Dec 1998 16:11:54   NN
   #53308# TF_ONE51 Umstellung

      Rev 1.33   15 Dec 1998 15:57:42   ANK
   #60209# Anwenden der neuen statische Methode GetDefItemWidth bei StatusbarControls

      Rev 1.32   03 Dec 1998 20:02:28   ANK
   #58683# Groesse hat wieder sib_autosize

      Rev 1.31   02 Nov 1998 16:53:10   ANK
   #58683# Statusbar-Eintrag 'Groesse' jetzt veraenderbar in der Breite

      Rev 1.30   23 Oct 1998 12:31:58   NN
   #58244# Registrierung von ein paar EditEngine-Feldern ist schon in der Ofa

      Rev 1.29   02 Oct 1998 14:10:22   NN
   #53308# Services fuer StarOne registrieren

      Rev 1.28   11 Sep 1998 14:23:02   ER
   #42680# Feldbefehle erweitert fuer Titel/Pfad/Dateiname

      Rev 1.27   14 Apr 1998 17:32:44   MBA
   Eingabezeile innerhalb der Task

      Rev 1.26   09 Apr 1998 22:12:52   ANK
   Redlining Erweiterungen

      Rev 1.25   13 Mar 1998 13:51:58   ANK
   Erweiterungen

      Rev 1.24   08 Mar 1998 20:37:22   ANK
   Redlining Dialog

      Rev 1.23   10 Feb 1998 15:24:44   TJ
   inlude wg. internal compiler error

      Rev 1.22   06 Feb 1998 14:15:52   HJS
   includes

      Rev 1.21   23 Jan 1998 19:02:34   NN
   include

      Rev 1.20   05 Dec 1997 20:06:30   ANK
   Includes geaendert

      Rev 1.19   25 Nov 1997 11:32:26   TJ
   includes

      Rev 1.18   14 Oct 1997 19:53:52   ANK
   Neue Funktionsbox

      Rev 1.17   06 Aug 1997 15:41:58   TRI
   VCL: Anpassungen

      Rev 1.16   30 Jun 1997 12:28:14   NN
   Feldbefehle nach Svx verschoben

      Rev 1.15   04 Jun 1997 12:53:04   ER
   363 MUSS: SIDs

      Rev 1.14   23 May 1997 20:26:38   NN
   RegisterChildWindow fuer ScFormulaDlgWrapper

      Rev 1.13   21 Apr 1997 18:28:50   NN
   im ScDLL::Exit kein ScGlobal::Clear

      Rev 1.12   18 Apr 1997 13:09:28   NN
   im Exit kein ScModuleDummy anlegen

      Rev 1.11   25 Mar 1997 10:43:04   NN
   #37851# SvxURLField schon in der OffApp registriert

      Rev 1.10   08 Feb 1997 19:07:48   NN
   Zeichenfunktionen und Ausrichtung sind jetzt Svx-Controller

      Rev 1.9   28 Jan 1997 10:00:06   NN
   PutItem(SID_ATTR_METRIC) am Module

      Rev 1.8   20 Dec 1996 15:54:32   NN
   #34679# Menue und Accelerator mit Config-IDs

      Rev 1.7   13 Dec 1996 18:59:50   NN
   SvxSearchDialogWrapper nicht mehr registrieren

      Rev 1.6   24 Nov 1996 15:23:48   NN
   ScGlobal::Clear rufen

      Rev 1.5   22 Nov 1996 11:55:02   ER
   #33275# signal() Wrapper an der App wg. BLC-Dummbatz

      Rev 1.4   18 Nov 1996 18:59:00   ER
   new: ScColRowNameRangesDlg

      Rev 1.3   14 Nov 1996 22:01:10   NN
   RegisterChildWindowContext fuer Navigator

      Rev 1.2   14 Nov 1996 14:18:52   NN
   vor dem Anlegen des ScModule nur die VersionMaps initialisieren

      Rev 1.1   06 Nov 1996 14:18:34   NN
   ScModule mit ObjectFactory vom ModuleDummy konstruieren

      Rev 1.0   05 Nov 1996 14:33:12   NN
   Initial revision.

------------------------------------------------------------------------*/

#pragma SEG_EOFMODULE


