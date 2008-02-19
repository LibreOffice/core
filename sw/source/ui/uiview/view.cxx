 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: view.cxx,v $
 *
 *  $Revision: 1.107 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-19 13:58:47 $
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


#include <string> // HACK: prevent conflict between STLPORT and Workshop headers

#include <stdlib.h>
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#include <sfx2/docinf.hxx>

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif
#ifndef _SV_GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
#ifndef _VCL_INPUTCTX_HXX //autogen
#include <vcl/inputctx.hxx>
#endif
#ifndef _SBX_SBXOBJECT_HXX //autogen
#include <basic/sbxobj.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_UNDOOPT_HXX
#include <svtools/undoopt.hxx>
#endif
#ifndef _SVTOOLS_LINGUCFG_HXX_
#include <svtools/lingucfg.hxx>
#endif
#ifndef _SV_PRINTDLG_HXX_
#include <svtools/printdlg.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_USEROPTIONS_HXX
#include <svtools/useroptions.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFX_PRINTER_HXX
#include <sfx2/printer.hxx>
#endif
#include <sfx2/app.hxx>
#ifndef _SVX_RULER_HXX //autogen
#include <svx/ruler.hxx>
#endif
#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif
#ifndef _SVX_FMSHELL_HXX //autogen
#include <svx/fmshell.hxx>
#endif
#ifndef _SVX_EXTRUSION_BAR_HXX
#include <svx/extrusionbar.hxx>
#endif
#ifndef _SVX_FONTWORK_BAR_HXX
#include <svx/fontworkbar.hxx>
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
#ifndef _MEDIASH_HXX
#include <mediash.hxx>
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
#ifndef _PVIEW_HXX
#include <pview.hxx>
#endif
#ifndef _SWDTFLVR_HXX
#include <swdtflvr.hxx>
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
#ifndef _CFGITEMS_HXX
#include <cfgitems.hxx>
#endif
#ifndef _PRTOPT_HXX
#include <prtopt.hxx>
#endif
#ifndef _SWPRTOPT_HXX
#include <swprtopt.hxx>
#endif
#ifndef _LINGUISTIC_LNGPROPS_HHX_
#include <linguistic/lngprops.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif
//#include <sfx2/app.hxx>
#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
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
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif

// #107253#
#ifndef _SWLINGUCONFIG_HXX
#include <swlinguconfig.hxx>
#endif

#ifndef _SWFORMATCLIPBOARD_HXX
#include "formatclipboard.hxx"
#endif

#ifndef _POSTITMGR_HXX
#include <PostItMgr.hxx>
#endif
#include <annotsh.hxx>

#include <unomid.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::scanner;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;


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

