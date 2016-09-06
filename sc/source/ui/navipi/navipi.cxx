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

#include <rangelst.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/event.hxx>
#include <sfx2/imgmgr.hxx>
#include <sfx2/navigat.hxx>
#include <svl/stritem.hxx>
#include <svl/urlbmk.hxx>
#include <vcl/settings.hxx>
#include <unotools/charclass.hxx>
#include <stdlib.h>

#include "viewdata.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"
#include "document.hxx"
#include "dbdata.hxx"
#include "rangenam.hxx"
#include "rangeutl.hxx"
#include "popmenu.hxx"
#include "scresid.hxx"
#include "scmod.hxx"
#include "navicfg.hxx"
#include "navcitem.hxx"
#include "navipi.hrc"
#include "navipi.hxx"
#include "navsett.hxx"
#include "markdata.hxx"

#include <algorithm>

//  tolerance, how much spac above the folded size is still small
#define SCNAV_MINTOL        5

//  maximum values for UI
#define SCNAV_MAXCOL        (MAXCOLCOUNT)
// macro is sufficient since only used in ctor
#define SCNAV_COLDIGITS     (static_cast<sal_Int32>( floor( log10( static_cast<double>(SCNAV_MAXCOL)))) + 1)   // 1...256...18278
// precomputed constant because it is used in every change of spin button field
static const sal_Int32 SCNAV_COLLETTERS = ::ScColToAlpha(SCNAV_MAXCOL).getLength();    // A...IV...ZZZ

#define SCNAV_MAXROW        (MAXROWCOUNT)

void ScNavigatorDlg::ReleaseFocus()
{
    SfxViewShell* pCurSh = SfxViewShell::Current();

    if ( pCurSh )
    {
        vcl::Window* pShellWnd = pCurSh->GetWindow();
        if ( pShellWnd )
            pShellWnd->GrabFocus();
    }
}

//  class ColumnEdit

ColumnEdit::ColumnEdit( ScNavigatorDlg* pParent, const ResId& rResId )
    :   SpinField   ( pParent, rResId ),
        rDlg        ( *pParent ),
        nCol        ( 0 ),
        nKeyGroup   ( KEYGROUP_ALPHA )
{
    SetMaxTextLen( SCNAV_COLDIGITS );   // 1...256...18278 or A...IV...ZZZ
}

ColumnEdit::~ColumnEdit()
{
}

bool ColumnEdit::Notify( NotifyEvent& rNEvt )
{
    bool bHandled = SpinField::Notify( rNEvt );

    MouseNotifyEvent nType = rNEvt.GetType();
    if ( nType == MouseNotifyEvent::KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        vcl::KeyCode aCode = pKEvt->GetKeyCode();

        if ( !aCode.IsMod1() && !aCode.IsMod2() )
        {
            //! Input Validation (only alphanumerics, max 2-3 digits)
            //! was before VCL not forwarded keyinput
            //! rethink this!!!

            if ( aCode.GetCode() == KEY_RETURN )
            {
                ScNavigatorDlg::ReleaseFocus();
                ExecuteCol();
                bHandled = true;
            }
        }
    }
    else if ( nType == MouseNotifyEvent::LOSEFOCUS )    // LoseFocus not called at VCL
        EvalText();                         // nCol set

    return bHandled;
}

void ColumnEdit::LoseFocus()
{
    EvalText();
}

void ColumnEdit::Up()
{
    nCol++;

    if ( nCol <= SCNAV_MAXCOL )
        SetCol( nCol );
    else
        nCol--;
}

void ColumnEdit::Down()
{
    if ( nCol>1 )
        SetCol( nCol-1 );
}

void ColumnEdit::First()
{
    nCol = 1;
    SetText(OUString('A'));
}

void ColumnEdit::Last()
{
    OUString aStr;
    nCol = NumToAlpha( SCNAV_MAXCOL, aStr );
    SetText( aStr );
}

void ColumnEdit::EvalText()
{
    OUString aStrCol = GetText();

    if (!aStrCol.isEmpty())
    {
        //  nKeyGroup is no longer set at VCL, in cause of lack of keyinput

        if ( CharClass::isAsciiNumeric(aStrCol) )
            nCol = NumStrToAlpha( aStrCol );
        else
            nCol = AlphaToNum( aStrCol );
    }
    else
        nCol = 0;

    SetText( aStrCol );
    nKeyGroup = KEYGROUP_ALPHA;
}

void ColumnEdit::ExecuteCol()
{
    SCROW nRow = rDlg.aEdRow->GetRow();

    EvalText(); // setzt nCol

    if ( (nCol > 0) && (nRow > 0) )
        rDlg.SetCurrentCell( nCol-1, nRow-1 );
}

void ColumnEdit::SetCol( SCCOL nColNo )
{
    OUString aStr;

    if ( nColNo == 0 )
    {
        nCol = 0;
        SetText( aStr );
    }
    else
    {
        nColNo = NumToAlpha( nColNo, aStr );
        nCol = nColNo;
        SetText( aStr );
    }
}

SCCOL ColumnEdit::AlphaToNum( OUString& rStr )
{
    SCCOL  nColumn = 0;

    if ( CharClass::isAsciiAlpha( rStr) )
    {
        rStr = rStr.toAsciiUpperCase();

        if (::AlphaToCol( nColumn, rStr))
            ++nColumn;

        if ( (rStr.getLength() > SCNAV_COLLETTERS) || (nColumn > SCNAV_MAXCOL) )
        {
            nColumn = SCNAV_MAXCOL;
            NumToAlpha( nColumn, rStr );
        }
    }
    else
        rStr.clear();

    return nColumn;
}

