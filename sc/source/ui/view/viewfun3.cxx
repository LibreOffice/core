/*************************************************************************
 *
 *  $RCSfile: viewfun3.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:10 $
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

//----------------------------------------------------------------------------

#define _SV_NOXSOUND

#ifdef WIN
    #define _MENUBTN_HXX
#endif

#define _BASE_DLGS_HXX
#define _BIGINT_HXX
#define _CACHESTR_HXX
#define _CONFIG_HXX
#define _CURSOR_HXX
#define _CTRLTOOL_HXX
#define _DLGCFG_HXX
#define _DYNARR_HXX
#define _EXTATTR_HXX
#define _FILDLG_HXX
#define _FONTDLG_HXX
#define _FRM3D_HXX
#define _INTRO_HXX
#define _ISETBWR_HXX
#define _NO_SVRTF_PARSER_HXX
#define _MACRODLG_HXX
#define _MODALDLG_HXX
#define _MOREBUTTON_HXX
#define _OUTLINER_HXX
//#define _PRNDLG_HXX
//#define _POLY_HXX
#define _PVRWIN_HXX
//#define _QUEUE_HXX
#define _RULER_HXX
#define _SCRWIN_HXX
#define _SETBRW_HXX
//#define _STACK_HXX
//#define _STATUS_HXX ***
#define _STDCTRL_HXX
#define _STDMENU_HXX
//#define _TAB_HXX
#define _TABBAR_HXX
#define _TREELIST_HXX
#define _VALUESET_HXX
#define _VCATTR_HXX
#define _VCBRW_HXX
#define _VCTRLS_HXX
#define _VCSBX_HXX
#define _VCONT_HXX
#define _VDRWOBJ_HXX

//#define _SELENG_HXX
//#define _SOUND_HXX
//#define _SYSDLG_HXX




#define _PASSWD_HXX

#define _SFX_DOCFILE_HXX
//#define _SFX_DOCFILT_HXX
#define _SFX_DOCINF_HXX
#define _SFX_DOCSH_HXX
#define _SFXDOCFILE_HXX
//#define _SFXDOCFILT_HXX
//#define _SFXDOCINF_HXX
//#define _SFXDOCSH_HXX
#define _SFX_PRNMON_HXX
#define _SFX_RESMGR_HXX
#define _SFX_TEMPLDLG_HXX
//#define _SFXAPPWIN_HXX
#define _SFXBASIC_HXX
#define _SFXCTRLITEM
#define _SFXDLGCFG_HXX
//#define _SFXDISPATCH_HXX
#define _SFXFILEDLG_HXX
//#define _SFXIMGMGR_HXX
#define _SFXIPFRM_HXX
#define _SFX_MACRO_HXX
#define _SFXMNUITEM_HXX
#define _SFXMNUMGR_HXX
#define _SFXMULTISEL_HXX
//#define _SFXMSG_HXX
#define _SFXMSGDESCR_HXX
#define _SFXMSGPOOL_HXX
#define _SFX_MINFITEM_HXX
#define _SFXOBJFACE_HXX
#define _SFXOBJFAC_HXX
#define _SFX_SAVEOPT_HXX
#define _SFXSTBITEM_HXX
#define _SFXSTBMGR_HXX
#define _SFXTBXCTRL_HXX
#define _SFXTBXMGR_HXX

#define _SI_HXX
//#define _SI_DLL_HXX
//#define _SIDLL_HXX
//#define _SI_NOITEMS
//#define _SI_NOOTHERFORMS
//#define _SI_NOSBXCONTROLS
//#define _SINOSBXCONTROLS
//#define _SI_NODRW
//#define _SI_NOCONTROL

#define _SVBOXITM_HXX
#define _SVCONTNR_HXX     //

#define _SDR_NOTRANSFORM

#define _SVDRAG_HXX
#define _SVINCVW_HXX
#define _SV_MULTISEL_HXX
#define _SVRTV_HXX
#define _SVTABBX_HXX
#define _SVTREEBOX_HXX
#define _SVTREELIST_HXX

#define _SVX_DAILDLL_HXX
#define _SVX_HYPHEN_HXX
#define _SVX_IMPGRF_HXX
#define _SVX_LAYCTRL_HXX
#define _SVX_OPTITEMS_HXX
#define _SVX_OPTGERL_HXX
#define _SVX_OPTSAVE_HXX
#define _SVX_OPTSPELL_HXX
#define _SVX_OPTPATH_HXX
#define _SVX_OPTLINGU_HXX
#define _SVX_RULER_HXX
#define _SVX_RULRITEM_HXX
#define _SVX_SELCTRL_HXX
#define _SVX_SPLWRAP_HXX
#define _SVX_SPLDLG_HXX
#define _SVX_STDDLG_HXX
#define _SVX_THESDLG_HXX

// INCLUDE -------------------------------------------------------------------

#include "scitems.hxx"
#include <svx/dbexch.hrc>
#include <svx/svdetc.hxx>
#include <sfx2/dispatch.hxx>
#include <offmgr/app.hxx>
#include <svtools/stritem.hxx>
#include <svtools/ptitem.hxx>
#include <svtools/urlbmk.hxx>
#include <so3/clsids.hxx>
#include <sot/formats.hxx>
#include <vcl/exchange.hxx>
#include <vcl/graph.hxx>
#include <vcl/virdev.hxx>
#include <vcl/msgbox.hxx>

#include "viewfunc.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"
#include "docfunc.hxx"
#include "undoblk.hxx"
#include "refundo.hxx"
#include "globstr.hrc"
#include "global.hxx"
#include "dataobj.hxx"
#include "rangenam.hxx"
#include "dbcolect.hxx"
#include "impex.hxx"            // Sylk-ID fuer CB
#include "chgtrack.hxx"
#include "waitoff.hxx"
#include "sc.hrc"

// STATIC DATA ---------------------------------------------------------------


//============================================================================

//  GlobalName der Writer-DocShell kommt jetzt aus so3/clsids.hxx

//----------------------------------------------------------------------------
//      C U T

void ScViewFunc::CutToClip( ScDocument* pClipDoc )
{
    const BOOL bRecord=TRUE;                    // Undo/Redo

    UpdateInputLine();

    if (!SelectionEditable())                   // Bereich editierbar?
    {
        ErrorMessage( STR_PROTECTIONERR );
        return;
    }

    ScRange aRange;                             // zu loeschender Bereich
    if ( GetViewData()->GetSimpleArea( aRange ) )
    {
        ScDocument* pDoc = GetViewData()->GetDocument();
        ScDocShell* pDocSh = GetViewData()->GetDocShell();
        ScMarkData& rMark = GetViewData()->GetMarkData();

        ScDocShellModificator aModificator( *pDocSh );

        if (!rMark.IsMarked())                  // Markierung setzen, falls nicht vorhanden
        {
            DoneBlockMode();
            InitOwnBlockMode();
            rMark.SetMarkArea( aRange );
        }

        CopyToClip( pClipDoc, TRUE );           // Ab ins Clipboard

        ScAddress aOldEnd( aRange.aEnd );       // Zusammengefasste Zellen im Bereich?
        pDoc->ExtendMerge( aRange, TRUE );

        ScDocument* pUndoDoc = NULL;
        if ( bRecord )
        {
            pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( pDoc, aRange.aStart.Tab(), aRange.aEnd.Tab() );
            pDoc->CopyToDocument( aRange, IDF_ALL, FALSE, pUndoDoc );
            pDoc->BeginDrawUndo();
        }

        USHORT nExtFlags = 0;
        if ( pDoc->HasAttrib( aRange, HASATTR_PAINTEXT ) )
            nExtFlags |= SC_PF_LINES;

        HideCursor();                           // Cursor aendert sich !

        rMark.MarkToMulti();
        pDoc->DeleteSelection( IDF_ALL, rMark );
        rMark.MarkToSimple();

        if ( !AdjustRowHeight( aRange.aStart.Row(), aRange.aEnd.Row() ) )
            pDocSh->PostPaint( aRange, PAINT_GRID, nExtFlags );

        if ( bRecord )                          // erst jetzt ist Draw-Undo verfuegbar
            pDocSh->GetUndoManager()->AddUndoAction(
                new ScUndoCut( pDocSh, aRange, aOldEnd, pUndoDoc ) );

        aModificator.SetDocumentModified();
        ShowCursor();                           // Cursor aendert sich !
        pDocSh->UpdateOle(GetViewData());

        CellContentChanged();
    }
    else
        ErrorMessage( STR_NOMULTISELECT );
}


//----------------------------------------------------------------------------
//      C O P Y

void ScViewFunc::CopyToClip( ScDocument* pClipDoc, BOOL bCut )
{
    UpdateInputLine();

    BOOL bSysClip = FALSE;
    if ( !pClipDoc )                            // kein eigenes Clipboard
    {
        pClipDoc = ScGlobal::GetClipDoc();      // ab ins System!
        bSysClip = TRUE;
    }

    ScRange aRange;
    if ( GetViewData()->GetSimpleArea( aRange ) )
    {
        ScDocument* pDoc = GetViewData()->GetDocument();
        ScMarkData& rMark = GetViewData()->GetMarkData();
        if ( !pDoc->HasSelectedBlockMatrixFragment(
/*!*/                   aRange.aStart.Col(), aRange.aStart.Row(),
                        aRange.aEnd.Col(),   aRange.aEnd.Row(),
                        rMark ) )
        {
            if (bSysClip)
            {
//?             Clipboard::Clear();         // loescht ggf. altes ScDataObject und gibt ClipDoc frei

                SvDataObjectRef pDummyObj = new SvDataObject;
                pDummyObj->CopyClipboard();
            }

            if ( !bCut )
            {
                ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
                if ( pChangeTrack )
                    pChangeTrack->ResetLastCut();   // kein CutMode mehr
            }

            pDoc->CopyToClip( aRange.aStart.Col(), aRange.aStart.Row(),
/*!*/                         aRange.aEnd.Col(),   aRange.aEnd.Row(),
                              bCut, pClipDoc, FALSE, &rMark );
            ScGlobal::SetClipDocName( pDoc->GetDocumentShell()->GetTitle( SFX_TITLE_FULLNAME ) );
            pClipDoc->ExtendMerge( aRange, TRUE );

            if (bSysClip)
            {
                SvDataObjectRef pClipObj = new ScDataObject(pClipDoc,FALSE,
                                                            GetViewData()->GetDocShell(),
                                                            &GetViewData()->GetOptions());
                pClipObj->CopyClipboard();
            }
        }
        else
            ErrorMessage(STR_MATRIXFRAGMENTERR);
    }
    else
        ErrorMessage(STR_NOMULTISELECT);
}


