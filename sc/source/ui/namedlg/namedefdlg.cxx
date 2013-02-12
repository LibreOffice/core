/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 *  Copyright (C) 2011 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer) ]
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "namedefdlg.hxx"

#include <vcl/msgbox.hxx>
#include <sfx2/app.hxx>

#include "document.hxx"
#include "globstr.hrc"
#include "globalnames.hxx"
#include "rangenam.hxx"
#include "reffact.hxx"
#include "undorangename.hxx"
#include "tabvwsh.hxx"
#include "tokenarray.hxx"

// defines -------------------------------------------------------------------

#define ABS_SREF          SCA_VALID \
    | SCA_COL_ABSOLUTE | SCA_ROW_ABSOLUTE | SCA_TAB_ABSOLUTE
#define ABS_DREF          ABS_SREF \
    | SCA_COL2_ABSOLUTE | SCA_ROW2_ABSOLUTE | SCA_TAB2_ABSOLUTE
#define ABS_SREF3D      ABS_SREF | SCA_TAB_3D
#define ABS_DREF3D      ABS_DREF | SCA_TAB_3D

ScNameDefDlg::ScNameDefDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
        ScViewData* pViewData, std::map<rtl::OUString, ScRangeName*> aRangeMap,
        const ScAddress& aCursorPos, const bool bUndo ) :
    ScAnyRefDlg( pB, pCW, pParent, RID_SCDLG_NAMES_DEFINE ),
    maEdName( this, ScResId( ED_NAME ) ),
    maEdRange( this, this, ScResId( ED_RANGE ) ),
    maRbRange( this, ScResId( RB_RANGE ), &maEdRange, this ),
    maLbScope( this, ScResId( LB_SCOPE ) ),
    maBtnMore( this, ScResId( BTN_MORE ) ),
    maBtnRowHeader( this, ScResId( BTN_ROWHEADER ) ),
    maBtnColHeader( this, ScResId( BTN_COLHEADER ) ),
    maBtnPrintArea( this, ScResId( BTN_PRINTAREA ) ),
    maBtnCriteria( this, ScResId( BTN_CRITERIA ) ),
    maBtnAdd( this, ScResId( BTN_ADD ) ),
    maBtnCancel( this, ScResId( BTN_CANCEL ) ),
    maFtInfo( this, ScResId( FT_INFO ) ),
    maFtName( this, ScResId( FT_NAME ) ),
    maFtRange( this, ScResId( FT_RANGE ) ),
    maFtScope( this, ScResId( FT_SCOPE ) ),
    maFlDiv( this, ScResId( FL_DIV ) ),
    mbUndo( bUndo ),
    mpDoc( pViewData->GetDocument() ),
    mpDocShell ( pViewData->GetDocShell() ),
    maCursorPos( aCursorPos ),

    maGlobalNameStr  ( ScGlobal::GetRscString(STR_GLOBAL_SCOPE) ),
    maErrInvalidNameStr( ScGlobal::GetRscString(STR_ERR_NAME_INVALID)),
    maErrNameInUse   ( ScGlobal::GetRscString(STR_ERR_NAME_EXISTS)),
    maStrInfoDefault ( SC_RESSTR(STR_DEFAULT_INFO)),
    maRangeMap( aRangeMap )
{
    // Initialize scope list.
    maLbScope.InsertEntry(maGlobalNameStr);
    maLbScope.SelectEntryPos(0);
    SCTAB n = mpDoc->GetTableCount();
    for (SCTAB i = 0; i < n; ++i)
    {
        rtl::OUString aTabName;
        mpDoc->GetName(i, aTabName);
        maLbScope.InsertEntry(aTabName);
    }

    maBtnCancel.SetClickHdl( LINK( this, ScNameDefDlg, CancelBtnHdl));
    maBtnAdd.SetClickHdl( LINK( this, ScNameDefDlg, AddBtnHdl ));
    maBtnMore.SetClickHdl( LINK( this, ScNameDefDlg, MoreBtnHdl ));
    maEdName.SetModifyHdl( LINK( this, ScNameDefDlg, NameModifyHdl ));
    maEdRange.SetGetFocusHdl( LINK( this, ScNameDefDlg, AssignGetFocusHdl ) );

    maFtInfo.SetStyle(WB_VCENTER);
    maFtInfo.SetText(maStrInfoDefault);

    maBtnAdd.Disable(); // empty name is invalid

    maBtnRowHeader.Hide();
    maBtnColHeader.Hide();
    maBtnCriteria.Hide();
    maBtnPrintArea.Hide();

    FreeResource();

    String aAreaStr;
    ScRange aRange;

    pViewData->GetSimpleArea( aRange );
    aRange.Format( aAreaStr, ABS_DREF3D, mpDoc,
            ScAddress::Details(mpDoc->GetAddressConvention(), 0, 0) );

    maEdRange.SetText( aAreaStr );

    Selection aCurSel = Selection( 0, SELECTION_MAX );
    maEdName.GrabFocus();
    maEdName.SetSelection( aCurSel );
}

