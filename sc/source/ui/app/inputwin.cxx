/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <algorithm>

#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include <sfx2/app.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/editview.hxx>
#include <editeng/editstat.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/lspcitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/event.hxx>
#include <sfx2/imgmgr.hxx>
#include <stdlib.h>
#include <editeng/scriptspaceitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <vcl/cursor.hxx>
#include <vcl/help.hxx>
#include <vcl/settings.hxx>
#include <svl/stritem.hxx>
#include <stdio.h>

#include "inputwin.hxx"
#include "scmod.hxx"
#include "uiitems.hxx"
#include "global.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "globstr.hrc"
#include "reffact.hxx"
#include "editutil.hxx"
#include "inputhdl.hxx"
#include "tabvwsh.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "appoptio.hxx"
#include "rangenam.hxx"
#include <formula/compiler.hrc>
#include "dbdata.hxx"
#include "rangeutl.hxx"
#include "docfunc.hxx"
#include "funcdesc.hxx"
#include "markdata.hxx"
#include <editeng/fontitem.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include "AccessibleEditObject.hxx"
#include "AccessibleText.hxx"
#include <svtools/miscopt.hxx>
#include <comphelper/string.hxx>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XController.hpp>

#define THESIZE             1000000 
#define TBX_WINDOW_HEIGHT   22 
#define LEFT_OFFSET         5
#define INPUTWIN_MULTILINES 6
const long BUTTON_OFFSET = 2; 
const long ADDITIONAL_BORDER = 1; 
const long ADDITIONAL_SPACE = 4; 

using com::sun::star::uno::Reference;
using com::sun::star::uno::UNO_QUERY;

using com::sun::star::frame::XLayoutManager;
using com::sun::star::frame::XModel;
using com::sun::star::frame::XFrame;
using com::sun::star::frame::XController;
using com::sun::star::beans::XPropertySet;


enum ScNameInputType
{
    SC_NAME_INPUT_CELL,
    SC_NAME_INPUT_RANGE,
    SC_NAME_INPUT_NAMEDRANGE,
    SC_NAME_INPUT_DATABASE,
    SC_NAME_INPUT_ROW,
    SC_NAME_INPUT_SHEET,
    SC_NAME_INPUT_DEFINE,
    SC_NAME_INPUT_BAD_NAME,
    SC_NAME_INPUT_BAD_SELECTION,
    SC_MANAGE_NAMES
};


ScTextWndBase::ScTextWndBase( Window* pParent,  WinBits nStyle )
    : Window ( pParent, nStyle )
{
    if ( IsNativeControlSupported( CTRL_EDITBOX, PART_ENTIRE_CONTROL ) )
    {
        SetType( WINDOW_CALCINPUTLINE );
        SetBorderStyle( WINDOW_BORDER_NWF );
    }
}





SFX_IMPL_CHILDWINDOW_WITHID(ScInputWindowWrapper,FID_INPUTLINE_STATUS)

ScInputWindowWrapper::ScInputWindowWrapper( Window*          pParentP,
                                            sal_uInt16           nId,
                                            SfxBindings*     pBindings,
                                            SfxChildWinInfo* /* pInfo */ )
    :   SfxChildWindow( pParentP, nId )
{
    ScInputWindow* pWin=new ScInputWindow( pParentP, pBindings );
    pWindow = pWin;

    pWin->Show();

    pWin->SetSizePixel( pWin->CalcWindowSizePixel() );

    eChildAlignment = SFX_ALIGN_LOWESTTOP;
    pBindings->Invalidate( FID_TOGGLEINPUTLINE );
}



SfxChildWinInfo ScInputWindowWrapper::GetInfo() const
{
    SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();
    return aInfo;
}



#define IMAGE(id) pImgMgr->SeekImage(id)
static bool lcl_isExperimentalMode()
{
    
    
    return true;
}





static ScTextWndBase* lcl_chooseRuntimeImpl( Window* pParent, SfxBindings* pBind )
{
    ScTabViewShell* pViewSh = NULL;
    SfxDispatcher* pDisp = pBind->GetDispatcher();
    if ( pDisp )
    {
        SfxViewFrame* pViewFrm = pDisp->GetFrame();
        if ( pViewFrm )
            pViewSh = PTR_CAST( ScTabViewShell, pViewFrm->GetViewShell() );
    }

    if ( !lcl_isExperimentalMode() )
        return new ScTextWnd( pParent, pViewSh );
    return new ScInputBarGroup( pParent, pViewSh );
}

ScInputWindow::ScInputWindow( Window* pParent, SfxBindings* pBind ) :

        ToolBox         ( pParent, WinBits(WB_CLIPCHILDREN) ),
        aWndPos         ( this ),
        pRuntimeWindow ( lcl_chooseRuntimeImpl( this, pBind ) ),
        aTextWindow    ( *pRuntimeWindow ),
        pInputHdl       ( NULL ),
        aTextOk         ( ScResId( SCSTR_QHELP_BTNOK ) ),       
        aTextCancel     ( ScResId( SCSTR_QHELP_BTNCANCEL ) ),
        aTextSum        ( ScResId( SCSTR_QHELP_BTNSUM ) ),
        aTextEqual      ( ScResId( SCSTR_QHELP_BTNEQUAL ) ),
        mnMaxY          (0),
        bIsOkCancelMode ( false ),
        bInResize       ( false ),
        mbIsMultiLine   ( lcl_isExperimentalMode() )
{
    ScModule*        pScMod  = SC_MOD();
    SfxImageManager* pImgMgr = SfxImageManager::GetImageManager( pScMod );

    
    
    ScTabViewShell* pViewSh = NULL;
    SfxDispatcher* pDisp = pBind->GetDispatcher();
    if ( pDisp )
    {
        SfxViewFrame* pViewFrm = pDisp->GetFrame();
        if ( pViewFrm )
            pViewSh = PTR_CAST( ScTabViewShell, pViewFrm->GetViewShell() );
    }
    OSL_ENSURE( pViewSh, "no view shell for input window" );

    
    InsertWindow    ( 1, &aWndPos, 0,                                     0 );
    InsertSeparator (                                                     1 );
    InsertItem      ( SID_INPUT_FUNCTION, IMAGE( SID_INPUT_FUNCTION ), 0, 2 );
    InsertItem      ( SID_INPUT_SUM,      IMAGE( SID_INPUT_SUM ), 0,      3 );
    InsertItem      ( SID_INPUT_EQUAL,    IMAGE( SID_INPUT_EQUAL ), 0,    4 );
    InsertSeparator (                                                     5 );
    InsertWindow    ( 7, &aTextWindow, 0,                                 6 );

    aWndPos    .SetQuickHelpText( ScResId( SCSTR_QHELP_POSWND ) );
    aWndPos    .SetHelpId       ( HID_INSWIN_POS );
    aTextWindow.SetQuickHelpText( ScResId( SCSTR_QHELP_INPUTWND ) );
    aTextWindow.SetHelpId       ( HID_INSWIN_INPUT );

    

    SetItemText ( SID_INPUT_FUNCTION, ScResId( SCSTR_QHELP_BTNCALC ) );
    SetHelpId   ( SID_INPUT_FUNCTION, HID_INSWIN_CALC );

    SetItemText ( SID_INPUT_SUM, aTextSum );
    SetHelpId   ( SID_INPUT_SUM, HID_INSWIN_SUMME );

    SetItemText ( SID_INPUT_EQUAL, aTextEqual );
    SetHelpId   ( SID_INPUT_EQUAL, HID_INSWIN_FUNC );

    SetHelpId( HID_SC_INPUTWIN );   

    aWndPos     .Show();
    aTextWindow.Show();

    pInputHdl = SC_MOD()->GetInputHdl( pViewSh, false );    
    if (pInputHdl)
        pInputHdl->SetInputWindow( this );

    if (pInputHdl && !pInputHdl->GetFormString().isEmpty())
    {
        
        
        

        aTextWindow.SetTextString( pInputHdl->GetFormString() );
    }
    else if ( pInputHdl && pInputHdl->IsInputMode() )
    {
        
        
        

        aTextWindow.SetTextString( pInputHdl->GetEditString() );    
        if ( pInputHdl->IsTopMode() )
            pInputHdl->SetMode( SC_INPUT_TABLE );       
    }
    else if ( pViewSh )
        pViewSh->UpdateInputHandler( true ); 

    pImgMgr->RegisterToolBox( this );
    SetAccessibleName(ScResId(STR_ACC_TOOLBAR_FORMULA));
}

ScInputWindow::~ScInputWindow()
{
    sal_Bool bDown = ( ScGlobal::pSysLocale == NULL );    

    
    
    

    if ( !bDown )
    {
        TypeId aScType = TYPE(ScTabViewShell);
        SfxViewShell* pSh = SfxViewShell::GetFirst( &aScType );
        while ( pSh )
        {
            ScInputHandler* pHdl = ((ScTabViewShell*)pSh)->GetInputHandler();
            if ( pHdl && pHdl->GetInputWindow() == this )
            {
                pHdl->SetInputWindow( NULL );
                pHdl->StopInputWinEngine( false );  
            }
            pSh = SfxViewShell::GetNext( *pSh, &aScType );
        }
    }

    SfxImageManager::GetImageManager( SC_MOD() )->ReleaseToolBox( this );
}

void ScInputWindow::SetInputHandler( ScInputHandler* pNew )
{
    

    if ( pNew != pInputHdl )
    {
        
        

        pInputHdl = pNew;
        if (pInputHdl)
            pInputHdl->SetInputWindow( this );
    }
}