DBG_NAME(viewhdl)

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
    if ( !GetViewFrame()->GetFrame()->IsInPlace() &&
         (nsSelectionType::SEL_OLE|nsSelectionType::SEL_GRF) & nSelType )
    {
        if ( !pWrtShell->IsSelObjProtected(FLYPROTECT_CONTENT) )
        {
            if ( nSelType & nsSelectionType::SEL_OLE )
            {
                SetVerbs( GetWrtShell().GetOLEObject()->getSupportedVerbs() );
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

/*--------------------------------------------------------------------
    Description:
    called by the SwEditWin when it gets the focus
 --------------------------------------------------------------------*/

void SwView::GotFocus() const
{
    // if we got the focus, and the form shell *is* on the top of the dispatcher
    // stack, then we need to rebuild the stack (the form shell doesn't belong to
    // the top then)
    const SfxDispatcher& rDispatcher = const_cast< SwView* >( this )->GetDispatcher();
    SfxShell* pTopShell = rDispatcher.GetShell( 0 );
    FmFormShell* pAsFormShell = PTR_CAST( FmFormShell, pTopShell );
    if ( pAsFormShell )
    {
        pAsFormShell->ForgetActiveControl();
        const_cast< SwView* >( this )->AttrChangedNotify( pWrtShell );
    }
    else if ( mpPostItMgr )
    {
        SwAnnotationShell* pAsAnnotationShell = PTR_CAST( SwAnnotationShell, pTopShell );
        if ( pAsAnnotationShell )
        {
            mpPostItMgr->SetActivePostIt(0);
            const_cast< SwView* >( this )->AttrChangedNotify( pWrtShell );
        }
    }
}

/*--------------------------------------------------------------------
    Description:
    called by the FormShell when a form control is focused. This is
    a request to put the form shell on the top of the dispatcher
    stack
 --------------------------------------------------------------------*/

IMPL_LINK( SwView, FormControlActivated, FmFormShell*, EMPTYARG )
{
    // if a form control has been activated, and the form shell is not on the top
    // of the dispatcher stack, then we need to activate it
    const SfxDispatcher& rDispatcher = GetDispatcher();
    const SfxShell* pTopShell = rDispatcher.GetShell( 0 );
    const FmFormShell* pAsFormShell = PTR_CAST( FmFormShell, pTopShell );
    if ( !pAsFormShell )
    {
        // if we're editing text currently, cancel this
        SdrView *pSdrView = pWrtShell ? pWrtShell->GetDrawView() : NULL;
        if ( pSdrView && pSdrView->IsTextEdit() )
            pSdrView->SdrEndTextEdit( sal_True );

        const_cast< SwView* >( this )->AttrChangedNotify( pWrtShell );
    }

    return 0L;
}

void SwView::SelectShell()
{
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
//      Achtung: SelectShell fuer die WebView mitpflegen
//
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

          if(bInDtor)
        return;
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
                                & ~nsSelectionType::SEL_TBL_CELLS);

    if ( pFormShell && pFormShell->IsActiveControl() )
        nNewSelectionType |= nsSelectionType::SEL_FOC_FRM_CTRL;

    if ( nNewSelectionType == nSelectionType )
    {
        GetViewFrame()->GetBindings().InvalidateAll( sal_False );
        if ( nSelectionType & nsSelectionType::SEL_OLE ||
             nSelectionType & nsSelectionType::SEL_GRF )
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
            USHORT nId = static_cast< USHORT >( rDispatcher.GetObjectBarId( SFX_OBJECTBAR_OBJECT ));
            if ( nId )
                pBarCfg->SetTopToolbar( nSelectionType, nId );

            SfxShell *pSfxShell;
            for ( sal_uInt16 i = 0; sal_True; ++i )
            {
                pSfxShell = rDispatcher.GetShell( i );
                if  (  pSfxShell->ISA( SwBaseShell )
                    || pSfxShell->ISA( SwDrawTextShell )
                    || pSfxShell->ISA( svx::ExtrusionBar )
                    || pSfxShell->ISA( svx::FontworkBar )
                    || pSfxShell->ISA( SwAnnotationShell )
                    )
                {
                    rDispatcher.Pop( *pSfxShell, SFX_SHELL_POP_DELETE );
                }
                else if ( pSfxShell->ISA( FmFormShell ) )
                {
                    rDispatcher.Pop( *pSfxShell );
                }
                else
                    break;
            }
        }

        BOOL bInitFormShell = sal_False;
        if (!pFormShell)
        {
            bInitFormShell = sal_True;
            pFormShell = new FmFormShell( this );
            pFormShell->SetControlActivationHandler( LINK( this, SwView, FormControlActivated ) );
            StartListening(*pFormShell);
        }

        BOOL bSetExtInpCntxt = sal_False;
        nSelectionType = nNewSelectionType;
        ShellModes eShellMode;

        if ( !( nSelectionType & nsSelectionType::SEL_FOC_FRM_CTRL ) )
            rDispatcher.Push( *pFormShell );

        if ( nSelectionType & nsSelectionType::SEL_OLE )
        {
            eShellMode = SHELL_MODE_OBJECT;
            pShell = new SwOleShell( *this );
            rDispatcher.Push( *pShell );
        }
        else if ( nSelectionType & nsSelectionType::SEL_FRM
            || nSelectionType & nsSelectionType::SEL_GRF)
        {
            eShellMode = SHELL_MODE_FRAME;
            pShell = new SwFrameShell( *this );
            rDispatcher.Push( *pShell );
            if(nSelectionType & nsSelectionType::SEL_GRF )
            {
                eShellMode = SHELL_MODE_GRAPHIC;
                pShell = new SwGrfShell( *this );
                rDispatcher.Push( *pShell );
            }
        }
        else if ( nSelectionType & nsSelectionType::SEL_DRW )
        {
            eShellMode = SHELL_MODE_DRAW;
            pShell = new SwDrawShell( *this );
            rDispatcher.Push( *pShell );

            if ( nSelectionType & nsSelectionType::SEL_BEZ )
            {
                eShellMode = SHELL_MODE_BEZIER;
                pShell = new SwBezierShell( *this );
                rDispatcher.Push( *pShell );
            }
            else if( nSelectionType & nsSelectionType::SEL_MEDIA )
            {
                eShellMode = SHELL_MODE_MEDIA;
                pShell = new SwMediaShell( *this );
                rDispatcher.Push( *pShell );
            }

            if (nSelectionType & nsSelectionType::SEL_EXTRUDED_CUSTOMSHAPE)
            {
                eShellMode = SHELL_MODE_EXTRUDED_CUSTOMSHAPE;
                pShell = new svx::ExtrusionBar(this);
                rDispatcher.Push( *pShell );
            }
            if (nSelectionType & nsSelectionType::SEL_FONTWORK)
            {
                eShellMode = SHELL_MODE_FONTWORK;
                pShell = new svx::FontworkBar(this);
                rDispatcher.Push( *pShell );
            }
        }
        else if ( nSelectionType & nsSelectionType::SEL_DRW_FORM )
        {
            eShellMode = SHELL_MODE_DRAW_FORM;
            pShell = new SwDrawFormShell( *this );

            rDispatcher.Push( *pShell );
        }
        else if ( nSelectionType & nsSelectionType::SEL_DRW_TXT )
        {
            bSetExtInpCntxt = sal_True;
            eShellMode = SHELL_MODE_DRAWTEXT;
            rDispatcher.Push( *(new SwBaseShell( *this )) );
            pShell = new SwDrawTextShell( *this );
            rDispatcher.Push( *pShell );
        }
        else if ( nSelectionType & nsSelectionType::SEL_POSTIT )
        {
            eShellMode = SHELL_MODE_POSTIT;
            pShell = new SwAnnotationShell( *this );
            rDispatcher.Push( *pShell );
        }
        else
        {
            bSetExtInpCntxt = sal_True;
            eShellMode = SHELL_MODE_TEXT;
            sal_uInt32 nHelpId = 0;
            if ( nSelectionType & nsSelectionType::SEL_NUM )
            {
                eShellMode = SHELL_MODE_LIST_TEXT;
                pShell = new SwListShell( *this );
                nHelpId = pShell->GetHelpId();
                rDispatcher.Push( *pShell );
            }
            pShell = new SwTextShell(*this);
            if(nHelpId)
                pShell->SetHelpId(nHelpId);
            rDispatcher.Push( *pShell );
            if ( nSelectionType & nsSelectionType::SEL_TBL )
            {
                eShellMode = eShellMode == SHELL_MODE_LIST_TEXT ? SHELL_MODE_TABLE_LIST_TEXT
                                                        : SHELL_MODE_TABLE_TEXT;
                pShell = new SwTableShell( *this );
                rDispatcher.Push( *pShell );
            }
        }

        if ( nSelectionType & nsSelectionType::SEL_FOC_FRM_CTRL )
            rDispatcher.Push( *pFormShell );

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
        // TODO/LATER: maybe now the Flush() command is superfluous?!
        rDispatcher.Flush();

        Point aPnt = GetEditWin().OutputToScreenPixel(GetEditWin().GetPointerPosPixel());
        aPnt = GetEditWin().PixelToLogic(aPnt);
        GetEditWin().UpdatePointer(aPnt);

        SdrView* pDView = GetWrtShell().GetDrawView();
        if ( bInitFormShell && pDView )
            pFormShell->SetView(PTR_CAST(FmFormView, pDView));

    }
    //Guenstiger Zeitpunkt fuer die Kommunikation mit OLE-Objekten?
    if ( GetDocShell()->GetDoc()->IsOLEPrtNotifyPending() )
        GetDocShell()->GetDoc()->PrtOLENotify( sal_False );

    //jetzt das Tabellen-Update
    if(bUpdateTable)
        pWrtShell->UpdateTable();

    GetViewImpl()->GetUNOObject_Impl()->NotifySelChanged();
}

//Zusammenspiel: AttrChangedNotify() und TimeoutHdl.
//Falls noch Actions offen sind keine Aktualisierung, da der
//Cursor auf der Core-Seite im Wald stehen kann.
//Da wir aber keine Stati mehr liefern koennen und wollen locken wir
//stattdessen den Dispatcher.



extern "C"
{
    int lcl_CmpIds( const void *pFirst, const void *pSecond)
    {
        return (*(sal_uInt16*)pFirst) - (*(sal_uInt16*)pSecond);
    }
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

    BOOL bOldUndo = pWrtShell->DoesUndo();
    pWrtShell->DoUndo( FALSE );
    SelectShell();
    pWrtShell->DoUndo( bOldUndo );
    bAttrChgNotified = sal_False;
    GetViewImpl()->GetUNOObject_Impl()->NotifySelChanged();

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
            !pWrtShell->GetDrawView()->GetMarkedObjectList().GetMarkCount() ))
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
        case SHELL_MODE_TEXT:
        case SHELL_MODE_LIST_TEXT:
        case SHELL_MODE_TABLE_TEXT:
        case SHELL_MODE_TABLE_LIST_TEXT:
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
        default:
            ;
        }

    }

    if( nDisableFlags != rDis.GetDisableFlags() )
    {
        rDis.SetDisableFlags( nDisableFlags );
        GetViewFrame()->GetBindings().InvalidateAll( sal_True );
    }
}