//----------------------------------------------------------------------------
//      P A S T E

void ScViewFunc::PasteDraw()
{
    ScViewData* pViewData = GetViewData();
    USHORT nPosX = pViewData->GetCurX();
    USHORT nPosY = pViewData->GetCurY();
    Point aPos = GetActiveWin()->PixelToLogic( pViewData->GetScrPos( nPosX, nPosY,
                                                pViewData->GetActivePart() ) );
    PasteDraw( aPos, ScGlobal::GetClipModel() );
}

void ScViewFunc::PasteFromSystem()
{
    UpdateInputLine();

    if (ScGlobal::IsClipCaptured())
        PasteFromClip( IDF_ALL );
    else if (ScGlobal::IsClipDraw())
        PasteDraw();
    else
    {
        SvDataObjectRef pClipObj = SvDataObject::PasteClipboard();
        if (pClipObj.Is())
        {
            ULONG nBiff = Exchange::RegisterFormatName(
                    String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Biff5")));

                //  als erstes SvDraw-Model, dann Grafik
                //  (Grafik darf nur bei einzelner Grafik drinstehen)

            if (pClipObj->HasFormat( SOT_FORMATSTR_ID_DRAWING ))
                PasteFromSystem( SOT_FORMATSTR_ID_DRAWING );
            else if (pClipObj->HasFormat( SOT_FORMATSTR_ID_SVXB ))
                PasteFromSystem( SOT_FORMATSTR_ID_SVXB );
            else if (pClipObj->HasFormat( SOT_FORMATSTR_ID_EMBED_SOURCE ))
            {
                //  Wenn es vom Writer kommt, statt OLE RTF einfuegen

                SvObjectDescriptor aDesc( pClipObj );
                //  GlobalName vom Writer wie da in docsh.cxx
                SvGlobalName aWriterName( SO3_SW_CLASSID );
                SvGlobalName aSwWebName( SO3_SWWEB_CLASSID );
                if ((aDesc.GetClassName() == aWriterName || aDesc.GetClassName() == aSwWebName) &&
                        pClipObj->HasFormat(FORMAT_RTF))
                    PasteFromSystem( FORMAT_RTF );
                else
                    PasteFromSystem( SOT_FORMATSTR_ID_EMBED_SOURCE );
            }
            else if (pClipObj->HasFormat( SOT_FORMATSTR_ID_LINK_SOURCE ))
                PasteFromSystem( SOT_FORMATSTR_ID_LINK_SOURCE );
            else if (pClipObj->HasFormat(FORMAT_PRIVATE))
                PasteFromClip( IDF_ALL );
            else if (pClipObj->HasFormat(nBiff))        // before xxx_OLE formats
                PasteFromSystem(nBiff);
            else if (pClipObj->HasFormat( SOT_FORMATSTR_ID_EMBED_SOURCE_OLE ))
                PasteFromSystem( SOT_FORMATSTR_ID_EMBED_SOURCE_OLE );
            else if (pClipObj->HasFormat( SOT_FORMATSTR_ID_LINK_SOURCE_OLE ))
                PasteFromSystem( SOT_FORMATSTR_ID_LINK_SOURCE_OLE );
            else if (pClipObj->HasFormat(FORMAT_RTF))
                PasteFromSystem(FORMAT_RTF);
            else if (pClipObj->HasFormat(SOT_FORMATSTR_ID_HTML))
                PasteFromSystem(SOT_FORMATSTR_ID_HTML);
            else if (pClipObj->HasFormat(SOT_FORMATSTR_ID_HTML_SIMPLE))
                PasteFromSystem(SOT_FORMATSTR_ID_HTML_SIMPLE);
            else if (pClipObj->HasFormat(SOT_FORMATSTR_ID_SYLK))
                PasteFromSystem(SOT_FORMATSTR_ID_SYLK);
            else if (pClipObj->HasFormat(FORMAT_STRING))
                PasteFromSystem(FORMAT_STRING);
            else if (pClipObj->HasFormat(FORMAT_GDIMETAFILE))
                PasteFromSystem(FORMAT_GDIMETAFILE);
            else if (pClipObj->HasFormat(FORMAT_BITMAP))
                PasteFromSystem(FORMAT_BITMAP);
//          else
//              ErrorMessage(STR_PASTE_ERROR);
        }
//      else
//          ErrorMessage(STR_PASTE_ERROR);
    }

    //  keine Fehlermeldung, weil SID_PASTE in der idl das FastCall-Flag hat,
    //  also auch gerufen wird, wenn nichts im Clipboard steht (#42531#)
}

BOOL ScViewFunc::PasteFromSystem( ULONG nFormatId, BOOL bApi )
{
    UpdateInputLine();

    BOOL bRet = TRUE;
    if ( nFormatId == 0 && ScGlobal::IsClipCaptured() )
        PasteFromClip( IDF_ALL );
    else
    {
        SvDataObjectRef pClipObj = SvDataObject::PasteClipboard();
        if (!pClipObj.Is())
            return FALSE;

        bRet = PasteDataFormat( nFormatId, pClipObj,
                                GetViewData()->GetCurX(), GetViewData()->GetCurY() );

        if ( !bRet && !bApi )
            ErrorMessage(STR_PASTE_ERROR);
    }
    return bRet;
}


//----------------------------------------------------------------------------
//      P A S T E

//  Drag & Drop
BOOL ScViewFunc::PasteDataObject( SvDataObject* pObject, USHORT nPosX, USHORT nPosY,
                                    Window* pWin, Point* pLogicPos )
{
    ULONG nFormatId = 0;

    ULONG nSba     = SOT_FORMATSTR_ID_SBA_DATAEXCHANGE;
    ULONG nField   = Exchange::RegisterFormatName(
                        String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(SBA_FIELDEXCHANGE_FORMAT)));
    ULONG nDDE     = SOT_FORMATSTR_ID_LINK;
    ULONG nBiff    = Exchange::RegisterFormatName(
                        String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Biff5")));

    //  eine DB-Tabelle aus dem Explorer hat Sba und Bookmark -> Bookmark nur wenn nicht Sba

    if ( INetBookmark::HasFormat(*pObject) && !pObject->HasFormat(nSba) )
        if (PasteBookmark( pObject, nPosX, nPosY ))
            return TRUE;

        //  als erstes SvDraw-Model, dann Grafik (darf nur bei einzelner Grafik drinstehen)

    if (pObject->HasFormat(SOT_FORMATSTR_ID_DRAWING))
        nFormatId = SOT_FORMATSTR_ID_DRAWING;
    else if (pObject->HasFormat(SOT_FORMATSTR_ID_SVXB))
        nFormatId = SOT_FORMATSTR_ID_SVXB;
    else if (pObject->HasFormat(SOT_FORMATSTR_ID_EMBED_SOURCE))
    {
        //  Wenn es vom Writer kommt, statt OLE RTF einfuegen

        SvObjectDescriptor aDesc( pObject );
        //  GlobalName vom Writer wie da in docsh.cxx
        SvGlobalName aWriterName( SO3_SW_CLASSID );
        SvGlobalName aSwWebName( SO3_SWWEB_CLASSID );
        if ((aDesc.GetClassName() == aWriterName || aDesc.GetClassName() == aSwWebName) &&
                pObject->HasFormat(FORMAT_RTF))
            nFormatId = FORMAT_RTF;
        else
            nFormatId = SOT_FORMATSTR_ID_EMBED_SOURCE;
    }
    else if (pObject->HasFormat(SOT_FORMATSTR_ID_LINK_SOURCE))
        nFormatId = SOT_FORMATSTR_ID_LINK_SOURCE;
    else if (pObject->HasFormat(nSba))      // Sba muss vor RTF kommen, damit bei Daten aus dem
        nFormatId = nSba;                   // Beamer importiert wird (RTF kopiert nur die Inhalte)
    else if (pObject->HasFormat(nField))
        nFormatId = nField;
    else if (pObject->HasFormat(nBiff))     // before xxx_OLE formats
        nFormatId = nBiff;
    else if (pObject->HasFormat(SOT_FORMATSTR_ID_EMBED_SOURCE_OLE))
        nFormatId = SOT_FORMATSTR_ID_EMBED_SOURCE_OLE;
    else if (pObject->HasFormat(SOT_FORMATSTR_ID_LINK_SOURCE_OLE))
        nFormatId = SOT_FORMATSTR_ID_LINK_SOURCE_OLE;
    else if (pObject->HasFormat(FORMAT_RTF))
        nFormatId = FORMAT_RTF;
    else if (pObject->HasFormat(SOT_FORMATSTR_ID_HTML))
        nFormatId = SOT_FORMATSTR_ID_HTML;
    else if (pObject->HasFormat(SOT_FORMATSTR_ID_HTML_SIMPLE))
        nFormatId = SOT_FORMATSTR_ID_HTML_SIMPLE;
    else if (pObject->HasFormat(SOT_FORMATSTR_ID_SYLK))
        nFormatId = SOT_FORMATSTR_ID_SYLK;
    else if (pObject->HasFormat(nDDE))
        nFormatId = nDDE;
    else if (pObject->HasFormat(FORMAT_STRING))
        nFormatId = FORMAT_STRING;
    else if (pObject->HasFormat(FORMAT_GDIMETAFILE))
        nFormatId = FORMAT_GDIMETAFILE;
    else if (pObject->HasFormat(FORMAT_BITMAP))
        nFormatId = FORMAT_BITMAP;
    else if (pObject->HasFormat(FORMAT_FILE))
        nFormatId = FORMAT_FILE;

    if (nFormatId)
        return PasteDataFormat( nFormatId, pObject, nPosX, nPosY, pWin, pLogicPos );
    else
        return FALSE;
}

//  Drag & Drop als Link
BOOL ScViewFunc::LinkDataObject( SvDataObject* pObject, USHORT nPosX, USHORT nPosY,
                                    Window* pWin, Point* pLogicPos )
{
    ULONG nFormatId = 0;

    //  Links: nur OLE-Link / DDE
    //! bei OLE-Link auch Unterscheidung, ob's vom Writer kommt ???

    ULONG nDDE     = SOT_FORMATSTR_ID_LINK;

    if (pObject->HasFormat( SOT_FORMATSTR_ID_LINK_SOURCE ))
        nFormatId = SOT_FORMATSTR_ID_LINK_SOURCE;
    else if (pObject->HasFormat( SOT_FORMATSTR_ID_LINK_SOURCE_OLE ))
        nFormatId = SOT_FORMATSTR_ID_LINK_SOURCE_OLE;
    else if (pObject->HasFormat(nDDE))
        nFormatId = nDDE;

    if (nFormatId)
        return PasteDataFormat( nFormatId, pObject, nPosX, nPosY, pWin, pLogicPos );

    if (pObject->HasFormat(FORMAT_FILE))
    {
        SvData aData( FORMAT_FILE );
        if (pObject->GetData( &aData ))
        {
            //  Datei als Link einfuegen

            String aFile;
            aData.GetData( aFile );
            SfxStringItem aNameItem( FID_INSERT_FILE, aFile );
            Point aPos;
            if (pLogicPos) aPos = *pLogicPos;
            SfxPointItem aPosItem( FN_PARAM_1, aPos );
            SfxBoolItem aLinkItem( FN_PARAM_2, TRUE );
            SfxDispatcher* pDisp = GetViewData()->GetViewShell()->GetViewFrame()->GetDispatcher();
            if (pDisp)
                pDisp->Execute( FID_INSERT_FILE, SFX_CALLMODE_ASYNCHRON,
                                    &aNameItem, &aPosItem, &aLinkItem, 0L );
            return TRUE;
        }
    }

    //  #66028# Internet Explorer draggt URLs als Link
    if ( INetBookmark::HasFormat(*pObject) )
        if (PasteBookmark( pObject, nPosX, nPosY ))
            return TRUE;

    return FALSE;
}

BOOL ScViewFunc::PasteOnDrawObject( SvDataObject* pObject, SdrObject* pHitObj, BOOL bLink )
{
    BOOL bRet = FALSE;
    if ( bLink )
    {
        if ( pObject->HasFormat(SOT_FORMATSTR_ID_SVXB) )
        {
            SvData aData(SOT_FORMATSTR_ID_SVXB);
            if ( pObject->GetData( &aData ) )
            {
                Graphic* pGraphic = NULL;
                if ( aData.GetData( (SvDataCopyStream**) &pGraphic,
                                    Graphic::StaticType(), TRANSFER_MOVE ) )
                {
                    bRet = ApplyGraphicToObject( pHitObj, *pGraphic );
                }
                delete pGraphic;
            }
        }
        else if ( pObject->HasFormat(FORMAT_GDIMETAFILE) )
        {
            SvData aData(FORMAT_GDIMETAFILE);
            if ( pObject->GetData( &aData ) )
            {
                GDIMetaFile* pMtf = NULL;
                if (aData.GetData(&pMtf, TRANSFER_MOVE))
                {
                    bRet = ApplyGraphicToObject( pHitObj, Graphic(*pMtf) );
                }
                delete pMtf;
            }
        }
        else if ( pObject->HasFormat(FORMAT_BITMAP) )
        {
            SvData aData(FORMAT_BITMAP);
            if ( pObject->GetData( &aData ) )
            {
                Bitmap* pBmp = NULL;
                if (aData.GetData(&pBmp, TRANSFER_MOVE))
                {
                    bRet = ApplyGraphicToObject( pHitObj, Graphic(*pBmp) );
                }
                delete pBmp;
            }
        }
    }
    else
    {
        //  ham' wa noch nich
    }
    return bRet;
}

//
//      Einfuegen auf Tabelle:
//

//  internes Paste

BOOL ScViewFunc::PasteFromClip( USHORT nFlags, ScDocument* pClipDoc,
                                    USHORT nFunction, BOOL bSkipEmpty,
                                    BOOL bTranspose, BOOL bAsLink,
                                    InsCellCmd eMoveMode, USHORT nUndoExtraFlags )
{
    if (!pClipDoc)
        pClipDoc = ScGlobal::GetClipDoc();

    //  fuer Undo etc. immer alle oder keine Inhalte sichern
    USHORT nContFlags = IDF_NONE;
    if (nFlags & IDF_CONTENTS)
        nContFlags |= IDF_CONTENTS;
    if (nFlags & IDF_ATTRIB)
        nContFlags |= IDF_ATTRIB;
    // evtl. Attribute ins Undo ohne sie vom Clip ins Doc zu kopieren
    USHORT nUndoFlags = nContFlags;
    if (nUndoExtraFlags & IDF_ATTRIB)
        nUndoFlags |= IDF_ATTRIB;

    BOOL bCutMode = pClipDoc->IsCutMode();      // bei Transpose aus dem Original-Clipdoc

    ScDocument* pOrigClipDoc = NULL;
    ScDocument* pTransClip = NULL;
    if ( bTranspose )
    {
        USHORT nX,nY;
        pClipDoc->GetClipArea( nX, nY );
        if ( nY > MAXCOL )                      // zuviele Zeilen zum Transponieren
        {
            ErrorMessage(STR_PASTE_FULL);
            return FALSE;
        }
        pOrigClipDoc = pClipDoc;        // fuer Referenzen

        pTransClip = new ScDocument( SCDOCMODE_CLIP );
        pClipDoc->TransposeClip( pTransClip, nFlags, bAsLink );
        pClipDoc = pTransClip;
    }

    const BOOL bRecord = TRUE;

    USHORT nStartCol;
    USHORT nStartRow;
    USHORT nStartTab;
    USHORT nEndCol;
    USHORT nEndRow;
    USHORT nEndTab;
    USHORT nClipSizeX;
    USHORT nClipSizeY;
    pClipDoc->GetClipArea( nClipSizeX, nClipSizeY );

    ScDocument* pDoc = GetViewData()->GetDocument();
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScMarkData& rMark = GetViewData()->GetMarkData();
    SfxUndoManager* pUndoMgr = pDocSh->GetUndoManager();

    ScDocShellModificator aModificator( *pDocSh );

    if ( rMark.IsMultiMarked() )
    {
        rMark.MarkToSimple();
        if ( rMark.IsMultiMarked() )
        {       // "Einfuegen auf Mehrfachselektion nicht moeglich"
            ErrorMessage(STR_MSSG_PASTEFROMCLIP_0);
            delete pTransClip;
            return FALSE;
        }
    }

    USHORT nMarkAddX = 0;
    USHORT nMarkAddY = 0;

    if ( rMark.IsMarked() )
    {
        ScRange aMarkRange;
        rMark.GetMarkArea( aMarkRange );
        nStartCol = aMarkRange.aStart.Col();
        nStartRow = aMarkRange.aStart.Row();
        nStartTab = aMarkRange.aStart.Tab();
        nEndCol = aMarkRange.aEnd.Col();
        nEndRow = aMarkRange.aEnd.Row();
        nEndTab = aMarkRange.aEnd.Tab();
        USHORT nBlockAddX = nEndCol-nStartCol;
        USHORT nBlockAddY = nEndRow-nStartRow;

        //  #58422# Nachfrage, wenn die Selektion groesser als 1 Zeile/Spalte, aber kleiner
        //  als das Clipboard ist (dann wird ueber die Selektion hinaus eingefuegt)

        //  ClipSize ist nicht Groesse, sondern Differenz
        if ( ( nBlockAddX && nBlockAddX < nClipSizeX ) ||
             ( nBlockAddY && nBlockAddY < nClipSizeY ) )
        {
            ScWaitCursorOff aWaitOff( GetFrameWin() );
            String aMessage = ScGlobal::GetRscString( STR_PASTE_BIGGER );
            QueryBox aBox( GetViewData()->GetDialogParent(),
                            WinBits(WB_YES_NO | WB_DEF_NO), aMessage );
            if ( aBox.Execute() != RET_YES )
            {
                delete pTransClip;
                return FALSE;
            }
        }

        if (nBlockAddX > nClipSizeX)
            nMarkAddX = nBlockAddX - nClipSizeX;            // fuer Merge-Test
        else
            nEndCol = nStartCol + nClipSizeX;

        if (nBlockAddY > nClipSizeY)
            nMarkAddY = nBlockAddY - nClipSizeY;            // fuer Merge-Test
        else
            nEndRow = nStartRow + nClipSizeY;
    }
    else
    {
        nStartCol = GetViewData()->GetCurX();
        nStartRow = GetViewData()->GetCurY();
        nStartTab = GetViewData()->GetTabNo();
        nEndCol = nStartCol + nClipSizeX;
        nEndRow = nStartRow + nClipSizeY;
        nEndTab = nStartTab;
    }

    //  Zielbereich, wie er angezeigt wird:
    ScRange aUserRange( nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab );

    //  Sollen Zellen eingefuegt werden?
    //  (zu grosse nEndCol/nEndRow werden weiter unten erkannt)
    BOOL bInsertCells = ( eMoveMode != INS_NONE && nEndCol <= MAXCOL && nEndRow <= MAXROW );
    if ( bInsertCells )
    {
        String aUndo = ScGlobal::GetRscString( STR_UNDO_PASTE );
        pUndoMgr->EnterListAction( aUndo, aUndo );

        MarkRange( aUserRange );            // wird vor CopyFromClip sowieso gesetzt

        // #72930# CutMode is reset on insertion of cols/rows but needed again on cell move
        BOOL bCut = pClipDoc->IsCutMode();
        if (!InsertCells( eMoveMode ))      // kann eingefuegt werden?
        {
            delete pTransClip;              // abbrechen
            pUndoMgr->LeaveListAction();
            return FALSE;
        }
        if ( bCut )
            pClipDoc->SetCutMode( bCut );
    }

    USHORT nClipStartX;                         // Clipboard-Bereich erweitern
    USHORT nClipStartY;
    pClipDoc->GetClipStart( nClipStartX, nClipStartY );
    USHORT nUndoEndCol = nClipStartX + nClipSizeX;
    USHORT nUndoEndRow = nClipStartY + nClipSizeY;
    BOOL bClipOver = pClipDoc->
        ExtendMerge( nClipStartX,nClipStartY, nUndoEndCol,nUndoEndRow, nStartTab, FALSE );
    nUndoEndCol -= nClipStartX + nClipSizeX;
    nUndoEndRow -= nClipStartY + nClipSizeY;
    nUndoEndCol += nEndCol;
    nUndoEndRow += nEndRow;                         // Bereich in Zielkoordinaten

//  if (nUndoEndCol < nEndCol) nUndoEndCol = nEndCol;
//  if (nUndoEndRow < nEndRow) nUndoEndRow = nEndRow;

//  nUndoEndCol += nMarkAddX;
//  nUndoEndRow += nMarkAddY;

    if (nUndoEndCol>MAXCOL || nUndoEndRow>MAXROW)
    {
        ErrorMessage(STR_PASTE_FULL);
        delete pTransClip;
        if ( bInsertCells )
            pUndoMgr->LeaveListAction();
        return FALSE;
    }

    pDoc->ExtendMerge( nStartCol,nStartRow, nUndoEndCol,nUndoEndRow, nStartTab, FALSE );

        //  Test auf Zellschutz

    if (!pDoc->IsBlockEditable( nStartTab, nStartCol,nStartRow, nUndoEndCol,nUndoEndRow ))
    {
        ErrorMessage(STR_PROTECTIONERR);
        delete pTransClip;
        if ( bInsertCells )
            pUndoMgr->LeaveListAction();
        return FALSE;
    }

        //! Test auf Ueberlappung
        //! nur wirkliche Schnittmenge testen !!!!!!!

    //  pDoc->HasCommonAttr( StartCol,nStartRow, nUndoEndCol,nUndoEndRow, nStartTab,
    //                          pClipDoc, nClipStartX, nClipStartY );

    if (bClipOver)
        if (pDoc->HasAttrib( nStartCol,nStartRow,nStartTab, nUndoEndCol,nUndoEndRow,nStartTab,
                                HASATTR_OVERLAPPED ))
        {       // "Zusammenfassen nicht verschachteln !"
            ErrorMessage(STR_MSSG_PASTEFROMCLIP_1);
            delete pTransClip;
            if ( bInsertCells )
                pUndoMgr->LeaveListAction();
            return FALSE;
        }

    if ( !bCutMode )
    {
        ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
        if ( pChangeTrack )
            pChangeTrack->ResetLastCut();   // kein CutMode mehr
    }

    BOOL bColInfo = ( nStartRow==0 && nEndRow==MAXROW );
    BOOL bRowInfo = ( nStartCol==0 && nEndCol==MAXCOL );

    ScDocument* pUndoDoc    = NULL;
    ScDocument* pRefUndoDoc = NULL;
    ScDocument* pRedoDoc    = NULL;
    ScRangeName* pUndoRange = NULL;
    ScRangeName* pRedoRange = NULL;
    ScDBCollection* pUndoDB = NULL;
    ScDBCollection* pRedoDB = NULL;

    if ( bRecord )
    {
        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        pUndoDoc->InitUndo( pDoc, nStartTab, nEndTab, bColInfo, bRowInfo );
        pDoc->CopyToDocument( nStartCol, nStartRow, nStartTab, nUndoEndCol, nUndoEndRow, nEndTab,
                                nUndoFlags, FALSE, pUndoDoc );

        if ( bCutMode )
        {
            pRefUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pRefUndoDoc->InitUndo( pDoc, 0, pDoc->GetTableCount()-1, FALSE, FALSE );

            ScRangeName* pDocRange = pDoc->GetRangeName();
            if (pDocRange->GetCount())
                pUndoRange = new ScRangeName( *pDocRange );
            ScDBCollection* pDocDB = pDoc->GetDBCollection();
            if (pDocDB->GetCount())
                pUndoDB = new ScDBCollection( *pDocDB );
        }
    }

    //  waren vorher Linien da?
    USHORT nExtFlags = 0;
    if (pDoc->HasAttrib( nStartCol,nStartRow,nStartTab, nEndCol,nEndRow,nEndTab, HASATTR_PAINTEXT ))
        nExtFlags |= SC_PF_LINES;

    if (GetViewData()->IsActive())
    {
        DoneBlockMode();
        InitOwnBlockMode();
    }
    rMark.SetMarkArea( aUserRange );

    HideCursor();                           // Cursor aendert sich !

        //
        //  Aus Clipboard kopieren,
        //  wenn gerechnet werden soll, Originaldaten merken
        //

    ScDocument* pMixDoc = NULL;
    if ( bSkipEmpty || nFunction )
    {
        if ( nFlags & IDF_CONTENTS )
        {
            pMixDoc = new ScDocument( SCDOCMODE_UNDO );
            pMixDoc->InitUndo( pDoc, nStartTab, nEndTab );
            pDoc->CopyToDocument( nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab,
                                    IDF_CONTENTS, FALSE, pMixDoc );
        }
    }

    if (!bAsLink)
    {
        //  normal kopieren
        pDoc->CopyFromClip( aUserRange, rMark, nFlags, pRefUndoDoc, pClipDoc ); // Original-Bereich

        // bei Transpose Referenzen per Hand anpassen
        if ( bTranspose && bCutMode && (nFlags & IDF_CONTENTS) )
            pDoc->UpdateTranspose( aUserRange.aStart, pOrigClipDoc, rMark, pRefUndoDoc );
    }
    else if (!bTranspose)
    {
        //  kopieren mit bAsLink=TRUE
        pDoc->CopyFromClip( aUserRange, rMark, nFlags, pRefUndoDoc, pClipDoc, TRUE, TRUE );
    }
    else
    {
        //  alle Inhalte kopieren (im TransClipDoc stehen nur Formeln)
        pDoc->CopyFromClip( aUserRange, rMark, nContFlags, pRefUndoDoc, pClipDoc );
    }
    pDoc->ExtendMerge( nStartCol, nStartRow, nEndCol, nEndRow, nStartTab, TRUE );   // Refresh
                                                                                    // und Bereich neu

    if ( pMixDoc )              // Rechenfunktionen mit Original-Daten auszufuehren ?
    {
        pDoc->MixDocument( aUserRange, nFunction, bSkipEmpty, pMixDoc );
    }
    delete pMixDoc;

    //
    //
    //

    //  sind hinterher Linien da?
    if (!nExtFlags & SC_PF_LINES)
        if (pDoc->HasAttrib( nStartCol,nStartRow,nStartTab, nEndCol,nEndRow,nEndTab, HASATTR_PAINTEXT ))
            nExtFlags |= SC_PF_LINES;

        //  ggf. Autofilter-Koepfe loeschen
    if (bCutMode)
        if (pDoc->RefreshAutoFilter( nClipStartX,nClipStartY, nClipStartX+nClipSizeX,
                                        nClipStartY+nClipSizeY, nStartTab ))
            pDocSh->PostPaint( nClipStartX,nClipStartY,nStartTab,
                                nClipStartX+nClipSizeX,nClipStartY,nStartTab,
                                PAINT_GRID );

    ShowCursor();                           // Cursor aendert sich !

    //!     Block-Bereich bei RefUndoDoc weglassen !!!

    if ( bRecord )
    {
        // Redo-Daten werden erst beim ersten Undo kopiert
        // ohne RefUndoDoc muss das Redo-Doc noch nicht angelegt werden

        if (pRefUndoDoc)
        {
            pRedoDoc = new ScDocument( SCDOCMODE_UNDO );
            pRedoDoc->InitUndo( pDoc, nStartTab, nEndTab, bColInfo, bRowInfo );

            //      angepasste Referenzen ins Redo-Doc

            USHORT nTabCount = pDoc->GetTableCount();
            pRedoDoc->AddUndoTab( 0, nTabCount-1 );
            pDoc->CopyUpdated( pRefUndoDoc, pRedoDoc );

            //      alte Referenzen ins Undo-Doc

            //! Tabellen selektieren ?
            pUndoDoc->AddUndoTab( 0, nTabCount-1 );
            pRefUndoDoc->DeleteArea( nStartCol, nStartRow, nEndCol, nEndRow, rMark, IDF_ALL );
            pRefUndoDoc->CopyToDocument( 0,0,0, MAXCOL,MAXROW,nTabCount-1,
                                            IDF_FORMULA, FALSE, pUndoDoc );
            delete pRefUndoDoc;
        }

        if ( bCutMode )
        {
            if ( pUndoRange )
                pRedoRange = new ScRangeName( *pDoc->GetRangeName() );
            if ( pUndoDB )
                pRedoDB = new ScDBCollection( *pDoc->GetDBCollection() );
        }

        //!     testen, ob Bereich wirklich geaendert

        pUndoMgr->AddUndoAction(
            new ScUndoPaste( pDocSh,
                                nStartCol, nStartRow, nStartTab,
                                nUndoEndCol, nUndoEndRow, nEndTab, rMark,
                                pUndoDoc, pRedoDoc, nFlags | nUndoFlags,
                                pUndoRange, pRedoRange, pUndoDB, pRedoDB,
                                FALSE ) );  // FALSE = Redo-Daten sind nicht kopiert
    }

    if ( bInsertCells )
        pUndoMgr->LeaveListAction();

    USHORT nPaint = PAINT_GRID;
    if (bColInfo)
    {
        nPaint |= PAINT_TOP;
        nUndoEndCol = MAXCOL;               // nur zum Zeichnen !
    }
    if (bRowInfo)
    {
        nPaint |= PAINT_LEFT;
        nUndoEndRow = MAXROW;               // nur zum Zeichnen !
    }
    pDocSh->PostPaint( nStartCol, nStartRow, nStartTab,
                        nUndoEndCol, nUndoEndRow, nEndTab, nPaint, nExtFlags );
    AdjustBlockHeight();                    //! Paint/Undo ?
    aModificator.SetDocumentModified();
    pDocSh->UpdateOle(GetViewData());

    SelectionChanged();

    delete pTransClip;
    return TRUE;
}


//----------------------------------------------------------------------------
//      D R A G   A N D   D R O P
//
//  innerhalb des Dokuments

void ScViewFunc::MoveBlockTo( const ScRange& rSource, const ScAddress& rDestPos,
                                BOOL bCut, BOOL bRecord, BOOL bPaint )
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    HideAllCursors();       // wegen zusammengefassten
    BOOL bSuccess = pDocSh->GetDocFunc().MoveBlock(
                                rSource, rDestPos, bCut, bRecord, bPaint, FALSE );
    ShowAllCursors();
    if (bSuccess)
    {
        //  Zielbereich markieren
        ScAddress aDestEnd(
                    rDestPos.Col() + rSource.aEnd.Col() - rSource.aStart.Col(),
                    rDestPos.Row() + rSource.aEnd.Row() - rSource.aStart.Row(),
                    rDestPos.Tab() );
        MarkRange( ScRange( rDestPos, aDestEnd ), FALSE );          //! FALSE ???

        pDocSh->UpdateOle(GetViewData());
        SelectionChanged();
    }
}