bool ScInputWindow::UseSubTotal(ScRangeList* pRangeList) const
{
    bool bSubTotal = false;
    ScTabViewShell* pViewSh = PTR_CAST( ScTabViewShell, SfxViewShell::Current() );
    if ( pViewSh )
    {
        ScDocument* pDoc = pViewSh->GetViewData()->GetDocument();
        size_t nRangeCount (pRangeList->size());
        size_t nRangeIndex (0);
        while (!bSubTotal && nRangeIndex < nRangeCount)
        {
            const ScRange* pRange = (*pRangeList)[nRangeIndex];
            if( pRange )
            {
                SCTAB nTabEnd(pRange->aEnd.Tab());
                SCTAB nTab(pRange->aStart.Tab());
                while (!bSubTotal && nTab <= nTabEnd)
                {
                    SCROW nRowEnd(pRange->aEnd.Row());
                    SCROW nRow(pRange->aStart.Row());
                    while (!bSubTotal && nRow <= nRowEnd)
                    {
                        if (pDoc->RowFiltered(nRow, nTab))
                            bSubTotal = true;
                        else
                            ++nRow;
                    }
                    ++nTab;
                }
            }
            ++nRangeIndex;
        }

        const ScDBCollection::NamedDBs& rDBs = pDoc->GetDBCollection()->getNamedDBs();
        ScDBCollection::NamedDBs::const_iterator itr = rDBs.begin(), itrEnd = rDBs.end();
        for (; !bSubTotal && itr != itrEnd; ++itr)
        {
            const ScDBData& rDB = *itr;
            if (!rDB.HasAutoFilter())
                continue;

            nRangeIndex = 0;
            while (!bSubTotal && nRangeIndex < nRangeCount)
            {
                const ScRange* pRange = (*pRangeList)[nRangeIndex];
                if( pRange )
                {
                    ScRange aDBArea;
                    rDB.GetArea(aDBArea);
                    if (aDBArea.Intersects(*pRange))
                        bSubTotal = true;
                }
                ++nRangeIndex;
            }
        }
    }
    return bSubTotal;
}

void ScInputWindow::Select()
{
    ScModule* pScMod = SC_MOD();
    ToolBox::Select();

    switch ( GetCurItemId() )
    {
        case SID_INPUT_FUNCTION:
            {
                
                SfxViewFrame* pViewFrm = SfxViewFrame::Current();
                if ( pViewFrm && !pViewFrm->GetChildWindow( SID_OPENDLG_FUNCTION ) )
                {
                    pViewFrm->GetDispatcher()->Execute( SID_OPENDLG_FUNCTION,
                                              SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD );

                    
                    

                }
            }
            break;

        case SID_INPUT_CANCEL:
            pScMod->InputCancelHandler();
            SetSumAssignMode();
            break;

        case SID_INPUT_OK:
            pScMod->InputEnterHandler();
            SetSumAssignMode();
            aTextWindow.Invalidate();       
            break;

        case SID_INPUT_SUM:
            {
                ScTabViewShell* pViewSh = PTR_CAST( ScTabViewShell, SfxViewShell::Current() );
                if ( pViewSh )
                {
                    const ScMarkData& rMark = pViewSh->GetViewData()->GetMarkData();
                    if ( rMark.IsMarked() || rMark.IsMultiMarked() )
                    {
                        ScRangeList aMarkRangeList;
                        rMark.FillRangeListWithMarks( &aMarkRangeList, false );
                        ScDocument* pDoc = pViewSh->GetViewData()->GetDocument();

                        
                        bool bEmpty = false;
                        const size_t nCount = aMarkRangeList.size();
                        for ( size_t i = 0; i < nCount; ++i )
                        {
                            const ScRange aRange( *aMarkRangeList[i] );
                            if ( pDoc->IsBlockEmpty( aRange.aStart.Tab(),
                                    aRange.aStart.Col(), aRange.aStart.Row(),
                                    aRange.aEnd.Col(), aRange.aEnd.Row() ) )
                            {
                                bEmpty = true;
                                break;
                            }
                        }

                        if ( bEmpty )
                        {
                            ScRangeList aRangeList;
                            const sal_Bool bDataFound = pViewSh->GetAutoSumArea( aRangeList );
                            if ( bDataFound )
                            {
                                ScAddress aAddr = aRangeList.back()->aEnd;
                                aAddr.IncRow();
                                const sal_Bool bSubTotal( UseSubTotal( &aRangeList ) );
                                pViewSh->EnterAutoSum( aRangeList, bSubTotal, aAddr );
                            }
                        }
                        else
                        {
                            const sal_Bool bSubTotal( UseSubTotal( &aMarkRangeList ) );
                            for ( size_t i = 0; i < nCount; ++i )
                            {
                                const ScRange aRange( *aMarkRangeList[i] );
                                const bool bSetCursor = ( i == nCount - 1 ? true : false );
                                const bool bContinue = ( i != 0  ? true : false );
                                if ( !pViewSh->AutoSum( aRange, bSubTotal, bSetCursor, bContinue ) )
                                {
                                    pViewSh->MarkRange( aRange, false, false );
                                    pViewSh->SetCursor( aRange.aEnd.Col(), aRange.aEnd.Row() );
                                    const ScRangeList aRangeList;
                                    ScAddress aAddr = aRange.aEnd;
                                    aAddr.IncRow();
                                    const OUString aFormula = pViewSh->GetAutoSumFormula(
                                        aRangeList, bSubTotal, aAddr );
                                    SetFuncString( aFormula );
                                    break;
                                }
                            }
                        }
                    }
                    else                                    
                    {
                        ScRangeList aRangeList;
                        const sal_Bool bDataFound = pViewSh->GetAutoSumArea( aRangeList );
                        const sal_Bool bSubTotal( UseSubTotal( &aRangeList ) );
                        ScAddress aAddr = pViewSh->GetViewData()->GetCurPos();
                        const OUString aFormula = pViewSh->GetAutoSumFormula( aRangeList, bSubTotal, aAddr );
                        SetFuncString( aFormula );

                        if ( bDataFound && pScMod->IsEditMode() )
                        {
                            ScInputHandler* pHdl = pScMod->GetInputHdl( pViewSh );
                            if ( pHdl )
                            {
                                pHdl->InitRangeFinder( aFormula );

                                
                                
                                const sal_Int32 nOpen = aFormula.indexOf('(');
                                const sal_Int32 nLen = aFormula.getLength();
                                if ( nOpen != -1 && nLen > nOpen )
                                {
                                    sal_uInt8 nAdd(1);
                                    if (bSubTotal)
                                        nAdd = 3;
                                    ESelection aSel(0,nOpen+nAdd,0,nLen-1);
                                    EditView* pTableView = pHdl->GetTableView();
                                    if (pTableView)
                                        pTableView->SetSelection(aSel);
                                    EditView* pTopView = pHdl->GetTopView();
                                    if (pTopView)
                                        pTopView->SetSelection(aSel);
                                }
                            }
                        }
                    }
                }
            }
            break;

        case SID_INPUT_EQUAL:
        {
            aTextWindow.StartEditEngine();
            if ( pScMod->IsEditMode() )         
            {
                aTextWindow.StartEditEngine();

                sal_Int32 nStartPos = 1;
                sal_Int32 nEndPos = 1;

                ScTabViewShell* pViewSh = PTR_CAST( ScTabViewShell, SfxViewShell::Current() );
                if ( pViewSh )
                {
                    const OUString& rString = aTextWindow.GetTextString();
                    const sal_Int32 nLen = rString.getLength();

                    ScDocument* pDoc = pViewSh->GetViewData()->GetDocument();
                    CellType eCellType = pDoc->GetCellType( pViewSh->GetViewData()->GetCurPos() );
                    switch ( eCellType )
                    {
                        case CELLTYPE_VALUE:
                        {
                            nEndPos = nLen + 1;
                            aTextWindow.SetTextString("=" +  rString);
                            break;
                        }
                        case CELLTYPE_STRING:
                        case CELLTYPE_EDIT:
                            nStartPos = 0;
                            nEndPos = nLen;
                            break;
                        case CELLTYPE_FORMULA:
                            nEndPos = nLen;
                            break;
                        default:
                            aTextWindow.SetTextString("=");
                            break;
                    }
                }

                EditView* pView = aTextWindow.GetEditView();
                if (pView)
                {
                    pView->SetSelection( ESelection(0, nStartPos, 0, nEndPos) );
                    pScMod->InputChanged(pView);
                    SetOkCancelMode();
                    pView->SetEditEngineUpdateMode(true);
                }
            }
            break;
        }
    }
}

void ScInputWindow::Paint( const Rectangle& rRect )
{
    ToolBox::Paint( rRect );

    
    
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    SetLineColor( rStyleSettings.GetShadowColor() );

    Size aSize = GetSizePixel();
    DrawLine( Point( 0, aSize.Height() - 1 ), Point( aSize.Width() - 1, aSize.Height() - 1 ) );
}

void ScInputWindow::Resize()
{
    ToolBox::Resize();
    if ( mbIsMultiLine )
    {
        aTextWindow.Resize();
        Size aSize = GetSizePixel();
        aSize.Height() = CalcWindowSizePixel().Height() + ADDITIONAL_BORDER;
        ScInputBarGroup* pGroupBar = dynamic_cast< ScInputBarGroup* > ( pRuntimeWindow.get() );
        if ( pGroupBar )
        {
            
            
            
            
            
            
            if ( pGroupBar->GetNumLines() > 1 )
                aSize.Height() += pGroupBar->GetVertOffset() + ADDITIONAL_SPACE;
        }
        SetSizePixel(aSize);
        Invalidate();
    }
    else
    {
        long nWidth = GetSizePixel().Width();
        long nLeft  = aTextWindow.GetPosPixel().X();
        Size aSize  = aTextWindow.GetSizePixel();

        aSize.Width() = std::max( ((long)(nWidth - nLeft - 5)), (long)0 );

        aTextWindow.SetSizePixel( aSize );
        aTextWindow.Invalidate();
    }
}

void ScInputWindow::SetFuncString( const OUString& rString, bool bDoEdit )
{
    
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    EnableButtons( pViewFrm && !pViewFrm->GetChildWindow( SID_OPENDLG_FUNCTION ) );
    aTextWindow.StartEditEngine();

    ScModule* pScMod = SC_MOD();
    if ( pScMod->IsEditMode() )
    {
        if ( bDoEdit )
            aTextWindow.GrabFocus();
        aTextWindow.SetTextString( rString );
        EditView* pView = aTextWindow.GetEditView();
        if (pView)
        {
            sal_Int32 nLen = rString.getLength();

            if ( nLen > 0 )
            {
                nLen--;
                pView->SetSelection( ESelection( 0, nLen, 0, nLen ) );
            }

            pScMod->InputChanged(pView);
            if ( bDoEdit )
                SetOkCancelMode();          

            pView->SetEditEngineUpdateMode(true);
        }
    }
}