SwView::SwView( SfxViewFrame *_pFrame, SfxViewShell* pOldSh )
    : SfxViewShell( _pFrame, SWVIEWFLAGS ),

    aPageStr( SW_RES( STR_PAGE )),
    nNewPage(USHRT_MAX),
    pNumRuleNodeFromDoc(0), // #i23726#
    pEditWin( new SwEditWin( &_pFrame->GetWindow(), *this ) ),
    pWrtShell(0),
    pShell(0),
    pFormShell(0),
    pHScrollbar(0),
    pVScrollbar(0),
    pScrollFill(new ScrollBarBox( &_pFrame->GetWindow(), _pFrame->GetFrame()->GetParentFrame() ? 0 : WB_SIZEABLE )),
    pHRuler( new SvxRuler(&GetViewFrame()->GetWindow(), pEditWin,
                    SVXRULER_SUPPORT_TABS |
                    SVXRULER_SUPPORT_PARAGRAPH_MARGINS |
                    SVXRULER_SUPPORT_BORDERS |
                    SVXRULER_SUPPORT_NEGATIVE_MARGINS|
                    SVXRULER_SUPPORT_REDUCED_METRIC,
                    GetViewFrame()->GetBindings(),
                    WB_STDRULER | WB_EXTRAFIELD | WB_BORDER)),
    pVRuler(new SvxRuler(&GetViewFrame()->GetWindow(), pEditWin,
                            SVXRULER_SUPPORT_TABS | SVXRULER_SUPPORT_PARAGRAPH_MARGINS_VERTICAL|
                                SVXRULER_SUPPORT_BORDERS | SVXRULER_SUPPORT_REDUCED_METRIC,
                            GetViewFrame()->GetBindings(),
                            WB_VSCROLL |  WB_EXTRAFIELD | WB_BORDER )),
    pTogglePageBtn(0),
    pPageUpBtn(0),
    pPageDownBtn(0),
    pNaviBtn(0),
    pGlosHdl(0),
    pDrawActual(0),
    pLastTableFormat(0),
    pFormatClipboard(new SwFormatClipboard()),
    mpPostItMgr(0),
    nSelectionType( INT_MAX ),
    nDrawSfxId( USHRT_MAX ),
    nFormSfxId( USHRT_MAX ),
    nLastPasteDestination( 0xFFFF ),
    nLeftBorderDistance( 0 ),
    nRightBorderDistance( 0 ),
    bInMailMerge(FALSE),
    bInDtor(FALSE),
    bOldShellWasPagePreView(FALSE)
{
    // OD 18.12.2002 #103492# - According to discussion with MBA and further
    // investigations, no old SfxViewShell will be set as parameter <pOldSh>,
    // if function "New Window" is performed to open an additional view beside
    // an already existing one.
    // If the view is switch from one to another, the 'old' view is given by
    // parameter <pOldSh>.

    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwView::SwView" );

    bCenterCrsr = bTopCrsr = bAllwaysShowSel = bTabColFromDoc = bTabRowFromDoc =
    bSetTabColFromDoc = bSetTabRowFromDoc = bAttrChgNotified = bAttrChgNotifiedWithRegistrations =
    bVerbsActive = bDrawRotate = bInOuterResizePixel = bInInnerResizePixel =
    bPasteState = bPasteSpecialState = bMakeSelectionVisible = sal_False;

    bShowAtResize = bDrawSelMode = bDocSzUpdated = sal_True;

    _CreateScrollbar( TRUE );
    _CreateScrollbar( FALSE );

    pViewImpl = new SwView_Impl(this);
    SetName(C2S("View"));
    SetWindow( pEditWin );

    aTimer.SetTimeout( 120 );

    SwDocShell* pDocSh = PTR_CAST( SwDocShell, _pFrame->GetObjectShell() );
    BOOL bOldModifyFlag = pDocSh->IsEnableSetModified();
    if(bOldModifyFlag)
        pDocSh->EnableSetModified( sal_False );
    ASSERT( pDocSh, "View ohne DocShell." );
    SwWebDocShell* pWebDShell = PTR_CAST( SwWebDocShell, pDocSh );

    const SwMasterUsrPref *pUsrPref = SW_MOD()->GetUsrPref(0 != pWebDShell);
    SwViewOption aUsrPref( *pUsrPref);

    //! get lingu options without loading lingu DLL
    SvtLinguOptions aLinguOpt;

    // #107253# Replaced SvtLinguConfig with SwLinguConfig wrapper with UsageCount
    SwLinguConfig().GetOptions( aLinguOpt );

    aUsrPref.SetOnlineSpell( aLinguOpt.bIsSpellAuto );
    aUsrPref.SetHideSpell( aLinguOpt.bIsSpellHideMarkings );

    sal_Bool bOldShellWasSrcView = FALSE;

    // OD 18.12.2002 #103492# - determine, if there is an existing view for
    // document
    SfxViewShell* pExistingSh = 0;
    if ( pOldSh )
        pExistingSh = pOldSh;
    else
    {
        SfxViewFrame *pF = SfxViewFrame::GetFirst( pDocSh );
        if( pF == _pFrame )
            pF = SfxViewFrame::GetNext( *pF, pDocSh );
        if( pF )
            pExistingSh = pF->GetViewShell();
    }

    // determine type of existing view
    if( pExistingSh &&
        pExistingSh->IsA( TYPE( SwPagePreView ) ) )
    {
        sSwViewData = ((SwPagePreView*)pExistingSh)->GetPrevSwViewData();
        sNewCrsrPos = ((SwPagePreView*)pExistingSh)->GetNewCrsrPos();
        nNewPage = ((SwPagePreView*)pExistingSh)->GetNewPage();
        bOldShellWasPagePreView = TRUE;
    }
    else if( pExistingSh &&
             pExistingSh->IsA( TYPE( SwSrcView ) ) )
        bOldShellWasSrcView = TRUE;

    RTL_LOGFILE_CONTEXT_TRACE( aLog, "before create WrtShell" );
    if(PTR_CAST( SwView, pExistingSh))
    {
        pWrtShell = new SwWrtShell( *((SwView*)pExistingSh)->pWrtShell,
                                    pEditWin, *this);
//MA: Das kann doch nur zu einem GPF fuehren!
//      nSelectionType = ((SwView*)pOldSh)->nSelectionType;
    }
    else
    {
        SwDoc& rDoc = *((SwDocShell*)pDocSh)->GetDoc();

        if( !bOldShellWasSrcView && pWebDShell && !bOldShellWasPagePreView )
            rDoc.set(IDocumentSettingAccess::BROWSE_MODE, true);

        //Fuer den BrowseMode wollen wir keinen Factor uebernehmen.
        if( rDoc.get(IDocumentSettingAccess::BROWSE_MODE) && aUsrPref.GetZoomType() != SVX_ZOOM_PERCENT )
        {
            aUsrPref.SetZoomType( SVX_ZOOM_PERCENT );
            aUsrPref.SetZoom( 100 );
        }
        if(pDocSh->IsPreview())
            aUsrPref.SetZoomType( SVX_ZOOM_WHOLEPAGE );
        pWrtShell = new SwWrtShell( rDoc, pEditWin, *this, &aUsrPref );
        //#97610# creating an SwView from a SwPagePreView needs to
        // add the ViewShell to the ring of the other ViewShell(s)
        if(bOldShellWasPagePreView)
        {
            ViewShell& rPreviewViewShell = *((SwPagePreView*)pExistingSh)->GetViewShell();
            pWrtShell->MoveTo(&rPreviewViewShell);
            //#95521# to update the field command et.al. if necessary
            const SwViewOption* pPreViewOpt = rPreviewViewShell.GetViewOptions();
            if( pPreViewOpt->IsFldName() != aUsrPref.IsFldName() ||
                    pPreViewOpt->IsShowHiddenField() != aUsrPref.IsShowHiddenField() ||
                    pPreViewOpt->IsShowHiddenPara() != aUsrPref.IsShowHiddenPara() ||
                    pPreViewOpt->IsShowHiddenChar() != aUsrPref.IsShowHiddenChar() )
                rPreviewViewShell.ApplyViewOptions(aUsrPref);
            // OD 09.01.2003 #106334# - reset design mode at draw view for form
            // shell, if needed.
            if ( ((SwPagePreView*)pExistingSh)->ResetFormDesignMode() &&
                 pWrtShell->HasDrawView() )
            {
                SdrView* pDrawView = pWrtShell->GetDrawView();
                pDrawView->SetDesignMode( ((SwPagePreView*)pExistingSh)->FormDesignModeToReset() );
            }
        }
    }
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "after create WrtShell" );

    // --> OD 2005-02-11 #i38810# - assure that modified state of document
    // isn't reset, if document is already modified.
    const bool bIsDocModified = pWrtShell->GetDoc()->IsModified();
    // <--

    // JP 05.02.99: Bug 61495 - damit unter anderem das HLineal im
    //              ReadonlyFall nicht angezeigt wird
    aUsrPref.SetReadonly( pWrtShell->GetViewOptions()->IsReadonly() );

    //Kein Margin fuer OLE!
    Size aBrwsBorder;
    if( SFX_CREATE_MODE_EMBEDDED != pDocSh->GetCreateMode() )
        aBrwsBorder = GetMargin();

    if( _pFrame->GetFrameType() & SFXFRAME_INTERNAL )
        pWrtShell->SetFrameView( aBrwsBorder );
    else
        pWrtShell->SetBrowseBorder( aBrwsBorder );

    // Im CTOR duerfen keine Shell wechsel erfolgen, die muessen ueber
    // den Timer "zwischen gespeichert" werden. Sonst raeumt der SFX
    // sie wieder vom Stack!
    sal_Bool bOld = bNoInterrupt;
    bNoInterrupt = sal_True;

    pHRuler->SetActive( sal_True );
    pVRuler->SetActive( sal_True );

    SfxViewFrame* pViewFrame = GetViewFrame();
    if( pViewFrame->GetFrame()->GetParentFrame())
    {
        aUsrPref.SetViewHRuler(sal_False);
        aUsrPref.SetViewVRuler(sal_False);
    }

    StartListening( *pViewFrame, TRUE );
    StartListening( *pDocSh, TRUE );

    // Vom HLineal den ZOOM-Faktor einstellen
    Fraction aZoomFract( aUsrPref.GetZoom(), 100 );
    pHRuler->SetZoom( aZoomFract );
    pVRuler->SetZoom( aZoomFract );
    pHRuler->SetDoubleClickHdl(LINK( this, SwView, ExecRulerClick ));
    FieldUnit eMetric = pUsrPref->GetHScrollMetric();
    pHRuler->SetUnit( eMetric );
    eMetric = pUsrPref->GetVScrollMetric();
    pVRuler->SetUnit( eMetric );

    // DocShell setzen
    pDocSh->SetView( this );
    SW_MOD()->SetView( this );

    mpPostItMgr = new SwPostItMgr(this);

    // Die DocSize erfragen und verarbeiten. Ueber die Handler konnte
    // die Shell nicht gefunden werden, weil die Shell innerhalb CTOR-Phase
    // nicht in der SFX-Verwaltung bekannt ist.
    DocSzChgd( pWrtShell->GetDocSize() );

        // AttrChangedNotify Link setzen
    pWrtShell->SetChgLnk(LINK(this, SwView, AttrChangedNotify));

    if( pDocSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED &&
        //TODO/LATER: why a cast here?
        //!((SvEmbeddedObject *)pDocSh)->GetVisArea().IsEmpty() )
        //SetVisArea( ((SvEmbeddedObject *)pDocSh)->GetVisArea(),sal_False);
        !pDocSh->GetVisArea(ASPECT_CONTENT).IsEmpty() )
        SetVisArea( pDocSh->GetVisArea(ASPECT_CONTENT),sal_False);

    SwEditShell::SetUndoActionCount(
        static_cast< USHORT >( SW_MOD()->GetUndoOptions().GetUndoCount() ) );
    pWrtShell->DoUndo( 0 != SwEditShell::GetUndoActionCount() );

    const BOOL bBrowse = pWrtShell->getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE);
    SetNewWindowAllowed(!bBrowse);

    ShowVScrollbar(aUsrPref.IsViewVScrollBar());
    ShowHScrollbar(aUsrPref.IsViewHScrollBar());
    pHScrollbar->SetAuto(bBrowse);
    if( aUsrPref.IsViewHRuler() )
        CreateTab();
    if( aUsrPref.IsViewVRuler() )
        CreateVLineal();

    pWrtShell->SetUIOptions( aUsrPref );
    pWrtShell->SetReadOnlyAvailable( aUsrPref.IsCursorInProtectedArea() );
    pWrtShell->ApplyAccessiblityOptions(SW_MOD()->GetAccessibilityOptions());

    if( UseObjectSize() )
    {
        //Damit der Sfx _rechtzeitig weiss_, wie gross die sheet::Border sind.
        SvBorder aTmp;
        CalcAndSetBorderPixel( aTmp, sal_True );
    }

    if( pWrtShell->GetDoc()->IsUpdateExpFld() )
    {
        SET_CURR_SHELL( pWrtShell );
        pWrtShell->StartAction();
        pWrtShell->CalcLayout();
        pWrtShell->GetDoc()->UpdateFlds(NULL, false);
        pWrtShell->EndAction();
        pWrtShell->GetDoc()->SetUpdateExpFldStat( sal_False );
    }

    // ggfs. alle Verzeichnisse updaten:
    if( pWrtShell->GetDoc()->IsUpdateTOX() )
    {
        SfxRequest aSfxRequest( FN_UPDATE_TOX, SFX_CALLMODE_SLOT, GetPool() );
        Execute( aSfxRequest );
        pWrtShell->GetDoc()->SetUpdateTOX( sal_False );     // wieder zurueck setzen
        pWrtShell->SttEndDoc(TRUE);
    }

    // kein ResetModified, wenn es schone eine View auf dieses Doc gibt
    SfxViewFrame* pVFrame = GetViewFrame();
    SfxViewFrame* pFirst = SfxViewFrame::GetFirst(pDocSh);
    // zur Zeit(360) wird die View erst nach dem Ctor eingetragen
    // der folgende Ausdruck funktioniert auch, wenn sich das aendert
    //JP 27.07.98: wenn per Undo nicht mehr die Modifizierung aufhebar ist,
    //              so setze das Modified NICHT zurueck.
    // --> OD 2005-02-11 #i38810# - no reset of modified state, if document
    // was already modified.
    if ( !pWrtShell->GetDoc()->IsUndoNoResetModified() &&
         ( !pFirst || pFirst == pVFrame ) &&
         !bIsDocModified )
    // <--
    {
        pWrtShell->ResetModified();
    }

    bNoInterrupt = bOld;

    // wird ein GlobalDoc neu angelegt, soll auch der Navigator erzeugt werden
    if( pDocSh->IsA(SwGlobalDocShell::StaticType()) &&
        !pVFrame->GetChildWindow( SID_NAVIGATOR ))
    {
        SfxBoolItem aNavi(SID_NAVIGATOR, sal_True);
        GetDispatcher().Execute(SID_NAVIGATOR, SFX_CALLMODE_ASYNCHRON, &aNavi, 0L);
    }


    /*uno::Reference< awt::XWindow >  aTmpRef;
    _pFrame->GetFrame()->GetFrameInterface()->setComponent( aTmpRef,
                                            pViewImpl->GetUNOObject_Impl());*/

   uno::Reference< frame::XFrame >  xFrame = pVFrame->GetFrame()->GetFrameInterface();

    uno::Reference< frame::XFrame >  xBeamerFrame = xFrame->findFrame(
            OUString::createFromAscii("_beamer"), frame::FrameSearchFlag::CHILDREN);
    if(xBeamerFrame.is())
    {
        SwDBData aData = pWrtShell->GetDBData();
        SW_MOD()->ShowDBObj( *this, aData );
    }

    // has anybody calls the attrchanged handler in the constructor?
    if( bAttrChgNotifiedWithRegistrations )
    {
        GetViewFrame()->GetBindings().LEAVEREGISTRATIONS();
        if( aTimer.IsActive() )
            aTimer.Stop();
    }

    aTimer.SetTimeoutHdl(LINK(this, SwView, TimeoutHdl));
    bAttrChgNotified = bAttrChgNotifiedWithRegistrations = sal_False;
    if(bOldModifyFlag)
        pDocSh->EnableSetModified( sal_True );
    InvalidateBorder();

}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


