/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <config_features.h>

#include <sfx2/msg.hxx>
#include <svl/srchitem.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/infobar.hxx>
#include <sfx2/templdlg.hxx>
#include <svx/srchdlg.hxx>
#include <basic/sbxobj.hxx>
#include <uivwimp.hxx>
#include <svx/fmshell.hxx>
#include <svx/extrusionbar.hxx>
#include <svx/fontworkbar.hxx>
#include <vcl/inputctx.hxx>

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

// TECHNICALLY not possible !!
#include <beziersh.hxx>
#include <drawsh.hxx>
#include <drwtxtsh.hxx>
#include <annotsh.hxx>

#include <wview.hxx>
#include <wdocsh.hxx>

#include <sfx2/request.hxx>
    // needed for -fsanitize=function visibility of typeinfo for functions of
    // type void(SfxShell*,SfxRequest&) defined in swslots.hxx
#include <sfx2/viewfac.hxx>
#define ShellClass_SwWebView
#define ShellClass_Text
#include <swslots.hxx>

SFX_IMPL_NAMED_VIEWFACTORY(SwWebView, "Default")
{
    SFX_VIEW_REGISTRATION(SwWebDocShell);
}

SFX_IMPL_INTERFACE(SwWebView, SwView)

void SwWebView::InitInterface_Impl()
{
    GetStaticInterface()->RegisterChildWindow(SvxSearchDialogWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(SfxInfoBarContainerChild::GetChildWindowId());

    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_TOOLS, SfxVisibilityFlags::Standard|SfxVisibilityFlags::Server,
                                            ToolbarId::Webtools_Toolbox);
}


SwWebView::SwWebView(SfxViewFrame* _pFrame, SfxViewShell* _pShell) :
    SwView(_pFrame, _pShell)
{
}

SwWebView::~SwWebView()
{
}