SCCOL ColumnEdit::NumStrToAlpha( OUString& rStr )
{
    SCCOL  nColumn = 0;

    if ( CharClass::isAsciiNumeric(rStr) )
        nColumn = NumToAlpha( (SCCOL)rStr.toInt32(), rStr );
    else
        rStr.clear();

    return nColumn;
}

SCCOL ColumnEdit::NumToAlpha( SCCOL nColNo, OUString& rStr )
{
    if ( nColNo > SCNAV_MAXCOL )
        nColNo = SCNAV_MAXCOL;
    else if ( nColNo < 1 )
        nColNo = 1;

    ::ScColToAlpha( rStr, nColNo - 1);

    return nColNo;
}

//  class RowEdit

RowEdit::RowEdit( ScNavigatorDlg* pParent, const ResId& rResId )
    :   NumericField( pParent, rResId ),
        rDlg        ( *pParent )
{
    SetMax( SCNAV_MAXROW);
    SetLast( SCNAV_MAXROW);
}

RowEdit::~RowEdit()
{
}

bool RowEdit::Notify( NotifyEvent& rNEvt )
{
    bool bHandled = NumericField::Notify( rNEvt );

    if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        vcl::KeyCode aCode = pKEvt->GetKeyCode();
        if ( aCode.GetCode() == KEY_RETURN && !aCode.IsMod1() && !aCode.IsMod2() )
        {
            ScNavigatorDlg::ReleaseFocus();
            ExecuteRow();
            bHandled = true;
        }
    }

    return bHandled;
}

void RowEdit::LoseFocus()
{
}

void RowEdit::ExecuteRow()
{
    SCCOL nCol = rDlg.aEdCol->GetCol();
    SCROW nRow = (SCROW)GetValue();

    if ( (nCol > 0) && (nRow > 0) )
        rDlg.SetCurrentCell( nCol-1, nRow-1 );
}

//  class ScDocListBox

ScDocListBox::ScDocListBox( ScNavigatorDlg* pParent, const ResId& rResId )
    :   ListBox ( pParent, rResId ),
        rDlg    ( *pParent )
{
}

ScDocListBox::~ScDocListBox()
{
}

void ScDocListBox::Select()
{
    ScNavigatorDlg::ReleaseFocus();

    OUString aDocName = GetSelectEntry();
    rDlg.aLbEntries->SelectDoc( aDocName );
}

//  class CommandToolBox

CommandToolBox::CommandToolBox( ScNavigatorDlg* pParent, const ResId& rResId )
    :   ToolBox ( pParent, rResId ),
        rDlg    ( *pParent )
{
    InitImageList();    // ImageList members of ScNavigatorDlg must be initialized before!

    SetSizePixel( CalcWindowSizePixel() );
    SetDropdownClickHdl( LINK(this, CommandToolBox, ToolBoxDropdownClickHdl) );
    SetItemBits( IID_DROPMODE, GetItemBits( IID_DROPMODE ) | ToolBoxItemBits::DROPDOWNONLY );
}

CommandToolBox::~CommandToolBox()
{
}

void CommandToolBox::Select( sal_uInt16 nSelId )
{
    //  Modus umschalten ?

    if ( nSelId == IID_ZOOMOUT || nSelId == IID_SCENARIOS )
    {
        NavListMode eOldMode = rDlg.eListMode;
        NavListMode eNewMode;

        if ( nSelId == IID_SCENARIOS )
        {
            if ( eOldMode == NAV_LMODE_SCENARIOS )
                eNewMode = NAV_LMODE_AREAS;
            else
                eNewMode = NAV_LMODE_SCENARIOS;
        }
        else                                            // on/off
        {
            if ( eOldMode == NAV_LMODE_NONE )
                eNewMode = NAV_LMODE_AREAS;
            else
                eNewMode = NAV_LMODE_NONE;
        }
        rDlg.SetListMode( eNewMode );
        UpdateButtons();
    }
    else
        switch ( nSelId )
        {
            case IID_DATA:
                rDlg.MarkDataArea();
                break;
            case IID_UP:
                rDlg.StartOfDataArea();
                break;
            case IID_DOWN:
                rDlg.EndOfDataArea();
                break;
            case IID_CHANGEROOT:
                rDlg.aLbEntries->ToggleRoot();
                UpdateButtons();
                break;
        }
}

void CommandToolBox::Select()
{
    Select( GetCurItemId() );
}

void CommandToolBox::Click()
{
}

IMPL_LINK_NOARG_TYPED(CommandToolBox, ToolBoxDropdownClickHdl, ToolBox *, void)
{
    // the popup menue of the drop modus has to be called in the
    // click (button down) and not in the select (button up)

    if ( GetCurItemId() == IID_DROPMODE )
    {
        ScPopupMenu aPop( ScResId( RID_POPUP_DROPMODE ) );
        aPop.CheckItem( RID_DROPMODE_URL + rDlg.GetDropMode() );
        aPop.Execute( this, GetItemRect(IID_DROPMODE), PopupMenuFlags::ExecuteDown );
        sal_uInt16 nId = aPop.GetSelected();

        EndSelection();     // bevore SetDropMode (SetDropMode calls SetItemImage)

        if ( nId >= RID_DROPMODE_URL && nId <= RID_DROPMODE_COPY )
            rDlg.SetDropMode( nId - RID_DROPMODE_URL );

        //  reset the highlighted button
        Point aPoint;
        MouseEvent aLeave( aPoint, 0, MouseEventModifiers::LEAVEWINDOW | MouseEventModifiers::SYNTHETIC );
        MouseMove( aLeave );
    }
}