SwView::~SwView()
{
    delete mpPostItMgr;

    bInDtor = TRUE;
    pEditWin->Hide(); // damit kein Paint Aerger machen kann!
    // An der SwDocShell den Pointer auf die View ruecksetzen
    SwDocShell* pDocSh = GetDocShell();
    if( pDocSh && pDocSh->GetView() == this )
        pDocSh->SetView( 0 );
    if ( SW_MOD()->GetView() == this )
        SW_MOD()->SetView( 0 );

    if( aTimer.IsActive() && bAttrChgNotifiedWithRegistrations )
        GetViewFrame()->GetBindings().LEAVEREGISTRATIONS();

    //JP 27.11.00: Bug 80631 - the last view must end the text edit
    SdrView *pSdrView = pWrtShell ? pWrtShell->GetDrawView() : 0;
    if( pSdrView && pSdrView->IsTextEdit() )
        pSdrView->SdrEndTextEdit( sal_True );

    SetWindow( 0 );

    pViewImpl->Invalidate();
    EndListening(*GetViewFrame());
    EndListening(*GetDocShell());
    delete pScrollFill;
    delete pWrtShell;
    pWrtShell = 0;      // Auf 0 setzen, damit folgende DToren nicht drauf zugreifen
    pShell = 0;
    delete pHScrollbar;
    delete pVScrollbar;
    delete pHRuler;
    delete pVRuler;
    delete pTogglePageBtn;
    delete pPageUpBtn;
    delete pNaviBtn;
    delete pPageDownBtn;
    delete pGlosHdl;
    delete pViewImpl;
    delete pEditWin;
    delete pFormatClipboard;
}

