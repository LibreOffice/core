/*************************************************************************
 *
 *  $RCSfile: docsh.cxx,v $
 *
 *  $Revision: 1.71 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-23 10:52:08 $
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

// System - Includes -----------------------------------------------------

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include "scitems.hxx"
#include <svx/eeitem.hxx>
#ifndef _SVX_SVXENUM_HXX
#include <svx/svxenum.hxx>
#endif
#ifndef _SVX_ALGITEM_HXX
#include <svx/algitem.hxx>
#endif

#define ITEMID_FIELD EE_FEATURE_FIELD

#include <svtools/securityoptions.hxx>
#include <tools/stream.hxx>
#include <tools/string.hxx>
#include <tools/urlobj.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/virdev.hxx>
#include <vcl/waitobj.hxx>
#include <svtools/ctrltool.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/zforlist.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dinfdlg.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/evntconf.hxx>
#include <sfx2/sfx.hrc>
#include <sfx2/topfrm.hxx>
#include <svx/srchitem.hxx>
#include <svx/svxmsbas.hxx>
#include <svtools/fltrcfg.hxx>
#include <so3/clsids.hxx>
#include <unotools/charclass.hxx>
#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif

#ifndef _SFXREQUEST_HXX
#include <sfx2/request.hxx>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_UPDATEDOCMODE_HPP_
#include <com/sun/star/document/UpdateDocMode.hpp>
#endif


#include "scabstdlg.hxx" //CHINA001
#include <sot/formats.hxx>
#define SOT_FORMATSTR_ID_STARCALC_30 SOT_FORMATSTR_ID_STARCALC

#ifndef SO2_DECL_SVSTORAGESTREAM_DEFINED
#define SO2_DECL_SVSTORAGESTREAM_DEFINED
SO2_DECL_REF(SvStorageStream)
#endif

// INCLUDE ---------------------------------------------------------------

#include "cell.hxx"
#include "global.hxx"
#include "filter.hxx"
#include "scmod.hxx"
#include "tabvwsh.hxx"
#include "docfunc.hxx"
#include "imoptdlg.hxx"
#include "impex.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "globstr.hrc"
//CHINA001 #include "tpstat.hxx"
#include "scerrors.hxx"
#include "brdcst.hxx"
#include "stlpool.hxx"
#include "autostyl.hxx"
#include "attrib.hxx"
#include "asciiopt.hxx"
#include "waitoff.hxx"
#include "docpool.hxx"      // LoadCompleted
#include "progress.hxx"
#include "pntlock.hxx"
#include "collect.hxx"
#include "docuno.hxx"
#include "appoptio.hxx"
#include "detdata.hxx"
#include "printfun.hxx"
#include "dociter.hxx"
#include "cellform.hxx"
#include "chartlis.hxx"
#include "hints.hxx"
#include "xmlwrap.hxx"
#include "drwlayer.hxx"
#include "refreshtimer.hxx"
#include "dbcolect.hxx"
#include "scextopt.hxx"
#include "compiler.hxx"
#include "cfgids.hxx"

#include "docsh.hxx"

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

// STATIC DATA -----------------------------------------------------------

//  Stream-Namen im Storage

const sal_Char __FAR_DATA ScDocShell::pStarCalcDoc[] = STRING_SCSTREAM;     // "StarCalcDocument"
const sal_Char __FAR_DATA ScDocShell::pStyleName[] = "SfxStyleSheets";

//  Filter-Namen (wie in sclib.cxx)

static const sal_Char __FAR_DATA pFilterSc50[]      = "StarCalc 5.0";
//static const sal_Char __FAR_DATA pFilterSc50Temp[]    = "StarCalc 5.0 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterSc40[]      = "StarCalc 4.0";
//static const sal_Char __FAR_DATA pFilterSc40Temp[]    = "StarCalc 4.0 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterSc30[]      = "StarCalc 3.0";
//static const sal_Char __FAR_DATA pFilterSc30Temp[]    = "StarCalc 3.0 Vorlage/Template";
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
static const sal_Char __FAR_DATA pFilterHtmlWebQ[]  = "calc_HTML_WebQuery";
static const sal_Char __FAR_DATA pFilterRtf[]       = "Rich Text Format (StarCalc)";

//----------------------------------------------------------------------

#define ScDocShell
#include "scslots.hxx"


SFX_IMPL_INTERFACE(ScDocShell,SfxObjectShell, ScResId(SCSTR_DOCSHELL))
{
    SFX_CHILDWINDOW_REGISTRATION( SID_HYPERLINK_INSERT );
}

//  GlobalName der aktuellen Version:
SFX_IMPL_OBJECTFACTORY( ScDocShell, SFXOBJECTSHELL_STD_NORMAL, scalc, SvGlobalName(SO3_SC_CLASSID) )
{
    Factory().SetDocumentServiceName( rtl::OUString::createFromAscii( "com.sun.star.sheet.SpreadsheetDocument" ) );
    //Factory().GetFilterContainer()->SetDetectFilter( ScDLL::DetectFilter );
    Factory().RegisterMenuBar( ScResId(SCCFG_MENUBAR) );
    Factory().RegisterPluginMenuBar( ScResId(SCCFG_PLUGINMENU) );
    Factory().RegisterAccel( ScResId(SCCFG_ACCELERATOR) );
}

TYPEINIT1( ScDocShell, SfxObjectShell );        // SfxInPlaceObject: kein Type-Info ?

//------------------------------------------------------------------

void __EXPORT ScDocShell::FillClass( SvGlobalName* pClassName,
                                        ULONG* pFormat,
                                        String* pAppName,
                                        String* pFullTypeName,
                                        String* pShortTypeName,
                                        long nFileFormat ) const
{
    SfxInPlaceObject::FillClass( pClassName, pFormat, pAppName,
                                pFullTypeName, pShortTypeName, nFileFormat );

    if ( nFileFormat == SOFFICE_FILEFORMAT_31 )
    {
        *pClassName     = SvGlobalName( SO3_SC_CLASSID_30 );
        *pFormat        = SOT_FORMATSTR_ID_STARCALC_30;
        *pAppName       = String( ScResId( SCSTR_30_APPLICATION ) );
        *pFullTypeName  = String( ScResId( SCSTR_30_LONG_DOCNAME ) );
        *pShortTypeName = String( ScResId( SCSTR_SHORT_SCDOC_NAME ) );
    }
    else if ( nFileFormat == SOFFICE_FILEFORMAT_40 )
    {
        *pClassName     = SvGlobalName( SO3_SC_CLASSID_40 );
        *pFormat        = SOT_FORMATSTR_ID_STARCALC_40;
        *pAppName       = String( ScResId( SCSTR_40_APPLICATION ) );
        *pFullTypeName  = String( ScResId( SCSTR_40_LONG_DOCNAME ) );
        *pShortTypeName = String( ScResId( SCSTR_SHORT_SCDOC_NAME ) );
    }
    else if ( nFileFormat == SOFFICE_FILEFORMAT_50 )
    {
        *pClassName     = SvGlobalName( SO3_SC_CLASSID_50 );
        *pFormat        = SOT_FORMATSTR_ID_STARCALC_50;
        *pAppName       = String( ScResId( SCSTR_50_APPLICATION ) );
        *pFullTypeName  = String( ScResId( SCSTR_50_LONG_DOCNAME ) );
        *pShortTypeName = String( ScResId( SCSTR_SHORT_SCDOC_NAME ) );
    }
    else if ( nFileFormat == SOFFICE_FILEFORMAT_60 )
    {
        *pClassName     = SvGlobalName( SO3_SC_CLASSID_60 );
        *pFormat        = SOT_FORMATSTR_ID_STARCALC_60;
        *pFullTypeName  = String( ScResId( SCSTR_LONG_SCDOC_NAME ) );
        *pShortTypeName = String( ScResId( SCSTR_SHORT_SCDOC_NAME ) );
    }
    else if ( nFileFormat == SOFFICE_FILEFORMAT_8 )
    {
        *pClassName     = SvGlobalName( SO3_SC_CLASSID_60 );
        *pFormat        = SOT_FORMATSTR_ID_STARCALC_8;
        *pFullTypeName  = String( RTL_CONSTASCII_USTRINGPARAM("calc8") );
        *pShortTypeName = String( ScResId( SCSTR_SHORT_SCDOC_NAME ) );
    }
    else
        DBG_ERROR("wat fuer ne Version?");
}

//------------------------------------------------------------------

void ScDocShell::DoEnterHandler()
{
    ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
    if (pViewSh)
        if (pViewSh->GetViewData()->GetDocShell() == this)
            SC_MOD()->InputEnterHandler();
}

//------------------------------------------------------------------

SCTAB ScDocShell::GetSaveTab()
{
    SCTAB nTab = 0;
    ScTabViewShell* pSh = GetBestViewShell();
    if (pSh)
    {
        const ScMarkData& rMark = pSh->GetViewData()->GetMarkData();
        for ( nTab = 0; nTab <= MAXTAB; nTab++ )    // erste markierte Tabelle
            if ( rMark.GetTableSelect( nTab ) )
                break;
    }
    return nTab;
}

//------------------------------------------------------------------

BOOL ScDocShell::LoadCalc( SvStorage* pStor )       // StarCalc 3, 4 or 5 file
{
    //  MacroCallMode is no longer needed, state is kept in SfxObjectShell now

    BOOL bRet = TRUE;

    SvStorageStreamRef aPoolStm = pStor->OpenStream( String::CreateFromAscii(pStyleName), STREAM_STD_READ );
    SvStorageStreamRef aDocStm  = pStor->OpenStream( String::CreateFromAscii(pStarCalcDoc), STREAM_STD_READ );
    ULONG nPoolErr = aPoolStm->GetError();
    ULONG nDocErr  = aDocStm->GetError();

    ScProgress* pProgress = NULL;
    SfxObjectCreateMode eShellMode = GetCreateMode();
    if ( eShellMode == SFX_CREATE_MODE_STANDARD && !nDocErr )
    {
        ULONG nCurPos = aDocStm->Tell();
        ULONG nEndPos = aDocStm->Seek( STREAM_SEEK_TO_END );
        aDocStm->Seek( nCurPos );
        ULONG nRange = nEndPos - nCurPos;
        pProgress = new ScProgress( this, ScGlobal::GetRscString(STR_LOAD_DOC), nRange );
    }

    if ( nPoolErr == ERRCODE_IO_NOTEXISTS && nDocErr == ERRCODE_IO_NOTEXISTS )
    {
        //  leerer Storage = leeres Dokument -> wie InitNew

        aDocument.MakeTable(0);
        Size aSize( (long) ( STD_COL_WIDTH           * HMM_PER_TWIPS * OLE_STD_CELLS_X ),
                    (long) ( ScGlobal::nStdRowHeight * HMM_PER_TWIPS * OLE_STD_CELLS_Y ) );
        // hier muss auch der Start angepasst werden
        SetVisAreaOrSize( Rectangle( Point(), aSize ), TRUE );
        aDocument.GetStyleSheetPool()->CreateStandardStyles();
        aDocument.UpdStlShtPtrsFrmNms();
    }
    else if ( !nPoolErr && !nDocErr )
    {
        aPoolStm->SetVersion(pStor->GetVersion());
        aDocStm->SetVersion(pStor->GetVersion());

        aDocument.Clear();          // keine Referenzen auf Pool behalten!

        RemoveItem( SID_ATTR_CHAR_FONTLIST );
        RemoveItem( ITEMID_COLOR_TABLE );
        RemoveItem( ITEMID_GRADIENT_LIST );
        RemoveItem( ITEMID_HATCH_LIST );
        RemoveItem( ITEMID_BITMAP_LIST );
        RemoveItem( ITEMID_DASH_LIST );
        RemoveItem( ITEMID_LINEEND_LIST );

        aDocument.LoadPool( *aPoolStm, FALSE );     // FALSE: RefCounts nicht laden
        bRet = (aPoolStm->GetError() == 0);

        if ( !bRet )
        {
            pStor->SetError( aPoolStm->GetError() );
            DBG_ERROR( "Fehler im Pool-Stream" );
        }
        else if (eShellMode != SFX_CREATE_MODE_ORGANIZER)
        {
            //  ViewOptions are not completely stored (to avoid warnings),
            //  so must be initialized from global settings.
            //! This can be removed if a new file format loads all ViewOptions !!!

            aDocument.SetViewOptions( SC_MOD()->GetViewOptions() );

            ULONG nErrCode = aPoolStm->GetErrorCode();
            const ByteString aStrKey = pStor->GetKey();
            const BOOL   bKeySet = (aStrKey.Len() > 0);

            if ( bKeySet )
                aDocStm->SetKey( aStrKey );         // Passwort setzen

            bRet = aDocument.Load( *aDocStm, pProgress );

            if ( !bRet && bKeySet ) // Passwort falsch
            {
                SetError( ERRCODE_SFX_WRONGPASSWORD );
                pStor->SetError( ERRCODE_SFX_WRONGPASSWORD );
            }

            if ( aDocStm->GetError() )
            {
                //  Zeile-8192-Meldung braucht nur zu kommen, wenn die Zeilen
                //  beschraenkt sind, sonst ist "falsches Format" besser
#ifdef SC_LIMIT_ROWS
                if ( aDocument.HasLostData() )                  // zuviele Zeilen?
                    pStor->SetError( SCERR_IMPORT_8K_LIMIT );
                else
#endif
                    pStor->SetError( aDocStm->GetError() );
                bRet = FALSE;
                DBG_ERROR( "Fehler im Document-Stream" );
            }
            else
            {
                if ( !(nErrCode & ERRCODE_WARNING_MASK) )
                    nErrCode = aDocStm->GetErrorCode();
                if ( nErrCode & ERRCODE_WARNING_MASK )
                    SetError( nErrCode );
            }

            if (bRet)
            {
                aDocument.UpdateFontCharSet();

                UpdateLinks();              // verknuepfte Tabellen in Link-Manager
                RemoveUnknownObjects();     // unbekannte Ole-Objekte loeschen
            }
        }
        else            // SFX_CREATE_MODE_ORGANIZER
        {
            //  UpdateStdNames is called from ScDocument::Load, but is also needed
            //  if only the styles are loaded!
            ScStyleSheetPool* pStylePool = aDocument.GetStyleSheetPool();
            if (pStylePool)
                pStylePool->UpdateStdNames();   // correct style names for different languages
        }

        if (bRet)
            aDocument.GetPool()->MyLoadCompleted();
    }
    else
    {
        DBG_ERROR( "Stream-Fehler");
        bRet = FALSE;
    }

    if (!aDocument.HasTable(0))
        aDocument.MakeTable(0);

    if (eShellMode == SFX_CREATE_MODE_EMBEDDED)
    {
        SCTAB nVisTab = aDocument.GetVisibleTab();
        BOOL bHasVis = aDocument.HasTable(nVisTab);
        if ( SfxInPlaceObject::GetVisArea().IsEmpty() || !bHasVis )
        {
            if (!bHasVis)
            {
                nVisTab = 0;
                aDocument.SetVisibleTab(nVisTab);
            }
            SCCOL nStartCol;
            SCROW nStartRow;
            aDocument.GetDataStart( nVisTab, nStartCol, nStartRow );
            SCCOL nEndCol;
            SCROW nEndRow;
            aDocument.GetPrintArea( nVisTab, nEndCol, nEndRow );
            if (nStartCol>nEndCol)
                nStartCol = nEndCol;
            if (nStartRow>nEndRow)
                nStartRow = nEndRow;
            // hier muss auch der Start angepasst werden
            SetVisAreaOrSize( aDocument.GetMMRect( nStartCol,nStartRow, nEndCol,nEndRow, nVisTab ),
                                TRUE );
        }
    }

    delete pProgress;

    return bRet;
}


BOOL ScDocShell::SaveCalc( SvStorage* pStor )           // Calc 3, 4 or 5 file
{
    BOOL bRet = TRUE;

    ScProgress* pProgress = NULL;
    SfxObjectCreateMode eShellMode = GetCreateMode();
    if ( eShellMode == SFX_CREATE_MODE_STANDARD )
    {
        ULONG nRange = aDocument.GetWeightedCount() + 1;
        pProgress = new ScProgress( this, ScGlobal::GetRscString(STR_SAVE_DOC), nRange );
    }

    SvStorageStreamRef aPoolStm = pStor->OpenStream( String::CreateFromAscii(pStyleName) );
    if( !aPoolStm->GetError() )
    {
        aPoolStm->SetVersion(pStor->GetVersion());
        aPoolStm->SetSize(0);
        bRet = aDocument.SavePool( *aPoolStm );
        if ( aPoolStm->GetErrorCode() && !pStor->GetErrorCode() )
            pStor->SetError(aPoolStm->GetErrorCode());
    }
    else
    {
        DBG_ERROR( "Stream Error" );
        bRet = FALSE;
    }

    if ( bRet && eShellMode != SFX_CREATE_MODE_ORGANIZER )
    {
        SvStorageStreamRef aDocStm  = pStor->OpenStream( String::CreateFromAscii(pStarCalcDoc) );
        if( !aDocStm->GetError() )
        {
            aDocStm->SetVersion(pStor->GetVersion());
            aDocStm->SetKey(pStor->GetKey());               // Passwort setzen
            aDocStm->SetSize(0);
            bRet = aDocument.Save( *aDocStm, pProgress );
            DBG_ASSERT( bRet, "Error while saving" );

            if ( aDocument.HasLostData() )
            {
                //  Warnung, dass nicht alles gespeichert wurde

                if (!pStor->GetError())
                    pStor->SetError(SCWARN_EXPORT_MAXROW);
            }
            else if ( aDocStm->GetErrorCode() && !pStor->GetErrorCode() )
                pStor->SetError(aDocStm->GetErrorCode());
            else if ( SvtFilterOptions::Get()->IsLoadExcelBasicStorage() )
            {
                //  #75497# warning if MS VBA macros are lost
                //  GetSaveWarningOfMSVBAStorage checks if sub-storage with VBA macros is present
                //  (only possible when editing Excel documents -- storage is not copied to
                //  StarCalc files)
                ULONG nVBAWarn = SvxImportMSVBasic::GetSaveWarningOfMSVBAStorage( *this );
                if ( nVBAWarn && !pStor->GetErrorCode() )
                    pStor->SetError( nVBAWarn );
            }
        }
        else
        {
            DBG_ERROR( "Stream Error" );
            bRet = FALSE;
        }
    }

    delete pProgress;

    return bRet;
}

void ScDocShell::BeforeXMLLoading()
{
    // prevent unnecessary broadcasts and updates
    DBG_ASSERT(pModificator == NULL, "The Modificator should not exist");
    pModificator = new ScDocShellModificator( *this );

    aDocument.SetImportingXML( TRUE );
    // prevent unnecessary broadcasts and "half way listeners"
    aDocument.SetInsertingFromOtherDoc( TRUE );

    if (GetCreateMode() != SFX_CREATE_MODE_ORGANIZER)
        ScColumn::bDoubleAlloc = sal_True;
}

void ScDocShell::AfterXMLLoading(sal_Bool bRet)
{
    if (GetCreateMode() != SFX_CREATE_MODE_ORGANIZER)
    {
        UpdateLinks();
        // don't prevent establishing of listeners anymore
        aDocument.SetInsertingFromOtherDoc( FALSE );
        if ( bRet )
        {
            ScChartListenerCollection* pChartListener = aDocument.GetChartListenerCollection();
            if (pChartListener)
                pChartListener->UpdateDirtyCharts();

            // #95582#; set the table names of linked tables to the new path
            SCTAB nTabCount = aDocument.GetTableCount();
            for (SCTAB i = 0; i < nTabCount; ++i)
            {
                if (aDocument.IsLinked( i ))
                {
                    String aName;
                    aDocument.GetName(i, aName);
                    String aLinkTabName = aDocument.GetLinkTab(i);
                    xub_StrLen nLinkTabNameLength = aLinkTabName.Len();
                    xub_StrLen nNameLength = aName.Len();
                    if (nLinkTabNameLength < nNameLength)
                    {

                        // remove the quottes on begin and end of the docname and restore the escaped quotes
                        const sal_Unicode* pNameBuffer = aName.GetBuffer();
                        if ( *pNameBuffer == '\'' && // all docnames have to have a ' character on the first pos
                            ScGlobal::UnicodeStrChr( pNameBuffer, SC_COMPILER_FILE_TAB_SEP ) )
                        {
                            rtl::OUStringBuffer aDocURLBuffer;
                            BOOL bQuote = TRUE;         // Dokumentenname ist immer quoted
                            ++pNameBuffer;
                            while ( bQuote && *pNameBuffer )
                            {
                                if ( *pNameBuffer == '\'' && *(pNameBuffer-1) != '\\' )
                                    bQuote = FALSE;
                                else if( !(*pNameBuffer == '\\' && *(pNameBuffer+1) == '\'') )
                                    aDocURLBuffer.append(*pNameBuffer);     // falls escaped Quote: nur Quote in den Namen
                                ++pNameBuffer;
                            }


                            if( *pNameBuffer == SC_COMPILER_FILE_TAB_SEP )  // after the last quote of the docname should be the # char
                            {
                                xub_StrLen nIndex = nNameLength - nLinkTabNameLength;
                                INetURLObject aINetURLObject(aDocURLBuffer.makeStringAndClear());
                                if( aName.Equals(aLinkTabName, nIndex, nLinkTabNameLength) &&
                                    (aName.GetChar(nIndex - 1) == '#') && // before the table name should be the # char
                                    !aINetURLObject.HasError()) // the docname should be a valid URL
                                {
                                    aName = ScGlobal::GetDocTabName( aDocument.GetLinkDoc( i ), aDocument.GetLinkTab( i ) );
                                    aDocument.RenameTab(i, aName, TRUE, TRUE);
                                }
                                // else;  nothing has to happen, because it is a user given name
                            }
                            // else;  nothing has to happen, because it is a user given name
                        }
                        // else;  nothing has to happen, because it is a user given name
                    }
                    // else;  nothing has to happen, because it is a user given name
                }
            }
        }
        ScColumn::bDoubleAlloc = sal_False;
    }
    else
        aDocument.SetInsertingFromOtherDoc( FALSE );

    aDocument.SetImportingXML( FALSE );
    bIsEmpty = FALSE;

    if (pModificator)
    {
        delete pModificator;
        pModificator = NULL;
    }
    else
        DBG_ERROR("The Modificator should exist");
}

BOOL ScDocShell::LoadXML( SfxMedium* pMedium, SvStorage* pStor )
{
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "sb99857", "ScDocShell::LoadXML" );

    //  MacroCallMode is no longer needed, state is kept in SfxObjectShell now

    // no Seek(0) here - always loading from storage, GetInStream must not be called

    BeforeXMLLoading();

    ScXMLImportWrapper aImport( aDocument, pMedium, pStor );

    sal_Bool bRet(sal_False);
    if (GetCreateMode() != SFX_CREATE_MODE_ORGANIZER)
        bRet = aImport.Import(sal_False);
    else
        bRet = aImport.Import(sal_True);

    AfterXMLLoading(bRet);

    //! row heights...

    return bRet;
}

BOOL ScDocShell::SaveXML( SfxMedium* pMedium, SvStorage* pStor )
{
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "sb99857", "ScDocShell::SaveXML" );

    ScXMLImportWrapper aImport( aDocument, pMedium, pStor );
    sal_Bool bRet(sal_False);
    if (GetCreateMode() != SFX_CREATE_MODE_ORGANIZER)
        bRet = aImport.Export(sal_False);
    else
        bRet = aImport.Export(sal_True);
    return bRet;
}

BOOL __EXPORT ScDocShell::Load( SvStorage* pStor )
{
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::Load" );

    ScRefreshTimerProtector( aDocument.GetRefreshTimerControlAddress() );

    DBG_ASSERT( pStor, "Load without storage?" );
    BOOL bXML = ( pStor->GetVersion() >= SOFFICE_FILEFORMAT_60 );

    //  only the latin script language is loaded
    //  -> initialize the others from options (before loading)
    InitOptions();

    GetUndoManager()->Clear();

    BOOL bRet = SfxInPlaceObject::Load( pStor );
    if( bRet )
    {
        if (GetMedium())
        {
            SFX_ITEMSET_ARG( GetMedium()->GetItemSet(), pUpdateDocItem, SfxUInt16Item, SID_UPDATEDOCMODE, sal_False);
            nCanUpdate = pUpdateDocItem ? pUpdateDocItem->GetValue() : com::sun::star::document::UpdateDocMode::NO_UPDATE;
        }

        if (bXML)
        {
            //  prepare a valid document for XML filter
            //  (for ConvertFrom, InitNew is called before)
            aDocument.MakeTable(0);
            aDocument.GetStyleSheetPool()->CreateStandardStyles();
            aDocument.UpdStlShtPtrsFrmNms();

            bRet = LoadXML( GetMedium(), pStor );
        }
        else
            bRet = LoadCalc( pStor );
    }

    if (!bRet && !pStor->GetError())
        pStor->SetError( SVSTREAM_FILEFORMAT_ERROR );

    if (pStor->GetError())
        SetError( pStor->GetError() );

    InitItems();
    CalcOutputFactor();

    // #73762# invalidate eventually temporary table areas
    if ( bRet )
        aDocument.InvalidateTableArea();

    bIsEmpty = FALSE;
    FinishedLoading( SFX_LOADED_MAINDOCUMENT | SFX_LOADED_IMAGES );
    return bRet;
}


void __EXPORT ScDocShell::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                         const SfxHint& rHint, const TypeId& rHintType )
{
    if (rHint.ISA(SfxSimpleHint))                               // ohne Parameter
    {
        ULONG nSlot = ((const SfxSimpleHint&)rHint).GetId();
        switch ( nSlot )
        {
            case SFX_HINT_TITLECHANGED:
                aDocument.SetName( SfxShell::GetName() );
                //  RegisterNewTargetNames gibts nicht mehr
                SFX_APP()->Broadcast(SfxSimpleHint( SC_HINT_DOCNAME_CHANGED )); // Navigator
                break;
        }
    }
    else if (rHint.ISA(SfxStyleSheetHint))                      // Vorlagen geaendert
        NotifyStyle((const SfxStyleSheetHint&) rHint);
    else if (rHint.ISA(ScAutoStyleHint))
    {
        //! direct call for AutoStyles

        //  this is called synchronously from ScInterpreter::ScStyle,
        //  modifying the document must be asynchronous
        //  (handled by AddInitial)

        ScAutoStyleHint& rStlHint = (ScAutoStyleHint&)rHint;
        ScRange aRange = rStlHint.GetRange();
        String aName1 = rStlHint.GetStyle1();
        String aName2 = rStlHint.GetStyle2();
        UINT32 nTimeout = rStlHint.GetTimeout();

        if (!pAutoStyleList)
            pAutoStyleList = new ScAutoStyleList(this);
        pAutoStyleList->AddInitial( aRange, aName1, nTimeout, aName2 );
    }
}

    // Inhalte fuer Organizer laden


BOOL __EXPORT ScDocShell::LoadFrom( SvStorage* pStor )
{
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::LoadFrom" );

    ScRefreshTimerProtector( aDocument.GetRefreshTimerControlAddress() );

    DBG_ASSERT( pStor, "Nanu... LoadFrom ohne Storage?" );
    BOOL bXML = ( pStor->GetVersion() >= SOFFICE_FILEFORMAT_60 );

    WaitObject aWait( GetDialogParent() );

    BOOL bRet = FALSE;

    if (GetMedium())
    {
        SFX_ITEMSET_ARG( GetMedium()->GetItemSet(), pUpdateDocItem, SfxUInt16Item, SID_UPDATEDOCMODE, sal_False);
        nCanUpdate = pUpdateDocItem ? pUpdateDocItem->GetValue() : com::sun::star::document::UpdateDocMode::NO_UPDATE;
    }

    if ( bXML )
    {
        //  until loading/saving only the styles in XML is implemented,
        //  load the whole file

        bRet = LoadXML( GetMedium(), pStor );
        InitItems();
    }
    else
    {
        SvStorageStreamRef aPoolStm = pStor->OpenStream( String::CreateFromAscii(pStyleName), STREAM_STD_READ );
        if ( !aPoolStm->GetError() )
        {
            aPoolStm->SetVersion(pStor->GetVersion());

            aDocument.Clear();                      // keine Referenzen auf Pool behalten!
            RemoveItem( SID_ATTR_CHAR_FONTLIST );
            RemoveItem( ITEMID_COLOR_TABLE );
            RemoveItem( ITEMID_GRADIENT_LIST );
            RemoveItem( ITEMID_HATCH_LIST );
            RemoveItem( ITEMID_BITMAP_LIST );
            RemoveItem( ITEMID_DASH_LIST );
            RemoveItem( ITEMID_LINEEND_LIST );

            aDocument.LoadPool( *aPoolStm, TRUE );      // TRUE: RefCounts aus Datei laden
            bRet = (aPoolStm->GetError() == 0);
            DBG_ASSERT( bRet, "Error in pool stream" );

            //  UpdateStdNames is called from ScDocument::Load, but is also needed
            //  if only the styles are loaded!
            ScStyleSheetPool* pStylePool = aDocument.GetStyleSheetPool();
            if (pStylePool)
                pStylePool->UpdateStdNames();   // correct style names for different languages

            //  Hier auf keinen Fall LoadCompleted, weil ohne Laden der Tabellen die RefCounts
            //  nicht hochgezaehlt wurden.
            //  Die Items wuerden dann geloescht, und beim Speichern wuerde Muell herauskommen.
            //  Darum die Ref-Counts aus der Datei laden (TRUE bei LoadPool).
            //  (Bug #37635#)

            InitItems();
            //  CalcOutputFactor interessiert hier nicht
        }
    }

    SfxObjectShell::LoadFrom( pStor );

    return bRet;
}


BOOL __EXPORT ScDocShell::ConvertFrom( SfxMedium& rMedium )
{
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::ConvertFrom" );

    BOOL bRet = FALSE;              // FALSE heisst Benutzerabbruch !!
                                    // bei Fehler: Fehler am Stream setzen!!

    ScRefreshTimerProtector( aDocument.GetRefreshTimerControlAddress() );

    GetUndoManager()->Clear();

    // ob nach dem Import optimale Spaltenbreiten gesetzt werden sollen
    BOOL bSetColWidths = FALSE;
    BOOL bSetSimpleTextColWidths = FALSE;
    BOOL bSimpleColWidth[MAXCOLCOUNT];
    memset( bSimpleColWidth, 1, (MAXCOLCOUNT) * sizeof(BOOL) );
    ScRange aColWidthRange;
    // ob nach dem Import optimale Zeilenhoehen gesetzt werden sollen
    BOOL bSetRowHeights = FALSE;

    aConvFilterName.Erase(); //@ #BugId 54198

    //  Alle Filter brauchen die komplette Datei am Stueck (nicht asynchron),
    //  darum vorher per CreateFileStream dafuer sorgen, dass die komplette
    //  Datei uebertragen wird.
    rMedium.GetPhysicalName();  //! CreateFileStream direkt rufen, wenn verfuegbar

    SFX_ITEMSET_ARG( rMedium.GetItemSet(), pUpdateDocItem, SfxUInt16Item, SID_UPDATEDOCMODE, sal_False);
    nCanUpdate = pUpdateDocItem ? pUpdateDocItem->GetValue() : com::sun::star::document::UpdateDocMode::NO_UPDATE;

    const SfxFilter* pFilter = rMedium.GetFilter();
    if (pFilter)
    {
        String aFltName = pFilter->GetFilterName();

        aConvFilterName=aFltName; //@ #BugId 54198

        BOOL bCalc3 = ( aFltName.EqualsAscii(pFilterSc30) );
        BOOL bCalc4 = ( aFltName.EqualsAscii(pFilterSc40) );
        if (!bCalc3 && !bCalc4)
            aDocument.SetInsertingFromOtherDoc( TRUE );

        if (bCalc3 || bCalc4)                   // Calc3/4 - "Import"
        {
            //  wait cursor is handled with progress bar
            SvStorage* pStor = rMedium.GetStorage();
            if ( pStor )
            {
                bRet = SfxInPlaceObject::Load( pStor );
                if( bRet )
                    bRet = LoadCalc( pStor );
                if (!bRet && !pStor->GetError())
                    pStor->SetError( SVSTREAM_FILEFORMAT_ERROR );
            }
            else
                DBG_ERROR("Calc3/4: kein Storage");
        }
        else if (aFltName.EqualsAscii(pFilterXML))
            bRet = LoadXML( &rMedium, NULL );
        else if (aFltName.EqualsAscii(pFilterSc10))
        {
            SvStream* pStream = rMedium.GetInStream();
            if (pStream)
            {
                FltError eError = ScImportStarCalc10( *pStream, &aDocument );
                if (eError != eERR_OK)
                {
                    if (!GetError())
                        SetError(eError);
                }
                else
                    bRet = TRUE;
            }
        }
        else if (aFltName.EqualsAscii(pFilterLotus))
        {
//          SvStream* pStream = rMedium.GetInStream();
//          if (pStream)
            {
                String sItStr;
                SfxItemSet*  pSet = rMedium.GetItemSet();
                const SfxPoolItem* pItem;
                if ( pSet && SFX_ITEM_SET ==
                     pSet->GetItemState( SID_FILE_FILTEROPTIONS, TRUE, &pItem ) )
                {
                    sItStr = ((const SfxStringItem*)pItem)->GetValue();
                }

                if (sItStr.Len() == 0)
                {
                    //  default for lotus import (from API without options):
                    //  IBM_437 encoding

                    sItStr = ScGlobal::GetCharsetString( RTL_TEXTENCODING_IBM_437 );
                }

                ScColumn::bDoubleAlloc = TRUE;
                FltError eError = ScImportLotus123( rMedium, &aDocument,
                                    ScGlobal::GetCharsetValue(sItStr));
                ScColumn::bDoubleAlloc = FALSE;
                if (eError != eERR_OK)
                {
                    if (!GetError())
                        SetError(eError);

                    if( ( eError & ERRCODE_WARNING_MASK ) == ERRCODE_WARNING_MASK )
                        bRet = TRUE;
                }
                else
                    bRet = TRUE;

//              rMedium.CloseInStream();
            }
        }
        else if ( aFltName.EqualsAscii(pFilterExcel4) || aFltName.EqualsAscii(pFilterExcel5) ||
                   aFltName.EqualsAscii(pFilterExcel95) || aFltName.EqualsAscii(pFilterExcel97) ||
                   aFltName.EqualsAscii(pFilterEx4Temp) || aFltName.EqualsAscii(pFilterEx5Temp) ||
                   aFltName.EqualsAscii(pFilterEx95Temp) || aFltName.EqualsAscii(pFilterEx97Temp) )
        {
            EXCIMPFORMAT eFormat = EIF_AUTO;
            if ( aFltName.EqualsAscii(pFilterExcel4) || aFltName.EqualsAscii(pFilterEx4Temp) )
                eFormat = EIF_BIFF_LE4;
            else if ( aFltName.EqualsAscii(pFilterExcel5) || aFltName.EqualsAscii(pFilterExcel95) ||
                      aFltName.EqualsAscii(pFilterEx5Temp) || aFltName.EqualsAscii(pFilterEx95Temp) )
                eFormat = EIF_BIFF5;
            else if ( aFltName.EqualsAscii(pFilterExcel97) || aFltName.EqualsAscii(pFilterEx97Temp) )
                eFormat = EIF_BIFF8;

            MakeDrawLayer();                //! im Filter
            CalcOutputFactor();             // #93255# prepare update of row height
            ScColumn::bDoubleAlloc = TRUE;
            FltError eError = ScImportExcel( rMedium, &aDocument, eFormat );
            ScColumn::bDoubleAlloc = FALSE;
            aDocument.UpdateFontCharSet();
            if ( aDocument.IsChartListenerCollectionNeedsUpdate() )
                aDocument.UpdateChartListenerCollection();              //! fuer alle Importe?

            // #75299# all graphics objects must have names
            aDocument.EnsureGraphicNames();

            if (eError == SCWARN_IMPORT_RANGE_OVERFLOW)
            {
                if (!GetError())
                    SetError(eError);
                bRet = TRUE;
            }
            else if (eError != eERR_OK)
            {
                if (!GetError())
                    SetError(eError);
            }
            else
                bRet = TRUE;

            // #93255# update of row height done inside of Excel filter to speed up chart import
//            bSetRowHeights = TRUE;      //  #75357# optimal row heights must be updated
        }
        else if (aFltName.EqualsAscii(pFilterAscii))
        {
            SfxItemSet*  pSet = rMedium.GetItemSet();
            const SfxPoolItem* pItem;
            ScAsciiOptions aOptions;
            BOOL bOptInit = FALSE;

            if ( pSet && SFX_ITEM_SET ==
                 pSet->GetItemState( SID_FILE_FILTEROPTIONS, TRUE, &pItem ) )
            {
                aOptions.ReadFromString( ((const SfxStringItem*)pItem)->GetValue() );
                bOptInit = TRUE;
            }

            if ( !bOptInit )
            {
                //  default for ascii import (from API without options):
                //  ISO8859-1/MS_1252 encoding, comma, double quotes

                aOptions.SetCharSet( RTL_TEXTENCODING_MS_1252 );
                aOptions.SetFieldSeps( (sal_Unicode) ',' );
                aOptions.SetTextSep( (sal_Unicode) '"' );
            }

            FltError eError = eERR_OK;
            BOOL bOverflow = FALSE;

            if( ! rMedium.IsStorage() )
            {
                ScImportExport  aImpEx( &aDocument );
                aImpEx.SetExtOptions( aOptions );

                SvStream* pInStream = rMedium.GetInStream();
                if (pInStream)
                {
                    pInStream->SetStreamCharSet( aOptions.GetCharSet() );
                    pInStream->Seek( 0 );
                    bRet = aImpEx.ImportStream( *pInStream );
                    eError = bRet ? eERR_OK : SCERR_IMPORT_CONNECT;
                    aDocument.StartAllListeners();
                    aDocument.SetDirty();
                    bOverflow = aImpEx.IsOverflow();
                }
                else
                    DBG_ERROR( "No Stream" );
            }

            if (eError != eERR_OK)
            {
                if (!GetError())
                    SetError(eError);
            }
            else if ( bOverflow )
            {
                if (!GetError())
                    SetError(SCWARN_IMPORT_RANGE_OVERFLOW);
            }
            bSetColWidths = TRUE;
            bSetSimpleTextColWidths = TRUE;
        }
        else if (aFltName.EqualsAscii(pFilterDBase))
        {
            String sItStr;
            SfxItemSet*  pSet = rMedium.GetItemSet();
            const SfxPoolItem* pItem;
            if ( pSet && SFX_ITEM_SET ==
                 pSet->GetItemState( SID_FILE_FILTEROPTIONS, TRUE, &pItem ) )
            {
                sItStr = ((const SfxStringItem*)pItem)->GetValue();
            }

            if (sItStr.Len() == 0)
            {
                //  default for dBase import (from API without options):
                //  IBM_850 encoding

                sItStr = ScGlobal::GetCharsetString( RTL_TEXTENCODING_IBM_850 );
            }

            ULONG eError = DBaseImport( rMedium.GetPhysicalName(),
                    ScGlobal::GetCharsetValue(sItStr), bSimpleColWidth );

            if (eError != eERR_OK)
            {
                if (!GetError())
                    SetError(eError);
                bRet = ( eError == SCWARN_IMPORT_RANGE_OVERFLOW );
            }
            else
                bRet = TRUE;

            aColWidthRange.aStart.SetRow( 1 );  // Spaltenheader nicht
            bSetColWidths = TRUE;
            bSetSimpleTextColWidths = TRUE;
            // Memo-Felder fuehren zu einem bSimpleColWidth[nCol]==FALSE
            for ( SCCOL nCol=0; nCol <= MAXCOL && !bSetRowHeights; nCol++ )
            {
                if ( !bSimpleColWidth[nCol] )
                    bSetRowHeights = TRUE;
            }
        }
        else if (aFltName.EqualsAscii(pFilterDif))
        {
            SvStream* pStream = rMedium.GetInStream();
            if (pStream)
            {
                FltError eError;
                String sItStr;
                SfxItemSet*  pSet = rMedium.GetItemSet();
                const SfxPoolItem* pItem;
                if ( pSet && SFX_ITEM_SET ==
                     pSet->GetItemState( SID_FILE_FILTEROPTIONS, TRUE, &pItem ) )
                {
                    sItStr = ((const SfxStringItem*)pItem)->GetValue();
                }

                if (sItStr.Len() == 0)
                {
                    //  default for DIF import (from API without options):
                    //  ISO8859-1/MS_1252 encoding

                    sItStr = ScGlobal::GetCharsetString( RTL_TEXTENCODING_MS_1252 );
                }

                eError = ScImportDif( *pStream, &aDocument, ScAddress(0,0,0),
                                    ScGlobal::GetCharsetValue(sItStr));
                if (eError != eERR_OK)
                {
                    if (!GetError())
                        SetError(eError);

                    if( ( eError & ERRCODE_WARNING_MASK ) == ERRCODE_WARNING_MASK )
                        bRet = TRUE;
                }
                else
                    bRet = TRUE;

                rMedium.CloseInStream();
            }
            bSetColWidths = TRUE;
            bSetSimpleTextColWidths = TRUE;
        }
        else if (aFltName.EqualsAscii(pFilterSylk))
        {
            FltError eError = SCERR_IMPORT_UNKNOWN;
            if( !rMedium.IsStorage() )
            {
                ScImportExport aImpEx( &aDocument );

                SvStream* pInStream = rMedium.GetInStream();
                if (pInStream)
                {
                    pInStream->Seek( 0 );
                    bRet = aImpEx.ImportStream( *pInStream, SOT_FORMATSTR_ID_SYLK );
                    eError = bRet ? eERR_OK : SCERR_IMPORT_UNKNOWN;
                    aDocument.StartAllListeners();
                    aDocument.SetDirty();
                }
                else
                    DBG_ERROR( "No Stream" );
            }

            if ( eError != eERR_OK && !GetError() )
                SetError(eError);
            bSetColWidths = TRUE;
            bSetSimpleTextColWidths = TRUE;
        }
        else if (aFltName.EqualsAscii(pFilterRtf))
        {
            FltError eError = SCERR_IMPORT_UNKNOWN;
            if( !rMedium.IsStorage() )
            {
                SvStream* pInStream = rMedium.GetInStream();
                if (pInStream)
                {
                    pInStream->Seek( 0 );
                    ScRange aRange;
                    eError = ScImportRTF( *pInStream, &aDocument, aRange );
                    if (eError != eERR_OK)
                    {
                        if (!GetError())
                            SetError(eError);

                        if( ( eError & ERRCODE_WARNING_MASK ) == ERRCODE_WARNING_MASK )
                            bRet = TRUE;
                    }
                    else
                        bRet = TRUE;
                    aDocument.StartAllListeners();
                    aDocument.SetDirty();
                    bSetColWidths = TRUE;
                    bSetRowHeights = TRUE;
                }
                else
                    DBG_ERROR( "No Stream" );
            }

            if ( eError != eERR_OK && !GetError() )
                SetError(eError);
        }
        else if (aFltName.EqualsAscii(pFilterHtml) || aFltName.EqualsAscii(pFilterHtmlWebQ))
        {
            FltError eError = SCERR_IMPORT_UNKNOWN;
            BOOL bWebQuery = aFltName.EqualsAscii(pFilterHtmlWebQ);
            if( !rMedium.IsStorage() )
            {
                SvStream* pInStream = rMedium.GetInStream();
                if (pInStream)
                {
                    pInStream->Seek( 0 );
                    ScRange aRange;
                    // HTML macht eigenes ColWidth/RowHeight
                    CalcOutputFactor();
                    eError = ScImportHTML( *pInStream, &aDocument, aRange,
                                            GetOutputFactor(), !bWebQuery );
                    if (eError != eERR_OK)
                    {
                        if (!GetError())
                            SetError(eError);

                        if( ( eError & ERRCODE_WARNING_MASK ) == ERRCODE_WARNING_MASK )
                            bRet = TRUE;
                    }
                    else
                        bRet = TRUE;
                    aDocument.StartAllListeners();
                    aDocument.SetDirty();
                }
                else
                    DBG_ERROR( "No Stream" );
            }

            if ( eError != eERR_OK && !GetError() )
                SetError(eError);
        }
        else
        {
            if (!GetError())
                SetError(SCERR_IMPORT_NI);
        }

        if (!bCalc3)
            aDocument.SetInsertingFromOtherDoc( FALSE );
    }
    else
        DBG_ERROR("Kein Filter bei ConvertFrom");

    InitItems();
    CalcOutputFactor();
    if ( bRet && (bSetColWidths || bSetRowHeights) )
    {   // Spaltenbreiten/Zeilenhoehen anpassen, Basis 100% Zoom
        Fraction aZoom( 1, 1 );
        double nPPTX = ScGlobal::nScreenPPTX * (double) aZoom
            / GetOutputFactor();    // Faktor ist Drucker zu Bildschirm
        double nPPTY = ScGlobal::nScreenPPTY * (double) aZoom;
        VirtualDevice aVirtDev;
        //  all sheets (for Excel import)
        SCTAB nTabCount = aDocument.GetTableCount();
        for (SCTAB nTab=0; nTab<nTabCount; nTab++)
        {
            SCCOL nEndCol;
            SCROW nEndRow;
            aDocument.GetCellArea( nTab, nEndCol, nEndRow );
            aColWidthRange.aEnd.SetCol( nEndCol );
            aColWidthRange.aEnd.SetRow( nEndRow );
            ScMarkData aMark;
            aMark.SetMarkArea( aColWidthRange );
            aMark.MarkToMulti();
            // Reihenfolge erst Breite dann Hoehe ist wichtig (vergl. hund.rtf)
            if ( bSetColWidths )
            {
                for ( SCCOL nCol=0; nCol <= nEndCol; nCol++ )
                {
                    USHORT nWidth = aDocument.GetOptimalColWidth(
                        nCol, nTab, &aVirtDev, nPPTX, nPPTY, aZoom, aZoom, FALSE, &aMark,
                        (bSetSimpleTextColWidths && bSimpleColWidth[nCol]) );
                    aDocument.SetColWidth( nCol, nTab,
                        nWidth + (USHORT)ScGlobal::nLastColWidthExtra );
                }
            }
//          if ( bSetRowHeights )
//          {
//              //  nExtra must be 0
//              aDocument.SetOptimalHeight( 0, nEndRow, nTab, 0, &aVirtDev,
//                  nPPTX, nPPTY, aZoom, aZoom, FALSE );
//          }
        }
        if ( bSetRowHeights )
            UpdateAllRowHeights();      // with vdev or printer, depending on configuration
    }
    FinishedLoading( SFX_LOADED_MAINDOCUMENT | SFX_LOADED_IMAGES );

    // #73762# invalidate eventually temporary table areas
    if ( bRet )
        aDocument.InvalidateTableArea();

    bIsEmpty = FALSE;

    return bRet;
}


void __EXPORT ScDocShell::HandsOff()
{
    ScDrawLayer* pDrawLayer = aDocument.GetDrawLayer();

    SfxInPlaceObject::HandsOff();

    if( pDrawLayer )
        pDrawLayer->ReleasePictureStorage();
}


BOOL __EXPORT ScDocShell::Save()
{
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::Save" );

    ScRefreshTimerProtector( aDocument.GetRefreshTimerControlAddress() );

    SvStorage* pStor = GetStorage();
    DBG_ASSERT( pStor, "Save: no storage" );
    BOOL bXML = ( pStor->GetVersion() >= SOFFICE_FILEFORMAT_60 );

    //  DoEnterHandler hier nicht (wegen AutoSave), ist im ExecuteSave

    ScChartListenerCollection* pCharts = aDocument.GetChartListenerCollection();
    if (pCharts)
        pCharts->UpdateDirtyCharts();                   // Charts, die noch upgedated werden muessen
    if (pAutoStyleList)
        pAutoStyleList->ExecuteAllNow();                // Vorlagen-Timeouts jetzt ausfuehren
    if (GetCreateMode()== SFX_CREATE_MODE_STANDARD)
        SvInPlaceObject::SetVisArea( Rectangle() );     // normal bearbeitet -> keine VisArea

    // #77577# save additionally XML in storage
    if ( GetCreateMode() != SFX_CREATE_MODE_EMBEDDED && !bXML )
        AddXMLAsZipToTheStorage( *pStor );

    //  wait cursor is handled with progress bar
    BOOL bRet = SfxInPlaceObject::Save();
    if( bRet )
    {
        if (bXML)
            bRet = SaveXML( NULL, pStor );
        else
            bRet = SaveCalc( pStor );
    }
    return bRet;
}


BOOL __EXPORT ScDocShell::SaveAs( SvStorage* pStor )
{
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::SaveAs" );

    ScRefreshTimerProtector( aDocument.GetRefreshTimerControlAddress() );

    DBG_ASSERT( pStor, "SaveAs without storage?" );
    BOOL bXML = ( pStor->GetVersion() >= SOFFICE_FILEFORMAT_60 );

    //  DoEnterHandler hier nicht (wegen AutoSave), ist im ExecuteSave

    ScChartListenerCollection* pCharts = aDocument.GetChartListenerCollection();
    if (pCharts)
        pCharts->UpdateDirtyCharts();                   // Charts, die noch upgedated werden muessen
    if (pAutoStyleList)
        pAutoStyleList->ExecuteAllNow();                // Vorlagen-Timeouts jetzt ausfuehren
    if (GetCreateMode()== SFX_CREATE_MODE_STANDARD)
        SvInPlaceObject::SetVisArea( Rectangle() );     // normal bearbeitet -> keine VisArea

    // #77577# save additionally XML in storage
    if ( GetCreateMode() != SFX_CREATE_MODE_EMBEDDED && !bXML )
        AddXMLAsZipToTheStorage( *pStor );

    //  wait cursor is handled with progress bar
    BOOL bRet = SfxInPlaceObject::SaveAs( pStor );
    if( bRet )
    {
        if (bXML)
            bRet = SaveXML( NULL, pStor );
        else
            bRet = SaveCalc( pStor );
    }

    return bRet;
}


BOOL __EXPORT ScDocShell::IsInformationLost()
{
/*
    const SfxFilter *pFilt = GetMedium()->GetFilter();
    BOOL bRet = pFilt && pFilt->IsAlienFormat() && bNoInformLost;
    if (bNoInformLost)                  // nur einmal!!
        bNoInformLost = FALSE;
    return bRet;
*/
    //!!! bei Gelegenheit ein korrekte eigene Behandlung einbauen

    return SfxObjectShell::IsInformationLost();
}


