/*************************************************************************
 *
 *  $RCSfile: view.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:49 $
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

#include <string> // HACK: prevent conflict between STLPORT and Workshop headers

#include <stdlib.h>
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SV_GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
#ifndef _VCL_INPUTCTX_HXX //autogen
#include <vcl/inputctx.hxx>
#endif
#ifndef _SBX_SBXOBJECT_HXX //autogen
#include <svtools/sbxobj.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFX_SAVEOPT_HXX //autogen
#include <sfx2/saveopt.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SVX_RULER_HXX //autogen
#include <svx/ruler.hxx>
#endif
#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif
#ifndef _SBASLTID_HRC //autogen
#include <offmgr/sbasltid.hrc>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _OFA_OSPLCFG_HXX
#include <offmgr/osplcfg.hxx>
#endif

#ifndef _SVX_FMSHELL_HXX //autogen
#include <svx/fmshell.hxx>
#endif
#ifndef _UNOTXVW_HXX
#include <unotxvw.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _SWHINTS_HXX
#include <swhints.hxx>
#endif

#ifndef _SWMODULE_HXX //autogen
#include <swmodule.hxx>
#endif
#ifndef SW_INPUTWIN_HXX //autogen
#include <inputwin.hxx>
#endif
#ifndef _CHARTINS_HXX //autogen
#include <chartins.hxx>
#endif
#ifndef _UIVWIMP_HXX
#include <uivwimp.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _EDTWIN_HXX
#include <edtwin.hxx>
#endif
#ifndef _TEXTSH_HXX
#include <textsh.hxx>
#endif
#ifndef _LISTSH_HXX
#include <listsh.hxx>
#endif
#ifndef _TABSH_HXX
#include <tabsh.hxx>
#endif
#ifndef _GRFSH_HXX
#include <grfsh.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _FRMSH_HXX
#include <frmsh.hxx>
#endif
#ifndef _OLESH_HXX
#include <olesh.hxx>
#endif
#ifndef _DRAWSH_HXX
#include <drawsh.hxx>
#endif
#ifndef _DRAWBASE_HXX
#include <drawbase.hxx>
#endif
#ifndef _DRFORMSH_HXX
#include <drformsh.hxx>
#endif
#ifndef _DRWTXTSH_HXX
#include <drwtxtsh.hxx>
#endif
#ifndef _BEZIERSH_HXX
#include <beziersh.hxx>
#endif
#ifndef _GLOBDOC_HXX
#include <globdoc.hxx>
#endif
#ifndef _SCROLL_HXX
#include <scroll.hxx>
#endif
#ifndef _GLOBDOC_HXX
#include <globdoc.hxx>
#endif
#ifndef _NAVIPI_HXX
#include <navipi.hxx>
#endif
#ifndef _GLOSHDL_HXX
#include <gloshdl.hxx>
#endif
#ifndef _USRPREF_HXX
#include <usrpref.hxx>
#endif
#ifndef _SRCVIEW_HXX
#include <srcview.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _WDOCSH_HXX
#include <wdocsh.hxx>
#endif
#ifndef _WVIEW_HXX
#include <wview.hxx>
#endif
#ifndef _WORKCTRL_HXX
#include <workctrl.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _BARCFG_HXX
#include <barcfg.hxx>
#endif
#ifndef _DOC_HXX //autogen
#include <doc.hxx>
#endif
#ifndef _PVIEW_HXX
#include <pview.hxx>
#endif

#ifndef _VIEW_HRC
#include <view.hrc>
#endif
#ifndef _GLOBDOC_HRC
#include <globdoc.hrc>
#endif
#ifndef _FRMUI_HRC
#include <frmui.hrc>
#endif


#ifndef _LINGU_LNGPROPS_HHX_
#include <lingu/lngprops.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif

#ifndef _COM_SUN_STAR_SCANNER_SCANNERCONTEXT_HPP_
#include <com/sun/star/scanner/ScannerContext.hpp>
#endif
#ifndef _COM_SUN_STAR_SCANNER_XSCANNERMANAGER_HPP_
#include <com/sun/star/scanner/XScannerManager.hpp>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::scanner;
using namespace ::rtl;
#define C2U(cChar) OUString::createFromAscii(cChar)
#define C2S(cChar) String::CreateFromAscii(cChar)

extern sal_Bool bNoInterrupt;       // in mainwn.cxx

#define SWVIEWFLAGS ( SFX_VIEW_MAXIMIZE_FIRST|          \
                      SFX_VIEW_OBJECTSIZE_EMBEDDED|     \
                      SFX_VIEW_CAN_PRINT|               \
                      SFX_VIEW_HAS_PRINTOPTIONS)

//MA 06. Nov. 95: Each raus in Absprache mit MI wg. Bug 21523
//                    SFX_VIEW_OPTIMIZE_EACH|

/*--------------------------------------------------------------------
    Beschreibung:   Statics
 --------------------------------------------------------------------*/

int bDocSzUpdated = 1;

SvxSearchItem*  SwView::pSrchItem   = 0;

sal_uInt16          SwView::nInsertCtrlState        = FN_INSERT_TABLE;
sal_uInt16          SwView::nWebInsertCtrlState     = FN_INSERT_TABLE;
sal_uInt16          SwView::nInsertObjectCtrlState  = SID_INSERT_DIAGRAM;

sal_Bool            SwView::bExtra      = sal_False;
sal_Bool            SwView::bFound      = sal_False;
sal_Bool            SwView::bJustOpened = sal_False;


SvxSearchDialog*    SwView::pSrchDlg    = 0;
SearchAttrItemList* SwView::pSrchList   = 0;
SearchAttrItemList* SwView::pReplList   = 0;

DBG_NAME(viewhdl);

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