/*--------------------------------------------------------------------
    Beschreibung:   DocShell rausgrabbeln ueber das FrameWindow
 --------------------------------------------------------------------*/


SwDocShell* SwView::GetDocShell()
{
    SfxObjectShell* pDocShell = GetViewFrame()->GetObjectShell();
    return PTR_CAST(SwDocShell, pDocShell);
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
//#i43146# go to the last editing position when opening own files
bool lcl_IsOwnDocument( SwView& rView )
{
    SfxDocumentInfo& rInfo = rView.GetDocShell()->GetDocInfo();
    const String& rCreated =   rInfo.GetAuthor();
    const String& rChanged = rInfo.GetModificationAuthor();
    const String& rFullName = SW_MOD()->GetUserOptions().GetFullName();
    bool bRet = rFullName.Len() &&
            (rChanged.Len() && rChanged == rFullName ) ||
            (!rChanged.Len() && rCreated.Len() && rCreated == rFullName );
    return bRet;
}


void SwView::ReadUserData( const String &rUserData, sal_Bool bBrowse )
{
    if ( rUserData.GetTokenCount() > 1 &&
        //Fuer Dokumente ohne Layout nur im OnlineLayout oder beim
        //Forward/Backward
         (!pWrtShell->IsNewLayout() || pWrtShell->getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE) || bBrowse) )
    {
        //#i43146# go to the last editing position when opening own files
        bool bIsOwnDocument = lcl_IsOwnDocument( *this );

        SET_CURR_SHELL(pWrtShell);

        sal_uInt16 nPos = 0;

        // Nein, es ist *keine* gute Idee GetToken gleich im Point-Konstr.
        // aufzurufen, denn welcher Parameter wird zuerst ausgewertet?
        long nX = rUserData.GetToken( 0, ';', nPos ).ToInt32(),
             nY = rUserData.GetToken( 0, ';', nPos ).ToInt32();
        Point aCrsrPos( nX, nY );

        sal_uInt16 nZoomFactor =
            static_cast< sal_uInt16 >( rUserData.GetToken(0, ';', nPos ).ToInt32() );

        long nLeft  = rUserData.GetToken(0, ';', nPos ).ToInt32(),
             nTop   = rUserData.GetToken(0, ';', nPos ).ToInt32(),
             nRight = rUserData.GetToken(0, ';', nPos ).ToInt32(),
             nBottom= rUserData.GetToken(0, ';', nPos ).ToInt32();

        const long nAdd = pWrtShell->getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE) ? DOCUMENTBORDER : DOCUMENTBORDER*2;
        if ( nBottom <= (pWrtShell->GetDocSize().Height()+nAdd) )
        {
            pWrtShell->EnableSmooth( sal_False );

            const Rectangle aVis( nLeft, nTop, nRight, nBottom );

            sal_uInt16 nOff = 0;
            SvxZoomType eZoom;
            if( !pWrtShell->getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE) )
                eZoom = (SvxZoomType) (sal_uInt16)rUserData.GetToken(nOff, ';', nPos ).ToInt32();
            else
            {
                eZoom = SVX_ZOOM_PERCENT;
                ++nOff;
            }

            sal_Bool bSelectObj = (0 != rUserData.GetToken( nOff, ';', nPos ).ToInt32())
                                && pWrtShell->IsObjSelectable( aCrsrPos );

            //#i33307# restore editing position
            pViewImpl->SetRestorePosition(aCrsrPos, bSelectObj);
            // OD 11.02.2003 #100556# - set flag value to avoid macro execution.
            bool bSavedFlagValue = pWrtShell->IsMacroExecAllowed();
            pWrtShell->SetMacroExecAllowed( false );
//!!! pb (11.08.2004): #i32536#
// os: changed: The user data has to be read if the view is switched back from page preview
//#i43146# go to the last editing position when opening own files
            if(bOldShellWasPagePreView || bIsOwnDocument)
            {
                pWrtShell->SwCrsrShell::SetCrsr( aCrsrPos, !bSelectObj );
                if( bSelectObj )
                {
                    pWrtShell->SelectObj( aCrsrPos );
                    pWrtShell->EnterSelFrmMode( &aCrsrPos );
                }
            }

            // OD 11.02.2003 #100556# - reset flag value
            pWrtShell->SetMacroExecAllowed( bSavedFlagValue );

            // OD 08.04.2003 #108693# - set visible area before applying
            // information from print preview. Otherwise, the applied information
            // is lost.
//!!! pb (11.08.2004): #i32536#
// os: changed: The user data has to be read if the view is switched back from page preview
//#i43146# go to the last editing position when opening own files
            if(bOldShellWasPagePreView || bIsOwnDocument )
            {
                if ( bBrowse )
                    SetVisArea( aVis.TopLeft() );
                else
                    SetVisArea( aVis );
            }

            //apply information from print preview - if available
            if( sNewCrsrPos.Len() )
            {
                long nXTmp = sNewCrsrPos.GetToken( 0, ';' ).ToInt32(),
                      nYTmp = sNewCrsrPos.GetToken( 1, ';' ).ToInt32();
                Point aCrsrPos2( nXTmp, nYTmp );
                bSelectObj = pWrtShell->IsObjSelectable( aCrsrPos2 );

                pWrtShell->SwCrsrShell::SetCrsr( aCrsrPos2, FALSE );
                if( bSelectObj )
                {
                    pWrtShell->SelectObj( aCrsrPos2 );
                    pWrtShell->EnterSelFrmMode( &aCrsrPos2 );
                }
                pWrtShell->MakeSelVisible();
                sNewCrsrPos.Erase();
            }
            else if(USHRT_MAX != nNewPage)
            {
                pWrtShell->GotoPage(nNewPage, TRUE);
                nNewPage = USHRT_MAX;
            }

            SelectShell();

            pWrtShell->StartAction();
            const SwViewOption* pVOpt = pWrtShell->GetViewOptions();
            if( pVOpt->GetZoom() != nZoomFactor || pVOpt->GetZoomType() != eZoom )
                SetZoom( eZoom, nZoomFactor);

            pWrtShell->LockView( sal_True );
            pWrtShell->EndAction();
            pWrtShell->LockView( sal_False );
            pWrtShell->EnableSmooth( sal_True );
        }
    }
}