void CommandToolBox::UpdateButtons()
{
    NavListMode eMode = rDlg.eListMode;
    CheckItem( IID_SCENARIOS,   eMode == NAV_LMODE_SCENARIOS );
    CheckItem( IID_ZOOMOUT,     eMode != NAV_LMODE_NONE );

    //  Umschalten-Button:
    if ( eMode == NAV_LMODE_SCENARIOS || eMode == NAV_LMODE_NONE )
    {
        EnableItem( IID_CHANGEROOT, false );
        CheckItem( IID_CHANGEROOT, false );
    }
    else
    {
        EnableItem( IID_CHANGEROOT );
        bool bRootSet = rDlg.aLbEntries->GetRootType() != ScContentId::ROOT;
        CheckItem( IID_CHANGEROOT, bRootSet );
    }

    sal_uInt16 nImageId = 0;
    switch ( rDlg.nDropMode )
    {
        case SC_DROPMODE_URL:   nImageId = RID_IMG_DROP_URL;  break;
        case SC_DROPMODE_LINK:  nImageId = RID_IMG_DROP_LINK; break;
        case SC_DROPMODE_COPY:  nImageId = RID_IMG_DROP_COPY; break;
    }
    SetItemImage( IID_DROPMODE, Image(ScResId(nImageId)) );
}

void CommandToolBox::InitImageList()
{
    ImageList& rImgLst = rDlg.aCmdImageList;

    sal_uInt16 nCount = GetItemCount();
    for (sal_uInt16 i = 0; i < nCount; i++)
    {
        sal_uInt16 nId = GetItemId(i);
        SetItemImage( nId, rImgLst.GetImage( nId ) );
    }
}

void CommandToolBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( rDCEvt.GetType() == DataChangedEventType::SETTINGS && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        //  update item images

        InitImageList();
        UpdateButtons();    // drop mode
    }

    ToolBox::DataChanged( rDCEvt );
}

//  class ScNavigatorSettings

ScNavigatorSettings::ScNavigatorSettings() :
    mnRootSelected( ScContentId::ROOT ),
    mnChildSelected( SC_CONTENT_NOCHILD )
{
}

//  class ScNavigatorDlgWrapper

SFX_IMPL_CHILDWINDOWCONTEXT( ScNavigatorDialogWrapper, SID_NAVIGATOR )

ScNavigatorDialogWrapper::ScNavigatorDialogWrapper(
                                    vcl::Window*          pParent,
                                    sal_uInt16           nId,
                                    SfxBindings*     pBind,
                                    SfxChildWinInfo* /* pInfo */ ) :
        SfxChildWindowContext( nId )
{
    pNavigator = VclPtr<ScNavigatorDlg>::Create( pBind, this, pParent, true );
    SetWindow( pNavigator );

    //  handle configurations elsewhere,
    //  only size of pInfo matters now

    Size aInfoSize = pParent->GetOutputSizePixel();     // outside defined size
    Size aNavSize = pNavigator->GetOutputSizePixel();   // Default-Size

    aNavSize.Width()  = std::max( aInfoSize.Width(),  aNavSize.Width() );
    aNavSize.Height() = std::max( aInfoSize.Height(), aNavSize.Height() );
    pNavigator->nListModeHeight = std::max( aNavSize.Height(), pNavigator->nListModeHeight );

    //  The size could be changed in another module,
    //  therefore we have to or have not to display the content
    //  in dependence of the current size

    bool bSmall = ( aInfoSize.Height() <= pNavigator->aInitSize.Height() + SCNAV_MINTOL );
    NavListMode eNavMode = NAV_LMODE_NONE;
    if (!bSmall)
    {
        //  if scenario was active, switch on

        ScNavipiCfg& rCfg = SC_MOD()->GetNavipiCfg();
        NavListMode eLastMode = (NavListMode) rCfg.GetListMode();
        if ( eLastMode == NAV_LMODE_SCENARIOS )
            eNavMode = NAV_LMODE_SCENARIOS;
        else
            eNavMode = NAV_LMODE_AREAS;
    }

    // Do not set the size of the float again (sal_False at SetListMode), so that the
    // navigator is not expanded, if it was minimized (#38872#).

    pNavigator->SetListMode( eNavMode, false );     // FALSE: do not set the Float size

    sal_uInt16 nCmdId;
    switch (eNavMode)
    {
        case NAV_LMODE_SCENARIOS:   nCmdId = IID_SCENARIOS; break;
        case NAV_LMODE_AREAS:       nCmdId = IID_AREAS;     break;
// The following case can never be reach due to how eNavMode is set-up
//        case NAV_LMODE_DOCS:        nCmdId = IID_DOCS;      break;
//        case NAV_LMODE_DBAREAS:     nCmdId = IID_DBAREAS;   break;
        default:                    nCmdId = 0;
    }
    if (nCmdId)
    {
        pNavigator->aTbxCmd->CheckItem( nCmdId );
        pNavigator->DoResize();
    }

    pNavigator->bFirstBig = ( nCmdId == 0 );    // later
}

void ScNavigatorDialogWrapper::Resizing( Size& rSize )
{
    static_cast<ScNavigatorDlg*>(GetWindow())->Resizing(rSize);
}

// class ScNavigatorPI

#define CTRL_ITEMS 4

#define REGISTER_SLOT(i,id) \
    ppBoundItems[i]=new ScNavigatorControllerItem(id,*this,rBindings);