void ScInputWindow::SetPosString( const OUString& rStr )
{
    aWndPos.SetPos( rStr );
}

void ScInputWindow::SetTextString( const OUString& rString )
{
    if (rString.getLength() <= 32767)
        aTextWindow.SetTextString(rString);
    else
        aTextWindow.SetTextString(rString.copy(0, 32767));
}

void ScInputWindow::SetOkCancelMode()
{
    
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    EnableButtons( pViewFrm && !pViewFrm->GetChildWindow( SID_OPENDLG_FUNCTION ) );

    ScModule* pScMod = SC_MOD();
    SfxImageManager* pImgMgr = SfxImageManager::GetImageManager( pScMod );
    if (!bIsOkCancelMode)
    {
        RemoveItem( 3 ); 
        RemoveItem( 3 );
        InsertItem( SID_INPUT_CANCEL, IMAGE( SID_INPUT_CANCEL ), 0, 3 );
        InsertItem( SID_INPUT_OK,     IMAGE( SID_INPUT_OK ),     0, 4 );
        SetItemText ( SID_INPUT_CANCEL, aTextCancel );
        SetHelpId   ( SID_INPUT_CANCEL, HID_INSWIN_CANCEL );
        SetItemText ( SID_INPUT_OK,     aTextOk );
        SetHelpId   ( SID_INPUT_OK,     HID_INSWIN_OK );
        bIsOkCancelMode = true;
    }
}

void ScInputWindow::SetSumAssignMode()
{
    
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    EnableButtons( pViewFrm && !pViewFrm->GetChildWindow( SID_OPENDLG_FUNCTION ) );

    ScModule* pScMod = SC_MOD();
    SfxImageManager* pImgMgr = SfxImageManager::GetImageManager( pScMod );
    if (bIsOkCancelMode)
    {
        
        RemoveItem( 3 );
        RemoveItem( 3 );
        InsertItem( SID_INPUT_SUM,   IMAGE( SID_INPUT_SUM ),     0, 3 );
        InsertItem( SID_INPUT_EQUAL, IMAGE( SID_INPUT_EQUAL ),   0, 4 );
        SetItemText ( SID_INPUT_SUM,   aTextSum );
        SetHelpId   ( SID_INPUT_SUM,   HID_INSWIN_SUMME );
        SetItemText ( SID_INPUT_EQUAL, aTextEqual );
        SetHelpId   ( SID_INPUT_EQUAL, HID_INSWIN_FUNC );
        bIsOkCancelMode = false;

        SetFormulaMode(false);      
    }
}

void ScInputWindow::SetFormulaMode( bool bSet )
{
    aWndPos.SetFormulaMode(bSet);
    aTextWindow.SetFormulaMode(bSet);
}

void ScInputWindow::SetText( const OUString& rString )
{
    ToolBox::SetText(rString);
}

OUString ScInputWindow::GetText() const
{
    return ToolBox::GetText();
}

bool ScInputWindow::IsInputActive()
{
    return aTextWindow.IsInputActive();
}

EditView* ScInputWindow::GetEditView()
{
    return aTextWindow.GetEditView();
}

void ScInputWindow::MakeDialogEditView()
{
    aTextWindow.MakeDialogEditView();
}

void ScInputWindow::StopEditEngine( bool bAll )
{
    aTextWindow.StopEditEngine( bAll );
}

void ScInputWindow::TextGrabFocus()
{
    aTextWindow.TextGrabFocus();
}

void ScInputWindow::TextInvalidate()
{
    aTextWindow.Invalidate();
}

void ScInputWindow::SwitchToTextWin()
{
    

    aTextWindow.StartEditEngine();
    if ( SC_MOD()->IsEditMode() )
    {
        aTextWindow.TextGrabFocus();
        EditView* pView = aTextWindow.GetEditView();
        if (pView)
        {
            sal_Int32 nPara =  pView->GetEditEngine()->GetParagraphCount() ? ( pView->GetEditEngine()->GetParagraphCount() - 1 ) : 0;
            sal_Int32 nLen = pView->GetEditEngine()->GetTextLen( nPara );
            ESelection aSel( nPara, nLen, nPara, nLen );
            pView->SetSelection( aSel );                
        }
    }
}

void ScInputWindow::PosGrabFocus()
{
    aWndPos.GrabFocus();
}

void ScInputWindow::EnableButtons( bool bEnable )
{
    
    if ( bEnable && !IsEnabled() )
        Enable();

    EnableItem( SID_INPUT_FUNCTION,                                   bEnable );
    EnableItem( bIsOkCancelMode ? SID_INPUT_CANCEL : SID_INPUT_SUM,   bEnable );
    EnableItem( bIsOkCancelMode ? SID_INPUT_OK     : SID_INPUT_EQUAL, bEnable );

}

void ScInputWindow::StateChanged( StateChangedType nType )
{
    ToolBox::StateChanged( nType );

    if ( nType == STATE_CHANGE_INITSHOW ) Resize();
}

void ScInputWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( rDCEvt.GetType() == DATACHANGED_SETTINGS && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        
        ScModule*        pScMod  = SC_MOD();
        SfxImageManager* pImgMgr = SfxImageManager::GetImageManager( pScMod );
        

        SetItemImage( SID_INPUT_FUNCTION, IMAGE( SID_INPUT_FUNCTION ) );
        if ( bIsOkCancelMode )
        {
            SetItemImage( SID_INPUT_CANCEL, IMAGE( SID_INPUT_CANCEL ) );
            SetItemImage( SID_INPUT_OK,     IMAGE( SID_INPUT_OK ) );
        }
        else
        {
            SetItemImage( SID_INPUT_SUM,   IMAGE( SID_INPUT_SUM ) );
            SetItemImage( SID_INPUT_EQUAL, IMAGE( SID_INPUT_EQUAL ) );
        }
    }

    ToolBox::DataChanged( rDCEvt );
}

bool ScInputWindow::IsPointerAtResizePos()
{
    if ( GetOutputSizePixel().Height() - GetPointerPosPixel().Y() <= 4  )
        return true;
    else
        return false;
}

void ScInputWindow::MouseMove( const MouseEvent& rMEvt )
{
    if ( mbIsMultiLine )
    {
        Point aPosPixel = GetPointerPosPixel();

        ScInputBarGroup* pGroupBar = dynamic_cast< ScInputBarGroup* > ( pRuntimeWindow.get() );

        if ( bInResize || IsPointerAtResizePos() )
            SetPointer( Pointer( POINTER_WINDOW_SSIZE ) );
        else
            SetPointer( Pointer( POINTER_ARROW ) );

        if ( bInResize )
        {
            
            long nResizeThreshold = ( (long)TBX_WINDOW_HEIGHT * 0.7 );
            bool bResetPointerPos = false;

            
            if ( aPosPixel.Y() >= mnMaxY )
            {
                bResetPointerPos = true;
                aPosPixel.Y() = mnMaxY;
            } 
            else if ( GetOutputSizePixel().Height() - aPosPixel.Y() < -nResizeThreshold  )
            {
                pGroupBar->IncrementVerticalSize();
                bResetPointerPos = true;
            } 
            else if ( ( GetOutputSizePixel().Height() - aPosPixel.Y()  ) > nResizeThreshold )
            {
                bResetPointerPos = true;
                pGroupBar->DecrementVerticalSize();
            }

            if ( bResetPointerPos )
            {
                aPosPixel.Y() =  GetOutputSizePixel().Height();
                SetPointerPosPixel( aPosPixel );
            }
        }
    }
    ToolBox::MouseMove( rMEvt );
}

void ScInputWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( mbIsMultiLine )
    {
        if ( rMEvt.IsLeft() )
        {
            if ( IsPointerAtResizePos() )
            {
                
                CaptureMouse();
                bInResize = true;
                
                
                
                
                
                
                ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
                mnMaxY =  GetOutputSizePixel().Height() + ( pViewSh->GetGridHeight(SC_SPLIT_TOP) + pViewSh->GetGridHeight(SC_SPLIT_BOTTOM) ) - TBX_WINDOW_HEIGHT;
            }
        }
    }
    ToolBox::MouseButtonDown( rMEvt );
}
void ScInputWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( mbIsMultiLine )
    {
        ReleaseMouse();
        if ( rMEvt.IsLeft() )
        {
            bInResize = false;
            mnMaxY = 0;
        }
    }
    ToolBox::MouseButtonUp( rMEvt );
}






ScInputBarGroup::ScInputBarGroup(Window* pParent, ScTabViewShell* pViewSh)
    :   ScTextWndBase        ( pParent, WinBits(WB_HIDE |  WB_TABSTOP ) ),
        aMultiTextWnd        ( this, pViewSh ),
        aButton              ( this, WB_TABSTOP | WB_RECTSTYLE | WB_SMALLSTYLE ),
        aScrollBar           ( this, WB_TABSTOP | WB_VERT | WB_DRAG ),
        nVertOffset          ( 0 )
{
      aMultiTextWnd.Show();
      aMultiTextWnd.SetQuickHelpText( ScResId( SCSTR_QHELP_INPUTWND ) );
      aMultiTextWnd.SetHelpId( HID_INSWIN_INPUT );

      Size aSize( GetSettings().GetStyleSettings().GetScrollBarSize(), aMultiTextWnd.GetPixelHeightForLines(1) );

      aButton.SetClickHdl( LINK( this, ScInputBarGroup, ClickHdl ) );
      aButton.SetSizePixel( aSize );
      aButton.Enable();
      aButton.SetSymbol( SYMBOL_SPIN_DOWN  );
      aButton.SetQuickHelpText( ScResId( SCSTR_QHELP_EXPAND_FORMULA ) );
      aButton.Show();

      aScrollBar.SetSizePixel( aSize );
      aScrollBar.SetScrollHdl( LINK( this, ScInputBarGroup, Impl_ScrollHdl ) );
}

