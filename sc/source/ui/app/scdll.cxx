/*
 *  $RCSfile: scdll.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 10:12:57 $
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

#ifndef _FM_FMOBJFAC_HXX
#include <svx/fmobjfac.hxx>
#endif
#ifndef _SVX_SIIMPORT_HXX
#include <svx/siimport.hxx>
#endif
#ifndef _OBJFAC3D_HXX
#include <svx/objfac3d.hxx>
#endif

#include <sot/clsids.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/app.hxx>
#include <comphelper/types.hxx>

#ifndef _SVX_EXTRUSION_CONTROLS_HXX
#include <svx/extrusioncontrols.hxx>
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

// Child-Windows
#include "reffact.hxx"
#include "navipi.hxx"
#include "inputwin.hxx"
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

//  Filter-Namen (wie in docsh.cxx)

static const sal_Char __FAR_DATA pFilterSc50[]      = "StarCalc 5.0";
static const sal_Char __FAR_DATA pFilterSc50Temp[]  = "StarCalc 5.0 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterSc40[]      = "StarCalc 4.0";
static const sal_Char __FAR_DATA pFilterSc40Temp[]  = "StarCalc 4.0 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterSc30[]      = "StarCalc 3.0";
static const sal_Char __FAR_DATA pFilterSc30Temp[]  = "StarCalc 3.0 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterSc10[]      = "StarCalc 1.0";
static const sal_Char __FAR_DATA pFilterXML[]       = "StarOffice XML (Calc)";
static const sal_Char __FAR_DATA pFilterAscii[]     = "Text - txt - csv (StarCalc)";
static const sal_Char __FAR_DATA pFilterLotus[]     = "Lotus";
static const sal_Char __FAR_DATA pFilterExcel4[]    = "MS Excel 4.0";
static const sal_Char __FAR_DATA pFilterEx4Temp[]   = "MS Excel 4.0 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterExcel5[]    = "MS Excel 5.0/95";
static const sal_Char __FAR_DATA pFilterEx5Temp[]   = "MS Excel 5.0/95 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterExcel95[]   = "MS Excel 95";
static const sal_Char __FAR_DATA pFilterEx95Temp[]  = "MS Excel 95 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterExcel97[]   = "MS Excel 97";
static const sal_Char __FAR_DATA pFilterEx97Temp[]  = "MS Excel 97 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterDBase[]     = "dBase";
static const sal_Char __FAR_DATA pFilterDif[]       = "DIF";
static const sal_Char __FAR_DATA pFilterSylk[]      = "SYLK";
static const sal_Char __FAR_DATA pFilterHtml[]      = "HTML (StarCalc)";
static const sal_Char __FAR_DATA pFilterHtmlWeb[]   = "calc_HTML_WebQuery";
static const sal_Char __FAR_DATA pFilterRtf[]       = "Rich Text Format (StarCalc)";

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
    ResId( nId, SC_MOD()->GetResMgr() )
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

    ScDocShell::RegisterFactory( SDT_SC_DOCFACTPRIO );

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

    // Svx-StatusBar-Controller
    SvxInsertStatusBarControl       ::RegisterControl(SID_ATTR_INSERT,      pMod);
    SvxSelectionModeControl         ::RegisterControl(SID_STATUS_SELMODE,   pMod);
    SvxZoomStatusBarControl         ::RegisterControl(SID_ATTR_ZOOM,        pMod);
    SvxModifyControl                ::RegisterControl(SID_DOC_MODIFIED,     pMod);
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

    SvxSearchDialogWrapper::RegisterChildWindow(FALSE, pMod);
    SvxHlinkDlgWrapper::RegisterChildWindow(FALSE, pMod);
    SvxFontWorkChildWindow      ::RegisterChildWindow(FALSE, pMod);
    SvxHyperlinkDlgWrapper      ::RegisterChildWindow(FALSE, pMod);
    SvxIMapDlgChildWindow       ::RegisterChildWindow(FALSE, pMod);
    GalleryChildWindow          ::RegisterChildWindow(FALSE, pMod);

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

    // factory for dummy import of old si-controls in 3.1 documents
    SiImportFactory();


    pMod->PutItem( SfxUInt16Item( SID_ATTR_METRIC, pMod->GetAppOptions().GetAppMetric() ) );

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

BOOL lcl_MayBeAscii( SvStream& rStream )
{
    //  ASCII is considered possible if there are no null bytes

    rStream.Seek(STREAM_SEEK_TO_BEGIN);

    BOOL bNullFound = FALSE;
    BYTE aBuffer[ 4097 ];
    const BYTE* p = aBuffer;
    ULONG nBytesRead = rStream.Read( aBuffer, 4096 );

    if ( nBytesRead >= 2 &&
            ( ( aBuffer[0] == 0xff && aBuffer[1] == 0xfe ) ||
              ( aBuffer[0] == 0xfe && aBuffer[1] == 0xff ) ) )
    {
        //  unicode file may contain null bytes
        return TRUE;
    }

    while( nBytesRead-- )
        if( !*p++ )
        {
            bNullFound = TRUE;
            break;
        }

    return !bNullFound;
}

BOOL lcl_MayBeDBase( SvStream& rStream )
{
    //  for dBase, look for the 0d character at the end of the header

    rStream.Seek(STREAM_SEEK_TO_END);
    ULONG nSize = rStream.Tell();

    // length of header starts at 8

    if ( nSize < 10 )
        return FALSE;
    rStream.Seek(8);
    USHORT nHeaderLen;
    rStream >> nHeaderLen;

    if ( nHeaderLen < 32 || nSize < nHeaderLen )
        return FALSE;

    // last byte of header must be 0d

    rStream.Seek( nHeaderLen - 1 );
    BYTE nEndFlag;
    rStream >> nEndFlag;

    return ( nEndFlag == 0x0d );
}

BOOL lcl_IsAnyXMLFilter( const SfxFilter* pFilter )
{
    if ( !pFilter )
        return FALSE;

    //  TRUE for XML file or template
    //  (template filter has no internal name -> allow configuration key names)

    String aName = pFilter->GetFilterName();
    return aName.EqualsAscii(pFilterXML) ||
           aName.EqualsAscii("calc_StarOffice_XML_Calc") ||
           aName.EqualsAscii("calc_StarOffice_XML_Calc_Template");
}
#if 0
ULONG __EXPORT ScDLL::DetectFilter( SfxMedium& rMedium, const SfxFilter** ppFilter,
                                    SfxFilterFlags nMust, SfxFilterFlags nDont )
{
    //  #59915# laut MBA darf hier nur ERRCODE_NONE, ERRCODE_ABORT und ERRCODE_FORCEQUIET
    //  zurueckgegeben werden...

    if ( SVSTREAM_OK != rMedium.GetError() )
        return ERRCODE_ABORT;   // ERRCODE_IO_GENERAL

    //  Formate, die sicher erkannt werden:

    SvStorage* pStorage = rMedium.GetStorage();
    if ( pStorage )
    {
        String      aStreamName;

        // Erkennung ueber contained streams
        // Excel-5 / StarCalc 3.0

        aStreamName = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Workbook"));
        BOOL bExcel97Stream = ( pStorage->IsContained( aStreamName ) && pStorage->IsStream( aStreamName ) );

        aStreamName = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Book"));
        BOOL bExcel5Stream = ( pStorage->IsContained( aStreamName ) && pStorage->IsStream( aStreamName ) );

        if ( bExcel97Stream )
        {
            String aOldName;
            if ( *ppFilter ) aOldName = (*ppFilter)->GetFilterName();
            if ( aOldName.EqualsAscii(pFilterEx97Temp) )
            {
                //  Excel 97 template selected -> keep selection
            }
            else if ( bExcel5Stream &&
                        ( aOldName.EqualsAscii(pFilterExcel5) || aOldName.EqualsAscii(pFilterEx5Temp) ||
                          aOldName.EqualsAscii(pFilterExcel95) || aOldName.EqualsAscii(pFilterEx95Temp) ) )
            {
                //  dual format file and Excel 5 selected -> keep selection
            }
            else
            {
                //  else use Excel 97 filter
                *ppFilter = SFX_APP()->GetFilter( ScDocShell::Factory(),
                                      String::CreateFromAscii(pFilterExcel97) );
            }
            return ERRCODE_NONE;
        }
        if ( bExcel5Stream )
        {
            String aOldName;
            if ( *ppFilter ) aOldName = (*ppFilter)->GetFilterName();
            if ( aOldName.EqualsAscii(pFilterExcel95) || aOldName.EqualsAscii(pFilterEx95Temp) ||
                    aOldName.EqualsAscii(pFilterEx5Temp) )
            {
                //  Excel 95 oder Vorlage (5 oder 95) eingestellt -> auch gut
            }
            else if ( aOldName.EqualsAscii(pFilterEx97Temp) )
            {
                // #101923# auto detection has found template -> return Excel5 template
                *ppFilter = SFX_APP()->GetFilter( ScDocShell::Factory(),
                                        String::CreateFromAscii(pFilterEx5Temp) );
            }
            else
            {
                //  sonst wird als Excel 5-Datei erkannt
                *ppFilter = SFX_APP()->GetFilter( ScDocShell::Factory(),
                                        String::CreateFromAscii(pFilterExcel5) );
            }
            return ERRCODE_NONE;
        }

        aStreamName = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(STRING_SCSTREAM));
        if ( pStorage->IsContained( aStreamName ) && pStorage->IsStream( aStreamName ) )
        {
            //  Unterscheidung 3.0 / 4.0 / 5.0 ueber Clipboard-Id
            ULONG nStorFmt = pStorage->GetFormat();
            if ( nStorFmt == SOT_FORMATSTR_ID_STARCALC_30 )
                *ppFilter = SFX_APP()->GetFilter( ScDocShell::Factory(),
                                            String::CreateFromAscii(pFilterSc30) );
            else if ( nStorFmt == SOT_FORMATSTR_ID_STARCALC_40 )
                *ppFilter = SFX_APP()->GetFilter( ScDocShell::Factory(),
                                            String::CreateFromAscii(pFilterSc40) );
            else
                *ppFilter = SFX_APP()->GetFilter( ScDocShell::Factory(),
                                            String::CreateFromAscii(pFilterSc50) );
            return ERRCODE_NONE;
        }

        //  XML package file: Stream "Content.xml" or "content.xml"
        aStreamName = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("content.xml"));
        String aOldXML = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Content.xml"));
        if ( ( pStorage->IsContained( aStreamName ) && pStorage->IsStream( aStreamName ) ) ||
             ( pStorage->IsContained( aOldXML ) && pStorage->IsStream( aOldXML ) ) )
        {
            //  #85794# don't accept other applications' xml formats,
            //  recognized by clipboard id
            ULONG nStorageFormat = pStorage->GetFormat();
            if ( nStorageFormat == 0 || nStorageFormat == SOT_FORMATSTR_ID_STARCALC_60 )
            {
                //  if XML template is set, don't modify
                if (!lcl_IsAnyXMLFilter(*ppFilter))
                    *ppFilter = SFX_APP()->GetFilter( ScDocShell::Factory(),
                                                      String::CreateFromAscii(pFilterXML) );
                return ERRCODE_NONE;
            }
        }
    }
    else    // no storage
    {
        SvStream &rStr = *rMedium.GetInStream();
        if ( &rStr == NULL )
            return ERRCODE_ABORT;   // ERRCODE_IO_GENERAL

        // Tabelle mit Suchmustern
        // Bedeutung der Sequenzen
        // 0x00??: genau Byte 0x?? muss an dieser Stelle stehen
        // 0x0100: ein Byte ueberlesen (don't care)
        // 0x02nn: ein Byte aus 0xnn Alternativen folgt
        // 0x8000: Erkennung abgeschlossen
        //

#define M_DC        0x0100
#define M_ALT(ANZ)  0x0200+ANZ
#define M_ENDE      0x8000

        const UINT16 pLotus[] =         // Lotus 1/1A/2
            { 0x0000, 0x0000, 0x0002, 0x0000,
              M_ALT(2), 0x0004, 0x0006,
              0x0004, M_ENDE };

        const UINT16 pExcel1[] =        // Excel Biff/3/4 Tabellen
            { 0x0009,
              M_ALT(2), 0x0002, 0x0004,
              0x0006, 0x0000, M_DC, M_DC, 0x0010, 0x0000,
              M_DC, M_DC, M_ENDE };

        const UINT16 pExcel2[] =        // Excel Biff3/4 Workbooks
            { 0x0009,
              M_ALT(2), 0x0002, 0x0004,
              0x0006, 0x0000, M_DC, M_DC, 0x0000, 0x0001,
              M_DC, M_DC, M_ENDE };

        const UINT16 pExcel3[] =        // Excel Biff2 Tabellen
            { 0x0009, 0x0000, 0x0004, 0x0000,
              M_DC, M_DC, 0x0010, 0x0000, M_ENDE };

        const UINT16 pSc10[] =          // StarCalc 1.0 Dokumente
            { 'B', 'l', 'a', 'i', 's', 'e', '-', 'T', 'a', 'b', 'e', 'l', 'l',
              'e', 0x000A, 0x000D, 0x0000,    // Sc10CopyRight[16]
              M_DC, M_DC, M_DC, M_DC, M_DC, M_DC, M_DC, M_DC, M_DC, M_DC, M_DC,
              M_DC, M_DC,                   // Sc10CopyRight[29]
              M_ALT(2), 0x0065, 0x0066,     // Versionsnummer 101 oder 102
              0x0000,
              M_ENDE };

        const UINT16 pLotus2[] =        // Lotus >3
            { 0x0000, 0x0000, 0x001A, 0x0000,   // Rec# + Len (26)
              M_ALT(2), 0x0000, 0x0002,         // File Revision Code
              0x0010,
              0x0004, 0x0000,                   // File Revision Subcode
              M_ENDE };

        const UINT16 pDIF1[] =          // DIF mit CR-LF
            {
            'T', 'A', 'B', 'L', 'E',
            M_DC, M_DC,
            '0', ',', '1',
            M_DC, M_DC,
            '\"',
            M_ENDE };

        const UINT16 pDIF2[] =          // DIF mit CR oder LF
            {
            'T', 'A', 'B', 'L', 'E',
            M_DC,
            '0', ',', '1',
            M_DC,
            '\"',
            M_ENDE };

        const UINT16 pSylk[] =          // Sylk
            {
            'I', 'D', ';', 'P',
            M_ENDE };

#ifdef SINIX
        const UINT16 nAnzMuster = 9;    // sollte fuer indiz. Zugriff stimmen...
        UINT16 *ppMuster[ nAnzMuster ];         // Arrays mit Suchmustern
        ppMuster[ 0 ] = pLotus;
        ppMuster[ 1 ] = pExcel1;
        ppMuster[ 2 ] = pExcel2;
        ppMuster[ 3 ] = pExcel3;
        ppMuster[ 4 ] = pSc10;
        ppMuster[ 5 ] = pDIF1;
        ppMuster[ 6 ] = pDIF2;
        ppMuster[ 7 ] = pSylk;
        ppMuster[ 8 ] = pLotus2;                // Lotus immer ganz hinten wegen Ini-Eintrag
#else
        const UINT16 *ppMuster[] =      // Arrays mit Suchmustern
            {
            pLotus,
            pExcel1,
            pExcel2,
            pExcel3,
            pSc10,
            pDIF1,
            pDIF2,
            pSylk,
            pLotus2
            };
        const UINT16 nAnzMuster = sizeof(ppMuster) / sizeof(ppMuster[0]);
#endif

        const sal_Char* pFilterName[ nAnzMuster ] =     // zugehoerige Filter
            {
            pFilterLotus,
            pFilterExcel4,
            pFilterExcel4,
            pFilterExcel4,
            pFilterSc10,
            pFilterDif,
            pFilterDif,
            pFilterSylk,
            pFilterLotus
            };

        const UINT16 nByteMask = 0xFF;

        // suchen Sie jetzt!
        // ... realisiert ueber 'Mustererkennung'

        BYTE            nAkt;
        BOOL            bSync;          // Datei und Muster stimmen ueberein
        USHORT          nFilter;        // Zaehler ueber alle Filter
        const UINT16    *pSearch;       // aktuelles Musterwort
        UINT16          nFilterLimit = nAnzMuster;

        // nur solange, bis es etwas Globales gibt
        // funzt nur, solange Eintraege fuer WK3 letzte Muster-Tabelle ist!
        ScLibOptions aLibOpt;
        if( !aLibOpt.GetWK3Flag() )
            nFilterLimit--;

        for ( nFilter = 0 ; nFilter < nFilterLimit ; nFilter++ )
        {
            rStr.Seek( 0 ); // am Anfang war alles Uebel...
            rStr >> nAkt;
            pSearch = ppMuster[ nFilter ];
            bSync = TRUE;
            while( !rStr.IsEof() && bSync )
            {
                register UINT16 nMuster = *pSearch;

                if( nMuster < 0x0100 )
                { //                                direkter Byte-Vergleich
                    if( ( BYTE ) nMuster != nAkt )
                        bSync = FALSE;
                }
                else if( nMuster & M_DC )
                { //                                             don't care
                }
                else if( nMuster & M_ALT(0) )
                { //                                      alternative Bytes
                    BYTE nAnzAlt = ( BYTE ) nMuster;
                    bSync = FALSE;          // zunaechst unsynchron
                    while( nAnzAlt > 0 )
                    {
                        pSearch++;
                        if( ( BYTE ) *pSearch == nAkt )
                            bSync = TRUE;   // jetzt erst Synchronisierung
                        nAnzAlt--;
                    }
                }
                else if( nMuster & M_ENDE )
                { //                                        Format detected
                    if ( pFilterName[nFilter] == pFilterExcel4 && *ppFilter &&
                            (*ppFilter)->GetFilterName().EqualsAscii(pFilterEx4Temp) )
                    {
                        //  Excel 4 erkannt, Excel 4 Vorlage eingestellt -> auch gut
                    }
                    else
                    {   // gefundenen Filter einstellen
                        *ppFilter = SFX_APP()->GetFilter(
                            ScDocShell::Factory(),
                            String::CreateFromAscii(pFilterName[ nFilter ]) );
                    }

                    return ERRCODE_NONE;
                }
                else
                { //                                         Tabellenfehler
                    DBG_ERROR( "-ScApplication::DetectFilter(): Fehler in Mustertabelle");
                }

                pSearch++;
                rStr >> nAkt;
            }
        }

        String aPresetFilterName;
        if ( *ppFilter )
            aPresetFilterName = (*ppFilter)->GetFilterName();

        // ASCII cannot be recognized.
        // #i3341# But if the Text/CSV filter was set (either by the user or
        // file extension) it takes precedence over HTML and RTF and dBase
        // detection. Otherwise something like, for example, "lala <SUP> gugu"
        // would trigger HTML to be recognized.

        if ( aPresetFilterName.EqualsAscii(pFilterAscii) && lcl_MayBeAscii( rStr ) )
            return ERRCODE_NONE;

        // get file header

        rStr.Seek( 0 );
        const int nTrySize = 80;
        ByteString aHeader;
        for ( int j = 0; j < nTrySize && !rStr.IsEof(); j++ )
        {
            sal_Char c;
            rStr >> c;
            aHeader += c;
        }
        aHeader += '\0';

        // test for HTML

        if ( HTMLParser::IsHTMLFormat( aHeader.GetBuffer() ) )
        {
            if ( aPresetFilterName.EqualsAscii(pFilterHtml) )
            {
                // old HTML filter is allowed, default is WebQuery filter
            }
            else
                *ppFilter = SFX_APP()->GetFilter( ScDocShell::Factory(),
                                                  String::CreateFromAscii(pFilterHtmlWeb) );
            return ERRCODE_NONE;
        }

        // test for RTF

        if ( aHeader.CompareTo( "{\\rtf", 5 ) == COMPARE_EQUAL )
        {
            *ppFilter = SFX_APP()->GetFilter( ScDocShell::Factory(),
                                              String::CreateFromAscii(pFilterRtf) );
            return ERRCODE_NONE;
        }

        // #97832#; we don't have a flat xml filter
/*      if ( aHeader.CompareTo( "<?xml", 5 ) == COMPARE_EQUAL )
        {
            //  if XML template is set, don't modify
            if (!lcl_IsAnyXMLFilter(*ppFilter))
                *ppFilter = SFX_APP()->GetFilter( ScDocShell::Factory(),
                                                  String::CreateFromAscii(pFilterXML) );
            return ERRCODE_NONE;
        }*/

        // dBase cannot safely be recognized - only test if the filter was set
        if ( aPresetFilterName.EqualsAscii(pFilterDBase) && lcl_MayBeDBase( rStr ) )
            return ERRCODE_NONE;
    }

    return ERRCODE_ABORT;       // war nix
}
#endif