void SwView::ReadUserDataSequence ( const uno::Sequence < beans::PropertyValue >& rSequence, sal_Bool bBrowse )
{
    if(GetDocShell()->IsPreview())
        return;
    //#i43146# go to the last editing position when opening own files
    bool bIsOwnDocument = lcl_IsOwnDocument( *this );
    sal_Int32 nLength = rSequence.getLength();
    if (nLength && (!pWrtShell->IsNewLayout() || pWrtShell->getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE) || bBrowse) )
    {
        SET_CURR_SHELL(pWrtShell);
        const beans::PropertyValue *pValue = rSequence.getConstArray();
        const SwRect& rRect = pWrtShell->GetCharRect();
        const Rectangle &rVis = GetVisArea();

        long nX = rRect.Left(), nY = rRect.Top(), nLeft = rVis.Left(), nTop = rVis.Top();
        long nRight = bBrowse ? LONG_MIN : rVis.Right(), nBottom = bBrowse ? LONG_MIN : rVis.Bottom();
        sal_Int16 nZoomType = static_cast< sal_Int16 >(pWrtShell->GetViewOptions()->GetZoomType());
        sal_Int16 nZoomFactor = static_cast < sal_Int16 > (pWrtShell->GetViewOptions()->GetZoom());
        sal_Bool bSelectedFrame = ( pWrtShell->GetSelFrmType() != FRMTYPE_NONE ),
                 bGotViewLeft = sal_False, bGotViewTop = sal_False, bGotVisibleLeft = sal_False,
                 bGotVisibleTop = sal_False, bGotVisibleRight = sal_False,
                 bGotVisibleBottom = sal_False, bGotZoomType = sal_False,
                 bGotZoomFactor = sal_False, bGotIsSelectedFrame = sal_False;

        for (sal_Int16 i = 0 ; i < nLength; i++)
        {
            if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ViewLeft" ) ) )
            {
               pValue->Value >>= nX;
               nX = MM100_TO_TWIP( nX );
               bGotViewLeft = sal_True;
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ViewTop" ) ) )
            {
               pValue->Value >>= nY;
               nY = MM100_TO_TWIP( nY );
               bGotViewTop = sal_True;
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "VisibleLeft" ) ) )
            {
               pValue->Value >>= nLeft;
               nLeft = MM100_TO_TWIP( nLeft );
               bGotVisibleLeft = sal_True;
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "VisibleTop" ) ) )
            {
               pValue->Value >>= nTop;
               nTop = MM100_TO_TWIP( nTop );
               bGotVisibleTop = sal_True;
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "VisibleRight" ) ) )
            {
               pValue->Value >>= nRight;
               nRight = MM100_TO_TWIP( nRight );
               bGotVisibleRight = sal_True;
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "VisibleBottom" ) ) )
            {
               pValue->Value >>= nBottom;
               nBottom = MM100_TO_TWIP( nBottom );
               bGotVisibleBottom = sal_True;
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ZoomType" ) ) )
            {
               pValue->Value >>= nZoomType;
               bGotZoomType = sal_True;
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ZoomFactor" ) ) )
            {
               pValue->Value >>= nZoomFactor;
               bGotZoomFactor = sal_True;
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "IsSelectedFrame" ) ) )
            {
               bSelectedFrame = * (sal_Bool *) pValue->Value.getValue();
               bGotIsSelectedFrame = sal_True;
            }
            pValue++;
        }
        if (bGotVisibleBottom)
        {
            Point aCrsrPos( nX, nY );
            const long nAdd = pWrtShell->getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE) ? DOCUMENTBORDER : DOCUMENTBORDER*2;
            if (nBottom <= (pWrtShell->GetDocSize().Height()+nAdd) )
            {
                pWrtShell->EnableSmooth( sal_False );
                const Rectangle aVis( nLeft, nTop, nRight, nBottom );

                sal_uInt16 nOff = 0;
                SvxZoomType eZoom;
                if ( !pWrtShell->getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE) )
                    eZoom = static_cast < SvxZoomType > ( nZoomType );
                else
                {
                    eZoom = SVX_ZOOM_PERCENT;
                    ++nOff;
                }
                if (bGotIsSelectedFrame)
                {
                    sal_Bool bSelectObj = (sal_False != bSelectedFrame )
                                        && pWrtShell->IsObjSelectable( aCrsrPos );

                    // OD 11.02.2003 #100556# - set flag value to avoid macro execution.
                    bool bSavedFlagValue = pWrtShell->IsMacroExecAllowed();
                    pWrtShell->SetMacroExecAllowed( false );
//!!! pb (11.08.2004): #i32536#
// os: changed: The user data has to be read if the view is switched back from page preview
//#i43146# go to the last editing position when opening own files
                    //#i33307# restore editing position
                    pViewImpl->SetRestorePosition(aCrsrPos, bSelectObj);
                    if(bOldShellWasPagePreView|| bIsOwnDocument)
                    {
                        pWrtShell->SwCrsrShell::SetCrsr( aCrsrPos, !bSelectObj );
                        if( bSelectObj )
                        {
                            pWrtShell->SelectObj( aCrsrPos );
                            pWrtShell->EnterSelFrmMode( &aCrsrPos );
                        }
                    }

                    // OD 11.02.2003 #100556# - reset flag value
                    pWrtShell->SetMacroExecAllowed( bSavedFlagValue );
                }
                SelectShell();

                pWrtShell->StartAction();
                const SwViewOption* pVOpt = pWrtShell->GetViewOptions();
                if ( bGotZoomType && bGotZoomFactor &&
                   ( pVOpt->GetZoom() != nZoomFactor || pVOpt->GetZoomType() != eZoom ) )
                    SetZoom( eZoom, nZoomFactor, sal_True );