ScInputBarGroup::~ScInputBarGroup()
{

}

void
ScInputBarGroup::InsertAccessibleTextData( ScAccessibleEditLineTextData& rTextData )
{
    aMultiTextWnd.InsertAccessibleTextData( rTextData );
}

void
ScInputBarGroup::RemoveAccessibleTextData( ScAccessibleEditLineTextData& rTextData )
{
    aMultiTextWnd.RemoveAccessibleTextData( rTextData );
}

const OUString&
ScInputBarGroup::GetTextString() const
{
    return aMultiTextWnd.GetTextString();
}

void ScInputBarGroup::SetTextString( const OUString& rString )
{
    aMultiTextWnd.SetTextString(rString);
}

void ScInputBarGroup::Resize()
{
    Window *w=GetParent();
    ScInputWindow *pParent;
    pParent=dynamic_cast<ScInputWindow*>(w);

    if(pParent==NULL)
    {
        OSL_FAIL("The parent window pointer pParent is null");
        return;
    }

    long nWidth = pParent->GetSizePixel().Width();
    long nLeft  = GetPosPixel().X();

    Size aSize  = GetSizePixel();
    aSize.Width() = std::max( ((long)(nWidth - nLeft - LEFT_OFFSET)), (long)0 );

    aScrollBar.SetPosPixel(Point( aSize.Width() - aButton.GetSizePixel().Width(), aButton.GetSizePixel().Height() ) );

    Size aTmpSize( aSize );
    aTmpSize.Width() = aTmpSize.Width() - aButton.GetSizePixel().Width() - BUTTON_OFFSET;
    aMultiTextWnd.SetSizePixel(aTmpSize);

    aMultiTextWnd.Resize();

    aSize.Height() = aMultiTextWnd.GetSizePixel().Height();

    SetSizePixel(aSize);

    if( aMultiTextWnd.GetNumLines() > 1 )
    {
        aButton.SetSymbol( SYMBOL_SPIN_UP  );
        aButton.SetQuickHelpText( ScResId( SCSTR_QHELP_COLLAPSE_FORMULA ) );
        Size scrollSize = aButton.GetSizePixel();
        scrollSize.Height() = aMultiTextWnd.GetSizePixel().Height() - aButton.GetSizePixel().Height();
        aScrollBar.SetSizePixel( scrollSize );

        Size aOutSz = aMultiTextWnd.GetOutputSize();

        aScrollBar.SetVisibleSize( aOutSz.Height() );
        aScrollBar.SetPageSize( aOutSz.Height() );
        aScrollBar.SetLineSize( aMultiTextWnd.GetTextHeight() );
        aScrollBar.SetRange( Range( 0, aMultiTextWnd.GetEditEngTxtHeight() ) );

        aScrollBar.Resize();
        aScrollBar.Show();
    }
    else
    {
        aButton.SetSymbol( SYMBOL_SPIN_DOWN  );
        aButton.SetQuickHelpText( ScResId( SCSTR_QHELP_EXPAND_FORMULA ) );
        aScrollBar.Hide();
    }

    aButton.SetPosPixel(Point(aSize.Width() - aButton.GetSizePixel().Width(), 0));

    Invalidate();
}

void ScInputBarGroup::StopEditEngine( bool bAll )
{
    aMultiTextWnd.StopEditEngine( bAll );
}

void ScInputBarGroup::StartEditEngine()
{
    aMultiTextWnd.StartEditEngine();
}


void ScInputBarGroup::MakeDialogEditView()
{
    aMultiTextWnd.MakeDialogEditView();
}


EditView* ScInputBarGroup::GetEditView()
{
    return aMultiTextWnd.GetEditView();
}

bool ScInputBarGroup::IsInputActive()
{
    return aMultiTextWnd.IsInputActive();
}

void ScInputBarGroup::SetFormulaMode(bool bSet)
{
    aMultiTextWnd.SetFormulaMode(bSet);
}

void ScInputBarGroup::IncrementVerticalSize()
{
    aMultiTextWnd.SetNumLines( aMultiTextWnd.GetNumLines() + 1 );
    TriggerToolboxLayout();
}

void ScInputBarGroup::DecrementVerticalSize()
{
    if ( aMultiTextWnd.GetNumLines() > 1 )
    {
        aMultiTextWnd.SetNumLines( aMultiTextWnd.GetNumLines() - 1 );
        TriggerToolboxLayout();
    }
}

IMPL_LINK_NOARG(ScInputBarGroup, ClickHdl)
{
    Window *w=GetParent();
    ScInputWindow *pParent;
    pParent=dynamic_cast<ScInputWindow*>(w);

    if(pParent==NULL)
    {
        OSL_FAIL("The parent window pointer pParent is null");
        return 1;
    }
    if( aMultiTextWnd.GetNumLines() > 1 )
    {
        aMultiTextWnd.SetNumLines( 1 );
    }
    else
    {
        aMultiTextWnd.SetNumLines( aMultiTextWnd.GetLastNumExpandedLines() );
    }
    TriggerToolboxLayout();
    
    if (  SC_MOD()->GetInputHdl()->IsTopMode() )
        aMultiTextWnd.GrabFocus();
    return 0;
}

void ScInputBarGroup::TriggerToolboxLayout()
{
    Window *w=GetParent();
    ScInputWindow *pParent;
    pParent=dynamic_cast<ScInputWindow*>(w);
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();

    
    
    
    if ( !nVertOffset )
        nVertOffset = pParent->GetItemPosRect( pParent->GetItemCount() - 1 ).Top();

    if ( pViewFrm )
    {
        Reference< com::sun::star::beans::XPropertySet > xPropSet( pViewFrm->GetFrame().GetFrameInterface(), UNO_QUERY );
        Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager;

        if ( xPropSet.is() )
        {
            com::sun::star::uno::Any aValue = xPropSet->getPropertyValue("LayoutManager");
            aValue >>= xLayoutManager;
        }

        if ( xLayoutManager.is() )
        {
            if ( aMultiTextWnd.GetNumLines() > 1)
                pParent->SetToolbarLayoutMode( TBX_LAYOUT_LOCKVERT );
            else
                pParent->SetToolbarLayoutMode( TBX_LAYOUT_NORMAL );
            xLayoutManager->lock();
            DataChangedEvent aFakeUpdate( DATACHANGED_SETTINGS, NULL,  SETTINGS_STYLE );
            
            
            
            
            
            
            pParent->DataChanged( aFakeUpdate);
            
            
            
            pParent->Resize();
            
            xLayoutManager->unlock();
        }
    }
}

IMPL_LINK_NOARG(ScInputBarGroup, Impl_ScrollHdl)
{
    aMultiTextWnd.DoScroll();
    return 0;
}

void ScInputBarGroup::TextGrabFocus()
{
    aMultiTextWnd.TextGrabFocus();
}





ScMultiTextWnd::ScMultiTextWnd( ScInputBarGroup* pParen, ScTabViewShell* pViewSh )
    :
        ScTextWnd( pParen, pViewSh ),
        mrGroupBar(* pParen ),
        mnLines( 1 ),
        mnLastExpandedLines( INPUTWIN_MULTILINES ),
        mbInvalidate( false )
{
    Size aBorder;
    aBorder = CalcWindowSize( aBorder);
    mnBorderHeight = aBorder.Height();
}

ScMultiTextWnd::~ScMultiTextWnd()
{
}

void ScMultiTextWnd::Paint( const Rectangle& rRect )
{
    EditView* pView = GetEditView();
    if ( pView )
    {
        if ( mbInvalidate )
        {
            pView->Invalidate();
            mbInvalidate = false;
        }
        pEditView->Paint( rRect );
    }
}

EditView* ScMultiTextWnd::GetEditView()
{
    if ( !pEditView )
        InitEditEngine();
    return pEditView;
}

long ScMultiTextWnd::GetPixelHeightForLines( long nLines )
{
    
    return ( nLines * LogicToPixel( Size( 0, GetTextHeight() ) ).Height() ) + mnBorderHeight;
}

void ScMultiTextWnd::SetNumLines( long nLines )
{
    mnLines = nLines;
    if ( nLines > 1 )
    {
        mnLastExpandedLines = nLines;
        Resize();
    }
}

void ScMultiTextWnd::Resize()
{
    
    
    Size aTextBoxSize = GetSizePixel();

    aTextBoxSize.Height() = GetPixelHeightForLines( mnLines );
    SetSizePixel( aTextBoxSize );

    if(pEditView)
    {
        Size aOutputSize = GetOutputSizePixel();
        Rectangle aOutputArea = PixelToLogic( Rectangle( Point(), aOutputSize ));
        pEditView->SetOutputArea( aOutputArea );

        
        long nMaxVisAreaTop = pEditEngine->GetTextHeight() - aOutputArea.GetHeight();
        if (pEditView->GetVisArea().Top() > nMaxVisAreaTop)
        {
            pEditView->Scroll(0, pEditView->GetVisArea().Top() - nMaxVisAreaTop);
        }

        pEditEngine->SetPaperSize( PixelToLogic( Size( aOutputSize.Width(), 10000 ) ) );
    }

    SetScrollBarRange();
}

IMPL_LINK(ScMultiTextWnd, ModifyHdl, EENotify*, pNotify)
{
    ScTextWnd::NotifyHdl( pNotify );
    return 0;
}

IMPL_LINK(ScMultiTextWnd, NotifyHdl, EENotify*, pNotify)
{
    
    
    
    

    if ( pNotify && ( pNotify->eNotificationType == EE_NOTIFY_TEXTVIEWSCROLLED
                 ||   pNotify->eNotificationType == EE_NOTIFY_TEXTHEIGHTCHANGED ) )
        SetScrollBarRange();
    return 0;
}

long ScMultiTextWnd::GetEditEngTxtHeight()
{
    return pEditView ? pEditView->GetEditEngine()->GetTextHeight() : 0;
}