// Xcl-like column width measured in characters of standard font.
xub_StrLen lcl_ScDocShell_GetColWidthInChars( USHORT nWidth )
{
    // double fColScale = 1.0;
    double  f = nWidth;
    f *= 1328.0 / 25.0;
    f += 90.0;
    f *= 1.0 / 23.0;
    // f /= fColScale * 256.0;
    f /= 256.0;

    return xub_StrLen( f );
}


void lcl_ScDocShell_GetFixedWidthString( String& rStr, const ScDocument& rDoc,
        SCTAB nTab, SCCOL nCol, BOOL bValue, SvxCellHorJustify eHorJust )
{
    xub_StrLen nLen = lcl_ScDocShell_GetColWidthInChars(
            rDoc.GetColWidth( nCol, nTab ) );
    if ( nLen < rStr.Len() )
    {
        if ( bValue )
            rStr.AssignAscii( "###" );
        rStr.Erase( nLen );
    }
    if ( nLen > rStr.Len() )
    {
        if ( bValue && eHorJust == SVX_HOR_JUSTIFY_STANDARD )
            eHorJust = SVX_HOR_JUSTIFY_RIGHT;
        switch ( eHorJust )
        {
            case SVX_HOR_JUSTIFY_RIGHT:
            {
                String aTmp;
                aTmp.Fill( nLen - rStr.Len() );
                rStr.Insert( aTmp, 0 );
            }
            break;
            case SVX_HOR_JUSTIFY_CENTER:
            {
                xub_StrLen nLen2 = (nLen - rStr.Len()) / 2;
                String aTmp;
                aTmp.Fill( nLen2 );
                rStr.Insert( aTmp, 0 );
                rStr.Expand( nLen );
            }
            break;
            default:
                rStr.Expand( nLen );
        }
    }
}