//!!! pb (11.08.2004): #i32536#
// os: changed: The user data has to be read if the view is switched back from page preview
//#i43146# go to the last editing position when opening own files
                if(bOldShellWasPagePreView||bIsOwnDocument)
                {
                    if ( bBrowse && bGotVisibleLeft && bGotVisibleTop )
                    {
                        Point aTopLeft(aVis.TopLeft());
                        //#i76699# make sure the document is still centered
                        const SwTwips lBorder = IsDocumentBorder() ? DOCUMENTBORDER : 2 * DOCUMENTBORDER;
                        SwTwips nEditWidth = GetEditWin().GetOutputSize().Width();
                        if(nEditWidth > (aDocSz.Width() + lBorder ))
                            aTopLeft.X() = ( aDocSz.Width() + lBorder - nEditWidth  ) / 2;
                        else
                        {
                            //check if the values are possible
                            long nXMax = pHScrollbar->GetRangeMax() - pHScrollbar->GetVisibleSize();
                            if( aTopLeft.X() > nXMax )
                                aTopLeft.X() = nXMax < 0 ? 0 : nXMax;
                        }
                        SetVisArea( aTopLeft );
                    }
                    else if (bGotVisibleLeft && bGotVisibleTop && bGotVisibleRight && bGotVisibleBottom )
                        SetVisArea( aVis );
                }

                pWrtShell->LockView( sal_True );
                pWrtShell->EndAction();
                pWrtShell->LockView( sal_False );
                pWrtShell->EnableSmooth( sal_True );
            }
        }
    }
}
#define NUM_VIEW_SETTINGS 10
void SwView::WriteUserDataSequence ( uno::Sequence < beans::PropertyValue >& rSequence, sal_Bool bBrowse )
{
    const SwRect& rRect = pWrtShell->GetCharRect();
    const Rectangle& rVis = GetVisArea();
    Any aAny;

    rSequence.realloc ( NUM_VIEW_SETTINGS );
    sal_Int16 nIndex = 0;
    beans::PropertyValue *pValue = rSequence.getArray();

    sal_uInt16 nViewID( GetViewFrame()->GetCurViewId());
    pValue->Name = rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM( "ViewId" ) );
    OUStringBuffer sBuffer ( OUString(RTL_CONSTASCII_USTRINGPARAM( "view" ) ) );
    SvXMLUnitConverter::convertNumber(sBuffer, static_cast<sal_Int32>(nViewID));
    pValue->Value <<= sBuffer.makeStringAndClear();
    pValue++;nIndex++;

    pValue->Name = OUString ( RTL_CONSTASCII_USTRINGPARAM ( "ViewLeft" ) );
    pValue->Value <<= TWIP_TO_MM100 ( rRect.Left() );
    pValue++;nIndex++;

    pValue->Name = OUString ( RTL_CONSTASCII_USTRINGPARAM ( "ViewTop" ) );
    pValue->Value <<= TWIP_TO_MM100 ( rRect.Top() );
    pValue++;nIndex++;

    pValue->Name = OUString ( RTL_CONSTASCII_USTRINGPARAM ( "VisibleLeft" ) );
    pValue->Value <<= TWIP_TO_MM100 ( rVis.Left() );
    pValue++;nIndex++;

    pValue->Name = OUString ( RTL_CONSTASCII_USTRINGPARAM ( "VisibleTop" ) );
    pValue->Value <<= TWIP_TO_MM100 ( rVis.Top() );
    pValue++;nIndex++;

    pValue->Name = OUString ( RTL_CONSTASCII_USTRINGPARAM ( "VisibleRight" ) );
    pValue->Value <<= TWIP_TO_MM100 ( bBrowse ? LONG_MIN : rVis.Right() );
    pValue++;nIndex++;

    pValue->Name = OUString ( RTL_CONSTASCII_USTRINGPARAM ( "VisibleBottom" ) );
    pValue->Value <<= TWIP_TO_MM100 ( bBrowse ? LONG_MIN : rVis.Bottom() );
    pValue++;nIndex++;

    sal_Int16 nInt16 = static_cast< sal_Int16 >(pWrtShell->GetViewOptions()->GetZoomType());
    pValue->Name = OUString ( RTL_CONSTASCII_USTRINGPARAM ( "ZoomType" ) );
    pValue->Value <<= nInt16;
    pValue++;nIndex++;

    pValue->Name = OUString ( RTL_CONSTASCII_USTRINGPARAM ( "ZoomFactor" ) );
    pValue->Value <<= static_cast < sal_Int16 > (pWrtShell->GetViewOptions()->GetZoom());
    pValue++;nIndex++;

    pValue->Name = OUString ( RTL_CONSTASCII_USTRINGPARAM ( "IsSelectedFrame" ) );
    sal_Bool bIsSelected = FRMTYPE_NONE == pWrtShell->GetSelFrmType() ? sal_False : sal_True;
    pValue->Value.setValue ( &bIsSelected, ::getBooleanCppuType() );
    nIndex++;

    if ( nIndex < NUM_VIEW_SETTINGS )
        rSequence.realloc ( nIndex );
}
#undef NUM_VIEW_SETTINGS



void SwView::ShowCursor( FASTBOOL bOn )
{
    //JP 10.10.2001: Bug 90461 - don't scroll the cursor into the visible area
    BOOL bUnlockView = !pWrtShell->IsViewLocked();
    pWrtShell->LockView( TRUE );    //lock visible section

    if( !bOn )
        pWrtShell->HideCrsr();
    else if( !pWrtShell->IsFrmSelected() && !pWrtShell->IsObjSelected() )
        pWrtShell->ShowCrsr();

    if( bUnlockView )
        pWrtShell->LockView( FALSE );
}