void ScMultiTextWnd::SetScrollBarRange()
{
    if ( pEditView )
    {
        ScrollBar& rVBar = mrGroupBar.GetScrollBar();
        rVBar.SetRange( Range( 0, GetEditEngTxtHeight() ) );
        long currentDocPos = pEditView->GetVisArea().TopLeft().Y();
        rVBar.SetThumbPos( currentDocPos );
    }
}

void
ScMultiTextWnd::DoScroll()
{
    if ( pEditView )
    {
        ScrollBar& rVBar = mrGroupBar.GetScrollBar();
        long currentDocPos = pEditView->GetVisArea().TopLeft().Y();
        long nDiff = currentDocPos - rVBar.GetThumbPos();
        pEditView->Scroll( 0, nDiff );
        currentDocPos = pEditView->GetVisArea().TopLeft().Y();
        rVBar.SetThumbPos( currentDocPos );
    }
}

void ScMultiTextWnd::StartEditEngine()
{
    
    SfxObjectShell* pObjSh = SfxObjectShell::Current();
    if ( pObjSh && pObjSh->IsInModalMode() )
        return;

    if ( !pEditView || !pEditEngine )
    {
        InitEditEngine();
    }

    SC_MOD()->SetInputMode( SC_INPUT_TOP );

    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    if (pViewFrm)
        pViewFrm->GetBindings().Invalidate( SID_ATTR_INSERT );
}

static void lcl_ExtendEditFontAttribs( SfxItemSet& rSet )
{
    const SfxPoolItem& rFontItem = rSet.Get( EE_CHAR_FONTINFO );
    rSet.Put( rFontItem, EE_CHAR_FONTINFO_CJK );
    rSet.Put( rFontItem, EE_CHAR_FONTINFO_CTL );
    const SfxPoolItem& rHeightItem = rSet.Get( EE_CHAR_FONTHEIGHT );
    rSet.Put( rHeightItem, EE_CHAR_FONTHEIGHT_CJK );
    rSet.Put( rHeightItem, EE_CHAR_FONTHEIGHT_CTL );
    const SfxPoolItem& rWeightItem = rSet.Get( EE_CHAR_WEIGHT );
    rSet.Put( rWeightItem, EE_CHAR_WEIGHT_CJK );
    rSet.Put( rWeightItem, EE_CHAR_WEIGHT_CTL );
    const SfxPoolItem& rItalicItem = rSet.Get( EE_CHAR_ITALIC );
    rSet.Put( rItalicItem, EE_CHAR_ITALIC_CJK );
    rSet.Put( rItalicItem, EE_CHAR_ITALIC_CTL );
    const SfxPoolItem& rLangItem = rSet.Get( EE_CHAR_LANGUAGE );
    rSet.Put( rLangItem, EE_CHAR_LANGUAGE_CJK );
    rSet.Put( rLangItem, EE_CHAR_LANGUAGE_CTL );
}

static void lcl_ModifyRTLDefaults( SfxItemSet& rSet )
{
    rSet.Put( SvxAdjustItem( SVX_ADJUST_RIGHT, EE_PARA_JUST ) );

    
    

    
    
    
    SvxLineSpacingItem aItem( SVX_LINESPACE_TWO_LINES, EE_PARA_SBL );
    aItem.SetPropLineSpace( 200 );
    rSet.Put( aItem );
}

static void lcl_ModifyRTLVisArea( EditView* pEditView )
{
    Rectangle aVisArea = pEditView->GetVisArea();
    Size aPaper = pEditView->GetEditEngine()->GetPaperSize();
    long nDiff = aPaper.Width() - aVisArea.Right();
    aVisArea.Left()  += nDiff;
    aVisArea.Right() += nDiff;
    pEditView->SetVisArea(aVisArea);
}


void ScMultiTextWnd::InitEditEngine()
{
    ScFieldEditEngine* pNew;
    ScTabViewShell* pViewSh = GetViewShell();
    ScDocShell* pDocSh = NULL;
    if ( pViewSh )
    {
        pDocSh = pViewSh->GetViewData()->GetDocShell();
        ScDocument* pDoc = pViewSh->GetViewData()->GetDocument();
        pNew = new ScFieldEditEngine(pDoc, pDoc->GetEnginePool(), pDoc->GetEditPool());
    }
    else
        pNew = new ScFieldEditEngine(NULL, EditEngine::CreatePool(), NULL, true);
    pNew->SetExecuteURL( false );
    pEditEngine = pNew;

    Size barSize=GetSizePixel();
    pEditEngine->SetUpdateMode( false );
    pEditEngine->SetPaperSize( PixelToLogic(Size(barSize.Width(),10000)) );
    pEditEngine->SetWordDelimiters(
                    ScEditUtil::ModifyDelimiters( pEditEngine->GetWordDelimiters() ) );

    UpdateAutoCorrFlag();

    {
        SfxItemSet* pSet = new SfxItemSet( pEditEngine->GetEmptyItemSet() );
        pEditEngine->SetFontInfoInItemSet( *pSet, aTextFont );
        lcl_ExtendEditFontAttribs( *pSet );
        
        pSet->Put( SvxScriptSpaceItem( false, EE_PARA_ASIANCJKSPACING ) );
        if ( bIsRTL )
            lcl_ModifyRTLDefaults( *pSet );
        pEditEngine->SetDefaults( pSet );
    }

    
    

    sal_Bool bFilled = false;
    ScInputHandler* pHdl = SC_MOD()->GetInputHdl();
    if ( pHdl )			
        bFilled = pHdl->GetTextAndFields( *pEditEngine );

    pEditEngine->SetUpdateMode( true );

    
    if (bFilled && pEditEngine->GetText() == aString)
        Invalidate();						
    else
        pEditEngine->SetText(aString);		

    pEditView = new EditView( pEditEngine, this );
    pEditView->SetInsertMode(bIsInsertMode);

    
    sal_uLong n = pEditView->GetControlWord();
    pEditView->SetControlWord( n | EV_CNTRL_SINGLELINEPASTE	);

    pEditEngine->InsertView( pEditView, EE_APPEND );

    Resize();

    if ( bIsRTL )
        lcl_ModifyRTLVisArea( pEditView );

    pEditEngine->SetModifyHdl(LINK(this, ScMultiTextWnd, ModifyHdl));
    pEditEngine->SetNotifyHdl(LINK(this, ScMultiTextWnd, NotifyHdl));

    if (!maAccTextDatas.empty())
        maAccTextDatas.back()->StartEdit();

    
    
    if (pDocSh)
    {
        ScDocument* pDoc = pDocSh->GetDocument();	
        sal_uInt8 nScript = pDoc->GetStringScriptType( aString );
        if ( nScript & SCRIPTTYPE_COMPLEX )
            Invalidate();
    }
}

void ScMultiTextWnd::StopEditEngine( bool bAll )
{
    if ( pEditEngine )
        pEditEngine->SetNotifyHdl(Link());
    ScTextWnd::StopEditEngine( bAll );
}

void ScMultiTextWnd::SetTextString( const OUString& rNewString )
{
    
    
    
    
    
    mbInvalidate = true; 
    ScTextWnd::SetTextString( rNewString );
    SetScrollBarRange();
    DoScroll();
}




ScTextWnd::ScTextWnd( Window* pParent, ScTabViewShell* pViewSh )
    :   ScTextWndBase        ( pParent, WinBits(WB_HIDE | WB_BORDER) ),
        DragSourceHelper( this ),
        pEditEngine  ( NULL ),
        pEditView    ( NULL ),
        bIsInsertMode( true ),
        bFormulaMode ( false ),
        bInputMode   ( false ),
        mpViewShell(pViewSh)
{
    EnableRTL( false );     

    bIsRTL = GetSettings().GetLayoutRTL();

    
    Font aAppFont = GetFont();
    aTextFont = aAppFont;
    aTextFont.SetSize( PixelToLogic( aAppFont.GetSize(), MAP_TWIP ) );  

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    Color aBgColor= rStyleSettings.GetWindowColor();
    Color aTxtColor= rStyleSettings.GetWindowTextColor();

    aTextFont.SetTransparent ( true );
    aTextFont.SetFillColor   ( aBgColor );
    
    aTextFont.SetColor       (aTxtColor);
    aTextFont.SetWeight      ( WEIGHT_NORMAL );

    Size aSize(1,TBX_WINDOW_HEIGHT);
    Size aMinEditSize( Edit::GetMinimumEditSize() );
    if( aMinEditSize.Height() > aSize.Height() )
        aSize.Height() = aMinEditSize.Height();
    SetSizePixel        ( aSize );
    SetBackground       ( aBgColor );
    SetLineColor        ( COL_BLACK );
    SetMapMode          ( MAP_TWIP );
    SetPointer          ( POINTER_TEXT );
    SetFont( aTextFont );
}

ScTextWnd::~ScTextWnd()
{
    while (!maAccTextDatas.empty()) {
        maAccTextDatas.back()->Dispose();
    }
    delete pEditView;
    delete pEditEngine;
}

void ScTextWnd::Paint( const Rectangle& rRect )
{
    if (pEditView)
        pEditView->Paint( rRect );
    else
    {
        SetFont( aTextFont );

        long nDiff =  GetOutputSizePixel().Height()
                    - LogicToPixel( Size( 0, GetTextHeight() ) ).Height();


        long nStartPos = 0;
        if ( bIsRTL )
        {
            
            nStartPos += GetOutputSizePixel().Width() -
                        LogicToPixel( Size( GetTextWidth( aString ), 0 ) ).Width();

            
        }

        DrawText( PixelToLogic( Point( nStartPos, nDiff/2 ) ), aString );
    }
}

void ScTextWnd::Resize()
{
    if (pEditView)
    {
        Size aSize = GetOutputSizePixel();
        long nDiff =  aSize.Height()
                    - LogicToPixel( Size( 0, GetTextHeight() ) ).Height();

        pEditView->SetOutputArea(
            PixelToLogic( Rectangle( Point( 0, (nDiff > 0) ? nDiff/2 : 1 ),
                                     aSize ) ) );
    }
}

void ScTextWnd::MouseMove( const MouseEvent& rMEvt )
{
    if (pEditView)
        pEditView->MouseMove( rMEvt );
}