void lcl_ScDocShell_WriteEmptyFixedWidthString( SvStream& rStream,
        const ScDocument& rDoc, SCTAB nTab, SCCOL nCol )
{
    String aString;
    lcl_ScDocShell_GetFixedWidthString( aString, rDoc, nTab, nCol, FALSE,
            SVX_HOR_JUSTIFY_STANDARD );
    rStream.WriteUnicodeOrByteText( aString );
}


void ScDocShell::AsciiSave( SvStream& rStream, const ScImportOptions& rAsciiOpt )
{
    sal_Unicode cDelim    = rAsciiOpt.nFieldSepCode;
    sal_Unicode cStrDelim = rAsciiOpt.nTextSepCode;
    CharSet eCharSet      = rAsciiOpt.eCharSet;
    BOOL bFixedWidth      = rAsciiOpt.bFixedWidth;

    CharSet eOldCharSet = rStream.GetStreamCharSet();
    rStream.SetStreamCharSet( eCharSet );
    USHORT nOldNumberFormatInt = rStream.GetNumberFormatInt();
    ByteString aStrDelimEncoded;    // only used if not Unicode
    UniString aStrDelimDecoded;     // only used if context encoding
    BOOL bContextOrNotAsciiEncoding;
    if ( eCharSet == RTL_TEXTENCODING_UNICODE )
    {
        rStream.StartWritingUnicodeText();
        bContextOrNotAsciiEncoding = FALSE;
    }
    else
    {
        aStrDelimEncoded = ByteString( cStrDelim, eCharSet );
        rtl_TextEncodingInfo aInfo;
        aInfo.StructSize = sizeof(aInfo);
        if ( rtl_getTextEncodingInfo( eCharSet, &aInfo ) )
        {
            bContextOrNotAsciiEncoding =
                (((aInfo.Flags & RTL_TEXTENCODING_INFO_CONTEXT) != 0) ||
                 ((aInfo.Flags & RTL_TEXTENCODING_INFO_ASCII) == 0));
            if ( bContextOrNotAsciiEncoding )
                aStrDelimDecoded = String( aStrDelimEncoded, eCharSet );
        }
        else
            bContextOrNotAsciiEncoding = FALSE;
    }

    SCCOL nStartCol = 0;
    SCROW nStartRow = 0;
    SCTAB nTab = GetSaveTab();
    SCCOL nEndCol;
    SCROW nEndRow;
    aDocument.GetCellArea( nTab, nEndCol, nEndRow );

    ScProgress aProgress( this, ScGlobal::GetRscString( STR_SAVE_DOC ), nEndRow );

    String aString;

    ScTabViewShell* pViewSh = PTR_CAST(ScTabViewShell, SfxViewShell::Current());
    const ScViewOptions& rOpt = (pViewSh)
                                ? pViewSh->GetViewData()->GetOptions()
                                : aDocument.GetViewOptions();
    BOOL bShowFormulas = rOpt.GetOption( VOPT_FORMULAS );
    BOOL bTabProtect = aDocument.IsTabProtected( nTab );

    SCCOL nCol;
    SCROW nRow;
    SCCOL nNextCol = nStartCol;
    SCROW nNextRow = nStartRow;
    SCCOL nEmptyCol;
    SCROW nEmptyRow;
    SvNumberFormatter& rFormatter = *aDocument.GetFormatTable();

    ScHorizontalCellIterator aIter( &aDocument, nTab, nStartCol, nStartRow,
        nEndCol, nEndRow );
    ScBaseCell* pCell;
    while ( pCell = aIter.GetNext( nCol, nRow ) )
    {
        BOOL bProgress = FALSE;     // only upon line change
        if ( nNextRow < nRow )
        {   // empty rows or/and empty columns up to end of row
            bProgress = TRUE;
            for ( nEmptyCol = nNextCol; nEmptyCol < nEndCol; nEmptyCol++ )
            {   // remaining columns of last row
                if ( bFixedWidth )
                    lcl_ScDocShell_WriteEmptyFixedWidthString( rStream,
                            aDocument, nTab, nEmptyCol );
                else if ( cDelim != 0 )
                    rStream.WriteUniOrByteChar( cDelim );
            }
            endlub( rStream );
            nNextRow++;
            for ( nEmptyRow = nNextRow; nEmptyRow < nRow; nEmptyRow++ )
            {   // completely empty rows
                for ( nEmptyCol = nStartCol; nEmptyCol < nEndCol; nEmptyCol++ )
                {
                    if ( bFixedWidth )
                        lcl_ScDocShell_WriteEmptyFixedWidthString( rStream,
                                aDocument, nTab, nEmptyCol );
                    else if ( cDelim != 0 )
                        rStream.WriteUniOrByteChar( cDelim );
                }
                endlub( rStream );
            }
            for ( nEmptyCol = nStartCol; nEmptyCol < nCol; nEmptyCol++ )
            {   // empty columns at beginning of row
                if ( bFixedWidth )
                    lcl_ScDocShell_WriteEmptyFixedWidthString( rStream,
                            aDocument, nTab, nEmptyCol );
                else if ( cDelim != 0 )
                    rStream.WriteUniOrByteChar( cDelim );
            }
            nNextRow = nRow;
        }
        else if ( nNextCol < nCol )
        {   // empty columns in same row
            for ( nEmptyCol = nNextCol; nEmptyCol < nCol; nEmptyCol++ )
            {   // columns in between
                if ( bFixedWidth )
                    lcl_ScDocShell_WriteEmptyFixedWidthString( rStream,
                            aDocument, nTab, nEmptyCol );
                else if ( cDelim != 0 )
                    rStream.WriteUniOrByteChar( cDelim );
            }
        }
        if ( nCol == nEndCol )
        {
            bProgress = TRUE;
            nNextCol = nStartCol;
            nNextRow = nRow + 1;
        }
        else
            nNextCol = nCol + 1;

        CellType eType = pCell->GetCellType();
        if ( bTabProtect )
        {
            const ScProtectionAttr* pProtAttr =
                (const ScProtectionAttr*) aDocument.GetAttr(
                                                            nCol, nRow, nTab, ATTR_PROTECTION );
            if ( pProtAttr->GetHideCell() ||
                    ( eType == CELLTYPE_FORMULA && bShowFormulas &&
                      pProtAttr->GetHideFormula() ) )
                eType = CELLTYPE_NONE;  // hide
        }
        BOOL bString;
        switch ( eType )
        {
            case CELLTYPE_NOTE:
            case CELLTYPE_NONE:
                aString.Erase();
                bString = FALSE;
                break;
            case CELLTYPE_FORMULA :
                {
                    USHORT nErrCode;
                    if ( bShowFormulas )
                    {
                        ((ScFormulaCell*)pCell)->GetFormula( aString );
                        bString = TRUE;
                    }
                    else if ( nErrCode = ((ScFormulaCell*)pCell)->GetErrCode() )
                    {
                        aString = ScGlobal::GetErrorString( nErrCode );
                        bString = TRUE;
                    }
                    else if ( ((ScFormulaCell*)pCell)->IsValue() )
                    {
                        ULONG nFormat;
                        aDocument.GetNumberFormat( nCol, nRow, nTab, nFormat );
                        if ( bFixedWidth )
                        {
                            Color* pDummy;
                            ScCellFormat::GetString( pCell, nFormat, aString, &pDummy, rFormatter );
                        }
                        else
                            ScCellFormat::GetInputString( pCell, nFormat, aString, rFormatter );
                        bString = FALSE;
                    }
                    else
                    {
                        ((ScFormulaCell*)pCell)->GetString( aString );
                        bString = TRUE;
                    }
                }
                break;
            case CELLTYPE_STRING :
                ((ScStringCell*)pCell)->GetString( aString );
                bString = TRUE;
                break;
            case CELLTYPE_EDIT :
                ((ScEditCell*)pCell)->GetString( aString );
                bString = TRUE;
                break;
            case CELLTYPE_VALUE :
                {
                    ULONG nFormat;
                    aDocument.GetNumberFormat( nCol, nRow, nTab, nFormat );
                    if ( bFixedWidth )
                    {
                        Color* pDummy;
                        ScCellFormat::GetString( pCell, nFormat, aString, &pDummy, rFormatter );
                    }
                    else
                        ScCellFormat::GetInputString( pCell, nFormat, aString, rFormatter );
                    bString = FALSE;
                }
                break;
            default:
                DBG_ERROR( "ScDocShell::AsciiSave: unknown CellType" );
                aString.Erase();
                bString = FALSE;
        }

        if ( bFixedWidth )
        {
            SvxCellHorJustify eHorJust = (SvxCellHorJustify)
                ((const SvxHorJustifyItem*) aDocument.GetAttr( nCol, nRow,
                nTab, ATTR_HOR_JUSTIFY ))->GetValue();
            lcl_ScDocShell_GetFixedWidthString( aString, aDocument, nTab, nCol,
                    !bString, eHorJust );
            rStream.WriteUnicodeOrByteText( aString );
        }
        else if ( bString )
        {
            if ( cStrDelim != 0 ) //@ BugId 55355
            {
                if ( eCharSet == RTL_TEXTENCODING_UNICODE )
                {
                    xub_StrLen nPos = aString.Search( cStrDelim );
                    while ( nPos != STRING_NOTFOUND )
                    {
                        aString.Insert( cStrDelim, nPos );
                        nPos = aString.Search( cStrDelim, nPos+2 );
                    }
                    rStream.WriteUniOrByteChar( cStrDelim, eCharSet );
                    rStream.WriteUnicodeText( aString );
                    rStream.WriteUniOrByteChar( cStrDelim, eCharSet );
                }
                else
                {
                    // #105549# This is nasty. The Unicode to byte encoding
                    // may convert typographical quotation marks to ASCII
                    // quotation marks, which may interfer with the delimiter,
                    // so we have to escape delimiters after the string has
                    // been encoded. Since this may happen also with UTF-8
                    // encoded typographical quotation marks if such was
                    // specified as a delimiter we have to check for the full
                    // encoded delimiter string, not just one character.
                    // Now for RTL_TEXTENCODING_ISO_2022_... and similar brain
                    // dead encodings where one code point (and especially a
                    // low ASCII value) may represent different characters, we
                    // have to convert forth and back and forth again. Same for
                    // UTF-7 since it is a context sensitive encoding too.

                    if ( bContextOrNotAsciiEncoding )
                    {
                        // to byte encoding
                        ByteString aStrEnc( aString, eCharSet );
                        // back to Unicode
                        UniString aStrDec( aStrEnc, eCharSet );
                        // search on re-decoded string
                        xub_StrLen nPos = aStrDec.Search( aStrDelimDecoded );
                        while ( nPos != STRING_NOTFOUND )
                        {
                            aStrDec.Insert( aStrDelimDecoded, nPos );
                            nPos = aStrDec.Search( aStrDelimDecoded,
                                    nPos+1+aStrDelimDecoded.Len() );
                        }
                        // write byte re-encoded
                        rStream.WriteUniOrByteChar( cStrDelim, eCharSet );
                        rStream.WriteUnicodeOrByteText( aStrDec, eCharSet );
                        rStream.WriteUniOrByteChar( cStrDelim, eCharSet );
                    }
                    else
                    {
                        ByteString aStrEnc( aString, eCharSet );
                        // search on encoded string
                        xub_StrLen nPos = aStrEnc.Search( aStrDelimEncoded );
                        while ( nPos != STRING_NOTFOUND )
                        {
                            aStrEnc.Insert( aStrDelimEncoded, nPos );
                            nPos = aStrEnc.Search( aStrDelimEncoded,
                                    nPos+1+aStrDelimEncoded.Len() );
                        }
                        // write byte encoded
                        rStream.Write( aStrDelimEncoded.GetBuffer(),
                                aStrDelimEncoded.Len() );
                        rStream.Write( aStrEnc.GetBuffer(), aStrEnc.Len() );
                        rStream.Write( aStrDelimEncoded.GetBuffer(),
                                aStrDelimEncoded.Len() );
                    }
                }
            }
            else
                rStream.WriteUnicodeOrByteText( aString );
        }
        else
            rStream.WriteUnicodeOrByteText( aString );

        if( nCol < nEndCol )
        {
            if(cDelim!=0) //@ BugId 55355
                rStream.WriteUniOrByteChar( cDelim );
        }
        else
            endlub( rStream );

        if ( bProgress )
            aProgress.SetStateOnPercent( nRow );
    }

    // write out empty if requested
    if ( nNextRow <= nEndRow )
    {
        for ( nEmptyCol = nNextCol; nEmptyCol < nEndCol; nEmptyCol++ )
        {   // remaining empty columns of last row
            if ( bFixedWidth )
                lcl_ScDocShell_WriteEmptyFixedWidthString( rStream,
                        aDocument, nTab, nEmptyCol );
            else if ( cDelim != 0 )
                rStream.WriteUniOrByteChar( cDelim );
        }
        endlub( rStream );
        nNextRow++;
    }
    for ( nEmptyRow = nNextRow; nEmptyRow <= nEndRow; nEmptyRow++ )
    {   // entire empty rows
        for ( nEmptyCol = nStartCol; nEmptyCol < nEndCol; nEmptyCol++ )
        {
            if ( bFixedWidth )
                lcl_ScDocShell_WriteEmptyFixedWidthString( rStream,
                        aDocument, nTab, nEmptyCol );
            else if ( cDelim != 0 )
                rStream.WriteUniOrByteChar( cDelim );
        }
        endlub( rStream );
    }

    rStream.SetStreamCharSet( eOldCharSet );
    rStream.SetNumberFormatInt( nOldNumberFormatInt );
}