void ScNameDefDlg::CancelPushed()
{
    if (mbUndo)
        Close();
    else
    {
        ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
        pViewSh->SwitchBetweenRefDialogs(this);
    }
}

bool ScNameDefDlg::IsFormulaValid()
{
    ScCompiler aComp( mpDoc, maCursorPos);
    aComp.SetGrammar( mpDoc->GetGrammar() );
    ScTokenArray* pCode = aComp.CompileString(maEdRange.GetText());
    if (pCode->GetCodeError())
    {
        //TODO: info message
        delete pCode;
        return false;
    }
    else
    {
        delete pCode;
        return true;
    }
}

bool ScNameDefDlg::IsNameValid()
{
    rtl::OUString aScope = maLbScope.GetSelectEntry();
    rtl::OUString aName = maEdName.GetText();

    ScRangeName* pRangeName = NULL;
    if(aScope == maGlobalNameStr)
    {
        pRangeName = maRangeMap.find(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(STR_GLOBAL_RANGE_NAME)))->second;
    }
    else
    {
        pRangeName = maRangeMap.find(aScope)->second;
    }

    maFtInfo.SetControlBackground(GetSettings().GetStyleSettings().GetDialogColor());
    if ( aName.isEmpty() )
    {
        maBtnAdd.Disable();
        maFtInfo.SetText(maStrInfoDefault);
        return false;
    }
    else if (!ScRangeData::IsNameValid( aName, mpDoc ))
    {
        maFtInfo.SetControlBackground(GetSettings().GetStyleSettings().GetHighlightColor());
        maFtInfo.SetText(maErrInvalidNameStr);
        maBtnAdd.Disable();
        return false;
    }
    else if (pRangeName->findByUpperName(ScGlobal::pCharClass->uppercase(aName)))
    {
        maFtInfo.SetControlBackground(GetSettings().GetStyleSettings().GetHighlightColor());
        maFtInfo.SetText(maErrNameInUse);
        maBtnAdd.Disable();
        return false;
    }

    if (!IsFormulaValid())
    {
        maFtInfo.SetControlBackground(GetSettings().GetStyleSettings().GetHighlightColor());
        maBtnAdd.Disable();
        return false;
    }

    maFtInfo.SetText(maStrInfoDefault);
    maBtnAdd.Enable();
    return true;
}

