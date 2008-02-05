/************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: scdll.cxx,v $
 *
 *  $Revision: 1.43 $
 *
 *  last change: $Author: ihi $ $Date: 2008-02-05 15:44:47 $
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
#include "precompiled_sc.hxx"



#include <svx/eeitem.hxx>


#ifndef _FM_FMOBJFAC_HXX
#include <svx/fmobjfac.hxx>
#endif
#ifndef _OBJFAC3D_HXX
#include <svx/objfac3d.hxx>
#endif
#ifndef _SVX_TBXCOLOR_HXX
#include <svx/tbxcolor.hxx>
#endif

#include <sot/clsids.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/app.hxx>
#include <avmedia/mediaplayer.hxx>
#include <avmedia/mediatoolbox.hxx>
#include <comphelper/types.hxx>

#ifndef _SVX_EXTRUSION_CONTROLS_HXX
#include <svx/extrusioncontrols.hxx>
#endif
#ifndef _SVX_FONTWORK_GALLERY_DIALOG_HXX
#include <svx/fontworkgallery.hxx>
#endif
#ifndef _SVX_TBXCUSTOMSHAPES_HXX
#include <svx/tbxcustomshapes.hxx>
#endif

#include <svtools/parhtml.hxx>
#include <sot/formats.hxx>
#define SOT_FORMATSTR_ID_STARCALC_30 SOT_FORMATSTR_ID_STARCALC

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
#include <svx/flditem.hxx>
#include <svx/modctrl.hxx>
#include <svx/pszctrl.hxx>
#include <svx/fntctl.hxx>
#include <svx/fntszctl.hxx>
#include <svx/grafctrl.hxx>
#include <svx/galbrws.hxx>
#include <svx/clipboardctl.hxx>
#include <svx/lboxctrl.hxx>
#ifndef _SVX_VERT_TEXT_TBXCTRL_HXX
#include <svx/verttexttbxctrl.hxx>
#endif
#include <svx/formatpaintbrushctrl.hxx>
#include "tbinsert.hxx"

#include <svx/xmlsecctrl.hxx>
// Child-Windows
#include "reffact.hxx"
#include "navipi.hxx"
#include "inputwin.hxx"
#include "spelldialog.hxx"
#include <svx/fontwork.hxx>
#include <svx/srchdlg.hxx>
#include <svx/hyprlink.hxx>
#include <svx/hyperdlg.hxx>
#include <svx/imapdlg.hxx>

#include "editutil.hxx"
#include <svx/svdfield.hxx>     //  SdrRegisterFieldClasses
#include <rtl/logfile.hxx>

#include "dwfunctr.hxx"
#include "acredlin.hxx"

//------------------------------------------------------------------

//  filter detection can't use ScFilterOptions (in sc-dll),
//  so access to wk3 flag must be implemented here again

class ScLibOptions : public utl::ConfigItem
{
    BOOL        bWK3Flag;

public:
                ScLibOptions();
    BOOL        GetWK3Flag() const          { return bWK3Flag; }
};

#define CFGPATH_LIBFILTER       "Office.Calc/Filter/Import/Lotus123"
#define ENTRYSTR_WK3            "WK3"

ScLibOptions::ScLibOptions() :
    ConfigItem( rtl::OUString::createFromAscii( CFGPATH_LIBFILTER ) ),
    bWK3Flag( FALSE )
{
    com::sun::star::uno::Sequence<rtl::OUString> aNames(1);
    aNames[0] = rtl::OUString::createFromAscii( ENTRYSTR_WK3 );
    com::sun::star::uno::Sequence<com::sun::star::uno::Any> aValues = GetProperties(aNames);
    if ( aValues.getLength() == 1 && aValues[0].hasValue() )
        bWK3Flag = comphelper::getBOOL( aValues[0] );
}


//------------------------------------------------------------------

ScResId::ScResId( USHORT nId ) :
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

//REMOVE        ScDocShell::RegisterFactory( SDT_SC_DOCFACTPRIO );

    ScDocShell::Factory().SetDocumentServiceName( rtl::OUString::createFromAscii( "com.sun.star.sheet.SpreadsheetDocument" ) );

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
//  SvxFontHeightToolBoxControl     ::RegisterControl(SID_ATTR_CHAR_FONTHEIGHT, pMod);
    SvxFontColorToolBoxControl      ::RegisterControl(SID_ATTR_CHAR_COLOR,      pMod);
    SvxColorToolBoxControl          ::RegisterControl(SID_BACKGROUND_COLOR,     pMod);
    SvxFrameToolBoxControl          ::RegisterControl(SID_ATTR_BORDER,          pMod);
    SvxFrameLineStyleToolBoxControl ::RegisterControl(SID_FRAME_LINESTYLE,      pMod);
    SvxFrameLineColorToolBoxControl ::RegisterControl(SID_FRAME_LINECOLOR,      pMod);
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

    // Svx-StatusBar-Controller
    SvxInsertStatusBarControl       ::RegisterControl(SID_ATTR_INSERT,      pMod);
    SvxSelectionModeControl         ::RegisterControl(SID_STATUS_SELMODE,   pMod);
    SvxZoomStatusBarControl         ::RegisterControl(SID_ATTR_ZOOM,        pMod);
    SvxModifyControl                ::RegisterControl(SID_DOC_MODIFIED,     pMod);
    XmlSecStatusBarControl          ::RegisterControl( SID_SIGNATURE,       pMod );

    SvxPosSizeStatusBarControl      ::RegisterControl(SID_ATTR_SIZE,        pMod);

    // Svx-Menue-Controller
    SvxFontMenuControl              ::RegisterControl(SID_ATTR_CHAR_FONT,       pMod);
    SvxFontSizeMenuControl          ::RegisterControl(SID_ATTR_CHAR_FONTHEIGHT, pMod);

    // CustomShape extrusion controller
    svx::ExtrusionDepthControl::RegisterControl( SID_EXTRUSION_DEPTH_FLOATER, pMod );
    svx::ExtrusionDirectionControl::RegisterControl( SID_EXTRUSION_DIRECTION_FLOATER, pMod );
    svx::ExtrusionLightingControl::RegisterControl( SID_EXTRUSION_LIGHTING_FLOATER, pMod );
    svx::ExtrusionSurfaceControl::RegisterControl( SID_EXTRUSION_SURFACE_FLOATER, pMod );
    svx::ExtrusionColorControl::RegisterControl( SID_EXTRUSION_3D_COLOR, pMod );

    svx::FontWorkShapeTypeControl::RegisterControl( SID_FONTWORK_SHAPE_TYPE, pMod );
    svx::FontWorkAlignmentControl::RegisterControl( SID_FONTWORK_ALIGNMENT_FLOATER, pMod );
    svx::FontWorkCharacterSpacingControl::RegisterControl( SID_FONTWORK_CHARACTER_SPACING_FLOATER, pMod );

    //  Child-Windows

    // Hack: Eingabezeile mit 42 registrieren, damit sie im PlugIn immer sichtbar ist
    ScInputWindowWrapper        ::RegisterChildWindow(42, pMod, SFX_CHILDWIN_TASK|SFX_CHILDWIN_FORCEDOCK);
    ScNavigatorDialogWrapper    ::RegisterChildWindowContext(
            sal::static_int_cast<sal_uInt16>(ScTabViewShell::GetInterfaceId()), pMod);
    ScSolverDlgWrapper          ::RegisterChildWindow(FALSE, pMod);
    ScOptSolverDlgWrapper       ::RegisterChildWindow(FALSE, pMod);
    ScNameDlgWrapper            ::RegisterChildWindow(FALSE, pMod);
    ScPivotLayoutWrapper        ::RegisterChildWindow(FALSE, pMod);
    ScTabOpDlgWrapper           ::RegisterChildWindow(FALSE, pMod);
    ScFilterDlgWrapper          ::RegisterChildWindow(FALSE, pMod);
    ScSpecialFilterDlgWrapper   ::RegisterChildWindow(FALSE, pMod);
    ScDbNameDlgWrapper          ::RegisterChildWindow(FALSE, pMod);
    ScConsolidateDlgWrapper     ::RegisterChildWindow(FALSE, pMod);
    ScPrintAreasDlgWrapper      ::RegisterChildWindow(FALSE, pMod);
    ScCondFormatDlgWrapper      ::RegisterChildWindow(FALSE, pMod);
    ScColRowNameRangesDlgWrapper::RegisterChildWindow(FALSE, pMod);
    ScFormulaDlgWrapper         ::RegisterChildWindow(FALSE, pMod);

    // First docking Window for Calc
    ScFunctionChildWindow       ::RegisterChildWindow(FALSE, pMod);

    // Redlining- Window
    ScAcceptChgDlgWrapper       ::RegisterChildWindow(FALSE, pMod);
    ScSimpleRefDlgWrapper       ::RegisterChildWindow(FALSE, pMod, SFX_CHILDWIN_ALWAYSAVAILABLE|SFX_CHILDWIN_NEVERHIDE );
    ScHighlightChgDlgWrapper    ::RegisterChildWindow(FALSE, pMod);

    SvxSearchDialogWrapper      ::RegisterChildWindow(FALSE, pMod);
    SvxHlinkDlgWrapper          ::RegisterChildWindow(FALSE, pMod);
    SvxFontWorkChildWindow      ::RegisterChildWindow(FALSE, pMod);
    SvxHyperlinkDlgWrapper      ::RegisterChildWindow(FALSE, pMod, SFX_CHILDWIN_FORCEDOCK);
    SvxIMapDlgChildWindow       ::RegisterChildWindow(FALSE, pMod);
    GalleryChildWindow          ::RegisterChildWindow(FALSE, pMod);
    ScSpellDialogChildWindow    ::RegisterChildWindow(FALSE, pMod);
    ::avmedia::MediaPlayer      ::RegisterChildWindow(FALSE, pMod);

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

    // 3D-Objekt-Factory eintragen
    E3dObjFactory();

    // ::com::sun::star::form::component::Form-Objekt-Factory eintragen
    FmFormObjFactory();

    pMod->PutItem( SfxUInt16Item( SID_ATTR_METRIC, sal::static_int_cast<UINT16>(pMod->GetAppOptions().GetAppMetric()) ) );

    //  StarOne Services are now handled in the registry
}

void ScDLL::Exit()
{
    // the SxxModule must be destroyed
    ScModule **ppShlPtr = (ScModule**) GetAppData(SHL_CALC);
    delete (*ppShlPtr);
    (*ppShlPtr) = NULL;

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

    // signatures
    rStatusBar.InsertItem( SID_SIGNATURE, XmlSecStatusBarControl::GetDefItemWidth( rStatusBar ), SIB_USERDRAW );
    rStatusBar.SetHelpId(SID_SIGNATURE, SID_SIGNATURE);

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

// DetectFilter functionality has moved - please update your bookmarks
// see sc/source/ui/unoobj/scdetect.cxx, have a nice day.