BOOL __EXPORT ScDocShell::ConvertTo( SfxMedium &rMed )
{
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::ConvertTo" );

    ScRefreshTimerProtector( aDocument.GetRefreshTimerControlAddress() );

    //  #i6500# don't call DoEnterHandler here (doesn't work with AutoSave),
    //  it's already in ExecuteSave (as for Save and SaveAs)

    if (pAutoStyleList)
        pAutoStyleList->ExecuteAllNow();                // Vorlagen-Timeouts jetzt ausfuehren
    if (GetCreateMode()== SFX_CREATE_MODE_STANDARD)
        SvInPlaceObject::SetVisArea( Rectangle() );     // normal bearbeitet -> keine VisArea

    DBG_ASSERT( rMed.GetFilter(), "Filter == 0" );

    BOOL bRet = FALSE;
    String aFltName = rMed.GetFilter()->GetFilterName();

/*
    if (aFltName.EqualsAscii(pFilterLotus))
    {
        SvStream* pStream = rMed.GetOutStream();
        if (pStream)
        {
            FltError eError = ScExportLotus123( *pStream, &aDocument, ExpWK1,
                                                CHARSET_IBMPC_437 );
            bRet = eError == eERR_OK;
        }
    }
    else
*/
    if ( aFltName.EqualsAscii(pFilterSc30) || aFltName.EqualsAscii(pFilterSc40) )
    {
        //  Calc3/4 - "Export"
        //! wird nicht gerufen (geht ueber Save/SaveAs) - wird das noch gebraucht ???????

        DBG_ERROR("ConvertTo fuer altes Calc gibt's noch...");
        SvStorage* pStor = rMed.GetStorage();
        if ( pStor )
        {
            //  wait cursor is handled with progress bar
            bRet = SfxInPlaceObject::SaveAs( pStor );
            if( bRet )
                bRet = SaveCalc( pStor );

            String aShortTypeName = String( ScResId(SCSTR_SHORT_SCDOC_NAME) );  // "Dokument"
            if ( aFltName.EqualsAscii(pFilterSc30) )
            {
                //  Ole-Geraffel auf 3.0-Werte setzen:
                SvGlobalName aName( SO3_SC_CLASSID_30 );
                UINT32 nClip = SOT_FORMATSTR_ID_STARCALC_30;
                pStor->SetClass( aName, nClip, aShortTypeName );
            }
            else
            {
                //  Ole-Geraffel auf 4.0-Werte setzen:
                SvGlobalName aName( SO3_SC_CLASSID_40 );
                UINT32 nClip = SOT_FORMATSTR_ID_STARCALC_40;
                pStor->SetClass( aName, nClip, aShortTypeName );
            }
        }
    }
    else if (aFltName.EqualsAscii(pFilterXML))
    {
        bRet = SaveXML( &rMed, NULL );
    }
    else if (aFltName.EqualsAscii(pFilterExcel5) || aFltName.EqualsAscii(pFilterExcel95) ||
             aFltName.EqualsAscii(pFilterExcel97) || aFltName.EqualsAscii(pFilterEx5Temp) ||
             aFltName.EqualsAscii(pFilterEx95Temp) || aFltName.EqualsAscii(pFilterEx97Temp))
    {
        WaitObject aWait( GetDialogParent() );

        ScTabViewShell* pViewShell = GetBestViewShell();
        if( pViewShell )
        {
            ScExtDocOptions* pExtDocOpt = aDocument.GetExtDocOptions();
            if( !pExtDocOpt )
                aDocument.SetExtDocOptions( pExtDocOpt = new ScExtDocOptions );
            pViewShell->GetViewData()->WriteExtOptions( *pExtDocOpt );
        }

        BOOL bFake97 = ( aFltName.EqualsAscii(pFilterExcel97) || aFltName.EqualsAscii(pFilterEx97Temp) );
        FltError eError = ScExportExcel5( rMed, &aDocument, bFake97, RTL_TEXTENCODING_MS_1252 );

        if (eError && !GetError())
            SetError(eError);

        if( ( eError & ERRCODE_WARNING_MASK ) == ERRCODE_WARNING_MASK )
            bRet = TRUE;
        else
            bRet = eError == eERR_OK;
    }
    else if (aFltName.EqualsAscii(pFilterAscii))
    {
        SvStream* pStream = rMed.GetOutStream();
        if (pStream)
        {
            String sItStr;
            SfxItemSet*  pSet = rMed.GetItemSet();
            const SfxPoolItem* pItem;
            if ( pSet && SFX_ITEM_SET ==
                 pSet->GetItemState( SID_FILE_FILTEROPTIONS, TRUE, &pItem ) )
            {
                sItStr = ((const SfxStringItem*)pItem)->GetValue();
            }

            if ( sItStr.Len() == 0 )
            {
                //  default for ascii export (from API without options):
                //  ISO8859-1/MS_1252 encoding, comma, double quotes

                ScImportOptions aDefOptions( ',', '"', RTL_TEXTENCODING_MS_1252 );
                sItStr = aDefOptions.BuildString();
            }

            WaitObject aWait( GetDialogParent() );
            ScImportOptions aOptions( sItStr );
            AsciiSave( *pStream, aOptions );
            bRet = TRUE;

            if (aDocument.GetTableCount() > 1)
                if (!rMed.GetError())
                    rMed.SetError(SCWARN_EXPORT_ASCII);
        }
    }
    else if (aFltName.EqualsAscii(pFilterDBase))
    {
        String sCharSet;
        SfxItemSet* pSet = rMed.GetItemSet();
        const SfxPoolItem* pItem;
        if ( pSet && SFX_ITEM_SET ==
             pSet->GetItemState( SID_FILE_FILTEROPTIONS, TRUE, &pItem ) )
        {
            sCharSet = ((const SfxStringItem*)pItem)->GetValue();
        }

        if (sCharSet.Len() == 0)
        {
            //  default for dBase export (from API without options):
            //  IBM_850 encoding

            sCharSet = ScGlobal::GetCharsetString( RTL_TEXTENCODING_IBM_850 );
        }

        WaitObject aWait( GetDialogParent() );
// HACK damit Sba geoffnetes TempFile ueberschreiben kann
        rMed.CloseOutStream();
        BOOL bHasMemo = FALSE;

        ULONG eError = DBaseExport( rMed.GetPhysicalName(),
                        ScGlobal::GetCharsetValue(sCharSet), bHasMemo );

        if ( eError != eERR_OK && (eError & ERRCODE_WARNING_MASK) )
        {
//!         if ( !rMed.GetError() )
//!             rMed.SetError( eError );
            eError = eERR_OK;
        }
//!     else if ( aDocument.GetTableCount() > 1 && !rMed.GetError() )
//!         rMed.SetError( SCWARN_EXPORT_ASCII );

        INetURLObject aTmpFile( rMed.GetPhysicalName(), INET_PROT_FILE );
        if ( bHasMemo )
            aTmpFile.setExtension( String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("dbt")) );
        if ( eError != eERR_OK )
        {
            if (!GetError())
                SetError(eError);
            if ( bHasMemo && IsDocument( aTmpFile ) )
                KillFile( aTmpFile );
        }
        else
        {
            bRet = TRUE;
            if ( bHasMemo )
            {
                SfxStringItem* pNameItem =
                    (SfxStringItem*) rMed.GetItemSet()->GetItem( SID_FILE_NAME );
                INetURLObject aDbtFile( pNameItem->GetValue(), INET_PROT_FILE );
                aDbtFile.setExtension( String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("dbt")) );
                if ( IsDocument( aDbtFile ) && !KillFile( aDbtFile ) )
                    bRet = FALSE;
                if ( bRet && !MoveFile( aTmpFile, aDbtFile ) )
                    bRet = FALSE;
                if ( !bRet )
                {
                    KillFile( aTmpFile );
                    if ( !GetError() )
                        SetError( SCERR_EXPORT_DATA );
                }
            }
        }
    }
    else if (aFltName.EqualsAscii(pFilterDif))
    {
        SvStream* pStream = rMed.GetOutStream();
        if (pStream)
        {
            String sItStr;
            SfxItemSet*  pSet = rMed.GetItemSet();
            const SfxPoolItem* pItem;
            if ( pSet && SFX_ITEM_SET ==
                 pSet->GetItemState( SID_FILE_FILTEROPTIONS, TRUE, &pItem ) )
            {
                sItStr = ((const SfxStringItem*)pItem)->GetValue();
            }

            if (sItStr.Len() == 0)
            {
                //  default for DIF export (from API without options):
                //  ISO8859-1/MS_1252 encoding

                sItStr = ScGlobal::GetCharsetString( RTL_TEXTENCODING_MS_1252 );
            }

            WaitObject aWait( GetDialogParent() );
            ScExportDif( *pStream, &aDocument, ScAddress(0,0,0),
                ScGlobal::GetCharsetValue(sItStr) );
            bRet = TRUE;

            if (aDocument.GetTableCount() > 1)
                if (!rMed.GetError())
                    rMed.SetError(SCWARN_EXPORT_ASCII);
        }
    }
    else if (aFltName.EqualsAscii(pFilterSylk))
    {
        SvStream* pStream = rMed.GetOutStream();
        if ( pStream )
        {
            WaitObject aWait( GetDialogParent() );

            SCCOL nEndCol;
            SCROW nEndRow;
            aDocument.GetCellArea( 0, nEndCol, nEndRow );
            ScRange aRange( 0,0,0, nEndCol,nEndRow,0 );

            ScImportExport aImExport( &aDocument, aRange );
            aImExport.SetFormulas( TRUE );
            bRet = aImExport.ExportStream( *pStream, SOT_FORMATSTR_ID_SYLK );
        }
    }
    else if (aFltName.EqualsAscii(pFilterHtml))
    {
        SvStream* pStream = rMed.GetOutStream();
        if ( pStream )
        {
            WaitObject aWait( GetDialogParent() );
            ScImportExport aImExport( &aDocument );
            aImExport.SetStreamPath( rMed.GetName() );
            bRet = aImExport.ExportStream( *pStream, SOT_FORMATSTR_ID_HTML );
            if ( bRet && aImExport.GetNonConvertibleChars().Len() )
                SetError( *new StringErrorInfo(
                    SCWARN_EXPORT_NONCONVERTIBLE_CHARS,
                    aImExport.GetNonConvertibleChars(),
                    ERRCODE_BUTTON_OK | ERRCODE_MSG_INFO ) );
        }
    }
    else
    {
        if (GetError())
            SetError(SCERR_IMPORT_NI);
    }
    return bRet;
}