ScNavigatorDlg::ScNavigatorDlg( SfxBindings* pB, SfxChildWindowContext* pCW, vcl::Window* pParent,
    const bool bUseStyleSettingsBackground) :
        Window( pParent, ScResId(RID_SCDLG_NAVIGATOR) ),
        rBindings   ( *pB ),                                // is used in CommandToolBox ctor
        aCmdImageList( ScResId( IL_CMD ) ),
        aFtCol      ( VclPtr<FixedInfo>::Create( this, ScResId( FT_COL ) ) ),
        aEdCol      ( VclPtr<ColumnEdit>::Create( this, ScResId( ED_COL ) ) ),
        aFtRow      ( VclPtr<FixedInfo>::Create( this, ScResId( FT_ROW ) ) ),
        aEdRow      ( VclPtr<RowEdit>::Create( this, ScResId( ED_ROW ) ) ),
        aTbxCmd     ( VclPtr<CommandToolBox>::Create( this, ScResId( TBX_CMD ) ) ),
        aLbEntries  ( VclPtr<ScContentTree>::Create( this, ScResId( LB_ENTRIES ) ) ),
        aWndScenarios( VclPtr<ScScenarioWindow>::Create( this,ScResId( STR_QHLP_SCEN_LISTBOX), ScResId(STR_QHLP_SCEN_COMMENT)) ),
        aLbDocuments( VclPtr<ScDocListBox>::Create( this, ScResId( LB_DOCUMENTS ) ) ),
        aStrDragMode ( ScResId( STR_DRAGMODE ) ),
        aStrDisplay  ( ScResId( STR_DISPLAY ) ),
        aStrActiveWin( ScResId( STR_ACTIVEWIN ) ),
        pContextWin ( pCW ),
        pMarkArea   ( nullptr ),
        pViewData   ( nullptr ),
        nListModeHeight( 0 ),
        nInitListHeight( 0 ),
        eListMode   ( NAV_LMODE_NONE ),
        nDropMode   ( SC_DROPMODE_URL ),
        nCurCol     ( 0 ),
        nCurRow     ( 0 ),
        nCurTab     ( 0 ),
        bFirstBig   ( false ),
        mbUseStyleSettingsBackground(bUseStyleSettingsBackground)
{
    ScNavipiCfg& rCfg = SC_MOD()->GetNavipiCfg();
    nDropMode = rCfg.GetDragMode();
    //  eListMode is set from outside, Root further below

    aLbDocuments->SetDropDownLineCount(9);
    OUString aOpen(" (");
    aStrActive = aOpen;
    aStrActive += OUString( ScResId( STR_ACTIVE ) );
    aStrActive += ")";                                      // " (active)"
    aStrNotActive = aOpen;
    aStrNotActive += OUString( ScResId( STR_NOTACTIVE ) );
    aStrNotActive += ")";                                   // " (not active)"
    aStrHidden = aOpen;
    aStrHidden += OUString( ScResId( STR_HIDDEN ) );
    aStrHidden += ")";                                      // " (hidden)"

    aTitleBase = GetText();

    const long nListboxYPos =
        ::std::max(
            (aTbxCmd->GetPosPixel().Y() + aTbxCmd->GetSizePixel().Height()),
            (aEdRow->GetPosPixel().Y() + aEdRow->GetSizePixel().Height()) )
        + 4;
    aLbEntries->setPosSizePixel( 0, nListboxYPos, 0, 0, PosSizeFlags::Y);

    nBorderOffset = aLbEntries->GetPosPixel().X();

    aInitSize.Width()  =  aTbxCmd->GetPosPixel().X()
                        + aTbxCmd->GetSizePixel().Width()
                        + nBorderOffset;
    aInitSize.Height() = aLbEntries->GetPosPixel().Y();

    nInitListHeight = aLbEntries->GetSizePixel().Height();
    nListModeHeight =  aInitSize.Height()
                     + nInitListHeight;

    ppBoundItems = new ScNavigatorControllerItem* [CTRL_ITEMS];

    rBindings.ENTERREGISTRATIONS();

    REGISTER_SLOT( 0, SID_CURRENTCELL       );
    REGISTER_SLOT( 1, SID_CURRENTTAB        );
    REGISTER_SLOT( 2, SID_CURRENTDOC        );
    REGISTER_SLOT( 3, SID_SELECT_SCENARIO   );

    rBindings.LEAVEREGISTRATIONS();

    StartListening( *(SfxGetpApp()) );
    StartListening( rBindings );

    aLbDocuments->Hide();        // does not exist at NAV_LMODE_NONE

    aLbEntries->InitWindowBits(true);

    aLbEntries->SetSpaceBetweenEntries(0);
    aLbEntries->SetSelectionMode( SINGLE_SELECTION );
    aLbEntries->SetDragDropMode( DragDropMode::CTRL_MOVE |
                                 DragDropMode::CTRL_COPY |
                                 DragDropMode::ENABLE_TOP );

    //  was a category chosen as root?
    ScContentId nLastRoot = rCfg.GetRootType();
    if ( nLastRoot != ScContentId::ROOT )
        aLbEntries->SetRootType( nLastRoot );

    aLbEntries->Refresh();
    GetDocNames();

    aTbxCmd->UpdateButtons();

    UpdateColumn();
    UpdateRow();
    UpdateTable();
    aLbEntries->Hide();
    aWndScenarios->Hide();
    aWndScenarios->SetPosPixel( aLbEntries->GetPosPixel() );

    aContentIdle.SetIdleHdl( LINK( this, ScNavigatorDlg, TimeHdl ) );
    aContentIdle.SetPriority( SchedulerPriority::LOWEST );

    FreeResource();

    aLbEntries->SetAccessibleRelationLabeledBy(aLbEntries.get());
    aTbxCmd->SetAccessibleRelationLabeledBy(aTbxCmd.get());
    aLbDocuments->SetAccessibleName(aStrActiveWin);

    if (pContextWin == nullptr)
    {
        // When the context window is missing then the navigator is
        // displayed in the sidebar and has the whole deck to fill.
        // Therefore hide the button that hides all controls below the
        // top two rows of buttons.
        aTbxCmd->Select(IID_ZOOMOUT);
        aTbxCmd->RemoveItem(aTbxCmd->GetItemPos(IID_ZOOMOUT));
    }
    aLbEntries->SetNavigatorDlgFlag(true);
}