void ScNameDefDlg::AddPushed()
{
    rtl::OUString aScope = maLbScope.GetSelectEntry();
    rtl::OUString aName = maEdName.GetText();
    rtl::OUString aExpression = maEdRange.GetText();

    if (aName.isEmpty())
    {
        return;
    }
    if (aScope.isEmpty())
    {
        return;
    }

    ScRangeName* pRangeName = NULL;
    if(aScope == maGlobalNameStr)
    {
        pRangeName = maRangeMap.find(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(STR_GLOBAL_RANGE_NAME)))->second;
    }
    else
    {
        pRangeName = maRangeMap.find(aScope)->second;
    }
    if (!pRangeName)
        return;

    if (!IsNameValid()) //should not happen, but make sure we don't break anything
        return;
    else
    {
        if ( mpDoc )
        {
            ScRangeData*    pNewEntry   = NULL;
            RangeType       nType       = RT_NAME;

            pNewEntry = new ScRangeData( mpDoc,
                    aName,
                    aExpression,
                    maCursorPos,
                    nType );
            if (pNewEntry)
            {
                nType = nType
                    | (maBtnRowHeader .IsChecked() ? RT_ROWHEADER  : RangeType(0))
                    | (maBtnColHeader .IsChecked() ? RT_COLHEADER  : RangeType(0))
                    | (maBtnPrintArea .IsChecked() ? RT_PRINTAREA  : RangeType(0))
                    | (maBtnCriteria  .IsChecked() ? RT_CRITERIA   : RangeType(0));
                pNewEntry->AddType(nType);
            }

            // aExpression valid?
            if ( 0 == pNewEntry->GetErrCode() )
            {
                if ( !pRangeName->insert( pNewEntry ) )
                    pNewEntry = NULL;

                if (mbUndo)
                {
                    // this means we called directly through the menu

                    SCTAB nTab;
                    // if no table with that name is found, assume global range name
                    if (!mpDoc->GetTable(aScope, nTab))
                        nTab = -1;

                    mpDocShell->GetUndoManager()->AddUndoAction(
                            new ScUndoAddRangeData( mpDocShell, pNewEntry, nTab) );

                    // set table stream invalid, otherwise RangeName won't be saved if no other
                    // call invalidates the stream
                    if (nTab != -1)
                        mpDoc->SetStreamValid(nTab, false);
                    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREAS_CHANGED ) );
                    Close();
                }
                else
                {
                    maName = aName;
                    maScope = aScope;
                    ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
                    pViewSh->SwitchBetweenRefDialogs(this);
                }
            }
            else
            {
                delete pNewEntry;
                Selection aCurSel = Selection( 0, SELECTION_MAX );
                maEdRange.GrabFocus();
                maEdRange.SetSelection( aCurSel );
            }
        }
    }
}

void ScNameDefDlg::GetNewData(rtl::OUString& rName, rtl::OUString& rScope)
{
    rName = maName;
    rScope = maScope;
}

sal_Bool ScNameDefDlg::IsRefInputMode() const
{
    return maEdRange.IsEnabled();
}

void ScNameDefDlg::RefInputDone( sal_Bool bForced)
{
    ScAnyRefDlg::RefInputDone(bForced);
    IsNameValid();
}

void ScNameDefDlg::SetReference( const ScRange& rRef, ScDocument* pDocP )
{
    if ( maEdRange.IsEnabled() )
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart(&maEdRange);
        String aRefStr;
        rRef.Format( aRefStr, ABS_DREF3D, pDocP,
                ScAddress::Details(pDocP->GetAddressConvention(), 0, 0) );
        maEdRange.SetRefString( aRefStr );
    }
}

sal_Bool ScNameDefDlg::Close()
{
    return DoClose( ScNameDefDlgWrapper::GetChildWindowId() );
}

void ScNameDefDlg::SetActive()
{
    maEdRange.GrabFocus();
    RefInputDone();
}

namespace {

void MoveWindow( Window& rButton, long nPixel)
{
    Point aPoint = rButton.GetPosPixel();
    aPoint.Y() += nPixel;
    rButton.SetPosPixel(aPoint);
}

}

void ScNameDefDlg::MorePushed()
{
    Size nSize = GetSizePixel();

    //depending on the state of the button, move all elements below up/down
    long nPixel = 65;
    if (!maBtnMore.GetState())
    {
        nPixel *= -1;
        maBtnRowHeader.Hide();
        maBtnColHeader.Hide();
        maBtnPrintArea.Hide();
        maBtnCriteria.Hide();
    }
    else
    {
        maBtnRowHeader.Show();
        maBtnColHeader.Show();
        maBtnPrintArea.Show();
        maBtnCriteria.Show();
    }
    nSize.Height() += nPixel;
    SetSizePixel(nSize);
    MoveWindow(maBtnAdd, nPixel);
    MoveWindow(maBtnCancel, nPixel);
    MoveWindow(maFlDiv, nPixel);
}

IMPL_LINK_NOARG(ScNameDefDlg, CancelBtnHdl)
{
    CancelPushed();
    return 0;
}

IMPL_LINK_NOARG(ScNameDefDlg, AddBtnHdl)
{
    AddPushed();
    return 0;
};

IMPL_LINK_NOARG(ScNameDefDlg, NameModifyHdl)
{
    IsNameValid();
    return 0;
}

IMPL_LINK_NOARG(ScNameDefDlg, AssignGetFocusHdl)
{
    IsNameValid();
    return 0;
}

IMPL_LINK_NOARG(ScNameDefDlg, MoreBtnHdl)
{
    MorePushed();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