BOOL __EXPORT ScDocShell::SaveCompleted( SvStorage * pStor )
{
    return SfxInPlaceObject::SaveCompleted( pStor );
}


BOOL __EXPORT ScDocShell::DoSaveCompleted( SfxMedium * pNewStor )
{
    BOOL bRet = SfxObjectShell::DoSaveCompleted( pNewStor );

    //  SC_HINT_DOC_SAVED fuer Wechsel ReadOnly -> Read/Write
    Broadcast( SfxSimpleHint( SC_HINT_DOC_SAVED ) );
    return bRet;
}


USHORT __EXPORT ScDocShell::PrepareClose( BOOL bUI, BOOL bForBrowsing )
{
    if(SC_MOD()->GetCurRefDlgId()>0)
    {
        SfxViewFrame* pFrame = SfxViewFrame::GetFirst( this, TYPE(SfxTopViewFrame) );
        if( pFrame )
        {
            SfxViewShell* p = pFrame->GetViewShell();
            ScTabViewShell* pViewSh = PTR_CAST(ScTabViewShell,p);
            if(pViewSh!=NULL)
            {
                Window *pWin=pViewSh->GetWindow();
                if(pWin!=NULL) pWin->GrabFocus();
            }
        }

        return FALSE;
    }
    if ( aDocument.IsInLinkUpdate() || aDocument.IsInInterpreter() )
    {
        ErrorMessage(STR_CLOSE_ERROR_LINK);
        return FALSE;
    }

    DoEnterHandler();

    USHORT nRet = SfxObjectShell::PrepareClose( bUI, bForBrowsing );
    if (nRet == TRUE)                       // TRUE = schliessen
        aDocument.DisableIdle(TRUE);        // nicht mehr drin rumpfuschen !!!

    return nRet;
}