ScNavigatorDlg::~ScNavigatorDlg()
{
    disposeOnce();
}

void ScNavigatorDlg::dispose()
{
    aContentIdle.Stop();

    sal_uInt16 i;
    for ( i=0; i<CTRL_ITEMS; i++ )
        delete ppBoundItems[i];

    delete [] ppBoundItems;
    delete pMarkArea;

    EndListening( *(SfxGetpApp()) );
    EndListening( rBindings );

    aFtCol.disposeAndClear();
    aEdCol.disposeAndClear();
    aFtRow.disposeAndClear();
    aEdRow.disposeAndClear();
    aTbxCmd.disposeAndClear();
    aLbEntries.disposeAndClear();
    aWndScenarios.disposeAndClear();
    aLbDocuments.disposeAndClear();
    vcl::Window::dispose();
}

void ScNavigatorDlg::Resizing( Size& rNewSize )  // Size = Outputsize?
{
    FloatingWindow* pFloat = pContextWin!=nullptr ? pContextWin->GetFloatingWindow() : nullptr;
    if ( pFloat )
    {
        Size aMinOut = pFloat->GetMinOutputSizePixel();

        if ( rNewSize.Width() < aMinOut.Width() )
            rNewSize.Width() = aMinOut.Width();

        if ( eListMode == NAV_LMODE_NONE )
            rNewSize.Height() = aInitSize.Height();
        else
        {
            if ( rNewSize.Height() < aMinOut.Height() )
                rNewSize.Height() = aMinOut.Height();
        }
    }
}

void ScNavigatorDlg::Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect )
{
    if (mbUseStyleSettingsBackground)
    {
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
        Color aBgColor = rStyleSettings.GetFaceColor();
        Wallpaper aBack( aBgColor );

        SetBackground( aBack );
        aFtCol->SetBackground( aBack );
        aFtRow->SetBackground( aBack );
    }
    else
    {
        aFtCol->SetBackground(Wallpaper());
        aFtRow->SetBackground(Wallpaper());
    }

    Window::Paint(rRenderContext, rRect);
}

void ScNavigatorDlg::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( rDCEvt.GetType() == DataChangedEventType::SETTINGS && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        //  toolbox images are exchanged in CommandToolBox::DataChanged
        Invalidate();
    }

    Window::DataChanged( rDCEvt );
}

void ScNavigatorDlg::Resize()
{
    DoResize();
}

void ScNavigatorDlg::DoResize()
{
    Size aNewSize = GetOutputSizePixel();
    long nTotalHeight = aNewSize.Height();

    //  if the navigator is docked, the window is probably at first small generated,
    //  then there is a resize to the actual size -> switch on content

    bool bSmall = ( nTotalHeight <= aInitSize.Height() + SCNAV_MINTOL );
    if ( !bSmall && bFirstBig )
    {
        //  Switch on content again as described in the config

        bFirstBig = false;
        NavListMode eNavMode = NAV_LMODE_AREAS;
        ScNavipiCfg& rCfg = SC_MOD()->GetNavipiCfg();
        NavListMode eLastMode = (NavListMode) rCfg.GetListMode();
        if ( eLastMode == NAV_LMODE_SCENARIOS )
            eNavMode = NAV_LMODE_SCENARIOS;
        SetListMode( eNavMode, false );         // FALSE: do not set the Float size
    }

    //  even if the content is not visible, adapt the size,
    //  so the width fit

    Point aEntryPos = aLbEntries->GetPosPixel();
    Point aListPos = aLbDocuments->GetPosPixel();
    aNewSize.Width() -= 2*nBorderOffset;
    Size aDocSize = aLbDocuments->GetSizePixel();
    aDocSize.Width() = aNewSize.Width();

    if(!bSmall)
    {

        long nListHeight = aLbDocuments->GetSizePixel().Height();
        aNewSize.Height() -= ( aEntryPos.Y() + nListHeight + 2*nBorderOffset );
        if(aNewSize.Height()<0) aNewSize.Height()=0;

        aListPos.Y() = aEntryPos.Y() + aNewSize.Height() + nBorderOffset;

        if(aListPos.Y() > aLbEntries->GetPosPixel().Y())
                            aLbDocuments->SetPosPixel( aListPos );

    }
    aLbEntries->SetSizePixel( aNewSize );
    aWndScenarios->SetSizePixel( aNewSize );
    aLbDocuments->SetSizePixel( aDocSize );

    bool bListMode = (eListMode != NAV_LMODE_NONE);
    if (pContextWin != nullptr)
    {
        FloatingWindow* pFloat = pContextWin->GetFloatingWindow();
        if ( pFloat && bListMode )
            nListModeHeight = nTotalHeight;
    }
}

