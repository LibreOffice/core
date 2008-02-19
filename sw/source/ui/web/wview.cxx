/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: wview.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-19 14:01:11 $
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


#include <itemdef.hxx>

#ifndef _SVX_SRCHITEM_HXX
#include <svx/srchitem.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFX_TEMPLDLG_HXX //autogen
#include <sfx2/templdlg.hxx>
#endif
#ifndef _SVX_SRCHDLG_HXX //autogen
#include <svx/srchdlg.hxx>
#endif
#ifndef _SBX_SBXOBJECT_HXX //autogen
#include <basic/sbxobj.hxx>
#endif
#ifndef _UIVWIMP_HXX
#include <uivwimp.hxx>
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

#include <sfx2/objface.hxx>

#ifndef _SWMODULE_HXX //autogen
#include <swmodule.hxx>
#endif
#ifndef _UNOTXVW_HXX //autogen
#include <unotxvw.hxx>
#endif
#include <swtypes.hxx>
#include <cmdid.h>
#include <globals.hrc>
#include <wrtsh.hxx>
#include <edtwin.hxx>
#include <wgrfsh.hxx>
#include <wfrmsh.hxx>
#include <wolesh.hxx>
#include <wtabsh.hxx>
#include <wlistsh.hxx>
#include <wformsh.hxx>
#include <wtextsh.hxx>
#include <barcfg.hxx>
#include <doc.hxx>

// EIGENTLICH nicht moeglich !!
#include <beziersh.hxx>
#include <drawsh.hxx>
#include <drwtxtsh.hxx>
#include <annotsh.hxx>

#include <wview.hxx>
#include <wdocsh.hxx>
#include <web.hrc>
#include <shells.hrc>

#define SwWebView
#define SearchAttributes
#define ReplaceAttributes
#define SearchSettings
#define _ExecSearch ExecSearch
#define _StateSearch StateSearch
#define Frames
#define Graphics
#define OLEObjects
#define Controls
#define Text
#define Frame
#define Graphic
#define Object
#define Draw
#define DrawText
#define TextInTable
#define ListInText
#define ListInTable
#define Page
#include <svx/svxslots.hxx>
#include <swslots.hxx>


SFX_IMPL_VIEWFACTORY(SwWebView, SW_RES(STR_NONAME))
{
    SFX_VIEW_REGISTRATION(SwWebDocShell);
}


SFX_IMPL_INTERFACE( SwWebView, SwView, SW_RES(RID_WEBTOOLS_TOOLBOX) )
{
    SFX_CHILDWINDOW_REGISTRATION(SfxTemplateDialogWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(SvxSearchDialogWrapper::GetChildWindowId());
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_TOOLS|
                                SFX_VISIBILITY_STANDARD|SFX_VISIBILITY_SERVER,
                                SW_RES(RID_WEBTOOLS_TOOLBOX) );
}

TYPEINIT1(SwWebView,SwView)

/*-----------------22.01.97 14.27-------------------

--------------------------------------------------*/


SwWebView::SwWebView(SfxViewFrame* _pFrame, SfxViewShell* _pShell) :
    SwView(_pFrame, _pShell)
{
}

/*-----------------22.01.97 14.27-------------------

--------------------------------------------------*/


SwWebView::~SwWebView()
{
}

/*-----------------23.01.97 09.01-------------------

--------------------------------------------------*/