void ScDocShell::PrepareReload()
{
    SfxObjectShell::PrepareReload();    // tut nichts?

    //  Das Disconnect von DDE-Links kann Reschedule ausloesen.
    //  Wenn die DDE-Links erst im Dokument-dtor geloescht werden, kann beim Reload
    //  aus diesem Reschedule das DDE-Link-Update fuer das neue Dokument ausgeloest
    //  werden. Dabei verklemmt sicht dann irgendwas.
    //  -> Beim Reload die DDE-Links des alten Dokuments vorher disconnecten

    aDocument.DisconnectDdeLinks();
}


String ScDocShell::GetOwnFilterName()           // static
{
    return String::CreateFromAscii(pFilterSc50);
}

String ScDocShell::GetWebQueryFilterName()      // static
{
    return String::CreateFromAscii(pFilterHtmlWebQ);
}

String ScDocShell::GetAsciiFilterName()         // static
{
    return String::CreateFromAscii(pFilterAscii);
}

String ScDocShell::GetLotusFilterName()         // static
{
    return String::CreateFromAscii(pFilterLotus);
}

String ScDocShell::GetDBaseFilterName()         // static
{
    return String::CreateFromAscii(pFilterDBase);
}

String ScDocShell::GetDifFilterName()           // static
{
    return String::CreateFromAscii(pFilterDif);
}