void ScNavigatorDlg::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>( &rHint );
    if ( pSimpleHint )
    {
        const sal_uInt32 nHintId = pSimpleHint->GetId();

        if ( nHintId == SC_HINT_DOCNAME_CHANGED )
        {
            aLbEntries->ActiveDocChanged();
        }
        else if ( NAV_LMODE_NONE == eListMode )
        {
            //  Table not any more
        }
        else
        {
            switch ( nHintId )
            {
                case SC_HINT_TABLES_CHANGED:
                    aLbEntries->Refresh( ScContentId::TABLE );
                    break;

                case SC_HINT_DBAREAS_CHANGED:
                    aLbEntries->Refresh( ScContentId::DBAREA );
                    break;

                case SC_HINT_AREAS_CHANGED:
                    aLbEntries->Refresh( ScContentId::RANGENAME );
                    break;

                case SC_HINT_DRAW_CHANGED:
                    aLbEntries->Refresh( ScContentId::GRAPHIC );
                    aLbEntries->Refresh( ScContentId::OLEOBJECT );
                    aLbEntries->Refresh( ScContentId::DRAWING );
                    break;

                case SC_HINT_AREALINKS_CHANGED:
                    aLbEntries->Refresh( ScContentId::AREALINK );
                    break;

                //  SFX_HINT_DOCCHANGED not only at document change

                case SC_HINT_NAVIGATOR_UPDATEALL:
                    UpdateAll();
                    break;

                case FID_DATACHANGED:
                case FID_ANYDATACHANGED:
                    aContentIdle.Start();      // Do not search notes immediately
                    break;
                case FID_KILLEDITVIEW:
                    aLbEntries->ObjectFresh( ScContentId::OLEOBJECT );
                    aLbEntries->ObjectFresh( ScContentId::DRAWING );
                    aLbEntries->ObjectFresh( ScContentId::GRAPHIC );
                    break;
                default:
                    break;
            }
        }
    }
    else if ( dynamic_cast<const SfxEventHint*>(&rHint) )
    {
        sal_uLong nEventId = static_cast<const SfxEventHint&>(rHint).GetEventId();
        if ( nEventId == SFX_EVENT_ACTIVATEDOC )
        {
            aLbEntries->ActiveDocChanged();
            UpdateAll();
        }
    }
}

IMPL_LINK_TYPED( ScNavigatorDlg, TimeHdl, Idle*, pIdle, void )
{
    if ( pIdle != &aContentIdle )
        return;

    aLbEntries->Refresh( ScContentId::NOTE );
}

void ScNavigatorDlg::SetDropMode(sal_uInt16 nNew)
{
    nDropMode = nNew;
    aTbxCmd->UpdateButtons();

    ScNavipiCfg& rCfg = SC_MOD()->GetNavipiCfg();
    rCfg.SetDragMode(nDropMode);
}

void ScNavigatorDlg::SetCurrentCell( SCCOL nColNo, SCROW nRowNo )
{
    if ( (nColNo+1 != nCurCol) || (nRowNo+1 != nCurRow) )
    {
        // SID_CURRENTCELL == Item #0 clear cache, so it's possible
        // setting the current cell even in combined areas
        ppBoundItems[0]->ClearCache();

        ScAddress aScAddress( nColNo, nRowNo, 0 );
        OUString aAddr(aScAddress.Format(ScRefFlags::ADDR_ABS));

        bool bUnmark = false;
        if ( GetViewData() )
            bUnmark = !pViewData->GetMarkData().IsCellMarked( nColNo, nRowNo );

        SfxStringItem   aPosItem( SID_CURRENTCELL, aAddr );
        SfxBoolItem     aUnmarkItem( FN_PARAM_1, bUnmark );     // cancel selektion

        rBindings.GetDispatcher()->ExecuteList(SID_CURRENTCELL,
                                  SfxCallMode::SYNCHRON | SfxCallMode::RECORD,
                                  { &aPosItem, &aUnmarkItem });
    }
}

void ScNavigatorDlg::SetCurrentCellStr( const OUString& rName )
{
    ppBoundItems[0]->ClearCache();
    SfxStringItem   aNameItem( SID_CURRENTCELL, rName );

    rBindings.GetDispatcher()->ExecuteList(SID_CURRENTCELL,
                              SfxCallMode::SYNCHRON | SfxCallMode::RECORD,
                              { &aNameItem });
}

void ScNavigatorDlg::SetCurrentTable( SCTAB nTabNo )
{
    if ( nTabNo != nCurTab )
    {
        // Table for basic is base-1
        SfxUInt16Item aTabItem( SID_CURRENTTAB, static_cast<sal_uInt16>(nTabNo) + 1 );
        rBindings.GetDispatcher()->ExecuteList(SID_CURRENTTAB,
                                  SfxCallMode::SYNCHRON | SfxCallMode::RECORD,
                                  { &aTabItem });
    }
}

void ScNavigatorDlg::SetCurrentTableStr( const OUString& rName )
{
    if (!GetViewData()) return;

    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB nCount = pDoc->GetTableCount();
    OUString aTabName;
    SCTAB nLastSheet = 0;

    for (SCTAB i = 0; i<nCount; i++)
    {
        pDoc->GetName(i, aTabName);
        if (aTabName.equals(rName))
        {
            // Check if this is a Scenario sheet and if so select the sheet
            // where it belongs to, which is the previous non-Scenario sheet.
            if (pDoc->IsScenario(i))
            {
                SetCurrentTable(nLastSheet);
                return;
            }
            else
            {
                SetCurrentTable(i);
                return;
            }
        }
        else
        {
            if (!pDoc->IsScenario(i))
                nLastSheet = i;
        }
    }
}