void SwWebView::SelectShell()
{
    // Entscheidung, ob UpdateTable gerufen werden muss
    BOOL bUpdateTable = FALSE;
    const SwFrmFmt* pCurTableFmt = GetWrtShell().GetTableFmt();
    if(pCurTableFmt && pCurTableFmt != GetLastTblFrmFmt())
    {
        bUpdateTable = TRUE; // kann erst spaeter ausgefuehrt werden
    }
    SetLastTblFrmFmt(pCurTableFmt);
    //SEL_TBL und SEL_TBL_CELLS koennen verodert sein!
    int nNewSelectionType = (GetWrtShell().GetSelectionType()
                                & ~nsSelectionType::SEL_TBL_CELLS);

    int _nSelectionType = GetSelectionType();
    if ( nNewSelectionType == _nSelectionType )
    {
        GetViewFrame()->GetBindings().InvalidateAll( FALSE );
        if ( _nSelectionType & nsSelectionType::SEL_OLE ||
             _nSelectionType & nsSelectionType::SEL_GRF )
            //Fuer Grafiken und OLE kann sich natuerlich das Verb aendern!
            ImpSetVerb( nNewSelectionType );
    }
    else
    {

    //  DELETEZ(pxSelectionObj); //Selektionsobjekt loeschen
        SfxDispatcher &rDispatcher = *GetViewFrame()->GetDispatcher();
        SwToolbarConfigItem *pBarCfg = SW_MOD()->GetWebToolbarConfig();

        if( GetCurShell() )
        {
            rDispatcher.Flush();        // alle gecachten Shells wirklich loeschen

            //Zur alten Selektion merken welche Toolbar sichtbar war
            sal_Int32 nId = rDispatcher.GetObjectBarId( SFX_OBJECTBAR_OBJECT );
            if ( nId )
                pBarCfg->SetTopToolbar( _nSelectionType, nId );

            SfxShell *pSfxShell;
            USHORT i;
            for ( i = 0; TRUE; ++i )
            {
                pSfxShell = rDispatcher.GetShell( i );
                if ( !( pSfxShell->ISA( SwBaseShell ) ||
                    pSfxShell->ISA( SwDrawTextShell ) || pSfxShell->ISA( SwAnnotationShell ) ) )
                    break;
            }
            pSfxShell = rDispatcher.GetShell( --i );
            ASSERT( pSfxShell, "My Shell ist lost in space" );
            rDispatcher.Pop( *pSfxShell, SFX_SHELL_POP_UNTIL | SFX_SHELL_POP_DELETE);
        }

        BOOL bInitFormShell = FALSE;
        if( !GetFormShell() )
        {
            bInitFormShell = TRUE;
            SetFormShell( new FmFormShell( this ) );
            rDispatcher.Push( *GetFormShell() );
        }

        BOOL bSetExtInpCntxt = FALSE;
        _nSelectionType = nNewSelectionType;
        SetSelectionType( _nSelectionType );
        ShellModes eShellMode;

        if ( _nSelectionType & nsSelectionType::SEL_OLE )
        {
            eShellMode = SHELL_MODE_OBJECT;
            SetShell( new SwWebOleShell( *this ));
            rDispatcher.Push( *GetCurShell() );
        }
        else if ( _nSelectionType & nsSelectionType::SEL_FRM
            || _nSelectionType & nsSelectionType::SEL_GRF)
        {
            eShellMode = SHELL_MODE_FRAME;
            SetShell( new SwWebFrameShell( *this ));
            rDispatcher.Push( *GetCurShell() );
            if(_nSelectionType & nsSelectionType::SEL_GRF )
            {
                eShellMode = SHELL_MODE_GRAPHIC;
                SetShell( new SwWebGrfShell( *this ));
                rDispatcher.Push( *GetCurShell() );
            }
        }
        else if ( _nSelectionType & nsSelectionType::SEL_FRM )
        {
            eShellMode = SHELL_MODE_FRAME;
            SetShell( new SwWebFrameShell( *this ));
            rDispatcher.Push( *GetCurShell() );
        }
        else if ( _nSelectionType & nsSelectionType::SEL_DRW )
        {
            eShellMode = SHELL_MODE_DRAW;
            SetShell( new svx::ExtrusionBar( this ) );
            rDispatcher.Push( *GetCurShell() );

            eShellMode = SHELL_MODE_DRAW;
            SetShell( new svx::FontworkBar( this ) );
            rDispatcher.Push( *GetCurShell() );

            SetShell( new SwDrawShell( *this ));
            rDispatcher.Push( *GetCurShell() );
            if ( _nSelectionType & nsSelectionType::SEL_BEZ )
            {
                eShellMode = SHELL_MODE_BEZIER;
                SetShell( new SwBezierShell( *this ));
                rDispatcher.Push( *GetCurShell() );
            }

        }
        else if ( _nSelectionType & nsSelectionType::SEL_DRW_FORM )
        {
            eShellMode = SHELL_MODE_DRAW_FORM;
            SetShell( new SwWebDrawFormShell( *this ));

            rDispatcher.Push( *GetCurShell() );
        }
        else if ( _nSelectionType & nsSelectionType::SEL_DRW_TXT )
        {
            eShellMode = SHELL_MODE_DRAWTEXT;
            rDispatcher.Push( *(new SwBaseShell( *this )) );
            SetShell( new SwDrawTextShell( *this ));
            rDispatcher.Push( *GetCurShell() );
        }
        else if ( _nSelectionType & nsSelectionType::SEL_POSTIT )
        {
            eShellMode = SHELL_MODE_POSTIT;
            SetShell( new SwAnnotationShell( *this ) );
            rDispatcher.Push( *GetCurShell() );
        }
        else
        {
            bSetExtInpCntxt = TRUE;
            eShellMode = SHELL_MODE_TEXT;
            if ( _nSelectionType & nsSelectionType::SEL_NUM )
            {
                eShellMode = SHELL_MODE_LIST_TEXT;
                SetShell( new SwWebListShell( *this ));
                rDispatcher.Push( *GetCurShell() );
            }
            SetShell( new SwWebTextShell(*this));
            rDispatcher.Push( *GetCurShell() );
            if ( _nSelectionType & nsSelectionType::SEL_TBL )
            {
                eShellMode = eShellMode == SHELL_MODE_LIST_TEXT ? SHELL_MODE_TABLE_LIST_TEXT
                                                        : SHELL_MODE_TABLE_TEXT;
                SetShell( new SwWebTableShell( *this ));
                rDispatcher.Push( *GetCurShell() );
            }
        }
        ImpSetVerb( _nSelectionType );
        GetViewImpl()->SetShellMode(eShellMode);

        if( !GetDocShell()->IsReadOnly() )
        {
            if( bSetExtInpCntxt && GetWrtShell().HasReadonlySel() )
                bSetExtInpCntxt = FALSE;

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

        Point aPnt = GetEditWin().GetPointerPosPixel();
        aPnt = GetEditWin().PixelToLogic(aPnt);
        GetEditWin().UpdatePointer(aPnt);

        if ( bInitFormShell && GetWrtShell().GetDrawView() )
            GetFormShell()->SetView( PTR_CAST( FmFormView,
                                                GetWrtShell().GetDrawView()));


    }
    GetViewImpl()->GetUNOObject_Impl()->NotifySelChanged();

    //Guenstiger Zeitpunkt fuer die Kommunikation mit OLE-Objekten?
    if ( GetDocShell()->GetDoc()->IsOLEPrtNotifyPending() )
        GetDocShell()->GetDoc()->PrtOLENotify( FALSE );

    //jetzt das Tabellen-Update
    if(bUpdateTable)
        GetWrtShell().UpdateTable();
}