void ScTextWnd::MouseButtonDown( const MouseEvent& rMEvt )
{
    if (!HasFocus())
    {
        StartEditEngine();
        if ( SC_MOD()->IsEditMode() )
            GrabFocus();
    }

    if (pEditView)
    {
        pEditView->SetEditEngineUpdateMode( true );
        pEditView->MouseButtonDown( rMEvt );
    }
}

void ScTextWnd::MouseButtonUp( const MouseEvent& rMEvt )
{
    if (pEditView)
        if (pEditView->MouseButtonUp( rMEvt ))
        {
            if ( rMEvt.IsMiddle() &&
                     GetSettings().GetMouseSettings().GetMiddleButtonAction() == MOUSE_MIDDLE_PASTESELECTION )
            {
                
                SC_MOD()->InputChanged( pEditView );
            }
            else
                SC_MOD()->InputSelection( pEditView );
        }
}

void ScTextWnd::Command( const CommandEvent& rCEvt )
{
    bInputMode = true;
    sal_uInt16 nCommand = rCEvt.GetCommand();
    if ( pEditView /* && nCommand == COMMAND_STARTDRAG */ )
    {
        ScModule* pScMod = SC_MOD();
        ScTabViewShell* pStartViewSh = ScTabViewShell::GetActiveViewShell();

        
        

        
        pScMod->SetInEditCommand( true );
        pEditView->Command( rCEvt );
        pScMod->SetInEditCommand( false );

        
        
        

        if ( nCommand == COMMAND_STARTDRAG )
        {
            
            ScTabViewShell* pEndViewSh = ScTabViewShell::GetActiveViewShell();
            if ( pEndViewSh != pStartViewSh && pStartViewSh != NULL )
            {
                ScViewData* pViewData = pStartViewSh->GetViewData();
                ScInputHandler* pHdl = pScMod->GetInputHdl( pStartViewSh );
                if ( pHdl && pViewData->HasEditView( pViewData->GetActivePart() ) )
                {
                    pHdl->CancelHandler();
                    pViewData->GetView()->ShowCursor();     
                }
            }
        }
        else if ( nCommand == COMMAND_CURSORPOS )
        {
            
        }
        else if ( nCommand == COMMAND_INPUTLANGUAGECHANGE )
        {
            
            

            SfxViewFrame* pViewFrm = SfxViewFrame::Current();
            if (pViewFrm)
            {
                SfxBindings& rBindings = pViewFrm->GetBindings();
                rBindings.Invalidate( SID_ATTR_CHAR_FONT );
                rBindings.Invalidate( SID_ATTR_CHAR_FONTHEIGHT );
            }
        }
        else if ( nCommand == COMMAND_WHEEL )
        {
            
        }
        else
            SC_MOD()->InputChanged( pEditView );
    }
    else
        Window::Command(rCEvt);     

    bInputMode = false;
}

void ScTextWnd::StartDrag( sal_Int8 /* nAction */, const Point& rPosPixel )
{
    if ( pEditView )
    {
        CommandEvent aDragEvent( rPosPixel, COMMAND_STARTDRAG, true );
        pEditView->Command( aDragEvent );

        
        
    }
}

void ScTextWnd::KeyInput(const KeyEvent& rKEvt)
{
    bInputMode = true;
    if (!SC_MOD()->InputKeyEvent( rKEvt ))
    {
        sal_Bool bUsed = false;
        ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
        if ( pViewSh )
            bUsed = pViewSh->SfxKeyInput(rKEvt);    
        if (!bUsed)
            Window::KeyInput( rKEvt );
    }
    bInputMode = false;
}

void ScTextWnd::GetFocus()
{
    ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
    if ( pViewSh )
        pViewSh->SetFormShellAtTop( false );     
}

void ScTextWnd::LoseFocus()
{
}

OUString ScTextWnd::GetText() const
{
    

    if ( pEditEngine )
        return pEditEngine->GetText();
    else
        return GetTextString();
}

void ScTextWnd::SetFormulaMode( bool bSet )
{
    if ( bSet != bFormulaMode )
    {
        bFormulaMode = bSet;
        UpdateAutoCorrFlag();
    }
}

void ScTextWnd::UpdateAutoCorrFlag()
{
    if ( pEditEngine )
    {
        sal_uLong nControl = pEditEngine->GetControlWord();
        sal_uLong nOld = nControl;
        if ( bFormulaMode )
            nControl &= ~EE_CNTRL_AUTOCORRECT;      
        else
            nControl |= EE_CNTRL_AUTOCORRECT;       
        if ( nControl != nOld )
            pEditEngine->SetControlWord( nControl );
    }
}

ScTabViewShell* ScTextWnd::GetViewShell()
{
    return mpViewShell;
}

void ScTextWnd::StartEditEngine()
{
    
    SfxObjectShell* pObjSh = SfxObjectShell::Current();
    if ( pObjSh && pObjSh->IsInModalMode() )
        return;

    if ( !pEditView || !pEditEngine )
    {
        ScFieldEditEngine* pNew;
        ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
        if ( pViewSh )
        {
            ScDocument* pDoc = pViewSh->GetViewData()->GetDocument();
            pNew = new ScFieldEditEngine(pDoc, pDoc->GetEnginePool(), pDoc->GetEditPool());
        }
        else
            pNew = new ScFieldEditEngine(NULL, EditEngine::CreatePool(), NULL, true);
        pNew->SetExecuteURL( false );
        pEditEngine = pNew;

        pEditEngine->SetUpdateMode( false );
        pEditEngine->SetPaperSize( Size( bIsRTL ? USHRT_MAX : THESIZE, 300 ) );
        pEditEngine->SetWordDelimiters(
                        ScEditUtil::ModifyDelimiters( pEditEngine->GetWordDelimiters() ) );

        UpdateAutoCorrFlag();

        {
            SfxItemSet* pSet = new SfxItemSet( pEditEngine->GetEmptyItemSet() );
            pEditEngine->SetFontInfoInItemSet( *pSet, aTextFont );
            lcl_ExtendEditFontAttribs( *pSet );
            
            pSet->Put( SvxScriptSpaceItem( false, EE_PARA_ASIANCJKSPACING ) );
            if ( bIsRTL )
                lcl_ModifyRTLDefaults( *pSet );
            pEditEngine->SetDefaults( pSet );
        }

        
        

        sal_Bool bFilled = false;
        ScInputHandler* pHdl = SC_MOD()->GetInputHdl();
        if ( pHdl )         
            bFilled = pHdl->GetTextAndFields( *pEditEngine );

        pEditEngine->SetUpdateMode( true );

        
        if (bFilled && pEditEngine->GetText() == aString)
            Invalidate();                       
        else
            pEditEngine->SetText(aString);      

        pEditView = new EditView( pEditEngine, this );
        pEditView->SetInsertMode(bIsInsertMode);

        
        sal_uLong n = pEditView->GetControlWord();
        pEditView->SetControlWord( n | EV_CNTRL_SINGLELINEPASTE );

        pEditEngine->InsertView( pEditView, EE_APPEND );

        Resize();

        if ( bIsRTL )
            lcl_ModifyRTLVisArea( pEditView );

        pEditEngine->SetModifyHdl(LINK(this, ScTextWnd, NotifyHdl));

        if (!maAccTextDatas.empty())
            maAccTextDatas.back()->StartEdit();

        
        

        if ( pObjSh && pObjSh->ISA(ScDocShell) )
        {
            ScDocument* pDoc = ((ScDocShell*)pObjSh)->GetDocument();    
            sal_uInt8 nScript = pDoc->GetStringScriptType( aString );
            if ( nScript & SCRIPTTYPE_COMPLEX )
                Invalidate();
        }
    }

    SC_MOD()->SetInputMode( SC_INPUT_TOP );

    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    if (pViewFrm)
        pViewFrm->GetBindings().Invalidate( SID_ATTR_INSERT );
}

IMPL_LINK_NOARG(ScTextWnd, NotifyHdl)
{
    if (pEditView && !bInputMode)
    {
        ScInputHandler* pHdl = SC_MOD()->GetInputHdl();

        
        

        if ( pHdl && !pHdl->IsInOwnChange() )
            pHdl->InputChanged( pEditView, true );  
    }

    return 0;
}

void ScTextWnd::StopEditEngine( bool bAll )
{
    if (pEditView)
    {
        if (!maAccTextDatas.empty())
            maAccTextDatas.back()->EndEdit();

        ScModule* pScMod = SC_MOD();

        if (!bAll)
            pScMod->InputSelection( pEditView );
        aString = pEditEngine->GetText();
        bIsInsertMode = pEditView->IsInsertMode();
        sal_Bool bSelection = pEditView->HasSelection();
        pEditEngine->SetModifyHdl(Link());
        DELETEZ(pEditView);
        DELETEZ(pEditEngine);

        if ( pScMod->IsEditMode() && !bAll )
            pScMod->SetInputMode(SC_INPUT_TABLE);

        SfxViewFrame* pViewFrm = SfxViewFrame::Current();
        if (pViewFrm)
            pViewFrm->GetBindings().Invalidate( SID_ATTR_INSERT );

        if (bSelection)
            Invalidate();           
    }
}

static sal_Int32 findFirstNonMatchingChar(const OUString& rStr1, const OUString rStr2)
{
    
    const sal_Unicode*  pStr1 = rStr1.getStr();
    const sal_Unicode*  pStr2 = rStr2.getStr();
    sal_Int32      i = 0;
    while ( i < rStr1.getLength() )
    {
        
        if ( *pStr1 != *pStr2 )
            return i;
        ++pStr1,
        ++pStr2,
        ++i;
    }

    return i;
}