ErrCode SwView::DoVerb( long nVerb )
{
    if ( !GetViewFrame()->GetFrame()->IsInPlace() )
    {
        SwWrtShell &rSh = GetWrtShell();
        const int nSel = rSh.GetSelectionType();
        if ( nSel & nsSelectionType::SEL_OLE )
            rSh.LaunchOLEObj( nVerb );
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

String SwView::GetSelectionText( sal_Bool bCompleteWrds )
{
    return GetSelectionTextParam( bCompleteWrds, sal_True );
}

/*-----------------09/16/97 09:50am-----------------

--------------------------------------------------*/
String  SwView::GetSelectionTextParam( sal_Bool bCompleteWrds,
                                       sal_Bool bEraseTrail )
{
    String sReturn;
    if( bCompleteWrds && !GetWrtShell().HasSelection() )
        GetWrtShell().SelWrd();

    GetWrtShell().GetSelectedText( sReturn );
    if( bEraseTrail )
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
    BOOL bCallBase = sal_True;
    if ( rHint.ISA(SfxSimpleHint) )
    {
        sal_uInt32 nId = ((SfxSimpleHint&)rHint).GetId();
        switch ( nId )
        {
            // --> OD 2005-03-03 #i43775# - sub shells will be destroyed by the
            // dispatcher, if the view frame is dying. Thus, reset member <pShell>.
            case SFX_HINT_DYING:
                {
                    if ( &rBC == GetViewFrame() )
                    {
                        ResetSubShell();
                    }
                }
                break;
            // <--
            case SFX_HINT_MODECHANGED:
                {
                    // Modalmodus-Umschaltung?
                    sal_Bool bModal = GetDocShell()->IsInModalMode();
                    pHRuler->SetActive( !bModal );
                    pVRuler->SetActive( !bModal );
                }

                /* kein break hier */

            case SFX_HINT_TITLECHANGED:
                if ( GetDocShell()->IsReadOnly() != GetWrtShell().GetViewOptions()->IsReadonly() )
                {
                    SwWrtShell &rSh = GetWrtShell();
                    rSh.SetReadonlyOption( GetDocShell()->IsReadOnly() );

                    if ( rSh.GetViewOptions()->IsViewVRuler() )
                        CreateVLineal();
                    else
                        KillVLineal();
                    if ( rSh.GetViewOptions()->IsViewHRuler() )
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
                        GetDispatcher().Execute( SID_FM_DESIGN_MODE, SFX_CALLMODE_SYNCHRON,
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

void SwView::ScannerEventHdl( const EventObject& /*rEventObject*/ )
{
    uno::Reference< XScannerManager > xScanMgr = SW_MOD()->GetScannerManager();
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

/*-----------------09/03/97 04:12pm-----------------

--------------------------------------------------*/
sal_uInt16  SwView::PrepareClose( sal_Bool bUI, sal_Bool bForBrowsing )
{
    SfxViewFrame* pVFrame = GetViewFrame();
    pVFrame->SetChildWindow( SwInputChild::GetChildWindowId(), sal_False );
    if( pVFrame->GetDispatcher()->IsLocked() )
        pVFrame->GetDispatcher()->Lock(sal_False);

    sal_uInt16 nRet;
    if ( pFormShell &&
         sal_True != (nRet = pFormShell->PrepareClose( bUI, bForBrowsing )) )

    {
        return nRet;
    }
    return SfxViewShell::PrepareClose( bUI, bForBrowsing );
}



    // status methods for clipboard.
    // Status changes now notified from the clipboard.
BOOL SwView::IsPasteAllowed()
{
    USHORT nPasteDestination = SwTransferable::GetSotDestination( *pWrtShell );
    if( nLastPasteDestination != nPasteDestination )
    {
        TransferableDataHelper aDataHelper(
                        TransferableDataHelper::CreateFromSystemClipboard(
                                                        &GetEditWin()) );
        if( aDataHelper.GetXTransferable().is() )
        {
            bPasteState = SwTransferable::IsPaste( *pWrtShell, aDataHelper );
            bPasteSpecialState = SwTransferable::IsPasteSpecial(
                                                    *pWrtShell, aDataHelper );
        }
        else
            bPasteState = bPasteSpecialState = FALSE;

        if( 0xFFFF == nLastPasteDestination )  // the init value
            pViewImpl->AddClipboardListener();
        nLastPasteDestination = nPasteDestination;
    }
    return bPasteState;
}

BOOL SwView::IsPasteSpecialAllowed()
{
    if ( pFormShell && pFormShell->IsActiveControl() )
        return FALSE;

    USHORT nPasteDestination = SwTransferable::GetSotDestination( *pWrtShell );
    if( nLastPasteDestination != nPasteDestination )
    {
        TransferableDataHelper aDataHelper(
                        TransferableDataHelper::CreateFromSystemClipboard(
                                                        &GetEditWin()) );
        if( aDataHelper.GetXTransferable().is() )
        {
            bPasteState = SwTransferable::IsPaste( *pWrtShell, aDataHelper );
            bPasteSpecialState = SwTransferable::IsPasteSpecial(
                                                    *pWrtShell, aDataHelper );
        }
        else
            bPasteState = bPasteSpecialState = FALSE;

        if( 0xFFFF == nLastPasteDestination )  // the init value
            pViewImpl->AddClipboardListener();
    }
    return bPasteSpecialState;
}
/* -----------------------------12.07.01 13:25--------------------------------

 ---------------------------------------------------------------------------*/
void SwView::NotifyDBChanged()
{
    GetViewImpl()->GetUNOObject_Impl()->NotifyDBChanged();
}

/*--------------------------------------------------------------------
    Beschreibung:   Drucken
 --------------------------------------------------------------------*/

void SwView::MakeOptions( PrintDialog* pDlg, SwPrtOptions& rOpts,
         BOOL* pPrtProspect, BOOL* pPrtProspect_RTL, BOOL bWeb, SfxPrinter* pPrt, SwPrintData* pData )
{
    SwAddPrinterItem* pAddPrinterAttr;
    if( pPrt && SFX_ITEM_SET == pPrt->GetOptions().GetItemState(
        FN_PARAM_ADDPRINTER, FALSE, (const SfxPoolItem**)&pAddPrinterAttr ))
    {
        pData = pAddPrinterAttr;
    }
    else if(!pData)
    {
        pData = SW_MOD()->GetPrtOptions(bWeb);
    }
    rOpts = *pData;
    if( pPrtProspect )
        *pPrtProspect = pData->bPrintProspect;
    if( pPrtProspect_RTL )
        *pPrtProspect_RTL = pData->bPrintProspect_RTL;
    rOpts.aMulti.SetTotalRange( Range( 0, RANGE_MAX ) );
    rOpts.aMulti.SelectAll( FALSE );
    rOpts.nCopyCount = 1;
    rOpts.bCollate = FALSE;
    rOpts.bPrintSelection = FALSE;
    rOpts.bJobStartet = FALSE;

    if ( pDlg )
    {
        rOpts.nCopyCount = pDlg->GetCopyCount();
        rOpts.bCollate = pDlg->IsCollateChecked();
        if ( pDlg->GetCheckedRange() == PRINTDIALOG_SELECTION )
        {
            rOpts.aMulti.SelectAll();
            rOpts.bPrintSelection = TRUE;
        }
        else if ( PRINTDIALOG_ALL == pDlg->GetCheckedRange() )
            rOpts.aMulti.SelectAll();
        else
        {
            rOpts.aMulti = MultiSelection( pDlg->GetRangeText() );
            rOpts.aMulti.SetTotalRange( Range( 0, RANGE_MAX ) );
        }
    }
    else
        rOpts.aMulti.SelectAll();
    rOpts.aMulti.Select( 0, FALSE );
}

/* -----------------------------28.10.02 13:25--------------------------------

 ---------------------------------------------------------------------------*/
SfxObjectShellRef & SwView::GetTmpSelectionDoc()
{
    return GetViewImpl()->GetTmpSelectionDoc();
}
/* -----------------------------31.10.02 13:25--------------------------------

 ---------------------------------------------------------------------------*/
SfxObjectShellRef & SwView::GetOrCreateTmpSelectionDoc()
{
    SfxObjectShellRef &rxTmpDoc = GetViewImpl()->GetTmpSelectionDoc();
    if (!rxTmpDoc.Is())
    {
        SwXTextView *pImpl = GetViewImpl()->GetUNOObject_Impl();
        rxTmpDoc = pImpl->BuildTmpSelectionDoc(
                    GetViewImpl()->GetEmbeddedObjRef() );
    }
    return rxTmpDoc;
}
/* -----------------3/31/2003 12:39PM----------------

 --------------------------------------------------*/
void SwView::AddTransferable(SwTransferable& rTransferable)
{
    GetViewImpl()->AddTransferable(rTransferable);
}
