/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <sfx2/msg.hxx>
#include <svl/srchitem.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/templdlg.hxx>
#include <svx/srchdlg.hxx>
#include <basic/sbxobj.hxx>
#include <uivwimp.hxx>
#include <svx/fmshell.hxx>
#include <svx/extrusionbar.hxx>
#include <svx/fontworkbar.hxx>

#include <sfx2/objface.hxx>
#include <swmodule.hxx>
#include <unotxvw.hxx>
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
#include <swslots.hxx>


SFX_IMPL_NAMED_VIEWFACTORY(SwWebView, "Default")
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
    sal_Bool bUpdateTable = sal_False;
    const SwFrmFmt* pCurTableFmt = GetWrtShell().GetTableFmt();
    if(pCurTableFmt && pCurTableFmt != GetLastTblFrmFmt())
    {
        bUpdateTable = sal_True; // kann erst spaeter ausgefuehrt werden
    }
    SetLastTblFrmFmt(pCurTableFmt);
    //SEL_TBL und SEL_TBL_CELLS koennen verodert sein!
    int nNewSelectionType = (GetWrtShell().GetSelectionType()
                                & ~nsSelectionType::SEL_TBL_CELLS);

    int _nSelectionType = GetSelectionType();
    if ( nNewSelectionType == _nSelectionType )
    {
        GetViewFrame()->GetBindings().InvalidateAll( sal_False );
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
            sal_uInt16 i;
            for ( i = 0; sal_True; ++i )
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

        sal_Bool bInitFormShell = sal_False;
        if( !GetFormShell() )
        {
            bInitFormShell = sal_True;
            SetFormShell( new FmFormShell( this ) );
            rDispatcher.Push( *GetFormShell() );
        }

        sal_Bool bSetExtInpCntxt = sal_False;
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
            bSetExtInpCntxt = sal_True;
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
        GetDocShell()->GetDoc()->PrtOLENotify( sal_False );

    //jetzt das Tabellen-Update
    if(bUpdateTable)
        GetWrtShell().UpdateTable();
}