BOOL ScDocShell::HasAutomaticTableName( const String& rFilter )     // static
{
    //  TRUE for those filters that keep the default table name
    //  (which is language specific)

    return rFilter.EqualsAscii( pFilterAscii )
        || rFilter.EqualsAscii( pFilterLotus )
        || rFilter.EqualsAscii( pFilterExcel4 )
        || rFilter.EqualsAscii( pFilterEx4Temp )
        || rFilter.EqualsAscii( pFilterDBase )
        || rFilter.EqualsAscii( pFilterDif )
        || rFilter.EqualsAscii( pFilterSylk )
        || rFilter.EqualsAscii( pFilterHtml )
        || rFilter.EqualsAscii( pFilterRtf );
}

//==================================================================

#define __SCDOCSHELL_INIT \
        aDocument       ( SCDOCMODE_DOCUMENT, this ), \
        pUndoManager    ( NULL ), \
        pFontList       ( NULL ), \
        bHeaderOn       ( TRUE ), \
        bFooterOn       ( TRUE ), \
        pDocHelper      ( NULL ), \
        pAutoStyleList  ( NULL ), \
        pOldJobSetup    ( NULL ), \
        pPaintLockData  ( NULL ), \
        nPrtToScreenFactor( 1.0 ), \
        bIsEmpty        ( TRUE ), \
        bIsInUndo       ( FALSE ), \
        bDocumentModifiedPending( FALSE ), \
        nDocumentLock   ( 0 ), \
        nCanUpdate (com::sun::star::document::UpdateDocMode::ACCORDING_TO_CONFIG), \
        bUpdateEnabled  ( TRUE ), \
        pOldAutoDBRange ( NULL ), \
        pVirtualDevice_100th_mm ( NULL ), \
        pModificator    ( NULL )