inline SfxDispatcher &SwView::GetDispatcher()
{
    return *GetViewFrame()->GetDispatcher();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwView::ImpSetVerb( int nSelType )
{
    sal_Bool bResetVerbs = bVerbsActive;
    if ( !GetDocShell()->GetProtocol().IsInPlaceActive() &&
         (SwWrtShell::SEL_OLE|SwWrtShell::SEL_GRF) & nSelType )
    {
        if ( !pWrtShell->IsSelObjProtected(FLYPROTECT_CONTENT) )
        {
            if ( nSelType & SwWrtShell::SEL_OLE )
            {
                SvInPlaceObjectRef xRef = GetWrtShell().GetOLEObj();
                SetVerbs( &xRef->GetVerbList() );
                bVerbsActive = sal_True;
                bResetVerbs = sal_False;
            }
            if ( nSelType & SwWrtShell::SEL_GRF &&

                 SFX_APP()->HasFeature( SFX_FEATURE_SIMAGE ) )
            {
                SvVerb aVerb( 0, SW_RESSTR( STR_VERB_FOR_GRF ));
                SvVerbList aLst;
                aLst.Append( aVerb );
                SetVerbs( &aLst );
                bVerbsActive = sal_True;
                bResetVerbs = sal_False;
            }
        }
    }
    if ( bResetVerbs )
    {
        SetVerbs( 0 );
        bVerbsActive = sal_False;
    }
}


void SwView::SelectShell()
{
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
//      Achtung: SelectShell fuer die WebView mitpflegen
//
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


    // Entscheidung, ob UpdateTable gerufen werden muss
    sal_Bool bUpdateTable = sal_False;
    const SwFrmFmt* pCurTableFmt = pWrtShell->GetTableFmt();
    if(pCurTableFmt && pCurTableFmt != pLastTableFormat)
    {
        bUpdateTable = sal_True; // kann erst spaeter ausgefuehrt werden
    }
    pLastTableFormat = pCurTableFmt;

    //SEL_TBL und SEL_TBL_CELLS koennen verodert sein!
    int nNewSelectionType = (pWrtShell->GetSelectionType()
                                & ~SwWrtShell::SEL_TBL_CELLS);

    if ( nNewSelectionType == nSelectionType )
    {
        GetViewFrame()->GetBindings().InvalidateAll( sal_False );
        if ( nSelectionType & SwWrtShell::SEL_OLE ||
             nSelectionType & SwWrtShell::SEL_GRF )
            //Fuer Grafiken und OLE kann sich natuerlich das Verb aendern!
            ImpSetVerb( nNewSelectionType );
    }
    else
    {

        SfxDispatcher &rDispatcher = GetDispatcher();
        SwToolbarConfigItem *pBarCfg = SW_MOD()->GetToolbarConfig();

    //  DELETEZ(pxSelectionObj); //Selektionsobjekt loeschen
        if ( pShell )
        {
            rDispatcher.Flush();        // alle gecachten Shells wirklich loeschen

            //Zur alten Selektion merken welche Toolbar sichtbar war
            sal_uInt16 nId = rDispatcher.GetObjectBarId( SFX_OBJECTBAR_OBJECT );
            if ( nId )
                pBarCfg->SetTopToolbar( nSelectionType, nId );

            SfxShell *pSfxShell;
            sal_uInt16 i;
            for ( i = 0; sal_True; ++i )
            {
                pSfxShell = rDispatcher.GetShell( i );
                if ( !(pSfxShell->ISA( SwBaseShell ) ||
                    pSfxShell->ISA( SwDrawTextShell )) )
                    break;
            }
            pSfxShell = rDispatcher.GetShell( --i );
            ASSERT( pSfxShell, "My Shell ist lost in space" );
            rDispatcher.Pop( *pSfxShell, SFX_SHELL_POP_UNTIL | SFX_SHELL_POP_DELETE);
        }

        FASTBOOL bInitFormShell = sal_False;
        if (!pFormShell)
        {
            bInitFormShell = sal_True;
            pFormShell = new FmFormShell( this );
            rDispatcher.Push( *pFormShell );

            StartListening(*pFormShell);
        }

        FASTBOOL bSetExtInpCntxt = sal_False;
        nSelectionType = nNewSelectionType;
        ShellModes eShellMode;

        if ( nSelectionType & SwWrtShell::SEL_OLE )
        {
            eShellMode = SEL_OBJECT;
            pShell = new SwOleShell( *this );
            rDispatcher.Push( *pShell );
        }
        else if ( nSelectionType & SwWrtShell::SEL_FRM
            || nSelectionType & SwWrtShell::SEL_GRF)
        {
            eShellMode = SEL_FRAME;
            pShell = new SwFrameShell( *this );
            rDispatcher.Push( *pShell );
            if(nSelectionType & SwWrtShell::SEL_GRF )
            {
                eShellMode = SEL_GRAPHIC;
                pShell = new SwGrfShell( *this );
                rDispatcher.Push( *pShell );
            }
        }
        else if ( nSelectionType & SwWrtShell::SEL_DRW )
        {
            eShellMode = SEL_DRAW;
            pShell = new SwDrawShell( *this );

            rDispatcher.Push( *pShell );
            if ( nSelectionType & SwWrtShell::SEL_BEZ )
            {
                eShellMode = SEL_BEZIER;
                pShell = new SwBezierShell( *this );
                rDispatcher.Push( *pShell );
            }
        }
        else if ( nSelectionType & SwWrtShell::SEL_DRW_FORM )
        {
            eShellMode = SEL_DRAW_FORM;
            pShell = new SwDrawFormShell( *this );

            rDispatcher.Push( *pShell );
        }
        else if ( nSelectionType & SwWrtShell::SEL_DRW_TXT )
        {
            eShellMode = SEL_DRAWTEXT;
            rDispatcher.Push( *(new SwBaseShell( *this )) );
            pShell = new SwDrawTextShell( *this );
            rDispatcher.Push( *pShell );
        }
        else
        {
            bSetExtInpCntxt = sal_True;
            eShellMode = SEL_TEXT;
            sal_uInt32 nHelpId = 0;
            if ( nSelectionType & SwWrtShell::SEL_NUM )
            {
                eShellMode = SEL_LIST_TEXT;
                pShell = new SwListShell( *this );
                nHelpId = pShell->GetHelpId();
                rDispatcher.Push( *pShell );
            }
            pShell = new SwTextShell(*this);
            if(nHelpId)
                pShell->SetHelpId(nHelpId);
            rDispatcher.Push( *pShell );
            if ( nSelectionType & SwWrtShell::SEL_TBL )
            {
                eShellMode = eShellMode == SEL_LIST_TEXT ? SEL_TABLE_LIST_TEXT
                                                        : SEL_TABLE_TEXT;
                pShell = new SwTableShell( *this );
                rDispatcher.Push( *pShell );
            }
        }
        pViewImpl->SetShellMode(eShellMode);
        ImpSetVerb( nSelectionType );

        if( !GetDocShell()->IsReadOnly() )
        {
            if( bSetExtInpCntxt && GetWrtShell().HasReadonlySel() )
                bSetExtInpCntxt = sal_False;

            InputContext aCntxt( GetEditWin().GetInputContext() );
            aCntxt.SetOptions( bSetExtInpCntxt
                                ? (aCntxt.GetOptions() |
                                        ( INPUTCONTEXT_TEXT |
                                            INPUTCONTEXT_EXTTEXTINPUT ))
                                : (aCntxt.GetOptions() & ~
                                        ( INPUTCONTEXT_TEXT |
                                            INPUTCONTEXT_EXTTEXTINPUT )) );
            GetEditWin().SetInputContext( aCntxt );
        }

        //Zur neuen Selektion die Toolbar aktivieren, die auch beim letzten Mal
        //aktiviert war
        //Vorher muss ein Flush() sein, betrifft aber lt. MBA nicht das UI und ist
        //kein Performance-Problem
        rDispatcher.Flush();
        sal_uInt16 nId = pBarCfg->GetTopToolbar( nSelectionType );
        if ( USHRT_MAX != nId )
            rDispatcher.ShowObjectBar( nId );

        Point aPnt = GetEditWin().OutputToScreenPixel(GetEditWin().GetPointerPosPixel());
        aPnt = GetEditWin().PixelToLogic(aPnt);
        GetEditWin().UpdatePointer(aPnt);

        if ( bInitFormShell && GetWrtShell().GetDrawView() )
            pFormShell->SetView(PTR_CAST(FmFormView, GetWrtShell().GetDrawView()));

    }
    //Guenstiger Zeitpunkt fuer die Kommunikation mit OLE-Objekten?
    if ( GetDocShell()->GetDoc()->IsOLEPrtNotifyPending() )
        GetDocShell()->GetDoc()->PrtOLENotify( sal_False );

    //jetzt das Tabellen-Update
    if(bUpdateTable)
        pWrtShell->UpdateTable();
}

//Zusammenspiel: AttrChangedNotify() und TimeoutHdl.
//Falls noch Actions offen sind keine Aktualisierung, da der
//Cursor auf der Core-Seite im Wald stehen kann.
//Da wir aber keine Stati mehr liefern koennen und wollen locken wir
//stattdessen den Dispatcher.



int
#if defined( WNT )
 __cdecl
#endif
#if defined( ICC )
 _Optlink
#endif
    lcl_CmpIds( const void *pFirst, const void *pSecond)
{
    return (*(sal_uInt16*)pFirst) - (*(sal_uInt16*)pSecond);
}



IMPL_LINK( SwView, AttrChangedNotify, SwWrtShell *, EMPTYARG )
{
    if ( GetEditWin().IsChainMode() )
        GetEditWin().SetChainMode( sal_False );

    //Opt: Nicht wenn PaintLocked. Beim Unlock wird dafuer nocheinmal ein
    //Notify ausgeloest.
    if( !pWrtShell->IsPaintLocked() && !bNoInterrupt &&
        GetDocShell()->IsReadOnly() )
        _CheckReadonlyState();

    // JP 19.01.99: Cursor in Readonly Bereichen
    if( !pWrtShell->IsPaintLocked() && !bNoInterrupt )
        _CheckReadonlySelection();

    if( !bAttrChgNotified )
    {
        if ( pWrtShell->BasicActionPend() || bNoInterrupt ||
             GetDispatcher().IsLocked() ||               //do not confuse the SFX
             GetViewFrame()->GetBindings().IsInUpdate() )//do not confuse the SFX
        {
            bAttrChgNotified = sal_True;
            aTimer.Start();

            const SfxPoolItem *pItem;
            if ( SFX_ITEM_SET != GetObjectShell()->GetMedium()->GetItemSet()->
                                    GetItemState( SID_HIDDEN, sal_False, &pItem ) ||
                 !((SfxBoolItem*)pItem)->GetValue() )
            {
                GetViewFrame()->GetBindings().ENTERREGISTRATIONS();
                bAttrChgNotifiedWithRegistrations = sal_True;
            }
        }
        else
            SelectShell();
    }
    GetViewImpl()->GetUNOObject_Impl()->NotifySelChanged();
    return 0;
}



IMPL_LINK( SwView, TimeoutHdl, Timer *, EMPTYARG )
{
    DBG_PROFSTART(viewhdl);

    if( pWrtShell->BasicActionPend() || bNoInterrupt )
    {
        aTimer.Start();
        DBG_PROFSTOP(viewhdl);
        return 0;
    }

    if ( bAttrChgNotifiedWithRegistrations )
    {
        GetViewFrame()->GetBindings().LEAVEREGISTRATIONS();
        bAttrChgNotifiedWithRegistrations = sal_False;
    }

    _CheckReadonlyState();
    _CheckReadonlySelection();

    SelectShell();
    bAttrChgNotified = sal_False;

    DBG_PROFSTOP(viewhdl);
    return 0;
}

void SwView::_CheckReadonlyState()
{
    SfxDispatcher &rDis = GetDispatcher();
    //Um erkennen zu koennen ob bereits disabled ist!
    SfxItemState eStateRO, eStateProtAll;
    const SfxPoolItem *pItem;
    // JP 29.04.97: von einem nur uns bekannten Slot den Status abfragen.
    //              Ansonsten kennen andere den Slot; wie z.B. die BasidIde
    eStateRO = rDis.QueryState( FN_INSERT_BOOKMARK, pItem );
    eStateProtAll = rDis.QueryState( FN_EDIT_REGION, pItem );
    sal_Bool bChgd = sal_False;

    if ( !pWrtShell->IsCrsrReadonly() )
    {
        static sal_uInt16 aROIds[] =
        {
            SID_DELETE,                 FN_BACKSPACE,               FN_SHIFT_BACKSPACE,
            SID_UNDO,
            SID_REDO,                   SID_REPEAT,                 SID_PASTE,
            FN_PASTESPECIAL,            SID_SBA_BRW_INSERT,
            SID_BACKGROUND_COLOR,       FN_INSERT_BOOKMARK,
            FN_INSERT_SYMBOL,           FN_INSERT_SOFT_HYPHEN,
            FN_INSERT_HARDHYPHEN,       FN_INSERT_HARD_SPACE,       FN_INSERT_BREAK,
            FN_INSERT_LINEBREAK,        FN_INSERT_COLUMN_BREAK,     FN_INSERT_BREAK_DLG,
            FN_DELETE_SENT,             FN_DELETE_BACK_SENT,        FN_DELETE_WORD,
            FN_DELETE_BACK_WORD,        FN_DELETE_LINE,             FN_DELETE_BACK_LINE,
            FN_DELETE_PARA,             FN_DELETE_BACK_PARA,        FN_DELETE_WHOLE_LINE,
            FN_CALCULATE,               FN_FORMAT_RESET,
            FN_POSTIT,                  FN_JAVAEDIT,                SID_ATTR_PARA_ADJUST_LEFT,
            SID_ATTR_PARA_ADJUST_RIGHT, SID_ATTR_PARA_ADJUST_CENTER,SID_ATTR_PARA_ADJUST_BLOCK,
            SID_ATTR_PARA_LINESPACE_10, SID_ATTR_PARA_LINESPACE_15, SID_ATTR_PARA_LINESPACE_20,
            SID_ATTR_CHAR_FONT,         SID_ATTR_CHAR_FONTHEIGHT,   SID_ATTR_CHAR_COLOR_BACKGROUND,
            SID_ATTR_CHAR_COLOR_BACKGROUND_EXT,                     SID_ATTR_CHAR_COLOR_EXT,
            SID_ATTR_CHAR_COLOR,        SID_ATTR_CHAR_WEIGHT,       SID_ATTR_CHAR_POSTURE,
            SID_ATTR_CHAR_UNDERLINE,    SID_ATTR_FLASH,             SID_ATTR_CHAR_STRIKEOUT,
            FN_UNDERLINE_DOUBLE,        SID_ATTR_CHAR_CONTOUR,      SID_ATTR_CHAR_SHADOWED,
            SID_ATTR_CHAR_AUTOKERN,     SID_ATTR_CHAR_ESCAPEMENT,   FN_SET_SUPER_SCRIPT,
            FN_SET_SUB_SCRIPT,          SID_ATTR_CHAR_CASEMAP,      SID_ATTR_CHAR_LANGUAGE,
            SID_ATTR_CHAR_KERNING,      SID_CHAR_DLG,               SID_ATTR_CHAR_WORDLINEMODE,
            FN_GROW_FONT_SIZE,          FN_SHRINK_FONT_SIZE,        FN_TXTATR_INET,
            FN_FORMAT_DROPCAPS,         SID_ATTR_PARA_ADJUST,       SID_ATTR_PARA_LINESPACE,
            SID_ATTR_PARA_SPLIT,        SID_ATTR_PARA_KEEP,         SID_ATTR_PARA_WIDOWS,
            SID_ATTR_PARA_ORPHANS,
            SID_ATTR_PARA_MODEL,        SID_PARA_DLG,
            FN_SELECT_PARA,             SID_DEC_INDENT,
            SID_INC_INDENT
        };
        static sal_Bool bFirst = sal_True;
        if ( bFirst )
        {
            qsort( (void*)aROIds, sizeof(aROIds)/sizeof(sal_uInt16), sizeof(sal_uInt16), lcl_CmpIds );
            bFirst = sal_False;
        }
        if ( SFX_ITEM_DISABLED == eStateRO )
        {
            rDis.SetSlotFilter( sal_Bool(2), sizeof(aROIds)/sizeof(sal_uInt16), aROIds );
            bChgd = sal_True;
        }
    }
    else if( pWrtShell->IsAllProtect() )
    {
        if ( SFX_ITEM_DISABLED == eStateProtAll )
        {
            static sal_uInt16 aAllProtIds[] = { SID_SAVEDOC, FN_EDIT_REGION };
            static sal_Bool bAllProtFirst = sal_True;
            if ( bAllProtFirst )
            {
                qsort( (void*)aAllProtIds, sizeof(aAllProtIds)/sizeof(sal_uInt16), sizeof(sal_uInt16), lcl_CmpIds );
                bAllProtFirst = sal_False;
            }
            rDis.SetSlotFilter( sal_Bool(2),
                                sizeof(aAllProtIds)/sizeof(sal_uInt16),
                                aAllProtIds );
            bChgd = sal_True;
        }
    }
    else if ( SFX_ITEM_DISABLED != eStateRO ||
                SFX_ITEM_DISABLED != eStateProtAll )
    {
        bChgd = sal_True;
        rDis.SetSlotFilter();
    }
    if ( bChgd )
        GetViewFrame()->GetBindings().InvalidateAll(sal_True);
}

void SwView::_CheckReadonlySelection()
{
    sal_uInt32 nDisableFlags = 0;
    SfxDispatcher &rDis = GetDispatcher();

    if( pWrtShell->HasReadonlySel() &&
        ( !pWrtShell->GetDrawView() ||
            !pWrtShell->GetDrawView()->GetMarkList().GetMarkCount() ))
        nDisableFlags |= SW_DISABLE_ON_PROTECTED_CURSOR;

    if( (SW_DISABLE_ON_PROTECTED_CURSOR & nDisableFlags ) !=
        (SW_DISABLE_ON_PROTECTED_CURSOR & rDis.GetDisableFlags() ) )
    {
        // zusaetzlich am Window den InputContext umsetzen, damit in
        // japanischen / chinesischen Versionen die externe Eingabe
        // ab-/angeschaltet wird. Das aber nur wenn auch die richtige
        // Shell auf dem Stack steht.
        switch( pViewImpl->GetShellMode() )
        {
        case SEL_TEXT:
        case SEL_LIST_TEXT:
        case SEL_TABLE_TEXT:
        case SEL_TABLE_LIST_TEXT:
            {
//JP 22.01.99: temporaere Loesung!!! Sollte bei jeder Cursorbewegung
//          den Font von der akt. Einfuegeposition setzen, also ausserhalb
//          dieses if's. Aber TH wertet den Font zur Zeit nicht aus und
//          das besorgen erscheint mir hier zu teuer zu sein.
//          Ausserdem haben wir keinen Font, sondern nur Attribute aus denen
//          die Textformatierung dann den richtigen Font zusammen baut.

                InputContext aCntxt( GetEditWin().GetInputContext() );
                aCntxt.SetOptions( SW_DISABLE_ON_PROTECTED_CURSOR & nDisableFlags
                                    ? (aCntxt.GetOptions() & ~
                                            ( INPUTCONTEXT_TEXT |
                                                INPUTCONTEXT_EXTTEXTINPUT ))
                                    : (aCntxt.GetOptions() |
                                            ( INPUTCONTEXT_TEXT |
                                                INPUTCONTEXT_EXTTEXTINPUT )) );
                GetEditWin().SetInputContext( aCntxt );
            }
            break;
        }

    }

    if( nDisableFlags != rDis.GetDisableFlags() )
    {
        rDis.SetDisableFlags( nDisableFlags );
        GetViewFrame()->GetBindings().InvalidateAll( sal_True );
    }
}


SwView::SwView( SfxViewFrame *pFrame, SfxViewShell* pOldSh )
    : SfxViewShell( pFrame, SWVIEWFLAGS ),
    aPageStr( SW_RES( STR_PAGE )),
    pEditWin( new SwEditWin( &pFrame->GetWindow(), *this ) ),
    pShell(0),
    pWrtShell(0),
    pFormShell(0),
    pHScrollbar(0),
    pVScrollbar(0),
    pScrollFill(0),
    pHLineal( new SvxRuler(&GetViewFrame()->GetWindow(), pEditWin,
                    SVXRULER_SUPPORT_TABS |
                    SVXRULER_SUPPORT_PARAGRAPH_MARGINS |
                    SVXRULER_SUPPORT_BORDERS |
                    SVXRULER_SUPPORT_NEGATIVE_MARGINS,
                    GetViewFrame()->GetBindings(),
                    WB_STDRULER | WB_EXTRAFIELD | WB_3DLOOK | WB_BORDER)),
    pVLineal(0),
    pTogglePageBtn(0),
    pPageUpBtn(0),
    pPageDownBtn(0),
    pNaviBtn(0),
    pGlosHdl(0),
    pDrawActual(0),
    pLastTableFormat(0),
    nDrawSfxId( USHRT_MAX ),
    nFormSfxId( USHRT_MAX ),
    nSelectionType( INT_MAX )
{
    bCenterCrsr = bTopCrsr = bAllwaysShowSel = bTabColFromDoc =
    bSetTabColFromDoc = bAttrChgNotified = bAttrChgNotifiedWithRegistrations =
    bVerbsActive = bIsApi = bDrawRotate = bInOuterResizePixel = sal_False;

    bShowAtResize = bDrawSelMode = bDocSzUpdated = sal_True;

    pViewImpl = new SwView_Impl(this);
    SetName(C2S("View"));
    SetWindow( pEditWin );

    aTimer.SetTimeout( 120 );

    SwDocShell* pDocSh = PTR_CAST( SwDocShell, pFrame->GetObjectShell() );
    ASSERT( pDocSh, "View ohne DocShell." );
    SwWebDocShell* pWebDShell = PTR_CAST( SwWebDocShell, pDocSh );

    SwViewOption aUsrPref( *SW_MOD()->GetUsrPref( 0 != pWebDShell ));
    uno::Reference< beans::XPropertySet >  xProp( ::GetLinguPropertySet() );
    sal_Bool bVal;

    bVal = xProp.is() ?
            *(sal_Bool*)xProp->getPropertyValue( C2U(UPN_IS_SPELL_AUTO)).getValue() : sal_False;
    aUsrPref.SetOnlineSpell( bVal );
    bVal = xProp.is() ?
            *(sal_Bool*)xProp->getPropertyValue( C2U(UPN_IS_SPELL_HIDE)).getValue() : sal_False;
    aUsrPref.SetHideSpell( bVal );

    if( !pOldSh )
    {
        //Gibt es schon eine Sicht auf das Dokument?
        SfxViewFrame *pF = SfxViewFrame::GetFirst( pDocSh );
        if( pF == pFrame )
            pF = SfxViewFrame::GetNext( *pF, pDocSh );
        if( pF )
            pOldSh = pF->GetViewShell();
    }
    else if( pOldSh->IsA( TYPE( SwPagePreView ) ) )
    {
        sSwViewData = ((SwPagePreView*)pOldSh)->GetPrevSwViewData();
        sNewCrsrPos = ((SwPagePreView*)pOldSh)->GetNewCrsrPos();
    }

    if(PTR_CAST( SwView, pOldSh))
    {
        pWrtShell = new SwWrtShell(((SwView*)pOldSh)->pWrtShell, pEditWin, *this);
//MA: Das kann doch nur zu einem GPF fuehren!
//      nSelectionType = ((SwView*)pOldSh)->nSelectionType;
    }
    else
    {
        SwDoc *pDoc = ((SwDocShell*)pDocSh)->GetDoc();

        if( PTR_CAST(SwSrcView, pOldSh) || pWebDShell )
            pDoc->SetBrowseMode( sal_True );

        //Fuer den BrowseMode wollen wir keinen Factor uebernehmen.
        if( pDoc->IsBrowseMode() && aUsrPref.GetZoomType() != SVX_ZOOM_PERCENT )
        {
            aUsrPref.SetZoomType( SVX_ZOOM_PERCENT );
            aUsrPref.SetZoom( 100 );
        }
        pWrtShell = new SwWrtShell( pDoc, ::GetSpellChecker(), ::GetHyphenator(),
                                    pEditWin, *this,
                                    0, &aUsrPref);
    }

    // JP 05.02.99: Bug 61495 - damit unter anderem das HLineal im
    //              ReadonlyFall nicht angezeigt wird
    aUsrPref.SetReadonly( pWrtShell->GetViewOptions()->IsReadonly() );

    //Kein Margin fuer OLE!
    Size aBrwsBorder;
    if( SFX_CREATE_MODE_EMBEDDED != pDocSh->GetCreateMode() )
        aBrwsBorder = GetMargin();

    if( pFrame->GetFrameType() & SFXFRAME_INTERNAL )
        pWrtShell->SetFrameView( aBrwsBorder );
    else
        pWrtShell->SetBrowseBorder( aBrwsBorder );

    // Im CTOR duerfen keine Shell wechsel erfolgen, die muessen ueber
    // den Timer "zwischen gespeichert" werden. Sonst raeumt der SFX
    // sie wieder vom Stack!
    sal_Bool bOld = bNoInterrupt;
    bNoInterrupt = sal_True;

    pHLineal->SetActive( sal_True );

    SfxViewFrame* pViewFrame = GetViewFrame();
    if( pViewFrame->GetFrame()->GetParentFrame())
    {
        aUsrPref.SetViewTabwin(sal_False);
        aUsrPref.SetViewVLin(sal_False);
    }

    StartListening( *pViewFrame );
    StartListening( *pDocSh );

    // Vom HLineal den ZOOM-Faktor einstellen
    pHLineal->SetZoom( Fraction( aUsrPref.GetZoom(), 100 ) );
    pHLineal->SetDoubleClickHdl(LINK( this, SwView, ExecRulerClick ));
    FieldUnit eMetric = ::GetDfltMetric( 0 != pWebDShell );
    if( FUNIT_MM == eMetric )
        eMetric = FUNIT_CM;
    pHLineal->SetUnit( eMetric );

    // DocShell setzen
    pDocSh->SetView( this );
    SW_MOD()->SetView( this );

    // Die DocSize erfragen und verarbeiten. Ueber die Handler konnte
    // die Shell nicht gefunden werden, weil die Shell innerhalb CTOR-Phase
    // nicht in der SFX-Verwaltung bekannt ist.
    DocSzChgd( pWrtShell->GetDocSize() );

        // AttrChangedNotify Link setzen
    pWrtShell->SetChgLnk(LINK(this, SwView, AttrChangedNotify));

    if( pDocSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED &&
        !((SvEmbeddedObject *)pDocSh)->GetVisArea().IsEmpty() )
        SetVisArea( ((SvEmbeddedObject *)pDocSh)->GetVisArea(),sal_False);

    SwEditShell::SetUndoActionCount( SFX_APP()->GetOptions().GetUndoCount() );
    pWrtShell->DoUndo( 0 != SwEditShell::GetUndoActionCount() );

    const FASTBOOL bBrowse = pWrtShell->GetDoc()->IsBrowseMode();
    SetNewWindowAllowed(!bBrowse);

    if( aUsrPref.IsViewVScrollBar() )
        CreateVScrollbar();
    if( aUsrPref.IsViewHScrollBar() || bBrowse )
        CreateHScrollbar();
    if( aUsrPref.IsViewTabwin() )
        CreateTab();
    if( aUsrPref.IsViewVLin() )
        CreateVLineal();

    pWrtShell->SetUIOptions( aUsrPref );
    pWrtShell->SetReadOnlyAvailable( aUsrPref.IsCursorInProtectedArea() );

    if( UseObjectSize() )
    {
        //Damit der Sfx _rechtzeitig weiss_, wie gross die ::com::sun::star::sheet::Border sind.
        SvBorder aTmp;
        CalcAndSetBorderPixel( aTmp, sal_True );
    }

    if( pWrtShell->GetDoc()->IsUpdateExpFld() )
    {
        SET_CURR_SHELL( pWrtShell );
        pWrtShell->StartAction();
        pWrtShell->CalcLayout();
        pWrtShell->GetDoc()->UpdateFlds();
        pWrtShell->EndAction();
        pWrtShell->GetDoc()->SetUpdateExpFldStat( sal_False );
    }

    // ggfs. alle Verzeichnisse updaten:
    if( pWrtShell->GetDoc()->IsUpdateTOX() )
    {
        SfxRequest aSfxRequest( FN_UPDATE_TOX, SFX_CALLMODE_SLOT, GetPool() );
        Execute( aSfxRequest );
        pWrtShell->GetDoc()->SetUpdateTOX( sal_False );     // wieder zurueck setzen
        pWrtShell->SwCrsrShell::SttDoc();
    }

    // kein ResetModified, wenn es schone eine View auf dieses Doc gibt
    SfxViewFrame* pVFrame = GetViewFrame();
    SfxViewFrame* pFirst = SfxViewFrame::GetFirst(pDocSh);
    // zur Zeit(360) wird die View erst nach dem Ctor eingetragen
    // der folgende Ausdruck funktioniert auch, wenn sich das aendert
    //JP 27.07.98: wenn per Undo nicht mehr die Modifizierung aufhebar ist,
    //              so setze das Modified NICHT zurueck.
    if( !pWrtShell->GetDoc()->IsUndoNoResetModified() &&
        (!pFirst || pFirst == pVFrame ) )
        pWrtShell->ResetModified();

    bNoInterrupt = bOld;

    // wird ein GlobalDoc neu angelegt, soll auch der Navigator erzeugt werden
    if( pDocSh->IsA(SwGlobalDocShell::StaticType()) &&
        !pVFrame->GetChildWindow( SID_NAVIGATOR ))
    {
        SfxBoolItem aNavi(SID_NAVIGATOR, sal_True);
        GetDispatcher().Execute(SID_NAVIGATOR, SFX_CALLMODE_ASYNCHRON, &aNavi, 0L);
    }


    uno::Reference< awt::XWindow >  aTmpRef;
    pFrame->GetFrame()->GetFrameInterface()->setComponent( aTmpRef,
                                            pViewImpl->GetUNOObject_Impl());

    aTimer.Stop();
    aTimer.SetTimeoutHdl(LINK(this, SwView, TimeoutHdl));
    bAttrChgNotified = bAttrChgNotifiedWithRegistrations = sal_False;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


SwView::~SwView()
{
    pEditWin->Hide(); // damit kein Paint Aerger machen kann!
    // An der SwDocShell den Pointer auf die View ruecksetzen
    SwDocShell* pDocSh = GetDocShell();
    if( pDocSh && pDocSh->GetView() == this )
        pDocSh->SetView( 0 );
    if ( SW_MOD()->GetView() == this )
        SW_MOD()->SetView( 0 );

    if( aTimer.IsActive() && bAttrChgNotifiedWithRegistrations )
        GetViewFrame()->GetBindings().LEAVEREGISTRATIONS();

    SetWindow( 0 );

    EndListening(*GetViewFrame());
    EndListening(*GetDocShell());
    delete pScrollFill;
    delete pWrtShell;
    pWrtShell = 0;      // Auf 0 setzen, damit folgende DToren nicht drauf zugreifen
    pShell = 0;
    delete pHScrollbar;
    delete pVScrollbar;
    delete pHLineal;
    delete pVLineal;
    delete pTogglePageBtn;
    delete pPageUpBtn;
    delete pNaviBtn;
    delete pPageDownBtn;
    delete pGlosHdl;
    delete pViewImpl;
    delete pEditWin;
}

/*--------------------------------------------------------------------
    Beschreibung:   DocShell rausgrabbeln ueber das FrameWindow
 --------------------------------------------------------------------*/


SwDocShell* SwView::GetDocShell()
{
    return PTR_CAST(SwDocShell, GetViewFrame()->GetObjectShell());
}

/*--------------------------------------------------------------------
    Beschreibung:   CursorPos merken
 --------------------------------------------------------------------*/


void SwView::WriteUserData( String &rUserData, sal_Bool bBrowse )
{
    //Das Browse-Flag wird vom Sfx durchgereicht, wenn Dokumente gebrowsed
    //werden (nicht zu verwechseln mit dem BrowseMode).
    //Die dann gespeicherten Daten sind nicht persistent!!

    const SwRect& rRect = pWrtShell->GetCharRect();
    const Rectangle& rVis = GetVisArea();

    rUserData = String::CreateFromInt32( rRect.Left() );
    rUserData += ';';
    rUserData += String::CreateFromInt32( rRect.Top() );
    rUserData += ';';
    rUserData += String::CreateFromInt32( pWrtShell->GetViewOptions()->GetZoom() );
    rUserData += ';';
    rUserData += String::CreateFromInt32( rVis.Left() );
    rUserData += ';';
    rUserData += String::CreateFromInt32( rVis.Top() );
    rUserData += ';';
    rUserData += String::CreateFromInt32( bBrowse ? LONG_MIN : rVis.Right());
    rUserData += ';';
    rUserData += String::CreateFromInt32( bBrowse ? LONG_MIN : rVis.Bottom());
    rUserData += ';';
    rUserData += String::CreateFromInt32(
            (sal_uInt16)pWrtShell->GetViewOptions()->GetZoomType());//eZoom;
    rUserData += ';';
    rUserData += FRMTYPE_NONE == pWrtShell->GetSelFrmType() ? '0' : '1';
}

/*--------------------------------------------------------------------
    Beschreibung: CursorPos setzen
 --------------------------------------------------------------------*/


void SwView::ReadUserData( const String &rUserData, sal_Bool bBrowse )
{
    if ( rUserData.GetTokenCount() > 1 &&
        //Fuer Dokumente ohne Layout nur im OnlineLayout oder beim
        //Forward/Backward
         (!pWrtShell->IsNewLayout() || pWrtShell->IsBrowseMode() || bBrowse) )
    {
        SET_CURR_SHELL(pWrtShell);

        sal_uInt16 nPos = 0;

        // Nein, es ist *keine* gute Idee GetToken gleich im Point-Konstr.
        // aufzurufen, denn welcher Parameter wird zuerst ausgewertet?
        long nX = rUserData.GetToken( 0, ';', nPos ).ToInt32(),
             nY = rUserData.GetToken( 0, ';', nPos ).ToInt32();
        Point aCrsrPos( nX, nY );

        sal_uInt16 nZoomFactor  = rUserData.GetToken(0, ';', nPos ).ToInt32();

        long nLeft  = rUserData.GetToken(0, ';', nPos ).ToInt32(),
             nTop   = rUserData.GetToken(0, ';', nPos ).ToInt32(),
             nRight = rUserData.GetToken(0, ';', nPos ).ToInt32(),
             nBottom= rUserData.GetToken(0, ';', nPos ).ToInt32();

        const long nAdd = pWrtShell->IsBrowseMode() ? DOCUMENTBORDER : DOCUMENTBORDER*2;
        if ( nBottom <= (pWrtShell->GetDocSize().Height()+nAdd) )
        {
            pWrtShell->EnableSmooth( sal_False );

            const Rectangle aVis( nLeft, nTop, nRight, nBottom );

            sal_uInt16 nOff = 0;
            SvxZoomType eZoom;
            if( !pWrtShell->GetDoc()->IsBrowseMode() )
                eZoom = (SvxZoomType) (sal_uInt16)rUserData.GetToken(nOff, ';', nPos ).ToInt32();
            else
            {
                eZoom = SVX_ZOOM_PERCENT;
                ++nOff;
            }

            sal_Bool bSelectObj = (0 != rUserData.GetToken( nOff, ';', nPos ).ToInt32())
                                && pWrtShell->IsObjSelectable( aCrsrPos );

            pWrtShell->SwCrsrShell::SetCrsr( aCrsrPos, !bSelectObj );
            if( bSelectObj )
            {
                pWrtShell->SelectObj( aCrsrPos );
                pWrtShell->EnterSelFrmMode( &aCrsrPos );
            }

            SelectShell();

            pWrtShell->StartAction();
            const SwViewOption* pVOpt = pWrtShell->GetViewOptions();
            if( pVOpt->GetZoom() != nZoomFactor || pVOpt->GetZoomType() != eZoom )
                SetZoom( eZoom, nZoomFactor);
            if ( bBrowse )
                SetVisArea( aVis.TopLeft() );
            else
                SetVisArea( aVis );

            pWrtShell->LockView( sal_True );
            pWrtShell->EndAction();
            pWrtShell->LockView( sal_False );
            pWrtShell->EnableSmooth( sal_True );
        }
    }
}

void SwView::UIDeactivate( SvInPlaceObject* )
{
    GetEditWin().OleToGrf();
}



void SwView::ShowCursor( FASTBOOL bOn )
{
    if ( bOn )
        pWrtShell->ShowCrsrs( sal_True );
    else
        pWrtShell->HideCrsrs();
}



ErrCode SwView::DoVerb( long nVerb )
{
    if ( !GetDocShell()->GetProtocol().IsInPlaceActive() )
    {
        SwWrtShell &rSh = GetWrtShell();
        const int nSel = rSh.GetSelectionType();
        if ( nSel & SwWrtShell::SEL_OLE )
            rSh.LaunchOLEObj( nVerb );
        else if ( nSel & SwWrtShell::SEL_GRF && nVerb == 0 )
            GetDispatcher().Execute( SID_SIM_START );
    }
    return ERRCODE_NONE;
}

/*-----------------17.02.98 13:33-------------------
    nur sal_True fuer eine Textselektion zurueck geben
--------------------------------------------------*/
sal_Bool SwView::HasSelection( sal_Bool  bText ) const
{
    return bText ? GetWrtShell().SwCrsrShell::HasSelection()
                 : GetWrtShell().HasSelection();
}

/*-----------------09/16/97 09:50am-----------------

--------------------------------------------------*/

String SwView::GetSelectionText( sal_Bool bComplete )
{
    return GetSelectionTextParam( bComplete, sal_True );
}

/*-----------------09/16/97 09:50am-----------------

--------------------------------------------------*/
String  SwView::GetSelectionTextParam(  sal_Bool bComplete,
                                            sal_Bool bEraseTrail,
                                            sal_Bool bExtendedDelimiter )
{
    String sReturn;

    if (bComplete && !GetWrtShell().HasSelection())
    {
        const char sDelimiter[] = " \t\n\r";
        const char sExtDelimiter[] = " \t\n\r.,:;()<>{}";
        WordSelection::SetWordDelimiter(bExtendedDelimiter ?
                                            sExtDelimiter : sDelimiter);
        GetWrtShell().SelWrd();
        WordSelection::ResetWordDelimiter();
    }

    GetWrtShell().GetSelectedText(sReturn);
    if(bEraseTrail)
        sReturn.EraseTrailingChars();
    return sReturn;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/



SwGlossaryHdl* SwView::GetGlosHdl()
{
    if(!pGlosHdl)
        pGlosHdl = new SwGlossaryHdl(GetViewFrame(), pWrtShell);
    return pGlosHdl;
}

/*-----------------05.09.96 15.50-------------------

--------------------------------------------------*/



void SwView::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    FASTBOOL bCallBase = sal_True;
    if ( rHint.ISA(SfxSimpleHint) )
    {
        sal_uInt32 nId = ((SfxSimpleHint&)rHint).GetId();
        switch ( nId )
        {
            case SFX_HINT_MODECHANGED:
                {
                    // Modalmodus-Umschaltung?
                    sal_Bool bModal = GetDocShell()->IsInModalMode();
                    if(pHLineal)
                        pHLineal->SetActive( !bModal );
                    if(pVLineal)
                        pVLineal->SetActive( !bModal );
                }

                /* kein break hier */

            case SFX_HINT_TITLECHANGED:
                if ( GetDocShell()->IsReadOnly() != GetWrtShell().GetViewOptions()->IsReadonly() )
                {
                    SwWrtShell &rSh = GetWrtShell();
                    rSh.SetReadonlyOption( GetDocShell()->IsReadOnly() );

                    if ( rSh.GetViewOptions()->IsViewVLin() )
                        CreateVLineal();
                    else
                        KillVLineal();
                    if ( rSh.GetViewOptions()->IsViewTabwin() )
                        CreateTab();
                    else
                        KillTab();
                    SfxBoolItem aItem( SID_FM_DESIGN_MODE, !GetDocShell()->IsReadOnly());
                    GetDispatcher().Execute( SID_FM_DESIGN_MODE, SFX_CALLMODE_ASYNCHRON,
                                                &aItem, 0L );
                }
                break;

            case SW_BROADCAST_DRAWVIEWS_CREATED:
                {
                    bCallBase = sal_False;
                    if ( GetFormShell() )
                    {
                        GetFormShell()->SetView(
                            PTR_CAST(FmFormView, GetWrtShell().GetDrawView()) );
                        SfxBoolItem aItem( SID_FM_DESIGN_MODE, !GetDocShell()->IsReadOnly());
                        GetDispatcher().Execute( SID_FM_DESIGN_MODE, SFX_CALLMODE_ASYNCHRON,
                                                  &aItem, 0L );
                    }
                }
                break;
        }
    }
    else if(rHint.ISA(FmDesignModeChangedHint))
    {
        sal_Bool bDesignMode = ((FmDesignModeChangedHint&)rHint).GetDesignMode();
        if (!bDesignMode && GetDrawFuncPtr())
        {
            GetDrawFuncPtr()->Deactivate();
            SetDrawFuncPtr(NULL);
            LeaveDrawCreate();
            AttrChangedNotify(pWrtShell);
        }
    }

    if ( bCallBase )
        SfxViewShell::Notify(rBC, rHint);
}

/*-----------------02.12.96 12:36-------------------

--------------------------------------------------*/
#if defined WIN || defined WNT || defined UNX

void SwView::ScannerEventHdl( const EventObject& rEventObject )
{
    Reference< XScannerManager > xScanMgr = SW_MOD()->GetScannerManager();
    if( xScanMgr.is() )
    {
        const ScannerContext    aContext( xScanMgr->getAvailableScanners().getConstArray()[ 0 ] );
        const ScanError         eError = xScanMgr->getError( aContext );

        if( ScanError_ScanErrorNone == eError )
        {
            const uno::Reference< awt::XBitmap > xBitmap( xScanMgr->getBitmap( aContext ) );

            if( xBitmap.is() )
            {
                const BitmapEx aScanBmp( VCLUnoHelper::GetBitmap( xBitmap ) );

                if( !!aScanBmp )
                {
                    Graphic aGrf(aScanBmp);
                    pWrtShell->Insert( aEmptyStr, aEmptyStr, aGrf );
                }
            }
        }
    }
    SfxBindings& rBind = GetViewFrame()->GetBindings();
    rBind.Invalidate( SID_TWAIN_SELECT );
    rBind.Invalidate( SID_TWAIN_TRANSFER );
//JP 26.06.00: the appwindow doen't exist
//  Application::GetAppWindow()->EnableInput( TRUE );
}
#endif

/*-----------------04.03.97 15:07-------------------

--------------------------------------------------*/


void    SwView::StopShellTimer()
{
    if(aTimer.IsActive())
    {
        aTimer.Stop();
        if ( bAttrChgNotifiedWithRegistrations )
        {
            GetViewFrame()->GetBindings().LEAVEREGISTRATIONS();
            bAttrChgNotifiedWithRegistrations = sal_False;
        }
        SelectShell();
        bAttrChgNotified = sal_False;
    }
}

/*-----------------15.12.97 10:15-------------------

--------------------------------------------------*/
void SwView::PrepareGetSelection()
{
    if(!pShell)
        SelectShell();
}

/*-----------------09/03/97 04:12pm-----------------

--------------------------------------------------*/
sal_uInt16  SwView::PrepareClose( sal_Bool bUI, sal_Bool bForBrowsing )
{
    SfxViewFrame* pVFrame = GetViewFrame();
    SfxApplication* pSfxApp = SFX_APP();
    pVFrame->SetChildWindow( SwInputChild::GetChildWindowId(), sal_False );
    if(pSfxApp->IsDispatcherLocked())
        pSfxApp->LockDispatcher(sal_False);
    pVFrame->SetChildWindow( SwInsertChartChild::GetChildWindowId(), sal_False );

    sal_uInt16 nRet;
    if ( pFormShell &&
         sal_True != (nRet = pFormShell->PrepareClose( bUI, bForBrowsing )) )

    {
        return nRet;
    }
    return SfxViewShell::PrepareClose( bUI, bForBrowsing );
}

/*------------------------------------------------------------------------
    $Log: not supported by cvs2svn $
    Revision 1.448  2000/09/18 16:06:12  willem.vandorp
    OpenOffice header added.

    Revision 1.447  2000/09/08 08:12:53  os
    Change: Set/Toggle/Has/Knows/Show/GetChildWindow

    Revision 1.446  2000/09/07 15:59:32  os
    change: SFX_DISPATCHER/SFX_BINDINGS removed

    Revision 1.445  2000/09/07 08:23:47  os
    #77494# use dispatcher from the ViewFrame

    Revision 1.444  2000/08/29 14:00:43  ama
    New: Negative margins

    Revision 1.443  2000/08/15 14:09:39  jp
    Task #77422#: PrintPreView in the same window

    Revision 1.442  2000/08/15 14:08:15  jp
    Task #77422#: PrintPreView in the same window

    Revision 1.441  2000/08/14 17:25:37  jp
    Task #77422#: PrintPreView in the same window

    Revision 1.440  2000/08/08 13:48:14  os
    #77423# separate graphic shell

    Revision 1.439  2000/07/24 10:10:28  jp
    Bug #76909#: AttrChg-Timer can only be started after leaving the SwView CTOR

    Revision 1.438  2000/06/26 10:44:09  jp
    must change: GetAppWindow->GetDefaultDevice

    Revision 1.437  2000/05/30 10:42:17  os
    unused slots removed

    Revision 1.436  2000/05/29 08:03:23  os
    new scanner interface

    Revision 1.435  2000/05/26 07:21:34  os
    old SW Basic API Slots removed

    Revision 1.434  2000/05/24 13:13:57  hr
    conflict between STLPORT and Workshop header

    Revision 1.433  2000/05/23 19:42:30  jp
    Bugfixes for Unicode

    Revision 1.432  2000/05/10 11:53:20  os
    Basic API removed

    Revision 1.431  2000/04/18 15:02:50  os
    UNICODE

    Revision 1.430  2000/03/23 07:50:25  os
    UNO III

    Revision 1.429  2000/03/03 15:17:04  os
    StarView remainders removed

    Revision 1.428  2000/02/11 14:59:19  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.427  1999/12/10 13:09:42  tl
    #70383# SvxGetLinguPropertySet => ::GetLinguPropertySet

    Revision 1.426  1999/11/30 10:04:42  jp
    Bug #69906#: CTOR - update fields before any TOX

    Revision 1.425  1999/11/24 18:35:58  tl
    check for Service availability

    Revision 1.424  1999/11/19 16:40:24  os
    modules renamed

    Revision 1.423  1999/11/17 14:28:15  jp
    HasSelection: if text-flag set, then returns only true when the textcursor has an selection

    Revision 1.422  1999/10/25 19:43:19  tl
    ongoing ONE_LINGU implementation

    Revision 1.421  1999/10/20 10:56:16  jp
    Bug #67821#: CTOR - SynChildWindow handeld by HasUIFeature

    Revision 1.420  1999/10/06 12:21:13  os
    #68902# correct detection of FrameSet

    Revision 1.419  1999/09/10 13:19:10  os
    Chg: resource types removed

    Revision 1.418  1999/09/10 09:36:31  jp
    Bug #68559#: AttrChangeNotify - if have to start the timer, then check in TimeoutHdl the readony state

    Revision 1.417  1999/08/31 08:40:20  TL
    #if[n]def ONE_LINGU inserted (for transition of lingu to StarOne)


      Rev 1.416   31 Aug 1999 10:40:20   TL
   #if[n]def ONE_LINGU inserted (for transition of lingu to StarOne)

      Rev 1.415   04 Aug 1999 10:19:38   JP
   have to change: FmFormShell becomes the SfxViewShell

      Rev 1.414   15 Jul 1999 13:04:42   MA
   #67402# no invalidation while the dispatcher IsInUpdate()

      Rev 1.413   08 Jul 1999 17:36:44   JP
   Bug #67527#: AttrChangedNotify - if interrupt flag is set, then dont check readonly selection

      Rev 1.412   07 Jul 1999 15:09:28   MA
   use bindings from own view frame

      Rev 1.411   07 Jul 1999 12:30:58   MA
   #67331# restart Timer if dispatcher is locked

      Rev 1.410   07 Jul 1999 08:52:42   OS
   #67471# NotifySelChanged in AttrChangedNotify

      Rev 1.409   22 Jun 1999 17:37:32   JP
   Bug #43028#: disableflag for edit in mailbody

      Rev 1.408   21 Jun 1999 15:24:58   JP
   Interface changes: Twain

      Rev 1.407   10 Jun 1999 12:06:26   JP
   have to change: no AppWin from SfxApp

      Rev 1.406   05 May 1999 11:23:24   OS
   #63790# Tabellen-Update nur bei Bedarf rufen

      Rev 1.405   21 Apr 1999 09:32:26   MA
   #63638# API-Change prepareclose

      Rev 1.404   22 Mar 1999 09:10:14   OS
   #63704# PrepareClose: Auf FormShell achten

      Rev 1.403   16 Mar 1999 08:58:26   MA
   #59840# ReadUserData, NewLayout nicht fuer forward/backward auswerten

      Rev 1.402   08 Mar 1999 13:38:36   MA
   #62792# OLE-Printer-Benachrichtigung vervollstaendigt

      Rev 1.401   05 Feb 1999 17:33:00   JP
    Bug #61491#: AttrChgNotify: pruefen, ob ueberhaupt eine DrawView existiert

      Rev 1.400   05 Feb 1999 13:33:34   JP
   Bug #61495#: CTOR: an den UserPref das ReadonlyFlag setzen
   Bug #61491#: AttrChgNotify: kein Readonly bei Rahmen-/Object-Selektion

      Rev 1.399   02 Feb 1999 13:18:56   JP
   Bug #61335#: Schnistelle zu SwView geaendert/aufgeraeumt

      Rev 1.398   27 Jan 1999 09:58:26   OS
   #56371# TF_ONE51

      Rev 1.397   26 Jan 1999 15:33:42   OS
   #58677# Cursor in Readonly-Bereichen

      Rev 1.396   22 Jan 1999 16:55:40   JP
   Bug Task #58677#: Crsr in Readonly Bereichen zulassen

      Rev 1.395   19 Jan 1999 22:56:56   JP
   Task #58677#: Crsr in Readonly Bereichen zulassen

      Rev 1.394   17 Dec 1998 19:18:28   JP
   Task #59490# ExtTextInput fuer japan./chine. Version

      Rev 1.393   02 Dec 1998 14:13:06   MA
   #59988# ReadUserData, OnlineLayout auch ohne Layout

      Rev 1.392   09 Nov 1998 14:06:54   JP
   #58860# warum alles immer nur halb

      Rev 1.391   03 Nov 1998 19:53:00   MA
   #58860# nicht immer enterregistrations rufen

      Rev 1.390   02 Nov 1998 15:49:32   JP
   Task #57724#: ggfs. PrepareClose an der FormShell rufen.

      Rev 1.389   21 Oct 1998 10:56:14   OM
   #57586# Redlining nach Autoformat

      Rev 1.388   20 Oct 1998 11:40:24   OM
   #57586# Redlining nach Autoformat

      Rev 1.387   01 Oct 1998 17:48:56   MA
   #57380# Kein Layout, keine Userdaten

      Rev 1.386   01 Oct 1998 15:59:50   OM
   #52654# SelectionChangeListener

      Rev 1.385   08 Sep 1998 17:05:10   OS
   #56134# Metric fuer Text und HTML getrennt

      Rev 1.384   04 Sep 1998 17:06:38   MA
   #55396# Auch bei browse ignorieren weil bei jedem Laden gesetzt

      Rev 1.383   03 Sep 1998 11:44:44   MA
   #55396# scheinbar unsinnige VisArea im ReadUserData ignorieren

      Rev 1.382   31 Aug 1998 17:48:34   OM
   #55613# Selektion drucken

      Rev 1.381   31 Aug 1998 11:19:40   OS
   #55750# OnlineSpelling immer aus der OFA uebernehmen

      Rev 1.380   20 Aug 1998 17:25:16   MH
   chg: twain.hxx

      Rev 1.379   27 Jul 1998 16:55:42   JP
   Bug #45529#: LoadStyles muss ggfs. das Zuruecksetzen des ModifiedFlags verhindern

      Rev 1.378   20 Jul 1998 10:45:14   MA
   #53013# Nach Anweisung abgeklemmt

      Rev 1.377   17 Jul 1998 16:19:30   OM
   #51862# Im AktivenMode keine ::com::sun::star::form::Forms einfuegen

      Rev 1.376   14 Jul 1998 10:43:36   OM
   #52748# Twain-Menueeintrag

      Rev 1.375   06 Jul 1998 13:32:10   MH
   chg: setComponent

      Rev 1.374   30 Jun 1998 13:00:08   MA
   das war wohl grad etwas zu gruendlich

      Rev 1.372   09 Jun 1998 15:32:22   OM
   VC-Controls entfernt

      Rev 1.371   02 Jun 1998 15:49:32   OS
   setController() am FrameInterface rufen

      Rev 1.370   02 Apr 1998 17:30:58   OM
   FrameProtect: Abfrage umgestellt

      Rev 1.369   02 Apr 1998 10:07:00   MA
   new: ToolbarCfg

      Rev 1.368   20 Mar 1998 13:23:56   MA
   OleVis2Page durch BrowseMode ersetzt

      Rev 1.367   18 Mar 1998 12:28:50   MA
   #Wichtig fuer ::com::sun::star::form::Forms# FormShell nach dem Push initialisieren

      Rev 1.366   16 Mar 1998 12:03:18   MA
   Verbindung FormShell->DrawView immer herstellen

      Rev 1.365   15 Mar 1998 15:14:06   OM
   Synchron-Button

      Rev 1.364   13 Mar 1998 15:20:54   OS
   Lineale nach dem Umschalten in den Readonly-Mode #48190#

      Rev 1.363   12 Mar 1998 12:59:04   OS
   SID_NEWWINDOW nicht mehr ueberladen

      Rev 1.362   11 Mar 1998 17:01:52   OM
   DB-FormShell

      Rev 1.361   10 Mar 1998 14:17:46   OM
   Konstruktionsmodul fuer ::com::sun::star::form::Forms

      Rev 1.360   26 Feb 1998 09:53:56   OS
   HasSelection : const

      Rev 1.359   19 Feb 1998 17:21:48   OS
   HasSelection

      Rev 1.358   11 Feb 1998 10:33:46   MA
   #47035# und CopyCtor entfernt

      Rev 1.357   22 Jan 1998 17:48:54   OS
   SID_BACKGROUND_PATTERN entsorgt

      Rev 1.356   07 Jan 1998 13:36:24   OS
   Lineale nach MODECHANGED-Hint aktivieren/deaktivieren #46023#, EditWin im Dtor zuerst hiden#46436#

      Rev 1.355   06 Jan 1998 17:56:38   OS
   _Impl immer anlegen #46360#

      Rev 1.354   06 Jan 1998 07:28:38   OS
   getrennter Status fuer Insert-Controller #46322#

      Rev 1.353   16 Dec 1997 12:00:24   OS
   Impl-Pointer fuer UNO

      Rev 1.352   29 Nov 1997 16:49:54   MA
   includes

      Rev 1.351   28 Nov 1997 11:27:12   TJ
   include

      Rev 1.350   21 Nov 1997 15:00:12   MA
   includes

      Rev 1.349   05 Nov 1997 11:52:10   MA
   chain

      Rev 1.348   31 Oct 1997 11:36:10   OS
   GetSelectionTextParam auch mit erweiterten Delimitern #45212#

      Rev 1.347   22 Sep 1997 13:11:10   MA
   #44052# bInOuterResizePixel ist jetzt Member

      Rev 1.346   18 Sep 1997 14:01:12   OS
   Pointer::GetPosPixel fuer VCL ersetzt

      Rev 1.345   16 Sep 1997 10:25:04   OS
   neu: GetSelectionTextParam #43766#

      Rev 1.344   09 Sep 1997 13:34:04   OS
   include

      Rev 1.343   09 Sep 1997 11:35:32   OS
   header

      Rev 1.342   04 Sep 1997 17:14:38   MA
   includes

      Rev 1.341   03 Sep 1997 16:28:20   OS
   im PrepareClose die Rechenleiste und den ChartDialog schliessen

      Rev 1.340   02 Sep 1997 15:04:02   JP
   AttChgdNotify: SID_SaveDoc bei ReadOnlyUI freigeschaltet

      Rev 1.339   02 Sep 1997 11:18:28   JP
   temp. Erweiterung fuers Speichern bei ReadOnlyUI

------------------------------------------------------------------------*/