void ScTextWnd::SetTextString( const OUString& rNewString )
{
    if ( rNewString != aString )
    {
        bInputMode = true;

        

        if (!pEditEngine)
        {
            sal_Bool bPaintAll;
            if ( bIsRTL )
                bPaintAll = sal_True;
            else
            {
                
                sal_uInt8 nOldScript = 0;
                sal_uInt8 nNewScript = 0;
                SfxObjectShell* pObjSh = SfxObjectShell::Current();
                if ( pObjSh && pObjSh->ISA(ScDocShell) )
                {
                    
                    ScDocument* pDoc = ((ScDocShell*)pObjSh)->GetDocument();
                    nOldScript = pDoc->GetStringScriptType( aString );
                    nNewScript = pDoc->GetStringScriptType( rNewString );
                }
                bPaintAll = ( nOldScript & SCRIPTTYPE_COMPLEX ) || ( nNewScript & SCRIPTTYPE_COMPLEX );
            }

            if ( bPaintAll )
            {
                
                Invalidate();
            }
            else
            {
                long nTextSize = 0;
                sal_Int32 nDifPos;
                if (rNewString.getLength() > aString.getLength())
                    nDifPos = findFirstNonMatchingChar(rNewString, aString);
                else
                    nDifPos = findFirstNonMatchingChar(aString, rNewString);

                long nSize1 = GetTextWidth(aString);
                long nSize2 = GetTextWidth(rNewString);
                if ( nSize1>0 && nSize2>0 )
                    nTextSize = std::max( nSize1, nSize2 );
                else
                    nTextSize = GetOutputSize().Width();        

                Point aLogicStart = PixelToLogic(Point(0,0));
                long nStartPos = aLogicStart.X();
                long nInvPos = nStartPos;
                if (nDifPos)
                    nInvPos += GetTextWidth(aString,0,nDifPos);

                sal_uInt16 nFlags = 0;
                if ( nDifPos == aString.getLength() )         
                    nFlags = INVALIDATE_NOERASE;        
                Invalidate( Rectangle( nInvPos, 0,
                                        nStartPos+nTextSize, GetOutputSize().Height()-1 ),
                            nFlags );
            }
        }
        else
        {
            pEditEngine->SetText(rNewString);
        }

        aString = rNewString;

        if (!maAccTextDatas.empty())
            maAccTextDatas.back()->TextChanged();

        bInputMode = false;
    }
}

const OUString& ScTextWnd::GetTextString() const
{
    return aString;
}

bool ScTextWnd::IsInputActive()
{
    return HasFocus();
}

EditView* ScTextWnd::GetEditView()
{
    return pEditView;
}

void ScTextWnd::MakeDialogEditView()
{
    if ( pEditView ) return;

    ScFieldEditEngine* pNew;
    ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
    if ( pViewSh )
    {
        ScDocument* pDoc = pViewSh->GetViewData()->GetDocument();
        pNew = new ScFieldEditEngine(pDoc, pDoc->GetEnginePool(), pDoc->GetEditPool());
    }
    else
        pNew = new ScFieldEditEngine(NULL, EditEngine::CreatePool(), NULL, true);
    pNew->SetExecuteURL( false );
    pEditEngine = pNew;

    pEditEngine->SetUpdateMode( false );
    pEditEngine->SetWordDelimiters( pEditEngine->GetWordDelimiters() + "=" );
    pEditEngine->SetPaperSize( Size( bIsRTL ? USHRT_MAX : THESIZE, 300 ) );

    SfxItemSet* pSet = new SfxItemSet( pEditEngine->GetEmptyItemSet() );
    pEditEngine->SetFontInfoInItemSet( *pSet, aTextFont );
    lcl_ExtendEditFontAttribs( *pSet );
    if ( bIsRTL )
        lcl_ModifyRTLDefaults( *pSet );
    pEditEngine->SetDefaults( pSet );
    pEditEngine->SetUpdateMode( true );

    pEditView   = new EditView( pEditEngine, this );
    pEditEngine->InsertView( pEditView, EE_APPEND );

    Resize();

    if ( bIsRTL )
        lcl_ModifyRTLVisArea( pEditView );

    if (!maAccTextDatas.empty())
        maAccTextDatas.back()->StartEdit();
}

void ScTextWnd::ImplInitSettings()
{
    bIsRTL = GetSettings().GetLayoutRTL();

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    Color aBgColor= rStyleSettings.GetWindowColor();
    Color aTxtColor= rStyleSettings.GetWindowTextColor();

    aTextFont.SetFillColor   ( aBgColor );
    aTextFont.SetColor       (aTxtColor);
    SetBackground           ( aBgColor );
    Invalidate();
}

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > ScTextWnd::CreateAccessible()
{
    return new ScAccessibleEditObject(GetAccessibleParentWindow()->GetAccessible(), NULL, this,
        OUString(ScResId(STR_ACC_EDITLINE_NAME)),
        OUString(ScResId(STR_ACC_EDITLINE_DESCR)), ScAccessibleEditObject::EditLine);
}

void ScTextWnd::InsertAccessibleTextData( ScAccessibleEditLineTextData& rTextData )
{
    OSL_ENSURE( ::std::find( maAccTextDatas.begin(), maAccTextDatas.end(), &rTextData ) == maAccTextDatas.end(),
        "ScTextWnd::InsertAccessibleTextData - passed object already registered" );
    maAccTextDatas.push_back( &rTextData );
}

void ScTextWnd::RemoveAccessibleTextData( ScAccessibleEditLineTextData& rTextData )
{
    AccTextDataVector::iterator aEnd = maAccTextDatas.end();
    AccTextDataVector::iterator aIt = ::std::find( maAccTextDatas.begin(), aEnd, &rTextData );
    OSL_ENSURE( aIt != aEnd, "ScTextWnd::RemoveAccessibleTextData - passed object not registered" );
    if( aIt != aEnd )
        maAccTextDatas.erase( aIt );
}



void ScTextWnd::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
    else
        Window::DataChanged( rDCEvt );
}

void ScTextWnd::TextGrabFocus()
{
    GrabFocus();
}





ScPosWnd::ScPosWnd( Window* pParent ) :
    ComboBox    ( pParent, WinBits(WB_HIDE | WB_DROPDOWN) ),
    pAccel      ( NULL ),
    nTipVisible ( 0 ),
    bFormulaMode( false )
{
    Size aSize( GetTextWidth( OUString("GW99999:GW99999") ),
                GetTextHeight() );
    aSize.Width() += 25;    
    aSize.Height() = CalcWindowSizePixel(11);       
    SetSizePixel( aSize );

    FillRangeNames();

    StartListening( *SFX_APP() );       
}

ScPosWnd::~ScPosWnd()
{
    EndListening( *SFX_APP() );

    HideTip();

    delete pAccel;
}

void ScPosWnd::SetFormulaMode( bool bSet )
{
    if ( bSet != bFormulaMode )
    {
        bFormulaMode = bSet;

        if ( bSet )
            FillFunctions();
        else
            FillRangeNames();

        HideTip();
    }
}

void ScPosWnd::SetPos( const OUString& rPosStr )
{
    if ( aPosStr != rPosStr )
    {
        aPosStr = rPosStr;
        SetText(aPosStr);
    }
}

namespace {

OUString createLocalRangeName(const OUString& rName, const OUString& rTableName)
{
    OUStringBuffer aString (rName);
    aString.append(" (");
    aString.append(rTableName);
    aString.append(")");
    return aString.makeStringAndClear();
}

}

void ScPosWnd::FillRangeNames()
{
    Clear();

    SfxObjectShell* pObjSh = SfxObjectShell::Current();
    if ( pObjSh && pObjSh->ISA(ScDocShell) )
    {
        ScDocument* pDoc = ((ScDocShell*)pObjSh)->GetDocument();

        InsertEntry(ScGlobal::GetRscString( STR_MANAGE_NAMES ));
        SetSeparatorPos(0);

        ScRange aDummy;
        std::set<OUString> aSet;
        ScRangeName* pRangeNames = pDoc->GetRangeName();
        if (!pRangeNames->empty())
        {
            ScRangeName::const_iterator itrBeg = pRangeNames->begin(), itrEnd = pRangeNames->end();
            for (ScRangeName::const_iterator itr = itrBeg; itr != itrEnd; ++itr)
            {
                if (itr->second->IsValidReference(aDummy))
                    aSet.insert(itr->second->GetName());
            }
        }
        for (SCTAB i = 0; i < pDoc->GetTableCount(); ++i)
        {
            ScRangeName* pLocalRangeName = pDoc->GetRangeName(i);
            if (pLocalRangeName && !pLocalRangeName->empty())
            {
                OUString aTableName;
                pDoc->GetName(i, aTableName);
                for (ScRangeName::const_iterator itr = pLocalRangeName->begin(); itr != pLocalRangeName->end(); ++itr)
                {
                    if (itr->second->IsValidReference(aDummy))
                        aSet.insert(createLocalRangeName(itr->second->GetName(), aTableName));
                }
            }
        }

        if (!aSet.empty())
        {
            for (std::set<OUString>::iterator itr = aSet.begin();
                    itr != aSet.end(); ++itr)
            {
                InsertEntry(*itr);
            }
        }
    }
    SetText(aPosStr);
}

void ScPosWnd::FillFunctions()
{
    Clear();

    OUString aFirstName;
    const ScAppOptions& rOpt = SC_MOD()->GetAppOptions();
    sal_uInt16 nMRUCount = rOpt.GetLRUFuncListCount();
    const sal_uInt16* pMRUList = rOpt.GetLRUFuncList();
    if (pMRUList)
    {
        const ScFunctionList* pFuncList = ScGlobal::GetStarCalcFunctionList();
        sal_uLong nListCount = pFuncList->GetCount();
        for (sal_uInt16 i=0; i<nMRUCount; i++)
        {
            sal_uInt16 nId = pMRUList[i];
            for (sal_uLong j=0; j<nListCount; j++)
            {
                const ScFuncDesc* pDesc = pFuncList->GetFunction( j );
                if ( pDesc->nFIndex == nId && pDesc->pFuncName )
                {
                    InsertEntry( *pDesc->pFuncName );
                    if (aFirstName.isEmpty())
                        aFirstName = *pDesc->pFuncName;
                    break;  
                }
            }
        }
    }

    
    



    SetText(aFirstName);
}