void ScNavigatorDlg::SetCurrentObject( const OUString& rName )
{
    SfxStringItem aNameItem( SID_CURRENTOBJECT, rName );
    rBindings.GetDispatcher()->ExecuteList( SID_CURRENTOBJECT,
                              SfxCallMode::SYNCHRON | SfxCallMode::RECORD,
                              { &aNameItem });
}

void ScNavigatorDlg::SetCurrentDoc( const OUString& rDocName )        // activate
{
    SfxStringItem aDocItem( SID_CURRENTDOC, rDocName );
    rBindings.GetDispatcher()->ExecuteList( SID_CURRENTDOC,
                              SfxCallMode::SYNCHRON | SfxCallMode::RECORD,
                              { &aDocItem });
}

ScTabViewShell* ScNavigatorDlg::GetTabViewShell()
{
    return dynamic_cast<ScTabViewShell*>( SfxViewShell::Current()  );
}

ScNavigatorSettings* ScNavigatorDlg::GetNavigatorSettings()
{
    //  Don't store the settings pointer here, because the settings belong to
    //  the view, and the view may be closed while the navigator is open (reload).
    //  If the pointer is cached here again later for performance reasons, it has to
    //  be forgotten when the view is closed.

    ScTabViewShell* pViewSh = GetTabViewShell();
    return pViewSh ? pViewSh->GetNavigatorSettings() : nullptr;
}

bool ScNavigatorDlg::GetViewData()
{
    ScTabViewShell* pViewSh = GetTabViewShell();
    pViewData = pViewSh ? &pViewSh->GetViewData() : nullptr;

    return ( pViewData != nullptr );
}

void ScNavigatorDlg::UpdateColumn( const SCCOL* pCol )
{
    if ( pCol )
        nCurCol = *pCol;
    else if ( GetViewData() )
        nCurCol = pViewData->GetCurX() + 1;

    aEdCol->SetCol( nCurCol );
    CheckDataArea();
}

void ScNavigatorDlg::UpdateRow( const SCROW* pRow )
{
    if ( pRow )
        nCurRow = *pRow;
    else if ( GetViewData() )
        nCurRow = pViewData->GetCurY() + 1;

    aEdRow->SetRow( nCurRow );
    CheckDataArea();
}

void ScNavigatorDlg::UpdateTable( const SCTAB* pTab )
{
    if ( pTab )
        nCurTab = *pTab;
    else if ( GetViewData() )
        nCurTab = pViewData->GetTabNo();

    CheckDataArea();
}

void ScNavigatorDlg::UpdateAll()
{
    switch ( eListMode )
    {
        case NAV_LMODE_DOCS:
        case NAV_LMODE_DBAREAS:
        case NAV_LMODE_AREAS:
            aLbEntries->Refresh();
            break;

        case NAV_LMODE_NONE:
            //! ???
            break;

        default:
            break;
    }

    aContentIdle.Stop();       // not again
}

void ScNavigatorDlg::SetListMode( NavListMode eMode, bool bSetSize )
{
    if ( eMode != eListMode )
    {
        if ( eMode != NAV_LMODE_NONE )
            bFirstBig = false;              // do not switch automatically any more

        eListMode = eMode;

        switch ( eMode )
        {
            case NAV_LMODE_NONE:
                ShowList( false, bSetSize );
                break;

            case NAV_LMODE_AREAS:
            case NAV_LMODE_DBAREAS:
            case NAV_LMODE_DOCS:
                aLbEntries->Refresh();
                ShowList( true, bSetSize );
                break;

            case NAV_LMODE_SCENARIOS:
                ShowScenarios( bSetSize );
                break;
        }

        aTbxCmd->UpdateButtons();

        if ( eMode != NAV_LMODE_NONE )
        {
            ScNavipiCfg& rCfg = SC_MOD()->GetNavipiCfg();
            rCfg.SetListMode( (sal_uInt16) eMode );
        }
    }

    if ( pMarkArea )
        UnmarkDataArea();
}

void ScNavigatorDlg::ShowList( bool bShow, bool bSetSize )
{
    FloatingWindow* pFloat = pContextWin!=nullptr ? pContextWin->GetFloatingWindow() : nullptr;
    Size aSize = GetParent()->GetOutputSizePixel();

    if ( bShow )
    {
        Size aMinSize = aInitSize;

        aMinSize.Height() += nInitListHeight;
        if ( pFloat )
            pFloat->SetMinOutputSizePixel( aMinSize );
        aSize.Height() = nListModeHeight;
        aLbEntries->Show();
        aLbDocuments->Show();
    }
    else
    {
        if ( pFloat )
        {
            pFloat->SetMinOutputSizePixel( aInitSize );
            nListModeHeight = aSize.Height();
        }
        aSize.Height() = aInitSize.Height();
        aLbEntries->Hide();
        aLbDocuments->Hide();
    }
    aWndScenarios->Hide();

    if ( pFloat )
    {
        if ( bSetSize )
            pFloat->SetOutputSizePixel( aSize );
    }
    else
    {
        SfxNavigator* pNav = dynamic_cast<SfxNavigator*>(GetParent());
        if (pNav != nullptr)
        {
            Size aFloating = pNav->GetFloatingSize();
            aFloating.Height() = aSize.Height();
            pNav->SetFloatingSize( aFloating );
        }
    }
}