void SwWebView::SelectShell()
{
#if HAVE_FEATURE_DESKTOP
    // Decision whether UpdateTable must be called
    bool bUpdateTable = false;
    const SwFrameFormat* pCurTableFormat = GetWrtShell().GetTableFormat();
    if(pCurTableFormat && pCurTableFormat != GetLastTableFrameFormat())
    {
        bUpdateTable = true; // can only be executed later
    }
    SetLastTableFrameFormat(pCurTableFormat);
    //SEL_TBL and SEL_TBL_CELLS can be ored!
    SelectionType nNewSelectionType = GetWrtShell().GetSelectionType()
                                & ~SelectionType::TableCell;

    SelectionType _nSelectionType = GetSelectionType();
    if ( nNewSelectionType == _nSelectionType )
    {
        GetViewFrame()->GetBindings().InvalidateAll( false );
        if ( _nSelectionType & SelectionType::Ole ||
             _nSelectionType & SelectionType::Graphic )
            //The verb may of course change for graphics and OLE!
            ImpSetVerb( nNewSelectionType );
    }
    else
    {
        SfxDispatcher &rDispatcher = *GetViewFrame()->GetDispatcher();
        SwToolbarConfigItem *pBarCfg = SW_MOD()->GetWebToolbarConfig();

        if( GetCurShell() )
        {
            rDispatcher.Flush();        // really delete all cached shells

            //Additional to the old selection remember which toolbar was visible.
            ToolbarId eId = rDispatcher.GetObjectBarId(SFX_OBJECTBAR_OBJECT);
            if (eId != ToolbarId::None)
                pBarCfg->SetTopToolbar( _nSelectionType, eId );

            SfxShell *pSfxShell;
            sal_uInt16 i;
            for ( i = 0; true; ++i )
            {
                pSfxShell = rDispatcher.GetShell( i );
                if ( !( dynamic_cast< const SwBaseShell *>( pSfxShell ) !=  nullptr ||
                    dynamic_cast< const SwDrawTextShell *>( pSfxShell ) || dynamic_cast< const SwAnnotationShell *>( pSfxShell ) !=  nullptr ) )
                    break;
            }
            if (i)
            {
                pSfxShell = rDispatcher.GetShell( --i );
                OSL_ENSURE( pSfxShell, "My Shell is lost in space" );
                rDispatcher.Pop( *pSfxShell, SfxDispatcherPopFlags::POP_UNTIL | SfxDispatcherPopFlags::POP_DELETE);
            }
        }

        bool bInitFormShell = false;
        if( !GetFormShell() )
        {
            bInitFormShell = true;
            SetFormShell( new FmFormShell( this ) );
            rDispatcher.Push( *GetFormShell() );
        }

        bool bSetExtInpCntxt = false;
        _nSelectionType = nNewSelectionType;
        SetSelectionType( _nSelectionType );
        ShellMode eShellMode;

        if ( _nSelectionType & SelectionType::Ole )
        {
            eShellMode = ShellMode::Object;
            SetShell( new SwWebOleShell( *this ));
            rDispatcher.Push( *GetCurShell() );
        }
        else if ( _nSelectionType & SelectionType::Frame
            || _nSelectionType & SelectionType::Graphic)
        {
            eShellMode = ShellMode::Frame;
            SetShell( new SwWebFrameShell( *this ));
            rDispatcher.Push( *GetCurShell() );
            if(_nSelectionType & SelectionType::Graphic )
            {
                eShellMode = ShellMode::Graphic;
                SetShell( new SwWebGrfShell( *this ));
                rDispatcher.Push( *GetCurShell() );
            }
        }
        else if ( _nSelectionType & SelectionType::Frame )
        {
            eShellMode = ShellMode::Frame;
            SetShell( new SwWebFrameShell( *this ));
            rDispatcher.Push( *GetCurShell() );
        }
        else if ( _nSelectionType & SelectionType::DrawObject )
        {
            eShellMode = ShellMode::Draw;
            SetShell( new svx::ExtrusionBar( this ) );
            rDispatcher.Push( *GetCurShell() );

            SetShell( new svx::FontworkBar( this ) );
            rDispatcher.Push( *GetCurShell() );

            SetShell( new SwDrawShell( *this ));
            rDispatcher.Push( *GetCurShell() );
            if ( _nSelectionType & SelectionType::Ornament )
            {
                eShellMode = ShellMode::Bezier;
                SetShell( new SwBezierShell( *this ));
                rDispatcher.Push( *GetCurShell() );
            }

        }
        else if ( _nSelectionType & SelectionType::DbForm )
        {
            eShellMode = ShellMode::DrawForm;
            SetShell( new SwWebDrawFormShell( *this ));

            rDispatcher.Push( *GetCurShell() );
        }
        else if ( _nSelectionType & SelectionType::DrawObjectEditMode )
        {
            eShellMode = ShellMode::DrawText;
            rDispatcher.Push( *(new SwBaseShell( *this )) );
            SetShell( new SwDrawTextShell( *this ));
            rDispatcher.Push( *GetCurShell() );
        }
        else if ( _nSelectionType & SelectionType::PostIt )
        {
            eShellMode = ShellMode::PostIt;
            SetShell( new SwAnnotationShell( *this ) );
            rDispatcher.Push( *GetCurShell() );
        }
        else
        {
            bSetExtInpCntxt = true;
            eShellMode = ShellMode::Text;
            if ( _nSelectionType & SelectionType::NumberList )
            {
                eShellMode = ShellMode::ListText;
                SetShell( new SwWebListShell( *this ));
                rDispatcher.Push( *GetCurShell() );
            }
            SetShell( new SwWebTextShell(*this));
            rDispatcher.Push( *GetCurShell() );
            if ( _nSelectionType & SelectionType::Table )
            {
                eShellMode = eShellMode == ShellMode::ListText ? ShellMode::TableListText
                                                        : ShellMode::TableText;
                SetShell( new SwWebTableShell( *this ));
                rDispatcher.Push( *GetCurShell() );
            }
        }
        ImpSetVerb( _nSelectionType );
        GetViewImpl()->SetShellMode(eShellMode);

        if( !GetDocShell()->IsReadOnly() )
        {
            if( bSetExtInpCntxt && GetWrtShell().HasReadonlySel() )
                bSetExtInpCntxt = false;

            InputContext aCntxt( GetEditWin().GetInputContext() );
            aCntxt.SetOptions( bSetExtInpCntxt
                                ? (aCntxt.GetOptions() |
                                        ( InputContextFlags::Text |
                                            InputContextFlags::ExtText ))
                                : (aCntxt.GetOptions() & ~
                                        InputContextFlags( InputContextFlags::Text |
                                            InputContextFlags::ExtText )) );
            GetEditWin().SetInputContext( aCntxt );
        }

        //Additional to the selection enable the toolbar, which was
        //activated last time
        //Before must be a Flush(), but concerns according to MBA not the
        //user interface and is not a performance issue.
        // TODO/LATER: maybe now the Flush() command is superfluous?!
        rDispatcher.Flush();

        Point aPnt = GetEditWin().GetPointerPosPixel();
        aPnt = GetEditWin().PixelToLogic(aPnt);
        GetEditWin().UpdatePointer(aPnt);

        if ( bInitFormShell && GetWrtShell().GetDrawView() )
            GetFormShell()->SetView( dynamic_cast< FmFormView* >(
                                                GetWrtShell().GetDrawView()));

    }
    GetViewImpl()->GetUNOObject_Impl()->NotifySelChanged();

    //Opportune time for the communication with OLE objects?
    if ( GetDocShell()->GetDoc()->IsOLEPrtNotifyPending() )
        GetDocShell()->GetDoc()->PrtOLENotify( false );

    //now the table update
    if(bUpdateTable)
        GetWrtShell().UpdateTable();
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