void ScPosWnd::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( !bFormulaMode )
    {
        

        if ( rHint.ISA(SfxSimpleHint) )
        {
            sal_uLong nHintId = ((SfxSimpleHint&)rHint).GetId();
            if ( nHintId == SC_HINT_AREAS_CHANGED || nHintId == SC_HINT_NAVIGATOR_UPDATEALL)
                FillRangeNames();
        }
        else if ( rHint.ISA(SfxEventHint) )
        {
            sal_uLong nEventId = ((SfxEventHint&)rHint).GetEventId();
            if ( nEventId == SFX_EVENT_ACTIVATEDOC )
                FillRangeNames();
        }
    }
}

void ScPosWnd::HideTip()
{
    if ( nTipVisible )
    {
        Help::HideTip( nTipVisible );
        nTipVisible = 0;
    }
}

static ScNameInputType lcl_GetInputType( const OUString& rText )
{
    ScNameInputType eRet = SC_NAME_INPUT_BAD_NAME;      

    ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
    if ( pViewSh )
    {
        ScViewData* pViewData = pViewSh->GetViewData();
        ScDocument* pDoc = pViewData->GetDocument();
        SCTAB nTab = pViewData->GetTabNo();
        formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();

        

        ScRange aRange;
        ScAddress aAddress;
        ScRangeUtil aRangeUtil;
        SCTAB nNameTab;
        sal_Int32 nNumeric;

        if (rText == ScGlobal::GetRscString(STR_MANAGE_NAMES))
            eRet = SC_MANAGE_NAMES;
        else if ( aRange.Parse( rText, pDoc, eConv ) & SCA_VALID )
            eRet = SC_NAME_INPUT_RANGE;
        else if ( aAddress.Parse( rText, pDoc, eConv ) & SCA_VALID )
            eRet = SC_NAME_INPUT_CELL;
        else if ( aRangeUtil.MakeRangeFromName( rText, pDoc, nTab, aRange, RUTL_NAMES, eConv ) )
            eRet = SC_NAME_INPUT_NAMEDRANGE;
        else if ( aRangeUtil.MakeRangeFromName( rText, pDoc, nTab, aRange, RUTL_DBASE, eConv ) )
            eRet = SC_NAME_INPUT_DATABASE;
        else if ( comphelper::string::isdigitAsciiString( rText ) &&
                  ( nNumeric = rText.toInt32() ) > 0 && nNumeric <= MAXROW+1 )
            eRet = SC_NAME_INPUT_ROW;
        else if ( pDoc->GetTable( rText, nNameTab ) )
            eRet = SC_NAME_INPUT_SHEET;
        else if ( ScRangeData::IsNameValid( rText, pDoc ) )     
        {
            if ( pViewData->GetSimpleArea( aRange ) == SC_MARK_SIMPLE )
                eRet = SC_NAME_INPUT_DEFINE;
            else
                eRet = SC_NAME_INPUT_BAD_SELECTION;
        }
        else
            eRet = SC_NAME_INPUT_BAD_NAME;
    }

    return eRet;
}

void ScPosWnd::Modify()
{
    ComboBox::Modify();

    HideTip();

    if ( !IsTravelSelect() && !bFormulaMode )
    {
        

        ScNameInputType eType = lcl_GetInputType( GetText() );      
        sal_uInt16 nStrId = 0;
        switch ( eType )
        {
            case SC_NAME_INPUT_CELL:
                nStrId = STR_NAME_INPUT_CELL;
                break;
            case SC_NAME_INPUT_RANGE:
            case SC_NAME_INPUT_NAMEDRANGE:
                nStrId = STR_NAME_INPUT_RANGE;      
                break;
            case SC_NAME_INPUT_DATABASE:
                nStrId = STR_NAME_INPUT_DBRANGE;
                break;
            case SC_NAME_INPUT_ROW:
                nStrId = STR_NAME_INPUT_ROW;
                break;
            case SC_NAME_INPUT_SHEET:
                nStrId = STR_NAME_INPUT_SHEET;
                break;
            case SC_NAME_INPUT_DEFINE:
                nStrId = STR_NAME_INPUT_DEFINE;
                break;
            default:
                
                break;
        }

        if ( nStrId )
        {
            

            Window* pWin = GetSubEdit();
            if (!pWin)
                pWin = this;
            Point aPos;
            Cursor* pCur = pWin->GetCursor();
            if (pCur)
                aPos = pWin->LogicToPixel( pCur->GetPos() );
            aPos = pWin->OutputToScreenPixel( aPos );
            Rectangle aRect( aPos, aPos );

            OUString aText = ScGlobal::GetRscString( nStrId );
            sal_uInt16 nAlign = QUICKHELP_LEFT|QUICKHELP_BOTTOM;
            nTipVisible = Help::ShowTip(pWin, aRect, aText, nAlign);
        }
    }
}

void ScPosWnd::Select()
{
    ComboBox::Select();     

    HideTip();

    if (!IsTravelSelect())
        DoEnter();
}

void ScPosWnd::DoEnter()
{
    OUString aText = GetText();
    if ( !aText.isEmpty() )
    {
        if ( bFormulaMode )
        {
            ScModule* pScMod = SC_MOD();
            if ( aText == ScGlobal::GetRscString(STR_FUNCTIONLIST_MORE) )
            {
                
                

                
                SfxViewFrame* pViewFrm = SfxViewFrame::Current();
                if ( pViewFrm && !pViewFrm->GetChildWindow( SID_OPENDLG_FUNCTION ) )
                    pViewFrm->GetDispatcher()->Execute( SID_OPENDLG_FUNCTION,
                                              SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD );
            }
            else
            {
                ScTabViewShell* pViewSh = PTR_CAST( ScTabViewShell, SfxViewShell::Current() );
                ScInputHandler* pHdl = pScMod->GetInputHdl( pViewSh );
                if (pHdl)
                    pHdl->InsertFunction( aText );
            }
        }
        else
        {
            

            ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
            if ( pViewSh )
            {
                ScViewData* pViewData = pViewSh->GetViewData();
                ScDocShell* pDocShell = pViewData->GetDocShell();
                ScDocument* pDoc = pDocShell->GetDocument();

                ScNameInputType eType = lcl_GetInputType( aText );
                if ( eType == SC_NAME_INPUT_BAD_NAME || eType == SC_NAME_INPUT_BAD_SELECTION )
                {
                    sal_uInt16 nId = ( eType == SC_NAME_INPUT_BAD_NAME ) ? STR_NAME_ERROR_NAME : STR_NAME_ERROR_SELECTION;
                    pViewSh->ErrorMessage( nId );
                }
                else if ( eType == SC_NAME_INPUT_DEFINE )
                {
                    ScRangeName* pNames = pDoc->GetRangeName();
                    ScRange aSelection;
                    if ( pNames && !pNames->findByUpperName(ScGlobal::pCharClass->uppercase(aText)) &&
                            (pViewData->GetSimpleArea( aSelection ) == SC_MARK_SIMPLE) )
                    {
                        ScRangeName aNewRanges( *pNames );
                        ScAddress aCursor( pViewData->GetCurX(), pViewData->GetCurY(), pViewData->GetTabNo() );
                        OUString aContent(aSelection.Format(SCR_ABS_3D, pDoc, pDoc->GetAddressConvention()));
                        ScRangeData* pNew = new ScRangeData( pDoc, aText, aContent, aCursor );
                        if ( aNewRanges.insert(pNew) )
                        {
                            pDocShell->GetDocFunc().ModifyRangeNames( aNewRanges );
                            pViewSh->UpdateInputHandler(true);
                        }
                    }
                }
                else if (eType == SC_MANAGE_NAMES)
                {
                    sal_uInt16          nId  = ScNameDlgWrapper::GetChildWindowId();
                    SfxViewFrame* pViewFrm = pViewSh->GetViewFrame();
                    SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                    SC_MOD()->SetRefDialog( nId, pWnd ? false : sal_True );
                }
                else
                {
                    
                    if (eType == SC_NAME_INPUT_CELL || eType == SC_NAME_INPUT_RANGE)
                    {
                        
                        
                        ScRange aRange(0,0,pViewData->GetTabNo());
                        aRange.ParseAny(aText, pDoc, pDoc->GetAddressConvention());
                        aText = aRange.Format(SCR_ABS_3D, pDoc, ::formula::FormulaGrammar::CONV_OOO);
                    }

                    SfxStringItem aPosItem( SID_CURRENTCELL, aText );
                    SfxBoolItem aUnmarkItem( FN_PARAM_1, true );        

                    pViewSh->GetViewData()->GetDispatcher().Execute( SID_CURRENTCELL,
                                        SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD,
                                        &aPosItem, &aUnmarkItem, 0L );
                }
            }
        }
    }
    else
        SetText( aPosStr );

    ReleaseFocus_Impl();
}

bool ScPosWnd::Notify( NotifyEvent& rNEvt )
{
    bool nHandled = false;

    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();

        switch ( pKEvt->GetKeyCode().GetCode() )
        {
            case KEY_RETURN:
                DoEnter();
                nHandled = true;
                break;

            case KEY_ESCAPE:
                if (nTipVisible)
                {
                    
                    HideTip();
                }
                else
                {
                    if (!bFormulaMode)
                        SetText( aPosStr );
                    ReleaseFocus_Impl();
                }
                nHandled = true;
                break;
        }
    }

    if ( !nHandled )
        nHandled = ComboBox::Notify( rNEvt );

    if ( rNEvt.GetType() == EVENT_LOSEFOCUS )
        HideTip();

    return nHandled;
}

void ScPosWnd::ReleaseFocus_Impl()
{
    HideTip();

    SfxViewShell* pCurSh = SfxViewShell::Current();
    ScInputHandler* pHdl = SC_MOD()->GetInputHdl( PTR_CAST( ScTabViewShell, pCurSh ) );
    if ( pHdl && pHdl->IsTopMode() )
    {
        

        ScInputWindow* pInputWin = pHdl->GetInputWindow();
        if (pInputWin)
        {
            pInputWin->TextGrabFocus();
            return;
        }
    }

    

    if ( pCurSh )
    {
        Window* pShellWnd = pCurSh->GetWindow();

        if ( pShellWnd )
            pShellWnd->GrabFocus();
    }
}






/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