void ScNavigatorDlg::ShowScenarios( bool bSetSize )
{
    FloatingWindow* pFloat = pContextWin!=nullptr ? pContextWin->GetFloatingWindow() : nullptr;
    Size aSize = GetParent()->GetOutputSizePixel();

    Size aMinSize = aInitSize;
    aMinSize.Height() += nInitListHeight;
    if ( pFloat )
        pFloat->SetMinOutputSizePixel( aMinSize );
    aSize.Height() = nListModeHeight;

    rBindings.Invalidate( SID_SELECT_SCENARIO );
    rBindings.Update( SID_SELECT_SCENARIO );

    aWndScenarios->Show();
    aLbDocuments->Show();
    aLbEntries->Hide();

    if ( pFloat )
    {
        if ( bSetSize )
            pFloat->SetOutputSizePixel( aSize );
    }
    else
    {
        SfxNavigator* pNav = static_cast<SfxNavigator*>(GetParent());
        Size aFloating = pNav->GetFloatingSize();
        aFloating.Height() = aSize.Height();
        pNav->SetFloatingSize( aFloating );
    }
}

//      documents for Dropdown-Listbox

void ScNavigatorDlg::GetDocNames( const OUString* pManualSel )
{
    aLbDocuments->Clear();
    aLbDocuments->SetUpdateMode( false );

    ScDocShell* pCurrentSh = dynamic_cast<ScDocShell*>( SfxObjectShell::Current()  );

    OUString aSelEntry;
    SfxObjectShell* pSh = SfxObjectShell::GetFirst();
    while ( pSh )
    {
        if ( dynamic_cast<const ScDocShell*>( pSh) !=  nullptr )
        {
            OUString aName = pSh->GetTitle();
            OUString aEntry = aName;
            if (pSh == pCurrentSh)
                aEntry += aStrActive;
            else
                aEntry += aStrNotActive;
            aLbDocuments->InsertEntry( aEntry );

            if ( pManualSel ? ( aName == *pManualSel )
                            : ( pSh == pCurrentSh ) )
                aSelEntry = aEntry;                     // complete entry for selection
        }

        pSh = SfxObjectShell::GetNext( *pSh );
    }

    aLbDocuments->InsertEntry( aStrActiveWin );

    OUString aHidden =  aLbEntries->GetHiddenTitle();
    if (!aHidden.isEmpty())
    {
        OUString aEntry = aHidden;
        aEntry += aStrHidden;
        aLbDocuments->InsertEntry( aEntry );

        if ( pManualSel && aHidden == *pManualSel )
            aSelEntry = aEntry;
    }

    aLbDocuments->SetUpdateMode( true );

    aLbDocuments->SelectEntry( aSelEntry );
}

void ScNavigatorDlg::MarkDataArea()
{
    ScTabViewShell* pViewSh = GetTabViewShell();

    if ( pViewSh )
    {
        if ( !pMarkArea )
            pMarkArea = new ScArea;

        pViewSh->MarkDataArea();
        ScRange aMarkRange;
        pViewSh->GetViewData().GetMarkData().GetMarkArea(aMarkRange);
        pMarkArea->nColStart = aMarkRange.aStart.Col();
        pMarkArea->nRowStart = aMarkRange.aStart.Row();
        pMarkArea->nColEnd = aMarkRange.aEnd.Col();
        pMarkArea->nRowEnd = aMarkRange.aEnd.Row();
        pMarkArea->nTab = aMarkRange.aStart.Tab();
    }
}

void ScNavigatorDlg::UnmarkDataArea()
{
    ScTabViewShell* pViewSh = GetTabViewShell();

    if ( pViewSh )
    {
        pViewSh->Unmark();
        DELETEZ( pMarkArea );
    }
}

void ScNavigatorDlg::CheckDataArea()
{
    if ( aTbxCmd->IsItemChecked( IID_DATA ) && pMarkArea )
    {
        if (   nCurTab   != pMarkArea->nTab
            || nCurCol <  pMarkArea->nColStart+1
            || nCurCol >  pMarkArea->nColEnd+1
            || nCurRow <  pMarkArea->nRowStart+1
            || nCurRow >  pMarkArea->nRowEnd+1 )
        {
            aTbxCmd->SetItemState( IID_DATA, TriState(TRISTATE_TRUE) );
            aTbxCmd->Select( IID_DATA );
        }
    }
}

void ScNavigatorDlg::StartOfDataArea()
{
    //  pMarkArea evaluate ???

    if ( GetViewData() )
    {
        ScMarkData& rMark = pViewData->GetMarkData();
        ScRange aMarkRange;
        rMark.GetMarkArea( aMarkRange );

        SCCOL nCol = aMarkRange.aStart.Col();
        SCROW nRow = aMarkRange.aStart.Row();

        if ( (nCol+1 != aEdCol->GetCol()) || (nRow+1 != aEdRow->GetRow()) )
            SetCurrentCell( nCol, nRow );
    }
}

void ScNavigatorDlg::EndOfDataArea()
{
    //  pMarkArea evaluate ???

    if ( GetViewData() )
    {
        ScMarkData& rMark = pViewData->GetMarkData();
        ScRange aMarkRange;
        rMark.GetMarkArea( aMarkRange );

        SCCOL nCol = aMarkRange.aEnd.Col();
        SCROW nRow = aMarkRange.aEnd.Row();

        if ( (nCol+1 != aEdCol->GetCol()) || (nRow+1 != aEdRow->GetRow()) )
            SetCurrentCell( nCol, nRow );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