//  Link innerhalb des Dokuments

void ScViewFunc::LinkBlock( const ScRange& rSource, const ScAddress& rDestPos )
{
    //  Test auf Ueberlappung

    if ( rSource.aStart.Tab() == rDestPos.Tab() )
    {
        USHORT nDestEndCol = rDestPos.Col() + ( rSource.aEnd.Col() - rSource.aStart.Col() );
        USHORT nDestEndRow = rDestPos.Row() + ( rSource.aEnd.Row() - rSource.aStart.Row() );

        if ( rSource.aStart.Col() <= nDestEndCol && rDestPos.Col() <= rSource.aEnd.Col() &&
             rSource.aStart.Row() <= nDestEndRow && rDestPos.Row() <= rSource.aEnd.Row() )
        {
            ErrorMessage( STR_ERR_LINKOVERLAP );
            return;
        }
    }

    //  Ausfuehren per Paste

    ScDocument* pDoc = GetViewData()->GetDocument();
    ScDocument* pClipDoc = new ScDocument( SCDOCMODE_CLIP );
    pDoc->CopyTabToClip( rSource.aStart.Col(), rSource.aStart.Row(),
                            rSource.aEnd.Col(), rSource.aEnd.Row(),
                            rSource.aStart.Tab(), pClipDoc );

    //  Zielbereich markieren (Cursor setzen, keine Markierung)

    if ( GetViewData()->GetTabNo() != rDestPos.Tab() )
        SetTabNo( rDestPos.Tab() );

    MoveCursorAbs( rDestPos.Col(), rDestPos.Row(), SC_FOLLOW_NONE, FALSE, FALSE );

    //  Paste

    PasteFromClip( IDF_ALL, pClipDoc, PASTE_NOFUNC, FALSE, FALSE, TRUE );       // als Link

    delete pClipDoc;
}