//------------------------------------------------------------------

ScDocShell::ScDocShell( const ScDocShell& rShell )
    :   SfxObjectShell( rShell.GetCreateMode() ),
        aDdeTextFmt(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("TEXT"))),
        bNoInformLost( TRUE ),
        __SCDOCSHELL_INIT
{
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::ScDocShell" );

    SetPool( &SC_MOD()->GetPool() );

    SetShell(this);
    bIsInplace = rShell.bIsInplace;

    pDocFunc = new ScDocFunc(*this);

    //  SetBaseModel needs exception handling
    ScModelObj::CreateAndSet( this );

    StartListening(*this);
    SfxStyleSheetPool* pStlPool = aDocument.GetStyleSheetPool();
    if (pStlPool)
        StartListening(*pStlPool);

    GetPageOnFromPageStyleSet( NULL, 0, bHeaderOn, bFooterOn );
    SetHelpId( HID_SCSHELL_DOCSH );

    //  InitItems und CalcOutputFactor werden jetzt nach bei Load/ConvertFrom/InitNew gerufen
}

//------------------------------------------------------------------

ScDocShell::ScDocShell( SfxObjectCreateMode eMode )
    :   SfxObjectShell( eMode ),
        aDdeTextFmt(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("TEXT"))),
        bNoInformLost( TRUE ),
        __SCDOCSHELL_INIT
{
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::ScDocShell" );

    SetPool( &SC_MOD()->GetPool() );

    SetShell(this);
    bIsInplace = (eMode == SFX_CREATE_MODE_EMBEDDED);
    //  wird zurueckgesetzt, wenn nicht inplace

    pDocFunc = new ScDocFunc(*this);

    //  SetBaseModel needs exception handling
    ScModelObj::CreateAndSet( this );

    StartListening(*this);
    SfxStyleSheetPool* pStlPool = aDocument.GetStyleSheetPool();
    if (pStlPool)
        StartListening(*pStlPool);
    SetHelpId( HID_SCSHELL_DOCSH );

    aDocument.GetDBCollection()->SetRefreshHandler(
        LINK( this, ScDocShell, RefreshDBDataHdl ) );

    //  InitItems und CalcOutputFactor werden jetzt nach bei Load/ConvertFrom/InitNew gerufen
}

//------------------------------------------------------------------

__EXPORT ScDocShell::~ScDocShell()
{
    ResetDrawObjectShell(); // #55570# falls der Drawing-Layer noch versucht, darauf zuzugreifen

    SfxStyleSheetPool* pStlPool = aDocument.GetStyleSheetPool();
    if (pStlPool)
        EndListening(*pStlPool);
    EndListening(*this);

    delete pAutoStyleList;

    SfxApplication *pSfxApp = SFX_APP();
    if ( pSfxApp->GetDdeService() )             // DDE vor Dokument loeschen
        pSfxApp->RemoveDdeTopic( this );

    delete pDocFunc;
    delete pUndoManager;
    delete pFontList;

    delete pPaintLockData;

    delete pOldJobSetup;        // gesetzt nur bei Fehler in StartJob()

    delete pVirtualDevice_100th_mm;

    delete pOldAutoDBRange;

    if (pModificator)
    {
        DBG_ERROR("The Modificator should not exist");
        delete pModificator;
    }
}

//------------------------------------------------------------------

SfxUndoManager* __EXPORT ScDocShell::GetUndoManager()
{
    if (!pUndoManager)
        pUndoManager = new SfxUndoManager;
    return pUndoManager;
}

void ScDocShell::SetModified( BOOL bModified )
{
    SfxInPlaceObject::SetModified( bModified );
    Broadcast( SfxSimpleHint( SFX_HINT_DOCCHANGED ) );
}


void ScDocShell::SetDocumentModified( BOOL bIsModified /* = TRUE */ )
{
    //  BroadcastUno muss auch mit pPaintLockData sofort passieren
    //! auch bei SetDrawModified, wenn Drawing angebunden ist
    //! dann eigener Hint???

    if (bIsModified)
        aDocument.BroadcastUno( SfxSimpleHint( SFX_HINT_DATACHANGED ) );

    if ( pPaintLockData && bIsModified )
    {
        pPaintLockData->SetModified();          // spaeter...
        return;
    }

    SetDrawModified( bIsModified );

    if ( bIsModified )
    {
        if ( aDocument.IsAutoCalcShellDisabled() )
            SetDocumentModifiedPending( TRUE );
        else
        {
            SetDocumentModifiedPending( FALSE );
            aDocument.InvalidateStyleSheetUsage();
            aDocument.InvalidateTableArea();
            aDocument.InvalidateLastTableOpParams();
            aDocument.Broadcast( SC_HINT_DATACHANGED, BCA_BRDCST_ALWAYS, NULL );
            if ( aDocument.IsForcedFormulaPending() && aDocument.GetAutoCalc() )
                aDocument.CalcFormulaTree( TRUE );
            PostDataChanged();

            //  Detective AutoUpdate:
            //  Update if formulas were modified (DetectiveDirty) or the list contains
            //  "Trace Error" entries (#75362# - Trace Error can look completely different
            //  after changes to non-formula cells).

            ScDetOpList* pList = aDocument.GetDetOpList();
            if ( pList && ( aDocument.IsDetectiveDirty() || pList->HasAddError() ) &&
                 pList->Count() && !IsInUndo() && SC_MOD()->GetAppOptions().GetDetectiveAuto() )
            {
                GetDocFunc().DetectiveRefresh(TRUE);    // TRUE = caused by automatic update
            }
            aDocument.SetDetectiveDirty(FALSE);         // always reset, also if not refreshed
        }
    }
}

//  SetDrawModified - ohne Formel-Update
//  (Drawing muss auch beim normalen SetDocumentModified upgedated werden,
//   z.B. bei Tabelle loeschen etc.)

void ScDocShell::SetDrawModified( BOOL bIsModified /* = TRUE */ )
{
    BOOL bUpdate = ( bIsModified != IsModified() );

    SetModified( bIsModified );

    if (bUpdate)
    {
        SfxBindings* pBindings = GetViewBindings();
        if (pBindings)
        {
            pBindings->Invalidate( SID_SAVEDOC );
            pBindings->Invalidate( SID_DOC_MODIFIED );
        }
    }

    if (bIsModified)
    {
        if ( aDocument.IsChartListenerCollectionNeedsUpdate() )
        {
            aDocument.UpdateChartListenerCollection();
            SFX_APP()->Broadcast(SfxSimpleHint( SC_HINT_DRAW_CHANGED ));    // Navigator
        }
        SC_MOD()->AnythingChanged();
    }
}

void ScDocShell::SetInUndo(BOOL bSet)
{
    bIsInUndo = bSet;
}


void ScDocShell::GetDocStat( ScDocStat& rDocStat )
{
    SfxPrinter* pPrinter = GetPrinter();

    aDocument.GetDocStat( rDocStat );
    rDocStat.nPageCount = 0;

    if ( pPrinter )
        for ( SCTAB i=0; i<rDocStat.nTableCount; i++ )
            rDocStat.nPageCount +=
                (USHORT) ScPrintFunc( this, pPrinter, i ).GetTotalPages();
}


SfxDocumentInfoDialog* __EXPORT ScDocShell::CreateDocumentInfoDialog(
                                         Window *pParent, const SfxItemSet &rSet )
{
    SfxDocumentInfoDialog* pDlg   = new SfxDocumentInfoDialog( pParent, rSet );
    ScDocShell*            pDocSh = PTR_CAST(ScDocShell,SfxObjectShell::Current());

    //nur mit Statistik, wenn dieses Doc auch angezeigt wird, nicht
    //aus dem Doc-Manager

    if( pDocSh == this )
    {
        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001
        ::CreateTabPage ScDocStatPageCreate =   pFact->GetTabPageCreatorFunc( RID_SCPAGE_STAT );
        DBG_ASSERT(ScDocStatPageCreate, "Tabpage create fail!");//CHINA001
        pDlg->AddTabPage( 42,
            ScGlobal::GetRscString( STR_DOC_STAT ),
            ScDocStatPageCreate,
            NULL);
//CHINA001      pDlg->AddTabPage( 42,
//CHINA001      ScGlobal::GetRscString( STR_DOC_STAT ),
//CHINA001      ScDocStatPage::Create,
//CHINA001      NULL );
    }
    return pDlg;
}

Window* ScDocShell::GetDialogParent()
{
    ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
    if ( pViewSh )
        return pViewSh->GetDialogParent();
    else
        return Application::GetDefDialogParent();
}

VirtualDevice* ScDocShell::GetVirtualDevice_100th_mm()
{
    if (!pVirtualDevice_100th_mm)
    {
        pVirtualDevice_100th_mm = new VirtualDevice;
        pVirtualDevice_100th_mm->SetMapMode( MAP_100TH_MM );
    }
    return pVirtualDevice_100th_mm;
}

// --- ScDocShellModificator ------------------------------------------

ScDocShellModificator::ScDocShellModificator( ScDocShell& rDS )
        :
        rDocShell( rDS ),
        aProtector( rDS.GetDocument()->GetRefreshTimerControlAddress() )
{
    ScDocument* pDoc = rDocShell.GetDocument();
    bAutoCalcShellDisabled = pDoc->IsAutoCalcShellDisabled();
    bIdleDisabled = pDoc->IsIdleDisabled();
    pDoc->SetAutoCalcShellDisabled( TRUE );
    pDoc->DisableIdle( TRUE );
}


ScDocShellModificator::~ScDocShellModificator()
{
    ScDocument* pDoc = rDocShell.GetDocument();
    pDoc->SetAutoCalcShellDisabled( bAutoCalcShellDisabled );
    if ( !bAutoCalcShellDisabled && rDocShell.IsDocumentModifiedPending() )
        rDocShell.SetDocumentModified();    // last one shuts off the lights
    pDoc->DisableIdle( bIdleDisabled );
}


void ScDocShellModificator::SetDocumentModified()
{
    ScDocument* pDoc = rDocShell.GetDocument();
    if ( !pDoc->IsImportingXML() )
    {
        // AutoCalcShellDisabled temporaer restaurieren
        BOOL bDisabled = pDoc->IsAutoCalcShellDisabled();
        pDoc->SetAutoCalcShellDisabled( bAutoCalcShellDisabled );
        rDocShell.SetDocumentModified();
        pDoc->SetAutoCalcShellDisabled( bDisabled );
    }
    else
    {
        // uno broadcast is necessary for api to work
        // -> must also be done during xml import
        pDoc->BroadcastUno( SfxSimpleHint( SFX_HINT_DATACHANGED ) );
    }
}
